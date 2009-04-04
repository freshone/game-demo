/*
 *	Game.cpp
 *
 *	Attack of the Orbs
 *	Demo Game
 *	by Jeremy McCarthy
 *	
 */

#pragma comment(lib,"fmodvc.lib")
#include <fstream>
#include <time.h>
#include <deque>
#include <windows.h>
#include <gl/glut.h>
#include <fmod/fmod.h>
#include "renderer.h"
#include "camera.h"
using namespace std;

//Global values
Camera* theCamera;
Renderer* theRenderer;
deque<Point3D> theWorld;
int keyDown[256];
int orbsCaptured = 0;
int orbsReleased = 0;
bool gameOver = false;
bool paused = false;
bool splash = false;
bool turbo = false;
FSOUND_STREAM* musicBuffer;
FSOUND_SAMPLE* coinBuffer;
FSOUND_SAMPLE* bubbleBuffer;

//Configuration info
int w = 1280;
int h = 1024;
int bpp = 32;
int refresh = 60;
unsigned short mouseSens = 7;
const char configFile[] = "config.cfg";

void display()
{
	theRenderer->display();
}

void keyboard(unsigned char key, int x, int y)
{
	keyDown[key] = 1;
}

void keyboardUp(unsigned char key, int x, int y)
{
	keyDown[key] = 0;
}

void updateScore()
{
	int score = ((double)orbsCaptured / orbsReleased) * 100;
	theRenderer->setScore(score, orbsCaptured, orbsReleased);
}

void updateListenerOrient()
{
	Vector3D f, t;
	Point3D loc;
	float pos[3];

	loc = theCamera->getLocation();
	pos[0] = loc.x;
	pos[1] = loc.y;
	pos[2] = loc.z;
	f = theCamera->getN();
	t = theCamera->getV();

	FSOUND_3D_Listener_SetAttributes(pos, NULL, f.x, f.y, f.z, t.x, t.y, t.z);
}

void gameLoop(deque<Point3D>* myWorld)
{
	Point3D treasure;
	int bound = theRenderer->getBoundary();
	int sleepTime;

	//seed random number generator
	srand((unsigned)time(NULL));	

	//main gameloop
	while(!gameOver){
		if(turbo){
            sleepTime = 10;			
		}
		else {
			sleepTime = 3000 - (orbsCaptured*900.0 / (orbsReleased+1));
		}

		Sleep(sleepTime);
		if(!paused){
			//subtracting rand from another rand gives us
			//       -bound < position < +bound
			//positions the treasure within the boundaries
			treasure.x = (rand() % bound) - (rand() % bound);
			treasure.y = (rand() % bound) - (rand() % bound);
			treasure.z = (rand() % bound) - (rand() % bound);

			float pos[] = { treasure.x, treasure.y, treasure.z };

			//add treasure to the world
			myWorld->push_back(treasure);

			if(!turbo){
				FSOUND_PlaySoundEx(1, bubbleBuffer, NULL, TRUE);
				FSOUND_3D_SetAttributes(1, &pos[0], NULL);
				FSOUND_SetPaused(1, FALSE);
			}
			orbsReleased++;
			updateScore();
		}
	}
}

void mousePassiveHandler(int x, int y)
{
	if(paused){
		return;
	}

	int dx = x - w/2;
	int dy = y - h/2;
	
	if(dx != 0 || dy != 0){	
		glutWarpPointer(w/2.0,h/2.0);
		
		theCamera->yaw(36 * mouseSens * (double)dx / w);
		theCamera->pitch(36 * mouseSens * (double)dy / h);
	}

	updateListenerOrient();
}

void mouseMotionHandler(int x, int y)
{
	if(paused){
		return;
	}

	int dx = x - w/2;
	int dy = y - h/2;
	
	if(dx != 0 || dy != 0){	
		glutWarpPointer(w/2.0,h/2.0);
		theCamera->roll(-36 * mouseSens * (double)dx / w);
	}

	updateListenerOrient();
}

void detectCollision()
{
	Point3D playerPos = theCamera->getLocation();
	Point3D itemPos;
	double xd, yd, zd;

	for(unsigned int i = 0; i < theWorld.size(); i++){
		itemPos = theWorld[i];
		xd = playerPos.x - itemPos.x;
		yd = playerPos.y - itemPos.y;
		zd = playerPos.z - itemPos.z;

		//distance between two points formula
		if(xd*xd + yd*yd + zd*zd < 1.5*1.5){
			theWorld.erase(theWorld.begin() + i);
			FSOUND_PlaySound(FSOUND_FREE, coinBuffer);
			orbsCaptured++;
			updateScore();
		}
	}
}

void inputLoop()
{
	//Accelleration of player in u,v,n directions
	int uAccel = 0;
	int vAccel = 0;
	int nAccel = 0;

	while(!gameOver){
		if(!paused){

			// N DIRECTION IN CAMERA COORDINATES
			if(keyDown['w'] == 1){	//accellerate forward
				if(nAccel > -100){
					nAccel--;
				}
			}
			else {					//else speed degrades
				if(nAccel < 0){
					nAccel++;
				}
			}
			if(keyDown['s'] == 1){
				if(nAccel < 100){
					nAccel++;
				}
			}
			else {
				if(nAccel > 0){
					nAccel--;
				}
			}

			// U DIRECTION IN CAMERA COORDINATES
			if(keyDown['a'] == 1){
				if(uAccel > -100){
					uAccel--;
				}
			}
			else {
				if(uAccel < 0){
					uAccel++;
				}
			}
			if(keyDown['d'] == 1){
				if(uAccel < 100){
					uAccel++;
				}
			}
			else {
				if(uAccel > 0){
					uAccel--;
				}
			}

			//"Jetpack"
			if(keyDown[' '] == 1){
				if(vAccel < 100){
					vAccel++;
				}
			}
			else {
				if(vAccel > 0){
					vAccel--;
				}
			}

			if(keyDown['t'] == 1){
				turbo = !turbo;
				keyDown['t'] = 0;
			}

			theCamera->slide(uAccel * .005, vAccel * .005, nAccel * .005);
			detectCollision();
			updateListenerOrient();
		}

		if(keyDown[27] == 1){
			if(splash){
				theRenderer->setSplash(false);
				keyDown[27] = 0;
				paused = false;
				splash = false;
				glutWarpPointer(w/2.0,h/2.0);
			}
			else {
				exit(0);
			}
		}

		if(keyDown['p'] == 1 && !splash){
			paused = !paused;
			theRenderer->setPaused(paused);			
			keyDown['p'] = 0;
			glutWarpPointer(w/2.0,h/2.0);
		}

		Sleep(10);
	}
}

void renderLoop()
{
	while(!gameOver){
        glutPostRedisplay();
		FSOUND_Update();
		Sleep(10);
	}
}

void writeConfig()
{
	ofstream ofs(configFile);

	//save settings
	if(ofs){
		ofs << "width " << w << endl;
		ofs << "height " << h << endl;
		ofs << "bpp " << bpp << endl;
		ofs << "refresh " << refresh << endl;

		ofs.close();
	}
}

void readConfig()
{
	char buffer[128];
	ifstream ifs(configFile);

	//read in file and process
	if(ifs){
		while(!ifs.eof()){
			ifs >> buffer;

			if(!strcmp(buffer,"width"))
				ifs >> w;
			else if(!strcmp(buffer,"height"))
				ifs >> h;
			else if(!strcmp(buffer,"bpp"))
				ifs >> bpp;
			else if(!strcmp(buffer,"refresh"))
				ifs >> refresh;
			//else: error input
		}
		ifs.close();
	}
	else {
		//unable to load config file, using default values
		writeConfig();
	}
}

void initSFX()
{
	FSOUND_SetOutput(FSOUND_OUTPUT_DSOUND);
	FSOUND_SetDriver(0);
	FSOUND_SetMixer(FSOUND_MIXER_AUTODETECT);
	FSOUND_Init(44100, 32, 0);
	musicBuffer = FSOUND_Stream_Open("sounds/music.mp3", 0, 0, 0);
	coinBuffer = FSOUND_Sample_Load(2, "sounds/coin.wav", 0, 0, 0);
	bubbleBuffer = FSOUND_Sample_Load(1, "sounds/bubble.wav", 0, 0, 0);
	FSOUND_Stream_Play(0, musicBuffer);
	FSOUND_Stream_SetMode(musicBuffer, FSOUND_LOOP_NORMAL);
	FSOUND_SetVolume(0, 40);
	FSOUND_Sample_SetMinMaxDistance(bubbleBuffer, 10, 10000);
}

void closeSFX()
{
	FSOUND_Stream_Close(musicBuffer);
	FSOUND_Sample_Free(coinBuffer);
	FSOUND_Sample_Free(bubbleBuffer);
	FSOUND_Close();
}

void main(int argc, char** argv)
{
	HANDLE hGameThread, hInputThread, hRenderThread;
	char gameMode[128];

	//init glut, create the window
	readConfig();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	sprintf(gameMode,"%ix%i:%i@%i",w,h,bpp,refresh);
	glutGameModeString(gameMode);
	glutEnterGameMode();
	glutInitWindowSize(w,h);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutWarpPointer(w/2.0,h/2.0);

	//create camera and renderer and link them
	theCamera = new Camera();
	theRenderer = new Renderer(w,h);
	theRenderer->setCamera(theCamera);
	theRenderer->setWorld(&theWorld);

	//register functions
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutMotionFunc(mouseMotionHandler);
	glutPassiveMotionFunc(mousePassiveHandler);

	initSFX();

	//start gameloop/input/renderer on other threads
	hGameThread = CreateThread(NULL, 0, (unsigned long (__stdcall *)(void *))gameLoop, &theWorld, 0, NULL);
	hInputThread = CreateThread(NULL, 0, (unsigned long (__stdcall *)(void *))inputLoop, &keyDown, 0, NULL);
	hRenderThread = CreateThread(NULL, 0, (unsigned long (__stdcall *)(void *))renderLoop, NULL, 0, NULL);

	//start OpenGL cranking
	theRenderer->setSplash(true);
	paused = true;
	splash = true;
	glutMainLoop();
	
	WaitForSingleObject(hInputThread, INFINITE);
	WaitForSingleObject(hGameThread, INFINITE);

	//the game is over
	closeSFX();
	delete theCamera;
	delete theRenderer;
	glutLeaveGameMode();
}