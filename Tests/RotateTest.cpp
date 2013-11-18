//Created by: Travis Bueter
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <pthread.h>
#include <string>
#include <sstream>
#include <cmath>
#include <errno.h>
#include <stdint.h>
#include <json/json.h>

using namespace std;

enum pThread_arg { Send = 0, Get = 1 };

void json_parse(json_object *jobj);
void Create_pThread(pThread_arg data);

/*struct Renderers {
	int layer;
	float Background1[3];
	float Background2[3];
	float LookAt[10];
	float size[2];
	float origin[2];
}*/

typedef float (*SqMatPtr)[4];
bool transform_Concatenation_PreMatrix = false;
bool camera_Transform_Concatenation_PreMatrix = false;
bool camera_ViewTransform_Concatenation_PreMatrix = false;
float transform_Concatenation_Pre_Matrix[4][4];
float camera_Transform_Concatenation_Pre_Matrix[4][4];
float camera_ViewTransform_Concatenation_Pre_Matrix[4][4];
float transform_Matrix[4][4];
float camera_Transform_Matrix[4][4];
float camera_ViewTransform_Matrix[4][4];
float center[3];
float DirectionOfProjection[3];
float ViewPlaneNormal[3];
float Distance;
int s;
sockaddr_in svr;
float MaxSize;
pthread_t thread;
//Renderers* ren;

struct CameraState
{
	float Position[3];
	float FocalPoint[3];
	float ViewUp[3];
} camera;

float Norm(const float x[3])
{
	return static_cast<float> (sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2]));
}

void Identity(float Elements[16])
{
	Elements[0] = Elements[5] = Elements[10] = Elements[15] = 1.0;
	Elements[1] = Elements[2] = Elements[3] = Elements[4] =
	Elements[6] = Elements[7] = Elements[8] = Elements[9] =
	Elements[11] = Elements[12] = Elements[13] = Elements[14] = 0.0;
}

void Multiply4x4(const float a[16], const float b[16], float c[16])
{
	SqMatPtr aMat = (SqMatPtr) a;
	SqMatPtr bMat = (SqMatPtr) b;
	SqMatPtr cMat = (SqMatPtr) c;
	int i, k;
	float Accum[4][4];
	
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

void camera_Transform_Matrix_Update()
{
	Identity(*camera_Transform_Matrix);
	Multiply4x4(*camera_Transform_Matrix, *camera_Transform_Concatenation_Pre_Matrix, *camera_Transform_Matrix);
}

void camera_ViewTransform_Matrix_Update()
{
	Identity(*camera_ViewTransform_Matrix);
	Multiply4x4(*camera_ViewTransform_Matrix, *camera_ViewTransform_Concatenation_Pre_Matrix, *camera_ViewTransform_Matrix);
}

void transform_Matrix_Update()
{
	Identity(*transform_Matrix);
	Multiply4x4(*transform_Matrix, *transform_Concatenation_Pre_Matrix, *transform_Matrix);
}

void transform_Concatenate(const float elements[16])
{
	if(!transform_Concatenation_PreMatrix)
	{
		transform_Concatenation_PreMatrix = true;
		Identity(*transform_Concatenation_Pre_Matrix);
	}
	Multiply4x4(*transform_Concatenation_Pre_Matrix, elements, *transform_Concatenation_Pre_Matrix);
}

void Transform_Concatenate(const float elements[16])
{
	if(!camera_Transform_Concatenation_PreMatrix)
	{
		camera_Transform_Concatenation_PreMatrix = true;
		Identity(*camera_Transform_Concatenation_Pre_Matrix);
	}
	Multiply4x4(*camera_Transform_Concatenation_Pre_Matrix, elements, *camera_Transform_Concatenation_Pre_Matrix);
}

void ViewTransform_Concatenate(const float elements[16])
{
	if(!camera_ViewTransform_Concatenation_PreMatrix)
	{
		camera_ViewTransform_Concatenation_PreMatrix = true;
		Identity(*camera_ViewTransform_Concatenation_Pre_Matrix);
	}
	Multiply4x4(*camera_ViewTransform_Concatenation_Pre_Matrix, elements, *camera_ViewTransform_Concatenation_Pre_Matrix);
}

void Translate(float x, float y, float z)
{
	if (x == 0.0 && y == 0.0 && z == 0.0)
	{
		return;
	}

    float matrix[4][4];
    Identity(*matrix);

    matrix[0][3] = x;
    matrix[1][3] = y;
    matrix[2][3] = z;
    
    transform_Concatenate(*matrix);
    
}

void OrthogonalizeViewUp()
{
	camera_ViewTransform_Matrix_Update();
	
	camera.ViewUp[0] = camera_ViewTransform_Matrix[1][0];
	camera.ViewUp[1] = camera_ViewTransform_Matrix[1][1];
	camera.ViewUp[2] = camera_ViewTransform_Matrix[1][2];
}

void RotateWXYZ(float angle, float x, float y, float z)
{
	if (angle == 0.0 || (x == 0.0 && y == 0.0 && z == 0.0))
	{
	    return;
	}

	angle = angle*0.017453292f;
	
	float w = cos(0.5*angle);
	float f = sin(0.5*angle)/sqrt(x*x+y*y+z*z);
	x *= f;
	y *= f;
	z *= f;
	
	float matrix[4][4];
	Identity(*matrix);
	
	float ww = w*w;
	float wx = w*x;
	float wy = w*y;
	float wz = w*z;
	
	float xx = x*x;
	float yy = y*y;
	float zz = z*z;
	
	float xy = x*y;
	float xz = x*z;
	float yz = y*z;
	
	float s = ww - xx - yy - zz;
	
	matrix[0][0] = xx*2 + s;
	matrix[1][0] = (xy + wz)*2;
	matrix[2][0] = (xz - wy)*2;
	
	matrix[0][1] = (xy - wz)*2;
	matrix[1][1] = yy*2 + s;
	matrix[2][1] = (yz + wx)*2;
	
	matrix[0][2] = (xz + wy)*2;
	matrix[1][2] = (yz - wx)*2;
	matrix[2][2] = zz*2 + s;
	
	transform_Concatenate(*matrix);
}

void GetDirectionOfProjection(float x[3])
{
	for(int i = 0; i < 3; i++)
	{
		x[i] = DirectionOfProjection[i];
	}
}

float Normalize(float x[3])
{
	float den;
	if ( ( den = Norm( x ) ) != 0.0 )
	{
		for (int i=0; i < 3; i++)
		{
			x[i] /= den;
		}
	}
	return den;
}

void Cross(const float x[3], const float y[3], float z[3])
{
	float Zx = x[1] * y[2] - x[2] * y[1];
	float Zy = x[2] * y[0] - x[0] * y[2];
	float Zz = x[0] * y[1] - x[1] * y[0];
	z[0] = Zx; z[1] = Zy; z[2] = Zz;
}

void MultiplyPoint(const float t[16], const float in[4], float out[4])
{
	float v1 = in[0];
	float v2 = in[1];
	float v3 = in[2];
	float v4 = in[3];

	out[0] = v1*t[0]  + v2*t[1]  + v3*t[2]  + v4*t[3];
	out[1] = v1*t[4]  + v2*t[5]  + v3*t[6]  + v4*t[7];
	out[2] = v1*t[8]  + v2*t[9]  + v3*t[10] + v4*t[11];
	out[3] = v1*t[12] + v2*t[13] + v3*t[14] + v4*t[15];
}

void SetupCamera()
{
	float matrix[4][4];
	Identity(*matrix);
	
	float *viewSideways =    matrix[0];
	float *orthoViewUp =     matrix[1];
	float *viewPlaneNormal = matrix[2];
	
	// set the view plane normal from the view vector
	viewPlaneNormal[0] = camera.Position[0] - camera.FocalPoint[0];
	viewPlaneNormal[1] = camera.Position[1] - camera.FocalPoint[1];
	viewPlaneNormal[2] = camera.Position[2] - camera.FocalPoint[2];
	Normalize(viewPlaneNormal);
	
	// orthogonalize viewUp and compute viewSideways
	Cross(camera.ViewUp,viewPlaneNormal,viewSideways);
	Normalize(viewSideways);
	Cross(viewPlaneNormal,viewSideways,orthoViewUp);
	
	// translate by the vector from the position to the origin
	float delta[4];
	delta[0] = -camera.Position[0];
	delta[1] = -camera.Position[1];
	delta[2] = -camera.Position[2];
	delta[3] = 0.0; // yes, this should be zero, not one
	
	MultiplyPoint(*matrix,delta,delta);
	
	matrix[0][3] = delta[0];
	matrix[1][3] = delta[1];
	matrix[2][3] = delta[2];
	
	// apply the transformation
	Transform_Concatenate(*matrix); //Transform->Concatenation->Concatenate(elements);
}

void SetMatrix(const float elements[16])
{
	camera_ViewTransform_Concatenation_PreMatrix = false;
	ViewTransform_Concatenate(elements);
}

void ComputeViewTransform()
{
	camera_Transform_Concatenation_PreMatrix = false; //Camera->Transform->Concatenation->Identity();
	SetupCamera(); //Camera->Transform->SetupCamera();
	camera_Transform_Matrix_Update();
	SetMatrix(*camera_Transform_Matrix); //Camera->ViewTransform->SetMatrix( Camera->Transform->Matrix );
}

void ComputeViewPlaneNormal()
{
	ViewPlaneNormal[0] = -DirectionOfProjection[0];
	ViewPlaneNormal[1] = -DirectionOfProjection[1];
	ViewPlaneNormal[2] = -DirectionOfProjection[2];
}

void ComputeDistance()
{
	float dx = camera.FocalPoint[0] - camera.Position[0];
	float dy = camera.FocalPoint[1] - camera.Position[1];
	float dz = camera.FocalPoint[2] - camera.Position[2];
	
	Distance = sqrt(dx*dx + dy*dy + dz*dz);
	
	if (Distance < 1e-20)
	{
		Distance = 1e-20;
		
		float *vec = DirectionOfProjection;
		
		// recalculate FocalPoint
		camera.FocalPoint[0] = camera.Position[0] + vec[0]*Distance;
		camera.FocalPoint[1] = camera.Position[1] + vec[1]*Distance;
		camera.FocalPoint[2] = camera.Position[2] + vec[2]*Distance;
	}
	
	DirectionOfProjection[0] = dx/Distance;
	DirectionOfProjection[1] = dy/Distance;
	DirectionOfProjection[2] = dz/Distance;
	
	ComputeViewPlaneNormal();
}

void GetPosition(float x[3])
{
	for(int i = 0; i < 3; i++)
	{
		x[i] = camera.Position[i];
	}
}

void SetPosition(const float x, const float y, const float z)
{
	if (x == camera.Position[0] &&
	    y == camera.Position[1] &&
	    z == camera.Position[2])
	    {
	    	return;
	    }

	camera.Position[0] = x;
	camera.Position[1] = y;
	camera.Position[2] = z;

	ComputeViewTransform();
	ComputeDistance();
	camera_ViewTransform_Matrix_Update();
}

void GetFocalPoint(float x[3])
{
	for(int i = 0; i < 3; i++)
	{
		x[i] = camera.FocalPoint[i];
	}
}

void SetFocalPoint(const float x, const float y, const float z)
{
	if (x == camera.FocalPoint[0] &&
	    y == camera.FocalPoint[1] &&
	    z == camera.FocalPoint[2])
	{
	    return;
	}
	
	camera.FocalPoint[0] = x;
	camera.FocalPoint[1] = y;
	camera.FocalPoint[2] = z;
	
	ComputeViewTransform();
	ComputeDistance();
	camera_ViewTransform_Matrix_Update();
}

void GetViewUp(float x[3])
{
	for(int i = 0; i < 3; i++)
	{
		x[i] = camera.ViewUp[i];
	}
}

void SetViewUp(float x, float y, float z)
{
	// normalize ViewUp, but do _not_ orthogonalize it by default
	float norm = sqrt(x*x + y*y + z*z);
	
	if(norm != 0)
	{
		x /= norm;
		y /= norm;
		z /= norm;
	}
	else
	{
		x = 0;
		y = 1;
		z = 0;
	}
	
	if (x == camera.ViewUp[0] &&
		y == camera.ViewUp[1] &&
		z == camera.ViewUp[2])
	{
		return;
	}
	
	camera.ViewUp[0] = x;
	camera.ViewUp[1] = y;
	camera.ViewUp[2] = z;
	
	ComputeViewTransform();
	ComputeDistance();
	camera_ViewTransform_Matrix_Update();
}



void ApplyTransform()
{
	float posOld[4], posNew[4], fpOld[4], fpNew[4], vuOld[4], vuNew[4];
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
	
	transform_Matrix_Update();
	MultiplyPoint(*transform_Matrix, posOld, posNew);
	MultiplyPoint(*transform_Matrix, fpOld, fpNew);
	MultiplyPoint(*transform_Matrix, vuOld, vuNew);
	
	vuNew[0] -= posNew[0];
	vuNew[1] -= posNew[1];
	vuNew[2] -= posNew[2];
	
	SetPosition(posNew[0], posNew[1], posNew[2]);
	SetFocalPoint(fpNew[0], fpNew[1], fpNew[2]);
	SetViewUp(vuNew[0], vuNew[1], vuNew[2]);
}

int Receive(const int socket, void* data, int len)
{
	char* buffer = (char*)data;
	int total = 0;
	do
	{
		int nRecvd = recv(socket, buffer+total, len-total, 0);
		if(nRecvd == 0)
		{
			return 0;
		}
		else if(nRecvd == -1)
		{
			printf("Immediate receive Failure\n");
			return -1;
		}
		total += nRecvd;
	}while(total < len);
	return total;
}

bool receiveReadyCommand(int socket)
{
	int command = 0;
	Receive(socket, &command, 4);
	if(command == 1)
		return true;
	else
		return false;
}

void *SendData()
{
	int command = 4;
	if(receiveReadyCommand(s))
	{
		cout << "Ready Command Received" << endl;
		if(-1 == send(s, &command, sizeof(command), 0))
		{
			printf("Command Send Fail. \n");
			exit(1);
		}
		if(-1 == send(s, &camera, sizeof(CameraState), 0))
		{
			printf("Data Send Fail. \n");
			exit(1);
		}
	}
	else
		cout << "Ready Command not Received" << endl;

	pthread_exit(NULL);
}

void printData()
{
	cout << "Center: [" << center[0] << "," << center[1] << "," << center[2] << "]" << endl;
	cout << "FocalPoint: [" << camera.FocalPoint[0] << "," << camera.FocalPoint[1] << "," << camera.FocalPoint[2] << "]" << endl;
	cout << "ViewUp: [" << camera.ViewUp[0] << "," << camera.ViewUp[1] << "," << camera.ViewUp[2] << "]" << endl;
	cout << "Position: [" << camera.Position[0] << "," << camera.Position[1] << "," << camera.Position[2] << "]" << endl;
	cout << "Size: " << MaxSize << endl;
	cout << endl;
}

void Clear_Transform()
{
	transform_Concatenation_PreMatrix = false;
	Identity(*transform_Concatenation_Pre_Matrix);
	Identity(*transform_Matrix);
}

float *GetFocalPoint()
{
	return camera.FocalPoint;
}

float *GetPosition()
{
	return camera.Position;
}

float *GetViewUp()
{
	return camera.ViewUp;
}

void Rotate(float dx, float dy)
{	
	Identity(*transform_Matrix);

	float scale = Norm(camera.Position);
	if(scale<=0.0)
	{
		scale = Norm(camera.FocalPoint);
		if(scale<=0.0)
		{
			scale = 1.0;
		}
	}
	
	float* temp = GetFocalPoint();
	SetFocalPoint(temp[0]/scale, temp[1]/scale, temp[2]/scale);
	temp = GetPosition();
	SetPosition(temp[0]/scale, temp[1]/scale, temp[2]/scale);
	
	float v2[3];
	// translate to center
	transform_Concatenation_PreMatrix = false;
	Translate(center[0]/scale, center[1]/scale, center[2]/scale);
	
	//azimuth
	OrthogonalizeViewUp();
	float *viewUp = GetViewUp();
	RotateWXYZ(360.0*dx, viewUp[0], viewUp[1], viewUp[2]);
	
	//elevation
	Cross(DirectionOfProjection, viewUp, v2);
	RotateWXYZ(-360.0*dy, v2[0], v2[1], v2[2]);

	// translate back
	Translate(-center[0]/scale, -center[1]/scale, -center[2]/scale);
	ApplyTransform();
	OrthogonalizeViewUp(); 
	
	temp = GetFocalPoint();
	SetFocalPoint(temp[0]*scale, temp[1]*scale, temp[2]*scale);
	temp = GetPosition();
	SetPosition(temp[0]*scale, temp[1]*scale, temp[2]*scale);

	Create_pThread(Send);
	Clear_Transform();
}

void Roll(float dx)
{
	float axis[3];
	Identity(*transform_Matrix);

	float* fp = GetFocalPoint();
	float* pos = GetPosition();
	
	axis[0] = fp[0] - pos[0];
	axis[1] = fp[1] - pos[1];
	axis[2] = fp[2] - pos[2];

	float v2[3];
	// translate to center
	transform_Concatenation_PreMatrix = false;
	Translate(center[0], center[1], center[2]);
	
	RotateWXYZ(-360.0*dx, axis[0], axis[1], axis[2]);

	// translate back
	Translate(-center[0], -center[1], -center[2]);
	ApplyTransform();
	OrthogonalizeViewUp(); 
	
	Create_pThread(Send);
	Clear_Transform();
}

void print_json_value(json_object *jobj){
  enum json_type type;
  printf("type: ",type);
  type = json_object_get_type(jobj); /*Getting the type of the json object*/
  switch (type) {
    case json_type_boolean: printf("json_type_boolean\n");
                         printf("value: %s\n", json_object_get_boolean(jobj)? "true": "false");
                         break;
    case json_type_double: printf("json_type_double\n");
                        printf("          value: %lf\n", json_object_get_double(jobj));
                         break;
    case json_type_int: printf("json_type_int\n");
                        printf("          value: %d\n", json_object_get_int(jobj));
                         break;
    case json_type_string: printf("json_type_string\n");
                         printf("          value: %s\n", json_object_get_string(jobj));
                         break;
  }
}

void json_parse_array( json_object *jobj, char *key) {
  enum json_type type;
  json_object *jarray = jobj; /*Simply get the array*/
  if(key) {
    jarray = json_object_object_get(jobj, key); /*Getting the array if it is a key value pair*/
  }

  int arraylen = json_object_array_length(jarray); /*Getting the length of the array*/
  printf("Array Length: %d\n",arraylen);
  int i;
  json_object * jvalue;

  for (i=0; i< arraylen; i++){
    jvalue = json_object_array_get_idx(jarray, i); /*Getting the array element at position i*/
    type = json_object_get_type(jvalue);
    if (type == json_type_array) {
      json_parse_array(jvalue, NULL);
    }
    else if (type != json_type_object) {
      printf("value[%d]: ",i);
      print_json_value(jvalue);
    }
    else {
      json_parse(jvalue);
    }
  }
}

void json_get_array_values( json_object *jobj, char *key, float a[]) {
  enum json_type type;

  json_object *jarray = jobj; /*Simply get the array*/
  if(key) {
    jarray = json_object_object_get(jobj, key); /*Getting the array if it is a key value pair*/
	if(!jarray) {
		cout << "Uh oh" << endl;
		return;
	}		
  }

  int arraylen = json_object_array_length(jarray); /*Getting the length of the array*/

  int i;
  json_object * jvalue;

  for (i=0; i< arraylen; i++){
    jvalue = json_object_array_get_idx(jarray, i); /*Getting the array element at position i*/
    type = json_object_get_type(jvalue);
    if (type == json_type_array) {
      json_get_array_values(jvalue, NULL, a);
    }
    else if (type != json_type_object) {
    	  a[i] = json_object_get_double(jvalue);
      }
    else {
      json_get_array_values(jvalue, "LookAt", a);
      return;
    }
  }
}

/*Parsing the json object*/
void json_parse(json_object * jobj) {
  enum json_type type;
  json_object_object_foreach(jobj, key, val) { /*Passing through every array element*/
	printf("Key = %s\n",key);    
    type = json_object_get_type(val);
    switch (type) {
      case json_type_boolean: 
      case json_type_double: 
      case json_type_int: 
      case json_type_string: print_json_value(val);
                           break; 
      case json_type_object: printf("json_type_object\n");
                           jobj = json_object_object_get(jobj, key);
                           json_parse(jobj); 
                           break;
      case json_type_array: printf("type: json_type_array, ");
                          json_parse_array(jobj, key);
                          break;
    }
  }
}

void json_get_MaxSize(json_object * jobj, float &a)
{
	enum json_type type;
    const char *key = "MaxSize";
	json_object *j = jobj;
	j = json_object_object_get(jobj,key);
	if(j){
		a = (float)json_object_get_double(j);
		cout << a << endl;
	}
	else
		cout << "Error retrieving value" << endl;
}

void *GetData()
{
	int ready = 0;
	int command = 2;
	int size;
	float cam[10];
	
	while(ready != 1)
	{
		if((size = Receive(s, &ready, sizeof(ready))) != 4)
			printf("Size not 4 bytes. Continue anyways.\n");
	}
	
	if(-1 == send(s, &command, sizeof(command), 0))
	{
		printf("Command Send Fail. \n");
		exit(1);
	}	
	
	unsigned long long length;
	if(!Receive(s, &length, sizeof(unsigned long long)))
	{
		printf("Receive Failure\n");
	}
	
	char metadata[length+1];

	if(!Receive(s, &metadata, length))
	{
		printf("MetaData not received.\n");
	}
	metadata[length] = '\0';
	//printf("%s\n",&metadata);	
	json_object * jobj = json_tokener_parse((char*)metadata);

	json_get_MaxSize(jobj, MaxSize);

	json_get_array_values(jobj, "Center", center);

	json_get_array_values(jobj, "Renderers", cam);

	for(int i = 0; i < 9; i++)
	{
		if(i < 3)
		{
			camera.FocalPoint[i] = cam[i+1];
		}
		else if((i >= 3)&&(i < 6))
		{
			camera.ViewUp[i-3] = cam[i+1];
		}
		else
		{
			camera.Position[i-6] = cam[i+1];
		}
	}
	cout << "MaxSize: " << MaxSize << endl;
	printData();
	pthread_exit(NULL);
}

void ClearSocket()
{
	int ready = 0;
	int command = 5;
	int size;

	while(ready != 1)
	{
		if((size = Receive(s, &ready, sizeof(ready))) != 4)
			printf("Size not 4 bytes. Continue anyways.\n");
	}
	
	if(-1 == send(s, &command, sizeof(command), 0))
	{
		printf("Command Send Fail. \n");
		exit(1);
	}	
}

void ConnectSocket()
{
    svr.sin_family = AF_INET;
	svr.sin_port = htons(40000);
	inet_aton("127.0.1.1", &svr.sin_addr);

    s = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == s)
	{
		printf("Socket creation failed. Now exiting. \n");
		exit(1);
	}
	else
		printf("Socket created. \n");

	if(connect(s, (const sockaddr*)&svr, sizeof(sockaddr_in)) == -1)
	{
		printf("Connection failed. Exiting. \n");
		exit(1);
	}
	else
		printf("Connection successful. \n");
}

void *SendDataHelper(void *arg) { return SendData(); }
void *GetDataHelper(void *arg) { return GetData(); }

void Create_pThread(pThread_arg data)
{
	pthread_join(thread, NULL);
	if(data == Send)
	{
		int rc = pthread_create(&thread, NULL, &SendDataHelper, NULL);
		if (rc)
		{
			cout << "Error: unable to create thread," << rc << endl;
			exit(-1);
		}
	}
	else if(data == Get)
	{
		pthread_join(thread, NULL);
		int rc = pthread_create(&thread, NULL, &GetDataHelper, NULL);
		if (rc)
		{
			cout << "Error: unable to create thread," << rc << endl;
			exit(-1);
		}
	}
}

int main(int argc, char** argv)
{
	Distance = 0;
	DirectionOfProjection[0] = 0.0;
	DirectionOfProjection[1] = 0.0;
	DirectionOfProjection[2] = 0.0;
	ViewPlaneNormal[0] = 0;
	ViewPlaneNormal[1] = 0;
	ViewPlaneNormal[2] = 0;
	MaxSize = 0;

	Identity(*transform_Concatenation_Pre_Matrix);
	Identity(*camera_Transform_Concatenation_Pre_Matrix);
	Identity(*camera_ViewTransform_Concatenation_Pre_Matrix);
	Identity(*transform_Matrix);
	Identity(*camera_Transform_Matrix);
	Identity(*camera_ViewTransform_Matrix);
	
    ConnectSocket();


	Create_pThread(Get);

	/*bool done = FALSE;
	char c;
	while(!done){
		cout << "Change Pos(p), FP(f), or VU(v)?: ";
		cin >> c;
		switch(c){
			case 'p':
				cout << "Set Position.\n" << endl;
				cout << "X: ";
				cin >> camera.Position[0];
				cout << "Y: ";
				cin >> camera.Position[1];
				cout << "Z: ";
				cin >> camera.Position[2];
				break;
			case 'f':
				cout << "Set Focal Point.\n" << endl;
				cout << "X: ";
				cin >> camera.FocalPoint[0];
				cout << "Y: ";
				cin >> camera.FocalPoint[1];
				cout << "Z: ";
				cin >> camera.FocalPoint[2];
				break;
			case 'v':
				cout << "Set Position.\n" << endl;
				cout << "X: ";
				cin >> camera.ViewUp[0];
				cout << "Y: ";
				cin >> camera.ViewUp[1];
				cout << "Z: ";
				cin >> camera.ViewUp[2];
				break;
			default:
				break;
		}
		SendData();
		printData();
	}*/
	for(int j = 0; j < 3; j++)
	{
		for(int i = 0; i < 100; i++)
		{
			Rotate(0.0001*i,0.0001*i);
			cout << "Rotate" << i << endl;
			if(i == 99)
			{
				Create_pThread(Get);
				cout << "GetData" << endl;
			}
		}
	}


	close(s);

	return 0;
}

	

	
