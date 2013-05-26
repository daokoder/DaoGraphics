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


int DaoxOBBox3D_Contain( DaoxOBBox3D *self, DaoxVector3D point )
{
	DaoxVector3D X = DaoxVector3D_Sub( & self->X, & self->O );
	DaoxVector3D Y = DaoxVector3D_Sub( & self->Y, & self->O );
	DaoxVector3D Z = DaoxVector3D_Sub( & self->Z, & self->O );
	double x, dx = DaoxVector3D_Norm2( & X );
	double y, dy = DaoxVector3D_Norm2( & Y );
	double z, dz = DaoxVector3D_Norm2( & Z );
	point = DaoxVector3D_Sub( & point, & self->O );
	x = DaoxVector3D_Dot( & X, & point );
	y = DaoxVector3D_Dot( & Y, & point );
	z = DaoxVector3D_Dot( & Z, & point );
	return (x >= 0.0 && x <= dx) && (y >= 0.0 && y <= dy) && (z >= 0.0 && z <= dz);
}
void DaoxOBBox3D_ComputeBoundingBox( DaoxOBBox3D *self, DaoxVector3D points[], int count )
{
	DaoxVector3D xaxis1, xaxis2, yaxis1, yaxis2, zaxis1, zaxis2;
	DaoxVector3D first, second, third, zero = {0.0,0.0,0.0};
	DaoxVector3D xaxis, yaxis, zaxis;
	double xmin, xmax, ymin, ymax, zmin, zmax, margin = 1E-3;
	double max = -0.0, max1 = -0.0, max2 = -0.0;
	daoint i, j;

	self->O = self->X = self->Y = self->Z = zero;
	self->C = zero;
	self->R = 0;
	if( count == 0 ) return;

	first = second = third = points[0];
	for(i=0; i<count; ++i){
		DaoxVector3D point = points[i];
		double dist2 = DaoxVector3D_Dist2( & point, & second );
		if( dist2 > max ){
			max = dist2;
			first = point;
		}
	}
	/* Find the vertex that is furthest from the "first": */
	for(i=0; i<count; ++i){
		DaoxVector3D point = points[i];
		double dist = DaoxVector3D_Dist2( & first, & point );
		if( dist >= max1 ){
			xaxis = point;
			max1 = dist;
		}
	}
	xaxis = DaoxVector3D_Sub( & xaxis, & first );
	max1 = sqrt( max1 );
	xaxis.x /= max1;
	xaxis.y /= max1;
	xaxis.z /= max1;
	/* Find the vertex that is furthest from the line formed by "first" and "xaxis": */
	for(i=0; i<count; ++i){
		DaoxVector3D point = points[i];
		DaoxVector3D point2 = DaoxVector3D_Sub( & point, & first );
		double dot = DaoxVector3D_Dot( & point2, & xaxis );
		DaoxVector3D sub = DaoxVector3D_Scale( & xaxis, dot );
		DaoxVector3D pp = DaoxVector3D_Sub( & point2, & sub );
		double dist = DaoxVector3D_Norm2( & pp );
		if( dist >= max2 ){
			yaxis = pp;
			max2 = dist;
		}
	}
	max2 = sqrt( max2 );
	yaxis.x /= max2;
	yaxis.y /= max2;
	yaxis.z /= max2;
	zaxis = DaoxVector3D_Cross( & xaxis, & yaxis );

	//printf( "dot: %9f\n", DaoxVector3D_Dot( & xaxis, & yaxis ) );

	/* Construct the bounding box aligned to the new "xaxis", "yaxis" and "zaxis": */
	xmin = xmax = ymin = ymax = zmin = zmax = 0.0;
	for(i=0; i<count; ++i){
		DaoxVector3D point = points[i];
		DaoxVector3D point2 = DaoxVector3D_Sub( & point, & first );
		double dotx = DaoxVector3D_Dot( & point2, & xaxis );
		double doty = DaoxVector3D_Dot( & point2, & yaxis );
		double dotz = DaoxVector3D_Dot( & point2, & zaxis );
		if( dotx <= xmin ) xmin = dotx;
		if( doty <= ymin ) ymin = doty;
		if( dotz <= zmin ) zmin = dotz;
		if( dotx >= xmax ) xmax = dotx;
		if( doty >= ymax ) ymax = doty;
		if( dotz >= zmax ) zmax = dotz;
	}
	xmin -= margin;
	ymin -= margin;
	zmin -= margin;
	xmax += margin;
	ymax += margin;
	zmax += margin;
	xaxis1 = DaoxVector3D_Scale( & xaxis, xmin );
	yaxis1 = DaoxVector3D_Scale( & yaxis, ymin );
	zaxis1 = DaoxVector3D_Scale( & zaxis, zmin );
	xaxis2 = DaoxVector3D_Scale( & xaxis, xmax - xmin );
	yaxis2 = DaoxVector3D_Scale( & yaxis, ymax - ymin );
	zaxis2 = DaoxVector3D_Scale( & zaxis, zmax - zmin );
	self->O = first;
	self->O = DaoxVector3D_Add( & self->O, & xaxis1 );
	self->O = DaoxVector3D_Add( & self->O, & yaxis1 );
	self->O = DaoxVector3D_Add( & self->O, & zaxis1 );
	self->X = DaoxVector3D_Add( & self->O, & xaxis2 );
	self->Y = DaoxVector3D_Add( & self->O, & yaxis2 );
	self->Z = DaoxVector3D_Add( & self->O, & zaxis2 );

	xaxis2 = DaoxVector3D_Scale( & xaxis2, 0.5 );
	yaxis2 = DaoxVector3D_Scale( & yaxis2, 0.5 );
	zaxis2 = DaoxVector3D_Scale( & zaxis2, 0.5 );
	self->C = self->O;
	self->C = DaoxVector3D_Add( & self->C, & xaxis2 );
	self->C = DaoxVector3D_Add( & self->C, & yaxis2 );
	self->C = DaoxVector3D_Add( & self->C, & zaxis2 );
	self->R = sqrt( DaoxVector3D_Dist2( & self->C, & self->O ) );

	//return;

	for(i=0; i<count; ++i){
		DaoxVector3D point = points[i];
		if( DaoxOBBox3D_Contain( self, point ) == 0 ) printf( "%5i\n", i );
	}
}



DaoxMeshChunk* DaoxMeshChunk_New( DaoxMeshUnit *unit )
{
	DaoxMeshChunk *self = (DaoxMeshChunk*) dao_calloc( 1, sizeof(DaoxMeshChunk) );
	self->triangles = DaoxPlainArray_New( sizeof(int) );
	self->unit = unit;
	self->parent = self->left = self->right = NULL;
	return self;
}
void DaoxMeshChunk_Delete( DaoxMeshChunk *self )
{
	if( self->left ) DaoxMeshChunk_Delete( self->left );
	if( self->right ) DaoxMeshChunk_Delete( self->right );
	DaoxPlainArray_Delete( self->triangles );
	dao_free( self );
}
void DaoxMeshChunk_ResetBoundingBox( DaoxMeshChunk *self, DaoxPlainArray *buffer )
{
	//printf( "DaoxMeshChunk_ResetBoundingBox: %i\n", self->triangles->size );
	DaoxVertex *vertices = self->unit->vertices->pod.vertices;
	DaoxTriangle *triangles = self->unit->triangles->pod.triangles;
	daoint i, j;

	DaoxPlainArray_ResetSize( buffer, 0 );
	for(i=0; i<self->triangles->size; ++i){
		DaoxTriangle triangle = triangles[ self->triangles->pod.ints[i] ];
		for(j=0; j<3; ++j){
			DaoxVector3D *point = (DaoxVector3D*) DaoxPlainArray_Push( buffer );
			*point = vertices[triangle.index[j]].point;
		}
	}
	DaoxOBBox3D_ComputeBoundingBox( & self->obbox, buffer->pod.vectors3d, buffer->size );
}




DaoxMeshUnit* DaoxMeshUnit_New()
{
	DaoxMeshUnit *self = (DaoxMeshUnit*) dao_calloc( 1, sizeof(DaoxMeshUnit) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_mesh_unit );
	self->vertices = DaoxPlainArray_New( sizeof(DaoxVertex) );
	self->triangles = DaoxPlainArray_New( sizeof(DaoxTriangle) );
	self->tree = NULL;
	self->mesh = NULL;
	self->material = NULL;
	return self;
}
void DaoxMeshUnit_Delete( DaoxMeshUnit *self )
{
	if( self->tree ) DaoxMeshChunk_Delete( self->tree );
	DaoxPlainArray_Delete( self->vertices );
	DaoxPlainArray_Delete( self->triangles );
	DaoGC_DecRC( (DaoValue*) self->mesh );
	DaoGC_DecRC( (DaoValue*) self->material );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}
void DaoxMeshUnit_SetMaterial( DaoxMeshUnit *self, DaoxMaterial *material )
{
	DaoGC_ShiftRC( (DaoValue*) material, (DaoValue*) self->material );
	self->material = material;
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
	DaoxVertex *vertices = self->vertices->pod.vertices;
	DaoxTriangle *triangles = self->triangles->pod.triangles;
	DaoxPlainArray *points = DaoxPlainArray_New( sizeof(DaoxVector3D) );
	DArray *nodes = DArray_New(0);
	daoint i, j, half, capacity = 0;
	if( maxtriangles <= 0 ) maxtriangles = MIN_MESH_CHUNK;
	if( self->tree == NULL ) self->tree = DaoxMeshChunk_New( self );
	self->tree->triangles->size = 0;
	for(i=0; i<self->triangles->size; ++i) DaoxPlainArray_PushInt( self->tree->triangles, i );
	printf( "DaoxMeshUnit_UpdateTree: %i\n", maxtriangles );
	DArray_Append( nodes, self->tree );
	for(i=0; i<nodes->size; ++i){
		DaoxMeshChunk *node = (DaoxMeshChunk*) nodes->items.pVoid[i];
		DaoxVector3D OX, OY, OZ, longest;
		int *ids = node->triangles->pod.ints;
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
			DaoxTriangle triangle = triangles[ node->triangles->pod.ints[j] ];
			DaoxVector3D A = vertices[ triangle.index[0] ].point;
			DaoxVector3D B = vertices[ triangle.index[1] ].point;
			DaoxVector3D C = vertices[ triangle.index[2] ].point;
			DaoxVector3D AB = DaoxVector3D_Add( & A, & B );
			DaoxVector3D ABC = DaoxVector3D_Add( & AB, & C );
			ABC = DaoxVector3D_Scale( & ABC, 0.333333333 );
			ABC = DaoxVector3D_Sub( & ABC, & node->obbox.O );
			sorting[j].index = node->triangles->pod.ints[j];
			sorting[j].value = DaoxVector3D_Dot( & ABC, & OX );
		}
		TriangleInfo_QuickSort( sorting, 0, node->triangles->size - 1 );
		if( node->left == NULL ) node->left = DaoxMeshChunk_New( self );
		if( node->right == NULL ) node->right = DaoxMeshChunk_New( self );
		node->left->triangles->size = 0;
		node->right->triangles->size = 0;
		half = node->triangles->size/2;
		for(j=0; j<half; ++j){
			DaoxPlainArray_PushInt( node->left->triangles, sorting[j].index );
		}
		for(j=half; j<node->triangles->size; ++j){
			DaoxPlainArray_PushInt( node->right->triangles, sorting[j].index );
		}
		DArray_Append( nodes, node->left );
		DArray_Append( nodes, node->right );
	}
	self->obbox = self->tree->obbox;
	DaoxPlainArray_Delete( points );
	DArray_Delete( nodes );
}




DaoxMesh* DaoxMesh_New()
{
	DaoxMesh *self = (DaoxMesh*) dao_calloc( 1, sizeof(DaoxMesh) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_mesh );
	self->units = DArray_New(D_VALUE);
	return self;
}
void DaoxMesh_Delete( DaoxMesh *self )
{
	DArray_Delete( self->units );
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
	DArray_Append( self->units, unit );
	return unit;
}
void DaoxMesh_UpdateTree( DaoxMesh *self, int maxtriangles )
{
	daoint i;
	for(i=0; i<self->units->size; ++i){
		DaoxMeshUnit *unit = (DaoxMeshUnit*) self->units->items.pVoid[i];
		DaoxMeshUnit_UpdateTree( unit, maxtriangles );
	}
}
void DaoxMesh_ResetBoundingBox( DaoxMesh *self )
{
	DaoxPlainArray *points = DaoxPlainArray_New( sizeof(DaoxVector3D) );
	daoint i, j, k;
	for(i=0; i<self->units->size; ++i){
		DaoxMeshUnit *unit = (DaoxMeshUnit*) self->units->items.pVoid[i];
		for(j=0; j<unit->triangles->size; ++j){
			DaoxTriangle *triangle = unit->triangles->pod.triangles + j;
			for(k=0; k<3; ++k){
				DaoxVector3D *point = DaoxPlainArray_PushVector3D( points );
				*point = unit->vertices->pod.vertices[triangle->index[k]].point;
			}
		}
	}
	DaoxOBBox3D_ComputeBoundingBox( & self->obbox, points->pod.vectors3d, points->size );
	DaoxPlainArray_Delete( points );
}
static void DaoxMesh_MakeTriangle( DaoxMesh *self, int c, float x, float y, float dx, float dy, float z )
{
	DaoxMeshUnit *unit = DaoxMesh_AddUnit( self );
	DaoxMaterial *material = DaoxMaterial_New();
	DaoxVertex *vertices = unit->vertices->pod.vertices;
	DaoxTriangle *triangles = unit->triangles->pod.triangles;
	DaoxVector3D norm = {0.0,0.0,1.0};
	int i;

	DaoxMeshUnit_SetMaterial( unit, material );
	DaoxPlainArray_Resize( unit->vertices, 3 );
	DaoxPlainArray_Resize( unit->triangles, 1 );
	vertices = unit->vertices->pod.vertices;
	triangles = unit->triangles->pod.triangles;
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
	{-0.5, -0.5,  0.5},
	{-0.5, -0.5, -0.5},
	{-0.5,  0.5, -0.5},
	{-0.5,  0.5,  0.5},
	{ 0.5, -0.5,  0.5},
	{ 0.5, -0.5, -0.5},
	{ 0.5,  0.5, -0.5},
	{ 0.5,  0.5,  0.5}
};
static int box_faces[][4] =
{
	{ 4, 3, 2, 1 },
	{ 2, 6, 5, 1 },
	{ 3, 7, 6, 2 },
	{ 8, 7, 3, 4 },
	{ 5, 8, 4, 1 },
	{ 6, 7, 8, 5 }
};

void DaoxMesh_MakeBoxObject( DaoxMesh *self )
{
	int i, j;
	DaoxMeshUnit *unit = DaoxMesh_AddUnit( self );
	unit->vertices->size = 0;
	unit->triangles->size = 0;
	for(i=0; i<8; ++i){
		DaoxVertex *vertex = DaoxPlainArray_PushVertex( unit->vertices );
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
			DaoxTriangle *triangle = DaoxPlainArray_PushTriangle( unit->triangles );
			triangle->index[0] = face[0] - 1;
			triangle->index[1] = face[j-1] - 1;
			triangle->index[2] = face[j] - 1;
		}
	}
}



