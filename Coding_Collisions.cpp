/*
 * Class: CS-330-T5621 Computer Graphics and Visualization
 * Instructor: Malcolm Wabara, M.S
 * Assignment: 8-3 Assignment: Coding Collisions
 * Student: Preston Burkhardt
 * Date: 25 June 2022
 */

#include <GLFW\glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
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

//void processInput(GLFWwindow* window, Brick &brick);
//void genBall(GLFWwindow* window, int key, int scancode, int action, int mods);

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
	double halfside;

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
		halfside = width / 2;
	};

	void drawBrick() {
		if (onoff == ON) {
			//double halfside = width / 2;

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


glm::vec2 GetRandomDirection() {
	auto x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	return { x, 1 };
}

class Circle {
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.01;
	glm::vec2 direction;
	bool deleted;

	Circle(double xx, double yy, double rr, glm::vec2 dir, float rad, float r, float g, float b) {
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
		deleted = false;
	}

	//checks for collisions with bricks
	void brickCollision(Brick* brk) {

		//reflective bricks
		if (brk->brick_type == REFLECTIVE) {
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width)) {
				direction *= -1;
				this->x = this->x + (direction.x * 0.03);
				this->y = this->y + (direction.y * 0.04);
				this->speed += 0.001;
			}
		} 
		//destructable bricks
		else if (brk->brick_type == DESTRUCTABLE) {
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width) && (brk->hitCount >= brk->lifeCount)) {
				brk->onoff = OFF;
			} else if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width) && (brk->hitCount < brk->lifeCount)) {
				++brk->hitCount;
				brk->red = rand() % 2;
				brk->green = rand() % 2;
				brk->blue = rand() % 2;
				while ((brk->red == 0) && (brk->green == 0) && (brk->blue == 0)) {
					brk->red = rand() % 2;
					brk->green = rand() % 2;
					brk->blue = rand() % 2;
				}
				direction *= -1;
				this->x = this->x + (direction.x * 0.03);
				this->y = this->y + (direction.y * 0.04);
			}
		}
	}

	//checks for circle collision
	void circleCollision(Circle& otherCircle) {
		auto circleDist = sqrt(std::pow(otherCircle.x - this->x, 2) + (std::pow(otherCircle.y - this->y, 2)));

		if (circleDist < this->radius + otherCircle.radius) {
			// COLLISION!
			direction.y *= -1;
			direction.x *= -1;


			this->x += direction.x * (this->radius - (circleDist / 2));
			this->y += direction.y * (this->radius - (circleDist / 2));

			otherCircle.direction.x *= -1;
			otherCircle.direction.y *= -1;

			otherCircle.x += otherCircle.direction.x * (otherCircle.radius - (circleDist / 2));
			otherCircle.y += otherCircle.direction.y * (otherCircle.radius - (circleDist / 2));

			this->radius = this->radius / 2;
			otherCircle.radius = otherCircle.radius / 2;
		}
	}

	void MoveOneStep() {
		if (this->y < -1 + this->radius || this->y > 1 - this->radius) {
			direction.y *= -1;
		}

		if (this->x < -1 + this->radius || this->x > 1 - this->radius) {
			direction.x *= -1;
		}

		this->x += direction.x * speed;
		this->y += direction.y * speed;
	}

	void DrawCircle() {
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * this->radius) + this->x, (sin(degInRad) * this->radius) + this->y);
		}
		glEnd();
	}
};


vector<Circle> world;
vector<Brick> rowOfBricks1;

void processInput(GLFWwindow* window, Brick& brick);
void genBall(GLFWwindow* window, int key, int scancode, int action, int mods);
int lifeCount = 5;


int main(void) {

	cout << "GOAL: " << endl;
	cout << "   Bounce the ball into the blocks along the top of the screen while not letting it hit the bottom of the screen." << endl;
	cout << "   Blocks in different rows have varying amounts of hit points." << endl;
	cout << "   Deplete a block's hit points by hitting it with the ball." << endl;
	cout << "   If two or more balls collide, their radius is halved." << endl;
	cout << "   The balls' speed increases as they are reflected with the bottom block." << endl;
	cout << "   You have 5 lives to complete the goal." << endl << endl;

	cout << "CONTROLS: " << endl;
	cout << "   MOVEMENT - " << endl;
	cout << "      A - Left" << endl;
	cout << "      D - Right" << endl << endl;

	cout << "   BALL - " << endl;
	cout << "      SPACE BAR - Generate one ball" << endl << endl;

	cout << "*******************************************" << endl << endl;

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

		//avoids setting the brick to invisible color
		while ((randR == 0) && (randG == 0) && (randB == 0)) {
			randR = rand() % 2;
			randG = rand() % 2;
			randB = rand() % 2;
		}

		while (startX <= 0.95) {
			Brick newBrick(DESTRUCTABLE, startX, startY, 0.1, randR, randG, randB, numOfRows - i);
			rowOfBricks1.push_back(newBrick);
			startX += 0.11;
		}
		startX = -0.95;
		startY -= 0.15;
	}


	//Brick testBrick(DESTRUCTABLE, 0.95, 0.70, 0.1, 1, 1, 0);

	Brick brick51(REFLECTIVE, 0, -0.9, 0.2, 1, 0.5, 0.5, 100);

	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window, brick51);

		//Movement
		for (int i = 0; i < world.size(); i++) {
			Circle &currCircle = world[i];

			//check brick collision
			for (int j = 0; j < rowOfBricks1.size(); j++) {
				currCircle.brickCollision(&rowOfBricks1[j]);
			}

			//check circle collision
			if (i + 1 < world.size()) {
				for (int l = i + 1; l < world.size(); l++) {
					Circle& otherCircle = world[l];
					currCircle.circleCollision(otherCircle);
				}
			}

			//count a miss
			if (currCircle.y - currCircle.radius <= -1.0) {
				lifeCount--;
				if (lifeCount < 0) {
					cout << endl << "*** GAME OVER ***" << endl;
					return 1;
				}
				cout << "Lives Remaining: " << lifeCount << endl;
			}

			world[i].brickCollision(&brick51);
			world[i].MoveOneStep();
			world[i].DrawCircle();
		}

		for (int k = 0; k < rowOfBricks1.size(); k++) {
			rowOfBricks1[k].drawBrick();
		}

		//testBrick.drawBrick();

		brick51.drawBrick();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window, Brick& brick) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	//refective brick movement
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		brick.x -= 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		brick.x += 0.01;
	}


	glfwSetKeyCallback(window, genBall);
}

void genBall(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		double r, g, b;
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;

		//avoids setting the ball to invisible color
		while (r == 0.0 && g == 0.0 && b == 0.0) {
			r = rand() / 10000;
			g = rand() / 10000;
			b = rand() / 10000;
		}

		Circle B(0, -0.55, 02, GetRandomDirection(), 0.05, r, g, b);
		world.push_back(B);
	}
}