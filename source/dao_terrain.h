/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2014, Limin Fu
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


#ifndef __DAO_TERRAIN_H__
#define __DAO_TERRAIN_H__


#include "dao_scene.h"



typedef struct DaoxTerrainPoint DaoxTerrainPoint;
typedef struct DaoxTerrainPatch DaoxTerrainPatch;
typedef struct DaoxTerrainBlock DaoxTerrainBlock;

struct DaoxTerrainPoint
{
	uint_t             index;
	ushort_t           level;
	ushort_t           count;

	DaoxVector3D       pos;
	DaoxVector3D       norm;

	DaoxTerrainPoint  *east;
	DaoxTerrainPoint  *west;
	DaoxTerrainPoint  *south;
	DaoxTerrainPoint  *north;
	DaoxTerrainPoint  *bottom;
};


/*
// Indexing of vertex points and sub patches:
//    1-----Y-----0
//    |     |     |
//    |     |     |
//    ------O-----X
//    |     |__|__|
//    |     |  |  |
//    2-----------3
*/
struct DaoxTerrainPatch
{
	uchar_t  visible;
	uchar_t  smooth;
	float    minHeight;
	float    maxHeight;

	DaoxTerrainPoint  *center;
	DaoxTerrainPoint  *points[4];
	DaoxTerrainPatch  *subs[4];
};

struct DaoxTerrainBlock
{
	DaoxTerrainPatch  *patchTree;
	DaoxTerrainPoint  *baseCenter;

	DaoxTerrainBlock  *east;
	DaoxTerrainBlock  *west;
	DaoxTerrainBlock  *south;
	DaoxTerrainBlock  *north;
};

struct DaoxTerrain
{
	DaoxSceneNode  base;

	float  width;   /* x-axis; */
	float  length;  /* y-axis; */
	float  height;  /* z-axis; */
	float  depth;

	DList   *blocks;
	DList   *vertices;
	DArray  *triangles;

	DaoxTerrainPatch  *patchTree;
	DaoxTerrainPoint  *baseCenter;

	DaoxImage     *heightmap;
	DaoxMaterial  *material;

	DList   *pointList;
	DList   *pointCache;
	DList   *patchCache;
};
extern DaoType *daox_type_terrain;

DaoxTerrain* DaoxTerrain_New();
void DaoxTerrain_Delete( DaoxTerrain *self );

void DaoxTerrain_SetSize( DaoxTerrain *self, float width, float length, float height );
void DaoxTerrain_SetHeightmap( DaoxTerrain *self, DaoxImage *heightmap );
void DaoxTerrain_SetMaterial( DaoxTerrain *self, DaoxMaterial *material );
void DaoxTerrain_Refine( DaoxTerrain *self, DaoxTerrainPatch *patch );
void DaoxTerrain_Rebuild( DaoxTerrain *self );
void DaoxTerrain_UpdateView( DaoxTerrain *self, DaoxViewFrustum *frustum );



#endif
