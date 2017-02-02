//
//  TPODFile.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _TATLAS_H_
#define _TATLAS_H_

#include "config.h"
#include "common.h"

#include <string>
#include <vector>

#include <stdlib.h>

class TAtlas {
public:
							TAtlas() { atlasdat = NULL; atlasdatsize = 0; }
	virtual					~TAtlas() { if (atlasdat != NULL) free(atlasdat); }
private:
	int						atlaswidth, atlasheight;
	unsigned int			atlasname;
	unsigned char			*atlasdat;
	unsigned int			atlasdatsize;
	bool					pendingLoading;
	std::string				pendingFilename;
public:
	static TAtlas			*get_cached_or_load(const char *fname, bool deferredLoading=false);
    static std::vector<TAtlas *>	atlas_cache;
    static std::vector<std::string>	atlas_name_cache;
public:
	void					load_atlas(const char *fname, bool deferredLoading=false);
//	void					load_atlas_now(const char *fname);
	void					load_atlas_now(std::string &fname);
	void					get_atlas_uvs(float *uvs, int frame, int subframe=0);
	void					get_atlas_subframe_bounds(float *bds, int frame, int subframe=0);
	void					draw_atlas_texture(int frame, float x, float y, float r=1, float g=1, float b=1, float a=1);			// round and truncate x and y
	void					draw_atlas_texture_c(int frame, float x, float y, float r=1, float g=1, float b=1, float a=1, int rendermode=0);
	void					draw_atlas_texture_c_no_dblt(int frame, float x, float y, float r=1, float g=1, float b=1, float a=1, int rendermode=0);
	void					draw_atlas_texture_f(int frame, float x, float y, float r=1, float g=1, float b=1, float a=1, int rendermode=0);
	void					draw_atlas_texture_f_additive(int frame, float x, float y, float r=1, float g=1, float b=1, float a=1);
	void					draw_atlas_sequence_center(int seq, int seqframe, float x, float y, float r=1, float g=1, float b=1, float a=1, int rendermode=0);	// keyed by numbers from convert_atlas
	void					draw_atlas_sequence_f(int seq, int seqframe, float x, float y, float r=1, float g=1, float b=1, float a=1);	// keyed by numbers from convert_atlas
public:
// draw a frame, centering according to hjust, vjust, compensating for image's scale of construction
//  hjust/vjust: 0 for left-justified, 0.5 for centered, 1.0 for right-justified
//  if image is constructed for 1024x768 (iPad) of scale 2.1333, but then drawn on a iPhone/retina 960/640, scale by 2/2.1333 to draw
//	float iscl = gDeviceScale / innateScale;
	void					draw_scaled_HUD_item(int frame, float x, float y, float iscl, float hjust, float vjust, float r=1, float g=1, float b=1, float a=1);
	void					draw_scaled_HUD_item_additive(int frame, float x, float y, float iscl, float hjust, float vjust, float r=1, float g=1, float b=1, float a=1);
public:
	void					draw_atlas_texture_9slice(int frame, float x, float y, float Ax, float dBx, float Cx, float Ay, float dBy, float Cy, int mode=0, float r=1, float g=1, float b=1, float a=1);		// A&C regions do not stretch; dBx==dBy==0 is normal render
public:
	int						get_atlas_num_frames();
	const char				*get_atlas_original_file_name(int frame);
	int						get_atlas_frame_number_for_file_name(const char *filename, bool quiet=false);
	int						get_atlas_num_frames_for_sequence(int seq);
	int						get_atlas_first_frame_for_sequence(int seq);
	int						get_atlas_sequence_for_frame(int frame);
	int						get_atlas_frame_width(int frame);
	int						get_atlas_frame_height(int frame);
	int						get_atlas_sequence_frame_width(int seq, int frame);
	int						get_atlas_sequence_frame_height(int seq, int frame);
	unsigned int			get_atlas_glname();
};

#endif // !_TATLAS_H_
