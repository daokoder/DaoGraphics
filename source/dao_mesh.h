/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2013-2014, Limin Fu
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


#ifndef __DAO_MESH_H__
#define __DAO_MESH_H__

#include "dao_common.h"


typedef struct DaoxSkinParam  DaoxSkinParam;
typedef struct DaoxMeshChunk  DaoxMeshChunk;
typedef struct DaoxMeshUnit   DaoxMeshUnit;
typedef struct DaoxMesh       DaoxMesh;

typedef struct DaoxMeshNode   DaoxMeshNode;
typedef struct DaoxMeshEdge   DaoxMeshEdge;
typedef struct DaoxMeshFace   DaoxMeshFace;
typedef struct DaoxMeshFrame  DaoxMeshFrame;



struct DaoxSkinParam
{
	int    joints[4];
	float  weights[4];
};

/*
// All coordinates in the mesh vertices and bouding boxes are local.
*/

struct DaoxMeshChunk
{
	DaoxOBBox3D     obbox;      /* with local coordinates in the mesh; */
	DArray         *triangles;  /* <int>: with triangle indices in DaoxMeshUnit; */

	DaoxMeshUnit   *unit;
	DaoxMeshChunk  *parent;
	DaoxMeshChunk  *left;
	DaoxMeshChunk  *right;
};
DaoxMeshChunk* DaoxMeshChunk_New( DaoxMeshUnit *unit );
void DaoxMeshChunk_Delete( DaoxMeshChunk *self );



struct DaoxMeshUnit
{
	DAO_CSTRUCT_COMMON;

	DaoxMesh        *mesh;
	DaoxMeshChunk   *tree;
	DaoxMaterial    *material;
	DArray          *skinParams;
	DArray          *vertices;  /* <DaoxVertex>: local coordinates; */
	DArray          *triangles; /* <DaoxTriangle>: local coordinates (for face norms); */
	DaoxOBBox3D      obbox;     /* local coordinates; */
	uint_t           index;     /* unit index in the mesh; */
};
extern DaoType *daox_type_mesh_unit;

DaoxMeshUnit* DaoxMeshUnit_New();
void DaoxMeshUnit_Delete( DaoxMeshUnit *self );

void DaoxMeshUnit_MoveBy( DaoxMeshUnit *self, float dx, float dy, float dz );
void DaoxMeshUnit_ScaleBy( DaoxMeshUnit *self, float fx, float fy, float fz );
void DaoxMeshUnit_SetMaterial( DaoxMeshUnit *self, DaoxMaterial *material );
void DaoxMeshUnit_UpdateNormTangents( DaoxMeshUnit *self, int donormal, int dotangent );




struct DaoxMesh
{
	DAO_CSTRUCT_COMMON;

	DList       *units;
	DaoxOBBox3D  obbox;  /* local coordinates; */
};
extern DaoType *daox_type_mesh;

DaoxMesh* DaoxMesh_New();
void DaoxMesh_Delete( DaoxMesh *self );

DaoxMeshUnit* DaoxMesh_AddUnit( DaoxMesh *self );
void DaoxMesh_SetMaterial( DaoxMesh *self, DaoxMaterial *material );
void DaoxMesh_ResetBoundingBox( DaoxMesh *self );
void DaoxMesh_UpdateNormTangents( DaoxMesh *self, int norm, int tan );
void DaoxMesh_UpdateTree( DaoxMesh *self, int maxtriangles );
void DaoxMesh_MakeViewFrustumCorners( DaoxMesh *self, float fov, float ratio, float near );

DaoxMeshUnit* DaoxMesh_MakeBox( DaoxMesh *self, float wx, float wy, float wz );
DaoxMeshUnit* DaoxMesh_MakeCube( DaoxMesh *self );
DaoxMeshUnit* DaoxMesh_MakeSphere( DaoxMesh *self, float radius, int resolution );




struct DaoxMeshNode
{
	DaoxVector3D  pos;
	DaoxVector3D  norm;
	int           id;
};

struct DaoxMeshEdge
{
	DaoxMeshNode  *start;
	DaoxMeshNode  *end;

	DaoxMeshEdge  *left;
	DaoxMeshEdge  *right;
};

/* Just trangle face: */
struct DaoxMeshFace
{
	DaoxMeshNode  *nodes[3];
	DaoxMeshEdge  *edges[3];
	DaoxMeshFace  *splits[4];
};

struct DaoxMeshFrame
{
	DList  *nodes;
	DList  *edges;
	DList  *faces;
	int     usedNodes;
	int     usedEdges;
	int     usedFaces;
};

DaoxMeshFrame* DaoxMeshFrame_New();
void DaoxMeshFrame_Delete( DaoxMeshFrame *self );

void DaoxMeshFrame_Split( DaoxMeshFrame *self, DaoxMeshFace *face );
void DaoxMeshFrame_Export( DaoxMeshFrame *self, DaoxMeshUnit *unit );

void DaoxMeshFrame_MakeSphere( DaoxMeshFrame *self, float radius, int resolution );

#endif
