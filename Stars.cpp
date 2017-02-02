// Stars

#include "Stars.h"
#include "FPGameStatus.h"

#include "HAL.h"
#include "stdlib.h"

extern float gScreenAspectRatio;
extern HAL *gHAL;
extern FPGameStatus gGS;

// stars

#define STARS_SPEED_UP 6
#define STARS_MAX 2000
#define STARS_CUBE_HALFSIZE (12000.0f/STARS_SPEED_UP)
#define STARS_FAR_CLIP (14000.f/STARS_SPEED_UP)
#define STARS_QUADRATIC_ATTENUATION 0.0000000000015f*STARS_SPEED_UP*STARS_SPEED_UP*STARS_SPEED_UP

static float *stars_points;
static unsigned char *stars_colors;


void stars_init() {
	int i;
	stars_points = new float[STARS_MAX * 3];
	for (i=0; i<STARS_MAX; i++) {
		stars_points[3*i+0] = 2.0f * rand() / RAND_MAX - 1.0f;		// range -1.0..1.0f
		stars_points[3*i+1] = 2.0f * rand() / RAND_MAX - 1.0f;		// range -1.0..1.0f
		stars_points[3*i+2] = 2.0f * rand() / RAND_MAX - 1.0f;		// range -1.0..1.0f
	}
	stars_colors = new unsigned char[STARS_MAX * 4];
	for (i=0; i<STARS_MAX; i++) {
		stars_colors[4*i+0] = (unsigned char)(255.0f * rand() / RAND_MAX);		// range 0..255
		stars_colors[4*i+1] = (unsigned char)(255.0f * rand() / RAND_MAX);		// range 0..255
		stars_colors[4*i+2] = (unsigned char)(255.0f * rand() / RAND_MAX);		// range 0..255
		stars_colors[4*i+3] = (unsigned char)(255.0f * rand() / RAND_MAX);		// range 0..255
	}
}

#ifdef __APPLE__
#include <TargetConditionals.h>
#if (TARGET_IPHONE_SIMULATOR == 0) && (TARGET_OS_IPHONE == 1)
//#define DO_LOOP_ASM
#else
#endif
#endif

#ifdef DO_LOOP_ASM
#include "common_macros.h"

void compute(const float* view_xyz, float* dst_vec, int cnt) {
  asm volatile (
                "mov r4, %1               \n\t"
				"mov r1, %1               \n\t"
                "mov r5, %3               \n\t"		// not clear why this is %3 not %2!!!!!!!!!!!!!
				"fldmias  %0, {s8-s10}    \n\t"			// view location
				 
                VFP_VECTOR_LENGTH(2)
                
"\n1:\t"
                "fldmias r1!, {s16-s18}   \n\t"

                "fsubs s12, s16, s8       \n\t"
                "ftosis s12, s12          \n\t"
                "ftosis s13, s13          \n\t"
                "ftosis s14, s14          \n\t"
                "fsitos s12, s12          \n\t"
                "fsitos s13, s13          \n\t"
                "fsitos s14, s14          \n\t"
                "fsubs s16, s16, s12      \n\t"
                "fsubs s16, s16, s12      \n\t"
                
                "fstmias r4!, {s16-s18}   \n\t" 
				"subs r5, r5, #1          \n\t"
				"bgt 1b                   \n\t"
                VFP_VECTOR_LENGTH_ZERO
                : "=r" (dst_vec)			// modified?
                : "r" (dst_vec), "0" (view_xyz), "r" (cnt)
                : "r0", "r1", "r4", "r5"
                );  
}

#else


static void compute(const float* view_xyz, float* dst_vec, int cnt) {
	for (int i=0; i<cnt; i++) {
		 *dst_vec -= 2.0f*(int)(*dst_vec - view_xyz[0]);
		 dst_vec++;
		 *dst_vec -= 2.0f*(int)(*dst_vec - view_xyz[1]);
		 dst_vec++;
		 *dst_vec -= 2.0f*(int)(*dst_vec - view_xyz[2]);
		 dst_vec++;
	}
}

static void compute_clean(const float* view_xyz, float* dst_vec, int cnt) {
	for (int i=0; i<cnt; i++) {
		 *dst_vec -= (int)(*dst_vec - view_xyz[0]);
		 dst_vec++;
		 *dst_vec -= (int)(*dst_vec - view_xyz[1]);
		 dst_vec++;
		 *dst_vec -= (int)(*dst_vec - view_xyz[2]);
		 dst_vec++;
	}
}

#endif


#define SPEED_FACTOR 1
extern bool IsDoubleScreen();


void stars_clean(TMatrix *viewMatrix) {
	// update stars locations to keep them within a box near the camera
	float xyz[3];
	xyz[0] = (viewMatrix->f[12] / SPEED_FACTOR) / STARS_CUBE_HALFSIZE;
	xyz[1] = (viewMatrix->f[13] / SPEED_FACTOR) / STARS_CUBE_HALFSIZE;
	xyz[2] = (viewMatrix->f[14] / SPEED_FACTOR) / STARS_CUBE_HALFSIZE;
	compute_clean(xyz, stars_points, STARS_MAX);
}

void stars_draw(TMatrix *viewMatrix, float a) {
	// update stars locations to keep them within a box near the camera
	float xyz[3];
	xyz[0] = (viewMatrix->f[12] / SPEED_FACTOR) / STARS_CUBE_HALFSIZE;
	xyz[1] = (viewMatrix->f[13] / SPEED_FACTOR) / STARS_CUBE_HALFSIZE;
	xyz[2] = (viewMatrix->f[14] / SPEED_FACTOR) / STARS_CUBE_HALFSIZE;
	compute(xyz, stars_points, STARS_MAX);

	if (!gGS.user_star_field) return;
	
	float fov = 0.785398185f;
	TMatrix m;
	TMatrix::perspectiveFovRH(m, fov, gScreenAspectRatio, 4.0f, STARS_FAR_CLIP, false); //+BAS
	gHAL->matrix_load(m, 1);
    if (gGS.user_screen_flip) gHAL->matrix_rotate(180, 0, 0, 1, 1);
	gHAL->matrix_push();


	float k1 = 0.00000001f;//0.000000001f;//((1.0f / STARS_FALLOFF_END_VAL) - 1.0f) / (STARS_FALLOFF_END - STARS_FALLOFF_START);		// 1.0f is 1/STARS_FALLOFF_START_VAL
	float k0 = 0.00001f;//1.0f - k1 * STARS_FALLOFF_START;//(STARS_FALLOFF_END - (STARS_FALLOFF_START / STARS_FALLOFF_END_VAL)) / (STARS_FALLOFF_END - STARS_FALLOFF_START);
k1 = STARS_QUADRATIC_ATTENUATION;
k0 = 0.0f;

	gHAL->render_stars(stars_points, STARS_MAX, STARS_CUBE_HALFSIZE, SPEED_FACTOR, viewMatrix, a, k0, k1);

	gHAL->matrix_pop();
}


void stars_exit() {
	delete [] stars_points;
	delete [] stars_colors;
}


