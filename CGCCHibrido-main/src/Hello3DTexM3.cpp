#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace glm;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

int setupShader();

GLuint loadSimpleOBJ(string filePATH, int &nVertices);

GLuint loadTexture(string filePath, int &width, int &height);

//tela
const GLuint WIDTH = 1000;
const GLuint HEIGHT = 1000;

//controles 
bool rotateX = false;
bool rotateY = false;
bool rotateZ = false;

float posX = 0.0f;
float posY = 0.0f;
float posZ = 0.0f;

float scaleFactor = 1.0f;

//shaders
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec2 texCoord;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"out vec2 TexCoord;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(position, 1.0);\n"
"   TexCoord = texCoord;\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 450\n"
"in vec2 TexCoord;\n"
"\n"
"uniform sampler2D texBuffer;\n"
"\n"
"out vec4 color;\n"
"\n"
"void main()\n"
"{\n"
"   color = texture(texBuffer, TexCoord);\n"
"}\n\0";

//função main
int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "Modulo 3 - OBJ + Textura",
        nullptr,
        nullptr
    );

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Erro GLAD\n";
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    glEnable(GL_DEPTH_TEST);

    //setup do shader
    GLuint shaderID = setupShader();

    // necessario pro OBJ
    int nVertices;

    GLuint VAO = loadSimpleOBJ(
        "../assets/Modelos3D/untitled.obj",
        nVertices
    );

    //textura
    int imgWidth, imgHeight;

    GLuint texID = loadTexture(
        "../assets/Modelos3D/untitled.png",
        imgWidth,
        imgHeight
    );

    glUseProgram(shaderID);

    glUniform1i(
        glGetUniformLocation(shaderID, "texBuffer"),
        0
    );

    //matrizes
    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projLoc = glGetUniformLocation(shaderID, "projection");

    //posições
    glm::vec3 objPositions[] =
    {
        glm::vec3(0,0,0),
        glm::vec3(2,0,0),
        glm::vec3(-2,0,0),
        glm::vec3(0,2,0)
    };

    //loop principal
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(1,1,1,1);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //view
        glm::mat4 view = glm::translate(
            glm::mat4(1.0f),
            glm::vec3(0.0f, 0.0f, -5.0f)
        );

        //projecao
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)WIDTH / (float)HEIGHT,
            0.1f,
            100.0f
        );

        glUniformMatrix4fv(
            viewLoc,
            1,
            GL_FALSE,
            glm::value_ptr(view)
        );

        glUniformMatrix4fv(
            projLoc,
            1,
            GL_FALSE,
            glm::value_ptr(projection)
        );

        //textura
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, texID);

        // obj
        glBindVertexArray(VAO);

        float angle = glfwGetTime();

        for (int i = 0; i < 4; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            // translação
            model = glm::translate(
                model,
                glm::vec3(posX, posY, posZ)
            );

            model = glm::translate(
                model,
                objPositions[i]
            );

            // escala
            model = glm::scale(
                model,
                glm::vec3(scaleFactor)
            );

            // rotação
            if (rotateX)
            {
                model = glm::rotate(
                    model,
                    angle,
                    glm::vec3(1,0,0)
                );
            }
            else if (rotateY)
            {
                model = glm::rotate(
                    model,
                    angle,
                    glm::vec3(0,1,0)
                );
            }
            else if (rotateZ)
            {
                model = glm::rotate(
                    model,
                    angle,
                    glm::vec3(0,0,1)
                );
            }
            else
            {
                model = glm::rotate(
                    model,
                    glm::radians(30.0f),
                    glm::vec3(1,1,0)
                );
            }

            glUniformMatrix4fv(
                modelLoc,
                1,
                GL_FALSE,
                glm::value_ptr(model)
            );

            glDrawArrays(
                GL_TRIANGLES,
                0,
                nVertices
            );
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();

    return 0;
}

// teclas do teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    float speed = 0.2f;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // rotacao
    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        rotateX = true;
        rotateY = false;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = true;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = false;
        rotateZ = true;
    }

    //movimentação
    if (key == GLFW_KEY_W)
        posZ -= speed;

    if (key == GLFW_KEY_S)
        posZ += speed;

    if (key == GLFW_KEY_A)
        posX -= speed;

    if (key == GLFW_KEY_D)
        posX += speed;

    if (key == GLFW_KEY_I)
        posY += speed;

    if (key == GLFW_KEY_J)
        posY -= speed;

    //escala
    if (key == GLFW_KEY_O)
    {
        scaleFactor -= 0.1f;

        if (scaleFactor < 0.1f)
            scaleFactor = 0.1f;
    }

    if (key == GLFW_KEY_K)
    {
        scaleFactor += 0.1f;
    }
}

// função do setup do shader
int setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,
        1,
        &vertexShaderSource,
        NULL
    );

    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,
        1,
        &fragmentShaderSource,
        NULL
    );

    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);

    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);

    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// função do load OBJ (baseado em https://github.com/guilhermechagaskurtz/CGCCHibrido/tree/main/Code%20snippets)
GLuint loadSimpleOBJ(string filePATH, int &nVertices)
{
    vector<glm::vec3> vertices;

    vector<glm::vec2> texCoords;

    vector<GLfloat> vBuffer;

    ifstream arqEntrada(filePATH.c_str());

    if (!arqEntrada.is_open())
    {
        cerr << "Erro ao abrir OBJ\n";

        return -1;
    }

    string line;

    while (getline(arqEntrada, line))
    {
        istringstream ssline(line);

        string word;

        ssline >> word;

        // vertices
        if (word == "v")
        {
            glm::vec3 v;

            ssline >> v.x >> v.y >> v.z;

            vertices.push_back(v);
        }

        // UV
        else if (word == "vt")
        {
            glm::vec2 vt;

            ssline >> vt.x >> vt.y;

            texCoords.push_back(vt);
        }

        // faces
        else if (word == "f")
            {
                vector<string> faceVertices;

                while (ssline >> word)
                {
                    faceVertices.push_back(word);
                }

                // triangulação em fan
                for (int i = 1; i < faceVertices.size() - 1; i++)
                {
                    string triangle[3] =
                    {
                        faceVertices[0],
                        faceVertices[i],
                        faceVertices[i + 1]
                    };

                    for (int j = 0; j < 3; j++)
                    {
                        int vi = 0;
                        int ti = 0;

                        istringstream ss(triangle[j]);

                        string index;

                        // índice do vértice
                        getline(ss, index, '/');
                        vi = stoi(index) - 1;

                        // índice da textura
                        getline(ss, index, '/');
                        ti = stoi(index) - 1;

                        // posição
                        vBuffer.push_back(vertices[vi].x);
                        vBuffer.push_back(vertices[vi].y);
                        vBuffer.push_back(vertices[vi].z);

                        // UV
                        vBuffer.push_back(texCoords[ti].x);
                        vBuffer.push_back(texCoords[ti].y);
                    }
                }
            }
    }

    arqEntrada.close();

    GLuint VBO;
    GLuint VAO;

    glGenBuffers(1, &VBO);

    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vBuffer.size() * sizeof(GLfloat),
        vBuffer.data(),
        GL_STATIC_DRAW
    );

    // posição
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    // UV
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),
        (void*)(3 * sizeof(float))
    );

    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    nVertices = vBuffer.size() / 5;

    return VAO;
}

// load da textura
GLuint loadTexture(string filePath, int &width, int &height)
{
    GLuint texID;

    glGenTextures(1, &texID);

    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    int nrChannels;

    unsigned char *data = stbi_load(
        filePath.c_str(),
        &width,
        &height,
        &nrChannels,
        0
    );

    if (data)
    {
        if (nrChannels == 3)
        {
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGB,
                width,
                height,
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                data
            );
        }
        else
        {
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                width,
                height,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                data
            );
        }

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Erro ao carregar textura\n";
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}