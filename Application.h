//
//  Application.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _APPLICATION_H
#define _APPLICATION_H

#include "config.h"
#include "common.h"

// app shell interface - Application.cpp
bool shellInitApplication();					// called by SState after there's been a chance to change the screen
bool shellQuitApplication(bool full_cleanup);	
bool shellUpdateScene(double t);				// send GAMESTATE_MESSAGE_UPDATE to GameDispatcher.game_state_dispatcher()
bool shellRenderScene();						// send GAMESTATE_MESSAGE_RENDER to GameDispatcher.game_state_dispatcher()

bool shellCheckForSavedGame();					// check for saved_game.xml
bool shellPauseGame();							// pause game if applicable
bool shellSaveState();							// save state
void shellReadState();
void shellDeleteState();						// erase state
bool shellRetryDownload();						// retry download if failed


#ifdef PCport
#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D
#define VK_ESCAPE         0x1B
#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F
#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87
#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5
extern bool isKeyDown(int code);
extern bool isKeyRecent(int code);	// also clears the flag
//
#define JK_U             0x0001
#define JK_D             0x0002
#define JK_L             0x0004
#define JK_R             0x0008
#define JK_START         0x0010
#define JK_BACK          0x0020
#define JK_LTHUMB_CLICK  0x0040
#define JK_RTHUMB_CLICK  0x0080
#define JK_LSHOULDER     0x0100
#define JK_RSHOULDER     0x0200
#define JK_A             0x1000
#define JK_B             0x2000
#define JK_X             0x4000
#define JK_Y             0x8000
extern bool isJoystickButtonDown(int code);
extern bool isJoystickButtonRecent(int code);	// also clears the flag
#define JV_LTRIGGER      0
#define JV_RTRIGGER      1
#define JV_THUMB_LX      2
#define JV_THUMB_LY      3
#define JV_THUMB_RX      4
#define JV_THUMB_RY      5
extern float getJoystickValue(int code);
#endif // PCport

#endif // !_APPLICATION_H


