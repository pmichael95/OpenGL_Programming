#ifndef FILEWRITER_H
#define FILEWRITER_H

// Standard includes
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h> // Include GLEW

using namespace std;

void writeInput(vector<GLfloat> &cpoints, vector<GLfloat> &tpoints, char type, bool &finished_spline) {
	// We take in the control points, and the trajectory points
	// We also take in the type of drawing to be done, which will change what we output to files

	ofstream file;
	file.open("input_a2.txt"); // Open the file. This overwrites it in the process of writing to it. 

	// Now need to check which type of drawing we're performing
	if (toupper(type) == 'R') {
		// ROTATIONAL SHAPES
		file << "1\n"; // 1 to symbolize rotational instead of translational
		file << "70\n"; // Default to 70 spans
		// --  WRITE THE CPOINTS (control pts) -- //
		file << (cpoints.size() / 3) << "\n"; // cpoints.size() / 3 gives us the # of sets of pts to write
		for (int i = 0; i < cpoints.size(); i+=3) {
			file << cpoints[i] << " ";
			file << cpoints[i + 1] << " ";
			file << cpoints[i + 2];
			file << " \n"; // New line after a set of 3 pts
		}
	}
	else {
		// TRANSLATIONAL SHAPES
		// First line to write is a '0' to symbolize translational curves
		file << "0\n"; // Newline to go to next line in file
		// --  WRITE THE CPOINTS (control pts) -- //
		file << (cpoints.size() / 3) << "\n"; // cpoints.size() / 3 gives us the # of sets of pts to write
		for (int i = 0; i < cpoints.size(); i+=3) {
			file << cpoints[i] << " ";
			file << cpoints[i + 1] << " ";
			file << cpoints[i + 2];
			file << "\n"; // New line after a set of 3 pts
		}
		// -- WRITE THE TPOINTS (trajectory pts) -- //
		file << (tpoints.size() / 3) << "\n"; // tpoints.size() / 3 gives us the # of sets of pts to write
		for (int j = 0; j < tpoints.size(); j += 3) {
			file << tpoints[j] << " ";
			file << tpoints[j + 1] << " ";
			file << tpoints[j + 2];
			file << "\n"; // New line after a set of 3 pts
		}

	}
	finished_spline = true;
	file.close(); // Close file when done
}

#endif