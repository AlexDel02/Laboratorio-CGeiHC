/*
 * PROYECTO FINAL:
 * Computacion Grafica e Interaccion Humano-Computadora
 * Version con Modelos Unificados
 */
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"
#include "AvatarJerarquico.h"
#include "CamarasProyecto.h"
#include "AmbienteEscenario.h"
#include "CoheteKeyframes.h"

 // Iluminacion
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;
const float PI = 3.14159265f;

Window mainWindow(1366, 768);
std::vector<MeshModel*> meshListModel;
std::vector<Shader> shaderList;
Camera camera;
Camera camaraRefinamiento;
Camera camaraPanoramica;
AvatarJerarquico avatar;
RecorridosProyecto recorridos;
AmbienteEscenario ambiente;
CoheteKeyframes cohete;
glm::mat4 vistaActiva(1);
glm::vec3 ojoActivo(0);
int planetaAvatar = 0;


// Skyboxes espaciales para cada planeta
Skybox skyboxPlanetaExtraccion;
Skybox skyboxPlanetaRefinamiento;

// Texturas
Texture texSueloExtraccion;
Texture texSueloRefinamiento;
Texture texEsferaPlaneta1;
Texture texEsferaPlaneta2;
Texture texPlana;

// Modelos 3D Unificados
Model Asteroide_M;
// El avatar ahora se carga por piezas en AvatarJerarquico.h.
Model Sigismund_M;         // Sigismund
Model Mob_M;               // Mob
Model Rocky_M;             // Rocky
Model Goku_M;              // Goku
// Fauna hostil del planeta de extraccion (convertida de GLB a OBJ)
Model Tillagemon_M;        // jefe de la fauna hostil
Model Alien_M;             // alien agachado
Model AlienMonstruo_M;     // alien de patas largas
Model AlienAlado_M;        // alien con alas, vuela sobre el planeta

// Materiales
Material Material_brillante;
Material Material_opaco;

// Tiempo
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
// deltaTime se mide en segundos; sin multiplicador dependiente del FPS.

// Iluminacion
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

// Coordenadas de los dos Planetas
const glm::vec3 centroPlaneta1(0.0f, 0.0f, 0.0f);       // Planeta 1: Extraccion
const glm::vec3 centroPlaneta2(2100.0f, 0.0f, -2100.0f);  // Planeta 2: Refinamiento | antes (1600, 0, -1600), se aleja para que no choquen los planetas mas grandes
const float radioTerrenoCaminable = 450.0f; // antes 225, planeta x2 para que la pista quepa en la mitad
const float radioEsferaPlaneta = 650.0f; // antes 325, planeta x2
// La esfera se corta exactamente donde su seccion tiene radio 45.
const float profundidadCentroPlaneta = std::sqrt(
    radioEsferaPlaneta * radioEsferaPlaneta - radioTerrenoCaminable * radioTerrenoCaminable);
const float alturaOjosCamara = 3.0f;

// Factores de escala por modelo
const float ESCALA_ASTEROIDE = 0.5f;
const float ESCALA_INSECTOIDE = 0.30f;
const float ESCALA_SIGISMUND = 15.0f;
const float ESCALA_MOB = 2050.0f;
const float ESCALA_ROCKY = 20.0f;
const float ESCALA_GOKU = 10.0f;
const float ESCALA_TILLAGEMON = 10.6f;      // ~70 de alto, el jefe
const float ESCALA_ALIEN = 0.40f;          // ~28 de alto
const float ESCALA_ALIEN_MONSTRUO = 3.7f;  // ~35 de alto
const float ESCALA_ALIEN_ALADO = 4.2f;     // ~20 de alto, ~49 de punta a punta de las alas

const float OFFSET_YAW_INSECTOIDE = 180.0f;

// Estructura para asteroides en orbita
struct DatosAsteroide {
    float radioOrbita;
    float velocidadOrbita;
    float anguloInicial;
    float alturaBase;
    float amplitudVertical;
    glm::vec3 escala;
    glm::vec3 ejeRotacion;
    float velocidadRotacionPropia;
};
std::vector<DatosAsteroide> cinturonAsteroides;

void InicializarAsteroides(int cantidad)
{
    for (int i = 0; i < cantidad; i++)
    {
        DatosAsteroide ast;
        ast.radioOrbita = 2300.0f + (i * 50.0f); // antes 1500 + i*40, el anillo queda por fuera de los planetas grandes
        ast.velocidadOrbita = 0.03f + (i % 5) * 0.01f;
        ast.anguloInicial = i * (360.0f / cantidad) * toRadians;
        ast.alturaBase = -15.0f + (i % 7) * 5.0f;
        ast.amplitudVertical = 4.0f + (i % 4) * 1.5f;
        float s = 0.5f + (i % 4) * 0.35f;
        ast.escala = glm::vec3(s, s, s);
        ast.ejeRotacion = glm::normalize(glm::vec3(1.0f, (i % 3) + 0.5f, (i % 2) + 0.2f));
        ast.velocidadRotacionPropia = 20.0f + (i * 8.0f);
        cinturonAsteroides.push_back(ast);
    }
}

// Suelo plano -> hombro redondeado Hermite -> cuerpo esferico.
void CrearEsferaTexturizada(float radio, int meridianos, int paralelos)
{
    std::vector<GLfloat> vertices;
    std::vector<unsigned int> indices;
    const int filasTransicion = 16;
    const float radioUnion = 580.0f; // antes 290, planeta x2
    const float ancho = radioUnion - radioTerrenoCaminable;
    const float alturaEsfera = std::sqrt(radio * radio - radioUnion * radioUnion);
    const float yFinal = alturaEsfera - profundidadCentroPlaneta;
    const float pendienteFinal = -radioUnion / alturaEsfera;
    const float phiUnion = std::asin(radioUnion / radio);
    for (int i = 0; i <= paralelos; ++i)
    {
        float r, y;
        glm::vec2 normalPerfil;
        if (i <= filasTransicion)
        {
            float t = static_cast<float>(i) / filasTransicion;
            float t2 = t * t, t3 = t2 * t;
            r = radioTerrenoCaminable + ancho * t;
            // Ambos extremos coinciden en altura Y y pendiente con sus vecinos.
            y = (-2 * t3 + 3 * t2) * yFinal + (t3 - t2) * ancho * pendienteFinal;
            float pendiente = ((-6 * t2 + 6 * t) * yFinal +
                (3 * t2 - 2 * t) * ancho * pendienteFinal) / ancho;
            normalPerfil = glm::normalize(glm::vec2(-pendiente, 1));
            y += profundidadCentroPlaneta;
        }
        else
        {
            float t = static_cast<float>(i - filasTransicion) / (paralelos - filasTransicion);
            float phi = phiUnion + t * (PI - phiUnion);
            r = radio * std::sin(phi);
            y = radio * std::cos(phi);
            normalPerfil = glm::vec2(std::sin(phi), std::cos(phi));
        }
        for (int j = 0; j <= meridianos; ++j)
        {
            float u = static_cast<float>(j) / meridianos;
            float a = 2 * PI * j / meridianos, c = std::cos(a), z = std::sin(a);
            // Se conserva la convencion de normales invertidas del shader base.
            vertices.insert(vertices.end(), { r * c, y, r * z, u,
                static_cast<float>(i) / paralelos,
                -normalPerfil.x * c, -normalPerfil.y, -normalPerfil.x * z });
        }
    }
    for (int i = 0; i < paralelos; ++i)
        for (int j = 0; j < meridianos; ++j)
        {
            unsigned int p1 = i * (meridianos + 1) + j, p2 = p1 + meridianos + 1;
            indices.insert(indices.end(), { p1, p2, p1 + 1, p1 + 1, p2, p2 + 1 });
        }
    MeshModel* esfera = new MeshModel();
    esfera->CreateMeshModel(vertices.data(), indices.data(),
        static_cast<unsigned int>(vertices.size()), static_cast<unsigned int>(indices.size()));
    meshListModel.push_back(esfera);
}

void CreateObjects()
{
    // Disco unitario XZ: el render lo escala al radio caminable.
    const int segmentos = 256; // antes 192, mas segmentos para que el borde del planeta grande se vea redondo
    std::vector<GLfloat> vertices = { 0, 0, 0, 5, 5, 0, -1, 0 };
    std::vector<unsigned int> indices;
    for (int i = 0; i <= segmentos; ++i)
    {
        float angulo = 2 * PI * i / segmentos;
        float x = std::cos(angulo), z = std::sin(angulo);
        vertices.insert(vertices.end(), { x, 0, z, (x + 1) * 5, (z + 1) * 5, 0, -1, 0 });
        if (i < segmentos)
            indices.insert(indices.end(), { 0u, static_cast<unsigned int>(i + 2), static_cast<unsigned int>(i + 1) });
    }
    MeshModel* pisoPlaneta = new MeshModel();
    pisoPlaneta->CreateMeshModel(vertices.data(), indices.data(),
        static_cast<unsigned int>(vertices.size()), static_cast<unsigned int>(indices.size()));
    meshListModel.push_back(pisoPlaneta);
    // Ambos bordes usan los mismos 96 segmentos para evitar grietas.
    CrearEsferaTexturizada(radioEsferaPlaneta, segmentos, 80);
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

int main()
{
    mainWindow = Window(1366, 768);
    if (mainWindow.Initialise() != 0) return 1;

    CreateObjects();
    CreateShaders();
    if (!ambiente.Cargar(radioTerrenoCaminable)) { std::puts("Fallo al cargar ambiente: revisar Models/Ambiente y Textures."); return 1; } // ahora recibe el radio para acomodar la pista
    if (!cohete.Cargar()) { std::puts("No se pudo cargar el cohete o sus keyframes."); return 1; }
    InicializarAsteroides(10);

    camera = Camera(glm::vec3(-230.0f, alturaOjosCamara, 160.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 64.0f, 0.5f); // antes (-115, 3, 80), x2 por el planeta mas grande; velocidad antes 21.35, el insecto es 6 veces mas grande
    camaraRefinamiento = Camera(centroPlaneta2 + glm::vec3(0, alturaOjosCamara, 200), glm::vec3(0, 1, 0), -90, 0, 64.0f, 0.5f); // antes z = 100, x2; velocidad antes 21.35
    camaraPanoramica = Camera(glm::vec3(1050, 900, 900), glm::vec3(0, 1, 0), -90, -25, 50, 0.5f); // antes (800, 600, 600) con pitch -23, mira al punto medio entre planetas

    // Texturas
    texSueloExtraccion = Texture("Textures/sueloextraccion.png");
    texSueloExtraccion.LoadTextureA();

    texSueloRefinamiento = Texture("Textures/suelorefinamiento.png");
    texSueloRefinamiento.LoadTextureA();

    texEsferaPlaneta1 = Texture("Textures/sueloextraccion.png");
    texEsferaPlaneta1.LoadTextureA();

    texEsferaPlaneta2 = Texture("Textures/suelorefinamiento.png");
    texEsferaPlaneta2.LoadTextureA();

    texPlana = Texture("Textures/plain.png");
    texPlana.LoadTextureA();

    // Skyboxes
    std::vector<std::string> skyboxFacesP1 = {
        "Textures/Skybox/right.png", "Textures/Skybox/left.png",
        "Textures/Skybox/bottom.png", "Textures/Skybox/top.png",
        "Textures/Skybox/front.png", "Textures/Skybox/back.png"
    };
    skyboxPlanetaExtraccion = Skybox(skyboxFacesP1);
    skyboxPlanetaRefinamiento = Skybox(skyboxFacesP1);

    // Carga de modelos y muestra de tiempo de carga en terminal para debuggear
    double t0 = glfwGetTime();
    Asteroide_M.LoadModel("Models/asteroide.obj");
    printf("asteroide: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
    if (!avatar.Cargar())
    {
        std::puts("Faltan piezas del avatar. Conserva la carpeta Models/Avatar y sus materiales.");
        return 1;
    }
    printf("insectoide: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
    Sigismund_M.LoadModel("Models/sigismund.obj");
    printf("sigismund: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
    Mob_M.LoadModel("Models/mob.obj");
    printf("mob: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
    Rocky_M.LoadModel("Models/rocky.obj");
    printf("rocky: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
    Goku_M.LoadModel("Models/goku.obj");
    printf("goku: %.2f s\n", glfwGetTime() - t0);
    t0 = glfwGetTime();
    Tillagemon_M.LoadModel("Models/tillagemon.obj");
    Alien_M.LoadModel("Models/alien.obj");
    AlienMonstruo_M.LoadModel("Models/alien_monstruo.obj");
    AlienAlado_M.LoadModel("Models/alien_alado.obj");
    printf("fauna hostil: %.2f s\n", glfwGetTime() - t0);

    Material_brillante = Material(1.0f, 32);
    Material_opaco = Material(0.3f, 4);

    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 0.3f, 0.8f, 0.0f, -1.0f, -0.5f);

    spotLights[0] = SpotLight(0.8f, 1.0f, 0.9f, 0.0f, 2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.2f, -1.0f, 1.0f, 0.02f, 0.001f, 22.0f);

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
        uniformSpecularIntensity = 0, uniformShininess = 0, uniformColor = 0;

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.5f, 8000.0f); // antes near 0.1 y far 2000 (luego 6000), para ver el otro planeta sin z-fighting

    GLfloat now = 0.0f;
    glm::mat4 model(1.0f);
    glm::vec3 color(1.0f, 1.0f, 1.0f);

    shaderList[0].UseShader();
    GLint programaTerreno = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programaTerreno);
    const GLint uniformCorteAlfa = glGetUniformLocation(programaTerreno, "corteAlfa");
    const GLint uniformTerreno = glGetUniformLocation(programaTerreno, "texturaTerreno");
    const GLint uniformOrigenTerreno = glGetUniformLocation(programaTerreno, "origenTerreno");
    // Usar los mipmaps existentes para reducir el parpadeo de las texturas a distancia.
    Texture* texturasPlanetas[] = { &texSueloExtraccion, &texSueloRefinamiento,
        &texEsferaPlaneta1, &texEsferaPlaneta2 };
    for (Texture* textura : texturasPlanetas)
    {
        textura->UseTexture();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    lastTime = static_cast<GLfloat>(glfwGetTime());
    std::puts("1 P1 | 2 P2 | 3 transporte | 4 panorama libre | 5 carrera | Shift correr"); // se agrega la ayuda de Shift
    std::puts("6 Vista cohete | 7 Vista orbita | F6 reproducir/pausa | F7 reiniciar | F5 guardar | F9 leer | F8 siguiente cuadro");
    while (!mainWindow.getShouldClose())
    {
        now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;
        glfwPollEvents();
        mainWindow.ActualizarAnimacionTeclado(deltaTime);
        int accionCohete;
        while ((accionCohete = mainWindow.ConsumirAccionCohete()) != 0) {
            if (accionCohete == 1) cohete.Alternar();
            if (accionCohete == 2) cohete.Reiniciar();
            if (accionCohete == 3) cohete.Escribir();
            if (accionCohete == 4) cohete.Leer();
            if (accionCohete == 5) cohete.Paso();
        }
        cohete.Actualizar(deltaTime);
        bool* keys = mainWindow.getsKeys();
        GLint camActiva = mainWindow.getCamaraActiva();
        float distanciaAvatar = 0;
        if (camActiva == 1 || camActiva == 2)
        {
            planetaAvatar = camActiva - 1;
            Camera& control = planetaAvatar == 0 ? camera : camaraRefinamiento;
            const glm::vec3& centro = planetaAvatar == 0 ? centroPlaneta1 : centroPlaneta2;
            glm::vec3 antes = control.getCameraPosition();
            // Se conserva Camera::keyControlPlaneta, sus ejes XZ y sus limites.
            control.keyControlPlaneta(keys, deltaTime, centro, radioTerrenoCaminable, alturaOjosCamara);
            control.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
            // El terreno ya no tiene esquinas: mantener el avatar dentro del disco.
            glm::vec3 posicion = control.getCameraPosition();
            glm::vec3 offset(posicion.x - centro.x, 0, posicion.z - centro.z);
            float distanciaCentro = glm::length(offset);
            const float margenAvatar = 50.0f * ESCALA_INSECTOIDE; // antes 2.5, ahora crece con el insecto
            float limite = radioTerrenoCaminable - margenAvatar;
            if (distanciaCentro > limite)
            {
                offset *= limite / distanciaCentro;
                posicion.x = centro.x + offset.x;
                posicion.z = centro.z + offset.z;
                glm::vec3 frente = control.getCameraDirection();
                float yaw = glm::degrees(std::atan2(frente.z, frente.x));
                float pitch = glm::degrees(std::asin(glm::clamp(frente.y, -1.0f, 1.0f)));
                control.setCameraPosition(posicion, yaw, pitch);
            }
            distanciaAvatar = glm::length(control.getCameraPosition() - antes);
        }
        else if (camActiva == 4)
        {
            camaraPanoramica.keyControl(keys, deltaTime);
            camaraPanoramica.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
        }
        else { mainWindow.getXChange(); mainWindow.getYChange(); }
        if (mainWindow.getCambioCamaraPendiente()) mainWindow.confirmarCambioCamara();
        Camera& controlAvatar = planetaAvatar == 0 ? camera : camaraRefinamiento;
        const glm::vec3& centroActual = planetaAvatar == 0 ? centroPlaneta1 : centroPlaneta2;
        glm::vec3 posAvatar = controlAvatar.getCameraPosition();
        posAvatar.y = centroActual.y;
        glm::vec3 dirAvatar = controlAvatar.getHorizontalDirection();
        float yawAvatar = std::atan2(dirAvatar.x, dirAvatar.z) + OFFSET_YAW_INSECTOIDE * toRadians;
        float pitchAvatar = std::asin(glm::clamp(controlAvatar.getCameraDirection().y, -1.0f, 1.0f));
        avatar.Actualizar(deltaTime, distanciaAvatar * 0.05f / ESCALA_INSECTOIDE, pitchAvatar); // la zancada se ajusta al tamaño del insecto
        avatar.Preparar(posAvatar, yawAvatar, ESCALA_INSECTOIDE);
        recorridos.Actualizar(deltaTime, centroPlaneta1, centroPlaneta2);
        if (camActiva == 1 || camActiva == 2)
        {
            glm::vec3 objetivo = posAvatar + glm::vec3(0, 56.0f * ESCALA_INSECTOIDE, 0); // antes 2.8 fijo, ahora a la altura de la cabeza del insecto
            ojoActivo = objetivo - controlAvatar.getCameraDirection() * (240.0f * ESCALA_INSECTOIDE); // antes 9 fijo, ahora se aleja segun el tamaño del insecto
            ojoActivo.y = glm::max(ojoActivo.y, centroActual.y + 16.0f * ESCALA_INSECTOIDE); // antes 0.8
            vistaActiva = glm::lookAt(ojoActivo, objetivo, glm::vec3(0, 1, 0));
        }
        else if (camActiva == 3)
        {
            // La camara sigue al transporte tal como se dibuja (levantado sobre el piso) y se aleja segun su tamaño real
            ObjetivoSeguimiento transporteVisto = ambiente.TransporteDibujado(recorridos);
            vistaActiva = VistaSeguimiento(transporteVisto, 1.6f * ambiente.LargoTransporte(), 0.45f * ambiente.LargoTransporte(), ojoActivo); // antes 18 y 7 fijos
        }
        else if (camActiva == 6)
        {
            // El encuadre se abre con la altura para incluir las etapas en el suelo.
            float altura = glm::max(48.0f, cohete.AlturaCapsula());
            glm::vec3 objetivo = cohete.origen + glm::vec3(0, altura * 0.52f, 0);
            ojoActivo = objetivo + glm::vec3(36 + altura * 0.16f, 12 + altura * 0.2f, 72 + altura * 1.32f);
            vistaActiva = glm::lookAt(ojoActivo, objetivo, glm::vec3(0,1,0));
        }
        else if (camActiva == 7)
        {
            glm::vec3 objetivo = cohete.CentroOrbita();
            ojoActivo = objetivo + glm::vec3(0, 135, 170);
            vistaActiva = glm::lookAt(ojoActivo, objetivo, glm::vec3(0, 1, 0));
        }
        else if (camActiva == 5)
            vistaActiva = VistaSeguimiento(recorridos.carrera, 2.5f * ambiente.LargoNave(), 0.8f * ambiente.LargoNave(), ojoActivo); // antes 30 y 10, ahora segun el largo de la nave
        else
        {
            vistaActiva = camaraPanoramica.calculateViewMatrix();
            ojoActivo = camaraPanoramica.getCameraPosition();
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (camActiva == 2) skyboxPlanetaRefinamiento.DrawSkybox(vistaActiva, projection);
        else skyboxPlanetaExtraccion.DrawSkybox(vistaActiva, projection);

        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformEyePosition = shaderList[0].GetEyePositionLocation();
        uniformColor = shaderList[0].getColorLocation();
        uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        uniformShininess = shaderList[0].GetShininessLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(vistaActiva));
        glm::vec3 posCam = ojoActivo;
        glUniform3f(uniformEyePosition, posCam.x, posCam.y, posCam.z);

        // Iluminacion solar
        float anguloSol = (now / 120.0f) * 2.0f * PI;
        float intensidadSol = glm::clamp((sin(anguloSol) + 1.0f) * 0.5f, 0.15f, 0.9f);
        mainLight = DirectionalLight(1.0f, 0.98f, 0.9f, intensidadSol * 0.4f, intensidadSol, cos(anguloSol), -abs(sin(anguloSol)) - 0.2f, -0.3f);
        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, 0);

        // La fuente existente se vincula al nodo cabeza, incluso en vistas espaciales.
        spotLights[0].SetFlash(avatar.PosicionLinterna(), avatar.DireccionLinterna());
        unsigned int spotActivas = mainWindow.getLinternaAvatar() ? 1 : 0;
        shaderList[0].SetSpotLights(spotLights, spotActivas);

        // 1. Misma escala de textura en suelo, hombro y cuerpo.
        glUniform1i(uniformTerreno, 1);
        glUniform3fv(uniformOrigenTerreno, 1, glm::value_ptr(centroPlaneta1));
        // Renderizar Planeta 1 y Planeta 2
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta1);
        model = glm::scale(model, glm::vec3(radioTerrenoCaminable, 1.0f, radioTerrenoCaminable));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        texSueloExtraccion.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshListModel[0]->RenderMeshModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta1 + glm::vec3(0.0f, -profundidadCentroPlaneta, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        texEsferaPlaneta1.UseTexture();
        meshListModel[1]->RenderMeshModel();

        model = glm::mat4(1.0f);
        glUniform3fv(uniformOrigenTerreno, 1, glm::value_ptr(centroPlaneta2));
        model = glm::translate(model, centroPlaneta2);
        model = glm::scale(model, glm::vec3(radioTerrenoCaminable, 1.0f, radioTerrenoCaminable));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        texSueloRefinamiento.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshListModel[0]->RenderMeshModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta2 + glm::vec3(0.0f, -profundidadCentroPlaneta, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        texEsferaPlaneta2.UseTexture();
        meshListModel[1]->RenderMeshModel();

        // Los demas modelos conservan sus UV y materiales originales.
        glUniform1i(uniformTerreno, 0);
        // 2. Cinturon de Asteroides
        glm::vec3 centroSistema = (centroPlaneta1 + centroPlaneta2) * 0.5f;
        texPlana.UseTexture();
        for (size_t i = 0; i < cinturonAsteroides.size(); i++)
        {
            const DatosAsteroide& ast = cinturonAsteroides[i];
            float anguloOrbita = ast.anguloInicial + (now * ast.velocidadOrbita);
            float x = centroSistema.x + ast.radioOrbita * cos(anguloOrbita);
            float z = centroSistema.z + ast.radioOrbita * sin(anguloOrbita);
            float y = ast.alturaBase + ast.amplitudVertical * sin(now * 0.6f + ast.anguloInicial);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, y, z));
            model = glm::rotate(model, glm::radians(now * ast.velocidadRotacionPropia), ast.ejeRotacion);
            model = glm::scale(model, ast.escala * ESCALA_ASTEROIDE);
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
            Asteroide_M.RenderModel();
        }

        // 3. Personajes Planeta 1 (Sigismund y Mob)
        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta1 + glm::vec3(-260.0f, AmbienteEscenario::ApoyoEnPiso(Sigismund_M, ESCALA_SIGISMUND), -170.0f)); // antes (-130, 0, -85), x2 en XZ; y para que pise el suelo
        model = glm::scale(model, glm::vec3(ESCALA_SIGISMUND));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Sigismund_M.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta1 + glm::vec3(-290.0f, AmbienteEscenario::ApoyoEnPiso(Mob_M, ESCALA_MOB, true), 0.0f)); // antes (-145, 1, 0), x2 en XZ; y para que pise el suelo (esta volteado en X)
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(ESCALA_MOB));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Mob_M.RenderModel();

        // 4. Personajes Planeta 2 (Rocky y Goku sobre la Nube)
        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta2 + glm::vec3(-100.0f, AmbienteEscenario::ApoyoEnPiso(Rocky_M, ESCALA_ROCKY), -100.0f)); // antes (-50, 2, -50), x2 en XZ; y para que pise el suelo
        model = glm::scale(model, glm::vec3(ESCALA_ROCKY));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Rocky_M.RenderModel();

        // Goku (oscilacion senoidal vertical)
        float yNube = AmbienteEscenario::ApoyoEnPiso(Goku_M, ESCALA_GOKU) + 3.5f + 0.6f * sin(now * 2.5f); // se suma el apoyo para que la nube flote sobre el piso
        glm::vec3 posGokuNube = centroPlaneta2 + glm::vec3(-150.0f, yNube, -100.0f); // antes (-75, yNube, -50), x2 en XZ
        model = glm::mat4(1.0f);
        model = glm::translate(model, posGokuNube);
        model = glm::scale(model, glm::vec3(ESCALA_GOKU));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        // Acabado mate para el personaje y la nube, sin brillo plastico.
        glUniform1f(uniformSpecularIntensity, 0.0f);
        glUniform1f(uniformShininess, 1.0f);
        Goku_M.RenderModel();

        // 5. Avatar con cuerpo padre, cabeza y cuatro patas locales.
        avatar.Dibujar(uniformModel, uniformColor, uniformSpecularIntensity, uniformShininess);

        // 6. Fauna hostil del Planeta 1, en el lado -X junto a Sigismund y Mob (la pista ocupa el lado +X)
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta1 + glm::vec3(-50.0f, AmbienteEscenario::ApoyoEnPiso(Tillagemon_M, ESCALA_TILLAGEMON), -300.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(ESCALA_TILLAGEMON));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Tillagemon_M.RenderModel();

       model = glm::mat4(1.0f);
       model = glm::translate(model, centroPlaneta1 + glm::vec3(-80.0f, AmbienteEscenario::ApoyoEnPiso(Alien_M, ESCALA_ALIEN), -150.0f));
       model = glm::rotate(model, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
       model = glm::scale(model, glm::vec3(ESCALA_ALIEN));
       glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
       Alien_M.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta1 + glm::vec3(-200.0f, AmbienteEscenario::ApoyoEnPiso(AlienMonstruo_M, ESCALA_ALIEN_MONSTRUO), -250.0f));
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(ESCALA_ALIEN_MONSTRUO));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        AlienMonstruo_M.RenderModel();

        // El alado vuela en circulos sobre Sigismund y Mob, subiendo y bajando un poco
        float anguloVuelo = now * 0.25f;
        glm::vec3 posVuelo = centroPlaneta1 + glm::vec3(-220.0f + 140.0f * cos(anguloVuelo), 90.0f + 6.0f * sin(now * 1.5f), 20.0f + 140.0f * sin(anguloVuelo));
        model = glm::mat4(1.0f);
        model = glm::translate(model, posVuelo);
        model = glm::rotate(model, std::atan2(-sin(anguloVuelo), cos(anguloVuelo)), glm::vec3(0.0f, 1.0f, 0.0f)); // mira hacia donde avanza
        model = glm::rotate(model, glm::radians(-15.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // ladeado hacia el centro de la vuelta
        model = glm::scale(model, glm::vec3(ESCALA_ALIEN_ALADO));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        AlienAlado_M.RenderModel();

        // Modelos del escenario, pista real y ambas naves con sus texturas.
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        ambiente.Dibujar(centroPlaneta1, centroPlaneta2, recorridos,
            uniformModel, uniformColor, uniformCorteAlfa);

        // Cohete integrado al suelo del Planeta 1.
        glUniform1i(uniformTerreno, 0);
        glUniform1f(uniformCorteAlfa, 0);
        glUniform3f(uniformColor, 1, 1, 1);
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        cohete.Dibujar(uniformModel);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}