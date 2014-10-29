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





typedef struct DaoxTerrainPoint     DaoxTerrainPoint;
typedef struct DaoxTerrainBorder    DaoxTerrainBorder;
typedef struct DaoxTerrainTriangle  DaoxTerrainTriangle;
typedef struct DaoxTerrainBlock     DaoxTerrainBlock;
typedef struct DaoxTerrain          DaoxTerrain;

typedef struct DaoxTerrainParams    DaoxTerrainParams;
typedef struct DaoxTerrainGenerator DaoxTerrainGenerator;


enum DaoxTerrainTypes
{
	DAOX_RECT_TERRAIN ,
	DAOX_HEX_TERRAIN
};


struct DaoxTerrainParams
{
	float  faultScale;
	float  amplitude;
	float  resolution;
};


struct DaoxTerrainPoint
{
	DaoxVector3D  pos;
	DaoxVector3D  norm;
	DaoxVector3D  tan;
	DaoxVector2D  tex;
	float         flat;
	int           id;
};

struct DaoxTerrainBorder
{
	DaoxTerrainPoint   *start;
	DaoxTerrainPoint   *end;

	DaoxTerrainBorder  *left;
	DaoxTerrainBorder  *right;
};

struct DaoxTerrainTriangle
{
	DaoxTerrainPoint     *points[3];
	DaoxTerrainBorder    *borders[3];
	DaoxTerrainTriangle  *splits[4];
};

struct DaoxTerrainBlock
{
	DAO_CSTRUCT_COMMON;

	short                 geotype;
	short                 sides;
	DaoxTerrainPoint     *center;
	DaoxTerrainTriangle  *splits[6];
	DaoxTerrainBorder    *borders[6];
	DaoxTerrainBorder    *spokes[6];
	DaoxTerrainBlock     *neighbors[6];
	DaoxTerrainBlock     *next;
	DaoxTerrainParams    *params;
	DaoxMeshUnit         *mesh;
};
extern DaoType *daox_type_terrain_block;

DaoxTerrainBlock* DaoxTerrainBlock_New( int sides );
void DaoxTerrainBlock_Delete( DaoxTerrainBlock *self );

struct DaoxTerrain
{
	DaoxSceneNode  base;
	DaoxMesh      *mesh;

	DaoArray     *heightmap;

	DaoxTerrainBlock  *first;
	DaoxTerrainBlock  *last;

	DList     *points;
	DList     *borders;
	DList     *tiles;

	short  shape;
	short  rows;
	short  columns;
	short  circles;
	float  width;
	float  length;
	float  xbsize;
	float  ybsize;
	float  radius;
	float  height;
	float  depth;
	float  textureScale;
	int    changes;

	DList  *buffer;
};
extern DaoType *daox_type_terrain;

DaoxTerrain* DaoxTerrain_New();
void DaoxTerrain_Delete( DaoxTerrain *self );

void DaoxTerrain_SetRectAutoBlocks( DaoxTerrain *self, float width, float length );
void DaoxTerrain_SetRectBlocks( DaoxTerrain *self, int rows, int columns, float blocksize );
void DaoxTerrain_SetHexBlocks( DaoxTerrain *self, int circles, float blocksize );

void DaoxTerrain_SetHeightmap( DaoxTerrain *self, DaoArray *heightmap );

void DaoxTerrain_InitBlocks( DaoxTerrain *self );
void DaoxTerrain_Rebuild( DaoxTerrain *self );
void DaoxTerrain_Generate( DaoxTerrain *self, int seed );

DaoxTerrainBlock* DaoxTerrain_GetBlock( DaoxTerrain *self, int side, int radius, int offset );

void DaoxTerrain_Export( DaoxTerrain *self, DaoxTerrain *terrain );



struct DaoxTerrainGenerator
{
	DAO_CSTRUCT_COMMON;

	DaoxTerrain    *terrain;

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
