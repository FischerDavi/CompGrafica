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

// estrutura do obj
struct OBJ
{
    GLuint VAO;
    int nVertices;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

// prototipos
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

int setupShader();

GLuint loadSimpleOBJ(string filePATH, int &nVertices);

GLuint loadTexture(string filePath, int &width, int &height);

// tela
const GLuint WIDTH = 1000;
const GLuint HEIGHT = 1000;

// objetos
vector<OBJ> objetos;

int objetoSelecionado = 0;

// luzes
bool keyLightEnabled = true;
bool fillLightEnabled = true;
bool backLightEnabled = true;

// shaders
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec2 texCoord;\n"
"layout (location = 2) in vec3 normal;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"out vec2 TexCoord;\n"
"out vec3 Normal;\n"
"out vec3 FragPos;\n"
"\n"
"void main()\n"
"{\n"
"   FragPos = vec3(model * vec4(position, 1.0));\n"
"   Normal = mat3(transpose(inverse(model))) * normal;\n"
"\n"
"   gl_Position = projection * view * vec4(FragPos, 1.0);\n"
"\n"
"   TexCoord = texCoord;\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 450\n"

"in vec2 TexCoord;\n"
"in vec3 Normal;\n"
"in vec3 FragPos;\n"

"uniform sampler2D texBuffer;\n"

"uniform vec3 viewPos;\n"

"uniform vec3 keyLightPos;\n"
"uniform vec3 fillLightPos;\n"
"uniform vec3 backLightPos;\n"

"uniform bool keyEnabled;\n"
"uniform bool fillEnabled;\n"
"uniform bool backEnabled;\n"

"out vec4 color;\n"

"vec3 calculateLight(\n"
"   vec3 lightPos,\n"
"   vec3 lightColor,\n"
"   float intensity\n"
")\n"
"{\n"
"   vec3 ambient = 0.15 * lightColor;\n"

"   vec3 norm = normalize(Normal);\n"
"   vec3 lightDir = normalize(lightPos - FragPos);\n"

"   float diff = max(dot(norm, lightDir), 0.0);\n"

"   float distance = length(lightPos - FragPos);\n"

"   float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);\n"

"   vec3 diffuse = diff * lightColor * attenuation * intensity;\n"

"   vec3 viewDir = normalize(viewPos - FragPos);\n"
"   vec3 reflectDir = reflect(-lightDir, norm);\n"

"   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"

"   vec3 specular = spec * lightColor * attenuation * 0.5;\n"

"   return ambient + diffuse + specular;\n"
"}\n"

"void main()\n"
"{\n"

"   vec3 result = vec3(0.0);\n"

"   if(keyEnabled)\n"
"   {\n"
"       result += calculateLight(\n"
"           keyLightPos,\n"
"           vec3(1.0, 1.0, 1.0),\n"
"           1.0\n"
"       );\n"
"   }\n"

"   if(fillEnabled)\n"
"   {\n"
"       result += calculateLight(\n"
"           fillLightPos,\n"
"           vec3(0.7, 0.7, 0.7),\n"
"           0.4\n"
"       );\n"
"   }\n"

"   if(backEnabled)\n"
"   {\n"
"       result += calculateLight(\n"
"           backLightPos,\n"
"           vec3(1.0, 1.0, 1.0),\n"
"           0.7\n"
"       );\n"
"   }\n"

"   vec4 texColor = texture(texBuffer, TexCoord);\n"

"   color = vec4(result, 1.0) * texColor;\n"
"}\0";

// main
int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "AV2 - Iluminacao 3 Pontos",
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

    GLuint shaderID = setupShader();

    // textura
    int imgWidth, imgHeight;

    GLuint texID = loadTexture(
        "../assets/Modelos3D/Suzanne.png",
        imgWidth,
        imgHeight
    );

    // obj1
    OBJ obj1;

    obj1.VAO = loadSimpleOBJ(
        "../assets/Modelos3D/Suzanne.obj",
        obj1.nVertices
    );

    obj1.position = glm::vec3(-1.5f, 0.0f, 0.0f);
    obj1.rotation = glm::vec3(0.0f);
    obj1.scale = glm::vec3(1.0f);

    objetos.push_back(obj1);

    // obj2
    OBJ obj2;

    obj2.VAO = loadSimpleOBJ(
        "../assets/Modelos3D/Suzanne.obj",
        obj2.nVertices
    );

    obj2.position = glm::vec3(1.5f, 0.0f, 0.0f);
    obj2.rotation = glm::vec3(0.0f);
    obj2.scale = glm::vec3(1.0f);

    objetos.push_back(obj2);

    // shader
    glUseProgram(shaderID);

    glUniform1i(
        glGetUniformLocation(shaderID, "texBuffer"),
        0
    );

    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projLoc = glGetUniformLocation(shaderID, "projection");

    // loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // camera
        glm::mat4 view = glm::translate(
            glm::mat4(1.0f),
            glm::vec3(0.0f, 0.0f, -6.0f)
        );

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

        //camera pos
        glUniform3f(
            glGetUniformLocation(shaderID, "viewPos"),
            0.0f,
            0.0f,
            6.0f
        );

        // luzes posição e enable
        OBJ principal = objetos[0];

        glm::vec3 keyLight =
            principal.position + glm::vec3(3.0f, 3.0f, 3.0f);

        glm::vec3 fillLight =
            principal.position + glm::vec3(-3.0f, 1.0f, 2.0f);

        glm::vec3 backLight =
            principal.position + glm::vec3(0.0f, 2.0f, -4.0f);

        glUniform3fv(
            glGetUniformLocation(shaderID, "keyLightPos"),
            1,
            glm::value_ptr(keyLight)
        );

        glUniform3fv(
            glGetUniformLocation(shaderID, "fillLightPos"),
            1,
            glm::value_ptr(fillLight)
        );

        glUniform3fv(
            glGetUniformLocation(shaderID, "backLightPos"),
            1,
            glm::value_ptr(backLight)
        );

        glUniform1i(
            glGetUniformLocation(shaderID, "keyEnabled"),
            keyLightEnabled
        );

        glUniform1i(
            glGetUniformLocation(shaderID, "fillEnabled"),
            fillLightEnabled
        );

        glUniform1i(
            glGetUniformLocation(shaderID, "backEnabled"),
            backLightEnabled
        );

        //textura
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, texID);

        // desenha obj
        for (int i = 0; i < objetos.size(); i++)
        {
            OBJ obj = objetos[i];

            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(
                model,
                obj.position
            );

            model = glm::rotate(
                model,
                glm::radians(obj.rotation.x),
                glm::vec3(1,0,0)
            );

            model = glm::rotate(
                model,
                glm::radians(obj.rotation.y),
                glm::vec3(0,1,0)
            );

            model = glm::rotate(
                model,
                glm::radians(obj.rotation.z),
                glm::vec3(0,0,1)
            );

            model = glm::scale(
                model,
                obj.scale
            );

            glUniformMatrix4fv(
                modelLoc,
                1,
                GL_FALSE,
                glm::value_ptr(model)
            );

            glBindVertexArray(obj.VAO);

            glDrawArrays(
                GL_TRIANGLES,
                0,
                obj.nVertices
            );
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();

    return 0;
}

// teclado 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    float speed = 0.2f;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // seleção
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        objetoSelecionado++;

        if (objetoSelecionado >= objetos.size())
        {
            objetoSelecionado = 0;
        }

        cout << "Objeto selecionado: "
             << objetoSelecionado
             << endl;
    }

    // luzes
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        keyLightEnabled = !keyLightEnabled;
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        fillLightEnabled = !fillLightEnabled;
    }

    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        backLightEnabled = !backLightEnabled;
    }

    OBJ &obj = objetos[objetoSelecionado];

    // movimento
    if (key == GLFW_KEY_W)
        obj.position.z -= speed;

    if (key == GLFW_KEY_S)
        obj.position.z += speed;

    if (key == GLFW_KEY_A)
        obj.position.x -= speed;

    if (key == GLFW_KEY_D)
        obj.position.x += speed;

    if (key == GLFW_KEY_I)
        obj.position.y += speed;

    if (key == GLFW_KEY_J)
        obj.position.y -= speed;

    // escala
    if (key == GLFW_KEY_O)
    {
        obj.scale -= glm::vec3(0.1f);

        if (obj.scale.x < 0.1f)
            obj.scale = glm::vec3(0.1f);
    }

    if (key == GLFW_KEY_K)
    {
        obj.scale += glm::vec3(0.1f);
    }

    // rotação
    if (key == GLFW_KEY_X)
        obj.rotation.x += 10.0f;

    if (key == GLFW_KEY_Y)
        obj.rotation.y += 10.0f;

    if (key == GLFW_KEY_Z)
        obj.rotation.z += 10.0f;
}

// setup do shader
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

//load obj
GLuint loadSimpleOBJ(string filePATH, int &nVertices)
{
    vector<glm::vec3> vertices;

    vector<glm::vec2> texCoords;

    vector<glm::vec3> normals;

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

        // normals
        else if (word == "vn")
        {
            glm::vec3 vn;

            ssline >> vn.x >> vn.y >> vn.z;

            normals.push_back(vn);
        }

        // faces
        else if (word == "f")
        {
            vector<string> faceVertices;

            while (ssline >> word)
            {
                faceVertices.push_back(word);
            }

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
                    int ni = 0;

                    istringstream ss(triangle[j]);

                    string index;

                    getline(ss, index, '/');
                    vi = stoi(index) - 1;

                    getline(ss, index, '/');
                    ti = stoi(index) - 1;

                    getline(ss, index, '/');
                    ni = stoi(index) - 1;

                    // posição
                    vBuffer.push_back(vertices[vi].x);
                    vBuffer.push_back(vertices[vi].y);
                    vBuffer.push_back(vertices[vi].z);

                    // UV
                    vBuffer.push_back(texCoords[ti].x);
                    vBuffer.push_back(texCoords[ti].y);

                    // normal
                    vBuffer.push_back(normals[ni].x);
                    vBuffer.push_back(normals[ni].y);
                    vBuffer.push_back(normals[ni].z);
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
        8 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    // UV
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        (void*)(3 * sizeof(float))
    );

    glEnableVertexAttribArray(1);

    // normal
    glVertexAttribPointer(
        2,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        (void*)(5 * sizeof(float))
    );

    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    nVertices = vBuffer.size() / 8;

    return VAO;
}

//textura
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

    stbi_set_flip_vertically_on_load(true);

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