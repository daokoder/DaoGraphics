/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2013-2016, Limin Fu
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

DaoxPainter* DaoxPainter_New( DaoxContext *ctx )
{
	DaoxPainter *self = (DaoxPainter*)dao_calloc(1,sizeof(DaoxPainter));
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_painter );
	if( ctx == NULL ) ctx = DaoxContext_New();
	GC_Assign( & self->context, ctx );
	self->shader = DaoxShader_New( ctx );
	self->buffer = DaoxBuffer_New( ctx );
	GC_IncRC( self->shader );
	GC_IncRC( self->buffer );
	DaoxPainter_InitShaders( self );
	DaoxPainter_InitBuffers( self );
	return self;
}
void DaoxPainter_Delete( DaoxPainter *self )
{
	DaoCstruct_Free( (DaoCstruct*) self );
	GC_DecRC( self->shader );
	GC_DecRC( self->buffer );
	GC_DecRC( self->context );
	dao_free( self );
}
void DaoxPainter_InitShaders( DaoxPainter *self )
{
	DaoxShader_Init2D( self->shader );
	DaoxContext_BindShader( self->context, self->shader );
}
void DaoxPainter_InitBuffers( DaoxPainter *self )
{
	int pos  = self->shader->attributes.position;
	int texKLMO = self->shader->attributes.texKLMO;
	DaoxBuffer_Init2D( self->buffer, pos, texKLMO );
	DaoxContext_BindBuffer( self->context, self->buffer );
}
float DaoxPainter_CanvasScale( DaoxPainter *self, DaoxCanvas *canvas )
{
	DaoxAABBox2D box = canvas->viewport;
	float xscale = fabs( box.right - box.left ) / (self->context->deviceWidth + 1);
	float yscale = fabs( box.top - box.bottom ) / (self->context->deviceHeight + 1);
	return 0.5 * (xscale + yscale);
}





void DaoxVG_BufferVertices2D( DaoGLVertex2D *glvertices, DArray *points )
{
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex2D *glvertex = glvertices + i;
		DaoxVector3D point = points->data.vectors3d[i];
		glvertex->pos.x = point.x;
		glvertex->pos.y = point.y;
		glvertex->texKLMO.k = 0;
		glvertex->texKLMO.l = 0;
		glvertex->texKLMO.m = 0;
		glvertex->texKLMO.o = point.z;
		//printf( "%6i: %9f\n", i, point.z );
	}
}
void DaoxVG_BufferBeziers2D( DaoGLVertex2D *glvertices, DArray *points )
{
	DaoxBezierPoint *points2 = (DaoxBezierPoint*) points->data.base;
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex2D *glvertex = glvertices + i;
		DaoxBezierPoint point = points2[i];
		glvertex->pos.x = point.pos.x;
		glvertex->pos.y = point.pos.y;
		glvertex->texKLMO.k = point.klm.x;
		glvertex->texKLMO.l = point.klm.y;
		glvertex->texKLMO.m = point.klm.z;
		glvertex->texKLMO.o = point.offset;
		//printf( "%6i: %9f\n", i, point.offset );
	}
}
void DaoxVG_BufferVertices3D( DaoGLVertex3DVG *glvertices, DArray *points )
{
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex3DVG *glvertex = glvertices + i;
		DaoxVector3D point = points->data.vectors3d[i];
		glvertex->pos.x = point.x;
		glvertex->pos.y = point.y;
		glvertex->pos.z = 0;
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
void DaoxVG_BufferBeziers3D( DaoGLVertex3DVG *glvertices, DArray *points )
{
	DaoxBezierPoint *points2 = (DaoxBezierPoint*) points->data.base;
	int i, vertexCount = points->size;
	for(i=0; i<vertexCount; ++i){
		DaoGLVertex3DVG *glvertex = glvertices + i;
		DaoxBezierPoint point = points2[i];
		glvertex->pos.x = point.pos.x;
		glvertex->pos.y = point.pos.y;
		glvertex->pos.z = 0;
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
void DaoxVG_BufferVertices( DaoxBuffer *buffer, void *glvertices, DArray *points )
{
	if( buffer->vertexSize == sizeof(DaoGLVertex2D) ){
		DaoxVG_BufferVertices2D( (DaoGLVertex2D*) glvertices, points );
	}else if( buffer->vertexSize == sizeof(DaoGLVertex3DVG) ){
		DaoxVG_BufferVertices3D( (DaoGLVertex3DVG*) glvertices, points );
	}
}
void DaoxVG_BufferBeziers( DaoxBuffer *buffer, void *glvertices, DArray *points )
{
	if( buffer->vertexSize == sizeof(DaoGLVertex2D) ){
		DaoxVG_BufferBeziers2D( (DaoGLVertex2D*) glvertices, points );
	}else if( buffer->vertexSize == sizeof(DaoGLVertex3DVG) ){
		DaoxVG_BufferBeziers3D( (DaoGLVertex3DVG*) glvertices, points );
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
void DaoxVG_BufferImageRect( DaoxBuffer *buffer, DaoxBrush *brush, void *glvertices, DaoGLTriangle *gltriangles )
{
	DaoGLTriangle *triangles = gltriangles;
	int offset = buffer->vertexOffset + brush->offset1;
	int w = brush->texture->image->width;
	int h = brush->texture->image->height;
	int i;

	if( buffer->vertexSize == sizeof(DaoGLVertex2D) ){
		DaoGLVertex2D *vertices = (DaoGLVertex2D*) glvertices;
		vertices[0].pos.x = 0;  vertices[0].pos.y = 0;
		vertices[1].pos.x = w;  vertices[1].pos.y = 0;
		vertices[2].pos.x = w;  vertices[2].pos.y = h;
		vertices[3].pos.x = 0;  vertices[3].pos.y = h;
		vertices[0].texKLMO.k = 0;  vertices[0].texKLMO.l = 0;
		vertices[1].texKLMO.k = 1;  vertices[1].texKLMO.l = 0;
		vertices[2].texKLMO.k = 1;  vertices[2].texKLMO.l = 1;
		vertices[3].texKLMO.k = 0;  vertices[3].texKLMO.l = 1;
		for(i=0; i<4; ++i){
			vertices[i].texKLMO.m = 0.0;
			vertices[i].texKLMO.o = 0.0;
		}
	}else if( buffer->vertexSize == sizeof(DaoGLVertex3DVG) ){
		DaoGLVertex3DVG *vertices = (DaoGLVertex3DVG*) glvertices;
		vertices[0].pos.x = 0;  vertices[0].pos.y = 0;
		vertices[1].pos.x = w;  vertices[1].pos.y = 0;
		vertices[2].pos.x = w;  vertices[2].pos.y = h;
		vertices[3].pos.x = 0;  vertices[3].pos.y = h;
		vertices[0].texKLMO.k = 0;  vertices[0].texKLMO.l = 0;
		vertices[1].texKLMO.k = 1;  vertices[1].texKLMO.l = 0;
		vertices[2].texKLMO.k = 1;  vertices[2].texKLMO.l = 1;
		vertices[3].texKLMO.k = 0;  vertices[3].texKLMO.l = 1;
		for(i=0; i<4; ++i){
			vertices[i].pos.z = 0;
			vertices[i].norm.x = 0;
			vertices[i].norm.y = 0;
			vertices[i].norm.z = 1;
			vertices[i].texKLMO.m = 0;
			vertices[i].texKLMO.o = 0;
		}
	}
	triangles[0].index[0] = offset;
	triangles[0].index[1] = offset + 1;
	triangles[0].index[2] = offset + 2;
	triangles[1].index[0] = offset + 2;
	triangles[1].index[1] = offset + 3;
	triangles[1].index[2] = offset;
}

#define USE_STENCIL
void DaoxPainter_PaintItemData( DaoxPainter *self, DaoxCanvas *canvas, DaoxCanvasNode *item )
{
	DaoxShader *shader = self->shader;
	DaoxBuffer *buffer = self->buffer;
	DaoxBrush *brush = item->brush;
	DaoxPathMesh *mesh = item->mesh;
	DaoGLTriangle *fillTriangles = self->triangleBuffer + mesh->fillTriangleOffset;
	DaoGLTriangle *strokeTriangles = self->triangleBuffer + mesh->strokeTriangleOffset;
	void *fillVertices = self->vertexBuffer + mesh->fillVertexOffset * buffer->vertexSize;
	void *fillVertices2 = self->vertexBuffer + mesh->fillVertexOffset2 * buffer->vertexSize;
	void *strokeVertices = self->vertexBuffer + mesh->strokeVertexOffset * buffer->vertexSize;
	void *strokeVertices2 = self->vertexBuffer + mesh->strokeVertexOffset2 * buffer->vertexSize;
	float resolution = DaoxPainter_CanvasScale( self, canvas );
	float scale = item->scale;
	float stroke = brush->strokeStyle.width / (resolution + 1E-16);
	int i, fill = brush->fillColor.alpha > EPSILON || brush->fillGradient != NULL;
	int fillVertexCount = 0;
	int fillVertexCount2 = 0;
	int strokeVertexCount = 0;
	int strokeVertexCount2 = 0;
	int fillTriangleCount = 0;
	int strokeTriangleCount = 0;

	fill &= mesh->fillPoints->size > 0;
	if( fill ){
		fillVertexCount = mesh->fillPoints->size;
		fillVertexCount2 = mesh->fillBeziers->size;
		fillTriangleCount = mesh->fillTriangles->size;
	}
	if( mesh->strokePoints->size ){
		strokeVertexCount = mesh->strokePoints->size;
		strokeVertexCount2 = mesh->strokeBeziers->size;
		strokeTriangleCount = mesh->strokeTriangles->size;
	}

	glUniform1i(shader->uniforms.hasDiffuseTexture, 0 );
	glUniform4fv( shader->uniforms.brushColor, 1, & item->brush->strokeColor.red );

	/* Use item->mesh->path->length, since item->path->length may have not been set: */
	glUniform1f(shader->uniforms.graphScale, scale );
	glUniform1f(shader->uniforms.pathLength, mesh->path->length );

	if( fill ){
		int fillTriangleOffset = buffer->triangleOffset + mesh->fillTriangleOffset;
		int fillVertexOffset2 = buffer->vertexOffset + mesh->fillVertexOffset2;
		void *indices = (void*) (fillTriangleOffset*sizeof(GLint)*3);
		glUniform1i( shader->uniforms.dashCount, 0 );
		glUniform1f( shader->uniforms.alphaBlending, 1.0 );
		glUniform4fv( shader->uniforms.brushColor, 1, & item->brush->fillColor.red );
		DaoxShader_MakeGradientSampler( shader, item->brush->fillGradient, 1 );
		glDrawElements( GL_TRIANGLES, 3*fillTriangleCount, GL_UNSIGNED_INT, indices );
		glDrawArrays( GL_TRIANGLES, fillVertexOffset2, fillVertexCount2 );
	}

#ifdef USE_STENCIL
	glEnable( GL_STENCIL_TEST );
	glStencilFunc( GL_NOTEQUAL, 0x01, 0x01);
	glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
#endif
	if( mesh->strokePoints->size && stroke > 1E-3 ){
		int strokeTriangleOffset = buffer->triangleOffset + mesh->strokeTriangleOffset;
		int strokeVertexOffset2 = buffer->vertexOffset + mesh->strokeVertexOffset2;
		void *indices = (void*) (strokeTriangleOffset*sizeof(GLint)*3);
		float alpha = stroke < 1.0 ? sqrt(stroke) : 1.0;
		glUniform1f( shader->uniforms.alphaBlending, alpha );
		glUniform4fv( shader->uniforms.brushColor, 1, & item->brush->strokeColor.red );
		DaoxShader_MakeDashSampler( shader, item->brush );
		DaoxShader_MakeGradientSampler( shader, item->brush->strokeGradient, 0 );
		glDrawElements( GL_TRIANGLES, 3*strokeTriangleCount, GL_UNSIGNED_INT, indices );
		glDrawArrays( GL_TRIANGLES, strokeVertexOffset2, strokeVertexCount2 );
	}


#ifdef USE_STENCIL
	glStencilFunc( GL_ALWAYS, 0x0, 0x01);
	glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
	glUniform1f( shader->uniforms.alphaBlending, 0.0 );
	if( item->mesh->strokePoints->size && stroke > 1E-3 ){
		int strokeTriangleOffset = buffer->triangleOffset + mesh->strokeTriangleOffset;
		int strokeVertexOffset2 = buffer->vertexOffset + mesh->strokeVertexOffset2;
		void *indices = (void*) (strokeTriangleOffset*sizeof(GLint)*3);
		glUniform4fv( shader->uniforms.brushColor, 1, & item->brush->strokeColor.red );
		DaoxShader_MakeGradientSampler( shader, NULL, 0 );
		glDrawElements( GL_TRIANGLES, 3*strokeTriangleCount, GL_UNSIGNED_INT, indices );
		glDrawArrays( GL_TRIANGLES, strokeVertexOffset2, strokeVertexCount2 );
	}
	glDisable( GL_STENCIL_TEST );
#endif

}

void DaoxPainter_PaintImageItem( DaoxPainter *self, DaoxCanvasNode *item )
{
	size_t offset = self->buffer->triangleOffset + item->brush->offset2;
	void *indices = (void*)(offset*sizeof(GLint)*3);

	if( item->brush->texture == NULL ) return;

	if( item->brush->texture->changed || item->brush->texture->tid == 0 ){
		DaoxContext_BindTexture( self->context, item->brush->texture );
	}
	//printf( "DaoxPainter_PaintImageItem %i\n", item->brush->texture->tid );
	if( item->brush->texture->tid == 0 ) return;

	glActiveTexture(GL_TEXTURE0 + DAOX_DIFFUSE_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, item->brush->texture->tid);
	glUniform1i( self->shader->uniforms.hasDiffuseTexture, 1 );
	glUniform1i( self->shader->uniforms.diffuseTexture, DAOX_DIFFUSE_TEXTURE );
	glUniform1f( self->shader->uniforms.alphaBlending, 1.0 );
	glUniform1i( self->shader->uniforms.dashCount, 0 );
	glUniform1f( self->shader->uniforms.graphScale, item->scale );

	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices );
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

void DaoxPainter_UpdateItem( DaoxPainter *self, DaoxCanvas *canvas, DaoxCanvasNode *item, DaoxMatrix3D transform )
{
	DaoxOBBox2D obbox;
	DaoxMatrix3D inverse;
	DaoxVector3D itempos = {0.0,0.0,0.0};
	DaoxMatrix3D transform2 = DaoxCanvasNode_GetLocalTransform( item );
	GLfloat modelMatrix[16] = {0};
	float distance, diameter;
	float scale = DaoxPainter_CanvasScale( self, canvas );
	float stroke = item->brush->strokeStyle.width / (scale + 1E-16);
	int n = item->children ? item->children->size : 0;
	int k = stroke >= 1.0;
	int m = stroke >= 1E-3;
	int i, triangles;

	DaoxCanvasNode_Update( item, canvas );
	DaoxMatrix3D_Multiply( & transform, transform2 );
	obbox = DaoxOBBox2D_Transform( & item->obbox, & transform );
	itempos.x = obbox.O.x;
	itempos.y = obbox.O.y;
	distance = DaoxVector3D_Dist( & self->campos, & itempos );
	diameter = DaoxVector2D_Dist( obbox.X, obbox.Y );

	if( diameter < 1E-5 * distance * scale ) goto HandleChildrenItems;
	if( DaoxOBBox2D_Intersect( & self->obbox, & obbox ) < 0 ) goto HandleChildrenItems;

	DaoxGraphics_TransfromMatrix( transform, modelMatrix );
	glUniformMatrix4fv( self->shader->uniforms.modelMatrix, 1, 0, modelMatrix );
	if( item->visible && item->ctype == daox_type_canvas_image && item->brush->texture ){
		item->brush->offset1 = self->vertexCount;
		item->brush->offset2 = self->triangleCount;
		self->vertexCount += 4;
		self->triangleCount += 2;
	}else if( item->visible && item->path ){
		DaoxPathMesh *mesh = item->mesh;
		DaoxBrush *brush = item->brush;
		int fill = brush->fillColor.alpha > EPSILON || brush->fillGradient != NULL;
		fill &= mesh->fillPoints->size > 0;

		item->mesh->bufferred = 0;
		//printf( "1>> %i %i\n", self->vertexCount, self->triangleCount );
		if( fill ){
			mesh->fillVertexOffset = self->vertexCount;
			mesh->fillVertexOffset2 = self->vertexCount + mesh->fillPoints->size;
			mesh->fillTriangleOffset = self->triangleCount;
			self->vertexCount = mesh->fillVertexOffset2 + mesh->fillBeziers->size;
			self->triangleCount += mesh->fillTriangles->size;
		}
		if( mesh->strokePoints->size ){
			mesh->strokeVertexOffset = self->vertexCount;
			mesh->strokeVertexOffset2 = self->vertexCount + mesh->strokePoints->size;
			mesh->strokeTriangleOffset = self->triangleCount;
			self->vertexCount = mesh->strokeVertexOffset2 + mesh->strokeBeziers->size;
			self->triangleCount += mesh->strokeTriangles->size;
		}
		//printf( "2>> %i %i\n", self->vertexCount, self->triangleCount );
	}

HandleChildrenItems:
	for(i=0; i<n; i++){
		DaoxCanvasNode *it = item->children->items.pCanvasNode[i];
		DaoxPainter_UpdateItem( self, canvas, it, transform );
	}
}

void DaoxPainter_BufferItem( DaoxPainter *self, DaoxCanvas *canvas, DaoxCanvasNode *item, DaoxMatrix3D transform )
{
	DaoxOBBox2D obbox;
	DaoxMatrix3D inverse;
	DaoxVector3D itempos = {0.0,0.0,0.0};
	DaoxMatrix3D transform2 = DaoxCanvasNode_GetLocalTransform( item );
	DaoxBuffer *buffer = self->buffer;
	GLfloat modelMatrix[16] = {0};
	float distance, diameter;
	float scale = DaoxPainter_CanvasScale( self, canvas );
	float stroke = item->brush->strokeStyle.width / (scale + 1E-16);
	int n = item->children ? item->children->size : 0;
	int k = stroke >= 1.0;
	int m = stroke >= 1E-3;
	int i, triangles;

	DaoxMatrix3D_Multiply( & transform, transform2 );
	obbox = DaoxOBBox2D_Transform( & item->obbox, & transform );
	itempos.x = obbox.O.x;
	itempos.y = obbox.O.y;
	distance = DaoxVector3D_Dist( & self->campos, & itempos );
	diameter = DaoxVector2D_Dist( obbox.X, obbox.Y );

	if( diameter < 1E-5 * distance * scale ) goto HandleChildrenItems;
	if( DaoxOBBox2D_Intersect( & self->obbox, & obbox ) < 0 ) goto HandleChildrenItems;

	DaoxGraphics_TransfromMatrix( transform, modelMatrix );
	glUniformMatrix4fv( self->shader->uniforms.modelMatrix, 1, 0, modelMatrix );
	if( item->visible && item->ctype == daox_type_canvas_image && item->brush->texture ){
		DaoGLTriangle *fillTriangles = self->triangleBuffer + item->brush->offset2;
		void *fillVertices = self->vertexBuffer + item->brush->offset1 * buffer->vertexSize;
		DaoxVG_BufferImageRect( buffer, item->brush, fillVertices, fillTriangles );
	}else if( item->visible && item->path && item->mesh->bufferred == 0 ){
		DaoxPathMesh *mesh = item->mesh;
		DaoxBrush *brush = item->brush;
		DaoGLTriangle *fillTriangles = self->triangleBuffer + mesh->fillTriangleOffset;
		DaoGLTriangle *strokeTriangles = self->triangleBuffer + mesh->strokeTriangleOffset;
		void *fillVertices = self->vertexBuffer + mesh->fillVertexOffset * buffer->vertexSize;
		void *fillVertices2 = self->vertexBuffer + mesh->fillVertexOffset2 * buffer->vertexSize;
		void *strokeVertices = self->vertexBuffer + mesh->strokeVertexOffset * buffer->vertexSize;
		void *strokeVertices2 = self->vertexBuffer + mesh->strokeVertexOffset2 * buffer->vertexSize;
		int fill = brush->fillColor.alpha > EPSILON || brush->fillGradient != NULL;
		float scale = item->scale;
		fill &= mesh->fillPoints->size > 0;
		if( fill ){
			DaoxVG_BufferVertices( buffer, fillVertices, mesh->fillPoints );
			DaoxVG_BufferBeziers( buffer, fillVertices2, mesh->fillBeziers );
			DaoxVG_BufferTriangles( fillTriangles, mesh->fillTriangles, buffer->vertexOffset + mesh->fillVertexOffset );
		}
		if( mesh->strokePoints->size ){
			DaoxVG_BufferVertices( buffer, strokeVertices, mesh->strokePoints );
			DaoxVG_BufferBeziers( buffer, strokeVertices2, mesh->strokeBeziers );
			DaoxVG_BufferTriangles( strokeTriangles, mesh->strokeTriangles, buffer->vertexOffset + mesh->strokeVertexOffset );
		}
		mesh->bufferred = 1;
	}

HandleChildrenItems:
	for(i=0; i<n; i++){
		DaoxCanvasNode *it = item->children->items.pCanvasNode[i];
		DaoxPainter_BufferItem( self, canvas, it, transform );
	}
}

void DaoxPainter_PaintItem( DaoxPainter *self, DaoxCanvas *canvas, DaoxCanvasNode *item, DaoxMatrix3D transform )
{
	DaoxOBBox2D obbox;
	DaoxMatrix3D inverse;
	DaoxVector3D itempos = {0.0,0.0,0.0};
	DaoxMatrix3D transform2 = DaoxCanvasNode_GetLocalTransform( item );
	GLfloat modelMatrix[16] = {0};
	float distance, diameter;
	float scale = DaoxPainter_CanvasScale( self, canvas );
	float stroke = item->brush->strokeStyle.width / (scale + 1E-16);
	int n = item->children ? item->children->size : 0;
	int k = stroke >= 1.0;
	int m = stroke >= 1E-3;
	int i, triangles;

	DaoxMatrix3D_Multiply( & transform, transform2 );
	obbox = DaoxOBBox2D_Transform( & item->obbox, & transform );
	itempos.x = obbox.O.x;
	itempos.y = obbox.O.y;
	distance = DaoxVector3D_Dist( & self->campos, & itempos );
	diameter = DaoxVector2D_Dist( obbox.X, obbox.Y );
	//printf( "DaoxPainter_PaintItem 1: %s %g %g\n", item->ctype->name->chars, diameter, distance );

	if( diameter < 1E-5 * distance * scale ) goto HandleChildrenItems;
	if( DaoxOBBox2D_Intersect( & self->obbox, & obbox ) < 0 ) goto HandleChildrenItems;

	DaoxGraphics_TransfromMatrix( transform, modelMatrix );
	glUniformMatrix4fv( self->shader->uniforms.modelMatrix, 1, 0, modelMatrix );
	if( item->visible ){
		if( item->ctype == daox_type_canvas_image && item->brush->texture ){
			DaoxPainter_PaintImageItem( self, item );
		}else if( item->path ){
			DaoxPainter_PaintItemData( self, canvas, item );
		}
	}

HandleChildrenItems:
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
	DaoxMatrix4D canvasToWorld;
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

	canvasToWorld = DaoxSceneNode_GetWorldTransform( & canvas->base );
	worldToCanvas = DaoxMatrix4D_Inverse( & canvasToWorld );
	//printf( "1>> " ); DaoxMatrix4D_Print( & canvasToWorld );
	//printf( "2>> " ); DaoxMatrix4D_Print( & worldToCanvas );

	/*
	// Use lines passing the corners of the frustum on the far plane,
	// with direction parallel to the view direction to compute the visible
	// area on the canvas.
	//
	// NOTE: the intersections of the frustum corner edges will NOT work!
	// Because the intersections may lie behind the camera when the canvas
	// is still visible.
	*/
	points3d[0] = frustum.topLeftEdge;
	points3d[1] = frustum.topRightEdge;
	points3d[2] = frustum.bottomLeftEdge;
	points3d[3] = frustum.bottomRightEdge;
	origin = DaoxMatrix4D_Transform( & canvasToWorld, & origin );
	norm = DaoxMatrix4D_Transform( & canvasToWorld, & norm );
	for(i=0; i<4; ++i){
		DaoxVector3D point = DaoxVector3D_Scale( & points3d[i], frustum.far );
		point = DaoxVector3D_Add( & frustum.cameraPosition, & point );
		point = DaoxPlaneLineIntersect2( origin, norm, point, frustum.viewDirection );
		point = DaoxMatrix4D_Transform( & worldToCanvas, & point );
		points2d[i].x = point.x;
		points2d[i].y = point.y;
		//DaoxVector2D_Print( points2d + i );
	}
	DaoxOBBox2D_ResetBox( & self->obbox, points2d, 4 );
	self->campos = DaoxMatrix4D_Transform( & worldToCanvas, & frustum.cameraPosition );

	glUseProgram( self->shader->program );
	glUniformMatrix4fv( self->shader->uniforms.projMatrix, 1, 0, matrix2 );
	glUniformMatrix4fv( self->shader->uniforms.viewMatrix, 1, 0, matrix3 );
	glUniform1i(self->shader->uniforms.hasDiffuseTexture, 0 );
	glUniform1i(self->shader->uniforms.dashCount, 0 );
	glUniform1i(self->shader->uniforms.gradientType, 2 );
	glUniform1i(self->shader->uniforms.gradientStops, 2 );
	glUniform1f(self->shader->uniforms.gradientRadius, 200 );

	glActiveTexture(GL_TEXTURE0 + DAOX_GRADIENT_SAMPLER);
	glBindTexture(GL_TEXTURE_2D, self->shader->textures.gradientSampler);
	glUniform1i(self->shader->uniforms.gradientSampler, DAOX_GRADIENT_SAMPLER );

	glActiveTexture(GL_TEXTURE0 + DAOX_DASH_SAMPLER);
	glBindTexture(GL_TEXTURE_2D, self->shader->textures.dashSampler);
	glUniform1i(self->shader->uniforms.dashSampler, DAOX_DASH_SAMPLER );


	glBindVertexArray( self->buffer->vertexVAO );
	glBindBuffer( GL_ARRAY_BUFFER, self->buffer->vertexVBO );
	//glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->buffer->triangleVBO );


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef DAO_GRAPHICS_USE_GLES
	if( self->showMesh ){
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	}else{
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}
#endif

	DaoxMatrix3D transform = DaoxMatrix3D_Identity();
	GLfloat modelMatrix[16] = {0};
	DaoxGraphics_TransfromMatrix( transform, modelMatrix );
	glUniformMatrix4fv( self->shader->uniforms.modelMatrix, 1, 0, modelMatrix );

	self->vertexCount = self->triangleCount = 0;
	//printf( "1>> data count: %i %i\n", self->vertexCount, self->triangleCount );
	for(i=0; i<n; i++){
		DaoxCanvasNode *it = canvas->nodes->items.pCanvasNode[i];
		DaoxPainter_UpdateItem( self, canvas, it, canvas->transform );
	}

	//printf( "2>> data count: %i %i\n", self->vertexCount, self->triangleCount );
	self->vertexBuffer = DaoxBuffer_MapVertices( self->buffer, self->vertexCount );
	self->triangleBuffer = DaoxBuffer_MapTriangles( self->buffer, self->triangleCount );

	for(i=0; i<n; i++){
		DaoxCanvasNode *it = canvas->nodes->items.pCanvasNode[i];
		DaoxPainter_BufferItem( self, canvas, it, canvas->transform );
	}

	for(i=0; i<n; i++){
		DaoxCanvasNode *it = canvas->nodes->items.pCanvasNode[i];
		DaoxPainter_PaintItem( self, canvas, it, canvas->transform );
	}

	self->buffer->vertexOffset += self->vertexCount;
	self->buffer->triangleOffset += self->triangleCount;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindVertexArray(0);
}
void DaoxPainter_Paint( DaoxPainter *self, DaoxCanvas *canvas, DaoxAABBox2D viewport )
{
	DaoxCamera camera;
	DaoxColor bgcolor = canvas->background;
	float CX = 0.5*(viewport.left + viewport.right);
	float CY = 0.5*(viewport.top + viewport.bottom);
	float W = viewport.right - viewport.left;
	float H = viewport.top - viewport.bottom;

	memset( & camera, 0, sizeof(DaoxCamera) );
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

	if( bgcolor.alpha >= 1.0/255.0 ){
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor( bgcolor.red, bgcolor.green, bgcolor.blue, bgcolor.alpha );
	}

	DaoxPainter_PaintCanvas( self, canvas, & camera );
}
void DaoxPainter_Render( DaoxPainter *self, DaoxCanvas *canvas, DaoxCamera *camera )
{
}


void DaoxPainter_PaintSubSceneImage( DaoxPainter *self, DaoxCanvas *canvas, DaoxAABBox2D viewport, DaoImage *image, DaoxAABBox2D rect )
{
	DaoxAABBox2D rect2, subViewport = viewport;
	DaoxColor bgcolor = canvas->background;
	int x = rect.left;
	int y = rect.bottom;
	int width = rect.right - rect.left + 1;
	int height = rect.top - rect.bottom + 1;
	int pixelBytes = image->buffer.stride;
	uchar_t *imageData = image->buffer.data.uchars + y * image->stride + x * pixelBytes;
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

	if( width > self->context->deviceWidth ){
		xmoreWin = width - self->context->deviceWidth;
		xmoreScene = xmoreWin * canvasWidth / width;
		destWidth = self->context->deviceWidth;
		subViewport.right = right - xmoreScene;
	}else{
		xwin = (self->context->deviceWidth - width);
		margin = xwin * canvasWidth / width;
		subViewport.right += margin;
	}
	if( height > self->context->deviceHeight ){
		ymoreWin = height - self->context->deviceHeight;
		ymoreScene = ymoreWin * canvasHeight / height;
		destHeight = self->context->deviceHeight;
		subViewport.top = top - ymoreScene;
	}else{
		ywin = (self->context->deviceHeight - height);
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
		rect2.left += self->context->deviceWidth;
		DaoxPainter_PaintSubSceneImage( self, canvas, subViewport, image, rect2 );
	}

	if( ymoreWin ){
		subViewport = viewport;
		subViewport.bottom = canvasTop;
		rect2 = rect;
		rect2.bottom += self->context->deviceHeight;
		DaoxPainter_PaintSubSceneImage( self, canvas, subViewport, image, rect2 );
	}

	if( xmoreWin && ymoreWin ){
		subViewport = viewport;
		subViewport.left = canvasRight;
		subViewport.bottom = canvasTop;
		rect2 = rect;
		rect2.left += self->context->deviceWidth;
		rect2.bottom += self->context->deviceHeight;
		DaoxPainter_PaintSubSceneImage( self, canvas, subViewport, image, rect2 );
	}
}
void DaoxPainter_PaintCanvasImage( DaoxPainter *self, DaoxCanvas *canvas, DaoxAABBox2D viewport, DaoImage *image, int width, int height )
{
	DaoxAABBox2D rect = { 0.0, 0.0, 0.0, 0.0 };

	image->depth = DAOX_IMAGE_BIT32;
	_DaoImage_Resize( image, width, height );

	rect.right = width - 1;
	rect.top = height - 1;
	DaoxPainter_PaintSubSceneImage( self, canvas, viewport, image, rect );
}
