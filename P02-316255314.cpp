//Prctica 2: indices, mesh, proyecciones, transformaciones geomtricas
#include <stdio.h>
#include <string.h>
#include<cmath>
#include<vector>
#include <glew.h>
#include <glfw3.h>
//glm
#include<glm.hpp>
#include<gtc\matrix_transform.hpp>
#include<gtc\type_ptr.hpp>
//clases para dar orden y limpieza al codigo
#include"Mesh.h"
#include"Shader.h"
#include"Window.h"
//Dimensiones de la ventana
const float toRadians = 3.14159265f/180.0; //grados a radianes
Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<MeshColor*> meshColorList;
std::vector<Shader>shaderList;
//Vertex Shader
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
static const char* vShaderColor = "shaders/shadercolor.vert";
static const char* fShaderColor = "shaders/shadercolor.frag";
//shaders nuevos se crearan aqui
// Vertex para colores solidos (puede ser shader.vert si ya tiene model/projection)
// Fragment por color
static const char* fRojo        = "shaders/rojo.frag";
static const char* fVerde       = "shaders/verde.frag";
static const char* fAzul        = "shaders/azul.frag";
static const char* fCafe        = "shaders/cafe.frag";
static const char* fVerdeOscuro = "shaders/verde_oscuro.frag";

float angulo = 0.0f;

//color cafe en RGB : 0.478, 0.255, 0.067

//Piramide triangular regular
void CreaPiramide()
{
	unsigned int indices[] = { 
		0,1,2,
		1,3,2,
		3,0,2,
		1,0,3
		
	};
	GLfloat vertices[] = {
		-0.5f, -0.5f,0.0f,	//0
		0.5f,-0.5f,0.0f,	//1
		0.0f,0.5f, -0.25f,	//2
		0.0f,-0.5f,-0.5f,	//3

	};
	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 12, 12);
	meshList.push_back(obj1);
}

//Vertices de un cubo
void CrearCubo()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};

GLfloat cubo_vertices[] = {
	// front
	-0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	// back
	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f
};
Mesh *cubo = new Mesh();
cubo->CreateMesh(cubo_vertices, cubo_indices,24, 36);
meshList.push_back(cubo);
}

void CrearInicialesCMG()
{
	// C
	GLfloat vertices_C[] = {
		-0.8f,0.4f,0.0f, -0.6f,0.4f,0.0f, -0.6f,0.5f,0.0f,
		-0.9f,0.8f,0.0f, -0.9f,0.4f,0.0f, -0.8f,0.4f,0.0f,
		-0.9f,0.8f,0.0f, -0.8f,0.8f,0.0f, -0.8f,0.4f,0.0f,
		-0.8f,0.8f,0.0f, -0.6f,0.8f,0.0f, -0.6f,0.7f,0.0f
	};
	unsigned int indices_C[] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
	Mesh* letraC = new Mesh();
	letraC->CreateMesh(vertices_C, indices_C, 36, 12);
	meshList.push_back(letraC);

	// M
	GLfloat vertices_M[] = {
		-0.2f,0.3f,0.0f, -0.2f,-0.2f,0.0f, -0.1f,-0.2f,0.0f,
		-0.2f,0.3f,0.0f, -0.1f,0.3f,0.0f, -0.1f,-0.2f,0.0f,
		 0.1f,-0.2f,0.0f, 0.2f,-0.2f,0.0f, 0.1f,0.3f,0.0f,
		 0.1f,0.3f,0.0f, 0.2f,0.3f,0.0f, 0.2f,-0.2f,0.0f,
		-0.1f,0.3f,0.0f, 0.0f,0.05f,0.0f, 0.1f,0.3f,0.0f
	};
	unsigned int indices_M[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14 };
	Mesh* letraM = new Mesh();
	letraM->CreateMesh(vertices_M, indices_M, 45, 15);
	meshList.push_back(letraM);

	// G
	GLfloat vertices_G[] = {
		0.4f,-0.1f,0.0f, 0.4f,-0.6f,0.0f, 0.5f,-0.6f,0.0f,
		0.4f,-0.1f,0.0f, 0.5f,-0.1f,0.0f, 0.5f,-0.6f,0.0f,
		0.5f,-0.6f,0.0f, 0.7f,-0.6f,0.0f, 0.7f,-0.5f,0.0f,
		0.55f,-0.35f,0.0f, 0.7f,-0.35f,0.0f, 0.7f,-0.45f,0.0f,
		0.5f,-0.1f,0.0f, 0.7f,-0.1f,0.0f, 0.7f,-0.2f,0.0f
	};
	unsigned int indices_G[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14 };
	Mesh* letraG = new Mesh();
	letraG->CreateMesh(vertices_G, indices_G, 45, 15);
	meshList.push_back(letraG);
}


//Aqui para mayor facilidad y seguimiento, hacemos una funcion.
void CreaCasita() {

// 1) Cuerpo (rectangulo rojo)
GLfloat casa_cuerpo[] = {
	//X		Y		Z		R		G		B
	-0.55f, -0.80f, 0.0f,	1.0f,	0.0f,	0.0f,
	 0.55f, -0.80f, 0.0f,	1.0f,	0.0f,	0.0f,
	 0.55f,  0.30f, 0.0f,	1.0f,	0.0f,	0.0f,

	-0.55f, -0.80f, 0.0f,	1.0f,	0.0f,	0.0f,
	 0.55f,  0.30f, 0.0f,	1.0f,	0.0f,	0.0f,
	-0.55f,  0.30f, 0.0f,	1.0f,	0.0f,	0.0f,
};
MeshColor* cuerpo = new MeshColor();
cuerpo->CreateMeshColor(casa_cuerpo, 36);
meshColorList.push_back(cuerpo);

// 2) Techo (tringulo azul)
GLfloat casa_techo[] = {
	//X		Y		Z		R		G		B
	-0.65f,  0.30f, 0.0f,	0.0f,	0.0f,	1.0f,
	 0.65f,  0.30f, 0.0f,	0.0f,	0.0f,	1.0f,
	 0.00f,  0.90f, 0.0f,	0.0f,	0.0f,	1.0f,
};
MeshColor* techo = new MeshColor();
techo->CreateMeshColor(casa_techo, 18);
meshColorList.push_back(techo);

// 3) Ventana izquierda (verde)
GLfloat ventana_izq[] = {
	//X		Y		Z		R		G		B
	-0.40f, -0.05f, 0.1f,	0.0f,	1.0f,	0.0f,
	-0.15f, -0.05f, 0.1f,	0.0f,	1.0f,	0.0f,
	-0.15f,  0.20f, 0.1f,	0.0f,	1.0f,	0.0f,

	-0.40f, -0.05f, 0.1f,	0.0f,	1.0f,	0.0f,
	-0.15f,  0.20f, 0.1f,	0.0f,	1.0f,	0.0f,
	-0.40f,  0.20f, 0.1f,	0.0f,	1.0f,	0.0f,
};
MeshColor* winL = new MeshColor();
winL->CreateMeshColor(ventana_izq, 36);
meshColorList.push_back(winL);

// 4) Ventana derecha (verde)
GLfloat ventana_der[] = {
	//X		Y		Z		R		G		B
	 0.15f, -0.05f, 0.01f,	0.0f,	1.0f,	0.0f,
	 0.40f, -0.05f, 0.01f,	0.0f,	1.0f,	0.0f,
	 0.40f,  0.20f, 0.01f,	0.0f,	1.0f,	0.0f,

	 0.15f, -0.05f, 0.01f,	0.0f,	1.0f,	0.0f,
	 0.40f,  0.20f, 0.01f,	0.0f,	1.0f,	0.0f,
	 0.15f,  0.20f, 0.01f,	0.0f,	1.0f,	0.0f,
};
MeshColor* winR = new MeshColor();
winR->CreateMeshColor(ventana_der, 36);
meshColorList.push_back(winR);

// 5) Puerta (verde)
GLfloat puerta[] = {
	//X		Y		Z		R		G		B
	-0.12f, -0.80f, 0.01f,	0.0f,	1.0f,	0.0f,
	 0.12f, -0.80f, 0.01f,	0.0f,	1.0f,	0.0f,
	 0.12f, -0.40f, 0.01f,	0.0f,	1.0f,	0.0f,

	-0.12f, -0.80f, 0.01f,	0.0f,	1.0f,	0.0f,
	 0.12f, -0.40f, 0.01f,	0.0f,	1.0f,	0.0f,
	-0.12f, -0.40f, 0.01f,	0.0f,	1.0f,	0.0f,
};
MeshColor* door = new MeshColor();
door->CreateMeshColor(puerta, 36);
meshColorList.push_back(door);

// 6) arbol izquierdo - tronco (cafe)
GLfloat tronco_L[] = {
	//X		Y		Z		R		G		B
	-0.90f, -0.80f, 0.0f,	0.478f,	0.255f,	0.067f,
	-0.80f, -0.80f, 0.0f,	0.478f,	0.255f,	0.067f,
	-0.80f, -0.60f, 0.0f,	0.478f,	0.255f,	0.067f,

	-0.90f, -0.80f, 0.0f,	0.478f,	0.255f,	0.067f,
	-0.80f, -0.60f, 0.0f,	0.478f,	0.255f,	0.067f,
	-0.90f, -0.60f, 0.0f,	0.478f,	0.255f,	0.067f,
};
MeshColor* trunkL = new MeshColor();
trunkL->CreateMeshColor(tronco_L, 36);
meshColorList.push_back(trunkL);

// 7) arbol izquierdo - copa (verde oscuro)
GLfloat copa_L[] = {
	//X		Y		Z		R		G		B
	-0.98f, -0.60f, 0.0f,	0.0f,	0.5f,	0.0f,
	-0.72f, -0.60f, 0.0f,	0.0f,	0.5f,	0.0f,
	-0.85f, -0.25f, 0.0f,	0.0f,	0.5f,	0.0f,
};
MeshColor* leavesL = new MeshColor();
leavesL->CreateMeshColor(copa_L, 18);
meshColorList.push_back(leavesL);

// 8) arbol derecho - tronco (cafe)
GLfloat tronco_R[] = {
	//X		Y		Z		R		G		B
	 0.80f, -0.80f, 0.0f,	0.478f,	0.255f,	0.067f,
	 0.90f, -0.80f, 0.0f,	0.478f,	0.255f,	0.067f,
	 0.90f, -0.60f, 0.0f,	0.478f,	0.255f,	0.067f,

	 0.80f, -0.80f, 0.0f,	0.478f,	0.255f,	0.067f,
	 0.90f, -0.60f, 0.0f,	0.478f,	0.255f,	0.067f,
	 0.80f, -0.60f, 0.0f,	0.478f,	0.255f,	0.067f,
};
MeshColor* trunkR = new MeshColor();
trunkR->CreateMeshColor(tronco_R, 36);
meshColorList.push_back(trunkR);

// 9) arbol derecho - copa (verde oscuro)
GLfloat copa_R[] = {
	//X		Y		Z		R		G		B
	 0.72f, -0.60f, 0.0f,	0.0f,	0.5f,	0.0f,
	 0.98f, -0.60f, 0.0f,	0.0f,	0.5f,	0.0f,
	 0.85f, -0.25f, 0.0f,	0.0f,	0.5f,	0.0f,
};
MeshColor* leavesR = new MeshColor();
leavesR->CreateMeshColor(copa_R, 18);
meshColorList.push_back(leavesR);
}


// Las letras y figuras
void CrearLetrasyFiguras()
{

	GLfloat vertices_letras[] = {
		-1.0f,-1.0f,0.5f,  0.0f,0.0f,1.0f,
		 1.0f,-1.0f,0.5f,  0.0f,0.0f,1.0f,
		 1.0f, 1.0f,0.5f,  0.0f,0.0f,1.0f,
	};

	MeshColor* letras = new MeshColor();
	letras->CreateMeshColor(vertices_letras, 18);
	meshColorList.push_back(letras);

	// Tringulo rojo (base)
	GLfloat vertices_triangulorojo[] = {
		-1.0f,-1.0f,0.5f,  1.0f,0.0f,0.0f,
		 1.0f,-1.0f,0.5f,  1.0f,0.0f,0.0f,
		 0.0f, 1.0f,0.5f,  1.0f,0.0f,0.0f,
	};

	MeshColor* triangulorojo = new MeshColor();
	triangulorojo->CreateMeshColor(vertices_triangulorojo, 18);
	meshColorList.push_back(triangulorojo);

	// Cuadrado verde (base)
	GLfloat vertices_cuadradoverde[] = {
		-0.5f,-0.5f,0.5f,  0.0f,1.0f,0.0f,
		 0.5f,-0.5f,0.5f,  0.0f,1.0f,0.0f,
		 0.5f, 0.5f,0.5f,  0.0f,1.0f,0.0f,
		-0.5f,-0.5f,0.5f,  0.0f,1.0f,0.0f,
		 0.5f, 0.5f,0.5f,  0.0f,1.0f,0.0f,
		-0.5f, 0.5f,0.5f,  0.0f,1.0f,0.0f,
	};

	MeshColor* cuadradoverde = new MeshColor();
	cuadradoverde->CreateMeshColor(vertices_cuadradoverde, 36);
	meshColorList.push_back(cuadradoverde);


	// Tringulo azul (mismo que el rojo, pero azul)
	GLfloat vertices_trianguloazul[] = {
		-1.0f,-1.0f,0.5f,  0.0f,0.0f,1.0f,
		 1.0f,-1.0f,0.5f,  0.0f,0.0f,1.0f,
		 0.0f, 1.0f,0.5f,  0.0f,0.0f,1.0f,
	};

	MeshColor* trianguloazul = new MeshColor();
	trianguloazul->CreateMeshColor(vertices_trianguloazul, 18);
	meshColorList.push_back(trianguloazul);

	// Tringulo verde (mismo tringulo pero subido 0.5 en Y)
	GLfloat vertices_trianguloverde[] = {
		-1.0f,-0.5f,0.5f,  0.0f,1.0f,0.0f,
		 1.0f,-0.5f,0.5f,  0.0f,1.0f,0.0f,
		 0.0f, 1.5f,0.5f,  0.0f,1.0f,0.0f,
	};

	MeshColor* trianguloverde = new MeshColor();
	trianguloverde->CreateMeshColor(vertices_trianguloverde, 18);
	meshColorList.push_back(trianguloverde);

	// Cuadrado rojo (mismo que el verde, solo cambia el color)
	GLfloat vertices_cuadradorojo[] = {
		-0.5f,-0.5f,0.5f,  1.0f,0.0f,0.0f,
		 0.5f,-0.5f,0.5f,  1.0f,0.0f,0.0f,
		 0.5f, 0.5f,0.5f,  1.0f,0.0f,0.0f,
		-0.5f,-0.5f,0.5f,  1.0f,0.0f,0.0f,
		 0.5f, 0.5f,0.5f,  1.0f,0.0f,0.0f,
		-0.5f, 0.5f,0.5f,  1.0f,0.0f,0.0f,
	};

	MeshColor* cuadradorojo = new MeshColor();
	cuadradorojo->CreateMeshColor(vertices_cuadradorojo, 36);
	meshColorList.push_back(cuadradorojo);

	// Cuadrado caf (mismo que el verde con RGB dado)
	GLfloat vertices_cuadradocafe[] = {
		-0.5f,-0.5f,0.5f,  0.478f,0.255f,0.067f,
		 0.5f,-0.5f,0.5f,  0.478f,0.255f,0.067f,
		 0.5f, 0.5f,0.5f,  0.478f,0.255f,0.067f,
		-0.5f,-0.5f,0.5f,  0.478f,0.255f,0.067f,
		 0.5f, 0.5f,0.5f,  0.478f,0.255f,0.067f,
		-0.5f, 0.5f,0.5f,  0.478f,0.255f,0.067f,
	};

	MeshColor* cuadradocafe = new MeshColor();
	cuadradocafe->CreateMeshColor(vertices_cuadradocafe, 36);
	meshColorList.push_back(cuadradocafe);
}




void CreateShaders()
{

	Shader *shader1 = new Shader(); //shader para usar indices: objetos: cubo y  pirmide
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	Shader *shader2 = new Shader();//shader para usar color como parte del VAO: letras 
	shader2->CreateFromFiles(vShaderColor, fShaderColor);
	shaderList.push_back(*shader2);

//shaders nuevos para la casita
Shader* shRojo = new Shader();
shRojo->CreateFromFiles(vShader, fRojo);
shaderList.push_back(*shRojo);

Shader* shVerde = new Shader();
shVerde->CreateFromFiles(vShader, fVerde);
shaderList.push_back(*shVerde);

Shader* shAzul = new Shader();
shAzul->CreateFromFiles(vShader, fAzul);
shaderList.push_back(*shAzul);

Shader* shCafe = new Shader();
shCafe->CreateFromFiles(vShader, fCafe);
shaderList.push_back(*shCafe);

Shader* shVerdeOsc = new Shader();
shVerdeOsc->CreateFromFiles(vShader, fVerdeOscuro);
shaderList.push_back(*shVerdeOsc);


}


int main()
{
	mainWindow = Window(800, 600);
	mainWindow.Initialise();
	CreaPiramide(); //indice 0 en MeshList
	CrearCubo();//indice 1 en MeshList
	CrearInicialesCMG(); //indices 2,3,4 en MeshList
	//CrearLetrasyFiguras(); //usa MeshColor, indices en MeshColorList
	//CreaCasita();
	CreateShaders();
	GLuint uniformProjection = 0;
	GLuint uniformModel = 0;
	//Projection: Matriz de Dimension 4x4 para indicar si vemos en 2D( orthogonal) o en 3D) perspectiva
	//glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
	glm::mat4 projection = glm::perspective(glm::radians(60.0f)	,mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);
	
	//Model: Matriz de Dimensin 4x4 en la cual se almacena la multiplicacin de las transformaciones geomtricas.
	glm::mat4 model(1.0); //fuera del while se usa para inicializar la matriz con una identidad
	
	//Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		//Recibir eventos del usuario
		glfwPollEvents();
		//Limpiar la ventana
		glClearColor(1.0f,1.0f,1.0f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Se agrega limpiar el buffer de profundidad
		
													
		//Para las letras hay que usar el segundo set de shaders con indice 1 en ShaderList 
		//shaderList[1].useShader();
		//uniformModel = shaderList[1].getModelLocation();
		//uniformProjection = shaderList[1].getProjectLocation();
		
		//Inicializar matriz de dimension 4x4 que servir como matriz de modelo para almacenar las transformaciones geomtricas
		//model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, -4.0f));
		//
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));//FALSE ES PARA QUE NO SEA TRANSPUESTA y se envan al shader como variables de tipo uniform
		//glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		//meshColorList[0]->RenderMeshColor();

		// ------ EJERCICIOS DE LA PRACTICA

		//Para el cubo y la piramide se usa el primer set de shaders con indice 0 en ShaderList
		//shaderList[0].useShader(); 
		//uniformModel = shaderList[0].getModelLocation();
		//uniformProjection = shaderList[0].getProjectLocation();
		//angulo += 0.01;
		//Inicializar matriz de dimensin 4x4 que servir como matriz de modelo para almacenar las transformaciones geomtricas
		//model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
		//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));//FALSE ES PARA QUE NO SEA TRANSPUESTA
		//glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		//meshList[1]->RenderMesh();

		//meshColorList[3]->RenderMeshColor(); // tringulo azul
		//meshColorList[4]->RenderMeshColor(); // tringulo verde
		//meshColorList[5]->RenderMeshColor(); // cuadrado rojo
		//meshColorList[2]->RenderMeshColor(); // cuadrado verde
		//meshColorList[6]->RenderMeshColor(); // cuadrado caf

		// Casita (empieza en el indice 7)
		//meshColorList[7]->RenderMeshColor();  // cuerpo
		//meshColorList[8]->RenderMeshColor();  // techo
		//meshColorList[9]->RenderMeshColor();  // ventana izq
		//meshColorList[10]->RenderMeshColor(); // ventana der
		//meshColorList[11]->RenderMeshColor(); // puerta
		//meshColorList[12]->RenderMeshColor(); // tronco izq
		//meshColorList[13]->RenderMeshColor(); // copa izq
		//meshColorList[14]->RenderMeshColor(); // tronco der
		//meshColorList[15]->RenderMeshColor(); // copa der


		// DESARROLLO DE LA PRCTICA 2

//indices de shaders para la casita
const int SH_ROJO = 2;
const int SH_VERDE = 3;
const int SH_AZUL = 4;
const int SH_CAFE = 5;
const int SH_VERDE_OSCURO = 6;

//Iniciales en diagonal de abajo hacia arriba
//C (rojo) abajo
shaderList[SH_ROJO].useShader();
uniformModel = shaderList[SH_ROJO].getModelLocation();
uniformProjection = shaderList[SH_ROJO].getProjectLocation();
model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(0.0f, -1.10f, -4.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
meshList[2]->RenderMesh();

//M (verde) al centro
shaderList[SH_VERDE].useShader();
uniformModel = shaderList[SH_VERDE].getModelLocation();
uniformProjection = shaderList[SH_VERDE].getProjectLocation();
model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(0.0f, -0.20f, -4.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
meshList[3]->RenderMesh();

//G (azul) arriba
shaderList[SH_AZUL].useShader();
uniformModel = shaderList[SH_AZUL].getModelLocation();
uniformProjection = shaderList[SH_AZUL].getProjectLocation();
model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(0.0f, 0.70f, -4.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
meshList[4]->RenderMesh();

//No se borra la casita ni se comenta, solo se desactiva.
if (false)
{
// Para que todo quede alineado
float zCasa = -4.0f;
float yPisoCasa = -0.80f;

//variable de ir cambiando el ngulo
angulo += 0.01;

// muros
float sxC = 1.30f, syC = 1.00f, szC = 1.00f;
float yCentroCuerpo = yPisoCasa + 0.5f * syC;   // queda apoyado en yPisoCasa

shaderList[SH_ROJO].useShader();
uniformModel = shaderList[SH_ROJO].getModelLocation();
uniformProjection = shaderList[SH_ROJO].getProjectLocation();

model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(0.0f, yCentroCuerpo, zCasa));
//model = glm::rotate(model,glm::radians(angulo), glm::vec3(0.0f, 1.0f, 0.0f));
model = glm::scale(model, glm::vec3(sxC, syC, szC));

glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
meshList[1]->RenderMesh(); // cubo = cuerpo


//techo
float sxT = 1.60f, syT = 0.80f, szT = 1.20f;
float yTechoCuerpo = yPisoCasa + syC;
float yCentroTecho = yTechoCuerpo + 0.5f * syT; // lo apoyo sobre el techo del cuerpo

shaderList[SH_AZUL].useShader();
uniformModel = shaderList[SH_AZUL].getModelLocation();
uniformProjection = shaderList[SH_AZUL].getProjectLocation();

model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(0.0f, yCentroTecho, zCasa));
model = glm::scale(model, glm::vec3(sxT, syT, szT));

glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
meshList[0]->RenderMesh(); // pirmide = techo


// puerta
float sxP = 0.35f, syP = 0.55f, szP = 0.20f;
float yCentroPuerta = yPisoCasa + 0.5f * syP;

shaderList[SH_VERDE].useShader();
uniformModel = shaderList[SH_VERDE].getModelLocation();
uniformProjection = shaderList[SH_VERDE].getProjectLocation();

model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(0.0f, yCentroPuerta, zCasa + 0.65f)); // frente del muro
model = glm::scale(model, glm::vec3(sxP, syP, szP));

glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
meshList[1]->RenderMesh(); // cubo = puerta


// ventanas
float sxW = 0.35f, syW = 0.35f, szW = 0.20f;
// Las ventanas las pongo ms arriba que la puerta, pero dentro del cuerpo
float yCentroVentana = yPisoCasa + 0.65f; // ajustable a gusto

// ventana izq
model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(-0.45f, yCentroVentana, zCasa + 0.65f));
model = glm::scale(model, glm::vec3(sxW, syW, szW));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
meshList[1]->RenderMesh();

// ventana der
model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(0.45f, yCentroVentana, zCasa + 0.65f));
model = glm::scale(model, glm::vec3(sxW, syW, szW));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
meshList[1]->RenderMesh();


// arbolitos
// Troncos caf
float sxTr = 0.20f, syTr = 0.55f, szTr = 0.20f;
float yCentroTronco = yPisoCasa + 0.5f * syTr;

// Copas verde oscuro (pirmide)
float sCopa = 0.55f;
float yTechoTronco = yPisoCasa + syTr;
float yCentroCopa = yTechoTronco + 0.5f * sCopa;

// tronco izq
shaderList[SH_CAFE].useShader();
uniformModel = shaderList[SH_CAFE].getModelLocation();
uniformProjection = shaderList[SH_CAFE].getProjectLocation();

model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(-0.82f, yCentroTronco, zCasa));
model = glm::scale(model, glm::vec3(sxTr, syTr, szTr));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
meshList[1]->RenderMesh();

// copa izq
shaderList[SH_VERDE_OSCURO].useShader();
uniformModel = shaderList[SH_VERDE_OSCURO].getModelLocation();
uniformProjection = shaderList[SH_VERDE_OSCURO].getProjectLocation();

model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(-0.82f, yCentroCopa, zCasa));
model = glm::scale(model, glm::vec3(sCopa, sCopa, sCopa));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
meshList[0]->RenderMesh();


// tronco der
shaderList[SH_CAFE].useShader();
uniformModel = shaderList[SH_CAFE].getModelLocation();
uniformProjection = shaderList[SH_CAFE].getProjectLocation();

model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(0.82f, yCentroTronco, zCasa));
model = glm::scale(model, glm::vec3(sxTr, syTr, szTr));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
meshList[1]->RenderMesh();

// copa der
shaderList[SH_VERDE_OSCURO].useShader();
uniformModel = shaderList[SH_VERDE_OSCURO].getModelLocation();
uniformProjection = shaderList[SH_VERDE_OSCURO].getProjectLocation();

model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(0.82f, yCentroCopa, zCasa));
model = glm::scale(model, glm::vec3(sCopa, sCopa, sCopa));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
meshList[0]->RenderMesh();






		}
		glUseProgram(0);
		mainWindow.swapBuffers();

	}
	return 0;
}
// inicializar matriz: glm::mat4 model(1.0);
// reestablecer matriz: model = glm::mat4(1.0);
//Traslacin
//model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));
//////////////// ROTACION //////////////////
//model = glm::rotate(model, 45 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
////////////////  ESCALA ////////////////
//model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
///////////////////// T+R////////////////
/*model = glm::translate(model, glm::vec3(valor, 0.0f, 0.0f));
model = glm::rotate(model, 45 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
model = glm::rotate(model, glm::radians(angulo), glm::vec3(0.0f, 1.0f, 0.0f));
*/
/////////////R+T//////////
/*model = glm::rotate(model, 45 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
model = glm::translate(model, glm::vec3(valor, 0.0f, 0.0f));
*/

