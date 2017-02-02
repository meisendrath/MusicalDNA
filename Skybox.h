//
//  Skybox.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _SKYBOX_H
#define _SKYBOX_H

#include "config.h"
#include "common.h"

void init_skybox();					// first time
void init_a_skybox(int level, bool firsttime=false, int which=-1);		// each additional time
void DrawSkybox(float skybox_a, TVector3 &viewpoint, bool flashing);

#endif // !_SKYBOX_H
