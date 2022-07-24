// Draw four triangles on a red background
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

// Read a shader source from a file
// store the shader source in a std::vector<char>
void read_shader_src(const char *fname, std::vector<char> &buffer);

// Compile a shader
GLuint load_and_compile_shader(const char *fname, GLenum shaderType);

// Create a program from two shaders
GLuint create_program(const char *path_vert_shader, const char *path_frag_shader);

// Called when the window is resized
void window_resized(GLFWwindow *window, int width, int height);

// Called for keyboard events
void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods);

// Render scene
void display(GLFWwindow *window, GLuint &vao);

// Initialize the data to be rendered
void initialize(GLuint &vao);

void error_callback(int code, const char* description);

int main () {
	// Initialize GLFW
	if ( !glfwInit()) {
		std::cerr << "Failed to initialize GLFW! I'm out!" << std::endl;
		exit(-1);
	}

	glfwSetErrorCallback(error_callback);

	// Use OpenGL 3.2 core profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// Open a window and attach an OpenGL rendering context to the window surface
	GLFWwindow *window = glfwCreateWindow(500, 500, "OpenGL 101", NULL, NULL);
	if(!window) {
		std::cerr << "Failed to open a window! I'm out!" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	// Set the window context current
	glfwMakeContextCurrent(window);

	// Register a callback function for window resize events
	glfwSetWindowSizeCallback(window, window_resized );

	// Register a callback function for keyboard pressed events
	glfwSetKeyCallback(window, keyboard);

	// Print the OpenGL version
	int major, minor, rev;
	major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
	minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
	rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
	std::cout << "OpenGL - " << major << "." << minor << "." << rev << std::endl;

	// Initialize GLEW
	/* glewExperimental = GL_TRUE; */
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW! I'm out!" << std::endl;
		glfwTerminate();
		exit(-1);
	}

	// Create a vertex array object
	GLuint vao;

	// Initialize the data to be rendered
	initialize(vao);

	// Create a rendering loop
	int should_close = GL_FALSE;

	while(!should_close) {
		// Display scene
		display(window, vao);

		// Pool for events
		glfwPollEvents();
		// Check if the window was closed
		should_close = glfwWindowShouldClose(window);
	}

	// Terminate GLFW
	glfwTerminate();

	return 0;
}

// Render scene
void display(GLFWwindow *window, GLuint &vao) {
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 12);

	// Swap front and back buffers
	glfwSwapBuffers(window);
}

void initialize(GLuint &vao) {
	// Use a Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// 4 triangles to be rendered
	GLfloat vertices_position[24] = {
		0.0, 0.0,
		0.5, 0.0,
		0.5, 0.5,
		
		0.0, 0.0,
		0.0, 0.5,
		-0.5, 0.5,
		
		0.0, 0.0,
		-0.5, 0.0,
		-0.5, -0.5,		

		0.0, 0.0,
		0.0, -0.5,
		0.5, -0.5,
	};

	// Create a Vector Buffer Object that will store the vertices on video memory
	GLuint vbo;
	glGenBuffers(1, &vbo);

	// Allocate space and upload the data from CPU to GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_position), vertices_position, GL_STATIC_DRAW);	

	GLuint shaderProgram = create_program("shaders/vert.shader", "shaders/frag.shader");

	// Get the location of the attributes that enters in the vertex shader
	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");

	// Specify how the data for position can be accessed
	glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable the attribute
	glEnableVertexAttribArray(position_attribute);
}

// Called when the window is resized
void window_resized(GLFWwindow *window, int width, int height) {
	// Use red to clear the screen
	glClearColor(1, 0, 0, 1);

	// Set the viewport
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);
}

// Called for keyboard events
void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if(key == 'Q' && action == GLFW_PRESS) {
		glfwTerminate();
		exit(0);
	}
}

// Read a shader source from a file
// store the shader source in a std::vector<char>
void read_shader_src(const char *fname, std::vector<char> &buffer) {
	std::ifstream in;
	in.open(fname, std::ios::binary);

	if(in.is_open()) {
		// Get the number of bytes stored in this file
		in.seekg(0, std::ios::end);
		size_t length = (size_t)in.tellg();

		// Go to start of the file
		in.seekg(0, std::ios::beg);

		// Read the content of the file in a buffer
		buffer.resize(length + 1);
		in.read(&buffer[0], length);
		in.close();
		// Add a valid C - string end
		buffer[length] = '\0';
	}
	else {
		std::cerr << "Unable to open " << fname << " I'm out!" << std::endl;
		exit(-1);
	}
}

// Compile a shader
GLuint load_and_compile_shader(const char *fname, GLenum shaderType) {
	// Load a shader from an external file
	std::vector<char> buffer;
	read_shader_src(fname, buffer);
	const char *src = &buffer[0];

	// Compile the shader
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	// Check the result of the compilation
	GLint test;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &test);
	if(!test) {
		std::cerr << "Shader compilation failed with this message:" << std::endl;
		std::vector<char> compilation_log(512);
		glGetShaderInfoLog(shader, compilation_log.size(), NULL, &compilation_log[0]);
		std::cerr << &compilation_log[0] << std::endl;
		glfwTerminate();
		exit(-1);
	}
	return shader;
}

// Create a program from two shaders
GLuint create_program(const char *path_vert_shader, const char *path_frag_shader) {
	// Load and compile the vertex and fragment shaders
	GLuint vertexShader = load_and_compile_shader(path_vert_shader, GL_VERTEX_SHADER);
	GLuint fragmentShader = load_and_compile_shader(path_frag_shader, GL_FRAGMENT_SHADER);

	// Attach the above shader to a program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	// Flag the shaders for deletion
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Link and use the program
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	return shaderProgram;
}


void error_callback(int code, const char* description) {
	std::cerr << "Error occurred: " << description << "(" << code << ")" << std::endl;
}
