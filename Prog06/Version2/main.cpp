//
//  main.c
//  Cellular Automaton
//
//  Created by Jean-Yves Hervé on 2018-04-09.
//	C++ version 2020-11-06

/*-------------------------------------------------------------------------+
 |	A graphic front end for a grid+state simulation.						|
 |																			|
 |	This application simply creates a glut window with a pane to display	|
 |	a colored grid and the other to display some state information.			|
 |	Sets up callback functions to handle menu, mouse and keyboard events.	|
 |	Normally, you shouldn't have to touch anything in this code, unless		|
 |	you want to change some of the things displayed, add menus, etc.		|
 |	Only mess with this after everything else works and making a backup		|
 |	copy of your project.  OpenGL & glut are tricky and it's really easy	|
 |	to break everything with a single line of code.							|
 |																			|
 |	Current keyboard controls:												|
 |																			|
 |		- 'ESC' --> exit the application									|
 |		- space bar --> resets the grid										|
 |																			|
 |		- 'c' --> toggle color mode on/off									|
 |		- 'b' --> toggles color mode off/on									|
 |		- 'l' --> toggles on/off grid line rendering						|
 |																			|
 |		- '+' --> increase simulation speed									|
 |		- '-' --> reduce simulation speed									|
 |																			|
 |		- '1' --> apply Rule 1 (Conway's classical Game of Life: B3/S23)	|
 |		- '2' --> apply Rule 2 (Coral: B3/S45678)							|
 |		- '3' --> apply Rule 3 (Amoeba: B357/S1358)							|
 |		- '4' --> apply Rule 4 (Maze: B3/S12345)							|
 |																			|
 +-------------------------------------------------------------------------*/

#include <iostream>
#include <ctime>
#include <unistd.h>
#include <sstream>
#include "gl_frontEnd.h"
#include <pthread.h>

using namespace std;

#if 0
//==================================================================================
#pragma mark -
#pragma mark Custom data types
//==================================================================================
#endif

typedef struct ThreadInfo
{
	//	you probably want these
	pthread_t threadID;
	unsigned int threadIndex;

	// the number of rows assigned to this thread
	unsigned int assignedRows;

	// relative start row
	unsigned int globalContextStartRow;

	// commmunicates to the psuedoMaster that it is ready to have its rows published to the GUI.
	bool setForPublishing = false;

} ThreadInfo;

pthread_t pseudoMaster;

#if 0
//==================================================================================
#pragma mark -
#pragma mark Function prototypes
//==================================================================================
#endif

void displayGridPane(void);
void displayStatePane(void);
void initializeApplication(void);
void cleanupAndquit(void);
void *threadFunc(void *);
void swapGrids(void);
unsigned int cellNewState(unsigned int i, unsigned int j);
bool checkInput(int argCount, const char **inputs);

//==================================================================================
//	Precompiler #define to let us specify how things should be handled at the
//	border of the frame
//==================================================================================

#if 0
//==================================================================================
#pragma mark -
#pragma mark Simulation mode:  behavior at edges of frame
//==================================================================================
#endif

#define FRAME_DEAD 0	//	cell borders are kept dead
#define FRAME_RANDOM 1	//	new random values are generated at each generation
#define FRAME_CLIPPED 2 //	same rule as elsewhere, with clipping to stay within bounds
#define FRAME_WRAP 3	//	same rule as elsewhere, with wrapping around at edges

//	Pick one value for FRAME_BEHAVIOR
#define FRAME_BEHAVIOR FRAME_DEAD

#if 0
//==================================================================================
#pragma mark -
#pragma mark Application-level global variables
//==================================================================================
#endif

//	Don't touch
extern int gMainWindow, gSubwindow[2];

//	The state grid and its dimensions.  We use two copies of the grid:
//		- currentGrid is the one displayed in the graphic front end
//		- nextGrid is the grid that stores the next generation of cell
//			states, as computed by our threads.
unsigned int *currentGrid;
unsigned int *nextGrid;
unsigned int **currentGrid2D;
unsigned int **nextGrid2D;

//	Piece of advice, whenever you do a grid-based (e.g. image processing),
//	implementastion, you should always try to run your code with a
//	non-square grid to spot accidental row-col inversion bugs.
//	When this is possible, of course (e.g. makes no sense for a chess program).
unsigned int numRows;
unsigned int numCols;
unsigned int maxThreads;

//	the number of live computation threads (that haven't terminated yet)
unsigned short numLiveThreads = 0;

unsigned int rule = GAME_OF_LIFE_RULE;

unsigned int colorMode = 0;

unsigned int generation = 0;

// used to alter run speed
int sleepClock = 1;

// ---------------------------
// this is basically used as a semaphore, afaik
// I realize the big thing with semaphores is that they are atomic, but
// I'm designing with the non-atomic nature in mind.
// ---------------------------

// when use hits escape, this becomes true, and the psuedoMain thread
// will stop at next generational loop and exit
bool fullStop = false;

// Signals if the grid is ready to be swapped.
bool printGrid = false;

//------------------------------
//	Threads and synchronization
//	Reminder of all declarations and function calls
//------------------------------
//pthread_mutex_t myLock;
//pthread_mutex_init(&myLock, NULL);
//int err = pthread_create(pthread_t*, NULL, threadFunc, ThreadInfo*);
//int pthread_join(pthread_t , void**);
//pthread_mutex_lock(&myLock);
//pthread_mutex_unlock(&myLock);

#if 0
//==================================================================================
#pragma mark -
#pragma mark Computation functions
//==================================================================================
#endif

//------------------------------------------------------------------------
//	You shouldn't have to change anything in the main function
//------------------------------------------------------------------------
int main(int argc, const char *argv[])
{
	if (checkInput(argc, argv))
	{

		//	This takes care of initializing glut and the GUI.
		//	You shouldn’t have to touch this
		initializeFrontEnd(argc, argv, displayGridPane, displayStatePane);

		//	Now we can do application-level initialization
		initializeApplication();

		//	Now would be the place & time to create mutex locks and threads
		if (pthread_create(&pseudoMaster, NULL, threadFunc, NULL) != 0)
		{
			cout << "Distributor failed to start" << endl;
			std::exit(0);
		}
		//	Now we enter the main loop of the program and to a large extend
		//	"lose control" over its execution.  The callback functions that
		//	we set up earlier will be called when the corresponding event
		//	occurs
		glutMainLoop();

		//	In fact this code is never reached because we only leave the glut main
		//	loop through an exit call.
		//	Free allocated resource before leaving (not absolutely needed, but
		//	just nicer.  Also, if you crash there, you know something is wrong
		//	in your code.
		free(currentGrid2D);
		free(currentGrid);

		//	This will never be executed (the exit point will be in one of the
		//	call back functions).
		return 0;
	}
}

//==================================================================================
//
//	This is a part that you have to edit and add to.
//
//==================================================================================

// check that the input meets program standards
bool checkInput(int argCount, const char **inputs)
{

	if (argCount < 4)
	{
		cout << "ERROR: too few arguements, correct usage : ./cell <width> <height> <number of threads>" << endl;
		return false;
	}
	else if (argCount > 4)
	{
		cout << "ERROR: too many arguements, correct usage : ./cell <width> <height> <number of threads>" << endl;
		return false;
	}

	for (int code = 1; code < 4; code++)
	{
		stringstream ferry;
		unsigned int toCheck;
		ferry << inputs[code];
		ferry >> toCheck;

		if (code == 1)
		{
			if (toCheck <= 5)
			{
				cout << "Invalid input, width must be larger than 5" << endl;
				return false;
			}
			numCols = toCheck;
		}
		if (code == 2)
		{
			if (toCheck <= 5)
			{
				cout << "Invalid input, height must be larger than 5" << endl;
				return false;
			}

			numRows = toCheck;
		}
		if (code == 3)
		{
			if (toCheck > numCols)
			{
				cout << "Invalid input, thread count cannot be larger than the height of the grid " << endl;
				return false;
			}

			maxThreads = toCheck;
		}
	}

	return true;
}

void initializeApplication(void)
{

	//--------------------
	//  Allocate 1D grids
	//--------------------
	currentGrid = new unsigned int[numRows * numCols];
	nextGrid = new unsigned int[numRows * numCols];

	//---------------------------------------------
	//  Scaffold 2D arrays on top of the 1D arrays
	//---------------------------------------------
	currentGrid2D = new unsigned int *[numRows];
	nextGrid2D = new unsigned int *[numRows];
	currentGrid2D[0] = currentGrid;
	nextGrid2D[0] = nextGrid;
	for (unsigned int i = 1; i < numRows; i++)
	{
		currentGrid2D[i] = currentGrid2D[i - 1] + numCols;
		nextGrid2D[i] = nextGrid2D[i - 1] + numCols;
	}

	//---------------------------------------------------------------
	//	All the code below to be replaced/removed
	//	I initialize the grid's pixels to have something to look at
	//---------------------------------------------------------------
	//	Yes, I am using the C random generator after ranting in class that the C random
	//	generator was junk.  Here I am not using it to produce "serious" data (as in a
	//	simulation), only some color, in meant-to-be-thrown-away code

	//	seed the pseudo-random generator
	srand((unsigned int)time(NULL));

	resetGrid();
}

//---------------------------------------------------------------------
//	You will need to implement/modify the two functions below
//---------------------------------------------------------------------

//creates threads, then regulates the printing of the grid through communication with the grids.
void *threadFunc(void *arg)
{

	numLiveThreads += 1;

	struct ThreadInfo TI[maxThreads];
	unsigned int overflow = numRows % maxThreads;
	unsigned int currentRow = 0;

	for (unsigned int i = 0; i < maxThreads; i++)
	{
		if (i < overflow)
		{
			TI[i].assignedRows = (numRows / maxThreads) + 1;
		}
		else
		{
			TI[i].assignedRows = (numRows / maxThreads);
		}

		TI[i].threadIndex = i;
		TI[i].globalContextStartRow = currentRow;

		currentRow += TI[i].assignedRows;
		pthread_create(&TI[i].threadID, NULL, oneGeneration, (void *)&TI[i]);
		numLiveThreads += 1;
	}

	usleep(25000);

	while (numLiveThreads != maxThreads + 1)
	{
	}
	while (!fullStop)
	{

		if (printGrid == false)
		{

			for (unsigned int j = 0; j < maxThreads; j++)
			{
				while (!TI[j].setForPublishing)
				{
				}
			}

			swapGrids();
			printGrid = true;
			while (printGrid && !fullStop)
			{
			};
			for (unsigned int k = maxThreads; k > 0; k--)
			{
				TI[k - 1].setForPublishing = false;
			}
		}
		for (int l = 0; l < sleepClock; l++)
		{
			usleep(25000);
		}
	}

	numLiveThreads -= 1;

	return NULL;
}

//	I have decided to go for maximum modularity and readability, at the
//	cost of some performance.  This may seem contradictory with the
//	very purpose of multi-threading our application.  I won't deny it.
//	My justification here is that this is very much an educational exercise,
//	my objective being for you to understand and master the mechanisms of
//	multithreading and synchronization with mutex.  After you get there,
//	you can micro-optimi1ze your synchronized multithreaded apps to your
//	heart's content.

// this is the function that is run by threads created in threadFunc.
void *oneGeneration(void *arg)
{

	//numLiveThreads += 1;

	struct ThreadInfo *localData;

	localData = (struct ThreadInfo *)arg;

	while (!fullStop)
	{
		if (!localData->setForPublishing)
		{
			for (unsigned int i = localData->globalContextStartRow; i < localData->globalContextStartRow + localData->assignedRows; i++)
			{

				for (unsigned int j = 0; j < numCols; j++)
				{

					unsigned int newState = cellNewState(i, j);

					//	In black and white mode, only alive/dead matters
					//	Dead is dead in any mode
					if (colorMode == 0 || newState == 0)
					{
						nextGrid2D[i][j] = newState;
					}
					//	in color mode, color reflext the "age" of a live cell
					else
					{
						//	Any cell that has not yet reached the "very old cell"
						//	stage simply got one generation older
						if (currentGrid2D[i][j] < NB_COLORS - 1)
							nextGrid2D[i][j] = currentGrid2D[i][j] + 1;
						//	An old cell remains old until it dies
						else
							nextGrid2D[i][j] = currentGrid2D[i][j];
					}
				}
			}

			localData->setForPublishing = true;
		}
		usleep(500);
	}

	numLiveThreads -= 1;

	return NULL;
}

//	This is the function that determines how a cell update its state
//	based on that of its neighbors.
//	1. As repeated in the comments below, this is a "didactic" implementation,
//	rather than one optimized for speed.  In particular, I refer explicitly
//	to the S/B elements of the "rule" in place.
//	2. This implentation allows for different behaviors along the edges of the
//	grid (no change, keep border dead, clipping, wrap around). All these
//	variants are used for simulations in research applications.
unsigned int cellNewState(unsigned int i, unsigned int j)
{

	//	First count the number of neighbors that are alive
	//----------------------------------------------------
	//	Again, this implementation makes no pretense at being the most efficient.
	//	I am just trying to keep things modular and somewhat readable
	int count = 0;

	//	Away from the border, we simply count how many among the cell's
	//	eight neighbors are alive (cell state > 0)
	if (i > 0 && i < numRows - 1 && j > 0 && j < numCols - 1)
	{
		//	remember that in C, (x == val) is either 1 or 0
		count = (currentGrid2D[i - 1][j - 1] != 0) +
				(currentGrid2D[i - 1][j] != 0) +
				(currentGrid2D[i - 1][j + 1] != 0) +
				(currentGrid2D[i][j - 1] != 0) +
				(currentGrid2D[i][j + 1] != 0) +
				(currentGrid2D[i + 1][j - 1] != 0) +
				(currentGrid2D[i + 1][j] != 0) +
				(currentGrid2D[i + 1][j + 1] != 0);
	}
	//	on the border of the frame...
	else
	{
#if FRAME_BEHAVIOR == FRAME_DEAD

		//	Hack to force death of a cell
		count = -1;

#elif FRAME_BEHAVIOR == FRAME_RANDOM

		count = rand() % 9;

#elif FRAME_BEHAVIOR == FRAME_CLIPPED

		if (i > 0)
		{
			if (j > 0 && currentGrid2D[i - 1][j - 1] != 0)
				count++;
			if (currentGrid2D[i - 1][j] != 0)
				count++;
			if (j < numCols - 1 && currentGrid2D[i - 1][j + 1] != 0)
				count++;
		}

		if (j > 0 && currentGrid2D[i][j - 1] != 0)
			count++;
		if (j < numCols - 1 && currentGrid2D[i][j + 1] != 0)
			count++;

		if (i < numRows - 1)
		{
			if (j > 0 && currentGrid2D[i + 1][j - 1] != 0)
				count++;
			if (currentGrid2D[i + 1][j] != 0)
				count++;
			if (j < numCols - 1 && currentGrid2D[i + 1][j + 1] != 0)
				count++;
		}

#elif FRAME_BEHAVIOR == FRAME_WRAPPED

		unsigned int iM1 = (i + numRows - 1) % numRows,
					 iP1 = (i + 1) % numRows,
					 jM1 = (j + numCols - 1) % numCols,
					 jP1 = (j + 1) % numCols;
		count = currentGrid2D[iM1][jM1] != 0 + currentGrid2D[iM1][j] != 0 + currentGrid2D[iM1][jP1] != 0 + currentGrid2D[i][jM1] != 0 + currentGrid2D[i][jP1] != 0 + currentGrid2D[iP1][jM1] != 0 + currentGrid2D[iP1][j] != 0 + currentGrid2D[iP1][jP1] != 0;

#else
#error undefined frame behavior
#endif

	} //	end of else case (on border)

	//	Next apply the cellular automaton rule
	//----------------------------------------------------
	//	by default, the grid square is going to be empty/dead
	unsigned int newState = 0;

	//	unless....

	switch (rule)
	{
	//	Rule 1 (Conway's classical Game of Life: B3/S23)
	case GAME_OF_LIFE_RULE:

		//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
		if (currentGrid2D[i][j] != 0)
		{
			if (count == 3 || count == 2)
				newState = 1;
		}
		//	if the grid square is currently empty, look at the "Birth of a new cell" rule
		else
		{
			if (count == 3)
				newState = 1;
		}
		break;

	//	Rule 2 (Coral Growth: B3/S45678)
	case CORAL_GROWTH_RULE:

		//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
		if (currentGrid2D[i][j] != 0)
		{
			if (count > 3)
				newState = 1;
		}
		//	if the grid square is currently empty, look at the "Birth of a new cell" rule
		else
		{
			if (count == 3)
				newState = 1;
		}
		break;

	//	Rule 3 (Amoeba: B357/S1358)
	case AMOEBA_RULE:

		//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
		if (currentGrid2D[i][j] != 0)
		{
			if (count == 1 || count == 3 || count == 5 || count == 8)
				newState = 1;
		}
		//	if the grid square is currently empty, look at the "Birth of a new cell" rule
		else
		{
			if (count == 3 || count == 5 || count == 7)
				newState = 1;
		}
		break;

	//	Rule 4 (Maze: B3/S12345)							|
	case MAZE_RULE:

		//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
		if (currentGrid2D[i][j] != 0)
		{
			if (count >= 1 && count <= 5)
				newState = 1;
		}
		//	if the grid square is currently empty, look at the "Birth of a new cell" rule
		else
		{
			if (count == 3)
				newState = 1;
		}
		break;

		break;

	default:
		cout << "Invalid rule number" << endl;
		std::exit(5);
	}

	return newState;
}

// signals to kill threads, confirms they are gone, and exits the program
void cleanupAndquit(void)
{
	fullStop = true;

	//	join the threads, this is waiting on the psuedo-master thread to shut all child threads, then itself down.
	while (numLiveThreads != 0)
	{
	}

	//	free the grids  <--- thank you for this
	delete currentGrid;
	delete currentGrid2D;
	delete nextGrid;
	delete nextGrid2D;

	std::exit(0);
}

#if 0
#pragma mark -
#pragma mark GUI functions
#endif

//==================================================================================
//	GUI functions
//	These are the functions that tie the simulation with the rendering.
//	Some parts are "don't touch."  Other parts need your intervention
//	to make sure that access to critical section is properly synchronized
//==================================================================================

void displayGridPane(void)
{
	//	This is OpenGL/glut magic.  Don't touch
	glutSetWindow(gSubwindow[GRID_PANE]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//---------------------------------------------------------
	//	This is the call that makes OpenGL render the grid.
	//
	//---------------------------------------------------------
	drawGrid(currentGrid2D, numRows, numCols);

	//	This is OpenGL/glut magic.  Don't touch
	glutSwapBuffers();

	glutSetWindow(gMainWindow);
}

void displayStatePane(void)
{
	//	This is OpenGL/glut magic.  Don't touch
	glutSetWindow(gSubwindow[STATE_PANE]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//---------------------------------------------------------
	//	This is the call that makes OpenGL render information
	//	about the state of the simulation.
	//	You may want to add stuff to display
	//---------------------------------------------------------
	drawState(numLiveThreads);

	//	This is OpenGL/glut magic.  Don't touch
	glutSwapBuffers();
	glutSetWindow(gMainWindow);
}

//	This callback function is called when a keyboard event occurs
//
void myKeyboardFunc(unsigned char c, int x, int y)
{
	(void)x;
	(void)y;

	switch (c)
	{
	//	'ESC' --> exit the application
	case 27:
		cleanupAndquit();
		break;

	//	spacebar --> resets the grid
	case ' ':
		resetGrid();
		break;

	//	'+' --> increase simulation speed
	case '+':
		if (sleepClock != 1)
		{
			sleepClock -= 1;
		}
		break;

	//	'-' --> reduce simulation speed
	case '-':

		sleepClock += 1;
		break;

	//	'1' --> apply Rule 1 (Game of Life: B23/S3)
	case '1':
		rule = GAME_OF_LIFE_RULE;
		break;

	//	'2' --> apply Rule 2 (Coral: B3_S45678)
	case '2':
		rule = CORAL_GROWTH_RULE;
		break;

	//	'3' --> apply Rule 3 (Amoeba: B357/S1358)
	case '3':
		rule = AMOEBA_RULE;
		break;

	//	'4' --> apply Rule 4 (Maze: B3/S12345)
	case '4':
		rule = MAZE_RULE;
		break;

	//	'c' --> toggles on/off color mode
	//	'b' --> toggles off/on color mode
	case 'c':
	case 'b':
		colorMode = !colorMode;
		break;

	//	'l' --> toggles on/off grid line rendering
	case 'l':
		toggleDrawGridLines();
		break;

	default:
		break;
	}

	glutSetWindow(gMainWindow);
	glutPostRedisplay();
}

void myTimerFunc(int value)
{
	//	value not used.  Warning suppression
	(void)value;

	//  possibly I do something to update the state information displayed
	//	in the "state" pane

	//==============================================
	//	This call must **DEFINITELY** go away.
	//	(when you add proper threading)
	//==============================================
	//threadFunc(NULL);
	while (printGrid == false)
	{
	};
	printGrid = false;

	//	This is not the way it should be done, but it seems that Apple is
	//	not happy with having marked glut as deprecated.  They are doing
	//	things to make it break
	//glutPostRedisplay();
	myDisplayFunc();

	//	And finally I perform the rendering
	glutTimerFunc(100, myTimerFunc, 0);
}

//---------------------------------------------------------------------
//	You shouldn't need to touch the functions below
//---------------------------------------------------------------------

void resetGrid(void)
{
	for (unsigned int i = 0; i < numRows; i++)
	{
		for (unsigned int j = 0; j < numCols; j++)
		{
			nextGrid2D[i][j] = rand() % 2;
		}
	}
	swapGrids();
}

//	This function swaps the current and next grids, as well as their
//	companion 2D grid.  Note that we only swap the "top" layer of
//	the 2D grids.
void swapGrids(void)
{
	//	swap grids
	unsigned int *tempGrid;
	unsigned int **tempGrid2D;

	tempGrid = currentGrid;
	currentGrid = nextGrid;
	nextGrid = tempGrid;
	//
	tempGrid2D = currentGrid2D;
	currentGrid2D = nextGrid2D;
	nextGrid2D = tempGrid2D;
}
