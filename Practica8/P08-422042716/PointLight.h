#pragma once
#include "Light.h"

class PointLight :
	public Light
{
public:
	PointLight();
	PointLight(GLfloat red, GLfloat green, GLfloat blue,
		GLfloat aIntensity, GLfloat dIntensity,
		GLfloat xPos, GLfloat yPos, GLfloat zPos,
		GLfloat con, GLfloat lin, GLfloat exp);

	void UseLight(GLuint ambientIntensityLocation, GLuint ambientcolorLocation,
		GLuint diffuseIntensityLocation, GLuint positionLocation,
		GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation);

	// Mueve la luz
	void SetPos(glm::vec3 pos);

	// Marca el estado; Shader filtra las luces apagadas al enviar uniformes.
	void SetEncendida(bool estado);
	bool EstaEncendida() { return encendida; }

	~PointLight();

protected:
	glm::vec3 position;

	GLfloat constant, linear, exponent;

	bool encendida;
};
