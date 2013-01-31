/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2012, Limin Fu
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

#ifndef __DAO_TRIANGULATOR_H__
#define __DAO_TRIANGULATOR_H__

#include "dao_common.h"

typedef struct DaoxVertexData  DaoxVertexData;

typedef struct DaoxTriangulator  DaoxTriangulator;


enum DaoxContourDirections
{
	DAOX_CLOCKWISE  = 1,
	DAOX_COUNTER_CW ,  /* counter-clockwise; */
};



struct DaoxVertexData
{
	uint_t  index;    /* index in the original point array; */
	uint_t  sorting;  /* index in the sorted vertex array; */

	ushort_t  contour;    /* contour index; */
	uchar_t   direction;  /* contour direction; */
	uchar_t   done;

	DaoxVertexData  *prev;  /* previous vertex; */
	DaoxVertexData  *next;  /* next vertex; */
};


DAO_DLL DaoxVertexData* DaoxVertexData_New( daoint index );
DAO_DLL void DaoxVertexData_Delete( DaoxVertexData *self );



struct DaoxTriangulator
{
	/* All the points in the polygon sorted by their x coordinates: */
	DaoxPlainArray  *points;

	DaoxVertexData  *start;

	/*
	// All the vertices for both the initial and intermediate polygons.
	// The original vertices are sorted by the x coordinates,
	// but the duplicated vertices for intermediate polygons are not.
	*/
	DArray  *vertices;  /* DArray<DaoxVertexData*>; */

	/* Sorted list of vertices for processing: */
	DArray  *worklist;  /* DArray<DaoxVertexData*>; */

	DaoxPlainArray  *triangles;  /* list of triple integers; */

	DaoxVertexData  *caches;  /* reusable vertices; */
};


DAO_DLL DaoxTriangulator* DaoxTriangulator_New();
DAO_DLL void DaoxTriangulator_Delete( DaoxTriangulator *self );
DAO_DLL void DaoxTriangulator_Reset( DaoxTriangulator *self );

DAO_DLL void DaoxTriangulator_PushPoint( DaoxTriangulator *self, float x, float y );
DAO_DLL int  DaoxTriangulator_CloseContour( DaoxTriangulator *self );

DAO_DLL void DaoxTriangulator_Triangulate( DaoxTriangulator *self );

#endif
