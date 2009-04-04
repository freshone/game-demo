/*
 *	Renderer.cpp
 *
 *	Attack of the Orbs
 *	Demo Game
 *	by Jeremy McCarthy
 
	
	This class encapsulates the functionality of the renderer. A single
	renderer object is instantiated in main(), and a pointer to the camera
	object is passed to the renderer.
 */

#pragma comment(lib, "GLaux.lib")
#include <time.h>
#include <gl/glaux.h>
#include "renderer.h"

//used to turn features on/off
bool textured = true;		//toggle texturing
bool materials = true;		//toggle materials
bool lighting = true;		//toggle shading

//Light definitions
GLfloat globalAmbient[] =	{ 0.6, 0.6, 0.6, 0.6 };

GLfloat light1Position[] =	{ 0.0, 0.0, 0.0, 1.0 };
GLfloat light1Diffuse[] =	{ 0.0, 0.0, 0.8, 1.0 };
GLfloat light1Specular[] =	{ 0.0, 0.0, 0.8, 1.0 };
GLfloat light1Ambient[] =	{ 0.0, 0.0, 0.3, 1.0 };

//Material definitions
GLfloat defAmbient[] =		{ 0.8, 0.8, 0.8, 1.0 };
GLfloat defDiffuse[] =		{ 0.8, 0.8, 0.8, 1.0 };
GLfloat defSpecular[] =		{ 0.0, 0.0, 0.0, 1.0 };
GLfloat defEmission[] =		{ 0.0, 0.0, 0.0, 1.0 };
GLfloat defShininess =		0.0;

GLfloat brickAmbient[] =	{ 0.0, 0.0, 0.0, 1.0 };
GLfloat brickDiffuse[] =	{ 0.0, 0.0, 0.0, 1.0 };
GLfloat brickSpecular[] =	{ 0.0, 0.0, 0.0, 1.0 };
GLfloat brickEmission[] =	{ 0.4, 0.1, 0.1, 0.5 };
GLfloat brickShininess =	0.0;

GLfloat ballAmbient[] =		{ 0.2, 0.2, 0.2, 0.7 };
GLfloat ballDiffuse[] =		{ 0.8, 0.8, 0.8, 0.7 };
GLfloat ballSpecular[] =	{ 0.8, 0.8, 0.8, 0.7 };
GLfloat ballEmission[] =	{ 0.3, 0.3, 0.3, 0.7 };
GLfloat ballShininess =		33.0;

GLfloat skyEmission[] =		{ 1.0, 1.0, 1.0, 0.5 };


//Member Functions
Renderer::Renderer(int width, int height)
			: frameCount(0),
			  splash(false),
			  paused(false)
{
	AUX_RGBImageRec* textureImage[4];
	
	score = 0;
	orbsCaptured = 0;
	orbsReleased = 0;
	w = width;
	h = height;
	initRoom();

	glEnable(GL_DEPTH_TEST | GL_POLYGON_SMOOTH);
	glEnable(GL_CULL_FACE);						//enable culling
	glCullFace(GL_BACK);						//cull back facing polys
	glShadeModel(GL_SMOOTH);					//enable smooth shading
	glClearColor(0,0,0,1);						//black background
	glClearDepth(1);							//depth buffer setup
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(75, (GLfloat)w/(GLfloat)h, 0.1, 200);

	glMatrixMode(GL_MODELVIEW);

	//init textures
	if(textured){
		glGenTextures(4, &textureID[0]);
		
		//load textures into main memory
		textureImage[0] = auxDIBImageLoad("tex/bricks.bmp");
		textureImage[1] = auxDIBImageLoad("tex/sky.bmp");
		textureImage[2] = auxDIBImageLoad("tex/grass.bmp");
		textureImage[3] = auxDIBImageLoad("tex/splash.bmp");

		for(int i = 0; i < 4; i++){
			if(textureImage[i]){
				if(textureImage[i]->data){
					glBindTexture(GL_TEXTURE_2D, textureID[i]);
					               
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

					//load into texture memory
					glTexImage2D(GL_TEXTURE_2D,
									0,						//mipmap level
									GL_RGB,
									textureImage[i]->sizeX,
									textureImage[i]->sizeY,
									0,						//border pixels
									GL_RGB,					//pixel format
									GL_UNSIGNED_BYTE,
									textureImage[i]->data);
					 
					//free main memory
					free(textureImage[i]->data);
				}
				free(textureImage[i]);
			}
		}
	}

	//init lighting
	if(lighting){
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
		
		glLightfv(GL_LIGHT1, GL_POSITION, light1Position);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Diffuse);
		glLightfv(GL_LIGHT1, GL_SPECULAR, light1Specular);
		glLightfv(GL_LIGHT1, GL_AMBIENT, light1Ambient);
		
		glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 9.0);
		glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.6);
		glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.05);
		glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0);
		glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.0);
		
		glEnable(GL_LIGHT1);
	}
}

Renderer::~Renderer(void)
{
	delete [] vertexBuffer;
	delete [] textureCoord;
	delete [] normalBuffer;
}

void Renderer::setCamera(Camera* inCamera)
{
	camera = inCamera;
}

Camera*	Renderer::getCamera(void)
{
	return camera;
}

int	Renderer::getBoundary(void)
{
	return WORLDSCALE - 1;
}

void Renderer::setWorld(deque<Point3D>* newWorld)
{
	theWorld = newWorld;
}

void Renderer::display(void)
{
	//clear window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	//draw splash screen
	if(splash){
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, textureID[3]);
		glPushMatrix();

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, -2);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1, -1, -2);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1, -1, -2);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1, 1, -2);	
		glEnd();
		
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
	}
	else {
		if(lighting){
			glEnable(GL_LIGHTING);
		}
		
		glPushMatrix();									//Push -- camera
		glLoadMatrixd(camera->getModelViewMatrix());
		
		glPushMatrix();									//Push -- draw world
		drawRoom();
		glPopMatrix();									//Pop  -- draw world

		drawTreasures();
		glPopMatrix();									//Pop  -- camera

		if(lighting){
			glDisable(GL_LIGHTING);
		}

		drawHUD();
		frameCount++;
	}
	glutSwapBuffers();
}

void Renderer::initRoom()
{
	GLdouble vB[] = {
		-40, -40, -40,		 40, -40, -40,		 40,  40, -40,		-40,  40, -40,		//back face
		-40, -40,  40,		-40,  40,  40,		 40,  40,  40,		 40, -40,  40,		//front face
		-40, -40, -40,		-40, -40,  40,		 40, -40,  40,		 40, -40, -40,		//bottom face
		-40,  40, -40,		 40,  40, -40,		 40,  40,  40,		-40,  40,  40,		//top face
		-40, -40, -40,		-40,  40, -40,		-40,  40,  40,		-40, -40,  40,		//left face
		 40, -40, -40,		 40, -40,  40,		 40,  40,  40,		 40,  40, -40};		//right face

	GLdouble tC[] = {
		0.0,0.0,	4.0,0.0,	4.0,4.0,	0.0,4.0,	//back
		4.0,0.0,	4.0,4.0,	0.0,4.0,	0.0,0.0,	//front
		20.0,20.0,	10.0,20.0,	10.0,0.0,	20.0,0.0,	//bottom
		0.0,1.0,	0.0,0.0,	0.5,0.0,	0.5,1.0,	//top 24-31
		4.0,0.0,	4.0,4.0,	0.0,4.0,	0.0,0.0,	//left
		0.0,0.0,	4.0,0.0,	4.0,4.0,	0.0,4.0};	//right

	GLdouble nB[] = {
		 0, 0, 1,	 0, 0, 1,	 0, 0, 1,	 0, 0, 1,	//back normal
		 0, 0,-1,	 0, 0,-1,	 0, 0,-1,	 0, 0,-1,	//front normal
		 0, 1, 0,	 0, 1, 0,	 0, 1, 0,	 0, 1, 0,	//bottom normal
		 0,-1, 0,	 0,-1, 0,	 0,-1, 0,	 0,-1, 0,	//top normal
		 1, 0, 0,	 1, 0, 0,	 1, 0, 0,	 1, 0, 0,	//left normal
		-1, 0, 0,	-1, 0, 0,	-1, 0, 0,	-1, 0, 0};	//right normal


	vertexBuffer = new GLdouble[sizeof(vB)];
	textureCoord = new GLdouble[sizeof(tC)];
	normalBuffer = new GLdouble[sizeof(nB)];

	for(int i = 0; i < sizeof(vB); i++){
		vertexBuffer[i] = vB[i];
		normalBuffer[i] = nB[i];
	}

	for(int i = 0; i < sizeof(tC); i++){
		textureCoord[i] = tC[i];
	}
}

void Renderer::drawRoom()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, vertexBuffer);

	if(lighting){
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_DOUBLE, 0, normalBuffer);
	}

	if(materials){
		glMaterialfv(GL_FRONT,GL_AMBIENT,brickAmbient);
		glMaterialfv(GL_FRONT,GL_DIFFUSE,brickDiffuse);
		glMaterialfv(GL_FRONT,GL_SPECULAR,brickSpecular);
		glMaterialfv(GL_FRONT,GL_EMISSION,brickEmission);
		glMaterialf (GL_FRONT,GL_SHININESS,brickShininess);
	}

	if(textured){
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		//animate the sky texture
		double x1,x2;
		
		x1 = textureCoord[24];
		x2 = textureCoord[28];
		
		x1 += 0.0002;
		x2 += 0.0002;
		
		textureCoord[24] = textureCoord[26] = x1;
		textureCoord[28] = textureCoord[30] = x2;
		
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_DOUBLE, 0, textureCoord);
		
		glEnable(GL_TEXTURE_2D);
		
		glBindTexture(GL_TEXTURE_2D, textureID[0]);
		glDrawArrays(GL_QUADS,0,8);
		glDrawArrays(GL_QUADS,16,8);
		glBindTexture(GL_TEXTURE_2D, textureID[1]);
		glMaterialfv(GL_FRONT, GL_EMISSION, skyEmission);
		glDrawArrays(GL_QUADS,12,4);
		glBindTexture(GL_TEXTURE_2D, textureID[2]);
		glDrawArrays(GL_QUADS,8,4);
		glMaterialfv(GL_FRONT, GL_EMISSION, defEmission);
		
		glDisable(GL_TEXTURE_2D);
	}
	else {
		glColor3d(1,1,0);
		glDrawArrays(GL_QUADS,0,4);
		glColor3d(1,0,0);
		glDrawArrays(GL_QUADS,4,4);
		glColor3d(1,.5,0);
		glDrawArrays(GL_QUADS,8,4);
		glColor3d(0,1,0);
		glDrawArrays(GL_QUADS,12,4);
		glColor3d(0,0,1);
		glDrawArrays(GL_QUADS,16,4);
		glColor3d(1,0,1);
		glDrawArrays(GL_QUADS,20,4);
	}
}

void Renderer::drawTreasures()
{
	Point3D tPoint;

	if(materials){
		glMaterialfv(GL_FRONT,GL_AMBIENT,ballAmbient);
		glMaterialfv(GL_FRONT,GL_DIFFUSE,ballDiffuse);
		glMaterialfv(GL_FRONT,GL_SPECULAR,ballSpecular);
		glMaterialf (GL_FRONT,GL_SHININESS,ballShininess);
	}

	for(unsigned int i = 0; i < (*theWorld).size(); i++){
		tPoint = (*theWorld)[i];

		glPushMatrix();
		glTranslated(tPoint.x,
						tPoint.y,
						tPoint.z);

		//sphere treasure
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glColor4d(1,1,0,.5);
		glutSolidSphere(1,25,25);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		if(!lighting){
			glRotated(90,-1,0,0);
			glPolygonMode(GL_FRONT,GL_LINE);
			glColor3d(.5,.5,0);
			glutSolidSphere(1,25,25);
			glPolygonMode(GL_FRONT,GL_FILL);
		}
		glPopMatrix();
	}
}

void Renderer::drawHUD()
{
	char outputBuffer[30];
	
	glPushMatrix();

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	//black background
	glColor4f(0,0,0,.7);
	
	//draw background box
	glBegin(GL_QUADS);
		glVertex3f(-1.9, 1.47, -2);
		glVertex3f(-1.9, 1.185, -2);
		glVertex3f(-1.2, 1.185, -2);
		glVertex3f(-1.2, 1.47, -2);

	//draw paused indicator
	if(paused){
		glVertex3f(-.2, .1, -2);
		glVertex3f(-.2, -.1, -2);
		glVertex3f(.2, -.1, -2);
		glVertex3f(.2, .1, -2);
	}
	glEnd();

	//white text
	glColor4f(1,1,1,1);

	sprintf(outputBuffer, "Captured:    %i", orbsCaptured);
	glRasterPos3f(-1.85,1.4,-2);
	printString(GLUT_BITMAP_9_BY_15,outputBuffer);

	sprintf(outputBuffer, "Remaining:   %i", orbsReleased - orbsCaptured);
	glRasterPos3f(-1.85,1.35,-2);
	printString(GLUT_BITMAP_9_BY_15,outputBuffer);

	sprintf(outputBuffer, "Score:       %i", score);
	glRasterPos3f(-1.85,1.30,-2);
	printString(GLUT_BITMAP_9_BY_15,outputBuffer);

	sprintf(outputBuffer, "FPS:         %#.2f", getFPS());
	glRasterPos3f(-1.85,1.22,-2);
	printString(GLUT_BITMAP_9_BY_15,outputBuffer);

	if(paused){
        sprintf(outputBuffer, "PAUSED");
		glRasterPos3f(-.085,-.01,-2);
		printString(GLUT_BITMAP_9_BY_15,outputBuffer);
	}
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	glPopMatrix();
}

void Renderer::setScore(int points, int captured, int total)
{
	score = points;
	orbsCaptured = captured;
	orbsReleased = total;
}

int Renderer::getScore()
{
	return 0;
}

void Renderer::setSplash(bool toggle)
{
	splash = toggle;
}

bool Renderer::getSplash()
{
	return splash;
}

void Renderer::setPaused(bool toggle)
{
	paused = toggle;
}

bool Renderer::getPaused()
{
	return paused;
}

float Renderer::getFPS()
{
	static float fps = 0;
	static clock_t last = 0;
	clock_t now;
	float delta;
	
	if(frameCount > 0){
		now  = clock();
		delta= (now - last);

		if(delta > 1000){
            fps = (frameCount*1000) / delta;
			frameCount = 0;
			last = now;
		}
	}

	return fps;
}

void Renderer::printString(void* font, char* str)
{
	for(unsigned int i = 0; i < strlen(str); i++)
		glutBitmapCharacter(font,str[i]);
}