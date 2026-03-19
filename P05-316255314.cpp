/*
Practica 5: Optimizacion y Carga de Modelos
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

#include "Window.h"
#include "Mesh.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

// Modelos del auto.
Model Auto_Base;
Model Auto_Cofre;
Model Auto_LlantaDD;   // delantera derecha
Model Auto_LlantaDI;   // delantera izquierda
Model Auto_LlantaTD;   // trasera derecha
Model Auto_LlantaTI;   // trasera izquierda

Skybox skybox;

// Posicion de las llantas (espacio local del modelo sin escalar)
GLfloat llantaX =  80.0f;  // distancia frontal/trasera
GLfloat llantaY =   6.0f;  // altura
GLfloat llantaZ =  100.0f;  // separacion lateral

GLfloat deltaTime = 0.0f;
GLfloat lastTime  = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Vertex Shader
static const char* vShader = "shaders/shader_m.vert";
// Fragment Shader
static const char* fShader = "shaders/shader_m.frag";


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

	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}



int main()
{
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.5f, 7.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 1.0f);

	// --- Carga del auto ---
	Auto_Base = Model();
	Auto_Base.LoadModel("Models/auto_base.obj");

	Auto_Cofre = Model();
	Auto_Cofre.LoadModel("Models/auto_cofre.obj");

	Auto_LlantaDD = Model();
	Auto_LlantaDD.LoadModel("Models/auto_llanta.obj");

	Auto_LlantaDI = Model();
	Auto_LlantaDI.LoadModel("Models/auto_llanta.obj");

	Auto_LlantaTD = Model();
	Auto_LlantaTD.LoadModel("Models/auto_llanta.obj");

	Auto_LlantaTI = Model();
	Auto_LlantaTI.LoadModel("Models/auto_llanta.obj");

	// --- Skybox ---
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
	glm::mat4 modelAuto(1.0);   // matriz base del auto (con traslacion)
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	// Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
		mainWindow.procesaMovimiento(deltaTime);

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Skybox
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

		shaderList[0].UseShader();
		uniformModel      = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView       = shaderList[0].GetViewLocation();
		uniformColor      = shaderList[0].getColorLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		// --- Piso ---
		color = glm::vec3(0.5f, 0.5f, 0.5f);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMesh();

		// =====================================================
		// DIBUJO DEL AUTO
		// =====================================================

		// Matriz raiz del auto (traslacion global + escala)
		modelAuto = glm::mat4(1.0);
		modelAuto = glm::translate(modelAuto, glm::vec3(mainWindow.getAutoX(), 0.0f, mainWindow.getAutoZ()));
		modelAuto = glm::scale(modelAuto, glm::vec3(0.1f, 0.1f, 0.1f));

		// --- Carroceria --
		color = glm::vec3(0.8f, 0.1f, 0.1f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = modelAuto;
		model = glm::translate(model, glm::vec3(0.0f, 12.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Auto_Base.RenderModel();

		// --- Cofre 
		color = glm::vec3(0.1f, 5.0f, 0.8f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = modelAuto;
		model = glm::translate(model, glm::vec3(0.0f, 28.0f, 115.0f));
		model = glm::rotate(model, glm::radians(mainWindow.getRotCofre()), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Auto_Cofre.RenderModel();

		// --- Llanta delantera derecha ---
		// Ajusta el vec3 de translate para colocarla en su posicion correcta
		color = glm::vec3(0.15f, 0.15f, 0.15f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = modelAuto;
		model = glm::translate(model, glm::vec3( llantaX, llantaY,  llantaZ));
		model = glm::rotate(model, glm::radians(mainWindow.getRotLlantaDD() + mainWindow.getRotLlantasGlobal()), glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Auto_LlantaDD.RenderModel();

		// --- Llanta delantera izquierda ---
		model = modelAuto;
		model = glm::translate(model, glm::vec3( llantaX, llantaY, -llantaZ));
		model = glm::rotate(model, glm::radians(mainWindow.getRotLlantaDI() + mainWindow.getRotLlantasGlobal()), glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Auto_LlantaDI.RenderModel();

		// --- Llanta trasera derecha ---
		model = modelAuto;
		model = glm::translate(model, glm::vec3(-llantaX, llantaY,  llantaZ));
		model = glm::rotate(model, glm::radians(mainWindow.getRotLlantaTD() + mainWindow.getRotLlantasGlobal()), glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Auto_LlantaTD.RenderModel();

		// --- Llanta trasera izquierda ---
		model = modelAuto;
		model = glm::translate(model, glm::vec3(-llantaX, llantaY, -llantaZ));
		model = glm::rotate(model, glm::radians(mainWindow.getRotLlantaTI() + mainWindow.getRotLlantasGlobal()), glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Auto_LlantaTI.RenderModel();

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
