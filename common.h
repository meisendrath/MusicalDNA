//
//  common.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _SCOMMON_H
#define _SCOMMON_H

#include "config.h"

#define kPI 3.1415926535f

class TMatrix;		// fwd

class TVector3 {
public:
							TVector3() { x = y = z = 0; }
							TVector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
public:
	float x, y, z;
public:
	static void				multiply(TVector3 &vOut, const TVector3 &vIn, const TMatrix &mIn);		// promotes to TVector4 with [x y z 0]
	static void				multiply4(TVector3 &vOut, const TVector3 &vIn, const TMatrix &mIn);		// promotes to TVector4 with [x y z 1]
	static void				lerp(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2, const float s);
	static float			dot(const TVector3 &v1, const TVector3 &v2);
	static void				cross(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2);
	static void				normalize(TVector3 &vOut, const TVector3 &vIn);
	static float			length(const TVector3 &vIn);
	static void				add(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2);
	static void				sub(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2);
	static void				addmul(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2, float mult_v2);
	static void				intp(TVector3 &vOut, const TVector3 &v1_if_t_is_0, const TVector3 &v2_if_t_is_1, float t);
    static void				scalarmul(TVector3 &vOut, const TVector3 &v1, float s);
};

//

class TVector4 {
public:
							TVector4() { x = y = z = w = 0; }
							TVector4(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
							TVector4(TVector3 &v, float _w) { x = v.x; y = v.y; z = v.z; w = _w; }
public:
	float x, y, z, w;
public:
//	static void				multiply(TVector3 &vOut, const TVector3 &vIn, const TMatrix &mIn);		// promotes to TVector4 with [x y z 0]
//	static void				multiply4(TVector3 &vOut, const TVector3 &vIn, const TMatrix &mIn);		// promotes to TVector4 with [x y z 1]
//	static void				lerp(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2, const float s);
//	static float			dot(const TVector3 &v1, const TVector3 &v2);
//	static void				cross(TVector3 &vOut, const TVector3 &v1, const TVector3 &v2);
//	static void				normalize(TVector3 &vOut, const TVector3 &vIn);
//	static float			length(const TVector3 &vIn);
};

//

class TQuaternion {
public:
							TQuaternion() { x = y = z = w = 0; }
							TQuaternion(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
							TQuaternion(TVector3 &v, float _w) { x = v.x; y = v.y; z = v.z; w = _w; }
public:
	float x, y, z, w;
public:
	static void				identity(TQuaternion &qOut);
	static void				rotationAxis(TQuaternion &qOut, const TVector3 &vAxis, const float fAngle);
	static void				toAxisAngle(const TQuaternion &qIn, TVector3 &vAxis, float &fAngle);
	static void				slerp(TQuaternion &qOut, const TQuaternion &qA, const TQuaternion &qB, const float t);
	static void				normalize(TQuaternion &quat);
	static void				rotation(TMatrix &mOut, const TQuaternion &quat);
	static void				multiply(TQuaternion &qOut, const TQuaternion &qA, const TQuaternion &qB);
};

//

class TMatrix {
public:
    float *operator[] (const int row) { return &f[row<<2]; }
	const TMatrix &operator= (const TMatrix &rhs);
	float f[16];
public:
	static void				multiply(TMatrix &mOut, const TMatrix &mA, const TMatrix &mB);
	static void				identity(TMatrix &mOut);
	static void				translate(TMatrix &mOut, const float fX, const float fY, const float fZ);
	static void				scaling(TMatrix &mOut, const float fX, const float fY, const float fZ);
	static void				rotationAxis(TMatrix &mOut, const float fAngle, const float fX, const float fY, const float fZ);
	static void				rotationX(TMatrix &mOut, const float fAngle);
	static void				rotationY(TMatrix &mOut, const float fAngle);
	static void				rotationZ(TMatrix &mOut, const float fAngle);
	static void				transpose(TMatrix &mOut, const TMatrix &mIn);
	static void				inverse(TMatrix &mOut, const TMatrix &mIn);
	static void				inverseEx(TMatrix &mOut, const TMatrix &mIn);
	static void				lookAtLH(TMatrix &mOut, const TVector3 &vEye, const TVector3 &vAt, const TVector3 &vUp);
	static void				lookAtRH(TMatrix &mOut, const TVector3 &vEye, const TVector3 &vAt, const TVector3 &vUp);
	static void				perspectiveFovLH(TMatrix &mOut, const float fFOVy, const float fAspect, const float fNear, const float fFar, const bool bRotate);
	static void				perspectiveFovRH(TMatrix &mOut, const float fFOVy, const float fAspect, const float fNear, const float fFar, const bool bRotate);
//	static void				orthoLH(TMatrix &mOut, const float left, const float right, const float bottom, const float top, const float zn, const float zf, const bool bRotate);
	static void				orthoRH(TMatrix &mOut, const float left, const float right, const float bottom, const float top, const float zn, const float zf, const bool bRotate);
	static void				frustumRH(TMatrix &mOut, const float left, const float right, const float bottom, const float top, const float zn, const float zf, const bool bRotate);
	static void				linearEqSolve(float * const pRes, float ** const pSrc, const int nCnt);			// 2D array of floats. 4 Eq linear problem is 5x4 matrix, constants in first column.
    void					dump();
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//#include <string>
//#include <vector>
//std::string format(const char *fmt, ...);
#include <string>
//#include <cstdarg>

//So you can use it like:
//std::string mystr = format("%s %d %10.5f", "omg", 1, 10.5);

//int printf(const char * __restrict format, ...);
std::string n2str(int t);
std::string format(const char* fmt, ...);

#ifdef ANDROID_NDK
#define printf(fmt, args...) fprintf(stderr, fmt, ##args)
#endif // ANDROID_NDK

#ifdef __APPLE__
extern void logprintf(const char* fmt, ...);
#define printf(fmt, args...) logprintf(fmt, ##args)
#endif // __APPLE__

#endif // !_SCOMMON_H


