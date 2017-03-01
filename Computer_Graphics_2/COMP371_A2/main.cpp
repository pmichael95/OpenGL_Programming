/*
* COMP371 - Computer Graphics
* Assignment #2
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
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "Shader.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "Camera.h"

using namespace std;

// Function prototypes
void window_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void Do_Movement();
void do_Spline(bool isCPoints);
void Subdivision(float u0, float u1, glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, glm::vec3 &p4, bool isCPoints);
glm::vec3 getSplinePoint(float u, glm::mat3x4 &control_matrix);
void doTrans(vector<glm::vec3> &points, vector<glm::vec3> &trajectory, vector<GLfloat> &draw_verts, vector<GLuint> &indices, int &width, int &height);
void doRotate(vector<glm::vec3> &points, vector<GLuint> &indices, int &spans, vector<GLfloat> &draw_verts);
void fillVBO(vector<GLfloat> &input);
void drawFunc(vector<GLfloat> &input);

// CONSTANT STEP AMOUNT FOR CAMERA
const GLfloat MOV_STEP = 0.1f;
// CONSTANT AMOUNT OF MAXIMUM NEEDED ENTERS (translational case)
const int MAX_NUM_ENTERS = 4;
// CONSTANT FOR THE MAXIMUM LINE LENGTH 
const float MAX_LINE_LENGTH = 0.1f;

// MVP Matrices
glm::mat4 view;
glm::mat4 projection;
glm::mat4 model;

// Model, view, & projection for shaders
GLint modelLoc;
GLint viewLoc;
GLint projLoc;

// Window dimensions
GLuint WIDTH = 800, HEIGHT = 800;

// Camera
glm::vec3 myCamera = glm::vec3(0.0f, 0.0f, 5.0f);
GLfloat fov = 45.0f; // 45 degrees of FOV
glm::vec3 step; // Step size on camera
bool keys[1024];

GLuint VBO, VAO, EBO; // Make the VBO, VAO, and EBO global for use in other methods

bool pressed_enter[MAX_NUM_ENTERS] = { false, false, false, false }; // When the user presses 'Enter', we draw the spline if it's 'R', otherwise need trajectory if 'T'
bool finished_spline = false; // Signal that we're finished with spline curves and can draw the mesh
bool backspace = false; // Bool value to track if we decided to wipe mesh
bool ready = false; // Bool value to signal if we're ready to draw mesh (file I/O, etc.)
bool donecp = false; // Bool value to track if we're done with control points, to move to trajectory points
vector<GLfloat> cpoints, controlpoints, tpoints, trajectorypoints; // Vector of GLfloats to hold our control points. This will take 3 values at each 'push', so an x, y, and z value
																   // controlpoints and trajectorypoints hold orthographic coordinates with z = 0
																   // cpoints and tpoints hold points in [-1, 1] with y = 0
vector<GLfloat> splined_cpoints, splined_tpoints; // cpoints or tpoints splined (holds all data to draw our splined curves)
char choice; // Choice holder, R for rotational, T for translational
char type = 'P'; // Type of drawing for points (GL_LINE_STRIP or GL_POINTS), defaulted to points

// Change the projection style based on what we're drawing
void chooseDisplayMode() {
	// If done with splines and ready to draw (all I/O and whatnot completed)
	if (finished_spline && ready)
		projection = glm::perspective(fov, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
	else // Else if we still are dealing with points drawing and splines
		projection = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, 0.0f, 100.0f);
}

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
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "COMP371 Assignment #2 - Philip Michael", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;

	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST); // Depth feeling

	// Set higher point size
	glPointSize(5.0f);

	// Build and compile our shader program
	Shader ourShader("vertex.shader", "fragment.shader");

	// === BEGIN USER INPUT PROCESSING === //
	bool correct_input = false; // Handle proper input
	do {
		cout << "Would you like to draw a translational sweep (T) or rotational sweep (R)? Please input your choice." << endl;
		cin >> choice;
		if (toupper(choice) == 'R' || toupper(choice) == 'T')
			correct_input = true;
		else {
			cout << "\nUnknown input. Please enter T for translational sweeps or R for rotational sweeps." << endl;
		}
	} while (!correct_input);

	// Output formatted
	cout << "You've decided to perform ";
	if (toupper(choice) == 'T')
		cout << "translational shapes." << endl;
	else
		cout << "rotational shapes." << endl;
	// === END USER INPUT PROCESSING === //

	// Set up some necessary variables
	vector<glm::vec3> points, trajectory; // Vec3 objects stored for our pts and trajectories
	vector<GLfloat> draw_verts; // The drawable vertices
	vector<GLuint> indices; // Vector object to hold the indices we pass to drawing the elements
	bool empty = false; // False if we have translational shape, true otherwise
	bool read = false; // False if we have yet to read the input file
	int width = 0, height = 0; // Spans: # of rotations; Width: Trajectory size; Height: Points size
	int spans = 0;

	// Set up buffers and objects
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// VAO
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	// Bind the VBO so we can use it to draw the spline points
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
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
		view = glm::lookAt(glm::vec3(0.0f, 0.0f, myCamera.z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		view = glm::rotate(view, step.x * 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		view = glm::rotate(view, step.z * 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		// Projection 
		chooseDisplayMode();
		// Get the uniform locations
		modelLoc = glGetUniformLocation(ourShader.Program, "model");
		viewLoc = glGetUniformLocation(ourShader.Program, "view");
		projLoc = glGetUniformLocation(ourShader.Program, "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// -- IF SPLINES ARE DONE, TIME TO READ FILE AND DRAW! -- //
		if (finished_spline && !ready) {
			// Pass the above variables to the FileReader.h function 'readInput' to process the input file
			readInput("input_a2.txt", points, trajectory, empty, spans, height, width);
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
			cout << "DONE READING. Points size: " << points.size() << ", Trajectory size: " << trajectory.size() << ", width: " << width << ", height: " << height << endl;
			// ------- BEGIN TRANSLATIONAL SHAPES -------//
			if (!empty) {
				doTrans(points, trajectory, draw_verts, indices, width, height);
			}
			// ------- END TRANSLATIONAL SHAPES -------//
			////////////////////////////////////////////
			// ------- BEGIN ROTATIONAL SHAPES ------//
			else {
				doRotate(points, indices, spans, draw_verts);
			}
			// ------- END ROTATIONAL SHAPES ------//
			ready = true;

		} // end global if

		if (ready && !backspace) { // IF READY TO DRAW THE SHAPE MESHES
								   // VBO
			fillVBO(draw_verts);

			// EBO
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices.front(), GL_STATIC_DRAW);

			// Position attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			glBindVertexArray(0); // Unbind VAO
								  // Bind VAO
			glBindVertexArray(VAO);

			// PRIMITIVES: GL_POINTS, GL_LINES, GL_TRIANGLES
			glDrawElements(GL_TRIANGLES, draw_verts.size() * 2, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
		else if (!finished_spline) {
			if (toupper(choice) == 'T') {
				if (controlpoints.size() != 0 && !donecp) {
					// Handle the control point drawing
					glBindVertexArray(VAO);
					glBindBuffer(GL_ARRAY_BUFFER, VBO);
					// VBO
					fillVBO(controlpoints);

					// Position attribute
					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
					glEnableVertexAttribArray(0);

					glBindVertexArray(0); // Unbind VAO

										  // Bind VAO
					glBindVertexArray(VAO);

					// PRIMITIVES: GL_POINTS, GL_LINES, GL_TRIANGLES
					drawFunc(cpoints);
				}
				else if (trajectorypoints.size() != 0 && donecp) {
					// Handle trajectory drawing
					glBindVertexArray(VAO);
					glBindBuffer(GL_ARRAY_BUFFER, VBO);
					// VBO
					fillVBO(trajectorypoints);

					// Position attribute
					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
					glEnableVertexAttribArray(0);

					glBindVertexArray(0); // Unbind VAO

										  // Bind VAO
					glBindVertexArray(VAO);

					// PRIMITIVES: GL_POINTS, GL_LINES, GL_TRIANGLES
					drawFunc(tpoints);
				}
			} // end if with choice
			else if (toupper(choice) == 'R' && controlpoints.size() != 0) {
				// Draw the control points
				glBindVertexArray(VAO);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				// VBO
				fillVBO(controlpoints);

				// Position attribute
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(0);

				glBindVertexArray(0); // Unbind VAO

									  // Bind VAO
				glBindVertexArray(VAO);

				// PRIMITIVES: GL_POINTS, GL_LINES, GL_TRIANGLES
				drawFunc(cpoints);
			}
		} // end else if with finished_spline


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
	// Set the new width and height of the window
	WIDTH = width;
	HEIGHT = height;
	// Re-initialize the viewport
	glViewport(0, 0, WIDTH, HEIGHT);
}

// Rotates model
void Do_Movement()
{
	if (keys[GLFW_KEY_RIGHT]) {
		step.z -= MOV_STEP;
	}
	else if (keys[GLFW_KEY_LEFT]) {
		step.z += MOV_STEP;
	}
	else if (keys[GLFW_KEY_DOWN]) {
		step.x -= MOV_STEP;
	}
	else if (keys[GLFW_KEY_UP]) {
		step.x += MOV_STEP;
	}
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if (key == GLFW_KEY_L && action == GLFW_PRESS && !finished_spline)
		type = 'L';
	else if (key == GLFW_KEY_P && action == GLFW_PRESS && !finished_spline)
		type = 'P';
	else if (key == GLFW_KEY_L && action == GLFW_PRESS && finished_spline)
		// WIREFRAME MODE
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else if (key == GLFW_KEY_F && action == GLFW_PRESS)
		// FILL MODE
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else if (key == GLFW_KEY_P && action == GLFW_PRESS)
		// POINTS MODE
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		if (toupper(choice) == 'R' && !pressed_enter[0]) {
			// Handle the rotation drawing now
			pressed_enter[0] = true; // Signal that now we want to do the spline (pressed enter the first time)
			pressed_enter[1] = true; // Signal that the next step is to draw the mesh when enter is pressed
			// --- NEED TO POPULATE CPOINTS WITH THE SPLINE POINTS --- //
			do_Spline(true);
			// ------------------------------------------------------ //
		}
		else if (toupper(choice) == 'R' && pressed_enter[1]) {
			writeInput(cpoints, tpoints, choice, finished_spline); // Call FileWriter to write
		}
		else if (toupper(choice) == 'T' && !pressed_enter[0]) {
			pressed_enter[0] = true; // Signal that now we want to do the spline (pressed enter the first time)
			pressed_enter[1] = true; // Signal that the next step is to draw the trajectory points (when we press enter)
			// --- NEED TO POPULATE CPOINTS WITH THE SPLINE POINTS --- //
			do_Spline(true);
			// ------------------------------------------------------ //
		}
		else if (toupper(choice) == 'T' && pressed_enter[1]) {
			// We drew the cpoints spline
			pressed_enter[1] = false;
			pressed_enter[2] = true; // Signal that the next step is to draw the next set of points
		}
		else if (toupper(choice) == 'T' && pressed_enter[2]) {
			pressed_enter[2] = false;
			pressed_enter[3] = true; // Signal that next step is to write to file
			// --- NEED TO POPULATE TPOINTS WITH THE SPLINE POINTS --- //
			do_Spline(false);
			// ------------------------------------------------------ //
		}
		else if (toupper(choice) == 'T' && pressed_enter[3]) {
			// We drew the tpoints spline
			// Now need to populate file
			writeInput(cpoints, tpoints, choice, finished_spline); // Call FileWriter to write
		}
	}
	else if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
		backspace = true; // Wipe drawing
	}
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

// Function that builds the spline (need Subdivide for part of it)
void do_Spline(bool isCPoints) {
	cout << "\ncpoints.size() = " << cpoints.size() << "\ntpoints.size() = " << tpoints.size() << endl;
	glm::vec3 p1, p2, p3, p4;
	if (isCPoints) {
		// Case for control points (first input set)
		// We need AT LEAST 4 sets control points (12 pts total)
		if ((cpoints.size() / 3) >= 4) {
			for (int i = 0; i < cpoints.size(); i += 3) {
				if ( (i + 11) <= cpoints.size()) {
					cout << "\n~~~ IN IF ~~~\n" << endl;
					// Set the points to pass to Subdivide
					p1 = glm::vec3(cpoints[i], cpoints[i + 1], cpoints[i + 2]); // 1st point
					p2 = glm::vec3(cpoints[i + 3], cpoints[i + 4], cpoints[i + 5]); // 2nd point
					p3 = glm::vec3(cpoints[i + 6], cpoints[i + 7], cpoints[i + 8]); // 3rd point
					p4 = glm::vec3(cpoints[i + 9], cpoints[i + 10], cpoints[i + 11]); // 4th point
					// Now use these points to spline using the subdivision algorithm
					Subdivision(0.0f, 1.0f, p1, p2, p3, p4, isCPoints);
				}
			}
			// For debugging, output the end result of the values of splined_cpoints
			cout << "=== PRINTING SPLINED_CPOINTS ===" << endl;
			cout << "splined_cpoints.size() = " << splined_cpoints.size() << endl;
			for (int i = 0; i < splined_cpoints.size(); i += 3) {
				cout << splined_cpoints[i] << " " << splined_cpoints[i + 1] << " " << splined_cpoints[i + 2] << endl;
			}

			// Now we need to convert it back into controlpoints (add it to it) but with orthographic dimensions (the x * WIDTH, the z * HEIGHT -> y)
			for (int i = 0; i < splined_cpoints.size(); i += 3) {
				controlpoints.push_back(floor(abs(splined_cpoints[i]) * WIDTH)); // Push the x value in orthographic
				controlpoints.push_back(floor(abs(splined_cpoints[i + 2]) * HEIGHT)); // Push the z value as the y value in orthographic
				controlpoints.push_back(0.0f); // Push a z-value of 0
			}


			cout << "=== PRINTING CONTROLPOINTS ===" << endl;
			for (int i = 0; i < controlpoints.size(); i += 3) {
				// Output controlpoints for debugging
				cout << controlpoints[i] << " " << controlpoints[i + 1] << " " << controlpoints[i + 2] << endl;
			}


			// Now push the splined points into cpoints
			for (int i = 0; i < splined_cpoints.size(); i += 3) {
				cpoints.push_back(splined_cpoints[i]);
				cpoints.push_back(splined_cpoints[i + 1]); // make y = 0
				cpoints.push_back(splined_cpoints[i + 2]);
			}
		}
		else {
			cerr << "ERROR::INSUFFICIENT_NUMBER_OF_CONTROL_POINTS_PROVIDED" << endl;
		}
	}
	else {
		// Trajectory points case, very similar
		if ((tpoints.size() / 3) >= 4) {
			for (int i = 0; i < tpoints.size(); i += 3) {
				if ((i + 11) <= tpoints.size()) {
					// Set the points to pass to Subdivide
					p1 = glm::vec3(tpoints[i], tpoints[i + 1], tpoints[i + 2]); // 1st point
					p2 = glm::vec3(tpoints[i + 3], tpoints[i + 4], tpoints[i + 5]); // 2nd point
					p3 = glm::vec3(tpoints[i + 6], tpoints[i + 7], tpoints[i + 8]); // 3rd point
					p4 = glm::vec3(tpoints[i + 9], tpoints[i + 10], tpoints[i + 11]); // 4th point
					// Now use these points to spline using the subdivision algorithm
					Subdivision(0.0f, 1.0f, p1, p2, p3, p4, isCPoints);
				}
			}
			// For debugging, output the end result of the values of splined_cpoints
			cout << "=== PRINTING SPLINED_TPOINTS ===" << endl;
			cout << "splined_tpoints.size() = " << splined_tpoints.size() << endl;
			for (int i = 0; i < splined_tpoints.size(); i += 3) {
				cout << splined_tpoints[i] << " " << splined_tpoints[i + 1] << " " << splined_tpoints[i + 2] << endl;
			}

			// Now we need to convert it back into controlpoints (add it to it) but with orthographic dimensions (the x * WIDTH, the z * HEIGHT -> y)
			for (int i = 0; i < splined_tpoints.size(); i += 3) {
				trajectorypoints.push_back(floor(abs(splined_tpoints[i]) * WIDTH)); // Push the x value in orthographic
				trajectorypoints.push_back(floor(abs(splined_tpoints[i + 1]) * HEIGHT)); // Push the y value in orthographic
				trajectorypoints.push_back(0.0f); // Push a z-value of 0
			}

			cout << "=== PRINTING TRAJECTORYPOINTS ===" << endl;
			for (int i = 0; i < trajectorypoints.size(); i += 3) {
				// Output controlpoints for debugging
				cout << trajectorypoints[i] << " " << trajectorypoints[i + 1] << " " << trajectorypoints[i + 2] << endl;
			}

			for (int i = 0; i < splined_tpoints.size(); i += 3) {
				tpoints.push_back(splined_tpoints[i]);
				tpoints.push_back(splined_tpoints[i + 1]);
				tpoints.push_back(splined_tpoints[i + 2]); // Make z value 0
			}
		}
		else {
			cerr << "ERROR::INSUFFICIENT_NUMBER_OF_TRAJECTORY_POINTS_PROVIDED" << endl;
		}
	}
}

// Subdivision algorithm
void Subdivision(float u0, float u1, glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, glm::vec3 &p4, bool isCPoints) {
	// Takes in u0, u1, and a max line threshhold, and 4 points 
	/* PSEUDOCODE
	umid = (u0 + u1) / 2
	vec x0 = f(u0) // Point on spline
	vec x1 = f(u1) // Second point on spline
	if( | x1 - x0 | > maxLineLength){
	Subdivide(u0, umid, maxLineLength)
	Subdivide(umid, u1, maxLineLength)
	}
	else
	drawLine(x0, x1) // Draw a line from the first point to the second (AKA just draw the points)
	// This means we push them to the splined_cpoints vector object
	*/

	// Find the umid
	float umid = (u0 + u1) / 2; 
	
	// Build the control matrix based on our input points
	GLfloat cMatrix[12] = { p1.x, p2.x, p3.x, p4.x, p1.y, p2.y, p3.y, p4.y, p1.z, p2.z, p3.z, p4.z };
	glm::mat3x4 control_matrix = glm::make_mat3x4(cMatrix);

	// Acquire the 2 points on the spline
	glm::vec3 x0 = getSplinePoint(u0, control_matrix);
	glm::vec3 x1 = getSplinePoint(u1, control_matrix);

	// Check distances between the 2 points
	if (abs(glm::distance(x1, x0)) > MAX_LINE_LENGTH) {
		Subdivision(u0, umid, p1, p2, p3, p4, isCPoints);
		Subdivision(umid, u1, p1, p2, p3, p4, isCPoints);
	}
	else {
		// If they are control points...
		if (isCPoints) {
			splined_cpoints.push_back(x0.x);
			splined_cpoints.push_back(x0.y);
			splined_cpoints.push_back(x0.z);
			splined_cpoints.push_back(x1.x);
			splined_cpoints.push_back(x1.y);
			splined_cpoints.push_back(x1.z);
		}
		// If they are trajectory points...
		else {
			splined_tpoints.push_back(x0.x);
			splined_tpoints.push_back(x0.y);
			splined_tpoints.push_back(x0.z);
			splined_tpoints.push_back(x1.x);
			splined_tpoints.push_back(x1.y);
			splined_tpoints.push_back(x1.z);
		}
	}
}

// Returns a spline point on curve
glm::vec3 getSplinePoint(float u, glm::mat3x4 &control_matrix) {
	// Set up the parametric vector
	glm::vec4 parametric = glm::vec4(pow(u, 3), pow(u, 2), u, 1); 

	// Output the parametric
	cout << "u_vec with u = " << u << " gave x= " << parametric.x << ", y= " << parametric.y << ", z=" << parametric.z << ", w= " << parametric.w << endl;

	// Set the basis matrix
	static GLfloat bMatrix[16] = { -0.5, 1, -0.5, 0, 1.5, -2.5, 0, 1, -1.5, 2, 0.5, 0, 0.5, -0.5, 0, 0 };
	static glm::mat4 basis_matrix = glm::make_mat4(bMatrix);

	// Transpose the matrices
	static glm::mat4 trans_basis = glm::transpose(basis_matrix);
	glm::mat4 trans_cont = glm::transpose(control_matrix);

	//Return result
	return glm::vec3((trans_cont * trans_basis) * parametric);
}

// Sets the VBO values
void fillVBO(vector<GLfloat> &input) {
	glBufferData(GL_ARRAY_BUFFER, input.size() * sizeof(GLfloat), &input.front(), GL_STATIC_DRAW);
}

// Calls DrawArrays to change type of input
void drawFunc(vector<GLfloat> &input) {
	switch (type) {
	case 'P': glDrawArrays(GL_POINTS, 0, input.size() / 3);
		break;
	case 'L': glDrawArrays(GL_LINE_STRIP, 0, input.size() / 3);
		break;
	default: glDrawArrays(GL_POINTS, 0, input.size() / 3);
	}
}

// Scroll wheel control
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset / 5; // Zoom in
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

// Mouse clicking
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !ready) {
		// If we clicked on the window to mark a control point
		// We check with ! ready to make sure we didn't already draw the shape, so no need for points
		double xpos, ypos; // We will need these to handle the control points
		glfwGetCursorPos(window, &xpos, &ypos); // At this point, xpos and ypos are populated
		cout << "xpos = " << xpos << ", ypos = " << ypos << endl;
		if (toupper(choice) == 'T' && !pressed_enter[0]) {
			// TRANSLATION CASE
			// We populate the cpoints
			cpoints.push_back(xpos / WIDTH);
			cout << xpos / WIDTH << endl; // Test to get the value
			cpoints.push_back(0.0f); // Y-val is 0 for all cpoints
			cpoints.push_back(ypos / HEIGHT); // The z-val is the y-val
			cout << ypos / HEIGHT << endl; // Test to get the value

			// -- NEED TO DRAW A POINT AT EACH CALL --//
			// Add actual values of window dimension relatives to controlpoints
			controlpoints.push_back(xpos);
			controlpoints.push_back(ypos);
			controlpoints.push_back(0.0f);

			cout << "=== IN CALLBACK, PRINTING CONTROLPOINTS ===" << endl;
			for (int i = 0; i < controlpoints.size(); i += 3) {
				cout << controlpoints[i] << " " << controlpoints[i + 1] << " " << controlpoints[i + 2] << endl;
			}
		}
		else if (toupper(choice) == 'T' && pressed_enter[0]) {
			// TRANSLATION CASE
			donecp = true; // Signal that we finished with control points
						   // We populate the tpoints
			tpoints.push_back(xpos / WIDTH);
			cout << xpos / WIDTH << endl; // Test to get the value
			tpoints.push_back(ypos / HEIGHT); // The y-val
			tpoints.push_back(0.0f); // Z-val is 0 for all cpoints
			cout << ypos / HEIGHT << endl; // Test to get the value

			// -- NEED TO DRAW A POINT AT EACH CALL --//
			// Add actual values of window dimension relatives to controlpoints
			trajectorypoints.push_back(xpos);
			trajectorypoints.push_back(ypos);
			trajectorypoints.push_back(0.0f);
		}
		else if (toupper(choice) == 'R' && !pressed_enter[0]) {
			// ROTATION CASE
			// We populate the cpoints
			cpoints.push_back(xpos / WIDTH);
			cout << xpos / WIDTH << endl; // Test to get the value
			cpoints.push_back(0.0f); // Y-val is 0 for all cpoints 
			cpoints.push_back(ypos / HEIGHT); // The z-val is the y-val
			cout << ypos / HEIGHT << endl; // Test to get the value

										   // -- NEED TO DRAW A POINT AT EACH CALL --//
										   // Add actual values of window dimension relatives to controlpoints (to draw points)
			controlpoints.push_back(xpos);
			controlpoints.push_back(ypos);
			controlpoints.push_back(0.0f);
		}
	}
}

// Handle the translation
void doTrans(vector<glm::vec3> &points, vector<glm::vec3> &trajectory, vector<GLfloat> &draw_verts, vector<GLuint> &indices, int &width, int &height) {
	// Output trajectory curve
	cout << "\n=== TRAJECTORY CURVE ===\n";
	cout << trajectory.size();
	cout << points.size();

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
	cout << "# of indices: " << indices.size() << endl;

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
	cout << "# of vertices: " << draw_verts.size() << endl;
}

// Handle the rotation
void doRotate(vector<glm::vec3> &points, vector<GLuint> &indices, int &spans, vector<GLfloat> &draw_verts) {
	// Put the points initially into draw_verts
	vector<GLfloat> verts;
	for (int i = 0; i < points.size(); i++) {
		draw_verts.push_back(points[i].x);
		draw_verts.push_back(points[i].y);
		draw_verts.push_back(points[i].z);
		verts.push_back(points[i].x);
		verts.push_back(points[i].y);
		verts.push_back(points[i].z);
	}

	float rotateAdd = (2 * glm::pi<float>()) / spans;
	for (int i = 1; i <= spans; i++) //rotate using sin & cos and populate vertices array
	{
		GLfloat inVal;
		GLfloat multVal;
		for (int j = 0; j < verts.size(); j++)
		{
			if (j % 3 == 0)
			{
				multVal = verts[j];
				inVal = multVal * cos(rotateAdd*i); //move x value
			}
			else if (j % 3 == 1)
			{
				inVal = multVal * sin(rotateAdd*i); //move y value
			}
			else if (j % 3 == 2)
			{
				inVal = verts[j]; //z values do not move
			}
			draw_verts.push_back(inVal); //add to vertices array
		}
	}

	// FIND INDICES
	for (int i = 0; i < (spans * points.size()) - 1; i++) {
		if (i % points.size() != points.size() - 1) { // Check so we don't go out of bounds
			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + points.size());
			indices.push_back(i + 1);
			indices.push_back(i + points.size());
			indices.push_back(i + 1 + points.size());
		}
	}
	cout << "# of vertices: " << draw_verts.size() << endl;
}