#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <experimental/filesystem>
#include <iomanip>
#include <iostream>
#include <vector>
#include <string.h>
#include <glm\fwd.hpp>
#include <glm\ext\vector_float3.hpp>
#include <glm\geometric.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\ext.hpp>
#include <stb_image.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ShaderProgram.hpp"

#define M_PI 3.14

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 1280;

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

int countMax = 1000;
int countActual = 0;
float delta = 1.0f;
float offsetX = 0.0f;
float offsetY = 0.0f;
float* vertexes = nullptr;

glm::mat4 matrix = glm::lookAt(glm::vec3{ 0.7f, 0.2f, 0.5f }, { 0.0f, 0.f, 0.5f }, { 0.f, 1.f, 0.f });

// Загружает для куба картинки
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}



// common/camera
float* make_coords();

//Функция обратного вызова для обработки событий клавиатуры
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        //Если нажата клавиша ESCAPE, то закрываем окно
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_S) {
        const float alpha = 0.01;
        matrix *= glm::mat4(glm::vec4{ cos(alpha), sin(alpha), 0, 0 }, { -sin(alpha), cos(alpha), 0, 0 }, { 0, 0, 1, 0 }, glm::vec4{ 0, 0, 0, 1 });
    }

    if (key == GLFW_KEY_W) {
        const float alpha = -0.01;
        matrix *= glm::mat4(glm::vec4{ cos(alpha), sin(alpha), 0, 0 }, { -sin(alpha), cos(alpha), 0, 0 }, { 0, 0, 1, 0 }, glm::vec4{ 0, 0, 0, 1 });
    }

    if (key == GLFW_KEY_D) {
        const float beta = 0.01;
        matrix *= glm::mat4(glm::vec4{ 1, 0, 0, 0 }, { 0, cos(beta), sin(beta), 0 }, { 0, -sin(beta), cos(beta), 0 }, glm::vec4{ 0, 0, 0, 1 });
    }

    if (key == GLFW_KEY_A) {
        const float beta = -0.01;
        matrix *= glm::mat4(glm::vec4{ 1, 0, 0, 0 }, { 0, cos(beta), sin(beta), 0 }, { 0, -sin(beta), cos(beta), 0 }, glm::vec4{ 0, 0, 0, 1 });
    }


}

void add(float* answer, std::vector<std::vector<glm::vec3>>& points, int i, int j) {
    answer[countActual++] = static_cast<float>(points[i][j].x);
    answer[countActual++] = static_cast<float>(points[i][j].y);
    answer[countActual++] = static_cast<float>(points[i][j].z);
}

void add(float* answer, glm::vec3 a) {
    answer[countActual++] = static_cast<float>(a.x);
    answer[countActual++] = static_cast<float>(a.y);
    answer[countActual++] = static_cast<float>(a.z);
}

float* make_coords() {


    const float aa = 1;
    const float bb = 1;
    const float cc = 1;

    const float u_max = M_PI;

    const float v_max = M_PI;

    std::vector<std::vector<glm::vec3>> points;
    int count_v = countMax;
    int count_u = count_v;
    countActual = 0;

    float deltaU = 2 * u_max / (count_u);
    float deltaV = 2 * v_max / (count_v);


    for (float u = -u_max, k = 0; k <= count_u; u = -u_max + 2 * u_max * k / (count_u), k++) {
        std::vector<glm::vec3> pointTemp;
        for (float v = -v_max, k1 = 0; k1 <= count_v; v = -v_max + 2 * v_max * k1 / (count_v), k1++) {
            float x = aa * sin(u) * cos(v) * delta + offsetX;
            float y = bb * sin(u) * sin(v) * delta;
            float z = cc * cos(u) * delta;

            pointTemp.push_back({ x * 0.2, y*0.3, z *0.4});
        }
        points.push_back(pointTemp);
    }

    float* answer = new float[(points.size() * points[0].size() + 2) * 36];

    for (int i = points.size(); i > 0; i--) {
        for (int j = 1; j <= points[0].size(); j++) {
            auto cros = cross(points[i - 1][j - 1] - points[i % points.size()][j - 1], points[i - 1][j % points[0].size()] - points[i % points.size()][j - 1]);
            float len = length(cros);
            cros /= len;


            if (len < 0.0000001) {
                continue;
            }

            add(answer, points, i - 1, j - 1);
            add(answer, cros);
            add(answer, points, i % static_cast<int>(points.size()), j - 1);
            add(answer, cros);
            add(answer, points, i - 1, j % static_cast<int>(points[0].size()));
            add(answer, cros);

            add(answer, points, i % static_cast<int>(points.size()), j - 1);
            add(answer, cros);
            add(answer, points, i - 1, j % points[0].size());
            add(answer, cros);
            add(answer, points, i % points.size(), j % points[0].size());
            add(answer, cros);

        }

    }

    std::cout << "answer_returned" << std::endl;
    std::cout << countActual << std::endl;
    return answer;
}

glm::vec3 a = glm::vec3{ 0.7f, 0.2f, 0.5f };

int main()
{
    // This sample shows that Data files are overwritten on each run using IDE.
    using namespace std::experimental;


    //Инициализируем библиотеку GLFW
    if (!glfwInit())
    {
        std::cerr << "ERROR: could not start GLFW3\n";
        exit(1);
    }

    //Устанавливаем параметры создания графического контекста
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Создаем графический контекст (окно)
    GLFWwindow* window = glfwCreateWindow(800, 600, "MIPT OpenGL demos", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "ERROR: could not open window with GLFW3\n";
        glfwTerminate();
        exit(1);
    }

    //Делаем этот контекст текущим
    glfwMakeContextCurrent(window);

    //Устанавливаем функцию обратного вызова для обработки событий клавиатуры
    glfwSetKeyCallback(window, keyCallback);

    //Инициализируем библиотеку GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    //=========================================================

    //Координаты вершин треугольника

    vertexes = make_coords();


    //Создаем буфер VertexBufferObject для хранения координат на видеокарте
    GLuint vbo;
    glGenBuffers(1, &vbo);

    //Делаем этот буфер текущим
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    //Копируем содержимое массива в буфер на видеокарте
    glBufferData(GL_ARRAY_BUFFER, countActual * sizeof(float), vertexes, GL_STATIC_DRAW);

    //=========================================================

    //Создаем объект VertexArrayObject для хранения настроек полигональной модели
    GLuint vao;
    glGenVertexArrays(1, &vao);

    //Делаем этот объект текущим
    glBindVertexArray(vao);

    //Делаем буфер с координатами текущим
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    //Включаем 0й вершинный атрибут
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    //Устанавливаем настройки:
    //0й атрибут,
    //3 компоненты типа GL_FLOAT,
    //не нужно нормализовать,
    //0 - значения расположены в массиве впритык,
    //0 - сдвиг от начала
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), reinterpret_cast<void*>(0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), reinterpret_cast<void*>(3 * sizeof(GL_FLOAT)));

    //=========================================================

    //Вершинный шейдер
    const char* vertexShaderText =
        "#version 450\n"

        "layout(location = 0) in vec3 vertexPosition;\n"
        "layout(location = 1) in vec3 normal;"
        "layout(location = 2) uniform mat4 viewMatrix;\n"
        "layout(location = 3) uniform mat4 projection;\n"
        "layout(location = 4) uniform vec3 CV;\n"
        ""

        "out vec3 R;"

        "void main()\n"
        "{\n"
        "    gl_Position = projection * viewMatrix * vec4(vertexPosition, 1.0);\n"
        "vec3 Normal2 =   ( projection * viewMatrix * vec4( mat3(transpose(inverse(mat3(1.0))))  * normal, 1.0)).xyz;"
        "vec3 I = (projection * viewMatrix * vec4(vertexPosition, 1.0)).xyz - (projection * viewMatrix * vec4(CV, 1.0)).xyz;"
        "R = reflect(I, normal);"
        "}\n";

    GLuint vs2 = glCreateShader(GL_VERTEX_SHADER);

    //Создаем шейдерный объект
    vs2 = glCreateShader(GL_VERTEX_SHADER);

    //Передаем в шейдерный объект текст шейдера
    glShaderSource(vs2, 1, &vertexShaderText, nullptr);

    //Компилируем шейдер
    glCompileShader(vs2);

    //Проверяем ошибки компиляции
    int status = -1;
    glGetShaderiv(vs2, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint errorLength;
        glGetShaderiv(vs2, GL_INFO_LOG_LENGTH, &errorLength);

        std::vector<char> errorMessage;
        errorMessage.resize(errorLength);

        glGetShaderInfoLog(vs2, errorLength, 0, errorMessage.data());

        std::cerr << "Failed to compile the shader:\n" << errorMessage.data() << std::endl;

        exit(1);
    }

    //=========================================================

    //Фрагментный шейдер
    const char* fragmentShaderText =
        "#version 330\n"

        "out vec4 fragColor; uniform samplerCube skybox;\n"

        "in vec3 R;\n"

        "void main()\n"
        "{\n"
        ""
        "fragColor = vec4(texture(skybox, R).rgb, 1.0);\n"
        "}\n";


    //Создаем шейдерный объект
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    //Передаем в шейдерный объект текст шейдера
    glShaderSource(fs, 1, &fragmentShaderText, nullptr);

    //Компилируем шейдер
    glCompileShader(fs);

    //Проверяем ошибки компиляции
    status = -1;
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint errorLength;
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &errorLength);

        std::vector<char> errorMessage;
        errorMessage.resize(errorLength);

        glGetShaderInfoLog(fs, errorLength, 0, errorMessage.data());

        std::cerr << "Failed to compile the shader:\n" << errorMessage.data() << std::endl;

        exit(1);
    }

    //=========================================================

    //Создаем шейдерную программу
    GLuint program = glCreateProgram();

    //Прикрепляем шейдерные объекты
    glAttachShader(program, vs2);
    glAttachShader(program, fs);

    //Линкуем программу
    glLinkProgram(program);

    //Проверяем ошибки линковки
    status = -1;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint errorLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &errorLength);

        std::vector<char> errorMessage;
        errorMessage.resize(errorLength);

        glGetProgramInfoLog(program, errorLength, 0, errorMessage.data());

        std::cerr << "Failed to link the program:\n" << errorMessage.data() << std::endl;

        exit(1);
    }

    //=========================================================

    ShaderProgram skyboxShader("C:\\arina5arina-opengl_tasks_2020-bffa363fcb2d\\task2\\692Naumov\\pic.vs", 
        "C:\\arina5arina-opengl_tasks_2020-bffa363fcb2d\\task2\\692Naumov\\pic.fs");

    GLuint skyboxVAO, skyboxVBO;

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    skyboxShader.use();
    skyboxShader.setIntUniform("skybox", 0);

        // Картинки загружаем
    std::vector<std::string> faces{
        "C:\\arina5arina-opengl_tasks_2020-bffa363fcb2d\\task2\\692Naumov\\surface\\right.jpg",
            "C:\\arina5arina-opengl_tasks_2020-bffa363fcb2d\\task2\\692Naumov\\surface\\left.jpg",
            "C:\\arina5arina-opengl_tasks_2020-bffa363fcb2d\\task2\\692Naumov\\surface\\top.jpg",
            "C:\\arina5arina-opengl_tasks_2020-bffa363fcb2d\\task2\\692Naumov\\surface\\bottom.jpg",
            "C:\\arina5arina-opengl_tasks_2020-bffa363fcb2d\\task2\\692Naumov\\surface\\front.jpg",
            "C:\\arina5arina-opengl_tasks_2020-bffa363fcb2d\\task2\\692Naumov\\surface\\back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    //Цикл рендеринга (пока окно не закрыто)

    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);

        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(matrix));

        glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(glm::perspective((float)glm::radians(100.0f), (float)width / (float)height, 0.01f, 10.0f)));
        
        glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(a));

            //Проверяем события ввода (здесь вызывается функция обратного вызова для обработки событий клавиатуры)
        glfwPollEvents();

        //Получаем размеры экрана (окна)
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        //Устанавливаем порт вывода на весь экран (окно)
        glViewport(0, 0, width, height);


        //Очищаем порт вывода (буфер цвета и буфер глубины)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();

        // remove translation from the view matrix
        skyboxShader.setMat4Uniform("view", glm::mat4(glm::mat3(matrix)));
        skyboxShader.setMat4Uniform("projection", glm::perspective((float)glm::radians(100.0f), (float)width / (float)height, 0.01f, 10.0f));
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);



        //Подключаем шейдерную программу
        glUseProgram(program);

        //Подключаем VertexArrayObject с настойками полигональной модели
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, countActual / 3);
        glBindVertexArray(0);
        
      

        
        glfwSwapBuffers(window); //Переключаем передний и задний буферы

        glfwPollEvents();
    }

    //Удаляем созданные объекты OpenGL
    glDeleteProgram(program);
    glDeleteShader(vs2);
    glDeleteShader(fs);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();

    return 0;
}