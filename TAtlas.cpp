#include "TAtlas.h"

//#include "stdio.h"
//#include "stdlib.h"
//#include "string.h"

#include "HAL.h"

extern HAL *gHAL;

/*
+------------+------------+------------+------------+
|          0 |            |         nframes         |
+------------+------------+------------+------------+
	+------------+------------+------------+------------+------------+------------+------------+------------+
	|         offset          |       nsubframes        |          origw          |          origh          |				// offset is from start of file to first subframe for this frame
	+------------+------------+------------+------------+------------+------------+------------+------------+
	|         offset          |       nsubframes        |          origw          |          origh          |
	+------------+------------+------------+------------+------------+------------+------------+------------+
	|         offset          |       nsubframes        |          origw          |          origh          |

	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |

// ALTERNATIVE LAYOUT: ("seq ref num" is intended to hold a sequence id; "frame in seq" places the frame somewhere in the sequence)

+------------+------------+------------+------------+
|          1 |            |         nframes         |
+------------+------------+------------+------------+
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|         offset          |       nsubframes        |          origw          |          origh          |       seq ref num       |      frame in seq       |				// offset is from start of file to first subframe for this frame
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|         offset          |       nsubframes        |          origw          |          origh          |       seq ref num       |      frame in seq       |
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|         offset          |       nsubframes        |          origw          |          origh          |       seq ref num       |      frame in seq       |

	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |
	+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+
	|            x            |            y            |            w            |            h            |           dx            |           dy            |
*/


TAtlas *TAtlas::get_cached_or_load(const char *fname, bool deferredLoading) {
	for (int i=0; i<atlas_cache.size(); i++) if (atlas_name_cache[i].compare(fname) == 0) return atlas_cache[i];
    TAtlas *a = new TAtlas;
    a->load_atlas(fname, deferredLoading);
	if (a->atlasdat == NULL) {
		delete a;
		return NULL;
	}
    atlas_cache.push_back(a);
    atlas_name_cache.push_back(fname);
    return a;
}
std::vector<TAtlas *> TAtlas::atlas_cache;
std::vector<std::string> TAtlas::atlas_name_cache;

////

void TAtlas::load_atlas(const char *fname, bool deferredLoading) {
//printf("load_atlas(%s) %s\n", fname, deferredLoading?"deferred":"");
	pendingLoading = deferredLoading;
//	sprintf(pendingFilename, "%s", fname);
	pendingFilename = fname;
	if (deferredLoading) return;
	load_atlas_now(pendingFilename);
}

//void TAtlas::load_atlas_now(const char *fname)
void TAtlas::load_atlas_now(std::string &fname)
{
//printf("load_atlas_now(%s)\n", fname);
	HAL::load_atlas_files(fname.c_str(), &atlaswidth, &atlasheight, &atlasname, &atlasdatsize, &atlasdat);
	pendingLoading = false;
}

void TAtlas::get_atlas_uvs(float *uvs, int frame, int subframe) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	int nframes = *(unsigned short *)(atlasdat+2);
//printf("draw_atlas_texture(): nframes = %d\n", nframes);
	if (frame >= nframes) return;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	if (*atlasdat & 1) rp += 6 * frame; else rp += 4 * frame;			// index to correct frame
	int offs = *rp++;
//	int nsubframes = *rp++;
//	int origw = *rp++;
//	int origh = *rp++;
	rp = (unsigned short *)(atlasdat + offs);
	rp += 6 * subframe;
	int fx = *rp++;		// location of subimage in file
	int fy = *rp++;
	int fw = *rp++;
	int fh = *rp++;
//	int dx = *(signed short *)rp++;		// offset from x,y to where to draw above subimage
//	int dy = *(signed short *)rp++;
	float		width = (float)atlaswidth,
				height = (float)atlasheight;
	uvs[0] = fx/width;
	uvs[1] = fy/height;
	uvs[2] = fx/width;
	uvs[3] = (fy+fh)/height;
	uvs[4] = (fx+fw)/width;
	uvs[5] = fy/height;
	uvs[6] = (fx+fw)/width;
	uvs[7] = (fy+fh)/height;
}

void TAtlas::get_atlas_subframe_bounds(float *bds, int frame, int subframe) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	int nframes = *(unsigned short *)(atlasdat+2);
//printf("draw_atlas_texture(): nframes = %d\n", nframes);
	if (frame >= nframes) return;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	if (*atlasdat & 1) rp += 6 * frame; else rp += 4 * frame;			// index to correct frame
	int offs = *rp++;
//	int nsubframes = *rp++;
//	int origw = *rp++;
//	int origh = *rp++;
	rp = (unsigned short *)(atlasdat + offs);
	rp += 6 * subframe;
rp++;//	int fx = *rp++;		// location of subimage in file
rp++;//	int fy = *rp++;
	int fw = *rp++;
	int fh = *rp++;
	int dx = *(signed short *)rp++;		// offset from x,y to where to draw above subimage
	int dy = *(signed short *)rp++;
	bds[0] = dx;
    bds[1] = dy;
    bds[2] = fw;
    bds[3] = fh;
}

void TAtlas::draw_atlas_texture_c(int frame, float x, float y, float r, float g, float b, float a, int rendermode) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	int nframes = *(unsigned short *)(atlasdat+2);
//printf("draw_atlas_texture(): nframes = %d\n", nframes);
	if (frame >= nframes) return;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	if (*atlasdat & 1) rp += 6 * frame; else rp += 4 * frame;			// index to correct frame
	int offs = *rp++;
	int nsubframes = *rp++;
	int origw = *rp++;
	int origh = *rp++;
	rp = (unsigned short *)(atlasdat + offs);
//printf("  f%d: offs = %d, nsubframes=%d, atxy=%g,%g, origwh=%d,%d\n", frame, offs, nsubframes, x, y, origw, origh);
	for (int sfi=0; sfi<nsubframes; sfi++) {
		int fx = *rp++;		// location of subimage in file
		int fy = *rp++;
		int fw = *rp++;
		int fh = *rp++;
		int dx = *(signed short *)rp++;		// offset from x,y to where to draw above subimage
		int dy = *(signed short *)rp++;
//printf("      %d: %d, %d, %d, %d, %d, %d\n", sfi, fx, fy, fw, fh, dx, dy);
		float		width = (float)atlaswidth,
					height = (float)atlasheight;
		x += dx - origw/2;
		y += dy - origh/2;
#ifdef never
		float		coordinates[] = {
									fx/width,		fy/height,
									fx/width,		(fy+fh)/height,
									(fx+fw)/width,	fy/height,
									(fx+fw)/width,	(fy+fh)/height,
									};
		float		vertices[] = {	x,		y,		100.0,
									x,		y+fh,	100.0,
									x+fw,	y,		100.0,
									x+fw,	y+fh,	100.0 };

		gHAL->set_mode(rendermode, r, g, b, a);		// colorizing draw_atlas_texture_c
		gHAL->texture_bind(0, atlasname);
		gHAL->blt_rect(vertices, coordinates, 4);
#else // never
		gHAL->dblt_rect(x, y, fw, fh, fx/width, fy/height, fw/width, fh/height, atlasname, rendermode, r, g, b, a);
#endif // never

//		glBindTexture(GL_TEXTURE_2D, atlasname);
//		glVertexPointer(3, GL_FLOAT, 0, vertices);
//		glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
//		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void TAtlas::draw_atlas_texture_c_no_dblt(int frame, float x, float y, float r, float g, float b, float a, int rendermode) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	int nframes = *(unsigned short *)(atlasdat+2);
//printf("draw_atlas_texture(): nframes = %d\n", nframes);
	if (frame >= nframes) return;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	if (*atlasdat & 1) rp += 6 * frame; else rp += 4 * frame;			// index to correct frame
	int offs = *rp++;
	int nsubframes = *rp++;
	int origw = *rp++;
	int origh = *rp++;
	rp = (unsigned short *)(atlasdat + offs);
//printf("  f%d: offs = %d, nsubframes=%d, atxy=%g,%g, origwh=%d,%d\n", frame, offs, nsubframes, x, y, origw, origh);
	for (int sfi=0; sfi<nsubframes; sfi++) {
		int fx = *rp++;		// location of subimage in file
		int fy = *rp++;
		int fw = *rp++;
		int fh = *rp++;
		int dx = *(signed short *)rp++;		// offset from x,y to where to draw above subimage
		int dy = *(signed short *)rp++;
//printf("      %d: %d, %d, %d, %d, %d, %d\n", sfi, fx, fy, fw, fh, dx, dy);
		float		width = (float)atlaswidth,
					height = (float)atlasheight;
		x += dx - origw/2;
		y += dy - origh/2;
#ifndef never
		float		coordinates[] = {
									fx/width,		fy/height,
									fx/width,		(fy+fh)/height,
									(fx+fw)/width,	fy/height,
									(fx+fw)/width,	(fy+fh)/height,
									};
		float		vertices[] = {	x,		y,		100.0,
									x,		y+fh,	100.0,
									x+fw,	y,		100.0,
									x+fw,	y+fh,	100.0 };

		gHAL->set_mode(rendermode, r, g, b, a);		// colorizing draw_atlas_texture_c
		gHAL->texture_bind(0, atlasname);
		gHAL->blt_rect(vertices, coordinates, 4);
#else // never
		gHAL->dblt_rect(x, y, fw, fh, fx/width, fy/height, fw/width, fh/height, atlasname, rendermode, r, g, b, a);
#endif // never

//		glBindTexture(GL_TEXTURE_2D, atlasname);
//		glVertexPointer(3, GL_FLOAT, 0, vertices);
//		glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
//		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void TAtlas::draw_atlas_texture_f(int frame, float x, float y, float r, float g, float b, float a, int rendermode) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	int nframes = *(unsigned short *)(atlasdat+2);
//printf("draw_atlas_texture(): nframes = %d\n", nframes);
	if (frame >= nframes) return;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	if (*atlasdat & 1) rp += 6 * frame; else rp += 4 * frame;			// index to correct frame
	int offs = *rp++;
	int nsubframes = *rp++;
//	int origw = *rp++;
//	int origh = *rp++;
	rp = (unsigned short *)(atlasdat + offs);
//printf("  f%d: offs = %d, nsubframes=%d, atxy=%g,%g, origwh=%d,%d\n", frame, offs, nsubframes, x, y, origw, origh);
	for (int sfi=0; sfi<nsubframes; sfi++) {
		int fx = *rp++;		// location of subimage in file
		int fy = *rp++;
		int fw = *rp++;
		int fh = *rp++;
		int dx = *(signed short *)rp++;		// offset from x,y to where to draw above subimage
		int dy = *(signed short *)rp++;
//printf("      %d: %d, %d, %d, %d, %d, %d\n", sfi, fx, fy, fw, fh, dx, dy);
		float		width = (float)atlaswidth,
					height = (float)atlasheight;
#ifdef never
		float		coordinates[] = {
									fx/width,		fy/height,
									fx/width,		(fy+fh)/height,
									(fx+fw)/width,	fy/height,
									(fx+fw)/width,	(fy+fh)/height,
									};
		float		vertices[] = {	x+dx,		y+dy,		100.0,
									x+dx,		y+dy+fh,	100.0,
									x+dx+fw,	y+dy,		100.0,
									x+dx+fw,	y+dy+fh,	100.0 };

		gHAL->set_mode(rendermode, r, g, b, a);		// colorizing draw_atlas_texture_f
		gHAL->texture_bind(0, atlasname);
		gHAL->blt_rect(vertices, coordinates, 4);

#else // never
		gHAL->dblt_rect(x+dx, y+dy, fw, fh, fx/width, fy/height, fw/width, fh/height, atlasname, rendermode, r, g, b, a);
#endif // never

//		glBindTexture(GL_TEXTURE_2D, atlasname);
//		glVertexPointer(3, GL_FLOAT, 0, vertices);
//		glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
//		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void TAtlas::draw_atlas_texture_f_additive(int frame, float x, float y, float r, float g, float b, float a) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	int nframes = *(unsigned short *)(atlasdat+2);
//printf("draw_atlas_texture(): nframes = %d\n", nframes);
	if (frame >= nframes) return;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	if (*atlasdat & 1) rp += 6 * frame; else rp += 4 * frame;			// index to correct frame
	int offs = *rp++;
	int nsubframes = *rp++;
//	int origw = *rp++;
//	int origh = *rp++;
	rp = (unsigned short *)(atlasdat + offs);
//printf("  f%d: offs = %d, nsubframes=%d, atxy=%g,%g, origwh=%d,%d\n", frame, offs, nsubframes, x, y, origw, origh);
	for (int sfi=0; sfi<nsubframes; sfi++) {
		int fx = *rp++;		// location of subimage in file
		int fy = *rp++;
		int fw = *rp++;
		int fh = *rp++;
		int dx = *(signed short *)rp++;		// offset from x,y to where to draw above subimage
		int dy = *(signed short *)rp++;
//printf("      %d: %d, %d, %d, %d, %d, %d\n", sfi, fx, fy, fw, fh, dx, dy);
		float		width = (float)atlaswidth,
					height = (float)atlasheight;
#ifdef never
		float		coordinates[] = {
									fx/width,		fy/height,
									fx/width,		(fy+fh)/height,
									(fx+fw)/width,	fy/height,
									(fx+fw)/width,	(fy+fh)/height,
									};
		float		vertices[] = {	x+dx,		y+dy,		100.0,
									x+dx,		y+dy+fh,	100.0,
									x+dx+fw,	y+dy,		100.0,
									x+dx+fw,	y+dy+fh,	100.0 };

		gHAL->set_mode(4, r, g, b, a);		// colorizing draw_atlas_texture_f, additive
		gHAL->texture_bind(4, atlasname);
		gHAL->blt_rect(vertices, coordinates, 4);
#else // never
		gHAL->dblt_rect(x+dx, y+dy, fw, fh, fx/width, fy/height, fw/width, fh/height, atlasname, 4, r, g, b, a);
#endif // never

//		glBindTexture(GL_TEXTURE_2D, atlasname);
//		glVertexPointer(3, GL_FLOAT, 0, vertices);
//		glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
//		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void TAtlas::draw_atlas_texture_9slice(int frame, float x, float y, float Ax, float dBx, float Cx, float Ay, float dBy, float Cy, int rendermode, float r, float g, float b, float a) {		// A&C regions do not stretch; dBx==dBy==0 is normal render
	if (pendingLoading) load_atlas_now(pendingFilename);
	int nframes = *(unsigned short *)(atlasdat+2);
//printf("draw_atlas_texture(): nframes = %d\n", nframes);
	if (frame >= nframes) return;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	if (*atlasdat & 1) rp += 6 * frame; else rp += 4 * frame;			// index to correct frame
	int offs = *rp++;
	int nsubframes = *rp++;
//	int origw = *rp++;
//	int origh = *rp++;
	rp = (unsigned short *)(atlasdat + offs);
//printf("  f%d: offs = %d, nsubframes=%d\n", frame, offs, nsubframes);
	for (int sfi=0; sfi<nsubframes; sfi++) {
		int fx = *rp++;		// location of subimage in file
		int fy = *rp++;
		int fw = *rp++;
		int fh = *rp++;
		int dx = *(signed short *)rp++;		// offset from x,y to where to draw above subimage
		int dy = *(signed short *)rp++;
//printf("      %d: %d, %d, %d, %d, %d, %d\n", sfi, fx, fy, fw, fh, dx, dy);
		float		width = (float)atlaswidth,
					height = (float)atlasheight;
		x += dx;
		y += dy;
#ifdef never
		float		coordinates[] = {
									fx/width,			fy/height,
									fx/width,			(Ay+fy)/height,
									(Ax+fx)/width,		fy/height,
									(Ax+fx)/width,		(Ay+fy)/height,
									(fw-Cx+fx)/width,	fy/height,
									(fw-Cx+fx)/width,	(Ay+fy)/height,
									(fw+fx)/width,		fy/height,
									(fw+fx)/width,		(Ay+fy)/height,

									fx/width,			(Ay+fy)/height,
									fx/width,			(fh-Cy+fy)/height,
									(Ax+fx)/width,		(Ay+fy)/height,
									(Ax+fx)/width,		(fh-Cy+fy)/height,
									(fw-Cx+fx)/width,	(Ay+fy)/height,
									(fw-Cx+fx)/width,	(fh-Cy+fy)/height,
									(fw+fx)/width,		(Ay+fy)/height,
									(fw+fx)/width,		(fh-Cy+fy)/height,

									fx/width,			(fh-Cy+fy)/height,
									fx/width,			(fh+fy)/height,
									(Ax+fx)/width,		(fh-Cy+fy)/height,
									(Ax+fx)/width,		(fh+fy)/height,
									(fw-Cx+fx)/width,	(fh-Cy+fy)/height,
									(fw-Cx+fx)/width,	(fh+fy)/height,
									(fw+fx)/width,		(fh-Cy+fy)/height,
									(fw+fx)/width,		(fh+fy)/height,
									};
		float		vertices[] = {
									x,					y,				100.0,
									x,					y+Ay,			100.0,
									x+Ax,				y,				100.0,
									x+Ax,				y+Ay,			100.0,
									x+fw+dBx-Cx,		y,				100.0,
									x+fw+dBx-Cx,		y+Ay,			100.0,
									x+fw+dBx,			y,				100.0,
									x+fw+dBx,			y+Ay,			100.0,

									x,					y+Ay,			100.0,
									x,					y+fh+dBy-Cy,	100.0,
									x+Ax,				y+Ay,			100.0,
									x+Ax,				y+fh+dBy-Cy,	100.0,
									x+fw+dBx-Cx,		y+Ay,			100.0,
									x+fw+dBx-Cx,		y+fh+dBy-Cy,	100.0,
									x+fw+dBx,			y+Ay,			100.0,
									x+fw+dBx,			y+fh+dBy-Cy,	100.0,

									x,					y+fh+dBy-Cy,	100.0,
									x,					y+fh+dBy,		100.0,
									x+Ax,				y+fh+dBy-Cy,	100.0,
									x+Ax,				y+fh+dBy,		100.0,
									x+fw+dBx-Cx,		y+fh+dBy-Cy,	100.0,
									x+fw+dBx-Cx,		y+fh+dBy,		100.0,
									x+fw+dBx,			y+fh+dBy-Cy,	100.0,
									x+fw+dBx,			y+fh+dBy,		100.0,
								  };
		
		gHAL->set_mode(rendermode, r, g, b, a);		// colorizing 9 slice
		gHAL->texture_bind(0, atlasname);
		gHAL->dblt_9slice(vertices, coordinates);
#else
// define this to use dblt_rect_start() and dblt_rect_next()
//#define COMBINE_CHARS
#ifdef COMBINE_CHARS
		gHAL->dblt_rect_start(atlasname, rendermode, r, g, b, a);
        if (dBy == 0) {	// Ay + dBy + Cy == fh
            gHAL->dblt_rect_next(x, y, Ax, fh, fx/width, fy/height, Ax/width, fh/height);
            gHAL->dblt_rect_next(x+Ax, y, fw+dBx-Cx-Ax, fh, (Ax+fx)/width, fy/height, (fw-Cx-Ax)/width, fh/height);
            gHAL->dblt_rect_next(x+fw+dBx-Cx, y, Cx, fh, (fw+fx-Cx)/width, fy/height, (Cx)/width, fh/height);
        } else {
            gHAL->dblt_rect_next(x, y, Ax, Ay, fx/width, fy/height, Ax/width, Ay/height);
            gHAL->dblt_rect_next(x+Ax, y, fw+dBx-Cx-Ax, Ay, (Ax+fx)/width, fy/height, (fw-Cx-Ax)/width, Ay/height);
            gHAL->dblt_rect_next(x+fw+dBx-Cx, y, Cx, Ay, (fw+fx-Cx)/width, fy/height, (Cx)/width, Ay/height);
            gHAL->dblt_rect_next(x, y+Ay, Ax, fh+dBy-Cy-Ay, fx/width, (fy+Ay)/height, Ax/width, (fh-Cy-Ay)/height);
            gHAL->dblt_rect_next(x+Ax, y+Ay, fw+dBx-Cx-Ax, fh+dBy-Cy-Ay, (Ax+fx)/width, (fy+Ay)/height, (fw-Cx-Ax)/width, (fh-Cy-Ay)/height);
            gHAL->dblt_rect_next(x+fw+dBx-Cx, y+Ay, Cx, fh+dBy-Cy-Ay, (fw+fx-Cx)/width, (fy+Ay)/height, (Cx)/width, (fh-Cy-Ay)/height);
            gHAL->dblt_rect_next(x, y+fh+dBy-Cy, Ax, Cy, fx/width, (fy+fh-Cy)/height, Ax/width, Cy/height);
            gHAL->dblt_rect_next(x+Ax, y+fh+dBy-Cy, fw+dBx-Cx-Ax, Cy, (Ax+fx)/width, (fy+fh-Cy)/height, (fw-Cx-Ax)/width, Cy/height);
            gHAL->dblt_rect_next(x+fw+dBx-Cx, y+fh+dBy-Cy, Cx, Cy, (fw+fx-Cx)/width, (fy+fh-Cy)/height, (Cx)/width, Cy/height);
        }
#else // !COMBINE_CHARS
        if (dBy == 0) {	// Ay + dBy + Cy == fh
            gHAL->dblt_rect(x, y, Ax, fh, fx/width, fy/height, Ax/width, fh/height, atlasname, rendermode, r, g, b, a);
            gHAL->dblt_rect(x+Ax, y, fw+dBx-Cx-Ax, fh, (Ax+fx)/width, fy/height, (fw-Cx-Ax)/width, fh/height, atlasname, rendermode, r, g, b, a);
            gHAL->dblt_rect(x+fw+dBx-Cx, y, Cx, fh, (fw+fx-Cx)/width, fy/height, (Cx)/width, fh/height, atlasname, rendermode, r, g, b, a);
        } else {
            gHAL->dblt_rect(x, y, Ax, Ay, fx/width, fy/height, Ax/width, Ay/height, atlasname, rendermode, r, g, b, a);
            gHAL->dblt_rect(x+Ax, y, fw+dBx-Cx-Ax, Ay, (Ax+fx)/width, fy/height, (fw-Cx-Ax)/width, Ay/height, atlasname, rendermode, r, g, b, a);
            gHAL->dblt_rect(x+fw+dBx-Cx, y, Cx, Ay, (fw+fx-Cx)/width, fy/height, (Cx)/width, Ay/height, atlasname, rendermode, r, g, b, a);
            gHAL->dblt_rect(x, y+Ay, Ax, fh+dBy-Cy-Ay, fx/width, (fy+Ay)/height, Ax/width, (fh-Cy-Ay)/height, atlasname, rendermode, r, g, b, a);
            gHAL->dblt_rect(x+Ax, y+Ay, fw+dBx-Cx-Ax, fh+dBy-Cy-Ay, (Ax+fx)/width, (fy+Ay)/height, (fw-Cx-Ax)/width, (fh-Cy-Ay)/height, atlasname, rendermode, r, g, b, a);
            gHAL->dblt_rect(x+fw+dBx-Cx, y+Ay, Cx, fh+dBy-Cy-Ay, (fw+fx-Cx)/width, (fy+Ay)/height, (Cx)/width, (fh-Cy-Ay)/height, atlasname, rendermode, r, g, b, a);
            gHAL->dblt_rect(x, y+fh+dBy-Cy, Ax, Cy, fx/width, (fy+fh-Cy)/height, Ax/width, Cy/height, atlasname, rendermode, r, g, b, a);
            gHAL->dblt_rect(x+Ax, y+fh+dBy-Cy, fw+dBx-Cx-Ax, Cy, (Ax+fx)/width, (fy+fh-Cy)/height, (fw-Cx-Ax)/width, Cy/height, atlasname, rendermode, r, g, b, a);
            gHAL->dblt_rect(x+fw+dBx-Cx, y+fh+dBy-Cy, Cx, Cy, (fw+fx-Cx)/width, (fy+fh-Cy)/height, (Cx)/width, Cy/height, atlasname, rendermode, r, g, b, a);
        }
#endif // !COMBINE_CHARS
#endif
//		glBindTexture(GL_TEXTURE_2D, atlasname);
//		glVertexPointer(3, GL_FLOAT, 0, vertices);
//		glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
//		glDrawArrays(GL_TRIANGLE_STRIP,  0, 8);
//		glDrawArrays(GL_TRIANGLE_STRIP,  8, 8);
//		glDrawArrays(GL_TRIANGLE_STRIP, 16, 8);
	}
}


void TAtlas::draw_atlas_sequence_center(int seq, int seqframe, float x, float y, float r, float g, float b, float a, int rendermode) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	if ((*atlasdat & 1) == 0) return;
	// loop thru frames looking for first match for seq (frame 0) -> return if none
	int nframes = *(unsigned short *)(atlasdat+2);
	int firstframe = -1;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	for (int i=0; i<nframes; i++) {
		if (seq == rp[4]) {
			firstframe = i;
			break;
		}
		rp += 6;
	}
	if (firstframe < 0) return;
	// count number of frames in the sequence
	int nseqframes = 0;
	for (int i=firstframe; i<nframes; i++) {
		if (seq != rp[4]) {
			break;
		}
		nseqframes++;
		rp += 6;
	}
	// 
	seqframe %= nseqframes;
	// draw frame (firstframe + seqframe)
	draw_atlas_texture_c(firstframe + seqframe, x, y, r, g, b, a, rendermode);
}

void TAtlas::draw_atlas_sequence_f(int seq, int seqframe, float x, float y, float r, float g, float b, float a) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	if ((*atlasdat & 1) == 0) return;
	// loop thru frames looking for first match for seq (frame 0) -> return if none
	int nframes = *(unsigned short *)(atlasdat+2);
	int firstframe = -1;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	for (int i=0; i<nframes; i++) {
		if (seq == rp[4]) {
			firstframe = i;
			break;
		}
		rp += 6;
	}
	if (firstframe < 0) return;
	// count number of frames in the sequence
	int nseqframes = 0;
	for (int i=firstframe; i<nframes; i++) {
		if (seq != rp[4]) {
			break;
		}
		nseqframes++;
		rp += 6;
	}
	// 
	seqframe %= nseqframes;
	// draw frame (firstframe + seqframe)
	draw_atlas_texture_f(firstframe + seqframe, x, y, r, g, b, a);
}

// draw a frame, centering according to hjust, vjust, compensating for image's scale of construction
//  hjust/vjust: 0 for left-justified, 0.5 for centered, 1.0 for right-justified
//  if image is constructed for 1024x768 (iPad) of scale 2.1333, but then drawn on a iPhone/retina 960/640, scale by 2/2.1333 to draw
//	float iscl = gDeviceScale / innateScale;
void TAtlas::draw_scaled_HUD_item(int frame, float x, float y, float iscl, float hjust, float vjust, float r, float g, float b, float a) {
	gHAL->matrix_push();
	gHAL->matrix_translate(x, y, 0);
	gHAL->matrix_scale(iscl, iscl, iscl);
	gHAL->matrix_translate(-hjust*get_atlas_frame_width(frame), -vjust*get_atlas_frame_height(frame), 0);
	draw_atlas_texture_f(frame, 0, 0, r, g, b, a);
	gHAL->matrix_pop();
}

void TAtlas::draw_scaled_HUD_item_additive(int frame, float x, float y, float iscl, float hjust, float vjust, float r, float g, float b, float a) {
	gHAL->matrix_push();
	gHAL->matrix_translate(x, y, 0);
	gHAL->matrix_scale(iscl, iscl, iscl);
	gHAL->matrix_translate(-hjust*get_atlas_frame_width(frame), -vjust*get_atlas_frame_height(frame), 0);
	draw_atlas_texture_f_additive(frame, 0, 0, r, g, b, a);
	gHAL->matrix_pop();
}


void TAtlas::draw_atlas_texture(int frame, float x, float y, float r, float g, float b, float a) {
	draw_atlas_texture_f(frame, (float)(int)(x + 0.5f), (float)(int)(y + 0.5f), r, g, b, a);
}


const char *TAtlas::get_atlas_original_file_name(int frame) {
//	if (atlasdat == NULL) {
//		load_atlas_data_now(pendingFilename, pendingMip);
		if (atlasdat == NULL) return "no_data";
//	}
//	//
	int nframes = *(unsigned short *)(atlasdat+2);
	if (frame >= nframes) return "frame_too_big";
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	if (*atlasdat & 1) rp += 6 * frame; else rp += 4 * frame;			// index to correct frame
	int offs = rp[0];
	int nsubframes = rp[1];
//	int origw = rp[2];
//	int origh = rp[3];
	if (nframes == (frame+1)) {											// last frame
		rp = (unsigned short *)(atlasdat + offs);
		rp += 6 * nsubframes;
		if (((unsigned char *)rp) >= (atlasdat + atlasdatsize)) return "no_name";			// if the last subframe is immediately followed by the end of file, no name
		return (const char *)rp;
	} else {
		if (*atlasdat & 1) rp += 6; else rp += 4;						// go to the next frame
		if ((offs+6*2*nsubframes) == *rp) return "no_name";				// if the last subframe is immediately followed by the first subframe of the next frame, no name
		rp = (unsigned short *)(atlasdat + offs);
		rp += 6 * nsubframes;
		return (const char *)rp;
	}
}

int TAtlas::get_atlas_frame_number_for_file_name(const char *filename, bool quiet) {
//	if (atlasdat == NULL) {
//		load_atlas_data_now(pendingFilename, pendingMip);
		if (atlasdat == NULL) return -1;
//	}
	std::string filename2 = filename;
	int f2len = (int)filename2.length();
	if (f2len > 4) {
			 if (strcmp(filename2.c_str()+f2len-4, ".png") == 0) filename2.resize(f2len-4);
		else if (strcmp(filename2.c_str()+f2len-4, ".PNG") == 0) filename2.resize(f2len-4);
	}
	
	int nframes = *(unsigned short *)(atlasdat+2);
	unsigned short *rp;
	const char *strptr = "no_name";
//printf("\n    nframes = %d\n", nframes);
	for (int frame=0; frame<nframes; frame++) {
		rp = (unsigned short *)(atlasdat + 4);
		if (*atlasdat & 1) rp += 6 * frame; else rp += 4 * frame;			// index to correct frame
		int offs = rp[0];
		int nsubframes = rp[1];
//		int origw = rp[2];
//		int origh = rp[3];
		if (nframes == (frame+1)) {											// last frame
			strptr = (char *)(atlasdat + offs);
			strptr += 2 * 6 * nsubframes;
			if ((strptr) >= (const char *)(atlasdat + atlasdatsize)) strptr = "no_name";			// if the last subframe is immediately followed by the end of file, no name
		} else {
			if (*atlasdat & 1) rp += 6; else rp += 4;						// go to the next frame
			if ((offs+6*2*nsubframes) == *rp) strptr = "no_name"; else {				// if the last subframe is immediately followed by the first subframe of the next frame, no name
				strptr = (char *)(atlasdat + offs);
				strptr += 2 * 6 * nsubframes;
			}
		}
//printf("TAtlas::get_atlas_frame_number_for_file_name(%s)  '%s'=?='%s'\n", filename, strptr, filename2.c_str());
		if (strcmp(strptr, filename) == 0) return frame;
		if (strcmp(strptr, filename2.c_str()) == 0) return frame;
	}
//
if (!quiet) printf("ERROR: '%s' not found in atlas %s\n", filename, pendingFilename.c_str());
	return -1;
}

int TAtlas::get_atlas_num_frames_for_sequence(int seq) {
	int i;
	if (pendingLoading) load_atlas_now(pendingFilename);
	if (atlasdat == NULL) return 0;
	if ((*atlasdat & 1) == 0) return 0;
	int nframes = *(unsigned short *)(atlasdat+2);
	int firstframe = -1;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	for (i=0; i<nframes; i++) {
		if (seq == rp[4]) {
			firstframe = i;
			break;
		}
		rp += 6;
	}
	if (firstframe < 0) return 0;
	// count number of frames in the sequence
	int nseqframes = 0;
	for (i=firstframe; i<nframes; i++) {
		if (seq != rp[4]) {
			break;
		}
		nseqframes++;
		rp += 6;
	}
	return nseqframes;
}

int TAtlas::get_atlas_first_frame_for_sequence(int seq) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	if (atlasdat == NULL) return 0;
	if ((*atlasdat & 1) == 0) return 0;
	int nframes = *(unsigned short *)(atlasdat+2);
	int firstframe = -1;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	for (int i=0; i<nframes; i++) {
		if (seq == rp[4]) {
			firstframe = i;
			break;
		}
		rp += 6;
	}
	if (firstframe < 0) return 0;
	return firstframe;
}

int TAtlas::get_atlas_sequence_for_frame(int frame) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	if ((*atlasdat & 1) == 0) return -1;
	rp += 6 * frame;
	rp++;		// skip offs
	rp++;		// skip nsubframes
	rp++;		// skip origw
	rp++;		// skip origh
	return *rp;
}

int TAtlas::get_atlas_num_frames() {
	if (pendingLoading) load_atlas_now(pendingFilename);
	if (atlasdat == NULL) return 0;
	return *(unsigned short *)(atlasdat+2);
}

int TAtlas::get_atlas_frame_width(int frame) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	if (*atlasdat & 1) rp += 6 * frame; else rp += 4 * frame;			// index to correct frame
	rp++;		// skip offs
	rp++;		// skip nsubframes
	return *rp;
}

int TAtlas::get_atlas_frame_height(int frame) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	if (*atlasdat & 1) rp += 6 * frame; else rp += 4 * frame;			// index to correct frame
	rp++;		// skip offs
	rp++;		// skip nsubframes
	rp++;		// skip origw
	return *rp;
}

int TAtlas::get_atlas_sequence_frame_width(int seq, int frame) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	if ((*atlasdat & 1) == 0) return 0;
	// loop thru frames looking for first match for seq (frame 0) -> return if none
	int nframes = *(unsigned short *)(atlasdat+2);
	int firstframe = -1;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	for (int i=0; i<nframes; i++) {
		if (seq == rp[4]) {
			firstframe = i;
			break;
		}
		rp += 6;
	}
	if (firstframe < 0) return 0;
	return get_atlas_frame_width(frame + firstframe);
}

int TAtlas::get_atlas_sequence_frame_height(int seq, int frame) {
	if (pendingLoading) load_atlas_now(pendingFilename);
	if ((*atlasdat & 1) == 0) return 0;
	// loop thru frames looking for first match for seq (frame 0) -> return if none
	int nframes = *(unsigned short *)(atlasdat+2);
	int firstframe = -1;
	unsigned short *rp = (unsigned short *)(atlasdat + 4);
	for (int i=0; i<nframes; i++) {
		if (seq == rp[4]) {
			firstframe = i;
			break;
		}
		rp += 6;
	}
	if (firstframe < 0) return 0;
	return get_atlas_frame_height(frame + firstframe);
}


unsigned int TAtlas::get_atlas_glname() {
	if (pendingLoading) load_atlas_now(pendingFilename);
	return atlasname;
}



