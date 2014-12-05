/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2012-2014, Limin Fu
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
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "dao_canvas.h"




DaoxGradient* DaoxGradient_New( int type )
{
	DaoxGradient *self = (DaoxGradient*) dao_calloc(1,sizeof(DaoxGradient));
	DaoType *ctype = daox_type_gradient;
	switch( type ){
	case DAOX_GRADIENT_BASE : ctype = daox_type_gradient; break;
	case DAOX_GRADIENT_LINEAR : ctype = daox_type_linear_gradient; break;
	case DAOX_GRADIENT_RADIAL : ctype = daox_type_radial_gradient; break;
	case DAOX_GRADIENT_PATH : ctype = daox_type_path_gradient; break;
	}
	DaoCstruct_Init( (DaoCstruct*)self, ctype );
	self->stops = DArray_New( sizeof(float) );
	self->colors = DArray_New( sizeof(DaoxColor) );
	self->gradient = type;
	return self;
}
void DaoxGradient_Delete( DaoxGradient *self )
{
	DArray_Delete( self->stops );
	DArray_Delete( self->colors );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}
void DaoxGradient_Add( DaoxGradient *self, float stop, DaoxColor color )
{
	DaoxColor *C = (DaoxColor*) DArray_Push( self->colors );
	DArray_PushFloat( self->stops, stop );
	*C = color;
}
void DaoxGradient_Copy( DaoxGradient *self, DaoxGradient *other )
{
	self->gradient = other->gradient;
	self->radius = other->radius;
	self->points[0] = other->points[0];
	self->points[1] = other->points[1];
	DArray_Assign( self->stops, other->stops );
	DArray_Assign( self->colors, other->colors );
}







DaoxBrush* DaoxBrush_New()
{
	DaoxBrush *self = (DaoxBrush*) dao_calloc(1,sizeof(DaoxBrush));
	DaoCstruct_Init( (DaoCstruct*)self, daox_type_brush );
	DaoxPathStyle_Init( & self->strokeStyle );
	self->strokeColor.alpha = 1.0;
	self->fontSize = 12.0;
	return self;
}
void DaoxBrush_Delete( DaoxBrush *self )
{
	if( self->strokeGradient ) DaoGC_DecRC( (DaoValue*) self->strokeGradient );
	if( self->fillGradient ) DaoGC_DecRC( (DaoValue*) self->fillGradient );
	if( self->texture ) DaoGC_DecRC( (DaoValue*) self->texture );
	if( self->font ) DaoGC_DecRC( (DaoValue*) self->font );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}
void DaoxBrush_Copy( DaoxBrush *self, DaoxBrush *other )
{
	self->strokeStyle = other->strokeStyle;
	self->fontSize = other->fontSize;
	self->strokeColor = other->strokeColor;
	self->fillColor = other->fillColor;
	GC_Assign( & self->font, other->font );
	GC_Assign( & self->texture, other->texture );
	if( other->strokeGradient ){
		self->strokeGradient = DaoxGradient_New(0);
		DaoxGradient_Copy( self->strokeGradient, other->strokeGradient );
		GC_IncRC( self->strokeGradient );
	}
	if( other->fillGradient ){
		self->fillGradient = DaoxGradient_New(0);
		DaoxGradient_Copy( self->fillGradient, other->fillGradient );
		GC_IncRC( self->fillGradient );
	}
}
void DaoxBrush_SetStrokeWidth( DaoxBrush *self, float width )
{
	self->strokeStyle.width = width;
}
void DaoxBrush_SetStrokeColor( DaoxBrush *self, DaoxColor color )
{
	self->strokeColor = color;
}
void DaoxBrush_SetFillColor( DaoxBrush *self, DaoxColor color )
{
	self->fillColor = color;
}
void DaoxBrush_SetDashPattern( DaoxBrush *self, float pat[], int n )
{
	DaoxPathStyle_SetDashes( & self->strokeStyle, n, pat );
}
void DaoxBrush_SetFont( DaoxBrush *self, DaoxFont *font, float size )
{
	GC_Assign( & self->font, font );
	self->fontSize = size;
}




void DaoxCanvasNode_ResetTransform( DaoxCanvasNode *self )
{
	self->rotation = DaoxVector2D_XY( 1.0, 0.0 );
	self->translation = DaoxVector2D_XY( 0.0, 0.0 );
}
void DaoxCanvasNode_Init( DaoxCanvasNode *self, DaoType *type )
{
	memset( self, 0, sizeof(DaoxCanvasNode) );
	DaoCstruct_Init( (DaoCstruct*)self, type );
	self->scale = 1.0;
	self->visible = 1;
	self->changed = 1;
	self->moved = 1;
	DaoxCanvasNode_ResetTransform( self );
}
void DaoxCanvasNode_Free( DaoxCanvasNode *self )
{
	if( self->children ) DList_Delete( self->children );
	if( self->path ) DaoGC_DecRC( (DaoValue*) self->path );
	if( self->mesh ) DaoGC_DecRC( (DaoValue*) self->mesh );
	DaoGC_DecRC( (DaoValue*) self->parent );
	DaoGC_DecRC( (DaoValue*) self->brush );
	DaoCstruct_Free( (DaoCstruct*) self );
}



DaoxCanvasNode* DaoxCanvasNode_New( DaoType *type )
{
	DaoxCanvasNode *self = (DaoxCanvasNode*) dao_calloc( 1, sizeof(DaoxCanvasNode) );
	DaoxCanvasNode_Init( self, type );
	return self;
}
void DaoxCanvasNode_Delete( DaoxCanvasNode *self )
{
	DaoxCanvasNode_Free( self );
	dao_free( self );
}
void DaoxCanvasNode_MarkDataChanged( DaoxCanvasNode *self )
{
	self->changed = 1;
	self->moved = 1;
	if( self->parent ) DaoxCanvasNode_MarkStateChanged( self->parent );
}
void DaoxCanvasNode_MarkStateChanged( DaoxCanvasNode *self )
{
	self->moved = 1;
	if( self->parent ) DaoxCanvasNode_MarkStateChanged( self->parent );
}
DaoxMatrix3D DaoxCanvasNode_GetLocalTransform( DaoxCanvasNode *self )
{
	DaoxMatrix3D transform;
	transform.A11 =   self->rotation.x;
	transform.A12 = - self->rotation.y;
	transform.A21 =   self->rotation.y;
	transform.A22 =   self->rotation.x;
	transform.B1 = self->translation.x;
	transform.B2 = self->translation.y;
	return transform;
}
static void DArray_PushOBBoxVertexPoints2D( DArray *self, DaoxOBBox2D *box )
{
	DaoxVector2D dY = DaoxVector2D_Sub( & box->Y, & box->O );
	DaoxVector2D P = DaoxVector2D_Add( & box->X, & dY );

	DArray_Reserve( self, self->size + 4 );
	DArray_PushVector2D( self, & box->O );
	DArray_PushVector2D( self, & box->X );
	DArray_PushVector2D( self, & box->Y );
	DArray_PushVector2D( self, & P );
}
void DaoxCanvasNode_Update( DaoxCanvasNode *self, DaoxCanvas *canvas )
{
	DArray *points;
	daoint i;

	if( self->moved == 0 && self->changed == 0 ) return;
	if( self->ctype == daox_type_canvas_image ) return;

	if( self->path != NULL && (self->mesh == NULL || self->changed) ){
		DaoxBrush *brush = self->brush;
		DaoxPathStyle style = self->brush->strokeStyle;
		DaoxPathMesh *mesh;

		style.fill = brush->fillColor.alpha > EPSILON || brush->fillGradient != NULL;
		style.width = style.width / (self->scale + EPSILON);
		mesh = DaoxPathCache_FindMesh( canvas->pathCache, self->path, & style );
		GC_Assign( & self->mesh, mesh );
		self->changed = 0;
	}
	self->moved = 0;

	points = DArray_New( sizeof(DaoxVector2D) );
	if( self->path && self->mesh ){
		DaoxOBBox2D obbox = DaoxOBBox2D_Scale( & self->mesh->path->obbox, self->scale );
		if( self->mesh->strokePoints->size ){
			float strokeWidth = self->brush ? self->brush->strokeStyle.width : 0;
			strokeWidth *= self->scale + EPSILON;
			obbox = DaoxOBBox2D_CopyWithMargin( & obbox, 0.5*strokeWidth );
		}
		DArray_PushOBBoxVertexPoints2D( points, & obbox );
	}
	if( self->children ){
		for(i=0; i<self->children->size; ++i){
			DaoxCanvasNode *it = self->children->items.pCanvasNode[i];
			DaoxCanvasNode_Update( it, canvas );
			DArray_PushOBBoxVertexPoints2D( points, & it->obbox );
		}
	}
	DaoxOBBox2D_ResetBox( & self->obbox, points->data.vectors2d, points->size );
	DArray_Delete( points );
}




DaoxCanvasLine* DaoxCanvasLine_New()
{
	return DaoxCanvasNode_New( daox_type_canvas_line );
}

DaoxCanvasRect* DaoxCanvasRect_New()
{
	return DaoxCanvasNode_New( daox_type_canvas_rect );
}

DaoxCanvasCircle* DaoxCanvasCircle_New()
{
	return DaoxCanvasNode_New( daox_type_canvas_circle );
}

DaoxCanvasEllipse* DaoxCanvasEllipse_New()
{
	return DaoxCanvasNode_New( daox_type_canvas_ellipse );
}

DaoxCanvasPath* DaoxCanvasPath_New()
{
	DaoxCanvasPath *self = (DaoxCanvasPath*)dao_calloc( 1, sizeof(DaoxCanvasPath));
	DaoxCanvasNode_Init( self,  daox_type_canvas_path );
	return self;
}

DaoxCanvasImage* DaoxCanvasImage_New()
{
	DaoxCanvasImage *self = (DaoxCanvasImage*)dao_calloc( 1, sizeof(DaoxCanvasImage));
	DaoxCanvasNode_Init( self,  daox_type_canvas_image );
	return self;
}

DaoxCanvasText* DaoxCanvasText_New()
{
	DaoxCanvasText *self = (DaoxCanvasText*) dao_calloc( 1, sizeof(DaoxCanvasText) );
	DaoxCanvasNode_Init( self, daox_type_canvas_text );
	return self;
}




void DaoxCanvasLine_Set( DaoxCanvasLine *self, float x1, float y1, float x2, float y2 )
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	float r = sqrt( dx*dx + dy*dy );
	float cosine = dx / (r + EPSILON);
	float sine = dy / (r + EPSILON);

	assert( self->ctype == daox_type_canvas_line );
	self->scale = r / DAOX_PATH_UNIT;
	self->rotation.x = cosine;
	self->rotation.y = sine;
	self->translation.x = x1;
	self->translation.y = y1;
	self->changed = 1;
}

void DaoxCanvasRect_Set( DaoxCanvasRect *self, float x1, float y1, float x2, float y2 )
{
	float w = fabs( x2 - x1 );
	float h = fabs( y2 - y1 );
	assert( self->ctype == daox_type_canvas_rect );
	DaoxCanvasNode_ResetTransform( self );
	self->translation.x = x1;
	self->translation.y = y1;
	self->changed = 1;
	self->scale = w / DAOX_PATH_UNIT;
}

void DaoxCanvasCircle_Set( DaoxCanvasCircle *self, float x, float y, float r )
{
	assert( self->ctype == daox_type_canvas_circle );
	self->scale = r / DAOX_PATH_UNIT;
	DaoxCanvasNode_ResetTransform( self );
	self->translation.x = x;
	self->translation.y = y;
	self->changed = 1;
}

void DaoxCanvasEllipse_Set( DaoxCanvasEllipse *self, float x, float y, float rx, float ry )
{
	assert( self->ctype == daox_type_canvas_ellipse );
	self->scale = rx / DAOX_PATH_UNIT;
	DaoxCanvasNode_ResetTransform( self );
	self->translation.x = x;
	self->translation.y = y;
	self->changed = 1;
}




#if 0
void DaoxCanvasPath_SetRelativeMode( DaoxCanvasPath *self, int relative )
{
	DaoxPath_SetRelativeMode( self->path, relative );
}
void DaoxCanvasPath_MoveTo( DaoxCanvasPath *self, float x, float y )
{
	assert( self->ctype == daox_type_canvas_path );
	DaoxPath_MoveTo( self->path, x, y );
}
void DaoxCanvasPath_LineTo( DaoxCanvasPath *self, float x, float y )
{
	assert( self->ctype == daox_type_canvas_path );
	DaoxPath_LineTo( self->path, x, y );
}
void DaoxCanvasPath_ArcTo( DaoxCanvasPath *self, float x, float y, float degrees )
{
	assert( self->ctype == daox_type_canvas_path );
	DaoxPath_ArcTo( self->path, x, y, degrees );
}
void DaoxCanvasPath_ArcBy( DaoxCanvasPath *self, float cx, float cy, float degrees )
{
	assert( self->ctype == daox_type_canvas_path );
	DaoxPath_ArcBy( self->path, cx, cy, degrees );
}
void DaoxCanvasPath_QuadTo( DaoxCanvasPath *self, float cx, float cy, float x, float y )
{
	assert( self->ctype == daox_type_canvas_path );
	DaoxPath_QuadTo( self->path, cx, cy, x, y );
}
void DaoxCanvasPath_CubicTo( DaoxCanvasPath *self, float cx, float cy, float x, float y )
{
	assert( self->ctype == daox_type_canvas_path );
	DaoxPath_CubicTo( self->path, cx, cy, x, y );
}
void DaoxCanvasPath_CubicTo2( DaoxCanvasLine *self, float cx0, float cy0, float cx, float cy, float x, float y )
{
	assert( self->ctype == daox_type_canvas_path );
	DaoxPath_CubicTo2( self->path, cx0, cy0, cx, cy, x, y );
}
void DaoxCanvasPath_Close( DaoxCanvasPath *self )
{
	assert( self->ctype == daox_type_canvas_path );
	DaoxPath_Close( self->path );
}

#endif







DaoxCanvas* DaoxCanvas_New( DaoxPathCache *pathCache )
{
	DaoxCanvas *self = (DaoxCanvas*) dao_calloc( 1, sizeof(DaoxCanvas) );
	DaoxSceneNode_Init( (DaoxSceneNode*) self, daox_type_canvas );
	self->transform = DaoxMatrix3D_Identity();
	self->brushes = DList_New( DAO_DATA_VALUE );
	self->nodes = DList_New( DAO_DATA_VALUE );
	self->actives = DList_New(0); /* No GC, item GC handled by ::nodes; */

	self->auxPath = DaoxPath_New();
	DaoGC_IncRC( (DaoValue*) self->auxPath );

	if( pathCache == NULL ) pathCache = DaoxPathCache_New();
	GC_Assign( & self->pathCache, pathCache );

	self->imageCache = DHash_New(0,0); /* No GC, item GC handled by ::brushes; */
	return self;
}
void DaoxCanvas_Delete( DaoxCanvas *self )
{
	DaoxSceneNode_Free( (DaoxSceneNode*) self );
	DList_Delete( self->nodes );
	DList_Delete( self->actives );
	DList_Delete( self->brushes );
	DMap_Delete( self->imageCache );
	GC_DecRC( self->pathCache );
	GC_DecRC( self->auxPath );
	dao_free( self );
}


void DaoxCanvas_SetViewport( DaoxCanvas *self, float left, float right, float bottom, float top )
{
	self->viewport.left = left;
	self->viewport.right = right;
	self->viewport.bottom = bottom;
	self->viewport.top = top;
}
void DaoxCanvas_SetBackground( DaoxCanvas *self, DaoxColor color )
{
	self->background = color;
}
DaoxBrush* DaoxCanvas_GetCurrentBrush( DaoxCanvas *self )
{
	if( self->brushes->size == 0 ) return NULL;
	return self->brushes->items.pBrush[self->brushes->size-1];
}
DaoxBrush* DaoxCanvas_GetOrPushBrush( DaoxCanvas *self )
{
	if( self->brushes->size == 0 ) DaoxCanvas_PushBrush( self, 0 );
	return DaoxCanvas_GetCurrentBrush( self );
}
DaoxBrush* DaoxCanvas_PushBrush( DaoxCanvas *self, int index )
{
	DaoxBrush *prev = DaoxCanvas_GetCurrentBrush( self );
	DaoxBrush *brush = NULL;
	if( index >=0 && index < self->brushes->size ){
		brush = (DaoxBrush*) self->brushes->items.pVoid[index];
	}else{
		brush = DaoxBrush_New();
		if( prev ) DaoxBrush_Copy( brush, prev );
	}
	DList_PushBack( self->brushes, brush );
	return brush;
}
void DaoxCanvas_PopBrush( DaoxCanvas *self )
{
	DList_PopBack( self->brushes );
}


void DaoxCanvas_UpdatePathMesh( DaoxCanvas *self, DaoxCanvasNode *node )
{
	DaoxPathMesh *mesh;
	DaoxBrush *brush = node->brush;
	DaoxPathStyle style = node->brush->strokeStyle;

	style.fill = brush->fillColor.alpha > EPSILON || brush->fillGradient != NULL;
	style.width = style.width / (node->scale + EPSILON);
	if( node->path == NULL ) return;

	mesh = DaoxPathCache_FindMesh( self->pathCache, node->path, & style );
	GC_Assign( & node->mesh, mesh );
}
void DaoxCanvas_AddNode( DaoxCanvas *self, DaoxCanvasNode *node )
{
	DaoxBrush *brush = node->brush;
	
	if( brush == NULL ){
		brush = DaoxCanvas_GetOrPushBrush( self );
		GC_Assign( & node->brush, brush );
	}
	DaoxCanvas_UpdatePathMesh( self, node );

	if( self->actives->size ){
		DaoxCanvasNode *activeNode = (DaoxCanvasNode*) DList_Back( self->actives );
		if( activeNode->children == NULL ) activeNode->children = DList_New( DAO_DATA_VALUE );
		GC_Assign( & node->parent, activeNode );
		DList_PushBack( activeNode->children, node );
		DaoxCanvasNode_MarkDataChanged( activeNode );
	}else{
		DList_PushBack( self->nodes, node );
	}
}

DaoxCanvasNode* DaoxCanvas_AddGroup( DaoxCanvas *self )
{
	DaoxCanvasNode *node = DaoxCanvasNode_New( daox_type_canvas_node );
	DaoxCanvas_AddNode( self, node );
	DList_PushBack( self->actives, node );
	return node;
}

DaoxCanvasLine* DaoxCanvas_AddLine( DaoxCanvas *self, float x1, float y1, float x2, float y2 )
{
	DaoxCanvasLine *node = DaoxCanvasLine_New();
	DaoxCanvasLine_Set( node, x1, y1, x2, y2 );
	DaoxCanvas_AddNode( self, node );
	DaoGC_IncRC( (DaoValue*) self->pathCache->unitLine );
	node->path = self->pathCache->unitLine;
	return node;
}

DaoxCanvasRect* DaoxCanvas_AddRect( DaoxCanvas *self, float x1, float y1, float x2, float y2, float rx, float ry )
{
	float w = fabs(x2 - x1);
	float h = fabs(y2 - y1);
	float r = h / (w + EPSILON);
	float rx2 = fabs(rx) / (w + EPSILON);
	float ry2 = fabs(ry) / (h + EPSILON);
	DaoxCanvasRect *node = DaoxCanvasRect_New();
	DaoxPath *path = self->auxPath;

	DaoxPath_Reset( path );
	DaoxPath_SetRelativeMode( path, 1 );
	if( fabs( rx2 ) > 1E-3 && fabs( ry2 ) > 1E-3 ){
		r *= DAOX_PATH_UNIT;
		ry2 *= r;
		rx2 *= DAOX_PATH_UNIT;
		DaoxPath_MoveTo( path, 0.0, ry2 );
		DaoxPath_CubicTo2( path, 0.0, -0.55*ry2, -0.55*rx2, 0.0, rx2, -ry2 );
		DaoxPath_LineTo( path, DAOX_PATH_UNIT - 2.0*rx2, 0.0 );
		DaoxPath_CubicTo2( path, 0.55*rx2, 0.0, 0.0, -0.55*ry2, rx2, ry2 );
		DaoxPath_LineTo( path, 0.0, r - 2.0*ry2 );
		DaoxPath_CubicTo2( path, 0.0, 0.55*ry2, 0.55*rx2, 0.0, -rx2, ry2 );
		DaoxPath_LineTo( path, -(DAOX_PATH_UNIT - 2.0*rx2), 0.0 );
		DaoxPath_CubicTo2( path, -0.55*rx2, 0.0, 0.0, 0.55*ry2, -rx2, -ry2 );
	}else{
		DaoxPath_MoveTo( path, 0.0, 0.0 );
		DaoxPath_LineTo( path, DAOX_PATH_UNIT, 0.0 );
		DaoxPath_LineTo( path, 0.0, r*DAOX_PATH_UNIT );
		DaoxPath_LineTo( path, -DAOX_PATH_UNIT, 0.0 );
	}
	DaoxPath_Close( path );

	node->path = DaoxPathCache_FindPath( self->pathCache, path );
	DaoGC_IncRC( (DaoValue*) node->path );

	DaoxCanvasRect_Set( node, x1, y1, x2, y2 );
	DaoxCanvas_AddNode( self, node );
	return node;
}

DaoxCanvasCircle* DaoxCanvas_AddCircle( DaoxCanvas *self, float x, float y, float r )
{
	DaoxCanvasCircle *node = DaoxCanvasCircle_New();
	DaoxPath *circle = self->pathCache->unitCircle3;

	if( r < 8.0 ){
		circle = self->pathCache->unitCircle1;
	}else if( r < 32.0 ){
		circle = self->pathCache->unitCircle2;
	}
	DaoGC_IncRC( (DaoValue*) circle );
	node->path = circle;

	DaoxCanvasCircle_Set( node, x, y, r );
	DaoxCanvas_AddNode( self, node );
	return node;
}

DaoxCanvasEllipse* DaoxCanvas_AddEllipse( DaoxCanvas *self, float x, float y, float rx, float ry )
{
	DaoxPath *path = self->auxPath;
	DaoxCanvasEllipse *node = DaoxCanvasEllipse_New();
	DaoxMatrix3D mat = DaoxMatrix3D_Identity();

	mat.A22 = ry / (rx + EPSILON);
	DaoxPath_Reset( path );
	DaoxPath_ImportPath( path, self->pathCache->unitCircle3, & mat );

	node->path = DaoxPathCache_FindPath( self->pathCache, path );
	DaoGC_IncRC( (DaoValue*) node->path );

	DaoxCanvasEllipse_Set( node, x, y, rx, ry );
	DaoxCanvas_AddNode( self, node );
	return node;
}


DaoxCanvasPath* DaoxCanvas_AddPath( DaoxCanvas *self, DaoxPath *path )
{
	DaoxCanvasPath *node = DaoxCanvasPath_New();
	DaoGC_IncRC( (DaoValue*) path );
	node->path = path;
	DaoxCanvas_AddNode( self, node );
	return node;
}
void DaoxCanvas_AddCharNodes( DaoxCanvas *self, DaoxCanvasText *textItem, DArray *text, float x, float y, float degrees )
{
	DaoxGlyph *glyph;
	DaoxBrush *brush;
	DaoxCanvasText *chnode;
	DaoxPath *textPath = textItem->path;
	DaoxFont *font = textItem->brush->font;
	DaoxVector2D rotation = {1.0, 0.0};
	DaoxVector2D charRotation = {1.0, 0.0};
	DaoxVector2D charTranslation = {0.0, 0.0};
	double width = textItem->brush->strokeStyle.width;
	double size = textItem->brush->fontSize;
	double scale = size / (float)font->fontHeight;
	double offset, advance, angle = degrees * M_PI / 180.0;
	daoint i;

	if( text->stride != 4 ) return;

	rotation.x = cos( angle );
	rotation.y = sin( angle );
	charRotation = rotation;

	DList_PushBack( self->actives, textItem );

	offset = x;
	for(i=0; i<text->size; ++i){
		DaoxAABBox2D bounds = {0.0,0.0,0.0,0.0};
		DaoxMatrix3D rotmat = {0.0,0.0,0.0,0.0,0.0,0.0};
		size_t ch = text->data.uints[i];
		glyph = DaoxFont_GetGlyph( font, ch );
		if( glyph == NULL ) break;

		rotmat.A11 = rotmat.A22 = rotation.x;
		rotmat.A12 = - rotation.y;
		rotmat.A21 = rotation.y;
		bounds.right = glyph->advanceWidth;
		bounds.top = font->lineSpace;
		bounds = DaoxAABBox2D_Transform( & bounds, & rotmat );
		advance = bounds.right - bounds.left;

		if( textItem->children == NULL ) textItem->children = DList_New( DAO_DATA_VALUE );

		chnode = DaoxCanvasPath_New();
		chnode->path = DaoxPathCache_FindPath( self->pathCache, glyph->shape );
		chnode->scale = scale;
		DaoGC_IncRC( (DaoValue*) chnode->path );
		DaoxCanvas_AddNode( self, chnode );
		
		if( textPath ){
			float adv = 0.99 * (scale * advance + width);
			DaoxVector3D pos1 = {0.0,0.0,0.0};
			DaoxVector3D pos2 = {0.0,0.0,0.0};
			DaoxPathSegment *res1 = DaoxPath_LocateByDistance( textPath, offset, & pos1 );
			DaoxPathSegment *res2 = DaoxPath_LocateByDistance( textPath, offset+adv, & pos2 );

			printf( ">> %c: %f %f %f\n", ch, pos1.x, pos1.y, pos1.z );

			if( pos1.z > -EPSILON ){
				float dx = pos2.x - pos1.x;
				float dy = pos2.y - pos1.y;
				float r = sqrt( dx*dx + dy*dy );
				float cos1 = dx / r;
				float sin1 = dy / r;
				float cos2 = rotation.x;
				float sin2 = rotation.y;
				charRotation.x = cos1 * cos2 - sin1 * sin2;
				charRotation.y = sin1 * cos2 + cos1 * sin2;
				charTranslation.x = pos1.x;
				charTranslation.y = pos1.y;
			}else{
				charTranslation.x += scale * advance + width;
			}
		}else{
			charTranslation.x = offset;
			charTranslation.y = y;
		}
		chnode->rotation = charRotation;
		chnode->translation = charTranslation;
		offset += scale * advance + width;
		chnode = NULL;
	}
	DList_PopBack( self->actives );
}
DaoxCanvasText* DaoxCanvas_AddText( DaoxCanvas *self, const char *text, float x, float y, float degrees )
{
	DaoxCanvasPath *node;
	DaoxBrush *brush;
	DString str = DString_WrapChars( text );
	DArray *codepoints;

	if( self->brushes->size == 0 ) return NULL;
	brush = DaoxCanvas_GetOrPushBrush( self );
	if( brush->font == NULL ) GC_Assign( & brush->font, DaoxFont_GetDefault() );

	node = DaoxCanvasText_New();
	DaoxCanvas_AddNode( self, node );
	codepoints = DArray_New( sizeof(uint_t) );
	DString_DecodeUTF8( & str, codepoints );
	DaoxCanvas_AddCharNodes( self, node, codepoints, x, y, degrees );
	DArray_Delete( codepoints );
	return node;
}
DaoxCanvasText* DaoxCanvas_AddPathText( DaoxCanvas *self, const char *text, DaoxPath *path, float degrees )
{
	DaoxCanvasPath *node;
	DaoxBrush *brush;
	DString str = DString_WrapChars( text );
	DArray *codepoints;

	if( self->brushes->size == 0 ) return NULL;
	brush = DaoxCanvas_GetOrPushBrush( self );
	if( brush->font == NULL ) return NULL;

	node = DaoxCanvasText_New();
	DaoxCanvas_AddNode( self, node );

	DaoxPath_Refine( path, 0.02*brush->fontSize, 0.02 );
	GC_Assign( & node->path, path );
	node->visible = 0;

	codepoints = DArray_New( sizeof(uint_t) );
	DString_DecodeUTF8( & str, codepoints );
	DaoxCanvas_AddCharNodes( self, node, codepoints, 0, 0, degrees );
	DArray_Delete( codepoints );
	return node;
}
DaoxCanvasImage* DaoxCanvas_AddImage( DaoxCanvas *self, DaoxImage *image, float x, float y, float w )
{
	DaoxCanvasPath *node = DaoxCanvasImage_New();
	DNode *it = DMap_Find( self->imageCache, image );

	if( it == NULL ){
		DaoxBrush *brush = DaoxCanvas_GetOrPushBrush( self );
		DaoxTexture *texture = DaoxTexture_New();
		DaoxTexture_SetImage( texture, image );
		GC_Assign( & brush->texture, texture );
		it = DMap_Insert( self->imageCache, image, brush );
	}
	GC_Assign( & node->brush, (DaoxBrush*) it->value.pValue );

	node->scale = w / image->width;
	node->translation.x = x;
	node->translation.y = y;
	node->obbox.O.x = node->obbox.O.y = 0;
	node->obbox.X.x = node->obbox.X.y = 0;
	node->obbox.Y.x = node->obbox.Y.y = 0;
	node->obbox.X.x = image->width;
	node->obbox.Y.y = image->height;

	DaoxCanvas_AddNode( self, node );
	return node;
}



