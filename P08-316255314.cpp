/*
Pr�ctica 7: Iluminaci�n 1 
*/
//para cargar imagen
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
//para probar el importer-
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

//para iluminaci�n
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture dadoTexture;
Texture waterTexture;

Model Kitt_M;
Model Llanta_M;
Model Blackhawk_M;
Model Lampara_M;
Model PezCuerpo_M;
Model PezAntena_M;
Model PezFoco_M;


Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;


//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";


//funci�n de calculo de normales por promedio de v�rtices 
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}


void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4,5,6,
	   4,6,7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,   //cambiarlo para que se pueda ver la luz
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,


	};
	
	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);

	Mesh* obj4 = new Mesh();
	obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
	meshList.push_back(obj4);

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);

}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


void CrearDado()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,

		// back
		8, 9, 10,
		10, 11, 8,

		// left
		12, 13, 14,
		14, 15, 12,
		// bottom
		16, 17, 18,
		18, 19, 16,
		// top
		20, 21, 22,
		22, 23, 20,

		// right
		4, 5, 6,
		6, 7, 4,

	};	GLfloat cubo_vertices[] = {
		// front (z=+0.5, normal +Z)
		//x		y		z		S		T			NX		NY		NZ
		-0.5f, -0.5f,  0.5f,	0.26f,  0.34f,		0.0f,	0.0f,	1.0f,	//0
		0.5f, -0.5f,  0.5f,		0.49f,	0.34f,		0.0f,	0.0f,	1.0f,	//1
		0.5f,  0.5f,  0.5f,		0.49f,	0.66f,		0.0f,	0.0f,	1.0f,	//2
		-0.5f,  0.5f,  0.5f,	0.26f,	0.66f,		0.0f,	0.0f,	1.0f,	//3
		// right (x=+0.5, normal +X)
		//x		y		z		S		T
		0.5f, -0.5f,  0.5f,	    0.0f,  0.0f,		1.0f,	0.0f,	0.0f,
		0.5f, -0.5f,  -0.5f,	1.0f,	0.0f,		1.0f,	0.0f,	0.0f,
		0.5f,  0.5f,  -0.5f,	1.0f,	1.0f,		1.0f,	0.0f,	0.0f,
		0.5f,  0.5f,  0.5f,	    0.0f,	1.0f,		1.0f,	0.0f,	0.0f,
		// back (z=-0.5, normal -Z)
		-0.5f, -0.5f, -0.5f,	0.0f,  0.0f,		0.0f,	0.0f,	-1.0f,
		0.5f, -0.5f, -0.5f,		1.0f,	0.0f,		0.0f,	0.0f,	-1.0f,
		0.5f,  0.5f, -0.5f,		1.0f,	1.0f,		0.0f,	0.0f,	-1.0f,
		-0.5f,  0.5f, -0.5f,	0.0f,	1.0f,		0.0f,	0.0f,	-1.0f,

		// left (x=-0.5, normal -X)
		//x		y		z		S		T
		-0.5f, -0.5f,  -0.5f,	0.0f,  0.0f,		-1.0f,	0.0f,	0.0f,
		-0.5f, -0.5f,  0.5f,	1.0f,	0.0f,		-1.0f,	0.0f,	0.0f,
		-0.5f,  0.5f,  0.5f,	1.0f,	1.0f,		-1.0f,	0.0f,	0.0f,
		-0.5f,  0.5f,  -0.5f,	0.0f,	1.0f,		-1.0f,	0.0f,	0.0f,

		// bottom (y=-0.5, normal -Y)
		//x		y		z		S		T
		-0.5f, -0.5f,  0.5f,	0.0f,  0.0f,		0.0f,	-1.0f,	0.0f,
		0.5f,  -0.5f,  0.5f,	1.0f,	0.0f,		0.0f,	-1.0f,	0.0f,
		 0.5f,  -0.5f,  -0.5f,	1.0f,	1.0f,		0.0f,	-1.0f,	0.0f,
		-0.5f, -0.5f,  -0.5f,	0.0f,	1.0f,		0.0f,	-1.0f,	0.0f,

		// top (y=+0.5, normal +Y)
		 //x		y		z		S		T
		 -0.5f, 0.5f,  0.5f,	0.0f,  0.0f,		0.0f,	1.0f,	0.0f,
		 0.5f,  0.5f,  0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
		  0.5f, 0.5f,  -0.5f,	1.0f,	1.0f,		0.0f,	1.0f,	0.0f,
		 -0.5f, 0.5f,  -0.5f,	0.0f,	1.0f,		0.0f,	1.0f,	0.0f,

	};

	Mesh* dado = new Mesh();
	dado->CreateMesh(cubo_vertices, cubo_indices, 192, 36);
	meshList.push_back(dado);

}




void CrearOctaedro()
{
	// Cada cara es un tri�ngulo independiente con su propia normal plana.
	// Normales calculadas con producto cruzado (v1-v0) x (v2-v0) y normalizadas.
	// Caras 1-4: hemisferio superior (ny = +0.4472)
	// Caras 5-8: hemisferio inferior (ny = -0.4472)
	unsigned int oct_indices[] = {
		 0,  1,  2,   // cara 1
		 3,  4,  5,   // cara 2
		 6,  7,  8,   // cara 3
		 9, 10, 11,   // cara 4
		12, 13, 14,   // cara 5
		15, 16, 17,   // cara 6
		18, 19, 20,   // cara 7
		21, 22, 23,   // cara 8
	};

	GLfloat oct_vertices[] = {
		// x       y       z        u       v         nx        ny        nz
		// Normales invertidas (inward) para coincidir con la convencion del shader
		// cara 1 - superior frontal
		-0.5f,  0.0f,  0.5f,    0.3f,  0.51f,    0.0f,  -0.4472f, -0.8944f,
		 0.5f,  0.0f,  0.5f,    0.7f,  0.51f,    0.0f,  -0.4472f, -0.8944f,
		 0.0f,  1.0f,  0.0f,    0.5f,  0.72f,    0.0f,  -0.4472f, -0.8944f,
		// cara 2 - superior derecha
		 0.5f,  0.0f,  0.5f,   0.25f,  0.49f,  -0.8944f, -0.4472f,  0.0f,
		 0.5f,  0.0f, -0.5f,   0.04f,  0.26f,  -0.8944f, -0.4472f,  0.0f,
		 0.0f,  1.0f,  0.0f,   0.47f,  0.26f,  -0.8944f, -0.4472f,  0.0f,
		// cara 3 - superior trasera
		 0.5f,  0.0f, -0.5f,   0.06f,  0.74f,    0.0f,  -0.4472f,  0.8944f,
		-0.5f,  0.0f, -0.5f,   0.25f,  0.51f,    0.0f,  -0.4472f,  0.8944f,
		 0.0f,  1.0f,  0.0f,   0.48f,  0.74f,    0.0f,  -0.4472f,  0.8944f,
		// cara 4 - superior izquierda
		-0.5f,  0.0f, -0.5f,   0.3f,  0.49f,   0.8944f, -0.4472f,  0.0f,
		-0.5f,  0.0f,  0.5f,   0.7f,  0.49f,   0.8944f, -0.4472f,  0.0f,
		 0.0f,  1.0f,  0.0f,   0.5f,  0.29f,   0.8944f, -0.4472f,  0.0f,
		// cara 5 - inferior frontal
		-0.5f,  0.0f,  0.5f,   0.94f,  0.76f,   0.0f,   0.4472f, -0.8944f,
		 0.5f,  0.0f,  0.5f,   0.75f,  0.98f,   0.0f,   0.4472f, -0.8944f,
		 0.0f, -1.0f,  0.0f,   0.52f,  0.76f,   0.0f,   0.4472f, -0.8944f,
		// cara 6 - inferior derecha
		 0.5f,  0.0f,  0.5f,   0.95f,  0.26f,  -0.8944f,  0.4472f,  0.0f,
		 0.5f,  0.0f, -0.5f,   0.75f,  0.49f,  -0.8944f,  0.4472f,  0.0f,
		 0.0f, -1.0f,  0.0f,   0.52f,  0.26f,  -0.8944f,  0.4472f,  0.0f,
		// cara 7 - inferior trasera
		 0.5f,  0.0f, -0.5f,   0.75f,  0.51f,   0.0f,   0.4472f,  0.8944f,
		-0.5f,  0.0f, -0.5f,   0.97f,  0.74f,   0.0f,   0.4472f,  0.8944f,
		 0.0f, -1.0f,  0.0f,   0.52f,  0.74f,   0.0f,   0.4472f,  0.8944f,
		// cara 8 - inferior izquierda
		-0.5f,  0.0f, -0.5f,   0.75f,  0.03f,   0.8944f,  0.4472f,  0.0f,
		-0.5f,  0.0f,  0.5f,   0.97f,  0.23f,   0.8944f,  0.4472f,  0.0f,
		 0.0f, -1.0f,  0.0f,   0.52f,  0.24f,   0.8944f,  0.4472f,  0.0f,
	};

	Mesh* oct = new Mesh();
	oct->CreateMesh(oct_vertices, oct_indices, 192, 24);
	meshList.push_back(oct); // meshList[5]
}

void CrearPecera()
{
	// meshList[6]: 4 paredes laterales con normales hacia adentro (sin fondo ni tapa)
	unsigned int paredes_indices[] = {
		 0,  1,  2,   2,  3,  0,   // frente
		 4,  5,  6,   6,  7,  4,   // trasera
		 8,  9, 10,  10, 11,  8,   // derecha
		12, 13, 14,  14, 15, 12,   // izquierda
	};
	GLfloat paredes_vertices[] = {
		//   x       y      z        u     v      nx    ny     nz
		// Frente (z=+0.5), normal inward
		-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f,  0.5f,   1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f,  0.5f,   0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
		// Trasera (z=-0.5), normal inward
		 0.5f, -0.5f, -0.5f,   0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,   1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,   1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,   0.0f, 1.0f,  0.0f,  0.0f,  1.0f,
		// Derecha (x=+0.5), normal inward
		 0.5f, -0.5f,  0.5f,   0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,   1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,   1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,   0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		// Izquierda (x=-0.5), normal inward
		-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,   1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,   1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
	};
	Mesh* paredes = new Mesh();
	paredes->CreateMesh(paredes_vertices, paredes_indices, 128, 24);
	meshList.push_back(paredes); // meshList[6]

	// meshList[7]: fondo (opaco oscuro), normal hacia arriba (inward para fondo)
	unsigned int fondo_indices[] = { 0, 1, 2,  2, 3, 0 };
	GLfloat fondo_vertices[] = {
		-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,   1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,   1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,   0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
	};
	Mesh* fondo = new Mesh();
	fondo->CreateMesh(fondo_vertices, fondo_indices, 32, 6);
	meshList.push_back(fondo); // meshList[7]

	// meshList[8]: tapa superior con textura de agua, normal hacia arriba (+Y), visible desde afuera
	unsigned int tapa_indices[] = { 0, 1, 2,  2, 3, 0 };
	GLfloat tapa_vertices[] = {
		-0.5f,  0.5f, -0.5f,   0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,   0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,   1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,   1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
	};
	Mesh* tapa = new Mesh();
	tapa->CreateMesh(tapa_vertices, tapa_indices, 32, 6);
	meshList.push_back(tapa); // meshList[8]
}


int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();
	CreateObjects();
	CrearDado();
	CrearOctaedro();
	CrearPecera();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	AgaveTexture = Texture("Textures/Agave.tga");
	AgaveTexture.LoadTextureA();
	dadoTexture = Texture("Textures/octaedro.png");
	dadoTexture.LoadTextureA();
	waterTexture = Texture("Textures/Agua.jpg");
	waterTexture.LoadTextureA();

	Kitt_M = Model();
	Kitt_M.LoadModel("Models/Autofbx/autotexturamod.fbx");
	Llanta_M = Model();
	Llanta_M.LoadModel("Models/llanta_optimizada.obj");
	Blackhawk_M = Model();
	Blackhawk_M.LoadModel("Models/uh60.obj");
	Lampara_M = Model();
	Lampara_M.LoadModel("Models/LamparaSteamp/source/stpk_Street light 03_lod_03.obj");
	PezCuerpo_M = Model();
	PezCuerpo_M.LoadModel("Models/pez_abisal/cuerpo_pez.obj");
	PezAntena_M = Model();
	PezAntena_M.LoadModel("Models/pez_abisal/antena_pez.obj");
	PezFoco_M = Model();
	PezFoco_M.LoadModel("Models/pez_abisal/foco_pez.obj");
	

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);


	//luz direccional, s�lo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.3f, 0.3f,   //intensidad ambiental y la difusa
		0.0f, 0.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	// [0] Luz puntual roja de decoracion (siempre activa)
	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		-6.0f, 1.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	// [1] Luz puntual blanca de la lampara steampunk - "lampara practica 7" (toggle tecla 3)
	pointLights[1] = PointLight(1.0f, 1.0f, 1.0f,
		0.0f, 1.35f,
		5.0f, 2.5f, 3.0f,
		0.3f, 0.02f, 0.01f);
	pointLightCount++;

	// [2] Luz puntual azul del foco del pez (toggle tecla 5, posicion se actualiza en loop via jerarquia)
	pointLights[2] = PointLight(0.0f, 0.2f, 1.0f,
		0.0f, 1.5f,
		-4.0f, 2.35f, -2.0f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	unsigned int spotLightCount = 0;
	// [0] Linterna ligada a la camara
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;

	// [1] Helicoptero - spotlight ADELANTE (45 grados hacia -X), empieza apagado
	spotLights[1] = SpotLight(1.0f, 1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 5.0f, 6.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		25.0f);
	spotLightCount++;

	// [2] Helicoptero - spotlight ATRAS (45 grados hacia +X), empieza apagado
	spotLights[2] = SpotLight(1.0f, 0.5f, 0.0f,
		0.0f, 0.0f,
		0.0f, 5.0f, 6.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		25.0f);
	spotLightCount++;

	// [3] Spotlight del pez abisal (verde brillante, bulbo rota via jerarquia)
	spotLights[3] = SpotLight(0.1f, 1.0f, 0.2f,
		0.0f, 5.0f,
		-4.0f, 2.35f, -2.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		12.0f);
	spotLightCount++;

	// --- Estado de controles ---
	bool lamparaOn = true;   // tecla 3: luz puntual roja
	bool key3Prev  = false;
	bool pezLuzOn  = true;   // tecla 5: luz puntual azul del pez
	bool key5Prev  = false;
	// Angulos de rotacion del bulbo/antena del pez en grados (J/H=Y, N/M=X, B/V=Z)
	float bulbRotX = 0.0f;
	float bulbRotY = 0.0f;
	float bulbRotZ = 0.0f;

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0, uniformAlpha = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// --- Tecla 3: toggle luz puntual roja (lampara practica 7) ---
		bool key3Now = mainWindow.getsKeys()[GLFW_KEY_3];
		if (key3Now && !key3Prev)
			lamparaOn = !lamparaOn;
		key3Prev = key3Now;

		// --- Tecla 5: toggle luz puntual azul del pez ---
		bool key5Now = mainWindow.getsKeys()[GLFW_KEY_5];
		if (key5Now && !key5Prev)
			pezLuzOn = !pezLuzOn;
		key5Prev = key5Now;

		// --- Teclas J/H/N/M/B/V: rotar el bulbo del pez (la luz hereda la posicion por jerarquia) ---
		if (mainWindow.getsKeys()[GLFW_KEY_J]) bulbRotY += 2.0f;
		if (mainWindow.getsKeys()[GLFW_KEY_H]) bulbRotY -= 2.0f;
		if (mainWindow.getsKeys()[GLFW_KEY_N]) bulbRotX += 2.0f;
		if (mainWindow.getsKeys()[GLFW_KEY_M]) bulbRotX -= 2.0f;
		if (mainWindow.getsKeys()[GLFW_KEY_B]) bulbRotZ += 2.0f;
		if (mainWindow.getsKeys()[GLFW_KEY_V]) bulbRotZ -= 2.0f;

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformAlpha = shaderList[0].GetAlphaLocation();

		//informaci�n en el shader de intensidad especular y brill
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		// Alpha 1.0 por defecto para todos los objetos opacos
		glUniform1f(uniformAlpha, 1.0f);

		// Linterna ligada a la camara
		glm::vec3 lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		// Posicion del helicoptero
		glm::vec3 heliPos = glm::vec3(mainWindow.getmuevehz(), 5.0f, 6.0f);

		// Spotlight ADELANTE del helicoptero (K = mueve -X): amarillo, 45 grados hacia adelante
		bool movingFwd = mainWindow.getsKeys()[GLFW_KEY_K];
		float fwdDiff = movingFwd ? 2.0f : 0.0f;
		spotLights[1] = SpotLight(1.0f, 1.0f, 0.0f, 0.0f, fwdDiff,
			heliPos.x, heliPos.y, heliPos.z,
			-1.0f, -1.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 25.0f);

		// Spotlight ATRAS del helicoptero (I = mueve +X): naranja, 45 grados hacia atras
		bool movingBkw = mainWindow.getsKeys()[GLFW_KEY_I];
		float bkwDiff = movingBkw ? 2.0f : 0.0f;
		spotLights[2] = SpotLight(1.0f, 0.5f, 0.0f, 0.0f, bkwDiff,
			heliPos.x, heliPos.y, heliPos.z,
			1.0f, -1.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 25.0f);

		// Posicion del pez: diagonal en Z (adelante) y Y (arriba)
		// O = sube/avanza (+Y +Z), P = baja/retrocede (-Y -Z); sin limites, pez puede salir de la pecera
		float pezVal = mainWindow.getmuevepez();
		glm::vec3 pezPos = glm::vec3(
			-4.0f,
			2.0f + pezVal * 0.4f,
			-2.0f + pezVal * 0.4f);

		// Jerarquia del bulbo: construir matriz de rotacion a partir de los angulos acumulados
		glm::mat4 bulbRot = glm::mat4(1.0f);
		bulbRot = glm::rotate(bulbRot, bulbRotY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		bulbRot = glm::rotate(bulbRot, bulbRotX * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		bulbRot = glm::rotate(bulbRot, bulbRotZ * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

		// Base de la antena: encima de la cabeza del pez (no en el centro del cuerpo)
		glm::vec3 antennaBase = pezPos + glm::vec3(0.0f, 0.25f, 0.1f);
		// Punta fija de la antena (posicion fija del bulbo, no orbita)
		glm::vec3 focoWorldPos = antennaBase + glm::vec3(0.0f, 0.45f, 0.25f);

		// Direccion de la luz: la antena apunta por defecto hacia +Y; la luz sale hacia donde apunta el bulbo
		glm::vec3 bulbLightDir = glm::normalize(glm::vec3(bulbRot * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

		// Spotlight del pez: hereda posicion y direccion del bulbo via jerarquia
		spotLights[3].SetFlash(focoWorldPos, bulbLightDir);

		// Luz puntual roja: siempre activa (decoracion)
		pointLights[0] = PointLight(1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			-6.0f, 1.5f, 1.5f, 0.3f, 0.2f, 0.1f);

		// Luz puntual blanca lampara steampunk - toggle tecla 3 (independiente)
		float lampDiff = lamparaOn ? 1.35f : 0.0f;
		pointLights[1] = PointLight(1.0f, 1.0f, 1.0f, 0.0f, lampDiff,
			5.0f, 2.5f, 3.0f, 0.3f, 0.02f, 0.01f);

		// Luz puntual azul bulbo del pez - toggle tecla 5 (independiente), sigue al foco via jerarquia
		float pezDiff = pezLuzOn ? 1.5f : 0.0f;
		pointLights[2] = PointLight(0.0f, 0.2f, 1.0f, 0.0f, pezDiff,
			focoWorldPos.x, focoWorldPos.y, focoWorldPos.z, 0.3f, 0.2f, 0.1f);

		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);



		glm::mat4 model(1.0);
		glm::mat4 modelaux(1.0);
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		meshList[2]->RenderMesh();

		//Instancia del coche 
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f + mainWindow.getmuevex(), -0.3f, mainWindow.getmuevez()));
		modelaux = model;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Kitt_M.RenderModel(uniformColor);

		//Llanta delantera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(7.0f, -0.5f, 8.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		color = glm::vec3(0.5f, 0.5f, 0.5f);//llanta con color gris
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta trasera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(15.5f, -0.5f, 8.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta delantera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(7.0f, -0.5f, 1.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta trasera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(15.5f, -0.5f, 1.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();
	

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(mainWindow.getmuevehz(), 5.0f, 6.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Blackhawk_M.RenderModel(uniformColor);


		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-5.0f, 5.0f, 6.0));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform1f(uniformAlpha, 1.0f);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		dadoTexture.UseTexture();
		meshList[4]->RenderMesh();

		// Dado de 8 caras (octaedro) con normales planas por cara
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 1.5f, 5.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		dadoTexture.UseTexture();
		meshList[5]->RenderMesh();

		// --- Lampara steampunk (practica 7) ---
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(5.0f, -1.0f, 3.0f));
		model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Lampara_M.RenderModel(uniformColor);

		// --- Pez abisal (cuerpo, antena, foco) ---
		// Cuerpo: solo traslacion
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		model = glm::mat4(1.0);
		model = glm::translate(model, pezPos);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PezCuerpo_M.RenderModel(uniformColor);

		// Antena: pieza fija conectora, sin rotacion, parte desde la cabeza del pez
		model = glm::mat4(1.0);
		model = glm::translate(model, antennaBase);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PezAntena_M.RenderModel(uniformColor);

		// Foco/bulbo: posicion fija en la punta de la antena, gira sobre su propio eje
		color = glm::vec3(0.1f, 1.0f, 0.3f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		model = glm::translate(model, focoWorldPos);
		model = model * bulbRot;
		model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PezFoco_M.RenderModel(uniformColor);

		// --- Pecera (4x3x4), centrada en (-4, 2.5, -2): fondo en y=1.0 ---
		// Fondo: opaco oscuro
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-4.0f, 2.5f, -2.0f));
		model = glm::scale(model, glm::vec3(4.0f, 3.0f, 4.0f));
		color = glm::vec3(0.0f, 0.0f, 0.0f); // negro
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform1f(uniformAlpha, 1.0f); // opaco
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		plainTexture.UseTexture();
		meshList[7]->RenderMesh(); // fondo

		// Tapa: textura de agua, 50% transparente, visible desde ambos lados
		color = glm::vec3(3.5f, 3.5f, 3.5f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform1f(uniformAlpha, 0.5f);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		waterTexture.UseTexture();
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		meshList[8]->RenderMesh(); // tapa
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glUniform1f(uniformAlpha, 1.0f);

		// Paredes: translucidas, azul casi transparente
		color = glm::vec3(0.1f, 0.4f, 1.0f); // azul vidrio
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform1f(uniformAlpha, 0.12f); // casi transparente
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		meshList[6]->RenderMesh(); // paredes
		glDisable(GL_BLEND);
		// Restaurar alpha opaco para objetos siguientes
		glUniform1f(uniformAlpha, 1.0f);

		//agave que sucede si lo renderizan antes del coche y el helicoptero
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, -4.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
