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
			DaoxVector3D *pos = (DaoxVector3D*) DArray_Push( buffer );
			*pos = vertices[triangle.index[j]].pos;
		}
	}
	DaoxOBBox3D_ComputeBoundingBox( & self->obbox, buffer->data.vectors3d, buffer->size );
}




DaoxMeshUnit* DaoxMeshUnit_New()
{
	DaoxMeshUnit *self = (DaoxMeshUnit*) dao_calloc( 1, sizeof(DaoxMeshUnit) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_mesh_unit );
	self->skinParams = DArray_New( sizeof(DaoxSkinParam) );
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
	DArray_Delete( self->skinParams );
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
		DaoxVector3D *pos = & self->vertices->data.vertices[i].pos;
		pos->x += dx;
		pos->y += dy;
		pos->z += dz;
	}
}
void DaoxMeshUnit_ScaleBy( DaoxMeshUnit *self, float fx, float fy, float fz )
{
	int i;

	for(i=0; i<self->vertices->size; ++i){
		DaoxVector3D *pos = & self->vertices->data.vertices[i].pos;
		pos->x *= fx;
		pos->y *= fy;
		pos->z *= fz;
	}
}
void DaoxMeshUnit_UpdateNormTangents( DaoxMeshUnit *self, int donormal, int dotangent )
{
	int i, j;

	for(i=0; i<self->vertices->size; ++i){
		DaoxVertex *vertex = & self->vertices->data.vertices[i];
		DaoxVector3D *norm = & vertex->norm;
		DaoxVector3D *tan = & vertex->tan;
		if( donormal ) norm->x = norm->y = norm->z = 0.0;
		if( dotangent ) tan->x = tan->y = tan->z = 0.0;
	}
	for(i=0; i<self->triangles->size; ++i){
		DaoxTriangle triangle = self->triangles->data.triangles[i];
		DaoxVertex *VA = & self->vertices->data.vertices[ triangle.index[0] ];
		DaoxVertex *VB = & self->vertices->data.vertices[ triangle.index[1] ];
		DaoxVertex *VC = & self->vertices->data.vertices[ triangle.index[2] ];
		DaoxVertex_UpdateNormalTangent( VA, VB, VC, donormal, dotangent );
	}
	for(i=0; i<self->vertices->size; ++i){
		DaoxVertex *vertex = & self->vertices->data.vertices[i];
		DaoxVector3D *norm = & vertex->norm;
		DaoxVector3D *tan = & vertex->tan;
		if( donormal ) *norm = DaoxVector3D_Normalize( norm );
		if( dotangent ) *tan = DaoxVector3D_Normalize( tan );
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

	DaoxMeshChunk_ResetBoundingBox( self->tree, points );
	self->obbox = self->tree->obbox;
	//DaoxOBBox3D_Print( & self->obbox );

	//printf( "DaoxMeshUnit_UpdateTree: %i\n", maxtriangles );
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
		/* Get the longest direction: */
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
			DaoxVector3D A = vertices[ triangle.index[0] ].pos;
			DaoxVector3D B = vertices[ triangle.index[1] ].pos;
			DaoxVector3D C = vertices[ triangle.index[2] ].pos;
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
void DaoxMesh_UpdateNormTangents( DaoxMesh *self, int norm, int tan )
{
	int i;
	for(i=0; i<self->units->size; ++i){
		DaoxMeshUnit *unit = (DaoxMeshUnit*) self->units->items.pVoid[i];
		DaoxMeshUnit_UpdateNormTangents( unit, norm, tan );
	}
}
void DaoxMesh_ResetBoundingBox( DaoxMesh *self )
{
	DArray *points = DArray_New( sizeof(DaoxVector3D) );
	daoint i, j, k;
	for(i=0; i<self->units->size; ++i){
		DaoxMeshUnit *unit = (DaoxMeshUnit*) self->units->items.pVoid[i];
		for(j=0; j<unit->vertices->size; ++j){
			DaoxVector3D point = unit->vertices->data.vertices[j].pos;
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
	vertices[0].pos.x = x;
	vertices[0].pos.y = y;
	vertices[1].pos.x = x + dx;
	vertices[1].pos.y = y;
	vertices[2].pos.x = x;
	vertices[2].pos.y = y + dy;
	triangles[0].index[0] = 0;
	triangles[0].index[1] = 1;
	triangles[0].index[2] = 2;
	for(i=0; i<3; ++i){
		vertices[i].pos.z = z;
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



DaoxMeshNode* DaoxMeshNode_New( float x, float y, float z )
{
	DaoxMeshNode *self = (DaoxMeshNode*) dao_calloc( 1, sizeof(DaoxMeshNode) );
	self->pos.x = x;
	self->pos.y = y;
	self->pos.z = z;
	return self;
}
DaoxMeshEdge* DaoxMeshEdge_New( DaoxMeshNode *start, DaoxMeshNode *end )
{
	DaoxMeshEdge *self = (DaoxMeshEdge*) dao_calloc( 1, sizeof(DaoxMeshEdge) );
	self->start = start;
	self->end = end;
	return self;
}
DaoxMeshFace* DaoxMeshFace_New()
{
	DaoxMeshFace *self = (DaoxMeshFace*) dao_calloc( 1, sizeof(DaoxMeshFace) );
	return self;
}
DaoxMeshFrame* DaoxMeshFrame_New()
{
	DaoxMeshFrame *self = (DaoxMeshFrame*) dao_calloc( 1, sizeof(DaoxMeshFrame) );
	self->nodes = DList_New(0);
	self->edges = DList_New(0);
	self->faces = DList_New(0);
	return self;
}
void DaoxMeshFrame_Delete( DaoxMeshFrame *self )
{
	int i;
	for(i=0; i<self->nodes->size; ++i) dao_free( self->nodes->items.pVoid[i] );
	for(i=0; i<self->edges->size; ++i) dao_free( self->edges->items.pVoid[i] );
	for(i=0; i<self->faces->size; ++i) dao_free( self->faces->items.pVoid[i] );
	DList_Delete( self->nodes );
	DList_Delete( self->edges );
	DList_Delete( self->faces );
	dao_free( self );
}
DaoxMeshNode* DaoxMeshFrame_MakeNode( DaoxMeshFrame *self, float x, float y, float z )
{
	DaoxMeshNode *node;
	if( self->usedNodes < self->nodes->size ){
		node = self->nodes->items.pVoid[ self->usedNodes ];
		self->usedNodes += 1;
		return node;
	}
	node = DaoxMeshNode_New( x, y, z );
	DList_Append( self->nodes, node );
	self->usedNodes += 1;
	return node;
}
DaoxMeshEdge* DaoxMeshFrame_MakeEdge( DaoxMeshFrame *self, DaoxMeshNode *start, DaoxMeshNode *end )
{
	DaoxMeshEdge *edge;
	if( self->usedEdges < self->edges->size ){
		edge = self->edges->items.pVoid[ self->usedEdges ];
		edge->start = start;
		edge->end = end;
		self->usedEdges += 1;
		return edge;
	}
	edge = DaoxMeshEdge_New( start, end );
	DList_Append( self->edges, edge );
	self->usedEdges += 1;
	return edge;
}
DaoxMeshFace* DaoxMeshFrame_MakeFace( DaoxMeshFrame *self )
{
	DaoxMeshFace *face;
	if( self->usedFaces < self->faces->size ){
		face = self->faces->items.pVoid[ self->usedFaces ];
		face->splits[0] = face->splits[1] = face->splits[2] = NULL;
		self->usedFaces += 1;
		return face;
	}
	face = DaoxMeshFace_New();
	DList_Append( self->faces, face );
	self->usedFaces += 1;
	return face;
}
void DaoxMeshFrame_Split( DaoxMeshFrame *self, DaoxMeshFace *face )
{
	DaoxMeshNode *node, *nodes[3];
	DaoxMeshEdge *common, *edges[3];
	DaoxMeshFace *center, *side;
	float minlen = DaoxVector3D_Dist( & face->nodes[0]->pos, & face->nodes[1]->pos );
	float maxlen = 0.0;
	int i, maxside = 0;

	for(i=0; i<4; ++i){
		if( face->splits[i] == NULL ) face->splits[i] = DaoxMeshFrame_MakeFace( self );
	}
	for(i=0; i<3; ++i){
		DaoxMeshNode *mid = NULL;
		DaoxMeshEdge *edge = face->edges[i];
		int dir = face->nodes[i] == edge->start;

		if( edge->left ){
			DaoxMeshEdge *left = edge->left;
			mid = left->start == edge->start ? left->end : left->start;
		}else{
			mid = DaoxMeshFrame_MakeNode( self, 0.0, 0.0, 0.0 );
			mid->pos = DaoxVector3D_Interpolate( edge->start->pos, edge->end->pos, 0.5 );
			edge->left = DaoxMeshFrame_MakeEdge( self, edge->start, mid );
			edge->right = DaoxMeshFrame_MakeEdge( self, mid, edge->end );
		}
		face->splits[0]->nodes[i] = mid;
		face->splits[i+1]->nodes[0] = face->nodes[i];
		face->splits[i+1]->nodes[1] = mid;
		face->splits[i+1]->edges[0] = dir ? edge->left : edge->right;
		face->splits[i==2?1:i+2]->nodes[0] = face->nodes[(i+1)%3];
		face->splits[i==2?1:i+2]->nodes[2] = mid;
		face->splits[i==2?1:i+2]->edges[2] = dir ? edge->right : edge->left;
	}
	for(i=0; i<3; ++i){
		DaoxMeshFace *mid = face->splits[0];
		DaoxMeshEdge *edge = DaoxMeshFrame_MakeEdge( self, mid->nodes[i], mid->nodes[(i+1)%3] );
		mid->edges[i] = edge;
		face->splits[i==2?1:i+2]->edges[1] = edge;
	}

	for(i=0; i<3; ++i){
		DaoxVector3D node1 = face->nodes[i]->pos;
		DaoxVector3D node2 = face->nodes[(i+1)%3]->pos;
		float len = DaoxVector3D_Dist( & node1, & node2 );
		if( minlen > len ) minlen = len;
		if( maxlen < len ){
			maxlen = len;
			maxside = i;
		}
	}
	if( maxlen < 1.5*minlen ) return;

	center = face->splits[0];
	memcpy( nodes, center->nodes, 3*sizeof(DaoxMeshNode*) );
	memcpy( edges, center->edges, 3*sizeof(DaoxMeshEdge*) );
	if( maxside != 0 ){
		for(i=0; i<3; ++i){
			center->nodes[i] = nodes[(i+maxside)%3];
			center->edges[i] = edges[(i+maxside)%3];
		}
		memcpy( nodes, center->nodes, 3*sizeof(DaoxMeshNode*) );
		memcpy( edges, center->edges, 3*sizeof(DaoxMeshEdge*) );
	}

	side = face->splits[1+(maxside+2)%3];
	common = side->edges[1];
	common->start = side->nodes[0];
	common->end = center->nodes[0];
	center->nodes[2] = side->nodes[0];
	center->edges[1] = side->edges[2];
	center->edges[2] = common;
	side->nodes[2] = center->nodes[0];
	side->edges[1] = edges[2];
	side->edges[2] = common;
}
void DaoxMeshFrame_Export( DaoxMeshFrame *self, DaoxMeshUnit *unit )
{
	int i;
	for(i=0; i<self->usedNodes; ++i){
		DaoxMeshNode *node = self->nodes->items.pMeshNode[i];
		DaoxVertex *vertex = DArray_PushVertex( unit->vertices, NULL );
		vertex->pos = node->pos;
		vertex->norm = node->norm;
		node->id = i;
	}
	for(i=0; i<self->usedFaces; ++i){
		DaoxMeshFace *face = self->faces->items.pMeshFace[i];
		DaoxTriangle *triangle;
		if( face->splits[0] != NULL ) continue;
		triangle = DArray_PushTriangle( unit->triangles, NULL );
		triangle->index[0] = face->nodes[0]->id;
		triangle->index[1] = face->nodes[1]->id;
		triangle->index[2] = face->nodes[2]->id;
	}
}




static float box_vertices[][3] =
{
	{ 0.5,  0.5, -0.5},
	{ 0.5, -0.5, -0.5},
	{-0.5, -0.5, -0.5},
	{-0.5,  0.5, -0.5},
	{-0.5,  0.5,  0.5},
	{ 0.5,  0.5,  0.5},
	{ 0.5, -0.5,  0.5},
	{-0.5, -0.5,  0.5}
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

DaoxMeshUnit* DaoxMesh_MakeBox( DaoxMesh *self, float wx, float wy, float wz )
{
	int i, j;
	DaoxMeshUnit *unit = DaoxMesh_AddUnit( self );
	for(i=0; i<8; ++i){
		DaoxVertex *vertex = DArray_PushVertex( unit->vertices, NULL );
		vertex->pos.x = wx * box_vertices[i][0];
		vertex->pos.y = wy * box_vertices[i][1];
		vertex->pos.z = wz * box_vertices[i][2];
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
	DaoxMeshUnit_UpdateNormTangents( unit, 1, 0 );
	DaoxMesh_ResetBoundingBox( self );
	return unit;
}

DaoxMeshUnit* DaoxMesh_MakeCube( DaoxMesh *self )
{
	return DaoxMesh_MakeBox( self, 1.0, 1.0, 1.0 );
}


#define XX 0.525731112119133606f
#define YY 0.850650808352039932f

static float icosahedron_vertices[][3] =
{
	{-XX, 0.0,  YY},
	{ XX, 0.0,  YY},
	{-XX, 0.0, -YY},
	{ XX, 0.0, -YY},
	{0.0,  YY,  XX},
	{0.0,  YY, -XX},
	{0.0, -YY,  XX},
	{0.0, -YY, -XX},
	{ YY,  XX, 0.0},
	{-YY,  XX, 0.0},
	{ YY, -XX, 0.0},
	{-YY, -XX, 0.0} 
};
static int icosahedron_faces[][3] =
{
	{ 0,  4,  1 },
	{ 0,  9,  4 },
	{ 9,  5,  4 },
	{ 4,  5,  8 },
	{ 4,  8,  1 },
	{ 8, 10,  1 },
	{ 8,  3, 10 },
	{ 5,  3,  8 },
	{ 5,  2,  3 },
	{ 2,  7,  3 },
	{ 7, 10,  3 },
	{ 7,  6, 10 },
	{ 7, 11,  6 },
	{11,  0,  6 },
	{ 0,  1,  6 },
	{ 6,  1, 10 },
	{ 9,  0, 11 },
	{ 9, 11,  2 },
	{ 9,  2,  5 },
	{ 7,  2, 11 } 
};
void DaoxMeshFrame_MakeIcosahedron( DaoxMeshFrame *self, float rx, float ry, float rz )
{
	DaoxMeshNode *nodes[12] = { NULL };
	DaoxMeshEdge *edges[256] = { NULL };
	int i, j;
	for(i=0; i<12; ++i){
		DaoxMeshNode *node = DaoxMeshFrame_MakeNode( self, 0.0, 0.0, 0.0 );
		node->pos.x = rx * icosahedron_vertices[i][0];
		node->pos.y = ry * icosahedron_vertices[i][1];
		node->pos.z = rz * icosahedron_vertices[i][2];
		nodes[i] = node;
	}
	for(i=0; i<20; ++i){
		DaoxMeshFace *face = DaoxMeshFrame_MakeFace( self );
		int *ids = icosahedron_faces[i];
		for(j=0; j<3; ++j){
			int id1 = ids[j];
			int id2 = ids[(j+1)%3];
			DaoxMeshNode *node1 = nodes[ id1 ];
			DaoxMeshNode *node2 = nodes[ id2 ];
			DaoxMeshEdge *edge = edges[ (id1<<4)|id2 ];
			if( edge == NULL ){
				edge = DaoxMeshFrame_MakeEdge( self, node1, node2 );
				edges[ (id1<<4)|id2 ] = edge;
				edges[ (id2<<4)|id1 ] = edge;
			}
			face->nodes[j] = node1;
			face->edges[j] = edge;
		}
	}
}
void DaoxMeshFrame_MakeSphere( DaoxMeshFrame *self, float radius, int resolution )
{
	int i, j;

	DaoxMeshFrame_MakeIcosahedron( self, radius, radius, radius );
	for(i=0; i<resolution; ++i){
		int usedNodes = self->usedNodes;
		int usedFaces = self->usedFaces;
		for(j=0; j<usedFaces; ++j){
			DaoxMeshFace *face = self->faces->items.pMeshFace[j];
			DaoxMeshFrame_Split( self, face );
		}
		for(j=usedNodes; j<self->usedNodes; ++j){
			DaoxMeshNode *node = self->nodes->items.pMeshNode[j];
			double scale = radius / sqrt( DaoxVector3D_Norm2( & node->pos ) );
			node->pos.x *= scale;
			node->pos.y *= scale;
			node->pos.z *= scale;
		}
	}
	for(i=0; i<self->usedNodes; ++i){
		DaoxMeshNode *node = self->nodes->items.pMeshNode[i];
		node->norm = DaoxVector3D_Normalize( & node->pos );
	}
}
DaoxMeshUnit* DaoxMesh_MakeSphere( DaoxMesh *self, float radius, int resolution )
{
	DaoxMeshUnit *unit = DaoxMesh_AddUnit( self );
	DaoxMeshFrame *meshFrame = DaoxMeshFrame_New();

	DaoxMeshFrame_MakeSphere( meshFrame, radius, resolution );
	DaoxMeshFrame_Export( meshFrame, unit );
	DaoxMeshFrame_Delete( meshFrame );
	DaoxMesh_ResetBoundingBox( self );
	return unit;
}
