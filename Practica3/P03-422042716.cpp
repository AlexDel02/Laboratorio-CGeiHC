// Práctica 3: Modelado Geométrico y Cámara Sintética.
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>

// GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/random.hpp>

// Clases
#include "Mesh.h"
#include "Shader.h"
#include "Sphere.h"
#include "Window.h"
#include "Camera.h"

using std::vector;

const float toRadians = 3.14159265f / 180.0f;
const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader> shaderList;

static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
static const char* vShaderColor = "shaders/shadercolor.vert";
Sphere sp = Sphere(1.0, 20, 20);


// CREACIÓN DE PRIMITIVAS

void CrearCubo() {
    unsigned int cubo_indices[] = {
        0,1,2, 2,3,0, 1,5,6, 6,2,1, 7,6,5, 5,4,7,
        4,0,3, 3,7,4, 4,5,1, 1,0,4, 3,2,6, 6,7,3
    };
    GLfloat cubo_vertices[] = {
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f
    };
    Mesh* cubo = new Mesh();
    cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
    meshList.push_back(cubo); // Index 0
}

void CrearPiramideTriangular() {
    unsigned int indices_piramide_triangular[] = {
        0,1,2, 0,1,3, 0,2,3, 1,2,3
    };
    GLfloat vertices_piramide_triangular[] = {
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };
    Mesh* piramidet = new Mesh();
    piramidet->CreateMesh(vertices_piramide_triangular, indices_piramide_triangular, 12, 12);
    meshList.push_back(piramidet); // Index 1
}

// Geometría para primitivas curvas
void CrearCilindro(int res, float R) {
    int n, i; GLfloat dt = 2 * PI / res, x, z, y = -0.5f;
    vector<GLfloat> vertices; vector<unsigned int> indices;

    for (n = 0; n <= res; n++) {
        x = (n != res) ? R * cos(n * dt) : R * cos(0 * dt);
        z = (n != res) ? R * sin(n * dt) : R * sin(0 * dt);
        for (i = 0; i < 6; i++) {
            switch (i) {
            case 0: vertices.push_back(x); break;
            case 1: vertices.push_back(y); break;
            case 2: vertices.push_back(z); break;
            case 3: vertices.push_back(x); break;
            case 4: vertices.push_back(0.5f); break;
            case 5: vertices.push_back(z); break;
            }
        }
    }
    for (n = 0; n <= res; n++) { x = R * cos(n * dt); z = R * sin(n * dt); vertices.push_back(x); vertices.push_back(-0.5f); vertices.push_back(z); }
    for (n = 0; n <= res; n++) { x = R * cos(n * dt); z = R * sin(n * dt); vertices.push_back(x); vertices.push_back(0.5f); vertices.push_back(z); }
    for (i = 0; i < (int)vertices.size(); i++) indices.push_back(i);

    Mesh* cilindro = new Mesh();
    cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
    meshList.push_back(cilindro); // Index 2
}

void CrearCono(int res, float R) {
    int n, i; GLfloat dt = 2 * PI / res, x, z, y = -0.5f;
    vector<GLfloat> vertices; vector<unsigned int> indices;

    vertices.push_back(0.0f); vertices.push_back(0.5f); vertices.push_back(0.0f);
    for (n = 0; n <= res; n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
    }
    vertices.push_back(R * cos(0) * dt); vertices.push_back(-0.5f); vertices.push_back(R * sin(0) * dt);
    for (i = 0; i < res + 2; i++) indices.push_back(i);

    Mesh* cono = new Mesh();
    cono->CreateMeshGeometry(vertices, indices, vertices.size(), res + 2);
    meshList.push_back(cono); // Index 3
}

void CrearPiramideCuadrangular() {
    unsigned int piramidecuadrangular_indices[] = {
        0,3,4, 3,2,4, 2,1,4, 1,0,4, 0,1,2, 0,2,3
    };
    GLfloat piramidecuadrangular_vertices[] = {
         0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f,  0.0f, 0.5f, 0.0f,
    };
    Mesh* piramidec = new Mesh();
    piramidec->CreateMesh(piramidecuadrangular_vertices, piramidecuadrangular_indices, 15, 18);
    meshList.push_back(piramidec); // Index 4
}

// 5 mallas separadas para pintar las caras triangulares
void CrearPiramideMulticolor() {
    // Index 5: Base (Cuadrada - Azul)
    GLfloat base_v[] = { -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f,0.5f, -0.5f,-0.5f,0.5f };
    unsigned int base_i[] = { 0,1,2, 0,2,3 };
    Mesh* mBase = new Mesh(); mBase->CreateMesh(base_v, base_i, 12, 6); meshList.push_back(mBase);

    // Index 6: Frente (Triángulo - Roja)
    GLfloat f_v[] = { -0.5f,-0.5f,0.5f,  0.5f,-0.5f,0.5f,  0.0f,0.5f,0.0f };
    unsigned int f_i[] = { 0,1,2 };
    Mesh* mF = new Mesh(); mF->CreateMesh(f_v, f_i, 9, 3); meshList.push_back(mF);

    // Index 7: Trasera (Triángulo - Verde)
    GLfloat b_v[] = { -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.0f,0.5f,0.0f };
    unsigned int b_i[] = { 1,0,2 };
    Mesh* mB = new Mesh(); mB->CreateMesh(b_v, b_i, 9, 3); meshList.push_back(mB);

    // Index 8: Izquierda (Triángulo - Amarilla)
    GLfloat l_v[] = { -0.5f,-0.5f,-0.5f,  -0.5f,-0.5f,0.5f,  0.0f,0.5f,0.0f };
    unsigned int l_i[] = { 0,1,2 };
    Mesh* mL = new Mesh(); mL->CreateMesh(l_v, l_i, 9, 3); meshList.push_back(mL);

    // Index 9: Derecha (Triángulo - Magenta)
    GLfloat r_v[] = { 0.5f,-0.5f,-0.5f,  0.5f,-0.5f,0.5f,  0.0f,0.5f,0.0f };
    unsigned int r_i[] = { 1,0,2 };
    Mesh* mR = new Mesh(); mR->CreateMesh(r_v, r_i, 9, 3); meshList.push_back(mR);
}

void CreateShaders() {
    Shader* shader1 = new Shader(); shader1->CreateFromFiles(vShader, fShader); shaderList.push_back(*shader1);
    Shader* shader2 = new Shader(); shader2->CreateFromFiles(vShaderColor, fShader); shaderList.push_back(*shader2);
}


// FUNCIONES DE DIBUJO E INSTANCIACIÓN 


// Dibuja pirámide con caras de distintos colores
void DibujarPiramideColorida(glm::mat4 model, GLuint uniformModel, GLuint uniformColor) {
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 1.0f))); meshList[5]->RenderMesh();
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 0.0f))); meshList[6]->RenderMesh();
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f))); meshList[7]->RenderMesh();
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.0f))); meshList[8]->RenderMesh();
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 1.0f))); meshList[9]->RenderMesh();
}

void DibujarFigura4Rombos(glm::mat4 baseModel, GLuint uniformModel, GLuint uniformColor) {
    // Función auxiliar para dibujar 2 pirámides unidas por la base
    // El Rombo completo mide 2.0 de alto (de Y = -1 a Y = 1). 
    auto dibujarRombo3D = [&](glm::vec3 posCentro) {
        glm::mat4 modelRombo = glm::translate(baseModel, posCentro);

        // Pirámide superior (apunta hacia arriba)
        glm::mat4 pArriba = glm::translate(modelRombo, glm::vec3(0.0f, 0.5f, 0.0f));
        DibujarPiramideColorida(pArriba, uniformModel, uniformColor);

        // Pirámide inferior (apunta hacia abajo)
        glm::mat4 pAbajo = glm::translate(modelRombo, glm::vec3(0.0f, -0.5f, 0.0f));
        pAbajo = glm::rotate(pAbajo, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        DibujarPiramideColorida(pAbajo, uniformModel, uniformColor);
    };

    // Rombos que chocan sus puntas en el centro (0,0,0)
    dibujarRombo3D(glm::vec3(0.0f, 1.0f, 0.0f));  // Rombo Superior
    dibujarRombo3D(glm::vec3(0.0f, -1.0f, 0.0f)); // Rombo Inferior

    // Rombos que tocan las esquinas de los rombos centrales
    dibujarRombo3D(glm::vec3(-0.5f, 0.0f, 0.0f)); // Rombo Izquierdo
    dibujarRombo3D(glm::vec3(0.5f, 0.0f, 0.0f));  // Rombo Derecho
}

void DibujarCohete(glm::mat4 baseModel, GLuint uniformModel, GLuint uniformColor) {
    glm::mat4 model;

    // Cuerpo principal 
    model = baseModel;
    model = glm::scale(model, glm::vec3(1.0f, 3.0f, 1.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.9f, 0.9f, 0.9f)));
    meshList[2]->RenderMeshGeometry();

    // Punta
    model = baseModel;
    model = glm::translate(model, glm::vec3(0.0f, 2.25f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f, 1.5f, 0.5f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.2f, 0.6f, 1.0f)));
    meshList[3]->RenderMeshGeometry();

    // Motor
    model = baseModel;
    model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
    model = glm::scale(model, glm::vec3(0.6f, 0.5f, 0.6f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.3f, 0.3f, 0.3f)));
    meshList[0]->RenderMesh();

    // Aletas
    for (int i = 0; i < 4; i++) {
        model = baseModel;
        model = glm::rotate(model, glm::radians(i * 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::translate(model, glm::vec3(1.5f, -0.75f, 0.0f));

        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        model = glm::scale(model, glm::vec3(1.5f, 1.5f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 0.0f)));
        meshList[4]->RenderMesh(); // Usa la pirámide cuadrangular genérica para el rojo sólido
    }
}

// MAIN

int main() {
    mainWindow = Window(800, 600);
    mainWindow.Initialise();

    glEnable(GL_DEPTH_TEST);

    CrearCubo();                 // 0
    CrearPiramideTriangular();   // 1
    CrearCilindro(30, 1.0f);     // 2 
    CrearCono(30, 2.0f);         // 3 
    CrearPiramideCuadrangular(); // 4
    CrearPiramideMulticolor();   // 5 a 9 

    CreateShaders();

    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 0.3f, 0.3f);

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformColor = 0;
    glm::mat4 projection = glm::perspective(glm::radians(60.0f),
        mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f, 100.0f);

    sp.init();
    sp.load();

    while (!mainWindow.getShouldClose()) {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        glClearColor(0.05f, 0.05f, 0.1f, 1.0f); // Fondo oscuro
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0].useShader();
        uniformModel = shaderList[0].getModelLocation();
        uniformProjection = shaderList[0].getProjectLocation();
        uniformView = shaderList[0].getViewLocation();
        uniformColor = shaderList[0].getColorLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        // Sistema Central (Captura la rotación del teclado)
        glm::mat4 baseModel = glm::mat4(1.0f);
        baseModel = glm::translate(baseModel, glm::vec3(0.0f, 0.0f, -9.0f)); // Se alejó un poco a Z=-9
        baseModel = glm::rotate(baseModel, glm::radians(mainWindow.getrotax()), glm::vec3(1.0f, 0.0f, 0.0f));
        baseModel = glm::rotate(baseModel, glm::radians(mainWindow.getrotay()), glm::vec3(0.0f, 1.0f, 0.0f));
        baseModel = glm::rotate(baseModel, glm::radians(mainWindow.getrotaz()), glm::vec3(0.0f, 0.0f, 1.0f));

        // Dibujar el Cohete 
        glm::mat4 modeloCohete = glm::translate(baseModel, glm::vec3(-3.0f, 0.0f, 0.0f));
        DibujarCohete(modeloCohete, uniformModel, uniformColor);

        // Dibujar la Figura de 4 Rombos 
        glm::mat4 modeloEstrella = glm::translate(baseModel, glm::vec3(3.0f, 0.0f, 0.0f));
        DibujarFigura4Rombos(modeloEstrella, uniformModel, uniformColor);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }
    return 0;
}