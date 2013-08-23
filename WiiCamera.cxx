

WiiCamera::WiiCamera()
{

	camera.FocalPoint[0] = 0.0;
	camera.FocalPoint[1] = 0.0;
	camera.FocalPoint[2] = 0.0;
	
	camera.ViewUp[0] = 0.0;
	camera.ViewUp[1] = 1.0;
	camera.ViewUp[2] = 0.0;	
	
	camera.Position[0] = 0.0;
	camera.Position[1] = 0.0;
	camera.Position[2] = 1.0;
	
	DirectionOfProjection[0] = 0.0;
	DirectionOfProjection[1] = 0.0;
	DirectionOfProjection[2] = 0.0;
	
	transform = new WiiTransform;
}

WiiCamera::~WiiCamera()
{
	transform->Delete();
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
	WiiMatrix *matrix = this->transform->GetMatrix();
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



double WiiCamera::Norm(const double x[3])
{
	return sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2]);
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
	this->camera.Position[0] = x;
	this->camera.Position[1] = y;
	this->camera.Position[2] = z;
}

void WiiCamera::GetDirectionOfProjection(double x[3])
{
	for(int i = 0; i < 3; i++)
	{
		x[i] = this->DirectionOfProjection[i];
	}
}