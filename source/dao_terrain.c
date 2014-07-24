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

void DaoxTerrain_SetSize( DaoxTerrain *self, float width, float length, float height )
{
	self->width = width;
	self->length = length;
	self->height = height;
}
void DaoxTerrain_SetHeightmap( DaoxTerrain *self, DaoxImage *heightmap )
{
	GC_Assign( & self->heightmap, heightmap );
}
void DaoxTerrain_SetMaterial( DaoxTerrain *self, DaoxMaterial *material )
{
	GC_Assign( & self->material, material );
}

float DaoxTerrain_GetPixel( DaoxTerrain *self, float x, float y )
{
	uchar_t *imgdata = self->heightmap->imageData;
	float hmx = (x * (self->heightmap->width-1)) / self->width;
	float hmy = (y * (self->heightmap->height-1)) / self->length;
	int hmx1 = (int) hmx, hmx2 = hmx1 + (hmx > hmx1);
	int hmy1 = (int) hmy, hmy2 = hmy1 + (hmy > hmy1);
	int widthStep = self->heightmap->widthStep;
	int pixelBytes = 1 + self->heightmap->depth;
	int pix0 = *(imgdata + hmy1 * widthStep + hmx1 * pixelBytes);
	int pix1 = *(imgdata + hmy1 * widthStep + hmx2 * pixelBytes);
	int pix2 = *(imgdata + hmy2 * widthStep + hmx2 * pixelBytes);
	int pix3 = *(imgdata + hmy2 * widthStep + hmx1 * pixelBytes);
	float alpha = hmx - hmx1, alpha2 = 1.0 - alpha;
	float beta = hmy - hmy1, beta2 = 1.0 - beta;
	float pix = alpha2 * beta2 * pix0 + alpha * beta * pix2;
	pix += alpha * beta2 * pix1 + alpha2 * beta * pix3;
	return pix;
}
float DaoxTerrain_GetHeight( DaoxTerrain *self, float x, float y )
{
	float pix = DaoxTerrain_GetPixel( self, x, y );
	return (self->height * pix) / 255.0;
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
	int mapWidth = self->heightmap->width;
	int mapHeight = self->heightmap->height;
	int pixelBytes = 1 + self->heightmap->depth;
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

	H0 = DaoxTerrain_GetPixel( self, corners[0]->pos.x, corners[0]->pos.y );
	H1 = DaoxTerrain_GetPixel( self, corners[1]->pos.x, corners[1]->pos.y );
	H2 = DaoxTerrain_GetPixel( self, corners[2]->pos.x, corners[2]->pos.y );
	H3 = DaoxTerrain_GetPixel( self, corners[3]->pos.x, corners[3]->pos.y );

	hmx1 = (corners[0]->pos.x * (mapWidth-1)) / self->width;
	hmx2 = (corners[1]->pos.x * (mapWidth-1)) / self->width;
	hmy1 = (corners[0]->pos.y * (mapHeight-1)) / self->length;
	hmy2 = (corners[3]->pos.y * (mapHeight-1)) / self->length;
	ihmx = (int) hmx1;
	ihmy = (int) hmy1;
	for(j=ihmy + (hmy1>ihmy); j<=hmy2; ++j){
		uchar_t *imgdata = self->heightmap->imageData + j * self->heightmap->widthStep;
		float beta = (j - hmy1) / (hmy2 - hmy1 + EPSILON);
		float beta2 = 1.0 - beta;
		for(i=ihmx + (hmx1>ihmx); i<=hmx2; ++i){
			float pixel = imgdata[ i * pixelBytes ];
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
	cell->minHeight = (self->height * min) / 255.0;
	cell->maxHeight = (self->height * max) / 255.0;
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
	float mapwidth = self->heightmap->width;
	float mapheight = self->heightmap->height;
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



