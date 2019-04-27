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
#include <string>

using std::vector;
using std::string;
using namespace LiteMath;
static const GLsizei WIDTH = 640, HEIGHT = 480; //размеры окна

GLfloat lastX = WIDTH / 2, lastY = HEIGHT / 2;

GLfloat yaw = -M_PI_2;
GLfloat pitch = 0.0f;
bool firstMouse = false;
float3 cameraPos = float3(0.0f, 0.0f, 3.0f);

float3 cameraFront = float3(0.0f, 0.0f, -1.0f);
float3 cameraUp = float3(0.0f, 1.0f, 0.0f);

bool keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void do_movement();

unsigned int loadCubemap(vector<std::string> faces);

float4x4 perspective(float fov, float aspect, float znear, float zfar) {

    float xymax = znear * tan(fov * M_PI / 360);
    float ymin = -xymax;
    float xmin = -xymax;

    float width = xymax - xmin;
    float height = xymax - ymin;

    float depth = zfar - znear;
    float q = -(zfar + znear) / depth;
    float qn = -2 * (zfar * znear) / depth;

    float w = 2 * znear / width;
    w = w / aspect;
    float h = 2 * znear / height;
    float data[] = {
            w, 0, 0, 0,
            0, h, 0, 0,
            0, 0, q, -1,
            0, 0, qn, 1
    };

    return float4x4(data);
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

    //запрашиваем контекст opengl версии 3.3
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
    // ???
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    if (initGL() != 0)
        return -1;

    glViewport(0, 0, WIDTH, HEIGHT);

    glEnable(GL_DEPTH_TEST);
    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    //создание шейдерной программы из двух файлов с исходниками шейдеров
    //используется класс-обертка ShaderProgram
    std::unordered_map<GLenum, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = "vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
    ShaderProgram program(shaders);
    GL_CHECK_ERRORS;

    glfwSwapInterval(1); // force 60 frames per second



    //Создаем и загружаем геометрию поверхности
    //


    GLfloat vertices[] = {
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f,

            -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,

            1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 0.0f, 0.0f,

            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,

            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,

            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 1.0f, 1.0f
    };
    GLuint VBO;
    GLuint VAO;

    VBO = 0;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    GLuint vertexLocation = 0; // simple layout, assume have only positions at location = 0

    //1 - location, 2 - vec3, 3 - type, 4 - normalize, 5 - offset, 6 - 0
    glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *) (0));
    glEnableVertexAttribArray(vertexLocation);
    //glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // Texture attribute
    GLuint texLocation = 2;
    glVertexAttribPointer(texLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(texLocation);

    glBindVertexArray(0);



    // Load and create a texture



    // ====================
    // Texture 1
    // ====================
    GLuint texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D,
                  texture1); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // Set texture wrapping to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    int width, height;
    unsigned char *image = SOIL_load_image("../container.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

    // ===================
    // Texture 2
    // ===================
    GLuint texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    image = SOIL_load_image("../awesomeface.png", &width, &height, 0, SOIL_LOAD_RGB);
    //SOIL_save_image("../mda.bmp", SOIL_SAVE_TYPE_BMP, width, height, 3, image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    // ===================
    // Texture 3
    // ===================
    vector<std::string> faces{
            "../bg/right.jpg",
            "../bg/left.jpg",
            "../bg/top.jpg",
            "../bg/bottom.jpg",
            "../bg/front.jpg",
            "../bg/back.jpg"
    };
    GLuint texture3 = loadCubemap(faces);

//    GLuint texture3;
//    glGenTextures(1, &texture3);
//    glBindTexture(GL_TEXTURE_2D, texture3);
//    // Set our texture parameters
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    // Set texture filtering
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    // Load, create texture and generate mipmaps
//    image = SOIL_load_image("../mw.jpg", &width, &height, 0, SOIL_LOAD_RGB);
//    //SOIL_save_image("../mda.bmp", SOIL_SAVE_TYPE_BMP, width, height, 3, image);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
//    glGenerateMipmap(GL_TEXTURE_2D);
//    SOIL_free_image_data(image);
//    glBindTexture(GL_TEXTURE_2D, 0);

    //цикл обработки сообщений и отрисовки сцены каждый кадр

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        do_movement();

        //очищаем экран каждый кадр
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        GL_CHECK_ERRORS;

        program.StartUseShader();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        program.SetUniform("Texture1", 0);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        program.SetUniform("Texture2", 1);
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture3);
        program.SetUniform("Texture3", 2);

        GL_CHECK_ERRORS;

        float4x4 view = transpose(lookAtTransposed(cameraPos, cameraPos + cameraFront, cameraUp));
        program.SetUniform("view", view);

        float4x4 projection = transpose(
                projectionMatrixTransposed(45.0f, (GLfloat) WIDTH / (GLfloat) HEIGHT, 0.1f, 100.0f));
        program.SetUniform("projection", projection);


        glBindVertexArray(VAO);





        //program.SetUniform("view", view);

        float4x4 model = translate4x4(float3(0.0f, 0.0f, 0.0f));
        program.SetUniform("model", model);
        program.SetUniform("tex_type", 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = rotate_X_4x4(45.0);
        model = mul(translate4x4(float3(10.0f, 10.0f, 0.0f)), model);
        program.SetUniform("model", model);
        program.SetUniform("tex_type", 1);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //model = translate4x4(float3(10.0f, 10.0f, 0.0f));
        //view = transpose(lookAtTransposed(cameraPos, cameraPos + float3(0.0f, 0.0f, -1.0f), float3(0.0f, 1.0f, 0.0f)));
        //view[]
//        glDepthFunc(GL_LEQUAL);
//        float4x4 cubeview = transpose(lookAtTransposed(cameraPos, cameraPos + cameraFront, cameraUp));;
//        cubeview.row[0].w = 0.0f;
//        cubeview.row[1].w = 0.0f;
//        cubeview.row[2].w = 0.0f;
//        program.SetUniform("view", cubeview);
//        program.SetUniform("tex_type", 2);
//        glDrawArrays(GL_TRIANGLES, 0, 36);
//        glDepthFunc(GL_LESS);

        glBindVertexArray(0);

        GL_CHECK_ERRORS;
        program.StopUseShader();

        glfwSwapBuffers(window);
    }

    //очищаем vboи vao перед закрытием программы
    //
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

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
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}