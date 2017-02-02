//
//  common.cpp
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#include "config.h"
#include "common.h"

#include <math.h>
#include <string.h>     // for memcpy()
//#include <stdlib.h>
#include <stdio.h>

#ifdef PCport			// define BUILD_DX9 for matrix handling
#define BUILD_DX9
#endif // PCport

#ifdef __APPLE__
#include <TargetConditionals.h>
//#if (TARGET_IPHONE_SIMULATOR == 0) && (TARGET_OS_IPHONE == 1)
//#ifdef _ARM_ARCH_7
//<hash>include "../oolongengine-read-only/Oolong Engine2/Math/neonmath/neon_matrix_impl.h"			// oolong
//#else
//<hash>include "../oolongengine-read-only/Oolong Engine2/Math/vfpmath/matrix_impl.h"			// oolong
//#endif
//#endif
#endif

//#define SANITY_CHECK
#ifdef SANITY_CHECK
#include "Mathematics.h"
#else // !SANITY_CHECK
#endif // !SANITY_CHECK



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////








//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




#ifndef __NEON_MATRIX_IMPL_H__
#define __NEON_MATRIX_IMPL_H__

#if (TARGET_IPHONE_SIMULATOR == 0) && (TARGET_OS_IPHONE == 1)
#ifdef _ARM_ARCH_7
#ifdef __arm__
#include "arm/arch.h"
#endif

// Matrixes are assumed to be stored in column major format according to OpenGL
// specification.

// Multiplies two 4x4 matrices (a,b) outputing a 4x4 matrix (output)
void NEON_Matrix4Mul(const float* a, const float* b, float* output );

// Multiplies a 4x4 matrix (m) with a vector 4 (v), outputing a vector 4
void NEON_Matrix4Vector4Mul(const float* m, const float* v, float* output);

#endif
#endif

#endif // __NEON_MATRIX_IMPL_H__



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




#if (TARGET_IPHONE_SIMULATOR == 0) && (TARGET_OS_IPHONE == 1)
#ifdef _ARM_ARCH_7
#else
//??? #include "../Math/vfpmath/matrix_impl.h"
#endif
#endif




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



static const TMatrix c_mIdentity = {
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	}
};

const TMatrix &TMatrix::operator= (const TMatrix &rhs) {
	memcpy(f, rhs.f, sizeof(f));
//	for (int i=0; i<16; i++) f[i] = rhs.f[i];
	return *this;
}
void TMatrix::multiply(TMatrix &mOut, const TMatrix &mA, const TMatrix &mB) {
#ifdef SANITY_CHECK
	MatrixMultiply(*(MATRIX *)&mOut, *(MATRIX *)&mA, *(MATRIX *)&mB);
#else // !SANITY_CHECK
//#if (TARGET_IPHONE_SIMULATOR == 0) && (TARGET_OS_IPHONE == 1)
//	#ifdef _ARM_ARCH_7
//		NEON_Matrix4Mul( mA.f, mB.f, mOut.f );
//	#else
//		Matrix4Mul(mA.f,
//					mB.f,
//					mOut.f);
//	#endif
//#else	
	TMatrix mRet;

	// Perform calculation on a dummy matrix (mRet)
	mRet.f[ 0] = mA.f[ 0]*mB.f[ 0] + mA.f[ 1]*mB.f[ 4] + mA.f[ 2]*mB.f[ 8] + mA.f[ 3]*mB.f[12];
	mRet.f[ 1] = mA.f[ 0]*mB.f[ 1] + mA.f[ 1]*mB.f[ 5] + mA.f[ 2]*mB.f[ 9] + mA.f[ 3]*mB.f[13];
	mRet.f[ 2] = mA.f[ 0]*mB.f[ 2] + mA.f[ 1]*mB.f[ 6] + mA.f[ 2]*mB.f[10] + mA.f[ 3]*mB.f[14];
	mRet.f[ 3] = mA.f[ 0]*mB.f[ 3] + mA.f[ 1]*mB.f[ 7] + mA.f[ 2]*mB.f[11] + mA.f[ 3]*mB.f[15];

	mRet.f[ 4] = mA.f[ 4]*mB.f[ 0] + mA.f[ 5]*mB.f[ 4] + mA.f[ 6]*mB.f[ 8] + mA.f[ 7]*mB.f[12];
	mRet.f[ 5] = mA.f[ 4]*mB.f[ 1] + mA.f[ 5]*mB.f[ 5] + mA.f[ 6]*mB.f[ 9] + mA.f[ 7]*mB.f[13];
	mRet.f[ 6] = mA.f[ 4]*mB.f[ 2] + mA.f[ 5]*mB.f[ 6] + mA.f[ 6]*mB.f[10] + mA.f[ 7]*mB.f[14];
	mRet.f[ 7] = mA.f[ 4]*mB.f[ 3] + mA.f[ 5]*mB.f[ 7] + mA.f[ 6]*mB.f[11] + mA.f[ 7]*mB.f[15];

	mRet.f[ 8] = mA.f[ 8]*mB.f[ 0] + mA.f[ 9]*mB.f[ 4] + mA.f[10]*mB.f[ 8] + mA.f[11]*mB.f[12];
	mRet.f[ 9] = mA.f[ 8]*mB.f[ 1] + mA.f[ 9]*mB.f[ 5] + mA.f[10]*mB.f[ 9] + mA.f[11]*mB.f[13];
	mRet.f[10] = mA.f[ 8]*mB.f[ 2] + mA.f[ 9]*mB.f[ 6] + mA.f[10]*mB.f[10] + mA.f[11]*mB.f[14];
	mRet.f[11] = mA.f[ 8]*mB.f[ 3] + mA.f[ 9]*mB.f[ 7] + mA.f[10]*mB.f[11] + mA.f[11]*mB.f[15];

	mRet.f[12] = mA.f[12]*mB.f[ 0] + mA.f[13]*mB.f[ 4] + mA.f[14]*mB.f[ 8] + mA.f[15]*mB.f[12];
	mRet.f[13] = mA.f[12]*mB.f[ 1] + mA.f[13]*mB.f[ 5] + mA.f[14]*mB.f[ 9] + mA.f[15]*mB.f[13];
	mRet.f[14] = mA.f[12]*mB.f[ 2] + mA.f[13]*mB.f[ 6] + mA.f[14]*mB.f[10] + mA.f[15]*mB.f[14];
	mRet.f[15] = mA.f[12]*mB.f[ 3] + mA.f[13]*mB.f[ 7] + mA.f[14]*mB.f[11] + mA.f[15]*mB.f[15];

	// Copy result in pResultMatrix
	mOut = mRet;
//#endif
#endif // !SANITY_CHECK
}
void TMatrix::identity(TMatrix &mOut) {
#ifdef SANITY_CHECK
	MatrixIdentity(*(MATRIX *)&mOut);
#else // !SANITY_CHECK
	mOut.f[ 0]=1.0f;	mOut.f[ 4]=0.0f;	mOut.f[ 8]=0.0f;	mOut.f[12]=0.0f;
	mOut.f[ 1]=0.0f;	mOut.f[ 5]=1.0f;	mOut.f[ 9]=0.0f;	mOut.f[13]=0.0f;
	mOut.f[ 2]=0.0f;	mOut.f[ 6]=0.0f;	mOut.f[10]=1.0f;	mOut.f[14]=0.0f;
	mOut.f[ 3]=0.0f;	mOut.f[ 7]=0.0f;	mOut.f[11]=0.0f;	mOut.f[15]=1.0f;
#endif // !SANITY_CHECK
}
void TMatrix::translate(TMatrix &mOut, const float fX, const float fY, const float fZ) {
#ifdef SANITY_CHECK
	MatrixTranslation(*(MATRIX *)&mOut, fX, fY, fZ);
#else // !SANITY_CHECK
	mOut.f[ 0]=1.0f;	mOut.f[ 4]=0.0f;	mOut.f[ 8]=0.0f;	mOut.f[12]=fX;
	mOut.f[ 1]=0.0f;	mOut.f[ 5]=1.0f;	mOut.f[ 9]=0.0f;	mOut.f[13]=fY;
	mOut.f[ 2]=0.0f;	mOut.f[ 6]=0.0f;	mOut.f[10]=1.0f;	mOut.f[14]=fZ;
	mOut.f[ 3]=0.0f;	mOut.f[ 7]=0.0f;	mOut.f[11]=0.0f;	mOut.f[15]=1.0f;
#endif // !SANITY_CHECK
}
void TMatrix::scaling(TMatrix &mOut, const float fX, const float fY, const float fZ) {
#ifdef SANITY_CHECK
	MatrixScaling(*(MATRIX *)&mOut, fX, fY, fZ);
#else // !SANITY_CHECK
	mOut.f[ 0]=fX;		mOut.f[ 4]=0.0f;	mOut.f[ 8]=0.0f;	mOut.f[12]=0.0f;
	mOut.f[ 1]=0.0f;	mOut.f[ 5]=fY;		mOut.f[ 9]=0.0f;	mOut.f[13]=0.0f;
	mOut.f[ 2]=0.0f;	mOut.f[ 6]=0.0f;	mOut.f[10]=fZ;		mOut.f[14]=0.0f;
	mOut.f[ 3]=0.0f;	mOut.f[ 7]=0.0f;	mOut.f[11]=0.0f;	mOut.f[15]=1.0f;
#endif // !SANITY_CHECK
}
void TMatrix::rotationAxis(TMatrix &mOut, const float fAngle, const float fX, const float fY, const float fZ) {
#ifdef SANITY_CHECK
	MatrixRotationAxis(*(MATRIX *)&mOut, fAngle, fX, fY, fZ);
#else // !SANITY_CHECK
	TVector3 axis(fX, fY, fZ);
	TVector3::normalize(axis, axis);
	float s = (float)sin(fAngle);
	float c = (float)cos(fAngle);
	float x, y, z;
	
	x = axis.x;
	y = axis.y;
	z = axis.z;
	
	mOut.f[ 0] = x * x * (1 - c) + c;
	mOut.f[ 4] = x * y * (1 - c) - (z * s);
	mOut.f[ 8] = x * z * (1 - c) + (y * s);
	mOut.f[12] = 0;
	
	mOut.f[ 1] = y * x * (1 - c) + (z * s);
	mOut.f[ 5] = y * y * (1 - c) + c;
	mOut.f[ 9] = y * z * (1 - c) - (x * s);
	mOut.f[13] = 0;
	
	mOut.f[ 2] = z * x * (1 - c) - (y * s);
	mOut.f[ 6] = z * y * (1 - c) + (x * s);
	mOut.f[10] = z * z * (1 - c) + c;
	mOut.f[14] = 0.0f;
	
	mOut.f[ 3] = 0.0f;
	mOut.f[ 7] = 0.0f;
	mOut.f[11] = 0.0f;
	mOut.f[15] = 1.0f;
#endif // !SANITY_CHECK
}
void TMatrix::rotationX(TMatrix &mOut, const float fAngle) {
#ifdef SANITY_CHECK
	MatrixRotationX(*(MATRIX *)&mOut, fAngle);
#else // !SANITY_CHECK
	float		fCosine, fSine;

	/* Precompute cos and sin */
#if defined(BUILD_DX9) || defined(BUILD_D3DM)
	fCosine	= (float)cos(-fAngle);
	fSine	= (float)sin(-fAngle);
#else
	fCosine	= (float)cos(fAngle);
	fSine	= (float)sin(fAngle);
#endif

	/* Create the trigonometric matrix corresponding to X Rotation */
	mOut.f[ 0]=1.0f;	mOut.f[ 4]=0.0f;	mOut.f[ 8]=0.0f;	mOut.f[12]=0.0f;
	mOut.f[ 1]=0.0f;	mOut.f[ 5]=fCosine;	mOut.f[ 9]=fSine;	mOut.f[13]=0.0f;
	mOut.f[ 2]=0.0f;	mOut.f[ 6]=-fSine;	mOut.f[10]=fCosine;	mOut.f[14]=0.0f;
	mOut.f[ 3]=0.0f;	mOut.f[ 7]=0.0f;	mOut.f[11]=0.0f;	mOut.f[15]=1.0f;
#endif // !SANITY_CHECK
}
void TMatrix::rotationY(TMatrix &mOut, const float fAngle) {
#ifdef SANITY_CHECK
	MatrixRotationY(*(MATRIX *)&mOut, fAngle);
#else // !SANITY_CHECK
	float		fCosine, fSine;

	/* Precompute cos and sin */
#if defined(BUILD_DX9) || defined(BUILD_D3DM)
	fCosine	= (float)cos(-fAngle);
	fSine	= (float)sin(-fAngle);
#else
	fCosine	= (float)cos(fAngle);
	fSine	= (float)sin(fAngle);
#endif

	/* Create the trigonometric matrix corresponding to Y Rotation */
	mOut.f[ 0]=fCosine;		mOut.f[ 4]=0.0f;	mOut.f[ 8]=-fSine;		mOut.f[12]=0.0f;
	mOut.f[ 1]=0.0f;		mOut.f[ 5]=1.0f;	mOut.f[ 9]=0.0f;		mOut.f[13]=0.0f;
	mOut.f[ 2]=fSine;		mOut.f[ 6]=0.0f;	mOut.f[10]=fCosine;		mOut.f[14]=0.0f;
	mOut.f[ 3]=0.0f;		mOut.f[ 7]=0.0f;	mOut.f[11]=0.0f;		mOut.f[15]=1.0f;
#endif // !SANITY_CHECK
}
void TMatrix::rotationZ(TMatrix &mOut, const float fAngle) {
#ifdef SANITY_CHECK
	MatrixRotationZ(*(MATRIX *)&mOut, fAngle);
#else // !SANITY_CHECK
	float		fCosine, fSine;

	/* Precompute cos and sin */
#if defined(BUILD_DX9) || defined(BUILD_D3DM)
	fCosine =	(float)cos(-fAngle);
	fSine =		(float)sin(-fAngle);
#else
	fCosine =	(float)cos(fAngle);
	fSine =		(float)sin(fAngle);
#endif

	/* Create the trigonometric matrix corresponding to Z Rotation */
	mOut.f[ 0]=fCosine;		mOut.f[ 4]=fSine;	mOut.f[ 8]=0.0f;	mOut.f[12]=0.0f;
	mOut.f[ 1]=-fSine;		mOut.f[ 5]=fCosine;	mOut.f[ 9]=0.0f;	mOut.f[13]=0.0f;
	mOut.f[ 2]=0.0f;		mOut.f[ 6]=0.0f;	mOut.f[10]=1.0f;	mOut.f[14]=0.0f;
	mOut.f[ 3]=0.0f;		mOut.f[ 7]=0.0f;	mOut.f[11]=0.0f;	mOut.f[15]=1.0f;
#endif // !SANITY_CHECK
}
void TMatrix::transpose(TMatrix &mOut, const TMatrix &mIn) {
#ifdef SANITY_CHECK
	MatrixTranspose(*(MATRIX *)&mOut, *(MATRIX *)&mIn);
#else // !SANITY_CHECK
	TMatrix mTmp;

	mTmp.f[ 0]=mIn.f[ 0];	mTmp.f[ 4]=mIn.f[ 1];	mTmp.f[ 8]=mIn.f[ 2];	mTmp.f[12]=mIn.f[ 3];
	mTmp.f[ 1]=mIn.f[ 4];	mTmp.f[ 5]=mIn.f[ 5];	mTmp.f[ 9]=mIn.f[ 6];	mTmp.f[13]=mIn.f[ 7];
	mTmp.f[ 2]=mIn.f[ 8];	mTmp.f[ 6]=mIn.f[ 9];	mTmp.f[10]=mIn.f[10];	mTmp.f[14]=mIn.f[11];
	mTmp.f[ 3]=mIn.f[12];	mTmp.f[ 7]=mIn.f[13];	mTmp.f[11]=mIn.f[14];	mTmp.f[15]=mIn.f[15];

	mOut = mTmp;
#endif // !SANITY_CHECK
}
void TMatrix::inverse(TMatrix &mOut, const TMatrix &mIn) {
#ifdef SANITY_CHECK
	MatrixInverse(*(MATRIX *)&mOut, *(MATRIX *)&mIn);
#else // !SANITY_CHECK
	TMatrix mDummyMatrix;
	double det_1;
	double pos, neg, temp;

	/* Calculate the determinant of submatrix A and determine if the
	   the matrix is singular as limited by the double precision
	   floating-point data representation. */
	pos = neg = 0.0;
	temp =  mIn.f[ 0] * mIn.f[ 5] * mIn.f[10];
	if (temp >= 0.0) pos += temp; else neg += temp;
	temp =  mIn.f[ 4] * mIn.f[ 9] * mIn.f[ 2];
	if (temp >= 0.0) pos += temp; else neg += temp;
	temp =  mIn.f[ 8] * mIn.f[ 1] * mIn.f[ 6];
	if (temp >= 0.0) pos += temp; else neg += temp;
	temp = -mIn.f[ 8] * mIn.f[ 5] * mIn.f[ 2];
	if (temp >= 0.0) pos += temp; else neg += temp;
	temp = -mIn.f[ 4] * mIn.f[ 1] * mIn.f[10];
	if (temp >= 0.0) pos += temp; else neg += temp;
	temp = -mIn.f[ 0] * mIn.f[ 9] * mIn.f[ 6];
	if (temp >= 0.0) pos += temp; else neg += temp;
	det_1 = pos + neg;

	/* Is the submatrix A singular? */
	double sing = det_1 / (pos - neg);
	if (sing < 0) sing = -sing;
	if ((det_1 == 0.0) || (sing < 1.0e-15)) {		/* Matrix M has no inverse */
		printf("Matrix has no inverse: singular matrix\n");
		return;
	} else {
		/* Calculate inverse(A) = adj(A) / det(A) */
		det_1 = 1.0 / det_1;
		mDummyMatrix.f[ 0] =   ( mIn.f[ 5] * mIn.f[10] - mIn.f[ 9] * mIn.f[ 6] ) * (float)det_1;
		mDummyMatrix.f[ 1] = - ( mIn.f[ 1] * mIn.f[10] - mIn.f[ 9] * mIn.f[ 2] ) * (float)det_1;
		mDummyMatrix.f[ 2] =   ( mIn.f[ 1] * mIn.f[ 6] - mIn.f[ 5] * mIn.f[ 2] ) * (float)det_1;
		mDummyMatrix.f[ 4] = - ( mIn.f[ 4] * mIn.f[10] - mIn.f[ 8] * mIn.f[ 6] ) * (float)det_1;
		mDummyMatrix.f[ 5] =   ( mIn.f[ 0] * mIn.f[10] - mIn.f[ 8] * mIn.f[ 2] ) * (float)det_1;
		mDummyMatrix.f[ 6] = - ( mIn.f[ 0] * mIn.f[ 6] - mIn.f[ 4] * mIn.f[ 2] ) * (float)det_1;
		mDummyMatrix.f[ 8] =   ( mIn.f[ 4] * mIn.f[ 9] - mIn.f[ 8] * mIn.f[ 5] ) * (float)det_1;
		mDummyMatrix.f[ 9] = - ( mIn.f[ 0] * mIn.f[ 9] - mIn.f[ 8] * mIn.f[ 1] ) * (float)det_1;
		mDummyMatrix.f[10] =   ( mIn.f[ 0] * mIn.f[ 5] - mIn.f[ 4] * mIn.f[ 1] ) * (float)det_1;

		/* Calculate -C * inverse(A) */
		mDummyMatrix.f[12] = - ( mIn.f[12] * mDummyMatrix.f[ 0] + mIn.f[13] * mDummyMatrix.f[ 4] + mIn.f[14] * mDummyMatrix.f[ 8] );
		mDummyMatrix.f[13] = - ( mIn.f[12] * mDummyMatrix.f[ 1] + mIn.f[13] * mDummyMatrix.f[ 5] + mIn.f[14] * mDummyMatrix.f[ 9] );
		mDummyMatrix.f[14] = - ( mIn.f[12] * mDummyMatrix.f[ 2] + mIn.f[13] * mDummyMatrix.f[ 6] + mIn.f[14] * mDummyMatrix.f[10] );

		/* Fill in last row */
		mDummyMatrix.f[ 3] = 0.0f;
		mDummyMatrix.f[ 7] = 0.0f;
		mDummyMatrix.f[11] = 0.0f;
		mDummyMatrix.f[15] = 1.0f;
	}

	/* Copy contents of dummy matrix in pfMatrix */
	mOut = mDummyMatrix;
#endif // !SANITY_CHECK
}
void TMatrix::inverseEx(TMatrix &mOut, const TMatrix &mIn) {
#ifdef SANITY_CHECK
	MatrixInverseEx(*(MATRIX *)&mOut, *(MATRIX *)&mIn);
#else // !SANITY_CHECK
	TMatrix		mTmp;
	float 			*ppfRows[4];
	float 			pfRes[4];
	float 			pfIn[20];
	int				i, j;

	for(i = 0; i < 4; ++i)
		ppfRows[i] = &pfIn[i * 5];

	/* Solve 4 sets of 4 linear equations */
	for(i = 0; i < 4; ++i) {
		for(j = 0; j < 4; ++j) {
			ppfRows[j][0] = c_mIdentity.f[i + 4 * j];
			memcpy(&ppfRows[j][1], &mIn.f[j * 4], 4 * sizeof(float));
		}

		TMatrix::linearEqSolve(pfRes, (float**)ppfRows, 4);

		for(j = 0; j < 4; ++j) {
			mTmp.f[i + 4 * j] = pfRes[j];
		}
	}

	mOut = mTmp;
#endif // !SANITY_CHECK
}
void TMatrix::lookAtLH(TMatrix &mOut, const TVector3 &vEye, const TVector3 &vAt, const TVector3 &vUp) {
#ifdef SANITY_CHECK
	MatrixLookAtLH(*(MATRIX *)&mOut, *(VECTOR3 *)&vEye, *(VECTOR3 *)&vAt, *(VECTOR3 *)&vUp);
#else // !SANITY_CHECK
	TVector3 f, vUpActual, s, u;
	TMatrix t;

	f.x = vEye.x - vAt.x;
	f.y = vEye.y - vAt.y;
	f.z = vEye.z - vAt.z;

	TVector3::normalize(f, f);
	TVector3::normalize(vUpActual, vUp);
	TVector3::cross(s, f, vUpActual);
	TVector3::cross(u, s, f);

	mOut.f[ 0] = s.x;
	mOut.f[ 1] = u.x;
	mOut.f[ 2] = -f.x;
	mOut.f[ 3] = 0;

	mOut.f[ 4] = s.y;
	mOut.f[ 5] = u.y;
	mOut.f[ 6] = -f.y;
	mOut.f[ 7] = 0;

	mOut.f[ 8] = s.z;
	mOut.f[ 9] = u.z;
	mOut.f[10] = -f.z;
	mOut.f[11] = 0;

	mOut.f[12] = 0;
	mOut.f[13] = 0;
	mOut.f[14] = 0;
	mOut.f[15] = 1;

	TMatrix::translate(t, -vEye.x, -vEye.y, -vEye.z);
	TMatrix::multiply(mOut, t, mOut);
#endif // !SANITY_CHECK
}
void TMatrix::lookAtRH(TMatrix &mOut, const TVector3 &vEye, const TVector3 &vAt, const TVector3 &vUp) {
#ifdef SANITY_CHECK
	MatrixLookAtRH(*(MATRIX *)&mOut, *(VECTOR3 *)&vEye, *(VECTOR3 *)&vAt, *(VECTOR3 *)&vUp);
#else // !SANITY_CHECK
	TVector3 f, vUpActual, s, u;
	TMatrix t;

	f.x = vAt.x - vEye.x;
	f.y = vAt.y - vEye.y;
	f.z = vAt.z - vEye.z;

	TVector3::normalize(f, f);
	TVector3::normalize(vUpActual, vUp);
	TVector3::cross(s, f, vUpActual);
	TVector3::cross(u, s, f);

	mOut.f[ 0] = s.x;
	mOut.f[ 1] = u.x;
	mOut.f[ 2] = -f.x;
	mOut.f[ 3] = 0;

	mOut.f[ 4] = s.y;
	mOut.f[ 5] = u.y;
	mOut.f[ 6] = -f.y;
	mOut.f[ 7] = 0;

	mOut.f[ 8] = s.z;
	mOut.f[ 9] = u.z;
	mOut.f[10] = -f.z;
	mOut.f[11] = 0;

	mOut.f[12] = 0;
	mOut.f[13] = 0;
	mOut.f[14] = 0;
	mOut.f[15] = 1;

	TMatrix::translate(t, -vEye.x, -vEye.y, -vEye.z);
	TMatrix::multiply(mOut, t, mOut);
#endif // !SANITY_CHECK
}
void TMatrix::perspectiveFovLH(TMatrix &mOut, const float fFOVy, const float fAspect, const float fNear, const float fFar, const bool bRotate) {
#ifdef SANITY_CHECK
	MatrixPerspectiveFovLH(*(MATRIX *)&mOut, fFOVy, fAspect, fNear, fFar, bRotate);
#else // !SANITY_CHECK
	float f, n, fRealAspect;

	if (bRotate) fRealAspect = 1.0f / fAspect;
	else fRealAspect = fAspect;

	// cotangent(a) == 1.0f / tan(a);
	f = 1.0f / (float)tan(fFOVy * 0.5f);
	n = 1.0f / (fFar - fNear);

	mOut.f[ 0] = f / fRealAspect;
	mOut.f[ 1] = 0;
	mOut.f[ 2] = 0;
	mOut.f[ 3] = 0;

	mOut.f[ 4] = 0;
	mOut.f[ 5] = f;
	mOut.f[ 6] = 0;
	mOut.f[ 7] = 0;

	mOut.f[ 8] = 0;
	mOut.f[ 9] = 0;
	mOut.f[10] = fFar * n;
	mOut.f[11] = 1;

	mOut.f[12] = 0;
	mOut.f[13] = 0;
	mOut.f[14] = -fFar * fNear * n;
	mOut.f[15] = 0;

	if (bRotate) {
		TMatrix mRotation, mTemp = mOut;
		TMatrix::rotationZ(mRotation, 90.0f*kPI/180.0f);
		TMatrix::multiply(mOut, mTemp, mRotation);
	}
#endif // !SANITY_CHECK
}
void TMatrix::perspectiveFovRH(TMatrix &mOut, const float fFOVy, const float fAspect, const float fNear, const float fFar, const bool bRotate) {
#ifdef SANITY_CHECK
	MatrixPerspectiveFovRH(*(MATRIX *)&mOut, fFOVy, fAspect, fNear, fFar, bRotate);
#else // !SANITY_CHECK
	float f, n, fRealAspect;

	if (bRotate) fRealAspect = 1.0f / fAspect;
	else fRealAspect = fAspect;

	// cotangent(a) == 1.0f / tan(a);
	f = 1.0f / (float)tan(fFOVy * 0.5f);
	n = 1.0f / (fNear - fFar);

	mOut.f[ 0] = f / fRealAspect;
	mOut.f[ 1] = 0;
	mOut.f[ 2] = 0;
	mOut.f[ 3] = 0;

	mOut.f[ 4] = 0;
	mOut.f[ 5] = f;
	mOut.f[ 6] = 0;
	mOut.f[ 7] = 0;

	mOut.f[ 8] = 0;
	mOut.f[ 9] = 0;
	mOut.f[10] = (fFar + fNear) * n;
	mOut.f[11] = -1;

	mOut.f[12] = 0;
	mOut.f[13] = 0;
	mOut.f[14] = (2 * fFar * fNear) * n;
	mOut.f[15] = 0;

	if (bRotate) {
		TMatrix mRotation, mTemp = mOut;
		TMatrix::rotationZ(mRotation, -90.0f*kPI/180.0f);
		TMatrix::multiply(mOut, mTemp, mRotation);
	}
#endif // !SANITY_CHECK
}
//void TMatrix::orthoLH(TMatrix &mOut, const float left, const float right, const float bottom, const float top, const float zn, const float zf, const bool bRotate) {
//#ifdef SANITY_CHECK
//	MatrixOrthoLH(*(MATRIX *)&mOut, w, h, zn, zf, bRotate);
//#else // !SANITY_CHECK
//	mOut.f[ 0] = 2 / w;
//	mOut.f[ 1] = 0;
//	mOut.f[ 2] = 0;
//	mOut.f[ 3] = 0;
//
//	mOut.f[ 4] = 0;
//	mOut.f[ 5] = 2 / h;
//	mOut.f[ 6] = 0;
//	mOut.f[ 7] = 0;
//
//	mOut.f[ 8] = 0;
//	mOut.f[ 9] = 0;
//	mOut.f[10] = 1 / (zf - zn);
//	mOut.f[11] = zn / (zn - zf);
//
//	mOut.f[12] = 0;
//	mOut.f[13] = 0;
//	mOut.f[14] = 0;
//	mOut.f[15] = 1;
//
//	if (bRotate) {
//		TMatrix mRotation, mTemp = mOut;
//		TMatrix::rotationZ(mRotation, -90.0f*kPI/180.0f);
//		TMatrix::multiply(mOut, mRotation, mTemp);
//	}
//#endif // !SANITY_CHECK
//}
void TMatrix::orthoRH(TMatrix &mOut, const float left, const float right, const float bottom, const float top, const float zn, const float zf, const bool bRotate) {
#ifdef SANITY_CHECK
	MatrixOrthoRH(*(MATRIX *)&mOut, w, h, zn, zf, bRotate);
#else // !SANITY_CHECK
	mOut.f[ 0] = 2 / (right - left);
	mOut.f[ 1] = 0;
	mOut.f[ 2] = 0;
	mOut.f[ 3] = 0;

	mOut.f[ 4] = 0;
	mOut.f[ 5] = 2 / (top - bottom);
	mOut.f[ 6] = 0;
	mOut.f[ 7] = 0;

	mOut.f[ 8] = 0;
	mOut.f[ 9] = 0;
	mOut.f[10] = -1 / (zn - zf);
	mOut.f[11] = 0;

	mOut.f[12] = -(right + left) / (right - left);
	mOut.f[13] = -(top + bottom) / (top - bottom);
	mOut.f[14] = zn / (zn - zf);
	mOut.f[15] = 1;

	if (bRotate) {
		TMatrix mRotation, mTemp = mOut;
		TMatrix::rotationZ(mRotation, -90.0f*kPI/180.0f);
		TMatrix::multiply(mOut, mRotation, mTemp);
	}
#endif // !SANITY_CHECK
}
void TMatrix::frustumRH(TMatrix &mOut, const float left, const float right, const float bottom, const float top, const float zn, const float zf, const bool bRotate) {
	mOut.f[ 0] = 2 * zn / (right - left);
	mOut.f[ 1] = 0;
	mOut.f[ 2] = 0;
	mOut.f[ 3] = 0;

	mOut.f[ 4] = 0;
	mOut.f[ 5] = 2 * zn / (top - bottom);
	mOut.f[ 6] = 0;
	mOut.f[ 7] = 0;

	mOut.f[ 8] = (right + left) / (right - left);
	mOut.f[ 9] = (top + bottom) / (top - bottom);
	mOut.f[10] = -(zf + zn) / (zf - zn);
	mOut.f[11] = -1;

	mOut.f[12] = 0;
	mOut.f[13] = 0;
	mOut.f[14] = -2 * zf * zn / (zf - zn);
	mOut.f[15] = 0;

	if (bRotate) {
		TMatrix mRotation, mTemp = mOut;
		TMatrix::rotationZ(mRotation, -90.0f*kPI/180.0f);
		TMatrix::multiply(mOut, mRotation, mTemp);
	}
}
void TMatrix::linearEqSolve(float * const pRes, float ** const pSrc, const int nCnt) {			// 2D array of floats. 4 Eq linear problem is 5x4 matrix, constants in first column.
#ifdef SANITY_CHECK
	MatrixLinearEqSolve(pRes, pSrc, nCnt);
#else // !SANITY_CHECK
	int		i, j, k;
	float	f;

#if 0
	/*
		Show the matrix in debug output
	*/
	_RPT1(_CRT_WARN, "LinearEqSolve(%d)\n", nCnt);
	for (i = 0; i < nCnt; ++i) {
		_RPT1(_CRT_WARN, "%.8f |", pSrc[i][0]);
		for(j = 1; j <= nCnt; ++j)
			_RPT1(_CRT_WARN, " %.8f", pSrc[i][j]);
		_RPT0(_CRT_WARN, "\n");
	}
#endif

	if (nCnt == 1) {
//		_ASSERT(pSrc[0][1] != 0);
		pRes[0] = pSrc[0][0] / pSrc[0][1];
		return;
	}

	// Loop backwards in an attempt avoid the need to swap rows
	i = nCnt;
	while (i) {
		--i;

		if (pSrc[i][nCnt] != 0) {
			// Row i can be used to zero the other rows; let's move it to the bottom
			if (i != (nCnt-1)) {
				for (j = 0; j <= nCnt; ++j) {
					// Swap the two values
					f = pSrc[nCnt-1][j];
					pSrc[nCnt-1][j] = pSrc[i][j];
					pSrc[i][j] = f;
				}
			}

			// Now zero the last columns of the top rows
			for (j = 0; j < (nCnt-1); ++j) {
//				_ASSERT(pSrc[nCnt-1][nCnt] != 0);
				f = pSrc[j][nCnt] / pSrc[nCnt-1][nCnt];

				// No need to actually calculate a zero for the final column
				for (k = 0; k < nCnt; ++k) {
					pSrc[j][k] -= f * pSrc[nCnt-1][k];
				}
			}

			break;
		}
	}

	// Solve the top-left sub matrix
	TMatrix::linearEqSolve(pRes, pSrc, nCnt - 1);

	// Now calc the solution for the bottom row
	f = pSrc[nCnt-1][0];
	for (k = 1; k < nCnt; ++k) {
		f -= pSrc[nCnt-1][k] * pRes[k-1];
	}
//	_ASSERT(pSrc[nCnt-1][nCnt] != 0);
	f /= pSrc[nCnt-1][nCnt];
	pRes[nCnt-1] = f;

#if 0
	{
		float fCnt;

		/*
			Verify that the result is correct
		*/
		fCnt = 0;
		for (i = 1; i <= nCnt; ++i) fCnt += pSrc[nCnt-1][i] * pRes[i-1];

		_ASSERT(_ABS(fCnt - pSrc[nCnt-1][0]) < 1e-3);
	}
#endif
#endif // !SANITY_CHECK
}

void TMatrix::dump() {
    printf("   %8f, %8f, %8f, %8f\n", f[0], f[1], f[2], f[3]);
    printf("   %8f, %8f, %8f, %8f\n", f[4], f[5], f[6], f[7]);
    printf("   %8f, %8f, %8f, %8f\n", f[8], f[9], f[10], f[11]);
    printf("   %8f, %8f, %8f, %8f\n", f[12], f[13], f[14], f[15]);
}

//

void TQuaternion::identity(
	TQuaternion		&qOut)
{
	qOut.x = 0;
	qOut.y = 0;
	qOut.z = 0;
	qOut.w = 1;
}


void TQuaternion::rotationAxis(
	TQuaternion		&qOut,
	const TVector3	&vAxis,
	const float			fAngle)
{
	float	fSin, fCos;

	fSin = (float)sin(fAngle * 0.5f);
	fCos = (float)cos(fAngle * 0.5f);

	/* Create quaternion */
	qOut.x = vAxis.x * fSin;
	qOut.y = vAxis.y * fSin;
	qOut.z = vAxis.z * fSin;
	qOut.w = fCos;

	/* Normalise it */
	TQuaternion::normalize(qOut);
}


void TQuaternion::toAxisAngle(
	const TQuaternion	&qIn,
	TVector3			&vAxis,
	float					&fAngle)
{
	float	fCosAngle, fSinAngle;
	double	temp;

	/* Compute some values */
	fCosAngle	= qIn.w;
	if (fCosAngle < 0) fCosAngle = 0;		// was causing nans
	if (fCosAngle > 1) fCosAngle = 1;		// was causing nans
	temp		= 1.0f - fCosAngle*fCosAngle;
	fAngle		= (float)acos(fCosAngle)*2.0f;
	fSinAngle	= (float)sqrt(temp);

	/* This is to avoid a division by zero */
	if ((float)fabsf(fSinAngle)<0.0005f)
		fSinAngle = 1.0f;

	/* Get axis vector */
	vAxis.x = qIn.x / fSinAngle;
	vAxis.y = qIn.y / fSinAngle;
	vAxis.z = qIn.z / fSinAngle;
}

void TQuaternion::slerp(
	TQuaternion			&qOut,
	const TQuaternion	&qA,
	const TQuaternion	&qB,
	const float				t)
{
	float		fCosine, fAngle, A, B;

	/* Parameter checking */
	if (t<0.0f || t>1.0f)
	{
		printf("TQuaternion::slerp : Bad parameters\n");
		qOut.x = 0;
		qOut.y = 0;
		qOut.z = 0;
		qOut.w = 1;
		return;
	}

	/* Find sine of Angle between Quaternion A and B (dot product between quaternion A and B) */
	fCosine = qA.w*qB.w + qA.x*qB.x + qA.y*qB.y + qA.z*qB.z;

	if (fCosine < 0)
	{
		TQuaternion qi;

		/*
			<http://www.magic-software.com/Documentation/Quaternions.pdf>

			"It is important to note that the quaternions q and -q represent
			the same rotation... while either quaternion will do, the
			interpolation methods require choosing one over the other.

			"Although q1 and -q1 represent the same rotation, the values of
			Slerp(t; q0, q1) and Slerp(t; q0,-q1) are not the same. It is
			customary to choose the sign... on q1 so that... the angle
			between q0 and q1 is acute. This choice avoids extra
			spinning caused by the interpolated rotations."
		*/
		qi.x = -qB.x;
		qi.y = -qB.y;
		qi.z = -qB.z;
		qi.w = -qB.w;

		TQuaternion::slerp(qOut, qA, qi, t);
		return;
	}

	if (fCosine > 1.0f) fCosine = 1.0f;
	fAngle = (float)acos(fCosine);

	/* Avoid a division by zero */
	if (fAngle==0.0f)
	{
		qOut = qA;
		return;
	}

	/* Precompute some values */
	A = (float)(sin((1.0f-t)*fAngle) / sin(fAngle));
	B = (float)(sin(t*fAngle) / sin(fAngle));

	/* Compute resulting quaternion */
	qOut.x = A * qA.x + B * qB.x;
	qOut.y = A * qA.y + B * qB.y;
	qOut.z = A * qA.z + B * qB.z;
	qOut.w = A * qA.w + B * qB.w;

	/* Normalise result */
	TQuaternion::normalize(qOut);
}



void TQuaternion::normalize(TQuaternion &quat)
{
	float	fMagnitude;
	double	temp;

	/* Compute quaternion magnitude */
	temp = quat.w*quat.w + quat.x*quat.x + quat.y*quat.y + quat.z*quat.z;
	fMagnitude = (float)sqrt(temp);

	/* Divide each quaternion component by this magnitude */
	if (fMagnitude!=0.0f)
	{
		fMagnitude = 1.0f / fMagnitude;
		quat.x *= fMagnitude;
		quat.y *= fMagnitude;
		quat.z *= fMagnitude;
		quat.w *= fMagnitude;
	}
}

// Create rotation matrix from submitted quaternion.
// Assuming the quaternion is of the form [X Y Z W]:
//
//						|       2     2									|
//						| 1 - 2Y  - 2Z    2XY - 2ZW      2XZ + 2YW		 0	|
//						|													|
//						|                       2     2					|
//					M = | 2XY + 2ZW       1 - 2X  - 2Z   2YZ - 2XW		 0	|
//						|													|
//						|                                      2     2		|
//						| 2XZ - 2YW       2YZ + 2XW      1 - 2X  - 2Y	 0	|
//						|													|
//						|     0			   0			  0          1  |
//

void TQuaternion::rotation(
	TMatrix				&mOut,
	const TQuaternion	&quat)
{
	const TQuaternion *pQ;

#if defined(BUILD_DX9) || defined(BUILD_D3DM)
	TQuaternion qInv;

	qInv.x = -quat.x;
	qInv.y = -quat.y;
	qInv.z = -quat.z;
	qInv.w = quat.w;

	pQ = &qInv;
#else
	pQ = &quat;
#endif

    /* Fill matrix members */
	mOut.f[0] = 1.0f - 2.0f*pQ->y*pQ->y - 2.0f*pQ->z*pQ->z;
	mOut.f[1] = 2.0f*pQ->x*pQ->y - 2.0f*pQ->z*pQ->w;
	mOut.f[2] = 2.0f*pQ->x*pQ->z + 2.0f*pQ->y*pQ->w;
	mOut.f[3] = 0.0f;

	mOut.f[4] = 2.0f*pQ->x*pQ->y + 2.0f*pQ->z*pQ->w;
	mOut.f[5] = 1.0f - 2.0f*pQ->x*pQ->x - 2.0f*pQ->z*pQ->z;
	mOut.f[6] = 2.0f*pQ->y*pQ->z - 2.0f*pQ->x*pQ->w;
	mOut.f[7] = 0.0f;

	mOut.f[8] = 2.0f*pQ->x*pQ->z - 2*pQ->y*pQ->w;
	mOut.f[9] = 2.0f*pQ->y*pQ->z + 2.0f*pQ->x*pQ->w;
	mOut.f[10] = 1.0f - 2.0f*pQ->x*pQ->x - 2*pQ->y*pQ->y;
	mOut.f[11] = 0.0f;

	mOut.f[12] = 0.0f;
	mOut.f[13] = 0.0f;
	mOut.f[14] = 0.0f;
	mOut.f[15] = 1.0f;
}


void TQuaternion::multiply(
	TQuaternion			&qOut,
	const TQuaternion	&qA,
	const TQuaternion	&qB)
{
	TVector3	CrossProduct;
	TQuaternion	result;

	/* Compute scalar component */
	result.w = (qA.w*qB.w) - (qA.x*qB.x + qA.y*qB.y + qA.z*qB.z);

	/* Compute cross product */
	CrossProduct.x = qA.y*qB.z - qA.z*qB.y;
	CrossProduct.y = qA.z*qB.x - qA.x*qB.z;
	CrossProduct.z = qA.x*qB.y - qA.y*qB.x;

	/* Compute result vector */
	result.x = (qA.w * qB.x) + (qB.w * qA.x) + CrossProduct.x;
	result.y = (qA.w * qB.y) + (qB.w * qA.y) + CrossProduct.y;
	result.z = (qA.w * qB.z) + (qB.w * qA.z) + CrossProduct.z;

	/* Normalize resulting quaternion */
	TQuaternion::normalize(result);
	
	qOut = result;
}

//

void TVector3::lerp(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2, const float s) {
#ifdef SANITY_CHECK
	MatrixVec3Lerp(*(VECTOR3 *)&vOut, *(VECTOR3 *)&v1, *(VECTOR3 *)&v2, s);
#else // !SANITY_CHECK
	vOut.x = v1.x + s * (v2.x - v1.x);
	vOut.y = v1.y + s * (v2.y - v1.y);
	vOut.z = v1.z + s * (v2.z - v1.z);
#endif // !SANITY_CHECK
}
float TVector3::dot(const TVector3 &v1, const TVector3 &v2) {
#ifdef SANITY_CHECK
	return MatrixVec3DotProduct(*(VECTOR3 *)&v1, *(VECTOR3 *)&v2);
#else // !SANITY_CHECK
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
#endif // !SANITY_CHECK
}
void TVector3::cross(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2)
{
#ifdef SANITY_CHECK
	MatrixVec3CrossProduct(*(VECTOR3 *)&vOut, *(VECTOR3 *)&v1, *(VECTOR3 *)&v2);
#else // !SANITY_CHECK
	TVector3 result;

	/* Perform calculation on a dummy VECTOR (result) */
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;

	/* Copy result in pOut */
	vOut = result;
#endif // !SANITY_CHECK
}
void TVector3::add(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2) {
	vOut.x = v1.x + v2.x;
	vOut.y = v1.y + v2.y;
	vOut.z = v1.z + v2.z;
}
void TVector3::sub(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2) {
	vOut.x = v1.x - v2.x;
	vOut.y = v1.y - v2.y;
	vOut.z = v1.z - v2.z;
}
void TVector3::addmul(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2, float s) {
	vOut.x = v1.x + s * v2.x;
	vOut.y = v1.y + s * v2.y;
	vOut.z = v1.z + s * v2.z;
}
void TVector3::intp(TVector3 &vOut, const TVector3 &v1_if_t_is_0, const TVector3 &v2_if_t_is_1, float t) {
	vOut.x = v1_if_t_is_0.x + t * (v2_if_t_is_1.x - v1_if_t_is_0.x);
	vOut.y = v1_if_t_is_0.y + t * (v2_if_t_is_1.y - v1_if_t_is_0.y);
	vOut.z = v1_if_t_is_0.z + t * (v2_if_t_is_1.z - v1_if_t_is_0.z);
}
void TVector3::scalarmul(TVector3 &vOut, const TVector3 &v1, float s) {
	vOut.x = s * v1.x;
	vOut.y = s * v1.y;
	vOut.z = s * v1.z;
}
//    goal.x = cam_up.x + t*(global_up_on_cam_xy_plane.x-cam_up.x);
//    goal.y = cam_up.y + t*(global_up_on_cam_xy_plane.y-cam_up.y);
//    goal.z = cam_up.z + t*(global_up_on_cam_xy_plane.z-cam_up.z);
void TVector3::normalize(TVector3 &vOut, const TVector3 &vIn) {
#ifdef SANITY_CHECK
	MatrixVec3Normalize(*(VECTOR3 *)&vOut, *(VECTOR3 *)&vIn);
#else // !SANITY_CHECK
	float	f;
	double	temp;

	temp = (double)(vIn.x * vIn.x + vIn.y * vIn.y + vIn.z * vIn.z);
	temp = 1.0 / sqrt(temp);
	f = (float)temp;

	vOut.x = vIn.x * f;
	vOut.y = vIn.y * f;
	vOut.z = vIn.z * f;
#endif // !SANITY_CHECK
}
float TVector3::length(const TVector3 &vIn) {
#ifdef SANITY_CHECK
	return MatrixVec3Length(*(VECTOR3 *)&vIn);
#else // !SANITY_CHECK
	double temp;

	temp = (double)(vIn.x * vIn.x + vIn.y * vIn.y + vIn.z * vIn.z);
	return (float) sqrt(temp);
#endif // !SANITY_CHECK
}
void TVector3::multiply(TVector3 &vOut, const TVector3 &vIn, const TMatrix &mIn) {
#ifdef SANITY_CHECK
	MatrixVec3Multiply(*(VECTOR3 *)&vOut, *(VECTOR3 *)&vIn, *(MATRIX *)&mIn);
#else // !SANITY_CHECK
	TVector3 result;

	result.x = mIn.f[ 0] * vIn.x + mIn.f[ 4] * vIn.y + mIn.f[ 8] * vIn.z;
	result.y = mIn.f[ 1] * vIn.x + mIn.f[ 5] * vIn.y + mIn.f[ 9] * vIn.z;
	result.z = mIn.f[ 2] * vIn.x + mIn.f[ 6] * vIn.y + mIn.f[10] * vIn.z;

	vOut = result;
#endif // !SANITY_CHECK
}
void TVector3::multiply4(TVector3 &vOut, const TVector3 &vIn, const TMatrix &mIn) {
//#ifdef SANITY_CHECK
//	MatrixVec3Multiply(*(VECTOR3 *)&vOut, *(VECTOR3 *)&vIn, *(MATRIX *)&mIn);
//#else // !SANITY_CHECK
	TVector3 result;

	result.x = mIn.f[ 0] * vIn.x + mIn.f[ 4] * vIn.y + mIn.f[ 8] * vIn.z + mIn.f[12];
	result.y = mIn.f[ 1] * vIn.x + mIn.f[ 5] * vIn.y + mIn.f[ 9] * vIn.z + mIn.f[13];
	result.z = mIn.f[ 2] * vIn.x + mIn.f[ 6] * vIn.y + mIn.f[10] * vIn.z + mIn.f[14];

	vOut = result;
//#endif // !SANITY_CHECK
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//std::string format(const char *fmt, ...) 
//{ 
//   using std::string;
//   using std::vector;
//
//   string retStr("");
//
//   if (NULL != fmt)
//   {
//      va_list marker = NULL; 
//
//      // initialize variable arguments
//      va_start(marker, fmt); 
//      
//      // Get formatted string length adding one for NULL
//      size_t len = _vscprintf(fmt, marker) + 1;
//               
//      // Create a char vector to hold the formatted string.
//      vector<char> buffer(len, '\0');
//      int nWritten = _vsnprintf_s(&buffer[0], buffer.size(), len, fmt, marker);    
//
//      if (nWritten > 0)
//      {
//         retStr = &buffer[0];
//      }
//            
//      // Reset variable arguments
//      va_end(marker); 
//   }
//
//   return retStr; 
//}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//missing string printf
//this is safe and convenient but not exactly efficient
std::string format(const char* fmt, ...){
    int size = 100;
	std::string str;
    va_list vl;
	while (1) {
		str.resize(size);
		va_start(vl, fmt);
		int nsize = vsnprintf((char *)str.c_str(), size, fmt, vl);
	    va_end(vl);
		if ((nsize > -1) && (nsize < size)) {
			str.resize(nsize);
			return str;
		}
		if (nsize > -1) {
			size = nsize + 1;
		} else {
			nsize *= 2;
		}
	}
    return str;
}
//So you can use it like:
//std::string mystr = format("%s %d %10.5f", "omg", 1, 10.5);

std::string n2str(int t) {
	if (t >= 1000000) {
		return format("%d,%03d,%03d", t/1000000, (t/1000)%1000, t%1000);
	} else if (t >= 10000) {
		return format("%d,%03d", t/1000, t%1000);
	} else {
		return format("%d", t);
	}
}
