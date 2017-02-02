//
//  TTextureFile.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _TTEXTUREFILE_H
#define _TTEXTUREFILE_H

#include "config.h"
#include "common.h"

#include <vector>
#include <string>

class TTextureFile {
public:
	TTextureFile() { textureglname = 0; }
	virtual ~TTextureFile() { }
public:
	static TTextureFile *getTextureFile(const char *fname, bool needs_mipmap=false);
	static TTextureFile *getARGBFile(const char *fname, int *out_width, int *out_height, bool needs_mipmap=false);
    virtual void		setup_repeat(bool is_repeat);
public:
	std::string	texturefilename;
	unsigned int textureglname;
	int			width, height;
};

#endif // !_TTEXTUREFILE_H


