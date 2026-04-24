#include <iostream>
using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Protótipos
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();
int setupGeometry();

// Tela
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Shaders
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"out vec4 finalColor;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(position, 1.0);\n"
"   finalColor = vec4(color, 1.0);\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"   color = finalColor;\n"
"}\n\0";

// CONTROLES
bool rotateX=false, rotateY=false, rotateZ=false;

float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
float scaleFactor = 1.0f;

int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Cubo 3D - Davi", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        cout << "Erro GLAD\n";

    glViewport(0, 0, WIDTH, HEIGHT);

    GLuint shaderID = setupShader();
    GLuint VAO = setupGeometry();

    glUseProgram(shaderID);

    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projLoc = glGetUniformLocation(shaderID, "projection");

    glEnable(GL_DEPTH_TEST);

    // POSIÇÕES DOS CUBOS
    glm::vec3 cubePositions[] = {
        glm::vec3(0, 0, 0),
        glm::vec3(2, 0, 0),
        glm::vec3(-2, 0, 0),
        glm::vec3(0, 2, 0)
    };

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(1,1,1,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // CAMERA (VIEW)
        glm::mat4 view = glm::translate(glm::mat4(1.0f),
                                        glm::vec3(0.0f, 0.0f, -5.0f));

        // PROJEÇÃO
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)WIDTH / (float)HEIGHT,
            0.1f,
            100.0f
        );

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        float angle = glfwGetTime();

        glBindVertexArray(VAO);

        for (int i = 0; i < 4; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            // TRANSLAÇÃO global + instância
            model = glm::translate(model, glm::vec3(posX, posY, posZ));
            model = glm::translate(model, cubePositions[i]);

            // ESCALA
            model = glm::scale(model, glm::vec3(scaleFactor));

            // ROTAÇÃO
            if (rotateX)
                model = glm::rotate(model, angle, glm::vec3(1,0,0));
            else if (rotateY)
                model = glm::rotate(model, angle, glm::vec3(0,1,0));
            else if (rotateZ)
                model = glm::rotate(model, angle, glm::vec3(0,0,1));
            else
                model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1,1,0)); // leve rotação padrão

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// TECLADO
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    float speed = 0.2f;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // ROTAÇÃO
    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    { rotateX = true; rotateY = false; rotateZ = false; }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    { rotateX = false; rotateY = true; rotateZ = false; }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    { rotateX = false; rotateY = false; rotateZ = true; }

    // TRANSLAÇÃO
    if (key == GLFW_KEY_W) posZ -= speed;
    if (key == GLFW_KEY_S) posZ += speed;
    if (key == GLFW_KEY_A) posX -= speed;
    if (key == GLFW_KEY_D) posX += speed;

    if (key == GLFW_KEY_I) posY += speed;
    if (key == GLFW_KEY_J) posY -= speed;

    // ESCALA
    if (key == GLFW_KEY_O) scaleFactor -= 0.1f;
    if (key == GLFW_KEY_K) scaleFactor += 0.1f;
}

// SHADER
int setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// GEOMETRIA DO CUBO
int setupGeometry()
{
    GLfloat vertices[] = {
        // Frente
        -0.5,-0.5,0.5, 1,0,0,  0.5,-0.5,0.5, 1,0,0,  0.5,0.5,0.5, 1,0,0,
        -0.5,-0.5,0.5, 1,0,0,  0.5,0.5,0.5, 1,0,0, -0.5,0.5,0.5, 1,0,0,

        // Trás
        -0.5,-0.5,-0.5, 0,1,0,  0.5,0.5,-0.5, 0,1,0,  0.5,-0.5,-0.5, 0,1,0,
        -0.5,-0.5,-0.5, 0,1,0, -0.5,0.5,-0.5, 0,1,0,  0.5,0.5,-0.5, 0,1,0,

        // Esquerda
        -0.5,-0.5,-0.5, 0,0,1, -0.5,-0.5,0.5, 0,0,1, -0.5,0.5,0.5, 0,0,1,
        -0.5,-0.5,-0.5, 0,0,1, -0.5,0.5,0.5, 0,0,1, -0.5,0.5,-0.5, 0,0,1,

        // Direita
         0.5,-0.5,-0.5, 1,1,0,  0.5,0.5,0.5, 1,1,0,  0.5,-0.5,0.5, 1,1,0,
         0.5,-0.5,-0.5, 1,1,0,  0.5,0.5,-0.5, 1,1,0,  0.5,0.5,0.5, 1,1,0,

        // Topo
        -0.5,0.5,-0.5, 0,1,1, -0.5,0.5,0.5, 0,1,1,  0.5,0.5,0.5, 0,1,1,
        -0.5,0.5,-0.5, 0,1,1,  0.5,0.5,0.5, 0,1,1,  0.5,0.5,-0.5, 0,1,1,

        // Base
        -0.5,-0.5,-0.5, 1,0,1,  0.5,-0.5,0.5, 1,0,1, -0.5,-0.5,0.5, 1,0,1,
        -0.5,-0.5,-0.5, 1,0,1,  0.5,-0.5,-0.5, 1,0,1, 0.5,-0.5,0.5, 1,0,1,
    };

    GLuint VBO, VAO;

    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    return VAO;
}