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
	void Translate(float x, float y, float z);
	void RotateWXYZ(float angle, float x, float y, float z);
	WiiMatrix *GetMatrix(){ this->Update(); return this->Matrix; };
	void SetupCamera(const float position[3], const float focalPoint[3], const float viewUp[3]);
	void SetMatrix(WiiMatrix* matrix) {
		this->SetMatrix(*matrix->Element); };
	void SetMatrix(const float elements[16]) {
		this->Identity(); this->Concatenate(elements); };
	void Delete() { delete this; };
	void Update();
	void Concatenate(const float elements[16]);
	
	// Description:
	// Compute the transformation in homogeneous (x,y,z,w) coordinates.
	// This method calls this->GetMatrix()->MultiplyPoint().
	void MultiplyPoint(const float in[4], float out[4]) {
	  this->GetMatrix()->MultiplyPoint(in,out);};
	
private:
	WiiMatrix *Matrix;
	WiiMatrix *PreMatrix;

};

#include "WiiTransform.cxx"

#endif // WIITRANSFORM_H