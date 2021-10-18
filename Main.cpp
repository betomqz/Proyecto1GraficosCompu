/*
* Código para generar una teselación de penrose.
* Autores: 
    - Fabio G. Calo Dizy, 191489    
    - José Alberto Márquez Luján, 187917
    - Pedro Olivares Sánchez, 190198
* Fecha de entrega: 17 de octubre de 2021
*
* Referencias: https://preshing.com/20110831/penrose-tiling-explained/
*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_s.h>

#include <iostream>
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

// #############################################################################
// Declaración de constantes a utilizar a lo largo de este programa
const int IMAGE_SIZE_X = SCR_WIDTH;
const int IMAGE_SIZE_Y = SCR_HEIGHT;
const int NUM_SUBDIVISONES = 7;
const double goldenRatio = (1 + sqrt(5)) / 2;
const double pi = 3.1415926535897932384626433832795028841971;

//Control del tiempo
/* 0: ### Inicio
*  1: ### Triángulo entra en escena
*  2: ### Triángulo se para un momento
*  3: ### Triángulo se mueve hacia la trselación y choca.
*  4: ### Triángulo se para un momento
*  5: ### Triángulo choca varias veces más.
*  6: ### Triángulo rota hasta quedar paralelo al suelo
*  7: ### Triángulo se cae
*  8: ### Triángulo caído se cambia de color
*  9: ### Nuevo triángulo sube
* 10: ### Triángulo rota hasta quedar como estaba antes
* 11: ### Triángulo nuevo se para un momento
* 12: ### Triángulo nuevo se incorpora a la teselación
* 13: ### La teselación completa se hace grande
*/
//                          0     1     2     3     4     5     6     7     8     9     10    11    12    13
const float tiempos[14] = { 2.0f, 1.0f, 1.5f, 1.5f, 0.5f, 2.0f, 0.5f, 0.5f, 4.0f, 2.0f, 2.0f, 1.0f, 2.0f, 3.5f };
int tiempoIndex = 0;

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
// Podría optimizarse liberando la memoria de los triángulos originales.
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

// --------------------------------- Creación de círculos
//Número de triángulos usados para aproximar un círculo
unsigned const int TRI_POR_CIRC = 10;

//Círculos
typedef struct Circ {
    int color; // 0 o 1 (blanco o negro)    
    float listaVert[TRI_POR_CIRC * 9];
} circ;

Circ crearCirc(int color, float x, float y, float radio) {
    Circ nuevoCirc;
    nuevoCirc.color = color;
    float theta = 2 * pi / TRI_POR_CIRC;

    for (int i = 0; i < 9 * TRI_POR_CIRC; i += 9) {
        //Primer vért.
        nuevoCirc.listaVert[i] = x;
        nuevoCirc.listaVert[i + 1] = y;
        nuevoCirc.listaVert[i + 2] = 0.0f;

        //Segundo vért.
        nuevoCirc.listaVert[i + 3] = x - radio * sin(-i * theta);
        nuevoCirc.listaVert[i + 4] = y - radio * cos(-i * theta);
        nuevoCirc.listaVert[i + 5] = 0.0f;

        //Tercer vért.
        nuevoCirc.listaVert[i + 6] = x - radio * sin(-(i + 1) * theta);
        nuevoCirc.listaVert[i + 7] = y - radio * cos(-(i + 1) * theta);
        nuevoCirc.listaVert[i + 8] = 0.0f;
    }
    return nuevoCirc;
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
    list<struct triangulo*> triangulos = {};    // Teselación principal. Estará incompleta
    list<struct triangulo*> triProtag = {};     // Triángulo protagonista.
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

        // Si es el triángulo protagonista, lo pondremos en el arreglo aparte.
        if (j == 0)
            triProtag.push_back(t);
        else
            triangulos.push_back(t);
    }    

    // Subdividimos los triángulos las veces que indice la constante NUM_SUBDIVISIONES.
    for (int j = 0; j < NUM_SUBDIVISONES; j++) {
        triangulos = subdividir(triangulos);
        triProtag = subdividir(triProtag);
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
    // Hacemos lo mismo con el triángulo protagonista
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

    // Ahora toca hacer los círculos.    
    list<Circ> listaCirc;
    vector<float> vertices2; //Lista de circ. blancos
    vector<float> vertices3; //Lista de circ. negros
    vector<int> indices2;
    vector<int> indices3;

    // Ojo izquierdo
    listaCirc.push_back(crearCirc(0, 0.3f, 0.1f, 0.05f));
    listaCirc.push_back(crearCirc(1, 0.288f, 0.1f, 0.03f));
    // Ojo derecho
    listaCirc.push_back(crearCirc(0, 0.38f, 0.1f, 0.05f));
    listaCirc.push_back(crearCirc(1, 0.368f, 0.1f, 0.03f));

    for (auto const& circActual : listaCirc) {
        if (circActual.color == 0) {
            for (int i = 0; i < TRI_POR_CIRC * 9; i++)
                vertices2.push_back(circActual.listaVert[i]);
        }
        else {
            for (int i = 0; i < TRI_POR_CIRC * 9; i++)
                vertices3.push_back(circActual.listaVert[i]);
        }
    }

    for (int i = 0; i < 3 * TRI_POR_CIRC * listaCirc.size() / 2; i++)
        indices2.push_back(i);

    for (int i = 0; i < 3 * TRI_POR_CIRC * listaCirc.size() / 2; i++)
        indices3.push_back(i);

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto 1", NULL, NULL);
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
    Shader ourShader("proyecto1.vs", "proyecto1.fs");

    // ------------------------------------------------------------------
    unsigned int VBOs[6], VAOs[6], EBOs[2];
    glGenVertexArrays(6, VAOs); // Generamos seis VAOs y seis Buffers
    glGenBuffers(6, VBOs);
    glGenBuffers(2, EBOs); // Solamente usamos EBO para los círculos (ojos)
    // Triángulos tipo cero de la teselación PRINCIPAL
    // --------------------
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, vert_ceros.size() * sizeof(float), &vert_ceros[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Triángulos tipo uno de la teselación PRINCIPAL
    // ---------------------
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, vert_unos.size() * sizeof(float), &vert_unos[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    // Triángulos tipo cero del triángulo PROTAGONISTA
    // --------------------
    glBindVertexArray(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, vert_ceros_protag.size() * sizeof(float), &vert_ceros_protag[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Triángulos tipo uno del triángulo PROTAGONISTA
    // ---------------------
    glBindVertexArray(VAOs[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
    glBufferData(GL_ARRAY_BUFFER, vert_unos_protag.size() * sizeof(float), &vert_unos_protag[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    // Círculos blancos (Ojos)
    // ---------------------
    glBindVertexArray(VAOs[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[4]);
    glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(float), &vertices2[0], GL_STATIC_DRAW);    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices2.size() * sizeof(int), &indices2[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //Círculos negros (Ojos)
    // ---------------------
    glBindVertexArray(VAOs[5]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[5]);
    glBufferData(GL_ARRAY_BUFFER, vertices3.size() * sizeof(float), &vertices3[0], GL_STATIC_DRAW);    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices3.size() * sizeof(int), &indices3[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Para dibujar únicamente los bordes
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    
    glfwSetTime(0.0f);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);        

        // Variables
        float scaleAmount = 1.0f;
        float aux;
        float rotAux = 0.0f;
        // Declaración de las matrices de transformación que vamos a estar usando. Son la identidad por default.
        glm::mat4 transform = glm::mat4(1.0f);          // Matriz para transformar la teselación principal
        glm::mat4 transform_protag = glm::mat4(1.0f);   // Matriz para transformar al triángulo protagonista        

        // Colores
        GLfloat color1[] = { 0.043f, 0.145f, 0.271f };                  // Color 1 de la teselación principal
        GLfloat color2[] = { 0.698f, 0.761f, 0.929f };                  // Color 2 de la teselación principal
        GLfloat color_cero_protag[] = { 0.8705f, 0.7686f, 0.2509f };    // Color cero del triángulo protagonista
        GLfloat color_uno_protag[] = { 0.8705f, 0.7686f, 0.2509f };     // Color uno del triángulo protagonista
        GLfloat color_ojos_blancos[] = { 1.0f, 1.0f, 1.0f };            // Color blanco de los ojos del protagonista
        GLfloat color_ojos_negros[] = { 0.0f, 0.0f, 0.0f };             // Color blanco de los ojos del protagonista

        // Control de tiempos
        if (tiempoIndex < sizeof(tiempos) / sizeof(float)) {
            if (glfwGetTime() <= tiempos[tiempoIndex]) {                
                // Animar. Usaremos puras transformaciones.                
                switch (tiempoIndex) {
                case 0:                    
                    // ### Inicio
                    // Tiempo: 2 segundos
                    
                    // Teselación principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));                    
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // El triángulo protagonista va a estar fuera de la escena
                    transform_protag = glm::translate(transform_protag, glm::vec3(1.0f, 0.0f, 0.0f));                    
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));                    
                    break;
                case 1:
                    // ### Triángulo entra en escena
                    // Tiempo: 1 segundos
                    
                    // Teselación principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // Triángulo protagonista                            
                    // El triángulo protagonista va a empezar en 1.0 y queremos que llegue a 0
                    transform_protag = glm::translate(transform_protag, glm::vec3(1 - glfwGetTime(), 0.0f, 0.0f));                    
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));                    
                    break;
                case 2:                   
                    // ### Triángulo se para un momento
                    // Tiempo: 1.5 segundos
                     
                    // Teselación principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    break;
                case 3:          
                    // ### Triángulo se mueve hacia la trselación y choca.
                    // Tiempo: 1.5 segundos
                    
                    // Teselación principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // Triángulo protagonista                                        
                    // 0.25\sin\left(\pi\left(\frac{4}{3}x + 0.5\right)\right) - 0.25
                    aux = 0.25 * sin(pi * (((float)4 / (float)3) * glfwGetTime() + 0.5)) - 0.25;
                    transform_protag = glm::translate(transform_protag, glm::vec3(aux, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));                    
                    break;
                case 4:
                    // ### Triángulo se para un momento
                    // Tiempo: 0.5 segundos
                    
                    // Teselación principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                                        
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    break;
                case 5: 
                    // ### Triángulo choca varias veces más.
                    // Tiempo: 2 segundos

                    // Teselación principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // Triángulo protagonista                                        
                    // y\ =\ 0.25\sin\left(\pi\left(3x+\frac{1}{2}\right)\right)-0.25
                    aux = 0.25 * sin(pi * (3 * glfwGetTime() + 0.5)) - 0.25;
                    transform_protag = glm::translate(transform_protag, glm::vec3(aux, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    //Cambiamos los colores de los triangulos
                    if (glfwGetTime() < .67) {
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
                    // ### Triángulo rota hasta quedar paralelo al suelo
                    // Tiempo: 0.5 segundos

                    // Teselación principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    rotAux = (float)1 / (float)5 * pi * (float)glfwGetTime();
                    transform_protag = glm::rotate(transform_protag, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    break;                    
                case 7:
                    // ### Triángulo se cae
                    // Tiempo: 0.5 segundos

                    // Teselación principal
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
                    // Triángulo caído se cambia de color
                    // Tiempo: 4 segundos
                    
                    // Teselación principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    
                    transform_protag = glm::translate(transform_protag, glm::vec3(0.0f, -1.0f, 0.0f));
                    rotAux = (float)1 / (float)10 * pi;
                    transform_protag = glm::rotate(transform_protag, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // Cambiamos los colores aleatoriamente
                    if (glfwGetTime() < 3) {
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
                    // Nuevo triángulo sube
                    // Tiempo: 2 segundos

                    // Teselación principal
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
                    // ### Triángulo rota hasta quedar como estaba antes
                    // Tiempo: 2 segundos

                    // Teselación principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // y=-\frac{\pi}{20}x+\frac{\pi}{10}
                    rotAux = - 0.05 * pi * (float)glfwGetTime() + pi / (float)10;
                    transform_protag = glm::rotate(transform_protag, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    // Cambiamos los colores
                    color_cero_protag[0] = 0.043f; color_cero_protag[1] = 0.145f; color_cero_protag[2] = 0.271f;
                    color_uno_protag[0] = 0.698f; color_uno_protag[1] = 0.761f; color_uno_protag[2] = 0.929f;
                    break;
                case 11:
                    // ### Triángulo nuevo se para un momento
                    // Tiempo: 1 segundo

                    // Teselación principal
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
                    // ### Triángulo nuevo se incorpora a la teselación
                    // Tiempo: 2 segundos

                    // Teselación principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    transform_protag = glm::translate(transform_protag, glm::vec3(-0.25*(float)glfwGetTime(), 0.0f, 0.0f));
                    scaleAmount = 0.5f;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));                    
                    // Cambiamos los colores
                    color_cero_protag[0] = 0.043f; color_cero_protag[1] = 0.145f; color_cero_protag[2] = 0.271f;
                    color_uno_protag[0] = 0.698f; color_uno_protag[1] = 0.761f; color_uno_protag[2] = 0.929f;

                    break;
                case 13:
                    // ### La teselación completa se hace grande
                    // Tiempo:  segundos

                    // Teselación principal
                    transform = glm::translate(transform, glm::vec3(-0.5f, 0.0f, 0.0f));                    
                    transform_protag = glm::translate(transform_protag, glm::vec3(-0.5f, 0.0f, 0.0f));

                    rotAux = 0.5 * (float)glfwGetTime();
                    transform_protag = glm::rotate(transform_protag, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));
                    transform = glm::rotate(transform, rotAux, glm::vec3(0.0f, 0.0f, 1.0f));

                    scaleAmount = 0.5*(float)glfwGetTime() + 0.5;
                    transform_protag = glm::scale(transform_protag, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
                    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));

                    // Cambiamos los colores
                    color_cero_protag[0] = 0.043f; color_cero_protag[1] = 0.145f; color_cero_protag[2] = 0.271f;
                    color_uno_protag[0] = 0.698f; color_uno_protag[1] = 0.761f; color_uno_protag[2] = 0.929f;
                    break;
                default:
                    std::cout << "Índice de tiempo inválido" << std::endl;
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

        // Ahora sí, propiamente, dibujamos los triángulos tipo cero de la teselación principal
        ourShader.use();
        unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));       // Le pasamos al shader la transformación que queremos.
        
        unsigned int color1Loc = glGetUniformLocation(ourShader.ID, "ourColor");
        glUniform3fv(color1Loc, 1, color1);        
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, vert_ceros.size());

        // Análogamente, dibujamos los triángulos tipo uno de la teselación principal                
        unsigned int color2Loc = glGetUniformLocation(ourShader.ID, "ourColor");
        glUniform3fv(color2Loc, 1, color2);     
        glBindVertexArray(VAOs[1]);
        glDrawArrays(GL_TRIANGLES, 0, vert_unos.size());

        // Cambiamos de transformación
        unsigned int transf_protag_loc = glGetUniformLocation(ourShader.ID, "transform");
        glUniformMatrix4fv(transf_protag_loc, 1, GL_FALSE, glm::value_ptr(transform_protag));

        // Dibujamos los triángulos tipo cero del triángulo protagonista                
        unsigned int color_cero_protag_loc = glGetUniformLocation(ourShader.ID, "ourColor");
        glUniform3fv(color_cero_protag_loc, 1, color_cero_protag);        
        glBindVertexArray(VAOs[2]);
        glDrawArrays(GL_TRIANGLES, 0, vert_ceros_protag.size());

        // Dibujamos los triángulos tipo uno del triángulo protagonista                
        unsigned int color_uno_protag_loc = glGetUniformLocation(ourShader.ID, "ourColor");
        glUniform3fv(color_uno_protag_loc, 1, color_uno_protag);
        glBindVertexArray(VAOs[3]);
        glDrawArrays(GL_TRIANGLES, 0, vert_unos_protag.size());

        if (tiempoIndex != 13 && tiempoIndex != 7 && tiempoIndex != 8) {
            // Dibujamos los círculos blancos                
            unsigned int color_blanco_loc = glGetUniformLocation(ourShader.ID, "ourColor");
            glUniform3fv(color_blanco_loc, 1, color_ojos_blancos);
            glBindVertexArray(VAOs[4]);
            glDrawElements(GL_TRIANGLES, 9 * TRI_POR_CIRC * listaCirc.size() / 2, GL_UNSIGNED_INT, 0);

            // Dibujamos los triángulos tipo uno del triángulo protagonista                
            unsigned int color_negro_loc = glGetUniformLocation(ourShader.ID, "ourColor");
            glUniform3fv(color_negro_loc, 1, color_ojos_negros);
            glBindVertexArray(VAOs[5]);
            glDrawElements(GL_TRIANGLES, 9 * TRI_POR_CIRC * listaCirc.size() / 2, GL_UNSIGNED_INT, 0);
        }        

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
}