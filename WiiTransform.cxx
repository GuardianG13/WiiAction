// File: 		WiiTransform.cpp
// Author:		Travis Bueter
// Description:	Class Definition of Transformation matrix



#ifndef PI
#define PI 3.14159265
#endif

WiiTransform::WiiTransform()
{
	this->Matrix = new WiiMatrix;
}

WiiTransform::~WiiTransform()
{
	this->Matrix->Delete();
}

void WiiTransform::Identity()
{
	this->Matrix->Identity();
}

void WiiTransform::Translate(double x, double y, double z)
{
	if (x == 0.0 && y == 0.0 && z == 0.0)
	{
		return;
	}

    double matrix[4][4];
    WiiMatrix::Identity(*matrix);

    matrix[0][3] = x;
    matrix[1][3] = y;
    matrix[2][3] = z;
    
    WiiMatrix::Multiply4x4(*this->Matrix->Element, *matrix, *this->Matrix->Element);
}

void WiiTransform::RotateWXYZ(double angle, double x, double y, double z)
{
	if (angle == 0.0 || (x == 0.0 && y == 0.0 && z == 0.0))
	{
	    return;
	}
	
	angle = PI*angle/180;
	
	double w = cos(0.5*angle);
	double f = sin(0.5*angle)/sqrt(x*x+y*y+z*z);
	x *= f;
	y *= f;
	z *= f;
	
	double matrix[4][4];
	WiiMatrix::Identity(*matrix);
	
	double ww = w*w;
	double wx = w*x;
	double wy = w*y;
	double wz = w*z;
	
	double xx = x*x;
	double yy = y*y;
	double zz = z*z;
	
	double xy = x*y;
	double xz = x*z;
	double yz = y*z;
	
	double s = ww - xx - yy - zz;
	
	matrix[0][0] = xx*2 + s;
	matrix[1][0] = (xy + wz)*2;
	matrix[2][0] = (xz - wy)*2;
	
	matrix[0][1] = (xy - wz)*2;
	matrix[1][1] = yy*2 + s;
	matrix[2][1] = (yz + wx)*2;
	
	matrix[0][2] = (xz + wy)*2;
	matrix[1][2] = (yz - wx)*2;
	matrix[2][2] = zz*2 + s;
	
	WiiMatrix::Multiply4x4(*this->Matrix->Element, *matrix, *this->Matrix->Element);
}