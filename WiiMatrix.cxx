// File: 		WiiMatrix.cpp
// Author:		Travis Bueter
// Description:	Class Definition of 4x4 Matrices

// Useful for viewing a double[16] as a double[4][4]
typedef double (*SqMatPtr)[4];

WiiMatrix::WiiMatrix()
{
	this->Identity(*this->Element);
}

WiiMatrix::~WiiMatrix()
{
	
}

void WiiMatrix::Identity()
{
	this->Identity(*this->Element);
}

void WiiMatrix::Identity(double Elements[16])
{
	Elements[0] = Elements[5] = Elements[10] = Elements[15] = 1.0;
	Elements[1] = Elements[2] = Elements[3] = Elements[4] =
	Elements[6] = Elements[7] = Elements[8] = Elements[9] =
	Elements[11] = Elements[12] = Elements[13] = Elements[14] = 0.0;
}

void WiiMatrix::Multiply4x4(const double a[16], const double b[16], double c[16])
{
	SqMatPtr aMat = (SqMatPtr) a;
	SqMatPtr bMat = (SqMatPtr) b;
	SqMatPtr cMat = (SqMatPtr) c;
	int i, k;
	double Accum[4][4];
	
	for (i = 0; i < 4; i++)
	{
		for (k = 0; k < 4; k++)
		{
		  Accum[i][k] = aMat[i][0] * bMat[0][k] +
						aMat[i][1] * bMat[1][k] +
						aMat[i][2] * bMat[2][k] +
						aMat[i][3] * bMat[3][k];
		}
	}
	
	// Copy to final dest
	for (i = 0; i < 4; i++)
	{
		cMat[i][0] = Accum[i][0];
		cMat[i][1] = Accum[i][1];
		cMat[i][2] = Accum[i][2];
		cMat[i][3] = Accum[i][3];
	}
}