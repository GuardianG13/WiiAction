// File: 		WiiAction.cpp
// Author:		Travis Bueter
// Description:	Class Definition of WiiMote interaction with ParaView

WiiAction::WiiAction()
{
	reloadWiimotes = 0;
	trigger = false;
	mod = 90*exp(-10);
	d_invert = 1;
	z_invert = 1;  
	p_xinvert = 1;
	p_yinvert = 1;
	r_xinvert = 1;
	r_yinvert = 1;
	s_invert = 1;
	toggle = 0;
	d_mod = 1;
	s_mod = 1;
	interactionMods[0] = 1;
	interactionMods[1] = 1;
	interactionMods[2] = 1;
	selectedMod = NONE;
	deadzone = 0.12;
	
	haltSendData = false;
	
	this->Center[1] = 0.0;
	this->Center[2] = 0.0;
	this->Center[3] = 0.0;
	
	this->cam_angle = 0.0;
	
	camState = new WiiCamera;
}

WiiAction::~WiiAction()
{
	camState->Delete();
	close(s);
}

void* WiiAction::SendData()
{
	int command = 4;
	if(receiveReadyCommand(s) && !haltSendData)
	{
		if(-1 == send(s, &command, sizeof(command), 0))
		{
			printf("Command Send Fail. \n");
			exit(1);
		}
		if(-1 == send(s, &camState->camera, sizeof(CameraState), 0))
		{
			printf("Data Send Fail. \n");
			exit(1);
		}
	}
	pthread_exit(NULL);
}

void WiiAction::SocketConnect()
{
	int ready = 0;
	int command = 2;
	int size;
	float cam[10];
	
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
	
	GetData(s);
	
}

void WiiAction::FindWiimote()
{
	cout << "Searching for wiimotes... Turn them on!" << endl;

	//Find the wiimote
	numFound = wii.Find(5);

	// Search for up to five seconds;

	cout << "Found " << numFound << " wiimotes" << endl;
	cout << "Connecting to wiimotes..." << endl;

	// Connect to the wiimote
	wiimotes = wii.Connect();

	cout << "Connected to " << (unsigned int)wiimotes.size() << " wiimote." << endl;

	
	// Setup the wiimotes
	for(index = 0, i = wiimotes.begin(); i != wiimotes.end(); ++i, ++index)
	{
		// Use a reference to make working with the iterator handy.
		CWiimote & wiimote = *i;

		//Set Leds
		wiimote.SetLEDs(LED_MAP[index]);

		//Rumble for 0.2 seconds as a connection ack
		wiimote.SetRumbleMode(CWiimote::ON);
		usleep(200000);
		wiimote.SetRumbleMode(CWiimote::OFF); 
	}
}

void WiiAction::Run()
{
	do
	{
		if(reloadWiimotes)
		{
			wiimotes = wii.GetWiimotes();
			reloadWiimotes = 0;
			trigger = false;
		}
		if(wii.Poll())
		{
			for(i = wiimotes.begin(); i != wiimotes.end(); ++i)
			{
				// Use a reference to make working with the iterator handy.
				CWiimote & wiimote = *i;

				switch(wiimote.GetEvent())
				{
		
					case CWiimote::EVENT_EVENT:
						 HandleEvent(wiimote);
						 break;
		
					case CWiimote::EVENT_STATUS:
						 if(!trigger)
						 {
							 PrintInstructions(wiimote);
							 trigger = true;
						 }
						 
						 break;
		
					case CWiimote::EVENT_DISCONNECT:
					case CWiimote::EVENT_UNEXPECTED_DISCONNECT:
						 HandleDisconnect(wiimote);
						 reloadWiimotes = 1;
						 break;
		
					case CWiimote::EVENT_READ_DATA:
						 HandleReadData(wiimote);
						 break;
		
					case CWiimote::EVENT_NUNCHUK_INSERTED:
						 HandleNunchukInserted(wiimote);
						 reloadWiimotes = 1;
						 break;
		
					case CWiimote::EVENT_NUNCHUK_REMOVED:
					case CWiimote::EVENT_MOTION_PLUS_REMOVED:
						 cout << "An expansion was removed." << endl;
						 HandleStatus(wiimote);
						 reloadWiimotes = 1;
						 break;
		
					default:
						break;
				}
			}
		}
	}while(wiimotes.size());
}

void WiiAction::Pan(CNunchuk &nc)
{
	nc.Joystick.GetPosition(angle,magnitude);
	if(magnitude >= deadzone)
	{
		float x = sin(angle*PI/180);
		float y = cos(angle*PI/180);
		magnitude += .1 - deadzone;
		float multiplier = interactionMods[0]*mod*exp(magnitude)*pow(magnitude+.56,6.5);
		float dx = p_xinvert*multiplier*x;
		float dy = -p_yinvert*multiplier*y;
		this->Pan(dx,dy);
	}
}

void WiiAction::Dolly(CNunchuk &nc)
{
	nc.Joystick.GetPosition(angle, magnitude);
	if(magnitude >= deadzone)
	{
		float y = cos(angle*PI/180);
		magnitude += .1 - deadzone;
		float multiplier = .1*mod*exp(magnitude)*pow(magnitude+.5,5);
		float dy = d_mod*d_invert*80*multiplier*y;
		this->Dolly(dy);

	}
}

void WiiAction::Zoom(CNunchuk &nc)
{
	nc.Joystick.GetPosition(angle, magnitude);
	if(magnitude >= deadzone)
	{
		float y = cos(angle*PI/180);
		magnitude += .1 - deadzone;
		float multiplier = .1*mod*exp(magnitude)*pow(magnitude+.5,5);
		float dy = interactionMods[2]*z_invert*multiplier*y;
		this->Zoom(dy);
	}
}

void WiiAction::Rotate(CNunchuk &nc)
{
	nc.Joystick.GetPosition(angle, magnitude);
	if(magnitude >= deadzone)
	{
		float x = sin(angle*PI/180);
		float y = cos(angle*PI/180);
		magnitude += .1 - deadzone;
		float multiplier = interactionMods[1]*mod*exp(magnitude)*pow(magnitude+.5,5);
		float dx = -r_xinvert*multiplier*x;
		float dy = -r_yinvert*multiplier*y;
		this->Rotate(dx,dy);
	}
}

void WiiAction::Roll(CNunchuk &nc)
{
	nc.Joystick.GetPosition(angle, magnitude);
	if(magnitude >= deadzone)
	{
		float x = sin(angle*PI/180);
		magnitude += .1 - deadzone;
		float multiplier = s_mod*mod*exp(magnitude)*pow(magnitude+.5,5);
		stringstream ss;
		ss.str(std::string());
		ss << "pvroll(" << -s_invert*.5*multiplier*x << ");";
		const std::string temp = ss.str();
		const char* cstr = temp.c_str();
		write(s, cstr, strlen(cstr));
	}
}

void WiiAction::Create_pThread()
{
	int rc = pthread_create(&thread, NULL, &WiiAction::SendHelper, this);
	if (rc)
	{
		cout << "Error: unable to create thread," << rc << endl;
		exit(-1);
	}
}

void WiiAction::HandleEvent(CWiimote &wm)
{
    int exType = wm.ExpansionDevice.GetType();
    if(exType == wm.ExpansionDevice.TYPE_NUNCHUK)
    {
    	float angle, magnitude;
    	CNunchuk &nc = wm.ExpansionDevice.Nunchuk;
    	if(!nc.Buttons.isHeld(CNunchukButtons::BUTTON_Z) && !nc.Buttons.isHeld(CNunchukButtons::BUTTON_C))
		{
			Pan(nc);
		}
    	if(nc.Buttons.isHeld(CNunchukButtons::BUTTON_Z) && !nc.Buttons.isHeld(CNunchukButtons::BUTTON_C))
    	{
    		if(toggle)
    			Dolly(nc);
    		else
    			Zoom(nc);
    		return;
    	}
    	if(nc.Buttons.isHeld(CNunchukButtons::BUTTON_C) && !nc.Buttons.isHeld(CNunchukButtons::BUTTON_Z))
    	{
    		Rotate(nc);
    		return;
    	}
    	if(nc.Buttons.isHeld(CNunchukButtons::BUTTON_C) && nc.Buttons.isHeld(CNunchukButtons::BUTTON_Z))
    	{
    		//Roll(nc);
    		return;
    	}
    }
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_A))
    {
    	PrintInstructions(wm);
    }
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_B))
	{
    	//Purpose of doing this is to allow the user a way to change the render camera
    	//without having to fight the wiimote. When reactivated, wiimote pulls new 
    	//camera data.
		if(!haltSendData)
		{
			//haltSendData = true; //Dont halt yet. Issue with receiving new data from socket.
		}
		else
		{
			GetData(s);
			haltSendData = false;
		}
	}
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_DOWN)) 
	{
		switch(selectedMod)
		{
		case PAN:
			selectedMod = ROTATE;
			break;
		case ROTATE:
			selectedMod = ZOOM;
			break;
		case ZOOM:
			selectedMod = AVIMODE;
			break;
		case AVIMODE:
			selectedMod = NONE;
			break;
		case NONE:
			selectedMod = PAN;
			break;
		default:
			cout << "ERROR: Default case hit, Down" << endl;
			exit(1);
			break;
		}
		InteractionRatesMenu(); // SetInvertion(); Needs alternate button suggest (+)/(-)
	}
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_RIGHT) && (selectedMod != NONE))
    {
    	unsigned int x = selectedMod;
    	if(x == 3)
    	{
    		r_yinvert *= -1;
    		p_yinvert *= -1;
    	}
    	else if(interactionMods[x] < 1)
    	{
    		interactionMods[x] += 0.1;
    	}
    	else if(interactionMods[x] < 10)
    	{
    		interactionMods[x] += 1.0;
    	}
    	InteractionRatesMenu();
    }
	if(wm.Buttons.isJustPressed(CButtons::BUTTON_LEFT) && (selectedMod != NONE)) 
	{
    	unsigned int x = selectedMod;
    	if(x == 3)
    	{
    		r_yinvert *= -1;
    		p_yinvert *= -1;
    	}
    	else if(interactionMods[x] < 0.2) //Weird bug causes interactionMods[x] = 0.2 -= 0.1 => 0.099999... 
    								 //this is a dirty fix
    	{
    		interactionMods[x] = 0.1;
    	}
    	else if(interactionMods[x] <= 1)
    	{
    		interactionMods[x] -= 0.1;
    	}
    	else
    	{
    		interactionMods[x] -= 1.0;
    	}
    	InteractionRatesMenu();
	}
	if(wm.Buttons.isJustPressed(CButtons::BUTTON_UP))
	{
		switch(selectedMod)
		{
		case PAN:
			selectedMod = NONE;
			break;
		case ROTATE:
			selectedMod = PAN;
			break;
		case ZOOM:
			selectedMod = ROTATE;
			break;
		case AVIMODE:
			selectedMod = ZOOM;
			break;
		case NONE:
			selectedMod = AVIMODE;
			break;
		default:
			cout << "ERROR: Default case hit, Up" << endl;
			exit(1);
			break;
		}
		InteractionRatesMenu();
	}
}

void WiiAction::SetInvertion()
{
	char function;
	printf("\n[Pan = p : Dolly = d : Zoom = z : Rotate = r : Spin = s] Enter 'c' to cancel at any time.\n");
	cout << "Which Function to Invert? ";
	cin >> function;
	switch(function)
	{
		case 'P':
		case 'p':
			cout << "\n Pan: X-coord [x], Y-coord [y], or both [b]? ";
			cin >> function;
			switch(function)
			{
				case 'X':
				case 'x':
					p_xinvert *= -1;
					printf("\nPan X-coord inverted.\n");
					break;
				case 'Y':
				case 'y':
					p_yinvert *= -1;
					printf("\nPan Y-coord inverted.\n");
					break;
				case 'B':
				case 'b':
					p_xinvert *= -1;
					p_yinvert *= -1;
					printf("\nPan X-coord and Y-coord inverted;\n");
					break;
				case 'C':
				case 'c':
					printf("\nCancelled.\n");
					break;
				default:
					printf("\nERROR: Invalid input.\n");
					break;
			}
			break;
		case 'D':
		case 'd':
			d_invert *= -1;
			printf("\nDolly Inverted.\n");
			break;
		case 'Z':
		case 'z':
			z_invert *= -1;
			printf("\nZoom Inverted.\n");
			break;
		case 'R':
		case 'r':
			cout << "\nRotate: X-coord [x], Y-coord [y], or both [b]? ";
			cin >> function;
			switch(function)
			{
				case 'X':
				case 'x':
					r_xinvert *= -1;
					printf("\nRotate X-coord inverted.\n");
					break;
				case 'Y':
				case 'y':
					r_yinvert *= -1;
					printf("\nRotate Y-coord inverted.\n");
					break;
				case 'B':
				case 'b':
					r_xinvert *= -1;
					r_yinvert *= -1;
					printf("\nRotate X-coord and Y-coord inverted;\n");
					break;
				case 'C':
				case 'c':
					printf("\nCancelled.\n");
					break;
				default:
					printf("\nERROR: Invalid input.\n");
					break;
			}
			break;
		case 'S':
		case 's':
			s_invert *= -1;
			printf("\nSpin inverted.\n");
			break;
		case 'C':
		case 'c':
			printf("\nCancelled.\n");
			break;
		default:
			printf("\nERROR: Invalid input.\n");
			break;
	}
}

void WiiAction::SetDeadzone()
{
	float value;
	printf("\nDefault Deadzone is '0.12'. Greater value equals greater deadzone and vice versa.\n");
	cout << "Set deadzone to [0.05 <-> 1.1]: ";
	cin >> value;
	if(!(value >= 0.05 && value <= 1.1))
	{
		printf("\nInvalid input.\n");
		return;
	}
	deadzone = value; 
}

void WiiAction::ToggleZoomingMode()
{
	toggle *= -1;
	if(toggle > 0)
	{
		printf("\nDolly mode active.\n");
	}
	else
	{
		printf("\nZoom mode active.\n");
	}
}

void WiiAction::InteractionRatesMenu()
{
	switch(selectedMod)
	{
		case PAN:
			std::system ( "clear" );
			cout << ">Pan rate value:    " << interactionMods[0]; 
			
			if(interactionMods[0] == 10) cout << " MAX" << endl;
			else if(interactionMods[0] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << " Rotate rate value: " << interactionMods[1];
			
			if(interactionMods[1] == 10) cout << " MAX" << endl;
			else if(interactionMods[1] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << " Zoom rate value:   " << interactionMods[2];
			
			if(interactionMods[2] == 10) cout << " MAX" << endl;
			else if(interactionMods[2] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << " Aviation Mode:     "; 
			
			if(r_yinvert == -1 && p_yinvert == -1) cout << "TRUE" << endl; else cout << "FALSE" << endl;
				
			cout << " Close" << endl;
			break;
		case ROTATE:
			std::system ( "clear" );

			cout << " Pan rate value:    " << interactionMods[0]; 
			
			if(interactionMods[0] == 10) cout << " MAX" << endl;
			else if(interactionMods[0] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << ">Rotate rate value: " << interactionMods[1];
			
			if(interactionMods[1] == 10) cout << " MAX" << endl;
			else if(interactionMods[1] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << " Zoom rate value:   " << interactionMods[2];
			
			if(interactionMods[2] == 10) cout << " MAX" << endl;
			else if(interactionMods[2] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << " Aviation Mode:     "; 
			
			if(r_yinvert == -1 && p_yinvert == -1) cout << "TRUE" << endl; else cout << "FALSE" << endl;
				
			cout << " Close" << endl;
			break;
		case ZOOM:
			std::system ( "clear" );
			cout << " Pan rate value:    " << interactionMods[0]; 
			
			if(interactionMods[0] == 10) cout << " MAX" << endl;
			else if(interactionMods[0] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << " Rotate rate value: " << interactionMods[1];
			
			if(interactionMods[1] == 10) cout << " MAX" << endl;
			else if(interactionMods[1] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << ">Zoom rate value:   " << interactionMods[2];
			
			if(interactionMods[2] == 10) cout << " MAX" << endl;
			else if(interactionMods[2] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << " Aviation Mode:     "; 
			
			if(r_yinvert == -1 && p_yinvert == -1) cout << "TRUE" << endl; else cout << "FALSE" << endl;
				
			cout << " Close" << endl;
			break;
		case AVIMODE:
			std::system ( "clear" );
			cout << " Pan rate value:    " << interactionMods[0]; 
			
			if(interactionMods[0] == 10) cout << " MAX" << endl;
			else if(interactionMods[0] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << " Rotate rate value: " << interactionMods[1];
			
			if(interactionMods[1] == 10) cout << " MAX" << endl;
			else if(interactionMods[1] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << " Zoom rate value:   " << interactionMods[2];
			
			if(interactionMods[2] == 10) cout << " MAX" << endl;
			else if(interactionMods[2] == 0.1) cout << " MIN" << endl;
			else cout << endl;
			
			cout << ">Aviation Mode:     "; 
			
			if(r_yinvert == -1 && p_yinvert == -1) cout << "TRUE" << endl; else cout << "FALSE" << endl;
				
			cout << " Close" << endl;
			break;
		case NONE:
			std::system ( "clear" );
			break;
		default:
			cout << "ERROR: Default case hit, InteractionRatesMenu" << endl;
			exit(1);
			break;
	}
}

void WiiAction::HandleStatus(CWiimote &wm)
{
	if(wm.GetBatteryLevel()<.2)
	{
		printf("CAUTION: Battery Level at %f%%\n\n", wm.GetBatteryLevel()*100);
	}
}

void WiiAction::HandleDisconnect(CWiimote &wm)
{
    printf("\n");
    printf("--- DISCONNECTED [wiimote id %i] ---\n", wm.GetID());
    printf("\n");
}

void WiiAction::HandleReadData(CWiimote &wm)
{
    printf("\n");
    printf("--- DATA READ [wiimote id %i] ---\n", wm.GetID());
    printf("\n");
}

void WiiAction::HandleNunchukInserted(CWiimote &wm)
{
    //printf("Nunchuk inserted on controller %i.\n", wm.GetID());
}

void WiiAction::PrintInstructions(CWiimote &wm)
{
	printf("-----------------------------------------------------------------\n");
	printf("Battery Level: %f %%\n\n", wm.GetBatteryLevel());
	printf("NUNCHUK COMMANDS:\n");
	printf("	Joystick w/ no buttons pressed  -  Pan\n");
	printf("	Joystick w/ 'C' button pressed  -  Rotate\n");
	printf("	Joystick w/ 'Z' button pressed  -  Zoom\n");
	printf("	Joystick w/ 'C' & 'Z'  pressed  -  Roll (Not Implemented Yet)\n");
	printf("\nWIIMOTE COMMANDS:\n");
	printf("	D-pad Up     -  Go up Interactor-Modifier menu\n");
	printf("	D-pad Down   -  Go down Interactor-Modifier menu\n");
	printf("	D-pad Left   -  Increase selected modifier value\n");
	printf("	D-pad Right  -  Decrease selected modifier value\n");
	printf("	A Button     -  Re-print Instructions\n");
	printf("-----------------------------------------------------------------\n\n");
}

void WiiAction::Dolly(float dy)
{
	if(camState == NULL)
		return;
	
	float pos[3], fp[3], vu[3], Ld[3];
	camState->GetPosition(pos);
	camState->GetFocalPoint(fp);
	camState->GetViewUp(vu);
	
	Ld[0] = fp[0]-pos[0];
	Ld[1] = fp[1]-pos[1];
	Ld[2] = fp[2]-pos[2];
	
	pos[0] = pos[0]-Ld[0]*dy*0.2;
	pos[1] = pos[1]-Ld[1]*dy*0.2;
	pos[2] = pos[2]-Ld[2]*dy*0.2;
	
	Create_pThread();
}

void WiiAction::Zoom(float dy)
{
	if(camState == NULL)
		return;
	
	float pos[3], fp[3], *norm, k, tmp;
	camState->GetPosition(pos);
	camState->GetFocalPoint(fp);
	norm = camState->GetDirectionOfProjection();
	k = dy * 1000;
	
	tmp = k * norm[0];
	pos[0] += tmp;
	fp[0] += tmp;
  
	tmp = k*norm[1];
	pos[1] += tmp;
	fp[1] += tmp;
  
	tmp = k * norm[2];
	pos[2] += tmp;
	fp[2] += tmp;
	
	camState->SetFocalPoint(fp[0], fp[1], fp[2]);
	camState->SetPosition(pos[0], pos[1], pos[2]);
	
	Create_pThread();
}

void WiiAction::Pan(float dx, float dy)
{
	if(camState == NULL)
		return;
	
	float pos[3], fp[3]; 
	camState->GetPosition(pos);
	camState->GetFocalPoint(fp);
	
	camState->OrthogonalizeViewUp();
	float *up = camState->GetViewUp();
	float *vpn = camState->GetViewPlaneNormal();
	float right[3];
	float scale, tmp;
	camState->GetViewUp(up);
	camState->GetViewPlaneNormal(vpn);
	WiiMath::Cross(vpn, up, right);
	
	scale = camState->GetParallelScale();
	dx *= scale * 2.0;
	dy *= scale * 2.0;
	
	tmp = (right[0]*dx + up[0]*dy);
	pos[0] += tmp;
	fp[0] += tmp; 
	tmp = (right[1]*dx + up[1]*dy); 
	pos[1] += tmp;
	fp[1] += tmp; 
	tmp = (right[2]*dx + up[2]*dy); 
	pos[2] += tmp;
	fp[2] += tmp; 
	camState->SetPosition(pos[0], pos[1], pos[2]);
	camState->SetFocalPoint(fp[0], fp[1], fp[2]);
	
	Create_pThread();
}

void WiiAction::Rotate(float dx, float dy)
{	
	
	if(camState == NULL)
		return;
	
	WiiTransform *transform = new WiiTransform();
	float scale = WiiMath::Norm(camState->GetPosition());
	if(scale<=0.0)
	{
		scale = WiiMath::Norm(camState->GetFocalPoint());
		if(scale<=0.0)
		{
			scale = 1.0;
		}
	}
	
	float* temp = camState->GetFocalPoint();
	camState->SetFocalPoint(temp[0]/scale, temp[1]/scale, temp[2]/scale);
	temp = camState->GetPosition();
	camState->SetPosition(temp[0]/scale, temp[1]/scale, temp[2]/scale);

	float v2[3];
	// translate to center
	transform->Identity();
	transform->Translate(this->Center[0]/scale, this->Center[1]/scale, this->Center[2]/scale);

	//azimuth
	camState->OrthogonalizeViewUp();
	float *viewUp = camState->GetViewUp();
	transform->RotateWXYZ(360.0*dx, viewUp[0], viewUp[1], viewUp[2]);

	//elevation
	WiiMath::Cross(camState->GetDirectionOfProjection(), viewUp, v2);
	transform->RotateWXYZ(-360.0*dy, v2[0], v2[1], v2[2]);

	// translate back
	transform->Translate(-this->Center[0]/scale, -this->Center[1]/scale, -this->Center[2]/scale);
	camState->ApplyTransform(transform);
	camState->OrthogonalizeViewUp();

	temp = camState->GetFocalPoint();
	camState->SetFocalPoint(temp[0]*scale, temp[1]*scale, temp[2]*scale);
	temp = camState->GetPosition();
	camState->SetPosition(temp[0]*scale, temp[1]*scale, temp[2]*scale);
	
	Create_pThread();
	
	transform->Delete();
}

int WiiAction::Receive(const int socket, void* data, int len)
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

bool WiiAction::receiveReadyCommand(int socket)
{
	int command = 0;
	Receive(socket, &command, 4);
	if(command == 1)
		return true;
	else
		return false;
}

void WiiAction::GetData(const int socket)
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
	
	char metadata[length];
	
	if(!Receive(s, &metadata, length))
	{
		printf("MetaData not received.\n");
	}
	
	json_object * jobj = json_tokener_parse((char*)metadata);
	
	json_get_array_values(jobj, "Center", this->Center);
		
	json_get_array_values(jobj, "Renderers", cam);
	
	this->cam_angle = cam[0];
	this->camState->SetFocalPoint(cam[1], cam[2], cam[3]);
	this->camState->SetViewUp(cam[4], cam[5], cam[6]);
	this->camState->SetPosition(cam[7], cam[8], cam[9]);
}


void WiiAction::json_get_array_values( json_object *jobj, char *key, float a[]) {
  enum json_type type;

  json_object *jarray = jobj; /*Simply get the array*/
  if(key) {
    jarray = json_object_object_get(jobj, key); /*Getting the array if it is a key value pair*/
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

void WiiAction::json_parse(json_object * jobj) {
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

void WiiAction::json_parse_array( json_object *jobj, char *key) {
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
      char* check = "LookAt";
      if(key == check)
      {
    	  printf("HERE");
      }
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

void WiiAction::print_json_value(json_object *jobj){
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






