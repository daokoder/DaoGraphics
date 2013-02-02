/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2013, Limin Fu
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

#include <math.h>
#include "dao_painter.h"

DaoxPainter* DaoxPainter_New()
{
	DaoxPainter *self = (DaoxPainter*)dao_calloc(1,sizeof(DaoxPainter));
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_painter );
	self->utility = DaoxCanvasUtility_New();
	DaoxPainter_InitShaders( self );
	DaoxPainter_InitBuffers( self );
	return self;
}
void DaoxPainter_Delete( DaoxPainter *self )
{
	DaoxShader_Free( & self->shader );
	DaoCstruct_Free( (DaoCstruct*) self );
	DaoxCanvasUtility_Delete( self->utility );
	dao_free( self );
}
void DaoxPainter_InitShaders( DaoxPainter *self )
{
	DaoxShader_Init2D( & self->shader );
	DaoxShader_Finalize2D( & self->shader );
}
void DaoxPainter_InitBuffers( DaoxPainter *self )
{
	int pos  = self->shader.attributes.position;
	int texKLMO = self->shader.attributes.texKLMO;
	DaoxBuffer_Init2D( & self->buffer, pos, texKLMO );
}
void DaoxPainter_Reset( DaoxPainter *self, DaoxCanvasItem *item )
{
	self->transform = NULL;
	self->item = item;
}









#if 0
void DaoxPainter_PaintCanvasText( DaoxPainter *self, DaoxCanvasText *text )
{
	int i, j, jt = DAOX_JUNCTION_FLAT;
	float scale, offset, maxlen, maxdiff;
	float gscale = DaoxCanvas_Scale( self->canvas );
	float width = text->state->strokeWidth;
	float size = text->state->fontSize;
	DaoxFont *font = text->state->font;
	DaoxGlyph *glyph;

	if( text->codepoint == 0 ) return;
	if( font == NULL ) return;
	
	scale = size / (float)font->fontHeight;
	maxlen = 8.0 * font->fontHeight / size; 
	maxdiff = 2.0 / size;

	DaoxPainter_Reset( self, text );
	self->junction = DAOX_JUNCTION_FLAT;
	self->maxlen = maxlen;
	self->maxdiff = maxdiff;
	self->strokeWidth /= scale + 1E-16;

	glyph = DaoxFont_GetCharGlyph( font, text->codepoint );
	DaoxPainter_PaintPath( self, glyph->shape );
}
#endif







void DaoxVG_BufferVertices2D( DaoGLVertex2D *glvertices, DaoxPlainArray *points )
{
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex2D *glvertex = glvertices + i;
		DaoxVector3D point = points->pod.vectors3d[i];
		glvertex->point.x = point.x;
		glvertex->point.y = point.y;
		glvertex->texKLMO.k = 0;
		glvertex->texKLMO.l = 0;
		glvertex->texKLMO.m = 0;
		glvertex->texKLMO.o = point.z;
		//printf( "%6i: %9f\n", i, point.z );
	}
}
void DaoxVG_BufferPatches2D( DaoGLVertex2D *glvertices, DaoxPlainArray *points )
{
	DaoxTexturedPoint *points2 = (DaoxTexturedPoint*) points->pod.data;
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex2D *glvertex = glvertices + i;
		DaoxTexturedPoint point = points2[i];
		glvertex->point.x = point.point.x;
		glvertex->point.y = point.point.y;
		glvertex->texKLMO.k = point.klm.x;
		glvertex->texKLMO.l = point.klm.y;
		glvertex->texKLMO.m = point.klm.z;
		glvertex->texKLMO.o = point.offset;
		//printf( "%6i: %9f\n", i, point.offset );
	}
}
void DaoxVG_BufferVertices3D( DaoGLVertex3DVG *glvertices, DaoxPlainArray *points )
{
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex3DVG *glvertex = glvertices + i;
		DaoxVector3D point = points->pod.vectors3d[i];
		glvertex->point.x = point.x;
		glvertex->point.y = point.y;
		glvertex->point.z = 0;
		glvertex->norm.x = 0;
		glvertex->norm.y = 0;
		glvertex->norm.z = 1;
		glvertex->texKLMO.k = 0;
		glvertex->texKLMO.l = 0;
		glvertex->texKLMO.m = 0;
		glvertex->texKLMO.o = point.z;
		//printf( "%6i: %9f\n", i, point.z );
	}
}
void DaoxVG_BufferPatches3D( DaoGLVertex3DVG *glvertices, DaoxPlainArray *points )
{
	DaoxTexturedPoint *points2 = (DaoxTexturedPoint*) points->pod.data;
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex3DVG *glvertex = glvertices + i;
		DaoxTexturedPoint point = points2[i];
		glvertex->point.x = point.point.x;
		glvertex->point.y = point.point.y;
		glvertex->point.z = 0;
		glvertex->norm.x = 0;
		glvertex->norm.y = 0;
		glvertex->norm.z = 1;
		glvertex->texKLMO.k = point.klm.x;
		glvertex->texKLMO.l = point.klm.y;
		glvertex->texKLMO.m = point.klm.z;
		glvertex->texKLMO.o = point.offset;
		//printf( "%6i: %9f\n", i, point.offset );
	}
}
void DaoxVG_BufferVertices( DaoxBuffer *buffer, void *glvertices, DaoxPlainArray *points )
{
	if( buffer->vertexSize == sizeof(DaoGLVertex2D) ){
		DaoxVG_BufferVertices2D( (DaoGLVertex2D*) glvertices, points );
	}else if( buffer->vertexSize == sizeof(DaoGLVertex3DVG) ){
		DaoxVG_BufferVertices3D( (DaoGLVertex3DVG*) glvertices, points );
	}
}
void DaoxVG_BufferPatches( DaoxBuffer *buffer, void *glvertices, DaoxPlainArray *points )
{
	if( buffer->vertexSize == sizeof(DaoGLVertex2D) ){
		DaoxVG_BufferPatches2D( (DaoGLVertex2D*) glvertices, points );
	}else if( buffer->vertexSize == sizeof(DaoGLVertex3DVG) ){
		DaoxVG_BufferPatches3D( (DaoGLVertex3DVG*) glvertices, points );
	}
}
void DaoxVG_BufferTriangles( DaoGLTriangle *gltriangles, DaoxPlainArray *triangles, int offset )
{
	int i, triangleCount = triangles->size;
	for(i=0; i<triangleCount; ++i){
		DaoGLTriangle *triangle = gltriangles + i;
		uint_t *indices = triangles->pod.triangles[i].index;
		triangle->index[0] = indices[0] + offset;
		triangle->index[1] = indices[1] + offset;
		triangle->index[2] = indices[2] + offset;
	}
}

#define USE_STENCIL
void DaoxVG_PaintItemData( DaoxShader *shader, DaoxBuffer *buffer, DaoxCanvas *canvas, DaoxCanvasItem *item )
{
	DaoxCanvasState *state = item->state;
	float scale = DaoxCanvas_Scale( canvas );
	float stroke = state->strokeWidth / (scale + 1E-16);
	int i, fill = item->path != NULL;
	int fillVertexCount = 0;
	int fillVertexCount2 = 0;
	int strokeVertexCount = 0;
	int strokeVertexCount2 = 0;
	int fillTriangleCount = 0;
	int strokeTriangleCount = 0;
	int vertexCount = 0;
	int triangleCount = 0;

	fill &= state->fillColor.alpha > EPSILON || state->fillGradient != NULL;
	if( fill ){
		fillVertexCount = item->path->mesh->points->size;
		fillVertexCount2 = item->path->mesh->patches->size;
		fillTriangleCount = item->path->mesh->triangles->size;
	}
	if( item->strokes ){
		strokeVertexCount = item->strokes->points->size;
		strokeVertexCount2 = item->strokes->patches->size;
		strokeTriangleCount = item->strokes->triangles->size;
	}
	vertexCount = fillVertexCount + fillVertexCount2 + strokeVertexCount + strokeVertexCount2;
	triangleCount = fillTriangleCount + strokeTriangleCount;

	DaoGLTriangle *fillTriangles = DaoxBuffer_MapTriangles( buffer, triangleCount );
	DaoGLTriangle *strokeTriangles = fillTriangles + fillTriangleCount;
	void *fillVertices = DaoxBuffer_MapVertices( buffer, vertexCount );
	void *fillVertices2 = fillVertices + fillVertexCount * buffer->vertexSize;
	void *strokeVertices = fillVertices2 + fillVertexCount2 * buffer->vertexSize;
	void *strokeVertices2 = strokeVertices + strokeVertexCount * buffer->vertexSize;
	int fillOffset = buffer->vertexOffset;
	int fillOffset2 = fillOffset + fillVertexCount;
	int strokeOffset = fillOffset2 + fillVertexCount2;
	int strokeOffset2 = strokeOffset + strokeVertexCount;

	//printf( "buffering: %15p %15p\n", fillVertices, fillTriangles );
	if( fill ){
		DaoxVG_BufferVertices( buffer, fillVertices, item->path->mesh->points );
		DaoxVG_BufferPatches( buffer, fillVertices2, item->path->mesh->patches );
		DaoxVG_BufferTriangles( fillTriangles, item->path->mesh->triangles, fillOffset );
	}

	if( item->strokes ){
		DaoxVG_BufferVertices( buffer, strokeVertices, item->strokes->points );
		DaoxVG_BufferPatches( buffer, strokeVertices2, item->strokes->patches );
		DaoxVG_BufferTriangles( strokeTriangles, item->strokes->triangles, strokeOffset );
	}

	//printf( "DaoxVG_PaintItemData: %i %i\n", vertexCount, triangleCount );

	glUniform1i(shader->uniforms.textureCount, 0 );
	glUniform4fv( shader->uniforms.brushColor, 1, & item->state->strokeColor.red );
	if( item->path ) glUniform1f(shader->uniforms.pathLength, item->path->length );

	if( fill ){
		void *indices = (void*) (buffer->triangleOffset*sizeof(GLint)*3);
		glUniform1f( shader->uniforms.alphaBlending, 1.0 );
		glUniform4fv( shader->uniforms.brushColor, 1, & item->state->fillColor.red );
		DaoxShader_MakeGradientSampler( shader, item->state->fillGradient, 1 );
		glDrawElements( GL_TRIANGLES, 3*fillTriangleCount, GL_UNSIGNED_INT, indices );
		glDrawArrays( GL_TRIANGLES, fillOffset2, fillVertexCount2 );
	}

#ifdef USE_STENCIL
	glEnable( GL_STENCIL_TEST );
	glStencilFunc( GL_NOTEQUAL, 0x01, 0x01);
	glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
#endif
	if( item->strokes && stroke > 1E-3 ){
		int offset = buffer->triangleOffset + fillTriangleCount;
		void *indices = (void*) (offset*sizeof(GLint)*3);
		float alpha = stroke < 1.0 ? sqrt(stroke) : 1.0;
		glUniform1f( shader->uniforms.alphaBlending, alpha );
		glUniform4fv( shader->uniforms.brushColor, 1, & item->state->strokeColor.red );
		DaoxShader_MakeDashSampler( shader, item->state );
		DaoxShader_MakeGradientSampler( shader, item->state->strokeGradient, 0 );
		glDrawElements( GL_TRIANGLES, 3*strokeTriangleCount, GL_UNSIGNED_INT, indices );
		glDrawArrays( GL_TRIANGLES, strokeOffset2, strokeVertexCount2 );
	}


#ifdef USE_STENCIL
	glStencilFunc( GL_ALWAYS, 0x0, 0x01);
	glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
	glUniform1f( shader->uniforms.alphaBlending, 0.0 );
	if( item->strokes && stroke > 1E-3 ){
		int offset = buffer->triangleOffset + fillTriangleCount;
		void *indices = (void*) (offset*sizeof(GLint)*3);
		glUniform4fv( shader->uniforms.brushColor, 1, & item->state->strokeColor.red );
		DaoxShader_MakeGradientSampler( shader, NULL, 0 );
		glDrawElements( GL_TRIANGLES, 3*strokeTriangleCount, GL_UNSIGNED_INT, indices );
		glDrawArrays( GL_TRIANGLES, strokeOffset2, strokeVertexCount2 );
	}
	glDisable( GL_STENCIL_TEST );
#endif


	buffer->vertexOffset += vertexCount;
	buffer->triangleOffset += triangleCount;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void DaoxPainter_PaintImageItem( DaoxPainter *self, DaoxCanvasItem *item )
{
#if 0
	int i;
	int x = item->points->vectors[0].x;
	int y = item->points->vectors[0].y;
	int w = item->texture->image->width;
	int h = item->texture->image->height;
	DaoxColor alpha = { 1.0, 1.0, 1.0, 1.0 };
	DaoGLVertex2D *vertices;
	DaoGLTriangle *triangles;
	void *indices;

	DaoxTexture_glInitTexture( item->texture );
	if( item->texture->tid == 0 ) return;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, item->texture->tid);
	glUniform1i(self->shader.uniforms.textureCount, 1 );
	glUniform1i(self->shader.uniforms.textures[0], 0);
	glUniform4fv( self->shader.uniforms.alpha, 1, & alpha.red );

	vertices = DaoxBuffer_MapVertices2D( & self->buffer, 4 );
	triangles = DaoxBuffer_MapTriangles( & self->buffer, 2 );
	vertices[0].point.x = x;
	vertices[0].point.y = y;
	vertices[1].point.x = x+w;
	vertices[1].point.y = y;
	vertices[2].point.x = x+w;
	vertices[2].point.y = y+h;
	vertices[3].point.x = x;
	vertices[3].point.y = y+h;
	vertices[0].color.r = 0.0;
	vertices[0].color.g = 0.0;
	vertices[1].color.r = 1.0;
	vertices[1].color.g = 0.0;
	vertices[2].color.r = 1.0;
	vertices[2].color.g = 1.0;
	vertices[3].color.r = 0.0;
	vertices[3].color.g = 1.0;
	for(i=0; i<4; ++i) vertices[i].color.b = vertices[i].color.a = 0.0;

	triangles[0].index[0] = self->buffer.vertexOffset;
	triangles[0].index[1] = self->buffer.vertexOffset + 1;
	triangles[0].index[2] = self->buffer.vertexOffset + 2;
	triangles[1].index[0] = self->buffer.vertexOffset + 2;
	triangles[1].index[1] = self->buffer.vertexOffset + 3;
	triangles[1].index[2] = self->buffer.vertexOffset;

	indices = (void*)(self->buffer.triangleOffset*sizeof(GLint)*3);
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices );

	self->buffer.vertexOffset += 4;
	self->buffer.triangleOffset += 2;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}
void DaoxGraphics_TransfromMatrix( DaoxMatrix3D transform, GLfloat matrix[16] )
{
	memset( matrix, 0, 16*sizeof(GLfloat) );
	matrix[0] = transform.A11;
	matrix[4] = transform.A12;
	matrix[12] = transform.B1;
	matrix[1] = transform.A21;
	matrix[5] = transform.A22;
	matrix[13] = transform.B2;
	matrix[15] = 1.0;
}


void DaoxPainter_PaintItem( DaoxPainter *self, DaoxCanvas *canvas, DaoxCanvasItem *item, DaoxMatrix3D transform )
{
	DaoxOBBox2D obbox;
	DaoxMatrix3D inverse;
	GLfloat modelMatrix[16] = {0};
	float scale = DaoxCanvas_Scale( canvas );
	float stroke = item->state->strokeWidth / (scale + 1E-16);
	int n = item->children ? item->children->size : 0;
	int k = stroke >= 1.0;
	int m = stroke >= 1E-3;
	int i, triangles;

	DaoxMatrix3D_Multiply( & transform, item->transform );
	obbox = DaoxOBBox2D_Transform( & item->obbox, & transform );

#if 0
	if( n == 0 && (item->bounds.right > item->bounds.left + 1E-6) ){
		DaoxAABBox2D box = DaoxAABBox2D_Transform( & item->bounds, & transform );

		if( box.left > canvas->viewport.right + 1 ) return;
		if( box.right < canvas->viewport.left - 1 ) return;
		if( box.bottom > canvas->viewport.top + 1 ) return;
		if( box.top < canvas->viewport.bottom - 1 ) return;
	}
#if 0
#endif

	inverse = DaoxMatrix3D_Inverse( & transform );
	bounds = DaoxAABBox2D_Transform( & item->canvas->viewport, & inverse );
	//DaoxAABBox2D_AddMargin( & bounds, 0.1 * (bounds.right - bounds.left) );
	DaoxAABBox2D_AddMargin( & bounds, item->state->strokeWidth + 1 );
	//DaoxAABBox2D_Print( & bounds );
	if( DaoxAABBox2D_Contain( & self->bounds, item->bounds ) == 0 ){
//		if( DaoxAABBox2D_Contain( & self->bounds, bounds ) == 0 )
//			DaoxPainter_Reset( self );
	}
	self->bounds = bounds;
#endif
	DaoxCanvasItem_Update( item, canvas );

	//if( triangles == 0 && n == 0 && item->texture == NULL ) return;


	DaoxGraphics_TransfromMatrix( transform, modelMatrix );
	glUniformMatrix4fv( self->shader.uniforms.modelMatrix, 1, 0, modelMatrix );
	//DaoxPainter_PaintItemData( self, canvas, item );
	DaoxVG_PaintItemData( & self->shader, & self->buffer, canvas, item );
	//if( item->texture ) DaoxPainter_PaintImageItem( self, item );

	for(i=0; i<n; i++){
		DaoxCanvasItem *it = (DaoxCanvasItem*) item->children->items.pVoid[i];
		DaoxPainter_PaintItem( self, canvas, it, transform );
	}
}
void MakeProjectionMatrix( DaoxViewFrustum *frustum, DaoxCamera *cam, GLfloat matrix[16] );

void DaoxPainter_PaintCanvas( DaoxPainter *self, DaoxCanvas *canvas, DaoxCamera *camera )
{
	DaoxMatrix4D viewMatrix;
	DaoxViewFrustum frustum;
	DaoxColor bgcolor = canvas->background;
	GLfloat matrix[16] = {0};
	GLfloat matrix2[16] = {0};
	GLfloat matrix3[16] = {0};
	int i, n = canvas->items->size;

	self->canvas = canvas;

	viewMatrix = DaoxSceneNode_GetWorldTransform( & camera->base );
	viewMatrix = DaoxMatrix4D_Inverse( & viewMatrix );
	DaoxMatrix4D_Export( & viewMatrix, matrix3 );

	DaoxViewFrustum_Init( & frustum, camera );
	MakeProjectionMatrix( & frustum, camera, matrix2 );

	glUseProgram( self->shader.program );
	glUniformMatrix4fv( self->shader.uniforms.projMatrix, 1, 0, matrix2 );
	glUniformMatrix4fv( self->shader.uniforms.viewMatrix, 1, 0, matrix3 );
	glUniform1i(self->shader.uniforms.textureCount, 0 );
	glUniform1i(self->shader.uniforms.dashCount, 0 );
	glUniform1i(self->shader.uniforms.gradientType, 2 );
	glUniform1i(self->shader.uniforms.gradientStops, 2 );
	glUniform1f(self->shader.uniforms.gradientRadius, 200 );

	glActiveTexture(GL_TEXTURE0 + DAOX_GRADIENT_SAMPLER);
	glBindTexture(GL_TEXTURE_1D, self->shader.textures.gradientSampler);
	glUniform1i(self->shader.uniforms.gradientSampler, DAOX_GRADIENT_SAMPLER );

	glActiveTexture(GL_TEXTURE0 + DAOX_DASH_SAMPLER);
	glBindTexture(GL_TEXTURE_1D, self->shader.textures.dashSampler);
	glUniform1i(self->shader.uniforms.dashSampler, DAOX_DASH_SAMPLER );

#if 0
#endif

	glBindVertexArray( self->buffer.vertexVAO );
	glBindBuffer( GL_ARRAY_BUFFER, self->buffer.vertexVBO );
	//glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->buffer.triangleVBO );


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor( bgcolor.red, bgcolor.green, bgcolor.blue, bgcolor.alpha );

	DaoxMatrix3D transform = DaoxMatrix3D_Identity();
	GLfloat modelMatrix[16] = {0};
	DaoxGraphics_TransfromMatrix( transform, modelMatrix );
	glUniformMatrix4fv( self->shader.uniforms.modelMatrix, 1, 0, modelMatrix );

#if 0
	glColor4f( 0.0, 1.0, 0.0, 0.5 );
	glBegin( GL_TRIANGLES );
	glVertex2f( 0.0, 0.0 );
	glVertex2f( 1.0, 0.0 );
	glVertex2f( 0.0, 1.0 );
	glEnd();
#endif

#if 0
	DaoGLVertex2D *vertices;
	DaoGLTriangle *triangles;
	void *indices;
	int x = -200;
	int y = -200;
	int w = 400;
	int h = 400;
	vertices = DaoxBuffer_MapVertices2D( & self->buffer, 3 );
	triangles = DaoxBuffer_MapTriangles( & self->buffer, 1 );
	vertices[0].point.x = x;
	vertices[0].point.y = y;
	vertices[1].point.x = x+w;
	vertices[1].point.y = y;
	vertices[2].point.x = x;
	vertices[2].point.y = y+h;
	vertices[0].color.r = 0.0;
	vertices[0].color.g = 0.0;
	vertices[1].color.r = 0.5;
	vertices[1].color.g = 0.0;
	vertices[2].color.r = 1.0;
	vertices[2].color.g = 1.0;
	vertices[2].color.g = 0.5;
	for(i=0; i<3; ++i) vertices[i].color.b = vertices[i].color.a = 0.0;

	triangles[0].index[0] = self->buffer.vertexOffset;
	triangles[0].index[1] = self->buffer.vertexOffset + 1;
	triangles[0].index[2] = self->buffer.vertexOffset + 2;

	indices = (void*)(self->buffer.triangleOffset*sizeof(GLint)*3);
	glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, indices );

	self->buffer.vertexOffset += 3;
	self->buffer.triangleOffset += 1;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return;
#endif

#ifdef USE_STENCIL
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
#endif

	printf( "DaoxPainter_Paint1: %i %i\n", self->buffer.vertexOffset, self->buffer.triangleOffset );
	for(i=0; i<n; i++){
		DaoxCanvasItem *it = (DaoxCanvasItem*) canvas->items->items.pVoid[i];
		DaoxPainter_PaintItem( self, canvas, it, canvas->transform );
	}
	printf( "DaoxPainter_Paint2: %i %i\n", self->buffer.vertexCapacity, self->buffer.triangleCapacity );
	glBindVertexArray(0);
}
void DaoxPainter_Paint( DaoxPainter *self, DaoxCanvas *canvas, DaoxAABBox2D viewport )
{
	DaoxCamera camera;
	float CX = 0.5*(viewport.left + viewport.right);
	float CY = 0.5*(viewport.top + viewport.bottom);
	float W = viewport.right - viewport.left;
	float H = viewport.top - viewport.bottom;

	camera.base.parent = NULL;
	camera.base.transform = DaoxMatrix4D_Identity();
	camera.aspectRatio = W / H;
	camera.fovAngle = 90.0;
	camera.nearPlane = 0.01*W;
	camera.farPlane = W;

	DaoxCamera_MoveXYZ( & camera, 0.0, 0.0, 0.3*W );
	DaoxCamera_LookAtXYZ( & camera, CX, CY, -1.0 );

	//DaoxCamera_MoveXYZ( & camera, 0.0, 0.0+70, 0.1*W );
	//DaoxCamera_LookAtXYZ( & camera, CX, CY+70, -1.0 );

	//DaoxCamera_MoveXYZ( & camera, 0.0, 0.0-100, 0.3*W );
	//DaoxCamera_LookAtXYZ( & camera, CX, CY-100, -1.0 );

	//DaoxCamera_MoveXYZ( & camera, 0.0+180, 0.0, 0.1*W );
	//DaoxCamera_LookAtXYZ( & camera, CX+180, CY, -1.0 );

	//DaoxCamera_MoveXYZ( & camera, 0.0, 0.0, 0.2*W );
	//DaoxCamera_LookAtXYZ( & camera, CX, CY, -1.0 );

	//DaoxCamera_MoveXYZ( & camera, 0.0+50, 0.0+100, 0.1*W );
	//DaoxCamera_LookAtXYZ( & camera, CX+50, CY+100, -1.0 );

	//DaoxCamera_MoveXYZ( & camera, 0.0+170, 0.0+130, 0.1*W );
	//DaoxCamera_LookAtXYZ( & camera, CX+170, CY+130, -1.0 );

	self->viewport = viewport;

	DaoxPainter_PaintCanvas( self, canvas, & camera );
}
void DaoxPainter_Render( DaoxPainter *self, DaoxCanvas *canvas, DaoxCamera *camera )
{
}


void DaoxPainter_PaintSubSceneImage( DaoxPainter *self, DaoxCanvas *canvas, DaoxAABBox2D viewport, DaoxImage *image, DaoxAABBox2D rect )
{
	DaoxAABBox2D rect2, subViewport = viewport;
	int x = rect.left;
	int y = rect.bottom;
	int width = rect.right - rect.left + 1;
	int height = rect.top - rect.bottom + 1;
	int pixelBytes = 1 + image->depth;
	uchar_t *imageData = image->imageData + y * image->widthStep + x * pixelBytes;
	float left = viewport.left;
	float right = viewport.right;
	float bottom = viewport.bottom;
	float top = viewport.top;
	float canvasWidth = right - left;
	float canvasHeight = top - bottom;
	float xmoreScene = 0.0, ymoreScene = 0.0;
	float margin, canvasRight, canvasTop;
	int destWidth = width, destHeight = height;
	int xmoreWin = 0, ymoreWin = 0;
	int xwin = 0, ywin = 0;

	glReadBuffer( GL_BACK );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
	glPixelStorei( GL_PACK_ROW_LENGTH, image->width );

	if( width > canvas->defaultWidth ){
		xmoreWin = width - canvas->defaultWidth;
		xmoreScene = xmoreWin * canvasWidth / width;
		destWidth = canvas->defaultWidth;
		subViewport.right = right - xmoreScene;
	}else{
		xwin = 0.5 * (canvas->defaultWidth - width);
		margin = xwin * canvasWidth / width;
		subViewport.left -= margin;
		subViewport.right += margin;
	}
	if( height > canvas->defaultHeight ){
		ymoreWin = height - canvas->defaultHeight;
		ymoreScene = ymoreWin * canvasHeight / height;
		destHeight = canvas->defaultHeight;
		subViewport.top = top - ymoreScene;
	}else{
		ywin = 0.5 * (canvas->defaultHeight - height);
		margin = ywin * canvasHeight / height;
		subViewport.bottom -= margin;
		subViewport.top += margin;
	}

	DaoxPainter_Paint( self, canvas, subViewport );
	glReadPixels( xwin, ywin, destWidth, destHeight, GL_RGBA, GL_UNSIGNED_BYTE, imageData );

	canvasRight = subViewport.right;
	canvasTop = subViewport.top;
	if( xmoreWin ){
		subViewport = viewport;
		subViewport.left = canvasRight;
		rect2 = rect;
		rect2.left += canvas->defaultWidth;
		DaoxPainter_PaintSubSceneImage( self, canvas, subViewport, image, rect2 );
	}

	if( ymoreWin ){
		subViewport = viewport;
		subViewport.bottom = canvasTop;
		rect2 = rect;
		rect2.bottom += canvas->defaultHeight;
		DaoxPainter_PaintSubSceneImage( self, canvas, subViewport, image, rect2 );
	}

	if( xmoreWin && ymoreWin ){
		subViewport = viewport;
		subViewport.left = canvasRight;
		subViewport.bottom = canvasTop;
		rect2 = rect;
		rect2.left += canvas->defaultWidth;
		rect2.bottom += canvas->defaultHeight;
		DaoxPainter_PaintSubSceneImage( self, canvas, subViewport, image, rect2 );
	}
}
void DaoxPainter_PaintCanvasImage( DaoxPainter *self, DaoxCanvas *canvas, DaoxAABBox2D viewport, DaoxImage *image, int width, int height )
{
	DaoxAABBox2D rect = { 0.0, 0.0, 0.0, 0.0 };
	float canvasWidth = viewport.right - viewport.left;
	float canvasHeight = viewport.top - viewport.bottom;

	image->depth = DAOX_IMAGE_BIT32;
	DaoxImage_Resize( image, width, height );

	rect.right = width - 1;
	rect.top = height - 1;
	DaoxPainter_PaintSubSceneImage( self, canvas, viewport, image, rect );
}
