//COMP 371 Assignment 1 || Jason Kalec(40009464)

#include "..\glew\glew.h"	// include GL Extension Wrangler
#include "..\glfw\glfw3.h"	// include GLFW helper library
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

const GLint WIDTH = 800, HEIGHT = 800;
const float TRIANGLE_MOVEMENT_STEP = 0.1f;

glm::vec3 rotationStep;
glm::vec3 moveCamera(0.0f, 0.0f, -5.0f);

std::vector<GLuint> indices;
std::vector<GLfloat> vertices;

int polygonMode = GL_TRIANGLES;
bool mouseButtonDown = false;
double previousY;
double previousX;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (GLFW_PRESS == action)
		{
			glfwGetCursorPos(window, &previousX, &previousY);
			mouseButtonDown = true;
		}
		else if (GLFW_RELEASE == action)
			mouseButtonDown = false;
	}
}

//escape key close window
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		rotationStep.z -= TRIANGLE_MOVEMENT_STEP;
	else if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		rotationStep.z += TRIANGLE_MOVEMENT_STEP;
	else if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
		rotationStep.x -= TRIANGLE_MOVEMENT_STEP;
	else if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
		rotationStep.x += TRIANGLE_MOVEMENT_STEP;
	else if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		polygonMode = GL_POINT;
	}
	else if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		polygonMode = GL_LINE;
	}
	else if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{
		polygonMode = GL_FILL;
	}	
}

static void enableMouseZoom(GLFWwindow* window)
{
	if (mouseButtonDown)
	{
		double x;
		double y;
		glfwGetCursorPos(window, &x, &y);

		if (previousY - y > 0)
		{
			moveCamera.z -= TRIANGLE_MOVEMENT_STEP;
			previousY = y;
		}
		else if (previousY - y < 0)
		{
			moveCamera.z += TRIANGLE_MOVEMENT_STEP;
			previousY = y;
		}
	}
}

static void translateOrRotate()
{
	ifstream input("input_a1.txt");
	int translateOrRotate = -1;
	input >> translateOrRotate;

	//TRANSLATION SWEEP START
	if (translateOrRotate == 0)
	{
		//READ FILE START
		int trajectorySize = 0;
		int profileSize = 0;
		input >> trajectorySize;
		std::vector<GLfloat> trajectory;
		for (int i = 0; i < trajectorySize * 3; i++)
		{
			GLfloat toInsert;
			input >> toInsert;
			trajectory.push_back(toInsert);
			vertices.push_back(toInsert);
		}

		input >> profileSize;
		std::vector<GLfloat> profile;
		for (int j = 0; j < profileSize * 3; j++)
		{
			GLfloat toInsert;
			input >> toInsert;
			profile.push_back(toInsert);
		}
		//READ FILE END

		glm::vec3 moveValues(trajectory[0] - profile[0],
			trajectory[1] - profile[1],
			trajectory[2] - profile[2]);

		for (int i = 0; i < profile.size(); i++) // move profile line into correct place
		{
			if (i % 3 == 0)
			{
				profile[i] += moveValues.x; //move x value
			}
			else if (i % 3 == 1)
			{
				profile[i] += moveValues.y; //move y value
			}
			else if (i % 3 == 2)
			{
				profile[i] += moveValues.z; //move z value
			}
		}

		for (int i = 1; i < profileSize; i++) //sweep across and add vertices to array
		{
			glm::vec3 moveValues2(profile[i * 3] - trajectory[0],
				profile[1 + i * 3] - trajectory[1],
				profile[2 + i * 3] - trajectory[2]);

			GLfloat toInsert;
			for (int j = 0; j < trajectory.size(); j++)
			{
				if (j % 3 == 0)
				{
					toInsert = trajectory[j] + moveValues2.x; //move x value
				}
				else if (j % 3 == 1)
				{
					toInsert = trajectory[j] + moveValues2.y; //move y value
				}
				else if (j % 3 == 2)
				{
					toInsert = trajectory[j] + moveValues2.z; // move z value
				}
				vertices.push_back(toInsert); //add to vertices array
			}
		}

		for (int i = 0; i < (trajectorySize * (profileSize - 1)) - 1; i++) //populate indices array
		{
			if (i%trajectorySize != trajectorySize - 1)
			{
				indices.push_back(i);
				indices.push_back(i + 1);
				indices.push_back(i + trajectorySize);
				indices.push_back(i + 1);
				indices.push_back(i + trajectorySize);
				indices.push_back(i + 1 + trajectorySize);
			}
		}
	}
	//TRANSLATION SWEEEP END

	//ROTATION SWEEP START
	else if (translateOrRotate == 1)
	{
		//READ FILE START
		int rotatePrecision = 0;
		int curveSize = 0;
		input >> rotatePrecision;
		input >> curveSize;
		std::vector<GLfloat> curve;
		for (int i = 0; i < curveSize * 3; i++)
		{
			GLfloat toInsert;
			input >> toInsert;
			curve.push_back(toInsert);
			vertices.push_back(toInsert);
		}
		//READ FILE END

		float const PI = 3.141592653589793238462f;

		float rotateAdd = (2 * PI) / rotatePrecision;
		for (int i = 1; i <= rotatePrecision; i++) //rotate using sin & cos and populate vertices array
		{
			GLfloat toInsert;
			GLfloat multiplier;
			for (int j = 0; j < curve.size(); j++)
			{
				if (j % 3 == 0)
				{
					multiplier = curve[j];
					toInsert = multiplier * cos(rotateAdd*i); //move x value
				}
				else if (j % 3 == 1)
				{
					toInsert = multiplier * sin(rotateAdd*i); //move y value
				}
				else if (j % 3 == 2)
				{
					toInsert = curve[j]; //z values do not move
				}
				vertices.push_back(toInsert); //add to vertices array
			}
		}

		for (int i = 0; i < (rotatePrecision * curveSize) - 1; i++) //populate indices array
		{
			if (i%curveSize != curveSize - 1)
			{
				indices.push_back(i);
				indices.push_back(i + 1);
				indices.push_back(i + curveSize);
				indices.push_back(i + 1);
				indices.push_back(i + curveSize);
				indices.push_back(i + 1 + curveSize);
			}
		}
	}
	//ROTATION SWEEP END
}

int main()
{
	//GLFW STUFF START
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "COMP 371 Assignment 1: Jason Kalec(40009464)", nullptr, nullptr);

	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	if (nullptr == window)
	{
		std::cout << "Failed to Create GLFW Window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);
	//GLFW STUFF END

	//GLEW STUFF START
	glewExperimental = GL_TRUE;

	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;

		return EXIT_FAILURE;
	}
	//GLEW STUFF END

	glViewport(0, 0, screenWidth, screenHeight);

	glEnable(GL_DEPTH_TEST);

	//GRAB VERTEX SHADER STUFF FROM FILE START
	string vertex_shader_path = "vertex.shader";
	string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_shader_path, ios::in);

	if (VertexShaderStream.is_open()) {
		string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ?\n", vertex_shader_path.c_str());
		getchar();
		exit(-1);
	}
	//GRAB VERTEX SHADER STUFF FROM FILE END

	//GRAB FRAGMENT SHADER STUFF FROM FILE START
	string fragment_shader_path = "fragment.shader";
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);

	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory?\n", fragment_shader_path.c_str());
		getchar();
		exit(-1);
	}
	//GRAB FRAGMENT SHADER STUFF FROM FILE END

	//SET UP VERTEX SHADER START
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(vertexShader, 1, &VertexSourcePointer, NULL);
	glCompileShader(vertexShader);
	// Check for compile time errors
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	//SET UP VERTEX SHADER END

	//SET UP FRAGMENT SHADER START
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(fragmentShader, 1, &FragmentSourcePointer, NULL);
	glCompileShader(fragmentShader);
	// Check for compile time errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	//SET UP FRAGMENT SHADER END

	//LINK SHADERS START
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader); 
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgram);
	//LINK SHADERS END

	translateOrRotate(); //ASSIGNMENT 1 TRANSLATE & ROTATE SWEEP FUNCTION!!!

	//VERTEX ARRAY OBJECT AND VERTEX BUFFER OBJECT STUFF START
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices.front(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices.front(), GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
	//VERTEXT ARRAY OBJECT AND VERTEX BUFFER OBJECT STUFF END

	glm::mat4 projection;
	projection = glm::perspective(45.0f, (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 1000.0f);
	
	//MAIN LOOP
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		glm::mat4 model;
		glm::mat4 view;
		view = glm::lookAt(	glm::vec3(0.0f, 0.0f, moveCamera.z),	//camera positioned here
							glm::vec3(0.0f, 0.0f, 0.0f),			//looks at origin
							glm::vec3(0.0f, 1.0f, 0.0f));
		view = glm::rotate(view, rotationStep.x * 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		view = glm::rotate(view, rotationStep.z * 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));

		//RESIZE HANDLING START
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		//RESIZE HANDLING END

		enableMouseZoom(window);

		GLint modelLocation = glGetUniformLocation(shaderProgram, "model");
		GLint viewLocation = glGetUniformLocation(shaderProgram, "view");
		GLint projectionLocation = glGetUniformLocation(shaderProgram, "projection");

		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO);
		glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
		glDrawElements(GL_TRIANGLES, indices.size() , GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;

}