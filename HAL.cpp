#include "HAL.h"

#ifdef PCport			// include one of the platform-specific HAL.h's
#include "HAL_DX9.h"
								#include "HAL_OGL.h"
#else // !PCport
#include "HAL_OGL.h"
#endif // !PCport

HAL *gHAL;

HAL *HAL::initial_setup() {
#ifdef PCport			// create the correct platform-specific HAL
#ifdef PCdirectx
	HAL *h = new HAL_DX9;			// this line selects a particular HAL implementation for a particular build
#else // !PCdirectx
	HAL *h = new HAL_OGL;			// this line selects a particular HAL implementation for a particular build
#endif // !PCdirectx
#else // !PCport
	HAL *h = new HAL_OGL;			// this line selects a particular HAL implementation for a particular build
#endif // !PCport
    h->init();

	TMatrix m;
	TMatrix::identity(m);
	h->stacks[0].push_back(m);
	h->stacks[1].push_back(m);
	h->stacks[2].push_back(m);

	return h;
}

void HAL::matrix_push(int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];

	TMatrix m;
	m = (*st)[(*st).size()-1];
	(*st).push_back(m);
}
void HAL::matrix_pop(int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];

	if ((*st).size() == 0) {
printf("ERROR: stack empty!\n");
		TMatrix m;
		TMatrix::identity(m);
		(*st).push_back(m);
	} else if ((*st).size() == 1) {
printf("ERROR: stack almost empty! (push/pop imbalance)\n");
		(*st).pop_back();
		TMatrix m;
		TMatrix::identity(m);
		(*st).push_back(m);
	} else {
		(*st).pop_back();
	}
	matrix_notify_changed((*st)[(*st).size()-1], modelProjView);
}
void HAL::matrix_translate(float x, float y, float z, int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];
	TMatrix &d = (*st)[(*st).size()-1];

	TMatrix m;
	TMatrix::translate(m, x, y, z);
	TMatrix::multiply(d, m, d);
	matrix_notify_changed(d, modelProjView);
}
void HAL::matrix_rotate(float ang, float axisx, float axisy, float axisz, int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];
	TMatrix &d = (*st)[(*st).size()-1];

	TMatrix m;
	TMatrix::rotationAxis(m, -ang*3.14159265f/180, -axisx, -axisy, -axisz);
	TMatrix::multiply(d, m, d);
	matrix_notify_changed(d, modelProjView);
}
void HAL::matrix_rotateX(float ang, int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];
	TMatrix &d = (*st)[(*st).size()-1];

	TMatrix m;
	TMatrix::rotationX(m, -ang*3.14159265f/180);
	TMatrix::multiply(d, m, d);
	matrix_notify_changed(d, modelProjView);
}
void HAL::matrix_rotateY(float ang, int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];
	TMatrix &d = (*st)[(*st).size()-1];

	TMatrix m;
	TMatrix::rotationY(m, -ang*3.14159265f/180);
	TMatrix::multiply(d, m, d);
	matrix_notify_changed(d, modelProjView);
}
void HAL::matrix_rotateZ(float ang, int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];
	TMatrix &d = (*st)[(*st).size()-1];

	TMatrix m;
	TMatrix::rotationZ(m, -ang*3.14159265f/180);
	TMatrix::multiply(d, m, d);
	matrix_notify_changed(d, modelProjView);
}
void HAL::matrix_scale(float sx, float sy, float sz, int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];
	TMatrix &d = (*st)[(*st).size()-1];

	TMatrix m;
	TMatrix::scaling(m, sx, sy, sz);
	TMatrix::multiply(d, m, d);
	matrix_notify_changed(d, modelProjView);
}
void HAL::matrix_get(TMatrix *mOut, int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];
	TMatrix &d = (*st)[(*st).size()-1];

	*mOut = d;
}
void HAL::matrix_loadidentity(int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];
	TMatrix &d = (*st)[(*st).size()-1];

	TMatrix::identity(d);
	matrix_notify_changed(d, modelProjView);
}
void HAL::matrix_load(TMatrix &m, int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];
	TMatrix &d = (*st)[(*st).size()-1];

	d = m;
	matrix_notify_changed(d, modelProjView);
}
void HAL::matrix_multiply(TMatrix &m, int modelProjView) {
	std::vector<TMatrix> *st;
	st = &stacks[modelProjView];
	TMatrix &d = (*st)[(*st).size()-1];

	TMatrix::multiply(d, m, d);
	matrix_notify_changed(d, modelProjView);
}
	// projection matrix only
void HAL::matrix_lookAt() {
//	std::vector<TMatrix> *st = &projStack;

}
void HAL::matrix_perspective() {
//	std::vector<TMatrix> *st = &projStack;

}
void HAL::matrix_ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
	std::vector<TMatrix> *st = &stacks[1];
	TMatrix &d = (*st)[(*st).size()-1];

	TMatrix m;
	TMatrix::orthoRH(m, left, right, bottom, top, zNear, zFar, false);
	TMatrix::multiply(d, m, d);
	matrix_notify_changed(d, 1);
}





#ifdef IMMEDIATE_RENDERING
#else // !IMMEDIATE_RENDERING
std::vector<TDeferredBlt>		DBlt_list;

void HAL::DBlt_flush() {
	if (DBlt_list.size() == 0) return;
	gHAL->matrix_push();
	gHAL->matrix_push(1);
	gHAL->matrix_push(2);
	gHAL->matrix_loadidentity(2);
	int nquads = 0;
	unsigned int i;
	for (i=0; i<DBlt_list.size(); i++) nquads += DBlt_list[i].rects.size();
	std::vector<float>		vertices;
	std::vector<float>		coordinates;
	std::vector<short>		indices;
	// fill out raw data
	int wquad = 0;
	for (i=0; i<DBlt_list.size(); i++) {
		TDeferredBlt *db = &DBlt_list[i];
		for (unsigned int j=0; j<db->rects.size(); j++) {
			TBltRect *br = &db->rects[j];
			vertices.push_back(br->lx);
			vertices.push_back(br->by);
			vertices.push_back(br->z);
			vertices.push_back(br->lx);
			vertices.push_back(br->ty);
			vertices.push_back(br->z);
			vertices.push_back(br->rx);
			vertices.push_back(br->by + ((db->tilted_rect)?br->tty:0));
			vertices.push_back(br->z);
			vertices.push_back(br->rx);
			vertices.push_back(br->ty + ((db->tilted_rect)?br->tty:0));
			vertices.push_back(br->z);
			coordinates.push_back(br->ltx);
			coordinates.push_back(br->bty);
			coordinates.push_back(br->ltx);
			coordinates.push_back(br->tty);
			coordinates.push_back(br->rtx);
			coordinates.push_back(br->bty);
			coordinates.push_back(br->rtx);
			coordinates.push_back(br->tty);
			indices.push_back((short)(4*wquad+0));
			indices.push_back((short)(4*wquad+1));
			indices.push_back((short)(4*wquad+2));
			indices.push_back((short)(4*wquad+1));
			indices.push_back((short)(4*wquad+3));
			indices.push_back((short)(4*wquad+2));
			wquad++;
		}
	}
	// create vbos for verts and texture coords
	// create ibo with fixed indices to make quads compact
	// call DX to transfer the data
	DBlt_draw_setup(&vertices[0], &coordinates[0], &indices[0], nquads);
// OGL:	glVertexPointer(3, GL_FLOAT, 0, vertices);
// OGL:	glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
	// draw
	wquad = 0;
	bool scissors_are_on = false;
	float sx, sy, sw, sh;
	for (i=0; i<DBlt_list.size(); i++) {
		TDeferredBlt *db = &DBlt_list[i];
		int nr = db->rects.size();
		if (nr == 0) continue;
		gHAL->matrix_load(db->model);
		gHAL->matrix_load(db->proj, 1);
		gHAL->set_mode(db->mode, db->r, db->g, db->b, db->a);
//		gHAL->set_depth_test(db->depth_test, db->depth_mask, db->depth_func);
		if (db->scissors_on) {
			if (!scissors_are_on || (db->sx != sx) || (db->sy != sy) || (db->sw != sw) || (db->sh != sh)) {
				scissors_are_on = true;
				sx = db->sx;
				sy = db->sy;
				sw = db->sw;
				sh = db->sh;
				gHAL->scissor(sx, sy, sw, sh);
			}
		} else {
			if (scissors_are_on) {
				scissors_are_on = false;
				gHAL->scissor_disable();
			}
		}
		gHAL->viewport(db->vx, db->vy, db->vw, db->vh);
		DBlt_draw_blt(db->th, &indices[6*wquad], wquad, nr);
// OGL:	glBindTexture(GL_TEXTURE_2D, th);
// OGL:	if (th == 0) glDisable(GL_TEXTURE_2D);
// OGL:	glDrawElements(GL_TRIANGLES, nquads*6, GL_UNSIGNED_SHORT, indices);
// OGL:	if (th == 0) glEnable(GL_TEXTURE_2D);
		if (db->mode == 10) gHAL->set_mode(11);					// clean up after mode 10 subtractive sprites
		wquad += nr;
	}
	//
	DBlt_draw_cleanup();
// OGL: NULL
	// release the vbos/ibo
	// clear the old list
	DBlt_list.resize(0);
	gHAL->matrix_pop();
	gHAL->matrix_pop(1);
	gHAL->matrix_pop(2);
	gHAL->set_mode(0, 1, 1, 1, 1);
}
int HAL::DBlt_num_pending() {
	return DBlt_list.size();
}
#endif // !IMMEDIATE_RENDERING

#ifdef IMMEDIATE_RENDERING
void HAL::DBlt_add(float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy, TextureHandle th, int mode, float r, float g, float b, float a) {
	float					vertices[12];
	float					coordinates[8];
//	// fill out raw data
	vertices[0] = x;
	vertices[1] = y+dy;
	vertices[2] = 0;
	vertices[3] = x;
	vertices[4] = y;
	vertices[5] = 0;
	vertices[6] = x+dx;
	vertices[7] = y+dy;// + ((db->tilted_rect)?br->tty:0);
	vertices[8] = 0;
	vertices[9] = x+dx;
	vertices[10] = y;// + ((db->tilted_rect)?br->tty:0);
	vertices[11] = 0;
	coordinates[0] = tx;
	coordinates[1] = ty+tdy;
	coordinates[2] = tx;
	coordinates[3] = ty;
	coordinates[4] = tx+tdx;
	coordinates[5] = ty+tdy;
	coordinates[6] = tx+tdx;
	coordinates[7] = ty;
	DBlt_draw_setup(&vertices[0], &coordinates[0], NULL, 0);
	gHAL->set_mode(mode, r, g, b, a);
	DBlt_draw_blt_array(th, 4);
	if (mode == 10) gHAL->set_mode(11);					// clean up after mode 10 subtractive sprites
}
#else // !IMMEDIATE_RENDERING
TDeferredBlt *HAL::DBlt_add(float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy, TextureHandle th, int mode, float r, float g, float b, float a) {
	TDeferredBlt db;
	TBltRect	rct;
	rct.lx = x; rct.ty = y; rct.rx = x+dx; rct.by = y+dy;
	rct.ltx = tx; rct.tty = ty; rct.rtx = tx+tdx; rct.bty = ty+tdy;
	rct.z = 0;
	db.rects.push_back(rct);
	db.th = th;
	db.mode = mode;
	db.r = r; db.g = g; db.b = b; db.a = a;
	gHAL->matrix_get(&db.model);
	gHAL->matrix_get(&db.proj, 1);
	gHAL->scissor_get(&db.scissors_on, &db.sx, &db.sy, &db.sw, &db.sh);
	gHAL->viewport_get(&db.vx, &db.vy, &db.vw, &db.vh);
//	gHAL->get_depth_test(&db.depth_test, &db.depth_mask, &db.depth_func);
	DBlt_list.push_back(db);
	return &DBlt_list[DBlt_list.size()-1];
}
#endif // !IMMEDIATE_RENDERING
#ifdef IMMEDIATE_RENDERING
void HAL::DBlt_add(float x, float y, float dx, float dy, int mode, float r, float g, float b, float a) {
	float					vertices[12];
//	// fill out raw data
	vertices[0] = x;
	vertices[1] = y+dy;
	vertices[2] = 0;
	vertices[3] = x;
	vertices[4] = y;
	vertices[5] = 0;
	vertices[6] = x+dx;
	vertices[7] = y+dy;// + ((db->tilted_rect)?br->tty:0);
	vertices[8] = 0;
	vertices[9] = x+dx;
	vertices[10] = y;// + ((db->tilted_rect)?br->tty:0);
	vertices[11] = 0;
	DBlt_draw_setup(&vertices[0], NULL, NULL, 0);
	gHAL->set_mode(mode, r, g, b, a);
	DBlt_draw_blt_array(0, 4);
	if (mode == 10) gHAL->set_mode(11);					// clean up after mode 10 subtractive sprites
}
#else // !IMMEDIATE_RENDERING
TDeferredBlt *HAL::DBlt_add(float x, float y, float dx, float dy, int mode, float r, float g, float b, float a) {
	TDeferredBlt db;
	TBltRect	rct;
	rct.lx = x; rct.ty = y; rct.rx = x+dx; rct.by = y+dy;
	rct.z = 0;
	db.rects.push_back(rct);
	db.th = 0;
	db.mode = mode;
	db.r = r; db.g = g; db.b = b; db.a = a;
	gHAL->matrix_get(&db.model);
	gHAL->matrix_get(&db.proj, 1);
	gHAL->scissor_get(&db.scissors_on, &db.sx, &db.sy, &db.sw, &db.sh);
	gHAL->viewport_get(&db.vx, &db.vy, &db.vw, &db.vh);
//	gHAL->get_depth_test(&db.depth_test, &db.depth_mask, &db.depth_func);
	DBlt_list.push_back(db);
	return &DBlt_list[DBlt_list.size()-1];
}
#endif // !IMMEDIATE_RENDERING
#ifdef IMMEDIATE_RENDERING
//void HAL::DBlt_new(TextureHandle th, int mode, float r, float g, float b, float a) {								// make a new list, for strings
//	TDeferredBlt db;
//	db.th = th;
//	db.mode = mode;
//	db.r = r; db.g = g; db.b = b; db.a = a;
//	gHAL->matrix_get(&db.model);
//	gHAL->matrix_get(&db.proj, 1);
//	gHAL->scissor_get(&db.scissors_on, &db.sx, &db.sy, &db.sw, &db.sh);
//	gHAL->viewport_get(&db.vx, &db.vy, &db.vw, &db.vh);
////	gHAL->get_depth_test(&db.depth_test, &db.depth_mask, &db.depth_func);
//	DBlt_list.push_back(db);
////	return &DBlt_list[DBlt_list.size()-1];
//}
#else // !IMMEDIATE_RENDERING
TDeferredBlt *HAL::DBlt_new(TextureHandle th, int mode, float r, float g, float b, float a) {								// make a new list, for strings
	TDeferredBlt db;
	db.th = th;
	db.mode = mode;
	db.r = r; db.g = g; db.b = b; db.a = a;
	gHAL->matrix_get(&db.model);
	gHAL->matrix_get(&db.proj, 1);
	gHAL->scissor_get(&db.scissors_on, &db.sx, &db.sy, &db.sw, &db.sh);
	gHAL->viewport_get(&db.vx, &db.vy, &db.vw, &db.vh);
//	gHAL->get_depth_test(&db.depth_test, &db.depth_mask, &db.depth_func);
	DBlt_list.push_back(db);
	return &DBlt_list[DBlt_list.size()-1];
}
#endif // !IMMEDIATE_RENDERING
#ifdef IMMEDIATE_RENDERING
#else // !IMMEDIATE_RENDERING
void HAL::DBlt_append(TDeferredBlt *db, float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy, float z) {			// add a blt to a list, for strings
	TBltRect	rct;
	rct.lx = x; rct.ty = y; rct.rx = x+dx; rct.by = y+dy;
	rct.ltx = tx; rct.tty = ty; rct.rtx = tx+tdx; rct.bty = ty+tdy;
	rct.z = z;
	db->rects.push_back(rct);
}
#endif // !IMMEDIATE_RENDERING



/*
void HAL_OGL::DBlt_draw_setup(float *vertices, float *coordinates, short *indices, int nquads) {
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
}
void HAL_OGL::DBlt_draw_blt(TextureHandle th, short *indices, int wquad, int nquads) {
	glBindTexture(GL_TEXTURE_2D, th);
	if (th == 0) glDisable(GL_TEXTURE_2D);
	glDrawElements(GL_TRIANGLES, nquads*6, GL_UNSIGNED_SHORT, indices);
	if (th == 0) glEnable(GL_TEXTURE_2D);
}
void HAL_OGL::DBlt_draw_cleanup() {
}
*/





/*
HAL ideas


gles 2.0 -> dx9! by Angle project, so do gles 2.0 first
maybe use shader super-language CG, compiles to GLES or DX9 shader language





// HAL.h
#ifndef _HAL_H_
#define _HAL_H_
#include "config.h"
class HAL {
public:
	static HAL			*initial_setup();
public:
	void				init() = 0;
	void				cleanup() = 0;
public:
	void				setup_1() = 0;
	void				draw_1() = 0;
};
#endif // !_HAL_H_

// HAL.cpp
#include "HAL.h"
#include "HAL_OGL.h"
HAL *HAL::initial_setup() {
	HAL *h = new HAL_OGL;			// this line selects a particular HAL implementation for a particular build
	h->init();
	return h;
}


// HAL_Decl.hincl
public:
	void				init();
	void				cleanup();
public:
	void				setup_1();
	void				draw_1();
private:


// HAL_OGL.h
#ifndef _HAL_OGL_H_
#define _HAL_OGL_H_
#include "HAL.h"
class HAL_OGL : public HAL {
#include "HAL_Decl.hincl"
// additional private members
};
#endif // !_HAL_OGL_H_

// HAL_OGL.cpp
#include "HAL_OGL.h"
void HAL_OGL::init() {
}
void HAL_OGL::cleanup() {
}
void HAL_OGL::setup_1() {
}
void HAL_OGL::draw_1() {
}


// HAL_GLES1.h
#ifndef _HAL_GLES1_H_
#define _HAL_GLES1_H_
#include "HAL.h"
class HAL_GLES1 : public HAL {
#include "HAL_Decl.hincl"
// additional private members
};
#endif // !_HAL_GLES1_H_

// HAL_GLES1.cpp
#include "HAL_GLES1.h"
void HAL_GLES1::init() {
}
void HAL_GLES1::cleanup() {
}
void HAL_GLES1::setup_1() {
}
void HAL_GLES1::draw_1() {
}


// HAL_GLES2.h
#ifndef _HAL_GLES2_H_
#define _HAL_GLES2_H_
#include "HAL.h"
class HAL_GLES2 : public HAL {
#include "HAL_Decl.hincl"
// additional private members
};
#endif // !_HAL_GLES2_H_

// HAL_GLES2.cpp
#include "HAL_GLES2.h"
void HAL_GLES2::init() {
}
void HAL_GLES2::cleanup() {
}
void HAL_GLES2::setup_1() {
}
void HAL_GLES2::draw_1() {
}


// HAL_DX9.h
#ifndef _HAL_DX9_H_
#define _HAL_DX9_H_
#include "HAL.h"
class HAL_DX9 : public HAL {
#include "HAL_Decl.hincl"
// additional private members
};
#endif // !_HAL_DX9_H_

// HAL_DX9.cpp
#include "HAL_DX9.h"
void HAL_DX9::init() {
}
void HAL_DX9::cleanup() {
}
void HAL_DX9::setup_1() {
}
void HAL_DX9::draw_1() {
}


// HAL_relay2.h
#ifndef _HAL_RELAY2_H_
#define _HAL_RELAY2_H_
#include "HAL_GLES1.h"
#include "HAL_DX9.h"
class HAL_relay2 : public HAL {
#include "HAL_Decl.hincl"
// additional private members
	HAL_GLES2			*gles;
	HAL_DX9				*dx9;
};
#endif // !_HAL_RELAY2_H_

// HAL_relay2.cpp
void HAL_relay2::init() {
	gles = new HAL_GL;
	dx9 = new HAL_GLES2;
	gles->init();
	dx9->init();
}
void HAL_relay2::cleanup() {
	gles->cleanup();
	dx9->cleanup();
	delete gles;
	delete dx9;
	gles = NULL;
	dx9 = NULL;
}
void HAL_relay2::setup_1() {
	gles->setup_1();
	dx9->setup_1();
}
void HAL_relay2::draw_1() {
	gles->draw_1();
	dx9->draw_1();
}

// typical client:
#include "HAL.h"
HAL *gHAL;
@ startup:
gHAL = HAL::initial_setup();
@ draw:
gHAL->setup_1();
gHAL->draw_1();



*/
