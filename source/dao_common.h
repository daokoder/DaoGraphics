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


#define DAO_LIST_ITEM_TYPES \
	struct DaoxMaterial     **pMaterial; \
	struct DaoxMeshChunk    **pMeshChunk; \
	struct DaoxMeshUnit     **pMeshUnit; \
	struct DaoxModel        **pModel; \
	struct DaoxLight        **pLight; \
	struct DaoxCanvas       **pCanvas; \
	struct DaoxCanvasState  **pCanvasState; \
	struct DaoxCanvasNode   **pCanvasNode; \
	struct DaoxSceneNode    **pSceneNode;

#define DAO_ARRAY_ITEM_TYPES \
	struct DaoxVector2D     *vectors2d; \
	struct DaoxVector3D     *vectors3d; \
	struct DaoxVertex       *vertices;  \
	struct DaoxTriangle     *triangles; \
	struct DaoxColor        *colors;    \
	struct DaoxPathSegment  *segments;


#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dao.h"
#include "daoStdtype.h"
#include "daoValue.h"
#include "daoGC.h"


#define EPSILON 1E-16

extern float daox_graphics_device_width;
extern float daox_graphics_device_height;


typedef struct DaoxColor     DaoxColor;
typedef struct DaoxMaterial  DaoxMaterial;

typedef struct DaoxVector2D  DaoxVector2D;  /* 2D float vector type (for 2D points); */
typedef struct DaoxVector3D  DaoxVector3D;  /* 3D float vector type (for 2D points); */

typedef struct DaoxMatrix2D  DaoxMatrix2D;  /* 2D float matrix type; */
typedef struct DaoxMatrix3D  DaoxMatrix3D;  /* 3D float matrix type (for 2D transforms); */
typedef struct DaoxMatrix4D  DaoxMatrix4D;  /* 4D float matrix type (for 3D transforms); */

typedef struct DaoxVertex    DaoxVertex;
typedef struct DaoxTriangle  DaoxTriangle;

typedef struct DaoxOBBox2D   DaoxOBBox2D;
typedef struct DaoxOBBox3D   DaoxOBBox3D;

typedef struct DaoxAABBox2D  DaoxAABBox2D;



extern const DaoxColor daox_black_color;
extern const DaoxColor daox_white_color;
extern const DaoxColor daox_red_color;
extern const DaoxColor daox_green_color;
extern const DaoxColor daox_blue_color;
extern const DaoxColor daox_gray_color;


struct DaoxColor
{
	float  red;
	float  green;
	float  blue;
	float  alpha;
};

DaoxColor DaoxColor_Darker( DaoxColor *self, float factor );
DaoxColor DaoxColor_Lighter( DaoxColor *self, float factor );



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
DaoxVector3D  DaoxVector3D_Interpolate( DaoxVector3D A, DaoxVector3D B, float t );
DaoxVector3D  DaoxVector3D_ProjectToPlane( DaoxVector3D *self, DaoxVector3D *planeNorm );

double DaoxVector3D_Norm2( DaoxVector3D *self );
double DaoxVector3D_Dot( DaoxVector3D *self, DaoxVector3D *other );
double DaoxVector3D_Angle( DaoxVector3D *self, DaoxVector3D *other );
double DaoxVector3D_Dist2( DaoxVector3D *self, DaoxVector3D *other );
double DaoxVector3D_Dist( DaoxVector3D *self, DaoxVector3D *other );
double DaoxVector3D_Difference( DaoxVector3D *self, DaoxVector3D *other );
void DaoxVector3D_Print( DaoxVector3D *self );

DaoxVector3D DaoxTriangle_Normal( DaoxVector3D *A, DaoxVector3D *B, DaoxVector3D *C );

DaoxVector3D DaoxPlaneLineIntersect( DaoxVector3D point, DaoxVector3D norm, DaoxVector3D P1, DaoxVector3D P2 );




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

void DaoxMatrix4D_Print( DaoxMatrix4D *self );





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

/*
// Return  1, if "self" contains "other";
// Return  0, if "self" intersects "other";
// Return -1, if "self" does not intersect "other";
*/
int  DaoxOBBox2D_Intersect( DaoxOBBox2D *self, DaoxOBBox2D *other );

int  DaoxOBBox2D_Intersect2( DaoxOBBox2D *self, DaoxOBBox2D *other, double tolerance );
DaoxOBBox2D DaoxOBBox2D_InitRect( float left, float right, float top, float bottom );
DaoxOBBox2D DaoxOBBox2D_Scale( DaoxOBBox2D *self, float scale );
DaoxOBBox2D DaoxOBBox2D_Transform( DaoxOBBox2D *self, DaoxMatrix3D *transfrom );
DaoxOBBox2D DaoxOBBox2D_CopyWithMargin( DaoxOBBox2D *self, double margin );
double DaoxOBBox2D_Area( DaoxOBBox2D *self );




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

DaoxOBBox3D DaoxOBBox3D_Scale( DaoxOBBox3D *self, float scale );
DaoxOBBox3D DaoxOBBox3D_Transform( DaoxOBBox3D *self, DaoxMatrix4D *transfrom );
DaoxOBBox3D DaoxOBBox3D_ToAABox( DaoxOBBox3D *self );

DaoxVector3D DaoxOBBox3D_GetDiagonalVertex( DaoxOBBox3D *self );

int DaoxOBBox3D_Contain( DaoxOBBox3D *self, DaoxVector3D point );
void DaoxOBBox3D_ComputeBoundingBox( DaoxOBBox3D *self, DaoxVector3D points[], int count );

void DaoxOBBox3D_Print( DaoxOBBox3D *self );





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




DaoxVector2D* DArray_PushVector2D( DArray *self, DaoxVector2D *vector2d );
DaoxVector3D* DArray_PushVector3D( DArray *self, DaoxVector3D *vector3d );
DaoxVertex*   DArray_PushVertex( DArray *self, DaoxVertex *vertex );
DaoxTriangle* DArray_PushTriangle( DArray *self, DaoxTriangle *triangle );

DaoxVector2D* DArray_PushVectorXY( DArray *self, float x, float y );
DaoxVector3D* DArray_PushVectorXYZ( DArray *self, float x, float y, float z );
DaoxTriangle* DArray_PushTriangleIJK( DArray *self, int i, int j, int k );



extern DaoVmSpace *__daoVmSpace;


#endif
