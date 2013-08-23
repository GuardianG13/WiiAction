// File: 		WiiTransform.h
// Author:		Travis Bueter
// Description:	

#ifndef WIITRANSFORM_H
#define WIITRANSFORM_H

#include "WiiMatrix.h"

class WiiTransform
{
public:
	WiiTransform();
	~WiiTransform();
	
	void Identity();
	void Translate(double x, double y, double z);
	void RotateWXYZ(double angle, double x, double y, double z);
	WiiMatrix *GetMatrix(){ return this->Matrix; };
	void Delete() { delete this; };
	
	
private:
	WiiMatrix *Matrix;
	
};

#include "WiiTransform.cxx"

#endif // WIITRANSFORM_H