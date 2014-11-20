/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2014, Limin Fu
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

#include"dao_terrain.h"
#include"daoNumtype.h"



DaoxTerrainParams* DaoxTerrainParams_New()
{
	DaoxTerrainParams *self = (DaoxTerrainParams*) dao_calloc( 1, sizeof(DaoxTerrainParams) );
	self->resolution = 1.0;
	self->amplitude = 1.0;
	self->faultScale = 1.0;
	return self;
}


DaoxTerrainPoint* DaoxTerrainPoint_New( float x, float y, float z )
{
	DaoxTerrainPoint *self = (DaoxTerrainPoint*) dao_calloc( 1, sizeof(DaoxTerrainPoint) );
	self->pos.x = x;
	self->pos.y = y;
	self->pos.z = z;
	return self;
}
DaoxTerrainBorder* DaoxTerrainBorder_New( DaoxTerrainPoint *start, DaoxTerrainPoint *end )
{
	DaoxTerrainBorder *self = (DaoxTerrainBorder*) dao_calloc( 1, sizeof(DaoxTerrainBorder) );
	self->start = start;
	self->end = end;
	return self;
}

DaoxTerrainTriangle* DaoxTerrainTriangle_New()
{
	DaoxTerrainTriangle *self = (DaoxTerrainTriangle*) dao_calloc( 1, sizeof(DaoxTerrainTriangle) );
	return self;
}
void DaoxTerrainTriangle_DeleteSplits( DaoxTerrainTriangle *self );
void DaoxTerrainTriangle_Delete( DaoxTerrainTriangle *self )
{
	DaoxTerrainTriangle_DeleteSplits( self );
	dao_free( self );
}
void DaoxTerrainTriangle_DeleteSplits( DaoxTerrainTriangle *self )
{
	int i;
	for(i=0; i<4; ++i){
		if( self->splits[i] ){
			DaoxTerrainTriangle_Delete( self->splits[i] );
			self->splits[i] = NULL;
		}
	}
}

DaoxTerrainBlock* DaoxTerrainBlock_New( int sides )
{
	DaoxTerrainBlock *self = (DaoxTerrainBlock*) dao_calloc( 1, sizeof(DaoxTerrainBlock) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_terrain_block );
	self->sides = sides;
	return self;
}
void DaoxTerrainBlock_Delete( DaoxTerrainBlock *self )
{
	int i;
	DaoCstruct_Free( (DaoCstruct*) self );
	for(i=0; i<self->sides; ++i){
		if( self->splits[i] ) DaoxTerrainTriangle_Delete( self->splits[i] );
	}
	dao_free( self );
}


DaoxTerrain* DaoxTerrain_New()
{
	DaoxTerrain *self = (DaoxTerrain*) dao_calloc( 1, sizeof(DaoxTerrain) );
	DaoxSceneNode_Init( (DaoxSceneNode*) self, daox_type_terrain );
	self->tiles = DList_New(0);
	self->borders = DList_New(0);
	self->points = DList_New(0);
	self->mesh = DaoxMesh_New();
	GC_IncRC( self->mesh );
	self->circles = 2;
	self->radius = 5.0;
	self->height = 1.0;
	self->depth = 1.0;
	self->textureScale = 1.0;
	self->buffer = DList_New(0);
	return self;
}
void DaoxTerrain_Delete( DaoxTerrain *self )
{
	GC_DecRC( self->mesh );
}


void DaoxTerrain_SetRectAutoBlocks( DaoxTerrain *self, float width, float length )
{
	int i, j, xnum, ynum, M = 1, N = 1;
	float epsilon = (width + length) / 100.0;
	float large = width, small = length;

	/*
	// Compute the terrain division such that the difference between
	// the width and the length of each block is sufficiently small.
	*/
	if( epsilon > 1.0 ) epsilon = 1.0;
	if( width < length ){
		large = length;
		small = width;
	}
	for(M=1; M<100; ++M){
		N = (large * M) / small;
		if( fabs( large/N - small/M ) < epsilon ) break;
	}
	xnum = N;
	ynum = M;
	if( width < length ){
		xnum = M;
		ynum = N;
	}
	printf( "xnum = %i, ynum = %i; %g %g\n", xnum, ynum, width/xnum, length/ynum );
	self->shape = DAOX_RECT_TERRAIN;
	self->rows = ynum;
	self->columns = xnum;
	self->xbsize = width / xnum;
	self->ybsize = length / ynum;
	self->width = width;
	self->length = length;
}
void DaoxTerrain_SetRectBlocks( DaoxTerrain *self, int rows, int columns, float blocksize )
{
	self->shape = DAOX_RECT_TERRAIN;
	self->rows = rows;
	self->columns = columns;
	self->xbsize = self->ybsize = blocksize;
	self->width = rows * blocksize;
	self->length = columns * blocksize;
}
void DaoxTerrain_SetHexBlocks( DaoxTerrain *self, int circles, float blocksize )
{
	double sin60 = sin( M_PI / 3.0 );
	double epsilon = 1E-6 * self->radius;
	self->shape = DAOX_HEX_TERRAIN;
	self->circles = circles;
	self->radius = blocksize;
	self->width = 2.0 * (2*self->circles - 1) * self->radius * sin60 + epsilon;
	self->length = 3.0 * (self->circles - 1) * self->radius + 2.0 * self->radius + epsilon;
}
double DaoArray_Max( DaoArray *self )
{
	int i;
	double max = self->size ? self->data.f[0] : 0.0;;
	for(i=0; i<self->size; ++i){
		if( max < self->data.f[i] ) max = self->data.f[i];
	}
	return max;
}
void DaoxTerrain_SetHeightmap( DaoxTerrain *self, DaoArray *heightmap )
{
	GC_Assign( & self->heightmap, heightmap );
	self->height = DaoArray_Max( heightmap );
}

DaoxTerrainBlock* DaoxTerrain_GetBlock( DaoxTerrain *self, int side, int radius, int offset )
{
	DaoxTerrainBlock *unit = self->first;
	if( self->first == NULL ) return NULL;
	if( side < 0 || side >= 6 ) return NULL;
	while( radius > 0 && unit->neighbors[side] ){
		unit = unit->neighbors[side];
		radius -= 1;
	}
	if( radius > 0 ) return NULL;
	while( offset > 0 && unit->next ){
		if( unit == self->last ) return NULL;
		unit = unit->next;
		offset -= 1;
	}
	if( offset > 0 ) return NULL;
	return unit;
}
void DaoxTerrainBlock_SetRectNeighbor( DaoxTerrainBlock *self, DaoxTerrainBlock *unit, int side )
{
	self->neighbors[side] = unit;
	unit->neighbors[(side+2)%4] = self;
}
void DaoxTerrainBlock_SetHexNeighbor( DaoxTerrainBlock *self, DaoxTerrainBlock *unit, int side )
{
	self->neighbors[side] = unit;
	unit->neighbors[(side+3)%6] = self;
}
DaoxTerrainPoint* DaoxTerrain_MakePoint( DaoxTerrain *self, float x, float y, float z )
{
	DaoxTerrainPoint *point = DaoxTerrainPoint_New( x, y, z );
	DList_Append( self->points, point );
	return point;
}
void DaoxTerrain_AddCircle( DaoxTerrain *self )
{
	DaoxTerrainBlock *unit, *unit2, *unit3;
	DaoxTerrainBlock *start = self->first;
	double cdist = 2.0 * self->radius * cos( M_PI / 6.0 );
	double angle = 2.0 * M_PI / 3.0;
	int i, j, k, side = 2, count = 1;
	while( start->neighbors[0] ){
		start = start->neighbors[0];
		count += 1;
	}
	unit = self->last->next;
	if( unit == NULL ){
		unit = DaoxTerrainBlock_New( 6 );
		unit->mesh = DaoxMesh_AddUnit( self->mesh );
		unit->center = DaoxTerrain_MakePoint( self, 0.0, 0.0, 0.0 );
		self->last->next = unit;
	}
	self->last = unit;
	for(j=0; j<6; ++j) unit->neighbors[j] = NULL;
	unit->center->pos = start->center->pos;
	unit->center->pos.x += 2.0 * self->radius * cos( M_PI / 6.0 );
	DaoxTerrainBlock_SetHexNeighbor( start, unit, 0 );
	start = unit;
	for(i=0; i<6; i+=1){
		if( i == 5 ) count -= 1;
		for(j=0; j<count; j+=1){
			unit = self->last->next;
			if( unit == NULL ){
				unit = DaoxTerrainBlock_New( 6 );
				unit->mesh = DaoxMesh_AddUnit( self->mesh );
				unit->center = DaoxTerrain_MakePoint( self, 0.0, 0.0, 0.0 );
				self->last->next = unit;
			}
			self->last = unit;
			for(k=0; k<6; ++k) unit->neighbors[k] = NULL;
			unit->center->pos.x = start->center->pos.x + cdist * cos( angle );
			unit->center->pos.y = start->center->pos.y + cdist * sin( angle );
			DaoxTerrainBlock_SetHexNeighbor( start, unit, side );
			k = (side + 1) % 6;
			unit2 = start->neighbors[k];
			DaoxTerrainBlock_SetHexNeighbor( unit2, unit, (k + 3 + 1) % 6 );
			k = (k + 3 + 2) % 6;
			unit3 = unit2->neighbors[k];
			if( unit3 ) DaoxTerrainBlock_SetHexNeighbor( unit3, unit, (k + 3 + 1) % 6 );
			start = unit;
		}
		angle += M_PI / 3.0;
		side = (side + 1) % 6;
	}
}
void DaoxTerrain_InitBlock( DaoxTerrain *self, DaoxTerrainBlock *unit )
{
	int i;

	for(i=0; i<unit->sides; ++i){
		if( unit->neighbors[i] ){
			unit->borders[i] = unit->neighbors[i]->borders[(i+unit->sides/2)%unit->sides];
		}
	}
	for(i=0; i<unit->sides; ++i){
		float a1 = i * M_PI / (unit->sides/2) - M_PI / unit->sides;
		float a2 = i * M_PI / (unit->sides/2) + M_PI / unit->sides;
		float x1 = unit->center->pos.x + self->radius * cos( a1 );
		float y1 = unit->center->pos.y + self->radius * sin( a1 );
		float x2 = unit->center->pos.x + self->radius * cos( a2 );
		float y2 = unit->center->pos.y + self->radius * sin( a2 );
		if( self->shape == DAOX_RECT_TERRAIN ){
			x1 = x2 = unit->center->pos.x;
			y1 = y2 = unit->center->pos.y;
			switch( i ){
			case 0 : x1 += 0.5 * self->xbsize; y1 -= 0.5 * self->ybsize; break;
			case 1 : x1 += 0.5 * self->xbsize; y1 += 0.5 * self->ybsize; break;
			case 2 : x1 -= 0.5 * self->xbsize; y1 += 0.5 * self->ybsize; break;
			case 3 : x1 -= 0.5 * self->xbsize; y1 -= 0.5 * self->ybsize; break;
			}
			switch( i ){
			case 0 : x2 += 0.5 * self->xbsize; y2 += 0.5 * self->ybsize; break;
			case 1 : x2 -= 0.5 * self->xbsize; y2 += 0.5 * self->ybsize; break;
			case 2 : x2 -= 0.5 * self->xbsize; y2 -= 0.5 * self->ybsize; break;
			case 3 : x2 += 0.5 * self->xbsize; y2 -= 0.5 * self->ybsize; break;
			}
		}
		DaoxVector2D p1 = DaoxVector2D_XY( x1, y1 );
		DaoxVector2D p2 = DaoxVector2D_XY( x2, y2 );
		DaoxTerrainBorder *prev = unit->borders[(i+unit->sides-1) % unit->sides];
		DaoxTerrainBorder *next = unit->borders[(i+1) % unit->sides];
		DaoxTerrainPoint *start = NULL;
		DaoxTerrainPoint *end = NULL;
		if( unit->borders[i] ) continue;
		if( prev ){
			DaoxVector2D q1 = DaoxVector2D_Vector3D( prev->start->pos );
			DaoxVector2D q2 = DaoxVector2D_Vector3D( prev->end->pos );
			double d1 = DaoxVector2D_Dist( q1, p1 );
			double d2 = DaoxVector2D_Dist( q2, p1 );
			start = d1 < d2 ? prev->start : prev->end;
		}
		if( next ){
			DaoxVector2D q1 = DaoxVector2D_Vector3D( next->start->pos );
			DaoxVector2D q2 = DaoxVector2D_Vector3D( next->end->pos );
			double d1 = DaoxVector2D_Dist( q1, p2 );
			double d2 = DaoxVector2D_Dist( q2, p2 );
			end = d1 < d2 ? next->start : next->end;
		}
		if( start == NULL ) start = DaoxTerrain_MakePoint( self, x1, y1, 0.0 );
		if( end == NULL ) end = DaoxTerrain_MakePoint( self, x2, y2, 0.0 );
		unit->borders[i] = DaoxTerrainBorder_New( start, end );
	}
	for(i=0; i<unit->sides; ++i){
		DaoxTerrainBorder *border = unit->borders[i];
		float a1 = i * M_PI / (unit->sides/2) - M_PI / unit->sides;
		float x1 = unit->center->pos.x + self->radius * cos( a1 );
		float y1 = unit->center->pos.y + self->radius * sin( a1 );
		if( self->shape == DAOX_RECT_TERRAIN ){
			x1 = unit->center->pos.x;
			y1 = unit->center->pos.y;
			switch( i ){
			case 0 : x1 += 0.5 * self->xbsize; y1 -= 0.5 * self->ybsize; break;
			case 1 : x1 += 0.5 * self->xbsize; y1 += 0.5 * self->ybsize; break;
			case 2 : x1 -= 0.5 * self->xbsize; y1 += 0.5 * self->ybsize; break;
			case 3 : x1 -= 0.5 * self->xbsize; y1 -= 0.5 * self->ybsize; break;
			}
		}
		DaoxVector2D p1 = DaoxVector2D_XY( x1, y1 );
		DaoxVector2D q1 = DaoxVector2D_Vector3D( border->start->pos );
		DaoxVector2D q2 = DaoxVector2D_Vector3D( border->end->pos );

		double d1 = DaoxVector2D_Dist( q1, p1 );
		double d2 = DaoxVector2D_Dist( q2, p1 );
		DaoxTerrainPoint *start = d1 < d2 ? border->start : border->end;

		unit->spokes[i] = DaoxTerrainBorder_New( unit->center, start );
	}

	for(i=0; i<unit->sides; ++i){
		DaoxTerrainTriangle *triangle = unit->splits[i];
		if( triangle == NULL ) unit->splits[i] = triangle = DaoxTerrainTriangle_New();
		triangle->borders[0] = unit->spokes[i];
		triangle->borders[1] = unit->borders[i];
		triangle->borders[2] = unit->spokes[(i+1)%unit->sides];
		triangle->points[0] = unit->center;
		triangle->points[1] = unit->spokes[i]->end;
		triangle->points[2] = unit->spokes[(i+1)%unit->sides]->end;
	}
}
void DaoxTerrain_InitRectBlocks( DaoxTerrain *self )
{
	DaoxTerrainBlock *unit;
	int i, j;
	for(i=0; i<self->columns; ++i){
		DaoxTerrainBlock *prev = NULL;
		float x1 = i * self->xbsize - 0.5 * self->width;
		float x2 = x1 + self->xbsize;
		for(j=0; j<self->rows; ++j){
			float y1 = j * self->ybsize - 0.5 * self->length;
			float y2 = y1 + self->ybsize;
			unit = DaoxTerrainBlock_New( 4 );
			unit->center = DaoxTerrain_MakePoint( self, 0.5*(x1 + x2), 0.5*(y1 + y2), 0.0 );
			unit->mesh = DaoxMesh_AddUnit( self->mesh );
			if( prev ){
				DaoxTerrainBlock_SetRectNeighbor( unit, prev, 3 );
				prev = prev->neighbors[2];
				if( prev ) DaoxTerrainBlock_SetRectNeighbor( unit, prev->neighbors[1], 2 );
			}else if( self->first ){
				prev = self->first;
				while( prev->neighbors[0] ) prev = prev->neighbors[0];
				DaoxTerrainBlock_SetRectNeighbor( unit, prev, 2 );
			}
			prev = unit;
			if( self->first == NULL ){
				self->first = self->last = unit;
			}else{
				self->last->next = unit;
				self->last = unit;
			}
		}
	}
	for(unit=self->first; unit!=NULL; unit=unit->next){
		DaoxTerrain_InitBlock( self, unit );
	}
}
void DaoxTerrain_InitHexBlocks( DaoxTerrain *self )
{
	DaoxTerrainBlock *unit;
	int i, j;
	if( self->first == NULL ){
		unit = DaoxTerrainBlock_New( 6 );
		unit->center = DaoxTerrain_MakePoint( self, 0.0, 0.0, 0.0 );
		unit->mesh = DaoxMesh_AddUnit( self->mesh );
		self->first = unit;
	}
	self->last = self->first;
	for(i=0; i<6; ++i) self->first->neighbors[i] = NULL;
	for(i=1; i<self->circles; ++i) DaoxTerrain_AddCircle( self );
	for(unit=self->first; unit!=self->last->next; unit=unit->next){
		DaoxTerrain_InitBlock( self, unit );
	}
}
void DaoxTerrain_InitBlocks( DaoxTerrain *self )
{
	if( self->shape == DAOX_RECT_TERRAIN ){
		DaoxTerrain_InitRectBlocks( self );
	}else{
		DaoxTerrain_InitHexBlocks( self );
	}
}

static void DList_Reverse( DList *self )
{
	int i;
	for(i=0; i<self->size/2; ++i){
		void *p = self->items.pVoid[i];
		self->items.pVoid[i] = self->items.pVoid[self->size-1-i];
		self->items.pVoid[self->size-1-i] = p;
	}
}
void DaoxTerrainBorder_GetPoints( DaoxTerrainBorder *self, DList *points )
{
	if( self->left ){
		DaoxTerrainBorder_GetPoints( self->left, points );
		DaoxTerrainBorder_GetPoints( self->right, points );
		return;
	}
	DList_Append( points, self->end );
}
DaoxTerrainTriangle* DaoxTerrain_LocateTriangle( DaoxTerrain *self, DaoxTerrainTriangle *triangle, float x, float y )
{
	int i;
	DaoxVector2D query = DaoxVector2D_XY( x, y );
	DaoxVector2D points[3];

	for(i=0; i<3; ++i) points[i] = DaoxVector2D_Vector3D( triangle->points[i]->pos );
	if( DaoxTriangle_Contain( points[0], points[1], points[2], query ) == 0 ) return NULL;
	if( triangle->splits[0] == NULL ) return triangle;
	for(i=0; i<4; ++i){
		DaoxTerrainTriangle *sub = DaoxTerrain_LocateTriangle( self, triangle->splits[i], x, y );
		if( sub ) return sub;
	}
	return triangle;
}
DaoxTerrainTriangle* DaoxTerrain_Locate( DaoxTerrain *self, float x, float y )
{
	DaoxTerrainBlock *unit;
	DaoxTerrainTriangle *triangle;
	int i;
	for(unit=self->first; unit!=self->last->next; unit=unit->next){
		for(i=0; i<unit->sides; ++i){
			triangle = DaoxTerrain_LocateTriangle( self, unit->splits[i], x, y );
			if( triangle ) return triangle;
		}
	}
	return NULL;
}
float DaoxTerrain_Interpolate( DaoxTerrainPoint *A, DaoxTerrainPoint *B, DaoxTerrainPoint *C, float x, float y )
{
	DaoxVector2D P = DaoxVector2D_XY( x, y );
	DaoxVector2D VA = DaoxVector2D_Vector3D( A->pos ); ;
	DaoxVector2D VB = DaoxVector2D_Vector3D( B->pos ); ;
	DaoxVector2D VC = DaoxVector2D_Vector3D( C->pos ); ;
	double PAB = fabs( DaoxTriangle_Area( P, VA, VB ) );
	double PBC = fabs( DaoxTriangle_Area( P, VB, VC ) );
	double PCA = fabs( DaoxTriangle_Area( P, VC, VA ) );
	double sum = PBC * A->pos.z + PCA * B->pos.z + PAB * C->pos.z;
	return sum / (PAB + PBC + PCA + EPSILON);
}
float DaoxTerrain_GetHeight( DaoxTerrain *self, float x, float y )
{
	int i, k = 0, divs = 0;
	DaoxVector2D P = DaoxVector2D_XY( x, y );
	DaoxTerrainTriangle *T = DaoxTerrain_Locate( self, x, y );

	if( T == NULL ) return 0.0;
	/*
	// No need to handle the case where there are vertices on one of the edges.
	// Since these vertices belong only to some of the neighboring triangles,
	// this means the edge and this triangle is relative smooth, and the edge
	// is divided only because of roughness of neighboring region, which must
	// lie at the other side of the edge. So interpolating using this undivided
	// triangle should be good enough.
	*/
	return DaoxTerrain_Interpolate( T->points[0], T->points[1], T->points[2], x, y );
}
float DaoArray_InterpolateValue( DaoArray *self, float width, float length, float x, float y )
{
	int mapWidth = self->dims[1];
	int mapHeight = self->dims[0];
	float hmx = (x * (mapWidth-1)) / width;
	float hmy = (y * (mapHeight-1)) / length;
	int hmx1 = (int) hmx, hmx2 = hmx1 + (hmx > hmx1);
	int hmy1 = (int) hmy, hmy2 = hmy1 + (hmy > hmy1);
	float pix0 = self->data.f[hmy1*mapWidth + hmx1];
	float pix1 = self->data.f[hmy1*mapWidth + hmx2];
	float pix2 = self->data.f[hmy2*mapWidth + hmx2];
	float pix3 = self->data.f[hmy2*mapWidth + hmx1];
	float alpha = hmx - hmx1, alpha2 = 1.0 - alpha;
	float beta = hmy - hmy1, beta2 = 1.0 - beta;
	float pix = alpha2 * beta2 * pix0 + alpha * beta * pix2;
	pix += alpha * beta2 * pix1 + alpha2 * beta * pix3;
	return pix;
}
static float DaoxTerrain_GetHMHeight( DaoxTerrain *self, float x, float y )
{
	double width = self->width;
	double length = self->length;
	if( self->heightmap == NULL ) return 0.0;
	return DaoArray_InterpolateValue( self->heightmap, width, length, x + 0.5*width, y + 0.5*length );
}

void DaoxTerrain_Split( DaoxTerrain *self, DaoxTerrainBlock *unit, DaoxTerrainTriangle *triangle )
{
	int i;
	for(i=0; i<4; ++i){
		if( triangle->splits[i] == NULL ) triangle->splits[i] = DaoxTerrainTriangle_New();
	}
	for(i=0; i<3; ++i){
		DaoxTerrainPoint *mid = NULL;
		DaoxTerrainBorder *border = triangle->borders[i];
		int dir = triangle->points[i] == border->start;

		if( border->left ){
			DaoxTerrainBorder *left = border->left;
			mid = left->start == border->start ? left->end : left->start;
		}else{
			mid = DaoxTerrain_MakePoint( self, 0.0, 0.0, 0.0 );
			mid->pos = DaoxVector3D_Interpolate( border->start->pos, border->end->pos, 0.5 );
			if( self->heightmap != NULL ){
				mid->pos.z = DaoxTerrain_GetHMHeight( self, mid->pos.x, mid->pos.y );
			}
			border->left = DaoxTerrainBorder_New( border->start, mid );
			border->right = DaoxTerrainBorder_New( mid, border->end );
		}
		triangle->splits[0]->points[i] = mid;
		triangle->splits[i+1]->points[0] = triangle->points[i];
		triangle->splits[i+1]->points[1] = mid;
		triangle->splits[i+1]->borders[0] = dir ? border->left : border->right;
		triangle->splits[i==2?1:i+2]->points[0] = triangle->points[(i+1)%3];
		triangle->splits[i==2?1:i+2]->points[2] = mid;
		triangle->splits[i==2?1:i+2]->borders[2] = dir ? border->right : border->left;
	}
	for(i=0; i<3; ++i){
		DaoxTerrainTriangle *mid = triangle->splits[0];
		DaoxTerrainBorder *border = DaoxTerrainBorder_New( mid->points[i], mid->points[(i+1)%3] );
		mid->borders[i] = border;
		triangle->splits[i==2?1:i+2]->borders[1] = border;
	}
}
void DaoxTerrain_FindMinMaxPixel( DaoxTerrain *self, DaoxVector2D points[3], float *min, float *max, DaoxVector3D *planePoint, DaoxVector3D *planeNormal )
{
	DaoxVector2D center = {0.0, 0.0};
	DaoxVector2D points2[3];
	DaoxVector2D points3[3];
	double epsilon = 1E-6 * self->radius;
	float width = self->width;
	float z, len = DaoxVector2D_Dist( points[0], points[1] );
	int i;

	if( self->heightmap ){
		if( len * self->heightmap->dims[1] / width < 1.0 ) return;
	}
	if( (*max - *min) > 0.1 * self->height && (*max - *min) > 0.05 * self->radius ) return;
	for(i=0; i<3; ++i){
		DaoxVector3D mid;
		points2[i] = DaoxVector2D_Interpolate( points[i], points[(i+1)%3], 0.5 );
		mid.z = DaoxTerrain_GetHMHeight( self, points2[i].x, points2[i].y ) - planePoint->z;
		mid.x = points2[i].x - planePoint->x;
		mid.y = points2[i].y - planePoint->y;
		z = DaoxVector3D_Dot( & mid, planeNormal );
		if( z < *min ) *min = z;
		if( z > *max ) *max = z;
	}
	DaoxTerrain_FindMinMaxPixel( self, points2, min, max, planePoint, planeNormal );
	for(i=0; i<3; ++i){
		memcpy( points3, points2, 3*sizeof(DaoxVector2D) );
		points3[i] = points[(i+2)%3];
		DaoxTerrain_FindMinMaxPixel( self, points3, min, max, planePoint, planeNormal );
	}
}

void DaoxTerrain_Refine( DaoxTerrain *self, DaoxTerrainBlock *unit, DaoxTerrainTriangle *triangle )
{
	DaoxVector3D point0 = triangle->points[0]->pos;
	DaoxVector3D point1 = triangle->points[1]->pos;
	DaoxVector3D point2 = triangle->points[2]->pos;
	DaoxVector3D normal = DaoxTriangle_Normal( & point0, & point1, & point2 );
	DaoxVector2D points[3];
	float len = DaoxVector3D_Dist( & point0, & point1 );
	float min = 0.0;
	float max = 0.0;
	int i;

	for(i=0; i<3; ++i){
		float z = triangle->points[i]->pos.z;
		points[i].x = triangle->points[i]->pos.x;
		points[i].y = triangle->points[i]->pos.y;
	}

	DaoxTerrain_FindMinMaxPixel( self, points, & min, & max, & point0, & normal );
	if( (max - min) < 0.1 * self->height && (max - min) < (0.1 * len + 0.1) ){ // XXX
		DaoxTerrainTriangle_DeleteSplits( triangle );
		return;
	}
	DaoxTerrain_Split( self, unit, triangle );
	for(i=0; i<4; ++i) DaoxTerrain_Refine( self, unit, triangle->splits[i] );
}
void DaoxTerrain_RebuildUnit( DaoxTerrain *self, DaoxTerrainBlock *unit )
{
	int i;

	unit->center->pos.z = DaoxTerrain_GetHMHeight( self, unit->center->pos.x, unit->center->pos.y );

	for(i=0; i<unit->sides; ++i){
		DaoxTerrainPoint *point = unit->spokes[i]->end;
		point->pos.z = DaoxTerrain_GetHMHeight( self, point->pos.x, point->pos.y );
	}
	for(i=0; i<unit->sides; ++i) DaoxTerrain_Refine( self, unit, unit->splits[i] );
}
int DaoxTerrainBorder_CountPoints( DaoxTerrainBorder *self )
{
	int k = 0;
	if( self->left ){
		k += DaoxTerrainBorder_CountPoints( self->left );
		k += DaoxTerrainBorder_CountPoints( self->right );
		return k + 1;
	}
	return 0;
}
void DaoxTerrain_Adjust( DaoxTerrain *self, DaoxTerrainBlock *unit, DaoxTerrainTriangle *triangle )
{
	int i, k = 0, divs = 0;
	if( triangle->splits[0] != NULL ){
		for(i=0; i<4; ++i) DaoxTerrain_Adjust( self, unit, triangle->splits[i] );
		return;
	}
	for(i=0; i<3; ++i){
		if( triangle->borders[i]->left != NULL ){
			divs += 1;
			k = i;
		}
	}
	if( divs >= 2 ){
		DaoxTerrain_Split( self, unit, triangle );
		self->changes += 1;
	}else if( divs == 1 ){
		int count = DaoxTerrainBorder_CountPoints( triangle->borders[k] );
		if( count >= 3 ){
			DaoxTerrain_Split( self, unit, triangle );
			self->changes += 1;
		}
	}
}
void DaoxTerrain_AdjustUnit( DaoxTerrain *self, DaoxTerrainBlock *unit )
{
	int i;
	for(i=0; i<unit->sides; ++i) DaoxTerrain_Adjust( self, unit, unit->splits[i] );
}
static void DaoxTerrain_UpdateNormals( DaoxTerrainPoint *A, DaoxTerrainPoint *B, DaoxTerrainPoint *C )
{
	DaoxVertex_UpdateNormalTangent( (DaoxVertex*) A, (DaoxVertex*) B, (DaoxVertex*) C, 1, 1 );
}
void DaoxTerrain_ComputeNormalTangents( DaoxTerrain *self, DaoxTerrainBlock *unit, DaoxTerrainTriangle *triangle )
{
	int i, k = 0, divs = 0;
	if( triangle->splits[0] != NULL ){
		for(i=0; i<4; ++i) DaoxTerrain_ComputeNormalTangents( self, unit, triangle->splits[i] );
		return;
	}
	for(i=0; i<3; ++i){
		if( triangle->borders[i]->left != NULL ){
			divs += 1;
			k = i;
		}
	}
	if( divs >= 2 ){
		printf( "WARNING: NOT SUPPOSED TO HAPPEND!\n" );
		return;
	}else if( divs == 1 ){
		DaoxTerrainPoint *p0 = triangle->points[(k+2)%3];
		DList *points = self->buffer;
		points->size = 0;
		DList_Append( points, triangle->borders[k]->start );
		DaoxTerrainBorder_GetPoints( triangle->borders[k], points );
		if( triangle->points[k] == triangle->borders[k]->end ) DList_Reverse( points );
		for(i=1; i<points->size; ++i){
			DaoxTerrainPoint *p1 = (DaoxTerrainPoint*) points->items.pVoid[i-1];
			DaoxTerrainPoint *p2 = (DaoxTerrainPoint*) points->items.pVoid[i];
			DaoxTerrain_UpdateNormals( p0, p1, p2 );
		}
		return;
	}
	DaoxTerrain_UpdateNormals( triangle->points[0], triangle->points[1], triangle->points[2] );
}
void DaoxTerrainBlock_ComputeTextureCoordinates( DaoxTerrainBlock *unit, DaoxTerrainBorder *border )
{
	DaoxTerrainPoint *mid;

	if( border->left == NULL ) return;

	mid = border->left->end;
	if( mid == border->start || mid == border->end ) mid = border->left->start;
	mid->tex = DaoxVector2D_Interpolate( border->start->tex, border->end->tex, 0.5 );

	DaoxTerrainBlock_ComputeTextureCoordinates( unit, border->left );
	DaoxTerrainBlock_ComputeTextureCoordinates( unit, border->right );
}
void DaoxTerrainBlock_ResetVertices( DaoxTerrainBlock *unit, DaoxTerrainBorder *border )
{
	if( border->left ){
		DaoxTerrainBlock_ResetVertices( unit, border->left );
		DaoxTerrainBlock_ResetVertices( unit, border->right );
		return;
	}
	border->start->id = border->end->id = 0;
	border->start->flat = 1.0;
	border->end->flat = 1.0;
}
void DaoxTerrain_ResetVertices( DaoxTerrain *self, DaoxTerrainBlock *unit, DaoxTerrainTriangle *triangle )
{
	int i;
	if( triangle->splits[0] != NULL ){
		for(i=0; i<3; ++i){
			DaoxTerrainBorder *border = triangle->borders[i];
			DaoxTerrainPoint *mid = border->left->end;
			if( mid == border->start || mid == border->end ) mid = border->left->start;
			mid->tex = DaoxVector2D_Interpolate( border->start->tex, border->end->tex, 0.5 );
		}
		for(i=0; i<4; ++i) DaoxTerrain_ResetVertices( self, unit, triangle->splits[i] );
		return;
	}
	/*
	// Borders should be used to export vertices, because they may belong to
	// triangles from other tiles.
	*/
	for(i=0; i<3; ++i){
		DaoxTerrainBlock_ComputeTextureCoordinates( unit, triangle->borders[i] );
		DaoxTerrainBlock_ResetVertices( unit, triangle->borders[i] );
	}
}
void DaoxTerrainBlock_ExportVertices( DaoxTerrainBlock *unit, DaoxTerrainBorder *border )
{
	if( border->left ){
		DaoxTerrainBlock_ExportVertices( unit, border->left );
		DaoxTerrainBlock_ExportVertices( unit, border->right );
		return;
	}
	if( border->start->id == 0 ){
		DaoxVertex *vertex = (DaoxVertex*) DArray_Push( unit->mesh->vertices );
		vertex->pos = border->start->pos;
		vertex->norm = DaoxVector3D_Normalize( & border->start->norm );
		vertex->tan = DaoxVector3D_Normalize( & border->start->tan );
		vertex->tex = border->start->tex;
		border->start->id = unit->mesh->vertices->size;
		border->start->norm = vertex->norm;
	}
	if( border->end->id == 0 ){
		DaoxVertex *vertex = (DaoxVertex*) DArray_Push( unit->mesh->vertices );
		vertex->pos = border->end->pos;
		vertex->norm = DaoxVector3D_Normalize( & border->end->norm );
		vertex->tan = DaoxVector3D_Normalize( & border->end->tan );
		vertex->tex = border->end->tex;
		border->end->id = unit->mesh->vertices->size;
		border->end->norm = vertex->norm;
	}
}
void DaoxTerrain_ExportVertices( DaoxTerrain *self, DaoxTerrainBlock *unit, DaoxTerrainTriangle *triangle )
{
	int i;
	if( triangle->splits[0] != NULL ){
		for(i=0; i<4; ++i) DaoxTerrain_ExportVertices( self, unit, triangle->splits[i] );
		return;
	}
	/*
	// Borders should be used to export vertices, because they may belong to
	// triangles from other tiles.
	*/
	for(i=0; i<3; ++i){
		DaoxTerrainBlock_ExportVertices( unit, triangle->borders[i] );
	}
}
void DaoxTerrainBlock_UpdateGeoState( DaoxTerrainBlock *self, DaoxTerrainPoint *A, DaoxTerrainPoint *B, DaoxTerrainPoint *C )
{
	DaoxVector3D norm = DaoxTriangle_Normal( & A->pos, & B->pos, & C->pos );
	float dot1 = DaoxVector3D_Dot( & A->norm, & norm );
	float dot2 = DaoxVector3D_Dot( & B->norm, & norm );
	float dot3 = DaoxVector3D_Dot( & C->norm, & norm );
	if( dot1 < A->flat ) A->flat = dot1;
	if( dot2 < B->flat ) B->flat = dot2;
	if( dot3 < C->flat ) C->flat = dot3;
}
void DaoxTerrainBlock_AddTriangles( DaoxTerrainBlock *self, DaoxTerrainPoint *start, DList *points )
{
	DaoxTriangle *mt;
	int i;
	for(i=1; i<points->size; ++i){
		DaoxTerrainPoint *p1 = (DaoxTerrainPoint*) points->items.pVoid[i-1];
		DaoxTerrainPoint *p2 = (DaoxTerrainPoint*) points->items.pVoid[i];
		DaoxTerrainBlock_UpdateGeoState( NULL, start, p1, p2 );
		mt = (DaoxTriangle*) DArray_Push( self->mesh->triangles );
		mt->index[0] = start->id - 1;
		mt->index[1] = p1->id - 1;
		mt->index[2] = p2->id - 1;
	}
}
void DaoxTerrain_ExportTriangles( DaoxTerrain *self, DaoxTerrainBlock *unit, DaoxTerrainTriangle *triangle )
{
	DaoxTriangle *mt;
	int i, k = 0, divs = 0;
	if( triangle->splits[0] != NULL ){
		for(i=0; i<4; ++i) DaoxTerrain_ExportTriangles( self, unit, triangle->splits[i] );
		return;
	}
	for(i=0; i<3; ++i){
		if( triangle->borders[i]->left != NULL ){
			divs += 1;
			k = i;
		}
	}
	if( divs >= 2 ){
		printf( "WARNING: NOT SUPPOSED TO HAPPEND!\n" );
		DaoxTerrain_Split( self, unit, triangle );
		DaoxTerrain_ExportTriangles( self, unit, triangle );
		return;
	}else if( divs == 1 ){
		DList *points = self->buffer;
		points->size = 0;
		DList_Append( points, triangle->borders[k]->start );
		DaoxTerrainBorder_GetPoints( triangle->borders[k], points );
		if( triangle->points[k] == triangle->borders[k]->end ) DList_Reverse( points );
		DaoxTerrainBlock_AddTriangles( unit, triangle->points[(k+2)%3], points );
		return;
	}
	DaoxTerrainBlock_UpdateGeoState( unit, triangle->points[0], triangle->points[1], triangle->points[2] );
	mt = (DaoxTriangle*) DArray_Push( unit->mesh->triangles );
	for(i=0; i<3; ++i){
		DaoxTerrainPoint *p2 = triangle->points[i];
		mt->index[i] = triangle->points[i]->id - 1;
	}
}
void DaoxTerrainBlock_InitTextureCoordinates( DaoxTerrainBlock *self )
{
	int i;

	self->center->tex.x = self->center->tex.y = 0.5;
	if( self->sides == 4 ){
		for(i=0; i<self->sides; ++i){
			self->spokes[i]->end->tex.x = 0.5 + 0.4 * (2*(i == 0 || i == 1) - 1);
			self->spokes[i]->end->tex.y = 0.5 + 0.4 * (2*(i == 1 || i == 2) - 1);
		}
	}else{
		for(i=0; i<self->sides; ++i){
			self->spokes[i]->end->tex.x = 0.5 + 0.4 * cos( i*M_PI/3 - M_PI/6 );
			self->spokes[i]->end->tex.y = 0.5 + 0.4 * sin( i*M_PI/3 - M_PI/6 );
		}
	}
}
void DaoxTerrain_BuildMesh( DaoxTerrain *self, DaoxTerrainBlock *unit )
{
	int i;
	unit->mesh->vertices->size = unit->mesh->triangles->size = 0;
	DaoxTerrainBlock_InitTextureCoordinates( unit );
	for(i=0; i<unit->sides; ++i) DaoxTerrain_ResetVertices( self, unit, unit->splits[i] );
	for(i=0; i<unit->sides; ++i) DaoxTerrain_ExportVertices( self, unit, unit->splits[i] );
	for(i=0; i<unit->sides; ++i) DaoxTerrain_ExportTriangles( self, unit, unit->splits[i] );
	printf( "vertices: %i\n", unit->mesh->vertices->size );
	printf( "triangles: %i\n", unit->mesh->triangles->size );
	for(i=0; i<unit->mesh->vertices->size; ++i){
		DaoxVertex *vertex = unit->mesh->vertices->data.vertices + i;
		vertex->tex.x *= self->textureScale;
		vertex->tex.y *= self->textureScale;
		//printf( "%9f %9f %9f\n", vertex->pos.x, vertex->pos.y, vertex->pos.z );
	}

}
void DaoxTerrain_FinalizeMesh( DaoxTerrain *self )
{
	DaoxTerrainBlock *unit;
	int i, j, k, count = 0;

	self->changes = 1;
	while( self->changes ){
		self->changes = 0;
		for(unit=self->first; unit!=self->last->next; unit=unit->next){
			DaoxTerrain_AdjustUnit( self, unit );
		}
		printf( "adjust: %i\n", self->changes );
	}
	for(unit=self->first; unit!=self->last->next; unit=unit->next){
		for(j=0; j<unit->sides; ++j) DaoxTerrain_ResetVertices( self, unit, unit->splits[j] );
		for(j=0; j<unit->sides; ++j) DaoxTerrain_ComputeNormalTangents( self, unit, unit->splits[j] );
	}
	for(unit=self->first; unit!=self->last->next; unit=unit->next){
		DaoxTerrain_BuildMesh( self, unit );
		count += 1;
	}
	printf( "DaoxTerrain_Rebuild %i %i\n", count, self->mesh->units->size );
	DaoxMesh_UpdateTree( self->mesh, 128 );
	DaoxMesh_ResetBoundingBox( self->mesh );
	self->base.obbox = self->mesh->obbox;
}
void DaoxTerrain_Rebuild( DaoxTerrain *self )
{
	DaoxTerrainBlock *unit;
	printf( "DaoxTerrain_Rebuild\n" );
	DaoxTerrain_InitBlocks( self );
	for(unit=self->first; unit!=self->last->next; unit=unit->next){
		DaoxTerrain_RebuildUnit( self, unit );
	}
	DaoxTerrain_FinalizeMesh( self );
}
void DaoxTerrain_RefineExportByPoint( DaoxTerrain *self, DaoxTerrainPoint *point, DaoxTerrain *terrain )
{
	DaoxTerrainTriangle *triangle;
	float width2 = terrain->width;
	float length2 = terrain->length;
	float x = point->pos.x * width2 / self->width;
	float y = point->pos.y * length2 / self->length;
	float a, b, c;
	int i;

	if( point->flat > 0.9 ) return;

	triangle = DaoxTerrain_Locate( terrain, x, y );
	if( triangle == NULL || triangle->splits[0] ) return;

	a = DaoxVector3D_Dist( & triangle->points[0]->pos, & triangle->points[1]->pos );
	b = DaoxVector3D_Dist( & triangle->points[1]->pos, & triangle->points[2]->pos );
	c = DaoxVector3D_Dist( & triangle->points[2]->pos, & triangle->points[0]->pos );
	if( DaoxTriangle_AreaBySideLength( a, b, c ) < 1E-4 * width2 * length2 ) return;

	DaoxTerrain_Split( terrain, NULL, triangle );
}
void DaoxTerrain_Export( DaoxTerrain *self, DaoxTerrain *terrain )
{
	float width = self->width;
	float length = self->length;
	float width2 = terrain->width;
	float length2 = terrain->length;
	int i, count = terrain->points->size + 1;
	DaoxTerrain_InitBlocks( terrain );
	while( count != terrain->points->size ){
		count = terrain->points->size;
		for(i=0; i<self->points->size; ++i){
			DaoxTerrainPoint *point = (DaoxTerrainPoint*) self->points->items.pVoid[i];
			DaoxTerrain_RefineExportByPoint( self, point, terrain );
		}
	}
	for(i=0; i<terrain->points->size; ++i){
		DaoxTerrainPoint *point = (DaoxTerrainPoint*) terrain->points->items.pVoid[i];
		float x = point->pos.x * width / width2;
		float y = point->pos.y * length / length2;
		point->pos.z = DaoxTerrain_GetHeight( self, x, y );
		point->pos.z += DaoxTerrain_GetHeight( self, x+1E-6, y );
		point->pos.z += DaoxTerrain_GetHeight( self, x-1E-6, y );
		point->pos.z += DaoxTerrain_GetHeight( self, x, y+1E-6 );
		point->pos.z += DaoxTerrain_GetHeight( self, x, y-1E-6 );
		point->pos.z *= 0.2;
	}
	DaoxTerrain_FinalizeMesh( terrain );
}



DaoxTerrainGenerator* DaoxTerrainGenerator_New()
{
	DaoxTerrainGenerator *self = (DaoxTerrainGenerator*) dao_calloc(1,sizeof(DaoxTerrainGenerator));
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_terrain_generator );
	self->terrain = DaoxTerrain_New();
	GC_IncRC( self->terrain );
	self->randGenerator = DaoRandGenerator_New( rand() );
	self->params.resolution = 1.0;
	self->params.amplitude = 1.0;
	self->params.faultScale = 1.0;
	return self;
}
void DaoxTerrainGenerator_Delete( DaoxTerrainGenerator *self )
{
	DaoCstruct_Free( (DaoCstruct*) self );
// TODO
}

void DaoxTerrainGenerator_ApplyFaultLine2( DaoxTerrainGenerator *self, DaoxTerrainBlock *unit, DaoxTerrainTriangle *triangle )
{
	DaoxVector2D point1 = DaoxVector2D_Vector3D( triangle->points[1]->pos );
	DaoxVector2D point2 = DaoxVector2D_Vector3D( triangle->points[2]->pos );
	DaoxTerrainParams *params = unit->params ? unit->params : & self->params;
	float len = DaoxVector2D_Dist( point1, point2 );
	float faultScale = 0.1 * self->faultDist;
	float maxChange = 0.1 * self->faultDist * params->amplitude;
	float minSize = 0.01 * self->diameter * params->resolution;
	float minDistToPoint = self->diameter;
	float distToFaultLine[3];
	float distToFaultPoint[3];
	float noise;
	int i;

	noise = DaoRandGenerator_GetNormal( self->randGenerator );
	if( noise >  1.0 ) noise =  1.0;
	if( noise < -1.0 ) noise = -1.0;
	maxChange += 0.1 * maxChange * noise;
	if( maxChange > 0.5*len ) maxChange = 0.5*len;
	if( triangle->splits[0] != NULL ){
		for(i=0; i<4; ++i) DaoxTerrainGenerator_ApplyFaultLine2( self, unit, triangle->splits[i] );
		return;
	}
	for(i=0; i<3; ++i){
		DaoxVector2D point = DaoxVector2D_Vector3D( triangle->points[i]->pos );
		distToFaultPoint[i] = DaoxVector2D_Dist( point, self->faultPoint );
		if( minDistToPoint > distToFaultPoint[i] ) minDistToPoint = distToFaultPoint[i];

		point = DaoxVector2D_Sub( & point, & self->faultPoint );
		distToFaultLine[i] = DaoxVector2D_Dot( & point, & self->faultNorm );
	}
	if( distToFaultLine[0] * distToFaultLine[1] < 0.0 || distToFaultLine[0] * distToFaultLine[2] < 0.0 ){
		float prob;
		if( len <= minSize ) goto Adjust;
		prob = exp( - minDistToPoint / faultScale );
		if( DaoRandGenerator_GetUniform( self->randGenerator ) > prob ) goto Adjust;
		DaoxTerrain_Split( self->terrain, unit, triangle );
		for(i=0; i<4; ++i) DaoxTerrainGenerator_ApplyFaultLine2( self, unit, triangle->splits[i] );
		return;
	}
Adjust:
	for(i=0; i<3; ++i){
		float factor1 = distToFaultLine[i] / (fabs(distToFaultLine[i]) + 1.0);
		float factor2 = exp( - distToFaultPoint[i] / faultScale );
		float offset = maxChange * factor1 * factor2;
		triangle->points[i]->pos.z += offset;
	}
}
void DaoxTerrainGenerator_ApplyFaultLine( DaoxTerrainGenerator *self )
{
	DaoxTerrain *terrain = self->terrain;
	DaoxTerrainBlock *unit;
	int i, j;

	for(unit=terrain->first; unit!=terrain->last->next; unit=unit->next){
		DaoxTerrainBlock_InitTextureCoordinates( unit );
		for(j=0; j<unit->sides; ++j) DaoxTerrain_ResetVertices( terrain, unit, unit->splits[j] );
	}
	for(unit=terrain->first; unit!=terrain->last->next; unit=unit->next){
		for(j=0; j<unit->sides; ++j){
			DaoxTerrainGenerator_ApplyFaultLine2( self, unit, unit->splits[j] );
		}
	}
}
void DaoxTerrainGenerator_Update( DaoxTerrainGenerator *self, int iterations )
{
	DaoxTerrainBlock *unit;
	DaoxTerrain *terrain = self->terrain;
	DaoRandGenerator *randgen = self->randGenerator;
	DaoxVector2D *faultPoint = & self->faultPoint;
	DaoxVector2D *faultNorm = & self->faultNorm;
	int i, j, k;

	printf( "DaoxTerrain_Update:\n" );
	for(i=0; i<iterations; ++i){
		float randAngle;
		faultPoint->x = self->diameter * (DaoRandGenerator_GetUniform( randgen ) - 0.5);
		faultPoint->y = self->diameter * (DaoRandGenerator_GetUniform( randgen ) - 0.5);
		faultNorm->x = DaoRandGenerator_GetUniform( randgen ) - 0.5;
		faultNorm->y = DaoRandGenerator_GetUniform( randgen ) - 0.5;
		*faultNorm = DaoxVector2D_Normalize( faultNorm );
		self->faultDist = 0.5 * self->diameter;
		DaoxTerrainGenerator_ApplyFaultLine( self );
		while( self->faultDist >= 0.01 * self->diameter ){
			DaoxVector2D faultDir = DaoxMatrix3D_RotateVector( *faultNorm, 0.5*M_PI );
			self->faultDist = self->faultDist * DaoRandGenerator_GetNormal( randgen );
			faultPoint->x += self->faultDist * faultDir.x;
			faultPoint->y += self->faultDist * faultDir.y;
			randAngle = 0.5 * M_PI * (1.0 + DaoRandGenerator_GetNormal( randgen ));
			*faultNorm = DaoxMatrix3D_RotateVector( faultDir, randAngle );
			*faultNorm = DaoxVector2D_Normalize( faultNorm );
			if( DaoRandGenerator_GetUniform( randgen ) ){
				faultNorm->x = - faultNorm->x;
				faultNorm->y = - faultNorm->y;
			}
			self->faultDist = fabs( self->faultDist );
			DaoxTerrainGenerator_ApplyFaultLine( self );
		}
	}
	DaoxTerrain_FinalizeMesh( terrain );
}
void DaoxTerrainGenerator_Generate( DaoxTerrainGenerator *self, int iterations, int seed )
{
	DaoxTerrainBlock *unit;
	DaoxTerrain *terrain = self->terrain;
	DaoRandGenerator *randgen = self->randGenerator;
	int i, j, k;

	self->diameter = terrain->width;
	DaoRandGenerator_Seed( self->randGenerator, seed );
	printf( "DaoxTerrain_Generate:\n" );
	for(unit=terrain->first; unit!=terrain->last->next; unit=unit->next){
		unit->center->pos.z = 0.0;
		for(j=0; j<unit->sides; ++j) unit->spokes[j]->end->pos.z = 0.0;
	}
	DaoxTerrainGenerator_Update( self, iterations );
}
