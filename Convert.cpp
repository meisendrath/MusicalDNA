// Convert.cpp : Converts from one image format to another, with processing
//

#include "config.h"
#include "common.h"

#include <string>
#include <vector>

//<hash>include "stdafx.h"
#include "stdlib.h"
#include "png.h"

// ALPHA IS OPACITY!!!
#define MAKE_ARGB8(a, r, g, b) (((a)<<24) | ((r)<<16) | ((g)<<8) | (b))
#define DECOMPOSE_ARGB8(c, a, r, g, b) a=((c)>>24)&0x0ff; r=((c)>>16)&0x0ff; g=((c)>>8)&0x0ff; b=(c)&0x0ff;

#define ABS(a) (((a)>0)?(a):(-(a)))

////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef FILE* my_file_type;

void my_read_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
	fread(data, 1, length, (FILE *)png_ptr->io_ptr);
}

void my_write_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
	fwrite(data, 1, length, (FILE *)png_ptr->io_ptr);
}

void my_flush_fn(png_structp png_ptr) {
	fflush((FILE *)png_ptr->io_ptr);
}

//void my_memread_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
//	int		*args = (int *)png_ptr->io_ptr;
//	memcpy(data, (void *)(args[0]+args[2]), length);
//	args[2] += length;
//}
//
//void my_memwrite_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
//	int		*args = (int *)png_ptr->io_ptr;
//	if (args[1] <= (int)(length+args[2])) {	// re-alloc
//		args[1] += length+4096;
//		void		*nu = malloc(args[1]);
//if (nu == NULL) {
//	fprintf(stderr, "my_memwrite_fn(): out of memory!!!!\n");
//	return;
//}
//		memcpy(nu, (void *)args[0], args[2]);
//		free((void *)args[0]);
//		args[0] = (int)nu;
//	}
//	memcpy((void *)(args[0]+args[2]), data, length);
//	args[2] += length;
//}
//
//void my_memflush_fn(png_structp png_ptr) {
//}
//
//void my_memclose_fn(int *args) {
//	if (args) free(args);
//}
//
//int *my_memopenwr_fn() {
//	int *args = (int *)malloc(64);
//	if (args == NULL) return NULL;
//
//	args[0] = (int)malloc(4096);	// buffer
//	if (args[0] == 0) {
//		my_memclose_fn(args);
//		return NULL;
//	}
//	args[1] = 4096;					// limit to current buffer
//	args[2] = 0;					// write ptr
//	return args;
//}
//
//int *my_memopenrd_fn(void *data, int length) {
//	int *args = (int *)malloc(64);
//	if (args == NULL) return NULL;
//
//	args[0] = (int)data;			// buffer
//	args[1] = length;				// limit to current buffer
//	args[2] = 0;					// read ptr
//	return args;
//}

my_file_type my_open_fn(const char *fname, const char *perms) {
	return fopen(fname, perms);
}

void my_close_fn(my_file_type fd) {
	fclose(fd);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

// read an image [file] into unpacked ARGB format
static int *decode_png(png_structp png_ptr, png_infop info_ptr, int *width, int *height) {
	int *im;
	unsigned char bg_red=0, bg_green=0, bg_blue=0;

	png_uint_32  image_width, image_height;
	int  bit_depth, color_type;
	unsigned char *image_data;

	int image_channels;
	png_uint_32 image_rowbytes;
	unsigned long bitmap_width, bitmap_height;

	png_read_info(png_ptr, info_ptr);  /* read all PNG info up to image data */

	/* alternatively, could make separate calls to png_get_image_width(),
	 * etc., but want bit_depth and color_type for later [don't care about
	 * compression_type and filter_type => NULLs] */

	png_get_IHDR(png_ptr, info_ptr, &image_width, &image_height, &bit_depth, &color_type, NULL, NULL, NULL);


// read and convert PNG

	bitmap_width = image_width;
	bitmap_height = image_height;
	*width = image_width;
	*height = image_height;

//				if (readpng_get_bgcolor(&(this->pGlobals->PNGGlobals), &bg_red, &bg_green, &bg_blue) > 1)
//int readpng_get_bgcolor(TPNGGlobals *pglobals, unsigned char *red, unsigned char *green, unsigned char *blue)
	{
		png_color_16p pBackground;

		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_bKGD)) {
			/* it is not obvious from the libpng documentation, but this function
			 * takes a pointer to a pointer, and it always returns valid red, green
			 * and blue values, regardless of color_type: */

			png_get_bKGD(png_ptr, info_ptr, &pBackground);

			/* however, it always returns the raw bKGD data, regardless of any
			 * bit-depth transformations, so check depth and adjust if necessary */

			if (bit_depth == 16) {
				bg_red   = (unsigned char)(pBackground->red   >> 8);
				bg_green = (unsigned char)(pBackground->green >> 8);
				bg_blue  = (unsigned char)(pBackground->blue  >> 8);
			} else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
				if (bit_depth == 1)
					bg_red = bg_green = bg_blue = (unsigned char)(pBackground->gray? 255 : 0);
				else if (bit_depth == 2)
					bg_red = bg_green = bg_blue = (unsigned char)((255/3) * pBackground->gray);
				else /* bit_depth == 4 */
					bg_red = bg_green = bg_blue = (unsigned char)((255/15) * pBackground->gray);
			} else {
				bg_red   = (unsigned char)pBackground->red;
				bg_green = (unsigned char)pBackground->green;
				bg_blue  = (unsigned char)pBackground->blue;
			}

		}
	}


//					image_data = readpng_get_image(&(this->pGlobals->PNGGlobals), PNG_DISPLAY_EXPONENT, &image_channels, &image_rowbytes);
//unsigned char *readpng_get_image(TPNGGlobals *pglobals, double display_exponent, int *pChannels, unsigned long *pRowbytes)


// decode the image, all at once
	{
//		double  gamma;
		unsigned int		  i;
		png_bytepp  row_pointers = NULL;



		/* expand palette images to RGB, low-bit-depth grayscale images to 8 bits,
		 * transparency chunks to full alpha channel; strip 16-bit-per-sample
		 * images to 8 bits per sample; and convert grayscale to RGB[A] */

		if (color_type == PNG_COLOR_TYPE_PALETTE)					png_set_expand(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)		png_set_expand(png_ptr);
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))		png_set_expand(png_ptr);
		if (bit_depth == 16)										png_set_strip_16(png_ptr);
//		if (color_type == PNG_COLOR_TYPE_GRAY ||
//			color_type == PNG_COLOR_TYPE_GRAY_ALPHA)				png_set_gray_to_rgb(png_ptr);


		/* unlike the example in the libpng documentation, we have *no* idea where
		 * this file may have come from--so if it doesn't have a file gamma, don't
		 * do any correction ("do no harm") */

//		if (png_get_gAMA(png_ptr, info_ptr, &gamma))
//			png_set_gamma(png_ptr, PNG_DISPLAY_EXPONENT, gamma);


		/* all transformations have been registered; now update info_ptr data,
		 * get rowbytes and channels, and allocate image memory */

		png_read_update_info(png_ptr, info_ptr);

		image_rowbytes = png_get_rowbytes(png_ptr, info_ptr);
		image_channels = (int)png_get_channels(png_ptr, info_ptr);

		if ((image_data = (unsigned char *)malloc(image_rowbytes*image_height)) == NULL) goto BAIL2;
		if ((row_pointers = (png_bytepp)malloc(image_height*sizeof(png_bytep))) == NULL) goto BAIL3;


		/* set the individual row_pointers to point at the correct offsets */

		for (i = 0;  i < image_height;  ++i)
			row_pointers[i] = image_data + i*image_rowbytes;


		/* now we can go ahead and just read the whole image */

		png_read_image(png_ptr, row_pointers);


		/* and we're done!  (png_read_end() can be omitted if no processing of
		 * post-IDAT text/time/etc. is desired) */

		free(row_pointers);
		row_pointers = NULL;

		png_read_end(png_ptr, NULL);
	}



	if (png_ptr && info_ptr) {
		png_destroy_read_struct(&(png_ptr), &(info_ptr), NULL);
		png_ptr = NULL;
		info_ptr = NULL;
	}

	// Create new bitmap
	im = (int *)malloc(bitmap_width * bitmap_height * 4);
	if (im == NULL) goto BAIL3;	// out of memory

	// Convert into TBitmap
	{
		unsigned char *src;
		int *dest;
		unsigned char r, g, b, a;
		unsigned long row, col;
		r=g=b=0;

		// Copy data (clamp to image)
		for (row = 0;  row < bitmap_height;  row++) {
			dest = im + row*bitmap_width;
			if (row <= image_height-1) {
				src = image_data + row*image_rowbytes;
			} else {
				src = image_data + (image_height-1)*image_rowbytes;
			}
			for (col = 0;  col < bitmap_width; col++) {
				if (col < image_width) {	// pull from image data
					switch (image_channels) {
						case 4:
							r = *src++;
							g = *src++;
							b = *src++;
							a = *src++;			// Grab alpha channel
							break;
						case 3:
							r = *src++;
							g = *src++;
							b = *src++;
							a = 255;			// No alpha channel
							break;
						case 2:
							r = *src++;
							a = *src++;			// Grab alpha channel
							b = g = r;
							break;
						case 1:
							r = *src++;
							a = 255;			// No alpha channel
							b = g = r;
							break;
					}
				}
				*dest++ = MAKE_ARGB8(a, r, g, b);
			}
		}
	}

	free(image_data);
	image_data = NULL;
	//
	return im;
BAIL3:
	free(image_data);
BAIL2:
	png_destroy_read_struct(&(png_ptr), &(info_ptr), NULL);
//BAIL1:
//	if (fd) my_close_fn(fd);
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

//int *read_memory_png(void *m, int length, int *width, int *height) {
//	int *im = NULL;
//	int	*args = NULL;
//	unsigned char sig[8];
//
//	png_structp png_ptr;
//	png_infop info_ptr;
//
//	args = my_memopenrd_fn(m, length);
//	if (args == NULL) goto BAIL1;
//
//	/* could pass pointers to user-defined error handlers instead of NULLs: */
//	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
//	if (!png_ptr) goto BAIL1;		/* out of memory */
//
//	info_ptr = png_create_info_struct(png_ptr);
//	if (!info_ptr) {		   /* out of memory */
//		png_destroy_read_struct(&(png_ptr), NULL, NULL);
//		goto BAIL1;
//	}
//
//	png_set_read_fn(png_ptr, (void *)args, my_memread_fn);
//
//	my_memread_fn(png_ptr, &sig[0], 8);
//	if (!png_check_sig(sig, 8)) goto BAIL2;		   /* bad signature */
//
//	png_set_sig_bytes(png_ptr, 8);  /* we already read the 8 signature bytes */
//
//
//
//	im = decode_png(png_ptr, info_ptr, width, height);		// converts to unpacked ARGB format
//
//
//
//	my_memclose_fn(args);
//
//	return im;
//BAIL2:
//	png_destroy_read_struct(&(png_ptr), &(info_ptr), NULL);
//BAIL1:
//	my_memclose_fn(args);
//	return NULL;
//}

int *read_file_png(const char *fname, int *width, int *height) {
	int *im = NULL;
	my_file_type	fd;
	unsigned char sig[8];

	png_structp png_ptr;
	png_infop info_ptr;

	fd = my_open_fn(fname, "rb");
	if (fd == NULL) goto BAIL1;

	/* could pass pointers to user-defined error handlers instead of NULLs: */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) goto BAIL1;		/* out of memory */

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {		   /* out of memory */
		png_destroy_read_struct(&(png_ptr), NULL, NULL);
		goto BAIL1;
	}

	/* we could create a second info struct here (end_info), but it's only
	 * useful if we want to keep pre- and post-IDAT chunk info separated
	 * (mainly for PNG-aware image editors and converters) */

#ifdef PNG_NO_STDIO
	png_set_read_fn(png_ptr, (void *)fd, my_read_fn);
#else // !PNG_NO_STDIO
	png_init_io(png_ptr, fd);
#endif // !PNG_NO_STDIO

	/* do a quick check that the file really is a PNG image; could
	 * have used slightly more general png_sig_cmp() function instead */

	my_read_fn(png_ptr, &sig[0], 8);
	if (!png_check_sig(sig, 8)) goto BAIL2;		   /* bad signature */

	png_set_sig_bytes(png_ptr, 8);  /* we already read the 8 signature bytes */



	im = decode_png(png_ptr, info_ptr, width, height);



	my_close_fn(fd);

	return im;
BAIL2:
	png_destroy_read_struct(&(png_ptr), &(info_ptr), NULL);
BAIL1:
	if (fd) my_close_fn(fd);
	return NULL;
}

int *read_open_file_png(my_file_type fd, int *width, int *height) {
	int *im = NULL;
	unsigned char sig[8];

	png_structp png_ptr;
	png_infop info_ptr;

	/* could pass pointers to user-defined error handlers instead of NULLs: */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) goto BAIL1;		/* out of memory */

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {		   /* out of memory */
		png_destroy_read_struct(&(png_ptr), NULL, NULL);
		goto BAIL1;
	}

	/* we could create a second info struct here (end_info), but it's only
	 * useful if we want to keep pre- and post-IDAT chunk info separated
	 * (mainly for PNG-aware image editors and converters) */

#ifdef PNG_NO_STDIO
	png_set_read_fn(png_ptr, (void *)fd, my_read_fn);
#else // !PNG_NO_STDIO
	png_init_io(png_ptr, fd);
#endif // !PNG_NO_STDIO

	/* do a quick check that the file really is a PNG image; could
	 * have used slightly more general png_sig_cmp() function instead */

	my_read_fn(png_ptr, &sig[0], 8);
	if (!png_check_sig(sig, 8)) goto BAIL2;		   /* bad signature */

	png_set_sig_bytes(png_ptr, 8);  /* we already read the 8 signature bytes */



	im = decode_png(png_ptr, info_ptr, width, height);



	my_close_fn(fd);

	return im;
BAIL2:
	png_destroy_read_struct(&(png_ptr), &(info_ptr), NULL);
BAIL1:
	if (fd) my_close_fn(fd);
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

// returns 0 if OK, -1 if not
/*
static int encode_png(unsigned short src_channels, unsigned short src_width, unsigned short src_height, void *src_bits, png_structp png_ptr, png_infop info_ptr, int skiprows, int skipcols, int realwidth, int realheight, int mode, int threshold) {
	int				bytes_per_pixel = src_channels;
	int				bit_depth, typ;

	if (realwidth == 0) realwidth = src_width;
	if (realheight == 0) realheight = src_height;

	switch (src_channels) {
		case 1:
			typ = PNG_COLOR_TYPE_GRAY;  bit_depth = 8;
			break;
		case 2:
			typ = PNG_COLOR_TYPE_GRAY_ALPHA;  bit_depth = 8;
			break;
		case 3:
			typ = PNG_COLOR_TYPE_RGB;  bit_depth = 8;		// is bit_depth = 4; needed?
			break;
		case 4:
			typ = PNG_COLOR_TYPE_RGBA;  bit_depth = 8;
			break;
	}

	unsigned char *bits;
	unsigned char *palt;
	unsigned char *palta;
	unsigned char *rptr;
	unsigned char *wptr;
	png_bytep *row_pointers;
	int colors_in_use, pale, a, r, g, b, sz, j, k, pix;
	switch (mode) {
		default:
			png_set_IHDR(png_ptr, info_ptr, realwidth, realheight, bit_depth, typ, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
			png_write_info(png_ptr, info_ptr);
//			png_set_invert_alpha(png_ptr);
			png_set_bgr(png_ptr);

			row_pointers = (png_bytep *)malloc(realheight * sizeof(png_bytep));
			if (row_pointers == NULL) return -1;

			for (k=0; k<realheight; k++) row_pointers[k] = ((png_bytep)src_bits) + ((skiprows+k)*src_width+skipcols)*bytes_per_pixel;
			for (k=0; k<realheight; k++) {
				rptr = ((png_bytep)src_bits) + ((skiprows+k)*src_width+skipcols)*bytes_per_pixel;
				for (j=0; j<realwidth; j++, rptr += 4) {
					rptr[3] &= 0x0c0;
				}
			}
			png_write_rows(png_ptr, &row_pointers[0], realheight);

			png_write_end(png_ptr, info_ptr);

			free((void *)row_pointers);
			break;
		case 1:						// convert L8 to L1
			typ = PNG_COLOR_TYPE_GRAY;  bit_depth = 1;
			png_set_IHDR(png_ptr, info_ptr, realwidth, realheight, bit_depth, typ, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
			png_write_info(png_ptr, info_ptr);
			sz = ((realwidth+31) >> 3) & 0x0ffffffc;
			bits = (unsigned char *)malloc(sz);
			if (bits == NULL) return -1;
			palt = (unsigned char *)malloc(1024);	// enough for 256 ARGBs
			if (palt == NULL) return -1;
			for (k=0; k<realheight; k++) {
				rptr = ((png_bytep)src_bits) + ((skiprows+k)*src_width+skipcols)*bytes_per_pixel;
				for (j=0; j<sz; j++) bits[j] = 0xff;
				wptr = bits;
				for (j=0; j<realwidth; j++, rptr++) {
					if (*rptr > threshold) *wptr ^= 1 << (7 - (j & 7));
					if ((j & 7) == 7) wptr++;
				}
				png_write_row(png_ptr, bits); 
			}
			png_write_end(png_ptr, info_ptr);
			free(palt);
			free(bits);
			break;
		case 4:						// palettize [A]RGB8 to P2, P4 or P8 as needed
			typ = PNG_COLOR_TYPE_PALETTE;  bit_depth = 4;
			sz = (realwidth+3) & 0x0ffffffc;
			bits = (unsigned char *)malloc(sz);
			if (bits == NULL) return -1;
#define PALETTE_BPP 3
			palt = (unsigned char *)malloc(PNG_MAX_PALETTE_LENGTH * PALETTE_BPP);	// enough for 256 RGBs
			if (palt == NULL) { free(bits); return -1; }
			palta = (unsigned char *)malloc(PNG_MAX_PALETTE_LENGTH);	// enough for 256 As
			if (palta == NULL) { free(palt); free(bits); return -1; }
			colors_in_use = 0;
			palt[0] = palt[1] = palt[2] = palt[3] = 0;
			for (k=0; k<realheight; k++) {
				rptr = ((png_bytep)src_bits) + ((skiprows+k)*src_width+skipcols)*bytes_per_pixel;
				for (j=0; j<realwidth; j++) {
					r = *rptr++;
					g = *rptr++;
					b = *rptr++;
					a = *rptr++;
a &= 0x0f0;
					for (pale=0; pale<colors_in_use; pale++) {
						if ((r == palt[PALETTE_BPP*pale+2]) && (g == palt[PALETTE_BPP*pale+1]) && (b == palt[PALETTE_BPP*pale+0]) &&
							(a == palta[pale])) break;
					}
					if (pale == colors_in_use) {
						palt[PALETTE_BPP*colors_in_use+2] = r;
						palt[PALETTE_BPP*colors_in_use+1] = g;
						palt[PALETTE_BPP*colors_in_use+0] = b;
						palta[colors_in_use] = a;
						if (colors_in_use < PNG_MAX_PALETTE_LENGTH) colors_in_use++;
					}
				}
			}





			if (colors_in_use > 255) fprintf(stderr, "TBitmapManager::save(): PALETTE OVERFLOW!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			if (colors_in_use < 4) bit_depth = 2;
			if (colors_in_use >= 16) bit_depth = 8;

//
fprintf(stderr, "encode_png(mode 4): bit_depth=%d\n", bit_depth);

			png_set_IHDR(png_ptr, info_ptr, realwidth, realheight, bit_depth, typ, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

			png_set_PLTE(png_ptr, info_ptr, (png_color_struct *)palt, colors_in_use);//PNG_MAX_PALETTE_LENGTH);

			png_set_tRNS(png_ptr, info_ptr, palta, colors_in_use, NULL);

			png_write_info(png_ptr, info_ptr);

			rptr = (unsigned char *)src_bits;
			for (k=0; k<realheight; k++) {
				rptr = ((png_bytep)src_bits) + ((skiprows+k)*src_width+skipcols)*bytes_per_pixel;
				for (j=0; j<sz; j++) bits[j] = 0;
				wptr = bits;
				for (j=0; j<realwidth; j++) {
					r = *rptr++;
					g = *rptr++;
					b = *rptr++;
					a = *rptr++;
a &= 0x0f0;
					for (pale=0; pale<colors_in_use; pale++) {
						if ((r == palt[PALETTE_BPP*pale+2]) && (g == palt[PALETTE_BPP*pale+1]) && (b == palt[PALETTE_BPP*pale+0]) &&
							(a == palta[pale])) break;
					}
					switch (bit_depth) {
						case 2:
							switch (j & 3) {
								case 0:		pix  = (pale & 0x03) << 6;					break;
								case 1:		pix |= (pale & 0x03) << 4;					break;
								case 2:		pix |= (pale & 0x03) << 2;					break;
								case 3:		pix |= (pale & 0x03) << 0;	*wptr++ = pix;	break;
							}
							break;
						case 4:
							switch (j & 1) {
								case 0:		pix  = (pale & 0x0f) << 4;					break;
								case 1:		pix |=  pale & 0x0f;		*wptr++ = pix;	break;
							}
							break;
						case 8:
							*wptr++ = pale;
							break;
					}
				}
				png_write_row(png_ptr, bits); 
			}

			png_write_end(png_ptr, info_ptr);
			free(palt);
			free(palta);
			free(bits);
			break;
	}
	return 0;
}
*/


////////////////////////////////////////////////////////////////////////////////////////////////////////

// write an image file
void write_L8_file(char *bmap, int bmapw, int bmaph, const char *filename, const char *filesuffix) {
	FILE			*fd = NULL;
	png_structp		png_ptr;
	png_infop		info_ptr;
	char			fname[500];
	char			tfname[500];
	int				err;
	int				bytes_per_pixel = 1;

	png_bytep *row_pointers;
	int k;

	sprintf(fname, "%s%s", filename, filesuffix);
//printf("write_L8_file(%s)\n", fname);
	/* open the file */
	sprintf(tfname, "%s.inuse", fname);
	fd = my_open_fn(tfname, "wb");
	if (fd == NULL) goto BAIL1;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) goto BAIL1;		/* out of memory */

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {		   /* out of memory */
		png_destroy_write_struct(&(png_ptr), NULL);
		goto BAIL1;
	}

	png_set_write_fn(png_ptr, (void *)fd, my_write_fn, my_flush_fn);

//	err = encode_png(4, bmapw, bmaph, bmap, png_ptr, info_ptr, 0, 0, 0, 0, mode, threshold);
//static int encode_png(unsigned short src_channels, unsigned short src_width, unsigned short src_height, void *src_bits, png_structp png_ptr, png_infop info_ptr, int skiprows, int skipcols, int realwidth, int realheight, int mode, int threshold)
	png_set_IHDR(png_ptr, info_ptr, bmapw, bmaph, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);
//	png_set_invert_alpha(png_ptr);
	png_set_bgr(png_ptr);

	row_pointers = (png_bytep *)malloc(bmaph * sizeof(png_bytep));
	if (row_pointers == NULL) err = -1; else {
		for (k=0; k<bmaph; k++) row_pointers[k] = ((png_bytep)bmap) + ((k)*bmapw)*bytes_per_pixel;
/*
		for (k=0; k<bmaph; k++) {
			int j;
			unsigned char *rptr = ((png_bytep)bmap) + ((k)*bmapw)*bytes_per_pixel;
			for (j=0; j<bmapw; j++, rptr += 4) {
				rptr[3] &= 0x0c0;
			}
		}
*/
		png_write_rows(png_ptr, &row_pointers[0], bmaph);

		png_write_end(png_ptr, info_ptr);

		free((void *)row_pointers);

		err = 0;
	}
//
	png_destroy_write_struct(&png_ptr, &info_ptr);

	my_close_fn(fd);

	if (err == 0) {
		remove(fname);
		rename(tfname, fname);
	} else {
		fprintf(stderr, "ERROR writing png file... out of memory\n");
	}

	return;

BAIL1:
	if (fd) my_close_fn(fd);
	fprintf(stderr, "ERROR writing png file... fopen(%s) failure\n", fname);
}

void write_ARGB_file(int *bmap, int bmapw, int bmaph, const char *filename, const char *filesuffix) {
	FILE			*fd = NULL;
	png_structp		png_ptr;
	png_infop		info_ptr;
	char			fname[500];
	char			tfname[500];
	int				err;
	int				bytes_per_pixel = 4;

	png_bytep *row_pointers;
	int k;

	sprintf(fname, "%s%s", filename, filesuffix);
//printf("write_ARGB_file(%s)\n", fname);
	/* open the file */
	sprintf(tfname, "%s.inuse", fname);
	fd = my_open_fn(tfname, "wb");
	if (fd == NULL) goto BAIL1;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) goto BAIL1;		/* out of memory */

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {		   /* out of memory */
		png_destroy_write_struct(&(png_ptr), NULL);
		goto BAIL1;
	}

	png_set_write_fn(png_ptr, (void *)fd, my_write_fn, my_flush_fn);

//	err = encode_png(4, bmapw, bmaph, bmap, png_ptr, info_ptr, 0, 0, 0, 0, mode, threshold);
//static int encode_png(unsigned short src_channels, unsigned short src_width, unsigned short src_height, void *src_bits, png_structp png_ptr, png_infop info_ptr, int skiprows, int skipcols, int realwidth, int realheight, int mode, int threshold)
	png_set_IHDR(png_ptr, info_ptr, bmapw, bmaph, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);
//	png_set_invert_alpha(png_ptr);
	png_set_bgr(png_ptr);

	row_pointers = (png_bytep *)malloc(bmaph * sizeof(png_bytep));
	if (row_pointers == NULL) err = -1; else {
		for (k=0; k<bmaph; k++) row_pointers[k] = ((png_bytep)bmap) + ((k)*bmapw)*bytes_per_pixel;
/*
		for (k=0; k<bmaph; k++) {
			int j;
			unsigned char *rptr = ((png_bytep)bmap) + ((k)*bmapw)*bytes_per_pixel;
			for (j=0; j<bmapw; j++, rptr += 4) {
				rptr[3] &= 0x0c0;
			}
		}
*/
		png_write_rows(png_ptr, &row_pointers[0], bmaph);

		png_write_end(png_ptr, info_ptr);

		free((void *)row_pointers);

		err = 0;
	}
//
	png_destroy_write_struct(&png_ptr, &info_ptr);

	my_close_fn(fd);

	if (err == 0) {
		remove(fname);
		rename(tfname, fname);
	} else {
		fprintf(stderr, "ERROR writing png file... out of memory\n");
	}

	return;

BAIL1:
	if (fd) my_close_fn(fd);
	fprintf(stderr, "ERROR writing png file... fopen(%s) failure\n", fname);
}

void write_P8_ARGB_file(unsigned char *bmap, int bmapw, int bmaph, int *palptr, int palsize, int n_alphas, const char *filename, const char *filesuffix) {
	FILE			*fd = NULL;
	png_structp		png_ptr;
	png_infop		info_ptr;
	char			fname[500];
	char			tfname[500];

	unsigned char *rptr;
	unsigned char *palt;
	unsigned char *palta;
	int i;

	sprintf(fname, "%s%s", filename, filesuffix);
//printf("write_P8_ARGB_file(%s)\n", fname);
	/* open the file */
	sprintf(tfname, "%s.inuse", fname);
	fd = my_open_fn(tfname, "wb");
	if (fd == NULL) goto BAIL1;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) goto BAIL1;		/* out of memory */

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {		   /* out of memory */
		png_destroy_write_struct(&(png_ptr), NULL);
		goto BAIL1;
	}

	png_set_write_fn(png_ptr, (void *)fd, my_write_fn, my_flush_fn);

//	err = encode_png(4, bmapw, bmaph, bmap, png_ptr, info_ptr, 0, 0, 0, 0, mode, threshold);
//static int encode_png(unsigned short src_channels, unsigned short src_width, unsigned short src_height, void *src_bits, png_structp png_ptr, png_infop info_ptr, int skiprows, int skipcols, int realwidth, int realheight, int mode, int threshold)
	palt = (unsigned char *)malloc(PNG_MAX_PALETTE_LENGTH * 3);	// enough for 256 RGBs
	if (palt == NULL) {		   /* out of memory */
		goto BAIL2;
	}
	palta = (unsigned char *)malloc(PNG_MAX_PALETTE_LENGTH);	// enough for 256 As
	if (palta == NULL) {		   /* out of memory */
		free(palt);
		goto BAIL2;
	}
	//
	for (i=0; i<palsize; i++) {
		int c, a, r, g, b;
		c = palptr[i];
		DECOMPOSE_ARGB8(c, a, r, g, b);
		palta[i] = a;
		palt[3*i+0] = r;
		palt[3*i+1] = g;
		palt[3*i+2] = b;
	}
	//
	png_set_IHDR(png_ptr, info_ptr, bmapw, bmaph, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_set_PLTE(png_ptr, info_ptr, (png_color_struct *)palt, palsize);//PNG_MAX_PALETTE_LENGTH);

	png_set_tRNS(png_ptr, info_ptr, palta, n_alphas, NULL);

	png_write_info(png_ptr, info_ptr);

	for (i=0; i<bmaph; i++) {
		rptr = ((png_bytep)bmap) + ((i)*bmapw);
		png_write_row(png_ptr, rptr);
	}

	png_write_end(png_ptr, info_ptr);
	free(palt);
	free(palta);
//
	png_destroy_write_struct(&png_ptr, &info_ptr);

	my_close_fn(fd);

	remove(fname);
	rename(tfname, fname);

	return;

BAIL2:
	png_destroy_write_struct(&png_ptr, &info_ptr);

BAIL1:
	if (fd) my_close_fn(fd);
	fprintf(stderr, "ERROR writing png file... fopen(%s) failure\n", fname);
}

void write_P8_ARGB_file_frMS(unsigned char *bmap, int bmapw, int bmaph, char *frames_map, int frames_map_size, int *palptr, int palsize, int n_alphas, const char *filename, const char *filesuffix) {
	FILE			*fd = NULL;
	png_structp		png_ptr;
	png_infop		info_ptr;
	char			fname[500];
	char			tfname[500];

	unsigned char *rptr;
	unsigned char *palt;
	unsigned char *palta;
	int i;
	const char *chunk_name = "frMS";

	sprintf(fname, "%s%s", filename, filesuffix);
//printf("write_P8_ARGB_file(%s)\n", fname);
	/* open the file */
	sprintf(tfname, "%s.inuse", fname);
	fd = my_open_fn(tfname, "wb");
	if (fd == NULL) goto BAIL1;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) goto BAIL1;		/* out of memory */

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {		   /* out of memory */
		png_destroy_write_struct(&(png_ptr), NULL);
		goto BAIL1;
	}

	png_set_write_fn(png_ptr, (void *)fd, my_write_fn, my_flush_fn);

//	err = encode_png(4, bmapw, bmaph, bmap, png_ptr, info_ptr, 0, 0, 0, 0, mode, threshold);
//static int encode_png(unsigned short src_channels, unsigned short src_width, unsigned short src_height, void *src_bits, png_structp png_ptr, png_infop info_ptr, int skiprows, int skipcols, int realwidth, int realheight, int mode, int threshold)
	palt = (unsigned char *)malloc(PNG_MAX_PALETTE_LENGTH * 3);	// enough for 256 RGBs
	if (palt == NULL) {		   /* out of memory */
		goto BAIL2;
	}
	palta = (unsigned char *)malloc(PNG_MAX_PALETTE_LENGTH);	// enough for 256 As
	if (palta == NULL) {		   /* out of memory */
		free(palt);
		goto BAIL2;
	}
	//
	for (i=0; i<palsize; i++) {
		int c, a, r, g, b;
		c = palptr[i];
		DECOMPOSE_ARGB8(c, a, r, g, b);
		palta[i] = a;
		palt[3*i+0] = r;
		palt[3*i+1] = g;
		palt[3*i+2] = b;
	}
	//
	png_set_IHDR(png_ptr, info_ptr, bmapw, bmaph, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_set_PLTE(png_ptr, info_ptr, (png_color_struct *)palt, palsize);//PNG_MAX_PALETTE_LENGTH);

	png_set_tRNS(png_ptr, info_ptr, palta, n_alphas, NULL);

	png_write_info(png_ptr, info_ptr);

	png_write_chunk(png_ptr, (unsigned char *)chunk_name, (unsigned char *)frames_map, frames_map_size);

	for (i=0; i<bmaph; i++) {
		rptr = ((png_bytep)bmap) + ((i)*bmapw);
		png_write_row(png_ptr, rptr);
	}

	png_write_end(png_ptr, info_ptr);
	free(palt);
	free(palta);
//
	png_destroy_write_struct(&png_ptr, &info_ptr);

	my_close_fn(fd);

	remove(fname);
	rename(tfname, fname);

	return;

BAIL2:
	png_destroy_write_struct(&png_ptr, &info_ptr);

BAIL1:
	if (fd) my_close_fn(fd);
	fprintf(stderr, "ERROR writing png file... fopen(%s) failure\n", fname);
}

void write_RGB_tRNS_file(int *bmap, int bmapw, int bmaph, const char *filename, const char *filesuffix, int trans_r, int trans_g, int trans_b) {
	FILE			*fd = NULL;
	png_structp		png_ptr;
	png_infop		info_ptr;
	char			fname[500];
	char			tfname[500];
	int				err;
	int				bytes_per_pixel = 4;

	png_bytep		row;
	int k;

	sprintf(fname, "%s%s", filename, filesuffix);
//printf("write_RGB_tRNS_file(%s)\n", fname);
	/* open the file */
	sprintf(tfname, "%s.inuse", fname);
	fd = my_open_fn(tfname, "wb");
	if (fd == NULL) goto BAIL1;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) goto BAIL1;		/* out of memory */

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {		   /* out of memory */
		png_destroy_write_struct(&(png_ptr), NULL);
		goto BAIL1;
	}

	png_set_write_fn(png_ptr, (void *)fd, my_write_fn, my_flush_fn);

//	err = encode_png(4, bmapw, bmaph, bmap, png_ptr, info_ptr, 0, 0, 0, 0, mode, threshold);
//static int encode_png(unsigned short src_channels, unsigned short src_width, unsigned short src_height, void *src_bits, png_structp png_ptr, png_infop info_ptr, int skiprows, int skipcols, int realwidth, int realheight, int mode, int threshold)
	png_set_IHDR(png_ptr, info_ptr, bmapw, bmaph, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_color_16 color;
	color.index = 0;
	color.red = trans_r;
	color.green = trans_g;
	color.blue = trans_b;
	png_set_tRNS(png_ptr, info_ptr, NULL, 0, &color);
	png_write_info(png_ptr, info_ptr);
//	png_set_invert_alpha(png_ptr);
	png_set_bgr(png_ptr);

	row = (png_bytep)malloc(bmaph * 3 * sizeof(png_bytep));
	if (row == NULL) err = -1; else {
		for (k=0; k<bmaph; k++) {
			int j;
			unsigned char *rptr = ((png_bytep)bmap) + ((k)*bmapw)*bytes_per_pixel;
			png_bytep wptr = row;
			for (j=0; j<bmapw; j++, rptr += 4) {
				*wptr++ = (png_byte)rptr[0];
				*wptr++ = (png_byte)rptr[1];
				*wptr++ = (png_byte)rptr[2];
			}
			//
			png_write_row(png_ptr, row);
		}

		png_write_end(png_ptr, info_ptr);

		free((void *)row);

		err = 0;
	}
//
	png_destroy_write_struct(&png_ptr, &info_ptr);

	my_close_fn(fd);

	if (err == 0) {
		remove(fname);
		rename(tfname, fname);
	} else {
		fprintf(stderr, "ERROR writing png file... out of memory\n");
	}

	return;

BAIL1:
	if (fd) my_close_fn(fd);
	fprintf(stderr, "ERROR writing png file... fopen(%s) failure\n", fname);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////

// shrink an image and return a new /shrinkratio image (caller assumes size is oldsize/shrinkratio)
//  offsets may cause some samples off the bitmap, in which case they don't contribute to the box filter
int *shrink_map(int *bmap, int bmapw, int bmaph, int shrinkratio, int offsetx, int offsety) {
	int x, y, xx, yy, a, r, g, b, d;
	int sz = ((bmaph+shrinkratio-1)/shrinkratio) * ((bmapw+shrinkratio-1)/shrinkratio);
	int *m = (int *)malloc(sz * 4);
	int *rm = bmap;
	int *wm = m;
	for (y=offsety; y<bmaph; y+=shrinkratio) {
		for (x=offsetx; x<bmapw; x+=shrinkratio) {
			a = r = g = b = d = 0;
			for (yy=y; yy<(y+shrinkratio); yy++) if (yy < bmaph) {
				rm = bmap + yy * bmapw + x;
				for (xx=x; xx<(x+shrinkratio); xx++) if (xx < bmapw) {
					int c, ca, cr, cg, cb;
					c = *rm++;
					DECOMPOSE_ARGB8(c, ca, cr, cg, cb);
					a += ca;
					r += cr;
					g += cg;
					b += cb;
					d++;
				}
			}
			// write out a pixel
if (wm < (m + sz))
			*wm++ = MAKE_ARGB8(a/d, r/d, g/d, b/d);
		}
	}
	return m;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

// make a new copy of the image, flipped
int *hflip_map(int *bmap, int bmapw, int bmaph) {
	int *m = (int *)malloc(bmapw * bmaph * 4);
	int *rm = bmap;
	int *wm = m;
	int x, y;
	for (y=0; y<bmaph; y++) {
		rm = bmap + y*bmapw + (bmapw-1);
		for (x=0; x<bmapw; x++) {
			*wm++ = *rm--;
		}
	}
	return m;
}

unsigned char *hflip_cmap(unsigned char *bmap, int bmapw, int bmaph) {
	unsigned char *m = (unsigned char *)malloc(bmapw * bmaph);
	unsigned char *rm = bmap;
	unsigned char *wm = m;
	int x, y;
	for (y=0; y<bmaph; y++) {
		rm = bmap + y*bmapw + (bmapw-1);
		for (x=0; x<bmapw; x++) {
			*wm++ = *rm--;
		}
	}
	return m;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

// make a new copy of the image
int *copy_map(int *bmap, int bmapw, int bmaph) {
	int *m = (int *)malloc(bmapw * bmaph * 4);
	int *rm = bmap;
	int *wm = m;
	int x, y;
	for (y=0; y<bmaph; y++) {
		for (x=0; x<bmapw; x++) {
			*wm++ = *rm++;
		}
	}
	return m;
}

unsigned char *copy_pmap(unsigned char *bmap, int bmapw, int bmaph) {
	unsigned char *m = (unsigned char *)malloc(bmapw * bmaph);
	unsigned char *rm = bmap;
	unsigned char *wm = m;
	int x, y;
	for (y=0; y<bmaph; y++) {
		for (x=0; x<bmapw; x++) {
			*wm++ = *rm++;
		}
	}
	return m;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

int replicate(int val, int sigbits) {
	int v = val >> sigbits;
	while (v != 0) {
		val += v;
		v = v >> sigbits;
	}
	return val;
}

// quantize an image IN PLACE by rounding each component to *bits bits, left shifted
void quantize_map(int *bmap, int bmapw, int bmaph, int abits, int rbits, int gbits, int bbits, int alpha_threshold=0x080) {
	int *rm = bmap;
	int x, y;
	for (y=0; y<bmaph; y++) {
		for (x=0; x<bmapw; x++) {
			int c, a, r, g, b;
			c = *rm;
			DECOMPOSE_ARGB8(c, a, r, g, b);
			if (abits == 0) {
				a = 255;
			} else if (abits == 1) {
//				a += 0x040;  if (a > 255) a = 255;  a &= 0x080;  a = replicate(a, abits);
				if (a > alpha_threshold) a = 0x0ff; else a = 0;
			} else {
				a += 0x080>>abits;  if (a > 255) a = 255;  a &= (0x0ff<<(8-abits))&0x0ff;  a = replicate(a, abits);
			}
			r += 0x080>>rbits;  if (r > 255) r = 255;  r &= (0x0ff<<(8-rbits))&0x0ff;  r = replicate(r, rbits);
			g += 0x080>>gbits;  if (g > 255) g = 255;  g &= (0x0ff<<(8-gbits))&0x0ff;  g = replicate(g, gbits);
			b += 0x080>>bbits;  if (b > 255) b = 255;  b &= (0x0ff<<(8-bbits))&0x0ff;  b = replicate(b, bbits);
			*rm++ = MAKE_ARGB8(a, r, g, b);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_IMAGES_COMBINED 4

typedef struct {
	int			argb[MAX_IMAGES_COMBINED];
	int			cnt;
} histogram_entry;

typedef struct {
	int			firstidx;		// index into histogram of first color in this box
	int			cnt;			// count of colors in this box
	int			sum;			// count of # pixels using any color in this box
	int			color[MAX_IMAGES_COMBINED];
} box_entry;

int g_num_images;
int g_which_image = 0;
//static int rsort_func_1(const void *ch1, const void *ch2) {
//	histogram_entry *c1 = (histogram_entry *)ch1;
//	histogram_entry *c2 = (histogram_entry *)ch2;
//	return c2->cnt - c1->cnt;
//}

static int sort_func_alpha_all(const void *ch1, const void *ch2) {
	histogram_entry *c1 = (histogram_entry *)ch1;
	histogram_entry *c2 = (histogram_entry *)ch2;
/*
	int min = 99999;
	int v;
	for (int i=0; i<g_num_images; i++) {
		v = ((c1->argb[i] >> 24) & 0x0ff) - ((c2->argb[i] >> 24) & 0x0ff);
		if (min > v) min = v;
	}
	return min;
*/
// all alphas == 0
// everything else: sort by highest alpha? is sorting important here?
// all alphas == 0x0ff
	int i, a, all_zero, all_ff, max_a, all_zero2, all_ff2, max_a2;
	all_zero = all_zero2 = 1;
	all_ff = all_ff2 = 1;
	max_a = max_a2 = 0;
	for (i=0; i<g_num_images; i++) {
		a = (c1->argb[i] >> 24) & 0x0ff;
		if (a != 0) all_zero = 0;
		if (a != 0x0ff) all_ff = 0;
		if (max_a < a) max_a = a;
		//
		a = (c2->argb[i] >> 24) & 0x0ff;
		if (a != 0) all_zero2 = 0;
		if (a != 0x0ff) all_ff2 = 0;
		if (max_a2 < a) max_a2 = a;
	}
	if ((all_zero == 1) && (all_zero2 != 1)) return -1;
	if ((all_zero != 1) && (all_zero2 == 1)) return +1;
	if ((all_ff == 1) && (all_ff2 != 1)) return +1;
	if ((all_ff != 1) && (all_ff2 == 1)) return -1;
	return 0;		// don't sort
}

static int sort_box_alpha(const void *ch1, const void *ch2) {
	box_entry *c1 = (box_entry *)ch1;
	box_entry *c2 = (box_entry *)ch2;
/*
	int min = 99999;
	int v;
	for (int i=0; i<g_num_images; i++) {
		v = ((c1->color[i] >> 24) & 0x0ff) - ((c2->color[i] >> 24) & 0x0ff);
		if (min > v) min = v;
	}
	return min;
*/
// all alphas == 0
// everything else: sort by highest alpha? is sorting important here?
// all alphas == 0x0ff
	int i, a, all_zero, all_ff, max_a, all_zero2, all_ff2, max_a2;
	all_zero = all_zero2 = 1;
	all_ff = all_ff2 = 1;
	max_a = max_a2 = 0;
	for (i=0; i<g_num_images; i++) {
		a = (c1->color[i] >> 24) & 0x0ff;
		if (a != 0) all_zero = 0;
		if (a != 0x0ff) all_ff = 0;
		if (max_a < a) max_a = a;
		//
		a = (c2->color[i] >> 24) & 0x0ff;
		if (a != 0) all_zero2 = 0;
		if (a != 0x0ff) all_ff2 = 0;
		if (max_a2 < a) max_a2 = a;
	}
	if ((all_zero == 1) && (all_zero2 != 1)) return -1;
	if ((all_zero != 1) && (all_zero2 == 1)) return +1;
	if ((all_ff == 1) && (all_ff2 != 1)) return +1;
	if ((all_ff != 1) && (all_ff2 == 1)) return -1;
	return 0;		// don't sort
}

static int sort_func_alpha(const void *ch1, const void *ch2) {
	histogram_entry *c1 = (histogram_entry *)ch1;
	histogram_entry *c2 = (histogram_entry *)ch2;
	return ((c1->argb[g_which_image] >> 24) & 0x0ff) - ((c2->argb[g_which_image] >> 24) & 0x0ff);
}

static int sort_func_red(const void *ch1, const void *ch2) {
	histogram_entry *c1 = (histogram_entry *)ch1;
	histogram_entry *c2 = (histogram_entry *)ch2;
	return ((c1->argb[g_which_image] >> 16) & 0x0ff) - ((c2->argb[g_which_image] >> 16) & 0x0ff);
}

static int sort_func_green(const void *ch1, const void *ch2) {
	histogram_entry *c1 = (histogram_entry *)ch1;
	histogram_entry *c2 = (histogram_entry *)ch2;
	return ((c1->argb[g_which_image] >>  8) & 0x0ff) - ((c2->argb[g_which_image] >>  8) & 0x0ff);
}

static int sort_func_blue(const void *ch1, const void *ch2) {
	histogram_entry *c1 = (histogram_entry *)ch1;
	histogram_entry *c2 = (histogram_entry *)ch2;
	return ((c1->argb[g_which_image]      ) & 0x0ff) - ((c2->argb[g_which_image]      ) & 0x0ff);
}

static int rsort_box_sum(const void *ch1, const void *ch2) {
	box_entry *c1 = (box_entry *)ch1;
	box_entry *c2 = (box_entry *)ch2;
	return c2->sum - c1->sum;
}

// palettize an image and return a new palettized image of same size
unsigned char *palettize_map(int *bmap, int bmapw, int bmaph, int *palptr, int palsize, int dodither=1) {
// use error diffusion to dither new P8 image
	int *errors = (int *)malloc(4 * 4 * (bmapw+2));		// read from this one
	if (errors == NULL) return NULL;
	int *errors_1 = (int *)malloc(4 * 4 * (bmapw+2));	// write to this one (swap at end of scan line)
	if (errors == NULL) { free(errors); return NULL; }
	int *rm = bmap;
	int x, y, i;
	for (i=0; i<(4 * (bmapw+2)); i++) errors[i] = errors_1[i] = 0;
	unsigned char *m = (unsigned char *)malloc(bmapw * bmaph);
	if (m != NULL) {
		unsigned char *wm = m;
		for (y=0; y<bmaph; y++) {
			int c, palidx;
			int a, r, g, b;
			int pa, pr, pg, pb;
			int ea, er, eg, eb;		// last error
			ea = er = eg = eb = 0;
			for (x=0; x<bmapw; x++) {
				// fetch and decompose source color
				c = *rm++;
				DECOMPOSE_ARGB8(c, a, r, g, b);
				if ((dodither != 0) && (a != 0)) {
					// subtract error[x+1] by component
					a -= errors[4*(x+1)+0];
					r -= errors[4*(x+1)+1];
					g -= errors[4*(x+1)+2];
					b -= errors[4*(x+1)+3];
					// subtract last error*7/16
					a -= (ea * 7) >> 4;
					r -= (er * 7) >> 4;
					g -= (eg * 7) >> 4;
					b -= (eb * 7) >> 4;
				}
				// search for best match ?SAD? in palette
				int bestSAD = 0x0fffffff;
				palidx = 0;
				for (i=0; i<palsize; i++) {
					int SAD = 0;
					c = palptr[i];
					DECOMPOSE_ARGB8(c, pa, pr, pg, pb);
					SAD += ABS(pa - a);
					SAD += ABS(pr - r);
					SAD += ABS(pg - g);
					SAD += ABS(pb - b);
					if (bestSAD > SAD) {
						palidx = i;
						bestSAD = SAD;
					}
				}
				c = palptr[palidx];
				DECOMPOSE_ARGB8(c, pa, pr, pg, pb);
				// output index
				*wm++ = palidx;
				// compute new error
				if (a == 0) {
					ea = er = eg = eb = 0;
				} else {
					ea = 0;//pa - a;		// ignore A error: just do the best you can
					er = pr - r;
					eg = pg - g;
					eb = pb - b;
				}
				// propagate error to the next scan line
				errors_1[4*(x  )+0] += (ea*3)>>4;
				errors_1[4*(x  )+1] += (er*3)>>4;
				errors_1[4*(x  )+2] += (eg*3)>>4;
				errors_1[4*(x  )+3] += (eb*3)>>4;
				errors_1[4*(x+1)+0] += (ea*5)>>4;
				errors_1[4*(x+1)+1] += (er*5)>>4;
				errors_1[4*(x+1)+2] += (eg*5)>>4;
				errors_1[4*(x+1)+3] += (eb*5)>>4;
				errors_1[4*(x+2)+0] += (ea  )>>4;
				errors_1[4*(x+2)+1] += (er  )>>4;
				errors_1[4*(x+2)+2] += (eg  )>>4;
				errors_1[4*(x+2)+3] += (eb  )>>4;
			}
			int *t = errors_1;
			errors_1 = errors;
			errors = t;
			for (i=0; i<(4 * (bmapw+2)); i++) errors_1[i] = 0;
		}
	}
	free(errors);
	free(errors_1);
	return m;
}

// palettize multiple images and return a new palettized image of same size
unsigned char *palettize_multi(int **bmaps, int bmapcnt, int bmapw, int bmaph, int *palptr, int palsize, int dodither=1) {
	int x, y, i, ii;
// use error diffusion to dither new P8 image
	int *errors = (int *)malloc(MAX_IMAGES_COMBINED * 4 * 4 * (bmapw+2));		// read from this one
	if (errors == NULL) return NULL;
	int *errors_1 = (int *)malloc(MAX_IMAGES_COMBINED * 4 * 4 * (bmapw+2));	// write to this one (swap at end of scan line)
	if (errors == NULL) { free(errors); return NULL; }
	int *rm[MAX_IMAGES_COMBINED];
	for (ii=0; ii<bmapcnt; ii++)
		rm[ii] = bmaps[ii];
	for (i=0; i<(MAX_IMAGES_COMBINED * 4 * (bmapw+2)); i++) errors[i] = errors_1[i] = 0;
	unsigned char *m = (unsigned char *)malloc(bmapw * bmaph);
	if (m != NULL) {
		unsigned char *wm = m;
		for (y=0; y<bmaph; y++) {
			int c, palidx;
			int a[MAX_IMAGES_COMBINED], r[MAX_IMAGES_COMBINED], g[MAX_IMAGES_COMBINED], b[MAX_IMAGES_COMBINED];
			int pa[MAX_IMAGES_COMBINED], pr[MAX_IMAGES_COMBINED], pg[MAX_IMAGES_COMBINED], pb[MAX_IMAGES_COMBINED];
			int ea[MAX_IMAGES_COMBINED], er[MAX_IMAGES_COMBINED], eg[MAX_IMAGES_COMBINED], eb[MAX_IMAGES_COMBINED];		// last error
			for (ii=0; ii<bmapcnt; ii++) ea[ii] = er[ii] = eg[ii] = eb[ii] = 0;
			for (x=0; x<bmapw; x++) {
				// fetch and decompose source color
				for (ii=0; ii<bmapcnt; ii++) {
					c = *rm[ii]++;
					DECOMPOSE_ARGB8(c, a[ii], r[ii], g[ii], b[ii]);
					if ((dodither != 0) && (a[ii] != 0)) {
						// subtract error[x+1] by component
						a[ii] -= errors[(4*(x+1)+0)*MAX_IMAGES_COMBINED+ii];
						r[ii] -= errors[(4*(x+1)+1)*MAX_IMAGES_COMBINED+ii];
						g[ii] -= errors[(4*(x+1)+2)*MAX_IMAGES_COMBINED+ii];
						b[ii] -= errors[(4*(x+1)+3)*MAX_IMAGES_COMBINED+ii];
						// subtract last error*7/16
						a[ii] -= (ea[ii] * 7) >> 4;
						r[ii] -= (er[ii] * 7) >> 4;
						g[ii] -= (eg[ii] * 7) >> 4;
						b[ii] -= (eb[ii] * 7) >> 4;
					}
				}
				// search for best match ?SAD? in palette
				int bestSAD = 0x0fffffff;
				palidx = 0;
				for (i=0; i<palsize; i++) {
					int SAD = 0;
					for (ii=0; ii<bmapcnt; ii++) {
						c = palptr[i + ii*256];
						DECOMPOSE_ARGB8(c, pa[ii], pr[ii], pg[ii], pb[ii]);
						SAD += ABS(pa[ii] - a[ii]);
						SAD += ABS(pr[ii] - r[ii]);
						SAD += ABS(pg[ii] - g[ii]);
						SAD += ABS(pb[ii] - b[ii]);
					}
					if (bestSAD > SAD) {
						palidx = i;
						bestSAD = SAD;
					}
				}
				// output index
				*wm++ = palidx;
				// compute error and propagate
				for (ii=0; ii<bmapcnt; ii++) {
					// fetch color
					c = palptr[palidx + ii*256];
					DECOMPOSE_ARGB8(c, pa[ii], pr[ii], pg[ii], pb[ii]);
					// compute new error
					if (a[ii] == 0) {
						ea[ii] = er[ii] = eg[ii] = eb[ii] = 0;
					} else {
						ea[ii] = 0;//pa[ii] - a[ii];		// ignore A error: just do the best you can
						er[ii] = pr[ii] - r[ii];
						eg[ii] = pg[ii] - g[ii];
						eb[ii] = pb[ii] - b[ii];
					}
					// propagate error to the next scan line
					errors_1[(4*(x  )+0)*MAX_IMAGES_COMBINED+ii] += (ea[ii]*3)>>4;
					errors_1[(4*(x  )+1)*MAX_IMAGES_COMBINED+ii] += (er[ii]*3)>>4;
					errors_1[(4*(x  )+2)*MAX_IMAGES_COMBINED+ii] += (eg[ii]*3)>>4;
					errors_1[(4*(x  )+3)*MAX_IMAGES_COMBINED+ii] += (eb[ii]*3)>>4;
					errors_1[(4*(x+1)+0)*MAX_IMAGES_COMBINED+ii] += (ea[ii]*5)>>4;
					errors_1[(4*(x+1)+1)*MAX_IMAGES_COMBINED+ii] += (er[ii]*5)>>4;
					errors_1[(4*(x+1)+2)*MAX_IMAGES_COMBINED+ii] += (eg[ii]*5)>>4;
					errors_1[(4*(x+1)+3)*MAX_IMAGES_COMBINED+ii] += (eb[ii]*5)>>4;
					errors_1[(4*(x+2)+0)*MAX_IMAGES_COMBINED+ii] += (ea[ii]  )>>4;
					errors_1[(4*(x+2)+1)*MAX_IMAGES_COMBINED+ii] += (er[ii]  )>>4;
					errors_1[(4*(x+2)+2)*MAX_IMAGES_COMBINED+ii] += (eg[ii]  )>>4;
					errors_1[(4*(x+2)+3)*MAX_IMAGES_COMBINED+ii] += (eb[ii]  )>>4;
				}
			}
			int *t = errors_1;
			errors_1 = errors;
			errors = t;
			for (i=0; i<(MAX_IMAGES_COMBINED * 4 * (bmapw+2)); i++) errors_1[i] = 0;
		}
	}
	free(errors);
	free(errors_1);
	return m;
}

// unpalettize an image and return a new unpalettized image of same size
int *unpalettize_map(unsigned char *bmap, int bmapw, int bmaph, int *palptr, int palsize) {
	int *m = (int *)malloc(4 * bmapw * bmaph);
	if (m != NULL) {
		int *wm = m;
		unsigned char *rm = bmap;
		int x, y, c;
		for (y=0; y<bmaph; y++) {
			for (x=0; x<bmapw; x++) {
				c = *rm++;
				if (c >= palsize) c = 0;
				*wm++ = palptr[c];
			}
		}
	}
	return m;
}

// for this map, generate a reasonable ARGB palette, sorted by increasing A
//  if given n images, try to make a single P8 image that has n palettes
int generate_palette(int **bmaps, int bmapcnt, int bmapw, int bmaph, int desired_colors, int *pals_out, int verbose=0) {
if (verbose) printf("* Generating Palette: target %d colors\n", desired_colors);
// make a list of all colors (ARGB)* with count (histogram)
	histogram_entry *histogram = NULL;
	int histsize = 0;
	int histalloc = 0;
	int x, y, i, ii, j, k, a, r, g, b;
	int curbox;
if (verbose) printf("* Gathering histogram\n");
	{
		int *rm[MAX_IMAGES_COMBINED];
		for (ii=0; ii<bmapcnt; ii++)
			rm[ii] = bmaps[ii];
		for (y=0; y<bmaph; y++) {
			for (x=0; x<bmapw; x++) {
				int c[MAX_IMAGES_COMBINED];
				for (ii=0; ii<bmapcnt; ii++) {
					c[ii] = *rm[ii]++;
//					if ((c[ii] & 0xff000000) == 0) c[ii] = 0;
				}
				for (j=0; j<histsize; j++) {
					int found = 1;
					for (ii=0; ii<bmapcnt; ii++)
						if (c[ii] != histogram[j].argb[ii]) found = 0;
					if (found) break;
				}
				if (j == histsize) {
					if (histsize >= histalloc) {
						histalloc += 100;
						histogram_entry *nh = (histogram_entry *)malloc(sizeof(histogram_entry) * histalloc);
						for (i=0; i<histalloc; i++) {
							for (ii=0; ii<MAX_IMAGES_COMBINED; ii++)
								nh[i].argb[ii] = 0;
							nh[i].cnt = 0;
						}
						for (i=0; i<histsize; i++) nh[i] = histogram[i];
						if (histogram != NULL) free(histogram);
						histogram = nh;
					}
					for (ii=0; ii<bmapcnt; ii++)
						histogram[j].argb[ii] = c[ii];
					histogram[j].cnt = 1;
					histsize++;
				} else {
					histogram[j].cnt++;
				}
			}
		}
	}
//	qsort(histogram, histsize, sizeof(histogram_entry), rsort_func_1);
if (verbose) printf("* Sorting histogram\n");
	g_num_images = bmapcnt;
	qsort(histogram, histsize, sizeof(histogram_entry), sort_func_alpha_all);
if (verbose) {
printf("histsize=%d, histalloc=%d\n", histsize, histalloc);
for (i=0; i<histsize; i++) {
	printf("      %4d:", i);
	for (ii=0; ii<MAX_IMAGES_COMBINED; ii++)
		printf(" %08x", histogram[i].argb[ii]);
	printf(" %d\n", histogram[i].cnt);
}
}
// alloc a list of boxes
if (verbose) printf("* Initing boxes\n");
	box_entry *boxes = (box_entry *)malloc(sizeof(box_entry) * desired_colors);
	for (i=0; i<desired_colors; i++) {
		boxes[i].cnt = 0;
		boxes[i].firstidx = 0;
		boxes[i].sum = 0;
		for (ii=0; ii<MAX_IMAGES_COMBINED; ii++)
			boxes[i].color[ii] = 0;
	}
	int boxes_in_use = 0;
	int sum;
// split into 3 boxes: all images alpha=0(transparent), in between, all images alpha=255(opaque)
	unsigned int c = 0;
	for (ii=0; ii<bmapcnt; ii++)
		c |= histogram[0].argb[ii] & 0xff000000;
	if (c == 0) {												// all image transparent pixels present
// box 0 is transparent (color does not matter)
		sum = 0;
		for (i=0; i<histsize; i++) {
			c = 0;
			for (ii=0; ii<bmapcnt; ii++)
				c |= histogram[i].argb[ii] & 0xff000000;
			if (c != 0) break;
			sum += histogram[i].cnt;
		}
		boxes[boxes_in_use].cnt = i;
		boxes[boxes_in_use].firstidx = 0;
		boxes[boxes_in_use].sum = sum;
		boxes_in_use++;
// box 1 is translucent
		boxes[boxes_in_use].cnt = histsize-i;
		boxes[boxes_in_use].firstidx = i;
		boxes[boxes_in_use].sum = bmapw*bmaph - sum;
		boxes_in_use++;
		c = 0xff000000;
		for (ii=0; ii<bmapcnt; ii++)
			c &= histogram[histsize-1].argb[ii] & 0xff000000;
		if (c == 0xff000000) {									// all image opaque pixels present
			// 1, 2, 3
// box 2 is opaque
			sum = 0;
			for (i=histsize; i>0; i--) {
				c = 0xff000000;
				for (ii=0; ii<bmapcnt; ii++)
					c &= histogram[i-1].argb[ii] & 0xff000000;
				if (c != 0xff000000) break;
				sum += histogram[i-1].cnt;
			}
			if (boxes[boxes_in_use-1].sum != sum) {
				boxes[boxes_in_use-1].cnt -= histsize-i;
				boxes[boxes_in_use-1].sum -= sum;
			} else {
				boxes_in_use--;
			}
			boxes[boxes_in_use].cnt = histsize-i;
			boxes[boxes_in_use].firstidx = i;
			boxes[boxes_in_use].sum = sum;
			boxes_in_use++;
		}
	} else {										// no transparent pixels present
// box 0 is translucent
		boxes[boxes_in_use].cnt = histsize;
		boxes[boxes_in_use].firstidx = 0;
		boxes[boxes_in_use].sum = bmapw*bmaph;
		boxes_in_use++;
		c = 0xff000000;
		for (ii=0; ii<bmapcnt; ii++)
			c &= histogram[histsize-1].argb[ii] & 0xff000000;
		if (c == 0xff000000) {									// all image opaque pixels present
			// 2, 3
// box 1 is opaque
			sum = 0;
			for (i=histsize; i>0; i--) {
				c = 0xff000000;
				for (ii=0; ii<bmapcnt; ii++)
					c &= histogram[i-1].argb[ii] & 0xff000000;
				if (c != 0xff000000) break;
				sum += histogram[i-1].cnt;
			}
			if (boxes[boxes_in_use-1].sum != sum) {
				boxes[boxes_in_use-1].cnt -= histsize-i;
				boxes[boxes_in_use-1].sum -= sum;
			} else {
				boxes_in_use--;
			}
			boxes[boxes_in_use].cnt = histsize-i;
			boxes[boxes_in_use].firstidx = i;
			boxes[boxes_in_use].sum = sum;
			boxes_in_use++;
		}
	}
//  qsort boxes to bring largest to the top
	qsort(boxes, boxes_in_use, sizeof(box_entry), rsort_box_sum);

if (verbose) {
printf(" --------- %d Boxes\n", boxes_in_use);
printf(" box#:  1st  cnt    color users\n");
printf("      hist:    color users\n");
for (curbox=0; curbox<boxes_in_use; curbox++) {
	printf(" %4d: %4d %4d", curbox, boxes[curbox].firstidx, boxes[curbox].cnt);
	for (ii=0; ii<MAX_IMAGES_COMBINED; ii++)
		printf(" %08x", boxes[curbox].color[ii]);
	printf(" %d\n", boxes[curbox].sum);
	for (j=0; j<boxes[curbox].cnt; j++) {
		k = boxes[curbox].firstidx+j;
		printf("      %4d:", k);
		for (ii=0; ii<MAX_IMAGES_COMBINED; ii++)
			printf(" %08x", histogram[k].argb[ii]);
		printf(" %d\n", histogram[k].cnt);
	}
}
}

// while <desired_colors boxes
if (verbose) printf("* Splitting boxes: ");
	while (boxes_in_use < desired_colors) {
//  find a box where cnt >= 2 -none-> done
if (verbose) printf(".");
		for (curbox=0; curbox<boxes_in_use; curbox++) {
			int c = 0;
			for (int jj=0; jj<boxes[curbox].cnt; jj++)
				for (ii=0; ii<bmapcnt; ii++)
					c |= histogram[boxes[curbox].firstidx+jj].argb[ii] & 0xff000000;
			if ((c != 0) && (boxes[curbox].cnt >= 2)) break;
		}
		if (curbox == boxes_in_use) break;			// didn't need all desired_colors colors
//  select largest range in a, r, g, or b
		int mina[MAX_IMAGES_COMBINED], maxa[MAX_IMAGES_COMBINED], minr[MAX_IMAGES_COMBINED], maxr[MAX_IMAGES_COMBINED], ming[MAX_IMAGES_COMBINED], maxg[MAX_IMAGES_COMBINED], minb[MAX_IMAGES_COMBINED], maxb[MAX_IMAGES_COMBINED];
		j = boxes[curbox].firstidx;
		for (ii=0; ii<bmapcnt; ii++) {
			DECOMPOSE_ARGB8(histogram[j].argb[ii], a, r, g, b);
			mina[ii] = maxa[ii] = a;
			minr[ii] = maxr[ii] = r;
			ming[ii] = maxg[ii] = g;
			minb[ii] = maxb[ii] = b;
			for (i=j+1; i<j+boxes[curbox].cnt; i++) {
				DECOMPOSE_ARGB8(histogram[i].argb[ii], a, r, g, b);
				if (mina[ii] > a) mina[ii] = a;  if (maxa[ii] < a) maxa[ii] = a;
				if (minr[ii] > r) minr[ii] = r;  if (maxr[ii] < r) maxr[ii] = r;
				if (ming[ii] > g) ming[ii] = g;  if (maxg[ii] < g) maxg[ii] = g;
				if (minb[ii] > b) minb[ii] = b;  if (maxb[ii] < b) maxb[ii] = b;
			}
			maxa[ii] -= mina[ii];
			maxr[ii] -= minr[ii];
			maxg[ii] -= ming[ii];
			maxb[ii] -= minb[ii];
		}
		int which_channel = 0;
		int which_image = 0;
		int max_value_found = 0;
		for (ii=0; ii<bmapcnt; ii++) {
			if (max_value_found < maxa[ii]) { max_value_found = maxa[ii]; which_channel = 0; which_image = ii; }
			if (max_value_found < maxr[ii]) { max_value_found = maxr[ii]; which_channel = 1; which_image = ii; }
			if (max_value_found < maxg[ii]) { max_value_found = maxg[ii]; which_channel = 2; which_image = ii; }
			if (max_value_found < maxb[ii]) { max_value_found = maxb[ii]; which_channel = 3; which_image = ii; }
		}
//  qsort histogram values within the box
		g_which_image = which_image;
		switch (which_channel) {
			case 0:		qsort(&histogram[j], boxes[curbox].cnt, sizeof(histogram_entry), sort_func_alpha);	break;
			case 1:		qsort(&histogram[j], boxes[curbox].cnt, sizeof(histogram_entry), sort_func_red);	break;
			case 2:		qsort(&histogram[j], boxes[curbox].cnt, sizeof(histogram_entry), sort_func_green);	break;
			case 3:		qsort(&histogram[j], boxes[curbox].cnt, sizeof(histogram_entry), sort_func_blue);	break;
		}
//  split the box so about half the pixels are in each of the new boxes (at least one color each)
		int lowersum = histogram[j].cnt;
		int halfsum = boxes[curbox].sum / 2;
		for (i=1; i<boxes[curbox].cnt-1; ++i) {
			if (lowersum >= halfsum) break;
			lowersum += histogram[j+i].cnt;
		}

//printf("   splitting %4d: %4d %4d %d\n", curbox, boxes[curbox].firstidx, boxes[curbox].cnt, boxes[curbox].sum);
		boxes[boxes_in_use].firstidx = j + i;
		boxes[boxes_in_use].cnt = boxes[curbox].cnt - i;
		boxes[boxes_in_use].sum = boxes[curbox].sum - lowersum;
		boxes[curbox].cnt = i;
		boxes[curbox].sum = lowersum;
//printf("   into      %4d: %4d %4d %d\n", curbox, boxes[curbox].firstidx, boxes[curbox].cnt, boxes[curbox].sum);
//printf("   and       %4d: %4d %4d %d\n", boxes_in_use, boxes[boxes_in_use].firstidx, boxes[boxes_in_use].cnt, boxes[boxes_in_use].sum);
		++boxes_in_use;
//  qsort boxes to bring largest to the top
		qsort(boxes, boxes_in_use, sizeof(box_entry), rsort_box_sum);
	}
if (verbose) printf("\n");
// for each box: find rep color by average weighted by count (-> palette)
	for (ii=0; ii<bmapcnt; ii++) {
		for (curbox=0; curbox<boxes_in_use; curbox++) {
			int sa, sr, sg, sb, ss;
			j = boxes[curbox].firstidx;
			sa = sr = sg = sb = ss = 0;
			for (i=j; i<(j+boxes[curbox].cnt); i++) {
				DECOMPOSE_ARGB8(histogram[i].argb[ii], a, r, g, b);
				sa += a * histogram[i].cnt;
				sr += r * histogram[i].cnt;
				sg += g * histogram[i].cnt;
				sb += b * histogram[i].cnt;
				ss += histogram[i].cnt;
/* alternative 1: not weighted
				sa += a;
				sr += r;
				sg += g;
				sb += b;
				ss++;
*/
			}
			sa /= ss;
			sr /= ss;
			sg /= ss;
			sb /= ss;
/* alternative 2: (not as good)
			compute min* and max* as
			sa = (maxa + mina) >> 1;
			sr = (maxr + minr) >> 1;
			sg = (maxg + ming) >> 1;
			sb = (maxb + minb) >> 1;
*/
			boxes[curbox].color[ii] = MAKE_ARGB8(sa, sr, sg, sb);
		}
	}
// sort palette by increasing alpha
if (verbose) printf("* Sorting Palette\n");
	qsort(boxes, boxes_in_use, sizeof(box_entry), sort_box_alpha);

if (verbose) {
printf(" --------- %d Boxes\n", boxes_in_use);
printf(" box#:  1st  cnt    color users\n");
printf("      hist:    color users\n");
for (curbox=0; curbox<boxes_in_use; curbox++) {
	printf(" %4d: %4d %4d", curbox, boxes[curbox].firstidx, boxes[curbox].cnt);
	for (ii=0; ii<MAX_IMAGES_COMBINED; ii++)
		printf(" %08x", boxes[curbox].color[ii]);
	printf(" %d\n", boxes[curbox].sum);
	for (j=0; j<boxes[curbox].cnt; j++) {
		k = boxes[curbox].firstidx+j;
		printf("      %4d:", k);
		for (ii=0; ii<MAX_IMAGES_COMBINED; ii++)
			printf(" %08x", histogram[k].argb[ii]);
		printf(" %d\n", histogram[k].cnt);
	}
}
}

	//
if (verbose) printf("* Converting Palette%s\n", bmapcnt>1?"s":"");
	for (ii=0; ii<bmapcnt; ii++) {
		for (curbox=0; curbox<boxes_in_use; curbox++) {
			pals_out[ii*desired_colors + curbox] = boxes[curbox].color[ii];
		}
		for (curbox=boxes_in_use; curbox<desired_colors; curbox++) {
			pals_out[ii*desired_colors + curbox] = 0;
		}
	}
	free(histogram);
	//
	return boxes_in_use;
}

