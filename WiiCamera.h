// File: 		WiiCamera.h
// Author:		Travis Bueter
// Description:	

#ifndef WIICAMERA_H
#define WIICAMERA_H

#include "WiiMath.h"
#include "WiiTransform.h"

#define PI 3.14159265

struct CameraState
{
	float Position[3];
	float FocalPoint[3];
	float ViewUp[3];
};

class WiiCamera
{
public:
	WiiCamera();
	~WiiCamera();
	
	void OrthogonalizeViewUp();
	void ApplyTransform(WiiTransform *t);
	void ComputeViewTransform();
	void ComputeDistance();
	
	void ComputeViewPlaneNormal();
	CameraState GetCamState();
	void SetCamState(const CameraState cam);
	void GetFocalPoint(float x[3]);
	float *GetFocalPoint() { return camera.FocalPoint; };
	void SetFocalPoint(const float x, const float y, const float z);
	void GetViewUp(float x[3]);
	float *GetViewUp() { return camera.ViewUp; };
	void SetViewUp(const float x, const float y, const float z);
	void GetPosition(float x[3]);
	float *GetPosition() { return camera.Position; };
	void SetPosition(const float x, const float y, const float z);
	void GetDirectionOfProjection(float x[3]);
	float *GetDirectionOfProjection() { return DirectionOfProjection; };
	void GetViewPlaneNormal(float x[3]);
	float *GetViewPlaneNormal() { return ViewPlaneNormal; };
	float GetParallelScale() { return ParallelScale; };
	void SetParallelScale(float scale);
	void Delete(){ delete this; }
	
	float GetDistance(float x, float y, float z);
	
	CameraState camera;
	
private:
	float viewAngle;
	float DirectionOfProjection[3];
	float ViewPlaneNormal[3];
	WiiTransform *ViewTransform;
	WiiTransform *Transform;
	float Distance;
	float ParallelScale;
	
};

#include "WiiCamera.cxx"

#endif // WIICAMERA_H