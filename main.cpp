#include <iostream>
#include <thread>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "m_driver.h"

int WINDOW_WIDTH = 12;
int WINDOW_HEIGHT = 22;
ArrayPtr WINDOW;
float * vertices;
long bufferSize;

#define KEY_ESC 0
#define KEY_W   1
#define KEY_A   2
#define KEY_S   3
#define KEY_D   4
#define KEY_SPACE 5
ArrayPtr KEYBOARD = std::make_shared<Array>(EvalVec(1, 6));

unsigned int VAO, VBO, shaderProgram;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

unsigned int compileProgram(const char* vSource, const char* fSource);

void init();
void destroy();
void getVertices();

unsigned int SCR_WIDTH = WINDOW_WIDTH * 30;
unsigned int SCR_HEIGHT = WINDOW_HEIGHT * 30;

#define STRINGIFY(A) #A

const char* vertexShaderSource = "#version 330 core\n" STRINGIFY(
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aColor;
        out vec3 ourColor;
        void main()
        {
            gl_Position = vec4(aPos, 1.0);
            ourColor = aColor;
        }
);

const char* fragmentShaderSource = "#version 330 core\n" STRINGIFY(
        out vec4 FragColor;
        in vec3 ourColor;
        void main()
        {
            FragColor = vec4(ourColor, 1.0f);
        }
);

void start(int argc, const char ** argv) {
    if (argc == 2) {
        // for global variable
        SymbolManager::addLayer();
        SymbolManager::addStatus(SymbolManager::MAIN);
        SymbolManager::addLayerF();

        // global variable
        SymDecl("WINDOW", std::make_shared<NumArray>(*WINDOW)).eval();
        SymDecl("KEYBOARD", std::make_shared<NumArray>(*KEYBOARD)).eval();
        SymDecl("KEY_ESC", std::make_shared<Num>(0)).eval();
        SymDecl("KEY_W", std::make_shared<Num>(1)).eval();
        SymDecl("KEY_A", std::make_shared<Num>(2)).eval();
        SymDecl("KEY_S", std::make_shared<Num>(3)).eval();
        SymDecl("KEY_D", std::make_shared<Num>(4)).eval();
        SymDecl("KEY_SPACE", std::make_shared<Num>(5)).eval();
        SymDecl("KEY_DOWN", std::make_shared<Num>(1)).eval();
        SymDecl("KEY_UP", std::make_shared<Num>(0)).eval();

        MC::Driver driver;
        if (std::strncmp(argv[1], "-o", 2) == 0)
            driver.parse(std::cin);
        else {
            driver.parse(argv[1]);
        }

        std::cout << "start finished" << std::endl;
    }
}

int main(const int argc, const char **argv) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "window", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    init();

    std::thread(start, argc, argv).detach();

    while (!glfwWindowShouldClose(window)) {

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO);

        getVertices();
        void * ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(ptr, vertices, bufferSize);
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glDrawArrays(GL_TRIANGLES, 0, WINDOW_HEIGHT * WINDOW_WIDTH * 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds (100));

    destroy();

    glfwTerminate();

    return 0;
}

void processInput(GLFWwindow* window) {
    KEYBOARD->data_[KEY_ESC] = (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) ? 1 : 0;
    KEYBOARD->data_[KEY_W] = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) ? 1 : 0;
    KEYBOARD->data_[KEY_S] = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) ? 1 : 0;
    KEYBOARD->data_[KEY_A] = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) ? 1 : 0;
    KEYBOARD->data_[KEY_D] = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) ? 1 : 0;
    KEYBOARD->data_[KEY_SPACE] = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) ? 1 : 0;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void printShaderLog(unsigned int shader) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

unsigned int compileProgram(const char* vSource, const char* fSource) {
    unsigned int vertexShader = 0;
    unsigned int fragmentShader = 0;

    unsigned int shaderProgram = glCreateProgram();

    if (vSource) {
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);
        printShaderLog(vertexShader);
        glAttachShader(shaderProgram, vertexShader);
    }

    if (fSource) {
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);
        printShaderLog(fragmentShader);
        glAttachShader(shaderProgram, fragmentShader);
    }

    glLinkProgram(shaderProgram);

    int success = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void init() {
    WINDOW = std::make_shared<Array>(EvalVec(3, 3, WINDOW_WIDTH, WINDOW_HEIGHT));

    vertices = new float[WINDOW_HEIGHT * WINDOW_WIDTH * 2 * 3 * 6];
    bufferSize = WINDOW_HEIGHT * WINDOW_WIDTH * 2 * 3 * 6 * sizeof(float);

    shaderProgram = compileProgram(vertexShaderSource, fragmentShaderSource);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glUseProgram(shaderProgram);
}

void destroy() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    delete[] vertices;
}

void getVertices() {
    float deltaX = 2.0f / WINDOW_WIDTH;
    float deltaY = 2.0f / WINDOW_HEIGHT;
    int * g_window = WINDOW->data_.get();

    int index = 0;
    float ox = -1.0f, oy = -1.0f;
    for (int i = 0; i < WINDOW_HEIGHT; i++)
        for (int j = 0; j < WINDOW_WIDTH; j++) {
            vertices[index++] = ox + j * deltaX; // x
            vertices[index++] = oy + i * deltaY; // y
            vertices[index++] = 0.0f;   // z
            for (int k = 0; k < 3; k++)
                vertices[index++] = *(g_window + i * (WINDOW_WIDTH * 3) + j * 3 + k) / 256.0f;

            vertices[index++] = ox + (j + 1) * deltaX; // x
            vertices[index++] = oy + i * deltaY; // y
            vertices[index++] = 0.0f;   // z
            for (int k = 0; k < 3; k++)
                vertices[index++] = *(g_window + i * (WINDOW_WIDTH * 3) + j * 3 + k) / 256.0f;

            vertices[index++] = ox + j * deltaX; // x
            vertices[index++] = oy + (i + 1) * deltaY; // y
            vertices[index++] = 0.0f;   // z
            for (int k = 0; k < 3; k++)
                vertices[index++] = *(g_window + i * (WINDOW_WIDTH * 3) + j * 3 + k) / 256.0f;

            vertices[index++] = ox + j * deltaX; // x
            vertices[index++] = oy + (i + 1) * deltaY; // y
            vertices[index++] = 0.0f;   // z
            for (int k = 0; k < 3; k++)
                vertices[index++] = *(g_window + i * (WINDOW_WIDTH * 3) + j * 3 + k) / 256.0f;

            vertices[index++] = ox + (j + 1) * deltaX; // x
            vertices[index++] = oy + i * deltaY; // y
            vertices[index++] = 0.0f;   // z
            for (int k = 0; k < 3; k++)
                vertices[index++] = *(g_window + i * (WINDOW_WIDTH * 3) + j * 3 + k) / 256.0f;

            vertices[index++] = ox + (j + 1) * deltaX; // x
            vertices[index++] = oy + (i + 1) * deltaY; // y
            vertices[index++] = 0.0f;   // z
            for (int k = 0; k < 3; k++)
                vertices[index++] = *(g_window + i * (WINDOW_WIDTH * 3) + j * 3 + k) / 256.0f;
        }
}