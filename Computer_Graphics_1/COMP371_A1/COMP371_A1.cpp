/*
* COMP371 - Computer Graphics
* Assignment #1
* Programmed by Philip Michael
* ID#40004861
* Several sources and code taken from learnopengl.com
*/

// Std. Includes
#include <string>
#include <iostream>
#include <vector>

// GLEW
#define GLEW_STATIC
#include <glew.h>

// GLFW
#include <glfw3.h>

// GLM Mathemtics
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Other includes
#include "Shader.h"
#include "FileReader.h"
#include "Camera.h"
//#include <SOIL.h>

using namespace std;

// Function prototypes
void window_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// View and projection matrices (x,y,z,w)
glm::mat4 view;
glm::mat4 projection;
glm::mat4 model;

// Model, view, & projection
GLint modelLoc;
GLint viewLoc;
GLint projLoc;

// Window dimensions
GLuint WIDTH = 800, HEIGHT = 800;

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
GLfloat yaw = -90.0f;	// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat pitch = 0.0f;
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
GLfloat fov = 45.0f;
bool keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

							// The MAIN function, from here we start the application and run the game loop
int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "COMP371 Assignment #1 - Philip Michael", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR);

	// WIREFRAME MODE
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST); // Depth feeling
	//glEnable(GL_CULL_FACE); // Cull back faces

	// Build and compile our shader program
	Shader ourShader("vertex.shader", "fragment.shader");
	//*** NOTE: FOR 3D MODELS NEED TO EDIT THE VERTEX SHADER TO HAVE:
	// gl_Position = projection * view * model * vec4(position, 1.0f);


	// Setup and call for input handling
	vector<glm::vec3> points, trajectory; // Vec3 objects stored for our pts and trajectories
	vector<GLfloat> draw_verts; // The drawable vertices
	vector<GLuint> indices; // Vector object to hold the indices we pass to drawing the elements
	bool empty = false; // False if we have translational shape, true otherwise
	int width = 0, height = 0; // Spans: # of rotations; Width: Trajectory size; Height: Points size
	int spans = 0;

	// Pass the above variables to the FileReader.h function 'readInput' to process the input file
	readInput("input_a1.txt", points, trajectory, empty, spans, height, width);
	// Output values (for debugging purposes)
	cout << "# of spans: " << spans << endl;
	cout << "Empty: " << empty << "\n" << endl;
	cout << "Width: " << width << "\n" << endl;
	cout << "Height: " << height << "\n" << endl;

	// Output points curve
	cout << "=== POINTS CURVE ===\n";
	for (int i = 0; i < points.size(); i++) {
		cout << "Index " << i << "; content = " << points[i].x << " " << points[i].y << " " << points[i].z << endl;
	}

	// ------- BEGIN TRANSLATIONAL SHAPES -------//
	if (!empty) {
		// Output trajectory curve
		cout << "\n=== TRAJECTORY CURVE ===\n";

		for (int i = 0; i < trajectory.size(); i++) {
			cout << "Index " << i << "; content = " << trajectory[i].x << " " << trajectory[i].y << " " << trajectory[i].z << endl;
		}

		// INDICES
		GLuint heightcalc; // Result of multiplying the height with our i value 
		for (int i = 0; i < trajectory.size(); i++) {
			// Outer i loop to go through the trajectory curve
			for (int j = 1; j < points.size(); j++) {
				// Inner j loop to go through with the points based on the trajectory (a point for all the trajectory)
				heightcalc = height * i; // Calculate the height to add
				if (i < (trajectory.size() - 1)) { // Error if we try to go over bounds
					indices.push_back((j - 1) + heightcalc); // This will pass 0 at first run [CASE OF MESH]
					indices.push_back(j + heightcalc); // This will pass 1 at first run [CASE OF MESH]
					indices.push_back((j - 1) + height + heightcalc); // This will pass 4 at first run [CASE OF MESH]
					indices.push_back(j + heightcalc); // This will pass 1 at first run [CASE OF MESH]
					indices.push_back(j + height + heightcalc); // This will pass 5 at first run [CASE OF MESH]
					indices.push_back((j - 1) + height + heightcalc); // This will pass 4 at first run [CASE OF MESH]
				}
			}
		}

		// OUTPUT INDICES
		cout << "\n==== INDICES ====\n" << endl;
		for (int i = 0; i < indices.size(); i += 3) {
			cout << indices[i] << " " << indices[i + 1] << " " << indices[i + 2] << endl;
		}

		// BEGIN TRANSLATION FOR THE SHAPE
		glm::vec3 tempvec;
		for (int i = 0; i < trajectory.size(); i++) {
			//tempvec = trajectory[i + 1] - trajectory[i];
			tempvec = trajectory[i] - points.back();
			// Now need to multiply this to the appropriate point in the points vector
			for (int j = 0; j < points.size(); j++) {
				glm::vec4 tv4;
				glm::mat4 trans; // Identity matrix
				trans = glm::translate(trans, tempvec);
				tv4 = trans * glm::vec4(points[j], 1.0f);
				draw_verts.push_back(ceil(tv4.x * 100) / 100);
				draw_verts.push_back(ceil(tv4.y * 100) / 100);
				draw_verts.push_back(ceil(tv4.z * 100) / 100);
			}
		}

		// OUTPUT VERTICES
		cout << "\n==== Vertices ===== \n" << endl;
		for (int i = 0; i < draw_verts.size(); i += 3) {
			//cout << "draw_verts at i = " << i << " gives: " << draw_verts[i] << endl;
			cout << draw_verts[i] << " " << draw_verts[i + 1] << " " << draw_verts[i + 2] << endl;
		}
	}
	// ------- END TRANSLATIONAL SHAPES -------//
	////////////////////////////////////////////
	// ------- BEGIN ROTATIONAL SHAPES ------//
	else {
		// Find the indices
		int num_sides = points.size() * 3 * (spans - 1); // # of vertex sets * 3 to form a triangle * # of spans - 1 , result is 54 for cylinder
		for (int i = 1; i <= (num_sides / 3); i+=2) {
			indices.push_back((i + points.size() - 1) - 1);
			indices.push_back((i + points.size() - 1));
			indices.push_back((i + points.size() - 1) + 1);
			indices.push_back((i + points.size() - 1) + 1);
			indices.push_back((i + points.size() - 1));
			indices.push_back((i + points.size() - 1) + 2);
		}

		// Output the indices
		cout << "==== INDICES ====\n" << endl;
		for (int i = 0; i < indices.size(); i+=3) {
			cout << indices[i] << " " << indices[i + 1] << " " << indices[i + 2] << endl;
		}
		
		cout << "# of indices: " << indices.size() << endl; // Output # of total indices

		// Put the points initially into draw_verts
		for (int i = 0; i < points.size(); i++) {
			draw_verts.push_back(points[i].x);
			draw_verts.push_back(points[i].y);
			draw_verts.push_back(points[i].z);
		}

		// Begin the rotation
		glm::mat4 trans; // Identity matrix
		for (int i = 0; i < spans; i++) {
			glm::vec4 tv4;
			trans = glm::rotate(trans, glm::radians((GLfloat)(360 / (spans - 1))), glm::vec3(0.0f, 0.0f, 1.0f)); // Perform rotation
			for (int j = 0; j < points.size(); j++) {
				tv4 = trans * glm::vec4(points[j], 1.0f);
				draw_verts.push_back(ceil(tv4.x * 100) / 100);
				draw_verts.push_back(ceil(tv4.y * 100) / 100);
				draw_verts.push_back(ceil(tv4.z * 100) / 100);
			}
		}

		// Output vertices
		cout << "\n==== VERTICES ====\n" << endl;
		for (int i = 0; i < draw_verts.size(); i+=3) {
			cout << draw_verts[i] << " " << draw_verts[i + 1] << " " << draw_verts[i + 2] << endl;
		}

		cout << "# of vertices: " << draw_verts.size() << endl; // Output # of total vertices
	}
	// ------- END ROTATIONAL SHAPES ------//

	// Set up buffers and objects
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// VAO
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);
	
	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, draw_verts.size() * sizeof(GLfloat), &draw_verts.front(), GL_STATIC_DRAW);

	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices.front(), GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		Do_Movement();

		// Clear the colorbuffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate Shader
		ourShader.Use();

		// --- CAMERA TRANSFORMATION --- //
		// Camera/View transformation
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		// Projection 
		projection = glm::perspective(fov, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
		// Bind VAO
		glBindVertexArray(VAO);

		// PRIMITIVES: GL_POINTS, GL_LINES, GL_TRIANGLES
		glDrawElements(GL_TRIANGLES, draw_verts.size() * 2, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		// Swap the buffers
		glfwSwapBuffers(window);
	}

	// Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

void window_size_callback(GLFWwindow* window, int width, int height) {
	//cout << "Window event called." << endl;
	// Set the new width and height of the window
	WIDTH = width;
	HEIGHT = height;
	// Re-initialize the viewport
	glViewport(0, 0, WIDTH, HEIGHT);
	// Re-calculate the perspective matrix
	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);
	// Re-calculate the view matrix
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	GLfloat cameraSpeed = 5.0f * deltaTime;
	if (keys[GLFW_KEY_UP])
		cameraPos += cameraSpeed * cameraFront;
	if (keys[GLFW_KEY_DOWN])
		cameraPos -= cameraSpeed * cameraFront;
	if (keys[GLFW_KEY_LEFT])
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (keys[GLFW_KEY_RIGHT])
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if ( key == GLFW_KEY_L && action == GLFW_PRESS)
		// WIREFRAME MODE
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else if (key == GLFW_KEY_F && action == GLFW_PRESS)
		// FILL MODE
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else if (key == GLFW_KEY_P && action == GLFW_PRESS)
		// POINTS MODE
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

bool firstMouse = true; // Track if it's the first time mouse enters the window

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
	lastX = xpos;
	lastY = ypos;

	GLfloat sensitivity = 0.05;	// Change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

}

// Handle scrolling in and out of shape
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}