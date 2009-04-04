/*
 *	Renderer.h
 *
 *	Attack of the Orbs
 *	Demo Game
 *	by Jeremy McCarthy
 *
 */

#ifndef RENDERER_H_
#define RENDERER_H_
#include <deque>
#include <gl/glut.h>
#include "camera.h"
using namespace std;

class Renderer
{
public:
			Renderer(int width, int height);
			~Renderer(void);
	void	display(void);
	int		getBoundary(void);
	void	setCamera(Camera* inCamera);
	Camera*	getCamera(void);
	void	setWorld(deque<Point3D>* newWorld);
	void	setScore(int points, int captured, int total);
	void	setSplash(bool toggle);
	bool	getSplash(void);
	void	setPaused(bool toggle);
	bool	getPaused(void);	

private:
	int		getScore(void);
	float	getFPS(void);
	void	initRoom(void);
	void	drawRoom(void);
	void	drawTreasures(void);
	void	drawHUD(void);
	void	printString(void* font, char* str);

	int w, h;
	int frameCount;
	int score;
	int orbsCaptured;
	int orbsReleased;
	bool splash;
	bool paused;
	GLdouble* vertexBuffer;
	GLdouble* normalBuffer;
	GLdouble* textureCoord;
	Camera* camera;
	deque<Point3D>* theWorld;
	GLuint textureID[3];

	static const int WORLDSCALE = 40;
};

#endif