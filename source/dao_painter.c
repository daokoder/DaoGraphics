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

#include <math.h>
#include "dao_painter.h"

DaoxPainter* DaoxPainter_New()
{
	DaoxPainter *self = (DaoxPainter*)dao_calloc(1,sizeof(DaoxPainter));
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_painter );
	DaoxPainter_InitShaders( self );
	DaoxPainter_InitBuffers( self );
	return self;
}
void DaoxPainter_Delete( DaoxPainter *self )
{
	DaoxShader_Free( & self->shader );
	DaoCstruct_Free( (DaoCstruct*) self );
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









void DaoxVG_BufferVertices2D( DaoGLVertex2D *glvertices, DArray *points, float scale )
{
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex2D *glvertex = glvertices + i;
		DaoxVector3D point = points->data.vectors3d[i];
		glvertex->pos.x = point.x * scale;
		glvertex->pos.y = point.y * scale;
		glvertex->texKLMO.k = 0;
		glvertex->texKLMO.l = 0;
		glvertex->texKLMO.m = 0;
		glvertex->texKLMO.o = point.z * scale;
		//printf( "%6i: %9f\n", i, point.z );
	}
}
void DaoxVG_BufferPatches2D( DaoGLVertex2D *glvertices, DArray *points, float scale )
{
	DaoxTexturedPoint *points2 = (DaoxTexturedPoint*) points->data.base;
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex2D *glvertex = glvertices + i;
		DaoxTexturedPoint point = points2[i];
		glvertex->pos.x = point.pos.x * scale;
		glvertex->pos.y = point.pos.y * scale;
		glvertex->texKLMO.k = point.klm.x;
		glvertex->texKLMO.l = point.klm.y;
		glvertex->texKLMO.m = point.klm.z;
		glvertex->texKLMO.o = point.offset * scale;
		//printf( "%6i: %9f\n", i, point.offset );
	}
}
void DaoxVG_BufferVertices3D( DaoGLVertex3DVG *glvertices, DArray *points, float scale )
{
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex3DVG *glvertex = glvertices + i;
		DaoxVector3D point = points->data.vectors3d[i];
		glvertex->pos.x = point.x * scale;
		glvertex->pos.y = point.y * scale;
		glvertex->pos.z = 0;
		glvertex->norm.x = 0;
		glvertex->norm.y = 0;
		glvertex->norm.z = 1;
		glvertex->texKLMO.k = 0;
		glvertex->texKLMO.l = 0;
		glvertex->texKLMO.m = 0;
		glvertex->texKLMO.o = point.z * scale;
		//printf( "%6i: %9f\n", i, point.z );
	}
}
void DaoxVG_BufferPatches3D( DaoGLVertex3DVG *glvertices, DArray *points, float scale )
{
	DaoxTexturedPoint *points2 = (DaoxTexturedPoint*) points->data.base;
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex3DVG *glvertex = glvertices + i;
		DaoxTexturedPoint point = points2[i];
		glvertex->pos.x = point.pos.x * scale;
		glvertex->pos.y = point.pos.y * scale;
		glvertex->pos.z = 0;
		glvertex->norm.x = 0;
		glvertex->norm.y = 0;
		glvertex->norm.z = 1;
		glvertex->texKLMO.k = point.klm.x;
		glvertex->texKLMO.l = point.klm.y;
		glvertex->texKLMO.m = point.klm.z;
		glvertex->texKLMO.o = point.offset * scale;
		//printf( "%6i: %9f\n", i, point.offset );
	}
}
void DaoxVG_BufferVertices( DaoxBuffer *buffer, void *glvertices, DArray *points, float scale )
{
	if( buffer->vertexSize == sizeof(DaoGLVertex2D) ){
		DaoxVG_BufferVertices2D( (DaoGLVertex2D*) glvertices, points, scale );
	}else if( buffer->vertexSize == sizeof(DaoGLVertex3DVG) ){
		DaoxVG_BufferVertices3D( (DaoGLVertex3DVG*) glvertices, points, scale );
	}
}
void DaoxVG_BufferPatches( DaoxBuffer *buffer, void *glvertices, DArray *points, float scale )
{
	if( buffer->vertexSize == sizeof(DaoGLVertex2D) ){
		DaoxVG_BufferPatches2D( (DaoGLVertex2D*) glvertices, points, scale );
	}else if( buffer->vertexSize == sizeof(DaoGLVertex3DVG) ){
		DaoxVG_BufferPatches3D( (DaoGLVertex3DVG*) glvertices, points, scale );
	}
}
void DaoxVG_BufferTriangles( DaoGLTriangle *gltriangles, DArray *triangles, int offset )
{
	int i, triangleCount = triangles->size;
	for(i=0; i<triangleCount; ++i){
		DaoGLTriangle *triangle = gltriangles + i;
		uint_t *indices = triangles->data.triangles[i].index;
		triangle->index[0] = indices[0] + offset;
		triangle->index[1] = indices[1] + offset;
		triangle->index[2] = indices[2] + offset;
	}
}

#define USE_STENCIL
void DaoxVG_PaintItemData( DaoxShader *shader, DaoxBuffer *buffer, DaoxCanvas *canvas, DaoxCanvasNode *item )
{
	DaoxCanvasState *state = item->state;
	float resolution = DaoxCanvas_Scale( canvas );
	float scale = item->scale;
	float stroke = state->strokeWidth * scale / (resolution + 1E-16);
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
		DaoxVG_BufferVertices( buffer, fillVertices, item->path->mesh->points, scale );
		DaoxVG_BufferPatches( buffer, fillVertices2, item->path->mesh->patches, scale );
		DaoxVG_BufferTriangles( fillTriangles, item->path->mesh->triangles, fillOffset );
	}

	if( item->strokes ){
		DaoxVG_BufferVertices( buffer, strokeVertices, item->strokes->points, scale );
		DaoxVG_BufferPatches( buffer, strokeVertices2, item->strokes->patches, scale );
		DaoxVG_BufferTriangles( strokeTriangles, item->strokes->triangles, strokeOffset );
	}

	//printf( "DaoxVG_PaintItemData: %i %i\n", vertexCount, triangleCount );

	glUniform1i(shader->uniforms.hasColorTexture, 0 );
	glUniform4fv( shader->uniforms.brushColor, 1, & item->state->strokeColor.red );
	if( item->path ) glUniform1f(shader->uniforms.pathLength, item->path->length );

	if( fill ){
		void *indices = (void*) (buffer->triangleOffset*sizeof(GLint)*3);
		glUniform1i( shader->uniforms.dashCount, 0 );
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

void DaoxPainter_PaintImageItem( DaoxPainter *self, DaoxCanvasNode *item )
{
	int i;
	int w = item->data.texture->image->width;
	int h = item->data.texture->image->height;
	DaoGLTriangle *triangles;
	void *indices;

	DaoxTexture_glInitTexture( item->data.texture );
	//printf( "DaoxPainter_PaintImageItem %i\n", item->data.texture->tid );
	if( item->data.texture->tid == 0 ) return;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, item->data.texture->tid);
	glUniform1i( self->shader.uniforms.hasColorTexture, 1 );
	glUniform1i( self->shader.uniforms.colorTexture, 0 );
	glUniform1f( self->shader.uniforms.alphaBlending, 1.0 );
	glUniform1i( self->shader.uniforms.dashCount, 0 );

	if( self->buffer.vertexSize == sizeof(DaoGLVertex2D) ){
		DaoGLVertex2D *vertices = DaoxBuffer_MapVertices2D( & self->buffer, 4 );
		vertices[0].pos.x = 0;
		vertices[0].pos.y = 0;
		vertices[1].pos.x = w;
		vertices[1].pos.y = 0;
		vertices[2].pos.x = w;
		vertices[2].pos.y = h;
		vertices[3].pos.x = 0;
		vertices[3].pos.y = h;
		vertices[0].texKLMO.k = 0;
		vertices[0].texKLMO.l = 0;
		vertices[1].texKLMO.k = 1;
		vertices[1].texKLMO.l = 0;
		vertices[2].texKLMO.k = 1;
		vertices[2].texKLMO.l = 1;
		vertices[3].texKLMO.k = 0;
		vertices[3].texKLMO.l = 1;
		for(i=0; i<4; ++i){
			vertices[i].texKLMO.m = 0.0;
			vertices[i].texKLMO.o = 0.0;
		}
	}else if( self->buffer.vertexSize == sizeof(DaoGLVertex3DVG) ){
		DaoGLVertex3D *vertices = DaoxBuffer_MapVertices3D( & self->buffer, 4 );
		vertices[0].pos.x = 0;
		vertices[0].pos.y = 0;
		vertices[1].pos.x = w;
		vertices[1].pos.y = 0;
		vertices[2].pos.x = w;
		vertices[2].pos.y = h;
		vertices[3].pos.x = 0;
		vertices[3].pos.y = h;
		for(i=0; i<4; ++i){
			vertices[i].pos.z = 0;
			vertices[i].norm.x = 0;
			vertices[i].norm.y = 0;
			vertices[i].norm.z = 1;
			vertices[i].tex.x = 0;
			vertices[i].tex.y = 1;
		}
	}
	triangles = DaoxBuffer_MapTriangles( & self->buffer, 2 );
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


void DaoxPainter_PaintItem( DaoxPainter *self, DaoxCanvas *canvas, DaoxCanvasNode *item, DaoxMatrix3D transform )
{
	DaoxOBBox2D obbox;
	DaoxMatrix3D inverse;
	DaoxVector3D itempos = {0.0,0.0,0.0};
	GLfloat modelMatrix[16] = {0};
	float distance, diameter;
	float scale = DaoxCanvas_Scale( canvas );
	float stroke = item->state->strokeWidth / (scale + 1E-16);
	int n = item->children ? item->children->size : 0;
	int k = stroke >= 1.0;
	int m = stroke >= 1E-3;
	int i, triangles;

	DaoxCanvasNode_Update( item, canvas );
	DaoxMatrix3D_Multiply( & transform, item->transform );
	obbox = DaoxOBBox2D_Transform( & item->obbox, & transform );
	itempos.x = obbox.O.x;
	itempos.y = obbox.O.y;
	distance = DaoxVector3D_Dist( & self->campos, & itempos );
	diameter = DaoxVector2D_Dist( obbox.X, obbox.Y );
	//printf( "DaoxPainter_PaintItem 1: %s %g %g\n", item->ctype->name->mbs, diameter, distance );
	if( diameter < 1E-5 * distance * scale ) return;
	//printf( "DaoxPainter_PaintItem 2: %s\n", item->ctype->name->mbs );
	if( DaoxOBBox2D_Intersect( & self->obbox, & obbox ) < 0 ) return;
	//printf( "DaoxPainter_PaintItem 3: %s\n", item->ctype->name->mbs );
	//printf( "DaoxPainter_PaintItem 2\n" );

	DaoxGraphics_TransfromMatrix( transform, modelMatrix );
	glUniformMatrix4fv( self->shader.uniforms.modelMatrix, 1, 0, modelMatrix );
	if( item->visible ){
		DaoxVG_PaintItemData( & self->shader, & self->buffer, canvas, item );
		if( item->ctype == daox_type_canvas_image && item->data.texture )
			DaoxPainter_PaintImageItem( self, item );
	}

	for(i=0; i<n; i++){
		DaoxCanvasNode *it = item->children->items.pCanvasNode[i];
		DaoxPainter_PaintItem( self, canvas, it, transform );
	}
}
void MakeProjectionMatrix( DaoxViewFrustum *frustum, DaoxCamera *cam, GLfloat matrix[16] );
void MakeOrthographicMatrix( DaoxViewFrustum *frustum, DaoxCamera *cam, GLfloat matrix[16] );

void DaoxPainter_PaintCanvas( DaoxPainter *self, DaoxCanvas *canvas, DaoxCamera *camera )
{
	DaoxVector3D origin = {0.0, 0.0, 0.0};
	DaoxVector3D norm = {0.0, 0.0, 1.0};
	DaoxVector3D points3d[4];
	DaoxVector2D points2d[4];
	DaoxMatrix4D viewMatrix;
	DaoxMatrix4D worldToCanvas;
	DaoxViewFrustum frustum;
	DaoxColor bgcolor = canvas->background;
	GLfloat matrix[16] = {0};
	GLfloat matrix2[16] = {0};
	GLfloat matrix3[16] = {0};
	int i, n = canvas->nodes->size;

	viewMatrix = DaoxSceneNode_GetWorldTransform( & camera->base );
	viewMatrix = DaoxMatrix4D_Inverse( & viewMatrix );
	DaoxMatrix4D_Export( & viewMatrix, matrix3 );

	DaoxViewFrustum_Init( & frustum, camera );
	MakeProjectionMatrix( & frustum, camera, matrix2 );

	worldToCanvas = DaoxSceneNode_GetWorldTransform( & canvas->base );
	worldToCanvas = DaoxMatrix4D_Inverse( & worldToCanvas );
	frustum = DaoxViewFrustum_Transform( & frustum, & worldToCanvas );
	self->campos = frustum.cameraPosition;
	points3d[0] = DaoxVector3D_Add( & self->campos, & frustum.topLeftEdge );
	points3d[1] = DaoxVector3D_Add( & self->campos, & frustum.topRightEdge );
	points3d[2] = DaoxVector3D_Add( & self->campos, & frustum.bottomLeftEdge );
	points3d[3] = DaoxVector3D_Add( & self->campos, & frustum.bottomRightEdge );
	points3d[0] = DaoxPlaneLineIntersect( origin, norm, self->campos, points3d[0] );
	points3d[1] = DaoxPlaneLineIntersect( origin, norm, self->campos, points3d[1] );
	points3d[2] = DaoxPlaneLineIntersect( origin, norm, self->campos, points3d[2] );
	points3d[3] = DaoxPlaneLineIntersect( origin, norm, self->campos, points3d[3] );
	for(i=0; i<4; ++i){
		points2d[i].x = points3d[i].x;
		points2d[i].y = points3d[i].y;
		//DaoxVector2D_Print( points2d + i );
	}
	DaoxOBBox2D_ResetBox( & self->obbox, points2d, 4 );

	glUseProgram( self->shader.program );
	glUniformMatrix4fv( self->shader.uniforms.projMatrix, 1, 0, matrix2 );
	glUniformMatrix4fv( self->shader.uniforms.viewMatrix, 1, 0, matrix3 );
	glUniform1i(self->shader.uniforms.hasColorTexture, 0 );
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

	for(i=0; i<n; i++){
		DaoxCanvasNode *it = canvas->nodes->items.pCanvasNode[i];
		DaoxPainter_PaintItem( self, canvas, it, canvas->transform );
	}
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
	camera.base.scale = DaoxVector3D_XYZ( 1.0, 1.0, 1.0 );
	camera.base.rotation = DaoxVector3D_XYZ( 0.0, 0.0, 0.0 );
	camera.base.translation = DaoxVector3D_XYZ( 0.0, 0.0, 0.0 );
	camera.aspectRatio = W / H;
	camera.fovAngle = 90.0;
	camera.nearPlane = 0.01*W;
	camera.farPlane = W;

	/*
	// For DaoxPainter_PaintCanvasImage() when rendering to big image,
	// with subdivided viewport.
	//
	// Don't use:
	//   DaoxCamera_MoveXYZ( & camera, CX, CY, 0.5*W );
	//   DaoxCamera_LookAtXYZ( & camera, CX, CY, -1.0 );
	// which will not correctly orient the camera for rendering correct subimages.
	*/
	DaoxSceneNode_MoveXYZ( (DaoxSceneNode*) & camera, CX, CY, 0.5*W );
	camera.viewTarget.x = CX;
	camera.viewTarget.y = CY;
	camera.viewTarget.z = -1.0;

	DaoxPainter_PaintCanvas( self, canvas, & camera );
}
void DaoxPainter_Render( DaoxPainter *self, DaoxCanvas *canvas, DaoxCamera *camera )
{
}


void DaoxPainter_PaintSubSceneImage( DaoxPainter *self, DaoxCanvas *canvas, DaoxAABBox2D viewport, DaoxImage *image, DaoxAABBox2D rect )
{
	DaoxAABBox2D rect2, subViewport = viewport;
	DaoxColor bgcolor = canvas->background;
	int x = rect.left;
	int y = rect.bottom;
	int width = rect.right - rect.left + 1;
	int height = rect.top - rect.bottom + 1;
	int pixelBytes = 1 + image->depth;
	uchar_t *imageData = image->imageData + y * image->widthStep + x * pixelBytes;
	int left = viewport.left;
	int right = viewport.right;
	int bottom = viewport.bottom;
	int top = viewport.top;
	int canvasWidth = right - left;
	int canvasHeight = top - bottom;
	int xmoreScene = 0.0, ymoreScene = 0.0;
	int margin, canvasRight, canvasTop;
	int destWidth = width, destHeight = height;
	int xmoreWin = 0, ymoreWin = 0;
	int xwin = 0, ywin = 0;

	glReadBuffer( GL_BACK );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
	glPixelStorei( GL_PACK_ROW_LENGTH, image->width );

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor( bgcolor.red, bgcolor.green, bgcolor.blue, bgcolor.alpha );

	if( width > canvas->defaultWidth ){
		xmoreWin = width - canvas->defaultWidth;
		xmoreScene = xmoreWin * canvasWidth / width;
		destWidth = canvas->defaultWidth;
		subViewport.right = right - xmoreScene;
	}else{
		xwin = (canvas->defaultWidth - width);
		margin = xwin * canvasWidth / width;
		subViewport.right += margin;
	}
	if( height > canvas->defaultHeight ){
		ymoreWin = height - canvas->defaultHeight;
		ymoreScene = ymoreWin * canvasHeight / height;
		destHeight = canvas->defaultHeight;
		subViewport.top = top - ymoreScene;
	}else{
		ywin = (canvas->defaultHeight - height);
		margin = ywin * canvasHeight / height;
		subViewport.top += margin;
	}

	DaoxPainter_Paint( self, canvas, subViewport );
	glReadPixels( 0, 0, destWidth, destHeight, GL_RGBA, GL_UNSIGNED_BYTE, imageData );

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

	image->depth = DAOX_IMAGE_BIT32;
	DaoxImage_Resize( image, width, height );

	rect.right = width - 1;
	rect.top = height - 1;
	/*
	// XXX: for some reason, rendering once will produce some artifacts
	// when the output image is big and require subdividing the viewport!
	*/
	DaoxPainter_PaintSubSceneImage( self, canvas, viewport, image, rect );
	DaoxPainter_PaintSubSceneImage( self, canvas, viewport, image, rect );
}
