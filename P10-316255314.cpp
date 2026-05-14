/*
Animaci�n:
Sesi�n 1:
Simple o b�sica:Por banderas y condicionales (m�s de 1 transformaci�n geom�trica se ve modificada
Sesi�n 2
Compleja: Por medio de funciones y algoritmos.
Textura Animada
*/
//para cargar imagen.
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

//variables para animaci�n
float movCoche;
float movOffset;
float rotllanta;
float rotllantaOffset;
bool avanza;
float toffsetflechau = 0.0f;
float toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0;
float toffsetnumerocambiav = 0.0;
int numeroActual = 0;
float angulovaria = 0.0f;
float engranajeAngle = 0.0f;

// ─── KEYFRAMES: ProyectorSteampunk ───────────────────────────────────────────
// Anima dos variables:
//   movProyector  — rotación Y del cuerpo (grados)
//   engranajeAngle — rotación local de engranajes (radianes)
//
// Tecla P: primera vez → secuencia de ENCENDIDO  (KF 0→6)
//          segunda vez → secuencia de APAGADO     (KF 6→0, inversa)
//
// La luz del proyector (spotLights[2]) se enciende cuando engranajeAngle >= LUZ_UMBRAL
// y se apaga cuando baja de ese umbral.
#define MAX_FRAMES_PROY 20
const float LUZ_UMBRAL = 6.0f;   // rad — umbral para encender la luz

typedef struct _frameProy {
	float movProyector;       // ángulo Y cuerpo (grados)
	float engranajeAng;       // ángulo engranajes (radianes)
	float movProyectorInc;    // incremento por step (calculado en interpolation)
	float engranajeAngInc;
} FRAME_PROY;

FRAME_PROY KeyFrameProy[MAX_FRAMES_PROY];
int FrameIndexProy = 7;    // número de keyframes definidos (KF 0..6)

// Estado de la animación
enum ProyState { PROY_OFF, PROY_TURNING_ON, PROY_ON, PROY_TURNING_OFF };
ProyState proyState    = PROY_OFF;
int  playIndexProy     = 0;
int  i_curr_steps_proy = 0;
const int I_MAX_STEPS_PROY = 90;
float movProyector = 0.0f;   // variable interpolada — reemplaza getobjetoMovAdicional()

void interpolationProy(int idx, int nextIdx) {
	KeyFrameProy[idx].movProyectorInc =
		(KeyFrameProy[nextIdx].movProyector - KeyFrameProy[idx].movProyector) / I_MAX_STEPS_PROY;
	KeyFrameProy[idx].engranajeAngInc =
		(KeyFrameProy[nextIdx].engranajeAng - KeyFrameProy[idx].engranajeAng) / I_MAX_STEPS_PROY;
}

void animateProy() {
	if (proyState == PROY_OFF || proyState == PROY_ON) return;

	if (i_curr_steps_proy >= I_MAX_STEPS_PROY) {
		// Avanzar al siguiente keyframe
		if (proyState == PROY_TURNING_ON) {
			playIndexProy++;
			if (playIndexProy >= FrameIndexProy - 1) {
				// Llegamos al último KF: proyector encendido y quieto
				proyState = PROY_ON;
				movProyector   = KeyFrameProy[FrameIndexProy - 1].movProyector;
				engranajeAngle = KeyFrameProy[FrameIndexProy - 1].engranajeAng;
				printf("Proyector: ENCENDIDO. Presiona P para apagar.\n");
				return;
			}
		} else { // PROY_TURNING_OFF
			playIndexProy--;
			if (playIndexProy < 0) {
				// Llegamos al KF0: proyector apagado
				proyState = PROY_OFF;
				playIndexProy  = 0;
				movProyector   = KeyFrameProy[0].movProyector;
				engranajeAngle = KeyFrameProy[0].engranajeAng;
				printf("Proyector: APAGADO. Presiona P para encender.\n");
				return;
			}
		}
		i_curr_steps_proy = 0;
		if (proyState == PROY_TURNING_ON)
			interpolationProy(playIndexProy, playIndexProy + 1);
		else
			interpolationProy(playIndexProy, playIndexProy - 1);
	} else {
		movProyector   += KeyFrameProy[playIndexProy].movProyectorInc;
		engranajeAngle += KeyFrameProy[playIndexProy].engranajeAngInc;
		i_curr_steps_proy++;
	}
}
// ─────────────────────────────────────────────────────────────────────────────

// Fuego y Aeolipile
bool fireActive = false;
bool fKeyWasPressed = false;
float fireStartWallTime = 0.0f;
float aeolipileAngle = 0.0f;
float fireScaleAnim = 1.0f;
float fireVibAngle = 0.0f;
float lastNumeroChangeTime = 0.0f;
bool fireStopping = false;
float fireStopWallTime = 0.0f;
const float SPIN_DECEL_SECS = 2.5f;

// Humo
float smokeIntensity = 0.0f;
float smokeStopWallTime = 0.0f;
bool smokeFading = false;
const float SMOKE_RISE_SECS = 3.0f;
const float SMOKE_FADE_SECS = 5.0f;

// Dimensiones reales del modelo brazo.obj
const float ARM_BOTTOM = 2.964f;
const float ARM_TOP    = 11.99f;
const float ARM_LEN    = ARM_TOP - ARM_BOTTOM;

// Catapulta animacion y esfera
enum CatapultState { CAT_IDLE, CAT_LAUNCHING, CAT_FLIGHT, CAT_RESET };
CatapultState catapultState = CAT_IDLE;
float brazoCatapultaAngle = 0.0f;
float launchStartTime = 0.0f;
const float LAUNCH_DURATION = 0.7f;
const float RELEASE_ANGLE = 90.0f;
bool sphereLaunched = false;
glm::vec3 spherePos(10.0f, 11.0f, -5.0f);
glm::vec3 sphereVel(0.0f, 0.0f, 0.0f);
int bounceCount = 0;
bool sphereVisible = true;
float resetTimer = 0.0f;
const float GRAVITY_CONST = 12.0f;
const float BOUNCE_DAMPING = 0.55f;
const int MAX_BOUNCES = 3;
const float FLOOR_Y = -2.0f;
const float SPHERE_RADIUS = 1.2f;
float lastPhysicsTime = 0.0f;
glm::vec3 holePos(17.0f, FLOOR_Y + 0.01f, -5.0f);
bool holeVisible = false;

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
Texture FuegoTexture;
Texture HumoTexture;
Texture NumerosTexture;
Texture Numero1Texture;
Texture Numero2Texture;



Model Kitt_M;
Model Llanta_M;
Model Pista_M;
Model Nave_M;
Model Ala_M;
Model Aeolipile_base_M;
Model Aeolipile_M;

Model ProyectorSteampunk_M;
Model Catapulta_M;
Model BrazoCatapulta_M;
Model EsferaMetalica_M;
Model Engranaje1_M;
Model Engranaje2_M;
Model Engranaje3_M;
Model EngranajeAdicional1_M;
Model EngranajeAdicional2_M;
Model EngranajeAtras1_M;
Model EngranajeAtras2_M;
Model EngranajeProyector1_M;
Model EngranajeProyector2_M;

Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;


//Sphere cabeza = Sphere(0.5, 20, 20);;
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
	meshList.push_back(obj6);

	Mesh* obj7 = new Mesh();
	obj7->CreateMesh(numeroVertices, numeroIndices, 32, 6);
	meshList.push_back(obj7);

	// Fuego billboard vertical (meshList[7])
	unsigned int fuegoIndices[] = {
		0, 1, 2,
		0, 2, 3,
	};
	GLfloat fuegoVertices[] = {
		-0.5f, -0.5f, 0.0f,	0.0f, 0.0f,		0.0f, 0.0f, -1.0f,
		 0.5f, -0.5f, 0.0f,	1.0f, 0.0f,		0.0f, 0.0f, -1.0f,
		 0.5f,  0.5f, 0.0f,	1.0f, 1.0f,		0.0f, 0.0f, -1.0f,
		-0.5f,  0.5f, 0.0f,	0.0f, 1.0f,		0.0f, 0.0f, -1.0f,
	};
	Mesh* obj8 = new Mesh();
	obj8->CreateMesh(fuegoVertices, fuegoIndices, 32, 6);
	meshList.push_back(obj8);

	// Disco hoyo en suelo (meshList[8]) — 20 segmentos
	{
		const int SEG = 20;
		const float PI2 = 6.28318f;
		std::vector<GLfloat> dv;
		std::vector<unsigned int> di;
		dv.insert(dv.end(), { 0.0f, 0.0f, 0.0f,  0.5f, 0.5f,  0.0f, 1.0f, 0.0f });
		for (int i = 0; i <= SEG; i++) {
			float a = (float)i / SEG * PI2;
			float cx = cosf(a), cz = sinf(a);
			dv.insert(dv.end(), { cx, 0.0f, cz,  (cx+1)*0.5f, (cz+1)*0.5f,  0.0f, 1.0f, 0.0f });
		}
		for (int i = 1; i <= SEG; i++) {
			di.push_back(0); di.push_back(i); di.push_back(i + 1);
		}
		Mesh* disk = new Mesh();
		disk->CreateMesh(dv.data(), di.data(), (unsigned int)dv.size(), (unsigned int)di.size());
		meshList.push_back(disk);
	}

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
	FuegoTexture = Texture("Textures/P9-2/Fuego-humo.png");
	FuegoTexture.LoadTextureA();
	HumoTexture = Texture("Textures/P9-2/Humo.png");
	HumoTexture.LoadTextureA();
	NumerosTexture = Texture("Textures/numerosbase.tga");
	NumerosTexture.LoadTextureA();
	Numero1Texture = Texture("Textures/numero1.tga");
	Numero1Texture.LoadTextureA();
	Numero2Texture = Texture("Textures/numero2.tga");
	Numero2Texture.LoadTextureA();


	Kitt_M = Model();
	Kitt_M.LoadModel("Models/kitt_optimizado.obj");
	Llanta_M = Model();
	Llanta_M.LoadModel("Models/llanta_optimizada.obj");
	Pista_M = Model();
	Pista_M.LoadModel("Models/pista.obj");
	Nave_M = Model();
	Nave_M.LoadModel("Models/nave.obj");
	Ala_M = Model();
	Ala_M.LoadModel("Models/ala.obj");
	Aeolipile_base_M = Model();
	Aeolipile_base_M.LoadModel("Models/Aeolipile/Aeolipile_base.obj");
	Aeolipile_M = Model();
	Aeolipile_M.LoadModel("Models/Aeolipile/Aeolipile.obj");

	ProyectorSteampunk_M = Model();
	ProyectorSteampunk_M.LoadModel("Models/ProyectorSteampunk/ProyectorSteampunk.obj");
	Catapulta_M = Model();
	Catapulta_M.LoadModel("Models/Catapulta/catapulta.obj");
	BrazoCatapulta_M = Model();
	BrazoCatapulta_M.LoadModel("Models/Catapulta/brazo.obj");
	EsferaMetalica_M = Model();
	EsferaMetalica_M.LoadModel("Models/Aeolipile/Aeolipile.obj");
	lastPhysicsTime = (float)glfwGetTime();
	Engranaje1_M = Model();
	Engranaje1_M.LoadModel("Models/ProyectorSteampunk/Engranaje1.obj");
	Engranaje2_M = Model();
	Engranaje2_M.LoadModel("Models/ProyectorSteampunk/Engranaje2.obj");
	Engranaje3_M = Model();
	Engranaje3_M.LoadModel("Models/ProyectorSteampunk/Engranaje3.obj");
	EngranajeAdicional1_M = Model();
	EngranajeAdicional1_M.LoadModel("Models/ProyectorSteampunk/Engranajeadicional1.obj");
	EngranajeAdicional2_M = Model();
	EngranajeAdicional2_M.LoadModel("Models/ProyectorSteampunk/Engranajeadicional2.obj");
	EngranajeAtras1_M = Model();
	EngranajeAtras1_M.LoadModel("Models/ProyectorSteampunk/Engranajeatras1.obj");
	EngranajeAtras2_M = Model();
	EngranajeAtras2_M.LoadModel("Models/ProyectorSteampunk/Engranajeatras2.obj");
	EngranajeProyector1_M = Model();
	EngranajeProyector1_M.LoadModel("Models/ProyectorSteampunk/Engranajeproyector1.obj");
	EngranajeProyector2_M = Model();
	EngranajeProyector2_M.LoadModel("Models/ProyectorSteampunk/Engranajeproyector2.obj");

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
		0.0f, 0.0f, -1.0f);
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

	//luz fija
	spotLights[1] = SpotLight(0.0f, 0.0f, 1.0f,
		1.0f, 2.0f,
		5.0f, 10.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		15.0f);
	spotLightCount++;

	// [2] Haz del proyector steampunk - se activa con F al girar engranajes, apagado al inicio
	spotLights[2] = SpotLight(0.85f, 0.9f, 1.0f,
		0.0f, 0.0f,
		0.0f, 1.5f, 5.0f,
		0.0f, -0.15f, -1.0f,
		1.0f, 0.05f, 0.01f,
		20.0f);
	spotLightCount++;



	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset=0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	
	movCoche = 0.0f;
	movOffset = 0.01f;
	rotllanta = 0.0f;
	rotllantaOffset = 10.0f;

	// ── Keyframes del ProyectorSteampunk ────────────────────────────────────
	//  KF0: apagado, todo quieto
	//  KF1: engranajes arrancan lento
	//  KF2: engranajes aceleran
	//  KF3: velocidad constante → luz prende (engranajeAng >= LUZ_UMBRAL=6.0)
	//  KF4: proyector gira a la derecha
	//  KF5: proyector gira a la izquierda
	//  KF6: regresa al centro, encendido estable
	//  Apagado = recorrer KF6→KF0 en reversa
	KeyFrameProy[0] = {   0.0f,  0.0f, 0,0 };  // apagado
	KeyFrameProy[1] = {   0.0f,  1.0f, 0,0 };  // engranajes arrancan lento
	KeyFrameProy[2] = {   0.0f,  3.0f, 0,0 };  // engranajes aceleran
	KeyFrameProy[3] = {   0.0f,  6.0f, 0,0 };  // velocidad constante, luz prende
	KeyFrameProy[4] = {  45.0f,  7.5f, 0,0 };  // gira derecha
	KeyFrameProy[5] = { -45.0f,  9.0f, 0,0 };  // gira izquierda
	KeyFrameProy[6] = {   0.0f, 10.5f, 0,0 };  // centro, encendido estable
	FrameIndexProy  = 7;

	// Estado inicial: apagado
	proyState          = PROY_OFF;
	playIndexProy      = 0;
	i_curr_steps_proy  = 0;
	movProyector       = KeyFrameProy[0].movProyector;
	engranajeAngle     = KeyFrameProy[0].engranajeAng;
	printf("ProyectorSteampunk: Presiona P para encender.\n");
	// ────────────────────────────────────────────────────────────────────────

	glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	
	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		angulovaria += 0.5f * deltaTime;

		// ── Tecla P: encender / apagar ProyectorSteampunk  ─
		if (mainWindow.getProyectorToggle()) {
			if (proyState == PROY_OFF) {
				proyState         = PROY_TURNING_ON;
				playIndexProy     = 0;
				i_curr_steps_proy = 0;
				movProyector      = KeyFrameProy[0].movProyector;
				engranajeAngle    = KeyFrameProy[0].engranajeAng;
				interpolationProy(0, 1);
				printf("Proyector: encendiendo...\n");
			} else if (proyState == PROY_ON) {
				proyState         = PROY_TURNING_OFF;
				playIndexProy     = FrameIndexProy - 2;
				i_curr_steps_proy = 0;
				interpolationProy(playIndexProy, playIndexProy - 1);
				printf("Proyector: apagando...\n");
			}
		}
		// Animación por keyframes del ProyectorSteampunk
		animateProy();

		// --- Numero cambiante a velocidad visible (cada 0.5s de tiempo real) ---
		{
			float nowReal = (float)glfwGetTime();
			if (nowReal - lastNumeroChangeTime > 0.5f) {
				numeroActual++;
				if (numeroActual > 8)
					numeroActual = 0;

				// Fila 1 (v=0.00): digitos 1-4
				if      (numeroActual == 0) { toffsetnumerocambiau = 0.0f;  toffsetnumerocambiav =  0.0f; }
				else if (numeroActual == 1) { toffsetnumerocambiau = 0.25f; toffsetnumerocambiav =  0.0f; }
				else if (numeroActual == 2) { toffsetnumerocambiau = 0.5f;  toffsetnumerocambiav =  0.0f; }
				else if (numeroActual == 3) { toffsetnumerocambiau = 0.75f; toffsetnumerocambiav =  0.0f; }
				// Fila 2 (v=-0.33): digitos 5-8
				else if (numeroActual == 4) { toffsetnumerocambiau = 0.0f;  toffsetnumerocambiav = -0.33f; }
				else if (numeroActual == 5) { toffsetnumerocambiau = 0.25f; toffsetnumerocambiav = -0.33f; }
				else if (numeroActual == 6) { toffsetnumerocambiau = 0.5f;  toffsetnumerocambiav = -0.33f; }
				else if (numeroActual == 7) { toffsetnumerocambiau = 0.75f; toffsetnumerocambiav = -0.33f; }
				// Fila 3 (v=-0.67): digito 9
				else if (numeroActual == 8) { toffsetnumerocambiau = 0.0f;  toffsetnumerocambiav = -0.67f; }

				lastNumeroChangeTime = nowReal;
			}
		}

		// --- Tecla F: toggle fuego ---
		{
			bool* keys = mainWindow.getsKeys();
			if (keys[GLFW_KEY_F] && !fKeyWasPressed) {
				fireActive = !fireActive;
				if (fireActive) {
					fireStartWallTime = (float)glfwGetTime();
					fireStopping = false;
				} else {
					// Apagar fuego: iniciar deceleration si ya estaba girando
					float elapsed = (float)glfwGetTime() - fireStartWallTime;
					if (elapsed > 5.0f) {
						fireStopping = true;
						fireStopWallTime = (float)glfwGetTime();
					}
				}
				fKeyWasPressed = true;
			}
			if (!keys[GLFW_KEY_F])
				fKeyWasPressed = false;
		}

		// --- Animacion del fuego y giro del Aeolipile (usando tiempo real) ---
		if (fireActive) {
			float elapsed = (float)glfwGetTime() - fireStartWallTime;
			fireScaleAnim = 1.0f + 0.2f * sinf(elapsed * 9.0f);
			fireVibAngle  = 8.0f * sinf(elapsed * 13.0f);
			if (elapsed > 5.0f) {
				aeolipileAngle += 120.0f * (deltaTime / 61.0f);
			}
		}

		// --- Inercia al apagar: desacelera el giro hasta detenerse ---
		if (fireStopping) {
			float stopElapsed = (float)glfwGetTime() - fireStopWallTime;
			float factor = 1.0f - (stopElapsed / SPIN_DECEL_SECS);
			if (factor > 0.0f) {
				aeolipileAngle += 120.0f * (deltaTime / 61.0f) * factor;
			} else {
				fireStopping = false;
			}
		}

		// --- Intensidad del humo ---
		if (fireActive) {
			float elapsed = (float)glfwGetTime() - fireStartWallTime;
			if (elapsed > 5.0f) {
				float smokeElapsed = elapsed - 5.0f;
				smokeIntensity = fminf(1.0f, smokeElapsed / SMOKE_RISE_SECS);
				smokeFading = false;
			}
		} else {
			if (smokeIntensity > 0.0f && !smokeFading) {
				smokeFading = true;
				smokeStopWallTime = (float)glfwGetTime();
			}
			if (smokeFading) {
				float fadeElapsed = (float)glfwGetTime() - smokeStopWallTime;
				smokeIntensity = fmaxf(0.0f, 1.0f - fadeElapsed / SMOKE_FADE_SECS);
				if (smokeIntensity <= 0.0f) smokeFading = false;
			}
		}

		// --- Catapulta y esfera metalica ---
		{
			float nowPhys = (float)glfwGetTime();
			float physDt = fminf(nowPhys - lastPhysicsTime, 0.05f);
			lastPhysicsTime = nowPhys;

			// Trigger: humo lleno → lanzar
			if (catapultState == CAT_IDLE && smokeIntensity >= 1.0f) {
				catapultState = CAT_LAUNCHING;
				launchStartTime = nowPhys;
				sphereLaunched = false;
				sphereVisible = true;
				brazoCatapultaAngle = 0.0f;
			}

			if (catapultState == CAT_LAUNCHING) {
				float elapsed = nowPhys - launchStartTime;
				float t = fminf(elapsed / LAUNCH_DURATION, 1.0f);
				brazoCatapultaAngle = t * 270.0f;

				// Esfera sigue el tope del brazo: formula con dimensiones reales del modelo
				if (!sphereLaunched) {
					// Esfera sigue el tope del brazo (tip local aprox (7,5,0), pivot en origen del modelo)
				float theta = brazoCatapultaAngle * toRadians;
					spherePos.x = 17.0f - 7.0f * cosf(theta) + 8.0f * sinf(theta);
					spherePos.y =         7.0f * sinf(theta) + 8.0f * cosf(theta);
					spherePos.z = -5.0f;
				}

				// Soltar esfera al llegar a RELEASE_ANGLE
				if (!sphereLaunched && brazoCatapultaAngle >= RELEASE_ANGLE) {
					sphereLaunched = true;
					catapultState = CAT_FLIGHT;
					sphereVel = glm::vec3(18.0f, 14.0f, 0.0f);
					bounceCount = 0;
				}
			}

			if (catapultState == CAT_FLIGHT) {
				sphereVel.y -= GRAVITY_CONST * physDt;
				spherePos += sphereVel * physDt;

				// Piso con radio de esfera para que no la atraviese
				if (spherePos.y <= FLOOR_Y + SPHERE_RADIUS) {
					spherePos.y = FLOOR_Y + SPHERE_RADIUS;
					sphereVel.y = -sphereVel.y * BOUNCE_DAMPING;
					sphereVel.x *= 0.88f;
					bounceCount++;

					if (bounceCount >= MAX_BOUNCES) {
						holePos = glm::vec3(spherePos.x, FLOOR_Y + 0.01f, spherePos.z);
						holeVisible = true;
						catapultState = CAT_RESET;
						resetTimer = nowPhys;
						sphereVisible = false;
					}
				}
			}

			if (catapultState == CAT_RESET) {
				if (nowPhys - resetTimer > 2.0f) {
					catapultState = CAT_IDLE;
					brazoCatapultaAngle = 0.0f;
					spherePos = glm::vec3(10.0f, 8.0f, -5.0f);
					sphereVel = glm::vec3(0.0f);
					bounceCount = 0;
					sphereVisible = true;
					sphereLaunched = false;
				}
			}
		}

		if (movCoche < 30.0f)
		{
			movCoche -= movOffset * deltaTime;
			//printf("avanza%f \n ",movCoche);
			rotllanta += rotllantaOffset * deltaTime;
		}



		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

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
		uniformTextureOffset = shaderList[0].getOffsetLocation();

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

		// Haz del proyector steampunk: se enciende cuando engranajeAngle >= LUZ_UMBRAL
		{
			float proyRad = (movProyector - 45.0f) * toRadians;
			glm::vec3 proyDir(-sinf(proyRad), 0.0f, -cosf(proyRad));
			float proyDiff = (engranajeAngle >= LUZ_UMBRAL) ? 2.5f : 0.0f;
			spotLights[2] = SpotLight(0.85f, 0.9f, 1.0f,
				0.0f, proyDiff,
				0.0f, 1.5f, 5.0f,
				proyDir.x, proyDir.y, proyDir.z,
				1.0f, 0.05f, 0.01f,
				20.0f);
		}

		//informaci�n al shader de fuentes de iluminaci�n
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);


		//reiniciar variables antes de que sean enviadas al shader

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



		//Instancia del coche 
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(movCoche - 50.0f, 0.5f, -2.0f));
		modelaux = model;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Kitt_M.RenderModel();

		//Llanta delantera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(7.0f, -0.5f, 8.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		color = glm::vec3(0.5f, 0.5f, 0.5f);//llanta con color gris
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta trasera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(15.5f, -0.5f, 8.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta delantera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(7.0f, -0.5f, 1.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta trasera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(15.5f, -0.5f, 1.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Aqu� va la nave con jerarqu�a de modelos, completar
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 3.0f, 1.5f)); //model,glm::vec3(movnave,sin(movnave)+3.0,1.0f)
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Nave_M.RenderModel();

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Ala_M.RenderModel();



		//AEOLIPILE
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(5.0f, -2.0f, -5.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Aeolipile_base_M.RenderModel();

		// Esfera superior gira cuando el fuego esta activo (5s delay)
		model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
		model = glm::rotate(model, aeolipileAngle * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Aeolipile_M.RenderModel();

		// Catapulta cuerpo
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(17.0f, 0.0f, -5.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Catapulta_M.RenderModel();

		// Brazo - pivote en origen del modelo (0,0,0)
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(17.0f, 0.0f, -5.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, brazoCatapultaAngle * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		BrazoCatapulta_M.RenderModel();

		// Hueco en el piso (quad oscuro donde desaparece la esfera)
		if (holeVisible) {
			color = glm::vec3(0.03f, 0.03f, 0.03f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			toffset = glm::vec2(0.0f, 0.0f);
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			model = glm::mat4(1.0);
			model = glm::translate(model, holePos);
			model = glm::scale(model, glm::vec3(1.2f, 1.0f, 1.2f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plainTexture.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[8]->RenderMesh();
		}

		// Esfera metalica
		if (sphereVisible) {
			color = glm::vec3(0.6f, 0.6f, 0.65f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			model = glm::mat4(1.0);
			model = glm::translate(model, spherePos);
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			EsferaMetalica_M.RenderModel();
		}

		/*A�adir para ejercicio
		1.- Agregar que el n�mero cambiante sea a una velocidad visible.
		Acomodar jerarqu�a del Aelopile
		2.- Giro del Aelopile
		3.-Fuego
			- Editar Textura para el fuego
			- Blending para el fuego
			- Animaci�n del fuego

		A�adir para el reporte
		1.- Humo
			- Editar Textura para el humo
			- Blending para el humo
			- Animaci�n del humo: no sale y sale
		2.- Brazo/palanca que lance la esfera met�lica
			-El humo inicia el movimiento del brazo palanca
			- Animaci�n del brazo
		3.- Esfera met�lica
			- Movimiento de la esfera met�lica por medio de f�sica b�sica: tiro parab�lico y rebote
		*/

		// ProyectorSteampunk
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		toffset = glm::vec2(0.0f, 0.0f);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		// ProyectorSteampunk completo - rota en Y por keyframes (movProyector)
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 5.0f));
		model = glm::rotate(model, movProyector * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		ProyectorSteampunk_M.RenderModel();

		// Engranajes - giran en su propio eje (T_mundo * T(c*s) * R * T(-c*s) * S)
		color = glm::vec3(0.8f, 0.55f, 0.3f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		{
			const glm::vec3 gWorld(0.0f, 0.0f, 5.0f);
			const float gS = 2.0f;
			const glm::vec3 gAxis(0.0f, 0.0f, 1.0f);
			float gObjRad = movProyector * toRadians;

			// centros en model-space (bounding box) * escala
			struct GearEntry { Model* m; glm::vec3 c; };
			GearEntry gears[] = {
				{ &Engranaje1_M,          glm::vec3(-0.2056f,  0.1182f,  0.0577f) },
				{ &Engranaje2_M,          glm::vec3(-0.1093f,  0.1942f,  0.0576f) },
				{ &Engranaje3_M,          glm::vec3(-0.2404f,  0.2781f,  0.0600f) },
				{ &EngranajeAdicional1_M, glm::vec3(-0.3042f,  0.1453f,  0.0546f) },
				{ &EngranajeAdicional2_M, glm::vec3(-0.1319f,  0.1865f, -0.1731f) },
				{ &EngranajeAtras1_M,     glm::vec3(-0.2475f,  0.2781f, -0.0312f) },
				{ &EngranajeAtras2_M,     glm::vec3(-0.3565f,  0.2222f, -0.0363f) },
				{ &EngranajeProyector1_M, glm::vec3(-0.6199f,  0.5783f,  0.0615f) },
				{ &EngranajeProyector2_M, glm::vec3(-0.5595f,  0.0187f,  0.0530f) },
			};
			for (auto& g : gears) {
				glm::vec3 cS = g.c * gS;   // centro en espacio escalado
				model = glm::mat4(1.0f);
				model = glm::translate(model, gWorld);
				// rotacion del cuerpo del proyector (tecla J/L) - engranajes siguen al cuerpo
				model = glm::rotate(model, gObjRad, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::translate(model, cS);
				model = glm::rotate(model, engranajeAngle, gAxis);
				model = glm::translate(model, -cS);
				model = glm::scale(model, glm::vec3(gS));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				g.m->RenderModel();
			}
		}

		//Modelos con blending al final para que no afecten a los dem�s objetos, aunque tambi�n se pueden renderizar al inicio pero con blending  activado y desactivado


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

		//textura con movimiento
		//Importantes porque la variable uniform no podemos modificarla directamente
		toffsetflechau += 0.001;
		toffsetflechav = 0.000;
		//para que no se desborde la variable
		if (toffsetflechau > 1.0)
			toffsetflechau = 0.0;
		//if (toffsetv > 1.0)
		//	toffsetv = 0;
		//printf("\ntfosset %f \n", toffsetu);
		//pasar a la variable uniform el valor actualizado
		toffset = glm::vec2(toffsetflechau, toffsetflechav);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		FlechaTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();

		//plano con todos los n�meros
		toffsetnumerou = 0.0;
		toffsetnumerov = 0.0;
		toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-6.0f, 2.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[5]->RenderMesh();

		//n�mero 1
		//toffsetnumerou = 0.0;
		//toffsetnumerov = 0.0;
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-10.0f, 2.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		//glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[6]->RenderMesh();

		for (int i = 1; i < 4; i++)
		{
			//n�meros 2-4
			toffsetnumerou += 0.25;
			toffsetnumerov = 0.0;
			toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-10.0f - (i * 3.0), 2.0f, -6.0f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			NumerosTexture.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[6]->RenderMesh();

		}

		for (int j = 1; j < 5; j++)
		{
			//n�meros 5-8
			toffsetnumerou += 0.25;
			toffsetnumerov = -0.33;
			toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-7.0f - (j * 3.0), 5.0f, -6.0f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			NumerosTexture.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[6]->RenderMesh();
		}


		//n�mero cambiante 
		/*
		�C�mo hacer para que sea a una velocidad visible?
		*/
		// toffsetnumerocambiau/v ya se actualizan en la seccion de update
		toffset = glm::vec2(toffsetnumerocambiau, toffsetnumerocambiav);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-10.0f, 10.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[6]->RenderMesh();

		//cambiar autom�ticamente entre textura n�mero 1 y n�mero 2
		toffsetnumerou = 0.0;
		toffsetnumerov = 0.0;
		toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-13.0f, 10.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Numero1Texture.UseTexture();
		//if
		//Numero1Texture.UseTexture();
		//Numero2Texture.UseTexture();

		// --- FUEGO: billboard animado cerca de la base del Aeolipile
		if (fireActive) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			float fuegoScale = 1.5f * fireScaleAnim;
			model = glm::mat4(1.0f);
			// Posicion: base del Aeolipile (5, -2, -5) + un poco arriba
			model = glm::translate(model, glm::vec3(5.0f, -0.5f, -5.0f));
			// Pequeño giro de vibración
			model = glm::rotate(model, fireVibAngle * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, glm::vec3(fuegoScale, fuegoScale * 1.4f, fuegoScale));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			toffset = glm::vec2(0.0f, 0.0f);
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			FuegoTexture.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[7]->RenderMesh();

			glDisable(GL_BLEND);
		}

		// --- HUMO: 4 billboards subiendo desde el fuego hasta la parte baja del Aeolipile ---
		if (smokeIntensity > 0.0f) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			toffset = glm::vec2(0.0f, 0.0f);
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			HumoTexture.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

			float humoTime = (float)glfwGetTime();
			const int NUM_HUMO = 4;
			const float humoStartY = -0.5f;  // altura del fuego
			const float humoEndY   =  3.0f;  // parte baja de la esfera del Aeolipile

			for (int i = 0; i < NUM_HUMO; i++) {
				float phase = (float)i / NUM_HUMO;
				float t = fmodf(humoTime * 0.2f + phase, 1.0f);  // 0→1 ciclico, velocidad lenta
				float y = humoStartY + t * (humoEndY - humoStartY);
				float s = (0.4f + t * 1.4f) * smokeIntensity;    // crece conforme sube
				if (s < 0.01f) continue;

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(5.0f, y, -5.0f));
				model = glm::scale(model, glm::vec3(s, s * 1.1f, s));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				meshList[7]->RenderMesh();
			}
			glDisable(GL_BLEND);
		}

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}
