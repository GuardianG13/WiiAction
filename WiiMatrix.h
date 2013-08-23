// File: 		WiiMatrix.h
// Author:		Travis Bueter
// Description:	

#ifndef WIIMATRIX_H
#define WIIMATRIX_H

#include <cmath>

class WiiMatrix
{
public:
	double Element[4][4];
	
	WiiMatrix();
	~WiiMatrix();
	
	void Delete(){ delete this;}
	void Identity();
	static void Identity(double Elements[16]);
	static void Multiply4x4(const double a[16], const double b[16], double c[16]);
	double GetElement(int x, int y) { return this->Element[x][y]; };
		
private:
	
};

#include "WiiMatrix.cxx"

#endif // WIIMATRIX_H