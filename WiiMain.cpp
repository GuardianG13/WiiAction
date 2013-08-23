// File: 		WiiMain.cpp
// Author:		Travis Bueter
// Description:	Application of WiiMote interaction with ParaView

#include "WiiAction.h"

using namespace std;

int main(int argc, char** argv)
{
    WiiAction wii;
    wii.SocketConnect();
    wii.FindWiimote();
    wii.Run();
    return 0;
}