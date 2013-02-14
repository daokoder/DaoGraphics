/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2012,2013, Limin Fu
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __DAO_COMMON_H__
#define __DAO_COMMON_H__


#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "dao.h"
#include "daoStdtype.h"
#include "daoValue.h"


#define EPSILON 1E-16


typedef struct DaoxColor           DaoxColor;

typedef struct DaoxMaterial  DaoxMaterial;


struct DaoxColor
{
	float  red;
	float  green;
	float  blue;
	float  alpha;
};

extern const DaoxColor daox_black_color;
extern const DaoxColor daox_white_color;
extern const DaoxColor daox_red_color;
extern const DaoxColor daox_green_color;
extern const DaoxColor daox_blue_color;
extern const DaoxColor daox_gray_color;






typedef struct DaoxVector2D  DaoxVector2D;  /* 2D float vector type (for 2D points); */
typedef struct DaoxVector3D  DaoxVector3D;  /* 3D float vector type (for 2D points); */
typedef struct DaoxMatrix2D  DaoxMatrix2D;  /* 2D float matrix type; */
typedef struct DaoxMatrix3D  DaoxMatrix3D;  /* 3D float matrix type (for 2D transforms); */
typedef struct DaoxMatrix4D  DaoxMatrix4D;  /* 4D float matrix type (for 3D transforms); */


typedef struct DaoxVectorD2    DaoxVectorD2;    /* 2D double vector type; */
typedef struct DaoxVectorD3    DaoxVectorD3;    /* 3D double vector type; */
typedef struct DaoxVectorD4    DaoxVectorD4;    /* 4D double vector type; */
typedef union  DaoxMatrixD3X3  DaoxMatrixD3X3;  /* 3x3 double matrix type; */
typedef union  DaoxMatrixD4X4  DaoxMatrixD4X4;  /* 4x4 double matrix type; */


typedef struct DaoxVertex    DaoxVertex;
typedef struct DaoxTriangle  DaoxTriangle;

typedef struct DaoxLine      DaoxLine;

typedef struct DaoxOBBox2D  DaoxOBBox2D;
typedef struct DaoxOBBox3D  DaoxOBBox3D;

typedef struct DaoxAABBox2D  DaoxAABBox2D;

typedef struct DaoxPlainArray   DaoxPlainArray;



struct DaoxVector2D
{
	float  x;
	float  y;
};
DaoxVector2D DaoxVector2D_XY( float x, float y );
DaoxVector2D DaoxVector2D_Add( DaoxVector2D *self, DaoxVector2D *other );
DaoxVector2D DaoxVector2D_Sub( DaoxVector2D *self, DaoxVector2D *other );
DaoxVector2D DaoxVector2D_Scale( DaoxVector2D *self, double scale );
DaoxVector2D DaoxVector2D_Normalize( DaoxVector2D *self );
DaoxVector2D DaoxVector2D_Interpolate( DaoxVector2D A, DaoxVector2D B, float t );
//double DaoxVector2D_Dist2( DaoxVector2D *self, DaoxVector2D *other );
double DaoxVector2D_Dot( DaoxVector2D *self, DaoxVector2D *other );
double DaoxVector2D_Norm2( DaoxVector2D *self );
double DaoxVector2D_Dist( DaoxVector2D start, DaoxVector2D end );
double DaoxVector2D_Dist2( DaoxVector2D start, DaoxVector2D end );
void DaoxVector2D_Print( DaoxVector2D *self );

double DaoxTriangle_Area( DaoxVector2D A, DaoxVector2D B, DaoxVector2D C );
double DaoxTriangle_AngleCosine( DaoxVector2D C, DaoxVector2D A, DaoxVector2D B );
int DaoxTriangle_Contain( DaoxVector2D C, DaoxVector2D A, DaoxVector2D B, DaoxVector2D P );
int DaoxLine_Intersect( DaoxVector2D A, DaoxVector2D B, DaoxVector2D C, DaoxVector2D D, float *S, float *T );




struct DaoxLine
{
	DaoxVector2D  start;
	DaoxVector2D  end;
};




struct DaoxMatrix2D
{
	float  A;
	float  B;
	float  C;
	float  D;
};



struct DaoxVector3D
{
	float  x;
	float  y;
	float  z;
};

DaoxVector3D  DaoxVector3D_XYZ( float x, float y, float z );
DaoxVector3D  DaoxVector3D_Add( DaoxVector3D *self, DaoxVector3D *other );
DaoxVector3D  DaoxVector3D_Sub( DaoxVector3D *self, DaoxVector3D *other );
DaoxVector3D  DaoxVector3D_Mul( DaoxVector3D *self, DaoxVector3D *other );
DaoxVector3D  DaoxVector3D_Scale( DaoxVector3D *self, double scale );
DaoxVector3D  DaoxVector3D_Cross( DaoxVector3D *self, DaoxVector3D *other );
DaoxVector3D  DaoxVector3D_Normalize( DaoxVector3D *self );
double DaoxVector3D_Norm2( DaoxVector3D *self );
double DaoxVector3D_Dot( DaoxVector3D *self, DaoxVector3D *other );
double DaoxVector3D_Angle( DaoxVector3D *self, DaoxVector3D *other );
double DaoxVector3D_Dist2( DaoxVector3D *self, DaoxVector3D *other );
double DaoxVector3D_Difference( DaoxVector3D *self, DaoxVector3D *other );
void DaoxVector3D_Print( DaoxVector3D *self );

DaoxVector3D DaoxTriangle_Normal( DaoxVector3D *A, DaoxVector3D *B, DaoxVector3D *C );




struct DaoxMatrix3D
{
	float  A11, A12, B1;
	float  A21, A22, B2;
};
DaoxMatrix3D  DaoxMatrix3D_Identity();
DaoxMatrix3D  DaoxMatrix3D_PointRotation( DaoxVector2D point, float alpha );
DaoxMatrix3D  DaoxMatrix3D_MulMatrix( DaoxMatrix3D *self, DaoxMatrix3D *other );
DaoxVector2D  DaoxMatrix3D_MulVector( DaoxMatrix3D *self, DaoxVector2D *vector, float w );
DaoxVector2D  DaoxMatrix3D_RotateVector( DaoxVector2D vector, float alpha );

void DaoxMatrix3D_Set( DaoxMatrix3D *self, float *mat, int n );
void DaoxMatrix3D_RotateXAxisTo( DaoxMatrix3D *self, float x, float y );
void DaoxMatrix3D_RotateYAxisTo( DaoxMatrix3D *self, float x, float y );
void DaoxMatrix3D_SetScale( DaoxMatrix3D *self, float x, float y );
void DaoxMatrix3D_Multiply( DaoxMatrix3D *self, DaoxMatrix3D other );
DaoxVector2D DaoxMatrix3D_Transform( DaoxMatrix3D *self, DaoxVector2D point );
DaoxVector2D DaoxMatrix3D_TransformXY( DaoxMatrix3D *self, float x, float y );
DaoxMatrix3D DaoxMatrix3D_Inverse( DaoxMatrix3D *self );





struct DaoxMatrix4D
{
	float  A11, A12, A13, B1;
	float  A21, A22, A23, B2;
	float  A31, A32, A33, B3;
};

DaoxMatrix4D  DaoxMatrix4D_Identity();
DaoxMatrix4D  DaoxMatrix4D_InitRowMajor( float M[12] );
DaoxMatrix4D  DaoxMatrix4D_InitColumnMajor( float M[16] );
DaoxMatrix4D  DaoxMatrix4D_InitRows( float R0[4], float R1[4], float R2[4] );
DaoxMatrix4D  DaoxMatrix4D_InitColumns( float C0[3], float C1[3], float C2[3], float C3[3] );
DaoxMatrix4D  DaoxMatrix4D_Translation( float x, float y, float z );
DaoxMatrix4D  DaoxMatrix4D_AxisRotation( DaoxVector3D axis, float alpha );
DaoxMatrix4D  DaoxMatrix4D_EulerRotation( float alpha, float beta, float gamma );

DaoxVector3D  DaoxMatrix4D_MulVector( DaoxMatrix4D *self, DaoxVector3D *vector, float w );
DaoxMatrix4D  DaoxMatrix4D_MulMatrix( DaoxMatrix4D *self, DaoxMatrix4D *other );
DaoxMatrix4D  DaoxMatrix4D_Inverse( DaoxMatrix4D *self );
DaoxMatrix4D  DaoxMatrix4D_RotationOnly( DaoxMatrix4D *self );
DaoxMatrix4D  DaoxMatrix4D_TranslationOnly( DaoxMatrix4D *self );





struct DaoxVectorD2
{
	double  x, y;
};


struct DaoxVectorD3
{
	double  x, y, z;
};


struct DaoxVectorD4
{
	double  x, y, z, w;
};


union DaoxMatrixD3X3
{
	double  M[3][3];
	struct  {  DaoxVectorD3  V[3];  } V;
	struct  {
		double  A11, A12, A13;
		double  A21, A22, A23;
		double  A31, A32, A33;
	} A;
};

DaoxMatrixD3X3 DaoxMatrixD3X3_InitRows( DaoxVectorD3 V1, DaoxVectorD3 V2, DaoxVectorD3 V3 );
DaoxMatrixD3X3 DaoxMatrixD3X3_InitColumns( DaoxVectorD3 V1, DaoxVectorD3 V2, DaoxVectorD3 V3 );
double DaoxMatrixD3X3_Determinant( DaoxMatrixD3X3 *self );


union DaoxMatrixD4X4
{
	double  M[4][4];
	struct  {  DaoxVectorD4  V[4];  } V;
	struct  {
		double  A11, A12, A13, A14;
		double  A21, A22, A23, A24;
		double  A31, A32, A33, A34;
		double  A41, A42, A43, A44;
	} A;
};

DaoxMatrixD4X4 DaoxMatrixD4X4_MulMatrix( DaoxMatrixD4X4 *self, DaoxMatrixD4X4 *other );




struct DaoxVertex
{
	DaoxVector3D  point;
	DaoxVector3D  norm;
	DaoxVector2D  texUV;
};



struct DaoxTriangle
{
	uint_t  index[3];
};





/*
// 2D Oriented Bounding Box:
*/
struct DaoxOBBox2D
{
	DaoxVector2D  O;
	DaoxVector2D  X;
	DaoxVector2D  Y;
};
void DaoxOBBox2D_ResetBox( DaoxOBBox2D *self, DaoxVector2D points[], int count );
int  DaoxOBBox2D_Intersect( DaoxOBBox2D *self, DaoxOBBox2D *other );
int  DaoxOBBox2D_Intersect2( DaoxOBBox2D *self, DaoxOBBox2D *other, double tolerance );
DaoxOBBox2D DaoxOBBox2D_Transform( DaoxOBBox2D *self, DaoxMatrix3D *transfrom );
DaoxOBBox2D DaoxOBBox2D_CopyWithMargin( DaoxOBBox2D *self, double margin );




/*
// 3D Oriented Bounding Box:
*/
struct DaoxOBBox3D
{
	DaoxVector3D  O;
	DaoxVector3D  X;
	DaoxVector3D  Y;
	DaoxVector3D  Z;

	DaoxVector3D  C;
	float         R;
};
int DaoxOBBox3D_Contain( DaoxOBBox3D *self, DaoxVector3D point );





struct DaoxAABBox2D
{
	float  left;
	float  right;
	float  bottom;
	float  top;
};
void DaoxAABBox2D_AddMargin( DaoxAABBox2D *self, float margin );
void DaoxAABBox2D_Init( DaoxAABBox2D *self, DaoxVector2D point );
void DaoxAABBox2D_InitXY( DaoxAABBox2D *self, float x, float y );
void DaoxAABBox2D_Update( DaoxAABBox2D *self, DaoxVector2D point );
void DaoxAABBox2D_UpdateXY( DaoxAABBox2D *self, float x, float y );
DaoxAABBox2D DaoxAABBox2D_Transform( DaoxAABBox2D *self, DaoxMatrix3D *t );




/*
// Array for plain data structures:
*/
struct DaoxPlainArray
{
	union {
		void   *data;
		int    *ints;
		float  *floats;

		DaoxVector2D  *vectors2d;
		DaoxVector3D  *vectors3d;

		DaoxVertex    *vertices;
		DaoxTriangle  *triangles;

		DaoxColor     *colors;

		struct DaoxPathSegment  *segments;
	} pod;

	uint_t  size;
	uint_t  capacity;
	uint_t  stride;
};

DaoxPlainArray* DaoxPlainArray_New( int stride );
void DaoxPlainArray_Delete( DaoxPlainArray *self );
void DaoxPlainArray_Clear( DaoxPlainArray *self );
void DaoxPlainArray_Resize( DaoxPlainArray *self, int size );
void DaoxPlainArray_Reserve( DaoxPlainArray *self, int size );
void DaoxPlainArray_ResetSize( DaoxPlainArray *self, int size );
void* DaoxPlainArray_Push( DaoxPlainArray *self );
void* DaoxPlainArray_Get( DaoxPlainArray *self, int i );

void DaoxPlainArray_PushInt( DaoxPlainArray *self, int value );
void DaoxPlainArray_PushFloat( DaoxPlainArray *self, float value );
DaoxVector2D* DaoxPlainArray_PushVector2D( DaoxPlainArray *self );
DaoxVector3D* DaoxPlainArray_PushVector3D( DaoxPlainArray *self );
DaoxVertex*   DaoxPlainArray_PushVertex( DaoxPlainArray *self );
DaoxTriangle* DaoxPlainArray_PushTriangle( DaoxPlainArray *self );

DaoxVector2D* DaoxPlainArray_PushVectorXY( DaoxPlainArray *self, float x, float y );
DaoxVector3D* DaoxPlainArray_PushVectorXYZ( DaoxPlainArray *self, float x, float y, float z );
DaoxTriangle* DaoxPlainArray_PushTriangleIJK( DaoxPlainArray *self, int i, int j, int k );



extern DaoVmSpace *__daoVmSpace;


#endif
