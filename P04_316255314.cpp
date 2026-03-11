/*Práctica 4: Modelado Jerárquico.
*/

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>

// glm
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#include <gtc\random.hpp>

// clases para dar orden y limpieza al código
#include "Mesh.h"
#include "Shader.h"
#include "Sphere.h"
#include "Window.h"
#include "Camera.h"

using std::vector;

// Dimensiones de la ventana
const float toRadians = 3.14159265f / 180.0f;
const float PI = 3.14159265f;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader> shaderList;

// Vertex Shader
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";

Sphere sp = Sphere(1.0, 20, 20); // radio, slices, stacks

// =====================================================
// VARIABLES DE LA GRÚA
// =====================================================

// Rotación independiente de llantas
GLfloat rotLlanta1 = 0.0f;
GLfloat rotLlanta2 = 0.0f;
GLfloat rotLlanta3 = 0.0f;
GLfloat rotLlanta4 = 0.0f;

GLfloat velRotLlantas = 120.0f; // grados por segundo

// =====================================================
// VARIABLES DEL GATITO
// =====================================================

// Patas delanteras
GLfloat hombroIzq = 0.0f;
GLfloat codoIzq = 0.0f;

GLfloat hombroDer = 0.0f;
GLfloat codoDer = 0.0f;

// Patas traseras
GLfloat caderaIzq = 0.0f;
GLfloat rodillaIzq = 0.0f;

GLfloat caderaDer = 0.0f;
GLfloat rodillaDer = 0.0f;

// Cola
GLfloat colaBase = 0.0f;
GLfloat colaPunta = 0.0f;

// velocidad de giro de articulaciones
GLfloat velArt = 8.0f;

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

	Mesh* cubo = new Mesh();
	cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
	meshList.push_back(cubo);
}

// Pirámide triangular regular
void CrearPiramideTriangular()
{
	unsigned int indices_piramide_triangular[] = {
		0,1,2,
		1,3,2,
		3,0,2,
		1,0,3
	};

	GLfloat vertices_piramide_triangular[] = {
		-0.5f, -0.5f,  0.0f,
		 0.5f, -0.5f,  0.0f,
		 0.0f,  0.5f, -0.25f,
		 0.0f, -0.5f, -0.5f
	};

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices_piramide_triangular, indices_piramide_triangular, 12, 12);
	meshList.push_back(obj1);
}

/*
Crear cilindro y cono con arreglos dinámicos vector
*/
void CrearCilindro(int res, float R)
{
	int n, i;
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	// paredes del cilindro
	for (n = 0; n <= res; n++) {
		if (n != res) {
			x = R * cos((n)*dt);
			z = R * sin((n)*dt);
		}
		else {
			x = R * cos((0) * dt);
			z = R * sin((0) * dt);
		}

		for (i = 0; i < 6; i++) {
			switch (i) {
			case 0: vertices.push_back(x);    break;
			case 1: vertices.push_back(y);    break;
			case 2: vertices.push_back(z);    break;
			case 3: vertices.push_back(x);    break;
			case 4: vertices.push_back(0.5f); break;
			case 5: vertices.push_back(z);    break;
			}
		}
	}

	// circunferencia inferior
	for (n = 0; n <= res; n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0: vertices.push_back(x);      break;
			case 1: vertices.push_back(-0.5f);  break;
			case 2: vertices.push_back(z);      break;
			}
		}
	}

	// circunferencia superior
	for (n = 0; n <= res; n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0: vertices.push_back(x);    break;
			case 1: vertices.push_back(0.5f); break;
			case 2: vertices.push_back(z);    break;
			}
		}
	}

	for (i = 0; i < (int)vertices.size(); i++) indices.push_back(i);

	Mesh* cilindro = new Mesh();
	cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
	meshList.push_back(cilindro);
}

void CrearCono(int res, float R)
{
	int n, i;
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	vertices.push_back(0.0f);
	vertices.push_back(0.5f);
	vertices.push_back(0.0f);

	for (n = 0; n <= res; n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0: vertices.push_back(x); break;
			case 1: vertices.push_back(y); break;
			case 2: vertices.push_back(z); break;
			}
		}
	}

	vertices.push_back(R * cos(0) * dt);
	vertices.push_back(-0.5f);
	vertices.push_back(R * sin(0) * dt);

	for (i = 0; i < res + 2; i++) indices.push_back(i);

	Mesh* cono = new Mesh();
	cono->CreateMeshGeometry(vertices, indices, vertices.size(), res + 2);
	meshList.push_back(cono);
}

void CrearPiramideCuadrangular()
{
	vector<unsigned int> piramidecuadrangular_indices = {
		0,3,4,
		3,2,4,
		2,1,4,
		1,0,4,
		0,1,2,
		0,2,4
	};

	vector<GLfloat> piramidecuadrangular_vertices = {
		 0.5f,-0.5f, 0.5f,
		 0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f, 0.5f,
		 0.0f, 0.5f, 0.0f,
	};

	Mesh* piramide = new Mesh();
	piramide->CreateMeshGeometry(piramidecuadrangular_vertices, piramidecuadrangular_indices, 15, 18);
	meshList.push_back(piramide);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main()
{
	mainWindow = Window(800, 600);
	mainWindow.Initialise();

	CrearCubo();                 // índice 0
	CrearPiramideTriangular();   // índice 1
	CrearCilindro(25, 1.0f);     // índice 2
	CrearCono(25, 2.0f);         // índice 3
	CrearPiramideCuadrangular(); // índice 4
	CreateShaders();

	// Cámara con WASD + mouse
	camera = Camera(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-60.0f,
		0.0f,
		0.2f,
		0.2f
	);

	GLuint uniformProjection = 0;
	GLuint uniformModel = 0;
	GLuint uniformView = 0;
	GLuint uniformColor = 0;

	glm::mat4 projection = glm::perspective(
		glm::radians(60.0f),
		mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
		0.1f,
		100.0f
	);

	sp.init();
	sp.load();

	glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		glfwPollEvents();

		// Cámara
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
		glfwSetInputMode(mainWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		bool* keys = mainWindow.getsKeys();

		// ============================================================
		// CONTROLES DE LA GRÚA
		// ============================================================
		
		// Llanta 1: U / J
		if (keys[GLFW_KEY_U]) rotLlanta1 += velRotLlantas * deltaTime;
		if (keys[GLFW_KEY_J]) rotLlanta1 -= velRotLlantas * deltaTime;

		// Llanta 2: I / K
		if (keys[GLFW_KEY_I]) rotLlanta2 += velRotLlantas * deltaTime;
		if (keys[GLFW_KEY_K]) rotLlanta2 -= velRotLlantas * deltaTime;

		// Llanta 3: O / L
		if (keys[GLFW_KEY_O]) rotLlanta3 += velRotLlantas * deltaTime;
		if (keys[GLFW_KEY_L]) rotLlanta3 -= velRotLlantas * deltaTime;

		// Llanta 4: P / H
		if (keys[GLFW_KEY_P]) rotLlanta4 += velRotLlantas * deltaTime;
		if (keys[GLFW_KEY_H]) rotLlanta4 -= velRotLlantas * deltaTime;
		
		// ============================================================
		// CONTROLES DEL GATITO 
		// ============================================================

		// Pata delantera izquierda
		if (keys[GLFW_KEY_1]) hombroIzq += velArt * deltaTime;
		if (keys[GLFW_KEY_2]) hombroIzq -= velArt * deltaTime;
		if (keys[GLFW_KEY_3]) codoIzq += velArt * deltaTime;
		if (keys[GLFW_KEY_4]) codoIzq -= velArt * deltaTime;

		// Pata delantera derecha
		if (keys[GLFW_KEY_5]) hombroDer += velArt * deltaTime;
		if (keys[GLFW_KEY_6]) hombroDer -= velArt * deltaTime;
		if (keys[GLFW_KEY_7]) codoDer += velArt * deltaTime;
		if (keys[GLFW_KEY_8]) codoDer -= velArt * deltaTime;

		// Pata trasera izquierda
		if (keys[GLFW_KEY_T]) caderaIzq += velArt * deltaTime;
		if (keys[GLFW_KEY_Y]) caderaIzq -= velArt * deltaTime;
		if (keys[GLFW_KEY_G]) rodillaIzq += velArt * deltaTime;
		if (keys[GLFW_KEY_H]) rodillaIzq -= velArt * deltaTime;

		// Pata trasera derecha
		if (keys[GLFW_KEY_U]) caderaDer += velArt * deltaTime;
		if (keys[GLFW_KEY_I]) caderaDer -= velArt * deltaTime;
		if (keys[GLFW_KEY_J]) rodillaDer += velArt * deltaTime;
		if (keys[GLFW_KEY_K]) rodillaDer -= velArt * deltaTime;

		// Cola
		if (keys[GLFW_KEY_O]) colaBase += velArt * deltaTime;
		if (keys[GLFW_KEY_P]) colaBase -= velArt * deltaTime;
		if (keys[GLFW_KEY_L]) colaPunta += velArt * deltaTime;
		if (keys[GLFW_KEY_SEMICOLON]) colaPunta -= velArt * deltaTime;

		// Limpiar
		glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[0].useShader();
		uniformModel = shaderList[0].getModelLocation();
		uniformProjection = shaderList[0].getProjectLocation();
		uniformView = shaderList[0].getViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		// ---------------------------
		// RAÍZ DEL MODELO
		// ---------------------------
		glm::mat4 root = glm::mat4(1.0f);
		root = glm::translate(root, glm::vec3(0.0f, 0.0f, -12.0f));

		// ============================================================
		// GRÚA COMPLETA 
		// ============================================================
		
		// Esta es la raíz y padre de la grúa
		glm::mat4 modelAux = glm::mat4(1.0f);
		modelAux = glm::translate(modelAux, glm::vec3(0.0f, 1.2f, -12.0f));

		// Cabina principal
		glm::mat4 cabina = modelAux;
		cabina = glm::translate(cabina, glm::vec3(0.0f, 0.35f, 0.0f));
		glm::mat4 cabinaDraw = cabina;
		cabinaDraw = glm::rotate(cabinaDraw, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		cabinaDraw = glm::scale(cabinaDraw, glm::vec3(4.3f, 2.0f, 3.2f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(cabinaDraw));
		color = glm::vec3(1.0f, 0.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		// Base hija de cabina
		glm::mat4 baseNode = cabina;
		baseNode = glm::translate(baseNode, glm::vec3(0.0f, -1.35f, 0.0f));

		glm::mat4 baseDraw = baseNode;
		baseDraw = glm::scale(baseDraw, glm::vec3(6.2f, 1.3f, 4.8f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(baseDraw));
		color = glm::vec3(0.35f, 0.35f, 0.35f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[4]->RenderMeshGeometry();

		// Color de las llantas
		glm::vec3 colorLlanta = glm::vec3(0.05f, 0.05f, 0.05f);

		// Llanta 1
		glm::mat4 llanta1 = baseNode;
		llanta1 = glm::translate(llanta1, glm::vec3(-2.4f, -0.75f, 1.9f));
		llanta1 = glm::rotate(llanta1, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		llanta1 = glm::rotate(llanta1, glm::radians(rotLlanta1), glm::vec3(1.0f, 0.0f, 0.0f));
		llanta1 = glm::scale(llanta1, glm::vec3(0.9f, 0.9f, 0.6f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(llanta1));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorLlanta));
		meshList[2]->RenderMeshGeometry();

		// Llanta 2
		glm::mat4 llanta2 = baseNode;
		llanta2 = glm::translate(llanta2, glm::vec3(2.4f, -0.75f, 1.9f));
		llanta2 = glm::rotate(llanta2, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		llanta2 = glm::rotate(llanta2, glm::radians(rotLlanta2), glm::vec3(1.0f, 0.0f, 0.0f));
		llanta2 = glm::scale(llanta2, glm::vec3(0.9f, 0.9f, 0.6f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(llanta2));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorLlanta));
		meshList[2]->RenderMeshGeometry();

		// Llanta 3
		glm::mat4 llanta3 = baseNode;
		llanta3 = glm::translate(llanta3, glm::vec3(-2.4f, -0.75f, -1.9f));
		llanta3 = glm::rotate(llanta3, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		llanta3 = glm::rotate(llanta3, glm::radians(rotLlanta3), glm::vec3(1.0f, 0.0f, 0.0f));
		llanta3 = glm::scale(llanta3, glm::vec3(0.9f, 0.9f, 0.6f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(llanta3));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorLlanta));
		meshList[2]->RenderMeshGeometry();

		// Llanta 4
		glm::mat4 llanta4 = baseNode;
		llanta4 = glm::translate(llanta4, glm::vec3(2.4f, -0.75f, -1.9f));
		llanta4 = glm::rotate(llanta4, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		llanta4 = glm::rotate(llanta4, glm::radians(rotLlanta4), glm::vec3(1.0f, 0.0f, 0.0f));
		llanta4 = glm::scale(llanta4, glm::vec3(0.9f, 0.9f, 0.6f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(llanta4));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorLlanta));
		meshList[2]->RenderMeshGeometry();

		// Torreta hija de cabina
		glm::mat4 torreta = cabina;
		torreta = glm::translate(torreta, glm::vec3(0.0f, 1.2f, 0.0f));

		// Primera articulación
		glm::mat4 art1 = torreta;
		art1 = glm::translate(art1, glm::vec3(1.7f, 0.3f, 0.0f));
		art1 = glm::rotate(art1, glm::radians(135.0f + mainWindow.getarticulacion1()), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 art1Sphere = art1;
		art1Sphere = glm::scale(art1Sphere, glm::vec3(0.35f, 0.35f, 0.35f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(art1Sphere));
		color = glm::vec3(1.0f, 1.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// Primer brazo
		glm::mat4 brazo1 = art1;
		brazo1 = glm::translate(brazo1, glm::vec3(2.5f, 0.0f, 0.0f));
		glm::mat4 brazo1Draw = brazo1;
		brazo1Draw = glm::scale(brazo1Draw, glm::vec3(5.0f, 0.6f, 0.6f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(brazo1Draw));
		color = glm::vec3(0.85f, 0.0f, 0.85f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		// Segunda articulación
		glm::mat4 art2 = art1;
		art2 = glm::translate(art2, glm::vec3(5.0f, 0.0f, 0.0f));
		art2 = glm::rotate(art2, glm::radians(-90.0f + mainWindow.getarticulacion2()), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 art2Sphere = art2;
		art2Sphere = glm::scale(art2Sphere, glm::vec3(0.35f, 0.35f, 0.35f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(art2Sphere));
		color = glm::vec3(1.0f, 1.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// Segundo brazo
		glm::mat4 brazo2 = art2;
		brazo2 = glm::translate(brazo2, glm::vec3(2.5f, 0.0f, 0.0f));
		glm::mat4 brazo2Draw = brazo2;
		brazo2Draw = glm::scale(brazo2Draw, glm::vec3(5.0f, 0.6f, 0.6f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(brazo2Draw));
		color = glm::vec3(0.0f, 1.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		// Tercera articulación
		glm::mat4 art3 = art2;
		art3 = glm::translate(art3, glm::vec3(5.0f, 0.0f, 0.0f));
		art3 = glm::rotate(art3, glm::radians(90.0f + mainWindow.getarticulacion3()), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 art3Sphere = art3;
		art3Sphere = glm::scale(art3Sphere, glm::vec3(0.35f, 0.35f, 0.35f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(art3Sphere));
		color = glm::vec3(1.0f, 1.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// Tercer brazo
		glm::mat4 brazo3 = art3;
		brazo3 = glm::translate(brazo3, glm::vec3(2.5f, 0.0f, 0.0f));
		glm::mat4 brazo3Draw = brazo3;
		brazo3Draw = glm::scale(brazo3Draw, glm::vec3(5.0f, 0.6f, 0.6f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(brazo3Draw));
		color = glm::vec3(0.0f, 0.7f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		// Cuarta articulación
		glm::mat4 art4 = art3;
		art4 = glm::translate(art4, glm::vec3(5.0f, 0.0f, 0.0f));
		glm::mat4 art4Sphere = art4;
		art4Sphere = glm::scale(art4Sphere, glm::vec3(0.35f, 0.35f, 0.35f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(art4Sphere));
		color = glm::vec3(1.0f, 1.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// Canasta
		glm::mat4 canasta = art4;
		canasta = glm::translate(canasta, glm::vec3(0.8f, -0.8f, 0.0f));
		canasta = glm::rotate(canasta, glm::radians(mainWindow.getarticulacion4()), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 canastaDraw = canasta;
		canastaDraw = glm::scale(canastaDraw, glm::vec3(1.4f, 1.0f, 1.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(canastaDraw));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();
		

		// ============================================================
		// GATITO ROBOT (ACTIVO)
		// ============================================================

		/*
		
		// Colores
		glm::vec3 colorCuerpo = glm::vec3(0.70f, 0.70f, 0.78f);
		glm::vec3 colorArt = glm::vec3(1.0f, 0.85f, 0.20f);
		glm::vec3 colorCabeza = glm::vec3(0.78f, 0.78f, 0.86f);
		glm::vec3 colorOreja = glm::vec3(0.95f, 0.50f, 0.75f);
		glm::vec3 colorPatas = glm::vec3(0.55f, 0.80f, 1.0f);
		glm::vec3 colorPatitas = glm::vec3(0.90f, 0.90f, 0.95f);
		glm::vec3 colorCola = glm::vec3(0.70f, 0.55f, 0.95f);

		// Cuerpo
		glm::mat4 cuerpo = root;
		cuerpo = glm::translate(cuerpo, glm::vec3(0.0f, 0.8f, 0.0f));

		glm::mat4 cuerpoDraw = cuerpo;
		cuerpoDraw = glm::scale(cuerpoDraw, glm::vec3(4.5f, 2.0f, 2.3f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(cuerpoDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorCuerpo));
		meshList[0]->RenderMesh();

		// Cuello -> Cabeza -> Orejas
		glm::mat4 cuello = cuerpo;
		cuello = glm::translate(cuello, glm::vec3(2.55f, 0.55f, 0.0f));

		glm::mat4 cuelloDraw = cuello;
		cuelloDraw = glm::scale(cuelloDraw, glm::vec3(0.7f, 0.8f, 0.8f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(cuelloDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorCuerpo));
		meshList[0]->RenderMesh();

		glm::mat4 cabeza = cuello;
		cabeza = glm::translate(cabeza, glm::vec3(1.0f, 0.3f, 0.0f));

		glm::mat4 cabezaDraw = cabeza;
		cabezaDraw = glm::scale(cabezaDraw, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(cabezaDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorCabeza));
		sp.render();

		// Oreja izquierda
		glm::mat4 orejaIzq = cabeza;
		orejaIzq = glm::translate(orejaIzq, glm::vec3(0.15f, 0.95f, 0.45f));
		orejaIzq = glm::rotate(orejaIzq, glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		orejaIzq = glm::scale(orejaIzq, glm::vec3(0.45f, 0.65f, 0.35f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(orejaIzq));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorOreja));
		meshList[1]->RenderMesh();

		// Oreja derecha
		glm::mat4 orejaDer = cabeza;
		orejaDer = glm::translate(orejaDer, glm::vec3(0.15f, 0.95f, -0.45f));
		orejaDer = glm::rotate(orejaDer, glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		orejaDer = glm::scale(orejaDer, glm::vec3(0.45f, 0.65f, 0.35f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(orejaDer));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorOreja));
		meshList[1]->RenderMesh();

		// Pata delantera izquierda
		glm::mat4 artHombroIzq = cuerpo;
		artHombroIzq = glm::translate(artHombroIzq, glm::vec3(1.45f, -0.7f, 0.85f));
		artHombroIzq = glm::rotate(artHombroIzq, glm::radians(-90.0f + hombroIzq), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 artHombroIzqDraw = artHombroIzq;
		artHombroIzqDraw = glm::scale(artHombroIzqDraw, glm::vec3(0.22f, 0.22f, 0.22f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(artHombroIzqDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorArt));
		sp.render();

		glm::mat4 brazoIzq = artHombroIzq;
		brazoIzq = glm::translate(brazoIzq, glm::vec3(0.0f, -0.95f, 0.0f));

		glm::mat4 brazoIzqDraw = brazoIzq;
		brazoIzqDraw = glm::scale(brazoIzqDraw, glm::vec3(0.42f, 1.8f, 0.42f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(brazoIzqDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatas));
		meshList[0]->RenderMesh();

		glm::mat4 artCodoIzq = artHombroIzq;
		artCodoIzq = glm::translate(artCodoIzq, glm::vec3(0.0f, -1.8f, 0.0f));
		artCodoIzq = glm::rotate(artCodoIzq, glm::radians(codoIzq), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 artCodoIzqDraw = artCodoIzq;
		artCodoIzqDraw = glm::scale(artCodoIzqDraw, glm::vec3(0.20f, 0.20f, 0.20f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(artCodoIzqDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorArt));
		sp.render();

		glm::mat4 antebrazoIzq = artCodoIzq;
		antebrazoIzq = glm::translate(antebrazoIzq, glm::vec3(0.0f, -0.80f, 0.0f));

		glm::mat4 antebrazoIzqDraw = antebrazoIzq;
		antebrazoIzqDraw = glm::scale(antebrazoIzqDraw, glm::vec3(0.36f, 1.45f, 0.36f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(antebrazoIzqDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatas));
		meshList[0]->RenderMesh();

		glm::mat4 patitaDIzq = artCodoIzq;
		patitaDIzq = glm::translate(patitaDIzq, glm::vec3(0.18f, -1.55f, 0.0f));

		glm::mat4 patitaDIzqDraw = patitaDIzq;
		patitaDIzqDraw = glm::scale(patitaDIzqDraw, glm::vec3(0.55f, 0.20f, 0.48f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(patitaDIzqDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatitas));
		meshList[0]->RenderMesh();

		// Pata delantera derecha
		glm::mat4 artHombroDer = cuerpo;
		artHombroDer = glm::translate(artHombroDer, glm::vec3(1.45f, -0.7f, -0.85f));
		artHombroDer = glm::rotate(artHombroDer, glm::radians(-90.0f + hombroDer), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 artHombroDerDraw = artHombroDer;
		artHombroDerDraw = glm::scale(artHombroDerDraw, glm::vec3(0.22f, 0.22f, 0.22f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(artHombroDerDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorArt));
		sp.render();

		glm::mat4 brazoDer = artHombroDer;
		brazoDer = glm::translate(brazoDer, glm::vec3(0.0f, -0.95f, 0.0f));

		glm::mat4 brazoDerDraw = brazoDer;
		brazoDerDraw = glm::scale(brazoDerDraw, glm::vec3(0.42f, 1.8f, 0.42f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(brazoDerDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatas));
		meshList[0]->RenderMesh();

		glm::mat4 artCodoDer = artHombroDer;
		artCodoDer = glm::translate(artCodoDer, glm::vec3(0.0f, -1.8f, 0.0f));
		artCodoDer = glm::rotate(artCodoDer, glm::radians(codoDer), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 artCodoDerDraw = artCodoDer;
		artCodoDerDraw = glm::scale(artCodoDerDraw, glm::vec3(0.20f, 0.20f, 0.20f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(artCodoDerDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorArt));
		sp.render();

		glm::mat4 antebrazoDer = artCodoDer;
		antebrazoDer = glm::translate(antebrazoDer, glm::vec3(0.0f, -0.80f, 0.0f));

		glm::mat4 antebrazoDerDraw = antebrazoDer;
		antebrazoDerDraw = glm::scale(antebrazoDerDraw, glm::vec3(0.36f, 1.45f, 0.36f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(antebrazoDerDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatas));
		meshList[0]->RenderMesh();

		glm::mat4 patitaDDer = artCodoDer;
		patitaDDer = glm::translate(patitaDDer, glm::vec3(0.18f, -1.55f, 0.0f));

		glm::mat4 patitaDDerDraw = patitaDDer;
		patitaDDerDraw = glm::scale(patitaDDerDraw, glm::vec3(0.55f, 0.20f, 0.48f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(patitaDDerDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatitas));
		meshList[0]->RenderMesh();

		// Pata trasera izquierda
		glm::mat4 artCaderaIzq = cuerpo;
		artCaderaIzq = glm::translate(artCaderaIzq, glm::vec3(-1.45f, -0.7f, 0.85f));
		artCaderaIzq = glm::rotate(artCaderaIzq, glm::radians(-90.0f + caderaIzq), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 artCaderaIzqDraw = artCaderaIzq;
		artCaderaIzqDraw = glm::scale(artCaderaIzqDraw, glm::vec3(0.22f, 0.22f, 0.22f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(artCaderaIzqDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorArt));
		sp.render();

		glm::mat4 piernaIzq = artCaderaIzq;
		piernaIzq = glm::translate(piernaIzq, glm::vec3(0.0f, -0.95f, 0.0f));

		glm::mat4 piernaIzqDraw = piernaIzq;
		piernaIzqDraw = glm::scale(piernaIzqDraw, glm::vec3(0.45f, 1.8f, 0.45f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(piernaIzqDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatas));
		meshList[0]->RenderMesh();

		glm::mat4 artRodillaIzq = artCaderaIzq;
		artRodillaIzq = glm::translate(artRodillaIzq, glm::vec3(0.0f, -1.8f, 0.0f));
		artRodillaIzq = glm::rotate(artRodillaIzq, glm::radians(rodillaIzq), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 artRodillaIzqDraw = artRodillaIzq;
		artRodillaIzqDraw = glm::scale(artRodillaIzqDraw, glm::vec3(0.20f, 0.20f, 0.20f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(artRodillaIzqDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorArt));
		sp.render();

		glm::mat4 chamorroIzq = artRodillaIzq;
		chamorroIzq = glm::translate(chamorroIzq, glm::vec3(0.0f, -0.80f, 0.0f));

		glm::mat4 chamorroIzqDraw = chamorroIzq;
		chamorroIzqDraw = glm::scale(chamorroIzqDraw, glm::vec3(0.38f, 1.45f, 0.38f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(chamorroIzqDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatas));
		meshList[0]->RenderMesh();

		glm::mat4 patitaTIzq = artRodillaIzq;
		patitaTIzq = glm::translate(patitaTIzq, glm::vec3(0.18f, -1.55f, 0.0f));

		glm::mat4 patitaTIzqDraw = patitaTIzq;
		patitaTIzqDraw = glm::scale(patitaTIzqDraw, glm::vec3(0.60f, 0.20f, 0.50f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(patitaTIzqDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatitas));
		meshList[0]->RenderMesh();

		// Pata trasera derecha
		glm::mat4 artCaderaDer = cuerpo;
		artCaderaDer = glm::translate(artCaderaDer, glm::vec3(-1.45f, -0.7f, -0.85f));
		artCaderaDer = glm::rotate(artCaderaDer, glm::radians(-90.0f + caderaDer), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 artCaderaDerDraw = artCaderaDer;
		artCaderaDerDraw = glm::scale(artCaderaDerDraw, glm::vec3(0.22f, 0.22f, 0.22f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(artCaderaDerDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorArt));
		sp.render();

		glm::mat4 piernaDer = artCaderaDer;
		piernaDer = glm::translate(piernaDer, glm::vec3(0.0f, -0.95f, 0.0f));

		glm::mat4 piernaDerDraw = piernaDer;
		piernaDerDraw = glm::scale(piernaDerDraw, glm::vec3(0.45f, 1.8f, 0.45f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(piernaDerDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatas));
		meshList[0]->RenderMesh();

		glm::mat4 artRodillaDer = artCaderaDer;
		artRodillaDer = glm::translate(artRodillaDer, glm::vec3(0.0f, -1.8f, 0.0f));
		artRodillaDer = glm::rotate(artRodillaDer, glm::radians(rodillaDer), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 artRodillaDerDraw = artRodillaDer;
		artRodillaDerDraw = glm::scale(artRodillaDerDraw, glm::vec3(0.20f, 0.20f, 0.20f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(artRodillaDerDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorArt));
		sp.render();

		glm::mat4 chamorroDer = artRodillaDer;
		chamorroDer = glm::translate(chamorroDer, glm::vec3(0.0f, -0.80f, 0.0f));

		glm::mat4 chamorroDerDraw = chamorroDer;
		chamorroDerDraw = glm::scale(chamorroDerDraw, glm::vec3(0.38f, 1.45f, 0.38f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(chamorroDerDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatas));
		meshList[0]->RenderMesh();

		glm::mat4 patitaTDer = artRodillaDer;
		patitaTDer = glm::translate(patitaTDer, glm::vec3(0.18f, -1.55f, 0.0f));

		glm::mat4 patitaTDerDraw = patitaTDer;
		patitaTDerDraw = glm::scale(patitaTDerDraw, glm::vec3(0.60f, 0.20f, 0.50f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(patitaTDerDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorPatitas));
		meshList[0]->RenderMesh();

		// Cola
		glm::mat4 artTrasero = cuerpo;
		artTrasero = glm::translate(artTrasero, glm::vec3(-2.30f, 0.35f, 0.0f));
		artTrasero = glm::rotate(artTrasero, glm::radians(colaBase), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 artTraseroDraw = artTrasero;
		artTraseroDraw = glm::scale(artTraseroDraw, glm::vec3(0.22f, 0.22f, 0.22f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(artTraseroDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorArt));
		sp.render();

		glm::mat4 cola1 = artTrasero;
		cola1 = glm::translate(cola1, glm::vec3(-0.70f, 0.18f, 0.0f));
		cola1 = glm::rotate(cola1, glm::radians(25.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 cola1Draw = cola1;
		cola1Draw = glm::scale(cola1Draw, glm::vec3(1.35f, 0.22f, 0.22f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(cola1Draw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorCola));
		meshList[0]->RenderMesh();

		glm::mat4 artCola = cola1;
		artCola = glm::translate(artCola, glm::vec3(-0.78f, 0.0f, 0.0f));
		artCola = glm::rotate(artCola, glm::radians(colaPunta), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 artColaDraw = artCola;
		artColaDraw = glm::scale(artColaDraw, glm::vec3(0.18f, 0.18f, 0.18f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(artColaDraw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorArt));
		sp.render();

		glm::mat4 cola2 = artCola;
		cola2 = glm::translate(cola2, glm::vec3(-0.62f, 0.10f, 0.0f));
		cola2 = glm::rotate(cola2, glm::radians(18.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 cola2Draw = cola2;
		cola2Draw = glm::scale(cola2Draw, glm::vec3(1.05f, 0.17f, 0.17f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(cola2Draw));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorCola));
		meshList[0]->RenderMesh();
		*/

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}