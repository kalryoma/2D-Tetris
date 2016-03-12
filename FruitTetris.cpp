/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>
#include <ctime>

using namespace std;

bool endgame=false;

// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

int drop = 1;//drop distance
const int speed = 1000;//time interval to drop 

int shape;//shape of current tile
int orientation;//orientation of current shape

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsLshape[4][4] = {
	{vec2(-1, -1), vec2(-1, 0), vec2(0, 0), vec2(1, 0)},
	{vec2(1, -1), vec2(0, -1), vec2(0, 0), vec2(0, 1)},    
	{vec2(1, 1), vec2(1, 0), vec2(0, 0), vec2(-1, 0)},  
	{vec2(-1, 1), vec2(0, 1), vec2(0, 0), vec2(0, -1)}
};
//other shapes
vec2 allRotationsIshape[4][4] = {
	{vec2(-2, 0), vec2(-1, 0), vec2(0, 0), vec2(1, 0)},
	{vec2(0, -2), vec2(0, -1), vec2(0, 0), vec2(0, 1)},
	{vec2(2, 0), vec2(1, 0), vec2(0, 0), vec2(-1, 0)},
	{vec2(0, 2), vec2(0, 1), vec2(0, 0), vec2(0, -1)}
};
vec2 allRotationsSshape[4][4] = {
	{vec2(-1, -1), vec2(0, -1), vec2(0, 0), vec2(1, 0)},
	{vec2(1, -1), vec2(1, 0), vec2(0, 0), vec2(0, 1)},
	{vec2(1, 1), vec2(0, 1), vec2(0, 0), vec2(-1, 0)},
	{vec2(-1, 1), vec2(-1, 0), vec2(0, 0), vec2(0, -1)}
};
vec2 allRotationsTshape[4][4] = {
	{vec2(-1, 0), vec2(0, -1), vec2(0, 0), vec2(1, 0)},
	{vec2(0, -1), vec2(1, 0), vec2(0, 0), vec2(0, 1)},
	{vec2(1, 0), vec2(0, 1), vec2(0, 0), vec2(-1, 0)},
	{vec2(0, 1), vec2(-1, 0), vec2(0, 0), vec2(0, -1)}
};

// colors
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); 
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0);
//other colors
vec4 purple = vec4(1.0, 0.0, 1.0, 1.0);
vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
//fruit colors
vec4 colors[6] = {purple, red, yellow, green, orange, black};
//colors for current tile
int tilecolor[4];
//save color for every matrix
int matrixcolor[10][20];
 
//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//-------------------------------------------------------------------------------------------------------------------

// Returns true if the tile was successfully moved, or false if there was some issue
bool havespace(){//if there is any space for the current tile(orientation)
	for (int i = 0; i < 4; i++){
		int x = tilepos.x + tile[i].x;
		int y = tilepos.y + tile[i].y;
		if ((x<0) || (x>9) || (y<0) || (y>19))//not on board
			return false;
		if (board[x][y])//block is occupied
			return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------------------------------------

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x;
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// Two points are used by two triangles each
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile()
{
	tilepos = vec2(5 , 19); // Put the tile at the top of the board

	// Update the geometry VBO of current tile
	srand(time(NULL));
	shape = rand()%4;
	switch (shape) {
		case 0://L
			orientation = rand()%4;
			if (orientation!=0)
				tilepos = vec2(5 , 18);
			for (int i = 0; i < 4; i++)
				// Get the 4 pieces of the new tile
				tile[i] = allRotationsLshape[orientation][i];
			break;
		case 1://I
			orientation = rand()%4;
			if (orientation%4==1) 
				tilepos = vec2(5 , 18);
			if (orientation%4==3) 
				tilepos = vec2(5 , 17);
			for (int i = 0; i < 4; i++)
				// Get the 4 pieces of the new tile
				tile[i] = allRotationsIshape[orientation][i];
			break;
		case 2://S
			orientation = rand()%4;
			if (orientation!=0)
				tilepos = vec2(5 , 18);
			for (int i = 0; i < 4; i++)
				// Get the 4 pieces of the new tile
				tile[i] = allRotationsSshape[orientation][i];
			break;
		case 3://T
			orientation = rand()%4;
			if (orientation!=0)
				tilepos = vec2(5 , 18);
			for (int i = 0; i < 4; i++)
				// Get the 4 pieces of the new tile
				tile[i] = allRotationsTshape[orientation][i];
			break;
	}

	// Update the color VBO of current tile
	int color=0;
	vec4 newcolours[24];
	for (int i = 0; i < 24; i++){
		if (i%6==0){
			color = rand()%5;//randomize color
			tilecolor[i/6] = color;
		}
		newcolours[i] = colors[color];
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (!havespace())
		endgame = true;
	else
		updatetile();
}

//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[64]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false;
	// Indefault all matrix is black
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			matrixcolor[i][j] = 5;
	endgame=false;

	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Game initialization
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------

// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void shiftdown(int x, int y, int distance){//the column x will be shift down a block since row y
	//move every matrix down
	for (int i=y; i<20-distance; i++){
		board[x][i] = board[x][i+distance];
		matrixcolor[x][i] = matrixcolor[x][i+distance];
	}
	//set the last cells to defalt status
	for (int i=20-distance; i<20; i++){
		board[x][i] = false;
		matrixcolor[x][i] = 5;
	}
}

// Checks if the specified row (0 is the bottom 19 the top) is full
bool checkfullrow(int row){
	//return false this row won't change, return true if any changes happen
	//condition judgement
	for (int i=0; i<10; i++)
		if (!board[i][row])
			return false;
	for (int i=0; i<10; i++)
		shiftdown(i, row, 1);
	return true;
}
//check if any 3 matrices in a row are in the same color
bool checkrow(int row){
	bool result = false;//this row won't change, set to true if any changes happen
	//condition judgement
	for (int i=0; i<8; i++)
		if (board[i][row] && board[i+1][row] && board[i+2][row])
			if ( (matrixcolor[i][row]==matrixcolor[i+1][row]) && (matrixcolor[i+1][row]==matrixcolor[i+2][row]) ){
				shiftdown(i, row, 1);
				shiftdown(i+1, row, 1);
				shiftdown(i+2, row, 1);
				result = true;
			}
	return result;
}
//check if any 3 matrices in a column are in the same color
bool checkcolumn(int column){
	bool result = false;//this column won't change, set to true if any changes happen
	//condition judgement
	for (int i=0; i<18; i++)
		if (board[column][i] && board[column][i+1] && board[column][i+2])
			if ( (matrixcolor[column][i]==matrixcolor[column][i+1]) && (matrixcolor[column][i+1]==matrixcolor[column][i+2]) ){
				shiftdown(column, i, 3);
				result = true;
			}
	return result;
}
//check if any 3 matrices in a diagonal are in the same color
bool checkdiagonal(int x, int y, int diagonal){//check diagonal(0=>bottom-left to top-right, 1=>top-left to bottom-right)
	bool result = false;//no changes any more, set to true if any changes happen
	//condition judgement
	if (diagonal==0){
		int i = x;
		int j = y;
		while ((i<8) && (j<18)){
			if (board[i][j] && board[i+1][j+1] && board[i+2][j+2])
				if ( (matrixcolor[i][j]==matrixcolor[i+1][j+1]) && (matrixcolor[i+1][j+1]==matrixcolor[i+2][j+2]) ){
					shiftdown(i, j, 1);
					shiftdown(i+1, j+1, 1);
					shiftdown(i+2, j+2, 1);
					result = true;
				}
			i++;
			j++;
		}
	}
	if (diagonal==1){
		int i = x;
		int j = y;
		while ((i>2) && (j<18)){
			if (board[i][j] && board[i-1][j+1] && board[i-2][j+2])
				if ( (matrixcolor[i][j]==matrixcolor[i-1][j+1]) && (matrixcolor[i-1][j+1]==matrixcolor[i-2][j+2]) ){
					shiftdown(i, j, 1);
					shiftdown(i-1, j+1, 1);
					shiftdown(i-2, j+2, 1);
					result = true;
				}
			i--;
			j++;
		}
	}
	return result;
}
//check other part to see if need any shiftdown
bool checkboard(){
	bool result = false;
	for (int i=0; i<10; i++){
		int start = 0;
		while ( (board[i][start]) && (start<20) )
			start++;
		int end=start+1;
		while ( (!board[i][end]) && (end<20) )
			end++;
		if (end==20)
			continue;
		shiftdown(i, start, end-start);
		result = true;
	}
	return result;
}
//check if any changes happen
bool check(){
	bool result = false;//no changes any more, set to true if any changes happen
	for (int i=0; i<20; i++)
		if (checkrow(i) || checkfullrow(i))
			result = true;
	for (int i=0; i<10; i++)
		if (checkcolumn(i))
			result = true;
	for (int i=0; i<8; i++)
		if (checkdiagonal(i, 0, 0))
			result = true;
	for (int i=0; i<18; i++)
		if (checkdiagonal(0, i, 0))
			result = true;
	for (int i=2; i<10; i++)
		if (checkdiagonal(i, 0, 1))
			result = true;
	for (int i=0; i<18; i++)
		if (checkdiagonal(9, i, 1))
			result = true;
	if (result)//if any changes happen, scan the board for further shiftdown
		if (checkboard())
				result = true;
	return result;
}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
	//update matrix status
	for (int i = 0; i < 4; i++){
		int x = tilepos.x + tile[i].x;
		int y = tilepos.y + tile[i].y;
		board[x][y] = true;//update occupation
		matrixcolor[x][y] = tilecolor[i]; //update color of matrix according to current tile
	}
	bool flag = true;
	while (flag){//always updating until there are no changes
		//check if need to remove something
		flag = check();

		//save current tile to VBO
		vec4 boardcolours[1200];
		for (int i=0; i<1200; i++)
			boardcolours[i] = colors[matrixcolor[i%60/6][i/60]];
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4)*1200, boardcolours);
	}
	//lastest tile settled so generate a new tile
	newtile();
}

//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	initGrid();
	initBoard();
	initCurrentTile();
	newtile();
}
//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile, if there is room
void rotate(int shape, int orient)
{   
	//save tile before pivoting
	vec2 tmp[4];
	for (int i = 0; i < 4; i++)
		tmp[i] = tile[i];
	//pivot
	switch (shape) {
		case 0://L
			for (int i = 0; i < 4; i++)
				// Get the 4 pieces of the next orientation tile
				tile[i] = allRotationsLshape[(orient+1)%4][i];
			break;
		case 1://I
			for (int i = 0; i < 4; i++)
				// Get the 4 pieces of the next orientation tile
				tile[i] = allRotationsIshape[(orient+1)%4][i];
			break;
		case 2://S
			for (int i = 0; i < 4; i++)
				// Get the 4 pieces of the next orientation tile
				tile[i] = allRotationsSshape[(orient+1)%4][i];
			break;
		case 3://T
			for (int i = 0; i < 4; i++)
				// Get the 4 pieces of the next orientation tile
				tile[i] = allRotationsTshape[(orient+1)%4][i];
			break;
	}
	if (havespace())
		orientation = (orient+1)%4;
	else
		//load tile if don't have enough space to pivot
		for (int i = 0; i < 4; i++)
			tile[i] = tmp[i];
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y){
	switch(key) {
		case GLUT_KEY_UP://up
			rotate(shape, orientation);
			break;
		case GLUT_KEY_DOWN: //down
			tilepos.y -= drop;
			//make sure the tile don't cross the edge
			if (!havespace())
				tilepos.y += drop;
			break;
		case GLUT_KEY_LEFT: //left
			tilepos.x -= 1;
			//make sure the tile don't cross the edge
			if (!havespace())
				tilepos.x += 1;
			break;
		case GLUT_KEY_RIGHT: //right
			tilepos.x += 1;
			//make sure the tile don't cross the edge
			if (!havespace())
				tilepos.x -= 1;
			break;
	}
	updatetile();
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

//called every "speed" msec to do drop
void dropupdate(int drop){
	if (!endgame){
		tilepos.y -= drop;
		//make sure the tile don't cross the edge
		if (!havespace()){
			tilepos.y += drop;
			settile();
		}
		updatetile();
	}
	else{
		cout<<"You LOSE!"<<endl;
		cout<<"Press 'r' to try again or 'q' to quit."<<endl;
	}
	glutPostRedisplay();
	glutTimerFunc(speed, dropupdate, drop);
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	#ifdef __APPLE__
	#else
		glewInit();
	#endif
	//glewInit();
	init();

	glutTimerFunc(speed, dropupdate, drop);

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop

	return 0;
}
