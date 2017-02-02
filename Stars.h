//
//  Stars.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _STARS_H
#define _STARS_H

#include "config.h"
#include "common.h"

void stars_init();
void stars_draw(TMatrix *viewMatrix, float a);
void stars_exit();
void stars_clean(TMatrix *viewMatrix);

#endif // !_STARS_H


