#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <irrKlang.h>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib> 
#include <ctime>  

using namespace irrklang;

//models variables
#pragma region 
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_car;
Model_3DS model_pizza;
Model_3DS model_sign_stop;
Model_3DS model_sign_oneway;
Model_3DS model_sign_pedestrian;
Model_3DS model_tank;
Model_3DS model_building;
Model_3DS model_obstacle;
Model_3DS model_bush;
Model_3DS model_flag;
#pragma endregion

//textures
GLuint tex;
GLTexture tex_ground;

//frame Settings
int xCord = 1280;
int yCord = 720;

//sound data
#pragma region
irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();
irrklang::ISound* backgroundSound = nullptr;
irrklang::ISound* carSound = nullptr;
bool hasPlayedWinSound = false;
bool hasPlayedLoseSound = false;
#pragma endregion

//structs
#pragma region
class Vector
{
public:
	GLdouble x, y, z;
	Vector() {}
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
};

struct Collectable {
	Model_3DS model;
	Vector position;
	bool isVisible = true;
};

struct Sign {
	Vector position;
	int type;
};

struct Obstacle {
	Model_3DS model;
	Vector position;
	bool effective = true;
};
#pragma endregion

//Camera Settings
#pragma region 
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)xCord / (GLdouble)yCord;
GLdouble zNear = 0.01;
GLdouble zFar = 1000.0;
float camX = 0.0;
float camY = 7.0; //lw 3ayez aknak btbos odam decrease this val, lw aknek btwaty rasek increase this val
float camZ = 30.0;
Vector Eye(camX, camY, camZ);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);
int cameraZoom = 0;
#pragma endregion

//game data
#pragma region
char title[] = "Car in a Mission";
int score = 0;
int lives = 5;
bool gameOver = false;
bool winGame = false;
enum CurrScene { scene1, scene2 };
CurrScene currentScene = scene1;
enum CameraMode { firstPerson, thirdPerson };
CameraMode cameraMode = thirdPerson;
Vector carPosition(0, 0, 15);
float MIN_X = -10.0f;
float MAX_X = 10.0f;
float moveSpeed = 15.0f;
int timeRemaining = 60.0f;
bool gamePaused = false;
int tankCount = 1;
bool renderLight = true;
#pragma endregion

//fps settings
#pragma region
float camX2 = carPosition.x;
float camY2 = carPosition.y + 1.5;
float camZ2 = carPosition.z - 6.5;

float collectableScaleFactor = 1.0f;
float timeElapsed = 0.0f;
double scaleChange = 0.03;
float flagZPosition = -88;
float houseZPosition = -88;
#pragma endregion

//collectables data
#pragma region
std::vector<Collectable> collectables;
#pragma endregion

//obstacles data
#pragma region
std::vector<Obstacle> obstacles;
#pragma endregion

//signs data
#pragma region
std::vector<Sign> signPositions;
const float ROAD_WIDTH = 20.0f;
const float SPAWN_DISTANCE = 100;
const float REMOVE_DISTANCE = 10.0f;
float spawnTimer = 0.0f;
const float spawnInterval = 1.5f;
#pragma endregion

//methods declarations
#pragma region
void InitLightSource();
void InitMaterial();
void myInit(void);
void myKeyboard(unsigned char button, int x, int y);
void myReshape(int w, int h);
void LoadAssets();
void InitializeGLUT(int argc, char** argv);
void EnableOpenGLFeatures();
void RegisterCallbacks();
void playSound(const char* soundFile, bool loop);
void Render2DText(int score);
void DrawSkyBox();
void DrawModel(Model_3DS& model, const Vector& position, const Vector& scale, const Vector& rotation);
void SpawnCollectables();
void UpdateCollectables(float deltaTime);
void RenderGround();
bool isOverlapping(const Vector& newPosition);
void SpawnSign();
void UpdateSigns(float deltaTime);
void DrawSigns();
void myMouse(int button, int state, int x, int y);
void myMotion(int x, int y);
void DrawModelWithBoundingBox();
void InitializeRoad();
void UpdateRoad(float deltaTime, float playerPositionZ);
void RenderRoad();
void RenderRoadSegment(float zPosition);
void CheckAndHandleCollisions();
bool CheckCollisionWithCollectable(const Vector& carPos, const Collectable& collectable);
void CheckAndHandleCollectableCollisions();
bool CheckCollisionWithObstacle(const Vector& carPos, const Obstacle& obstacle);
void CheckAndHandleObstacleCollisions();
void UpdateObstacles(float deltaTime);
void SpawnObstacle();
void RenderHeadlights();
void mySpecialKeyboard(int key, int x, int y);
void timer(int value);
void DisplaySceneOne();
void DisplaySceneTwo();
void LoadScene2();
void getColorBasedOnTime(float elapsedTime, float& r, float& g, float& b);
#pragma endregion

//road data
#pragma region
std::vector<float> roadSegments;
const int NUM_ROAD_SEGMENTS = 10;
const float ROAD_SEGMENT_LENGTH = 38.0f;
const float SPAWN_DISTANCE2 = 100.0f;
#pragma endregion

//buildings data
#pragma region
std::vector<Vector> buildingPositions;
const float BUILDING_SPACING = 25.0f;
const float BUILDING_OFFSET_X = 20.0f;
const int NUM_BUILDINGS = 8;
const float BUILDING_REUSE_DISTANCE = 100.0f;
#pragma endregion

void InitializeBuildings() {
	buildingPositions.clear();

	for (int i = 0; i < NUM_BUILDINGS; ++i) {
		float zPosition = -SPAWN_DISTANCE2 + (i * BUILDING_SPACING);

		buildingPositions.push_back(Vector(-BUILDING_OFFSET_X, 0, zPosition));

		buildingPositions.push_back(Vector(BUILDING_OFFSET_X, 0, zPosition));
	}
}

void UpdateBuildings(float deltaTime, float carZ) {
	for (auto& position : buildingPositions) {
		position.z += moveSpeed * deltaTime;

		if (position.z > carZ + REMOVE_DISTANCE) {
			position.z -= NUM_BUILDINGS * BUILDING_SPACING;
		}
	}
}

void RenderBuildings() {
	for (const auto& position : buildingPositions) {
		DrawModel(model_building, position, Vector(0.01f, 0.03f, 0.01f), Vector(0, 0, 0));
	}
}

void getColorBasedOnTime(float elapsedTime, float& r, float& g, float& b) {
	r = 0.5f + 0.5f * sin(elapsedTime * 0.5f);
	g = 0.5f + 0.5f * sin(elapsedTime * 0.5f + 3.14f / 2.0f);
	b = 0.5f + 0.5f * sin(elapsedTime * 0.5f + 3.14f);
}

float Lerp(float start, float end, float t) {
	return start + t * (end - start);
}

void UpdateLightColor(float timeElapsed, GLfloat* lightColor) {
	float timeOfDay = fmod(timeElapsed, 30) / 30;

	GLfloat dayColor[3] = { 1.0f, 1.0f, 1.0f };
	GLfloat sunsetColor[3] = { 1.0f, 0.5f, 0.3f };
	GLfloat nightColor[3] = { 0.05f, 0.05f, 0.1f };
	if (timeOfDay < 0.25f) { // Day
		for (int i = 0; i < 3; ++i) {
			lightColor[i] = dayColor[i];
		}
	}
	else if (timeOfDay < 0.5f) { // Day to Sunset
		float t = (timeOfDay - 0.25f) / 0.25f;
		for (int i = 0; i < 3; ++i) {
			lightColor[i] = Lerp(dayColor[i], sunsetColor[i], t);
		}
	}
	else if (timeOfDay < 0.75f) { // Sunset to Night
		float t = (timeOfDay - 0.5f) / 0.25f;
		for (int i = 0; i < 3; ++i) {
			lightColor[i] = Lerp(sunsetColor[i], nightColor[i], t);
		}
	}
	else { // Night
		for (int i = 0; i < 3; ++i) {
			lightColor[i] = nightColor[i];
		}
	}
	lightColor[3] = 1.0f;
}

void DisplaySceneOne()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat lightIntensity[4];
	UpdateLightColor(timeElapsed, lightIntensity);
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	if (gameOver)
	{
		Render2DText(score);
		backgroundSound->stop();
		if (!hasPlayedLoseSound)
		{
			carSound->stop();
			carSound = engine->play2D("sounds/lose.wav", true, false, true);
			hasPlayedLoseSound = true;
		}
		glutSwapBuffers();
		return;
	}

	float r, g, b;
	getColorBasedOnTime(timeElapsed, r, g, b);

	glPushMatrix();
	glTranslatef(0, 0, +moveSpeed);

	// ground
	RenderRoad();

	RenderBuildings();

	// tanks (scaling)
	for (const auto& collectable : collectables) {
		DrawModel(model_tank, collectable.position, Vector(0.03f * collectableScaleFactor, 0.03f * collectableScaleFactor, 0.03f * collectableScaleFactor), Vector(0, 90, 0));
	}

	// barriers (changing colors)
	for (const auto& obstacle : obstacles) {
		glPushMatrix();
		glColor3f(r, g, b);
		DrawModel(model_obstacle, obstacle.position, Vector(1.3f, 1.5f, 1.5f), Vector(0, 180, 0));
		glPopMatrix();
	}

	// reset colors to 0
	glColor3f(1.0f, 1.0f, 1.0f);

	DrawSigns();

	//sky box
	DrawSkyBox();

	// flag at the end of the road


	if (timeRemaining <= 36) {
		flagZPosition += moveSpeed * 0.08;
		Vector flagPosition = Vector(0, -10, flagZPosition);
		DrawModel(model_flag, flagPosition, Vector(1, 1, 1), Vector(0, 0, 0));
	}

	glPopMatrix();

	// car model
	if (renderLight)
		RenderHeadlights();
	DrawModel(model_car, carPosition, Vector(1.3f, 1.5f, 1.5f), Vector(0, 180, 0));
	//DrawModelWithBoundingBox();

	Render2DText(score);

	glutSwapBuffers();
}

void DisplaySceneTwo(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat lightIntensity[4];
	UpdateLightColor(timeElapsed, lightIntensity);
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);


	if (winGame)
	{
		Render2DText(score);
		backgroundSound->stop();
		if (!hasPlayedWinSound)
		{
			carSound->stop();
			carSound = engine->play2D("sounds/cheering(win).wav", true, false, true);
			hasPlayedWinSound = true;
		}
		glutSwapBuffers();
		return;
	}
	else if (gameOver)
	{
		Render2DText(score);
		backgroundSound->stop();
		if (!hasPlayedLoseSound)
		{
			carSound->stop();
			carSound = engine->play2D("sounds/lose.wav", true, false, true);
			hasPlayedLoseSound = true;
		}
		glutSwapBuffers();
		return;
	}

	float r, g, b;
	getColorBasedOnTime(timeElapsed, r, g, b);

	glPushMatrix();
	glTranslatef(0, 0, +moveSpeed);

	RenderRoad();

	// pizzas (scaling)
	for (const auto& collectable : collectables) {
		DrawModel(model_pizza, collectable.position, Vector(0.01f * collectableScaleFactor, 0.01f * collectableScaleFactor, 0.01f * collectableScaleFactor), Vector(0, 90, 0));
	}

	// bushes (changing colors)
	for (const auto& obstacle : obstacles) {
		glPushMatrix();
		glColor3f(r, g, b);
		DrawModel(model_bush, obstacle.position, Vector(1.3f, 1.5f, 1.5f), Vector(0, 180, 0));
		glPopMatrix();
	}

	// resetting colors
	glColor3f(1.0f, 1.0f, 1.0f);


	// trees
	for (const auto& sign : signPositions) {
		DrawModel(model_tree, sign.position, Vector(1.3f, 1.5f, 1.5f), Vector(0, 180, 0));
	}

	DrawSkyBox();

	if (timeRemaining <= 6) {
		houseZPosition += moveSpeed * 0.08;
		Vector housePosition = Vector(0, 0, houseZPosition);
		DrawModel(model_house, housePosition, Vector(1, 1, 1), Vector(90, 0, 0));
	}

	glPopMatrix();

	// car
	if (renderLight)
		RenderHeadlights();
	DrawModel(model_car, carPosition, Vector(1.3f, 1.5f, 1.5f), Vector(0, 180, 0));

	Render2DText(score);

	glutSwapBuffers();
}

void LoadScene2() {
	tex_ground.Load("Textures/ground.bmp");
}

void timer(int value) {
	if (gameOver) return;

	if (currentScene == scene1) {
		glutDisplayFunc(DisplaySceneOne);
	}
	else {
		glutDisplayFunc(DisplaySceneTwo);
	}

	if (!gamePaused) {
		CheckAndHandleCollisions();
		UpdateBuildings(0.1, carPosition.z);
		UpdateRoad(0.1f, carPosition.z);
		UpdateSigns(0.1f);
		UpdateCollectables(0.1f);
		UpdateObstacles(0.1f);
	}

	collectableScaleFactor += scaleChange;
	if (collectableScaleFactor > 1 || collectableScaleFactor < 0.75)
		scaleChange = -scaleChange;

	timeElapsed = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	glutPostRedisplay();

	glutTimerFunc(100, timer, 0);
}

void decrementTime(int value) {
	if (timeRemaining >= 0 && !gameOver && !winGame) {
		timeRemaining--;

		if (timeRemaining == 30) {
			currentScene = scene2;
			tankCount = score / 10;
			LoadScene2();
		}

		if (currentScene == scene2 && timeRemaining % 7 == 0) {
			if (tankCount <= 0) {
				gameOver = true;
			}
			tankCount-=2;
		}

		if (timeRemaining <= 0) {
			winGame = true;
			moveSpeed = 0;
			return;
		}

		glutPostRedisplay();
		glutTimerFunc(1000, decrementTime, 0);
	}
}

void main(int argc, char** argv)
{
	InitializeGLUT(argc, argv);
	RegisterCallbacks();
	glutTimerFunc(100, timer, 0);
	glutTimerFunc(1000, decrementTime, 0);
	myInit();
	InitializeBuildings();
	InitializeRoad();
	LoadAssets();
	EnableOpenGLFeatures();
	if (!backgroundSound)
		backgroundSound = engine->play2D("sounds/bg_sound.wav", true, false, true);
	if (!carSound)
		carSound = engine->play2D("sounds/car moving.wav", true, false, true);
	glutMainLoop();
	engine->drop();
}

void RenderRoadSegment(float zPosition) {
	glPushMatrix();
	glTranslatef(0, 0, zPosition);
	RenderGround();
	glPopMatrix();
}

void InitializeRoad() {
	roadSegments.clear();

	for (int i = 0; i < NUM_ROAD_SEGMENTS; ++i) {
		float zPosition = -SPAWN_DISTANCE2 + (i * ROAD_SEGMENT_LENGTH);
		roadSegments.push_back(zPosition);
	}
}

void UpdateRoad(float deltaTime, float playerPositionZ) {
	for (auto& segment : roadSegments) {
		segment += moveSpeed * deltaTime;
	}

	for (auto& segment : roadSegments) {
		if (segment > playerPositionZ + SPAWN_DISTANCE2) {
			segment = playerPositionZ - SPAWN_DISTANCE2 - ROAD_SEGMENT_LENGTH;
		}
	}
}

void RenderRoad() {
	for (const auto& segmentZ : roadSegments) {
		RenderRoadSegment(segmentZ);
	}
}

bool CheckCollisionWithCollectable(const Vector& carPos, const Collectable& collectable) {
	float distance = sqrt(pow(carPos.x - collectable.position.x, 2) + pow(carPos.z - collectable.position.z - 15, 2));
	return distance < 2.21;
}

bool CheckCollisionWithObstacle(const Vector& carPos, const Obstacle& obstacle) {
	float distance = sqrt(pow(carPos.x - obstacle.position.x, 2) + pow(carPos.z - obstacle.position.z - 20, 2));
	return distance < 2.5;
}

void CheckAndHandleCollectableCollisions() {
	for (auto it = collectables.begin(); it != collectables.end();) {
		if (CheckCollisionWithCollectable(carPosition, *it)) {
			score += 10;
			if (currentScene == scene1)
				playSound("sounds/liquid(car-tank).wav", false);
			else
				playSound("sounds/chewing(car-food).wav", false);
			it->isVisible = false;

			it = collectables.erase(it);
		}
		else {
			++it;
		}
	}
}

void CheckAndHandleObstacleCollisions() {
	for (auto& obstacle : obstacles) {
		if (obstacle.effective && CheckCollisionWithObstacle(carPosition, obstacle)) {
			lives -= 1;
			if (lives <= 0) {
				gameOver = true;
				moveSpeed = 0;
			}
			playSound("sounds/crash(car-obs).wav", false);
			obstacle.effective = false;
			moveSpeed = 0.0f;
			gamePaused = true;
			printf("i collided and i am testing how stupid i am\n");
			return;
		}
	}
}

void CheckAndHandleCollisions() {
	CheckAndHandleCollectableCollisions();
	CheckAndHandleObstacleCollisions();
}

void SpawnSign() {
	Sign newSign;

	if (rand() % 2 == 0) {
		newSign.position = Vector(rand() % 3 + 1.5 * -12, 0, -SPAWN_DISTANCE);
	}
	else {
		newSign.position = Vector(rand() % 3 + 12, 0, -SPAWN_DISTANCE);
	}

	newSign.type = rand() % 3;
	signPositions.push_back(newSign);
}

void UpdateSigns(float deltaTime) {
	spawnTimer += deltaTime;

	if (spawnTimer >= spawnInterval) {
		SpawnSign();
		spawnTimer = 0.0f;
	}

	for (auto it = signPositions.begin(); it != signPositions.end();) {
		it->position.z += moveSpeed * deltaTime;

		if (it->position.z > REMOVE_DISTANCE) {
			it = signPositions.erase(it);
		}
		else {
			++it;
		}
	}
}

void DrawSigns() {
	for (const auto& sign : signPositions) {
		switch (sign.type) {
		case 0:
			DrawModel(model_sign_stop, sign.position, Vector(0.1f, 0.1f, 0.1f), Vector(0, 0, 0));
			break;
		case 1:
			DrawModel(model_sign_oneway, sign.position, Vector(0.1f, 0.1f, 0.1f), Vector(0, 0, 0));
			break;
		case 2:
			DrawModel(model_sign_pedestrian, sign.position, Vector(0.1f, 0.1f, 0.1f), Vector(0, 20, 0));
			break;
		}
	}
}

void SpawnCollectables() {
	Collectable newCollectable;

	newCollectable.position = Vector((rand() % 22) - 10, 0, -SPAWN_DISTANCE);

	if (currentScene == scene1)
		newCollectable.model = model_tank;
	else
		newCollectable.model = model_pizza;

	collectables.push_back(newCollectable);
}

void SpawnObstacle() {
	Obstacle newObstacle;

	newObstacle.position = Vector(rand() % 21 - 10, 0, -SPAWN_DISTANCE);

	while (isOverlapping(newObstacle.position)) {
		newObstacle.position = Vector(rand() % 21 - 10, 0, -SPAWN_DISTANCE);
	}

	if (currentScene == scene1)
		newObstacle.model = model_obstacle;
	else
		newObstacle.model = model_tree;

	obstacles.push_back(newObstacle);
}

void UpdateCollectables(float deltaTime) {
	spawnTimer += deltaTime;

	if (spawnTimer >= spawnInterval) {
		SpawnCollectables();
		spawnTimer = 0.0f;
	}

	for (auto it = collectables.begin(); it != collectables.end();) {
		it->position.z += moveSpeed * deltaTime;

		if (it->position.z > REMOVE_DISTANCE) {
			it = collectables.erase(it);
		}
		else {
			++it;
		}
	}
}

void UpdateObstacles(float deltaTime) {
	static float spawnTimer = 0.0f;
	const float spawnInterval = 3.0f;

	spawnTimer += deltaTime;
	if (spawnTimer >= spawnInterval) {
		SpawnObstacle();
		spawnTimer = 0.0f;
	}

	for (auto it = obstacles.begin(); it != obstacles.end();) {
		it->position.z += moveSpeed * deltaTime;

		if (it->position.z > REMOVE_DISTANCE) {
			it = obstacles.erase(it);
		}
		else {
			++it;
		}
	}
}

void DrawModelWithBoundingBox() {
	DrawModel(model_car, carPosition, Vector(1.3f, 1.5f, 1.5f), Vector(0, 180, 0));

	float carWidth = 2.5f;
	float carHeight = 1.5f;
	float carLength = 6.5f;

	float carLeft = carPosition.x - carWidth / 2;
	float carRight = carPosition.x + carWidth / 2;
	float carBottom = carPosition.y;
	float carTop = carPosition.y + carHeight;
	float carFront = carPosition.z + carLength / 2;
	float carBack = carPosition.z - carLength / 2;

	// Draw the bounding box as a wireframe
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);

	// Bottom rectangle
	glVertex3f(carLeft, carBottom, carFront);
	glVertex3f(carRight, carBottom, carFront);

	glVertex3f(carRight, carBottom, carFront);
	glVertex3f(carRight, carBottom, carBack);

	glVertex3f(carRight, carBottom, carBack);
	glVertex3f(carLeft, carBottom, carBack);

	glVertex3f(carLeft, carBottom, carBack);
	glVertex3f(carLeft, carBottom, carFront);

	// Top rectangle
	glVertex3f(carLeft, carTop, carFront);
	glVertex3f(carRight, carTop, carFront);

	glVertex3f(carRight, carTop, carFront);
	glVertex3f(carRight, carTop, carBack);

	glVertex3f(carRight, carTop, carBack);
	glVertex3f(carLeft, carTop, carBack);

	glVertex3f(carLeft, carTop, carBack);
	glVertex3f(carLeft, carTop, carFront);

	// Vertical lines connecting top and bottom
	glVertex3f(carLeft, carBottom, carFront);
	glVertex3f(carLeft, carTop, carFront);

	glVertex3f(carRight, carBottom, carFront);
	glVertex3f(carRight, carTop, carFront);

	glVertex3f(carRight, carBottom, carBack);
	glVertex3f(carRight, carTop, carBack);

	glVertex3f(carLeft, carBottom, carBack);
	glVertex3f(carLeft, carTop, carBack);

	glEnd();
}

void RenderHeadlights() {
	// Enable lighting
	glEnable(GL_LIGHTING);

	GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightAmbient[] = { 0.02f, 0.02f, 0.02f, 1.0f };
	GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glPushMatrix();
	glTranslatef(carPosition.x - 1.0f, carPosition.y + 1.0f, carPosition.z - 3.5f);

	//left headlight light source
	GLfloat leftLightPosition[] = { carPosition.x - 1.0f, carPosition.y + 1.0f, carPosition.z - 20.0f, 1.0f };  // Homogeneous coordinates
	glLightfv(GL_LIGHT0, GL_POSITION, leftLightPosition);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	const GLfloat CONSTANT_ATTENUATION = 0.8f;
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, CONSTANT_ATTENUATION);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05f);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.02f);

	glEnable(GL_LIGHT0);

	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 1.0f, 0.8f);
	glutSolidSphere(0.1, 10, 10);
	glEnable(GL_LIGHTING);

	glPopMatrix();

	//Right headlight
	glPushMatrix();
	glTranslatef(carPosition.x + 1.0f, carPosition.y + 1.0f, carPosition.z - 3.5f);

	// Set the position and properties for the right headlight light source
	GLfloat rightLightPosition[] = { carPosition.x + 1.0f, carPosition.y + 1.0f, carPosition.z - 20.0f, 1.0f };  // Homogeneous coordinates
	glLightfv(GL_LIGHT1, GL_POSITION, rightLightPosition);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
	const GLfloat CONSTANT_ATTENUATION2 = 0.8f;
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, CONSTANT_ATTENUATION2);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.2f);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.2f);

	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT0, GL_POSITION, leftLightPosition);
	glEnable(GL_LIGHT0);

	// Right headlight
	glLightfv(GL_LIGHT1, GL_POSITION, rightLightPosition);
	glEnable(GL_LIGHT1);

	// Render visual spheres for headlights
	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 1.0f, 0.8f);
	glutSolidSphere(0.1, 10, 10);
	glutSolidSphere(0.1, 10, 10);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void DrawModel(Model_3DS& model, const Vector& position, const Vector& scale, const Vector& rotation)
{
	glPushMatrix();
	glTranslatef(position.x, position.y, position.z);
	glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
	glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
	glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);
	glScalef(scale.x, scale.y, scale.z);
	model.Draw();
	glPopMatrix();
}

bool isOverlapping(const Vector& newPosition) {
	const float minDistance = 5.0f;

	for (const auto& collectable : collectables) {
		float dist = sqrtf(
			pow(collectable.position.x - newPosition.x, 2) +
			pow(collectable.position.y - newPosition.y, 2) +
			pow(collectable.position.z - newPosition.z, 2)
		);

		if (dist < minDistance) {
			return true;
		}
	}
	return false;
}

void myMouse(int button, int state, int x, int y)
{
	/*y = yCord - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}*/
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) { // Left mouse button pressed
		glLoadIdentity();
		if (cameraMode == thirdPerson) {
			cameraMode = firstPerson;
			Eye = Vector(carPosition.x, camY2, camZ2);
			Vector At(carPosition.x, 0, 0);
			Up = Vector(0, 1, 0);
			gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
		}
		else {
			cameraMode = thirdPerson;
			Vector Eye(camX, camY, camZ);
			Vector At(0, 0, 0);
			gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
		}
	}
}

void myKeyboard(unsigned char button, int x, int y)
{
	switch (button)
	{
	case 'w':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'r':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case '2':
		renderLight = !renderLight;
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void mySpecialKeyboard(int key, int x, int y)
{
	float moveAmount = 1.0f;

	if (gamePaused) {
		if (key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT) {
			moveSpeed = 8.0f; //3shan law rege3 b-full speed msh hnl72 nmshy men ganbaha w hykhosh gowa el barrier
			gamePaused = false;
		}
	}
	else {
		moveSpeed = 15.0f;
		switch (key) {
		case GLUT_KEY_LEFT:
			if (carPosition.x - moveAmount >= MIN_X) {
				carPosition.x -= moveAmount;
			}if (cameraMode == firstPerson) {
				Eye.x = carPosition.x;
				At.x = carPosition.x;
				glLoadIdentity();
				gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
			}
			break;
		case GLUT_KEY_RIGHT:
			if (carPosition.x + moveAmount <= MAX_X) {
				carPosition.x += moveAmount;
			}
			if (cameraMode == firstPerson) {
				Eye.x = carPosition.x;
				At.x = carPosition.x;
				glLoadIdentity();
				gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
			}
			break;
		}
	}
	glutPostRedisplay();
}

void myMotion(int x, int y)
{
	y = yCord - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

void RenderGround()
{
	// Set the ground color and enable textures, but do not affect lighting globally
	glColor3f(0.6f, 0.6f, 0.6f);  // Set the color for the ground
	glEnable(GL_TEXTURE_2D);      // Enable texture mapping

	// Bind and render the ground texture
	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);
	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);  // Ground normal (flat surface)

	// Define the texture coordinates and vertices
	glTexCoord2f(0, 0); glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0); glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5); glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5); glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	// Reset the color after ground rendering
	glColor3f(1.0f, 1.0f, 1.0f);  // Reset the color back to white
}

void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	xCord = w;
	yCord = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)xCord / (GLdouble)yCord, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

void LoadAssets()
{
	// Loading Model files
	model_house.Load("Models/house/house.3ds");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_car.Load("Models/car/Car.3ds");
	model_pizza.Load("Models/dominos/Models/Pizza.3ds");
	model_sign_stop.Load("Models/road-signs/neuro_stop_3ds.3ds");
	model_sign_oneway.Load("Models/road-signs/neuro_oneway_3ds.3ds");
	model_sign_pedestrian.Load("Models/road-signs/neuro_pedestrian_3ds.3ds");
	model_tank.Load("Models/tank/gasContain.3ds");
	model_obstacle.Load("Models/barrier/Road Barrier 01/Road Barrier 01a.3ds");
	model_building.Load("Models/building/BUILDINGS.3ds");
	model_bush.Load("Models/bush/Bush1.3ds");
	model_flag.Load("Models/finish/uploads_files_2024783_Flag_v1_001.3ds");
	// Loading texture files
	tex_ground.Load("Textures/road1.bmp");

	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

void InitializeGLUT(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(xCord, yCord);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(title);
}

void RegisterCallbacks()
{
	glutDisplayFunc(DisplaySceneOne);
	glutKeyboardFunc(myKeyboard);
	glutSpecialFunc(mySpecialKeyboard);
	glutMotionFunc(myMotion);
	glutMouseFunc(myMouse);
	glutReshapeFunc(myReshape);
}

void EnableOpenGLFeatures()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
}

void playSound(const char* soundFile, bool loop) {
	if (engine) {
		engine->play2D(soundFile, loop, false, true);
	}
}

void Render2DText(int score) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, 800.0, 0.0, 600.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if (gameOver) {
		if (tankCount <= 0) {
			glColor3f(1.0f, 0.0f, 0.0f);
			glRasterPos2f(xCord / 2.0 - 370, yCord / 2.5);
			char message[70];
			sprintf(message, "Game Over :( no enough fuel to continue , with a score of %d", score);
			for (char* c = message; *c != '\0'; c++) {
				glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
			}

		}
		else {
			glColor3f(1.0f, 0.0f, 0.0f);
			glRasterPos2f(xCord / 2.0 - 370, yCord / 2.5);
			char message[70];
			sprintf(message, "Game Over :( You ran out of lives, with a score of %d", score);
			for (char* c = message; *c != '\0'; c++) {
				glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
			}
		}
	}
	else if (winGame) {
		glColor3f(0.1137f, 0.6118f, 0.0980f);
		glRasterPos2f(xCord / 2.0 - 370, yCord / 2.5);
		char message[50];
		sprintf(message, "Game Win!! with a final score % d", score);
		for (char* c = message; *c != '\0'; c++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
		}
	}
	else {
		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos2f(50.0f, 530.0f);
		std::string scoreText = "Score: " + std::to_string(score);
		for (char c : scoreText) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
		}
		if (currentScene == scene1) {
			glColor3f(1.0f, 0.0f, 0.0f);
			glRasterPos2f(250.0f, 580.0f);
			std::string text = "Collect 10 gas tanks or more to support you in the countryside.";
			for (char c : text) {
				glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
			}
		}
		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos2f(50.0f, 500.0f);
		std::string livesText = "Lives: " + std::to_string(static_cast<int>(lives));
		for (char c : livesText) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
		}

		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos2f(50.0f, 470.0f);
		std::string timeText = "Time Remaining: " + std::to_string(static_cast<int>(timeRemaining));
		for (char c : timeText) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
		}
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}

void DrawSkyBox() {
	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 500, 100, 100);
	gluDeleteQuadric(qobj);
	glPopMatrix();
}