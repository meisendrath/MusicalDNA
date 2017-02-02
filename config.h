//
//  config.h
//
//  Created by Kimber Leigh Whyte
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _SCONFIG_H
#define _SCONFIG_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#if (TARGET_IPHONE_SIMULATOR == 0) && (TARGET_OS_IPHONE == 1)
#define USE_TEXTURE2D				 // must use Texture2D on the device, PNG files are too mangled by XCode...
#else
#endif
#endif

#define USE_TEXTURE2D
#endif // !_SCONFIG_H


