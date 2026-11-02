#pragma once
/*
  Humo.h  -  Practica 9-2: sistema de particulas de humo

  Cada motor del avion "emite" particulas. Una particula:
    - nace en el motor (opacidad 0) y aparece en unos instantes,
    - se queda en el aire (el avion se aleja y deja la estela),
    - sube un poco, crece y se desvanece,
    - muere EXACTAMENTE a los 2 segundos (VIDA_HUMO) y su lugar se reutiliza.
*/
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Mesh_tn.h"

const float VIDA_HUMO = 2.0f;          // segundos que dura cada particula

struct Particula
{
	glm::vec3 pos, vel;
	float edad, vida;
	float tam0, tam1;                  // tamano al nacer y al morir
	float angulo, giro;                // rotacion del sprite (rad, rad/s)
	bool viva;
};

class SistemaHumo
{
public:
	// Valores que se pueden ajustar
	float tasa = 70.0f;                // particulas por segundo y por motor
	float opacidadMax = 0.45f;
	float aparicion = 0.15f;           // segundos que tarda en aparecer por completo

	SistemaHumo(int maximo = 1000) : particulas(maximo), gen(12345)
	{
		Reiniciar();
	}

	void Reiniciar()
	{
		for (Particula& p : particulas) p.viva = false;
		for (int i = 0; i < MAX_EMISORES; i++) { tieneUltima[i] = false; acum[i] = 0.0f; }
		siguiente = 0;
	}

	// Llamar cada cuadro y para cada motor.
	//  emisor: 0,1,... | posActual: posicion del motor | dirEscape: hacia donde sale el humo
	void Emitir(int emisor, const glm::vec3& posActual, const glm::vec3& dirEscape, float dt)
	{
		if (dt <= 0.0f) { ultima[emisor] = posActual; tieneUltima[emisor] = true; return; }
		if (!tieneUltima[emisor]) { ultima[emisor] = posActual; tieneUltima[emisor] = true; }

		// Cuantas particulas toca crear en este cuadro (el sobrante pasa al siguiente cuadro)
		float a0 = acum[emisor];
		float total = a0 + tasa * dt;
		int n = (int)total;
		for (int k = 1; k <= n; k++)
		{
			// Se reparten a lo largo del camino recorrido por el motor en este cuadro,
			// asi la estela sale continua aunque el avion vaya muy rapido.
			float f = glm::clamp(((float)k - a0) / (tasa * dt), 0.0f, 1.0f);
			glm::vec3 p = glm::mix(ultima[emisor], posActual, f);
			float edadInicial = (1.0f - f) * dt;     // las primeras ya "llevan" un poco de vida
			Crear(p, dirEscape, edadInicial);
		}
		acum[emisor] = total - n;
		ultima[emisor] = posActual;
	}

	void Actualizar(float dt)
	{
		for (Particula& p : particulas)
		{
			if (!p.viva) continue;
			p.edad += dt;
			if (p.edad >= p.vida) { p.viva = false; continue; }   // desaparece a los 2 s
			p.vel *= (1.0f - 0.9f * dt);                          // el aire la frena
			p.vel.y += 0.6f * dt;                                 // el humo caliente sube
			p.pos += p.vel * dt;
			p.angulo += p.giro * dt;
		}
	}

	int Vivas() const
	{
		int n = 0;
		for (const Particula& p : particulas) if (p.viva) n++;
		return n;
	}

	// ---- Funciones que describen la apariencia segun la edad ----
	float Opacidad(const Particula& p) const
	{
		float u = p.edad / p.vida;                                 // 0 -> 1
		float sube = glm::clamp(p.edad / aparicion, 0.0f, 1.0f);   // aparece
		sube = sube * sube * (3.0f - 2.0f * sube);
		float baja = std::pow(1.0f - u, 1.5f);                     // se desvanece (0 a los 2 s)
		return opacidadMax * sube * baja;
	}
	float Tamano(const Particula& p) const
	{
		float u = p.edad / p.vida;
		return p.tam0 + (p.tam1 - p.tam0) * std::sqrt(u);          // crece rapido y luego mas despacio
	}
	glm::vec3 Color(const Particula& p) const
	{
		float u = glm::clamp(p.edad / 0.5f, 0.0f, 1.0f);           // al salir tiene un tono azulado
		return glm::mix(glm::vec3(0.70f, 0.85f, 1.00f), glm::vec3(0.85f, 0.85f, 0.85f), u);
	}

	// Dibuja todas las particulas como sprites que miran a la camara.
	// Hay que tener activo el shader del humo, la textura y el blending.
	void Dibujar(MeshModel* cuadro, const glm::mat4& vista, const glm::vec3& posCamara,
		GLint uModel, GLint uColor, GLint uOpacidad)
	{
		// Se dibujan de la mas lejana a la mas cercana para que la transparencia mezcle bien
		orden.clear();
		for (int i = 0; i < (int)particulas.size(); i++)
			if (particulas[i].viva) orden.push_back(i);
		std::sort(orden.begin(), orden.end(), [&](int a, int b) {
			glm::vec3 da = particulas[a].pos - posCamara, db = particulas[b].pos - posCamara;
			return glm::dot(da, da) > glm::dot(db, db);
		});

		glm::mat4 haciaCamara = glm::mat4(glm::transpose(glm::mat3(vista)));   // inversa de la rotacion de la camara
		for (int i : orden)
		{
			const Particula& p = particulas[i];
			glm::mat4 m = glm::translate(glm::mat4(1.0f), p.pos) * haciaCamara;
			m = glm::rotate(m, p.angulo, glm::vec3(0.0f, 0.0f, 1.0f));
			m = glm::scale(m, glm::vec3(Tamano(p)));
			glm::vec3 c = Color(p);
			glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(m));
			glUniform3f(uColor, c.x, c.y, c.z);
			glUniform1f(uOpacidad, Opacidad(p));
			cuadro->RenderMeshModel();
		}
	}

	const std::vector<Particula>& Datos() const { return particulas; }

private:
	static const int MAX_EMISORES = 4;
	std::vector<Particula> particulas;
	std::vector<int> orden;
	std::mt19937 gen;
	size_t siguiente;
	glm::vec3 ultima[MAX_EMISORES];
	bool tieneUltima[MAX_EMISORES];
	float acum[MAX_EMISORES];

	float Azar(float a, float b) { return std::uniform_real_distribution<float>(a, b)(gen); }

	void Crear(const glm::vec3& pos, const glm::vec3& dirEscape, float edadInicial)
	{
		// Busca un lugar libre (si no hay, reutiliza el mas antiguo)
		size_t i = siguiente;
		for (size_t k = 0; k < particulas.size(); k++)
		{
			size_t j = (siguiente + k) % particulas.size();
			if (!particulas[j].viva) { i = j; break; }
		}
		siguiente = (i + 1) % particulas.size();

		Particula& p = particulas[i];
		p.vida = VIDA_HUMO;
		p.edad = edadInicial;
		p.vel = dirEscape * Azar(2.0f, 4.0f)                          // sale empujado hacia atras
			+ glm::vec3(Azar(-0.6f, 0.6f), Azar(0.0f, 0.5f), Azar(-0.6f, 0.6f));
		p.pos = pos + p.vel * edadInicial;
		p.tam0 = Azar(0.9f, 1.4f);
		p.tam1 = Azar(4.0f, 5.5f);
		p.angulo = Azar(0.0f, 6.2832f);
		p.giro = Azar(-0.8f, 0.8f);
		p.viva = true;
	}
};
