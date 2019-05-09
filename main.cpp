//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"
#include "sprites.h"
//External dependencies
#define GLFW_DLL

#include <SOIL.h>
#include <GLFW/glfw3.h>
#include <random>
#include <cmath>
#include <vector>
#include <algorithm>
#include <string>
#include <random>
#include <ctime>
#include <list>
//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>

using std::vector;
using std::string;
using std::list;
using namespace LiteMath;
static const GLsizei WIDTH = 640, HEIGHT = 640; //размеры окна

GLfloat lastX = WIDTH / 2, lastY = HEIGHT / 2;

GLfloat yaw = -M_PI_2;
GLfloat pitch = 0.0f;
bool firstMouse = false;
float3 cameraPos = float3(0.0f, 0.0f, 3.0f);

float3 cameraFront = normalize(float3(0.0f, 0.0f, -1.0f));
float3 cameraUp = normalize(float3(0.0f, 1.0f, 0.0f));

float2 cursorPos;

bool keys[1024];

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float score = 0;
float health = 100;

bool cabine = false;

std::mt19937 gen(time(0));

list<Bullet> temp_bullets;
float4x4 projection;
float4x4 proj_inv;
float4x4 view;
float4x4 view_inv;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void do_movement();
inline void speed_control() {
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void destroy_enemies(vector<Enemy> &enemies);
void update_trash(vector<float3> &trash);
void update_health();

inline float2 normalize_cursor(double x, double y){
    return float2(2 * x / WIDTH - 1.0, 2 * (HEIGHT - y) / HEIGHT - 1.0);
}

//unsigned int loadCubemap(vector<std::string> faces);

unsigned int loadTexture(char const *path);

//void loadModel(const aiScene *loaded_obj, std::vector<GLfloat> &mesh, std::vector<GLfloat> &tex);

void draw_text_info(ShaderProgram &font_shader, int sc, int hea);

int initGL() {
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return 0;
}

int main(int argc, char **argv) {
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Shooter", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetKeyCallback(window, key_callback);
    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    GLFWimage cursor;
    cursor.pixels = SOIL_load_image("../textures/cursor.png", &cursor.width, &cursor.height, 0, 0);
    glfwSetCursor(window, glfwCreateCursor(&cursor, 10, 10));
    if (initGL() != 0)
        return -1;

    glViewport(0, 0, WIDTH, HEIGHT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    std::unordered_map<GLenum, std::string> shaders;

    shaders[GL_VERTEX_SHADER] = "vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
    ShaderProgram objects_shader(shaders);

    shaders[GL_VERTEX_SHADER] = "sky_vs.glsl";
    shaders[GL_FRAGMENT_SHADER] = "sky_fs.glsl";
    ShaderProgram sky_shader(shaders);

    shaders[GL_VERTEX_SHADER] = "sprite_vs.glsl";
    shaders[GL_FRAGMENT_SHADER] = "sprite_fs.glsl";
    ShaderProgram sprite_shader(shaders);

    shaders[GL_VERTEX_SHADER] = "font_vs.glsl";
    shaders[GL_FRAGMENT_SHADER] = "font_fs.glsl";
    ShaderProgram font_shader(shaders);

//    shaders[GL_VERTEX_SHADER] = "ship_vs.glsl";
//    shaders[GL_VERTEX_SHADER] = "ship_vs.glsl";
//    shaders[GL_FRAGMENT_SHADER] = "ship_fs.glsl";
//    ShaderProgram ship_shader(shaders);

    shaders[GL_VERTEX_SHADER] = "fog_vs.glsl";
    //shaders[GL_GEOMETRY_SHADER] = "fog_gs.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fog_fs.glsl";
    ShaderProgram fog_shader(shaders);

    shaders[GL_VERTEX_SHADER] = "lines_vs.glsl";
    shaders[GL_GEOMETRY_SHADER] = "lines_gs.glsl";
    shaders[GL_FRAGMENT_SHADER] = "lines_fs.glsl";
    ShaderProgram lines_shader(shaders);

    glfwSwapInterval(1); // force 60 frames per second

    // Plane VAO
    float plane_vertices[] = {
            // positions          // texture Coords
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    };

    GLuint planeVBO, planeVAO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);

    GLuint planeLocation = 0;
    glEnableVertexAttribArray(planeLocation);
    glVertexAttribPointer(planeLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *) (0));

    GLuint planetexLocation = 1;
    glEnableVertexAttribArray(planetexLocation);
    glVertexAttribPointer(planetexLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          (GLvoid *) (3 * sizeof(GLfloat)));
    glBindVertexArray(0);

    // Sky VAO
    float sky_vertices[] = {
            // positions
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f
    };
    GLuint skyVBO, skyVAO;
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sky_vertices), sky_vertices, GL_STATIC_DRAW);

    GLuint skyLocation = 0;
    glEnableVertexAttribArray(skyLocation);
    glVertexAttribPointer(skyLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *) (0));

    GLuint skyTexture = 1;
    glEnableVertexAttribArray(skyTexture);
    glVertexAttribPointer(skyTexture, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *) (3 * sizeof(GLfloat)));
    glBindVertexArray(0);

    // Trash VAO
    float trash_vectices[] = {
            0.0f, 0.0f, 0.0f
    };
    GLuint trashVBO, trashVAO;
    glGenVertexArrays(1, &trashVAO);
    glGenBuffers(1, &trashVBO);
    glBindVertexArray(trashVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trashVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(trash_vectices), trash_vectices, GL_STATIC_DRAW);

    GLuint trashLocation = 0;
    glEnableVertexAttribArray(trashLocation);
    glVertexAttribPointer(trashLocation, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *) (0));

    glBindVertexArray(0);

    /// Load and create a texture
    GLuint cockpit_tex = loadTexture("../textures/cockpit.png");
    GLuint fog_tex = loadTexture("../textures/fog.png");

    Sprite explosion1(loadTexture("../textures/boom.png"), 9, 9, 81);
    Sprite asteroid1(loadTexture("../textures/asteroid1.png"), 8, 8, 32);
    Sprite asteroid2(loadTexture("../textures/asteroid2.png"), 5, 6, 30);

    Sprite ship1(loadTexture("../textures/ship1.png"), 1, 1, 1);
    Sprite ship2(loadTexture("../textures/ship2.png"), 1, 1, 1);
    Sprite ship3(loadTexture("../textures/ship3.png"), 1, 1, 1);
    Sprite ship4(loadTexture("../textures/ship4.png"), 1, 1, 1);

    GLuint bgt = loadTexture("../textures/background.jpg");

    GLuint digits_tex = loadTexture("../textures/digits.png");
    GLuint score_tex = loadTexture("../textures/score.png");
    GLuint health_tex = loadTexture("../textures/health.png");


    /// Presetting uniforms
    projection = transpose(projectionMatrixTransposed(45.0f, (GLfloat) WIDTH / (GLfloat) HEIGHT, 0.1f, 100.0f));
    proj_inv = inverse4x4(projection);
    view = float4x4();

    objects_shader.StartUseShader();
    objects_shader.SetUniform("projection", projection);
    objects_shader.SetUniform("Texture", 0);
    objects_shader.StopUseShader();

    sprite_shader.StartUseShader();
    sprite_shader.SetUniform("projection", projection);
    sprite_shader.SetUniform("Texture", 0);
    sprite_shader.SetUniform("boom", 1);
    sprite_shader.StopUseShader();

    sky_shader.StartUseShader();
    sky_shader.SetUniform("projection", projection);
    sky_shader.SetUniform("view", view);
    sky_shader.SetUniform("bg", 0);
    sky_shader.StopUseShader();

    lines_shader.StartUseShader();
    lines_shader.SetUniform("projection", projection);
    lines_shader.StopUseShader();

    fog_shader.StartUseShader();
    fog_shader.SetUniform("projection", projection);
    fog_shader.StopUseShader();

    font_shader.StartUseShader();
    font_shader.SetUniform("projection", projection);
    font_shader.SetUniform("Texture", 0);
    font_shader.SetUniform("score", 1);
    font_shader.SetUniform("health", 2);
    font_shader.StopUseShader();


    std::vector<Enemy> enemies = {
            Enemy(asteroid1, explosion1, false),
            Enemy(asteroid1, explosion1, false),
            Enemy(asteroid2, explosion1, false),
            Enemy(asteroid2, explosion1, false),
            Enemy(ship1, explosion1, true),
            Enemy(ship1, explosion1, true),
            Enemy(ship2, explosion1, true),
            Enemy(ship2, explosion1, true),
            Enemy(ship3, explosion1, true),
            Enemy(ship3, explosion1, true),
    };

    std::vector<float3> trash_points;
    {
        std::vector<float> points{-10.0, -7.0, -5.0, -2.0, -1.0, 1.0, 2.0, 5.0, 7.0, 10.0};
        std::normal_distribution<> dis(-40.0, -70.0);
        for (const auto &i : points) {
            for (const auto &j : points) {
                trash_points.emplace_back(i, j, dis(gen));
            }
        }
    }

    float4x4 model;
    std::vector<Fog> fogs(30, Fog(fog_tex));

    while (!glfwWindowShouldClose(window) && health > 0) {
        glfwPollEvents();

        // Camera movements
        speed_control();
        do_movement();

        // Clear window
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        view = transpose(lookAtTransposed(cameraPos, cameraPos + cameraFront, cameraUp));
        view_inv = inverse4x4(view);

        /// Sky
        glDepthFunc(GL_LEQUAL);
        sky_shader.StartUseShader();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bgt);
        glBindVertexArray(skyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        sky_shader.StopUseShader();
        glDepthFunc(GL_LESS);

        /// Fog
        fog_shader.StartUseShader();
        fog_shader.SetUniform("view", view);
        std::sort(fogs.begin(), fogs.end(),
                  [](const Fog &p1, Fog &p2) {
                      return p1.position.z < p2.position.z;
                  });
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fog_tex);
        glBindVertexArray(planeVAO);

        for (auto &flying_fog : fogs){
            if (!flying_fog.actual){
                flying_fog = Fog(fog_tex);
                continue;
            }
            model = rotate_Z_4x4(lastFrame * flying_fog.speed / 3);
            model.row[0].w = flying_fog.position.x;
            model.row[1].w = flying_fog.position.y;
            model.row[2].w = flying_fog.position.z;
            fog_shader.SetUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            flying_fog.move();
        }
        fog_shader.StopUseShader();
        glBindVertexArray(0);

        /// Sprites
        sprite_shader.StartUseShader();
        sprite_shader.SetUniform("view", view);

        glBindVertexArray(planeVAO);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, explosion1.texture);
        std::sort(enemies.begin(), enemies.end(),
                  [](const Enemy &p1, Enemy &p2) {
                      return p1.position.z < p2.position.z;
                  });
        destroy_enemies(enemies);
        time_t current = time(nullptr);
        for (auto &enemy : enemies) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, enemy.texture);
            model = translate4x4(enemy.position);
            sprite_shader.SetUniform("model", model);
            sprite_shader.SetUniform("is_alive", enemy.is_alive);
            sprite_shader.SetUniform("animation", enemy.animate());
            glDrawArrays(GL_TRIANGLES, 0, 6);
            if (!enemy.is_alive && (current - enemy.time_of_death) > 5) {
                enemy.respawn();
            }
            if (enemy.is_ship && enemy.is_alive && (current - enemy.time_of_shoot) > 3 && enemy.position.z > -30){
                temp_bullets.push_back(enemy.shoot());
            }
        }
        glBindVertexArray(0);
        sprite_shader.StopUseShader();


        /// Flying lines
        lines_shader.StartUseShader();
        lines_shader.SetUniform("view", view);

        glBindVertexArray(trashVAO);

        lines_shader.SetUniform("direction", float3(0.0, 0.0, 1.0));
        lines_shader.SetUniform("incolor", float3(0.0, 0.5, 0.75));
        for (auto &trash : trash_points){
            model = translate4x4(trash);
            lines_shader.SetUniform("model", model);
            glDrawArrays(GL_POINTS, 0, 1);
        }
        update_trash(trash_points);

        /// Bullets

        for (auto bull = temp_bullets.begin(); bull != temp_bullets.end();){
            if (bull->actual){
                model = translate4x4(bull->position);
                if (bull->enemy_strike){
                    lines_shader.SetUniform("incolor", float3(1.0, 0.0, 0.0));
                } else {
                    lines_shader.SetUniform("incolor", float3(0.0, 1.0, 0.0));
                }
                lines_shader.SetUniform("model", model);
                lines_shader.SetUniform("direction", bull->direction);
                glDrawArrays(GL_POINTS, 0, 1);
                bull->move();
                ++bull;
            } else {
                auto tmp = bull;
                ++tmp;
                temp_bullets.erase(bull);
                bull = tmp;
            }
        }

        lines_shader.StopUseShader();

        glBindVertexArray(0);

        /// Cockpit
        objects_shader.StartUseShader();
        objects_shader.SetUniform("view", view);

        glActiveTexture(GL_TEXTURE0);
        if (cabine){
            glBindTexture(GL_TEXTURE_2D, cockpit_tex);
            model = translate4x4(cameraPos + float3(0.0f, 0.0f, -1.1f));
        } else {
            glBindTexture(GL_TEXTURE_2D, ship4.texture);
            model = rotate_X_4x4(M_PI_2 + 0.1);
            model.row[0].w = cameraPos.x;
            model.row[1].w = cameraPos.y - 0.3;
            model.row[2].w = cameraPos.z - 1.1;
        }
        glBindVertexArray(planeVAO);
        objects_shader.SetUniform("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        objects_shader.StopUseShader();

        /// Score-health info

        font_shader.StartUseShader();
        font_shader.SetUniform("view", view);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, digits_tex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, score_tex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, health_tex);

        glBindVertexArray(planeVAO);
        draw_text_info(font_shader, int(score), int(health));

        glBindVertexArray(0);
        font_shader.StopUseShader();
        GL_CHECK_ERRORS;

        glfwSwapBuffers(window);
        score += deltaTime;
        health = health + deltaTime;
        health = health > 100 ? 100 : health;
    }

    glDeleteVertexArrays(1, &trashVAO);
    glDeleteBuffers(1, &trashVBO);

    glDeleteVertexArrays(1, &skyVAO);
    glDeleteBuffers(1, &skyVBO);

    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);

    glfwTerminate();
    return 0;
}


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_F2 && !cabine){
        cameraPos.y -= 2.0;
        cameraPos.z -= 3.0;
        cabine = true;
    }
    if (key == GLFW_KEY_F3 && cabine){
        cameraPos.y += 2.0;
        cameraPos.z += 3.0;
        cabine = false;
    }
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void do_movement() {
    // Camera controls
    GLfloat cameraSpeed = 5.0f * deltaTime;
//    if (keys[GLFW_KEY_W])
//        cameraPos += cameraSpeed * cameraFront;
//    if (keys[GLFW_KEY_S])
//        cameraPos -= cameraSpeed * cameraFront;
    if (keys[GLFW_KEY_W] && cameraPos.y < 5)
        cameraPos += cameraSpeed * cameraUp;
    if (keys[GLFW_KEY_S] && cameraPos.y > -5)
        cameraPos -= cameraSpeed * cameraUp;
    if (keys[GLFW_KEY_A] && cameraPos.x > -5)
        cameraPos -= normalize(cross(cameraFront, cameraUp)) * cameraSpeed;
    if (keys[GLFW_KEY_D] && cameraPos.x < 5)
        cameraPos += normalize(cross(cameraFront, cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    GLfloat sensitivity = 0.005;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    float3 front;
    front.x = cos(yaw) * cos(pitch);
    front.y = sin(pitch);
    front.z = sin(yaw) * cos(pitch);
    cameraFront = normalize(front);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods){
    if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS){
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        cursorPos = normalize_cursor(x, y);
        float4 mda = mul(proj_inv, float4(cursorPos.x, cursorPos.y, 1.0, 1.0));
        mda = mul(view_inv, mda);
        mda /= mda.w;
        temp_bullets.emplace_back(Bullet(float3(mda.x, mda.y, mda.z), false));
    }
}

inline bool hit(const Bullet &b, const float3 &a){
    return length(a - b.position) < 0.5;
}

void destroy_enemies(vector<Enemy> &enemies){
    for (auto bullet : temp_bullets){
        if (!bullet.enemy_strike){
            for (auto enemy = enemies.rbegin(); enemy != enemies.rend(); ++enemy){
                if (enemy->is_alive && bullet.actual){
                    if (hit(bullet, enemy->position)){
                        enemy->kill();
                        score += 25;
                    }
                }
            }
        } else {
            if (hit(bullet, cameraPos + float3(0.0, -1.0, 0.0))){
                health -= 1;
            }
        }

    }
}

void update_trash(vector<float3> &trash){
    float3 direction(0.0, 0.0, 1.0);
    for (auto &coord : trash){
        coord += direction * 50 * deltaTime;
        if (coord.z > 0){
            coord.z -= 70;
        }
    }
}

unsigned int loadTexture(char const *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = SOIL_load_image(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        SOIL_free_image_data(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        SOIL_free_image_data(data);
    }

    return textureID;
}

void draw_text_info(ShaderProgram &font_shader, int sc, int hea){
    int s1 = sc / 1000;
    int s2= sc / 100 % 10;
    int s3 = sc / 10 % 10;
    int s4 = sc % 10;
    int h1 = hea / 100;
    int h2 = hea / 10 % 10;
    int h3 = hea % 10;
    float4x4 model;
    model = translate4x4(cameraPos + float3(-0.35f, 0.4f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", -1);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = translate4x4(cameraPos + float3(-0.4f, 0.375f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", s1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    model = translate4x4(cameraPos + float3(-0.375f, 0.375f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", s2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    model = translate4x4(cameraPos + float3(-0.35f, 0.375f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", s3);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = translate4x4(cameraPos + float3(-0.325f, 0.375f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", s4);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = translate4x4(cameraPos + float3(0.34f, 0.4f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", -2);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = translate4x4(cameraPos + float3(0.35f, 0.375f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", h1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    model = translate4x4(cameraPos + float3(0.375f, 0.375f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", h2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    model = translate4x4(cameraPos + float3(0.4f, 0.375f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", h3);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
