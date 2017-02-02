// Convert.h : Header for Convert.cpp
//

#ifndef CONVERT_H
#define CONVERT_H

////////////////////////////////////////////////////////////////////////////////////////////////////////

int *read_memory_png(void *m, int length, int *width, int *height);
int *read_file_png(const char *fname, int *width, int *height);
int *read_open_file_png(FILE *fd, int *width, int *height);

////////////////////////////////////////////////////////////////////////////////////////////////////////

// write an image file
void write_L8_file(char *bmap, int bmapw, int bmaph, const char *filename, const char *filesuffix);
void write_ARGB_file(int *bmap, int bmapw, int bmaph, const char *filename, const char *filesuffix);
void write_P8_ARGB_file(unsigned char *bmap, int bmapw, int bmaph, int *palptr, int palsize, int n_alphas, const char *filename, const char *filesuffix);
void write_P8_ARGB_file_frMS(unsigned char *bmap, int bmapw, int bmaph, char *frames_map, int frames_map_size, int *palptr, int palsize, int n_alphas, const char *filename, const char *filesuffix);
void write_RGB_tRNS_file(int *bmap, int bmapw, int bmaph, const char *filename, const char *filesuffix, int trans_r, int trans_g, int trans_b);

////////////////////////////////////////////////////////////////////////////////////////////////////////

// shrink an image and return a new /shrinkratio image (caller assumes size is oldsize/shrinkratio)
//  offsets may cause some samples off the bitmap, in which case they don't contribute to the box filter
int *shrink_map(int *bmap, int bmapw, int bmaph, int shrinkratio, int offsetx, int offsety);

////////////////////////////////////////////////////////////////////////////////////////////////////////

// make a new copy of the image, flipped
int *hflip_map(int *bmap, int bmapw, int bmaph);
unsigned char *hflip_cmap(unsigned char *bmap, int bmapw, int bmaph);

////////////////////////////////////////////////////////////////////////////////////////////////////////

// make a new copy of the image
int *copy_map(int *bmap, int bmapw, int bmaph);
unsigned char *copy_pmap(unsigned char *bmap, int bmapw, int bmaph);

////////////////////////////////////////////////////////////////////////////////////////////////////////

// quantize an image IN PLACE by rounding each component to *bits bits, left shifted (ie 2 bits would be: c' = (c+0x020)&0x0c0;)
//  alpha_threshold used when abits == 1
void quantize_map(int *bmap, int bmapw, int bmaph, int abits, int rbits, int gbits, int bbits, int alpha_threshold=0x080);

////////////////////////////////////////////////////////////////////////////////////////////////////////

// palettize an image and return a new palettized image of same size
unsigned char *palettize_map(int *bmap, int bmapw, int bmaph, int *palptr, int palsize, int dodither=1);
unsigned char *palettize_multi(int **bmaps, int bmapcnt, int bmapw, int bmaph, int *palptr, int palsize, int dodither=1);

// unpalettize an image and return a new unpalettized image of same size
int *unpalettize_map(unsigned char *bmap, int bmapw, int bmaph, int *palptr, int palsize);

// for this map, generate a reasonable 256 color ARGB palette, sorted by increasing A
int generate_palette(int **bmaps, int bmapcnt, int bmapw, int bmaph, int desired_colors, int *pals_out, int verbose=0);

#endif // !CONVERT_H
