#ifndef FILEREADER_H
#define FILEREADER_H

#pragma warning(disable:4996)

#include <string>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

void readInput(const GLchar* fpath, vector<glm::vec3> &points, vector<glm::vec3> &trajectory, bool &empty, int &spans, int &width, int &height) {
	/*
	* Input: fpath - file path
	* Input: vec1 & vec2 - vector which will hold all the vertices, vec1 is profile curve, vec2 is trajectory curve
	* Input: empty - initially passed as false, if it returns true then we only have vec1, & vec2 is empty
	* Input: spans - the number of spans (only for rotational surfaces, which would ALSO return empty = true)
	*/

	FILE * file;
	int firstline, currline;
	GLfloat currval;
	glm::vec3 vertex;

	try {
		// Try to open the file
		file = fopen(fpath, "r");

		if (file == NULL) {
			cout << "ERROR::FILE_IS_EMPTY" << endl;
		}
		else {
			// Step 1: Read first int 
			// Check 0 or 1
			fscanf(file, "%d\n", &firstline);

			if (firstline == 0) {
				// This is the case for translational surfaces
				cout << "Read (case 1) " << firstline << endl;
				// Next line will have an integer representing the # of points for the profile curve
				// After those points there will be another integer for the # of points for the trajectory curve 
				fscanf(file, "%d\n", &currline); // Read next integer (width)
				width = currline;
				// At this point, currline holds an int value (1 to ?)
				// Must go through, line by line, adding to the points the rows UNTIL we finish based on currline
				// Each row will have 3 float values
				//cout << "\nFIRST SET\n" << endl;
				for (int i = 0; i < currline; i++) {
					fscanf(file, "%f", &vertex.x);
					fscanf(file, "%f", &vertex.y);
					fscanf(file, "%f", &vertex.z);
					points.push_back(vertex);
				}
				// Now we need to read another integer and process similarly points based on it
				// We will overwrite currline
				fscanf(file, "\n"); // New line to move onto the next
				fscanf(file, "%d\n", &currline); // Read height
				height = currline;
				//cout << "\nSECOND SET\n" << endl;
				// Now need to push these into trajectory
				for (int i = 0; i < currline; i++) {
					fscanf(file, "%f", &vertex.x);
					fscanf(file, "%f", &vertex.y);
					fscanf(file, "%f", &vertex.z);
					trajectory.push_back(vertex);
				}

				// ---- END FOR TRANSLATIONAL SURFACES ---- //

			}
			else {
				// This is all other cases (rotational surfaces)
				cout << "Read (case 2) " << firstline << endl;
				fscanf(file, "%d\n", &spans);

				// Here on, next int read will tell us the # of point sets for the profile curve
				fscanf(file, "%d\n", &currline);
				for (int i = 0; i < currline; i++) {
					fscanf(file, "%f", &vertex.x);
					fscanf(file, "%f", &vertex.y);
					fscanf(file, "%f", &vertex.z);
					points.push_back(vertex);
				}

				empty = true; // Set it to true since we only had one batch of points (no trajectory curve)
							  // ---- END FOR ROTATIONAL SURFACES ---- //
			}
		}

		// Close file stream
		fclose(file);
	}
	catch (exception e) {
		cout << "ERROR::INPUT_FILE_NOT_READ" << endl;
	}
}

#endif