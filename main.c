#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <sys/time.h>
#include <windows.h>

char player_name[200]; // stores the name of the player

// define some constants
#define TRUE 1
#define FALSE 0

// define some key codes
#define LEFT_KEY 75 // ASCII codes corresponding to certain keys
#define RIGHT_KEY 77
#define UP_KEY 72
#define DOWN_KEY 80
#define ESCAPE_KEY 27
#define PAUSE_KEY 'p'

// shape (tetromino) struct definition
typedef struct
{
	char **shape_matrix;  // stores the shape of the tetromino in a char matrix
	unsigned short width; // width of the tetromino
	short x, y;			  // x and y coordinates of the location of tetromino
} Shape;

#define TETRIS_BOARD_WIDTH 11  // width of the Tetris board
#define TETRIS_BOARD_HEIGHT 20 // height of the Tetris board

// Tetris struct definition
typedef struct
{
	char board[TETRIS_BOARD_HEIGHT][TETRIS_BOARD_WIDTH]; // game board
	unsigned int score, timer;							 // score and timer
} Tetris;

#define ROTATE_KEY UP_KEY
#define DEFAULT_TIMER 500000 // 500000 microseconds = half a second

// 7 tetrominos that are going to be used throughout the game
const Shape Tetrominos[7] = {
	{(char *[]){(char[]){0, 0, 0, 0},
				(char[]){1, 1, 1, 1},
				(char[]){0, 0, 0, 0},
				(char[]){0, 0, 0, 0}},
	 4, 0, 0}, // I tetromino
	{(char *[]){(char[]){1, 1},
				(char[]){1, 1}},
	 2, 0, 0}, // O tetromino
	{(char *[]){(char[]){0, 1, 0},
				(char[]){1, 1, 1},
				(char[]){0, 0, 0}},
	 3, 0, 0}, // T tetromino
	{(char *[]){(char[]){0, 1, 1},
				(char[]){1, 1, 0},
				(char[]){0, 0, 0}},
	 3, 0, 0}, // S tetromino
	{(char *[]){(char[]){1, 1, 0},
				(char[]){0, 1, 1},
				(char[]){0, 0, 0}},
	 3, 0, 0}, // Z tetromino
	{(char *[]){(char[]){0, 0, 1},
				(char[]){1, 1, 1},
				(char[]){0, 0, 0}},
	 3, 0, 0}, // J tetromino
	{(char *[]){(char[]){1, 0, 0},
				(char[]){1, 1, 1},
				(char[]){0, 0, 0}},
	 3, 0, 0} // L tetromino
};

Shape *current;
Tetris tetris = (Tetris){{{0}}, 0, DEFAULT_TIMER};
unsigned short game_on_flag_4 = TRUE;

void displayOpening();
void refreshScreen();
void displayBoard();
void displayShape();
void clearShapeFromConsole();
void addShapeToBoard();
void removeShapeFromBoard();
void generateShape();
Shape *duplicateShape(const Shape shape);
void removeShape(Shape *shape);
unsigned short validateShapePosition(Shape *shape);
void validateRows();
void rotateShape(Shape *shape);
void moveShape(char key);
unsigned short storeScore();
void PlayTetris();

// moves the cursor to a different (x, y) location on the terminal
void MoveCursorToXY(int x, int y);
void LoadingScreen();
int main()
{
	srand((unsigned int)time(NULL));
	// for play Tetris
	PlayTetris();
	return 0;
}

// Function to move the cursor to a specific position on the console
void MoveCursorToXY(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Function to print a loading screen with a progress bar
void LoadingScreen()
{
	int i;
	const int totalBars = 20;		  // Total number of bars in the progress bar
	const int totalFrames = 10;		  // Total number of frames to cycle through
	const char progressBarChar = 219; // Character to represent the progress bar

	MoveCursorToXY(36, 14);
	printf("Loading...");

	for (i = 0; i < totalFrames * 2; ++i)
	{
		MoveCursorToXY(30, 15);
		int barsToShow = (i % totalFrames) + 1;
		int j;
		for (j = 0; j < totalBars; ++j)
		{
			if (j < barsToShow)
				printf("%c", progressBarChar);
			else
				printf(" ");
		}

		fflush(stdout); // Flush the output buffer to ensure immediate printing

		Sleep(100); // Sleep for a short duration for animation effect

		MoveCursorToXY(30, 15);
		for (j = 0; j < totalBars; ++j)
		{
			printf(" "); // Clear the progress bar
		}
	}

	system("cls"); // Clear the screen after loading
}

// Welcome screen of the Tetris game where the instructions of the game are presented to the player
void displayOpening()
{
	system("cls");
	printf("\n\n\n\t\t\tWelcome to the Tetris game!\n\n\n"
		   "\t\t\tGame instructions:\n\n"
		   "\t\t\t-> Use left and right arrow keys to move blocks across the screen,\n"
		   "\t\t\t   down arrow key to bring them down faster, and the up arrow key\n"
		   "\t\t\t   to rotate them.\n\n"
		   "\t\t\t-> Your objective is to fill all the empty space in a row at the\n"
		   "\t\t\t   bottom of the screen. Filled rows will vanish and you'll be awarded\n"
		   "\t\t\t   100 points.\n\n"
		   "\t\t\t-> The game ends if a block reaches the top of the screen.\n\n"
		   "\t\t\t-> Press 'P' to pause the game. Press 'P' again to continue.\n\n"
		   "\t\t\t-> Press 'Esc' to exit the game (you will lose all progress).\n\n\n"
		   "\t\t\tPress any key to play the game...");

	if (getch() == ESCAPE_KEY)
		exit(0);

	system("cls");
}

// updates the frame of the Tetris game
void refreshScreen()
{
	struct timeval before, after;
	gettimeofday(&before, NULL);
	do
	{
		if (kbhit())
		{
			char key = getch(); // FPS: 10^6 us / tetris.timer us = 1 s / 0.5 s = 2
			moveShape(key);		// the FPS will gradually increase because every time a row is filled, tetris.timer will decrease by a millisecond
		}
		gettimeofday(&after, NULL);
		if ((unsigned int)(after.tv_sec * 1000000 + after.tv_usec - before.tv_sec * 1000000 - before.tv_usec) > tetris.timer)
		{
			before = after;
			moveShape(DOWN_KEY); // in every once in "timer", the tetromino will fall by a unit on the board downwards
		} // as the game progresses, pieces will start to fall faster and faster
	} while (game_on_flag_4);
	system("cls");
	printf("Game over!" // game over
		   "\nBetter luck next time."
		   "\n(Press C to continue...)");
	char key;
	do
	{
		key = getch();
		if (key == ESCAPE_KEY)
			exit(0);
	} while (key != 'c' && key != 'C');
	system("cls");
}

// prints the board of the Tetris game, including player's score
void displayBoard()
{
	MoveCursorToXY(30, 2);
	printf("SCORE: %u", tetris.score);
	MoveCursorToXY(30, 3);
	unsigned short i;
	for (i = 0; i < 2 * TETRIS_BOARD_WIDTH + 2; i++)
		printf("*");
	for (i = 0; i < TETRIS_BOARD_HEIGHT; i++)
	{
		MoveCursorToXY(30, 4 + i);
		printf("*");
		MoveCursorToXY(31 + 2 * TETRIS_BOARD_WIDTH, 4 + i);
		printf("*");
	}
	MoveCursorToXY(30, 4 + TETRIS_BOARD_HEIGHT);
	for (i = 0; i < 2 * TETRIS_BOARD_WIDTH + 2; i++)
		printf("*");
}

// prints the shapes onto the terminal
void displayShape()
{
	unsigned short i, j;
	for (i = 0; i < TETRIS_BOARD_HEIGHT; i++)
		for (j = 0; j < TETRIS_BOARD_WIDTH; j++)
			if (tetris.board[i][j])
			{
				MoveCursorToXY(31 + 2 * j, 4 + i);
				printf("O");
			}
}

// erases the shape seen on the terminal
void clearShapeFromConsole()
{
	unsigned short i, j;
	for (i = 0; i < current->width; i++)
		for (j = 0; j < current->width; j++)
			if (current->shape_matrix[i][j])
			{
				MoveCursorToXY(31 + 2 * (j + current->x), 4 + i + current->y);
				printf(" ");
			}
}

// stores the shape in the board with ones
void addShapeToBoard()
{
	unsigned short i, j;
	for (i = 0; i < current->width; i++)
		for (j = 0; j < current->width; j++)
			if (current->shape_matrix[i][j])
				tetris.board[current->y + i][current->x + j] = 1;
}

// deletes the stored shape with zeros
void removeShapeFromBoard()
{
	unsigned short i, j;
	for (i = 0; i < current->width; i++)
		for (j = 0; j < current->width; j++)
			if (current->shape_matrix[i][j])
				tetris.board[current->y + i][current->x + j] = 0;
}

// selects a random tetromino
void generateShape()
{
	current = duplicateShape(Tetrominos[rand() % 7]);
	current->x = rand() % (TETRIS_BOARD_WIDTH - current->width + 1);
	if (!validateShapePosition(current))
		game_on_flag_4 = FALSE;
}

// gets a separate copy of the given shape
Shape *duplicateShape(const Shape shape)
{
	Shape *copy = (Shape *)malloc(sizeof(Shape));
	copy->width = shape.width;
	copy->y = shape.y;
	copy->x = shape.x;
	copy->shape_matrix = (char **)malloc(copy->width * sizeof(char *));
	unsigned short i, j;
	for (i = 0; i < copy->width; i++)
	{
		copy->shape_matrix[i] = (char *)malloc(copy->width * sizeof(char));
		for (j = 0; j < copy->width; j++)
			copy->shape_matrix[i][j] = shape.shape_matrix[i][j];
	}
	return copy;
}

// deletes the given shape and deallocates memory
void removeShape(Shape *shape)
{
	unsigned short i;
	for (i = 0; i < shape->width; i++)
		free(shape->shape_matrix[i]);
	free(shape->shape_matrix);
	free(shape);
}

// checks the position of the shape
unsigned short validateShapePosition(Shape *shape)
{
	unsigned short i, j;
	for (i = 0; i < shape->width; i++)
		for (j = 0; j < shape->width; j++)
			if (shape->shape_matrix[i][j])
			{
				if (shape->x + j < 0 || shape->x + j >= TETRIS_BOARD_WIDTH || shape->y + i >= TETRIS_BOARD_HEIGHT) // if out of board boundaries
					return FALSE;
				else if (tetris.board[shape->y + i][shape->x + j]) // if another piece occupies the given position
					return FALSE;
			}
	return TRUE;
}

// checks if we have hit a full row or not
// if we have, adds 100 to the score for each filled row
void validateRows()
{
	unsigned short i, j, counter = 0;
	for (i = 0; i < TETRIS_BOARD_HEIGHT; i++)
	{
		unsigned short sum = 0;
		for (j = 0; j < TETRIS_BOARD_WIDTH; j++)
			sum += tetris.board[i][j];
		if (sum == TETRIS_BOARD_WIDTH)
		{
			counter++;
			for (j = 0; j < TETRIS_BOARD_WIDTH; j++)
			{
				MoveCursorToXY(31 + 2 * j, 4 + i);
				printf(" ");
			}
			unsigned short k;
			for (k = i; k >= 1; k--)
				for (j = 0; j < TETRIS_BOARD_WIDTH; j++)
				{
					if (!tetris.board[k - 1][j] && tetris.board[k][j])
					{
						MoveCursorToXY(31 + 2 * j, 4 + k);
						printf(" ");
					}
					tetris.board[k][j] = tetris.board[k - 1][j];
				}
			for (j = 0; j < TETRIS_BOARD_WIDTH; j++)
			{
				tetris.board[k][j] = 0;
				MoveCursorToXY(31 + 2 * j, 4 + k);
				printf(" ");
			}
		}
	}
	tetris.timer -= 1000;
	tetris.score += 100 * counter;
	MoveCursorToXY(37, 2);
	printf("%u", tetris.score);
}

// rotates the given tetromino clockwise
void rotateShape(Shape *shape)
{
	Shape *temp = duplicateShape(*shape);
	unsigned short i, j;
	for (i = 0; i < shape->width; i++)
		for (j = 0; j < shape->width; j++)
			shape->shape_matrix[i][j] = temp->shape_matrix[shape->width - j - 1][i];
	removeShape(temp);
}

// manipulates the tetromino according to which key is pressed
void moveShape(char key)
{
	Shape *temp = duplicateShape(*current);
	switch (key)
	{
	case LEFT_KEY: // if hit left arrow key, move the piece to the left
		clearShapeFromConsole();
		temp->x--;
		if (validateShapePosition(temp))
			current->x--;
		break;
	case RIGHT_KEY: // if hit right arrow key, move the piece to the right
		clearShapeFromConsole();
		temp->x++;
		if (validateShapePosition(temp))
			current->x++;
		break;
	case DOWN_KEY: // if hit down arrow key, move the piece downwards
		clearShapeFromConsole();
		temp->y++;
		if (validateShapePosition(temp))
			current->y++;
		else
		{
			addShapeToBoard();
			validateRows(); // after placing the shape, check if the any rows are filled
			generateShape();
		}
		break;
	case ROTATE_KEY: // if hit up arrow key, rotate the shape
		clearShapeFromConsole();
		rotateShape(temp);
		if (validateShapePosition(temp))
			rotateShape(current);
		break;
	case PAUSE_KEY: // pause the game
		do
		{
			key = getch();
			if (key == ESCAPE_KEY)
			{
				MoveCursorToXY(0, 4 + TETRIS_BOARD_HEIGHT);
				exit(0);
			}
		} while (key != PAUSE_KEY);
		break;
	case ESCAPE_KEY: // exit the game
		MoveCursorToXY(0, 4 + TETRIS_BOARD_HEIGHT);
		exit(0);
	}
	removeShape(temp);
	addShapeToBoard();
	displayShape();
	removeShapeFromBoard();
}

// records the scores of the Tetris game in "Records"
unsigned short storeScore()
{
	time_t mytime = time(NULL);
	FILE *info = fopen("score.txt", "a+");
	fprintf(info, "Player Name: %s\n", player_name);
	fprintf(info, "Played Date: %s", ctime(&mytime));
	fprintf(info, "Score: %u\n", tetris.score);
	fprintf(info, "__________________________________________________\n");
	fclose(info);
	system("cls");
	printf("Press 'Y' to view past records."
		   "\nPress 'R' to play again."
		   "\nPress any other key to exit.");

	char key = getch();
	switch (key)
	{
	case 'Y':
	case 'y':
		system("cls");
		info = fopen("score.txt", "r");
		if (info == NULL)
		{
			printf("Error opening file.");
			break;
		}
		char ch;
		while ((ch = fgetc(info)) != EOF)
			printf("%c", ch);
		fclose(info);
		printf("\nPress any key to continue...");
		getch(); // Wait for user input
		break;
	case 'R':
	case 'r':
		game_on_flag_4 = TRUE;
		tetris = (Tetris){{{0}}, 0, DEFAULT_TIMER};
		system("cls");
		return TRUE;
	case 'M':
	case 'm':
		return FALSE;
	}
	exit(0);
}

// the main function to play Tetris
void PlayTetris()
{
	unsigned short play_again_flag = FALSE;
	do
	{
		displayOpening();
		LoadingScreen();
		// initialize the game board
		printf("Enter your name:\n"); // get the name of the player
		gets(player_name);			  // get the name of the player
		generateShape();
		displayBoard();
		refreshScreen();
		play_again_flag = storeScore();
	} while (play_again_flag);
}