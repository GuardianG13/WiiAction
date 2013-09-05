

WiiCamera::WiiCamera()
{

	this->camera.FocalPoint[0] = 0.0;
	this->camera.FocalPoint[1] = 0.0;
	this->camera.FocalPoint[2] = 0.0;
	
	this->camera.ViewUp[0] = 0.0;
	this->camera.ViewUp[1] = 1.0;
	this->camera.ViewUp[2] = 0.0;	
	
	this->camera.Position[0] = 0.0;
	this->camera.Position[1] = 0.0;
	this->camera.Position[2] = 1.0;
	
	this->DirectionOfProjection[0] = 0.0;
	this->DirectionOfProjection[1] = 0.0;
	this->DirectionOfProjection[2] = 0.0;
	
	this->viewAngle = 30;
	viewTransform = new WiiTransform;
	Transform = new WiiTransform;
}

WiiCamera::~WiiCamera()
{
	ViewTransform->Delete();
	Transform->Delete();
}

void MultiplyPoint(const double t[16], const double in[4], double out[4])
{
	double v1 = in[0];
	double v2 = in[1];
	double v3 = in[2];
	double v4 = in[3];

	out[0] = v1*t[0]  + v2*t[1]  + v3*t[2]  + v4*t[3];
	out[1] = v1*t[4]  + v2*t[5]  + v3*t[6]  + v4*t[7];
	out[2] = v1*t[8]  + v2*t[9]  + v3*t[10] + v4*t[11];
	out[3] = v1*t[12] + v2*t[13] + v3*t[14] + v4*t[15];
}

void WiiCamera::OrthogonalizeViewUp()
{
	WiiMatrix *matrix = this->VietTransform->GetMatrix();
	this->camera.ViewUp[0] = matrix->GetElement(1,0);
	this->camera.ViewUp[1] = matrix->GetElement(1,1);
	this->camera.ViewUp[2] = matrix->GetElement(1,2);
}

void WiiCamera::ApplyTransform(double matrix[4][4])
{
	double posOld[4], posNew[4], fpOld[4], fpNew[4], vuOld[4], vuNew[4];
	for(int i = 0; i < 3; i++)
	{
		fpOld[i] = camera.FocalPoint[i];
		vuOld[i] = camera.ViewUp[i];
		posOld[i] = camera.Position[i];
	}
	posOld[3] = 1.0;
	fpOld[3] = 1.0;
	vuOld[3] = 1.0;

	vuOld[0] += posOld[0];
	vuOld[1] += posOld[1];
	vuOld[2] += posOld[2];
	
	MultiplyPoint(*matrix, posOld, posNew);
	MultiplyPoint(*matrix, fpOld, fpNew);
	MultiplyPoint(*matrix, vuOld, vuNew);
	
	vuNew[0] -= posNew[0];
	vuNew[1] -= posNew[1];
	vuNew[2] -= posNew[2];
	
	this->SetPosition(posNew[0], posNew[1], posNew[2]);
	this->SetFocalPoint(fpNew[0], fpNew[1], fpNew[2]);
	this->SetViewUp(vuNew[0], vuNew[1], vuNew[2]);
}

CameraState WiiCamera::GetCamState()
{
	return this->camera;
}

void WiiCamera::SetCamState(const CameraState cam)
{
	for(int i = 0; i < 3; i++)
	{
		this->camera.FocalPoint[i] = cam.FocalPoint[i];
		this->camera.ViewUp[i] = cam.ViewUp[i];
		this->camera.Position[i] = cam.Position[i];
	}
}

void WiiCamera::GetFocalPoint(double x[3])
{
	for(int i = 0; i < 3; i++)
	{
		x[i] = this->camera.FocalPoint[i];
	}
}

void WiiCamera::SetFocalPoint(const double x, const double y, const double z)
{
	this->camera.FocalPoint[0] = x;
	this->camera.FocalPoint[1] = y;
	this->camera.FocalPoint[2] = z;
}

void WiiCamera::GetViewUp(double x[3])
{
	for(int i = 0; i < 3; i++)
	{
		x[i] = this->camera.ViewUp[i];
	}
}

void WiiCamera::SetViewUp(const double x, const double y, const double z)
{
	this->camera.ViewUp[0] = x;
	this->camera.ViewUp[1] = y;
	this->camera.ViewUp[2] = z;
}

void WiiCamera::GetPosition(double x[3])
{
	for(int i = 0; i < 3; i++)
	{
		x[i] = this->camera.Position[i];
	}
}

void WiiCamera::SetPosition(const double x, const double y, const double z)
{
	if(x == this->camera.Position[0] && 
	   y == this->camera.Position[1] &&
	   z == this->camera.Position[2])
	{
		return;
	}
	
	this->camera.Position[0] = x;
	this->camera.Position[1] = y;
	this->camera.Position[2] = z;
	
	this->ComputeViewTransform();
	this->ComputeDistance();
	//this->ComputeCameraLightTransform();
}

void WiiCamera::GetDirectionOfProjection(double x[3])
{
	for(int i = 0; i < 3; i++)
	{
		x[i] = this->DirectionOfProjection[i];
	}
}

void WiiCamera::ComputeViewTransform()
{
	this->Transform->Identity();
	this->Transform->SetupCamera(this->camera.Position, this->camera.FocalPoint, this->camera.ViewUp);
	this->ViewTransform->SetMatrix(this->Transform->GetMatrix());
}

void WiiCamera::ComputeViewPlaneNormal()
{
	this->ViewPlaneNormal[0] = -this->DirectionOfProjection[0];
	this->ViewPlaneNormal[1] = -this->DirectionOfProjection[1];
	this->ViewPlaneNormal[2] = -this->DirectionOfProjection[2];
}

void WiiCamera::ComputeDistance()
{
	double dx = this->camera.FocalPoint[0] - this->camera.Position[0];
	double dy = this->camera.FocalPoint[1] - this->camera.Position[1];
	double dz = this->camera.FocalPoint[2] - this->camera.Position[2];
	
	this->Distance = sqrt(dx*dx + dy*dy + dz*dz);
	
	if (this->Distance < 1e-20)
	{
		this->Distance = 1e-20;		
		double *vec = this->DirectionOfProjection;
		
		// recalculate FocalPoint
		this->camera.FocalPoint[0] = this->camera.Position[0] + vec[0]*this->Distance;
		this->camera.FocalPoint[1] = this->camera.Position[1] + vec[1]*this->Distance;
		this->camera.FocalPoint[2] = this->camera.Position[2] + vec[2]*this->Distance;
	}
	
	this->DirectionOfProjection[0] = dx/this->Distance;
	this->DirectionOfProjection[1] = dy/this->Distance;
	this->DirectionOfProjection[2] = dz/this->Distance;
	
	this->ComputeViewPlaneNormal();
}