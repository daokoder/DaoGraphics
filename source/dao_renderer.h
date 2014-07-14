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

#ifndef __DAO_RENDERER_H__
#define __DAO_RENDERER_H__


#include "dao_scene.h"
#include "dao_opengl.h"


typedef struct DaoxDrawTask DaoxDrawTask; 
typedef struct DaoxRenderer DaoxRenderer;


struct DaoxDrawTask
{
	uint_t         shape;
	uint_t         offset;
	uint_t         vcount;
	uint_t         tcount;
	DList          units;
	DList          chunks;
	DaoxMatrix4D   matrix;   /* Object to world matrix; */
	DaoxMaterial  *material;
	DaoxTexture   *texture;
};



struct DaoxRenderer
{
	DAO_CSTRUCT_COMMON;

	uchar_t  showAxis;
	uint_t   targetWidth;
	uint_t   targetHeight;

	DaoxScene       *scene;
	DaoxCamera      *camera;

	DaoxMesh        *axisMesh;
	DaoxModel       *worldAxis;
	DaoxModel       *localAxis;

	DaoxViewFrustum  frustum;

	DaoxShader  shader;
	DaoxBuffer  terrainBuffer;
	DaoxBuffer  buffer;
	DaoxBuffer  bufferVG;

	DList   *dynamicTasks;
	DList   *staticTasks;

	DList   *terrains;
	DList   *canvases;
	DList   *taskCache;
	DMap    *map;
};
extern DaoType *daox_type_renderer;

DaoxRenderer* DaoxRenderer_New();
void DaoxRenderer_Delete( DaoxRenderer *self );
void DaoxRenderer_InitShaders( DaoxRenderer *self );
void DaoxRenderer_InitBuffers( DaoxRenderer *self );
void DaoxRenderer_Render( DaoxRenderer *self, DaoxScene *scene, DaoxCamera *cam );

void DaoxRenderer_SetCurrentCamera( DaoxRenderer *self, DaoxCamera *camera );
DaoxCamera* DaoxRenderer_GetCurrentCamera( DaoxRenderer *self );

#endif
