#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 700
#define MAP_WIDTH 4000
#define MAP_HEIGHT 4000
#define MAP_X_STARTING_POINT -1500
#define MAP_Y_STARTING_POINT -1650
#define TIME_OF_THE_GAME 60
#define HOW_MANY_DIRECTIONS 4
#define HOW_MANY_SCORES_TO_GRADE1 10
#define HOW_MANY_SCORES_TO_GRADE2 15
#define HOW_MANY_SCORES_TO_GRADE3 25
#define HOW_MANY_LIVES 5
#define IMMORTALITY_TIME 3
#define PLAYER_WIDTH 60
#define PLAYER_HEIGHT 80
#define PLAYER_SPEED 50
#define PLAYER_BULLET_WIDTH 15
#define PLAYER_BULLET_HEIGHT 5
#define PLAYER_BULLET_SPEED 1
#define HOW_MANY_SHOTS 100
#define BOUNS_MULTIPLIER 1.2
#define MAP_X_STARTING_POINT_FOR_PLAYER_BULLETS 2500
#define MAP_X_ENDING_POINT_FOR_PLAYER_BULLETS 2490
#define MAP_Y_STARTING_POINT_FOR_PLAYER_BULLETS 2365
#define MAP_Y_ENDING_POINT_FOR_PLAYER_BULLETS  2335
#define ENEMY_WIDTH 60
#define ENEMY_HEIGHT 80
#define NUMBER_OF_ENEMIES 25
#define NUMBER_OF_ENEMY_BULLETS 100
#define BULLET_ENEMY_WIDTH 30
#define BULLET_ENEMY_HEIGHT 30
#define MAP_X_ENDING_POINT_FOR_ENEMY_BULLETS 2480
#define MAP_Y_ENDING_POINT_FOR_ENEMY_BULLETS 2340
#define ENEMY_TYPE2_WIDTH 60
#define ENEMY_TYPE2_HEIGHT 80
#define NUMBER_OF_ENEMIES_TYPE2 40
#define NUMBER_OF_ENEMY_BULLETS_TYPE2 100
#define BULLET_ENEMY2_WIDTH 30
#define BULLET_ENEMY2_HEIGHT 30
#define ENEMY_TYPE3_WIDTH 60
#define ENEMY_TYPE3_HEIGHT 80
#define NUMBER_OF_ENEMIES_TYPE3 60
#define NUMBER_OF_ENEMY_BULLETS_TYPE3 20
#define BULLET_ENEMY3_WIDTH 50
#define BULLET_ENEMY3_HEIGHT 50
#define ENEMY_TYPE3_SPEED 0.3
#define MAP_X_ENDING_POINT_FOR_ENEMY_TYPE3 2470
#define MAP_Y_ENDING_POINT_FOR_ENEMY_TYPE3 2310
#define NUMBER_OF_DYNAMITE 50
#define DYNAMITE_WIDTH 60
#define DYNAMITE_HEIGHT 60
#define EXPLOSION_WIDTH 200
#define EXPLOSION_HEIGHT 200
#define FIRE_HEIGHT 30
#define FIRE_WIDTH 30
#define NUMBER_OF_MEDICATION 75
#define MEDICATION_WIDTH 30
#define MEDICATION_HEIGHT 50
#define NUMBER_OF_SQUARE_LIGHT 50
#define SQUARE_LIGHT_WIDTH 400
#define SQUARE_LIGHT_HEIGHT 400
#define NUMBER_OF_RECTANGLE_LIGHT 100
#define RECTANGLE_LIGHT_WIDTH 500
#define RECTANGLE_LIGHT_HEIGHT 300

struct Score {
	char playerName[100];
	int score = 0;
};

struct Coordinates {
	double x;
	double y;
};

struct Bullet {
	Coordinates coordinate;
	int dir;
	bool isShotLaunched = false;
};

struct Dynamite {
	Coordinates coordinate;
	double timeOfExplosion;
	Bullet bullets[HOW_MANY_DIRECTIONS];
};

struct Medication {
	Coordinates coordinate;
	double start;
};

struct Light {
	Coordinates coordinate;
	double start;
	double end;
};

struct Enemy {
	Coordinates coordinate;
	int dir;
	double dirChange;
	Bullet bullet[NUMBER_OF_ENEMY_BULLETS];
	double nextBulletShot = 0;
	bool dead = false;
};

// draw a text txt on surface screen, starting from the coordinate (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};

// draw a surface sprite on a surface screen in coordinate (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};

// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};

// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};

// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

void WhatIfYouWalkIntoEnemy(int numberOfEnemies, Enemy enemyCoordinates[], Bullet playerBulletCoordinates[], double worldTime, bool& isThePlayerAlive, 
	int& lives, double& timeOfTheLastDeath, int& scores, int numberOfEnemyBullets, int enemyBulletWidth, int enemyBulletHeight, Coordinates playerCoordinates,
	Enemy enemyCoordinatesType3[], double& bonusMultiplier, int& numberOfEnemiesForSpecifiedType)
{
	// collision between player and enemy
	for (int j = 0; j < numberOfEnemies; j++)
	{
		if (isThePlayerAlive == true && enemyCoordinates[j].dead == false && enemyCoordinates[j].coordinate.x - (ENEMY_WIDTH / 2) < SCREEN_WIDTH / 2 + (PLAYER_WIDTH / 2) &&
			enemyCoordinates[j].coordinate.x + (ENEMY_WIDTH / 2) > SCREEN_WIDTH / 2 - (PLAYER_WIDTH / 2) &&
			enemyCoordinates[j].coordinate.y - (ENEMY_HEIGHT / 2) < SCREEN_HEIGHT / 2 + (PLAYER_HEIGHT / 2) &&
			enemyCoordinates[j].coordinate.y + (ENEMY_HEIGHT / 2) > SCREEN_HEIGHT / 2 - (PLAYER_HEIGHT / 2))
		{
			lives--;
			scores--;
			timeOfTheLastDeath = worldTime;
			break;
		}
		// what happens if player3 will be in the edge of the map
		if (MAP_X_ENDING_POINT_FOR_ENEMY_TYPE3 + (-1 * playerCoordinates.x) + enemyCoordinates[j].coordinate.x < (SCREEN_WIDTH / 2) ||
			MAP_Y_ENDING_POINT_FOR_ENEMY_TYPE3 + (ENEMY_TYPE3_HEIGHT)+(-1 * playerCoordinates.y) + enemyCoordinates[j].coordinate.y >(MAP_HEIGHT + (SCREEN_HEIGHT / 2)) ||
			MAP_X_ENDING_POINT_FOR_ENEMY_TYPE3 + (ENEMY_TYPE3_WIDTH)+(-1 * playerCoordinates.x) + enemyCoordinates[j].coordinate.x > (MAP_WIDTH + (SCREEN_WIDTH / 2)) ||
			MAP_Y_ENDING_POINT_FOR_ENEMY_TYPE3 + (-1 * playerCoordinates.y) + enemyCoordinates[j].coordinate.y < (SCREEN_HEIGHT / 2))
		{
			if (enemyCoordinatesType3[j].dir == 1)
			{
				enemyCoordinatesType3[j].dir = 2;
			}
			else if (enemyCoordinatesType3[j].dir == 2)
			{
				enemyCoordinatesType3[j].dir = 1;
			}
			else if (enemyCoordinatesType3[j].dir == 3)
			{
				enemyCoordinatesType3[j].dir = 4;
			}
			else if (enemyCoordinatesType3[j].dir == 4)
			{
				enemyCoordinatesType3[j].dir = 3;
			}
		}
		//what happens if you shoot to enemy
		for (int i = 0; i < HOW_MANY_SHOTS; i++)
		{
			if (playerBulletCoordinates[i].isShotLaunched == true && enemyCoordinates[j].dead == false &&
				enemyCoordinates[j].coordinate.x - (ENEMY_WIDTH / 2) < playerBulletCoordinates[i].coordinate.x + (PLAYER_BULLET_WIDTH / 2) &&
				enemyCoordinates[j].coordinate.x + (ENEMY_WIDTH / 2) > playerBulletCoordinates[i].coordinate.x - (PLAYER_BULLET_WIDTH / 2) &&
				enemyCoordinates[j].coordinate.y - (ENEMY_HEIGHT / 2) < playerBulletCoordinates[i].coordinate.y + (PLAYER_BULLET_HEIGHT / 2) &&
				enemyCoordinates[j].coordinate.y + (ENEMY_HEIGHT / 2) > playerBulletCoordinates[i].coordinate.y - (PLAYER_BULLET_HEIGHT / 2))
			{
				enemyCoordinates[j].dead = true;
				numberOfEnemiesForSpecifiedType--;
				playerBulletCoordinates[i].isShotLaunched = false;
				playerBulletCoordinates[i].coordinate.x = SCREEN_WIDTH / 2;
				playerBulletCoordinates[i].coordinate.y = SCREEN_HEIGHT / 2;
				scores += 2 * bonusMultiplier; //additional point for shooting in series
				bonusMultiplier *= BOUNS_MULTIPLIER;
			}
			if (playerBulletCoordinates[i].isShotLaunched == true &&
				MAP_X_ENDING_POINT_FOR_PLAYER_BULLETS + (-1 * playerCoordinates.x) + playerBulletCoordinates[i].coordinate.x < (SCREEN_WIDTH / 2) ||
				MAP_Y_STARTING_POINT_FOR_PLAYER_BULLETS + (PLAYER_BULLET_HEIGHT)+(-1 * playerCoordinates.y) + playerBulletCoordinates[i].coordinate.y >(MAP_HEIGHT + (SCREEN_HEIGHT / 2)) ||
				MAP_X_STARTING_POINT_FOR_PLAYER_BULLETS + (PLAYER_BULLET_WIDTH)+(-1 * playerCoordinates.x) + playerBulletCoordinates[i].coordinate.x > (MAP_WIDTH + (SCREEN_WIDTH / 2)) ||
				MAP_Y_ENDING_POINT_FOR_PLAYER_BULLETS + (-1 * playerCoordinates.y) + playerBulletCoordinates[i].coordinate.y < (SCREEN_HEIGHT / 2))
			{
				playerBulletCoordinates[i].isShotLaunched = false;
			}
		}
		// what happens if enemy shoots you
		for (int k = 0; k < numberOfEnemyBullets; k++)
		{
			if (isThePlayerAlive == true && enemyCoordinates[j].bullet[k].isShotLaunched == true &&
				SCREEN_WIDTH / 2 - (PLAYER_WIDTH / 2) < enemyCoordinates[j].bullet[k].coordinate.x + (enemyBulletWidth / 2) &&
				SCREEN_WIDTH / 2 + (PLAYER_WIDTH / 2) > enemyCoordinates[j].bullet[k].coordinate.x - (enemyBulletWidth / 2) &&
				SCREEN_HEIGHT / 2 - (PLAYER_HEIGHT / 2) < enemyCoordinates[j].bullet[k].coordinate.y + (enemyBulletHeight / 2) &&
				SCREEN_HEIGHT / 2 + (PLAYER_HEIGHT / 2) > enemyCoordinates[j].bullet[k].coordinate.y - (enemyBulletHeight / 2))
			{
				timeOfTheLastDeath = worldTime;
				scores--;
				lives--;
				bonusMultiplier = 1;
				enemyCoordinates[j].bullet[k].isShotLaunched = false;
				break;
			}
			//what happens if enemy bullets will be in the edge of the map
			if (enemyCoordinates[j].bullet[k].isShotLaunched == true &&
				MAP_X_ENDING_POINT_FOR_ENEMY_BULLETS + (-1 * playerCoordinates.x) + enemyCoordinates[j].bullet[k].coordinate.x < (SCREEN_WIDTH / 2) ||
				MAP_Y_ENDING_POINT_FOR_ENEMY_BULLETS + (enemyBulletHeight)+(-1 * playerCoordinates.y) + enemyCoordinates[j].bullet[k].coordinate.y >(MAP_HEIGHT + (SCREEN_HEIGHT / 2)) ||
				MAP_X_ENDING_POINT_FOR_ENEMY_BULLETS + (enemyBulletWidth)+(-1 * playerCoordinates.x) + enemyCoordinates[j].bullet[k].coordinate.x > (MAP_WIDTH + (SCREEN_WIDTH / 2)) ||
				MAP_Y_ENDING_POINT_FOR_ENEMY_BULLETS + (-1 * playerCoordinates.y) + enemyCoordinates[j].bullet[k].coordinate.y < (SCREEN_HEIGHT / 2))
			{
				enemyCoordinates[j].bullet[k].isShotLaunched = false;
			}
		}
	}
}

void WhatIfYouWalkIntoMedication(double& worldTime, int& lives, Medication medicationCoordinates[])
{
	for (int j = 0; j < NUMBER_OF_MEDICATION; j++)
	{
		if (medicationCoordinates[j].start < worldTime &&
			medicationCoordinates[j].coordinate.x - (MEDICATION_WIDTH / 2) < SCREEN_WIDTH / 2 + (PLAYER_WIDTH / 2) &&
			medicationCoordinates[j].coordinate.x + (MEDICATION_WIDTH / 2) > SCREEN_WIDTH / 2 - (PLAYER_WIDTH / 2) &&
			medicationCoordinates[j].coordinate.y - (MEDICATION_HEIGHT / 2) < SCREEN_HEIGHT / 2 + (PLAYER_HEIGHT / 2) &&
			medicationCoordinates[j].coordinate.y + (MEDICATION_HEIGHT / 2) > SCREEN_HEIGHT / 2 - (PLAYER_HEIGHT / 2) && (lives < 5))
		{
			lives++;
			medicationCoordinates[j].start = TIME_OF_THE_GAME + 1;
			break;
		}
	}
}

void WhatIfYouWalkIntoDynamite(bool& isThePlayerAlive, Dynamite dynamiteCoordinates[], double& worldTime, double& timeOfTheLastDeath, int& lives,
	double& bonusMultiplier, int& scores, Bullet playerBulletCoordinates[], Coordinates playerCoordinates)
{
	// what happens if you walk into dynamite
	for (int j = 0; j < NUMBER_OF_DYNAMITE; j++)
	{
		if (isThePlayerAlive == true && dynamiteCoordinates[j].timeOfExplosion == 0.0 &&
			dynamiteCoordinates[j].coordinate.x - (DYNAMITE_WIDTH / 2) < SCREEN_WIDTH / 2 + (PLAYER_WIDTH / 2) &&
			dynamiteCoordinates[j].coordinate.x + (DYNAMITE_WIDTH / 2) > SCREEN_WIDTH / 2 - (PLAYER_WIDTH / 2) &&
			dynamiteCoordinates[j].coordinate.y - (DYNAMITE_HEIGHT / 2) < SCREEN_HEIGHT / 2 + (PLAYER_HEIGHT / 2) &&
			dynamiteCoordinates[j].coordinate.y + (DYNAMITE_HEIGHT / 2) > SCREEN_HEIGHT / 2 - (PLAYER_HEIGHT / 2))
		{
			timeOfTheLastDeath = worldTime;
			lives--;
			dynamiteCoordinates[j].timeOfExplosion = worldTime;
			break;
		}
		if (isThePlayerAlive == true && dynamiteCoordinates[j].timeOfExplosion != 0.0 && dynamiteCoordinates[j].timeOfExplosion < worldTime)
		{
			// animation of explosion
			int animation = (worldTime - dynamiteCoordinates[j].timeOfExplosion) / 0.05;
		
			if (animation >= 0 && animation < 15 && dynamiteCoordinates[j].coordinate.x - ((DYNAMITE_WIDTH + (animation * 10)) / 2) < SCREEN_WIDTH / 2 + (PLAYER_WIDTH / 2) &&
				dynamiteCoordinates[j].coordinate.x + ((DYNAMITE_WIDTH + (animation * 10)) / 2) > SCREEN_WIDTH / 2 - (PLAYER_WIDTH / 2) &&
				dynamiteCoordinates[j].coordinate.y - ((DYNAMITE_HEIGHT + (animation * 10)) / 2) < SCREEN_HEIGHT / 2 + (PLAYER_HEIGHT / 2) &&
				dynamiteCoordinates[j].coordinate.y + ((DYNAMITE_HEIGHT + (animation * 10)) / 2) > SCREEN_HEIGHT / 2 - (PLAYER_HEIGHT / 2))
			{
				scores--;
				timeOfTheLastDeath = worldTime;
				lives--;
				break;
			}
		}
		// dynamite shooting
		for (int i = 0; i < HOW_MANY_DIRECTIONS; i++)
		{
			if (dynamiteCoordinates[j].bullets[i].isShotLaunched == true && isThePlayerAlive == true &&
				dynamiteCoordinates[j].bullets[i].coordinate.x - (FIRE_WIDTH / 2) < SCREEN_WIDTH / 2 + (PLAYER_WIDTH / 2) &&
				dynamiteCoordinates[j].bullets[i].coordinate.x + (FIRE_WIDTH / 2) > SCREEN_WIDTH / 2 - (PLAYER_WIDTH / 2) &&
				dynamiteCoordinates[j].bullets[i].coordinate.y - (FIRE_HEIGHT / 2) < SCREEN_HEIGHT / 2 + (PLAYER_HEIGHT / 2) &&
				dynamiteCoordinates[j].bullets[i].coordinate.y + (FIRE_HEIGHT / 2) > SCREEN_HEIGHT / 2 - (PLAYER_HEIGHT / 2))
			{
				dynamiteCoordinates[j].bullets[i].isShotLaunched = false;
				scores--;
				timeOfTheLastDeath = worldTime;
				lives--;
				bonusMultiplier = 1;
				break;
			}
			// what happens if dynamite bullets will be in the edge of the map
			if (dynamiteCoordinates[j].bullets[i].isShotLaunched == true &&
				MAP_X_ENDING_POINT_FOR_ENEMY_BULLETS + (-1 * playerCoordinates.x) + dynamiteCoordinates[j].bullets[i].coordinate.x < (SCREEN_WIDTH / 2) ||
				MAP_Y_ENDING_POINT_FOR_ENEMY_BULLETS + (FIRE_HEIGHT)+(-1 * playerCoordinates.y) + dynamiteCoordinates[j].bullets[i].coordinate.y >(MAP_HEIGHT + (SCREEN_HEIGHT / 2)) ||
				MAP_X_ENDING_POINT_FOR_ENEMY_BULLETS + (FIRE_WIDTH)+(-1 * playerCoordinates.x) + dynamiteCoordinates[j].bullets[i].coordinate.x > (MAP_WIDTH + (SCREEN_WIDTH / 2)) ||
				MAP_Y_ENDING_POINT_FOR_ENEMY_BULLETS + (-1 * playerCoordinates.y) + dynamiteCoordinates[j].bullets[i].coordinate.y < (SCREEN_HEIGHT / 2))
			{
				dynamiteCoordinates[j].bullets[i].isShotLaunched = false;
			}
		}
		//what happens if you shoot dynamite
		for (int i = 0; i < HOW_MANY_SHOTS; i++)
		{
			if (playerBulletCoordinates[i].isShotLaunched == true &&
				dynamiteCoordinates[j].coordinate.x - (DYNAMITE_WIDTH / 2) < playerBulletCoordinates[i].coordinate.x + (PLAYER_BULLET_WIDTH / 2) &&
				dynamiteCoordinates[j].coordinate.x + (DYNAMITE_WIDTH / 2) > playerBulletCoordinates[i].coordinate.x - (PLAYER_BULLET_WIDTH / 2) &&
				dynamiteCoordinates[j].coordinate.y - (DYNAMITE_HEIGHT / 2) < playerBulletCoordinates[i].coordinate.y + (PLAYER_BULLET_HEIGHT / 2) &&
				dynamiteCoordinates[j].coordinate.y + (DYNAMITE_HEIGHT / 2) > playerBulletCoordinates[i].coordinate.y - (PLAYER_BULLET_HEIGHT / 2))
			{
				dynamiteCoordinates[j].timeOfExplosion = worldTime;
				playerBulletCoordinates[i].isShotLaunched = false;
				playerBulletCoordinates[i].coordinate.x = SCREEN_WIDTH / 2;
				playerBulletCoordinates[i].coordinate.y = SCREEN_HEIGHT / 2;
			}
		}
	}
}

void WhatIfYouWalkIntoLight(int numberOfLight, Light lightCoordinates[], double worldTime, bool& isThePlayerAlive, int& lives, double& timeOfTheLastDeath, 
	int& scores, Coordinates playerCoordinates, int lightWidth, int lightHeight, double& bonusMultiplier)
{
	for (int j = 0; j < numberOfLight; j++)
	{
		if (isThePlayerAlive == true && lightCoordinates[j].end - 0.01 < worldTime && lightCoordinates[j].end + 0.01 > worldTime &&
			lightCoordinates[j].coordinate.x - (lightWidth / 2) < SCREEN_WIDTH / 2 + (PLAYER_WIDTH / 2) &&
			lightCoordinates[j].coordinate.x + (lightWidth / 2) > SCREEN_WIDTH / 2 - (PLAYER_WIDTH / 2) &&
			lightCoordinates[j].coordinate.y - (lightHeight / 2) < SCREEN_HEIGHT / 2 + (PLAYER_HEIGHT / 2) &&
			lightCoordinates[j].coordinate.y + (lightHeight / 2) > SCREEN_HEIGHT / 2 - (PLAYER_HEIGHT / 2))
		{
			lives--;
			scores--;
			timeOfTheLastDeath = worldTime;
			break;
		}
	}
}

void DrawingCoordinatesForEnemies(int whatCharacter, bool& emptyPlace, int& drawingCoordinateX, int& drawingCoordinateY, int characterHeight, int characterWidth,
	Enemy enemyCoordinatesForSpecifiedType[], Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[], Dynamite dynamiteCoordinates[],
	Medication medicationCoordinates[], int numberOfCharacters)
{
	srand(time(NULL));
	for (int i = 0; i < numberOfCharacters; i++)
	{
		while (emptyPlace == false)
		{
			//drawing coordinates of enemies
			drawingCoordinateX = rand() % (MAP_WIDTH - characterWidth) + (MAP_X_STARTING_POINT + (characterWidth / 2));
			drawingCoordinateY = rand() % (MAP_HEIGHT - characterHeight) + (MAP_Y_STARTING_POINT + (characterHeight / 2));
			emptyPlace = true;

			//checking the collisions
			if ((SCREEN_WIDTH / 2) - (PLAYER_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
				(SCREEN_WIDTH / 2) + (PLAYER_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
				(SCREEN_HEIGHT / 2) - (PLAYER_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
				(SCREEN_HEIGHT / 2) + (PLAYER_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
			{
				emptyPlace = false;
			}
			for (int j = 0; j < NUMBER_OF_ENEMIES; j++)
			{
				if (enemyCoordinates[j].coordinate.x - (ENEMY_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
					enemyCoordinates[j].coordinate.x + (ENEMY_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
					enemyCoordinates[j].coordinate.y - (ENEMY_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
					enemyCoordinates[j].coordinate.y + (ENEMY_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
				{
					emptyPlace = false;
					break;
				}
			}
			if (whatCharacter == 2 || whatCharacter == 3)
			{
				for (int j = 0; j < NUMBER_OF_ENEMIES_TYPE2; j++)
				{
					if ((enemyCoordinatesType2[j].coordinate.x - (ENEMY_TYPE2_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
						enemyCoordinatesType2[j].coordinate.x + (ENEMY_TYPE2_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
						enemyCoordinatesType2[j].coordinate.y - (ENEMY_TYPE2_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
						enemyCoordinatesType2[j].coordinate.y + (ENEMY_TYPE2_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2)))
					{
						emptyPlace = false;
						break;
					}
				}
			}
			if (whatCharacter == 3)
			{
				for (int j = 0; j < NUMBER_OF_ENEMIES_TYPE3; j++)
				{
					if (enemyCoordinatesType3[j].coordinate.x - (ENEMY_TYPE3_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
						enemyCoordinatesType3[j].coordinate.x + (ENEMY_TYPE3_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
						enemyCoordinatesType3[j].coordinate.y - (ENEMY_TYPE3_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
						enemyCoordinatesType3[j].coordinate.y + (ENEMY_TYPE3_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
					{
						emptyPlace = false;
						break;
					}
				}
			}
		}
		enemyCoordinatesForSpecifiedType[i].coordinate.x = drawingCoordinateX;
		enemyCoordinatesForSpecifiedType[i].coordinate.y = drawingCoordinateY;

		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS; j++)
		{
			enemyCoordinatesForSpecifiedType[i].bullet[j].coordinate.x = drawingCoordinateX;
			enemyCoordinatesForSpecifiedType[i].bullet[j].coordinate.y = drawingCoordinateY;
			enemyCoordinatesForSpecifiedType[i].bullet[j].isShotLaunched = false;
		}
		enemyCoordinatesForSpecifiedType[i].nextBulletShot = (rand() % 200) / 100.0;
		emptyPlace = false;
	}
}

void DrawingCoordinatesForDynamites(bool& emptyPlace, int& drawingCoordinateX, int& drawingCoordinateY, int characterHeight, int characterWidth, Dynamite enemyCoordinatesForSpecifiedType[], Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[], Dynamite dynamiteCoordinates[], Medication medicationCoordinates[], int numberOfCharacters, double worldTime)
{
	srand(time(NULL));
	for (int i = 0; i < numberOfCharacters; i++)
	{
		while (emptyPlace == false)
		{
			// drawing coordinates of dynamite
			drawingCoordinateX = rand() % (MAP_WIDTH - characterWidth) + (MAP_X_STARTING_POINT + (characterWidth / 2));
			drawingCoordinateY = rand() % (MAP_HEIGHT - characterHeight) + (MAP_Y_STARTING_POINT + (characterHeight / 2));
			emptyPlace = true;

			// checking the collisions
			if ((SCREEN_WIDTH / 2) - (PLAYER_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
				(SCREEN_WIDTH / 2) + (PLAYER_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
				(SCREEN_HEIGHT / 2) - (PLAYER_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
				(SCREEN_HEIGHT / 2) + (PLAYER_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
			{
				emptyPlace = false;
			}
			for (int j = 0; j < NUMBER_OF_ENEMIES; j++)
			{
				if (enemyCoordinates[j].coordinate.x - (ENEMY_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
					enemyCoordinates[j].coordinate.x + (ENEMY_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
					enemyCoordinates[j].coordinate.y - (ENEMY_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
					enemyCoordinates[j].coordinate.y + (ENEMY_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
				{
					emptyPlace = false;
					break;
				}
			}
			for (int j = 0; j < NUMBER_OF_ENEMIES_TYPE2; j++)
			{
				if ((enemyCoordinatesType2[j].coordinate.x - (ENEMY_TYPE2_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
					enemyCoordinatesType2[j].coordinate.x + (ENEMY_TYPE2_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
					enemyCoordinatesType2[j].coordinate.y - (ENEMY_TYPE2_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
					enemyCoordinatesType2[j].coordinate.y + (ENEMY_TYPE2_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2)))
				{
					emptyPlace = false;
					break;
				}
			}
			for (int j = 0; j < NUMBER_OF_ENEMIES_TYPE3; j++)
			{
				if (enemyCoordinatesType3[j].coordinate.x - (ENEMY_TYPE3_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
					enemyCoordinatesType3[j].coordinate.x + (ENEMY_TYPE3_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
					enemyCoordinatesType3[j].coordinate.y - (ENEMY_TYPE3_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
					enemyCoordinatesType3[j].coordinate.y + (ENEMY_TYPE3_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
				{
					emptyPlace = false;
					break;
				}
			}
			for (int j = 0; j < NUMBER_OF_DYNAMITE; j++)
			{
				if (dynamiteCoordinates[j].coordinate.x - (DYNAMITE_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
					dynamiteCoordinates[j].coordinate.x + (DYNAMITE_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
					dynamiteCoordinates[j].coordinate.y - (DYNAMITE_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
					dynamiteCoordinates[j].coordinate.y + (DYNAMITE_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
				{
					emptyPlace = false;
					break;
				}
			}
		}
		enemyCoordinatesForSpecifiedType[i].coordinate.x = drawingCoordinateX;
		enemyCoordinatesForSpecifiedType[i].coordinate.y = drawingCoordinateY;
		enemyCoordinatesForSpecifiedType[i].timeOfExplosion = 0.0;

		for (int j = 0; j < 4; j++)
		{
			enemyCoordinatesForSpecifiedType[i].bullets[j].isShotLaunched = false;
			enemyCoordinatesForSpecifiedType[i].bullets[j].coordinate.x = drawingCoordinateX;
			enemyCoordinatesForSpecifiedType[i].bullets[j].coordinate.y = drawingCoordinateY;
			enemyCoordinatesForSpecifiedType[i].bullets[j].dir = j;
		}
		emptyPlace = false;
	}
}

void DrawingCoordinatesForMedicines(bool& emptyPlace, int& drawingCoordinateX, int& drawingCoordinateY, int characterHeight, int characterWidth, Medication enemyCoordinatesForSpecifiedType[], Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[], Dynamite dynamiteCoordinates[], Medication medicationCoordinates[], int numberOfCharacters)
{
	srand(time(NULL));
	for (int i = 0; i < numberOfCharacters; i++)
	{
		while (emptyPlace == false)
		{ 
			//drawing coordinates of medicine
			drawingCoordinateX = rand() % (MAP_WIDTH - characterWidth) + (MAP_X_STARTING_POINT + (characterWidth / 2));
			drawingCoordinateY = rand() % (MAP_HEIGHT - characterHeight) + (MAP_Y_STARTING_POINT + (characterHeight / 2));
			emptyPlace = true;

			//checking the collisions
			if ((SCREEN_WIDTH / 2) - (PLAYER_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
				(SCREEN_WIDTH / 2) + (PLAYER_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
				(SCREEN_HEIGHT / 2) - (PLAYER_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
				(SCREEN_HEIGHT / 2) + (PLAYER_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
			{
				emptyPlace = false;
			}
			for (int j = 0; j < NUMBER_OF_ENEMIES; j++)
			{
				if (enemyCoordinates[j].coordinate.x - (ENEMY_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
					enemyCoordinates[j].coordinate.x + (ENEMY_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
					enemyCoordinates[j].coordinate.y - (ENEMY_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
					enemyCoordinates[j].coordinate.y + (ENEMY_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
				{
					emptyPlace = false;
					break;
				}
			}
			for (int j = 0; j < NUMBER_OF_ENEMIES_TYPE2; j++)
			{
				if ((enemyCoordinatesType2[j].coordinate.x - (ENEMY_TYPE2_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
					enemyCoordinatesType2[j].coordinate.x + (ENEMY_TYPE2_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
					enemyCoordinatesType2[j].coordinate.y - (ENEMY_TYPE2_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
					enemyCoordinatesType2[j].coordinate.y + (ENEMY_TYPE2_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2)))
				{
					emptyPlace = false;
					break;
				}
			}
			for (int j = 0; j < NUMBER_OF_ENEMIES_TYPE3; j++)
			{
				if (enemyCoordinatesType3[j].coordinate.x - (ENEMY_TYPE3_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
					enemyCoordinatesType3[j].coordinate.x + (ENEMY_TYPE3_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
					enemyCoordinatesType3[j].coordinate.y - (ENEMY_TYPE3_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
					enemyCoordinatesType3[j].coordinate.y + (ENEMY_TYPE3_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
				{
					emptyPlace = false;
					break;
				}
			}
			for (int j = 0; j < NUMBER_OF_DYNAMITE; j++)
			{
				if (dynamiteCoordinates[j].coordinate.x - (DYNAMITE_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
					dynamiteCoordinates[j].coordinate.x + (DYNAMITE_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
					dynamiteCoordinates[j].coordinate.y - (DYNAMITE_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
					dynamiteCoordinates[j].coordinate.y + (DYNAMITE_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
				{
					emptyPlace = false;
					break;
				}
			}
			for (int j = 0; j < NUMBER_OF_MEDICATION; j++)
			{
				if (medicationCoordinates[j].coordinate.x - (MEDICATION_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
					medicationCoordinates[j].coordinate.x + (MEDICATION_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
					medicationCoordinates[j].coordinate.y - (MEDICATION_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
					medicationCoordinates[j].coordinate.y + (MEDICATION_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
				{
					emptyPlace = false;
					break;
				}
			}
		}
		enemyCoordinatesForSpecifiedType[i].coordinate.x = drawingCoordinateX;
		enemyCoordinatesForSpecifiedType[i].coordinate.y = drawingCoordinateY;
		enemyCoordinatesForSpecifiedType[i].start = (rand() % (TIME_OF_THE_GAME * 100) / 100);

		emptyPlace = false;
	}
}

void DrawingCoordinatesForLight(int whatObject, bool& emptyPlace, int& drawingCoordinateX, int& drawingCoordinateY, int characterHeight, int characterWidth, Light lightCoordinates[], Light squareLightCoordinates[], int numberOfCharacters)
{
	srand(time(NULL));
	for (int i = 0; i < numberOfCharacters; i++)
	{
		while (emptyPlace == false)
		{
			//drawing coordinates of light
			drawingCoordinateX = rand() % (MAP_WIDTH - characterWidth) + (MAP_X_STARTING_POINT + (characterWidth / 2));
			drawingCoordinateY = rand() % (MAP_HEIGHT - characterHeight) + (MAP_Y_STARTING_POINT + (characterHeight / 2));
			lightCoordinates[i].start = (rand() % (TIME_OF_THE_GAME * 100) / 100.0);
			lightCoordinates[i].end = lightCoordinates[i].start + 3;
			emptyPlace = true;

			//checking the collisions
			for (int j = 0; j < i; j++)
			{
				if (lightCoordinates[j].start > lightCoordinates[i].end || lightCoordinates[j].end < lightCoordinates[i].start)
					continue;
				if (lightCoordinates[j].coordinate.x - (characterWidth / 2) < drawingCoordinateX + (characterWidth / 2) &&
					lightCoordinates[j].coordinate.x + (characterWidth / 2) > drawingCoordinateX - (characterWidth / 2) &&
					lightCoordinates[j].coordinate.y - (characterHeight / 2) < drawingCoordinateY + (characterHeight / 2) &&
					lightCoordinates[j].coordinate.y + (characterHeight / 2) > drawingCoordinateY - (characterHeight / 2))
				{
					emptyPlace = false;
					break;
				}
			}
			if (whatObject == 2)
			{
				for (int k = 0; k < NUMBER_OF_SQUARE_LIGHT; k++)
				{
					if (squareLightCoordinates[k].start > lightCoordinates[i].end || squareLightCoordinates[k].end < lightCoordinates[i].start)
						continue;
					if (squareLightCoordinates[k].coordinate.x - (SQUARE_LIGHT_WIDTH / 2) < drawingCoordinateX + (characterWidth / 2) &&
						squareLightCoordinates[k].coordinate.x + (SQUARE_LIGHT_WIDTH / 2) > drawingCoordinateX - (characterWidth / 2) &&
						squareLightCoordinates[k].coordinate.y - (SQUARE_LIGHT_HEIGHT / 2) < drawingCoordinateY + (characterHeight / 2) &&
						squareLightCoordinates[k].coordinate.y + (SQUARE_LIGHT_HEIGHT / 2) > drawingCoordinateY - (characterHeight / 2))
					{
						emptyPlace = false;
						break;
					}
				}
			}
		}
		lightCoordinates[i].coordinate.x = drawingCoordinateX;
		lightCoordinates[i].coordinate.y = drawingCoordinateY;
		emptyPlace = false;
	}
}

void EnemyShooting(double& worldTime, Enemy enemyCoordinatesForGivenType[], int howManyEnemies, int HowManyBullets)
{
	for (int i = 0; i < howManyEnemies; i++)
	{
		if (enemyCoordinatesForGivenType[i].dead == false && enemyCoordinatesForGivenType[i].nextBulletShot < worldTime)
		{
			for (int j = 0; j < HowManyBullets; j++)
			{
				if (enemyCoordinatesForGivenType[i].bullet[j].isShotLaunched == false)
				{
					enemyCoordinatesForGivenType[i].bullet[j].dir = rand() % HOW_MANY_DIRECTIONS;
					enemyCoordinatesForGivenType[i].bullet[j].isShotLaunched = true;
					enemyCoordinatesForGivenType[i].bullet[j].coordinate.x = enemyCoordinatesForGivenType[i].coordinate.x;
					enemyCoordinatesForGivenType[i].bullet[j].coordinate.y = enemyCoordinatesForGivenType[i].coordinate.y;
					break;
				}

			}
			enemyCoordinatesForGivenType[i].nextBulletShot = (((rand() % 300) + 200) / 100.0) + worldTime;
		}
	}
}

void ScoresToDisplay(SDL_Surface* screen, SDL_Surface* charset, SDL_Event event, SDL_Texture* scrtex, SDL_Renderer* renderer)
{
	char text[128];
	Score* highScores;
	bool displayMenu = true;
	int i = 0;
	int numberOfLines = 0;
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	FILE* file = fopen("./HighScores.txt", "r");

	while (fgets(text, 128, file) != nullptr)
	{
		numberOfLines++;
	}

	fclose(file);

	highScores = new Score[numberOfLines];

	file = fopen("./HighScores.txt", "r");

	while (fgets(text, 128, file) != nullptr)
	{
		bool readPlayerName = true;
		for (int j = 0; j < 128; j++)
		{
			if (text[j] == ';')
			{
				highScores[i].playerName[j] = '\0';
				readPlayerName = false;
			}
			else if (text[j] == '\n')
			{
				break;
			}
			else if (readPlayerName == true)
			{
				highScores[i].playerName[j] = text[j];
			}
			else
			{
				int liczba = text[j] - '0';
				highScores[i].score *= 10;
				highScores[i].score += liczba;
			}
		}
		i++;
	}

	fclose(file);

	// sorting scores
	bool isSorted = false;
	while (isSorted == false)
	{
		isSorted = true;
		for (int i = 0; i < numberOfLines - 1; i++)
		{
			if (highScores[i].score < highScores[i + 1].score)
			{
				isSorted = false;
				Score temp = highScores[i];
				highScores[i] = highScores[i + 1];
				highScores[i + 1] = temp;
			}
		}
	}
	int start = 0;
	while (displayMenu == true) {
		SDL_FillRect(screen, NULL, czarny);
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 56, czerwony, niebieski);
		sprintf(text, "High scores");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 - (16 * 20), text, charset);

		int line = 0;
		int scoresToDisplay = 40;
		for (int i = start; i < start + scoresToDisplay && i < numberOfLines; i++)
		{
			sprintf(text, "%d. %s %d", i + 1, highScores[i].playerName, highScores[i].score);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 - (16 * (18 - line)), text, charset);
			line++;
		}
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_e)
				{
					displayMenu = false;
				}
				else if (event.key.keysym.sym == SDLK_UP && start > 0)
				{
					start--;
				}
				else if (event.key.keysym.sym == SDLK_DOWN && start < numberOfLines - scoresToDisplay)
				{
					start++;
				}
				break;
			case SDL_QUIT:
				displayMenu = 1;
				break;
			};
		};
	};
}

void NewGame(bool& emptyPlace, bool& emptyPlaceForType2, bool& emptyPlaceForType3, bool& emptyPlaceForDynamite, bool& emptyPlaceForMedication, int& drawingCoordinateX, int& drawingCoordinateXForType2, int& drawingCoordinateXForType3,
	int& drawingCoordinateXForDynamite, int& drawingCoordinateXForMedication, int& drawingCoordinateY, int& drawingCoordinateYForType2, int& drawingCoordinateYForType3, int& drawingCoordinateYForDynamite, int& drawingCoordinateYForMedication, Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[], Dynamite dynamiteCoordinates[],
	Medication medicationCoordinates[], int& lives, double& worldTime, Coordinates& playerCoordinates, int& scores, double& delta, double& timeOfTheLastDeath, int& t1, bool& emptyPlaceForSquareLight,
	bool& emptyPlaceForRectangleLight, int& drawingCoordinateXForSquareLight, int& drawingCoordinateXForRectangleLight, int& drawingCoordinateYForSquareLight, int& drawingCoordinateYForRectangleLight, Light squareLightCoordinates[], Light rectangleLightCoordinates[], double& lastShot, double& bonusMultiplier, Bullet playerBullets[])
{
	lives = HOW_MANY_LIVES;
	t1 = SDL_GetTicks();
	worldTime = 0;
	timeOfTheLastDeath = 0.0;
	scores = 0;
	delta = 0;
	lastShot = 0;
	bonusMultiplier = 1;
	playerCoordinates.x = SCREEN_WIDTH / 2;
	playerCoordinates.y = SCREEN_HEIGHT / 2;

	for (int i = 0; i < HOW_MANY_SHOTS; i++)
	{
		playerBullets[i].isShotLaunched = false;
	}

	DrawingCoordinatesForEnemies(1, emptyPlace, drawingCoordinateX, drawingCoordinateY, ENEMY_HEIGHT, ENEMY_WIDTH, enemyCoordinates, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates, medicationCoordinates, NUMBER_OF_ENEMIES);
	DrawingCoordinatesForEnemies(2, emptyPlaceForType2, drawingCoordinateXForType2, drawingCoordinateYForType2, ENEMY_TYPE2_HEIGHT, ENEMY_TYPE2_WIDTH, enemyCoordinatesType2, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates, medicationCoordinates, NUMBER_OF_ENEMIES_TYPE2);
	DrawingCoordinatesForEnemies(3, emptyPlaceForType3, drawingCoordinateXForType3, drawingCoordinateYForType3, ENEMY_TYPE3_HEIGHT, ENEMY_TYPE3_WIDTH, enemyCoordinatesType3, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates, medicationCoordinates, NUMBER_OF_ENEMIES_TYPE3);
	DrawingCoordinatesForDynamites(emptyPlaceForDynamite, drawingCoordinateXForDynamite, drawingCoordinateYForDynamite, EXPLOSION_HEIGHT, EXPLOSION_WIDTH, dynamiteCoordinates, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates, medicationCoordinates, NUMBER_OF_DYNAMITE, worldTime);
	DrawingCoordinatesForMedicines(emptyPlaceForMedication, drawingCoordinateXForMedication, drawingCoordinateYForMedication, MEDICATION_HEIGHT, MEDICATION_WIDTH, medicationCoordinates, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates, medicationCoordinates, NUMBER_OF_MEDICATION);
	DrawingCoordinatesForLight(1, emptyPlaceForSquareLight, drawingCoordinateXForSquareLight, drawingCoordinateYForSquareLight, SQUARE_LIGHT_HEIGHT, SQUARE_LIGHT_WIDTH, squareLightCoordinates, squareLightCoordinates, NUMBER_OF_SQUARE_LIGHT);
	DrawingCoordinatesForLight(2, emptyPlaceForRectangleLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForRectangleLight, RECTANGLE_LIGHT_HEIGHT, RECTANGLE_LIGHT_WIDTH, rectangleLightCoordinates, squareLightCoordinates, NUMBER_OF_RECTANGLE_LIGHT);
}

void ChooseLevel(SDL_Surface* screen, SDL_Surface* menu, SDL_Surface* charset, SDL_Event event, SDL_Texture* scrtex, SDL_Renderer* renderer, int& level, bool& emptyPlace, bool& emptyPlaceForType2, bool& emptyPlaceForType3, bool& emptyPlaceForDynamite, bool& emptyPlaceForMedication, int& drawingCoordinateX, int& drawingCoordinateXForType2, int& drawingCoordinateXForType3,
	int& drawingCoordinateXForDynamite, int& drawingCoordinateXForMedication, int& drawingCoordinateY, int& drawingCoordinateYForType2, int& drawingCoordinateYForType3, int& drawingCoordinateYForDynamite, int& drawingCoordinateYForMedication, Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[], Dynamite dynamiteCoordinates[],
	Medication medicationCoordinates[], int& lives, double& worldTime, Coordinates playerCoordinates, int& scores, double& delta, SDL_Surface* level1, SDL_Surface* level2, SDL_Surface* level3, double& timeOfTheLastDeath, int& t1, bool& emptyPlaceForSquareLight,
	bool& emptyPlaceForRectangleLight, int& drawingCoordinateXForSquareLight, int& drawingCoordinateXForRectangleLight, int& drawingCoordinateYForSquareLight, int& drawingCoordinateYForRectangleLight, Light squareLightCoordinates[], Light rectangleLightCoordinates[], double& lastShot, double& bonusMultiplier, Bullet playerBullets[])
{
	char text[128];
	bool showMenu = true;

	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	NewGame(emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3,
		drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication,
		enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates,
		medicationCoordinates, lives, worldTime, playerCoordinates, scores, delta, timeOfTheLastDeath, t1, emptyPlaceForSquareLight,
		emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBullets);

	while (showMenu == true) {
		SDL_FillRect(screen, NULL, czarny);
		DrawSurface(screen, menu, 500, 1000);

		if (level == 1)
		{
			DrawSurface(screen, level1, 500, 300);
			sprintf(text, "->");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2 + 16, screen->h / 2, text, charset);
		}
		else if (level == 2)
		{
			DrawSurface(screen, level2, 500, 300);
			sprintf(text, "<-");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2 - 16, screen->h / 2, text, charset);
			sprintf(text, "->");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2 + 16, screen->h / 2, text, charset);
		}
		else if (level == 3)
		{
			DrawSurface(screen, level3, 500, 300);
			sprintf(text, "<-");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2 - 16, screen->h / 2, text, charset);
		}

		sprintf(text, "Press enter to start new game on selected level");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 + 16, text, charset);
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_LEFT && level > 1)
				{
					level--;
				}
				else if (event.key.keysym.sym == SDLK_RIGHT && level < 3)
				{
					level++;
				}
				else if (event.key.keysym.sym == SDLK_RETURN)
				{
					return;
				}
				break;
			};
		};
	};
}

void ShowMenu(SDL_Surface* screen, SDL_Surface* menu, SDL_Surface* charset, SDL_Event event, SDL_Texture* scrtex, SDL_Renderer* renderer, int& quit, int& level, bool& emptyPlace, bool& emptyPlaceForType2, bool& emptyPlaceForType3, bool& emptyPlaceForDynamite, bool& emptyPlaceForMedication, int& drawingCoordinateX, int& drawingCoordinateXForType2, int& drawingCoordinateXForType3,
	int& drawingCoordinateXForDynamite, int& drawingCoordinateXForMedication, int& drawingCoordinateY, int& drawingCoordinateYForType2, int& drawingCoordinateYForType3, int& drawingCoordinateYForDynamite, int& drawingCoordinateYForMedication, Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[], Dynamite dynamiteCoordinates[],
	Medication medicationCoordinates[], int& lives, double& worldTime, Coordinates& playerCoordinates, int& scores, double& delta, SDL_Surface* level1, SDL_Surface* level2, SDL_Surface* level3, double& timeOfTheLastDeath, int& t1, bool& emptyPlaceForSquareLight,
	bool& emptyPlaceForRectangleLight, int& drawingCoordinateXForSquareLight, int& drawingCoordinateXForRectangleLight, int& drawingCoordinateYForSquareLight, int& drawingCoordinateYForRectangleLight, Light squareLightCoordinates[], Light rectangleLightCoordinates[], double& lastShot, double& bonusMultiplier, Bullet playerBullets[])
{
	char text[128];
	bool showMenu = true;

	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	while (showMenu == true) {

		SDL_FillRect(screen, NULL, czarny);
		DrawSurface(screen, menu, 500, 1000);
		sprintf(text, "[N]ew game");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 - 24, text, charset);
		sprintf(text, "[H]igh scores");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 - 8, text, charset);
		sprintf(text, "[C]hoose level");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 + 8, text, charset);
		sprintf(text, "[E]xit");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 + 24, text, charset);
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_e)
				{
					showMenu = false;
					quit = 1;
				}
				else if (event.key.keysym.sym == SDLK_n)
				{
					NewGame(emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3,
						drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication,
						enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates,
						medicationCoordinates, lives, worldTime, playerCoordinates, scores, delta, timeOfTheLastDeath, t1, emptyPlaceForSquareLight,
						emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBullets);

					showMenu = false;
				}
				else if (event.key.keysym.sym == SDLK_h)
				{
					ScoresToDisplay(screen, charset, event, scrtex, renderer);
				}
				else if (event.key.keysym.sym == SDLK_c)
				{
					ChooseLevel(screen, menu, charset, event, scrtex, renderer, level, emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3,
						drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication,
						enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates,
						medicationCoordinates, lives, worldTime, playerCoordinates, scores, delta, level1, level2, level3, timeOfTheLastDeath, t1, emptyPlaceForSquareLight,
						emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBullets);
				}
				break;
			case SDL_QUIT:
				quit = 1;
				break;
			};
		};
	};
}

void WinOrLose(SDL_Surface* screen, SDL_Surface* menu, SDL_Surface* cup1, SDL_Surface* cup2, SDL_Surface* cup3, SDL_Surface* charset, SDL_Event event, SDL_Texture* scrtex, SDL_Renderer* renderer, int& quit, int number, int& scores, int& level, bool& emptyPlace, bool& emptyPlaceForType2, bool& emptyPlaceForType3, bool& emptyPlaceForDynamite, bool& emptyPlaceForMedication, int& drawingCoordinateX, int& drawingCoordinateXForType2, int& drawingCoordinateXForType3,
	int& drawingCoordinateXForDynamite, int& drawingCoordinateXForMedication, int& drawingCoordinateY, int& drawingCoordinateYForType2, int& drawingCoordinateYForType3, int& drawingCoordinateYForDynamite, int& drawingCoordinateYForMedication, Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[], Dynamite dynamiteCoordinates[],
	Medication medicationCoordinates[], int& lives, double& worldTime, Coordinates& playerCoordinates, double& delta, SDL_Surface* level1, SDL_Surface* level2, SDL_Surface* level3, double& timeOfTheLastDeath, int& t1, bool& emptyPlaceForSquareLight,
	bool& emptyPlaceForRectangleLight, int& drawingCoordinateXForSquareLight, int& drawingCoordinateXForRectangleLight, int& drawingCoordinateYForSquareLight, int& drawingCoordinateYForRectangleLight, Light squareLightCoordinates[], Light rectangleLightCoordinates[], double& lastShot, double& bonusMultiplier, Bullet playerBullets[])
{
	char text[128];
	bool gameOver = true;
	bool showMenu = true;
	int playerNameIt = 0;
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x80);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x60, 0x60);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	bool saveScore = false;
	char playerName[128];

	for (int i = 0; i < 128; i++)
	{
		playerName[i] = '\0';
	}
	while (gameOver == true) {
		SDL_FillRect(screen, NULL, czarny);

		// what if you won
		if (number == 2)
		{
			sprintf(text, "YOU WON!");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2, text, charset);
			sprintf(text, "--> [M]ENU");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 1.125, text, charset);
			sprintf(text, "--> press [s] to save your score");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 1.75, text, charset);

			// checking what grade you got
			if (scores >= HOW_MANY_SCORES_TO_GRADE1 && scores <= HOW_MANY_SCORES_TO_GRADE2)
			{
				sprintf(text, "YOUR GRADE : 1/3");
				DrawSurface(screen, cup1, 500, 100);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 4, text, charset);
				sprintf(text, "Scores: %d", scores);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 3, text, charset);
			}
			else if (scores >= HOW_MANY_SCORES_TO_GRADE2 && scores < HOW_MANY_SCORES_TO_GRADE3)
			{
				sprintf(text, "YOUR GRADE : 2/3");
				DrawSurface(screen, cup2, 500, 100);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 4, text, charset);
				sprintf(text, "Scores: %d", scores);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 3, text, charset);
			}
			else
			{
				sprintf(text, "YOUR GRADE : 3/3");
				DrawSurface(screen, cup3, 500, 100);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 4, text, charset);
				sprintf(text, "Scores: %d", scores);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 3, text, charset);
			}
			if (level != 3)
			{
				sprintf(text, "Do you want to go to the next level?");
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 1.375, text, charset);
				sprintf(text, "[Y]ES");
				DrawString(screen, screen->w / 2.25 - strlen(text) * 8 / 2, screen->h / 1.25, text, charset);
				sprintf(text, "[N]O");
				DrawString(screen, screen->w / 1.75 - strlen(text) * 8 / 2, screen->h / 1.25, text, charset); \
			}
		}
		// what if you loosed
		else if (number == 1)
		{
			sprintf(text, "GAME OVER!");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2, text, charset);
			sprintf(text, "--> [M]ENU");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 1.25, text, charset);
			sprintf(text, "Scores: %d", scores);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 3, text, charset);
			sprintf(text, "Do you want to try again?");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 1.5, text, charset);
			sprintf(text, "[Y]ES");
			DrawString(screen, screen->w / 2.25 - strlen(text) * 8 / 2, screen->h / 1.375, text, charset);
			sprintf(text, "[N]O");
			DrawString(screen, screen->w / 1.75 - strlen(text) * 8 / 2, screen->h / 1.375, text, charset);
		}
		if (saveScore == true)
		{
			DrawString(screen, screen->w / 2.05 - strlen(text) * 8 / 2, screen->h / 1.65, playerName, charset);
		}
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (saveScore == true)
				{
					if (event.key.keysym.sym == SDLK_BACKSPACE && playerNameIt > 0)
						playerName[--playerNameIt] = '\0';
					else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
					{
						FILE* file = fopen("./highScores.txt", "a");

						sprintf(text, "%s;%d\n", playerName, scores);
						fputs(text, file);
						fclose(file);
						saveScore = false;
					}
					else if (event.key.keysym.sym >= 'a' && event.key.keysym.sym <= 'z')
						playerName[playerNameIt++] = event.key.keysym.sym;
				}
				else
				{
					if (event.key.keysym.sym == SDLK_y && (number != 2 || level == 3))
					{
						gameOver = false;
						NewGame(emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3,
							drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication,
							enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates,
							medicationCoordinates, lives, worldTime, playerCoordinates, scores, delta, timeOfTheLastDeath, t1, emptyPlaceForSquareLight,
							emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBullets);
					}
					else if (event.key.keysym.sym == SDLK_n)
					{
						gameOver = false;
						ShowMenu(screen, menu, charset, event, scrtex, renderer, quit, level, emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3,
							drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates,
							medicationCoordinates, lives, worldTime, playerCoordinates, scores, delta, level1, level2, level3, timeOfTheLastDeath, t1, emptyPlaceForSquareLight,
							emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBullets);
					}
					else if (event.key.keysym.sym == SDLK_s)
					{
						saveScore = true;
					}
					if (event.key.keysym.sym == SDLK_y && number == 2 && level != 3)
					{
						gameOver = false;
						level++;

						NewGame(emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3,
							drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication,
							enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates,
							medicationCoordinates, lives, worldTime, playerCoordinates, scores, delta, timeOfTheLastDeath, t1, emptyPlaceForSquareLight,
							emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBullets);
					}
					if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_e)
					{
						gameOver = false;
					}
					else if (event.key.keysym.sym == SDLK_m)
					{
						ShowMenu(screen, menu, charset, event, scrtex, renderer, quit, level, emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3,
							drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates,
							medicationCoordinates, lives, worldTime, playerCoordinates, scores, delta, level1, level2, level3, timeOfTheLastDeath, t1, emptyPlaceForSquareLight,
							emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBullets);
						gameOver = false;
					}
				}
				break;
			case SDL_QUIT:
				gameOver = false;
				quit = 1;
				break;
			};
		}
	}
}

void DrawSurfaceForLight(int numberOfSpecifiedTypeOfLight, double& worldTime, Light lightCoordinates[], SDL_Surface* screen, SDL_Surface* charset, char text[], SDL_Surface* light)
{
	for (int i = 0; i < numberOfSpecifiedTypeOfLight; i++)
	{
		if (lightCoordinates[i].start < worldTime && lightCoordinates[i].end > worldTime)
		{
			DrawSurface(screen, light, lightCoordinates[i].coordinate.x, lightCoordinates[i].coordinate.y);

			sprintf(text, " %.1lf s", (worldTime - lightCoordinates[i].end) * (-1));
			DrawString(screen, lightCoordinates[i].coordinate.x - strlen(text) * 8 / 2, lightCoordinates[i].coordinate.y, text, charset);
		}
	}
}
void DrawSurfaceForPlayer(bool& isThePlayerAlive, int whatDirection, int& x, SDL_Surface* screen, SDL_Surface* player[], int& up2, int& down2, int& left2, int& right2)
{
	int y = 0;
	int whichPicture = 0;
	int direction2 = 0;
	if (whatDirection == 0)
	{
		direction2 = up2;
		y = 0;
		whichPicture = 1;
	}
	else if (whatDirection == 1)
	{
		direction2 = up2;
		y = 1;
		whichPicture = 2;
	}
	else if (whatDirection == 2)
	{
		direction2 = down2;
		y = 2;
		whichPicture = 0;
	}
	else if (whatDirection == 3)
	{
		direction2 = left2;
		y = 3;
		whichPicture = 6;
	}
	else
	{
		direction2 = right2;
		y = 4;
		whichPicture = 4;
	}
	if (isThePlayerAlive == true)
	{
		if (y == whatDirection)
		{
			if (direction2 % 2 == 0)
			{
				DrawSurface(screen, player[whichPicture], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			}
			else if (direction2 % 2 == 1)
			{
				DrawSurface(screen, player[whichPicture + 1], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			}
		}
	}
	else // what if the player is immortal
	{
		if (x % 2 == 0)
		{
			if (direction2 % 2 == 0)
			{
				DrawSurface(screen, player[whichPicture], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			}
			else if (direction2 % 2 == 1)
			{
				DrawSurface(screen, player[whichPicture + 1], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			}
		}
	}
}

void DirectionOfBulletsForEnemies(double& worldTime, Enemy enemyCoordinates[], int numberOfEnemies, int numberOfEnemyBullets, SDL_Surface* screen, SDL_Surface* enemy, SDL_Surface* enemyBullet, int& level, SDL_Surface* star, SDL_Surface* star2, Enemy enemyCoordinatesForGivenType[])
{
	int x = 1, y = 1, z = 1, a = 1;

	//different coefficients for sine and cosine
	if (level == 1)
	{
		x = 0.25;
		y = 5;
	}
	else if (level == 3)
	{
		a = 0.5;
		z = 2;
	}
	for (int i = 0; i < numberOfEnemies; i++)
	{
		if (level != 3)
		{
			if (enemyCoordinates[i].dead == false)
				DrawSurface(screen, enemy, enemyCoordinates[i].coordinate.x, enemyCoordinates[i].coordinate.y);
		}

		for (int j = 0; j < numberOfEnemyBullets; j++)
		{
			if (enemyCoordinates[i].bullet[j].isShotLaunched == true)
			{
				if (level != 3)
				{
					DrawSurface(screen, enemyBullet, enemyCoordinates[i].bullet[j].coordinate.x, enemyCoordinates[i].bullet[j].coordinate.y);
				}
				else
				{
					if (enemyCoordinatesForGivenType[i].bullet[j].dir == 1)
					{
						DrawSurface(screen, star, enemyCoordinates[i].bullet[j].coordinate.x, enemyCoordinates[i].bullet[j].coordinate.y);
					}
					else
					{
						DrawSurface(screen, star2, enemyCoordinates[i].bullet[j].coordinate.x, enemyCoordinates[i].bullet[j].coordinate.y);
					}
				}
				// the trajectory of enemy bullets
				if (enemyCoordinates[i].bullet[j].dir == 3)
				{
					enemyCoordinates[i].bullet[j].coordinate.x += cos(x * a * worldTime);
					enemyCoordinates[i].bullet[j].coordinate.y += sin(y * z * worldTime);
				}
				else if (enemyCoordinates[i].bullet[j].dir == 2)
				{
					enemyCoordinates[i].bullet[j].coordinate.x -= cos(x * a * worldTime);
					enemyCoordinates[i].bullet[j].coordinate.y -= sin(y * z * worldTime);
				}
				else if (enemyCoordinates[i].bullet[j].dir == 0)
				{
					enemyCoordinates[i].bullet[j].coordinate.y -= sin(y * z * worldTime);
					enemyCoordinates[i].bullet[j].coordinate.x -= cos(x * a * worldTime);
				}
				else if (enemyCoordinates[i].bullet[j].dir == 1)
				{
					enemyCoordinates[i].bullet[j].coordinate.y += sin(y * z * worldTime);
					enemyCoordinates[i].bullet[j].coordinate.x -= cos(x * a * worldTime);
				}
			}
		}
	}
}

void DrawingSurfaceOfEnemyType3(Enemy enemyCoordinatesType3[], SDL_Surface* screen, SDL_Surface* enemyType3[], int& x)
{
	int y = 0;

	for (int i = 0; i < NUMBER_OF_ENEMIES_TYPE3; i++)
	{
		if (enemyCoordinatesType3[i].dir == 3)
		{
			y = 2;
		}
		else if (enemyCoordinatesType3[i].dir == 4)
		{
			y = 0;
		}
		else if (enemyCoordinatesType3[i].dir == 2)
		{
			y = 6;
		}
		else if (enemyCoordinatesType3[i].dir == 1)
		{
			y = 4;
		}
		// changing the picture of enemy3 each second
		if (x % 2 == 0)
		{
			if (enemyCoordinatesType3[i].dead == false)
				DrawSurface(screen, enemyType3[y], enemyCoordinatesType3[i].coordinate.x, enemyCoordinatesType3[i].coordinate.y);
		}
		else
		{
			if (enemyCoordinatesType3[i].dead == false)
				DrawSurface(screen, enemyType3[y + 1], enemyCoordinatesType3[i].coordinate.x, enemyCoordinatesType3[i].coordinate.y);
		}
	}
}

void DrawingSurfaceOfDynamite(double& worldTime, Dynamite dynamiteCoordinates[], SDL_Surface* screen, SDL_Surface* dynamite, SDL_Surface* explosion[], SDL_Surface* explosionBullet)
{
	//animation of expolsion
	for (int i = 0; i < NUMBER_OF_DYNAMITE; i++)
	{
		int animation = (worldTime - dynamiteCoordinates[i].timeOfExplosion) / 0.05;
		if (dynamiteCoordinates[i].timeOfExplosion == 0.0)
		{
			DrawSurface(screen, dynamite, dynamiteCoordinates[i].coordinate.x, dynamiteCoordinates[i].coordinate.y);
		}
		else if (animation >= 0 && animation < 15)
		{
			DrawSurface(screen, explosion[animation], dynamiteCoordinates[i].coordinate.x, dynamiteCoordinates[i].coordinate.y);
			if (animation == 14)
			{
				for (int j = 0; j < 4; j++)
				{
					dynamiteCoordinates[i].bullets[j].isShotLaunched = true;
					dynamiteCoordinates[i].bullets[j].coordinate.x = dynamiteCoordinates[i].coordinate.x;
					dynamiteCoordinates[i].bullets[j].coordinate.y = dynamiteCoordinates[i].coordinate.y;
				}
			}
		}
		else
		{
			for (int j = 0; j < 4; j++)
			{
				if (dynamiteCoordinates[i].bullets[j].isShotLaunched == true)
				{
					DrawSurface(screen, explosionBullet, dynamiteCoordinates[i].bullets[j].coordinate.x, dynamiteCoordinates[i].bullets[j].coordinate.y);
					if (dynamiteCoordinates[i].bullets[j].dir == 3)
					{
						dynamiteCoordinates[i].bullets[j].coordinate.x += PLAYER_BULLET_SPEED;
					}
					else if (dynamiteCoordinates[i].bullets[j].dir == 2)
					{
						dynamiteCoordinates[i].bullets[j].coordinate.x -= PLAYER_BULLET_SPEED;;
					}
					else if (dynamiteCoordinates[i].bullets[j].dir == 0)
					{
						dynamiteCoordinates[i].bullets[j].coordinate.y -= PLAYER_BULLET_SPEED;;
					}
					else if (dynamiteCoordinates[i].bullets[j].dir == 1)
					{
						dynamiteCoordinates[i].bullets[j].coordinate.y += PLAYER_BULLET_SPEED;;
					}
				}
			}
		}
	}
}

void DrawingSurfaceOfMedication(double& worldTime, Medication medicationCoordinates[], SDL_Surface* screen, SDL_Surface* medication, SDL_Surface* playerBullet, Bullet playerBulletCoordinates[])
{
	for (int i = 0; i < NUMBER_OF_MEDICATION; i++)
	{
		if (medicationCoordinates[i].start < worldTime)
			DrawSurface(screen, medication, medicationCoordinates[i].coordinate.x, medicationCoordinates[i].coordinate.y);
	}
	//drawing surface of enemy bullets
	for (int i = 0; i < HOW_MANY_SHOTS; i++)
	{
		if (playerBulletCoordinates[i].isShotLaunched == true)
		{
			DrawSurface(screen, playerBullet, playerBulletCoordinates[i].coordinate.x, playerBulletCoordinates[i].coordinate.y);
			if (playerBulletCoordinates[i].dir == 3)
			{
				playerBulletCoordinates[i].coordinate.x += PLAYER_BULLET_SPEED;;
			}
			else if (playerBulletCoordinates[i].dir == 2)
			{
				playerBulletCoordinates[i].coordinate.x -= PLAYER_BULLET_SPEED;;
			}
			else if (playerBulletCoordinates[i].dir == 0)
			{
				playerBulletCoordinates[i].coordinate.y -= PLAYER_BULLET_SPEED;;
			}
			else if (playerBulletCoordinates[i].dir == 1)
			{
				playerBulletCoordinates[i].coordinate.y += PLAYER_BULLET_SPEED;;
			}
		}
	}
}

void EnemyType3Movement(double& worldTime, Enemy enemyCoordinatesType3[])
{
	for (int i = 0; i < NUMBER_OF_ENEMIES_TYPE3; i++)
	{
		if (worldTime > enemyCoordinatesType3[i].dirChange)
		{
			enemyCoordinatesType3[i].dir = rand() % 4 + 1;
			enemyCoordinatesType3[i].dirChange = (((rand() % 300) + 200) / 100.0) + worldTime;
		}
		if (enemyCoordinatesType3[i].dir == 1)
		{
			enemyCoordinatesType3[i].coordinate.x += ENEMY_TYPE3_SPEED;
		}
		else if (enemyCoordinatesType3[i].dir == 2)
		{
			enemyCoordinatesType3[i].coordinate.x -= ENEMY_TYPE3_SPEED;
		}
		else if (enemyCoordinatesType3[i].dir == 3)
		{
			enemyCoordinatesType3[i].coordinate.y = enemyCoordinatesType3[i].coordinate.y - ENEMY_TYPE3_SPEED;
		}
		else if (enemyCoordinatesType3[i].dir == 4)
		{
			enemyCoordinatesType3[i].coordinate.y = enemyCoordinatesType3[i].coordinate.y + ENEMY_TYPE3_SPEED;
		}
	}
}

void PlaceForDisplayingAdditionalInformation(int& scores, int& lives, double& worldTime, SDL_Surface* screen, SDL_Surface* heart, SDL_Surface* heart2, SDL_Surface* heart3, SDL_Surface* heart4, SDL_Surface* heart5, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, char text[], SDL_Surface* cup1animation[], SDL_Surface* cup2animation[], SDL_Surface* cup3animation[])
{
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	int animation = worldTime / 0.1;
	animation %= 10;

	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 52, czerwony, niebieski);

	if (scores >= HOW_MANY_SCORES_TO_GRADE1 && scores < HOW_MANY_SCORES_TO_GRADE2)
	{
		DrawSurface(screen, cup1animation[animation], SCREEN_WIDTH / 2 - 320, SCREEN_HEIGHT / 2 - 320);
	}
	else if (scores >= HOW_MANY_SCORES_TO_GRADE2 && scores < HOW_MANY_SCORES_TO_GRADE3)
	{
		DrawSurface(screen, cup2animation[animation], SCREEN_WIDTH / 2 - 320, SCREEN_HEIGHT / 2 - 320);
	}
	else if (scores >= HOW_MANY_SCORES_TO_GRADE3)
	{
		DrawSurface(screen, cup3animation[animation], SCREEN_WIDTH / 2 - 320, SCREEN_HEIGHT / 2 - 320);
	}

	sprintf(text, " Time: %.1lf s", TIME_OF_THE_GAME - worldTime);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
	sprintf(text, "lives:");
	DrawString(screen, screen->w / 1.4 - strlen(text) * 8 / 2, 26, text, charset);
	sprintf(text, "Grade:");
	DrawString(screen, screen->w / 11 - strlen(text) * 8 / 2, 26, text, charset);

	// changing lives
	if (lives == 5)
	{
		DrawSurface(screen, heart5, SCREEN_WIDTH / 2 + 350, SCREEN_HEIGHT / 2 - 320);
	}
	else if (lives == 4)
	{
		DrawSurface(screen, heart4, SCREEN_WIDTH / 2 + 350, SCREEN_HEIGHT / 2 - 320);
	}
	else if (lives == 3)
	{
		DrawSurface(screen, heart3, SCREEN_WIDTH / 2 + 350, SCREEN_HEIGHT / 2 - 320);
	}
	else if (lives == 2)
	{
		DrawSurface(screen, heart2, SCREEN_WIDTH / 2 + 350, SCREEN_HEIGHT / 2 - 320);
	}
	else if (lives == 1)
	{
		DrawSurface(screen, heart, SCREEN_WIDTH / 2 + 350, SCREEN_HEIGHT / 2 - 320);
	}

	sprintf(text, "Scores: %d", scores);
	DrawString(screen, screen->w / 3.25 - strlen(text) * 8 / 2, 26, text, charset);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void Up(int& whatDirection, int& direction, Coordinates& playerCoordinates, Bullet playerBulletCoordinates[], Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[],
	Dynamite dynamiteCoordinates[], Medication medicationCoordinates[], Light squareLightCoordinates[], Light rectangleLightCoordinates[], int& up, int& up2)
{
	whatDirection = 1;
	playerCoordinates.y += PLAYER_SPEED;
	direction = 0;
	for (int i = 0; i < NUMBER_OF_ENEMIES; i++)
	{
		enemyCoordinates[i].coordinate.y += PLAYER_SPEED;
		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS; j++)
		{
			if (enemyCoordinates[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinates[i].bullet[j].coordinate.y += PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_ENEMIES_TYPE2; i++)
	{
		enemyCoordinatesType2[i].coordinate.y += PLAYER_SPEED;
		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS_TYPE2; j++)
		{
			if (enemyCoordinatesType2[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinatesType2[i].bullet[j].coordinate.y += PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_ENEMIES_TYPE3; i++)
	{
		enemyCoordinatesType3[i].coordinate.y += PLAYER_SPEED;
		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS_TYPE3; j++)
		{
			if (enemyCoordinatesType3[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinatesType3[i].bullet[j].coordinate.y += PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_DYNAMITE; i++)
	{
		dynamiteCoordinates[i].coordinate.y += PLAYER_SPEED;

		for (int j = 0; j < 4; j++)
		{
			if (dynamiteCoordinates[i].bullets[j].isShotLaunched == true)
				dynamiteCoordinates[i].bullets[j].coordinate.y += PLAYER_SPEED;
		}
	}
	for (int i = 0; i < NUMBER_OF_MEDICATION; i++)
	{
		medicationCoordinates[i].coordinate.y += PLAYER_SPEED;
	}
	for (int i = 0; i < NUMBER_OF_SQUARE_LIGHT; i++)
	{
		squareLightCoordinates[i].coordinate.y += PLAYER_SPEED;
	}
	for (int i = 0; i < NUMBER_OF_RECTANGLE_LIGHT; i++)
	{
		rectangleLightCoordinates[i].coordinate.y += PLAYER_SPEED;
	}
	for (int i = 0; i < HOW_MANY_SHOTS; i++)
	{
		if (playerBulletCoordinates[i].isShotLaunched == true)
		{
			playerBulletCoordinates[i].coordinate.y += PLAYER_SPEED;
		}
	}
	up++;
	if (up == 4)
	{
		up2++;
		up = 0;
	}
}

void Down(int& whatDirection, int& direction, Coordinates& playerCoordinates, Bullet playerBulletCoordinates[], Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[],
	Dynamite dynamiteCoordinates[], Medication medicationCoordinates[], Light squareLightCoordinates[], Light rectangleLightCoordinates[], int& down, int& down2)
{
	whatDirection = 2;
	playerCoordinates.y -= PLAYER_SPEED;
	direction = 1;
	for (int i = 0; i < NUMBER_OF_ENEMIES; i++)
	{
		enemyCoordinates[i].coordinate.y -= PLAYER_SPEED;
		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS; j++)
		{
			if (enemyCoordinates[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinates[i].bullet[j].coordinate.y -= PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_ENEMIES_TYPE2; i++)
	{
		enemyCoordinatesType2[i].coordinate.y -= PLAYER_SPEED;
		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS_TYPE2; j++)
		{
			if (enemyCoordinatesType2[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinatesType2[i].bullet[j].coordinate.y -= PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_ENEMIES_TYPE3; i++)
	{
		enemyCoordinatesType3[i].coordinate.y -= PLAYER_SPEED;
		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS_TYPE3; j++)
		{
			if (enemyCoordinatesType3[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinatesType3[i].bullet[j].coordinate.y -= PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_DYNAMITE; i++)
	{
		dynamiteCoordinates[i].coordinate.y -= PLAYER_SPEED;
		for (int j = 0; j < 4; j++)
		{
			if (dynamiteCoordinates[i].bullets[j].isShotLaunched == true)
				dynamiteCoordinates[i].bullets[j].coordinate.y -= PLAYER_SPEED;
		}
	}
	for (int i = 0; i < NUMBER_OF_MEDICATION; i++)
	{
		medicationCoordinates[i].coordinate.y -= PLAYER_SPEED;
	}
	for (int i = 0; i < NUMBER_OF_SQUARE_LIGHT; i++)
	{
		squareLightCoordinates[i].coordinate.y -= PLAYER_SPEED;
	}
	for (int i = 0; i < NUMBER_OF_RECTANGLE_LIGHT; i++)
	{
		rectangleLightCoordinates[i].coordinate.y -= PLAYER_SPEED;
	}
	for (int i = 0; i < HOW_MANY_SHOTS; i++)
	{
		if (playerBulletCoordinates[i].isShotLaunched == true)
		{
			playerBulletCoordinates[i].coordinate.y -= PLAYER_SPEED;
		}
	}
	down++;
	if (down == 4)
	{
		down2++;
		down = 0;
	}
}

void Left(int& whatDirection, int& direction, Coordinates& playerCoordinates, Bullet playerBulletCoordinates[], Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[],
	Dynamite dynamiteCoordinates[], Medication medicationCoordinates[], Light squareLightCoordinates[], Light rectangleLightCoordinates[], int& left, int& left2)
{
	whatDirection = 3;
	playerCoordinates.x += PLAYER_SPEED;
	direction = 2;
	for (int i = 0; i < NUMBER_OF_ENEMIES; i++)
	{
		enemyCoordinates[i].coordinate.x += PLAYER_SPEED;

		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS; j++)
		{
			if (enemyCoordinates[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinates[i].bullet[j].coordinate.x += PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_ENEMIES_TYPE2; i++)
	{
		enemyCoordinatesType2[i].coordinate.x += PLAYER_SPEED;
		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS_TYPE2; j++)
		{
			if (enemyCoordinatesType2[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinatesType2[i].bullet[j].coordinate.x += PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_ENEMIES_TYPE3; i++)
	{
		enemyCoordinatesType3[i].coordinate.x += PLAYER_SPEED;
		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS_TYPE3; j++)
		{
			if (enemyCoordinatesType3[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinatesType3[i].bullet[j].coordinate.x += PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_DYNAMITE; i++)
	{
		dynamiteCoordinates[i].coordinate.x += PLAYER_SPEED;
		for (int j = 0; j < 4; j++)
		{
			if (dynamiteCoordinates[i].bullets[j].isShotLaunched == true)
				dynamiteCoordinates[i].bullets[j].coordinate.x += PLAYER_SPEED;
		}
	}
	for (int i = 0; i < NUMBER_OF_MEDICATION; i++)
	{
		medicationCoordinates[i].coordinate.x += PLAYER_SPEED;
	}
	for (int i = 0; i < NUMBER_OF_SQUARE_LIGHT; i++)
	{
		squareLightCoordinates[i].coordinate.x += PLAYER_SPEED;
	}
	for (int i = 0; i < NUMBER_OF_RECTANGLE_LIGHT; i++)
	{
		rectangleLightCoordinates[i].coordinate.x += PLAYER_SPEED;
	}
	for (int i = 0; i < HOW_MANY_SHOTS; i++)
	{
		if (playerBulletCoordinates[i].isShotLaunched == true)
		{
			playerBulletCoordinates[i].coordinate.x += PLAYER_SPEED;
		}
	}
	left++;
	if (left == 4)
	{
		left2++;
		left = 0;
	}
}

void Right(int& whatDirection, int& direction, Coordinates& playerCoordinates, Bullet playerBulletCoordinates[], Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[],
	Dynamite dynamiteCoordinates[], Medication medicationCoordinates[], Light squareLightCoordinates[], Light rectangleLightCoordinates[], int& right, int& right2)
{
	whatDirection = 4;
	playerCoordinates.x -= PLAYER_SPEED;
	direction = 3;
	for (int i = 0; i < NUMBER_OF_ENEMIES; i++)
	{
		enemyCoordinates[i].coordinate.x -= PLAYER_SPEED;
		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS; j++)
		{
			if (enemyCoordinates[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinates[i].bullet[j].coordinate.x -= PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_ENEMIES_TYPE2; i++)
	{
		enemyCoordinatesType2[i].coordinate.x -= PLAYER_SPEED;
		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS_TYPE2; j++)
		{
			if (enemyCoordinatesType2[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinatesType2[i].bullet[j].coordinate.x -= PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_ENEMIES_TYPE3; i++)
	{
		enemyCoordinatesType3[i].coordinate.x -= PLAYER_SPEED;
		for (int j = 0; j < NUMBER_OF_ENEMY_BULLETS_TYPE3; j++)
		{
			if (enemyCoordinatesType3[i].bullet[j].isShotLaunched == true)
			{
				enemyCoordinatesType3[i].bullet[j].coordinate.x -= PLAYER_SPEED;
			}
		}
	}
	for (int i = 0; i < NUMBER_OF_DYNAMITE; i++)
	{
		dynamiteCoordinates[i].coordinate.x -= PLAYER_SPEED;
		for (int j = 0; j < 4; j++)
		{
			if (dynamiteCoordinates[i].bullets[j].isShotLaunched == true)
				dynamiteCoordinates[i].bullets[j].coordinate.x -= PLAYER_SPEED;
		}
	}
	for (int i = 0; i < NUMBER_OF_MEDICATION; i++)
	{
		medicationCoordinates[i].coordinate.x -= PLAYER_SPEED;
	}
	for (int i = 0; i < NUMBER_OF_SQUARE_LIGHT; i++)
	{
		squareLightCoordinates[i].coordinate.x -= PLAYER_SPEED;
	}
	for (int i = 0; i < NUMBER_OF_RECTANGLE_LIGHT; i++)
	{
		rectangleLightCoordinates[i].coordinate.x -= PLAYER_SPEED;
	}
	for (int i = 0; i < HOW_MANY_SHOTS; i++)
	{
		if (playerBulletCoordinates[i].isShotLaunched == true)
		{
			playerBulletCoordinates[i].coordinate.x -= PLAYER_SPEED;
		}
	}
	right++;
	if (right == 4)
	{
		right2++;
		right = 0;
	}
}

void PlayerMovement(int& whatDirection, int& direction, Coordinates& playerCoordinates, Bullet playerBulletCoordinates[], Enemy enemyCoordinates[],
	Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[], Dynamite dynamiteCoordinates[], Medication medicationCoordinates[],
	Light squareLightCoordinates[], Light rectangleLightCoordinates[], int& right, int& right2, int& left, int& left2, int& up, int& up2, int& down, int& down2,
	bool& emptyPlace, bool& emptyPlaceForType2, bool& emptyPlaceForType3, bool& emptyPlaceForDynamite, bool& emptyPlaceForMedication,
	int& drawingCoordinateX, int& drawingCoordinateXForType2, int& drawingCoordinateXForType3, int& drawingCoordinateXForDynamite,
	int& drawingCoordinateXForMedication, int& drawingCoordinateY, int& drawingCoordinateYForType2, int& drawingCoordinateYForType3,
	int& drawingCoordinateYForDynamite, int& drawingCoordinateYForMedication, int& lives, double& worldTime, int& scores, double& delta,
	double& timeOfTheLastDeath, int& t1, bool& emptyPlaceForSquareLight, bool& emptyPlaceForRectangleLight, int& drawingCoordinateXForSquareLight,
	int& drawingCoordinateXForRectangleLight, int& drawingCoordinateYForSquareLight, int& drawingCoordinateYForRectangleLight, double& lastShot,
	double& bonusMultiplier, SDL_Event event, int& quit, int& frames)
{
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
			else if (event.key.keysym.sym == SDLK_UP && playerCoordinates.y + PLAYER_HEIGHT / 2 + PLAYER_SPEED <= MAP_HEIGHT / 2 + (SCREEN_HEIGHT / 2))
			{
				Up(whatDirection, direction, playerCoordinates, playerBulletCoordinates, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3,
					dynamiteCoordinates, medicationCoordinates, squareLightCoordinates, rectangleLightCoordinates, up, up2);
			}
			else if (event.key.keysym.sym == SDLK_DOWN && playerCoordinates.y - PLAYER_HEIGHT / 2 - PLAYER_SPEED >= -MAP_HEIGHT / 2 + (SCREEN_HEIGHT / 2))
			{
				Down(whatDirection, direction, playerCoordinates, playerBulletCoordinates, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3,
					dynamiteCoordinates, medicationCoordinates, squareLightCoordinates, rectangleLightCoordinates, down, down2);

			}
			else if (event.key.keysym.sym == SDLK_LEFT && playerCoordinates.x + PLAYER_WIDTH / 2 + PLAYER_SPEED <= MAP_WIDTH / 2 + (SCREEN_WIDTH / 2))
			{
				Left(whatDirection, direction, playerCoordinates, playerBulletCoordinates, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3,
					dynamiteCoordinates, medicationCoordinates, squareLightCoordinates, rectangleLightCoordinates, left, left2);
			}
			else if (event.key.keysym.sym == SDLK_RIGHT && playerCoordinates.x - PLAYER_WIDTH / 2 - PLAYER_SPEED >= -MAP_WIDTH / 2 + (SCREEN_WIDTH / 2))
			{
				Right(whatDirection, direction, playerCoordinates, playerBulletCoordinates, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3,
					dynamiteCoordinates, medicationCoordinates, squareLightCoordinates, rectangleLightCoordinates, right, right2);
			}
			else if (event.key.keysym.sym == SDLK_SPACE && lastShot + 0.2 < worldTime)
			{
				for (int i = 0; i < HOW_MANY_SHOTS; i++)
				{
					if (playerBulletCoordinates[i].isShotLaunched == false)
					{
						playerBulletCoordinates[i].coordinate.x = SCREEN_WIDTH / 2;
						playerBulletCoordinates[i].coordinate.y = SCREEN_HEIGHT / 2;


						playerBulletCoordinates[i].isShotLaunched = true;
						playerBulletCoordinates[i].dir = direction;
						lastShot = worldTime;
						break;
					}
				}
			}
			else if (event.key.keysym.sym == SDLK_n)
			{
				NewGame(emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3,
					drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication,
					enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates,
					medicationCoordinates, lives, worldTime, playerCoordinates, scores, delta, timeOfTheLastDeath, t1, emptyPlaceForSquareLight,
					emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBulletCoordinates);
			}
			break;
		case SDL_QUIT:
			quit = 1;
			break;
		};
	};
	frames++;
}

void IsThePlayerAlive(bool& isThePlayerAlive, double& worldTime, double& timeOfTheLastDeath)
{
	if (timeOfTheLastDeath == 0.0 || timeOfTheLastDeath + IMMORTALITY_TIME < worldTime)
	{
		isThePlayerAlive = true;
	}
	else
	{
		isThePlayerAlive = false;
	}
}

void ConditionsToWin(SDL_Surface* screen, SDL_Surface* menu, SDL_Surface* cup1, SDL_Surface* cup2, SDL_Surface* cup3, SDL_Surface* charset, SDL_Event event, SDL_Texture* scrtex, SDL_Renderer* renderer, int& quit, int number, int& scores, int& level, bool& emptyPlace, bool& emptyPlaceForType2, bool& emptyPlaceForType3, bool& emptyPlaceForDynamite, bool& emptyPlaceForMedication, int& drawingCoordinateX, int& drawingCoordinateXForType2, int& drawingCoordinateXForType3,
	int& drawingCoordinateXForDynamite, int& drawingCoordinateXForMedication, int& drawingCoordinateY, int& drawingCoordinateYForType2, int& drawingCoordinateYForType3, int& drawingCoordinateYForDynamite, int& drawingCoordinateYForMedication, Enemy enemyCoordinates[], Enemy enemyCoordinatesType2[], Enemy enemyCoordinatesType3[], Dynamite dynamiteCoordinates[],
	Medication medicationCoordinates[], int& lives, double& worldTime, Coordinates& playerCoordinates, double& delta, SDL_Surface* level1, SDL_Surface* level2, SDL_Surface* level3, double& timeOfTheLastDeath, int& t1, bool& emptyPlaceForSquareLight,
	bool& emptyPlaceForRectangleLight, int& drawingCoordinateXForSquareLight, int& drawingCoordinateXForRectangleLight, int& drawingCoordinateYForSquareLight, int& drawingCoordinateYForRectangleLight, Light squareLightCoordinates[], Light rectangleLightCoordinates[], double& lastShot, double& bonusMultiplier, Bullet playerBullets[], int& numberOfEnemiesType1, int& numberOfEnemiesType2, int& numberOfEnemiesType3, Bullet playerBulletCoordinates[])
{
	if (lives == 0)
	{
		WinOrLose(screen, menu, cup1, cup2, cup3, charset, event, scrtex, renderer, quit, 1, scores, level, emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3,
			drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates,
			medicationCoordinates, lives, worldTime, playerCoordinates, delta, level1, level2, level3, timeOfTheLastDeath, t1, emptyPlaceForSquareLight,
			emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBulletCoordinates);

	}
	if ((worldTime > TIME_OF_THE_GAME) || numberOfEnemiesType1 == 0 || numberOfEnemiesType2 == 0 || numberOfEnemiesType3 == 0)
	{
		if (scores > HOW_MANY_SCORES_TO_GRADE1)
		{
			WinOrLose(screen, menu, cup1, cup2, cup3, charset, event, scrtex, renderer, quit, 2, scores, level, emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3,
				drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates,
				medicationCoordinates, lives, worldTime, playerCoordinates, delta, level1, level2, level3, timeOfTheLastDeath, t1, emptyPlaceForSquareLight,
				emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBulletCoordinates);
		}
		else
		{
			WinOrLose(screen, menu, cup1, cup2, cup3, charset, event, scrtex, renderer, quit, 1, scores, level, emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3,
				drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates,
				medicationCoordinates, lives, worldTime, playerCoordinates, delta, level1, level2, level3, timeOfTheLastDeath, t1, emptyPlaceForSquareLight,
				emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBulletCoordinates);
		}
	}
}

int LoadingBitmaps(SDL_Surface*& screen, SDL_Surface*& charset, SDL_Surface*& plansza, SDL_Surface* player[], SDL_Surface*& enemy, SDL_Surface*& enemyType2, SDL_Surface* enemyType3[], SDL_Surface*& playerBullet, SDL_Surface*& dynamite, SDL_Surface*& medication, SDL_Surface*& fire, SDL_Surface*& enemyBullet, SDL_Surface*& smoke,
	SDL_Surface*& star, SDL_Surface*& menu, SDL_Surface*& cup1, SDL_Surface*& cup2, SDL_Surface*& cup3, SDL_Surface*& heart, SDL_Surface*& heart2, SDL_Surface*& heart3, SDL_Surface*& heart4, SDL_Surface*& heart5, SDL_Surface* explosion[], SDL_Surface*& level1, SDL_Surface*& level2, SDL_Surface*& level3, SDL_Surface* cup1animation[],
	SDL_Surface* cup2animation[], SDL_Surface* cup3animation[], SDL_Surface*& lightSquare, SDL_Surface*& lightRectangle, SDL_Surface*& explosionBullet,  SDL_Surface *& star2, SDL_Texture*& scrtex, SDL_Window*& window,SDL_Renderer*& renderer, char text[])
{
	plansza = SDL_LoadBMP("./galaxy.bmp");
	enemy = SDL_LoadBMP("./alien.bmp");
	enemyType2 = SDL_LoadBMP("./alien2.bmp");
	playerBullet = SDL_LoadBMP("./ball.bmp");
	dynamite = SDL_LoadBMP("./dynamit.bmp");
	medication = SDL_LoadBMP("./medication.bmp");
	fire = SDL_LoadBMP("./ogien.bmp");
	enemyBullet = SDL_LoadBMP("./ogien.bmp");
	star = SDL_LoadBMP("./star.bmp");
	star2 = SDL_LoadBMP("./star2.bmp");
	smoke = SDL_LoadBMP("./smoke.bmp");
	menu = SDL_LoadBMP("./menu.bmp");
	cup1 = SDL_LoadBMP("./cup1.bmp");
	cup2 = SDL_LoadBMP("./cup2.bmp");
	cup3 = SDL_LoadBMP("./cup3.bmp");
	heart = SDL_LoadBMP("./heart.bmp");
	heart2 = SDL_LoadBMP("./heart2.bmp");
	heart3 = SDL_LoadBMP("./heart3.bmp");
	heart4 = SDL_LoadBMP("./heart4.bmp");
	heart5 = SDL_LoadBMP("./heart5.bmp");
	level1 = SDL_LoadBMP("./1.bmp");
	level2 = SDL_LoadBMP("./2.bmp");
	level3 = SDL_LoadBMP("./3.bmp");
	lightRectangle = SDL_LoadBMP("./light_rectangle.bmp");
	lightSquare = SDL_LoadBMP("./light_square.bmp");
	explosionBullet = SDL_LoadBMP("./fire.bmp");

	for (int i = 0; i < 8; i++)
	{
		sprintf(text, "./player%d.bmp", i);
		player[i] = SDL_LoadBMP(text);
		sprintf(text, "./witch%d.bmp", i);
		enemyType3[i] = SDL_LoadBMP(text);
	}
	for (int i = 0; i < 15; i++)
	{
		sprintf(text, "./explosion%d.bmp", i);
		explosion[i] = SDL_LoadBMP(text);
	}
	for (int i = 0; i < 10; i++)
	{
		sprintf(text, "./cupanim%d.bmp", i);
		cup1animation[i] = SDL_LoadBMP(text);
		sprintf(text, "./animationcup%d.bmp", i);
		cup2animation[i] = SDL_LoadBMP(text);
		sprintf(text, "./cupanimation%d.bmp", i);
		cup3animation[i] = SDL_LoadBMP(text);
	}
	if (plansza == NULL || player == NULL || enemy == NULL || enemyType2 == NULL || enemyType3 == NULL || playerBullet == NULL || dynamite == NULL
		|| medication == NULL || fire == NULL || enemyBullet == NULL || star == NULL || smoke == NULL || menu == NULL || cup1 == NULL || cup2 == NULL
		|| cup3 == NULL || heart == NULL || heart2 == NULL || heart3 == NULL || heart4 == NULL || heart5 == NULL || explosion == NULL || level1 == NULL || level2 == NULL
		|| level3 == NULL || cup1animation == NULL || cup2animation == NULL || cup3animation == NULL || lightSquare == NULL || lightRectangle == NULL || explosionBullet == NULL || star2 == NULL) {
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
}
// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	int t1, t2, quit, frames, rc, drawingCoordinateX = 0, direction = 0, drawingCoordinateY = 0, drawingCoordinateXForType2 = 0, drawingCoordinateYForType2 = 0, drawingCoordinateXForType3 = 0, drawingCoordinateYForType3 = 0, drawingCoordinateXForDynamite = 0, drawingCoordinateYForDynamite = 0, drawingCoordinateXForMedication = 0, scores = 0, whatDirection = 0, left = 0, left2 = 0, right = 0, right2 = 0, up = 0, up2 = 0, down = 0, down2 = 0, level = 1, drawingCoordinateYForMedication = 0, drawingCoordinateXForSquareLight = 0, drawingCoordinateYForSquareLight = 0, drawingCoordinateXForRectangleLight = 0, drawingCoordinateYForRectangleLight = 0, lives = HOW_MANY_LIVES, numberOfEnemiesType1 = NUMBER_OF_ENEMIES, numberOfEnemiesType2 = NUMBER_OF_ENEMIES_TYPE2, numberOfEnemiesType3 = NUMBER_OF_ENEMIES_TYPE3;
	double delta, fpsTimer, fps, distance, worldTime = 0, lastShot = 0.0, bonusMultiplier = 1, timeOfTheLastDeath = 0;
	bool emptyPlace = false, emptyPlaceForType2 = false, emptyPlaceForType3 = false, emptyPlaceForDynamite = false, emptyPlaceForMedication = false, emptyPlaceForRectangleLight = false, emptyPlaceForSquareLight = false, isThePlayerAlive = true;
	char text[128];
	SDL_Event event;
	SDL_Surface* screen, * charset, * plansza, * player[8], * enemy, * enemyType2, * enemyType3[8], * playerBullet, * dynamite, * medication, * fire, * enemyBullet, * smoke, * star, * menu, * cup1, * cup2, * cup3, * heart, * heart2, * heart3, * heart4, * heart5, * explosion[15], * level1, * level2, * level3, * cup1animation[10], * cup2animation[10], * cup3animation[10], * lightSquare, * lightRectangle, * explosionBullet, * star2;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	Coordinates playerCoordinates;
	Enemy enemyCoordinates[NUMBER_OF_ENEMIES], enemyCoordinatesType2[NUMBER_OF_ENEMIES_TYPE2], enemyCoordinatesType3[NUMBER_OF_ENEMIES_TYPE3];
	Dynamite dynamiteCoordinates[NUMBER_OF_DYNAMITE];
	Medication medicationCoordinates[NUMBER_OF_MEDICATION];
	Light squareLightCoordinates[NUMBER_OF_SQUARE_LIGHT], rectangleLightCoordinates[NUMBER_OF_RECTANGLE_LIGHT];
	Bullet playerBulletCoordinates[HOW_MANY_SHOTS], dynamiteBulletCoordinates[NUMBER_OF_DYNAMITE];
	frames = 0, fpsTimer = 0, fps = 0, quit = 0,distance = 10;
	playerCoordinates.x = SCREEN_WIDTH / 2;
	playerCoordinates.y = SCREEN_HEIGHT / 2;
	for (int i = 0; i < HOW_MANY_SHOTS; i++)
	{
		playerBulletCoordinates[i].coordinate.x = SCREEN_WIDTH / 2;
		playerBulletCoordinates[i].coordinate.y = SCREEN_HEIGHT / 2;
	}
	for (int k = 0; k < NUMBER_OF_DYNAMITE; k++)
	{
		dynamiteBulletCoordinates[k].coordinate.x = dynamiteCoordinates[k].coordinate.x;
		dynamiteBulletCoordinates[k].coordinate.y = dynamiteCoordinates[k].coordinate.y;
	}
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	// fullscreen mode
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_ShowCursor(SDL_DISABLE);
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(charset, true, 0x000000);
	LoadingBitmaps(screen, charset, plansza, player, enemy, enemyType2, enemyType3, playerBullet, dynamite, medication, fire, enemyBullet, smoke, star, menu, cup1, cup2, cup3, heart, heart2, heart3, heart4, heart5, explosion, level1, level2, level3, cup1animation, cup2animation, cup3animation, lightSquare, lightRectangle, explosionBullet, star2, scrtex, window, renderer, text);
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	SDL_PollEvent(&event);
	ShowMenu(screen, menu, charset, event, scrtex, renderer, quit, level, emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3, drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates, medicationCoordinates, lives, worldTime, playerCoordinates, scores, delta, level1, level2, level3, timeOfTheLastDeath, t1, emptyPlaceForSquareLight, emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBulletCoordinates);
	t1 = SDL_GetTicks();
	while (!quit) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		worldTime += delta;
		int x;
		x = round(worldTime);
		SDL_FillRect(screen, NULL, czarny);
		DrawSurface(screen, plansza, playerCoordinates.x, playerCoordinates.y);
		DrawSurfaceForLight(NUMBER_OF_SQUARE_LIGHT, worldTime, squareLightCoordinates, screen, charset, text, lightSquare);
		DrawSurfaceForLight(NUMBER_OF_RECTANGLE_LIGHT, worldTime, rectangleLightCoordinates, screen, charset, text, lightRectangle);
		DrawSurfaceForPlayer(isThePlayerAlive, whatDirection, x, screen, player, up2, down2, left2, right2);
		if (level == 1)
		{
			DirectionOfBulletsForEnemies(worldTime, enemyCoordinates, NUMBER_OF_ENEMIES, NUMBER_OF_ENEMY_BULLETS, screen, enemy, enemyBullet, level, star, star2, enemyCoordinates);
			WhatIfYouWalkIntoEnemy(NUMBER_OF_ENEMIES, enemyCoordinates, playerBulletCoordinates, worldTime, isThePlayerAlive, lives, timeOfTheLastDeath, scores, NUMBER_OF_ENEMY_BULLETS, BULLET_ENEMY_HEIGHT, BULLET_ENEMY_WIDTH, playerCoordinates, enemyCoordinatesType3, bonusMultiplier, numberOfEnemiesType1);
		}
		else if (level == 2)
		{
			DirectionOfBulletsForEnemies(worldTime, enemyCoordinatesType2, NUMBER_OF_ENEMIES_TYPE2, NUMBER_OF_ENEMY_BULLETS_TYPE2, screen, enemyType2, smoke, level, star, star2, enemyCoordinates);
			WhatIfYouWalkIntoEnemy(NUMBER_OF_ENEMIES_TYPE2, enemyCoordinatesType2, playerBulletCoordinates, worldTime, isThePlayerAlive, lives, timeOfTheLastDeath, scores, NUMBER_OF_ENEMY_BULLETS_TYPE2, BULLET_ENEMY2_HEIGHT, BULLET_ENEMY2_WIDTH, playerCoordinates, enemyCoordinatesType3, bonusMultiplier, numberOfEnemiesType2);
		}
		else
		{
			DirectionOfBulletsForEnemies(worldTime, enemyCoordinatesType3, NUMBER_OF_ENEMIES_TYPE3, NUMBER_OF_ENEMY_BULLETS_TYPE3, screen, enemyType2, star, level, star, star2, enemyCoordinates);
			DrawingSurfaceOfEnemyType3(enemyCoordinatesType3, screen, enemyType3, x);
			WhatIfYouWalkIntoEnemy(NUMBER_OF_ENEMIES_TYPE3, enemyCoordinatesType3, playerBulletCoordinates, worldTime, isThePlayerAlive, lives, timeOfTheLastDeath, scores, NUMBER_OF_ENEMY_BULLETS_TYPE3, BULLET_ENEMY3_HEIGHT, BULLET_ENEMY3_WIDTH, playerCoordinates, enemyCoordinatesType3, bonusMultiplier, numberOfEnemiesType3);
		}
		DrawingSurfaceOfDynamite(worldTime, dynamiteCoordinates, screen, dynamite, explosion, explosionBullet);
		DrawingSurfaceOfMedication(worldTime, medicationCoordinates, screen, medication, playerBullet, playerBulletCoordinates);
		PlaceForDisplayingAdditionalInformation(scores, lives, worldTime, screen, heart, heart2, heart3, heart4, heart5, charset, scrtex, renderer, text, cup1animation, cup2animation, cup3animation);
		IsThePlayerAlive(isThePlayerAlive,worldTime,timeOfTheLastDeath);
		WhatIfYouWalkIntoLight(NUMBER_OF_RECTANGLE_LIGHT, rectangleLightCoordinates, worldTime, isThePlayerAlive, lives, timeOfTheLastDeath, scores, playerCoordinates, RECTANGLE_LIGHT_WIDTH, RECTANGLE_LIGHT_HEIGHT, bonusMultiplier);
		WhatIfYouWalkIntoLight(NUMBER_OF_SQUARE_LIGHT, squareLightCoordinates, worldTime, isThePlayerAlive, lives, timeOfTheLastDeath, scores, playerCoordinates, SQUARE_LIGHT_WIDTH, SQUARE_LIGHT_HEIGHT, bonusMultiplier);
		WhatIfYouWalkIntoMedication(worldTime, lives, medicationCoordinates);
		WhatIfYouWalkIntoDynamite(isThePlayerAlive, dynamiteCoordinates, worldTime, timeOfTheLastDeath, lives, bonusMultiplier, scores, playerBulletCoordinates, playerCoordinates);
		ConditionsToWin(screen, menu, cup1, cup2, cup3, charset, event, scrtex, renderer, quit, 2, scores, level, emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3, drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates, medicationCoordinates, lives, worldTime, playerCoordinates, delta, level1, level2, level3, timeOfTheLastDeath, t1, emptyPlaceForSquareLight, emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight, drawingCoordinateXForRectangleLight, drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, squareLightCoordinates, rectangleLightCoordinates, lastShot, bonusMultiplier, playerBulletCoordinates, numberOfEnemiesType1, numberOfEnemiesType2, numberOfEnemiesType3,playerBulletCoordinates);
		EnemyShooting(worldTime, enemyCoordinates, NUMBER_OF_ENEMIES, NUMBER_OF_ENEMY_BULLETS);
		EnemyShooting(worldTime, enemyCoordinatesType2, NUMBER_OF_ENEMIES_TYPE2, NUMBER_OF_ENEMY_BULLETS_TYPE2);
		EnemyShooting(worldTime, enemyCoordinatesType3, NUMBER_OF_ENEMIES_TYPE3, NUMBER_OF_ENEMY_BULLETS_TYPE3);
		EnemyType3Movement(worldTime, enemyCoordinatesType3);
		PlayerMovement(whatDirection, direction, playerCoordinates, playerBulletCoordinates, enemyCoordinates, enemyCoordinatesType2, enemyCoordinatesType3, dynamiteCoordinates, medicationCoordinates, squareLightCoordinates, rectangleLightCoordinates, right, right2, left, left2, up, up2, down, down2, emptyPlace, emptyPlaceForType2, emptyPlaceForType3, emptyPlaceForDynamite, emptyPlaceForMedication, drawingCoordinateX, drawingCoordinateXForType2, drawingCoordinateXForType3, drawingCoordinateXForDynamite, drawingCoordinateXForMedication, drawingCoordinateY, drawingCoordinateYForType2, drawingCoordinateYForType3, drawingCoordinateYForDynamite, drawingCoordinateYForMedication, lives, worldTime, scores, delta, timeOfTheLastDeath, t1, emptyPlaceForSquareLight, emptyPlaceForRectangleLight, drawingCoordinateXForSquareLight,drawingCoordinateXForRectangleLight,  drawingCoordinateYForSquareLight, drawingCoordinateYForRectangleLight, lastShot, bonusMultiplier, event, quit, frames);
	};
		// freeing all surfaces
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 0;
	};
