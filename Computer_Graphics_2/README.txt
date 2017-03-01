		-----------------
		| PROGRAMMED BY:| 
		| Philip M	|
		|		|
		|		|
		| COURSE:	|
		| COMP371	|
		| GRAPHICS	|
		-----------------

// 1 \\ -  INTRODUCTION
This project builds on the previous one in that it asks the user to input points, then performs spline operations.
Finally, it displays splined points for a smooth curve and draws a final shape mesh.
The user is initially prompted to enter 'R' or 'T' for rotational or translational shapes respectively.
If the user selects to perform translational curves, they have to input points twice (once for profile, and another for trajectory).

// 2 \\ - FILE I/O + FORMATS
No pre-determined file creation is necessary.
The program will use user input to create a file with data, then load it to display a mesh.

// 3 \\ - USAGE INSTRUCTIONS
Upon launch, refer to the console that associates the displayable window.
There, enter 'r' (or 'R'), or 't' (or 'T') without ' ' to choose which type of mesh to create and draw.
Then, refer to the displayable window.
Enter any number of points as long as you have AT LEAST 4.
Then, press ENTER once. This will perform spline computations (as shown in console).
Then, it will display the spline curves along with your original points.
Then, press ENTER again.
This time, if you chose 'R', the final mesh will be computed and then drawn.
However, if you chose 'T', then enter more points in similar fashion.
Then press ENTER for the spline to show, and ENTER again to display the mesh.

/ 3.1 \ - Arrow Keys
Arrow keys are used to move the final displayed and drawn mesh's model.
UP: Rotates the model upwards on itself.
DOWN: Rotates the model downwards on itself.
LEFT: Rotates model left-wise on itself.
RIGHT: Rotates model right-wise on itself.

/ 3.2 \ - Mouse Clicks
Clicking is the only way to input points in the window.

/ 3.3 \ - Keys: 'P', 'L', and 'F'
These keys will change the display mode for the drawn shape (mesh).
P: Points
L: Lines
F: Filled (default)

/ 3.4 \ - Keys: 'P', 'L'
These keys will change the display mode for the input points.
P: Points (default)
L: Line Strips

/ 3.5 \ - Backspace
Upon pressing backspace, the screen will be wiped clean of the mesh.

// 4 \\ - FINAL REMARKS
This application fully functions for rotational and translational shapes.
To increase the number of points, modify the value of the MAX_LINE_LENGTH variable to be smaller.
For less points, make it bigger.


Thank you for reading!
Enjoy :)

- Philip M.