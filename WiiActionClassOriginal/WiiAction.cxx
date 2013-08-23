// File: 		WiiAction.cpp
// Author:		Travis Bueter
// Description:	Class Definition of WiiMote interaction with ParaView

WiiAction::WiiAction()
{
	reloadWiimotes = 0;
	render = false;
	trigger = false;
	mod = 9000*exp(-10);
	d_invert = 1;
	z_invert = 1;  
	p_xinvert = 1;
	p_yinvert = 1;
	r_xinvert = 1;
	r_yinvert = 1;
	s_invert = 1;
	toggle = 1;
	d_mod = 1;
	z_mod = 1;
	p_mod = 1;
	r_mod = 1;
	s_mod = 1;
	deadzone = 0.12;
}

WiiAction::~WiiAction()
{
	
}

void WiiAction::SocketConnect()
{
	svr.sin_family = AF_INET;
	svr.sin_port = htons(9000);
	inet_aton("127.0.0.1", &svr.sin_addr);

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

	if(-1 == write(s, "execfile('init.py');", sizeof("execfile('init.py');")-1))
	{
		printf("File failed. Exiting.");
		exit(1);
	}
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
		while(wii.Poll())
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
		float multiplier = p_mod*mod*exp(magnitude)*pow(magnitude+.56,6.5);
		stringstream ss;
		ss.str(std::string());
		ss << "pvpan(" << p_xinvert*multiplier*x << "," << p_yinvert*multiplier*y << ");";
		const std::string temp = ss.str();
		const char* cstr = temp.c_str();
		write(s, cstr, strlen(cstr));
		if(!render)
			render = true;
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
		stringstream ss;
		ss.str(std::string());
		ss << "pvdolly(" << d_mod*d_invert*80*multiplier*y << ");";
		const std::string temp = ss.str();
		const char* cstr = temp.c_str();
		write(s, cstr, strlen(cstr));
		if(!render)
			render = true;
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
		stringstream ss;
		ss.str(std::string());
		ss << "pvzoom(" << z_mod*z_invert*.1*multiplier*y << ");";
		const std::string temp = ss.str();
		const char* cstr = temp.c_str();
		write(s, cstr, strlen(cstr));
		if(!render)
			render = true;
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
		float multiplier = r_mod*1.5*mod*exp(magnitude)*pow(magnitude+.5,5);
		stringstream ss;
		ss.str(std::string());
		ss << "pvrotate(" << -r_xinvert*multiplier*x << "," << -r_yinvert*multiplier*y << ");";
		const std::string temp = ss.str();
		const char* cstr = temp.c_str();
		write(s, cstr, strlen(cstr));
		if(!render)
			render = true;
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
		if(!render)
			render = true;
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
    		if(toggle > 0)
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
    		Roll(nc);
    		return;
    	}
    	if(render)
    	{
    		Render();
    		render = false;
    	}
    }
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_A))
    {
    	PrintInstructions(wm);
    }
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_DOWN)) // INVERT
	{
    	SetInvertion();
	}
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_LEFT)) // Adjust Render Rate
    {
    	SetDeadzone();
    }
	if(wm.Buttons.isJustPressed(CButtons::BUTTON_RIGHT)) // Toggle Dolly/Zoom
	{
		ToggleZoomingMode();
	}
	if(wm.Buttons.isJustPressed(CButtons::BUTTON_UP))
	{
		SetInteractionRates();
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

void WiiAction::SetInteractionRates()
{
	char function;
	float value;
	printf("\n[Pan = p : Dolly = d : Zoom = z : Rotate = r : Spin = s] Enter 'c' to cancel.\n");
	cout << "Which function's rate to modify? ";
	cin >> function;
	if(function == 'c')
	{
		printf("\nCancelled.\n");
		return;
	}
	printf("\nValue must be positive. Value > 1 will magnify rate; Value < 1 will miniaturize rate. \nEntering \'1\' set rate to original value. Enter \'0\' to cancel.\n");
	cout << "Input modifying value: ";
	cin >> value;
	if(value < 0)
	{
		function = ';';
	}
	else if(value == 0)
	{
		function = 'c';
	}
	switch(function)
	{
		case 'P':
		case 'p':
			p_mod = value;
			printf("\nPan rate modified.\n");
			break;
		case 'D':
		case 'd':
			d_mod = value;
			printf("\nDolly rate modified.\n");
			break;
		case 'Z':
		case 'z':
			z_mod = value;
			printf("\nZoom rate modified.\n");
			break;
		case 'R':
		case 'r':
			printf("\nRotate rate modified.\n");
			r_mod = value;
			break;
		case 'S':
		case 's':
			s_mod = value;
			printf("\nSpin rate modified.\n");
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

void WiiAction::Render()
{
	write(s, "Render();", sizeof("Render();")-1);
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
	printf("	Joystick w/ 'Z' button pressed  -  Dolly(default)/Zoom\n");
	printf("	Joystick w/ 'C' & 'Z'  pressed  -  Roll\n");
	printf("\nWIIMOTE COMMANDS:\n");
	printf("	D-pad Up     -  Modifier Prompt\n");
	printf("	D-pad Down   -  Invert Prompt\n");
	printf("	D-pad Left   -  Set Render Rate and Joystick Deadzone\n");
	printf("	D-pad Right  -  Toggle Dolly/Zoom mode\n");
	printf("	A Button     -  Re-print Instructions\n");
	printf("-----------------------------------------------------------------\n\n");
}
