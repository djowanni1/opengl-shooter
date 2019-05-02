//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"
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

using std::vector;
using std::string;
using namespace LiteMath;
static const GLsizei WIDTH = 640, HEIGHT = 640; //размеры окна

GLfloat lastX = WIDTH / 2, lastY = HEIGHT / 2;

GLfloat yaw = -M_PI_2;
GLfloat pitch = 0.0f;
bool firstMouse = false;
float3 cameraPos = float3(0.0f, 0.0f, 3.0f);

float3 cameraFront = normalize(float3(0.0f, 0.0f, -1.0f));
float3 cameraUp = normalize(float3(0.0f, 1.0f, 0.0f));

bool keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


std::mt19937 gen(time(0));


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void do_movement();

unsigned int loadCubemap(vector<std::string> faces);

unsigned int load_texture(const char *image_name);

unsigned int loadTexture(char const *path);


class SpriteAnimator {
private:
    int count_frames;


    int stride_x, stride_y;
    float scale_x, scale_y;

    void UpdateStep() {
        step += deltaTime * fps;
        if (step > count_frames) {
            step -= count_frames;
        }
    }

public:
    GLuint texture;
    float fps;
    float step;
    SpriteAnimator(const char *path, int stride_x, int stride_y, int count_frames)
            : texture(loadTexture(path)), stride_x(stride_x), stride_y(stride_y), scale_x(1.0 / stride_x),
              scale_y(1.0 / stride_y), count_frames(count_frames), step(0) {
        std::uniform_real_distribution<> dis(10, 25);
        fps = dis(gen);
    }

    SpriteAnimator(const SpriteAnimator &other, bool create)
            : texture(other.texture), stride_x(other.stride_x), stride_y(other.stride_y), scale_x(other.scale_x),
              scale_y(other.scale_y), count_frames(other.count_frames), step(0) {
        std::uniform_real_distribution<> dis(10, 25);
        fps = dis(gen);
    }

    float3x3 animation() {
        UpdateStep();
        int off = int(step);
        float arr[] = {
                scale_x, 0.0, off % stride_x * scale_x,
                0.0, scale_y, off / stride_y * scale_y,
                0.0, 0.0, 1.0
        };
        return float3x3(arr);
    }
};

class Enemy {
private:

public:
    SpriteAnimator anim;
    SpriteAnimator explosion;
    bool is_alive;
    float3 direction;
    float3 position;

    Enemy(SpriteAnimator &anim, SpriteAnimator &explosion)
            : anim(anim, true), explosion(explosion, true), is_alive(true) {
        std::uniform_real_distribution<float> dis(-10, 10);
        position = float3(dis(gen), dis(gen), -50.0f + dis(gen));
        direction = cameraPos - position;
    };

    float3x3 animate() {
        GLfloat speed = 0.1f * deltaTime;
        position += direction * speed;
        if (is_alive){
            return anim.animation();
        } else {
            return explosion.animation();
        }

    }
};

inline void speed_control() {
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

int initGL() {
    int res = 0;
    //грузим функции opengl через glad
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

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
    GL_CHECK_ERRORS;

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
    float sky_vertices[] = {
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
    };
    GLuint skyVBO, skyVAO;
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sky_vertices), sky_vertices, GL_STATIC_DRAW);

    GLuint skyLocation = 0;
    glEnableVertexAttribArray(skyLocation);
    glVertexAttribPointer(skyLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *) (0));
    glBindVertexArray(0);


    // Load and create a texture
    GLuint texture1 = loadTexture("../container.jpg");
    //GLuint texture2 = loadTexture("../awesomeface.png");

    objects_shader.StartUseShader();
    objects_shader.SetUniform("Texture", 0);
    objects_shader.StopUseShader();

    SpriteAnimator boom("../boom.png", 9, 9, 81);
    SpriteAnimator asteroid1("../asteroid1.png", 8, 8, 32);
    SpriteAnimator asteroid2("../asteroid2.png", 5, 6, 30);
    //SpriteAnimator asteroid3("../asteroid3.png", 5, 6, 25, 15);

    sprite_shader.StartUseShader();
    sprite_shader.SetUniform("Texture", 0);
    sprite_shader.SetUniform("boom", 1);
    sprite_shader.StopUseShader();

    vector<std::string> faces{
            "../bg/right_1.jpg",
            "../bg/left_1.jpg",
            "../bg/top_1.jpg",
            "../bg/bottom_1.jpg",
            "../bg/front_1.jpg",
            "../bg/back_1.jpg"
    };
    GLuint background_tex = loadCubemap(faces);

    sky_shader.StartUseShader();
    sky_shader.SetUniform("skybox", 0);
    sky_shader.StopUseShader();

    std::vector<Enemy> asteroids = {
            Enemy(asteroid1, boom),
            Enemy(asteroid1, boom),
            Enemy(asteroid1, boom),
            Enemy(asteroid1, boom),
    };
    asteroids[0].is_alive = false;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Camera movements
        speed_control();
        do_movement();

        // Clear window
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        float4x4 view = transpose(lookAtTransposed(cameraPos, cameraPos + cameraFront, cameraUp));

        float4x4 projection = transpose(
                projectionMatrixTransposed(45.0f, (GLfloat) WIDTH / (GLfloat) HEIGHT, 0.1f, 100.0f));

        float4x4 model;
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

        view.row[0].w = 0.0f;
        view.row[1].w = 0.0f;
        view.row[2].w = 0.0f;
        sky_shader.SetUniform("view", view);
        sky_shader.SetUniform("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, background_tex);

        glBindVertexArray(skyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        sky_shader.StopUseShader();
        glDepthFunc(GL_LESS);
        //glDepthMask(GL_TRUE);

        view = transpose(lookAtTransposed(cameraPos, cameraPos + cameraFront, cameraUp));

        /// Sprites
        sprite_shader.StartUseShader();

        sprite_shader.SetUniform("view", view);
        sprite_shader.SetUniform("projection", projection);

        glBindVertexArray(planeVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, asteroid1.texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, boom.texture);
        std::sort(asteroids.begin(), asteroids.end(),
                  [](const Enemy &p1, Enemy &p2) {
                      return p1.position.z < p2.position.z;
                  });
        for (auto &astro : asteroids) {
            model = translate4x4(astro.position);
            sprite_shader.SetUniform("model", model);
            sprite_shader.SetUniform("is_alive", astro.is_alive);
            sprite_shader.SetUniform("animation", astro.animate());
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glBindVertexArray(0);
        sprite_shader.StopUseShader();


        /// objects
        objects_shader.StartUseShader();

        objects_shader.SetUniform("view", view);
        objects_shader.SetUniform("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glBindVertexArray(VAO);

        model = translate4x4(float3(10.0f, 0.0f, 0.0f));
        objects_shader.SetUniform("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        objects_shader.StopUseShader();


        GL_CHECK_ERRORS;

        glfwSwapBuffers(window);
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

//    if(pitch > M_PI_2)
//        pitch = M_PI_2;
//    if(pitch < -M_PI_2)
//        pitch = -M_PI_2;

    float3 front;
    front.x = cos(yaw) * cos(pitch);
    front.y = sin(pitch);
    front.z = sin(yaw) * cos(pitch);
    cameraFront = normalize(front);
}

unsigned int loadCubemap(vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = SOIL_load_image(faces[i].c_str(), &width, &height, &nrChannels, 0);
        std::cout << SOIL_last_result() << std::endl;
        //stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            SOIL_free_image_data(data);
            //stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            SOIL_free_image_data(data);
            // stbi_image_free(data);
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

unsigned int load_texture(const char *filename) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    int width, height;
    unsigned char *image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL_CHECK_ERRORS;
    return texture;
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


