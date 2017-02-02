public:
	virtual void			init();
	virtual void			cleanup();
public:
	virtual void			setup_1();
	virtual void			draw_1();
public:
	virtual void			new_frame_start();
	virtual void			new_frame_end();
	virtual void			viewport(float x, float y, float width, float height);
	virtual void			viewport_get(float *x, float *y, float *width, float *height);
	virtual void			scissor(float x, float y, float width, float height);
	virtual void			scissor_disable();
	virtual void			scissor_get(bool *isOn, float *x, float *y, float *width, float *height);
	virtual void			set_depth_test(int depth_test, int depth_mask, int depth_func);
	virtual void			get_depth_test(bool *depth_test, int *depth_mask, int *depth_func);
	virtual void			material_diffuse_get(float *r, float *g, float *b, float *a);
public:
	virtual void			set_mode(int mode, float r=1, float g=1, float b=1, float a=1);
	virtual void			set_masking(bool write_color_buffer, bool write_depth_buffer, int depth_mode=-1);
	virtual void			set_culling(bool enable, bool back);
public:
	virtual void			texture_bind(int unit, unsigned int texhandle);
	virtual void			texture_blur(int unit);
	virtual void			texture_setup_repeat(TextureHandle texhandle);
	virtual void			texture_setup_mipmap(TextureHandle texhandle);
	virtual void			texture_setup_clamp_to_edge(TextureHandle texhandle);
	virtual TextureHandle	texture_make(int mode, int width, int height, const void *data);
	virtual TextureHandle	texture_process_font(int w, int h, const void *data);
	virtual TextureHandle	texture_process_argb(int w, int h, const void *data, bool mipmap=false);
	virtual TextureHandle	texture_find(bool is_font, const char *fname, int *out_width, int *out_height, std::string *ret_found_fname=NULL);
	virtual TextureHandle	texture_load(bool is_font, const char *fname, int *out_width, int *out_height, std::string *ret_found_fname=NULL, bool needs_mipmap=false);
    virtual void			texture_unload(TextureHandle th);
    virtual int				texture_max_size();
public:
	virtual void			dblt_rect(float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy, TextureHandle th, int mode, float r, float g, float b, float a);
	virtual void			dfill_rect(float x, float y, float dx, float dy, int mode, float r, float g, float b, float a);
//	virtual void			dfill_rect_dy(float x, float y, float dx, float dy, int mode, float r, float g, float b, float a, float rdy);
	virtual void			dblt_rect_start(TextureHandle th, int mode, float r, float g, float b, float a);
	virtual void			dblt_rect_next(float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy);
#ifdef IMMEDIATE_RENDERING
#else // !IMMEDIATE_RENDERING
	virtual void			dblt_flush();
#endif // !IMMEDIATE_RENDERING
	virtual void			blt_rect(const float *vertices, const float *coordinates, int num_verts=4);
	virtual void			blt_rect8(const float *vertices, const float *coordinates, int num_verts=4);
	virtual void			blt_rect_nonstrip(const float *vertices, const float *coordinates, int num_verts=6);
	virtual void			blt_rect8_color(const float *vertices, const float *coordinates, const float *colors, int num_verts=4);
	virtual void			fill_rect(const float *vertices, int nv=4);
	virtual void			render_dots(int ndots, const float *verts, const float *colors, const float *radii);
	virtual unsigned int	load_PVR(const char *filename, int *ret_width, int *ret_height);
public:
	virtual VBOHandle		vbo_make(int sz, const void *data, unsigned int stride, unsigned int offset);
	virtual IBOHandle		ibo_make(int sz, const void *data, unsigned int stride, unsigned int offset);
	virtual void			vbo_ibo_bind(VBOHandle vbo, IBOHandle ibo, bool dup_tex_coords=false);	// ibo may be 0; dup_tex_coords determines if second texture unit is set up with same vbo
	virtual void			vbo_draw_setup_general(int num_verts, void *interleaved_data, unsigned int vertex_stride, void *vertex_data, unsigned int texcoord_stride, void *texcoord_data, unsigned int normals_stride, void *normals_data);
	virtual void			vbo_draw(int nFaces, int offset);
	virtual void			vbo_draw_strip(int nFaces, int offset);
	virtual void			vbo_ibo_unbind();
public:
	virtual void			skybox_delete_texture(TextureHandle t);
	virtual void			skybox_draw_setup(bool flashing, float a);
	virtual void			skybox_draw(bool flashing, const float *verts, const float *uvs, int num_verts=4);
	virtual void			skybox_draw_nebula_setup(float a);
	virtual void			skybox_draw_nebula_cleanup();
	virtual void			skybox_draw_cleanup();
public:
	virtual TextureHandle	warptube_texture_setup(void *data, int idx);
	virtual void			warptube_draw1_setup(float r, float g, float b, float a);
	virtual void			warptube_draw2_setup(bool flashing_white, float r, float g, float b, float a);
	virtual void			warptube_draw_cleanup();
public:
	virtual void			render_setup_color_and_material_for_skybox(bool flashme=false, int flashcolor=0, float a=1.0f);
	virtual void			render_setup_color_and_material_for_objects(float *amb=NULL);
	virtual void			render_setup_color_and_material_for_simple_render();
public:
	virtual void			lighting_off();
	virtual void			render_additive_setup(float r=1, float g=1, float b=1, float a=1);
	virtual void			render_explosion_setup(float r=1, float g=1, float b=1, float a=1);
	virtual void			render_additive_cleanup();
	virtual void			render_simple_setup();
	virtual void			render_simple_cleanup();
	virtual void			render_bg_setup();
	virtual void			render_objects_setup();
	virtual void			render_rect_setup(float r, float g, float b, float a);
	virtual void			render_big_map_setup();
	virtual void			render_big_map_cleanup();
	virtual void			render_popup_setup();
	virtual void			render_popup_cleanup();
public:
	virtual void			render_stars(const float *stars_points, int STARS_MAX, float STARS_CUBE_HALFSIZE, int SPEED_FACTOR, TMatrix *viewMatrix, float a, float k0, float k1);
	virtual void			render_icoverts(int ico2vertsvalid, const float *ico2validverts, const float *ico2validsizes, const unsigned long *ico2validcolors);
	virtual void			render_shield(float opacity, bool renderop_2sided, const float *colorcorners, const float *ico2corners, const float *ico2normals, const unsigned short *ico2tris, int nidxs=4 * 20 * 3, int nverts=42, float emissive=0.0f);
	virtual void			render_exhaust(bool two_layers, const float *verts, const float *uvs);
	virtual void			render_exhaust2(TextureHandle th, float r, float g, float b, float a);
	virtual void			render_emissive_setup1(TextureHandle t);
	virtual void			render_emissive_setup2(float *color);
	virtual void			render_emissive_cleanup();
	virtual int				render_alpha_setup1();
	virtual void			render_alpha_setup2();
	virtual void			render_alpha_cleanup(int savedDF);
public:
	virtual void			DBlt_draw_setup(float *vertices, float *coordinates, short *indices, int nquads);
	virtual void			DBlt_draw_cleanup();
	virtual void			DBlt_draw_blt(TextureHandle th, short *indices, int wquad, int nquads);
	virtual void			DBlt_draw_blt_array(TextureHandle th, int nverts);

private:
	virtual void			matrix_notify_changed(TMatrix &m, int modelProjView=0);
	virtual void			matrix_notify_get(TMatrix &m, int modelProjView=0);
private:
