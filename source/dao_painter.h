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

#ifndef __DAO_PAINTER_H__
#define __DAO_PAINTER_H__

#include "dao_canvas.h"
#include "dao_opengl.h"

typedef struct DaoxPainter  DaoxPainter;


struct DaoxPainter
{
	DAO_CSTRUCT_COMMON;

	uchar_t  showMesh;


	DaoxOBBox2D   obbox;
	DaoxVector3D  campos;

	DaoxContext  *context;
	DaoxShader   *shader;
	DaoxBuffer   *buffer;

	void          *vertexBuffer;
	DaoGLTriangle *triangleBuffer;

	uint_t  vertexCount;
	uint_t  triangleCount;
};
extern DaoType *daox_type_painter;

DaoxPainter* DaoxPainter_New( DaoxContext *ctx );
void DaoxPainter_Delete( DaoxPainter *self );

void DaoxPainter_InitShaders( DaoxPainter *self );
void DaoxPainter_InitBuffers( DaoxPainter *self );


void DaoxPainter_Paint( DaoxPainter *self, DaoxCanvas *canvas, DaoxAABBox2D viewport );
void DaoxPainter_PaintCanvas( DaoxPainter *self, DaoxCanvas *canvas, DaoxCamera *camera );

void DaoxPainter_PaintCanvasImage( DaoxPainter *self, DaoxCanvas *canvas, DaoxAABBox2D viewport, DaoxImage *image, int width, int height );


#endif
