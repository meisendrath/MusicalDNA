#ifndef _HAL_H_
#define _HAL_H_

#include "config.h"
#include "common.h"

#include <string>
#include <vector>

// bypass deferred bltting, its buggin on OGL
#define IMMEDIATE_RENDERING

// flags 1 is found in Documents/
// flags 2 is found in Library/Application Support/
// flags 4 is compressed, suffix ".mdna"
typedef void (SCAN_DIRECTORY_CALLBACK)(const char *fname, int flags, void *refcon_CurvedList);

typedef unsigned int TextureHandle;		// 0 means no texture
typedef unsigned int VBOHandle;			// 0 means no vertex buffer
typedef unsigned int IBOHandle;			// 0 means no index buffer

#ifdef IMMEDIATE_RENDERING
#else // !IMMEDIATE_RENDERING
/**
*/
// TBltRect describes a single quad texture blt (use one per character glyph) (supports 9slice via tdx and tdy)
class TBltRect {
public:
											TBltRect() {
												lx = ty = rx = by = 0;
												ltx = tty = rtx = bty = 0;
												z = 0;
											}
public:
	float						lx, ty, rx, by;			// screen rect
	float						ltx, tty, rtx, bty;		// texture coords rect (ignored if texture handle is 0, do fill)
	float						z;
};

/**
*/
// TDeferredBlt contains all the info needed to encapsulate a call to blt_rect(), including strings and 9slice
class TDeferredBlt {
public:
											TDeferredBlt() {
												th = 0;
												mode = 0;
												tilted_rect = false;
												r = g = b = a = 1;
												scissors_on = false;
												sx = sy = sw = sh = 0;
												vx = vy = vw = vh = 0;
												TMatrix::identity(proj);
												TMatrix::identity(model);
											}
public:
	std::vector<TBltRect>					rects;
	TextureHandle							th;
	int										mode;
	bool									tilted_rect;			// if true, add tty to y of verts with rx
	float									r, g, b, a;				// color gets moved to constColor in the const buffer at draw time
	bool									scissors_on;
	float									sx, sy, sw, sh;
	float									vx, vy, vw, vh;
//	bool									depth_test;
//	int										depth_mask, depth_func;
	TMatrix									proj;
	TMatrix									model;
};
#endif // !IMMEDIATE_RENDERING

class TTextureLoaded {
public:
	std::string					fname;
	std::string					found_fname;
	int							w, h;
	TextureHandle				gn;
};

/**
*/
//
class HAL {
public:
	static HAL				*initial_setup();		// create the appropriate HAL for this platform
public:
	virtual void			init() = 0;
	virtual void			cleanup() = 0;
public:
	virtual void			setup_1() = 0;
	virtual void			draw_1() = 0;
public:
	virtual void			new_frame_start() {}
	virtual void			new_frame_end() {}
	virtual void			viewport(float x, float y, float width, float height) {}
	virtual void			viewport_get(float *x, float *y, float *width, float *height) {}
	virtual void			scissor(float x, float y, float width, float height) {}
	virtual void			scissor_disable() {}
	virtual void			scissor_get(bool *isOn, float *x, float *y, float *width, float *height) {}
	virtual void			set_depth_test(int depth_test, int depth_mask, int depth_func) {}
	virtual void			get_depth_test(bool *depth_test, int *depth_mask, int *depth_func) {}
	virtual void			material_diffuse_get(float *r, float *g, float *b, float *a) {}
public:
	virtual void			set_mode(int mode, float r=1, float g=1, float b=1, float a=1) {}						// -1 means just set the color
	virtual void			set_masking(bool write_color_buffer, bool write_depth_buffer, int depth_mode=-1) {}		// -1=not set; 0=less; 1=lequal
	virtual void			set_culling(bool enable, bool back) {}
public:
	virtual void			texture_bind(int unit, TextureHandle texhandle) {}
	virtual void			texture_blur(int unit) {}
	virtual void			texture_setup_repeat(TextureHandle texhandle) {}
	virtual void			texture_setup_mipmap(TextureHandle texhandle) {}
	virtual void			texture_setup_clamp_to_edge(TextureHandle texhandle) {}
	virtual TextureHandle	texture_make(int mode, int width, int height, const void *data) { return 0; }
	virtual TextureHandle	texture_process_font(int w, int h, const void *data) { return 0; }
	virtual TextureHandle	texture_process_argb(int w, int h, const void *data, bool mipmap=false) { return 0; }
	virtual TextureHandle	texture_find(bool is_font, const char *fname, int *out_width, int *out_height, std::string *ret_found_fname=NULL) { return 0; }
	virtual TextureHandle	texture_load(bool is_font, const char *fname, int *out_width, int *out_height, std::string *ret_found_fname=NULL, bool needs_mipmap=false) { return 0; }
    virtual void			texture_unload(TextureHandle th) {}
    virtual int				texture_max_size() { return 0; }
public:
	virtual void			dblt_rect(float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy, TextureHandle th, int mode, float r, float g, float b, float a) {}
	virtual void			dfill_rect(float x, float y, float dx, float dy, int mode, float r, float g, float b, float a) {}
//	virtual void			dfill_rect_dy(float x, float y, float dx, float dy, int mode, float r, float g, float b, float a, float rdy) {}
	virtual void			dblt_rect_start(TextureHandle th, int mode, float r, float g, float b, float a) {}
	virtual void			dblt_rect_next(float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy) {}
	virtual void			dblt_rect_finish() {}
#ifdef IMMEDIATE_RENDERING
#else // !IMMEDIATE_RENDERING
	virtual void			dblt_flush() {}
#endif // !IMMEDIATE_RENDERING
	virtual void			blt_rect(const float *vertices, const float *coordinates, int num_verts=4) {}
	virtual void			blt_rect8(const float *vertices, const float *coordinates, int num_verts=4) {}
	virtual void			blt_rect_nonstrip(const float *vertices, const float *coordinates, int num_verts=6) {}
	virtual void			blt_rect8_color(const float *vertices, const float *coordinates, const float *colors, int num_verts=4) {}
	virtual void			fill_rect(const float *vertices, int nv=4) {}
	virtual void			render_dots(int ndots, const float *verts, const float *colors, const float *radii) {}
	virtual unsigned int	load_PVR(const char *filename, int *ret_width, int *ret_height) { return 0; }
public:
	virtual VBOHandle		vbo_make(int sz, const void *data, unsigned int stride, unsigned int offset) { return 0; }
	virtual IBOHandle		ibo_make(int sz, const void *data, unsigned int stride, unsigned int offset) { return 0; }
	virtual void			vbo_ibo_bind(VBOHandle vbo, IBOHandle ibo, bool dup_tex_coords=false) {}	// ibo may be 0; dup_tex_coords determines if second texture unit is set up with same vbo
	virtual void			vbo_draw_setup_general(int num_verts, void *interleaved_data, unsigned int vertex_stride, void *vertex_data, unsigned int texcoord_stride, void *texcoord_data, unsigned int normals_stride, void *normals_data) {}
	virtual void			vbo_draw(int nFaces, int offset) {}
	virtual void			vbo_draw_strip(int nFaces, int offset) {}
	virtual void			vbo_ibo_unbind() {}
public:
	virtual void			skybox_delete_texture(TextureHandle t) {}
	virtual void			skybox_draw_setup(bool flashing, float a) {}
	virtual void			skybox_draw(bool flashing, const float *verts, const float *uvs, int num_verts=4) {}
	virtual void			skybox_draw_nebula_setup(float a) {}
	virtual void			skybox_draw_nebula_cleanup() {}
	virtual void			skybox_draw_cleanup() {}
public:
	virtual TextureHandle	warptube_texture_setup(void *data, int idx) { return 0; }
public:
	virtual void			warptube_draw1_setup(float r, float g, float b, float a) {}
	virtual void			warptube_draw2_setup(bool flashing_white, float r, float g, float b, float a) {}
	virtual void			warptube_draw_cleanup() {}
public:
	virtual void			render_setup_color_and_material_for_skybox(bool flashme=false, int flashcolor=0, float a=1.0f) {}
	virtual void			render_setup_color_and_material_for_objects(float *amb=NULL) {}
	virtual void			render_setup_color_and_material_for_simple_render() {}
public:
	virtual void			lighting_off() {}
	virtual void			render_additive_setup(float r=1, float g=1, float b=1, float a=1) {}
	virtual void			render_explosion_setup(float r=1, float g=1, float b=1, float a=1) {}
	virtual void			render_additive_cleanup() {}
	virtual void			render_simple_setup() {}
	virtual void			render_simple_cleanup() {}
	virtual void			render_bg_setup() {}
	virtual void			render_objects_setup() {}
	virtual void			render_rect_setup(float r, float g, float b, float a) {}
	virtual void			render_big_map_setup() {}
	virtual void			render_big_map_cleanup() {}
	virtual void			render_popup_setup() {}
	virtual void			render_popup_cleanup() {}
public:
	virtual void			render_stars(const float *stars_points, int STARS_MAX, float STARS_CUBE_HALFSIZE, int SPEED_FACTOR, TMatrix *viewMatrix, float a, float k0, float k1) {}
	virtual void			render_icoverts(int ico2vertsvalid, const float *ico2validverts, const float *ico2validsizes, const unsigned long *ico2validcolors) {}
	virtual void			render_shield(float opacity, bool renderop_2sided, const float *colorcorners, const float *ico2corners, const float *ico2normals, const unsigned short *ico2tris, int nidxs=4 * 20 * 3, int nverts=42, float emissive=0.0f) {}
	virtual void			render_exhaust(bool two_layers, const float *verts, const float *uvs) {}
	virtual void			render_exhaust2(TextureHandle th, float r, float g, float b, float a) {}
	virtual void			render_emissive_setup1(TextureHandle t) {}
	virtual void			render_emissive_setup2(float *color) {}
	virtual void			render_emissive_cleanup() {}
	virtual int				render_alpha_setup1() { return 0; }
	virtual void			render_alpha_setup2() {}
	virtual void			render_alpha_cleanup(int savedDF) {}

// Matrix Stacks (not doing texture matrix stacks this way)
public:		// these routines modify the stack, then call matrix_notify_changed() to let the concrete implementation know
	void					matrix_push(int modelProjView=0);		// 1 for projection matrix
	void					matrix_pop(int modelProjView=0);
	void					matrix_translate(float x, float y, float z, int modelProjView=0);
	void					matrix_rotate(float ang, float axisx, float axisy, float axisz, int modelProjView=0);
	void					matrix_rotateX(float ang, int modelProjView=0);
	void					matrix_rotateY(float ang, int modelProjView=0);
	void					matrix_rotateZ(float ang, int modelProjView=0);
	void					matrix_scale(float sx, float sy, float sz, int modelProjView=0);
	void					matrix_get(TMatrix *mOut, int modelProjView=0);
	void					matrix_loadidentity(int modelProjView=0);
	void					matrix_load(TMatrix &m, int modelProjView=0);
	void					matrix_multiply(TMatrix &m, int modelProjView=0);
	// projection matrix only
	void					matrix_lookAt();
	void					matrix_perspective();
	void					matrix_ortho(float left, float right, float bottom, float top, float zNear, float zFar);
private:	// platform-specific layer is notified of changes to either matrix via (matrix_notify_changed)
	virtual void			matrix_notify_changed(TMatrix &m, int modelProjView=0) = 0;
	virtual void			matrix_notify_get(TMatrix &m, int modelProjView=0) = 0;
private:
	std::vector<TMatrix>	stacks[3];
protected:
	std::vector<TTextureLoaded> loaded_textures;

public:
#ifdef IMMEDIATE_RENDERING
	void					DBlt_add(float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy, TextureHandle th, int mode, float r, float g, float b, float a);
	void					DBlt_add(float x, float y, float dx, float dy, int mode, float r, float g, float b, float a);
#else // !IMMEDIATE_RENDERING
	void					DBlt_flush();
	int						DBlt_num_pending();
	TDeferredBlt			*DBlt_add(float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy, TextureHandle th, int mode, float r, float g, float b, float a);
	TDeferredBlt			*DBlt_add(float x, float y, float dx, float dy, int mode, float r, float g, float b, float a);
	TDeferredBlt			*DBlt_new(TextureHandle th, int mode, float r, float g, float b, float a);								// make a new list, for strings
	void					DBlt_append(TDeferredBlt *db, float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy, float z=0);			// add a blt to a list, for strings
	void					DBlt_finish();
#endif // !IMMEDIATE_RENDERING
private:
	virtual void			DBlt_draw_setup(float *vertices, float *coordinates, short *indices, int nquads) {}
	virtual void			DBlt_draw_cleanup() {}
	virtual void			DBlt_draw_blt(TextureHandle th, short *indices, int wquad, int nquads) {}
	virtual void			DBlt_draw_blt_array(TextureHandle th, int nverts) {}

public:		// hardware platform interface
	static unsigned char	*load_data_file(const char *fname, int *out_size, bool silent=false);
	static bool				file_exists(const char *fname, std::string *ret_found_filename=NULL);
//	static FILE				*open_file(const char *fname, std::string *ret_found_filename=NULL, const char *perm="rb");
	static void 			delete_file_write_docs_dir(const char *fname);
	static void 			delete_file_write_lib_dir(const char *fname);
	static FILE 			*open_file_write_docs_dir(const char *fname, const char *perm="wb");
	static FILE 			*open_file_write_lib_dir(const char *fname, const char *perm="wb");
	static void				rename_file_write_lib_dir(const char *oldname, const char *newname);
	static TextureHandle	load_font_file(const char *fname, int *out_width, int *out_height, std::string *ret_found_fname=NULL);
	static TextureHandle	load_argb_file(const char *fname, int *out_width, int *out_height, std::string *ret_found_fname=NULL, bool needs_mipmap=false);
	static void 			load_texture_file(const char *fname, TextureHandle *ret_glname, int *ret_width, int *ret_height, bool needs_mipmap=false);
	static void 			release_texture_file(TextureHandle glname);
	static void 			load_atlas_files(const char *fname, int *atlaswidth, int *atlasheight, unsigned int *atlasname, unsigned int *atlasdatsize, unsigned char **atlasdat);
	static void 			scan_song_directory(SCAN_DIRECTORY_CALLBACK callback, void *refcon_CurvedList);		// Documents/, Library/ then Bundle/
	static void				open_URL(const char *url);
	static void				set_idle_timer_enable(bool is_enabled);
//	static TextureHandle	texture_load_itunes_image(std::string &iconfname, std::string &fname, double duration, int *out_width, int *out_height);
//private:		// hardware platform interface support
//	static void				get_bundle_dir(char *dest, int sz);			// Bundle/
//	static void				get_library_dir(char *dest, int sz);		// Library/ (not visible to user, as files in Documents/ are)
//	static void				get_documents_dir(char *dest, int sz);		// Documents/ (visible to user in iTunes)

public:		// analytics
	static void				report_user_id(const char *uid);
	static void				report_origin(const char *origin, const char *version, const char *arg0key=NULL, const char *arg0=NULL, const char *arg1key=NULL, const char *arg1=NULL, const char *arg2key=NULL, const char *arg2=NULL);
	static void				report_event(const char *ename, const char *arg0key=NULL, const char *arg0=NULL, const char *arg1key=NULL, const char *arg1=NULL, const char *arg2key=NULL, const char *arg2=NULL);
	static void				report_event_timed_start(const char *ename, const char *arg0key=NULL, const char *arg0=NULL, const char *arg1key=NULL, const char *arg1=NULL, const char *arg2key=NULL, const char *arg2=NULL);
	static void				report_event_timed_end(const char *ename, const char *arg0key=NULL, const char *arg0=NULL, const char *arg1key=NULL, const char *arg1=NULL, const char *arg2key=NULL, const char *arg2=NULL);
};
#endif // !_HAL_H_
