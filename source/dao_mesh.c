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


#include "dao_mesh.h"
#include "dao_scene.h"




DaoxMeshChunk* DaoxMeshChunk_New( DaoxMeshUnit *unit )
{
	DaoxMeshChunk *self = (DaoxMeshChunk*) dao_calloc( 1, sizeof(DaoxMeshChunk) );
	self->triangles = DArray_New( sizeof(int) );
	self->unit = unit;
	self->parent = self->left = self->right = NULL;
	return self;
}
void DaoxMeshChunk_Delete( DaoxMeshChunk *self )
{
	if( self->left ) DaoxMeshChunk_Delete( self->left );
	if( self->right ) DaoxMeshChunk_Delete( self->right );
	DArray_Delete( self->triangles );
	dao_free( self );
}
void DaoxMeshChunk_ResetBoundingBox( DaoxMeshChunk *self, DArray *buffer )
{
	//printf( "DaoxMeshChunk_ResetBoundingBox: %i\n", self->triangles->size );
	DaoxVertex *vertices = self->unit->vertices->data.vertices;
	DaoxTriangle *triangles = self->unit->triangles->data.triangles;
	daoint i, j;

	DArray_Reset( buffer, 0 );
	for(i=0; i<self->triangles->size; ++i){
		DaoxTriangle triangle = triangles[ self->triangles->data.ints[i] ];
		for(j=0; j<3; ++j){
			DaoxVector3D *point = (DaoxVector3D*) DArray_Push( buffer );
			*point = vertices[triangle.index[j]].point;
		}
	}
	DaoxOBBox3D_ComputeBoundingBox( & self->obbox, buffer->data.vectors3d, buffer->size );
}




DaoxMeshUnit* DaoxMeshUnit_New()
{
	DaoxMeshUnit *self = (DaoxMeshUnit*) dao_calloc( 1, sizeof(DaoxMeshUnit) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_mesh_unit );
	self->vertices = DArray_New( sizeof(DaoxVertex) );
	self->triangles = DArray_New( sizeof(DaoxTriangle) );
	self->tree = NULL;
	self->mesh = NULL;
	self->material = NULL;
	return self;
}
void DaoxMeshUnit_Delete( DaoxMeshUnit *self )
{
	if( self->tree ) DaoxMeshChunk_Delete( self->tree );
	DArray_Delete( self->vertices );
	DArray_Delete( self->triangles );
	DaoGC_DecRC( (DaoValue*) self->mesh );
	DaoGC_DecRC( (DaoValue*) self->material );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}
void DaoxMeshUnit_MoveBy( DaoxMeshUnit *self, float dx, float dy, float dz )
{
	int i;

	for(i=0; i<self->vertices->size; ++i){
		DaoxVector3D *norm = & self->vertices->data.vertices[i].point;
		norm->x += dx;
		norm->y += dy;
		norm->z += dz;
	}
}
void DaoxMeshUnit_ScaleBy( DaoxMeshUnit *self, float fx, float fy, float fz )
{
	int i;

	for(i=0; i<self->vertices->size; ++i){
		DaoxVector3D *norm = & self->vertices->data.vertices[i].point;
		norm->x *= fx;
		norm->y *= fy;
		norm->z *= fz;
	}
}
void DaoxMeshUnit_UpdateNorms( DaoxMeshUnit *self )
{
	int i, j;

	for(i=0; i<self->vertices->size; ++i){
		DaoxVector3D *norm = & self->vertices->data.vertices[i].norm;
		norm->x = norm->y = norm->z = 0.0;
	}
	for(i=0; i<self->triangles->size; ++i){
		DaoxTriangle triangle = self->triangles->data.triangles[i];
		DaoxVector3D A = self->vertices->data.vertices[ triangle.index[0] ].point;
		DaoxVector3D B = self->vertices->data.vertices[ triangle.index[1] ].point;
		DaoxVector3D C = self->vertices->data.vertices[ triangle.index[2] ].point;
		DaoxVector3D AB = DaoxVector3D_Sub( & B, & A );
		DaoxVector3D BC = DaoxVector3D_Sub( & C, & B );
		DaoxVector3D facenorm = DaoxVector3D_Cross( & AB, & BC );
		facenorm = DaoxVector3D_Normalize( & facenorm );
		for(j=0; j<3; ++j){
			DaoxVector3D *vnorm = & self->vertices->data.vertices[ triangle.index[j] ].norm;
			vnorm->x += facenorm.x;
			vnorm->y += facenorm.y;
			vnorm->z += facenorm.z;
		}
	}
	for(i=0; i<self->vertices->size; ++i){
		DaoxVector3D *norm = & self->vertices->data.vertices[i].norm;
		*norm = DaoxVector3D_Normalize( norm );
	}
}
void DaoxMeshUnit_SetMaterial( DaoxMeshUnit *self, DaoxMaterial *material )
{
	GC_Assign( & self->material, material );
}

typedef struct TriangleInfo TriangleInfo;
struct TriangleInfo
{
	uint_t  index;
	double  value;
};

int TriangleInfo_LT( TriangleInfo *one, TriangleInfo *another )
{
	if( one->value != another->value ) return one->value < another->value;
	return one->index < another->index;
}

void TriangleInfo_QuickSort( TriangleInfo items[], int first, int last )
{
	TriangleInfo pivot, tmp;
	int lower = first + 1;
	int upper = last;

	if( first >= last ) return;
	tmp = items[first];
	items[first] = items[ (first+last)/2 ];
	items[ (first+last)/2 ] = tmp;
	pivot = items[ first ];

	while( lower <= upper ){
		while( lower < last && TriangleInfo_LT( & items[lower], & pivot ) ) lower ++;
		while( upper > first && TriangleInfo_LT( & pivot, & items[upper] ) ) upper --;
		if( lower < upper ){
			tmp = items[lower];
			items[lower] = items[upper];
			items[upper] = tmp;
			upper --;
		}
		lower ++;
	}
	tmp = items[first];
	items[first] = items[upper];
	items[upper] = tmp;
	if( first+1 < upper ) TriangleInfo_QuickSort( items, first, upper-1 );
	if( upper+1 < last ) TriangleInfo_QuickSort( items, upper+1, last );
}


#define MIN_MESH_CHUNK  128

void DaoxMeshUnit_UpdateTree( DaoxMeshUnit *self, int maxtriangles )
{
	TriangleInfo *sorting = NULL;
	DaoxVertex *vertices = self->vertices->data.vertices;
	DaoxTriangle *triangles = self->triangles->data.triangles;
	DArray *points = DArray_New( sizeof(DaoxVector3D) );
	DList *nodes = DList_New(0);
	daoint i, j, half, capacity = 0;
	if( maxtriangles <= 0 ) maxtriangles = MIN_MESH_CHUNK;
	if( self->tree == NULL ) self->tree = DaoxMeshChunk_New( self );
	self->tree->triangles->size = 0;
	for(i=0; i<self->triangles->size; ++i) DArray_PushInt( self->tree->triangles, i );
	printf( "DaoxMeshUnit_UpdateTree: %i\n", maxtriangles );
	DList_Append( nodes, self->tree );
	for(i=0; i<nodes->size; ++i){
		DaoxMeshChunk *node = (DaoxMeshChunk*) nodes->items.pVoid[i];
		DaoxVector3D OX, OY, OZ, longest;
		int *ids = node->triangles->data.ints;
		int count = node->triangles->size;
		float x2, y2, z2;

		DaoxMeshChunk_ResetBoundingBox( node, points );
		if( count == 0 ) continue;

		if( count <= maxtriangles ) continue;

		OX = DaoxVector3D_Sub( & node->obbox.X, & node->obbox.O );
		OY = DaoxVector3D_Sub( & node->obbox.Y, & node->obbox.O );
		OZ = DaoxVector3D_Sub( & node->obbox.Z, & node->obbox.O );
		x2 = DaoxVector3D_Norm2( & OX );
		y2 = DaoxVector3D_Norm2( & OY );
		z2 = DaoxVector3D_Norm2( & OZ );
		if( y2 > x2 ){
			OX = OY;
			x2 = y2;
		}
		if( z2 > x2 ){
			OX = OZ;
			x2 = z2;
		}
		if( node->triangles->size > capacity ){
			capacity = node->triangles->size;
			sorting = (TriangleInfo*) dao_realloc( sorting, capacity*sizeof(TriangleInfo) );
		}
		for(j=0; j<node->triangles->size; ++j){
			DaoxTriangle triangle = triangles[ node->triangles->data.ints[j] ];
			DaoxVector3D A = vertices[ triangle.index[0] ].point;
			DaoxVector3D B = vertices[ triangle.index[1] ].point;
			DaoxVector3D C = vertices[ triangle.index[2] ].point;
			DaoxVector3D AB = DaoxVector3D_Add( & A, & B );
			DaoxVector3D ABC = DaoxVector3D_Add( & AB, & C );
			ABC = DaoxVector3D_Scale( & ABC, 0.333333333 );
			ABC = DaoxVector3D_Sub( & ABC, & node->obbox.O );
			sorting[j].index = node->triangles->data.ints[j];
			sorting[j].value = DaoxVector3D_Dot( & ABC, & OX );
		}
		TriangleInfo_QuickSort( sorting, 0, node->triangles->size - 1 );
		if( node->left == NULL ) node->left = DaoxMeshChunk_New( self );
		if( node->right == NULL ) node->right = DaoxMeshChunk_New( self );
		node->left->triangles->size = 0;
		node->right->triangles->size = 0;
		half = node->triangles->size/2;
		for(j=0; j<half; ++j){
			DArray_PushInt( node->left->triangles, sorting[j].index );
		}
		for(j=half; j<node->triangles->size; ++j){
			DArray_PushInt( node->right->triangles, sorting[j].index );
		}
		DList_Append( nodes, node->left );
		DList_Append( nodes, node->right );
	}
	self->obbox = self->tree->obbox;
	DaoxOBBox3D_Print( & self->obbox );
	DArray_Delete( points );
	DList_Delete( nodes );
}




DaoxMesh* DaoxMesh_New()
{
	DaoxMesh *self = (DaoxMesh*) dao_calloc( 1, sizeof(DaoxMesh) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_mesh );
	self->units = DList_New( DAO_DATA_VALUE );
	return self;
}
void DaoxMesh_Delete( DaoxMesh *self )
{
	DList_Delete( self->units );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}
DaoxMeshUnit* DaoxMesh_AddUnit( DaoxMesh *self )
{
	DaoxMeshUnit *unit = DaoxMeshUnit_New();
	DaoGC_IncRC( (DaoValue*) self );
	DaoGC_IncRC( (DaoValue*) unit );
	unit->mesh = self;
	unit->index = self->units->size;
	DList_Append( self->units, unit );
	return unit;
}
void DaoxMesh_SetMaterial( DaoxMesh *self, DaoxMaterial *material )
{
	daoint i;
	for(i=0; i<self->units->size; ++i){
		DaoxMeshUnit *unit = (DaoxMeshUnit*) self->units->items.pVoid[i];
		DaoxMeshUnit_SetMaterial( unit, material );
	}
}
void DaoxMesh_UpdateTree( DaoxMesh *self, int maxtriangles )
{
	daoint i;
	for(i=0; i<self->units->size; ++i){
		DaoxMeshUnit *unit = (DaoxMeshUnit*) self->units->items.pVoid[i];
		DaoxMeshUnit_UpdateTree( unit, maxtriangles );
	}
}
void DaoxMesh_UpdateNorms( DaoxMesh *self )
{
	int i;
	for(i=0; i<self->units->size; ++i){
		DaoxMeshUnit *unit = (DaoxMeshUnit*) self->units->items.pVoid[i];
		DaoxMeshUnit_UpdateNorms( unit );
	}
}
void DaoxMesh_ResetBoundingBox( DaoxMesh *self )
{
	DArray *points = DArray_New( sizeof(DaoxVector3D) );
	daoint i, j, k;
	for(i=0; i<self->units->size; ++i){
		DaoxMeshUnit *unit = (DaoxMeshUnit*) self->units->items.pVoid[i];
		for(j=0; j<unit->vertices->size; ++j){
			DaoxVector3D point = unit->vertices->data.vertices[j].point;
			DArray_PushVector3D( points, & point );
		}
	}
	DaoxOBBox3D_ComputeBoundingBox( & self->obbox, points->data.vectors3d, points->size );
	DArray_Delete( points );
}
static void DaoxMesh_MakeTriangle( DaoxMesh *self, int c, float x, float y, float dx, float dy, float z )
{
	DaoxMeshUnit *unit = DaoxMesh_AddUnit( self );
	DaoxMaterial *material = DaoxMaterial_New();
	DaoxVertex *vertices = unit->vertices->data.vertices;
	DaoxTriangle *triangles = unit->triangles->data.triangles;
	DaoxVector3D norm = {0.0,0.0,1.0};
	int i;

	DaoxMeshUnit_SetMaterial( unit, material );
	DArray_Resize( unit->vertices, 3 );
	DArray_Resize( unit->triangles, 1 );
	vertices = unit->vertices->data.vertices;
	triangles = unit->triangles->data.triangles;
	vertices[0].point.x = x;
	vertices[0].point.y = y;
	vertices[1].point.x = x + dx;
	vertices[1].point.y = y;
	vertices[2].point.x = x;
	vertices[2].point.y = y + dy;
	triangles[0].index[0] = 0;
	triangles[0].index[1] = 1;
	triangles[0].index[2] = 2;
	for(i=0; i<3; ++i){
		vertices[i].point.z = z;
		vertices[i].norm = norm;
	}
	switch( c ){
	case 0 : material->diffuse = daox_gray_color; break;
	case 1 : material->diffuse = daox_red_color; break;
	case 2 : material->diffuse = daox_green_color; break;
	case 3 : material->diffuse = daox_blue_color; break;
	}
}
void DaoxMesh_MakeViewFrustumCorners( DaoxMesh *self, float fov, float ratio, float near )
{
	DaoxVector3D norm = {0.0,0.0,1.0};
	float xtan = tan( 0.5 * fov * M_PI / 180.0 );
	float right = near * xtan;
	float top = right / ratio;
	float width = 0.05*right;
	float z = - 1.01*near;

	DaoxMesh_MakeTriangle( self, 0, -right, -top, width, width, z );
	DaoxMesh_MakeTriangle( self, 1, right, -top, -width, width, z );
	DaoxMesh_MakeTriangle( self, 2, right, top, -width, -width, z );
	DaoxMesh_MakeTriangle( self, 3, -right, top, width, -width, z );
}



static float box_vertices[][3] =
{
	{ 0.5F,  0.5, -0.5},
	{ 0.5F, -0.5, -0.5},
	{-0.5F, -0.5, -0.5},
	{-0.5F,  0.5, -0.5},
	{-0.5F,  0.5,  0.5},
	{ 0.5F,  0.5,  0.5},
	{ 0.5F, -0.5,  0.5},
	{-0.5F, -0.5,  0.5}
};
static int box_faces[][4] =
{
	{ 0, 1, 2, 3 },
	{ 3, 4, 7, 2 },
	{ 0, 5, 6, 1 },
	{ 5, 4, 7, 6 },
	{ 5, 0, 3, 4 },
	{ 6, 1, 2, 7 }
};

DaoxMeshUnit* DaoxMesh_MakeBoxObject( DaoxMesh *self )
{
	int i, j;
	DaoxMeshUnit *unit = DaoxMesh_AddUnit( self );
	unit->vertices->size = 0;
	unit->triangles->size = 0;
	for(i=0; i<8; ++i){
		DaoxVertex *vertex = DArray_PushVertex( unit->vertices, NULL );
		vertex->point.x = box_vertices[i][0];
		vertex->point.y = box_vertices[i][1];
		vertex->point.z = box_vertices[i][2];
		vertex->norm.x = 1.0;
		vertex->norm.y = 0.0;
		vertex->norm.z = 0.0;
	}
	for(i=0; i<6; ++i){
		int *face = box_faces[i];
		for(j=2; j<4; ++j){
			DaoxTriangle *triangle = DArray_PushTriangle( unit->triangles, NULL );
			triangle->index[0] = face[0];
			triangle->index[1] = face[j-1];
			triangle->index[2] = face[j];
		}
	}
	DaoxMeshUnit_UpdateNorms( unit );
	DaoxMesh_ResetBoundingBox( self );
	return unit;
}



