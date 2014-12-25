
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
#include "dao_terrain.h"
#include "dao_particle.h"
#include "dao_renderer.h"
#include "dao_painter.h"




DaoxDrawTask* DaoxDrawTask_New()
{
	DaoxDrawTask *self = (DaoxDrawTask*) dao_calloc( 1, sizeof(DaoxDrawTask) );
	return self;
}
void DaoxDrawTask_Delete( DaoxDrawTask *self )
{
	DList_Clear( & self->units );
	DList_Clear( & self->chunks );
	dao_free( self );
}





DaoxRenderer* DaoxRenderer_New( DaoxContext *ctx )
{
	float width = 0.005;
	float length = 0.05;
	DaoxMaterial *omat, *xmat, *zmat, *ymat;
	DaoxMeshUnit *origin, *xaxis, *yaxis, *zaxis;
	DaoxRenderer *self = (DaoxRenderer*) dao_calloc( 1, sizeof(DaoxRenderer) );

	DaoCstruct_Init( (DaoCstruct*) self, daox_type_renderer );

	if( ctx == NULL ) ctx = DaoxContext_New();
	GC_Assign( & self->context, ctx );

	self->tasks = DList_New(0);
	self->tasks2 = DList_New(0);
	self->taskCache = DList_New(0);
	self->canvases = DList_New(0);
	self->map = DMap_New(0,0);

	self->shader = DaoxShader_New( ctx );
	self->buffer = DaoxBuffer_New( ctx );
	self->bufferSK = DaoxBuffer_New( ctx );
	self->bufferVG = DaoxBuffer_New( ctx );
	GC_IncRC( self->shader );
	GC_IncRC( self->buffer );
	GC_IncRC( self->bufferSK );
	GC_IncRC( self->bufferVG );

	DaoxRenderer_InitShaders( self );
	DaoxRenderer_InitBuffers( self );
	self->axisMesh = DaoxMesh_New();
	self->worldAxis = DaoxModel_New();
	self->localAxis = DaoxModel_New();
	GC_IncRC( self->axisMesh );
	GC_IncRC( self->worldAxis );
	GC_IncRC( self->localAxis );

	origin = DaoxMesh_MakeBox( self->axisMesh, 2*width, 2*width, 2*width );
	xaxis = DaoxMesh_MakeBox( self->axisMesh, length, width, width );
	yaxis = DaoxMesh_MakeBox( self->axisMesh, width, length, width );
	zaxis = DaoxMesh_MakeBox( self->axisMesh, width, width, length );

	DaoxMeshUnit_MoveBy( xaxis, 0.5*length, 0.0, 0.0 );
	DaoxMeshUnit_MoveBy( yaxis, 0.0, 0.5*length, 0.0 );
	DaoxMeshUnit_MoveBy( zaxis, 0.0, 0.0, 0.5*length );

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

	DaoxMesh_UpdateTree( self->axisMesh, 0 );
	DaoxModel_SetMesh( self->worldAxis, self->axisMesh );
	DaoxModel_SetMesh( self->localAxis, self->axisMesh );

	return self;
}
void DaoxRenderer_ClearDrawTasks( DaoxRenderer *self, DList *tasks )
{
	int i;
	for(i=0; i<tasks->size; ++i){
		DaoxDrawTask *task = tasks->items.pDrawTask[i];
		DList_PushBack( self->taskCache, task );
	}
	tasks->size = 0;
}
void DaoxRenderer_Delete( DaoxRenderer *self )
{
	int i;

	DaoxRenderer_ClearDrawTasks( self, self->tasks );
	DaoxRenderer_ClearDrawTasks( self, self->tasks2 );

	for(i=0; i<self->taskCache->size; ++i){
		DaoxDrawTask *task = self->taskCache->items.pDrawTask[i];
		DaoxDrawTask_Delete( task );
	}
	if( self->scene ) GC_DecRC( self->scene );
	if( self->camera ) GC_DecRC( self->camera );
	GC_DecRC( self->shader );
	GC_DecRC( self->buffer );
	GC_DecRC( self->bufferVG );
	GC_DecRC( self->bufferSK );
	GC_DecRC( self->context );
	DList_Delete( self->taskCache );
	DList_Delete( self->tasks );
	DList_Delete( self->tasks2 );
	DList_Delete( self->canvases );
	DMap_Delete( self->map );
	GC_DecRC( self->axisMesh );
	GC_DecRC( self->worldAxis );
	GC_DecRC( self->localAxis );
	DaoCstruct_Free( (DaoCstruct*) self );
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
	DaoxShader_Init3D( self->shader );
	DaoxContext_BindShader( self->context, self->shader );
}
void DaoxRenderer_InitBuffers( DaoxRenderer *self )
{
	int pos  = self->shader->attributes.position;
	int norm = self->shader->attributes.normal;
	int tan = self->shader->attributes.tangent;
	int texuv  = self->shader->attributes.texCoord;
	int texmo  = self->shader->attributes.texMO;
	int joints  = self->shader->attributes.joints;
	int weights = self->shader->attributes.weights;
	DaoxBuffer_Init3D( self->buffer, pos, norm, tan, texuv );
	DaoxBuffer_Init3DVG( self->bufferVG, pos, norm, texuv, texmo );
	DaoxBuffer_Init3DSK( self->bufferSK, pos, norm, tan, texuv, joints, weights );
	DaoxContext_BindBuffer( self->context, self->buffer );
	DaoxContext_BindBuffer( self->context, self->bufferVG );
	DaoxContext_BindBuffer( self->context, self->bufferSK );
}

DaoxDrawTask* DaoxRenderer_MakeDrawTask( DaoxRenderer *self )
{
	DaoxDrawTask *task = NULL;
	if( self->taskCache->size ){
		task = (DaoxDrawTask*) DList_PopBack( self->taskCache );
	}else{
		task = DaoxDrawTask_New();
	}
	task->shape = 0;
	task->tcount = 0;
	task->vcount = 0;
	task->terrainTileType = 0;
	task->particleType = 0;
	task->units.size = 0;
	task->chunks.size = 0;
	task->material = NULL;
	task->hexTile = NULL;
	task->hexTerrain = NULL;
	return task;
}

void DaoxRenderer_PrepareMeshChunk( DaoxRenderer *self, DaoxMeshChunk *chunk, DaoxDrawTask *task )
{
	DaoxOBBox3D obbox;
	daoint i, m, check;

	if( chunk->triangles->size == 0 ) return;

	obbox = DaoxOBBox3D_Transform( & chunk->obbox, & task->matrix );
	if( task->skeleton != NULL ){
		obbox = DaoxOBBox3D_Scale( & obbox, 8.0 );
	}
	check = DaoxViewFrustum_Visible( & self->frustum, & obbox );
	if( check < 0 ) return;

	if( chunk->left && chunk->left->triangles->size ){
		DaoxRenderer_PrepareMeshChunk( self, chunk->left, task );
	}
	if( chunk->right && chunk->right->triangles->size ){
		DaoxRenderer_PrepareMeshChunk( self, chunk->right, task );
	}

	if( chunk->left && chunk->left->triangles->size ) return;
	if( chunk->right && chunk->right->triangles->size ) return;

	task->tcount += chunk->triangles->size;
	DList_Append( & task->chunks, chunk );
}
void DaoxRenderer_PrepareModel( DaoxRenderer *self, DaoxModel *model, DaoxMatrix4D *objectToWorld )
{
	DaoxMesh *mesh = model->mesh;
	DaoxDrawTask *task = NULL;
	DaoxVertex *vertex;
	DNode *it;
	daoint i;

	//printf( "DaoxRenderer_PrepareModel:\n" );
	DMap_Reset( self->map );
	for(i=0; i<mesh->units->size; ++i){
		DaoxMeshUnit *unit = mesh->units->items.pMeshUnit[i];
		int currentCount = 0;
		if( unit->tree == NULL ) continue;
		it = DMap_Find( self->map, unit->material );
		if( it ){
			task = (DaoxDrawTask*) it->value.pVoid;
			task->skeleton = model->skeleton;
			currentCount = task->tcount;
		}else{
			task = DaoxRenderer_MakeDrawTask( self );
			task->matrix = *objectToWorld;
			task->material = unit->material;
			task->skeleton = model->skeleton;
			task->particleType = 0;
			DList_Append( model->skeleton ? self->tasks2 : self->tasks, task );
			DMap_Insert( self->map, task->material, task );
			if( DaoType_ChildOf( model->base.ctype, daox_type_emitter ) ){
				task->particleType = 1;
			}
		}
		DaoxRenderer_PrepareMeshChunk( self, unit->tree, task );
		if( task->tcount > currentCount ){
			DList_Append( & task->units, unit );
			task->vcount += unit->vertices->size;
		}
	}
	if( model->skeleton ) DaoxSkeleton_UpdateSkinningMatrices( model->skeleton );
}
void DaoxRenderer_PrepareTerrain( DaoxRenderer *self, DaoxTerrain *terrain, DaoxMatrix4D *objectToWorld )
{
	DaoxMesh *mesh = terrain->mesh;
	DaoxDrawTask *task = NULL;
	DaoxTerrainBlock *tile;
	DaoxVertex *vertex;
	DNode *it;
	daoint i;

	//printf( "DaoxRenderer_PrepareModel:\n" );
	for(tile=terrain->first; tile!=terrain->last->next; tile=tile->next){
		DaoxMeshUnit *unit = tile->mesh;
		int currentCount = 0;
		if( unit->tree == NULL ) continue;

		task = DaoxRenderer_MakeDrawTask( self );
		task->matrix = *objectToWorld;
		task->material = unit->material;
		DList_Append( self->tasks, task );
		task->terrainTileType = terrain->shape + 1;
		task->hexTile = tile;
		task->hexTerrain = terrain;

		DaoxRenderer_PrepareMeshChunk( self, unit->tree, task );

		if( task->tcount > currentCount ){
			DList_Append( & task->units, unit );
			task->vcount += unit->vertices->size;
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

	if( node->renderable == 0 ) goto PrepareChildren;

	objectToWorld = DaoxSceneNode_GetWorldTransform( node );
	obbox = DaoxOBBox3D_Transform( & node->obbox, & objectToWorld );

	if( ctype == daox_type_model && model->skeleton != NULL ){
		obbox = DaoxOBBox3D_Scale( & obbox, 8.0 );
	}

	// Check if the obbox box of the object intersect with the frustum;
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

	if( DaoType_ChildOf( node->ctype, daox_type_emitter ) ){
		DaoxEmitter *emitter = (DaoxEmitter*) node;
		DaoxMatrix4D worldToObj = DaoxMatrix4D_Inverse( & objectToWorld );
		DaoxVector3D campos;
		campos = DaoxMatrix4D_Transform( & worldToObj, & self->frustum.cameraPosition);
		DaoxEmitter_UpdateView( emitter, campos );
	}

	if( ctype == daox_type_terrain ){
		DaoxRenderer_PrepareTerrain( self, (DaoxTerrain*) node, & objectToWorld );
	}else{
		DaoxRenderer_PrepareModel( self, model, & objectToWorld );
	}

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


void DaoxScene_EstimateBoundingBox( DaoxScene *self, DaoxOBBox3D *obbox )
{
	DArray *points = DArray_New( sizeof(DaoxVector3D) );
	daoint i, j, k;
	for(i=0; i<self->nodes->size; ++i){
		DaoxSceneNode *node = self->nodes->items.pSceneNode[i];
		DaoxMatrix4D transform = DaoxSceneNode_GetWorldTransform( node );
		DaoxOBBox3D obbox = DaoxOBBox3D_Transform( & node->obbox, & transform );
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
void DaoxRenderer_UpdateBuffer( DaoxRenderer *self, DList *drawtasks, DaoxBuffer *buffer )
{
	int i, j, k, vertexCount = 0, triangleCount = 0;
	DaoGLVertex3D *glvertices = NULL;
	DaoGLSkinVertex3D *glskvertices = NULL;
	DaoGLTriangle *gltriangles;

	for(i=0; i<drawtasks->size; ++i){
		DaoxDrawTask *drawtask = drawtasks->items.pDrawTask[i];
		vertexCount += drawtask->vcount;
		triangleCount += drawtask->tcount;
	}
	glvertices = DaoxBuffer_MapVertices3D( buffer, vertexCount );
	gltriangles = DaoxBuffer_MapTriangles( buffer, triangleCount );
	if( buffer == self->bufferSK ){
		glskvertices = (DaoGLSkinVertex3D*) glvertices;
		glvertices = NULL;
	}

#ifdef DEBUG
	printf( "DaoxRenderer_UpdateBuffer: %i %i, %p %p\n", vertexCount, triangleCount, glvertices, gltriangles );
#endif

	vertexCount = 0;
	triangleCount = 0;
	for(i=0; i<drawtasks->size; ++i){
		DaoxDrawTask *drawtask = drawtasks->items.pDrawTask[i];
		DList *chunks = & drawtask->chunks;
		DList *units = & drawtask->units;
		int triangleOffset = triangleCount;

		if( chunks->size == 0 ) continue;
		DMap_Reset( self->map );
		for(j=0; j<units->size; ++j){
			DaoxMeshUnit *unit = units->items.pMeshUnit[j];
			int s, vertexOffset = buffer->vertexOffset + vertexCount;
			DMap_Insert( self->map, unit, (void*)(size_t) vertexOffset );
			for(k=0; k<unit->vertices->size; ++k){
				DaoxVertex *vertex = unit->vertices->data.vertices + k;
				DaoGLSkinVertex3D *skvertex = glskvertices + vertexCount + k;
				DaoGLVertex3D *glvertex = glvertices + vertexCount + k;
				if( glvertices == NULL ){
					DaoxSkinParam *param = unit->skinParams->data.skinparams + k;
					glvertex = (DaoGLVertex3D*) skvertex;
					for(s=0; s<4; ++s){
						skvertex->joints.j[s] = param->joints[s];
						skvertex->weights.w[s] = param->weights[s];
					}
				}
				glvertex->pos.x = vertex->pos.x;
				glvertex->pos.y = vertex->pos.y;
				glvertex->pos.z = vertex->pos.z;
				glvertex->norm.x = vertex->norm.x;
				glvertex->norm.y = vertex->norm.y;
				glvertex->norm.z = vertex->norm.z;
				glvertex->tan.x = vertex->tan.x;
				glvertex->tan.y = vertex->tan.y;
				glvertex->tan.z = vertex->tan.z;
				glvertex->tex.x = vertex->tex.x;
				glvertex->tex.y = vertex->tex.y;
			}
			vertexCount += unit->vertices->size;
		}
		for(j=0; j<chunks->size; ++j){
			DaoxMeshChunk *chunk = chunks->items.pMeshChunk[j];
			DaoxMeshUnit *unit = chunk->unit;
			DNode *it = DMap_Find( self->map, unit );
			int vertexOffset = it->value.pInt;
			for(k=0; k<chunk->triangles->size; ++k){
				int m = chunk->triangles->data.ints[k];
				DaoxTriangle *triangle = & unit->triangles->data.triangles[m];
				DaoGLTriangle *gltriangle = gltriangles + triangleCount + k;
				gltriangle->index[0] = triangle->index[0] + vertexOffset;
				gltriangle->index[1] = triangle->index[1] + vertexOffset;
				gltriangle->index[2] = triangle->index[2] + vertexOffset;
			}
			triangleCount += chunk->triangles->size;
		}
		drawtask->shape = GL_TRIANGLES;
		drawtask->offset = buffer->triangleOffset + triangleOffset;
	}

	//printf( "DaoxRenderer_UpdateBuffer: %i %i\n", vertexCount, triangleCount );
	//printf( "buffering: %15p %15p\n", glvertices, gltriangles );
	buffer->vertexOffset += vertexCount;
	buffer->triangleOffset += triangleCount;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
int DaoxRenderer_SetupTexture( DaoxRenderer *self, DaoxTexture *texture, int id, int uniform )
{
	if( texture->changed || texture->tid == 0 ){
		DaoxContext_BindTexture( self->context, texture );
	}
	if( texture->tid ){
		glActiveTexture( GL_TEXTURE0 + id );
		glBindTexture( GL_TEXTURE_2D, texture->tid );
		glUniform1i( uniform, id );
		return 1;
	}
	return 0;
}
void DaoxRenderer_DrawTask( DaoxRenderer *self, DaoxDrawTask *drawtask )
{
	DaoxColor dark = {0.2, 0.2, 0.2, 1.0};
	DaoxTexture *bumpTexture = NULL;
	DaoxTexture *diffuseTexture = NULL;
	DaoxTexture *emissionTexture = NULL;
	DaoxMaterial *material = drawtask->material;
	DaoxColor ambient = material ? material->ambient : dark;
	DaoxColor diffuse = material ? material->diffuse : dark;
	DaoxColor specular = material ? material->specular : dark;
	DaoxColor emission = material ? material->emission : daox_black_color;
	daoint K = drawtask->offset * sizeof(GLint);
	daoint M = drawtask->tcount;
	GLfloat matrix[16] = {0};
	int terrainTileType = 0;
	int tileTextureCount = 0;
	int tileTextureScale = 0;
	int hasDiffuseTexture = 0;
	int hasEmissionTexture = 0;
	int hasBumpTexture = 0;
	int i, j, k;

	if( drawtask->shape == GL_TRIANGLES ){
		K *= 3;
		M *= 3;
	}

	DaoxMatrix4D_Export( & drawtask->matrix, matrix );
	glUniformMatrix4fv( self->shader->uniforms.modelMatrix, 1, 0, matrix );

	glUniform4fv( self->shader->uniforms.ambientColor, 1, & ambient.red );
	glUniform4fv( self->shader->uniforms.diffuseColor, 1, & diffuse.red );
	glUniform4fv( self->shader->uniforms.specularColor, 1, & specular.red );
	glUniform4fv( self->shader->uniforms.emissionColor, 1, & emission.red );
	glUniform1f( self->shader->uniforms.shininess, material ? material->shininess : 2 );

	if( drawtask->skeleton ){
		GLfloat matRows[128][16];
		k = drawtask->skeleton->skinMats2->size;
		if( k > 128 ) k = 128;
		for(i=0; i<k; ++i){
			DaoxMatrix4D *mat = drawtask->skeleton->skinMats2->data.matrices4d + i;
			DaoxMatrix4D_Export( mat, matRows[i] );
		}
		glUniformMatrix4fv( self->shader->uniforms.skinMatRows, k, 0, matRows[0] );
	}
	glUniform1i( self->shader->uniforms.skinning, drawtask->skeleton != NULL );

	if( drawtask->hexTile && drawtask->hexTile->mesh->material && drawtask->hexTile->mesh->material->diffuseTexture ){
		DaoxMaterial *material = drawtask->hexTile->mesh->material;
		terrainTileType = drawtask->terrainTileType;
		tileTextureCount = 7;
		tileTextureScale =  drawtask->hexTerrain->textureScale;
		for(i=0; i<drawtask->hexTile->sides; ++i){
			DaoxTerrainBlock *neighbor = drawtask->hexTile->neighbors[i];
			DaoxMaterial *material2 = neighbor ? neighbor->mesh->material : material;
			if( material2 == NULL ) material2 = material;
			if( material->diffuseTexture->changed || material->diffuseTexture->tid == 0 ){
				DaoxContext_BindTexture( self->context, material->diffuseTexture );
			}
			glActiveTexture(GL_TEXTURE0 + DAOX_TILE_TEXTURE1 + i);
			glBindTexture(GL_TEXTURE_2D, material2->diffuseTexture->tid);
			glUniform1i(self->shader->uniforms.tileTextures[i], DAOX_TILE_TEXTURE1 + i );
		}
	}
	glUniform1i( self->shader->uniforms.particleType, drawtask->particleType );
	glUniform1i( self->shader->uniforms.terrainTileType, terrainTileType );
	glUniform1i( self->shader->uniforms.tileTextureCount, tileTextureCount );
	glUniform1f( self->shader->uniforms.tileTextureScale, tileTextureScale );

	if( material != NULL ) diffuseTexture = material->diffuseTexture;
	if( diffuseTexture ){
		int uniform = self->shader->uniforms.diffuseTexture;
		hasDiffuseTexture = 
			DaoxRenderer_SetupTexture( self, diffuseTexture, DAOX_DIFFUSE_TEXTURE, uniform );
	}
	if( material != NULL ) emissionTexture = material->emissionTexture;
	if( emissionTexture ){
		int uniform = self->shader->uniforms.emissionTexture;
		hasEmissionTexture = 
			DaoxRenderer_SetupTexture( self, emissionTexture, DAOX_EMISSION_TEXTURE, uniform );
	}
	if( material != NULL ) bumpTexture = material->bumpTexture;
	if( bumpTexture ){
		int uniform = self->shader->uniforms.bumpTexture;
		hasBumpTexture = 
			DaoxRenderer_SetupTexture( self, bumpTexture, DAOX_BUMP_TEXTURE, uniform );
	}
	//printf( "hasBumpTexture = %i\n", hasBumpTexture );
	glUniform1i(self->shader->uniforms.hasDiffuseTexture, hasDiffuseTexture );
	glUniform1i(self->shader->uniforms.hasEmissionTexture, hasEmissionTexture );
	glUniform1i(self->shader->uniforms.hasBumpTexture, hasBumpTexture );
	glDrawRangeElements( drawtask->shape, 0, M, M, GL_UNSIGNED_INT, (void*)K );
	glUniform1i(self->shader->uniforms.hasDiffuseTexture, 0 );
	glUniform1i(self->shader->uniforms.hasEmissionTexture, 0 );
	glUniform1i(self->shader->uniforms.hasBumpTexture, 0 );
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
	DaoxColor bgcolor = scene->background;
	GLfloat matrix[16] = {0};
	GLfloat matrix2[16] = {0};
	GLfloat matrix3[9] = {0};
	daoint i, j, lightCount = scene->lights->size;
	float cosine, sine;

	if( scene->nodes->size == 0 ) return;

	if( scene != self->scene ) GC_Assign( & self->scene, scene );

	if( cam == NULL ) cam = scene->camera;
	if( cam == NULL ) cam = self->camera;
	if( cam == NULL ){
		DaoxOBBox3D sceneObbox;
		DaoxVector3D P;

		self->camera = cam = DaoxCamera_New();
		DaoGC_IncRC( (DaoValue*) self->camera );

		DaoxScene_EstimateBoundingBox( scene, & sceneObbox );
		sceneObbox = DaoxOBBox3D_Scale( & sceneObbox, 1.2 );
		sceneObbox = DaoxOBBox3D_ToAABox( & sceneObbox );
		cam->farPlane = 10000 + 2*sceneObbox.R;

		P = DaoxOBBox3D_GetDiagonalVertex( & sceneObbox );
		DaoxCamera_LookAt( cam, sceneObbox.C );
		DaoxCamera_Move( cam, P );
		DaoxCamera_Orient( cam, 2 );
	}

	if( cam && cam != self->camera ){
		GC_Assign( & self->camera, cam );
	}
	cam = self->camera;
	cam->aspectRatio = self->context->deviceWidth / (float) self->context->deviceHeight;

	//DaoxMatrix4D_Print( & rotation );
#if 0
	int error = glGetError();
	printf( "line %i: %i\n", __LINE__, glGetError() );
#endif


	objectToWorld = DaoxSceneNode_GetWorldTransform( & cam->base );
	viewMatrix = DaoxMatrix4D_Inverse( & objectToWorld );
	//DaoxMatrix4D_Print( & cam->base.transform );
	//DaoxMatrix4D_Print( & objectToWorld );

	DaoxViewFrustum_Init( & fm, cam );
	fm.ratio = self->context->deviceWidth / (fm.right - fm.left);

	if( self->showAxis ) DaoxSceneNode_Move( (DaoxSceneNode*) self->worldAxis, fm.axisOrigin );

	self->frustum = fm;
	DList_Clear( self->canvases );
	DaoxRenderer_ClearDrawTasks( self, self->tasks );
	DaoxRenderer_ClearDrawTasks( self, self->tasks2 );
	for(i=0; i<scene->nodes->size; ++i){
		DaoxSceneNode *node = scene->nodes->items.pSceneNode[i];
		DaoxRenderer_PrepareNode( self, node );
	}
	if( self->showAxis ) DaoxRenderer_PrepareNode( self, (DaoxSceneNode*) self->worldAxis );
	if( self->tasks->size ) DaoxRenderer_UpdateBuffer( self, self->tasks, self->buffer );
	if( self->tasks2->size ) DaoxRenderer_UpdateBuffer( self, self->tasks2, self->bufferSK );

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

#ifndef DAO_GRAPHICS_USE_GLES
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if( self->showMesh ){
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	}else{
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}
#endif

	if( bgcolor.alpha >= 1.0/255.0 ){
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor( bgcolor.red, bgcolor.green, bgcolor.blue, bgcolor.alpha );
	}

	glUseProgram( self->shader->program );

	glUniform1f(self->shader->uniforms.time, Dao_GetCurrentTime() );
	glUniform1i(self->shader->uniforms.vectorGraphics, 0 );
	glUniform1i(self->shader->uniforms.hasDiffuseTexture, 0 );
	glUniform1i(self->shader->uniforms.hasBumpTexture, 0 );
	glUniform1i(self->shader->uniforms.dashCount, 0 );
	glUniform1i(self->shader->uniforms.gradientType, 0 );
	glUniform1i(self->shader->uniforms.gradientStops, 0 );
	glUniform1f(self->shader->uniforms.gradientRadius, 0 );
	glUniform1i(self->shader->uniforms.terrainTileType, 0 );
	glUniform1i(self->shader->uniforms.tileTextureCount, 0 );

	glActiveTexture( GL_TEXTURE0 + DAOX_GRADIENT_SAMPLER);
	glBindTexture( GL_TEXTURE_2D, self->shader->textures.gradientSampler );
	glUniform1i( self->shader->uniforms.gradientSampler, DAOX_GRADIENT_SAMPLER );

	glActiveTexture( GL_TEXTURE0 + DAOX_DASH_SAMPLER);
	glBindTexture( GL_TEXTURE_2D, self->shader->textures.dashSampler );
	glUniform1i( self->shader->uniforms.dashSampler, DAOX_DASH_SAMPLER );

	glUniformMatrix4fv( self->shader->uniforms.projMatrix, 1, 0, matrix2 );
	glUniformMatrix4fv( self->shader->uniforms.viewMatrix, 1, 0, matrix );
	glUniform3fv( self->shader->uniforms.cameraPosition, 1, & cameraPosition.x );
	glUniform1i( self->shader->uniforms.lightCount, lightCount );
	glUniform3fv( self->shader->uniforms.lightSource, lightCount, & lightSource[0].x );
	glUniform4fv( self->shader->uniforms.lightIntensity, lightCount, & lightIntensity[0].red );


	glBindVertexArray( self->buffer->vertexVAO );
	glBindBuffer( GL_ARRAY_BUFFER, self->buffer->vertexVBO );
	//glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->buffer->triangleVBO );

	for(i=0; i<self->tasks->size; ++i){
		DaoxRenderer_DrawTask( self, self->tasks->items.pDrawTask[i] );
	}
	glBindVertexArray(0);


	glBindVertexArray( self->bufferSK->vertexVAO );
	glBindBuffer( GL_ARRAY_BUFFER, self->bufferSK->vertexVBO );
	//glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->bufferSK->triangleVBO );

	for(i=0; i<self->tasks2->size; ++i){
		DaoxRenderer_DrawTask( self, self->tasks2->items.pDrawTask[i] );
	}
	glBindVertexArray(0);


	glDepthMask(GL_FALSE);
	glBindVertexArray( self->bufferVG->vertexVAO );
	glBindBuffer( GL_ARRAY_BUFFER, self->bufferVG->vertexVBO );
	//glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->bufferVG->triangleVBO );
	glUniform1i(self->shader->uniforms.vectorGraphics, 1 );
	for(i=0; i<self->canvases->size; ++i){
		DaoxCanvas *canvas = self->canvases->items.pCanvas[i];
		DaoxPainter painter;
		memset( & painter, 0, sizeof(DaoxPainter) );
		painter.showMesh = self->showMesh;
		painter.shader = self->shader;
		painter.buffer = self->bufferVG;
		painter.context = self->context;
		glUniform4fv( self->shader->uniforms.diffuseColor, 1, & canvas->background.red );
		DaoxPainter_PaintCanvas( & painter, canvas, cam );
	}
	glUniform1i(self->shader->uniforms.vectorGraphics, 0 );
	glBindVertexArray(0);
}



