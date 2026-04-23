/*
Animaci�n:
Sesi�n 1: Simple o b�sica:Por banderas y condicionales (m�s de 1 transformaci�n geom�trica se ve modificada)
Sesi�n 2: Compleja: Por medio de funciones y algoritmos. Textura Animada
*/

//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
//para probar el importer
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

// Sistema de waypoints para el recorrido del carro por la pista
struct WaypointCar {
	float x, y, z;    
	float yaw;        // orientacion horizontal (180 = apuntando -X)
	float pitch;      // inclinacion nariz (deg, + = sube, - = baja)
};
const int CAR_NUM_WP = 7;

WaypointCar carWaypoints[CAR_NUM_WP] = {
	{   0.0f,  2.0f,  0.0f,   0.0f,  0.0f },  // WP0: inicio recto (plano)
	{ -60.0f,  2.0f,  0.0f,   0.0f,  0.0f },  // WP1
	{ -75.0f,  4.0f, -3.0f,  -6.0f,  5.0f },  // WP2
	{ -90.0f, 14.0f,-14.0f, -30.0f, 17.0f },  // WP3
	{-120.0f, 29.0f, -1.0f,   0.0f, 35.0f },  // WP4
	{-150.0f, 31.0f,  1.0f,  -3.0f,  0.0f },  
	{-160.0f, 31.0f,  1.5f,  -3.0f,  0.0f },  // WP6: fin pista (STOP)
};
float carWpT      = 0.0f;   // interpolacion [0,1] dentro del segmento actual
int   carWpIdx    = 0;      // indice del waypoint actual
float carSpeed    = 0.5f;   // velocidad en unidades world / deltaTime-unit
bool  carFinished = false;  // true cuando llega al ultimo WP
bool  keyTPrev    = false;  // deteccion de flanco para tecla T (reinicio)
float rotllanta   = 0.0f;
float rotllantaOffset = 20.0f;
// Posicion/orientacion interpolada, calculada cada frame
float carX = 0.0f, carY = 2.0f, carZ = 0.0f;
float carYaw = 180.0f, carPitch = 0.0f;

//variables para animaci�n de la nave
// Ida: vuela de X=0 hacia X=-270 (sentido -X, mismo que el carro)
// Vuelta: gira 180 y regresa a X=0 aterrizando (no se repite)
float navePos       = 0.0f;    // posici�n X (empieza en el origen)
float naveOffset    = 0.5f;    // velocidad de vuelo
float naveYaw       = 0.0f;    // apunta en +X (direcci�n del primer vuelo)
float naveYawOffset = 1.0f;    // velocidad de giro al dar vuelta
int   naveAnimState = 0;       // 0=ida(-X), 1=gira 180, 2=regresa(+X), 3=aterrizada
const float NAVE_MAX_DIST = 270.0f;
float rotHelice       = 0.0f;   // �ngulo acumulado de las h�lices
float rotHeliceOffset = 200.0f; // velocidad de giro de h�lices

// Dado (octaedro de 8 caras)
float dadoY        = 15.0f;   // posicion Y actual (empieza arriba)
float dadoRotX     = 0.0f;    // rotacion giratoria acumulada en X
float dadoRotZ     = 0.0f;    // rotacion giratoria acumulada en Z
float dadoFallSpd  = 0.0f;    // velocidad de caida (acelerada por gravedad)
int   dadoState    = 0;       // 0=esperando, 1=cayendo+girando, 3=aterrizado
int   dadoFaceUp   = 0;       // cara elegida al azar (0-7)
bool  keyRPrev     = false;
// Rotaciones finales para mostrar cada cara hacia arriba (rotX, rotZ en grados)
// Calculadas para que la normal de cada cara apunte a +Y
float dadoFaceRX[8] = { -63.43f,    0.0f,  63.43f,    0.0f, -116.57f,    0.0f,  116.57f,    0.0f };
float dadoFaceRZ[8] = {    0.0f, -63.43f,   0.0f,  63.43f,     0.0f, -116.57f,    0.0f,  116.57f };

float toffsetflechau = 0.0f;
float toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0;
float angulovaria = 0.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture FlechaTexture;
Texture NumerosTexture;
Texture Numero1Texture;
Texture Numero2Texture;
Texture dadoTexture;



Model Kitt_M;
Model Llanta_M;
Model Pista_M;
Model Nave_M;
Model Ala_M;
Model Helice_M;

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



//c�lculo del promedio de las normales para sombreado de Phong
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
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,


	};
	

	unsigned int flechaIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat flechaVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int scoreIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat scoreVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int numeroIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat numeroVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		0.25f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		0.25f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

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

	Mesh* obj5 = new Mesh();
	obj5->CreateMesh(flechaVertices, flechaIndices, 32, 6);
	meshList.push_back(obj5);

	Mesh* obj6 = new Mesh();
	obj6->CreateMesh(scoreVertices, scoreIndices, 32, 6);
	meshList.push_back(obj6); // todos los n�meros

	Mesh* obj7 = new Mesh();
	obj7->CreateMesh(numeroVertices, numeroIndices, 32, 6);
	meshList.push_back(obj7); // solo un n�mero

	// octaedro dado de 8 caras (meshList[7])
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
		// Normales hacia AFUERA (outward) para iluminaci�n correcta
		// cara 1 - superior frontal
		-0.5f,  0.0f,  0.5f,    0.3f,  0.51f,    0.0f,  0.4472f,  0.8944f,
		 0.5f,  0.0f,  0.5f,    0.7f,  0.51f,    0.0f,  0.4472f,  0.8944f,
		 0.0f,  1.0f,  0.0f,    0.5f,  0.72f,    0.0f,  0.4472f,  0.8944f,
		// cara 2 - superior derecha
		 0.5f,  0.0f,  0.5f,   0.25f,  0.49f,   0.8944f,  0.4472f,  0.0f,
		 0.5f,  0.0f, -0.5f,   0.04f,  0.26f,   0.8944f,  0.4472f,  0.0f,
		 0.0f,  1.0f,  0.0f,   0.47f,  0.26f,   0.8944f,  0.4472f,  0.0f,
		// cara 3 - superior trasera
		 0.5f,  0.0f, -0.5f,   0.06f,  0.74f,    0.0f,  0.4472f, -0.8944f,
		-0.5f,  0.0f, -0.5f,   0.25f,  0.51f,    0.0f,  0.4472f, -0.8944f,
		 0.0f,  1.0f,  0.0f,   0.48f,  0.74f,    0.0f,  0.4472f, -0.8944f,
		// cara 4 - superior izquierda
		-0.5f,  0.0f, -0.5f,   0.3f,  0.49f,  -0.8944f,  0.4472f,  0.0f,
		-0.5f,  0.0f,  0.5f,   0.7f,  0.49f,  -0.8944f,  0.4472f,  0.0f,
		 0.0f,  1.0f,  0.0f,   0.5f,  0.29f,  -0.8944f,  0.4472f,  0.0f,
		// cara 5 - inferior frontal
		-0.5f,  0.0f,  0.5f,   0.94f,  0.76f,   0.0f,  -0.4472f,  0.8944f,
		 0.5f,  0.0f,  0.5f,   0.75f,  0.98f,   0.0f,  -0.4472f,  0.8944f,
		 0.0f, -1.0f,  0.0f,   0.52f,  0.76f,   0.0f,  -0.4472f,  0.8944f,
		// cara 6 - inferior derecha
		 0.5f,  0.0f,  0.5f,   0.95f,  0.26f,   0.8944f, -0.4472f,  0.0f,
		 0.5f,  0.0f, -0.5f,   0.75f,  0.49f,   0.8944f, -0.4472f,  0.0f,
		 0.0f, -1.0f,  0.0f,   0.52f,  0.26f,   0.8944f, -0.4472f,  0.0f,
		// cara 7 - inferior trasera
		 0.5f,  0.0f, -0.5f,   0.75f,  0.51f,   0.0f,  -0.4472f, -0.8944f,
		-0.5f,  0.0f, -0.5f,   0.97f,  0.74f,   0.0f,  -0.4472f, -0.8944f,
		 0.0f, -1.0f,  0.0f,   0.52f,  0.74f,   0.0f,  -0.4472f, -0.8944f,
		// cara 8 - inferior izquierda
		-0.5f,  0.0f, -0.5f,   0.75f,  0.03f,  -0.8944f, -0.4472f,  0.0f,
		-0.5f,  0.0f,  0.5f,   0.97f,  0.23f,  -0.8944f, -0.4472f,  0.0f,
		 0.0f, -1.0f,  0.0f,   0.52f,  0.24f,  -0.8944f, -0.4472f,  0.0f,
	};
	Mesh* oct = new Mesh();
	oct->CreateMesh(oct_vertices, oct_indices, 192, 24);
	meshList.push_back(oct); // meshList[7]

}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}




int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.5f, 0.5f);

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
	FlechaTexture = Texture("Textures/flechas.tga");
	FlechaTexture.LoadTextureA();
	NumerosTexture = Texture("Textures/numerosbase.tga");
	NumerosTexture.LoadTextureA();
	Numero1Texture = Texture("Textures/numero1.tga");
	Numero1Texture.LoadTextureA();
	Numero2Texture = Texture("Textures/numero2.tga");
	Numero2Texture.LoadTextureA();
	dadoTexture = Texture("Textures/octaedro.png");
	dadoTexture.LoadTextureA();


	Kitt_M = Model();
	Kitt_M.LoadModel("Models/Autofbx/autotexturamod.fbx");
	Llanta_M = Model();
	Llanta_M.LoadModel("Models/llanta_optimizada.obj");
	Pista_M = Model();
	Pista_M.LoadModel("Models/pista.obj");
	Nave_M = Model();
	Nave_M.LoadModel("Models/Nave-sinalas-sinhelices.obj");
	Ala_M = Model();
	Ala_M.LoadModel("Models/ala.obj");
	Helice_M = Model();
	Helice_M.LoadModel("Models/Helice.obj");

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
		0.3f, 0.3f,
		0.0f, -10.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	//Declaraci�n de primer luz puntual
	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 2.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	unsigned int spotLightCount = 0;
	//linterna
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;

	// [1] Nave - spotlight ATRAS (naranja, como helic�ptero)
	spotLights[1] = SpotLight(1.0f, 0.5f, 0.0f,
		0.0f, 0.0f,
		0.0f, 3.0f, 1.5f,
		1.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		25.0f);
	spotLightCount++;

	// [2] Auto - spotlight frontal (blanco-amarillo, sigue al carro)
	spotLights[2] = SpotLight(1.0f, 1.0f, 0.8f,
		0.0f, 2.0f,
		0.0f, 2.0f, 0.0f,
		-1.0f, -0.3f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);
	spotLightCount++;

	// [3] Nave - spotlight ADELANTE (amarillo, como helic�ptero)
	spotLights[3] = SpotLight(1.0f, 1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 3.0f, 1.5f,
		-1.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		25.0f);
	spotLightCount++;



	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset=0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	
	carWpIdx   = 0;
	carWpT     = 0.0f;
	carFinished= false;
	keyTPrev   = false;
	rotllanta  = 0.0f;

	// nave: empieza en el origen, vuela hacia +X, gira 180 y regresa
	navePos       = 0.0f;
	naveYaw       = 0.0f;    // apunta hacia +X (direcci�n de ida)
	naveAnimState = 0;

	// dado: empieza arriba esperando tecla R
	dadoY        = 15.0f;
	dadoFallSpd  = 0.0f;
	dadoRotX     = 0.0f;
	dadoRotZ     = 0.0f;
	dadoState    = 0;
	dadoFaceUp   = 0;
	srand((unsigned int)time(0));

	glm::vec3 lowerLight(0.0f,0.0f,0.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	lastTime = glfwGetTime();

	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		angulovaria += 0.5f * deltaTime;
		if (naveAnimState != 3)
			rotHelice += rotHeliceOffset * deltaTime;  // h�lices giran mientras vuela (estados 0-2)

		// Tecla T: reinicia recorrido del auto
		bool keyTNow = mainWindow.getsKeys()[GLFW_KEY_T];
		if (keyTNow && !keyTPrev) {
			carWpIdx    = 0;
			carWpT      = 0.0f;
			carFinished = false;
		}
		keyTPrev = keyTNow;

		// Interpola posicion/orientacion entre waypoints actuales
		int nextIdx = (carWpIdx < CAR_NUM_WP - 1) ? carWpIdx + 1 : CAR_NUM_WP - 1;
		WaypointCar& wp0 = carWaypoints[carWpIdx];
		WaypointCar& wp1 = carWaypoints[nextIdx];
		float t = carWpT;
		carX     = wp0.x     + (wp1.x     - wp0.x)     * t;
		carY     = wp0.y     + (wp1.y     - wp0.y)     * t;
		carZ     = wp0.z     + (wp1.z     - wp0.z)     * t;
		carYaw   = wp0.yaw   + (wp1.yaw   - wp0.yaw)   * t;
		carPitch = wp0.pitch + (wp1.pitch - wp0.pitch) * t;

		// Avanza por la pista si no ha terminado
		if (!carFinished) {
			rotllanta += rotllantaOffset * deltaTime;
			float dx = wp1.x - wp0.x, dy = wp1.y - wp0.y, dz = wp1.z - wp0.z;
			float segLen = sqrtf(dx*dx + dy*dy + dz*dz);
			if (segLen < 0.001f) segLen = 0.001f;
			carWpT += (carSpeed / segLen) * deltaTime;
			if (carWpT >= 1.0f) {
				carWpT = 0.0f;
				carWpIdx++;
				if (carWpIdx >= CAR_NUM_WP - 1) {
					carWpIdx    = CAR_NUM_WP - 1;
					carWpT      = 1.0f;
					carFinished = true;
				}
			}
		}

		// Animaci�n de la nave: ida en +X, gira 180, regresa en -X y aterriza (no se repite)
		switch (naveAnimState)
		{
		case 0: // Vuela de X=0 hacia X=+270 (direcci�n +X, opuesta al carro)
			navePos += naveOffset * deltaTime;
			if (navePos >= NAVE_MAX_DIST) {
				navePos = NAVE_MAX_DIST;
				naveAnimState = 1;
			}
			break;
		case 1: // Gira 180 grados (naveYaw de 0 a 180)
			naveYaw += naveYawOffset * deltaTime;
			if (naveYaw >= 180.0f) {
				naveYaw = 180.0f;
				naveAnimState = 2;
			}
			break;
		case 2: // Regresa de X=+270 a X=0, descendiendo para aterrizar
			navePos -= naveOffset * deltaTime;
			if (navePos <= 0.0f) {
				navePos = 0.0f;
				naveAnimState = 3;
			}
			break;
		case 3: // Aterrizada, no se repite
			break;
		}
		// state 1 = aterrizada, no hace nada

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Animaci�n del dado: tecla R lanza, cae girando, muestra cara aleatoria
		bool keyRNow = mainWindow.getsKeys()[GLFW_KEY_R];
		if (keyRNow && !keyRPrev && dadoState != 1) {
			dadoY       = 15.0f;
			dadoFallSpd = 0.0f;
			dadoRotX    = 0.0f;
			dadoRotZ    = 0.0f;
			dadoFaceUp  = rand() % 8;
			dadoState   = 1;
		}
		keyRPrev = keyRNow;

		if (dadoState == 1) {
			dadoFallSpd += 0.003f * deltaTime;   // gravedad suave (~80 frames de ca�da)
			dadoY       -= dadoFallSpd * deltaTime;
			dadoRotX    += 4.0f * deltaTime;     // giro visible pero no tan r�pido
			dadoRotZ    += 2.7f * deltaTime;
			if (dadoY <= -0.5f) {
				dadoY    = -0.5f;  // toca el suelo (piso en Y=-2, dado escala 1.5)
				dadoRotX = dadoFaceRX[dadoFaceUp];  // encaja mostrando cara aleatoria
				dadoRotZ = dadoFaceRZ[dadoFaceUp];
				dadoState = 3;  // aterrizado
			}
		}

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
		uniformTextureOffset = shaderList[0].getOffsetLocation(); // para la textura con movimiento

		//informaci�n en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// luz ligada a la c�mara de tipo flash
		lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		// Spotlight del auto: faro frontal P08-style (45 grados adelante-abajo)
		// Direccion sigue el heading del tramo actual de la pista
		{
			int ni = (carWpIdx < CAR_NUM_WP - 1) ? carWpIdx + 1 : CAR_NUM_WP - 1;
			glm::vec3 wpC(carWaypoints[carWpIdx].x, carWaypoints[carWpIdx].y, carWaypoints[carWpIdx].z);
			glm::vec3 wpN(carWaypoints[ni].x, carWaypoints[ni].y, carWaypoints[ni].z);
			glm::vec3 heading = (glm::length(wpN - wpC) > 0.001f) ?
				glm::normalize(wpN - wpC) : glm::vec3(-1.0f, 0.0f, 0.0f);
			glm::vec3 lPos(carX + heading.x * 3.0f, carY + 0.5f, carZ + heading.z * 3.0f);
			spotLights[2] = SpotLight(1.0f, 1.0f, 0.8f, 0.0f, 2.0f,
				lPos.x, lPos.y, lPos.z,
				heading.x, -1.0f, heading.z,
				1.0f, 0.0f, 0.0f, 20.0f);
		}

		// Spotlights de la nave: siguen posici�n de nave durante el aterrizaje
		// La nave viaja en -X, spotlight [3] ilumina la pista adelante/abajo
		// Altura de la nave seg�n estado:
		// - Ida (0) y giro (1): crucero alto constant
		// - Regreso (2): desciende gradualmente de 25 a 0.5 para aterrizar
		// - Aterrizada (3): en el suelo
		float naveLY;
		if (naveAnimState == 0 || naveAnimState == 1) {
			naveLY = 25.0f + sinf(angulovaria * 0.8f) * 2.0f;
		} else if (naveAnimState == 2) {
			float naveT = navePos / NAVE_MAX_DIST; // 1.0 al inicio del regreso, 0 al aterrizar
			naveLY = 0.5f + naveT * 24.5f + sinf(angulovaria * 0.8f) * 2.0f;
		} else {
			naveLY = 0.5f;
		}
		glm::vec3 naveLightPos(navePos, naveLY, 1.5f);
		// Luces siguen la orientaci�n de vuelo: delantera ilumina la pista hacia adelante
		float fwdX = (naveAnimState <= 1) ? 1.0f : -1.0f; // +X en ida, -X en regreso
		// [1] Faro frontal de la nave: 45 grados hacia adelante-abajo, centrado
		spotLights[1] = SpotLight(1.0f, 1.0f, 0.0f, 0.0f, 2.0f,
			naveLightPos.x + fwdX * 3.0f, naveLightPos.y, naveLightPos.z,
			fwdX, -1.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 25.0f);
		// [3] apagada
		spotLights[3] = SpotLight(0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 25.0f);

		//informaci�n al shader de fuentes de iluminaci�n
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);


		//Reinicializando variables cada ciclo de reloj
		model= glm::mat4(1.0);
		modelaux= glm::mat4(1.0);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		toffset = glm::vec2(0.0f, 0.0f);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();

		//Pista
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.1f, 2.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Pista_M.RenderModel();

		//Instancia del coche: sigue waypoints de la pista, escala reducida a 4
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(carX, carY, carZ));
		model = glm::rotate(model, carYaw * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -carPitch * toRadians, glm::vec3(0.0f, 0.0f, 1.0f)); // pitch: negativo = nariz sube
		modelaux = model; // llantas heredan posicion, yaw y pitch
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Kitt_M.RenderModel(uniformColor);

		// Llantas: offsets escalados a 4/7 de los originales (escala 4 vs 7)
		color = glm::vec3(0.5f, 0.5f, 0.5f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		//Llanta delantera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(5.1f, -0.6f, 3.4f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta trasera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(-6.3f, -0.6f, 3.4f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta delantera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(5.1f, -0.6f, -3.4f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta trasera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(-6.3f, -0.6f, -3.4f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();


		// Nave: sobrevuela la pista en un solo sentido y aterriza
		// naveLY y naveT ya calculados arriba en la secci�n de spotlights
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(navePos, naveLY, 1.5f));
		model = glm::rotate(model, naveYaw * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glm::mat4 naveAux = model; // alas/h�lices heredan jerarqu�a de la nave
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Nave_M.RenderModel();

		// Alas: aletean mientras vuela, se detienen al aterrizar
		float alaFlap = (naveAnimState != 3) ? sinf(angulovaria * 1.5f) * 30.0f : 0.0f;

		// Ala izquierda
		model = naveAux;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.3f));
		model = glm::rotate(model, alaFlap * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Ala_M.RenderModel();

		// Ala derecha: espejo en Z con scale negativo
		model = naveAux;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.3f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, -1.0f));
		model = glm::rotate(model, alaFlap * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glDisable(GL_CULL_FACE);
		Ala_M.RenderModel();
		glEnable(GL_CULL_FACE);

		// H�lice izquierda: gira mientras vuela
		model = naveAux;
		model = glm::translate(model, glm::vec3(-0.4f, -0.3f, -0.4f));
		model = glm::rotate(model, rotHelice * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Helice_M.RenderModel();

		// H�lice derecha
		model = naveAux;
		model = glm::translate(model, glm::vec3(-0.4f, -0.3f, 0.4f));
		model = glm::rotate(model, -rotHelice * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Helice_M.RenderModel();

		// Dado de 8 caras (octaedro): cae girando al presionar R, muestra cara aleatoria
		// glDisable culling: el dado gira libremente y todas sus caras deben ser visibles
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(5.0f, dadoY, 5.0f));
		model = glm::rotate(model, dadoRotX * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, dadoRotZ * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		dadoTexture.UseTexture();
		glDisable(GL_CULL_FACE);
		meshList[7]->RenderMesh();
		glEnable(GL_CULL_FACE);


		//Agave �qu� sucede si lo renderizan antes del coche y de la pista?
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.5f, -2.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
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
