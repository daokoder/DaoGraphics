/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2012-2014, Limin Fu
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

#ifndef __DAO_OPENGL_H__
#define __DAO_OPENGL_H__



#if defined(__APPLE__)

# ifdef DAO_GRAPHICS_USE_GLES
#   include <gl.h>
# else
    //#include <OpenGL/gl.h>
#   include <OpenGL/gl3.h>
# endif

#else

# define GL3_PROTOTYPES
# include <GL/gl.h>
//# include <GL/gl3.h>
# define __gl_h_

#endif



#include "dao_canvas.h"



typedef struct DaoGLVertex2D    DaoGLVertex2D;
typedef struct DaoGLVertex3D    DaoGLVertex3D;
typedef struct DaoGLVertex3DVG  DaoGLVertex3DVG;
typedef struct DaoGLTriangle    DaoGLTriangle;

typedef struct DaoxContext      DaoxContext;
typedef struct DaoxShader       DaoxShader;
typedef struct DaoxBuffer       DaoxBuffer;


enum DaoxGraphicsMode
{
	DAOX_GRAPHICS_2D ,
	DAOX_GRAPHICS_3D ,
	DAOX_GRAPHICS_3DVG
};


enum DaoxSamplerID
{
	DAOX_DASH_SAMPLER = 2 ,
	DAOX_GRADIENT_SAMPLER ,
	DAOX_TILE_TEXTURE1 ,
	DAOX_TILE_TEXTURE2 ,
	DAOX_TILE_TEXTURE3 ,
	DAOX_TILE_TEXTURE4 ,
	DAOX_TILE_TEXTURE5 ,
	DAOX_TILE_TEXTURE6
};




struct DaoGLVertex2D
{
	struct { GLfloat  x, y; }        pos;
	struct { GLfloat  k, l, m, o; }  texKLMO; /* texture coordinates or KLM for bezier curve; */
};

struct DaoGLVertex3D
{
	struct { GLfloat  x, y, z; }  pos;
	struct { GLfloat  x, y, z; }  norm;
	struct { GLfloat  x, y, z; }  tan;
	struct { GLfloat  x, y; }     tex;
};

struct DaoGLVertex3DVG
{
	struct { GLfloat  x, y, z; }     pos;
	struct { GLfloat  x, y, z; }     norm;
	struct { GLfloat  k, l, m, o; }  texKLMO;
};

struct DaoGLTriangle
{
	GLint index[3];
};





struct DaoxShader
{
	DAO_CSTRUCT_COMMON;

	void   *ctx;
	DList  *vertexSources;
	DList  *fragmentSources;

	uint_t  mode;
	uint_t  vertexShader;
	uint_t  fragmentShader;
	uint_t  program;

	struct {
		uint_t  vectorGraphics;
		uint_t  projMatrix;
		uint_t  viewMatrix;
		uint_t  modelMatrix;
		uint_t  cameraPosition;
		uint_t  lightCount;
		uint_t  lightSource;
		uint_t  lightIntensity;
		uint_t  ambientColor;
		uint_t  diffuseColor;
		uint_t  specularColor;
		uint_t  emissionColor;
		uint_t  fade_factor;
		uint_t  material;
		uint_t  hasColorTexture;
		uint_t  hasBumpTexture;
		uint_t  colorTexture;
		uint_t  bumpTexture;
		uint_t  alphaBlending;
		uint_t  graphScale;
		uint_t  pathLength;
		uint_t  brushColor;
		uint_t  dashCount;
		uint_t  dashSampler;
		uint_t  gradientType;
		uint_t  gradientStops;
		uint_t  gradientPoint1;
		uint_t  gradientPoint2;
		uint_t  gradientRadius;
		uint_t  gradientSampler;
		uint_t  terrainTileType;
		uint_t  tileTextureCount;
		uint_t  tileTextureScale;
		uint_t  tileTextures[6];
	} uniforms;

	struct {
		uint_t  position;
		uint_t  normal;
		uint_t  tangent;
		uint_t  texCoord;
		uint_t  texMO;
		uint_t  texKLMO;
	} attributes;

	struct {
		uint_t  dashSampler;
		uint_t  gradientSampler;
	} textures;
};
extern DaoType *daox_type_shader;

DaoxShader* DaoxShader_New();
void DaoxShader_Delete( DaoxShader *self );

void DaoxShader_Init2D( DaoxShader *self );
void DaoxShader_Init3D( DaoxShader *self );
void DaoxShader_AddShader( DaoxShader *self, int type, const char *source );
void DaoxShader_AppendShader( DaoxShader *self, int type, const char *source );

void DaoxShader_Build2D( DaoxShader *self );
void DaoxShader_Build3D( DaoxShader *self );
void DaoxShader_Free( DaoxShader *self );

void DaoxShader_MakeGradientSampler( DaoxShader *self, DaoxGradient *gradient, int fill );
void DaoxShader_MakeDashSampler( DaoxShader *self, DaoxBrush *brush );




struct DaoxBuffer
{
	DAO_CSTRUCT_COMMON;

	void    *ctx;

	uint_t   mode;
	uint_t   vertexVAO;
	uint_t   vertexVBO;
	uint_t   triangleVBO;

	uint_t   vertexOffset;
	uint_t   vertexCapacity;
	uint_t   triangleOffset;
	uint_t   triangleCapacity;

	uint_t   vertexSize;    /* size of each vertex; */
	uint_t   triangleSize;  /* size of each triangle; */
	uint_t   traitCount;

	struct {
		uint_t  uniform;
		uint_t  count;
		void   *offset;
	} traits[5];
};
extern DaoType *daox_type_buffer;

DaoxBuffer* DaoxBuffer_New();
void DaoxBuffer_Delete( DaoxBuffer *self );

void DaoxBuffer_Init2D( DaoxBuffer *self, int pos, int klmo );
void DaoxBuffer_Init3D( DaoxBuffer *self, int pos, int norm, int tan, int texuv, int texmo );
void DaoxBuffer_Init3DVG( DaoxBuffer *self, int pos, int norm, int texuv, int texmo );
void DaoxBuffer_Free( DaoxBuffer *self );

void* DaoxBuffer_MapVertices( DaoxBuffer *self, int count );
DaoGLVertex2D*   DaoxBuffer_MapVertices2D( DaoxBuffer *self, int count );
DaoGLVertex3D*   DaoxBuffer_MapVertices3D( DaoxBuffer *self, int count );
DaoGLVertex3DVG* DaoxBuffer_MapVertices3DVG( DaoxBuffer *self, int count );
DaoGLTriangle*   DaoxBuffer_MapTriangles( DaoxBuffer *self, int count );



/*
// DaoxContext:
// -- Responsible for freeing OpenGL resources;
// -- DaoxContext_Clear() must be called in the same thread that allocated the resources;
*/
struct DaoxContext
{
	DAO_CSTRUCT_COMMON;

	DList  *shaders;
	DList  *buffers;
	DList  *textures;

	int  deviceWidth;
	int  deviceHeight;
};
extern DaoType *daox_type_context;

DaoxContext* DaoxContext_New();
void DaoxContext_Delete( DaoxContext *self );
void DaoxContext_Clear( DaoxContext *self );

int DaoxContext_BindShader( DaoxContext *self, DaoxShader *shader );
int DaoxContext_BindBuffer( DaoxContext *self, DaoxBuffer *buffer );
int DaoxContext_BindTexture( DaoxContext *self, DaoxTexture *texture );


void DaoxMatrix4D_Export( DaoxMatrix4D *self, GLfloat matrix[16] );

#endif
