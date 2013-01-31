
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
#include "dao_renderer.h"








DaoxRenderer* DaoxRenderer_New()
{
	DaoxRenderer *self = (DaoxRenderer*) dao_calloc( 1, sizeof(DaoxRenderer) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_renderer );
	self->visibleModels = DArray_New(0);
	self->visibleChunks = DArray_New(0);
	self->drawLists = DArray_New(0);
	self->mapMaterials = DMap_New(0,0);
	self->vertices = DaoxPlainArray_New( sizeof(DaoxVertex) );
	self->triangles = DaoxPlainArray_New( sizeof(DaoxTriangle) );
	self->mapping = DaoxPlainArray_New( sizeof(int) );
	DaoxRenderer_InitShaders( self );
	DaoxRenderer_InitBuffers( self );
	return self;
}
void DaoxRenderer_Delete( DaoxRenderer *self )
{
	DaoxShader_Free( & self->shader );
	DaoCstruct_Free( (DaoCstruct*) self );
	DArray_Delete( self->drawLists );
	DArray_Delete( self->visibleChunks );
	DArray_Delete( self->visibleModels );
	DMap_Delete( self->mapMaterials );
	DaoxPlainArray_Delete( self->vertices );
	DaoxPlainArray_Delete( self->triangles );
	DaoxPlainArray_Delete( self->mapping );
	dao_free( self );
}


DaoxCamera* DaoxRenderer_GetCurrentCamera( DaoxRenderer *self )
{
	if( self->camera ) return self->camera;
	self->camera = DaoxCamera_New();
	DaoGC_IncRC( self->camera );
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
	int tex  = self->shader.attributes.texCoord;
	DaoxBuffer_Init3D( & self->buffer, pos, norm, tex );
}


void DaoxRenderer_PrepareMeshChunk( DaoxRenderer *self, DaoxModel *model, DaoxMeshChunk *chunk, DaoxViewFrustum *frustum, int knownVisible )
{
	DNode *it;
	DArray *chunks;
	DaoxMaterial *material;
	DaoxVertex *vertices = chunk->unit->vertices->pod.vertices;
	daoint i, m, check;

	if( chunk->triangles->size == 0 ) return;

	if( knownVisible == 0 ){
		check = DaoxViewFrustum_Visible( frustum, & chunk->obbox );
		if( check < 0 ) return;
		knownVisible = check > 0;
	}

	if( chunk->left && chunk->left->triangles->size )
		DaoxRenderer_PrepareMeshChunk( self, model, chunk->left, frustum, knownVisible );
	if( chunk->right && chunk->right->triangles->size )
		DaoxRenderer_PrepareMeshChunk( self, model, chunk->right, frustum, knownVisible );

	if( chunk->left && chunk->left->triangles->size ) return;
	if( chunk->right && chunk->right->triangles->size ) return;

	it = DMap_Find( self->mapMaterials, chunk->unit->material );
	m = it ? it->value.pInt : self->mapMaterials->size;
	if( it == NULL ){
		m = self->mapMaterials->size;
		if( m >= self->visibleChunks->size ){
			chunks = DArray_New(0);
			DArray_Append( self->visibleChunks, chunks );
		}
		chunks = self->visibleChunks->items.pArray[m];
		chunks->size = 0;
		DMap_Insert( self->mapMaterials, chunk->unit->material, m );
	}
	chunks = self->visibleChunks->items.pArray[m];
	DArray_Append( chunks, model );
	DArray_Append( chunks, chunk );
	self->triangleCount += chunk->triangles->size;
}
void DaoxRenderer_PrepareMesh( DaoxRenderer *self, DaoxModel *model, DaoxMesh *mesh )
{
	DaoxVertex *vertex;
	daoint i;

	for(i=0; i<mesh->units->size; ++i){
		DaoxMeshUnit *unit = (DaoxMeshUnit*) mesh->units->items.pVoid[i];
		int currentCount = self->triangleCount;
		if( unit->tree == NULL ) return;
		DaoxRenderer_PrepareMeshChunk( self, model, unit->tree, & self->localFrustum, 0 );
		if( self->triangleCount > currentCount ){
			DaoxPlainArray *vertices = unit->vertices;
			self->vertexCount += vertices->size;
		}
	}
}


void DaoxRenderer_PrepareNode( DaoxRenderer *self, DaoxSceneNode *node )
{
	DaoType *ctype = node->ctype;
	DaoxModel *model = (DaoxModel*) node;
	DaoxMatrix4D objectToWorld;
	DaoxMatrix4D worldToObject;
	daoint i;

	if( ctype != daox_type_model && ctype != daox_type_canvas ) goto PrepareChildren;

	// 1. Transform view frustum to object coordinates;
	objectToWorld = DaoxSceneNode_GetWorldTransform( node );
	worldToObject = DaoxMatrix4D_Inverse( & objectToWorld );
	self->localFrustum = DaoxViewFrustum_Transform( & self->frustum, & worldToObject );

	// 2. Check if the obbox box of the object intersect with the frustum;
	if( DaoxViewFrustum_Visible( & self->localFrustum, & node->obbox ) < 0 ) return;

	if( ctype == daox_type_canvas ){
		/* The canvas is locally placed on the xy-plane facing z-axis: */
		DaoxVector3D canvasNorm = {0.0,0.0,1.0};
		double dot = DaoxVector3D_Dot( & canvasNorm, & self->localFrustum.cameraPosition );
		if( dot < 0.0 ) return;
		//DaoxRenderer_PrepareCanvas( self, (DaoxCanvas*) node );
		return;
	}

	DaoxPlainArray_ResetSize( model->offsets, model->mesh->units->size );
	memset( model->offsets->pod.ints, 0, model->mesh->units->size*sizeof(int) );
	DaoxRenderer_PrepareMesh( self, model, model->mesh );

PrepareChildren:
	for(i=0; i<node->children->size; ++i){
		DaoxSceneNode *node2 = node->children->items.pVoid[i];
		DaoxRenderer_PrepareNode( self, node2 );
	}
}
void DaoxMatrix4D_Export( DaoxMatrix4D *self, GLfloat matrix[16] )
{
	matrix[0] = self->A11;
	matrix[1] = self->A21;
	matrix[2] = self->A31;
	matrix[3] = 0.0;
	matrix[4] = self->A12;
	matrix[5] = self->A22;
	matrix[6] = self->A32;
	matrix[7] = 0.0;
	matrix[8] = self->A13;
	matrix[9] = self->A23;
	matrix[10] = self->A33;
	matrix[11] = 0.0;
	matrix[12] = self->B1;
	matrix[13] = self->B2;
	matrix[14] = self->B3;
	matrix[15] = 1.0;
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

void DaoxRenderer_ReduceData( DaoxRenderer *self, DaoxSceneNode *node )
{
	DaoxModel *model = (DaoxModel*) node;
	daoint i;

	if( node->ctype != daox_type_model ) return;
	for(i=0; i<node->children->size; ++i){
		DaoxSceneNode *child = (DaoxSceneNode*) node->children->items.pVoid[i];
		DaoxRenderer_ReduceData( self, child );
	}
	if( abs( self->frameIndex - model->viewFrame ) < 1000 ) return;
	if( model->offsets->size == 0 ) return;
	model->offsets->size = 0;
	for(i=0; i<model->points->size; ++i){
		DaoxPlainArray_Clear( (DaoxPlainArray*) model->points->items.pVoid[i] );
		DaoxPlainArray_Clear( (DaoxPlainArray*) model->vnorms->items.pVoid[i] );
	}
	for(i=0; i<model->tnorms->size; ++i){
		DaoxPlainArray_Clear( (DaoxPlainArray*) model->tnorms->items.pVoid[i] );
	}
}

void DaoxRenderer_ProcessMeshChunk( DaoxRenderer *self, DaoxModel *model, DaoxMeshChunk *chunk )
{
	DaoxMeshUnit *unit = chunk->unit;
	DaoxMesh *mesh = unit->mesh;
	DaoxVector3D cameraPosition = self->frustum.cameraPosition;
	DaoxPlainArray *points = (DaoxPlainArray*) model->points->items.pVoid[unit->index];
	DaoxPlainArray *vnorms = (DaoxPlainArray*) model->vnorms->items.pVoid[unit->index];
	DaoxPlainArray *tnorms = (DaoxPlainArray*) model->tnorms->items.pVoid[unit->index];
	int vertexOffset = self->vertices->size;
	int triangleOffset = self->triangles->size;
	int i, j, id, *mapping;

	mapping = self->mapping->pod.ints;
	for(i=0; i<chunk->triangles->size; ++i){
		int triangleID = chunk->triangles->pod.ints[i];
		DaoxTriangle *trg, *triangle = unit->triangles->pod.triangles + triangleID;
		DaoxVector3D point = points->pod.vectors3d[triangle->index[0]];
		DaoxVector3D direction = point;

		direction.x -= cameraPosition.x;// - point.x;
		direction.y -= cameraPosition.y;// - point.y;
		direction.z -= cameraPosition.z;// - point.z;
		/* backface culling in world coordinates: */
		if( DaoxVector3D_Dot( & direction, & tnorms->pod.vectors3d[triangleID] ) > 0.0 ) continue;
		trg = DaoxPlainArray_PushTriangle( self->triangles );
		for(j=0; j<3; ++j){
			id = triangle->index[j];
			if( mapping[id] == 0 ){
				DaoxVertex *vertex = DaoxPlainArray_PushVertex( self->vertices );
				vertex->point = points->pod.vectors3d[id];
				vertex->norm = vnorms->pod.vectors3d[id];
				vertex->texUV = unit->vertices->pod.vertices[id].texUV;
				mapping[id] = self->vertices->size;
			}
			trg->index[j] = mapping[id]-1;
		}
	}
}
void DaoxRenderer_ProcessChunks( DaoxRenderer *self, DArray *chunks, int M, int N )
{
	DaoxModel *model = (DaoxModel*) chunks->items.pVoid[M];
	DaoxMeshChunk *chunk = (DaoxMeshChunk*) chunks->items.pVoid[M+1];
	int i;

	DaoxPlainArray_Reserve( self->mapping, chunk->unit->vertices->size );
	memset( self->mapping->pod.ints, 0, chunk->unit->vertices->size*sizeof(int) );

	DaoxModel_TransformMesh( model );
	for(i=M; i<N; i+=2){
		DaoxModel *model = (DaoxModel*) chunks->items.pVoid[i];
		DaoxMeshChunk *chunk = (DaoxMeshChunk*) chunks->items.pVoid[i+1];
		DaoxRenderer_ProcessMeshChunk( self, model, chunk );
	}
}
void DaoxRenderer_ProcessTriangles( DaoxRenderer *self )
{
	daoint i, j, k;
	self->vertices->size = 0;
	self->triangles->size = 0;
	for(i=0; i<self->visibleChunks->size; ++i){
		DaoxMaterial *material = NULL;
		DArray *chunks = self->visibleChunks->items.pArray[i];
		int offset = self->triangles->size;
		if( chunks->size == 0 ) continue;
		for(j=0; j<chunks->size; ){
			DaoxModel *model = (DaoxModel*) chunks->items.pVoid[j];
			DaoxMeshChunk *chunk = (DaoxMeshChunk*) chunks->items.pVoid[j+1];
			material = chunk->unit->material;
			for(k=j; k<chunks->size; k+=2){
				DaoxModel *model2 = (DaoxModel*) chunks->items.pVoid[k];
				if( model2 != model ) break;
			}
			DArray_Append( self->visibleModels, model );
			DaoxRenderer_ProcessChunks( self, chunks, j, k );
			j = k;
		}
		if( self->triangles->size == offset ) continue;
		DArray_Append( self->drawLists, offset );
		DArray_Append( self->drawLists, self->triangles->size - offset );
		DArray_Append( self->drawLists, material );
	}
}
void DaoxRenderer_UpdateBuffer( DaoxRenderer *self, DaoxVector3D camPos )
{
	int i;
	int vertexCount = self->vertices->size;
	int triangleCount = self->triangles->size;
	DaoGLVertex3D *glvertices = DaoxBuffer_MapVertices3D( & self->buffer, vertexCount );
	DaoGLTriangle *gltriangles = DaoxBuffer_MapTriangles( & self->buffer, triangleCount );
	//printf( "DaoxRenderer_UpdateBuffer: %i %i %i\n", vertexCount, triangleCount, self->drawLists->size );
	//printf( "buffering: %15p %15p %6i %6i\n", glvertices, gltriangles, self->vertexOffset, self->triangleOffset );
	for(i=0; i<vertexCount; ++i){
		DaoxVertex *vertex1 = self->vertices->pod.vertices + i;
		DaoGLVertex3D *vertex2 = glvertices + i;
		vertex2->point.x = vertex1->point.x;
		vertex2->point.y = vertex1->point.y;
		vertex2->point.z = vertex1->point.z;
		vertex2->norm.x = vertex1->norm.x;
		vertex2->norm.y = vertex1->norm.y;
		vertex2->norm.z = vertex1->norm.z;
		vertex2->texUV.x = vertex1->texUV.x;
		vertex2->texUV.y = vertex1->texUV.y;
	}
	for(i=0; i<triangleCount; ++i){
		DaoxTriangle triangle = self->triangles->pod.triangles[i];
		DaoGLTriangle *triangle2 = gltriangles + i;
		triangle2->index[0] = triangle.index[0] + self->buffer.vertexOffset;
		triangle2->index[1] = triangle.index[1] + self->buffer.vertexOffset;
		triangle2->index[2] = triangle.index[2] + self->buffer.vertexOffset;
	}
	for(i=0; i<self->drawLists->size; i+=3){
		daoint *data = self->drawLists->items.pInt + i;
		data[0] += self->buffer.triangleOffset;
	}
	self->buffer.vertexOffset += vertexCount;
	self->buffer.triangleOffset += triangleCount;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void DaoxRenderer_UpdateBuffer2( DaoxRenderer *self, DaoxVector3D camPos )
{
	int i, j, k, glv, glt;
	int vertexCount = self->vertexCount - self->buffer.vertexOffset;
	int triangleCount = self->triangleCount;
	DaoGLVertex3D *glvertices = DaoxBuffer_MapVertices3D( & self->buffer, vertexCount );
	DaoGLTriangle *gltriangles = DaoxBuffer_MapTriangles( & self->buffer, triangleCount );

	//printf( "DaoxRenderer_UpdateBuffer: %i %i\n", vertexCount, triangleCount );
	//printf( "buffering: %15p %15p %6i %6i\n", glvertices, gltriangles, self->vertexOffset[0], self->triangleOffset[0] );
	for(i=0,glv=0,glt=0; i<self->visibleChunks->size; ++i){
		DArray *chunks = self->visibleChunks->items.pArray[i];
		DaoxMaterial *material;
		DaoxMeshChunk *chunk;
		int offset = glt;

		if( chunks->size == 0 ) continue;
		chunk = (DaoxMeshChunk*) chunks->items.pVoid[1];
		material = chunk->unit->material;
		for(j=0; j<chunks->size; j+=2){
			DaoxModel *model = (DaoxModel*) chunks->items.pVoid[j];
			DaoxMeshChunk *chunk = (DaoxMeshChunk*) chunks->items.pVoid[j+1];
			DaoxMeshUnit *unit = chunk->unit;
			DaoxPlainArray *points, *vnorms, *tnorms;
			int *offsets = model->offsets->pod.ints;
			int offset2 = glv;

			if( offsets[unit->index] ) offset2 = offsets[unit->index] - 1;

			offset2 += self->buffer.vertexOffset;
			for(k=0; k<chunk->triangles->size; ++k,++glt){
				int m = chunk->triangles->pod.ints[k];
				DaoxTriangle *triangle = & unit->triangles->pod.triangles[m];
				DaoGLTriangle *triangle2 = gltriangles + glt;
				triangle2->index[0] = triangle->index[0] + offset2;
				triangle2->index[1] = triangle->index[1] + offset2;
				triangle2->index[2] = triangle->index[2] + offset2;
			}

			if( offsets[unit->index] ) continue;
			DArray_Append( self->visibleModels, model );
			DaoxModel_TransformMesh( model );
			points = (DaoxPlainArray*) model->points->items.pVoid[unit->index];
			vnorms = (DaoxPlainArray*) model->vnorms->items.pVoid[unit->index];
			tnorms = (DaoxPlainArray*) model->tnorms->items.pVoid[unit->index];

			offsets[unit->index] = glv + 1;
			for(k=0; k<points->size; ++k,++glv){
				DaoxVertex *vertex1 = unit->vertices->pod.vertices + k;
				DaoGLVertex3D *vertex2 = glvertices + glv;
				DaoxVector3D point = points->pod.vectors3d[k];
				DaoxVector3D norm = vnorms->pod.vectors3d[k];
				vertex2->point.x = point.x;
				vertex2->point.y = point.y;
				vertex2->point.z = point.z;
				vertex2->norm.x = norm.x;
				vertex2->norm.y = norm.y;
				vertex2->norm.z = norm.z;
				vertex2->texUV.x = vertex1->texUV.x;
				vertex2->texUV.y = vertex1->texUV.y;
			}
		}
		DArray_Append( self->drawLists, offset + self->buffer.triangleOffset );
		DArray_Append( self->drawLists, glt - offset );
		DArray_Append( self->drawLists, material );
	}
	self->buffer.vertexOffset += vertexCount;
	self->buffer.triangleOffset += triangleCount;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
		DaoGC_ShiftRC( (DaoValue*) scene, (DaoValue*) self->scene );
		self->scene = scene;
	}
	if( cam == NULL && self->camera == NULL ){
		self->camera = DaoxCamera_New();
		DaoGC_IncRC( self->camera );
	}
	if( cam && cam != self->camera ){
		DaoGC_ShiftRC( (DaoValue*) cam, (DaoValue*) self->camera );
		self->camera = cam;
	}
	cam = self->camera;

	//printf( "DaoxRenderer_Render: %i %i %i %i %f\n", scene->nodes->size, self->drawLists->size, self->vertexOffset, self->triangleOffset, angle_step );

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
	//DaoxCamera_MoveXYZ( cam, rand()%300, 52+rand()%300, rand()%300 );
	//DaoxCamera_LookAtXYZ( cam, 0, 50, 0 );

	objectToWorld = DaoxSceneNode_GetWorldTransform( & cam->base );
	viewMatrix = DaoxMatrix4D_Inverse( & objectToWorld );

	DaoxViewFrustum_Init( & fm, cam );
	if( DaoxViewFrustum_Difference( & fm, & self->frustum ) > EPSILON ){ // TODO: better handling;
		DaoxPlainArray *triangles;
		self->drawLists->size = 0;
		self->triangleCount = 0;
		self->vertexCount = self->buffer.vertexOffset;
		self->visibleModels->size = 0;
		self->frustum = fm;
		//printf( "prepare\n" );
		DMap_Clear( self->mapMaterials );
		for(i=0; i<scene->nodes->size; ++i){
			DaoxSceneNode *node = scene->nodes->items.pVoid[i];
			DaoxRenderer_PrepareNode( self, node );
		}
		if( self->vertexCount > self->buffer.vertexOffset ){
			//DaoxRenderer_ProcessTriangles( self );
			DaoxRenderer_UpdateBuffer2( self, fm.cameraPosition );
		}
	}
	self->frustum = fm;

	cameraPosition = DaoxSceneNode_GetWorldPosition( cam );
	//viewMatrix = DaoxMatrix4D_Identity();
	DaoxMatrix4D_Export( & viewMatrix, matrix );

	MakeProjectionMatrix( & fm, cam, matrix2 );

	if( lightCount > 32 ) lightCount = 32;
	for(i=0; i<lightCount; ++i){
		DaoxLight *light = (DaoxLight*) self->scene->lights->items.pVoid[i];
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


	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5, 0.5, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram( self->shader.program );

	glUniformMatrix4fv( self->shader.uniforms.projMatrix, 1, 0, matrix2 );
	glUniformMatrix4fv( self->shader.uniforms.viewMatrix, 1, 0, matrix );
	glUniform3fv(self->shader.uniforms.cameraPosition, 1, & cameraPosition.x );
	glUniform1i(self->shader.uniforms.lightCount, lightCount );
	glUniform3fv(self->shader.uniforms.lightSource, lightCount, & lightSource[0].x );
	glUniform4fv(self->shader.uniforms.lightIntensity, lightCount, & lightIntensity[0].red );

	glBindVertexArray( self->buffer.vertexVAO );
	glBindBuffer( GL_ARRAY_BUFFER, self->buffer.vertexVBO );
	//glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->buffer.triangleVBO );

	DaoxShader * shader = & self->shader;
	int stride = sizeof(DaoxVertex);
	for(i=0; i<self->drawLists->size; i+=3){
		daoint *data = self->drawLists->items.pInt + i;
		daoint max = self->buffer.vertexOffset; // TODO: better hint;
		daoint K = 3*data[0]*sizeof(GLint);
		daoint M = 3*data[1];
		int textureCount = 0;
		DaoxMaterial *material = (DaoxMaterial*) self->drawLists->items.pVoid[i+2];
		DaoxColor ambient = material ? material->ambient : daox_gray_color;
		DaoxColor diffuse = material ? material->diffuse : daox_gray_color;
		DaoxColor specular = material ? material->specular : daox_gray_color;
		//printf( "%3i %6i %6i\n", i, data[0], data[1] );
		glUniform4fv( self->shader.uniforms.ambientColor, 1, & ambient.red );
		glUniform4fv( self->shader.uniforms.diffuseColor, 1, & diffuse.red );
		glUniform4fv( self->shader.uniforms.specularColor, 1, & specular.red );
		if( material && material->texture1 ){
			DaoxTexture *texture = material->texture1;
			DaoxTexture_glInitTexture( texture );
			if( texture->tid ){
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture->tid);
				glUniform1i(self->shader.uniforms.textures[0], 0);
				textureCount = 1;
			}
		}
		glUniform1i(self->shader.uniforms.textureCount, textureCount );
		glDrawRangeElements( GL_TRIANGLES, 0, max, M, GL_UNSIGNED_INT, (void*)K );
	}
	glBindVertexArray(0);

	self->frameIndex += 1;
	for(i=0; i<self->visibleModels->size; ++i){
		DaoxModel *model = (DaoxModel*) self->visibleModels->items.pVoid[i];
		model->viewFrame = self->frameIndex;
	}
	for(i=0; i<scene->nodes->size; ++i){
		DaoxSceneNode *node = (DaoxSceneNode*) scene->nodes->items.pVoid[i];
		DaoxRenderer_ReduceData( self, node );
	}
}



