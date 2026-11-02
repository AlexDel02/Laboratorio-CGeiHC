#pragma once
// Practica 9-2: funciones matematicas independientes del dibujo.
#include <cmath>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

namespace p92
{
const float PI = 3.14159265359f;
const double CICLO = 8.37;

inline float Suave(float u)
{
    u = glm::clamp(u, 0.0f, 1.0f);
    return u * u * (3.0f - 2.0f * u);
}

inline float Apertura(double tiempo)
{
    float t = static_cast<float>(std::fmod(tiempo, CICLO));
    if (t < 1.0f) return 0.0f;
    if (t < 3.0f) return Suave((t - 1.0f) / 2.0f);
    if (t < 5.3f) return 1.0f;
    if (t < 7.3f) return 1.0f - Suave((t - 5.3f) / 2.0f);
    return 0.0f;
}

inline glm::mat4 Holocron(double tiempo)
{
    float t = static_cast<float>(std::fmod(tiempo, CICLO));
    glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(-12, 9, 0));
    m = glm::rotate(m, 2.0f * PI * t / static_cast<float>(CICLO), glm::vec3(0, 1, 0));
    return glm::scale(m, glm::vec3(0.8f));
}

inline glm::mat4 Esquina(double tiempo, const glm::vec3& centro, const glm::vec3& signo)
{
    float t = static_cast<float>(std::fmod(tiempo, CICLO));
    float distancia = 4.0f * Apertura(tiempo);
    // Una vuelta local completa: al cerrar equivale otra vez a cero grados.
    float giro = 2.0f * PI * Suave((t - 1.0f) / 6.3f);
    glm::vec3 radial = glm::normalize(signo);
    glm::mat4 m = Holocron(tiempo);
    m = glm::translate(m, centro + distancia * radial);
    m = glm::rotate(m, giro, radial);
    return glm::translate(m, -centro);
}

inline glm::vec3 Bezier(float u, const glm::vec3& a, const glm::vec3& b,
                        const glm::vec3& c, const glm::vec3& d)
{
    float v = 1.0f - u;
    return v*v*v*a + 3.0f*v*v*u*b + 3.0f*v*u*u*c + u*u*u*d;
}

inline glm::vec3 Tangente(float u, const glm::vec3& a, const glm::vec3& b,
                          const glm::vec3& c, const glm::vec3& d)
{
    float v = 1.0f - u;
    return 3.0f*v*v*(b-a) + 6.0f*v*u*(c-b) + 3.0f*u*u*(d-c);
}

struct Vuelo
{
    glm::vec3 posicion;
    glm::vec3 velocidad;
};

inline Vuelo Cohete(double tiempo)
{
    const glm::vec3 a(0,16,0);
    const glm::vec3 b = a + glm::vec3(0,8,0) * (8.0f/3.0f);
    const glm::vec3 c(20,50,0), d(45,55,0);
    double t = tiempo > 2.0 ? tiempo - 2.0 : 0.0;
    Vuelo vuelo;
    if (t <= 4.0)
    {
        float s = static_cast<float>(t);
        vuelo.posicion = glm::vec3(0, s*s, 0); // y = (1/2)*2*t^2
        vuelo.velocidad = glm::vec3(0, 2.0f*s, 0);
    }
    else if (t <= 12.0)
    {
        float u = static_cast<float>((t-4.0)/8.0);
        vuelo.posicion = Bezier(u,a,b,c,d);
        vuelo.velocidad = Tangente(u,a,b,c,d)/8.0f;
    }
    else
    {
        vuelo.velocidad = Tangente(1,a,b,c,d)/8.0f;
        vuelo.posicion = d + vuelo.velocidad * static_cast<float>(t-12.0);
    }
    vuelo.posicion += glm::vec3(9,0,0);
    return vuelo;
}

inline glm::mat4 MatrizCohete(const Vuelo& vuelo)
{
    // La trayectoria esta en XY: basta una rotacion sobre Z.
    float angulo = 0.0f;
    if (glm::dot(vuelo.velocidad, vuelo.velocidad) > 0.000001f)
        angulo = -std::atan2(vuelo.velocidad.x, vuelo.velocidad.y);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), vuelo.posicion);
    return glm::rotate(m, angulo, glm::vec3(0,0,1));
}
} // namespace p92
