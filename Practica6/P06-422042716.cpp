/*
Práctica 6: Texturizado
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
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<MeshColor*> meshListColor;
std::vector<MeshModel*> meshListModel;

std::vector<Shader> shaderList;

Camera camera;

Texture plainTexture;
Texture pisoTexture;
Texture dadoTexture;
Texture holocronTexture;

Model ModeloKitt;
Model ModeloLlanta;
Model ModeloDado;
Model ModeloHolocron;

Skybox skybox;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// Variables de rotación refactorizadas
GLfloat rotObjX = 0.0f;
GLfloat rotObjY = 0.0f;
GLfloat rotObjZ = 0.0f;
static double limiteFrames = 1.0 / 60.0;

static const char* vShader = "shaders/shader_texture.vert";
static const char* fShader = "shaders/shader_texture.frag";

void ConstruirObjetos()
{
	unsigned int indicesBase[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat verticesBase[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int indicesSuelo[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat verticesSuelo[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	unsigned int indicesVegetacion[] = {
		0, 1, 2,
		0, 2, 3,
		4, 5, 6,
		4, 6, 7
	};

	GLfloat verticesVegetacion[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,
	};

	MeshModel* obj1 = new MeshModel();
	obj1->CreateMeshModel(verticesBase, indicesBase, 32, 12);
	meshListModel.push_back(obj1);

	MeshModel* obj2 = new MeshModel();
	obj2->CreateMeshModel(verticesBase, indicesBase, 32, 12);
	meshListModel.push_back(obj2);

	MeshModel* obj3 = new MeshModel();
	obj3->CreateMeshModel(verticesSuelo, indicesSuelo, 32, 6);
	meshListModel.push_back(obj3);

	MeshModel* obj4 = new MeshModel();
	obj4->CreateMeshModel(verticesVegetacion, indicesVegetacion, 64, 12);
	meshListModel.push_back(obj4);
}

void CrearShaders()
{
	Shader* shaderBase = new Shader();
	shaderBase->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shaderBase);
}

void ConstruirDado()
{
	unsigned int indicesCubo[] = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		8, 10, 9, 10, 8, 11,
		12, 13, 14, 14, 15, 12,
		16, 18, 17, 18, 16, 19,
		20, 21, 22, 22, 23, 20,
	};

	GLfloat verticesCubo[] = {
		// x, y, z, S, T, nx, ny, nz
		// FRONT 
		-0.5f, -0.5f,  0.5f,    0.0f,     0.0f,     0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f,  0.5f,    0.3333f,  0.0f,     0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f,  0.5f,    0.3333f,  0.5f,     0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f,  0.5f,    0.0f,     0.5f,     0.0f,  0.0f, -1.0f,

		// RIGHT 
		 0.5f, -0.5f,  0.5f,    0.3333f,  0.0f,    -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,    0.6666f,  0.0f,    -1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,    0.6666f,  0.5f,    -1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,    0.3333f,  0.5f,    -1.0f,  0.0f,  0.0f,

		 // BACK 
		 -0.5f, -0.5f, -0.5f,    1.0f,     0.0f,     0.0f,  0.0f,  1.0f,
		  0.5f, -0.5f, -0.5f,    0.6666f,  0.0f,     0.0f,  0.0f,  1.0f,
		  0.5f,  0.5f, -0.5f,    0.6666f,  0.5f,     0.0f,  0.0f,  1.0f,
		 -0.5f,  0.5f, -0.5f,    1.0f,     0.5f,     0.0f,  0.0f,  1.0f,

		 // LEFT 
		 -0.5f, -0.5f, -0.5f,    0.0f,     0.5f,     1.0f,  0.0f,  0.0f,
		 -0.5f, -0.5f,  0.5f,    0.3333f,  0.5f,     1.0f,  0.0f,  0.0f,
		 -0.5f,  0.5f,  0.5f,    0.3333f,  1.0f,     1.0f,  0.0f,  0.0f,
		 -0.5f,  0.5f, -0.5f,    0.0f,     1.0f,     1.0f,  0.0f,  0.0f,

		 // BOTTOM
		 -0.5f, -0.5f,  0.5f,    0.6666f,  1.0f,     0.0f,  1.0f,  0.0f,
		  0.5f, -0.5f,  0.5f,    0.3333f,  1.0f,     0.0f,  1.0f,  0.0f,
		  0.5f, -0.5f, -0.5f,    0.3333f,  0.5f,     0.0f,  1.0f,  0.0f,
		 -0.5f, -0.5f, -0.5f,    0.6666f,  0.5f,     0.0f,  1.0f,  0.0f,

		 // UP 
		 -0.5f,  0.5f,  0.5f,    0.6666f,  0.5f,     0.0f, -1.0f,  0.0f,
		  0.5f,  0.5f,  0.5f,    1.0f,     0.5f,     0.0f, -1.0f,  0.0f,
		  0.5f,  0.5f, -0.5f,    1.0f,     1.0f,     0.0f, -1.0f,  0.0f,
		 -0.5f,  0.5f, -0.5f,    0.6666f,  1.0f,     0.0f, -1.0f,  0.0f,
	};

	MeshModel* objDado = new MeshModel();
	objDado->CreateMeshModel(verticesCubo, indicesCubo, 192, 36);
	meshListModel.push_back(objDado);
}

void ConstruirHolocron()
{
	unsigned int indicesOctaedro[] = {
		0, 1, 2,  3, 4, 5,  6, 7, 8,  9, 10, 11,
		12, 13, 14,  15, 16, 17,  18, 19, 20,  21, 22, 23,
		24, 25, 26,  27, 28, 29,  30, 31, 32,  33, 34, 35,
		36, 37, 38,  39, 40, 41,  42, 43, 44,  45, 46, 47,
		48, 49, 50,  51, 52, 53,  54, 55, 56,  57, 58, 59,
		60, 61, 62,  63, 64, 65,  66, 67, 68,  69, 70, 71,
		72, 73, 74,  75, 76, 77,  78, 79, 80,  81, 82, 83,
	};

	GLfloat verticesOctaedro[] = {
		// x, y, z, u, v, nx, ny, nz
		-2.45398f, -0.06338f, -2.47745f, 0.24886f, 0.42288f, -0.52904f, -0.58890f, -0.61099f,
		-1.18505f, -2.53049f, -1.19826f, 0.24561f, 0.29506f, -0.52904f, -0.58890f, -0.61099f,
		-2.51727f, -2.57909f,  0.00211f, 0.32065f, 0.29363f, -0.52904f, -0.58890f, -0.61099f,
		-2.45398f, -0.06338f, -2.47745f, 0.16096f, 0.89546f, -0.99846f,  0.04023f, -0.03811f,
		-2.64678f, -0.13904f,  2.49375f, 0.34296f, 0.71201f, -0.99846f,  0.04023f, -0.03811f,
		-2.45137f,  2.42175f,  0.07753f, 0.34605f, 0.89509f, -0.99846f,  0.04023f, -0.03811f,
		 2.52120f,  2.44824f,  0.06338f, 0.38624f, 0.42395f,  0.56607f,  0.59935f, -0.56597f,
		 2.53494f, -0.06114f, -2.58028f, 0.46922f, 0.28409f,  0.56607f,  0.59935f, -0.56597f,
		 0.04964f,  2.44012f, -2.41721f, 0.54247f, 0.42350f,  0.56607f,  0.59935f, -0.56597f,
		 0.04964f,  2.44012f, -2.41721f, 0.39399f, 0.64443f, -0.57367f,  0.58743f, -0.57079f,
		-2.45398f, -0.06338f, -2.47745f, 0.46864f, 0.51042f, -0.57367f,  0.58743f, -0.57079f,
		-2.45137f,  2.42175f,  0.07753f, 0.54702f, 0.64345f, -0.57367f,  0.58743f, -0.57079f,
		-0.14069f, -2.60193f,  2.55856f, 0.78525f, 0.66268f, -0.02235f,  0.00355f,  0.99974f,
		 0.05115f,  2.39978f,  2.54504f, 0.58618f, 0.88141f, -0.02235f,  0.00355f,  0.99974f,
		-2.64678f, -0.13904f,  2.49375f, 0.57762f, 0.66268f, -0.02235f,  0.00355f,  0.99974f,
		-2.45137f,  2.42175f,  0.07753f, 0.60623f, 0.65388f, -0.56493f,  0.58866f,  0.57820f,
		-2.64678f, -0.13904f,  2.49375f, 0.66930f, 0.52487f, -0.56493f,  0.58866f,  0.57820f,
		 0.05115f,  2.39978f,  2.54504f, 0.74935f, 0.65277f, -0.56493f,  0.58866f,  0.57820f,
		 0.05115f,  2.39978f,  2.54504f, 0.77007f, 0.62502f,  0.57317f,  0.57708f,  0.58176f,
		 2.59300f, -0.08522f,  2.50575f, 0.86101f, 0.47449f,  0.57317f,  0.57708f,  0.58176f,
		 2.52120f,  2.44824f,  0.06338f, 0.94327f, 0.62795f,  0.57317f,  0.57708f,  0.58176f,
		-2.64678f, -0.13904f,  2.49375f, 0.30873f, 0.40394f, -0.59336f, -0.58934f,  0.54826f,
		-1.32899f, -2.57838f,  1.29783f, 0.30918f, 0.29824f, -0.59336f, -0.58934f,  0.54826f,
		-0.14069f, -2.60193f,  2.55856f, 0.36982f, 0.29716f, -0.59336f, -0.58934f,  0.54826f,
		 2.59300f, -0.08522f,  2.50575f, 0.67514f, 0.44420f,  0.59328f, -0.56795f,  0.57047f,
		 1.38502f, -2.54947f,  1.30867f, 0.67284f, 0.32890f,  0.59328f, -0.56795f,  0.57047f,
		 2.63220f, -2.55389f,  0.00722f, 0.74217f, 0.32672f,  0.59328f, -0.56795f,  0.57047f,
		 2.53494f, -0.06114f, -2.58028f, 0.87936f, 0.46200f,  0.53772f, -0.57673f, -0.61500f,
		 1.34418f, -2.54257f, -1.29438f, 0.87892f, 0.32094f,  0.53772f, -0.57673f, -0.61500f,
		 0.08143f, -2.51320f, -2.42600f, 0.95763f, 0.32107f,  0.53772f, -0.57673f, -0.61500f,
		 0.08143f, -2.51320f, -2.42600f, 0.36448f, 0.68322f, -0.06410f,  0.00135f, -0.99794f,
		 0.04964f,  2.44012f, -2.41721f, 0.57379f, 0.89695f, -0.06410f,  0.00135f, -0.99794f,
		 2.53494f, -0.06114f, -2.58028f, 0.36076f, 0.89278f, -0.06410f,  0.00135f, -0.99794f,
		 1.34418f, -2.54257f, -1.29438f, 0.24085f, 0.15866f,  0.01066f, -0.99993f, -0.00281f,
		 1.38502f, -2.54947f,  1.30867f, 0.23977f, 0.09015f,  0.01066f, -0.99993f, -0.00281f,
		-1.32899f, -2.57838f,  1.29783f, 0.31122f, 0.09043f,  0.01066f, -0.99993f, -0.00281f,
		 2.63220f, -2.55389f,  0.00722f, 0.98488f, 0.69418f,  0.99973f,  0.02225f, -0.00630f,
		 2.52120f,  2.44824f,  0.06338f, 0.77165f, 0.90518f,  0.99973f,  0.02225f, -0.00630f,
		 2.59300f, -0.08522f,  2.50575f, 0.77429f, 0.69418f,  0.99973f,  0.02225f, -0.00630f,
		 0.04964f,  2.44012f, -2.41721f, 0.15564f, 0.69749f, -0.01144f,  0.99990f,  0.00813f,
		 0.05115f,  2.39978f,  2.54504f, 0.38384f, 0.46621f, -0.01144f,  0.99990f,  0.00813f,
		 2.52120f,  2.44824f,  0.06338f, 0.38490f, 0.69547f, -0.01144f,  0.99990f,  0.00813f,
		 0.04964f,  2.44012f, -2.41721f, 0.15564f, 0.69749f,  0.00076f,  0.99996f,  0.00813f,
		-2.45137f,  2.42175f,  0.07753f, 0.15373f, 0.46621f,  0.00076f,  0.99996f,  0.00813f,
		 0.05115f,  2.39978f,  2.54504f, 0.38384f, 0.46621f,  0.00076f,  0.99996f,  0.00813f,
		 0.08143f, -2.51320f, -2.42600f, 0.36448f, 0.68322f,  0.02213f,  0.00191f, -0.99975f,
		-2.45398f, -0.06338f, -2.47745f, 0.57732f, 0.68322f,  0.02213f,  0.00191f, -0.99975f,
		 0.04964f,  2.44012f, -2.41721f, 0.57379f, 0.89695f,  0.02213f,  0.00191f, -0.99975f,
		 2.63220f, -2.55389f,  0.00722f, 0.98488f, 0.69418f,  0.99962f,  0.02236f, -0.01603f,
		 2.53494f, -0.06114f, -2.58028f, 0.99013f, 0.90960f,  0.99962f,  0.02236f, -0.01603f,
		 2.52120f,  2.44824f,  0.06338f, 0.77165f, 0.90518f,  0.99962f,  0.02236f, -0.01603f,
		-0.14069f, -2.60193f,  2.55856f, 0.78525f, 0.66268f,  0.01744f,  0.00203f,  0.99984f,
		 2.59300f, -0.08522f,  2.50575f, 0.79621f, 0.88196f,  0.01744f,  0.00203f,  0.99984f,
		 0.05115f,  2.39978f,  2.54504f, 0.58618f, 0.88141f,  0.01744f,  0.00203f,  0.99984f,
		-2.45398f, -0.06338f, -2.47745f, 0.16096f, 0.89546f, -0.99915f, -0.01325f, -0.03895f,
		-2.51727f, -2.57909f,  0.00211f, 0.16174f, 0.71201f, -0.99915f, -0.01325f, -0.03895f,
		-2.64678f, -0.13904f,  2.49375f, 0.34296f, 0.71201f, -0.99915f, -0.01325f, -0.03895f,
		-2.45398f, -0.06338f, -2.47745f, 0.24886f, 0.42288f, -0.55796f, -0.58971f, -0.58387f,
		 0.08143f, -2.51320f, -2.42600f, 0.17182f, 0.29703f, -0.55796f, -0.58971f, -0.58387f,
		-1.18505f, -2.53049f, -1.19826f, 0.24561f, 0.29506f, -0.55796f, -0.58971f, -0.58387f,
		-2.64678f, -0.13904f,  2.49375f, 0.30873f, 0.40394f, -0.59524f, -0.58935f,  0.54620f,
		-2.51727f, -2.57909f,  0.00211f, 0.24764f, 0.29815f, -0.59524f, -0.58935f,  0.54620f,
		-1.32899f, -2.57838f,  1.29783f, 0.30918f, 0.29824f, -0.59524f, -0.58935f,  0.54620f,
		 2.59300f, -0.08522f,  2.50575f, 0.67514f, 0.44420f,  0.53374f, -0.56659f,  0.62776f,
		-0.14069f, -2.60193f,  2.55856f, 0.59705f, 0.32429f,  0.53374f, -0.56659f,  0.62776f,
		 1.38502f, -2.54947f,  1.30867f, 0.67284f, 0.32890f,  0.53374f, -0.56659f,  0.62776f,
		 2.53494f, -0.06114f, -2.58028f, 0.87936f, 0.46200f,  0.57823f, -0.57659f, -0.57721f,
		 2.63220f, -2.55389f,  0.00722f, 0.79396f, 0.31864f,  0.57823f, -0.57659f, -0.57721f,
		 1.34418f, -2.54257f, -1.29438f, 0.87892f, 0.32094f,  0.57823f, -0.57659f, -0.57721f,
		-1.32899f, -2.57838f,  1.29783f, 0.31122f, 0.09043f,  0.02023f, -0.99963f, -0.01801f,
		-2.51727f, -2.57909f,  0.00211f, 0.34250f, 0.12453f,  0.02023f, -0.99963f, -0.01801f,
		-1.18505f, -2.53049f, -1.19826f, 0.30742f, 0.15614f,  0.02023f, -0.99963f, -0.01801f,
		-1.18505f, -2.53049f, -1.19826f, 0.30742f, 0.15614f, -0.00552f, -0.99978f, -0.01978f,
		 0.08143f, -2.51320f, -2.42600f, 0.27408f, 0.18846f, -0.00552f, -0.99978f, -0.01978f,
		 1.34418f, -2.54257f, -1.29438f, 0.24085f, 0.15866f, -0.00552f, -0.99978f, -0.01978f,
		 1.34418f, -2.54257f, -1.29438f, 0.24085f, 0.15866f, -0.00620f, -0.99997f, -0.00255f,
		 2.63220f, -2.55389f,  0.00722f, 0.20695f, 0.12440f, -0.00620f, -0.99997f, -0.00255f,
		 1.38502f, -2.54947f,  1.30867f, 0.23977f, 0.09015f, -0.00620f, -0.99997f, -0.00255f,
		 1.38502f, -2.54947f,  1.30867f, 0.23977f, 0.09015f,  0.01076f, -0.99952f, -0.02881f,
		-0.14069f, -2.60193f,  2.55856f, 0.27994f, 0.05723f,  0.01076f, -0.99952f, -0.02881f,
		-1.32899f, -2.57838f,  1.29783f, 0.31122f, 0.09043f,  0.01076f, -0.99952f, -0.02881f,
		-1.32899f, -2.57838f,  1.29783f, 0.31122f, 0.09043f, -0.00551f, -0.99979f, -0.01949f,
		-1.18505f, -2.53049f, -1.19826f, 0.30742f, 0.15614f, -0.00551f, -0.99979f, -0.01949f,
		 1.34418f, -2.54257f, -1.29438f, 0.24085f, 0.15866f, -0.00551f, -0.99979f, -0.01949f,
	};

	MeshModel* objHolocron = new MeshModel();
	objHolocron->CreateMeshModel(verticesOctaedro, indicesOctaedro,
		sizeof(verticesOctaedro) / sizeof(verticesOctaedro[0]),
		sizeof(indicesOctaedro) / sizeof(indicesOctaedro[0]));
	meshListModel.push_back(objHolocron);
}

int main()
{
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();

	ConstruirObjetos();
	ConstruirDado();
	ConstruirHolocron();
	CrearShaders();

	camera = Camera(glm::vec3(-0.75f, 4.5f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 0.3f, 0.5f);

	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	dadoTexture = Texture("Textures/logos_starwars.png");
	dadoTexture.LoadTextureA();
	holocronTexture = Texture("Textures/holocron_logos2.jpeg");
	holocronTexture.LoadTextureA();

	ModeloDado = Model();
	ModeloDado.LoadModel("Models/cubo_starwars.obj");

	ModeloHolocron = Model();
	ModeloHolocron.LoadModel("Models/holocron_simple.obj");

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		GLfloat velocidadRotacion = 60.0f * (now - lastTime);
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limiteFrames;
		lastTime = now;

		glfwPollEvents();

		bool* teclasPresionadas = mainWindow.getsKeys();
		GLfloat direccionGiro = 1.0f - 2.0f * teclasPresionadas[GLFW_KEY_LEFT_SHIFT];
		rotObjX += velocidadRotacion * direccionGiro * teclasPresionadas[GLFW_KEY_E];
		rotObjY += velocidadRotacion * direccionGiro * teclasPresionadas[GLFW_KEY_R];
		rotObjZ += velocidadRotacion * direccionGiro * teclasPresionadas[GLFW_KEY_T];
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformColor = shaderList[0].getColorLocation();
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		color = glm::vec3(1.0f, 1.0f, 1.0f);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		pisoTexture.UseTexture();
		meshListModel[2]->RenderMeshModel();

		//Ejercicio 1: Dado optimizado Star Wars
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-1.5f, 4.5f, -2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dadoTexture.UseTexture();
		meshListModel[4]->RenderMeshModel();

		//Ejercicio 2: Cubo importado
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 4.5f, -2.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rota en X
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rota en Y

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		ModeloDado.RenderModel();

		//Holocron (Código)
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-8.5f, 4.5f, 0.0f));
		model = glm::rotate(model, rotObjX * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, rotObjY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotObjZ * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		holocronTexture.UseTexture();
		meshListModel[5]->RenderMeshModel();

		//Holocron (Importado)
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-4.5f, 2.5f, 0.0f));
		model = glm::rotate(model, rotObjX * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, rotObjY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotObjZ * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		ModeloHolocron.RenderModel();

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}