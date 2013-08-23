// File: 		WiiCamera.h
// Author:		Travis Bueter
// Description:	

#ifndef WIICAMERA_H
#define WIICAMERA_H

#include "WiiTransform.h"

#define PI 3.14159265

struct CameraState
{
	double FocalPoint[3];
	double ViewUp[3];
	double Position[3];
};

class WiiCamera
{
public:
	WiiCamera();
	~WiiCamera();
	
	void OrthogonalizeViewUp();
	void ApplyTransform(double matrix[4][4]);
//	void ComputeViewTransformation();
//	void ComputeDistance();
//	void ComputeCameraLightTransform();
	static double Norm(const double x[3]);
	CameraState GetCamState();
	void SetCamState(const CameraState cam);
	void GetFocalPoint(double x[3]);
	void SetFocalPoint(const double x, const double y, const double z);
	void GetViewUp(double x[3]);
	void SetViewUp(const double x, const double y, const double z);
	void GetPosition(double x[3]);
	void SetPosition(const double x, const double y, const double z);
	void GetDirectionOfProjection(double x[3]);
	void Delete(){ delete this; }
		
private:
	CameraState camera;
	double angle;
	double DirectionOfProjection[3];
	WiiTransform *transform;
	
	
	
};

#include "WiiCamera.cxx"

#endif // WIICAMERA_H