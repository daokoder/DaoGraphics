
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
#include "dao_renderer.h"








DaoxRenderer* DaoxRenderer_New()
{
	float width = 0.005;
	float length = 0.05;
	DaoxMaterial *omat, *xmat, *zmat, *ymat;
	DaoxMeshUnit *origin, *xaxis, *yaxis, *zaxis;
	DaoxRenderer *self = (DaoxRenderer*) dao_calloc( 1, sizeof(DaoxRenderer) );

	DaoCstruct_Init( (DaoCstruct*) self, daox_type_renderer );

	self->targetWidth  = 300;
	self->targetHeight = 200;
	self->visibleModels = DList_New(0);
	self->visibleChunks = DList_New(0);
	self->drawLists = DArray_New(sizeof(DaoxDrawTask));
	self->drawLists2 = DArray_New(sizeof(DaoxDrawTask));
	self->terrains = DList_New( DAO_DATA_VALUE );
	self->canvases = DList_New( DAO_DATA_VALUE );
	self->mapMaterials = DMap_New(0,0);
	self->vertices = DArray_New( sizeof(DaoxVertex) );
	self->triangles = DArray_New( sizeof(DaoxTriangle) );
	self->mapping = DArray_New( sizeof(int) );

	DaoxRenderer_InitShaders( self );
	DaoxRenderer_InitBuffers( self );

	self->axisMesh = DaoxMesh_New();
	self->worldAxis = DaoxModel_New();
	self->localAxis = DaoxModel_New();
	GC_IncRC( self->axisMesh );
	GC_IncRC( self->worldAxis );
	GC_IncRC( self->localAxis );

	origin = DaoxMesh_MakeBoxObject( self->axisMesh );
	xaxis = DaoxMesh_MakeBoxObject( self->axisMesh );
	yaxis = DaoxMesh_MakeBoxObject( self->axisMesh );
	zaxis = DaoxMesh_MakeBoxObject( self->axisMesh );

	omat = DaoxMaterial_New();
	xmat = DaoxMaterial_New();
	ymat = DaoxMaterial_New();
	zmat = DaoxMaterial_New();
	omat->emission = DaoxColor_Darker( & daox_gray_color, 0.5 );
	xmat->emission = DaoxColor_Darker( & daox_red_color, 0.75 );
	ymat->emission = DaoxColor_Darker( & daox_green_color, 0.75 );
	zmat->emission = DaoxColor_Darker( & daox_blue_color, 0.75 );

	DaoxMeshUnit_SetMaterial( origin, omat );
	DaoxMeshUnit_SetMaterial( xaxis, xmat );
	DaoxMeshUnit_SetMaterial( yaxis, ymat );
	DaoxMeshUnit_SetMaterial( zaxis, zmat );

	DaoxMeshUnit_ScaleBy( origin, 2*width, 2*width, 2*width );
	DaoxMeshUnit_ScaleBy( xaxis, length, width, width );
	DaoxMeshUnit_ScaleBy( yaxis, width, length, width );
	DaoxMeshUnit_ScaleBy( zaxis, width, width, length );
	DaoxMeshUnit_MoveBy( xaxis, 0.5*length, 0.0, 0.0 );
	DaoxMeshUnit_MoveBy( yaxis, 0.0, 0.5*length, 0.0 );
	DaoxMeshUnit_MoveBy( zaxis, 0.0, 0.0, 0.5*length );

	DaoxMesh_UpdateTree( self->axisMesh, 0 );
	DaoxModel_SetMesh( self->worldAxis, self->axisMesh );
	DaoxModel_SetMesh( self->localAxis, self->axisMesh );

	return self;
}
void DaoxRenderer_Delete( DaoxRenderer *self )
{
	DaoxShader_Free( & self->shader );
	DaoCstruct_Free( (DaoCstruct*) self );
	DList_Delete( self->terrains );
	DList_Delete( self->canvases );
	DArray_Delete( self->drawLists );
	DArray_Delete( self->drawLists2 );
	DList_Delete( self->visibleChunks );
	DList_Delete( self->visibleModels );
	DMap_Delete( self->mapMaterials );
	DArray_Delete( self->vertices );
	DArray_Delete( self->triangles );
	DArray_Delete( self->mapping );
	GC_DecRC( self->axisMesh );
	GC_DecRC( self->worldAxis );
	GC_DecRC( self->localAxis );
	dao_free( self );
}


void DaoxRenderer_SetCurrentCamera( DaoxRenderer *self, DaoxCamera *camera )
{
	GC_Assign( & self->camera, camera );
}
DaoxCamera* DaoxRenderer_GetCurrentCamera( DaoxRenderer *self )
{
	if( self->camera ) return self->camera;
	self->camera = DaoxCamera_New();
	DaoGC_IncRC( (DaoValue*) self->camera );
	return self->camera;
}

void DaoxRenderer_InitShaders( DaoxRenderer *self )
{
	DaoxShader_Init3D( & self->shader );
	DaoxShader_Finalize3D( & self->shader );
}
void DaoxRenderer_InitBuffers( DaoxRenderer *self )
{
	int pos  = self->shader.attributes.position;
	int norm = self->shader.attributes.normal;
	int texuv  = self->shader.attributes.texCoord;
	int texmo  = self->shader.attributes.texMO;
	DaoxBuffer_Init3D( & self->terrainBuffer, pos, norm, texuv, texmo );
	DaoxBuffer_Init3D( & self->buffer, pos, norm, texuv, texmo );
	DaoxBuffer_Init3DVG( & self->bufferVG, pos, norm, texuv, texmo );
}


void DaoxRenderer_PrepareMeshChunk( DaoxRenderer *self, DaoxModel *model, DaoxMeshChunk *chunk, DaoxMatrix4D *objectToWorld )
{
	DNode *it;
	DList *chunks;
	DaoxMaterial *material;
	DaoxVertex *vertices = chunk->unit->vertices->data.vertices;
	DaoxVector3D normal;
	DaoxOBBox3D obbox;
	daoint i, m, check;
	float angle;

	if( chunk->triangles->size == 0 ) return;

	normal = DaoxMatrix4D_MulVector( objectToWorld, & chunk->normal, 0.0 );
	angle = DaoxVector3D_Angle( & normal, & self->frustum.viewDirection );
	if( (chunk->angle + angle) < 0.5*M_PI ) return; /* Not facing to the camera; */

	obbox = DaoxOBBox3D_Transform( & chunk->obbox, objectToWorld );
	check = DaoxViewFrustum_Visible( & self->frustum, & obbox );
	if( check < 0 ) return;

	if( chunk->left && chunk->left->triangles->size )
		DaoxRenderer_PrepareMeshChunk( self, model, chunk->left, objectToWorld );
	if( chunk->right && chunk->right->triangles->size )
		DaoxRenderer_PrepareMeshChunk( self, model, chunk->right, objectToWorld );

	if( chunk->left && chunk->left->triangles->size ) return;
	if( chunk->right && chunk->right->triangles->size ) return;

	it = DMap_Find( self->mapMaterials, chunk->unit->material );
	m = it ? it->value.pInt : self->mapMaterials->size;
	if( it == NULL ){
		m = self->mapMaterials->size;
		if( m >= self->visibleChunks->size ){
			chunks = DList_New(0);
			DList_Append( self->visibleChunks, chunks );
		}
		chunks = self->visibleChunks->items.pList[m];
		chunks->size = 0;
		MAP_Insert( self->mapMaterials, chunk->unit->material, (daoint)m );
	}
	chunks = self->visibleChunks->items.pList[m];
	DList_Append( chunks, model );
	DList_Append( chunks, chunk );
	self->triangleCount += chunk->triangles->size;
}
void DaoxRenderer_PrepareMesh( DaoxRenderer *self, DaoxModel *model, DaoxMesh *mesh, DaoxMatrix4D *objectToWorld )
{
	DaoxVertex *vertex;
	daoint i;

	for(i=0; i<mesh->units->size; ++i){
		DaoxMeshUnit *unit = mesh->units->items.pMeshUnit[i];
		int currentCount = self->triangleCount;
		if( unit->tree == NULL ) return;
		DaoxRenderer_PrepareMeshChunk( self, model, unit->tree, objectToWorld );
		if( self->triangleCount > currentCount ){
			DArray *vertices = unit->vertices;
			self->vertexCount += vertices->size;
		}
	}
}

void DaoxRenderer_PrepareCanvas( DaoxRenderer *self, DaoxCanvas *canvas )
{
	DList_Append( self->canvases, canvas );
}

void DaoxRenderer_PrepareNode( DaoxRenderer *self, DaoxSceneNode *node )
{
	DaoType *ctype = node->ctype;
	DaoxModel *model = (DaoxModel*) node;
	DaoxMatrix4D objectToWorld;
	DaoxOBBox3D obbox;
	daoint i;

	if( ctype != daox_type_model && ctype != daox_type_canvas ) goto PrepareChildren;

	// 1. Transform view frustum to object coordinates;
	objectToWorld = DaoxSceneNode_GetWorldTransform( node );
	obbox = DaoxOBBox3D_Transform( & node->obbox, & objectToWorld );

	// 2. Check if the obbox box of the object intersect with the frustum;
	if( DaoxViewFrustum_Visible( & self->frustum, & obbox ) < 0 ) return;

	if( ctype == daox_type_canvas ){
		/* The canvas is locally placed on the xy-plane facing z-axis: */
		DaoxVector3D canvasNorm0 = {0.0,0.0,1.0};
		DaoxVector3D canvasNorm = DaoxMatrix4D_MulVector( & objectToWorld, & canvasNorm0, 0.0 );
		double dot = DaoxVector3D_Dot( & canvasNorm, & self->frustum.cameraPosition );
		if( dot < 0.0 ) return;
		DaoxRenderer_PrepareCanvas( self, (DaoxCanvas*) node );
		return;
	}

	DArray_Reset( model->offsets, model->mesh->units->size );
	memset( model->offsets->data.ints, 0, model->mesh->units->size*sizeof(int) );
	DaoxRenderer_PrepareMesh( self, model, model->mesh, & objectToWorld );

PrepareChildren:
	for(i=0; i<node->children->size; ++i){
		DaoxSceneNode *node2 = node->children->items.pSceneNode[i];
		DaoxRenderer_PrepareNode( self, node2 );
	}
}
void MakeProjectionMatrix( DaoxViewFrustum *frustum, DaoxCamera *cam, GLfloat matrix[16] )
{
	memset( matrix, 0, 16*sizeof(GLfloat) );
	matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0;
	matrix[0]  = 2 * cam->nearPlane / (frustum->right - frustum->left);
	matrix[5]  = 2 * cam->nearPlane / (frustum->top - frustum->bottom);
	matrix[8]  = (frustum->right + frustum->left) / (frustum->right - frustum->left);
	matrix[9]  = (frustum->top + frustum->bottom) / (frustum->top - frustum->bottom);
	matrix[10] = -(cam->farPlane + cam->nearPlane) / (cam->farPlane - cam->nearPlane);
	matrix[11] = -1;
	matrix[14] = -(2 * cam->farPlane * cam->nearPlane) / (cam->farPlane - cam->nearPlane);
	matrix[15] = 0;
}
void MakeOrthographicMatrix( DaoxViewFrustum *frustum, DaoxCamera *cam, GLfloat matrix[16] )
{
	memset( matrix, 0, 16*sizeof(GLfloat) );
	matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0;
	matrix[0]  = 2 / (frustum->right - frustum->left);
	matrix[5]  = 2 / (frustum->top - frustum->bottom);
	matrix[10] = -2 / (cam->farPlane - cam->nearPlane);
	matrix[12] = - (frustum->right + frustum->left) / (frustum->right - frustum->left);
	matrix[13] = - (frustum->top + frustum->bottom) / (frustum->top - frustum->bottom);
	matrix[14] = - (cam->farPlane + cam->nearPlane) / (cam->farPlane - cam->nearPlane);
	matrix[15] = 1;
}

void DaoxRenderer_ReduceData( DaoxRenderer *self, DaoxSceneNode *node )
{
	DaoxModel *model = (DaoxModel*) node;
	daoint i;

	if( node->ctype != daox_type_model ) return;
	for(i=0; i<node->children->size; ++i){
		DaoxSceneNode *child = (DaoxSceneNode*) node->children->items.pSceneNode[i];
		DaoxRenderer_ReduceData( self, child );
	}
	if( abs( self->frameIndex - model->viewFrame ) < 1000 ) return;
	if( model->offsets->size == 0 ) return;
	model->offsets->size = 0;
	for(i=0; i<model->positions->size; ++i){
		DArray_Clear( model->positions->items.pArray[i] );
		DArray_Clear( model->normals->items.pArray[i] );
	}
	for(i=0; i<model->tangents->size; ++i){
		DArray_Clear( model->tangents->items.pArray[i] );
	}
}

void DaoxRenderer_UpdateBuffer( DaoxRenderer *self, DaoxVector3D camPos )
{
	int i, j, k, glv, glt;
	int vertexCount = self->vertexCount - self->buffer.vertexOffset;
	int triangleCount = self->triangleCount;
	DaoGLVertex3D *glvertices = DaoxBuffer_MapVertices3D( & self->buffer, vertexCount );
	DaoGLTriangle *gltriangles = DaoxBuffer_MapTriangles( & self->buffer, triangleCount );

	//printf( "DaoxRenderer_UpdateBuffer: %i %i\n", vertexCount, triangleCount );
	//printf( "buffering: %15p %15p\n", glvertices, gltriangles );
	for(i=0,glv=0,glt=0; i<self->visibleChunks->size; ++i){
		DList *chunks = self->visibleChunks->items.pList[i];
		DaoxMaterial *material;
		DaoxMeshChunk *chunk;
		DaoxDrawTask *drawtask;
		int offset = glt;

		if( chunks->size == 0 ) continue;
		chunk = chunks->items.pMeshChunk[1];
		material = chunk->unit->material;
		for(j=0; j<chunks->size; j+=2){
			DaoxModel *model = chunks->items.pModel[j];
			DaoxMeshChunk *chunk = chunks->items.pMeshChunk[j+1];
			DaoxMeshUnit *unit = chunk->unit;
			DArray *positions, *normals, *tangents;
			int *offsets = model->offsets->data.ints;
			int offset2 = glv;

			if( offsets[unit->index] ) offset2 = offsets[unit->index] - 1;

			offset2 += self->buffer.vertexOffset;
			for(k=0; k<chunk->triangles->size; ++k,++glt){
				int m = chunk->triangles->data.ints[k];
				DaoxTriangle *triangle = & unit->triangles->data.triangles[m];
				DaoGLTriangle *triangle2 = gltriangles + glt;
				triangle2->index[0] = triangle->index[0] + offset2;
				triangle2->index[1] = triangle->index[1] + offset2;
				triangle2->index[2] = triangle->index[2] + offset2;
			}

			if( offsets[unit->index] ) continue;
			DList_Append( self->visibleModels, model );
			DaoxModel_TransformMesh( model );
			positions = model->positions->items.pArray[unit->index];
			normals = model->normals->items.pArray[unit->index];
			tangents = model->tangents->items.pArray[unit->index];

			offsets[unit->index] = glv + 1;
			for(k=0; k<positions->size; ++k,++glv){
				DaoxVertex *vertex1 = unit->vertices->data.vertices + k;
				DaoGLVertex3D *vertex2 = glvertices + glv;
				DaoxVector3D point = positions->data.vectors3d[k];
				DaoxVector3D norm = normals->data.vectors3d[k];
				vertex2->pos.x = point.x;
				vertex2->pos.y = point.y;
				vertex2->pos.z = point.z;
				vertex2->norm.x = norm.x;
				vertex2->norm.y = norm.y;
				vertex2->norm.z = norm.z;
				vertex2->tex.x = vertex1->tex.x;
				vertex2->tex.y = vertex1->tex.y;
			}
		}
		drawtask = (DaoxDrawTask*) DArray_Push( self->drawLists );
		drawtask->triangleOffset = offset + self->buffer.triangleOffset;
		drawtask->triangleCount = glt - offset;
		drawtask->material = material;
		drawtask->texture = NULL;
	}
	self->buffer.vertexOffset += vertexCount;
	self->buffer.triangleOffset += triangleCount;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void DaoxVG_PaintItemData( DaoxShader *shader, DaoxBuffer *buffer, DaoxCanvas *canvas, DaoxCanvasNode *item );
void DaoxRenderer_RenderCanvasNode( DaoxRenderer *self, DaoxCanvas *canvas, DaoxCanvasNode *item, DaoxMatrix3D transform )
{
	DaoxOBBox2D obbox;
	DaoxMatrix3D inverse;
	DaoxVector3D itempos = {0.0,0.0,0.0};
	GLfloat modelMatrix[16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
	float distance, diameter;
	float scale = 1;//DaoxCanvas_Scale( canvas );
	float stroke = item->state->strokeWidth / (scale + 1E-16);
	int n = item->children ? item->children->size : 0;
	int k = stroke >= 1.0;
	int m = stroke >= 1E-3;
	int i;

	DaoxCanvasNode_Update( item, canvas );
	DaoxMatrix3D_Multiply( & transform, item->transform );
	obbox = DaoxOBBox2D_Transform( & item->obbox, & transform );
#if 0
	itempos.x = obbox.O.x;
	itempos.y = obbox.O.y;
	distance = DaoxVector3D_Dist( & self->campos, & itempos );
	diameter = DaoxVector3D_Dist( & obbox.X, & obbox.Y );
	//printf( "DaoxPainter_PaintItem 1: %f\n", scale );
	if( diameter < 1E-5 * distance * scale ) return;
	if( DaoxOBBox2D_Intersect( & self->obbox, & obbox ) < 0 ) return;
	//printf( "DaoxPainter_PaintItem 2\n" );
#endif

	glUniformMatrix4fv( self->shader.uniforms.modelMatrix, 1, 0, modelMatrix );
	if( item->visible ){
		DaoxVG_PaintItemData( & self->shader, & self->bufferVG, canvas, item );
		//if( item->ctype == daox_type_canvas_image && item->data.texture )
		//	DaoxPainter_PaintImageItem( self, item );
	}

	for(i=0; i<n; i++){
		DaoxCanvasNode *it = item->children->items.pCanvasNode[i];
		DaoxRenderer_RenderCanvasNode( self, canvas, it, transform );
	}
}
void DaoxRenderer_RenderCanvas( DaoxRenderer *self, DaoxCanvas *canvas )
{
	int i, n = canvas->nodes->size;
	glUniform1i(self->shader.uniforms.vectorGraphics, 1 );
	for(i=0; i<n; i++){
		DaoxCanvasNode *it = canvas->nodes->items.pCanvasNode[i];
		DaoxRenderer_RenderCanvasNode( self, canvas, it, canvas->transform );
	}
	glUniform1i(self->shader.uniforms.vectorGraphics, 0 );
}
void DaoxScene_EstimateBoundingBox( DaoxScene *self, DaoxOBBox3D *obbox )
{
	DArray *points = DArray_New( sizeof(DaoxVector3D) );
	daoint i, j, k;
	for(i=0; i<self->nodes->size; ++i){
		DaoxSceneNode *node = self->nodes->items.pSceneNode[i];
		DaoxOBBox3D obbox = DaoxOBBox3D_Transform( & node->obbox, & node->transform );
		DaoxVector3D P = DaoxOBBox3D_GetDiagonalVertex( & obbox );
		if( node->ctype == daox_type_light ) continue;
		if( node->ctype == daox_type_camera ) continue;
		DArray_PushVector3D( points, & obbox.O );
		DArray_PushVector3D( points, & obbox.X );
		DArray_PushVector3D( points, & obbox.Y );
		DArray_PushVector3D( points, & obbox.Z );
		DArray_PushVector3D( points, & P );
	}
	DaoxOBBox3D_ComputeBoundingBox( obbox, points->data.vectors3d, points->size );
	DArray_Delete( points );
}
void DaoxRenderer_UpdateTerrainBuffer( DaoxRenderer *self )
{
	DaoGLVertex3D *glvertices;
	DaoGLTriangle *gltriangles;
	DaoxDrawTask *drawtask;
	int vertexCount = 0;
	int triangleCount = 0;
	int i, j;

	for(i=0; i<self->terrains->size; ++i){
		DaoxTerrain *terrain = self->terrains->items.pTerrain[i];
		vertexCount += terrain->vertices->size;
		triangleCount += terrain->triangles->size;
	}
	printf( ">> %i %i\n", vertexCount, triangleCount );
	glvertices = DaoxBuffer_MapVertices3D( & self->terrainBuffer, vertexCount );
	gltriangles = DaoxBuffer_MapTriangles( & self->terrainBuffer, triangleCount );
	vertexCount = triangleCount = 0;

	for(i=0; i<self->terrains->size; ++i){
		DaoxTerrain *terrain = self->terrains->items.pTerrain[i];
		DaoxColor dark = {0.1, 0.1, 0.1, 1.0};
		int M = 3*terrain->triangles->size;
		int K = triangleCount;
		int vboffset = self->terrainBuffer.vertexOffset;
		printf( "%i %i\n", vertexCount, triangleCount );
		for(j=0; j<terrain->vertices->size; ++j){
			DaoxTerrainPoint *point = (DaoxTerrainPoint*) terrain->vertices->items.pVoid[j];
			DaoGLVertex3D *glvertex = glvertices + vertexCount + j;
			glvertex->pos.x = point->pos.x;
			glvertex->pos.y = point->pos.y;
			glvertex->pos.z = point->pos.z;
			glvertex->norm.x = point->norm.x;
			glvertex->norm.y = point->norm.y;
			glvertex->norm.z = point->norm.z;
			glvertex->tex.x = point->pos.x / terrain->width + 0.5;
			glvertex->tex.y = point->pos.y / terrain->length + 0.5;
		}
		for(j=0; j<terrain->triangles->size; ++j){
			DaoxTriangle *triangle = & terrain->triangles->data.triangles[j];
			DaoGLTriangle *gltriangle = gltriangles + triangleCount + j;
			gltriangle->index[0] = triangle->index[0] + vertexCount + vboffset;
			gltriangle->index[1] = triangle->index[1] + vertexCount + vboffset;
			gltriangle->index[2] = triangle->index[2] + vertexCount + vboffset;
		}
		//printf( "%3i %6i %6i\n", i, data[0], data[1] );
		drawtask = (DaoxDrawTask*) DArray_Push( self->drawLists2 );
		drawtask->triangleOffset = self->terrainBuffer.triangleOffset + triangleCount;
		drawtask->triangleCount = terrain->triangles->size;
		drawtask->texture = terrain->texture;
		drawtask->material = NULL;

		vertexCount += terrain->vertices->size;
		triangleCount += terrain->triangles->size;
	}
	self->terrainBuffer.vertexOffset += vertexCount;
	self->terrainBuffer.triangleOffset += triangleCount;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void DaoxRenderer_DrawTask( DaoxRenderer *self, DaoxDrawTask *drawtask )
{
	DaoxColor dark = {0.1, 0.1, 0.1, 1.0};
	DaoxTexture *colorTexture = drawtask->texture;
	DaoxTexture *bumpTexture = NULL;
	DaoxMaterial *material = drawtask->material;
	DaoxColor ambient = material ? material->ambient : dark;
	DaoxColor diffuse = material ? material->diffuse : dark;
	DaoxColor specular = material ? material->specular : dark;
	DaoxColor emission = material ? material->emission : daox_black_color;
	daoint K = 3 * drawtask->triangleOffset * sizeof(GLint);
	daoint M = 3 * drawtask->triangleCount;
	int hasColorTexture = 0;
	int hasBumpTexture = 0;

	//printf( "%3i %6i %6i\n", i, data[0], data[1] );
	glUniform4fv( self->shader.uniforms.ambientColor, 1, & ambient.red );
	glUniform4fv( self->shader.uniforms.diffuseColor, 1, & diffuse.red );
	glUniform4fv( self->shader.uniforms.specularColor, 1, & specular.red );
	glUniform4fv( self->shader.uniforms.emissionColor, 1, & emission.red );

	if( colorTexture == NULL && material != NULL ) colorTexture = material->texture1;
	if( colorTexture ){
		DaoxTexture_glInitTexture( colorTexture );
		if( colorTexture->tid ){
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, colorTexture->tid);
			glUniform1i(self->shader.uniforms.colorTexture, 0);
			hasColorTexture = 1;
		}
	}
	if( bumpTexture == NULL && material != NULL ) bumpTexture = material->texture2;
	if( bumpTexture ){
		DaoxTexture_glInitTexture( bumpTexture );
		if( bumpTexture->tid ){
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, bumpTexture->tid);
			glUniform1i(self->shader.uniforms.bumpTexture, 0);
			hasBumpTexture = 1;
		}
	}
	glUniform1i(self->shader.uniforms.hasColorTexture, hasColorTexture );
	glUniform1i(self->shader.uniforms.hasBumpTexture, hasBumpTexture );
	glDrawRangeElements( GL_TRIANGLES, 0, M, M, GL_UNSIGNED_INT, (void*)K );
	/* TODO: better hint for glDrawRangeElements(); */
}
void DaoxRenderer_Render( DaoxRenderer *self, DaoxScene *scene, DaoxCamera *cam )
{
	DaoxViewFrustum fm;
	DaoxMatrix4D rotation;
	DaoxMatrix4D viewMatrix;
	DaoxMatrix4D objectToWorld;
	DaoxVector3D cameraPosition;
	DaoxVector3D lightSource[32];
	DaoxColor lightIntensity[32];
	DaoxVector3D zaxis = {0.0,0.0,1.0};
	GLfloat matrix[16] = {0};
	GLfloat matrix2[16] = {0};
	GLfloat matrix3[9] = {0};
	daoint i, j, lightCount = scene->lights->size;
	float cosine, sine;

	if( scene->nodes->size == 0 ) return;
	if( scene != self->scene ){
		GC_Assign( & self->scene, scene );
	}
	if( cam == NULL && self->camera == NULL ){
		self->camera = DaoxCamera_New();
		DaoGC_IncRC( (DaoValue*) self->camera );

	}
	if( cam && cam != self->camera ){
		GC_Assign( & self->camera, cam );
	}
	cam = self->camera;
	cam->aspectRatio = self->targetWidth / (float) self->targetHeight;

	//printf( "DaoxRenderer_Render: %i %i %i %i\n", scene->nodes->size, self->drawLists->size, self->buffer.vertexOffset, self->buffer.triangleOffset );

	//DaoxMatrix4D_Print( & rotation );
	int error = glGetError();

#if 0
	static float angle = 0.0;
	angle += 0.001;
	//printf( "angle = %f\n", angle );
	rotation = DaoxMatrix4D_AxisRotation( zaxis, 0.001 );
	cam->base.objectToWorld = DaoxMatrix4D_MulMatrix( & cam->base.objectToWorld, & rotation );
	cam->base.objectToParent = DaoxMatrix4D_MulMatrix( & cam->base.objectToParent, & rotation );
	//DaoxMatrix4D_Print( & cam->objectToWorld );
#endif

#if 0
	DaoxOBBox3D sceneObbox;
	DaoxScene_EstimateBoundingBox( scene, & sceneObbox );
	sceneObbox = DaoxOBBox3D_Scale( & sceneObbox, 1.2 );
	sceneObbox = DaoxOBBox3D_ToAABox( & sceneObbox );
	cam->farPlane = 10000 + 2*sceneObbox.R;

	DaoxVector3D P = DaoxOBBox3D_GetDiagonalVertex( & sceneObbox );
	DaoxVector3D_Print( & cam->viewTarget );
	DaoxCamera_LookAt( cam, sceneObbox.C );
	DaoxCamera_Move( cam, P );
	//DaoxCamera_RotateBy( cam, 0.005 );
	//DaoxCamera_AdjustToHorizon( cam );
	//DaoxCamera_Move( cam, sceneObbox.C );
	//DaoxCamera_LookAt( cam, sceneObbox.O );
	DaoxVector3D_Print( & sceneObbox.O );
	DaoxVector3D_Print( & sceneObbox.C );
	printf( "R = %f, %i\n", sceneObbox.R, lightCount );
	//DaoxCamera_MoveXYZ( cam, rand()%300, 52+rand()%300, rand()%300 );
	//DaoxCamera_LookAtXYZ( cam, 0, 50, 0 );
#endif


	objectToWorld = DaoxSceneNode_GetWorldTransform( & cam->base );
	viewMatrix = DaoxMatrix4D_Inverse( & objectToWorld );
	//DaoxMatrix4D_Print( & cam->base.transform );
	//DaoxMatrix4D_Print( & objectToWorld );

	DaoxViewFrustum_Init( & fm, cam );
	fm.ratio = self->targetWidth / (fm.right - fm.left);

	DaoxSceneNode_Move( (DaoxSceneNode*) self->worldAxis, fm.axisOrigin );

	if( DaoxViewFrustum_Difference( & fm, & self->frustum ) > EPSILON ){ // TODO: better handling;
		DArray *triangles;
		self->drawLists->size = 0;
		self->drawLists2->size = 0;
		self->triangleCount = 0;
		self->vertexCount = self->buffer.vertexOffset;
		self->visibleModels->size = 0;
		self->frustum = fm;
		//printf( "prepare\n" );
		DMap_Clear( self->mapMaterials );
		DList_Clear( self->canvases );
		DList_Clear( self->terrains );
		for(i=0; i<scene->nodes->size; ++i){
			DaoxSceneNode *node = scene->nodes->items.pSceneNode[i];
			DaoxRenderer_PrepareNode( self, node );
			if( node->ctype == daox_type_terrain ){
				DaoxTerrain_UpdateView( (DaoxTerrain*) node, & self->frustum );
				DList_Append( self->terrains, (DaoxTerrain*) node );
			}
		}
		DaoxRenderer_UpdateTerrainBuffer( self );
		if( self->showAxis ) DaoxRenderer_PrepareNode( self, (DaoxSceneNode*) self->worldAxis );
		if( self->vertexCount > self->buffer.vertexOffset ){
			DaoxRenderer_UpdateBuffer( self, fm.cameraPosition );
		}
	}
	self->frustum = fm;

	cameraPosition = DaoxSceneNode_GetWorldPosition( (DaoxSceneNode*) cam );
	//viewMatrix = DaoxMatrix4D_Identity();
	DaoxMatrix4D_Export( & viewMatrix, matrix );

	MakeProjectionMatrix( & fm, cam, matrix2 );

	if( lightCount > 32 ) lightCount = 32;
	for(i=0; i<lightCount; ++i){
		DaoxLight *light = self->scene->lights->items.pLight[i];
		lightSource[i] = DaoxSceneNode_GetWorldPosition( (DaoxSceneNode*) light );
		lightIntensity[i] = light->intensity;
	}
	if( lightCount == 0 ){
		lightCount = 1;
		lightSource[0].x = 0;
		lightSource[0].y = 100;
		lightSource[0].z = 1000;
		lightIntensity[0].red = lightIntensity[0].green = lightIntensity[0].blue = 1.0;
		lightIntensity[0].alpha = 1.0;
	}
#if 0
#endif


	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
#ifdef GL_LIGHTING
	glEnable(GL_LIGHTING);
#endif
	glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE); /* Not effective for canvas? */

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.5, 0.5, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	glUseProgram( self->shader.program );

	glUniform1i(self->shader.uniforms.vectorGraphics, 0 );
	glUniform1i(self->shader.uniforms.hasColorTexture, 0 );
	glUniform1i(self->shader.uniforms.hasBumpTexture, 0 );
	glUniform1i(self->shader.uniforms.dashCount, 0 );
	glUniform1i(self->shader.uniforms.gradientType, 0 );
	glUniform1i(self->shader.uniforms.gradientStops, 0 );
	glUniform1f(self->shader.uniforms.gradientRadius, 0 );

	glActiveTexture(GL_TEXTURE0 + DAOX_GRADIENT_SAMPLER);
	glBindTexture(GL_TEXTURE_1D, self->shader.textures.gradientSampler);
	glUniform1i(self->shader.uniforms.gradientSampler, DAOX_GRADIENT_SAMPLER );

	glActiveTexture(GL_TEXTURE0 + DAOX_DASH_SAMPLER);
	glBindTexture(GL_TEXTURE_1D, self->shader.textures.dashSampler);
	glUniform1i(self->shader.uniforms.dashSampler, DAOX_DASH_SAMPLER );

	glUniformMatrix4fv( self->shader.uniforms.projMatrix, 1, 0, matrix2 );
	glUniformMatrix4fv( self->shader.uniforms.viewMatrix, 1, 0, matrix );
	glUniform3fv(self->shader.uniforms.cameraPosition, 1, & cameraPosition.x );
	glUniform1i(self->shader.uniforms.lightCount, lightCount );
	glUniform3fv(self->shader.uniforms.lightSource, lightCount, & lightSource[0].x );
	glUniform4fv(self->shader.uniforms.lightIntensity, lightCount, & lightIntensity[0].red );


	glBindVertexArray( self->terrainBuffer.vertexVAO );
	glBindBuffer( GL_ARRAY_BUFFER, self->terrainBuffer.vertexVBO );
	//glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->terrainBuffer.triangleVBO );

	for(i=0; i<self->drawLists2->size; ++i){
		DaoxRenderer_DrawTask( self, self->drawLists2->data.drawtasks + i );
	}
	glBindVertexArray(0);


	glBindVertexArray( self->buffer.vertexVAO );
	glBindBuffer( GL_ARRAY_BUFFER, self->buffer.vertexVBO );
	//glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->buffer.triangleVBO );

	for(i=0; i<self->drawLists->size; ++i){
		DaoxRenderer_DrawTask( self, self->drawLists->data.drawtasks + i );
	}
	glBindVertexArray(0);


	glDepthMask(GL_FALSE);
	glBindVertexArray( self->bufferVG.vertexVAO );
	glBindBuffer( GL_ARRAY_BUFFER, self->bufferVG.vertexVBO );
	//glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->bufferVG.triangleVBO );
	for(i=0; i<self->canvases->size; ++i){
		DaoxCanvas *canvas = self->canvases->items.pCanvas[i];
		DaoxRenderer_RenderCanvas( self, canvas );
	}
	glBindVertexArray(0);


	self->frameIndex += 1;
	for(i=0; i<self->visibleModels->size; ++i){
		DaoxModel *model = self->visibleModels->items.pModel[i];
		model->viewFrame = self->frameIndex;
	}
	for(i=0; i<scene->nodes->size; ++i){
		DaoxSceneNode *node = scene->nodes->items.pSceneNode[i];
		DaoxRenderer_ReduceData( self, node );
	}
}



