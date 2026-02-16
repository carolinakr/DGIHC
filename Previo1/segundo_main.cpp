#include <stdio.h>
#include <string.h>
#include <glew.h>
#include <glfw3.h>
#include<windows.h>
#include <cstdlib>
#include <ctime>

//Dimensiones de la ventana
const int WIDTH = 800, HEIGHT = 800;
GLuint VAO, VBO, shader; //Son 3 variables específicas de GL. La u de GLuint es por unsigned.
float fondo1 = 0, fondo2 = 0, fondo3 = 0;

//LENGUAJE DE SHADER (SOMBRAS) GLSL
//Vertex Shader, se procesa por GPU
//recibir color, salida Vcolor
//te dice qué versión de shader, location=0 es buena práctica, guardamos pos en un vector de 3. gl_Position crea los vectores.
static const char* vShader = "						\n\
#version 330										\n\
layout (location =0) in vec3 pos;					\n\
void main()											\n\
{													\n\
gl_Position=vec4(pos.x,pos.y,pos.z,1.0f); 			\n\
}";

//Fragment Shader
//recibir Vcolor y dar de salida color
//procesa toda la información de color. 3 tipos: color plano, iluminación y texturizado.
//el vec4 es rojo, verde, azul y alfa. RGBA
static const char* fShader = "						\n\
#version 330										\n\
out vec4 color;										\n\
void main()											\n\
{													\n\
	color = vec4(0.0f,0.0f,0.0f,1.0f);	 			\n\
}";



void CrearTriangulo()
{
	GLfloat vertices[] = { //Son los 3 vértices. 
		//Esto fue para la C
		 -0.9f,0.8f,0.0f,
		 -0.9f,0.4f,0.0f,
		 -0.8f,0.4f,0.0f,

		 -0.9f,0.8f,0.0f,
		 -0.8f,0.8f,0.0f,
		 -0.8f,0.4f,0.0f,

		 -0.8f,0.8f,0.0f,
		 -0.6f,0.8f,0.0f,
		 -0.6f,0.7f,0.0f,

		 -0.8f,0.4f,0.0f,
		 -0.6f,0.4f,0.0f,
		 -0.6f,0.5f,0.0f,

		 //Esto fue para la M

		 -0.2f,0.3f,0.0f,
		 -0.2f,-0.2f,0.0f,
		 -0.1f,-0.2f,0.0f,

		 -0.2f,0.3f,0.0f,
		 -0.1f,0.3f,0.0f,
		 -0.1f,-0.2f,0.0f,

		 -0.1f,0.3f,0.0f,
		  0.0f,0.05f,0.0f,
		  0.1f,0.3f,0.0f,

		   0.1f,0.3f,0.0f,
		   0.2f,0.3f,0.0f,
		   0.2f,-0.2f,0.0f,

		   0.1f,-0.2f,0.0f,
		   0.2f,-0.2f,0.0f,
		   0.1f,0.3f,0.0f,

		   //Y aquí para la G

			0.4f,-0.1f,0.0f,
			0.4f,-0.6f,0.0f,
			0.5f,-0.6f,0.0f,


			0.4f,-0.1f,0.0f,
			0.5f,-0.1f,0.0f,
			0.5f,-0.6f,0.0f,

			 0.5f,-0.1f,0.0f,
			 0.7f,-0.1f,0.0f,
			 0.7f,-0.2f,0.0f,

			  0.5f,-0.6f,0.0f,
			  0.7f,-0.6f,0.0f,
			  0.7f,-0.5f,0.0f,

			   0.55f,-0.35f,0.0f,
			   0.7f,-0.35f,0.0f,
			   0.7f,-0.45f,0.0f,

	};
	glGenVertexArrays(1, &VAO); //generar 1 VAO. 
	glBindVertexArray(VAO);//asignar VAO. Los flotantes se guardan en VAO y se almacenan los vértices ahí.

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //pasarle los datos al VBO asignando tamano, los datos y en este caso es estático pues no se modificarán los valores. Como declaramos 9 vertices, son 9.

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);//Stride en caso de haber datos de color por ejemplo, es saltar cierta cantidad de dato. Los datos pointer los agrupa de 3 en 3.
		//O sea, los agrupas para que sean coordenadas y no sólo puntos.
		glEnableVertexAttribArray(0); //Le envía datos al vertexShader, la de de vec3 pos
		//agregar valores a vèrtices y luego declarar un nuevo vertexAttribPointer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

}
void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType) //Función para agregar los shaders a la tarjeta gráfica

//the Program recibe los datos de theShader

{
	GLuint theShader = glCreateShader(shaderType);//theShader es un shader que se crea de acuerdo al tipo de shader: vertex o fragment
	const GLchar* theCode[1];
	theCode[0] = shaderCode;//shaderCode es el texto que se le pasa a theCode
	GLint codeLength[1];
	codeLength[0] = strlen(shaderCode);//longitud del texto
	glShaderSource(theShader,1, theCode, codeLength);//Se le asigna al shader el código
	glCompileShader(theShader);//Se comila el shader
	GLint result = 0;
	GLchar eLog[1024] = { 0 };
	//verificaciones y prevención de errores
	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("EL error al compilar el shader %d es: %s \n",shaderType, eLog);
		return;
	}
	glAttachShader(theProgram, theShader);//Si no hubo problemas se asigna el shader a theProgram el cual asigna el código a la tarjeta gráfica
}

void CompileShaders() {
	shader= glCreateProgram(); //se crea un programa
	if (!shader)
	{
		printf("Error creando el shader");
		return;
	}
	AddShader(shader, vShader, GL_VERTEX_SHADER);//Agregar vertex shader
	AddShader(shader, fShader, GL_FRAGMENT_SHADER);//Agregar fragment shader
	//Para terminar de linkear el programa y ver que no tengamos errores
	GLint result = 0;
	GLchar eLog[1024] = { 0 };
	glLinkProgram(shader);//se linkean los shaders a la tarjeta gráfica
	 //verificaciones y prevención de errores
	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("EL error al linkear es: %s \n", eLog);
		return;
	}
	glValidateProgram(shader);
	glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("EL error al validar es: %s \n", eLog);
		return;
	}



}
int main()
{
	//Inicialización de GLFW
	if (!glfwInit())
	{
		printf("Falló inicializar GLFW");
		glfwTerminate();
		return 1;
	}

	//****  LAS SIGUIENTES 4 LÍNEAS SE COMENTAN EN DADO CASO DE QUE AL USUARIO NO LE FUNCIONE LA VENTANA Y PUEDA CONOCER LA VERSIÓN DE OPENGL QUE TIENE ****/

	//Asignando variables de GLFW y propiedades de ventana
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//para solo usar el core profile de OpenGL y no tener retrocompatibilidad
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//CREAR VENTANA
	GLFWwindow *mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Primer ventana", NULL, NULL);
	srand((unsigned)time(NULL));
	if (!mainWindow)
	{
		printf("Fallo en crearse la ventana con GLFW");
		glfwTerminate();
		return 1;
	}
	//Obtener tamaño de Buffer
	int BufferWidth, BufferHeight;
	glfwGetFramebufferSize(mainWindow, &BufferWidth, &BufferHeight);

	//asignar el contexto
	glfwMakeContextCurrent(mainWindow);

	//permitir nuevas extensiones
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Falló inicialización de GLEW");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	// Asignar valores de la ventana y coordenadas
	//Asignar Viewport
	glViewport(0, 0, BufferWidth, BufferHeight);

 //Llamada a las funciones creadas antes del main
	CrearTriangulo();
	CompileShaders();


	//Loop mientras no se cierra la ventana
	while (!glfwWindowShouldClose(mainWindow))
	{
		//Recibir eventos del usuario
		glfwPollEvents();

		double t = glfwGetTime();
		int phase = (int)(t / 2.0);

		static int lastPhase = -1;
		static float r, g, b;

		if (phase != lastPhase)
		{
			lastPhase = phase;

			r = (float)rand() / RAND_MAX;
			g = (float)rand() / RAND_MAX;
			b = (float)rand() / RAND_MAX;
		}

		glClearColor(r, g, b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shader);

		glBindVertexArray(VAO);
		//GL_TRIANGLES para triángulos
		//GL_POINTS para puntos
		//GL_LINES para líneas
		glDrawArrays(GL_TRIANGLES,0,42); //sólo podemos añadir puntos, líneas y triángulos. (0 - desde donde comienzo, 3 - donde termino)
		glBindVertexArray(0);

		glUseProgram(0);

		glfwSwapBuffers(mainWindow);
		 
		//NO ESCRIBIR NINGUNA LÍNEA DESPUÉS DE glfwSwapBuffers(mainWindow); 
	}


	return 0;
}