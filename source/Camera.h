/*
 *	Camera.h
 *
 *	Attack of the Orbs
 *	Demo Game
 *	by Jeremy McCarthy
 *
 */

#ifndef CAMERA_H_
#define CAMERA_H_
#include <math.h>

static const double rads = 0.0174532925;

//Simple datatype to store 3D coords
struct Point3D
{
	double x,y,z;
};

//Simple datatype to store/manipulate vectors
struct Vector3D
{
	double x,y,z;
	double length;

	inline double dot( const Vector3D& A ) const
	{
		return A.x*x + A.y*y + A.z*z;
	}

	void normalize(void)
	{
		length = sqrt(x*x+y*y+z*z);
		x = x/length;
		y = y/length;
		z = z/length;
		length = 1;
	}
};

class Camera
{
public:
			Camera(void);
			~Camera(void);
	void	slide(double du, double dv, double dn);
	void	roll(double d);
	void	yaw(double d);
	void	pitch(double d);
	
	void	setLocation(double x, double y, double z);
	void	setLocation(Point3D p);
	Point3D	getLocation(void);
	Vector3D getU(void);
	Vector3D getV(void);
	Vector3D getN(void);
	double	getX(void);
	double	getY(void);
	double	getZ(void);
	const	double*	getModelViewMatrix(void) const;

private:
	Point3D eyeLoc;
	Vector3D U,V,N;
	mutable double* modelViewMatrix;
};

#endif