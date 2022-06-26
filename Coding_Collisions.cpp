/*
 * Class: CS-330-T5621 Computer Graphics and Visualization
 * Instructor: Malcolm Wabara, M.S
 * Assignment: 8-3 Assignment: Coding Collisions
 * Student: Preston Burkhardt
 * Date: 25 June 2022
 */

#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);
void genBall(GLFWwindow* window, int key, int scancode, int action, int mods);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick {
public:
	float red, green, blue;
	float x, y, width;
	BRICKTYPE brick_type;
	ONOFF onoff;
	int hitCount = 0;
	int lifeCount;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb, int lifeCount) {
		brick_type = bt; 
		x = xx; 
		y = yy;
		width = ww; 
		red = rr;
		green = gg;
		blue = bb;
		onoff = ON;
		this->lifeCount = lifeCount;
	};

	void drawBrick() {
		if (onoff == ON) {
			double halfside = width / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};


class Circle {
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.01;
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b) {
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
	}

	void CheckCollision(Brick* brk) {
		if (brk->brick_type == REFLECTIVE) {
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width)) {
				direction = GetRandomDirection();
				x = x + 0.03;
				y = y + 0.04;
			}
		} 
		else if (brk->brick_type == DESTRUCTABLE) {
			++brk->hitCount;
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width) && (brk->hitCount >= brk->lifeCount)) {
				brk->onoff = OFF;
			}
			//direction = GetRandomDirection();
			//x = x + 0.03;
			//y = y + 0.04;
		}
	}

	int GetRandomDirection() {
		return (rand() % 8) + 1;
	}

	void MoveOneStep() {
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y > -1 + radius) {
				y -= speed;
			} else {
				direction = GetRandomDirection();
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius) {
				x += speed;
			} else {
				direction = GetRandomDirection();
			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius) {
				y += speed;
			} else {
				direction = GetRandomDirection();
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
			} else {
				direction = GetRandomDirection();
			}
		}
	}

	void DrawCircle() {
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};


vector<Circle> world;
vector<Brick> rowOfBricks1;


int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "Random World of Circles", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	//add bricks to top rows
	int numOfRows = 4;
	float startX = -0.95;
	float startY = 0.85;
	for (int i = 0; i < numOfRows; i++) {
		int randR = rand() % 2;
		int randG = rand() % 2;
		int randB = rand() % 2;
		while (startX <= 0.95) {
			Brick newBrick(DESTRUCTABLE, startX, startY, 0.1, randR, randG, randB, numOfRows);
			rowOfBricks1.push_back(newBrick);
			startX += 0.11;
		}
		startX = -0.95;
		startY -= 0.15;
	}


	//Brick testBrick(DESTRUCTABLE, 0.95, 0.70, 0.1, 1, 1, 0);


	Brick brick50(REFLECTIVE, 0.5, -0.33, 0.2, 1, 1, 0, 100);
	Brick brick51(REFLECTIVE, 0, 0, 0.2, 1, 0.5, 0.5, 100);

	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		//Movement
		for (int i = 0; i < world.size(); i++) {
			for (int j = 0; j < rowOfBricks1.size(); j++) {
				world[i].CheckCollision(&rowOfBricks1[j]);
			}


			world[i].CheckCollision(&brick50);
			world[i].CheckCollision(&brick51);
			world[i].MoveOneStep();
			world[i].DrawCircle();

		}

		for (int k = 0; k < rowOfBricks1.size(); k++) {
			rowOfBricks1[k].drawBrick();
		}

		//testBrick.drawBrick();

		brick50.drawBrick();
		brick51.drawBrick();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);


	glfwSetKeyCallback(window, genBall);
}

void genBall(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		double r, g, b;
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;
		Circle B(0, 0, 02, 2, 0.05, r, g, b);
		world.push_back(B);
	}
}