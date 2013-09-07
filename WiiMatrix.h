// File: 		WiiMatrix.h
// Author:		Travis Bueter
// Description:	

#ifndef WIIMATRIX_H
#define WIIMATRIX_H

class WiiMatrix
{
public:
	float Element[4][4];
	
	WiiMatrix();
	~WiiMatrix();
	
	void Delete(){ delete this;}
	void Identity();
	static void Identity(float Elements[16]);
	static void Multiply4x4(const float a[16], const float b[16], float c[16]);
	
	// Description:
	// Multiplies matrices a and b and stores the result in c.
	static void Multiply4x4(const WiiMatrix *a, const WiiMatrix *b, WiiMatrix *c) {
	  WiiMatrix::Multiply4x4(*a->Element,*b->Element,*c->Element); };
	
	static void MultiplyPoint(const float t[16], const float in[4], float out[4]);
	
	// Description:
	// Multiply a homogeneous coordinate by this matrix, i.e. out = A*in.
	// The in[4] and out[4] can be the same array.
	void MultiplyPoint(const float in[4], float out[4])
	  {WiiMatrix::MultiplyPoint(*this->Element,in,out); }
	
	float GetElement(int x, int y) { return this->Element[x][y]; };
		
private:
	
};

#include "WiiMatrix.cxx"

#endif // WIIMATRIX_H