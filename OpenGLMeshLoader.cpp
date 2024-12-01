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
using namespace irrklang;

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
#pragma endregion

//frame Settings
int xCord = 1280;
int yCord = 720;

//game data
#pragma region
char title[] = "Car Game";
int scoreScene1 = 0;
int scoreScene2 = 0;
const int maxScore = 15;
int lives = 5;
bool winGame = false;
enum CurrScene { scene1, scene2 };
CurrScene currentScene = scene1;
enum CameraMode { firstPerson, thirdPerson };
CameraMode cameraMode = thirdPerson;
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
#pragma endregion

int cameraZoom = 0;

//models variables
#pragma region 
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_car;
Model_3DS model_pizza;
Model_3DS model_sign_stop;
Model_3DS model_sign_direction;
Model_3DS model_sign_oneway;
Model_3DS model_sign_pedistrian;
Model_3DS model_tank;
Model_3DS model_building;
Model_3DS model_building2;
#pragma endregion

//textures
GLuint tex;
GLTexture tex_ground;

//sound data
#pragma region
irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();
irrklang::ISound* backgroundSound = nullptr;
bool hasPlayedWinSound = false;
bool hasPlayedLoseSound = false;
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
void Render2DText(int score, bool gameWin, bool gameLose);
void DrawSkyBox();
void DrawModel(Model_3DS& model, const Vector& position, const Vector& scale, const Vector& rotation);
#pragma endregion

void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	// ground
	RenderGround();

	// tree model
	DrawModel(model_tree, Vector(10, 0, 0), Vector(0.7, 0.7, 0.7),Vector(0,0,0));

	// house model
	DrawModel(model_house, Vector(0, 0, 0), Vector(1, 1, 1), Vector(90, 0, 0));

	// pizza
	DrawModel(model_pizza, Vector(10, 10, 10), Vector(0.008, 0.008, 0.008), Vector(0, 0, -90));

	// car model
	DrawModel(model_car, Vector(0, 0, 15), Vector(1.5f, 1.5f, 1.5f), Vector(0, 180, 0));

	// stop sign model
	DrawModel(model_sign_stop, Vector(0, 0, 0), Vector(0.1, 0.1, 0.1), Vector(0, 0, 0));

	// direction sign model
	DrawModel(model_sign_direction, Vector(10, 5, 2), Vector(0.1, 0.1, 0.1), Vector(0, 90, 0));

	// oneway sign model
	DrawModel(model_sign_oneway, Vector(10, 0, 10), Vector(0.1, 0.1, 0.1), Vector(0, 0, 0));

	// pedistrian sign model
	DrawModel(model_sign_pedistrian, Vector(12, 0, 7), Vector(0.1, 0.1, 0.1), Vector(0, 45, 0));

	// tank model
	DrawModel(model_tank, Vector(10, 0, 10), Vector(0.07, 0.07, 0.07), Vector(0, 0, 0));
	
	// building model, no errors but it is not visible for some unknown reason
	DrawModel(model_building, Vector(0, 0, 10), Vector(100, 100, 100), Vector(0, 0, 0));

	DrawModel(model_building2, Vector(0, 0, 10), Vector(0.1, 0.1, 0.1), Vector(0, 0, 0));

	//sky box
	DrawSkyBox();

	if (currentScene == scene1) {
		Render2DText(scoreScene1, false, false);
	}
	else if (currentScene == scene2) {
		Render2DText(scoreScene2, false, false);
	}
	glutSwapBuffers();
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

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

void myMouse(int button, int state, int x, int y)
{
	y = yCord - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
}

void main(int argc, char** argv)
{
	InitializeGLUT(argc, argv);
	RegisterCallbacks();
	myInit();
	LoadAssets();
	EnableOpenGLFeatures();
	if (!backgroundSound)
		backgroundSound = engine->play2D("bg_sound.wav", true, false, true);
	glutMainLoop();
	engine->drop();
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
	case 27:
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
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
	RenderGround();

	// Loading Model files
	model_house.Load("Models/house/house.3ds");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_car.Load("Models/car/Car.3ds");
	model_pizza.Load("Models/dominos/Models/Pizza.3ds");
	model_sign_stop.Load("Models/road-signs/neuro_stop_3ds.3ds");
	model_sign_direction.Load("Models/road-signs/neuro_direction_3ds.3ds");
	model_sign_oneway.Load("Models/road-signs/neuro_oneway_3ds.3ds");
	model_sign_pedistrian.Load("Models/road-signs/neuro_pedestrian_3ds.3ds");
	model_tank.Load("Models/tank/gasContain.3ds");
	model_building.Load("Models/building/Building_italian.3ds");
	model_building2.Load("Models/building2/Building.3DS");


	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

void InitializeGLUT(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(xCord, yCord);
	glutInitWindowPosition(100, 80);
	glutCreateWindow(title);
}

void RegisterCallbacks()
{
	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKeyboard);
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

void Render2DText(int score, bool gameWin, bool gameLose) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, 800.0, 0.0, 600.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if (gameLose) {
		glColor3f(1.0f, 0.0f, 0.0f);
		glRasterPos2f(xCord / 2.0 - 200, yCord / 2.0);
		char message[50];
		if (scoreScene1 < maxScore)
			sprintf(message, "Game Lose, You ran out of gas, with a score of %d out of %d", scoreScene1+scoreScene2,2* maxScore);
		else if (scoreScene2 < maxScore)
			sprintf(message, "Game Lose, Your player is hungry :( , final score %d out of %d", scoreScene1 + scoreScene2,2* maxScore);
		for (char* c = message; *c != '\0'; c++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
		}
	}
	else if (gameWin) {
		glColor3f(0.1137f, 0.6118f, 0.0980f);
		glRasterPos2f(xCord / 2.0 - 200, yCord / 2.0);
		char message[50];
		sprintf(message, "Game Win, with a final score % d out of % d", scoreScene1 + scoreScene2,2* maxScore);
		for (char* c = message; *c != '\0'; c++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
		}
	}
	else {
		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos2f(50.0f, 530.0f);
		std::string scoreText = "Score: " + std::to_string(score)+ " / "+ std::to_string(maxScore);
		for (char c : scoreText) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
		}

		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos2f(50.0f, 500.0f);
		std::string timeText = "Lives: " + std::to_string(static_cast<int>(lives));
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

void DrawModel(Model_3DS& model, const Vector& position, const Vector& scale, const Vector& rotation)
{
	glPushMatrix();
	glTranslatef(position.x, position.y, position.z); // Translate first
	glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);         // Rotate second
	glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
	glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);
	glScalef(scale.x, scale.y, scale.z);             // Scale last
	model.Draw();
	glPopMatrix();
}