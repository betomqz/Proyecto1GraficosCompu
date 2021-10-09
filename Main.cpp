/*
* Código para generar una teselación de penrose.
* Autor: José Alberto Márquez Luján, 187917
* Fecha de entrega: 18 de septiembre de 2021
*
* Referencias: https://preshing.com/20110831/penrose-tiling-explained/
*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
// Fue necesario incluir otras bibliotecas para simplificar el código. Principalmente
// para manejar números complejos y estructuras dinámicas.
#include <cmath>
#include <list>
#include <complex>
#include <vector>
using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// Tamaño de la pantalla
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 1000;

// Vertex Shader
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
// Fragment Shader para los triángulos tipo cero
const char* fragmentShader1Source = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.043f, 0.145f, 0.271f, 1.0f);\n"
"}\n\0";
// Fragment Shader para los triángulos tipo uno
const char* fragmentShader2Source = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.698f, 0.761f, 0.929f, 1.0f);\n"
"}\n\0";

// #############################################################################
// Declaración de constantes a utilizar a lo largo de este programa
const int IMAGE_SIZE_X = SCR_WIDTH;
const int IMAGE_SIZE_Y = SCR_HEIGHT;
const int NUM_SUBDIVISONES = 7;
const double goldenRatio = (1 + sqrt(5)) / 2;
const double pi = 3.1415926535897932384626433832795028841971;

// Estructura que guarda la información de los triángulos a dibujar
struct triangulo {
    int color;
    complex<double> A;
    complex<double> B;
    complex<double> C;
};

// Método para subdividir todos los triángulos de manera que el resultante sea un
// tipo de estructura como la de Penrose. Recibe una lista de apuntadores a los
// triángulos ya creados y regresa otra lista con la subdivisión de los triángulos.
// Podría optimizarse liberando la memoria de los triángulos originales, pero no
// dio tiempo.
list<struct triangulo*> subdividir(list<struct triangulo*> triangulos) {
    list<struct triangulo*> resultado = {};
    list<struct triangulo*> ::iterator it;
    // Iteramos sobre la lista que nos pasaron como argumento
    for (it = triangulos.begin(); it != triangulos.end(); ++it) {
        // Subdividimos al triángulo según el tipo de color que tenga
        if ((*it)->color == 0) {
            complex<double> P;
            struct triangulo* t1 = (struct triangulo*)malloc(sizeof(struct triangulo));
            struct triangulo* t2 = (struct triangulo*)malloc(sizeof(struct triangulo));
            P = (*it)->A + ((*it)->B - (*it)->A) / goldenRatio;

            t1->color = 0;
            t1->A = (*it)->C;
            t1->B = P;
            t1->C = (*it)->B;

            t2->color = 1;
            t2->A = P;
            t2->B = (*it)->C;
            t2->C = (*it)->A;

            resultado.push_back(t1);
            resultado.push_back(t2);
        }
        else {
            complex<double> Q;
            complex<double> R;
            struct triangulo* t1 = (struct triangulo*)malloc(sizeof(struct triangulo));
            struct triangulo* t2 = (struct triangulo*)malloc(sizeof(struct triangulo));
            struct triangulo* t3 = (struct triangulo*)malloc(sizeof(struct triangulo));

            Q = (*it)->B + ((*it)->A - (*it)->B) / goldenRatio;
            R = (*it)->B + ((*it)->C - (*it)->B) / goldenRatio;

            t1->color = 1;
            t1->A = R;
            t1->B = (*it)->C;
            t1->C = (*it)->A;

            t2->color = 1;
            t2->A = Q;
            t2->B = R;
            t2->C = (*it)->B;

            t3->color = 0;
            t3->A = R;
            t3->B = Q;
            t3->C = (*it)->A;

            resultado.push_back(t1);
            resultado.push_back(t2);
            resultado.push_back(t3);
        }
    }
    return resultado;
}

// Este método no tiene una aplicación real en el código; sin embargo, lo utilicé para asegurarme
// de que los valores que estaba generando el algoritmo fueran los correctos.
void imprimeTriangulos(list<struct triangulo*> triangulos) {
    list<struct triangulo*> ::iterator it;
    for (it = triangulos.begin(); it != triangulos.end(); ++it) {
        printf("%f, %f, 0.0 \n%f, %f, 0.0 \n%f, %f, 0.0\n", (*it)->A.real(), (*it)->A.imag(),
            (*it)->B.real(), (*it)->B.imag(), (*it)->C.real(), (*it)->C.imag());
    }
}
// #########################################################################################

// Método principal
int main()
{
    // Parte para calcular lo de Penrose
    // Empezamos con 10 triángulos alrededor del origen.
    list<struct triangulo*> triangulos = {};
    complex<double> A(0, 0);

    for (int j = 0; j < 10; j++) {
        struct triangulo* t = (struct triangulo*)malloc(sizeof(struct triangulo));
        t->color = 0;
        t->A = A;
        t->B = polar(1.0, ((2 * j - 1) * pi) / 10.0);
        t->C = polar(1.0, ((2 * j + 1) * pi) / 10.0);
        if (j % 2 == 0) {
            complex<double> aux = t->B;
            t->B = t->C;
            t->C = aux;
        }
        triangulos.push_back(t);
    }

    // Subdividimos los triángulos las veces que indice la constante NUM_SUBDIVISIONES.
    for (int j = 0; j < NUM_SUBDIVISONES; j++) {
        triangulos = subdividir(triangulos);
    }

    // Guardamos los triángulos en formato de vértices para OpenGL.
    list<struct triangulo*> ::iterator it;
    vector<float> vert_ceros;
    vector<float> vert_unos;
    for (it = triangulos.begin(); it != triangulos.end(); ++it) {
        if ((*it)->color == 0) {
            vert_ceros.push_back((float)(*it)->A.real());
            vert_ceros.push_back((float)(*it)->A.imag());
            vert_ceros.push_back(0.0);
            vert_ceros.push_back((float)(*it)->B.real());
            vert_ceros.push_back((float)(*it)->B.imag());
            vert_ceros.push_back(0.0);
            vert_ceros.push_back((float)(*it)->C.real());
            vert_ceros.push_back((float)(*it)->C.imag());
            vert_ceros.push_back(0.0);
        }
        else {
            vert_unos.push_back((float)(*it)->A.real());
            vert_unos.push_back((float)(*it)->A.imag());
            vert_unos.push_back(0.0);
            vert_unos.push_back((float)(*it)->B.real());
            vert_unos.push_back((float)(*it)->B.imag());
            vert_unos.push_back(0.0);
            vert_unos.push_back((float)(*it)->C.real());
            vert_unos.push_back((float)(*it)->C.imag());
            vert_unos.push_back(0.0);
        }
    }
    // #######################################################################################

    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw creación de la ventana
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Teselaciones de Penrose - 187917", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // construimos y compilamos los shaders
    // ------------------------------------
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShaderCero = glCreateShader(GL_FRAGMENT_SHADER);
    unsigned int fragmentShaderUno = glCreateShader(GL_FRAGMENT_SHADER);
    unsigned int shaderProgramCero = glCreateProgram();
    unsigned int shaderProgramUno = glCreateProgram();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glShaderSource(fragmentShaderCero, 1, &fragmentShader1Source, NULL);
    glCompileShader(fragmentShaderCero);
    glShaderSource(fragmentShaderUno, 1, &fragmentShader2Source, NULL);
    glCompileShader(fragmentShaderUno);

    glAttachShader(shaderProgramCero, vertexShader);
    glAttachShader(shaderProgramCero, fragmentShaderCero);
    glLinkProgram(shaderProgramCero);

    glAttachShader(shaderProgramUno, vertexShader);
    glAttachShader(shaderProgramUno, fragmentShaderUno);
    glLinkProgram(shaderProgramUno);

    // ------------------------------------------------------------------
    unsigned int VBOs[2], VAOs[2];
    glGenVertexArrays(2, VAOs); // Generamos dos VAOs y dos Buffers
    glGenBuffers(2, VBOs);
    // Triángulos tipo cero
    // --------------------
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, vert_ceros.size() * sizeof(float), &vert_ceros[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Triángulos tipo uno
    // ---------------------
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, vert_unos.size() * sizeof(float), &vert_unos[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);


    // Para dibujar únicamente los bordes
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Color de fondo rgb(87.1%,87.8%,90.2%)
        glClearColor(0.871f, 0.878f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Ahora sí, propiamente, dibujamos los triángulos tipo cero
        glUseProgram(shaderProgramCero);
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, vert_ceros.size());
        // Análogamente, dibujamos los triángulos tipo uno
        glUseProgram(shaderProgramUno);
        glBindVertexArray(VAOs[1]);
        glDrawArrays(GL_TRIANGLES, 0, vert_unos.size());

        // glfw: swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    glDeleteProgram(shaderProgramCero);
    glDeleteProgram(shaderProgramUno);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // 
    if (width >= height) {
        glViewport(0, 0, width, width);
    }
    else {
        glViewport(0, 0, height, height);
    }
    //glViewport(0, 0, width, height);
}