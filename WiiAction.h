// File: 		WiiAction.h
// Author:		Travis Bueter
// Description:	Class Definition of WiiMote interaction with ParaView

#ifndef WIIACTION_H
#define WIIACTION_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiicpp.h>
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
#include <stdint.h>
#include <json.h>

#include "WiiCamera.h"

#ifndef PI
#define PI 3.14159265
#endif

using namespace std;

enum interaction_t { PAN = 0, ROTATE, ZOOM, AVIMODE, NONE };


int LED_MAP[4] = {CWiimote::LED_1, CWiimote::LED_2, CWiimote::LED_3, CWiimote::LED_4};

class WiiAction
{
public:
	WiiAction();
	~WiiAction();
	
	void SocketConnect();
	void PrintInstructions(CWiimote &wm);
	void FindWiimote();
	void Run();
	void HandleEvent(CWiimote &wm);
	void SetInvertion();
	void SetDeadzone();
	void ToggleZoomingMode();
	void InteractionRatesMenu();
	void Pan(CNunchuk& nc);
	void Dolly(CNunchuk& nc);
	void Zoom(CNunchuk& nc);
	void Rotate(CNunchuk& nc);
	void Roll(CNunchuk& nc);
	void HandleStatus(CWiimote& wm);
	void HandleDisconnect(CWiimote& wm);
	void HandleReadData(CWiimote& wm);
	void HandleNunchukInserted(CWiimote& wm);
	void Create_pThread();
	
	//Socket fuctions
	bool receiveReadyCommand(int socket);
    int  Receive(const int socket, void* data, int len);
	void GetData(const int socket);
	void* SendData();
	static void *SendHelper(void *arg) { return ((WiiAction *)arg)->SendData(); }
	
	//Manipulation Functions
	void Rotate(float dx, float dy);
	void Pan(float dx, float dy);
	void Roll(float dx, float dy);
	void Zoom(float dy);
	void Dolly(float dy);		
	
	//JSON Functions
	void json_get_array_values(json_object *jobj, char *key, float a[]);
	void json_parse(json_object * jobj);
	void json_parse_array( json_object *jobj, char *key);
	void print_json_value(json_object *jobj);
	
private:
	/// Wii Dependents ///
	CWii wii;
	std::vector<CWiimote> wiimotes;
	std::vector<CWiimote>::iterator i;
	int reloadWiimotes;
	int numFound;
	int index;
	bool render;
	
	/// Socket Dependents ///
	int s;
	sockaddr_in svr;
	pthread_t thread;
	
	/// Interaction Dependents ///
	bool trigger;		//Janky way of preventing the instructions from printing twice upon activation.
	float mod;			//Modifier for all functions that allow for exponential manipulation.
	float magnitude;
	float angle;
	
	int d_invert;		//Variable that inverts the default Dolly controls when negative.
	int z_invert;		//Variable that inverts the default Zoom controls when negative.
	int p_xinvert;		//Variable that inverts the default Pan x-axis controls when negative.
	int p_yinvert;		//Variable that inverts the default Pan y-axis controls when negative.
	int r_xinvert;		//Variable that inverts the default Rotate x-axis controls when negative.
	int r_yinvert;		//Variable that inverts the default Rotate y-axis controls when negative.
	int s_invert;		//Variable that inverts the default Spin controls when negative.
	
	int toggle;			//Variable that toggles between Dolly/Zoom functions when positive/negative respectively.
	
					// For all Modifiers: Values must be positive. 
					// Value > 1: Magnifies rate; 1 > Value > 0: Miniturizes Rate.
	interaction_t selectedMod;
	float interactionMods[3]; //Contains modifiers for  Pan, Rotate, and Zoom, respectively.
							   //Dolly and Spin not included yet.
	float d_mod;		//Modifier that changes the rate of the Dolly function.
	float z_mod;		//Modifier that changes the rate of the Zoom function.
	float s_mod;		//Modifier that changes the rate of the Spin function.
	
	float deadzone;		//Modifies the dead zone of the joystick. Value must be between 0.05 & 1.1.
	
	///Manipulation Dependents ///
	WiiCamera *camState;
	float cam_angle;
	float Center[3];
	bool haltSendData;
	
};

#include "WiiAction.cxx"

#endif // WiiAction.h
