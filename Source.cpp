/*
* C�digo para generar una teselaci�n de penrose.
* Autor: Jos� Alberto M�rquez Luj�n, 187917
* Fecha de entrega: 18 de septiembre de 2021
*
* Referencias: https://preshing.com/20110831/penrose-tiling-explained/
*/

//#define STB_IMAGE_IMPLEMENTATION
#include <glad.h>
#include <glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"

#include <iostream>
// Fue necesario incluir otras bibliotecas para simplificar el c�digo. Principalmente
// para manejar n�meros complejos y estructuras din�micas.
#include <cmath>
#include <list>
#include <complex>
#include <vector>
using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// Tama�o de la pantalla
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 1000;

// #############################################################################
// Declaraci�n de constantes a utilizar a lo largo de este programa
const int IMAGE_SIZE_X = SCR_WIDTH;
const int IMAGE_SIZE_Y = SCR_HEIGHT;
const int NUM_SUBDIVISONES = 7;
const double goldenRatio = (1 + sqrt(5)) / 2;
const double pi = 3.1415926535897932384626433832795028841971;

//Control del tiempo
/* 0: ### Inicio
*  1: ### Tri�ngulo entra en escena
*  2: ### Tri�ngulo se para un momento
*  3: ### Tri�ngulo se mueve hacia la trselaci�n y choca.
*  4: ### Tri�ngulo se para un momento
*  5: ### Tri�ngulo choca varias veces m�s.
*  6: ### Tri�ngulo rota hasta quedar paralelo al suelo
*  7: ### Tri�ngulo se cae
*  8: ### Tri�ngulo ca�do se cambia de color
*  9: ### Nuevo tri�ngulo sube
* 10: ### Tri�ngulo rota hasta quedar como estaba antes
* 11: ### Tri�ngulo nuevo se para un momento
* 12: ### Tri�ngulo nuevo se incorpora a la teselaci�n
* 13: ### La teselaci�n completa se hace grande
*/
//                          0     1     2     3     4     5     6     7     8     9     10    11    12    13
const float tiempos[14] = { 2.0f, 1.0f, 1.5f, 1.5f, 0.5f, 2.0f, 0.5f, 0.5f, 4.0f, 2.0f, 2.0f, 1.0f, 2.0f, 3.5f };
int tiempoIndex = 0;

// Estructura que guarda la informaci�n de los tri�ngulos a dibujar
struct triangulo {
    int color;
    complex<double> A;
    complex<double> B;
    complex<double> C;
};

// M�todo para subdividir todos los tri�ngulos de manera que el resultante sea un
// tipo de estructura como la de Penrose. Recibe una lista de apuntadores a los
// tri�ngulos ya creados y regresa otra lista con la subdivisi�n de los tri�ngulos.
// Podr�a optimizarse liberando la memoria de los tri�ngulos originales.
list<struct triangulo*> subdividir(list<struct triangulo*> triangulos) {
    list<struct triangulo*> resultado = {};
    list<struct triangulo*> ::iterator it;
    // Iteramos sobre la lista que nos pasaron como argumento
    for (it = triangulos.begin(); it != triangulos.end(); ++it) {
        // Subdividimos al tri�ngulo seg�n el tipo de color que tenga
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

// Este m�todo no tiene una aplicaci�n real en el c�digo; sin embargo, lo utilic� para asegurarme
// de que los valores que estaba generando el algoritmo fueran los correctos.
void imprimeTriangulos(list<struct triangulo*> triangulos) {
    list<struct triangulo*> ::iterator it;
    for (it = triangulos.begin(); it != triangulos.end(); ++it) {
        printf("%f, %f, 0.0 \n%f, %f, 0.0 \n%f, %f, 0.0\n", (*it)->A.real(), (*it)->A.imag(),
            (*it)->B.real(), (*it)->B.imag(), (*it)->C.real(), (*it)->C.imag());
    }
}
// #########################################################################################

// M�todo principal
int main()
{
    // Parte para calcular lo de Penrose
    // Empezamos con 10 tri�ngulos alrededor del origen.
    list<struct triangulo*> triangulos = {};    // Teselaci�n principal. Estar� incompleta
    list<struct triangulo*> triProtag = {};     // Tri�ngulo protagonista.
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

        // Si es el tri�ngulo protagonista, lo pondremos en el arreglo aparte.
        if (j == 0)
            triProtag.push_back(t);
        else
            triangulos.push_back(t);
    }

    // Subdividimos los tri�ngulos las veces que indice la constante NUM_SUBDIVISIONES.
    for (int j = 0; j < NUM_SUBDIVISONES; j++) {
        triangulos = subdividir(triangulos);
        triProtag = subdividir(triProtag);
    }

    // Guardamos los tri�ngulos en formato de v�rtices para OpenGL.
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
    // Hacemos lo mismo con el tri�ngulo protagonista
    vector<float> vert_ceros_protag;
    vector<float> vert_unos_protag;
    for (it = triProtag.begin(); it != triProtag.end(); ++it) {
        if ((*it)->color == 0) {
            vert_ceros_protag.push_back((float)(*it)->A.real());
            vert_ceros_protag.push_back((float)(*it)->A.imag());
            vert_ceros_protag.push_back(0.0);
            vert_ceros_protag.push_back((float)(*it)->B.real());
            vert_ceros_protag.push_back((float)(*it)->B.imag());
            vert_ceros_protag.push_back(0.0);
            vert_ceros_protag.push_back((float)(*it)->C.real());
            vert_ceros_protag.push_back((float)(*it)->C.imag());
            vert_ceros_protag.push_back(0.0);
        }
        else {
            vert_unos_protag.push_back((float)(*it)->A.real());
            vert_unos_protag.push_back((float)(*it)->A.imag());
            vert_unos_protag.push_back(0.0);
            vert_unos_protag.push_back((float)(*it)->B.real());
            vert_unos_protag.push_back((float)(*it)->B.imag());
            vert_unos_protag.push_back(0.0);
            vert_unos_protag.push_back((float)(*it)->C.real());
            vert_unos_protag.push_back((float)(*it)->C.imag());
            vert_unos_protag.push_back(0.0);
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

    // glfw creaci�n de la ventana
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
    //Shader ourShader("proyecto1.vs.txt", "proyecto1.fs.txt");
    Shader ourShader("proyecto1.vs", "proyecto1.fs");

    // ------------------------------------------------------------------
    unsigned int VBOs[4], VAOs[4];
    glGenVertexArrays(4, VAOs); // Generamos cuatro VAOs y dos Buffers
    glGenBuffers(4, VBOs);
    // Tri�ngulos tipo cero de la teselaci�n PRINCIPAL
    // --------------------
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, vert_ceros.size() * sizeof(float), &vert_ceros[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Tri�ngulos tipo uno de la teselaci�n PRINCIPAL
    // ---------------------
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, vert_unos.size() * sizeof(float), &vert_unos[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    // Tri�ngulos tipo cero del tri�ngulo PROTAGONISTA
    // --------------------
    glBindVertexArray(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, vert_ceros_protag.size() * sizeof(float), &vert_ceros_protag[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Tri�ngulos tipo uno del tri�ngulo PROTAGONISTA
    // ---------------------
    glBindVertexArray(VAOs[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
    glBufferData(GL_ARRAY_BUFFER, vert_unos_protag.size() * sizeof(float), &vert_unos_protag[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);


    // Para dibujar �nicamente los bordes
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  

    glfwSetTime(0.0f);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Variables
        float scaleAmount = 1.0f;
        float aux;
        float rotAux = 0.0f;
        // Declaraci�n de las matrices de transformaci�n que vamos a estar usando. Son la identidad por default.
        glm::mat4 transform = glm::mat4(1.0f);          // Matriz para transformar la teselaci�n principal
        glm::mat4 transform_protag = glm::mat4(1.0f);   // Matriz para transformar al tri�ngulo protagonista

        // Colores
        GLfloat color1[] = { 0.043f, 0.145f, 0.271f };                  // Color 1 de la teselaci�n principal
        GLfloat color2[] = { 0.698f, 0.761f, 0.929f };                  // Color 2 de la teselaci�n principal
        GLfloat color_cero_protag[] = { 0.8705f, 0.7686f, 0.2509f };    // Color cero del tri�ngulo protagonista
        GLfloat color_uno_protag[] = { 0.8705f, 0.7686f, 0.2509f };     // Color uno del tri�ngulo protagonista

        // Control de tiempos
        if (tiempoIndex < sizeof(tiempos) / sizeof(float)) {
            if (glfwGetTime() <= tiempos[tiempoIndex]) {
                // Animar. Usaremos puras transformaciones.                
                switch (tiempoIndex) {
                case 0:
                    // ### Inicio
                    // Tiempo: 2 segundos

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // El tri�ngulo protagonista va a estar fuera de la escena
                    transform_protag = glm::translate(transform_protag, glm::vec3(1.0f, 0.0f, 0.0f));
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    break;
                case 1:
                    // ### Tri�ngulo entra en escena
                    // Tiempo: 1 segundos

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // Tri�ngulo protagonista                            
                    // El tri�ngulo protagonista va a empezar en 1.0 y queremos que llegue a 0
                    transform_protag = glm::translate(transform_protag, glm::vec3(1 - glfwGetTime(), 0.0f, 0.0f));
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    break;
                case 2:
                    // ### Tri�ngulo se para un momento
                    // Tiempo: 1.5 segundos

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    break;
                case 3:
                    // ### Tri�ngulo se mueve hacia la trselaci�n y choca.
                    // Tiempo: 1.5 segundos

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // Tri�ngulo protagonista                                        
                    // 0.25\sin\left(\pi\left(\frac{4}{3}x + 0.5\right)\right) - 0.25
                    aux = 0.25 * sin(pi * (((float)4 / (float)3) * glfwGetTime() + 0.5)) - 0.25;
                    transform_protag = glm::translate(transform_protag, glm::vec3(aux, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    break;
                case 4:
                    // ### Tri�ngulo se para un momento
                    // Tiempo: 0.5 segundos

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    break;
                case 5:
                    // ### Tri�ngulo choca varias veces m�s.
                    // Tiempo: 2 segundos


                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // Tri�ngulo protagonista                                        
                    // y\ =\ 0.25\sin\left(\pi\left(3x+\frac{1}{2}\right)\right)-0.25
                    aux = 0.25 * sin(pi * (3 * glfwGetTime() + 0.5)) - 0.25;
                    transform_protag = glm::translate(transform_protag, glm::vec3(aux, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                   
                    //Cambiamos los colores de los triangulos
                    if (glfwGetTime()<.67) {                       
                        color_cero_protag[0] = 1.0f; color_cero_protag[1] = 0.2f; color_cero_protag[2] = 0.2f;
                        color_uno_protag[0] = 1.0f; color_uno_protag[1] = 0.2f; color_uno_protag[2] = 0.2f;
                    }
                    else if (glfwGetTime() < 1.33) {
                        color_cero_protag[0] = 0.0f; color_cero_protag[1] = 0.5f; color_cero_protag[2] = 0.3f;
                        color_uno_protag[0] = 0.0f; color_uno_protag[1] = 0.5f; color_uno_protag[2] = 0.3f;
                    }
                    else {
                        color_cero_protag[0] = 0.2f; color_cero_protag[1] = 0.2f; color_cero_protag[2] = 1.0f;
                        color_uno_protag[0] = 0.2f; color_uno_protag[1] = 0.2f; color_uno_protag[2] = 1.0f;
                    }
                    break;
                case 6:
                    // ### Tri�ngulo rota hasta quedar paralelo al suelo
                    // Tiempo: 0.5 segundos
         
                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    rotAux = (float)1 / (float)5 * pi * (float)glfwGetTime();
                    transform_protag = glm::rotate(transform_protag, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    break;
                case 7:
                    // ### Tri�ngulo se cae
                    // Tiempo: 0.5 segundos

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    transform_protag = glm::translate(transform_protag, glm::vec3(0.0f, -2 * (float)glfwGetTime(), 0.0f));
                    rotAux = (float)1 / (float)10 * pi;
                    transform_protag = glm::rotate(transform_protag, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    break;
                case 8:
                    // Tri�ngulo ca�do se cambia de color
                    // Tiempo: 4 segundos

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    transform_protag = glm::translate(transform_protag, glm::vec3(0.0f, -1.0f, 0.0f));
                    rotAux = (float)1 / (float)10 * pi;
                    transform_protag = glm::rotate(transform_protag, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // Cambiamos los colores aleatoriamente
                    if (glfwGetTime() > .5 && glfwGetTime() < 3) {
                        color_cero_protag[0] = (float)rand() / (float)(RAND_MAX / 1); 
                        color_cero_protag[1] = (float)rand() / (float)(RAND_MAX / 1); 
                        color_cero_protag[2] = (float)rand() / (float)(RAND_MAX / 1);
                        color_uno_protag[0] = (float)rand() / (float)(RAND_MAX / 1); 
                        color_uno_protag[1] = (float)rand() / (float)(RAND_MAX / 1); 
                        color_uno_protag[2] = (float)rand() / (float)(RAND_MAX / 1);
                    }
                    //Hasta que llegamos al que encaja con la teselación
                    else {
                        color_cero_protag[0] = 0.043f; color_cero_protag[1] = 0.145f; color_cero_protag[2] = 0.271f;
                        color_uno_protag[0] = 0.698f; color_uno_protag[1] = 0.761f; color_uno_protag[2] = 0.929f;
                    }
                    
                    break;
                case 9:
                    // Nuevo tri�ngulo sube
                    // Tiempo: 2 segundos

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    transform_protag = glm::translate(transform_protag, glm::vec3(0.0f, 0.5 * (float)glfwGetTime() - 1, 0.0f));
                    rotAux = (float)1 / (float)10 * pi;
                    transform_protag = glm::rotate(transform_protag, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // Cambiamos los colores
                    color_cero_protag[0] = 0.043f; color_cero_protag[1] = 0.145f; color_cero_protag[2] = 0.271f;
                    color_uno_protag[0] = 0.698f; color_uno_protag[1] = 0.761f; color_uno_protag[2] = 0.929f;
                    break;
                case 10:
                    // ### Tri�ngulo rota hasta quedar como estaba antes
                    // Tiempo: 2 segundos
                    

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // y=-\frac{\pi}{20}x+\frac{\pi}{10}
                    rotAux = -0.05 * pi * (float)glfwGetTime() + pi / (float)10;
                    transform_protag = glm::rotate(transform_protag, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    // Cambiamos los colores
                    color_cero_protag[0] = 0.043f; color_cero_protag[1] = 0.145f; color_cero_protag[2] = 0.271f;
                    color_uno_protag[0] = 0.698f; color_uno_protag[1] = 0.761f; color_uno_protag[2] = 0.929f;
                    break;
                case 11:
                    // ### Tri�ngulo nuevo se para un momento
                    // Tiempo: 1 segundo

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    // Cambiamos los colores
                    color_cero_protag[0] = 0.043f; color_cero_protag[1] = 0.145f; color_cero_protag[2] = 0.271f;
                    color_uno_protag[0] = 0.698f; color_uno_protag[1] = 0.761f; color_uno_protag[2] = 0.929f;

                    break;
                case 12:
                    // ### Tri�ngulo nuevo se incorpora a la teselaci�n
                    // Tiempo: 2 segundos

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    transform_protag = glm::translate(transform_protag, glm::vec3(-0.25 * (float)glfwGetTime(), 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    // Cambiamos los colores
                    color_cero_protag[0] = 0.043f; color_cero_protag[1] = 0.145f; color_cero_protag[2] = 0.271f;
                    color_uno_protag[0] = 0.698f; color_uno_protag[1] = 0.761f; color_uno_protag[2] = 0.929f;

                    break;
                case 13:
                    // ### La teselaci�n completa se hace grande
                    // Tiempo: 3.5 segundos

                    // Teselaci�n principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    transform_protag = glm::translate(transform_protag, glm::vec3(-0.5f, 0.0f, 0.0f));

                    rotAux = 0.5 * (float)glfwGetTime();
                    transform_protag = glm::rotate(transform_protag, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));
                    transform = glm::rotate(transform, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));

                    scaleAmount = 0.5 * (float)glfwGetTime() + 0.5;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                   
                    // Cambiamos los colores
                    color_cero_protag[0] = 0.043f; color_cero_protag[1] = 0.145f; color_cero_protag[2] = 0.271f;
                    color_uno_protag[0] = 0.698f; color_uno_protag[1] = 0.761f; color_uno_protag[2] = 0.929f;
                    break;
                default:
                    std::cout << "�ndice de tiempo inv�lido" << std::endl;
                    glfwTerminate();
                    break;
                }
            }
            else {
                glfwSetTime(0.0f);
                tiempoIndex++;
            }
        }
        else {
            glfwTerminate();
            return 0;
        }


        // ##### RENDER

        // Color de fondo
        glClearColor(0.871f, 0.878f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Ahora s�, propiamente, dibujamos los tri�ngulos tipo cero de la teselaci�n principal
        ourShader.use();
        unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));       // Le pasamos al shader la transformaci�n que queremos.

        unsigned int color1Loc = glGetUniformLocation(ourShader.ID, "ourColor");
        glUniform3fv(color1Loc, 1, color1);
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, vert_ceros.size());

        // An�logamente, dibujamos los tri�ngulos tipo uno de la teselaci�n principal                
        unsigned int color2Loc = glGetUniformLocation(ourShader.ID, "ourColor");
        glUniform3fv(color2Loc, 1, color2);
        glBindVertexArray(VAOs[1]);
        glDrawArrays(GL_TRIANGLES, 0, vert_unos.size());

        // Cambiamos de transformaci�n
        unsigned int transf_protag_loc = glGetUniformLocation(ourShader.ID, "transform");
        glUniformMatrix4fv(transf_protag_loc, 1, GL_FALSE, glm::value_ptr(transform_protag));

        // Dibujamos los tri�ngulos tipo cero del tri�ngulo protagonista                
        unsigned int color_cero_protag_loc = glGetUniformLocation(ourShader.ID, "ourColor");
        glUniform3fv(color_cero_protag_loc, 1, color_cero_protag);
        glBindVertexArray(VAOs[2]);
        glDrawArrays(GL_TRIANGLES, 0, vert_ceros_protag.size());

        // Dibujamos los tri�ngulos tipo uno del tri�ngulo protagonista                
        unsigned int color_uno_protag_loc = glGetUniformLocation(ourShader.ID, "ourColor");
        glUniform3fv(color_uno_protag_loc, 1, color_uno_protag);
        glBindVertexArray(VAOs[3]);
        glDrawArrays(GL_TRIANGLES, 0, vert_unos_protag.size());

        // glfw: swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(4, VAOs);
    glDeleteBuffers(4, VBOs);

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