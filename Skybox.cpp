#include "Skybox.h"

#include <vector>
#include <string>

#include "HAL.h"
#include "math.h"
#include "stdlib.h"


//#define BLACKOUT_SKYBOX

extern HAL *gHAL;

extern double curtimeval, curtimeval_p;
extern double app_start_time;



#define SKYBOX_ZOOM			150.0f
#define SKYBOX_ADJUSTUVS	true

//static 
TextureHandle m_ui32SkyboxTex[6];

/* Skybox */
static float* g_skyboxVertices;
static float* g_skyboxUVs;

static bool do_nebula_effect = false;
static int skyboxsize;

static float random_rotation = 0;

/*******************************************************************************
 * Function Name  : DrawSkybox
 * Description    : Draws the skybox
 *******************************************************************************/
void DrawSkybox(float skybox_a, TVector3 &viewpoint, bool flashing)
{
	if (skyboxsize == 0) return;
	/* Draw the skybox around the camera position */
	gHAL->matrix_push();
	gHAL->matrix_rotate((float)(random_rotation * .120f), 0, 0, 1);
	gHAL->matrix_rotate((float)(random_rotation * .123f), 0, 1, 0);
	gHAL->matrix_translate(viewpoint.x, viewpoint.y, viewpoint.z);

	gHAL->skybox_draw_setup(flashing, skybox_a);

	for(int i = 0; i < 6; ++i) {
		// Set Data Pointers
		gHAL->texture_bind(0, m_ui32SkyboxTex[i]);
		// Draw
		gHAL->skybox_draw(flashing, &g_skyboxVertices[i*4*3], &g_skyboxUVs[i*4*2], 4);
	}


if (do_nebula_effect) {			// double vision effect: (needs no rotation!)
	float t = (float)(curtimeval - app_start_time) / 2;
	float move_magnitude = 0.5f;
	gHAL->matrix_translate(move_magnitude * sinf(t * 1.5f), move_magnitude * sinf(t * 2.5f), 4 * move_magnitude * sinf(t * 0.9f));
	gHAL->skybox_draw_nebula_setup(skybox_a);
	for(int i = 0; i < 6; ++i) {
		// Set Data Pointers
		gHAL->texture_bind(0, m_ui32SkyboxTex[i]);
		gHAL->texture_blur(0);
		// Draw
		gHAL->skybox_draw(flashing, &g_skyboxVertices[i*4*3], &g_skyboxUVs[i*4*2], 4);
	}
	//
	gHAL->skybox_draw_nebula_cleanup();
}

	gHAL->matrix_pop();

	gHAL->skybox_draw_cleanup();
}

/*!***************************************************************************
 @Function			SetVertex
 @Modified			Vertices
 @Input				index
 @Input				x
 @Input				y
 @Input				z
 @Description		Writes a vertex in a vertex array
 *****************************************************************************/
static void SetVertex(float** Vertices, int index, float x, float y, float z)
{
	(*Vertices)[index*3+0] = x;
	(*Vertices)[index*3+1] = y;
	(*Vertices)[index*3+2] = z;
}

/*!***************************************************************************
 @Function			SetUV
 @Modified			UVs
 @Input				index
 @Input				u
 @Input				v
 @Description		Writes a texture coordinate in a texture coordinate array
 *****************************************************************************/
static void SetUV(float** UVs, int index, float u, float v)
{
	(*UVs)[index*2+0] = u;
	(*UVs)[index*2+1] = v;
}

/*!***************************************************************************
 @Function		CreateSkybox
 @Input			scale			Scale the skybox
 @Input			adjustUV		Adjust or not UVs for PVRT compression
 @Input			textureSize		Texture size in pixels
 @Output		Vertices		Array of vertices
 @Output		UVs				Array of UVs
 @Description	Creates the vertices and texture coordinates for a skybox
 *****************************************************************************/
static void CreateSkybox(float scale, bool adjustUV, int textureSize, float** Vertices, float** UVs)
{
	*Vertices = new float[24*3];
	*UVs = new float[24*2];
	
	float unit = 1;
	float a0 = 0, a1 = unit;
	
	if (adjustUV)
	{
		float oneover = 1.0f / textureSize;
		a0 = 4.0f * oneover;
		a1 = unit - a0;
	}
	
	// Front
	SetVertex(Vertices, 0, -unit, +unit, -unit);
	SetVertex(Vertices, 1, +unit, +unit, -unit);
	SetVertex(Vertices, 2, -unit, -unit, -unit);
	SetVertex(Vertices, 3, +unit, -unit, -unit);
	SetUV(UVs, 0, a0, a1);
	SetUV(UVs, 1, a1, a1);
	SetUV(UVs, 2, a0, a0);
	SetUV(UVs, 3, a1, a0);
	
	// Right
	SetVertex(Vertices, 4, +unit, +unit, -unit);
	SetVertex(Vertices, 5, +unit, +unit, +unit);
	SetVertex(Vertices, 6, +unit, -unit, -unit);
	SetVertex(Vertices, 7, +unit, -unit, +unit);
	SetUV(UVs, 4, a0, a1);
	SetUV(UVs, 5, a1, a1);
	SetUV(UVs, 6, a0, a0);
	SetUV(UVs, 7, a1, a0);
	
	// Back
	SetVertex(Vertices, 8 , +unit, +unit, +unit);
	SetVertex(Vertices, 9 , -unit, +unit, +unit);
	SetVertex(Vertices, 10, +unit, -unit, +unit);
	SetVertex(Vertices, 11, -unit, -unit, +unit);
	SetUV(UVs, 8 , a0, a1);
	SetUV(UVs, 9 , a1, a1);
	SetUV(UVs, 10, a0, a0);
	SetUV(UVs, 11, a1, a0);
	
	// Left
	SetVertex(Vertices, 12, -unit, +unit, +unit);
	SetVertex(Vertices, 13, -unit, +unit, -unit);
	SetVertex(Vertices, 14, -unit, -unit, +unit);
	SetVertex(Vertices, 15, -unit, -unit, -unit);
	SetUV(UVs, 12, a0, a1);
	SetUV(UVs, 13, a1, a1);
	SetUV(UVs, 14, a0, a0);
	SetUV(UVs, 15, a1, a0);
	
	// Top
	SetVertex(Vertices, 16, -unit, +unit, +unit);
	SetVertex(Vertices, 17, +unit, +unit, +unit);
	SetVertex(Vertices, 18, -unit, +unit, -unit);
	SetVertex(Vertices, 19, +unit, +unit, -unit);
	SetUV(UVs, 16, a0, a1);
	SetUV(UVs, 17, a1, a1);
	SetUV(UVs, 18, a0, a0);
	SetUV(UVs, 19, a1, a0);
	
	// Bottom
	SetVertex(Vertices, 20, -unit, -unit, -unit);
	SetVertex(Vertices, 21, +unit, -unit, -unit);
	SetVertex(Vertices, 22, -unit, -unit, +unit);
	SetVertex(Vertices, 23, +unit, -unit, +unit);
	SetUV(UVs, 20, a0, a1);
	SetUV(UVs, 21, a1, a1);
	SetUV(UVs, 22, a0, a0);
	SetUV(UVs, 23, a1, a0);
	
	for (int i=0; i<24*3; i++) (*Vertices)[i] = (*Vertices)[i] * scale;
}

static void init_a_skybox_panel(std::string &prefix, int which, bool old) {
	std::string fn;
	switch (which) {
		case 0:		fn = prefix + (old?"0":"BK");		break;
		case 1:		fn = prefix + (old?"90":"RT");		break;
		case 2:		fn = prefix + (old?"180":"FR");		break;
		case 3:		fn = prefix + (old?"270":"LF");		break;
		case 4:		fn = prefix + (old?"bottom":"DN");	break;
		case 5:		fn = prefix + (old?"top":"UP");		break;
	}
	fn = "skyboxes/" + fn;
	int w, h;
	if (m_ui32SkyboxTex[which] != 0) gHAL->skybox_delete_texture(m_ui32SkyboxTex[which]);
	m_ui32SkyboxTex[which] = 0;
	w = h = 0;
gHAL->texture_bind(0, 0);	// Win32 build was trashing the bezel font here?
	HAL::load_texture_file(fn.c_str(), &m_ui32SkyboxTex[which], &w, &h);
	skyboxsize = w;
#ifdef DEBUG_SHOW_LOADING_DETAIL
printf("Skybox: loaded texture '%s' into handle %d\n", fn.c_str(), m_ui32SkyboxTex[which]);
#endif // DEBUG_SHOW_LOADING_DETAIL
	if (m_ui32SkyboxTex[which] == 0) printf("**ERROR** Failed to load texture '%s' for skybox.\n", fn.c_str());
}

void init_a_skybox(int level, bool firsttime, int which) {
	std::string prefix;
	if (level < 0) level = 0;

	bool old_skybox_format = true;
	switch (level) {
		default:
		case 0:			prefix = "thick_rb_";	old_skybox_format = true;		do_nebula_effect = false;		break;
		case 1:			prefix = "skybox04_";	old_skybox_format = false;		do_nebula_effect = false;		break;
		case 2:			prefix = "skybox27_";	old_skybox_format = false;		do_nebula_effect = true;		break;
		case 3:			prefix = "skybox17_";	old_skybox_format = false;		do_nebula_effect = false;		break;
		case 4:			prefix = "skybox25_";	old_skybox_format = false;		do_nebula_effect = false;		break;
	}

	if (which < 0) {
		init_a_skybox_panel(prefix, 0, old_skybox_format);
		init_a_skybox_panel(prefix, 1, old_skybox_format);
		init_a_skybox_panel(prefix, 2, old_skybox_format);
		init_a_skybox_panel(prefix, 3, old_skybox_format);
		init_a_skybox_panel(prefix, 4, old_skybox_format);
		init_a_skybox_panel(prefix, 5, old_skybox_format);
	} else {
		init_a_skybox_panel(prefix, which, old_skybox_format);
	}
}

void init_skybox() {
	for (int i=0; i<6; i++) m_ui32SkyboxTex[i] = 0;
	init_a_skybox(0, true);
	
	/*********************/
	/* Create the skybox */
	/*********************/
	CreateSkybox( SKYBOX_ZOOM, SKYBOX_ADJUSTUVS, skyboxsize, &g_skyboxVertices, &g_skyboxUVs );
	
	random_rotation = 1000 * (rand() / (float)RAND_MAX);
}


