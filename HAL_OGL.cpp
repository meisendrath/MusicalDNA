#include "HAL_OGL.h"


#include <stdlib.h>



#define GLES1			// select simpler code, later can bring back shaders



#ifdef __APPLE__
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else

#ifdef WIN32
#include <../glew-1.7.0/include/GL/glew.h>
#else // !WIN32
#ifndef PLATFORM_ANDROID
#include <gl/glew.h>
#endif // PLATFORM_ANDROID
#endif // !WIN32

#ifdef PLATFORM_ANDROID
#include <GLES/gl.h>
#include <GLES/glext.h>
#else // !PLATFORM_ANDROID
#include <gl/gl.h>
#endif // !PLATFORM_ANDROID
#endif

#include "TTextureFile.h"

#include "Convert.h"				// need this for PC input by png




static float lightAmbient[]				= { 0.0f, 0.0f, 0.0f, 1.0f };
static float lightBrightAmbient[]		= { 0.2f, 0.2f, 0.2f, 1.0f };
static float lightDiffuse[]				= { 1.0f, 1.0f, 1.0f, 1.0f };
static float lightSpecular[]			= { 0.2f, 0.2f, 0.2f, 1.0f };			// shared with ambient for ships
static float lightGlobalAmbient[]		= { 0.0f, 0.0f, 0.0f, 1.0f };
static float materialEmission[]			= { 0.0f, 0.0f, 0.0f, 0.0f };
static float materialAmbient[]			= { 0.0f, 0.0f, 0.0f, 1.0f };
static float materialDiffuse[]			= { 1.0f, 1.0f, 1.0f, 1.0f };
static float materialSpecular[]			= { 0.0f, 0.0f, 0.0f, 1.0f };


static std::vector<std::string> gGLTextureNames;
static std::vector<TextureHandle> gGLTextures;

TextureHandle g_exhaust_th;


#define CHECK_GL_ERROR() ({ GLenum __error = glGetError(); if(__error) printf("OpenGL error 0x%04X in %s\n", __error, __FUNCTION__); (__error ? false : true); })


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
#define GetGLErrorString(a) a
#define GetGLError()									\
{														\
	GLenum err = glGetError();							\
	while (err != GL_NO_ERROR) {						\
		printf("GLError %d set in File:%s Line:%d\n",	\
				GetGLErrorString(err),					\
				__FILE__,								\
				__LINE__);								\
		err = glGetError();								\
	}													\
}
*/


#pragma mark GLES1 and GLES2 blt

#ifdef GLES1
#else // !GLES1
unsigned int ShaderLoadSourceFromMemory(	const char* pszShaderCode, 
											const GLenum Type, 
											GLuint* const pObject)
											//string const pReturnError)
{
	// Create the shader object.
    *pObject = glCreateShader(Type);
	
	// Load the source code into it.
    glShaderSource(*pObject, 1, &pszShaderCode, NULL);
	
	// Compile the source code.
    glCompileShader(*pObject);

	// Test if compilation succeeded.
	GLint bShaderCompiled;
    glGetShaderiv(*pObject, GL_COMPILE_STATUS, &bShaderCompiled);
	if (!bShaderCompiled)
	{
		// There was an error here, first get the length of the log message.
		int i32InfoLogLength, i32CharsWritten;
		glGetShaderiv(*pObject, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		
		// Allocate enough space for the message, and retrieve it.
		char* pszInfoLog = new char[i32InfoLogLength];
        glGetShaderInfoLog(*pObject, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
		
		// Displays the error!
printf("Failed to compile shader: %s\n", pszInfoLog);
		delete [] pszInfoLog;
		
		// Delete shader.
		glDeleteShader(*pObject);
		
		// Return false, couldn't compile.
		return 0;
	}
// There was an error here, first get the length of the log message.
int i32InfoLogLength, i32CharsWritten;
glGetShaderiv(*pObject, GL_INFO_LOG_LENGTH, &i32InfoLogLength);

// Allocate enough space for the message, and retrieve it.
char* pszInfoLog = new char[i32InfoLogLength];
glGetShaderInfoLog(*pObject, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
printf("Compiled shader: %s\n", pszInfoLog);
	
	return 1;
}

unsigned int CreateProgram(	GLuint* const pProgramObject, 
							const GLuint VertexShader, 
							const GLuint FragmentShader, 
							const char** const pszAttribs,
							const int i32NumAttribs)
							//CString* const pReturnError)
{
	// Create the shader program.
	*pProgramObject = glCreateProgram();

	// Attach the fragment and vertex shaders to it.
	glAttachShader(*pProgramObject, FragmentShader);
	glAttachShader(*pProgramObject, VertexShader);

	// For every member in pszAttribs, bind the proper attributes.
	for (int i = 0; i < i32NumAttribs; ++i)
	{
		glBindAttribLocation(*pProgramObject, i, pszAttribs[i]);
	}

	// Link the program object
	glLinkProgram(*pProgramObject);
	
	// Check if linking succeeded.
	GLint bLinked;
	glGetProgramiv(*pProgramObject, GL_LINK_STATUS, &bLinked);
	if (!bLinked)
	{
		int i32InfoLogLength, i32CharsWritten;
		glGetProgramiv(*pProgramObject, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		char* pszInfoLog = new char[i32InfoLogLength];
		glGetProgramInfoLog(*pProgramObject, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
		// *pReturnError = CString("Failed to link: ") + pszInfoLog + "\n";
printf("Failed to link: %s\n", pszInfoLog);
		delete [] pszInfoLog;
		return 0;
	}
int i32InfoLogLength, i32CharsWritten;
glGetProgramiv(*pProgramObject, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
char* pszInfoLog = new char[i32InfoLogLength];
glGetProgramInfoLog(*pProgramObject, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
// *pReturnError = CString("Failed to link: ") + pszInfoLog + "\n";
printf("Linked: %s\n", pszInfoLog);

	// Actually use the created program.
	glUseProgram(*pProgramObject);

	return 1;
}







// This ties in with the shader attribute to link to openGL, see pszVertShader.
static const char* pszAttribs[] = { "myVertex", "myTexCoord" };
static const char* pszAttribs_blt2[] = { "myVertex", "myTexCoord", "myTex2Coord" };
static const char* pszAttribs_fill[] = { "myVertex" };

// Declare the fragment and vertex shaders for blt_predraw()/blt_rect()
static GLuint uiFragShader, uiVertShader;		// Used to hold the fragment and vertex shader handles
static GLuint uiProgramObject;					// Used to hold the program handle (made out of the two previous shaders)
// Handles for the uniform variables.
static int PMVMatrixHandle;
static int TextureHandle;
static int ColorHandle;

static GLuint uiFragShader_blt2, uiVertShader_blt2;		// Used to hold the fragment and vertex shader handles
static GLuint uiProgramObject_blt2;					// Used to hold the program handle (made out of the two previous shaders)
// Handles for the uniform variables.
static int PMVMatrixHandle_blt2;
static int TextureHandle_blt2;
static int Texture2Handle_blt2;
static int ColorHandle_blt2;

// now the fill variant:
// Declare the fragment and vertex shaders for fill_predraw()/fill_rect()
static GLuint uiFragShader_fill, uiVertShader_fill;
static GLuint uiProgramObject_fill;
// Handles for the uniform variables.
static int PMVMatrixHandle_fill;
static int ColorHandle_fill;
	

static char blt_vsh[] = "\
attribute highp vec4	myVertex;											\
attribute highp vec2	myTexCoord;											\
																			\
uniform mediump mat4	myPMVMatrix;										\
																			\
varying mediump vec2	v_textureCoord;										\
																			\
void main(void)																\
{																			\
	gl_Position = myPMVMatrix * myVertex;									\
	v_textureCoord = myTexCoord;											\
}																			\
";

static char blt_fsh[] = "\
varying mediump vec2	v_textureCoord;										\
uniform mediump vec4	u_color;											\
uniform sampler2D		s_texture;											\
																			\
void main(void)																\
{																			\
	gl_FragColor = texture2D(s_texture, v_textureCoord) * u_color;			\
}																			\
";



static char blt2_vsh[] = "\
attribute highp vec4	myVertex;											\
attribute highp vec2	myTexCoord;											\
attribute highp vec2	myTex2Coord;										\
																			\
uniform mediump mat4	myPMVMatrix;										\
																			\
varying mediump vec2	v_textureCoord;										\
varying mediump vec2	v_texture2Coord;									\
																			\
void main(void)																\
{																			\
	gl_Position = myPMVMatrix * myVertex;									\
	v_textureCoord = myTexCoord;											\
	v_texture2Coord = myTex2Coord;											\
}																			\
";

static char blt2_fsh[] = "\
varying mediump vec2	v_textureCoord;										\
varying mediump vec2	v_texture2Coord;									\
uniform mediump vec4	u_color;											\
uniform sampler2D		s_texture;											\
uniform sampler2D		s_texture2;											\
																			\
void main(void)																\
{																			\
	lowp vec4 temp = texture2D(s_texture2, v_texture2Coord) * u_color;		\
	temp = temp * texture2D(s_texture, v_textureCoord).a * 0.8;				\
	gl_FragColor = temp;													\
}																			\
";


static char fill_vsh[] = "\
attribute highp vec4	myVertex;											\
																			\
uniform mediump mat4	myPMVMatrix;										\
																			\
void main(void)																\
{																			\
	gl_Position = myPMVMatrix * myVertex;									\
}																			\
";

static char fill_fsh[] = "\
uniform mediump vec4	u_color2;											\
																			\
void main(void)																\
{																			\
	gl_FragColor = u_color2;												\
}																			\
";



void blt_init() {
	if(ShaderLoadSourceFromMemory(blt_fsh, GL_FRAGMENT_SHADER, &uiFragShader) == 0)
		printf("Loading the fragment shader fails");
	if(ShaderLoadSourceFromMemory(blt_vsh, GL_VERTEX_SHADER, &uiVertShader) == 0)
		printf("Loading the vertex shader fails");
	
	CreateProgram(&uiProgramObject, uiVertShader, uiFragShader, pszAttribs, 2);
	
	// First gets the location of that variable in the shader using its name
	PMVMatrixHandle = glGetUniformLocation(uiProgramObject, "myPMVMatrix");
	ColorHandle = glGetUniformLocation(uiProgramObject, "u_color");			
	TextureHandle = glGetUniformLocation(uiProgramObject, "s_texture");			
//
	if(ShaderLoadSourceFromMemory(blt2_fsh, GL_FRAGMENT_SHADER, &uiFragShader_blt2) == 0)
		printf("Loading the fragment shader blt2 fails");
	if(ShaderLoadSourceFromMemory(blt2_vsh, GL_VERTEX_SHADER, &uiVertShader_blt2) == 0)
		printf("Loading the vertex shader blt2 fails");
	
	CreateProgram(&uiProgramObject_blt2, uiVertShader_blt2, uiFragShader_blt2, pszAttribs_blt2, 2);
	
	// First gets the location of that variable in the shader using its name
	PMVMatrixHandle_blt2 = glGetUniformLocation(uiProgramObject, "myPMVMatrix");
	ColorHandle_blt2 = glGetUniformLocation(uiProgramObject, "u_color");			
	TextureHandle_blt2 = glGetUniformLocation(uiProgramObject, "s_texture");			
	Texture2Handle_blt2 = glGetUniformLocation(uiProgramObject, "s_texture2");			
//
	if(ShaderLoadSourceFromMemory(fill_fsh, GL_FRAGMENT_SHADER, &uiFragShader_fill) == 0)
		printf("Loading the fragment shader fill fails");
	if(ShaderLoadSourceFromMemory(fill_vsh, GL_VERTEX_SHADER, &uiVertShader_fill) == 0)
		printf("Loading the vertex shader fill fails");
	
	CreateProgram(&uiProgramObject_fill, uiVertShader_fill, uiFragShader_fill, pszAttribs_fill, 1);
	
	// First gets the location of that variable in the shader using its name
	PMVMatrixHandle_fill = glGetUniformLocation(uiProgramObject_fill, "myPMVMatrix");
	ColorHandle_fill = glGetUniformLocation(uiProgramObject_fill, "u_color2");			
}

//void blt_cleanup() {
//	// Frees the OpenGL handles for the program and the 2 shaders
//	glDeleteProgram(uiProgramObject);
//	glDeleteShader(uiFragShader);
//	glDeleteShader(uiVertShader);
//}

void blt_predraw(float *m, GLint texture, float r, float g, float b, float a) {
	glUseProgram(uiProgramObject);
	glUniformMatrix4fv(PMVMatrixHandle, 1, GL_FALSE, m);
	glUniform4f(ColorHandle, r, g, b, a);
	glUniform1i(TextureHandle, 0);//texture);
}

void blt2_predraw(float *m, GLint texture, GLint texture2, float r, float g, float b, float a) {
	glUseProgram(uiProgramObject_blt2);
	glUniformMatrix4fv(PMVMatrixHandle_blt2, 1, GL_FALSE, m);
	glUniform4f(ColorHandle_blt2, r, g, b, a);
	glUniform1i(TextureHandle_blt2, 0);//texture);
	glUniform1i(Texture2Handle_blt2, 1);//texture2);
}

void fill_predraw(float *m, float r, float g, float b, float a) {
	glUseProgram(uiProgramObject_fill);
	glUniformMatrix4fv(PMVMatrixHandle_fill, 1, GL_FALSE, m);
	glUniform4f(ColorHandle_fill, r, g, b, a);
}
#endif // !GLES1




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//bool DBlt_saved_tex;
//TextureHandle DBlt_saved_th;
//float DBlt_saved_vpx, DBlt_saved_vpy, DBlt_saved_vpw, DBlt_saved_vph;
//float DBlt_saved_scx, DBlt_saved_scy, DBlt_saved_scw, DBlt_saved_sch;
//bool DBlt_saved_sc_on;
//GLvoid *DBlt_saved_vert_ptr;
//GLint DBlt_saved_vert_size;
//GLenum DBlt_saved_vert_type;
//GLsizei DBlt_saved_vert_stride;
//bool DBlt_saved_tex_array;
//GLvoid *DBlt_saved_tc_ptr;
//GLint DBlt_saved_tc_size;
//GLenum DBlt_saved_tc_type;
//GLsizei DBlt_saved_tc_stride;

void HAL_OGL::DBlt_draw_setup(float *vertices, float *coordinates, short *indices, int nquads) {
//	DBlt_saved_tex = glIsEnabled(GL_TEXTURE_2D);
//	glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&DBlt_saved_th);
//	glGetIntegerv(GL_VERTEX_ARRAY_SIZE, (GLint*)&DBlt_saved_vert_size);
//	glGetIntegerv(GL_VERTEX_ARRAY_TYPE, (GLint*)&DBlt_saved_vert_type);
//	glGetIntegerv(GL_VERTEX_ARRAY_STRIDE, (GLint*)&DBlt_saved_vert_stride);
//	glGetPointerv(GL_VERTEX_ARRAY_POINTER, (void **)&DBlt_saved_vert_ptr);
//	DBlt_saved_tex_array = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
//	glGetIntegerv(GL_TEXTURE_COORD_ARRAY_SIZE, (GLint*)&DBlt_saved_tc_size);
//	glGetIntegerv(GL_TEXTURE_COORD_ARRAY_TYPE, (GLint*)&DBlt_saved_tc_type);
//	glGetIntegerv(GL_TEXTURE_COORD_ARRAY_STRIDE, (GLint*)&DBlt_saved_tc_stride);
//	glGetPointerv(GL_TEXTURE_COORD_ARRAY_POINTER, (void **)&DBlt_saved_tc_ptr);
//	viewport_get(&DBlt_saved_vpx, &DBlt_saved_vpy, &DBlt_saved_vpw, &DBlt_saved_vph);
//	scissor_get(&DBlt_saved_sc_on, &DBlt_saved_scx, &DBlt_saved_scy, &DBlt_saved_scw, &DBlt_saved_sch);

	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
/*
	TShaderDef::gDXShaders[idx_UVcC]->setup_vbuffer(0, &vertices[0], 4*nquads, sizeof(float3), 0);
	TShaderDef::gDXShaders[idx_UVcC]->setup_vbuffer(1, &coordinates[0], 4*nquads, sizeof(float2), 0);
	TShaderDef::gDXShaders[idx_UVcC]->setup_ibuffer(&indices[0], 6*nquads, sizeof(short));
	TShaderDef::gDXShaders[idx_UVcC]->setup_render(false);
*/
}
void HAL_OGL::DBlt_draw_blt_array(TextureHandle th, int nverts) {
	glBindTexture(GL_TEXTURE_2D, th);
	if (th == 0) glDisable(GL_TEXTURE_2D);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, nverts);
	if (th == 0) glEnable(GL_TEXTURE_2D);
}
void HAL_OGL::DBlt_draw_blt(TextureHandle th, short *indices, int wquad, int nquads) {
//if (rand() & 1) glDisable(GL_BLEND); else glEnable(GL_BLEND);
//if (rand() & 1) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); else glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, th);
	if (th == 0) glDisable(GL_TEXTURE_2D);
	glDrawElements(GL_TRIANGLES, nquads*6, GL_UNSIGNED_SHORT, indices);
	if (th == 0) glEnable(GL_TEXTURE_2D);
/*
	if (gConstantBufferDirty) {
		g_d3dContext->UpdateSubresource(gConstantBuffer.Get(), 0, NULL, gConstantBufferData, 0, 0);
		gConstantBufferDirty = false;
	}

	g_d3dContext->VSSetConstantBuffers(
		0,
		1,
		gConstantBuffer.GetAddressOf()
		);

#ifdef DEFER_BLEND_STATE
	if (gDeferredBlendState_dirty) {
		gDeferredBlendState_st->Release();
		g_d3dDevice->CreateBlendState(&gDeferredBlendState, &gDeferredBlendState_st);
		gDeferredBlendState_dirty = false;
	}
	g_d3dContext->OMSetBlendState(gDeferredBlendState_st, gDeferredBlendState_blendFactor, gDeferredBlendState_mask);
#endif // DEFER_BLEND_STATE

	g_d3dContext->DrawIndexed(db->rects.size()*6, 6*wquad, 0);
*/
}
void HAL_OGL::DBlt_draw_cleanup() {
//	glBindTexture(GL_TEXTURE_2D, DBlt_saved_th);
//	if (DBlt_saved_tex) {
//		glEnable(GL_TEXTURE_2D);
//	} else glDisable(GL_TEXTURE_2D);
//	if (DBlt_saved_tex_array) {
//		glEnable(GL_TEXTURE_COORD_ARRAY);
//		glTexCoordPointer(DBlt_saved_tc_size, DBlt_saved_tc_type, DBlt_saved_tc_stride, DBlt_saved_tc_ptr);
//	} else glDisable(GL_TEXTURE_COORD_ARRAY);
//	glVertexPointer(DBlt_saved_vert_size, DBlt_saved_vert_type, DBlt_saved_vert_stride, DBlt_saved_vert_ptr);
//	if (DBlt_saved_sc_on) scissor(DBlt_saved_scx, DBlt_saved_scy, DBlt_saved_scw, DBlt_saved_sch); else scissor_disable();
//	viewport(DBlt_saved_vpx, DBlt_saved_vpy, DBlt_saved_vpw, DBlt_saved_vph);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma mark HAL_OGL

void HAL_OGL::matrix_notify_changed(TMatrix &m, int modelProjView) {
	tos_matrix[modelProjView] = m;
	switch (modelProjView) {
		case 0:
		case 2:
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(tos_matrix[2].f);
			glMultMatrixf(tos_matrix[0].f);
			break;
		case 1:
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(m.f);
			glMatrixMode(GL_MODELVIEW);
			break;
	}
}
void HAL_OGL::matrix_notify_get(TMatrix &m, int modelProjView) {
	m = tos_matrix[modelProjView];
}

void HAL_OGL::init() {
	TMatrix::identity(tos_matrix[0]);
	TMatrix::identity(tos_matrix[1]);
	TMatrix::identity(tos_matrix[2]);

	// Sets the clear color
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	// Enables lighting. See BasicTnL for a detailed explanation
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glMatrixMode(GL_MODELVIEW);

//	int w, h;
	g_exhaust_th = 0;//texture_load(false, "exhaust.png", &w, &h);		// doesn't work here, defer until needed

#ifdef PVR_SUPPORT
	HAL_PVR::init();
#endif // PVR_SUPPORT
}
void HAL_OGL::cleanup() {
#ifdef PVR_SUPPORT
	HAL_PVR::cleanup();
#endif // PVR_SUPPORT
}
void HAL_OGL::new_frame_start() {
	glEnable(GL_DEPTH_TEST);

//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDepthMask(true);
//#ifdef FAKE_SCREEN_SIZE_TEST
//	glViewport(0, 0, gScreenWidth_real, gScreenHeight_real);
//	glScissor(0, 0, gScreenWidth_real, gScreenHeight_real);
//	glEnable(GL_SCISSOR_TEST);
//	glClear(GL_COLOR_BUFFER_BIT);
//#endif // FAKE_SCREEN_SIZE_TEST
	glClear(GL_DEPTH_BUFFER_BIT);
glDisable(GL_DEPTH_TEST); glDepthMask(false);
//#ifdef FAKE_SCREEN_SIZE_TEST
//	glViewport(0, 0, gScreenWidth, gScreenHeight);
//	glScissor(0, 0, gScreenWidth, gScreenHeight);
//	glDisable(GL_SCISSOR_TEST);
//#endif // FAKE_SCREEN_SIZE_TEST
}
void HAL_OGL::new_frame_end() {
#ifdef IMMEDIATE_RENDERING
#else // !IMMEDIATE_RENDERING
	DBlt_flush();
#endif // !IMMEDIATE_RENDERING
}
void HAL_OGL::viewport(float x, float y, float width, float height) {
	glViewport((int)x, (int)y, (int)width, (int)height);
}
void HAL_OGL::viewport_get(float *x, float *y, float *width, float *height) {
	GLint box[4];
	glGetIntegerv(GL_VIEWPORT, box);
	*x = box[0];
	*y = box[1];
	*width = box[2];
	*height = box[3];
}
void HAL_OGL::scissor(float x, float y, float width, float height) {
	glScissor((int)x, (int)y, (int)width, (int)height);
	glEnable(GL_SCISSOR_TEST);
}
void HAL_OGL::scissor_disable() {
	glDisable(GL_SCISSOR_TEST);
}
void HAL_OGL::scissor_get(bool *isOn, float *x, float *y, float *width, float *height) {
	GLint box[4];
	glGetIntegerv(GL_SCISSOR_BOX, box);
	*x = box[0];
	*y = box[1];
	*width = box[2];
	*height = box[3];
	*isOn = glIsEnabled(GL_SCISSOR_TEST);
}
void HAL_OGL::material_diffuse_get(float *r, float *g, float *b, float *a) {
	*r = materialDiffuse[0];
	*g = materialDiffuse[1];
	*b = materialDiffuse[2];
	*a = materialDiffuse[3];
}
void HAL_OGL::set_mode(int mode, float r, float g, float b, float a) {
	switch (mode) {
		case 0:								// start of TBezel::drawBezel(), drawing sprites from an atlas, which are premultiplied by XCode
											// anybody about to call TAtlas::draw_atlas_texture_f(), TAtlas::draw_atlas_texture_c(), TAtlas::draw_atlas_texture_9slice()
											// all atlas sprites go through here!!!
#ifdef PCport			// support for non-premultiplied textures (mode 0)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#else // !PCport
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif // !PCport
			glColor4f(r, g, b, a);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			break;
		case 1:								// middle of TBezel::drawBezel(), now we're drawing fonts, which are not premultiplied
											// anybody about to call draw_stringC(), draw_stringCN(), draw_stringW(), draw_stringWN()
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(r, g, b, a);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			break;
		case 2:								// for a non-PNG file or other data NOT pre-multiplied by XCode, such as 5x5 font
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(r, g, b, a);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			break;
		case 3:								// end of TBezel::drawBezel()
			glBindTexture(GL_TEXTURE_2D, 0);			// this prevents PC OpenGL from stomping on the last used texture (the button font) when loading a skybox...
			glColor4f(r, g, b, a);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			break;
		case 4:								// additive sprites, which might be pre-multiplied by XCode, such as shields
#ifdef PCport			// additive sprites noop
			glBlendFunc(GL_ONE, GL_ONE);
#else // !PCport
			glBlendFunc(GL_ONE, GL_ONE);
#endif // !PCport
			glColor4f(r, g, b, a);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			break;
		case 5:								// setup for rendering that seems to require a clean depth buffer
			glDepthMask(true);
			glClear(GL_DEPTH_BUFFER_BIT);
			break;
		case 6:								// GAMESTATE_FLY_MISSION render setup requires clearing depth and color buffers
			glDepthMask(true);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			break;
		case 7:								// exhaust, additive but in 3D, using depth buffer but not writing it
#ifdef PCport			// additive sprites noop
			glBlendFunc(GL_ONE, GL_ONE);
#else // !PCport
			glBlendFunc(GL_ONE, GL_ONE);
#endif // !PCport
			glColor4f(r, g, b, a);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glDepthMask(false);
			break;

		case 8:		// stars
#ifdef PCport
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else // !PCport
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif // !PCport
			glColor4f(r, g, b, a);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			break;
		case 9:
			break;

		case 10:							// subtractive, not additive
#ifdef PCport			// additive sprites noop
			glBlendFunc(GL_ONE, GL_ONE);
#else // !PCport
			glBlendFunc(GL_ONE, GL_ONE);
#endif // !PCport
			glColor4f(r, g, b, a);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			glBlendEquationOES(GL_FUNC_REVERSE_SUBTRACT_OES);
			break;
		case 11:							// undo subtractive
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquationOES(GL_FUNC_ADD_OES);
			break;

		case 20:							// start of draw_stringC(), draw_stringCN(), draw_stringW(), draw_stringWN()
			glActiveTexture(GL_TEXTURE0);
//glEnable(GL_TEXTURE_2D);
//			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef PCport
//??? are fonts looking good on the edges???
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else // !PCport
//??? are fonts looking good on the edges???
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#warning this option keeps changing!!!
//			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif // !PCport
			glColor4f(r, g, b, a);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			break;
		case 21:							// end of draw_stringC(), draw_stringCN(), draw_stringW(), draw_stringWN()
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case 22:							// 
			glActiveTexture(GL_TEXTURE0);
//glEnable(GL_TEXTURE_2D);
//			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef PCport
//??? are fonts looking good on the edges???
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else // !PCport
//??? are fonts looking good on the edges???
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif // !PCport
			glColor4f(r, g, b, a);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			break;

		case 25:							// draw_HUD_grid() just making sure (shouldn't be modulated anyway...)
											// crosshairs, colorizing 5x5 font
			glColor4f(r, g, b, a);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case 26:							// before calls to drawBezel()
											// popup scroll arrows
			glEnable(GL_BLEND);
			break;
		case 27:
			glDisable(GL_BLEND);
			break;

		case 31:							// start of GAMESTATE_DEATH2
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			glDepthMask(false);
			glEnable(GL_BLEND);
			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			break;

		case 100:								// start of HUD_draw()
			glEnable(GL_BLEND);
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
#ifndef PCport			// point sprites not implemented on PC OpenGL
			glDisable(GL_POINT_SPRITE_OES);
			glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
#endif // !PCport
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisable(GL_DEPTH_TEST);
			glDepthMask(false);
			break;
		case 101:								// end of HUD_draw()
			glEnableClientState(GL_NORMAL_ARRAY);
			glEnable(GL_DEPTH_TEST);
			glDepthMask(true);
glDisable(GL_DEPTH_TEST); glDepthMask(false);
			break;

		case 999:
			glFlush();
			break;
	}
}
void HAL_OGL::set_masking(bool write_color_buffer, bool write_depth_buffer, int depth_less_or_equal) {		// -1=not set; 0=less; 1=lequal
	glColorMask(write_color_buffer, write_color_buffer, write_color_buffer, write_color_buffer);
	glDepthMask(write_depth_buffer);
	switch (depth_less_or_equal) {
		case 0:			glDepthFunc(GL_LESS);		break;
		case 1:			glDepthFunc(GL_LEQUAL);		break;
	}
}
void HAL_OGL::set_culling(bool enable, bool back) {
	if (enable) {
		glEnable(GL_CULL_FACE);
		glCullFace(back?GL_BACK:GL_FRONT);
	} else {
		glDisable(GL_CULL_FACE);
	}
}

//

void HAL_OGL::texture_bind(int unit, unsigned int texhandle) {
	glBindTexture(GL_TEXTURE_2D, texhandle);
}
void HAL_OGL::texture_blur(int unit) {
}
void HAL_OGL::texture_setup_repeat(TextureHandle texhandle) {
	GLint					saveName;

	glGetIntegerv(GL_TEXTURE_BINDING_2D, &saveName);
	glBindTexture(GL_TEXTURE_2D, texhandle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glBindTexture(GL_TEXTURE_2D, saveName);
}

void HAL_OGL::texture_setup_mipmap(TextureHandle texhandle) {
	GLint					saveName;

	glGetIntegerv(GL_TEXTURE_BINDING_2D, &saveName);
	glBindTexture(GL_TEXTURE_2D, texhandle);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifndef PCport			// GL_GENERATE_MIPMAP not implemented on PC OpenGL
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);			// actually, this SHOULD be called if the file was not a PVR (that should already have mipmaps)
#endif // PCport

	glBindTexture(GL_TEXTURE_2D, saveName);
}

void HAL_OGL::texture_setup_clamp_to_edge(TextureHandle texhandle) {
	GLint					saveName;

	glGetIntegerv(GL_TEXTURE_BINDING_2D, &saveName);
	glBindTexture(GL_TEXTURE_2D, texhandle);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_2D, saveName);
}
TextureHandle HAL_OGL::texture_make(int mode, int width, int height, const void *data) {
	TextureHandle texName;
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	switch (mode) {
		case 0:		// USE_ALPHA_MODE_FOR_FONTS 
			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
			break;
		case 1:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
			break;
	}
	return texName;
}
TextureHandle HAL_OGL::texture_process_font(int w, int h, const void *data) {
	GLuint gn;
	GLint saveName;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &saveName);
	glGenTextures(1, &gn);
	glBindTexture(GL_TEXTURE_2D, gn);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, saveName);
	return gn;
}
TextureHandle HAL_OGL::texture_process_argb(int w, int h, const void *data, bool mipmap) {
	GLuint gn;
	GLint saveName;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &saveName);
	glGenTextures(1, &gn);
	glBindTexture(GL_TEXTURE_2D, gn);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifndef PCport			// GL_GENERATE_MIPMAP not implemented on PC OpenGL
	if (mipmap) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);			// actually, this SHOULD be called if the file was not a PVR (that should already have mipmaps)
#endif // PCport
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, saveName);
	return gn;
}

TextureHandle HAL_OGL::texture_find(bool is_font, const char *fname, int *out_width, int *out_height, std::string *ret_found_fname) {
	return 0;
}
#ifdef ANDROID_NDK
TextureHandle HAL_OGL::texture_load(bool is_font, const char *fname, int *out_width, int *out_height, std::string *ret_found_fname, bool needs_mipmap) {
	TextureHandle gn = 0;
    if (is_font) {
printf("ERROR: HAL_OGL::texture_load() not expecting font data\n");
	} else {
		for (int i=0; i<loaded_textures.size(); i++) {
			if (strcmp(fname, loaded_textures[i].fname.c_str()) == 0) {
				*out_width = loaded_textures[i].w;
				*out_height = loaded_textures[i].h;
		        if (ret_found_fname != NULL) *ret_found_fname = loaded_textures[i].found_fname;
//
printf("HAL_OGL::texture_load(): '%s' MATCHED %dx%d found='%s' gn=%d\n", loaded_textures[i].fname.c_str(), loaded_textures[i].w, loaded_textures[i].h, loaded_textures[i].found_fname.c_str(), loaded_textures[i].gn);
				return loaded_textures[i].gn;
			}
		}
		//
		int w, h;
		load_texture_file(fname, &gn, &w, &h, false);
        if (gn == 0) {
printf("HAL_OGL::texture_load(): '%s' failed\n", fname);
			return 0;
		}
		if (out_width != NULL) *out_width = w;
		if (out_height != NULL) *out_height = h;
		if (ret_found_fname != NULL) *ret_found_fname = fname;
		//
		TTextureLoaded t;
		t.fname = fname;
		t.found_fname = fname;
		t.w = w;
		t.h = h;
		t.gn = gn;
		loaded_textures.push_back(t);
//
printf("HAL_OGL::texture_load(): '%s' NEW %dx%d found='%s' gn=%d\n", fname, w, h, fname, gn);
	}
	return gn;
}
#else // !ANDROID_NDK
TextureHandle HAL_OGL::texture_load(bool is_font, const char *fname, int *out_width, int *out_height, std::string *ret_found_fname, bool needs_mipmap) {
	TextureHandle gn = 0;
    if (is_font) {
        int w, h;
		std::string found_fname;
		if (!file_exists(fname, &found_fname)) return 0;
//        FILE *fd = HAL::open_file(fname, &found_fname);
//        if (fd == NULL) return 0;
//        fclose(fd);
        int *m = read_file_png(found_fname.c_str(), &w, &h);
        if (m == NULL) return 0;
        unsigned char *mm = (unsigned char *)malloc(w * h);

//for (int j=0; j<w*h; j++) m[j] = ((rand() << 17) + (rand() << 2)) | 0xff000000;
//for (int j=0; j<w*h; j++) { int a = 0x0ff & (m[j] >> 24); m[j] = (a << 24) + (a << 16) + (a << 8) + a; }		// actually, this is a single-channel premultiply...
        for (int j=0; j<w*h; j++) { unsigned char a = 0x0ff & (m[j] >> 24); mm[j] = a; }

        gn = texture_process_font(w, h, mm);
        *out_width = w;
        *out_height = h;
        free(m);
        free(mm);
        if (ret_found_fname != NULL) *ret_found_fname = found_fname;
    } else {
		for (int i=0; i<loaded_textures.size(); i++) {
			if (strcmp(fname, loaded_textures[i].fname.c_str()) == 0) {
				*out_width = loaded_textures[i].w;
				*out_height = loaded_textures[i].h;
		        if (ret_found_fname != NULL) *ret_found_fname = loaded_textures[i].found_fname;
//
printf("HAL_OGL::texture_load(): '%s' MATCHED %dx%d found='%s' gn=%d\n", loaded_textures[i].fname.c_str(), loaded_textures[i].w, loaded_textures[i].h, loaded_textures[i].found_fname.c_str(), loaded_textures[i].gn);
				return loaded_textures[i].gn;
			}
		}
		//
    	int w, h;
		std::string found_fname;
		if (!file_exists(fname, &found_fname)) {
printf("HAL_OGL::texture_load(): '%s' not found\n", fname);
			return 0;
		}
//        FILE *fd = HAL::open_file(fname, &found_fname);
//        if (fd == NULL) return 0;
//        fclose(fd);
        int *m = read_file_png(found_fname.c_str(), &w, &h);
        if (m == NULL) {
printf("HAL_OGL::texture_load(): '%s' (%s) not readable\n", fname, found_fname.c_str());
			return 0;
		}

//for (int j=0; j<w*h; j++) { int a = 0x0ff & (m[j] >> 24); m[j] = (a << 24) + (a << 16) + (a << 8) + a; }		// actually, this is a single-channel premultiply...
        for (int j=0; j<w*h; j++) {
            int a = 0x0ff & (m[j] >> 24);
            int b = 0x0ff & (m[j] >> 16);
            int g = 0x0ff & (m[j] >>  8);
            int r = 0x0ff & (m[j]      );
            r = (r * a) >> 8;
            g = (g * a) >> 8;
            b = (b * a) >> 8;
            m[j] = (a << 24) + (r << 16) + (g << 8) + b;
//if (j<200) {
//printf("%08x ", m[j]);
//}
        }

        gn = texture_process_argb(w, h, m);
        free(m);
        *out_width = w;
        *out_height = h;
		//
		TTextureLoaded t;
		t.fname = fname;
		t.found_fname = found_fname;
		t.w = w;
		t.h = h;
		t.gn = gn;
		loaded_textures.push_back(t);
//
printf("HAL_OGL::texture_load(): '%s' NEW %dx%d found='%s' gn=%d\n", fname, w, h, found_fname.c_str(), gn);
    }
    return gn;
}
#endif // !ANDROID_NDK

void HAL_OGL::texture_unload(TextureHandle th) {
	glDeleteTextures(1, &th);
}
int HAL_OGL::texture_max_size() {
    int maxuv;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxuv);
	return maxuv;
}

//

void HAL_OGL::dblt_rect(float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy, TextureHandle th, int mode, float r, float g, float b, float a) {
//TDeferredBlt *blt = 
	DBlt_add(x, y, dx, dy, tx, ty, tdx, tdy, th, mode, r, g, b, a);
}

void HAL_OGL::dfill_rect(float x, float y, float dx, float dy, int mode, float r, float g, float b, float a) {
#ifdef never
#ifdef GLES1
	glDisable(GL_TEXTURE_2D);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, nv);
	glEnable(GL_TEXTURE_2D);
#else // !GLES1
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
glBindVertexArray(0);

//	if (__OPENGLES_VERSION >= 2)
	{

		glEnableVertexAttribArray(0);		// WARNING: this assumes the vertex data will always be #0, tc #1 WARNING
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
		
		glDisableVertexAttribArray(1);
		
		glDisableVertexAttribArray(2);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, nv);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
#endif // !GLES1
#else
//TDeferredBlt *blt = 
	DBlt_add(x, y, dx, dy, mode, r, g, b, a);
#endif
}

//void HAL_OGL::dfill_rect_dy(float x, float y, float dx, float dy, int mode, float r, float g, float b, float a, float rdy) {
//#ifdef IMMEDIATE_RENDERING
//	DBlt_add(x, y, dx, dy, mode, r, g, b, a);
//#else // !IMMEDIATE_RENDERING
//	TDeferredBlt *blt = DBlt_add(x, y, dx, dy, mode, r, g, b, a);
//	blt->rects[0].tty = rdy;
//	blt->tilted_rect = true;
//#endif // !IMMEDIATE_RENDERING
//}




//void HAL::DBlt_add(float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy, TextureHandle th, int mode, float r, float g, float b, float a) {
//	std::vector<float>		vertices;
//	std::vector<float>		coordinates;
////	// fill out raw data
//	vertices.push_back(x);
//	vertices.push_back(y+dy);
//	vertices.push_back(0);
//	vertices.push_back(x);
//	vertices.push_back(y);
//	vertices.push_back(0);
//	vertices.push_back(x+dx);
//	vertices.push_back(y+dy);// + ((db->tilted_rect)?br->tty:0));
//	vertices.push_back(0);
//	vertices.push_back(x+dx);
//	vertices.push_back(y);// + ((db->tilted_rect)?br->tty:0));
//	vertices.push_back(0);
//	coordinates.push_back(tx);
//	coordinates.push_back(ty+tdy);
//	coordinates.push_back(tx);
//	coordinates.push_back(ty);
//	coordinates.push_back(tx+tdx);
//	coordinates.push_back(ty+tdy);
//	coordinates.push_back(tx+tdx);
//	coordinates.push_back(ty);
//	DBlt_draw_setup(&vertices[0], &coordinates[0], NULL, 0);
//	gHAL->set_mode(mode, r, g, b, a);
//	DBlt_draw_blt_array(th, 4);
//	if (mode == 10) gHAL->set_mode(11);					// clean up after mode 10 subtractive sprites
//}




#ifdef IMMEDIATE_RENDERING
TextureHandle blt_this_texture = 0;
int blt_mode;
float blt_r, blt_g, blt_b, blt_a;
#else // !IMMEDIATE_RENDERING
TDeferredBlt *blt_being_built = NULL;
#endif // !IMMEDIATE_RENDERING
void HAL_OGL::dblt_rect_start(TextureHandle th, int mode, float r, float g, float b, float a) {
#ifdef IMMEDIATE_RENDERING
	blt_this_texture = th;
	blt_mode = mode;
	blt_r = r;
	blt_g = g;
	blt_b = b;
	blt_a = a;
#else // !IMMEDIATE_RENDERING
	blt_being_built = DBlt_new(th, mode, r, g, b, a);
#endif // !IMMEDIATE_RENDERING
}

void HAL_OGL::dblt_rect_next(float x, float y, float dx, float dy, float tx, float ty, float tdx, float tdy) {
#ifdef IMMEDIATE_RENDERING
	std::vector<float>		vertices;
	std::vector<float>		coordinates;
//	// fill out raw data
	vertices.push_back(x);
	vertices.push_back(y+dy);
	vertices.push_back(0);
	vertices.push_back(x);
	vertices.push_back(y);
	vertices.push_back(0);
	vertices.push_back(x+dx);
	vertices.push_back(y+dy);// + ((db->tilted_rect)?br->tty:0));
	vertices.push_back(0);
	vertices.push_back(x+dx);
	vertices.push_back(y);// + ((db->tilted_rect)?br->tty:0));
	vertices.push_back(0);
	coordinates.push_back(tx);
	coordinates.push_back(ty+tdy);
	coordinates.push_back(tx);
	coordinates.push_back(ty);
	coordinates.push_back(tx+tdx);
	coordinates.push_back(ty+tdy);
	coordinates.push_back(tx+tdx);
	coordinates.push_back(ty);
	DBlt_draw_setup(&vertices[0], &coordinates[0], NULL, 0);
	set_mode(blt_mode, blt_r, blt_g, blt_b, blt_a);
	DBlt_draw_blt_array(blt_this_texture, 4);
	if (blt_mode == 10) set_mode(11);					// clean up after mode 10 subtractive sprites
#else // !IMMEDIATE_RENDERING
	DBlt_append(blt_being_built, x, y, dx, dy, tx, ty, tdx, tdy);
#endif // !IMMEDIATE_RENDERING
}

#ifdef IMMEDIATE_RENDERING
#else // !IMMEDIATE_RENDERING
void HAL_OGL::dblt_flush() {
	DBlt_flush();
	set_mode(0, 1, 1, 1, 1);
}
#endif // !IMMEDIATE_RENDERING

void HAL_OGL::blt_rect_nonstrip(const float *vertices, const float *coordinates, int num_verts) {
//if (DBlt_num_pending() > 0) {
//printf("!!!\n");
//}
#ifdef GLES1
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
	glDrawArrays(GL_TRIANGLES, 0, num_verts);
#else // !GLES1
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
glBindVertexArray(0);

//	if (__OPENGLES_VERSION >= 2)
	{

		glEnableVertexAttribArray(0);		// WARNING: this assumes the vertex data will always be #0, tc #1 WARNING
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, coordinates);
		
		glDisableVertexAttribArray(2);
	}

	glDrawArrays(GL_TRIANGLES, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
#endif // !GLES1
}

void HAL_OGL::blt_rect(const float *vertices, const float *coordinates, int num_verts) {
//if (DBlt_num_pending() > 0) {
//printf("!!!\n");
//}
#ifdef GLES1
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, num_verts);
#else // !GLES1
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
glBindVertexArray(0);

//	if (__OPENGLES_VERSION >= 2)
	{

		glEnableVertexAttribArray(0);		// WARNING: this assumes the vertex data will always be #0, tc #1 WARNING
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, coordinates);
		
		glDisableVertexAttribArray(2);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
#endif // !GLES1
}

void HAL_OGL::blt_rect8(const float *vertices, const float *coordinates, int num_verts) {
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, num_verts);
}

void HAL_OGL::blt_rect8_color(const float *vertices, const float *coordinates, const float *colors, int num_verts) {
#ifdef IMMEDIATE_RENDERING
#else // !IMMEDIATE_RENDERING
if (DBlt_num_pending() > 0) {
printf("!!!\n");
}
#endif // !IMMEDIATE_RENDERING
#ifdef GLES1
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, num_verts);
#else // !GLES1
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
glBindVertexArray(0);

//	if (__OPENGLES_VERSION >= 2)
	{

		glEnableVertexAttribArray(0);		// WARNING: this assumes the vertex data will always be #0, tc #1, color #2 WARNING
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, coordinates);
		
		glEnableVertexAttribArray(2);
		glColorAttribPointer(2, 4, GL_UNSIGNED_BYTE, 0, colors);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
#endif // !GLES1
}

void HAL_OGL::fill_rect(const float *vertices, int nv) {
#ifdef IMMEDIATE_RENDERING
#else // !IMMEDIATE_RENDERING
if (DBlt_num_pending() > 0) {
printf("!!!\n");
}
#endif // !IMMEDIATE_RENDERING
#ifdef GLES1
	glDisable(GL_TEXTURE_2D);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, nv);
	glEnable(GL_TEXTURE_2D);
#else // !GLES1
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
glBindVertexArray(0);

//	if (__OPENGLES_VERSION >= 2)
	{

		glEnableVertexAttribArray(0);		// WARNING: this assumes the vertex data will always be #0, tc #1 WARNING
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
		
		glDisableVertexAttribArray(1);
		
		glDisableVertexAttribArray(2);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, nv);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
#endif // !GLES1
}

void HAL_OGL::render_dots(int ndots, const float *verts, const float *colors, const float *radii) {
if (g_exhaust_th == 0) { int w, h; g_exhaust_th = texture_load(false, "exhaust.png", &w, &h); }			// deferred from init() because texture_load did not work there

// use blt buffer...
// add each dot to the dblt buffer with DBlt_new()/DBlt_append()
// have to use individual dblt buffer items if they are to be colored?
// or is there room for a dblt2, that carries per-vertex colors

			matrix_push(2);
			matrix_push();
			//
			//
			TMatrix m, m2;
			matrix_get(&m);
			matrix_get(&m2, 2);
			matrix_loadidentity(2);
			TMatrix::multiply(m, m, m2);
			m.f[0] = 1.0f;
			m.f[1] = 0.0f;
			m.f[2] = 0.0f;
			m.f[4] = 0.0f;
			m.f[5] = 1.0f;
			m.f[6] = 0.0f;
			m.f[8] = 0.0f;
			m.f[9] = 0.0f;
			m.f[10] = 1.0f;
			matrix_load(m);

			for (int i=0; i<ndots; i++) {
				matrix_push();
				matrix_translate(verts[3*i+0], verts[3*i+1], 0);
                matrix_rotate(radii[3*i+2], 0, 0, 1);
				matrix_scale(radii[3*i+0]/4, radii[3*i+1]/4, 1);
				//
//				const int Z = 1;
//				matrix_translate(0, 0, -100);
				DBlt_add(1, 1, -2, -2, 0, 0, 1, 1, g_exhaust_th, 4, colors[4*i+0], colors[4*i+1], colors[4*i+2], colors[4*i+3]);
				matrix_pop();
			}

			matrix_pop(2);
			matrix_pop();



/*
if (DBlt_num_pending() > 0) {
printf("!!!\n");
}
	glBindTexture(GL_TEXTURE_2D, 0);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#ifndef PCport		// drawing radar dots: can't use GL_POINT_SIZE_ARRAY_OES on PC OpenGL
	glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
	glPointSizePointerOES(GL_FLOAT, 0, radii);
#endif // !PCport
	glEnable(GL_POINT_SMOOTH);									// round
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
glDisable(GL_LIGHTING);
glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPointParameterf(GL_POINT_SIZE_MIN, 1.0f);					// THIS IS NOT USED ON SIMULATOR, USED ON DEVICE!!!
	glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 24.0f);
	glPointParameterf(GL_POINT_SIZE_MAX, 74.0f);
	//
	glDrawArrays(GL_POINTS, 0, ndots);
	//
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#ifndef PCport			// drawing radar dots: can't use GL_POINT_SIZE_ARRAY_OES on PC OpenGL
	glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
#endif // !PCport
*/
}

unsigned int HAL_OGL::load_PVR(const char *filename, int *ret_width, int *ret_height) {
#ifdef PVR_SUPPORT
	unsigned int glname = HAL_PVR::load_PVR(filename, ret_width, ret_height);
	return glname;
#else // !PVR_SUPPORT
	return 0;
#endif // !PVR_SUPPORT
}

//

VBOHandle HAL_OGL::vbo_make(int sz, const void *data, unsigned int stride, unsigned int offset) {
	VBOHandle rv = 0;
	glGenBuffers(1, &rv);
	glBindBuffer(GL_ARRAY_BUFFER, rv);
	glBufferData(GL_ARRAY_BUFFER, sz, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return rv;
}
IBOHandle HAL_OGL::ibo_make(int sz, const void *data, unsigned int stride, unsigned int offset) {
	IBOHandle rv = 0;
	glGenBuffers(1, &rv);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rv);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return rv;
}
void HAL_OGL::vbo_ibo_bind(VBOHandle vbo, IBOHandle ibo, bool dup_tex_coords) {	// ibo may be 0; dup_tex_coords determines if second texture unit is set up with same vbo
	vbo_bound = vbo;
	ibo_bound = ibo;
	vbo_bound_with_dup_tex_coords = dup_tex_coords;
	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	
	if (vbo_bound_with_dup_tex_coords) {
		glClientActiveTexture(GL_TEXTURE1);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTexture(GL_TEXTURE0);
	}
}
void HAL_OGL::vbo_draw_setup_general(int num_verts, void *interleaved_data, unsigned int vertex_stride, void *vertex_data, unsigned int texcoord_stride, void *texcoord_data, unsigned int normals_stride, void *normals_data) {
	glVertexPointer(3, GL_FLOAT, vertex_stride, vertex_data);
	glTexCoordPointer(2, GL_FLOAT, texcoord_stride, texcoord_data);
	if (vbo_bound_with_dup_tex_coords) {
		glClientActiveTexture(GL_TEXTURE1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, texcoord_stride, texcoord_data);
		glClientActiveTexture(GL_TEXTURE0);
	}
	glNormalPointer(GL_FLOAT, normals_stride, normals_data);
}
void HAL_OGL::vbo_draw(int nFaces, int offset) {
	if (ibo_bound) {  // Indexed Triangle list
		glDrawElements(GL_TRIANGLES, nFaces*3, GL_UNSIGNED_SHORT, &((short*)0)[offset]);
	} else {  // Non-Indexed Triangle list
		glDrawArrays(GL_TRIANGLES, offset, nFaces*3);
	}
}
void HAL_OGL::vbo_draw_strip(int nFaces, int offset) {
	if (ibo_bound) {  // Indexed Triangle list
		glDrawElements(GL_TRIANGLE_STRIP, nFaces+2, GL_UNSIGNED_SHORT, &((short*)0)[offset]);
	} else {  // Non-Indexed Triangle list
		glDrawArrays(GL_TRIANGLE_STRIP, offset, nFaces+2);
	}
}
void HAL_OGL::vbo_ibo_unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	if (vbo_bound_with_dup_tex_coords) {
		glClientActiveTexture(GL_TEXTURE1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTexture(GL_TEXTURE0);
	}
}

//

void HAL_OGL::skybox_delete_texture(TextureHandle t) {
	glDeleteTextures(1, &t);
}
void HAL_OGL::skybox_draw_setup(bool flashing, float a) {
	if (a < 1.0f) {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4f(a, a, a, 1.0f);
		glDisable(GL_BLEND);
	} else {
//		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
//		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);  glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
//		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
//		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
//		//glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
//		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);  glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
//		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
//		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PREVIOUS);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
//		//glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, GL_PREVIOUS); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
		if (flashing) {
			glColor4f(materialDiffuse[0], materialDiffuse[1], materialDiffuse[2], materialDiffuse[3]);
		} else {
			glColor4f(0, 0, 0, materialDiffuse[3]);
		}
		glDisable(GL_BLEND);
	}

//	glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);

    glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
}
void HAL_OGL::skybox_draw(bool flashing, const float *verts, const float *uvs, int num_verts) {
	blt_rect(verts, uvs, num_verts);
}
void HAL_OGL::skybox_draw_nebula_setup(float a) {
	glEnable(GL_BLEND);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT,GL_TEXTURE_LOD_BIAS_EXT,1.0f);
	glColor4f(a, a, a, 0.75f);
}
void HAL_OGL::skybox_draw_nebula_cleanup() {
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT,GL_TEXTURE_LOD_BIAS_EXT,0.0f);
}
void HAL_OGL::skybox_draw_cleanup() {
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
glDisable(GL_DEPTH_TEST); glDepthMask(false);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

//

TextureHandle HAL_OGL::warptube_texture_setup(void *data, int idx) {
	GLint					saveName;
	GLuint					newName;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &saveName);
	glGenTextures(1, &newName);
	glBindTexture(GL_TEXTURE_2D, newName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 64, 64, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ((char *)data)+64*64*idx);
	glBindTexture(GL_TEXTURE_2D, saveName);
	return newName;
}
void HAL_OGL::warptube_draw1_setup(float r, float g, float b, float a) {
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);

	glEnable(GL_BLEND);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glColor4f(r, g, b, a);
}
void HAL_OGL::warptube_draw_cleanup() {
	glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisableClientState(GL_COLOR_ARRAY);
}
void HAL_OGL::warptube_draw2_setup(bool flashing_white, float r, float g, float b, float a) {
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);

	// Disable lighting
	glDisable(GL_LIGHTING);
	
	glDisable(GL_BLEND);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glColor4f(r, g, b, a);


//	glColor4f(0.6f+COLOR_VARIANCE*sin(t*3.7f), 0.7f+COLOR_VARIANCE*cos(t*6.7f), 0.98f+COLOR_VARIANCE*sin(t*5.0f), 0.75f);
	if (flashing_white) {
		glBlendFunc(GL_ONE, GL_ONE);
	} else {
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}
	glEnable(GL_BLEND);
//
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glEnableClientState(GL_COLOR_ARRAY);
}

//

void HAL_OGL::render_setup_color_and_material_for_skybox(bool flashme, int flashcolor, float a) {
	if (flashme) {
		switch (flashcolor) {
			case 0:			// red
				lightAmbient[0] = a;   lightAmbient[1] = 0.0f;   lightAmbient[2] = 0.0f;   lightAmbient[3] = 0.0f;
				lightDiffuse[0] = a;   lightDiffuse[1] = 0.0f;   lightDiffuse[2] = 0.0f;   lightDiffuse[3] = 0.0f;
				lightSpecular[0] = a;  lightSpecular[1] = 0.0f;  lightSpecular[2] = 0.0f;  lightSpecular[3] = 0.0f;
				lightGlobalAmbient[0] = a;  lightSpecular[1] = 0.0f;  lightSpecular[2] = 0.0f;  lightSpecular[3] = 0.0f;
				materialEmission[0] = a;  materialEmission[1] = 0.0f;  materialEmission[2] = 0.0f;  materialEmission[3] = a;
				materialAmbient[0] = a;  materialAmbient[1] = 0.0f;  materialAmbient[2] = 0.0f;  materialAmbient[3] = 0.0f;
				materialDiffuse[0] = a;  materialDiffuse[1] = 0.0f;  materialDiffuse[2] = 0.0f;  materialDiffuse[3] = a;
				materialSpecular[0] = a;  materialSpecular[1] = 0.0f;  materialSpecular[2] = 0.0f;  materialSpecular[3] = 0.0f;
				break;
			case 1:			// blue
				lightAmbient[0] = 0.0f;   lightAmbient[1] = 0.0f;   lightAmbient[2] = a;   lightAmbient[3] = 0.0f;
				lightDiffuse[0] = 0.0f;   lightDiffuse[1] = 0.0f;   lightDiffuse[2] = a;   lightDiffuse[3] = 0.0f;
				lightSpecular[0] = 0.0f;  lightSpecular[1] = 0.0f;  lightSpecular[2] = a;  lightSpecular[3] = 0.0f;
				lightGlobalAmbient[0] = 0.0f;  lightSpecular[1] = 0.0f;  lightSpecular[2] = a;  lightSpecular[3] = 0.0f;
				materialEmission[0] = 0.0f;  materialEmission[1] = 0.0f;  materialEmission[2] = a;  materialEmission[3] = a;
				materialAmbient[0] = 0.0f;  materialAmbient[1] = 0.0f;  materialAmbient[2] = a;  materialAmbient[3] = 0.0f;
				materialDiffuse[0] = 0.0f;  materialDiffuse[1] = 0.0f;  materialDiffuse[2] = a;  materialDiffuse[3] = a;
				materialSpecular[0] = 0.0f;  materialSpecular[1] = 0.0f;  materialSpecular[2] = a;  materialSpecular[3] = 0.0f;
				break;
			case 2:			// white
				lightAmbient[0] = a;   lightAmbient[1] = a;   lightAmbient[2] = a;   lightAmbient[3] = 0.0f;
				lightDiffuse[0] = a;   lightDiffuse[1] = a;   lightDiffuse[2] = a;   lightDiffuse[3] = 0.0f;
				lightSpecular[0] = a;  lightSpecular[1] = a;  lightSpecular[2] = a;  lightSpecular[3] = 0.0f;
				lightGlobalAmbient[0] = a;  lightSpecular[1] = a;  lightSpecular[2] = a;  lightSpecular[3] = 0.0f;
				materialEmission[0] = a;  materialEmission[1] = a;  materialEmission[2] = a;  materialEmission[3] = a;
				materialAmbient[0] = a;  materialAmbient[1] = a;  materialAmbient[2] = a;  materialAmbient[3] = 0.0f;
				materialDiffuse[0] = a;  materialDiffuse[1] = a;  materialDiffuse[2] = a;  materialDiffuse[3] = a;
				materialSpecular[0] = a;  materialSpecular[1] = a;  materialSpecular[2] = a;  materialSpecular[3] = 0.0f;
				break;
		}
	} else {
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		lightAmbient[0] = 0.0f;   lightAmbient[1] = 0.0f;   lightAmbient[2] = 0.0f;   lightAmbient[3] = a;
		lightDiffuse[0] = a;   lightDiffuse[1] = a;   lightDiffuse[2] = a;   lightDiffuse[3] = a;
		lightSpecular[0] = a*0.2f;  lightSpecular[1] = a*0.2f;  lightSpecular[2] = a*0.2f;  lightSpecular[3] = a;
		lightGlobalAmbient[0] = 0.0f;  lightSpecular[1] = 0.0f;  lightSpecular[2] = 0.0f;  lightSpecular[3] = a;
		materialEmission[0] = 0.0f;  materialEmission[1] = 0.0f;  materialEmission[2] = 0.0f;  materialEmission[3] = 0.0f;
		materialAmbient[0] = 0.0f;  materialAmbient[1] = 0.0f;  materialAmbient[2] = 0.0f;  materialAmbient[3] = a;
		materialDiffuse[0] = a;  materialDiffuse[1] = a;  materialDiffuse[2] = a;  materialDiffuse[3] = a;
		materialSpecular[0] = 0.0f;  materialSpecular[1] = 0.0f;  materialSpecular[2] = 0.0f;  materialSpecular[3] = 0.0f;
	}

	render_setup_color_and_material_for_objects();
}

void HAL_OGL::render_setup_color_and_material_for_objects(float *amb) {
	float lightPosition[] = { -1000.0f, -1000.0f, -500.0f, 0.0f };
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, (amb!=NULL)?amb:lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	float lightPosition1[] = { 1000.0f, 1000.0f, 500.0f, 0.0f };
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, (amb!=NULL)?amb:lightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightGlobalAmbient);

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.2f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (amb!=NULL)?amb:materialAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
}

void HAL_OGL::render_setup_color_and_material_for_simple_render() {
	glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
//glClear(GL_DEPTH_BUFFER_BIT);//glDisable(GL_DEPTH_TEST); glDepthMask(false);

	glEnable(GL_NORMALIZE);				// one of these is required when objects are scaled from world coords: COSTS FRAMERATE!!!
//	glEnable(GL_RESCALE_NORMAL);

	render_setup_color_and_material_for_objects(lightBrightAmbient);
//	{
//		float lightPosition[] = { -1000.0f, -1000.0f, -500.0f, 0.0f };
///*
//		float lightAmbient[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
//		float lightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
//		float lightSpecular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
//*/
//		glEnable(GL_LIGHT0);
//		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
//		glLightfv(GL_LIGHT0, GL_AMBIENT, lightBrightAmbient);
//		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
//		glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
//		float lightPosition1[] = { 1000.0f, 1000.0f, 500.0f, 0.0f };
//		glEnable(GL_LIGHT1);
//		glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
//		glLightfv(GL_LIGHT1, GL_AMBIENT, lightBrightAmbient);
//		glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
//		glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
//	}
//	{
///*
//		float lightAmbient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
////		float lightAmbient[]  = { 0.3f, 0.3f, 0.3f, 1.0f };
//		float lightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
//		float lightSpecular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
//*/
//		glEnable(GL_LIGHT0);
//		glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
//		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
//		glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
//
///*
//		float lightGlobalAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
////		float lightGlobalAmbient[] = { 0.6f, 0.6f, 0.6f, 1.0f };
//*/
//		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightGlobalAmbient);
//
//		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.2f);
///*
//		float materialEmission[] = { 0.0f, 0.0f, 0.0f, 0.0f };
////		float materialAmbient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
//		float materialAmbient[]  = { 0.6f, 0.6f, 0.6f, 1.0f };
//		float materialDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
//		float materialSpecular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//*/
//		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmission);
//		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, lightBrightAmbient);
//		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
//		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
//	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glEnable(GL_CULL_FACE);
}

//

void HAL_OGL::lighting_off() {
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
}
void HAL_OGL::render_additive_setup(float r, float g, float b, float a) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);						// additive
	glEnable(GL_DEPTH_TEST);
	glDepthMask(false);
//glDisable(GL_DEPTH_TEST); glDepthMask(false);
}
void HAL_OGL::render_explosion_setup(float r, float g, float b, float a) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);		// pre-mult alpha
	glEnable(GL_DEPTH_TEST);
	glDepthMask(false);
//glDisable(GL_DEPTH_TEST); glDepthMask(false);
}
void HAL_OGL::render_additive_cleanup() {
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
//~glDisable(GL_DEPTH_TEST); glDepthMask(false);
}
void HAL_OGL::render_simple_setup() {
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
//glClear(GL_DEPTH_BUFFER_BIT);//glDisable(GL_DEPTH_TEST); glDepthMask(false);
}
void HAL_OGL::render_simple_cleanup() {
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}
void HAL_OGL::render_bg_setup() {
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisable(GL_LIGHTING);
}
void HAL_OGL::render_objects_setup() {
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);				// one of these is required when objects are scaled from world coords: COSTS FRAMERATE!!!
	glDisable(GL_RESCALE_NORMAL);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
}
void HAL_OGL::render_rect_setup(float r, float g, float b, float a) {
//	float c[4];
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
//	glGetFloatv(GL_CURRENT_COLOR, c);
	glDisable(GL_LIGHTING);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
#ifndef PCport		// point sprites not implemented on PC OpenGL
	glDisable(GL_POINT_SPRITE_OES);
	glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
#endif // !PCport
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor4f(r, g, b, a);
}
void HAL_OGL::render_big_map_setup() {
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glEnable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void HAL_OGL::render_big_map_cleanup() {
}
void HAL_OGL::render_popup_setup() {
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glEnable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void HAL_OGL::render_popup_cleanup() {
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
glDisable(GL_DEPTH_TEST); glDepthMask(false);
}
void HAL_OGL::set_depth_test(int depth_test, int depth_mask, int depth_func) {
	if (depth_test > 0) glEnable(GL_DEPTH_TEST);
	if (depth_test < 0) glDisable(GL_DEPTH_TEST);
	if (depth_mask > 0) glDepthMask(true);
	if (depth_mask < 0) glDepthMask(false);
	if (depth_func > 0) glDepthFunc(depth_func);
}
void HAL_OGL::get_depth_test(bool *depth_test, int *depth_mask, int *depth_func) {
	if (depth_test != NULL) *depth_test = glIsEnabled(GL_DEPTH_TEST);
	if (depth_mask != 0) *depth_mask = glIsEnabled(GL_DEPTH_WRITEMASK);
	if (depth_func != NULL) glGetIntegerv(GL_DEPTH_FUNC, depth_func);
}

//

void HAL_OGL::render_stars(const float *stars_points, int STARS_MAX, float STARS_CUBE_HALFSIZE, int SPEED_FACTOR, TMatrix *viewMatrix, float a, float k0, float k1) {
if (g_exhaust_th == 0) { int w, h; g_exhaust_th = texture_load(false, "exhaust.png", &w, &h); }			// deferred from init() because texture_load did not work there

set_depth_test(-1, -1, 0);


			matrix_push(2);
			matrix_push();
			//
			TMatrix m, m1, m2;
			matrix_get(&m1);
			matrix_get(&m2, 2);
			matrix_loadidentity(2);
			TMatrix::multiply(m, m1, m2);
			m.f[0] = 1.0f;
			m.f[1] = 0.0f;
			m.f[2] = 0.0f;
			m.f[4] = 0.0f;
			m.f[5] = 1.0f;
			m.f[6] = 0.0f;
			m.f[8] = 0.0f;
			m.f[9] = 0.0f;
			m.f[10] = 1.0f;
			matrix_load(m);

			float r = 4;
			float scl = STARS_CUBE_HALFSIZE;


#ifdef never


			dblt_rect_start(g_exhaust_th, 4, 1, 1, 1, 1);
			for (int i=0; i<STARS_MAX; i++) {
//				matrix_push();
	//			matrix_translate(1000*stars_points[3*i+0], 1000*stars_points[3*i+1], 1000*stars_points[3*i+2]);
				//float s = 20;
				//matrix_scale(s, s, s/4);
				//
//				const int Z = 1;
//				matrix_translate(0, 0, -100);
TVector3 v(stars_points[3*i+0], stars_points[3*i+1], stars_points[3*i+2]);
//TMatrix mTmp;
//TMatrix::translate(mTmp, v.x, v.y, v.z);
//TMatrix::multiply(m, m2, mTmp);
//TVector3::multiply(v, v, m2);
//m = m2;
//m.f[12] += v.x;
//m.f[13] += v.y;
//m.f[14] += v.z;
TVector3::multiply(v, v, m2);
DBlt_append(blt_being_built, scl*v.x+r, scl*v.y+r, -2*r, -2*r, 0, 0, 1, 1, scl*v.z);
//				dblt_rect_next(10, 10, -20, -20, 0, 0, 1, 1);
//				matrix_pop();
			}


#else

			{
				static float *gStarsVerts = NULL;
				static float *gStarsUVs = NULL;
				static unsigned short *gStarsIndices = NULL;
				if (gStarsVerts == NULL) gStarsVerts = (float *)malloc(4 * 3 * STARS_MAX * sizeof(float));
				if (gStarsUVs == NULL) gStarsUVs = (float *)malloc(4 * 2 * STARS_MAX * sizeof(float));
				if (gStarsIndices == NULL) gStarsIndices = (unsigned short *)malloc(3 * 2 * STARS_MAX * sizeof(unsigned short));
				//
				for (int i=0; i<STARS_MAX; i++) {
					TVector3 v(stars_points[3*i+0], stars_points[3*i+1], stars_points[3*i+2]);
					TVector3::multiply(v, v, m2);
					float x = scl*v.x+r;
					float y = scl*v.y+r;
					float dx = -2*r;
					float dy = -2*r;
					float tx = 0;
					float ty = 0;
					float tdx = 1;
					float tdy = 1;
					float z = scl*v.z;
					// fill out gStarsVerts, gStarsUVs, gStarsIndices
					gStarsVerts[4 * 3 * i +  0] = x;
					gStarsVerts[4 * 3 * i +  1] = y+dy;
					gStarsVerts[4 * 3 * i +  2] = z;
					gStarsVerts[4 * 3 * i +  3] = x;
					gStarsVerts[4 * 3 * i +  4] = y;
					gStarsVerts[4 * 3 * i +  5] = z;
					gStarsVerts[4 * 3 * i +  6] = x+dx;
					gStarsVerts[4 * 3 * i +  7] = y+dy;
					gStarsVerts[4 * 3 * i +  8] = z;
					gStarsVerts[4 * 3 * i +  9] = x+dx;
					gStarsVerts[4 * 3 * i + 10] = y;
					gStarsVerts[4 * 3 * i + 11] = z;
					gStarsUVs[4 * 2 * i + 0] = tx;
					gStarsUVs[4 * 2 * i + 1] = ty+tdy;
					gStarsUVs[4 * 2 * i + 2] = tx;
					gStarsUVs[4 * 2 * i + 3] = ty;
					gStarsUVs[4 * 2 * i + 4] = tx+tdx;
					gStarsUVs[4 * 2 * i + 5] = ty+tdy;
					gStarsUVs[4 * 2 * i + 6] = tx+tdx;
					gStarsUVs[4 * 2 * i + 7] = ty;
					gStarsIndices[6 * i + 0] = (unsigned short)(4*i+0);
					gStarsIndices[6 * i + 1] = (unsigned short)(4*i+2);
					gStarsIndices[6 * i + 2] = (unsigned short)(4*i+1);
					gStarsIndices[6 * i + 3] = (unsigned short)(4*i+1);
					gStarsIndices[6 * i + 4] = (unsigned short)(4*i+2);
					gStarsIndices[6 * i + 5] = (unsigned short)(4*i+3);
				}
				//
				texture_bind(0, g_exhaust_th);
				set_mode(8, 1, 1, 1, 1);
				glVertexPointer(3, GL_FLOAT, 0, gStarsVerts);
				glTexCoordPointer(2, GL_FLOAT, 0, gStarsUVs);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				glDrawElements(GL_TRIANGLES, 3 * 2 * STARS_MAX, GL_UNSIGNED_SHORT, gStarsIndices);
				texture_bind(0, 0);
				set_mode(0, 1, 1, 1, 1);
			}

#endif


			matrix_pop(2);
			matrix_pop();



/*
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
//	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#ifndef PCport		// drawing stars: point sprites not implemented on PC OpenGL
	glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
	glDisable(GL_POINT_SPRITE_OES);
#endif // !PCport
	glPointSize(4.0f);

	glVertexPointer(3, GL_FLOAT, 0, stars_points);
//	glColorPointer(4, GL_UNSIGNED_BYTE, 0, stars_colors);
	glEnable(GL_POINT_SMOOTH);									// round
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPointParameterf(GL_POINT_SIZE_MIN, 2.0f);
	glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 3.0f);
	glPointParameterf(GL_POINT_SIZE_MAX, 5.0f);
	float att[3] = { 0.0f, 0.0f, 0.00001f };
	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, att);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	float v[4];
	v[0] = viewMatrix->f[12] / SPEED_FACTOR;	v[1] = viewMatrix->f[13] / SPEED_FACTOR;	v[2] = viewMatrix->f[14] / SPEED_FACTOR;	v[3] = 1.0f;
	glLightfv(GL_LIGHT0, GL_POSITION, v);
	v[0] = 0;	v[1] = 0;	v[2] = 1.0f;
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, v);
	v[0] = 1.0f;	v[1] = 1.0f;	v[2] = 1.0f;	v[3] = a;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, v);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, k0);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, k1);
	glNormal3f(viewMatrix->f[8], viewMatrix->f[9], viewMatrix->f[10]);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glBindTexture(GL_TEXTURE_2D, 0);

	matrix_scale(STARS_CUBE_HALFSIZE, STARS_CUBE_HALFSIZE, STARS_CUBE_HALFSIZE);

	glColor4f(1.0f, 1.0f, 1.0f, a);

	glDrawArrays(GL_POINTS, 0, STARS_MAX);

#ifndef PCport		// drawing stars: point sprites not implemented on PC OpenGL
	glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
#endif // !PCport
	glDisableClientState(GL_COLOR_ARRAY);
	glEnable(GL_LIGHTING);
	glDisable(GL_POINT_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
glDisable(GL_DEPTH_TEST); glDepthMask(false);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
*/
}

void HAL_OGL::render_icoverts(int ico2vertsvalid, const float *ico2validverts, const float *ico2validsizes, const unsigned long *ico2validcolors) {
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#ifndef PCport		// drawing ico2verts: point sprites not implemented on PC OpenGL
	glDisable(GL_POINT_SPRITE_OES);

	glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
//	glPointSizePointerOES(GL_FLOAT, sizeof(TDotGroup), &ico2verts[0].size);
	glPointSizePointerOES(GL_FLOAT, 0, ico2validsizes);
#endif // !PCport
//
	glPointSize(4.0f);

	glEnableClientState(GL_COLOR_ARRAY);
//	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(TDotGroup), &ico2verts[0].abgr);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, ico2validcolors);
//	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

	glEnableClientState(GL_VERTEX_ARRAY);
//	glVertexPointer(3, GL_FLOAT, sizeof(TDotGroup), &ico2verts[0].location);
	glVertexPointer(3, GL_FLOAT, 0, ico2validverts);

	glEnable(GL_POINT_SMOOTH);									// round
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glPointParameterf(GL_POINT_SIZE_MIN, 2.0f);					// THIS IS NOT USED ON SIMULATOR, USED ON DEVICE!!!
	glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 24.0f);
	glPointParameterf(GL_POINT_SIZE_MAX, 74.0f);
	float att[3] = { 1.0f, 0.0f, 0.0f };
	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, att);
	glDisable(GL_LIGHTING);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);

	glDrawArrays(GL_POINTS, 0, ico2vertsvalid);

#ifndef PCport		// drawing ico2verts: point sprites not implemented on PC OpenGL
	glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
#endif // !PCport
	glDisableClientState(GL_COLOR_ARRAY);
	glEnable(GL_LIGHTING);
	glDisable(GL_POINT_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
glDisable(GL_DEPTH_TEST); glDepthMask(false);
}

void HAL_OGL::render_shield(float opacity, bool renderop_2sided, const float *colorcorners, const float *ico2corners, const float *ico2normals, const unsigned short *ico2tris, int nidxs, int nverts, float emissive) {
	float materialDiffuse_1[]			= { 0.0f, 0.0f, 1.0f, opacity };				// used for diffuse for shield
	float materialDiffuse_white_op[]	= { 1.0f, 1.0f, 1.0f, 1.0f };	// (used for specular for shield,) put in diffuse before return
//	float materialDiffuse_0[]			= { 0.0f, 0.0f, 0.0f, 1.0f };			// put in specular before return

	float v[4];
	v[0] = v[1] = v[2] = emissive * 0.30f;
	v[3] = 1.0f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, v);

	if (renderop_2sided) glCullFace(GL_FRONT);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
texture_bind(0, 0);//	glBindTexture(GL_TEXTURE_2D, 0);
//	glDisable(GL_DEPTH_TEST);
//???????????????????????????????????????	glDepthMask(false);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
//	glEnable(GL_DITHER);			// don't ever use!!!
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnable(GL_COLOR_MATERIAL);
	glColorPointer(4, GL_FLOAT, 0, colorcorners);
//	glColor4f(0.0f, 0.0f, 1.0f, 0.5f);		// ignored when lighting is on
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, ico2corners);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glDisable(GL_ALPHA_TEST);









	glEnable(GL_LIGHTING);

	bool old_value = glIsEnabled(GL_NORMALIZE);
	glEnable(GL_NORMALIZE);				// one of these is required when objects are scaled from world coords: COSTS FRAMERATE!!!
//	glEnable(GL_RESCALE_NORMAL);



glDepthMask(true);
glEnable(GL_DEPTH_TEST);



	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, ico2normals);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse_1);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialDiffuse_white_op);
//	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 6);

	glDrawElements(GL_TRIANGLES, nidxs, GL_UNSIGNED_SHORT, ico2tris);

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse_white_op);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialDiffuse_0);
//	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.2f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
//	glDisableClientState(GL_NORMAL_ARRAY);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//	glDisable(GL_DITHER);
	glEnable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	if (!old_value) glDisable(GL_NORMALIZE);

	if (renderop_2sided) glCullFace(GL_BACK);

	v[0] = v[1] = v[2] = v[3] = 0.0f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, v);
}

void HAL_OGL::render_exhaust(bool two_layers, const float *verts, const float *uvs) {
/*
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);  glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	//glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	//glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);  glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	//glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PREVIOUS);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	//glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, GL_PREVIOUS); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
*/
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
//??	glBlendFunc(GL_SRC_ALPHA, GL_ONE);		// use the alpha in the constant color, otherwise this is basic additive translucency
	glBlendFunc(GL_ONE, GL_ONE);		// additive translucency
	glEnable(GL_DEPTH_TEST);
	glDepthMask(false);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glTexCoordPointer(2, GL_FLOAT, 0, uvs);
	//
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 1 * 4);
if (two_layers) glDrawArrays(GL_TRIANGLE_STRIP, 4, 1 * 4);			// extra layer looks really good
	//
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void HAL_OGL::render_exhaust2(TextureHandle th, float r, float g, float b, float a) {
#ifdef never
	DBlt_add(1, 1, -2, -2, 0, 0, 1, 1, th, 7, r, g, b, a);
#else
	float verts[] = { 1, -1, 0, 1, 1, 0, -1, -1, 0, -1, 1, 0 };
	float uvs[] = { 0, 1, 0, 0, 1, 1, 1, 0 };
	unsigned short idx[] = { 0, 1, 2, 1, 3, 2 };
	
	texture_bind(0, th);
	set_mode(7, r, g, b, a);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glTexCoordPointer(2, GL_FLOAT, 0, uvs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisable(GL_LIGHTING);

	glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_SHORT, idx);

	texture_bind(0, 0);
	set_mode(0, 1, 1, 1, 1);
	glEnable(GL_LIGHTING);
	glDepthMask(true);
#endif
}

void HAL_OGL::render_emissive_setup1(TextureHandle t) {
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	texture_bind(0, t);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);  glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	//glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);  glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PREVIOUS);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	//glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, GL_PREVIOUS); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
}
void HAL_OGL::render_emissive_setup2(float *color) {
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);  glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	//glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);  glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_CONSTANT);  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	//glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, GL_PREVIOUS); glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
}
void HAL_OGL::render_emissive_cleanup() {
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}
int HAL_OGL::render_alpha_setup1() {
	int savedDF;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glGetIntegerv(GL_DEPTH_FUNC, &savedDF);
	//
	glColorMask(false, false, false, false);
	glDepthMask(true);
	return savedDF;
}
void HAL_OGL::render_alpha_setup2() {
	glColorMask(true, true, true, true);
	glDepthMask(false);
	glDepthFunc(GL_LEQUAL);
}
void HAL_OGL::render_alpha_cleanup(int savedDF) {
	glColorMask(true, true, true, true);
	glDepthMask(true);
	//
	glDepthFunc(savedDF);
}

//

void HAL_OGL::setup_1() {
}
void HAL_OGL::draw_1() {
}









