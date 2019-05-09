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

void destroy_enemies(vector<Asteroid> &asteroids);
void update_trash(vector<float3> &trash);
void update_bullets(list<Bullet> &bullets){

}
inline bool hit(float4 &b_l, float4 &t_r){
    return b_l.x <= cursorPos.x && b_l.y <= cursorPos.y && cursorPos.x <= t_r.x && cursorPos.y <= t_r.y;
}
inline float2 normalize_cursor(double x, double y){
    return float2(2 * x / WIDTH - 1.0, 2 * (HEIGHT - y) / HEIGHT - 1.0);
}

unsigned int loadCubemap(vector<std::string> faces);

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

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
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
    cursor.pixels = SOIL_load_image("../cursor.png", &cursor.width, &cursor.height, 0, 0);
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

    //glVertexAttribPointer(location, data_len, type, normalize, stride, (GLvoid *)(offset))

    // box VAO
    float vertices[] = {
            // positions          // texture Coords
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vertexLocation = 0;
    glEnableVertexAttribArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *) (0));

    GLuint textureLocation = 1;
    glEnableVertexAttribArray(textureLocation);
    glVertexAttribPointer(textureLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          (GLvoid *) (3 * sizeof(GLfloat)));
    glBindVertexArray(0);

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
    /*float sky_vertices[] = {
            // positions
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
    };*/
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

//    Assimp::Importer importer;
//    const aiScene *loaded_obj = importer.ReadFile("../models/ship1/WR.obj", aiProcess_Triangulate);
//
//    std::vector<GLfloat> ship1_mesh;
//    std::vector<GLfloat> ship1_texture_coords;
//    loadModel(loaded_obj, ship1_mesh, ship1_texture_coords);
//    std::cout << ship1_mesh.size() << ' ' << ship1_texture_coords.size() << '\n';
////    for (int i = 0; i < ship1_mesh.size(); i+=3){
////        std::cout << ship1_mesh[i] << ' ' << ship1_mesh[i+1] << ' ' << ship1_mesh[i+2] << '\n';
////    }
//    GLuint shipVBO, shipVAO;
//    glGenVertexArrays(1, &shipVAO);
//    glGenBuffers(1, &shipVBO);
//    glBindVertexArray(shipVAO);
//    glBindBuffer(GL_ARRAY_BUFFER, shipVBO);
//    glBufferData(GL_ARRAY_BUFFER, ship1_mesh.size() * sizeof(GLfloat), ship1_mesh.data(), GL_STATIC_DRAW);
//
//    GLuint shipLocation = 0;
//    glEnableVertexAttribArray(shipLocation);
//    glVertexAttribPointer(shipLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *) (0));
//
//    glBindVertexArray(0);

    /// Load and create a texture
    GLuint texture1 = loadTexture("../cockpit.png");
    GLuint fog_tex = loadTexture("../fog.png");
    Sprite explosion1(loadTexture("../boom.png"), 9, 9, 81);
    Sprite asteroid1(loadTexture("../asteroid1.png"), 8, 8, 32);
    Sprite asteroid2(loadTexture("../asteroid2.png"), 5, 6, 30);

//    vector <std::string> faces{
//            "../bg/background.jpg",
//            "../bg/background.jpg",
//            "../bg/background.jpg",
//            "../bg/background.jpg",
//            "../bg/background.jpg",
//            "../bg/background.jpg"
//    };
//    GLuint background_tex = loadCubemap(faces);
    GLuint bgt = loadTexture("../background.jpg");

    GLuint digitstex = loadTexture("../digits.png");
    GLuint scoretex = loadTexture("../score.png");
    GLuint healthtex = loadTexture("../health.png");



    /// Presetting uniforms
    projection = transpose(
            projectionMatrixTransposed(45.0f, (GLfloat) WIDTH / (GLfloat) HEIGHT, 0.1f, 100.0f));
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
    //sky_shader.SetUniform("skybox", 1);
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

//    ship_shader.StartUseShader();
//    ship_shader.SetUniform("projection", projection);
//    ship_shader.StopUseShader();

    std::vector<Asteroid> asteroids = {
            Asteroid(asteroid1, explosion1),
            Asteroid(asteroid1, explosion1),
            Asteroid(asteroid2, explosion1),
            Asteroid(asteroid2, explosion1),
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
//    std::vector<float3> fog_points;
//    std::normal_distribution<> dis(0.0, 0.5);
//    for (int i = 0; i < 10000; i++){
////        float z = -10.0 + dis(gen) / 100;
////        if (z < -5){
////
////        }
//        fog_points.emplace_back(float3(dis(gen), dis(gen), dis(gen)));
//
//    }



    float4x4 model;
    std::vector<Fog> fogs = {
            Fog(fog_tex),
            Fog(fog_tex),
            Fog(fog_tex),
            Fog(fog_tex),
            Fog(fog_tex),
            Fog(fog_tex),
            Fog(fog_tex),
            Fog(fog_tex),
            Fog(fog_tex),
            Fog(fog_tex),
            Fog(fog_tex),
            Fog(fog_tex),
            Fog(fog_tex)
    };

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
        /// Start shader ->
        /// set view and projection matrix ->
        /// activate texture ->
        /// bind VAO ->
        /// model matrix + draw ->
        /// unbind VAO -> stop shader

        /// sky
        //glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        sky_shader.StartUseShader();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bgt);

        glBindVertexArray(skyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        sky_shader.StopUseShader();
        glDepthFunc(GL_LESS);
        //glDepthMask(GL_TRUE);

        view = transpose(lookAtTransposed(cameraPos, cameraPos + cameraFront, cameraUp));


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
            if (flying_fog.actual == false){
                flying_fog = Fog(fog_tex);
                continue;
            }
            //std::uniform_real_distribution<> diss(1,2);
            model = rotate_Z_4x4(lastFrame * flying_fog.speed / 2);
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
        std::sort(asteroids.begin(), asteroids.end(),
                  [](const Asteroid &p1, Asteroid &p2) {
                      return p1.position.z < p2.position.z;
                  });
        destroy_enemies(asteroids);
        time_t current = time(0);
        for (auto &astro : asteroids) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, astro.texture);
            model = translate4x4(astro.position);
            sprite_shader.SetUniform("model", model);
            sprite_shader.SetUniform("is_alive", astro.is_alive);
            sprite_shader.SetUniform("animation", astro.animate());
            glDrawArrays(GL_TRIANGLES, 0, 6);
            if (!astro.is_alive && (current - astro.time_of_death) > 5) {
                astro.respawn();
            }
        }
        glBindVertexArray(0);
        sprite_shader.StopUseShader();


        /// trash
        lines_shader.StartUseShader();

        lines_shader.SetUniform("view", view);
        glBindVertexArray(trashVAO);
        GL_CHECK_ERRORS;

        lines_shader.SetUniform("direction", float3(0.0, 0.0, 1.0));
        lines_shader.SetUniform("incolor", float3(0.0, 0.5, 0.75));
        for (auto &trash : trash_points){
            model = translate4x4(trash);
            lines_shader.SetUniform("model", model);
            glDrawArrays(GL_POINTS, 0, 1);
        }
        update_trash(trash_points);

        /// Bullets
        lines_shader.SetUniform("incolor", float3(0.0, 1.0, 0.0));
        for (auto bull = temp_bullets.begin(); bull != temp_bullets.end();){
            if (bull->actual){
                model = translate4x4(bull->position);
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





//        for (auto &fog_off : fog_points){
//            model = translate4x4(flying_fog.position + fog_off);
//            fog_shader.SetUniform("model", model);
//            glDrawArrays(GL_POINTS, 0, 1);
//            flying_fog.move();
//        }
//        model = translate4x4(float3(0.0, 0.0, -10.0));
//        fog_shader.SetUniform("model", model);
//        glDrawArrays(GL_TRIANGLES, 0, 36);
//
//        glBindVertexArray(0);
//        fog_shader.StopUseShader();


        /// objects
        objects_shader.StartUseShader();

        objects_shader.SetUniform("view", view);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glBindVertexArray(planeVAO);
        model = translate4x4(cameraPos + float3(0.0f, 0.0f, -1.1f));
        objects_shader.SetUniform("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);



        glBindVertexArray(0);
        objects_shader.StopUseShader();




        font_shader.StartUseShader();
        font_shader.SetUniform("view", view);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, digitstex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, scoretex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, healthtex);

        glBindVertexArray(planeVAO);
        draw_text_info(font_shader, int(score), int(health));

        glBindVertexArray(0);
        font_shader.StopUseShader();







//        ship_shader.StartUseShader();
//
//        ship_shader.SetUniform("view", view);
//        glBindVertexArray(shipVAO);
//        model = translate4x4(float3(0.0f, 0.0f, -30.0f));
//        ship_shader.SetUniform("model", model);
//        glDrawArrays(GL_TRIANGLES, 0, ship1_mesh.size() / 3);
//        //glDrawElements(GL_TRIANGLES, ship1_mesh.size() / 3, GL_UNSIGNED_INT, 0);


        GL_CHECK_ERRORS;

        glfwSwapBuffers(window);
        score += deltaTime;
        health = health + deltaTime;
        health = health > 100 ? 100 : health;
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

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
    if (keys[GLFW_KEY_W])
        cameraPos += cameraSpeed * cameraFront;
    if (keys[GLFW_KEY_S])
        cameraPos -= cameraSpeed * cameraFront;
    if (keys[GLFW_KEY_R])
        cameraPos += cameraSpeed * cameraUp;
    if (keys[GLFW_KEY_F])
        cameraPos -= cameraSpeed * cameraUp;
    if (keys[GLFW_KEY_A])
        cameraPos -= normalize(cross(cameraFront, cameraUp)) * cameraSpeed;
    if (keys[GLFW_KEY_D])
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
        temp_bullets.emplace_back(Bullet(float3(mda.x, mda.y, mda.z)));
    }
}

inline bool hit(Bullet &b, Asteroid &a){
    return length(a.position - b.position) < 0.5;
}

void destroy_enemies(vector<Asteroid> &asteroids){
    for (auto bullet : temp_bullets){
        for (auto astro = asteroids.rbegin(); astro != asteroids.rend(); ++astro){
            if (astro->is_alive && bullet.actual){
//            auto b_l = float4(astro->position + float3(-0.5, -0.5, 0.0));
//            b_l = mul(projection, mul(view, b_l));
//            b_l /= b_l.w;
//            auto t_r = float4(astro->position + float3(0.5, 0.5, 0.0));
//            t_r = mul(projection, mul(view, t_r));
//            t_r /= t_r.w;
                if (hit(bullet, *astro)){
                    astro->kill();
                    score += 25;
                    bullet.actual = false;
                    break;
                }
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

unsigned int loadCubemap(vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = SOIL_load_image(faces[i].c_str(), &width, &height, &nrChannels, 0);
        std::cout << SOIL_last_result() << std::endl;
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            SOIL_free_image_data(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            SOIL_free_image_data(data);
        }
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
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
    model = translate4x4(cameraPos + float3(-0.35f, -0.375f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", -1);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = translate4x4(cameraPos + float3(-0.4f, -0.4f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", s1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    model = translate4x4(cameraPos + float3(-0.375f, -0.4f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", s2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    model = translate4x4(cameraPos + float3(-0.35f, -0.4f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", s3);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = translate4x4(cameraPos + float3(-0.325f, -0.4f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", s4);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = translate4x4(cameraPos + float3(0.34f, -0.375f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", -2);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = translate4x4(cameraPos + float3(0.35f, -0.4f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", h1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    model = translate4x4(cameraPos + float3(0.375f, -0.4f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", h2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    model = translate4x4(cameraPos + float3(0.4f, -0.4f, -1.0f));
    font_shader.SetUniform("model", model);
    font_shader.SetUniform("number", h3);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

//void loadModel(const aiScene *loaded_obj, std::vector<GLfloat> &mesh, std::vector<GLfloat> &tex){
//    for (int i = 0; i < loaded_obj->mNumMeshes; ++i){
//        auto cur_mesh = loaded_obj->mMeshes[i];
//        for (int j = 0; j < cur_mesh->mNumVertices; ++j){
//            mesh.push_back(cur_mesh->mVertices->x);
//            mesh.push_back(cur_mesh->mVertices->y);
//            mesh.push_back(cur_mesh->mVertices->z);
//        }
//        if (cur_mesh->mTextureCoords[0] != nullptr){
//            for (int j = 0; j < cur_mesh->mNumVertices; ++j) {
//                tex.push_back(cur_mesh->mTextureCoords[0][j].x);
//                tex.push_back(cur_mesh->mTextureCoords[0][j].y);
//            }
//        }
//    }
//}
