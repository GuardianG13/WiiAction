# ---CMakeLists.txt---
# Project: WiiAction
# Author:  Travis Bueter

cmake_minimum_required (VERSION 2.6)

project (WiiAction)

add_executable (WiiAction WiiMain.cpp)

INCLUDE_DIRECTORIES (/usr/local/include/wiic
	/usr/local/Trolltech/Qt-4.8.3/include)

target_link_libraries (WiiAction -lwiicpp)
