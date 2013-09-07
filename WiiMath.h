// File: 		WiiMatrix.h
// Author:		Travis Bueter
// Description:	

#ifndef WIIMATH_H
#define WIIMATH_H

#include <cmath>

class WiiMath
{
public:

	// Description:
    // Compute the norm of 3-vector.
    static float Norm(const float x[3]) {
	  return static_cast<float> (sqrt( x[0] * x[0] + x[1] * x[1] + x[2] * x[2] ) );};
	
    // Description:
    // Normalize (in place) a 3-vector. Returns norm of vector.
    static float Normalize(float x[3]);
    
    // Description:
    // Cross product of two 3-vectors. Result (a x b) is stored in z.
    static void Cross(const float x[3], const float y[3], float z[3]);
		
protected:
    WiiMath() {};
    ~WiiMath() {};
};

//----------------------------------------------------------------------------
inline float WiiMath::Normalize(float x[3])
{
  float den;
  if ( ( den = WiiMath::Norm( x ) ) != 0.0 )
    {
    for (int i=0; i < 3; i++)
      {
      x[i] /= den;
      }
    }
  return den;
}

//----------------------------------------------------------------------------
// Cross product of two 3-vectors. Result (a x b) is stored in z[3].
inline void WiiMath::Cross(const float x[3], const float y[3], float z[3])
{
  float Zx = x[1] * y[2] - x[2] * y[1];
  float Zy = x[2] * y[0] - x[0] * y[2];
  float Zz = x[0] * y[1] - x[1] * y[0];
  z[0] = Zx; z[1] = Zy; z[2] = Zz;
}

#endif // WIIMATH_H