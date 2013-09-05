// File: 		WiiCamera.h
// Author:		Travis Bueter
// Description:	

#ifndef WIICAMERA_H
#define WIICAMERA_H

#include "WiiTransform.h"

#define PI 3.14159265

struct CameraState
{
	float FocalPoint[3];
	float ViewUp[3];
	float Position[3];
};

class WiiCamera
{
public:
	WiiCamera();
	~WiiCamera();
	
	void OrthogonalizeViewUp();
	void ApplyTransform(double matrix[4][4]);
	void ComputeViewTransform();
	void ComputeDistance();
	void ComputeViewPlaneNormal();
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
	double viewAngle;
	double DirectionOfProjection[3];
	WiiTransform *viewTransform;
	WiiTransform *Transform;
	double Distance;
	
};

#include "WiiCamera.cxx"

#endif // WIICAMERA_H