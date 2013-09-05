// File: 		WiiTransform.cpp
// Author:		Travis Bueter
// Description:	Class Definition of Transformation matrix



#ifndef PI
#define PI 3.14159265
#endif

WiiTransform::WiiTransform()
{
	this->Matrix = new WiiMatrix;
	this->PreMatrix = NULL;
}

WiiTransform::~WiiTransform()
{
	this->Matrix->Delete();
}

void WiiTransform::Identity()
{
	if(PreMatrix != NULL)
	{
		this->PreMatrix->Delete();
		this->PreMatrix = NULL;
	}
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

void WiiTransform::SetupCamera(const double position[3],
						       const double focalPoint[3],
						       const double viewUp[3])
{
  double matrix[4][4];
  WiiMatrix::Identity(*matrix);

  // the view directions correspond to the rows of the rotation matrix,
  // so we'll make the connection explicit
  double *viewSideways =    matrix[0];
  double *orthoViewUp =     matrix[1];
  double *viewPlaneNormal = matrix[2];

  // set the view plane normal from the view vector
  viewPlaneNormal[0] = position[0] - focalPoint[0];
  viewPlaneNormal[1] = position[1] - focalPoint[1];
  viewPlaneNormal[2] = position[2] - focalPoint[2];
  WiiAction::Normalize(viewPlaneNormal);

  // orthogonalize viewUp and compute viewSideways
  WiiAction::Cross(viewUp,viewPlaneNormal,viewSideways);
  WiiAction::Normalize(viewSideways);
  WiiAction::Cross(viewPlaneNormal,viewSideways,orthoViewUp);

  // translate by the vector from the position to the origin
  double delta[4];
  delta[0] = -position[0];
  delta[1] = -position[1];
  delta[2] = -position[2];
  delta[3] = 0.0; // yes, this should be zero, not one

  WiiMatrix::MultiplyPoint(*matrix,delta,delta);

  matrix[0][3] = delta[0];
  matrix[1][3] = delta[1];
  matrix[2][3] = delta[2];

  // apply the transformation
  this->Concatenate(*matrix);
}

void WiiTransform::Concatenate(const double elements[16])
{
	if(this->PreMatrix == NULL)
	{
		this->PreMatrix = new WiiMatrix;
	}
	vtkMatrix4x4::Multiply4x4(*this->PreMatrix->Element, elements,
	                          *this->PreMatrix->Element);
}

void WiiTransform::SetMatrix(const double elements[16])
{
	this->Identity();
	this->Concatenate(elements);
}





