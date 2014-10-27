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


/*
// TODO:
// 1. Procedural terrain generation:
// -- Free generation;
// -- Terrain type guided generation;
// 
//
// 2. Terrain texturing:
// -- Classify terrain cells based on its slope and smoothness;
// -- Support multiple textures for terrain;
// -- Support texture blending based on terrain types;
//    DaoxVertex::tan can be used to store terrain types and blending factors;
*/


typedef struct DaoxTerrainPoint  DaoxTerrainPoint;
typedef struct DaoxTerrainCell   DaoxTerrainCell;
typedef struct DaoxTerrainBlock  DaoxTerrainBlock;

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
// Indexing of vertex corners and sub cells:
//
//    3---------------2
//    |       |       |
//    |   1   |   0   |
//    |       |       |
//    Y-------+-------|
//    |       |   |   |
//    |   2   |---3---|
//    |       |   |   |
//    O-------X-------1
*/
struct DaoxTerrainCell
{
	uchar_t  visible;
	uchar_t  smooth;
	float    minHeight;
	float    maxHeight;

	DaoxTerrainPoint  *center;
	DaoxTerrainPoint  *corners[4];
	DaoxTerrainCell   *subcells[4];
};

struct DaoxTerrainBlock
{
	DaoxTerrainCell   *cellTree;
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

	DaoArray      *heightmap;
	DaoxMaterial  *material;

	DList   *pointList;
	DList   *pointCache;
	DList   *cellCache;
};
extern DaoType *daox_type_terrain;

DaoxTerrain* DaoxTerrain_New();
void DaoxTerrain_Delete( DaoxTerrain *self );

void DaoxTerrain_SetSize( DaoxTerrain *self, float width, float length );
void DaoxTerrain_SetHeightmap( DaoxTerrain *self, DaoArray *heightmap );
void DaoxTerrain_SetMaterial( DaoxTerrain *self, DaoxMaterial *material );
void DaoxTerrain_Refine( DaoxTerrain *self, DaoxTerrainCell *cell );
void DaoxTerrain_Rebuild( DaoxTerrain *self );
void DaoxTerrain_UpdateView( DaoxTerrain *self, DaoxViewFrustum *frustum );





typedef struct DaoxHexPoint    DaoxHexPoint;
typedef struct DaoxHexBorder   DaoxHexBorder;
typedef struct DaoxHexTriangle DaoxHexTriangle;
typedef struct DaoxHexUnit     DaoxHexUnit;
typedef struct DaoxHexTerrain  DaoxHexTerrain;

typedef struct DaoxTerrainParams    DaoxTerrainParams;
typedef struct DaoxTerrainGenerator DaoxTerrainGenerator;


struct DaoxTerrainParams
{
	float  faultScale;
	float  amplitude;
	float  resolution;
};


struct DaoxHexPoint
{
	DaoxVector3D  pos;
	DaoxVector3D  norm;
	DaoxVector3D  tan;
	DaoxVector2D  tex;
	int           id;
};

struct DaoxHexBorder
{
	DaoxHexPoint   *start;
	DaoxHexPoint   *end;
	DaoxHexBorder  *left;
	DaoxHexBorder  *right;
};

struct DaoxHexTriangle
{
	DaoxHexPoint     *points[3];
	DaoxHexBorder    *borders[3];
	DaoxHexTriangle  *splits[4];
};

struct DaoxHexUnit
{
	int               type;
	DaoxHexPoint     *center;
	DaoxHexTriangle  *splits[6];
	DaoxHexBorder    *borders[6];
	DaoxHexBorder    *spokes[6];
	DaoxHexUnit      *neighbors[6];
	DaoxHexUnit      *next;
	DaoxMeshUnit     *mesh;
	DaoxTerrainParams  *params;
};

struct DaoxHexTerrain
{
	DaoxSceneNode  base;
	DaoxMesh      *mesh;

	DaoArray     *heightmap;
	DaoxHexTerrain  *model;

	DaoxHexUnit  *first;
	DaoxHexUnit  *last;

	DList     *points;
	DList     *borders;
	DList     *tiles;

	int    circles;
	int    changes;
	float  radius;
	float  height;
	float  depth;
	float  textureScale;

	DList  *buffer;
};
extern DaoType *daox_type_hexterrain;

DaoxHexTerrain* DaoxHexTerrain_New();
void DaoxHexTerrain_Delete( DaoxHexTerrain *self );

void DaoxHexTerrain_SetSize( DaoxHexTerrain *self, int circles, float radius );
void DaoxHexTerrain_SetHeightmap( DaoxHexTerrain *self, DaoArray *heightmap );
void DaoxHexTerrain_Rebuild( DaoxHexTerrain *self );
void DaoxHexTerrain_Generate( DaoxHexTerrain *self, int seed );

DaoxHexUnit* DaoxHexTerrain_GetTile( DaoxHexTerrain *self, int side, int radius, int offset );




struct DaoxTerrainGenerator
{
	DAO_CSTRUCT_COMMON;

	DaoxHexTerrain    *terrain;

	DaoRandGenerator  *randGenerator;

	DaoxTerrainParams  params;
	DaoxVector2D       faultPoint;
	DaoxVector2D       faultNorm;
	float              faultDist;
	float              diameter;
};
extern DaoType *daox_type_terrain_generator;

DaoxTerrainGenerator* DaoxTerrainGenerator_New();
void DaoxTerrainGenerator_Delete( DaoxTerrainGenerator *self );

void DaoxTerrainGenerator_Update( DaoxTerrainGenerator *self, int iterations );
void DaoxTerrainGenerator_Generate( DaoxTerrainGenerator *self, int iterations, int seed );

#endif
