/*
 *	Camera.cpp
 *
 *	Attack of the Orbs
 *	Demo Game
 *	by Jeremy McCarthy

	
	This class encapsulates the camera functionality. A single camera object is
	instantiated in main(), and a pointer to that object is passed to the
	renderer object.
 */

#include "camera.h"

Camera::Camera()
{
	Point3D defaultEye = {0,0,35};
	Vector3D defaultU = {1,0,0};			//right
	Vector3D defaultV = {0,1,0};			//up
	Vector3D defaultN = {0,0,1};			//forward

	eyeLoc = defaultEye;
	U = defaultU;
	V = defaultV;
	N = defaultN;

	modelViewMatrix = new double[16];
}

Camera::~Camera()
{
	delete [] modelViewMatrix;
}

void Camera::slide(double du, double dv, double dn)
{
	//world boundary
	int boundary = 39;
	Point3D tempLoc;

	tempLoc.x = eyeLoc.x + du*U.x + dv*V.x + dn*N.x;
	tempLoc.y = eyeLoc.y + du*U.y + dv*V.y + dn*N.y;
	tempLoc.z = eyeLoc.z + du*U.z + dv*V.z + dn*N.z;

	//check x bounds
	if(fabs(tempLoc.x) < boundary)
		eyeLoc.x = tempLoc.x;
	else if(tempLoc.x >= boundary)
		eyeLoc.x = boundary;
	else if(tempLoc.y <= -boundary)
		eyeLoc.x = -boundary;

	//check y bounds
	if(fabs(tempLoc.y) < boundary)
		eyeLoc.y = tempLoc.y;
	else if(tempLoc.y >= boundary)
		eyeLoc.y = boundary;
	else if(tempLoc.y <= -boundary)
		eyeLoc.y = -boundary;

	//check z bounds
	if(fabs(tempLoc.z) < boundary)
		eyeLoc.z = tempLoc.z;
	else if(tempLoc.z >= boundary)
		eyeLoc.z = boundary;
	else if(tempLoc.y <= -boundary)
		eyeLoc.z = -boundary;
}

void Camera::pitch(double d)
{
	// convert deg to radians
	double theta = d * rads;

	double newVx = V.x * cos(theta) + N.x * sin(theta);
	double newVy = V.y * cos(theta) + N.y * sin(theta);
	double newVz = V.z * cos(theta) + N.z * sin(theta);
	
	double newNx = -V.x * sin(theta) + N.x * cos(theta);
	double newNy = -V.y * sin(theta) + N.y * cos(theta);
	double newNz = -V.z * sin(theta) + N.z * cos(theta);
	
	V.x = newVx;	V.y = newVy;	V.z = newVz;
	N.x = newNx;	N.y = newNy;	N.z = newNz;
}

void Camera::yaw(double d)
{
	// convert deg to radians
	double theta = d * rads;

	double newUx = U.x * cos(theta) + N.x * sin(theta);
	double newUy = U.y * cos(theta) + N.y * sin(theta);
	double newUz = U.z * cos(theta) + N.z * sin(theta);
	
	double newNx = -U.x * sin(theta) + N.x * cos(theta);
	double newNy = -U.y * sin(theta) + N.y * cos(theta);
	double newNz = -U.z * sin(theta) + N.z * cos(theta);
	
	U.x = newUx;	U.y = newUy;	U.z = newUz;
	N.x = newNx;	N.y = newNy;	N.z = newNz;
}

void Camera::roll(double d)
{
	// convert deg to radians
	double theta = d * rads;

	double newUx = U.x * cos(theta) + V.x * sin(theta);
	double newUy = U.y * cos(theta) + V.y * sin(theta);
	double newUz = U.z * cos(theta) + V.z * sin(theta);
	
	double newVx = -U.x * sin(theta) + V.x * cos(theta);
	double newVy = -U.y * sin(theta) + V.y * cos(theta);
	double newVz = -U.z * sin(theta) + V.z * cos(theta);
	
	U.x = newUx;	U.y = newUy;	U.z = newUz;
	V.x = newVx;	V.y = newVy;	V.z = newVz;
}

void Camera::setLocation(double x, double y, double z)
{
	eyeLoc.x = x;
	eyeLoc.y = y;
	eyeLoc.z = z;
}

Vector3D Camera::getU()					{ return U;	}
Vector3D Camera::getV()					{ return V;	}
Vector3D Camera::getN()					{ return N;	}
void	Camera::setLocation(Point3D p)	{ eyeLoc = p; }
Point3D	Camera::getLocation()			{ return eyeLoc; }
double	Camera::getX()					{ return eyeLoc.x; }
double	Camera::getY()					{ return eyeLoc.y; }
double	Camera::getZ()					{ return eyeLoc.z; }

const double* Camera::getModelViewMatrix() const
{
	//create a vector from the world origin to the eye
	Vector3D eyeVec = {eyeLoc.x, eyeLoc.y, eyeLoc.z};
	double* mat = modelViewMatrix;
	
	mat[0]=U.x;		mat[4]=U.y;		mat[8]=U.z;		mat[12]=-eyeVec.dot(U);
	mat[1]=V.x;		mat[5]=V.y;		mat[9]=V.z;		mat[13]=-eyeVec.dot(V);
	mat[2]=N.x;		mat[6]=N.y;		mat[10]=N.z;	mat[14]=-eyeVec.dot(N);
	mat[3]=0;		mat[7]=0;		mat[11]=0;		mat[15]=1.0;

	return mat;
}