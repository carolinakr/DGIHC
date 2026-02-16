#include <stdio.h>
#include <glew.h>
#include <glfw3.h>
//Dimensiones de la ventana
const int WIDTH = 800, HEIGHT = 800;

int main() //Creamos viewport y le ponemos color verde.
{
	//Inicialización de GLFW
	if (!glfwInit())
	{
		printf("Falló inicializar GLFW");
		glfwTerminate();
		return 1;
	}
	//****  LAS SIGUIENTES 4 LÍNEAS SE COMENTAN EN DADO CASO DE QUE AL USUARIO NO LE FUNCIONE LA VENTANA Y PUEDA CONOCER LA VERSIÓN DE OPENGL QUE TIENE ****/
	// GLFW librería de control, monitor, mouse, etc
	//Asignando variables de GLFW y propiedades de ventana
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //Porque estamos usando la versión 4
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//para solo usar el core profile de OpenGL y no tener retrocompatibilidad
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Aquí explicamos que estamos usando OPENGL. Hay 3 perfiles, nosotros estaremos utilizando el CORE. este profile es como el moderno y más completo.
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //Si trabajas con una versión y ve que puede utilizar u optimizar algo, que lo haga.

	//CREAR VENTANA
	GLFWwindow *mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Primer ventana", NULL, NULL); //px ancho, px alto, nombre, bandera, bandera. Las banderas son NULL.

	if (!mainWindow) //si falla al crearse, significa que nuestro hardware no soportó.
	{
		printf("Fallo en crearse la ventana con GLFW");
		glfwTerminate();
		return 1;
	}
	//Obtener tamaño de Buffer
	int BufferWidth, BufferHeight; //Guardamos dimensiones de pixeles en los buffers.
	glfwGetFramebufferSize(mainWindow, &BufferWidth, &BufferHeight); //FrameBuffer es memoria de video. Guarda los tamaños y datos de color.

	//asignar el contexto
	glfwMakeContextCurrent(mainWindow); //es la ventana activa.

	//permitir nuevas extensiones
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) //GLEW y GLAW tienen las funciones básicas de OPENGL de bajo nivel. 
	{
		printf("Falló inicialización de GLEW");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	// Asignar valores de la ventana y coordenadas
	//Asignar Viewport
	glViewport(0, 0, BufferWidth, BufferHeight); //Si empieza con gl, ya es GL. El viewport centra el origen y a partir del origen, ajustamos a la ventana
	printf("Version de Opengl: %s \n",glGetString(GL_VERSION));
	printf("Marca: %s \n", glGetString(GL_VENDOR));
	printf("Renderer: %s \n", glGetString(GL_RENDERER));
	printf("Shaders: %s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	//Loop mientras no se cierra la ventana
	while (!glfwWindowShouldClose(mainWindow)) //únicamente es falso cuando se le da x o alt f4.
	{
		//Recibir eventos del usuario, teclado y mouse.
		glfwPollEvents();

		//Limpiar la ventana. Deja el color de fondo de la ventana contenedora. POR ESO ES FONDO VERDE.
		glClearColor(0.0f,1.0f,0.0f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(mainWindow); //GL maneja buffer doble. Uno muestra y el otro va haciendo cálculos. Cada ciclo de reloj se cambia, entonces es imperceptible.
		//Todo lo escrito a partir de aquí no tendrá ningún impacto en la ventana.
	}


	return 0;
}