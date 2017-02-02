#ifndef _HAL_OGL_H_
#define _HAL_OGL_H_
#include "HAL.h"

class HAL_OGL : public HAL {
#include "HAL_incl.h"
// additional private members
	bool		vbo_bound_with_dup_tex_coords;	// used by unbind
	VBOHandle	vbo_bound;
	IBOHandle	ibo_bound;
	TMatrix		tos_matrix[3];
};
#endif // !_HAL_OGL_H_
