//
//  TTextureFile.cpp
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//


#include "TTextureFile.h"
#include "HAL.h"
#include "XMLParser.h"      // MYSTRCMP()




extern HAL *gHAL;



std::vector<TTextureFile*> gTextureFiles;
// return pointer to existing TTextureFile, or create a new one
//  fname does not contain ".pvr" or path
TTextureFile *TTextureFile::getTextureFile(const char *fname, bool needs_mipmap) {
	unsigned int i;
	for (i=0; i<gTextureFiles.size(); i++) if (MYSTRCMP(gTextureFiles[i]->texturefilename.c_str(), fname)) return gTextureFiles[i];
	gTextureFiles.push_back(new TTextureFile());
	i = (int)gTextureFiles.size()-1;
	gTextureFiles[i]->texturefilename = fname;

	HAL::load_texture_file(fname, &gTextureFiles[i]->textureglname, &gTextureFiles[i]->width, &gTextureFiles[i]->height, needs_mipmap);

	return gTextureFiles[i];
};



TTextureFile *TTextureFile::getARGBFile(const char *fname, int *out_width, int *out_height, bool needs_mipmap) {
	unsigned int i;
	for (i=0; i<gTextureFiles.size(); i++) if (MYSTRCMP(gTextureFiles[i]->texturefilename.c_str(), fname)) return gTextureFiles[i];
	int w, h;
	unsigned int glname;

	glname = HAL::load_argb_file(fname, &w, &h, NULL, needs_mipmap);
	if (glname == 0) return NULL;

	gTextureFiles.push_back(new TTextureFile());
	i = (int)gTextureFiles.size()-1;
	gTextureFiles[i]->texturefilename = fname;
	gTextureFiles[i]->textureglname = glname;
	gTextureFiles[i]->width = w;
	gTextureFiles[i]->height = h;

	return gTextureFiles[i];
};



void TTextureFile::setup_repeat(bool is_repeat) {
	if (is_repeat) gHAL->texture_setup_repeat(textureglname);
    else gHAL->texture_setup_clamp_to_edge(textureglname);
}

