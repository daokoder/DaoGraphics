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


DaoxTerrainPoint* DaoxTerrainPoint_New()
{
	DaoxTerrainPoint *self = (DaoxTerrainPoint*) dao_calloc( 1, sizeof(DaoxTerrainPoint) );
	return self;
}
void DaoxTerrainPoint_Delete( DaoxTerrainPoint *self )
{
	dao_free( self );
}


DaoxTerrainCell* DaoxTerrainCell_New()
{
	DaoxTerrainCell *self = (DaoxTerrainCell*) dao_calloc( 1, sizeof(DaoxTerrainCell) );
	return self;
}
void DaoxTerrainCell_Delete( DaoxTerrainCell *self )
{
	dao_free( self );
}
double DaoxTerrainCell_Size( DaoxTerrainCell *self )
{
	if( self->corners[0] == NULL || self->corners[1] == NULL ) return 0.0;
	return DaoxVector3D_Dist( & self->corners[0]->pos, & self->corners[1]->pos );
}


DaoxTerrainBlock* DaoxTerrainBlock_New()
{
	DaoxTerrainBlock *self = (DaoxTerrainBlock*) dao_calloc( 1, sizeof(DaoxTerrainBlock) );
	return self;
}
void DaoxTerrainBlock_Delete( DaoxTerrainBlock *self )
{
	dao_free( self );
}



DaoxTerrain* DaoxTerrain_New()
{
	DaoxTerrain *self = (DaoxTerrain*) dao_calloc( 1, sizeof(DaoxTerrain) );
	DaoxSceneNode_Init( (DaoxSceneNode*) self, daox_type_terrain );
	self->blocks = DList_New(0);
	self->vertices = DList_New(0);
	self->triangles = DArray_New( sizeof(DaoxTriangle) );
	self->pointList = DList_New(0);
	self->pointCache = DList_New(0);
	self->cellCache = DList_New(0);
	self->width = self->length = self->height = 1.0;
	self->depth = 1.0;
	return self;
}
void DaoxTerrain_CacheCell( DaoxTerrain *self, DaoxTerrainCell *cell )
{
	int i;
	for(i=0; i<4; ++i){
		if( cell->subcells[i] == NULL ) continue;
		DaoxTerrain_CacheCell( self, cell->subcells[i] );
		DList_Append( self->cellCache, cell->subcells[i] );
		cell->subcells[i] = NULL;
	}
	if( cell->center ){
		cell->center->count -= 1;
		if( cell->center->count == 0 ) DList_Append( self->pointCache, cell->center );
	}
	for(i=0; i<4; ++i){
		DaoxTerrainPoint *point = cell->corners[i];
		if( point == NULL ) continue;
		point->count -= 1;
		if( point->count == 0 ) DList_Append( self->pointCache, point );
		cell->corners[i] = NULL;
	}
}
void DaoxTerrain_CacheBlock( DaoxTerrain *self, DaoxTerrainBlock *block )
{
	DaoxTerrainCell *cell = block->cellTree;
	DaoxTerrainPoint *first;
	/* Caching bottom corners: */
	DList_Append( self->pointCache, block->baseCenter );
	for(first = cell->corners[0]; first != cell->corners[1]; first = first->east){
		if( first->east->bottom ) DList_Append( self->pointCache, first->east->bottom );
	}
	for(first = cell->corners[1]; first != cell->corners[2]; first = first->north){
		if( first->north->bottom ) DList_Append( self->pointCache, first->north->bottom );
	}
	for(first = cell->corners[2]; first != cell->corners[3]; first = first->west){
		if( first->west->bottom ) DList_Append( self->pointCache, first->west->bottom );
	}
	for(first = cell->corners[3]; first != cell->corners[0]; first = first->south){
		if( first->south->bottom ) DList_Append( self->pointCache, first->south->bottom );
	}
	DaoxTerrain_CacheCell( self, block->cellTree );
}
void DaoxTerrain_FreeBlocks( DaoxTerrain *self )
{
	int i;
	for(i=0; i<self->blocks->size; ++i){
		DaoxTerrainBlock *block = (DaoxTerrainBlock*) self->blocks->items.pVoid[i];
		DaoxTerrain_CacheBlock( self, block );
		DaoxTerrainBlock_Delete( block );
	}
	DList_Clear( self->blocks );
}
void DaoxTerrain_Delete( DaoxTerrain *self )
{
	int i;
	DaoxTerrain_FreeBlocks( self );
	for(i=0; i<self->pointCache->size; ++i) dao_free( self->pointCache->items.pVoid[i] );
	for(i=0; i<self->cellCache->size; ++i) dao_free( self->cellCache->items.pVoid[i] );
	DList_Delete( self->blocks );
	DList_Delete( self->vertices );
	DArray_Delete( self->triangles );
	DList_Delete( self->pointList );
	DList_Delete( self->pointCache );
	DList_Delete( self->cellCache );
	DaoxSceneNode_Free( (DaoxSceneNode*) self );
	dao_free( self );
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
void DaoxTerrain_SetSize( DaoxTerrain *self, float width, float length )
{
	self->width = width;
	self->length = length;
}
void DaoxTerrain_SetHeightmap( DaoxTerrain *self, DaoArray *heightmap )
{
	GC_Assign( & self->heightmap, heightmap );
	self->height = DaoArray_Max( heightmap );
}
void DaoxTerrain_SetMaterial( DaoxTerrain *self, DaoxMaterial *material )
{
	GC_Assign( & self->material, material );
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
float DaoxTerrain_GetHeight( DaoxTerrain *self, float x, float y )
{
	return DaoArray_InterpolateValue( self->heightmap, self->width, self->length, x, y );
}
static DaoxTerrainPoint* DaoxTerrain_MakePoint( DaoxTerrain *self, float x, float y )
{
	DaoxTerrainPoint *point = NULL;

	if( self->pointCache->size ){
		point = (DaoxTerrainPoint*) DList_PopBack( self->pointCache );
	}else{
		point = DaoxTerrainPoint_New();
	}
	point->index = 0;
	point->level = 0;
	point->count = 0;
	point->pos.x = x;
	point->pos.y = y;
	point->pos.z = DaoxTerrain_GetHeight( self, x, y );
	point->norm.x = point->norm.y = point->norm.z = 0.0;
	point->east = point->west = NULL;
	point->south = point->north = NULL;
	point->bottom = NULL;
	return point;
}
static DaoxTerrainCell* DaoxTerrain_MakeCell( DaoxTerrain *self, DaoxTerrainPoint *corners[4], int level )
{
	int i, j, ihmx, ihmy;
	int mapWidth = self->heightmap->dims[1];
	int mapHeight = self->heightmap->dims[0];
	float left = corners[0]->pos.x;
	float right = corners[2]->pos.x;
	float top = corners[2]->pos.y;
	float bottom = corners[0]->pos.y;
	float xcenter = 0.5*(left + right);
	float ycenter = 0.5*(top + bottom);
	float min = 0.0, max = 0.0;
	float hmx1, hmx2, hmy1, hmy2;
	float H0, H1, H2, H3;
	DaoxTerrainCell *cell = NULL;
	DList *pointList = self->pointList;

	if( self->cellCache->size ){
		cell = (DaoxTerrainCell*) DList_PopBack( self->cellCache );
	}else{
		cell = DaoxTerrainCell_New();
	}
	cell->center = DaoxTerrain_MakePoint( self, xcenter, ycenter );
	cell->center->level = level + 1;
	cell->center->count += 1;
	DList_Append( pointList, cell->center );
	for(i=0; i<4; ++i){
		cell->subcells[i] = NULL;
		cell->corners[i] = corners[i];
		corners[i]->count += 1;
	}

	H0 = DaoxTerrain_GetHeight( self, corners[0]->pos.x, corners[0]->pos.y );
	H1 = DaoxTerrain_GetHeight( self, corners[1]->pos.x, corners[1]->pos.y );
	H2 = DaoxTerrain_GetHeight( self, corners[2]->pos.x, corners[2]->pos.y );
	H3 = DaoxTerrain_GetHeight( self, corners[3]->pos.x, corners[3]->pos.y );

	hmx1 = (corners[0]->pos.x * (mapWidth-1)) / self->width;
	hmx2 = (corners[1]->pos.x * (mapWidth-1)) / self->width;
	hmy1 = (corners[0]->pos.y * (mapHeight-1)) / self->length;
	hmy2 = (corners[3]->pos.y * (mapHeight-1)) / self->length;
	ihmx = (int) hmx1;
	ihmy = (int) hmy1;
	for(j=ihmy + (hmy1>ihmy); j<=hmy2; ++j){
		float beta = (j - hmy1) / (hmy2 - hmy1 + EPSILON);
		float beta2 = 1.0 - beta;
		for(i=ihmx + (hmx1>ihmx); i<=hmx2; ++i){
			float pixel = self->heightmap->data.f[j*mapWidth + i];;
			float alpha = (i - hmx1) / (hmx2 - hmx1 + EPSILON);
			float alpha2 = 1.0 - alpha;
			float expected;
			expected  = alpha2 * beta2 * H0 + alpha * beta * H2;
			expected += alpha * beta2 * H1 + alpha2 * beta * H3;
			pixel -= expected;
			if( pixel < min ) min = pixel;
			if( pixel > max ) max = pixel;
		}
	}
	cell->minHeight = min;
	cell->maxHeight = max;
	return cell;
}
void DaoxTerrain_Refine( DaoxTerrain *self, DaoxTerrainCell *cell )
{
	int i, j, level = cell->center->level;
	float left = cell->corners[0]->pos.x;
	float right = cell->corners[2]->pos.x;
	float top = cell->corners[2]->pos.y;
	float bottom = cell->corners[0]->pos.y;
	float xcenter = cell->center->pos.x;
	float ycenter = cell->center->pos.y;
	float mapwidth = self->heightmap->dims[1];
	float mapheight = self->heightmap->dims[0];
	float mapsize = sqrt( mapwidth*mapwidth + mapheight*mapheight );
	float terrainsize = sqrt( self->width*self->width + self->length*self->length );
	float perpixel = terrainsize/mapsize;
	float cellsize = DaoxTerrainCell_Size( cell );
	float diff = cell->maxHeight - cell->minHeight;
	DaoxTerrainPoint *mid01, *mid12, *mid23, *mid30, *corners[4];
	DaoxTerrainPoint *center = cell->center;
	DList *pointList = self->pointList;

	if( diff < 0.1*self->height && diff < 0.1*(cellsize + 0.5*perpixel) ) return;

	mid01 = cell->corners[0];
	while( mid01 != cell->corners[1] && mid01->level != level ) mid01 = mid01->east;
	if( mid01 == cell->corners[1] ){
		mid01 = DaoxTerrain_MakePoint( self, xcenter, bottom );
		mid01->level = level;
		mid01->west = cell->corners[0];
		mid01->east = cell->corners[1];
		cell->corners[0]->east = mid01;
		cell->corners[1]->west = mid01;
		DList_Append( pointList, mid01 );
	}

	mid12 = cell->corners[1];
	while( mid12 != cell->corners[2] && mid12->level != level ) mid12 = mid12->north;
	if( mid12 == cell->corners[2] ){
		mid12 = DaoxTerrain_MakePoint( self, right, ycenter );
		mid12->level = level;
		mid12->south = cell->corners[1];
		mid12->north = cell->corners[2];
		cell->corners[1]->north = mid12;
		cell->corners[2]->south = mid12;
		DList_Append( pointList, mid12 );
	}

	mid23 = cell->corners[2];
	while( mid23 != cell->corners[3] && mid23->level != level ) mid23 = mid23->west;
	if( mid23 == cell->corners[3] ){
		mid23 = DaoxTerrain_MakePoint( self, xcenter, top );
		mid23->level = level;
		mid23->east = cell->corners[2];
		mid23->west = cell->corners[3];
		cell->corners[2]->west = mid23;
		cell->corners[3]->east = mid23;
		DList_Append( pointList, mid23 );
	}

	mid30 = cell->corners[3];
	while( mid30 != cell->corners[0] && mid30->level != level ) mid30 = mid30->south;
	if( mid30 == cell->corners[0] ){
		mid30 = DaoxTerrain_MakePoint( self, left, ycenter );
		mid30->level = level;
		mid30->north = cell->corners[3];
		mid30->south = cell->corners[0];
		cell->corners[3]->south = mid30;
		cell->corners[0]->north = mid30;
		DList_Append( pointList, mid30 );
	}
	center->east = mid12;
	center->west = mid30;
	center->south = mid01;
	center->north = mid23;
	mid30->east = center;
	mid12->west = center;
	mid23->south = center;
	mid01->north = center;

	corners[0] = cell->corners[0];
	corners[1] = mid01;
	corners[2] = center;
	corners[3] = mid30;
	cell->subcells[2] = DaoxTerrain_MakeCell( self, corners, level );

	corners[0] = mid01;
	corners[1] = cell->corners[1];
	corners[2] = mid12;
	corners[3] = center;
	cell->subcells[3] = DaoxTerrain_MakeCell( self, corners, level );

	corners[0] = center;
	corners[1] = mid12;
	corners[2] = cell->corners[2];
	corners[3] = mid23;
	cell->subcells[0] = DaoxTerrain_MakeCell( self, corners, level );

	corners[0] = mid30;
	corners[1] = center;
	corners[2] = mid23;
	corners[3] = cell->corners[3];
	cell->subcells[1] = DaoxTerrain_MakeCell( self, corners, level );

	for(i=0; i<4; ++i){
		DaoxTerrainCell *sub = cell->subcells[i];
		DaoxTerrain_Refine( self, sub );
	}
}
static void DaoxTerrain_UpdatePointNeighbors( DaoxTerrain *self, DaoxTerrainCell *cell )
{
	int i, j, level = cell->center->level;
	DaoxTerrainPoint *mid01, *mid12, *mid23, *mid30;
	DaoxTerrainPoint *center = cell->center;

	if( cell->subcells[0] ){
		for(i=0; i<4; ++i) DaoxTerrain_UpdatePointNeighbors( self, cell->subcells[i] );
		return;
	}

	mid01 = cell->corners[0];
	while( mid01 != cell->corners[1] && mid01->level != level ) mid01 = mid01->east;
	if( mid01 != cell->corners[1] ){
		center->south = mid01;
		mid01->north = center;
	}

	mid12 = cell->corners[1];
	while( mid12 != cell->corners[2] && mid12->level != level ) mid12 = mid12->north;
	if( mid12 != cell->corners[2] ){
		center->east = mid12;
		mid12->west = center;
	}

	mid23 = cell->corners[2];
	while( mid23 != cell->corners[3] && mid23->level != level ) mid23 = mid23->west;
	if( mid23 != cell->corners[3] ){
		center->north = mid23;
		mid23->south = center;
	}

	mid30 = cell->corners[3];
	while( mid30 != cell->corners[0] && mid30->level != level ) mid30 = mid30->south;
	if( mid30 != cell->corners[0] ){
		center->west = mid30;
		mid30->east = center;
	}
}
static void DaoxTerrain_UpdateNormals( DaoxTerrain *self, DaoxTerrainPoint *center, DaoxTerrainPoint *first, DaoxTerrainPoint *second )
{
	DaoxVector3D A = DaoxVector3D_Sub( & first->pos, & center->pos );
	DaoxVector3D B = DaoxVector3D_Sub( & second->pos, & first->pos );
	DaoxVector3D N = DaoxVector3D_Cross( & A, & B );
	N = DaoxVector3D_Normalize( & N );
	center->norm = DaoxVector3D_Add( & center->norm, & N );
	first->norm = DaoxVector3D_Add( & first->norm, & N );
	second->norm = DaoxVector3D_Add( & second->norm, & N );
}
static void DaoxTerrain_ComputeNormals( DaoxTerrain *self, DaoxTerrainCell *cell )
{
	int i;
	if( cell->subcells[0] == NULL ){
		DaoxTerrainPoint *first;
		for(first = cell->corners[0]; first != cell->corners[1]; first = first->east){
			DaoxTerrain_UpdateNormals( self, cell->center, first, first->east );
		}
		for(first = cell->corners[1]; first != cell->corners[2]; first = first->north){
			DaoxTerrain_UpdateNormals( self, cell->center, first, first->north );
		}
		for(first = cell->corners[2]; first != cell->corners[3]; first = first->west){
			DaoxTerrain_UpdateNormals( self, cell->center, first, first->west );
		}
		for(first = cell->corners[3]; first != cell->corners[0]; first = first->south){
			DaoxTerrain_UpdateNormals( self, cell->center, first, first->south );
		}
		return;
	}
	for(i=0; i<4; ++i) DaoxTerrain_ComputeNormals( self, cell->subcells[i] );
}
static DaoxTerrainBlock* DaoxTerrain_MakeBlock( DaoxTerrain *self, DaoxTerrainPoint *corners[4] )
{
	DaoxTerrainBlock *block = DaoxTerrainBlock_New();
	float x = corners[0]->pos.x + corners[2]->pos.x;
	float y = corners[0]->pos.y + corners[2]->pos.y;

	if( corners[0]->east == NULL ){
		corners[0]->east = corners[1];
		corners[1]->west = corners[0];
	}
	if( corners[1]->north == NULL ){
		corners[1]->north = corners[2];
		corners[2]->south = corners[1];
	}
	if( corners[2]->west == NULL ){
		corners[2]->west = corners[3];
		corners[3]->east = corners[2];
	}
	if( corners[3]->south == NULL ){
		corners[3]->south = corners[0];
		corners[0]->north = corners[3];
	}

	block->cellTree = DaoxTerrain_MakeCell( self, corners, 0 );
	block->baseCenter = DaoxTerrain_MakePoint( self, 0.5*x, 0.5*y );
	block->baseCenter->pos.z = - self->depth;
	return block;
}
static DaoxTerrainPoint* DaoxTerrain_MakeBottomPoint( DaoxTerrain *self, DaoxTerrainPoint *point )
{
	DaoxTerrainPoint *bottom = DaoxTerrain_MakePoint( self, 0.0, 0.0 );
	bottom->pos = point->pos;
	bottom->pos.z = - self->depth;
	point->bottom = bottom;
	return bottom;
}
void DaoxTerrain_FinalizeBlock( DaoxTerrain *self, DaoxTerrainBlock *block )
{
	DaoxTerrainCell *cell = block->cellTree;
	DaoxTerrainPoint *first;

	if( block->south == NULL ){
		for(first = cell->corners[0]; first != cell->corners[1]; first = first->east){
			DaoxTerrain_MakeBottomPoint( self, first->east );
		}
	}
	if( block->east == NULL ){
		for(first = cell->corners[1]; first != cell->corners[2]; first = first->north){
			DaoxTerrain_MakeBottomPoint( self, first->north );
		}
	}
	if( block->north == NULL ){
		for(first = cell->corners[2]; first != cell->corners[3]; first = first->west){
			DaoxTerrain_MakeBottomPoint( self, first->west );
		}
	}
	if( block->west == NULL ){
		for(first = cell->corners[3]; first != cell->corners[0]; first = first->south){
			DaoxTerrain_MakeBottomPoint( self, first->south );
		}
	}
	if( block->east && block->north ) DaoxTerrain_MakeBottomPoint( self, cell->corners[2] );
	if( block->east && block->south ) DaoxTerrain_MakeBottomPoint( self, cell->corners[1] );
	if( block->west && block->north ) DaoxTerrain_MakeBottomPoint( self, cell->corners[3] );
	if( block->west && block->south ) DaoxTerrain_MakeBottomPoint( self, cell->corners[0] );

	DaoxTerrain_ComputeNormals( self, block->cellTree );
}
void DaoxTerrain_Rebuild( DaoxTerrain *self )
{
	int i, j, xnum, ynum, M = 1, N = 1;
	float width = self->width;
	float length = self->length;
	float epsilon = (width + length) / 100.0;
	float large = width, small = length;
	double xstride, ystride;
	DList *pointList = self->pointList;
	DaoxTerrainPoint *first, *base;
	DaoxTerrainPoint *corners[4];
	DaoxTerrainBlock *west = NULL;

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

	DaoxTerrain_FreeBlocks( self );
	printf( "Cell Cache: %i\n", self->cellCache->size );
	
	if( self->heightmap == NULL ) return;

	self->pointList->size = 0;
	xstride = width / xnum;
	ystride = length / ynum;
	for(i=0; i<xnum; ++i){
		float x1 = i * xstride;
		float x2 = x1 + xstride;
		for(j=0; j<ynum; ++j){
			DaoxTerrainBlock *block = NULL;
			DaoxTerrainBlock *west = NULL;
			DaoxTerrainBlock *south = NULL;
			float y1 = j * ystride;
			float y2 = y1 + ystride;
			corners[0] = corners[1] = corners[2] = corners[3] = NULL;
			if( i ) west = self->blocks->items.pTerrainBlock[ (i-1)*ynum + j ];
			if( j ) south = (DaoxTerrainBlock*) DList_Back( self->blocks );
			if( west ){
				corners[0] = west->cellTree->corners[1];
				corners[3] = west->cellTree->corners[2];
			}
			if( south ){
				corners[0] = south->cellTree->corners[3];
				corners[1] = south->cellTree->corners[2];
			}
			if( corners[0] == NULL ){
				corners[0] = DaoxTerrain_MakePoint( self, x1, y1 );
				DList_Append( pointList, corners[0] );
			}
			if( corners[1] == NULL ){
				corners[1] = DaoxTerrain_MakePoint( self, x2, y1 );
				DList_Append( pointList, corners[1] );
			}
			if( corners[2] == NULL ){
				corners[2] = DaoxTerrain_MakePoint( self, x2, y2 );
				DList_Append( pointList, corners[2] );
			}
			if( corners[3] == NULL ){
				corners[3] = DaoxTerrain_MakePoint( self, x1, y2 );
				DList_Append( pointList, corners[3] );
			}
			block = DaoxTerrain_MakeBlock( self, corners );
			block->west = west;
			block->south = south;
			if( west ) west->east = block;
			if( south ) south->north = block;
			DList_Append( self->blocks, block );
		}
	}
	for(i=0; i<self->blocks->size; ++i){
		DaoxTerrainBlock *block = (DaoxTerrainBlock*) self->blocks->items.pVoid[i];
		DaoxTerrain_Refine( self, block->cellTree );
	}
	for(i=0; i<self->blocks->size; ++i){
		DaoxTerrainBlock *block = (DaoxTerrainBlock*) self->blocks->items.pVoid[i];
		DaoxTerrain_UpdatePointNeighbors( self, block->cellTree );
	}
	for(i=0; i<self->blocks->size; ++i){
		DaoxTerrainBlock *block = (DaoxTerrainBlock*) self->blocks->items.pVoid[i];
		DaoxTerrain_FinalizeBlock( self, block );
	}

	/* Making base corners: */
	for(i=0; i<pointList->size; ++i){
		DaoxTerrainPoint *point = (DaoxTerrainPoint*) pointList->items.pVoid[i];
		point->norm = DaoxVector3D_Normalize( & point->norm );
	}
}
static void DaoxTerrain_TryActivatePoint( DaoxTerrain *self, DaoxTerrainPoint *point )
{
	if( point->index == 0 ){
		DList_Append( self->vertices, point );
		point->index = self->vertices->size;
	}
}
static void DaoxTerrain_MakeTriangle( DaoxTerrain *self, DaoxTerrainPoint *center, DaoxTerrainPoint *first, DaoxTerrainPoint *second )
{
	DaoxTriangle triangle;
	triangle.index[0] = center->index - 1;
	triangle.index[1] = first->index - 1;
	triangle.index[2] = second->index - 1;
	DArray_PushTriangle( self->triangles, & triangle );
}
static void DaoxTerrain_MakeBaseTriangle( DaoxTerrain *self, DaoxTerrainPoint *first, DaoxTerrainPoint *second, DaoxTerrainPoint *baseCenter )
{
	DaoxTerrain_TryActivatePoint( self, first->bottom );
	DaoxTerrain_TryActivatePoint( self, second->bottom );
	DaoxTerrain_MakeTriangle( self, first->bottom, second, first );
	DaoxTerrain_MakeTriangle( self, second, first->bottom, second->bottom );
	DaoxTerrain_MakeTriangle( self, baseCenter, second->bottom, first->bottom );
}
static void DaoxTerrain_MakeBaseTriangle2( DaoxTerrain *self, DaoxTerrainPoint *first, DaoxTerrainPoint *second, DaoxTerrainPoint *center )
{
	DaoxTerrain_TryActivatePoint( self, first );
	DaoxTerrain_TryActivatePoint( self, second );
	DaoxTerrain_MakeTriangle( self, center, second, first );
}
static int DaoxTerrain_CellVisible( DaoxTerrain *self, DaoxTerrainCell *cell, DaoxViewFrustum *frustum )
{
	DaoxOBBox3D obbox;
	DaoxMatrix4D terrainToWorld = DaoxSceneNode_GetWorldTransform( & self->base );
	float margin = 1.0; // XXX:

	if( frustum == NULL ) return 1;
	if( DaoxTerrainCell_Size( cell ) < 0.01*(self->width + self->length) ) return 1;

	obbox.C = cell->center->pos;
	obbox.O = cell->corners[0]->pos;
	obbox.X = cell->corners[1]->pos;
	obbox.Y = cell->corners[3]->pos;
	obbox.Z = cell->corners[0]->pos;
	obbox.O.z += cell->minHeight - margin;
	obbox.X.z += cell->minHeight - margin;
	obbox.Y.z += cell->minHeight - margin;
	obbox.Z.z += cell->maxHeight + margin;
	obbox.C.z += 0.5*(cell->minHeight + cell->maxHeight);
	obbox.R = DaoxVector3D_Dist( & obbox.O, & obbox.C );
	obbox = DaoxOBBox3D_Transform( & obbox, & terrainToWorld );
	return DaoxViewFrustum_Visible( frustum, & obbox );
}
static int DaoxTerrain_CellHasFineView( DaoxTerrain *self, DaoxTerrainCell *cell, DaoxViewFrustum *frustum )
{
	DaoxMatrix4D toWorld = DaoxSceneNode_GetWorldTransform( & self->base );
	DaoxVector3D P0 = DaoxMatrix4D_MulVector( & toWorld, & cell->corners[0]->pos, 1.0 );
	float mindist, viewDiff, viewHeight;
	int i;

	if( frustum == NULL ) return 0;
	mindist = DaoxVector3D_Dist( & P0, & frustum->cameraPosition );
	for(i=1; i<3; ++i){
		DaoxVector3D P = DaoxMatrix4D_MulVector( & toWorld, & cell->corners[i]->pos, 1.0 );
		float dist = DaoxVector3D_Dist( & P, & frustum->cameraPosition );
		if( dist < mindist ) mindist = dist;
	}
	if( mindist <= frustum->near ) return 0;

	viewDiff = cell->maxHeight - cell->minHeight;
	viewDiff = frustum->ratio * frustum->near * viewDiff / mindist;
	viewHeight = frustum->ratio * frustum->near * self->height / mindist;
	return viewDiff < 3.0 && viewDiff < 0.1 * viewHeight;
}
static void DaoxTerrain_ActivateVertices( DaoxTerrain *self, DaoxTerrainCell *cell, DaoxViewFrustum *frustum )
{
	int i;

	cell->visible = DaoxTerrain_CellVisible( self, cell, frustum ) >= 0;
	if( ! cell->visible ){
		/* For building coarse but complete mesh: */
		for(i=0; i<4; ++i) DaoxTerrain_TryActivatePoint( self, cell->corners[i] );
		return;
	}

	DaoxTerrain_TryActivatePoint( self, cell->center );
	for(i=0; i<4; ++i) DaoxTerrain_TryActivatePoint( self, cell->corners[i] );

	cell->smooth = DaoxTerrain_CellHasFineView( self, cell, frustum );
	if( cell->subcells[0] == NULL || cell->smooth ) return;

	for(i=0; i<4; ++i) DaoxTerrain_ActivateVertices( self, cell->subcells[i], frustum );
}
static void DaoxTerrain_GenerateTriangles( DaoxTerrain *self, DaoxTerrainCell *cell, DaoxViewFrustum *frustum )
{
	int i, level = cell->center->level;

	if( ! cell->visible ){
		/* For building coarse but complete mesh: */
		DaoxTerrain_MakeTriangle( self, cell->corners[0], cell->corners[1], cell->corners[2] );
		DaoxTerrain_MakeTriangle( self, cell->corners[0], cell->corners[2], cell->corners[3] );
		return;
	}
	if( cell->subcells[0] == NULL || cell->smooth ){
		int active1, active2, active3, active4;
		DaoxTerrainPoint *first, *second = NULL;
		DaoxTerrainPoint *east = cell->center->east;
		DaoxTerrainPoint *west = cell->center->west;
		DaoxTerrainPoint *south = cell->center->south;
		DaoxTerrainPoint *north = cell->center->north;

		while( east && east->level != level ) east = east->east;
		while( west && west->level != level ) west = west->west;
		while( south && south->level != level ) south = south->south;
		while( north && north->level != level ) north = north->north;
		active1 = east && east->index > 0;
		active2 = west && west->index > 0;
		active3 = south && south->index > 0;
		active4 = north && north->index > 0;

		if( active1 == 0 && active2 == 0 && active3 == 0 && active4 == 0 ){
			DaoxTerrain_MakeTriangle( self, cell->corners[0], cell->corners[1], cell->corners[2] );
			DaoxTerrain_MakeTriangle( self, cell->corners[0], cell->corners[2], cell->corners[3] );
			return;
		}

		for(first = cell->corners[0]; first != cell->corners[1]; first = second){
			second = first->east;
			while( second->index == 0 ) second = second->east;
			DaoxTerrain_MakeTriangle( self, cell->center, first, second );
		}
		for(first = cell->corners[1]; first != cell->corners[2]; first = second){
			second = first->north;
			while( second->index == 0 ) second = second->north;
			DaoxTerrain_MakeTriangle( self, cell->center, first, second );
		}
		for(first = cell->corners[2]; first != cell->corners[3]; first = second){
			second = first->west;
			while( second->index == 0 ) second = second->west;
			DaoxTerrain_MakeTriangle( self, cell->center, first, second );
		}
		for(first = cell->corners[3]; first != cell->corners[0]; first = second){
			second = first->south;
			while( second->index == 0 ) second = second->south;
			DaoxTerrain_MakeTriangle( self, cell->center, first, second );
		}
		return;
	}
	for(i=0; i<4; ++i) DaoxTerrain_GenerateTriangles( self, cell->subcells[i], frustum );
}
static void DaoxTerrain_MakeDepthMesh( DaoxTerrain *self, DaoxTerrainBlock *block )
{
	DaoxTerrainCell *cell = block->cellTree;
	DaoxTerrainPoint *baseCenter = block->baseCenter;
	DaoxTerrainPoint *corner0 = cell->corners[0];
	DaoxTerrainPoint *corner1 = cell->corners[1];
	DaoxTerrainPoint *corner2 = cell->corners[2];
	DaoxTerrainPoint *corner3 = cell->corners[3];
	DaoxTerrainPoint *first;


	DaoxTerrain_TryActivatePoint( self, baseCenter );

	if( block->south ){
		DaoxTerrain_MakeBaseTriangle2( self, corner2->bottom, corner3->bottom, baseCenter );
	}else{
		for(first = cell->corners[0]; first != cell->corners[1]; ){
			DaoxTerrainPoint *second = first->east;
			while( second->index == 0 ) second = second->east;
			DaoxTerrain_MakeBaseTriangle( self, first, second, baseCenter );
			first = second;
		}
	}
	if( block->east ){
		DaoxTerrain_MakeBaseTriangle2( self, corner3->bottom, corner0->bottom, baseCenter );
	}else{
		for(first = cell->corners[1]; first != cell->corners[2]; ){
			DaoxTerrainPoint *second = first->north;
			while( second->index == 0 ) second = second->north;
			DaoxTerrain_MakeBaseTriangle( self, first, second, baseCenter );
			first = second;
		}
	}
	if( block->north ){
		DaoxTerrain_MakeBaseTriangle2( self, corner0->bottom, corner1->bottom, baseCenter );
	}else{
		for(first = cell->corners[2]; first != cell->corners[3]; ){
			DaoxTerrainPoint *second = first->west;
			while( second->index == 0 ) second = second->west;
			DaoxTerrain_MakeBaseTriangle( self, first, second, baseCenter );
			first = second;
		}
	}
	if( block->west ){
		DaoxTerrain_MakeBaseTriangle2( self, corner1->bottom, corner2->bottom, baseCenter );
	}else{
		for(first = cell->corners[3]; first != cell->corners[0]; ){
			DaoxTerrainPoint *second = first->south;
			while( second->index == 0 ) second = second->south;
			DaoxTerrain_MakeBaseTriangle( self, first, second, baseCenter );
			first = second;
		}
	}
}
void DaoxTerrain_UpdateView( DaoxTerrain *self, DaoxViewFrustum *frustum )
{
	daoint i;

	for(i=0; i<self->vertices->size; ++i){
		DaoxTerrainPoint *point = (DaoxTerrainPoint*) self->vertices->items.pVoid[i];
		point->index = 0;
	}
	self->vertices->size = 0;
	self->triangles->size = 0;

	if( self->blocks->size == 0 ) DaoxTerrain_Rebuild( self );

	for(i=0; i<self->blocks->size; ++i){
		DaoxTerrainBlock *block = (DaoxTerrainBlock*) self->blocks->items.pVoid[i];
		DaoxTerrain_ActivateVertices( self, block->cellTree, frustum );
	}
	for(i=0; i<self->blocks->size; ++i){
		DaoxTerrainBlock *block = (DaoxTerrainBlock*) self->blocks->items.pVoid[i];
		DaoxTerrain_GenerateTriangles( self, block->cellTree, frustum );
	}
	for(i=0; i<self->blocks->size; ++i){
		DaoxTerrainBlock *block = (DaoxTerrainBlock*) self->blocks->items.pVoid[i];
		DaoxTerrain_MakeDepthMesh( self, block );
	}
}





DaoxHexPoint* DaoxHexPoint_New( float x, float y )
{
	DaoxHexPoint *self = (DaoxHexPoint*) dao_calloc( 1, sizeof(DaoxHexPoint) );
	self->pos.x = x;
	self->pos.y = y;
	return self;
}
DaoxHexBorder* DaoxHexBorder_New( DaoxHexPoint *start, DaoxHexPoint *end )
{
	DaoxHexBorder *self = (DaoxHexBorder*) dao_calloc( 1, sizeof(DaoxHexBorder) );
	self->start = start;
	self->end = end;
	return self;
}

DaoxHexTriangle* DaoxHexTriangle_New()
{
	DaoxHexTriangle *self = (DaoxHexTriangle*) dao_calloc( 1, sizeof(DaoxHexTriangle) );
	return self;
}
void DaoxHexTriangle_DeleteSplits( DaoxHexTriangle *self );
void DaoxHexTriangle_Delete( DaoxHexTriangle *self )
{
	DaoxHexTriangle_DeleteSplits( self );
	dao_free( self );
}
void DaoxHexTriangle_DeleteSplits( DaoxHexTriangle *self )
{
	int i;
	for(i=0; i<4; ++i){
		if( self->splits[i] ){
			DaoxHexTriangle_Delete( self->splits[i] );
			self->splits[i] = NULL;
		}
	}
}

DaoxHexUnit* DaoxHexUnit_New()
{
	DaoxHexUnit *self = (DaoxHexUnit*) dao_calloc( 1, sizeof(DaoxHexUnit) );
	return self;
}
void DaoxHexUnit_Delete( DaoxHexUnit *self )
{
	int i;
	for(i=0; i<6; ++i){
		if( self->splits[i] ) DaoxHexTriangle_Delete( self->splits[i] );
	}
	dao_free( self );
}


DaoxHexTerrain* DaoxHexTerrain_New()
{
	DaoxHexTerrain *self = (DaoxHexTerrain*) dao_calloc( 1, sizeof(DaoxHexTerrain) );
	DaoxSceneNode_Init( (DaoxSceneNode*) self, daox_type_hexterrain );
	self->tiles = DList_New(0);
	self->borders = DList_New(0);
	self->points = DList_New(0);
	self->mesh = DaoxMesh_New();
	GC_IncRC( self->mesh );
	self->rows = 1;
	self->columns = 1;
	self->radius = 5.0;
	self->height = 1.0;
	self->depth = 1.0;
	self->textureScale = 1.0;
	self->buffer = DList_New(0);
	return self;
}
void DaoxHexTerrain_Delete( DaoxHexTerrain *self )
{
	GC_DecRC( self->mesh );
}

void DaoxHexTerrain_SetSize( DaoxHexTerrain *self, int rows, int cols, float radius )
{
	self->rows = rows;
	self->columns = cols;
	self->radius = radius;
}
void DaoxHexTerrain_SetHeightmap( DaoxHexTerrain *self, DaoArray *heightmap )
{
	GC_Assign( & self->heightmap, heightmap );
	self->height = DaoArray_Max( heightmap );
}

void DaoxHexTerrain_InitializeTile( DaoxHexTerrain *self, DaoxHexUnit *unit )
{
	int i;

	for(i=0; i<6; ++i){
		if( unit->neighbors[i] ){
			unit->borders[i] = unit->neighbors[i]->borders[(i+3)%6];
		}
	}
	for(i=0; i<6; ++i){
		float x1 = unit->center->pos.x + self->radius * cos( i*M_PI/3.0 );
		float y1 = unit->center->pos.y + self->radius * sin( i*M_PI/3.0 );
		float x2 = unit->center->pos.x + self->radius * cos( (i+1)*M_PI/3.0 );
		float y2 = unit->center->pos.y + self->radius * sin( (i+1)*M_PI/3.0 );
		DaoxVector2D p1 = DaoxVector2D_XY( x1, y1 );
		DaoxVector2D p2 = DaoxVector2D_XY( x2, y2 );
		DaoxHexBorder *prev = unit->borders[(i+5)%6];
		DaoxHexBorder *next = unit->borders[(i+1)%6];
		DaoxHexPoint *start = NULL;
		DaoxHexPoint *end = NULL;
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
		if( start == NULL ) start = DaoxHexPoint_New( x1, y1 );
		if( end == NULL ) end = DaoxHexPoint_New( x2, y2 );
		unit->borders[i] = DaoxHexBorder_New( start, end );
	}
	for(i=0; i<6; ++i){
		DaoxHexBorder *border = unit->borders[i];
		float x1 = unit->center->pos.x + self->radius * cos( i*M_PI/3.0 );
		float y1 = unit->center->pos.y + self->radius * sin( i*M_PI/3.0 );
		DaoxVector2D p1 = DaoxVector2D_XY( x1, y1 );
		DaoxVector2D q1 = DaoxVector2D_Vector3D( border->start->pos );
		DaoxVector2D q2 = DaoxVector2D_Vector3D( border->end->pos );

		double d1 = DaoxVector2D_Dist( q1, p1 );
		double d2 = DaoxVector2D_Dist( q2, p1 );
		DaoxHexPoint *start = d1 < d2 ? border->start : border->end;

		unit->spokes[i] = DaoxHexBorder_New( unit->center, start );
	}

	for(i=0; i<6; ++i){
		DaoxHexTriangle *triangle = unit->splits[i];
		if( triangle == NULL ) unit->splits[i] = triangle = DaoxHexTriangle_New();
		triangle->borders[0] = unit->spokes[i];
		triangle->borders[1] = unit->borders[i];
		triangle->borders[2] = unit->spokes[(i+1)%6];
		triangle->points[0] = unit->center;
		triangle->points[1] = unit->spokes[i]->end;
		triangle->points[2] = unit->spokes[(i+1)%6]->end;
	}
}
void DaoxHexTerrain_InitializeTiles( DaoxHexTerrain *self )
{
	float cos60 = 0.5;
	float sin60 = sin( M_PI / 3.0 );
	int i, j, k, count = self->rows * self->columns;
	printf( "DaoxHexTerrain_InitializeTiles\n" );
	while( self->tiles->size < count ){
		DaoxHexUnit *unit = DaoxHexUnit_New();
		unit->center = DaoxHexPoint_New(0.0,0.0);
		unit->mesh = DaoxMesh_AddUnit( self->mesh );
		DList_Append( self->tiles, unit );
	}
	while( self->tiles->size > count ){
		DaoxHexUnit *unit = (DaoxHexUnit*) DList_PopBack( self->tiles );
		DaoxHexUnit_Delete( unit );
	}
	for(i=0; i<count; ++i){
		DaoxHexUnit *unit = (DaoxHexUnit*) self->tiles->items.pVoid[i];
		int column = i / self->rows;
		int row = i % self->rows;

		unit->center->pos.x = column * self->radius * (1.0 + cos60) + self->radius;
		unit->center->pos.y = self->radius * sin60 * (2.0 * row + 1.0 + column % 2);

		for(j=0; j<6; ++j) unit->neighbors[j] = NULL;
		if( row ){
			k = column * self->rows + row - 1;
			unit->neighbors[4] = (DaoxHexUnit*) self->tiles->items.pVoid[k];
			unit->neighbors[4]->neighbors[1] = unit;
		}
		if( column && (column % 2) == 1 ){
			k = (column-1) * self->rows + row;
			unit->neighbors[3] = (DaoxHexUnit*) self->tiles->items.pVoid[k];
			unit->neighbors[3]->neighbors[0] = unit;
			if( (row+1) < self->rows ){
				k = (column-1) * self->rows + row + 1;
				unit->neighbors[2] = (DaoxHexUnit*) self->tiles->items.pVoid[k];
				unit->neighbors[2]->neighbors[5] = unit;
			}
		}else if( column && (column % 2) == 0 ){
			k = (column-1) * self->rows + row;
			unit->neighbors[2] = (DaoxHexUnit*) self->tiles->items.pVoid[k];
			unit->neighbors[2]->neighbors[5] = unit;
			if( row ){
				k = (column-1) * self->rows + row - 1;
				unit->neighbors[3] = (DaoxHexUnit*) self->tiles->items.pVoid[k];
				unit->neighbors[3]->neighbors[0] = unit;
			}
		}
		DaoxHexTerrain_InitializeTile( self, unit );
	}
}

float DaoxHexTerrain_GetHeight( DaoxHexTerrain *self, float x, float y )
{
	float sin60 = sin( M_PI / 3.0 );
	float width = (self->columns - 1) * 1.5 * self->radius + 2.0 * self->radius + EPSILON + 1;
	float length = self->rows * 2.0 * self->radius * sin60 + self->radius * sin60 + EPSILON + 1;
	if( self->heightmap == NULL ) return 0.0;
	return DaoArray_InterpolateValue( self->heightmap, width, length, x, y );
}

void DaoxHexTerrain_Split( DaoxHexTerrain *self, DaoxHexUnit *unit, DaoxHexTriangle *triangle )
{
	int i;
	for(i=0; i<4; ++i){
		if( triangle->splits[i] == NULL ) triangle->splits[i] = DaoxHexTriangle_New();
	}
	for(i=0; i<3; ++i){
		DaoxHexPoint *mid = NULL;
		DaoxHexBorder *border = triangle->borders[i];
		int dir = triangle->points[i] == border->start;

		if( border->left ){
			DaoxHexBorder *left = border->left;
			mid = left->start == border->start ? left->end : left->start;
		}else{
			mid = DaoxHexPoint_New(0.0,0.0);
			mid->pos = DaoxVector3D_Interpolate( border->start->pos, border->end->pos, 0.5 );
			if( self->heightmap != NULL ){
				mid->pos.z = DaoxHexTerrain_GetHeight( self, mid->pos.x, mid->pos.y );
			}
			border->left = DaoxHexBorder_New( border->start, mid );
			border->right = DaoxHexBorder_New( mid, border->end );
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
		DaoxHexTriangle *mid = triangle->splits[0];
		DaoxHexBorder *border = DaoxHexBorder_New( mid->points[i], mid->points[(i+1)%3] );
		mid->borders[i] = border;
		triangle->splits[i==2?1:i+2]->borders[1] = border;
	}
}
void DaoxHexTerrain_FindMinMaxPixel( DaoxHexTerrain *self, DaoxVector2D points[3], float *min, float *max, DaoxVector3D *planePoint, DaoxVector3D *planeNormal )
{
	DaoxVector2D center = {0.0, 0.0};
	DaoxVector2D points2[3];
	DaoxVector2D points3[3];
	float width = (self->columns - 1) * 1.5 * self->radius + 2.0 * self->radius;
	float z, len = DaoxVector2D_Dist( points[0], points[1] );
	int i;

	if( len * self->heightmap->dims[1] / width < 1.0 ) return;
	if( (*max - *min) > 0.1 * self->height && (*max - *min) > 0.05 * self->radius ) return;
	for(i=0; i<3; ++i){
		DaoxVector3D mid;
		points2[i] = DaoxVector2D_Interpolate( points[i], points[(i+1)%3], 0.5 );
		mid.z = DaoxHexTerrain_GetHeight( self, points2[i].x, points2[i].y ) - planePoint->z;
		mid.x = points2[i].x - planePoint->x;
		mid.y = points2[i].y - planePoint->y;
		z = DaoxVector3D_Dot( & mid, planeNormal );
		if( z < *min ) *min = z;
		if( z > *max ) *max = z;
	}
	DaoxHexTerrain_FindMinMaxPixel( self, points2, min, max, planePoint, planeNormal );
	for(i=0; i<3; ++i){
		memcpy( points3, points2, 3*sizeof(DaoxVector2D) );
		points3[i] = points[(i+2)%3];
		DaoxHexTerrain_FindMinMaxPixel( self, points3, min, max, planePoint, planeNormal );
	}
}

void DaoxHexTerrain_Refine( DaoxHexTerrain *self, DaoxHexUnit *unit, DaoxHexTriangle *triangle )
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

	DaoxHexTerrain_FindMinMaxPixel( self, points, & min, & max, & point0, & normal );
	if( (max - min) < 0.1 * self->height && (max - min) < (0.1 * len + 0.1) ){ // XXX
		DaoxHexTriangle_DeleteSplits( triangle );
		return;
	}
	DaoxHexTerrain_Split( self, unit, triangle );
	for(i=0; i<4; ++i) DaoxHexTerrain_Refine( self, unit, triangle->splits[i] );
}
void DaoxHexTerrain_RebuildUnit( DaoxHexTerrain *self, DaoxHexUnit *unit )
{
	int i;

	unit->center->pos.z = DaoxHexTerrain_GetHeight( self, unit->center->pos.x, unit->center->pos.y );

	for(i=0; i<6; ++i){
		DaoxHexPoint *point = unit->spokes[i]->end;
		point->pos.z = DaoxHexTerrain_GetHeight( self, point->pos.x, point->pos.y );
	}
	for(i=0; i<6; ++i) DaoxHexTerrain_Refine( self, unit, unit->splits[i] );
}
int DaoxHexBorder_CountPoints( DaoxHexBorder *self )
{
	int k = 0;
	if( self->left ){
		k += DaoxHexBorder_CountPoints( self->left );
		k += DaoxHexBorder_CountPoints( self->right );
		return k + 1;
	}
	return 0;
}
void DaoxHexTerrain_Adjust( DaoxHexTerrain *self, DaoxHexUnit *unit, DaoxHexTriangle *triangle )
{
	int i, k = 0, divs = 0;
	if( triangle->splits[0] != NULL ){
		for(i=0; i<4; ++i) DaoxHexTerrain_Adjust( self, unit, triangle->splits[i] );
		return;
	}
	for(i=0; i<3; ++i){
		if( triangle->borders[i]->left != NULL ){
			divs += 1;
			k = i;
		}
	}
	if( divs >= 2 ){
		DaoxHexTerrain_Split( self, unit, triangle );
		self->changes += 1;
	}else if( divs == 1 ){
		int count = DaoxHexBorder_CountPoints( triangle->borders[k] );
		if( count >= 3 ){
			DaoxHexTerrain_Split( self, unit, triangle );
			self->changes += 1;
		}
	}
}
void DaoxHexTerrain_AdjustUnit( DaoxHexTerrain *self, DaoxHexUnit *unit )
{
	int i;
	for(i=0; i<6; ++i) DaoxHexTerrain_Adjust( self, unit, unit->splits[i] );
}
void DaoxHexBorder_GetPoints( DaoxHexBorder *self, DList *points )
{
	if( self->left ){
		DaoxHexBorder_GetPoints( self->left, points );
		DaoxHexBorder_GetPoints( self->right, points );
		return;
	}
	DList_Append( points, self->end );
}
static void DaoxHexTerrain_UpdateNormals( DaoxHexPoint *A, DaoxHexPoint *B, DaoxHexPoint *C )
{
	DaoxVertex_UpdateNormalTangent( (DaoxVertex*) A, (DaoxVertex*) B, (DaoxVertex*) C, 1, 1 );
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
void DaoxHexTerrain_ComputeNormalTangents( DaoxHexTerrain *self, DaoxHexUnit *unit, DaoxHexTriangle *triangle )
{
	int i, k = 0, divs = 0;
	if( triangle->splits[0] != NULL ){
		for(i=0; i<4; ++i) DaoxHexTerrain_ComputeNormalTangents( self, unit, triangle->splits[i] );
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
		DaoxHexPoint *p0 = triangle->points[(k+2)%3];
		DList *points = self->buffer;
		points->size = 0;
		DList_Append( points, triangle->borders[k]->start );
		DaoxHexBorder_GetPoints( triangle->borders[k], points );
		if( triangle->points[k] == triangle->borders[k]->end ) DList_Reverse( points );
		for(i=1; i<points->size; ++i){
			DaoxHexPoint *p1 = (DaoxHexPoint*) points->items.pVoid[i-1];
			DaoxHexPoint *p2 = (DaoxHexPoint*) points->items.pVoid[i];
			DaoxHexTerrain_UpdateNormals( p0, p1, p2 );
		}
		return;
	}
	DaoxHexTerrain_UpdateNormals( triangle->points[0], triangle->points[1], triangle->points[2] );
}
void DaoxHexUnit_ComputeTextureCoordinates( DaoxHexUnit *unit, DaoxHexBorder *border )
{
	DaoxHexPoint *mid;

	if( border->left == NULL ) return;

	mid = border->left->end;
	if( mid == border->start || mid == border->end ) mid = border->left->start;
	mid->tex = DaoxVector2D_Interpolate( border->start->tex, border->end->tex, 0.5 );

	DaoxHexUnit_ComputeTextureCoordinates( unit, border->left );
	DaoxHexUnit_ComputeTextureCoordinates( unit, border->right );
}
void DaoxHexUnit_ResetVertices( DaoxHexUnit *unit, DaoxHexBorder *border )
{
	if( border->left ){
		DaoxHexUnit_ResetVertices( unit, border->left );
		DaoxHexUnit_ResetVertices( unit, border->right );
		return;
	}
	border->start->id = border->end->id = 0;
}
void DaoxHexTerrain_ResetVertices( DaoxHexTerrain *self, DaoxHexUnit *unit, DaoxHexTriangle *triangle )
{
	int i;
	if( triangle->splits[0] != NULL ){
		for(i=0; i<3; ++i){
			DaoxHexBorder *border = triangle->borders[i];
			DaoxHexPoint *mid = border->left->end;
			if( mid == border->start || mid == border->end ) mid = border->left->start;
			mid->tex = DaoxVector2D_Interpolate( border->start->tex, border->end->tex, 0.5 );
		}
		for(i=0; i<4; ++i) DaoxHexTerrain_ResetVertices( self, unit, triangle->splits[i] );
		return;
	}
	/*
	// Borders should be used to export vertices, because they may belong to
	// triangles from other tiles.
	*/
	for(i=0; i<3; ++i){
		DaoxHexUnit_ComputeTextureCoordinates( unit, triangle->borders[i] );
		DaoxHexUnit_ResetVertices( unit, triangle->borders[i] );
	}
}
void DaoxHexUnit_ExportVertices( DaoxHexUnit *unit, DaoxHexBorder *border )
{
	if( border->left ){
		DaoxHexUnit_ExportVertices( unit, border->left );
		DaoxHexUnit_ExportVertices( unit, border->right );
		return;
	}
	if( border->start->id == 0 ){
		DaoxVertex *vertex = (DaoxVertex*) DArray_Push( unit->mesh->vertices );
		vertex->pos = border->start->pos;
		vertex->norm = DaoxVector3D_Normalize( & border->start->norm );
		vertex->tan = DaoxVector3D_Normalize( & border->start->tan );
		vertex->tex = border->start->tex;
		border->start->id = unit->mesh->vertices->size;
	}
	if( border->end->id == 0 ){
		DaoxVertex *vertex = (DaoxVertex*) DArray_Push( unit->mesh->vertices );
		vertex->pos = border->end->pos;
		vertex->norm = DaoxVector3D_Normalize( & border->end->norm );
		vertex->tan = DaoxVector3D_Normalize( & border->end->tan );
		vertex->tex = border->end->tex;
		border->end->id = unit->mesh->vertices->size;
	}
}
void DaoxHexTerrain_ExportVertices( DaoxHexTerrain *self, DaoxHexUnit *unit, DaoxHexTriangle *triangle )
{
	int i;
	if( triangle->splits[0] != NULL ){
		for(i=0; i<4; ++i) DaoxHexTerrain_ExportVertices( self, unit, triangle->splits[i] );
		return;
	}
	/*
	// Borders should be used to export vertices, because they may belong to
	// triangles from other tiles.
	*/
	for(i=0; i<3; ++i){
		DaoxHexUnit_ExportVertices( unit, triangle->borders[i] );
	}
}
void DaoxHexUnit_AddTriangles( DaoxHexUnit *self, DaoxHexPoint *start, DList *points )
{
	DaoxTriangle *mt;
	int i;
	for(i=1; i<points->size; ++i){
		DaoxHexPoint *p1 = (DaoxHexPoint*) points->items.pVoid[i-1];
		DaoxHexPoint *p2 = (DaoxHexPoint*) points->items.pVoid[i];
		mt = (DaoxTriangle*) DArray_Push( self->mesh->triangles );
		mt->index[0] = start->id - 1;
		mt->index[1] = p1->id - 1;
		mt->index[2] = p2->id - 1;
	}
}
void DaoxHexTerrain_ExportTriangles( DaoxHexTerrain *self, DaoxHexUnit *unit, DaoxHexTriangle *triangle )
{
	DaoxTriangle *mt;
	int i, k = 0, divs = 0;
	if( triangle->splits[0] != NULL ){
		for(i=0; i<4; ++i) DaoxHexTerrain_ExportTriangles( self, unit, triangle->splits[i] );
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
		DaoxHexTerrain_Split( self, unit, triangle );
		DaoxHexTerrain_ExportTriangles( self, unit, triangle );
		return;
	}else if( divs == 1 ){
		DList *points = self->buffer;
		points->size = 0;
		DList_Append( points, triangle->borders[k]->start );
		DaoxHexBorder_GetPoints( triangle->borders[k], points );
		if( triangle->points[k] == triangle->borders[k]->end ) DList_Reverse( points );
		DaoxHexUnit_AddTriangles( unit, triangle->points[(k+2)%3], points );
		return;
	}
	mt = (DaoxTriangle*) DArray_Push( unit->mesh->triangles );
	for(i=0; i<3; ++i){
		DaoxHexPoint *p2 = triangle->points[i];
		mt->index[i] = triangle->points[i]->id - 1;
	}
}
void DaoxHexUnit_InitTextureCoordinates( DaoxHexUnit *self )
{
	int i;

	self->center->tex.x = self->center->tex.y = 0.5;
	for(i=0; i<6; ++i){
		self->spokes[i]->end->tex.x = 0.5 + 0.4 * cos( i*M_PI/3.0 );
		self->spokes[i]->end->tex.y = 0.5 + 0.4 * sin( i*M_PI/3.0 );
	}
}
void DaoxHexTerrain_BuildMesh( DaoxHexTerrain *self, DaoxHexUnit *unit )
{
	int i;
	unit->mesh->vertices->size = unit->mesh->triangles->size = 0;
	DaoxHexUnit_InitTextureCoordinates( unit );
	for(i=0; i<6; ++i) DaoxHexTerrain_ResetVertices( self, unit, unit->splits[i] );
	for(i=0; i<6; ++i) DaoxHexTerrain_ExportVertices( self, unit, unit->splits[i] );
	for(i=0; i<6; ++i) DaoxHexTerrain_ExportTriangles( self, unit, unit->splits[i] );
	printf( "vertices: %i\n", unit->mesh->vertices->size );
	printf( "triangles: %i\n", unit->mesh->triangles->size );
	for(i=0; i<unit->mesh->vertices->size; ++i){
		DaoxVertex *vertex = unit->mesh->vertices->data.vertices + i;
		vertex->tex.x *= self->textureScale;
		vertex->tex.y *= self->textureScale;
	}

}
void DaoxHexTerrain_FinalizeMesh( DaoxHexTerrain *self )
{
	int i, j, k, count = self->rows * self->columns;

	self->changes = 1;
	while( self->changes ){
		self->changes = 0;
		for(i=0; i<count; ++i){
			DaoxHexUnit *unit = (DaoxHexUnit*) self->tiles->items.pVoid[i];
			DaoxHexTerrain_AdjustUnit( self, unit );
		}
		printf( "adjust: %i\n", self->changes );
	}
	for(i=0; i<count; ++i){
		DaoxHexUnit *unit = (DaoxHexUnit*) self->tiles->items.pVoid[i];
		for(j=0; j<6; ++j) DaoxHexTerrain_ResetVertices( self, unit, unit->splits[j] );
		for(j=0; j<6; ++j) DaoxHexTerrain_ComputeNormalTangents( self, unit, unit->splits[j] );
	}
	for(i=0; i<count; ++i){
		DaoxHexUnit *unit = (DaoxHexUnit*) self->tiles->items.pVoid[i];
		DaoxHexTerrain_BuildMesh( self, unit );
	}
	printf( "DaoxHexTerrain_Rebuild %i %i\n", count, self->mesh->units->size );
	DaoxMesh_UpdateTree( self->mesh, 1024 );
	DaoxMesh_ResetBoundingBox( self->mesh );
	self->base.obbox = self->mesh->obbox;
}
void DaoxHexTerrain_Rebuild( DaoxHexTerrain *self )
{
	int i, j, k, count = self->rows * self->columns;
	printf( "DaoxHexTerrain_Rebuild\n" );
	DaoxHexTerrain_InitializeTiles( self );
	for(i=0; i<count; ++i){
		DaoxHexUnit *unit = (DaoxHexUnit*) self->tiles->items.pVoid[i];
		DaoxHexTerrain_RebuildUnit( self, unit );
	}
	DaoxHexTerrain_FinalizeMesh( self );
}


typedef struct DaoxTerrainProcedure DaoxTerrainProcedure;

struct DaoxTerrainProcedure
{
	float              faultDist;
	DaoxVector2D       faultPoint;
	DaoxVector2D       faultNorm;
	DaoRandGenerator  *randGenerator;
};

void DaoxHexTerrain_ApplyFaultLine2( DaoxHexTerrain *self, DaoxHexUnit *unit, DaoxHexTriangle *triangle, DaoxTerrainProcedure *procedure )
{
	DaoxVector2D point1 = DaoxVector2D_Vector3D( triangle->points[1]->pos );
	DaoxVector2D point2 = DaoxVector2D_Vector3D( triangle->points[2]->pos );
	float sin60 = sin( M_PI / 3.0 );
	float width = (self->columns - 1) * 1.5 * self->radius + 2.0 * self->radius + EPSILON + 1;
	float length = self->rows * 2.0 * self->radius * sin60 + self->radius * sin60 + EPSILON + 1;
	float diameter = sqrt(width*width + length*length);
	float faultScale1 = 0.01 * diameter;
	float faultScale2 = 0.05 * diameter;
	float len = DaoxVector2D_Dist( point1, point2 );
	float minSize = 0.01 * diameter;
	float maxChange = 0.01 * diameter;
	float minDistToPoint = diameter;
	float distToFaultLine[3];
	float distToFaultPoint[3];
	float noise;
	int i;

	noise = DaoRandGenerator_GetNormal( procedure->randGenerator );
	if( noise >  1.0 ) noise =  1.0;
	if( noise < -1.0 ) noise = -1.0;
	maxChange += 0.1 * minSize * noise;
	procedure->faultDist = fabs( procedure->faultDist );
	if( triangle->splits[0] != NULL ){
		for(i=0; i<4; ++i) DaoxHexTerrain_ApplyFaultLine2( self, unit, triangle->splits[i], procedure );
		return;
	}
	for(i=0; i<3; ++i){
		DaoxVector2D point = DaoxVector2D_Vector3D( triangle->points[i]->pos );
		distToFaultPoint[i] = DaoxVector2D_Dist( point, procedure->faultPoint );
		if( minDistToPoint > distToFaultPoint[i] ) minDistToPoint = distToFaultPoint[i];

		point = DaoxVector2D_Sub( & point, & procedure->faultPoint );
		distToFaultLine[i] = DaoxVector2D_Dot( & point, & procedure->faultNorm );
	}
	if( distToFaultLine[0] * distToFaultLine[1] < 0.0 || distToFaultLine[0] * distToFaultLine[2] < 0.0 ){
		float prob;
		if( len <= minSize ) return ;
		prob = exp( - minDistToPoint / faultScale2 );
		if( DaoRandGenerator_GetUniform( procedure->randGenerator ) > prob ) return;
		DaoxHexTerrain_Split( self, unit, triangle );
		for(i=0; i<4; ++i) DaoxHexTerrain_ApplyFaultLine2( self, unit, triangle->splits[i], procedure );
		return;
	}
	for(i=0; i<3; ++i){
		float factor1 = distToFaultLine[i] / (fabs(distToFaultLine[i]) + faultScale1);
		float factor2 = exp( - distToFaultPoint[i] / faultScale2 );
		float offset = maxChange * factor1 * factor2;
		triangle->points[i]->pos.z += offset;
	}
}
void DaoxHexTerrain_ApplyFaultLine( DaoxHexTerrain *self, DaoxTerrainProcedure *procedure )
{
	int i, j, k, count = self->rows * self->columns;

	for(i=0; i<count; ++i){
		DaoxHexUnit *unit = (DaoxHexUnit*) self->tiles->items.pVoid[i];
		DaoxHexUnit_InitTextureCoordinates( unit );
		for(j=0; j<6; ++j) DaoxHexTerrain_ResetVertices( self, unit, unit->splits[j] );
	}
	for(i=0; i<count; ++i){
		DaoxHexUnit *unit = (DaoxHexUnit*) self->tiles->items.pVoid[i];
		for(j=0; j<6; ++j){
			DaoxHexTerrain_ApplyFaultLine2( self, unit, unit->splits[j], procedure );
		}
	}
}
void DaoxHexTerrain_Generate( DaoxHexTerrain *self, int seed )
{
	DaoxTerrainProcedure procedure;
	DaoRandGenerator *randgen;
	DaoxVector2D *faultPoint = & procedure.faultPoint;
	DaoxVector2D *faultNorm = & procedure.faultNorm;
	float sin60 = sin( M_PI / 3.0 );
	float width = (self->columns - 1) * 1.5 * self->radius + 2.0 * self->radius + EPSILON + 1;
	float length = self->rows * 2.0 * self->radius * sin60 + self->radius * sin60 + EPSILON + 1;
	float diameter = sqrt(width*width + length*length);
	int i, j, k, count = self->rows * self->columns;
	int steps = 50;// + sqrt( self->rows + self->columns );

	procedure.randGenerator = randgen = DaoRandGenerator_New( seed );
	printf( "DaoxHexTerrain_Generate:\n" );
	DaoxHexTerrain_InitializeTiles( self );
	for(i=0; i<count; ++i){
		DaoxHexUnit *unit = (DaoxHexUnit*) self->tiles->items.pVoid[i];
		unit->center->pos.z = 0.0;
		for(j=0; j<6; ++j) unit->spokes[j]->end->pos.z = 0.0;
	}
	for(i=0; i<steps; ++i){
		float randAngle;
		faultPoint->x = width * DaoRandGenerator_GetUniform( randgen );
		faultPoint->y = length * DaoRandGenerator_GetUniform( randgen );
		faultNorm->x = DaoRandGenerator_GetUniform( randgen ) - 0.5;
		faultNorm->y = DaoRandGenerator_GetUniform( randgen ) - 0.5;
		*faultNorm = DaoxVector2D_Normalize( faultNorm );
		procedure.faultDist = 0.5 * diameter;
		DaoxHexTerrain_ApplyFaultLine( self, & procedure );
		while( procedure.faultDist >= 0.05 * diameter ){
			DaoxVector2D faultDir = DaoxMatrix3D_RotateVector( *faultNorm, 0.5*M_PI );
			procedure.faultDist = procedure.faultDist * DaoRandGenerator_GetNormal( randgen );
			faultPoint->x += procedure.faultDist * faultDir.x;
			faultPoint->y += procedure.faultDist * faultDir.y;
			randAngle = 0.5 * M_PI * DaoRandGenerator_GetNormal( randgen );
			*faultNorm = DaoxMatrix3D_RotateVector( faultDir, randAngle );
			*faultNorm = DaoxVector2D_Normalize( faultNorm );
			if( DaoRandGenerator_GetUniform( randgen ) ){
				faultNorm->x = - faultNorm->x;
				faultNorm->y = - faultNorm->y;
			}
			procedure.faultDist = fabs( procedure.faultDist );
			DaoxHexTerrain_ApplyFaultLine( self, & procedure );
		}
	}
	DaoxHexTerrain_FinalizeMesh( self );
}
