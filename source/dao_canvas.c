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




DaoType *daox_type_gradient = NULL;
DaoType *daox_type_linear_gradient = NULL;
DaoType *daox_type_radial_gradient = NULL;
DaoType *daox_type_path_gradient = NULL;
DaoType *daox_type_brush = NULL;
DaoType *daox_type_canvas = NULL;

DaoType *daox_type_canvas_node = NULL;
DaoType *daox_type_canvas_line = NULL;
DaoType *daox_type_canvas_rect = NULL;
DaoType *daox_type_canvas_circle = NULL;
DaoType *daox_type_canvas_ellipse = NULL;
DaoType *daox_type_canvas_polygon = NULL;
DaoType *daox_type_canvas_path = NULL;
DaoType *daox_type_canvas_text = NULL;
DaoType *daox_type_canvas_image = NULL;



float DaoxCanvas_Scale( DaoxCanvas *self );



DaoxColor DaoxColor_Interpolate( DaoxColor C1, DaoxColor C2, float start, float mid, float end )
{
	DaoxColor color;
	float at = (mid - start) / (end - start + 1E-9);
	color.red = (1.0 - at) * C1.red + at * C2.red;
	color.green = (1.0 - at) * C1.green + at * C2.green;
	color.blue = (1.0 - at) * C1.blue + at * C2.blue;
	color.alpha = (1.0 - at) * C1.alpha + at * C2.alpha;
	return color;
}







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


void DaoxColor_FromDaoValues( DaoxColor *self, DaoValue *values[] )
{
	self->red = values[0]->xFloat.value;
	self->green = values[1]->xFloat.value;
	self->blue = values[2]->xFloat.value;
	self->alpha = values[3]->xFloat.value;
}

static void GRAD_AddStop( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxGradient *self = (DaoxGradient*) p[0];
	DaoxColor color = {0.0,0.0,0.0,0.0};
	DaoxColor_FromDaoValues( & color, p[2]->xTuple.values );
	DaoxGradient_Add( self, p[1]->xFloat.value, color );
}
static void GRAD_AddStops( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxGradient *self = (DaoxGradient*) p[0];
	DaoList *stops = (DaoList*) p[1];
	int i, n = DaoList_Size( stops );
	for(i=0; i<n; ++i){
		DaoTuple *item = (DaoTuple*) DaoList_GetItem( stops, i );
		DaoxColor color = {0.0,0.0,0.0,0.0};
		DaoxColor_FromDaoValues( & color, item->values[1]->xTuple.values );
		DaoxGradient_Add( self, item->values[0]->xFloat.value, color );
	}
}

static DaoFuncItem DaoxGradientMeths[]=
{
	{ GRAD_AddStop,  "AddStop( self: ColorGradient, at: float, color: Color ) => ColorGradient" },
	{ GRAD_AddStops,  "AddStops( self: ColorGradient, stops: list<tuple<at:float,color:Color>> ) => ColorGradient" },
	{ NULL, NULL }
};

DaoTypeBase DaoxGradient_Typer =
{
	"ColorGradient", NULL, NULL, (DaoFuncItem*) DaoxGradientMeths, {NULL}, {NULL},
	(FuncPtrDel)DaoxGradient_Delete, NULL
};


static void LGRAD_SetStart( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxGradient *self = (DaoxGradient*) p[0];
	self->points[0].x = p[1]->xFloat.value;
	self->points[0].y = p[2]->xFloat.value;
}
static void LGRAD_SetEnd( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxGradient *self = (DaoxGradient*) p[0];
	self->points[1].x = p[1]->xFloat.value;
	self->points[1].y = p[2]->xFloat.value;
}

static DaoFuncItem DaoxLinearGradientMeths[]=
{
	{ LGRAD_SetStart,  "SetStart( self: LinearGradient, x: float, y: float )" },
	{ LGRAD_SetEnd,    "SetEnd( self: LinearGradient, x: float, y: float )" },
	{ NULL, NULL }
};

DaoTypeBase DaoxLinearGradient_Typer =
{
	"LinearGradient", NULL, NULL, (DaoFuncItem*) DaoxLinearGradientMeths,
	{ & DaoxGradient_Typer, NULL }, {NULL},
	(FuncPtrDel)DaoxGradient_Delete, NULL
};


static void RGRAD_SetRadius( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxGradient *self = (DaoxGradient*) p[0];
	self->radius = p[1]->xFloat.value;
}

static DaoFuncItem DaoxRadialGradientMeths[]=
{
	{ LGRAD_SetStart,  "SetCenter( self: RadialGradient, x: float, y: float )" },
	{ LGRAD_SetEnd,    "SetFocus( self: RadialGradient, x: float, y: float )" },
	{ RGRAD_SetRadius, "SetRadius( self: RadialGradient, r: float )" },
	{ NULL, NULL }
};

DaoTypeBase DaoxRadialGradient_Typer =
{
	"RadialGradient", NULL, NULL, (DaoFuncItem*) DaoxRadialGradientMeths,
	{ & DaoxGradient_Typer, NULL }, {NULL},
	(FuncPtrDel)DaoxGradient_Delete, NULL
};


static void PGRAD_AddStop( DaoProcess *proc, DaoValue *p[], int N )
{
}

static DaoFuncItem DaoxPathGradientMeths[]=
{
	{ NULL, NULL }
};

DaoTypeBase DaoxPathGradient_Typer =
{
	"PathGradient", NULL, NULL, (DaoFuncItem*) DaoxPathGradientMeths,
	{ & DaoxGradient_Typer, NULL}, {NULL},
	(FuncPtrDel)DaoxGradient_Delete, NULL
};









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
	if( self->strokeGradient ) DaoxGradient_Delete( self->strokeGradient );
	if( self->fillGradient ) DaoxGradient_Delete( self->fillGradient );
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
	if( other->strokeGradient ){
		self->strokeGradient = DaoxGradient_New(0);
		DaoxGradient_Copy( self->strokeGradient, other->strokeGradient );
	}
	if( other->fillGradient ){
		self->fillGradient = DaoxGradient_New(0);
		DaoxGradient_Copy( self->fillGradient, other->fillGradient );
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
	if( self->path ) DaoGC_DecRC( (DaoValue*) self->path );
	if( self->mesh ) DaoGC_DecRC( (DaoValue*) self->mesh );
	if( self->texture ) DaoGC_DecRC( (DaoValue*) self->texture );
	DaoCstruct_Free( (DaoCstruct*) self );
	DaoGC_DecRC( (DaoValue*) self->parent );
	DaoGC_DecRC( (DaoValue*) self->brush );
}



DaoxCanvasNode* DaoxCanvasNode_New( DaoType *type )
{
	DaoxCanvasNode *self = (DaoxCanvasNode*) dao_calloc( 1, sizeof(DaoxCanvasNode) );
	DaoxCanvasNode_Init( self, type );
	return self;
}
void DaoxCanvasNode_Delete( DaoxCanvasNode *self )
{
#warning"=================================GC for DaoxCanvasGroup field"
	if( self->children ) DList_Delete( self->children );
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
void DaoxCanvasLine_Delete( DaoxCanvasLine *self )
{
	DaoxCanvasNode_Delete( self );
}


DaoxCanvasRect* DaoxCanvasRect_New()
{
	return DaoxCanvasNode_New( daox_type_canvas_rect );
}
void DaoxCanvasRect_Delete( DaoxCanvasRect *self )
{
	DaoxCanvasNode_Delete( self );
}


DaoxCanvasCircle* DaoxCanvasCircle_New()
{
	return DaoxCanvasNode_New( daox_type_canvas_circle );
}
void DaoxCanvasCircle_Delete( DaoxCanvasCircle *self )
{
	DaoxCanvasNode_Delete( self );
}


DaoxCanvasEllipse* DaoxCanvasEllipse_New()
{
	return DaoxCanvasNode_New( daox_type_canvas_ellipse );
}
void DaoxCanvasEllipse_Delete( DaoxCanvasEllipse *self )
{
	DaoxCanvasNode_Delete( self );
}


DaoxCanvasPolygon* DaoxCanvasPolygon_New()
{
	DaoxCanvasPolygon *self = (DaoxCanvasPolygon*)dao_calloc( 1, sizeof(DaoxCanvasPolygon));
	DaoxCanvasNode_Init( self,  daox_type_canvas_polygon );
	return self;
}
void DaoxCanvasPolygon_Delete( DaoxCanvasPolygon *self )
{
	DaoxCanvasNode_Free( self );
	dao_free( self );
}


DaoxCanvasPath* DaoxCanvasPath_New()
{
	DaoxCanvasPath *self = (DaoxCanvasPath*)dao_calloc( 1, sizeof(DaoxCanvasPath));
	DaoxCanvasNode_Init( self,  daox_type_canvas_path );
	return self;
}
void DaoxCanvasPath_Delete( DaoxCanvasPath *self )
{
	DaoxCanvasNode_Free( self );
	dao_free( self );
}


DaoxCanvasImage* DaoxCanvasImage_New()
{
	DaoxCanvasImage *self = (DaoxCanvasImage*)dao_calloc( 1, sizeof(DaoxCanvasImage));
	DaoxCanvasNode_Init( self,  daox_type_canvas_image );
	return self;
}
void DaoxCanvasImage_Delete( DaoxCanvasImage *self )
{
	DaoxCanvasNode_Free( self );
	dao_free( self );
}


DaoxCanvasText* DaoxCanvasText_New()
{
	DaoxCanvasText *self = (DaoxCanvasText*) dao_calloc( 1, sizeof(DaoxCanvasText) );
	DaoxCanvasNode_Init( self, daox_type_canvas_text );
	return self;
}
void DaoxCanvasText_Delete( DaoxCanvasText *self )
{
	DaoxCanvasNode_Free( self );
	dao_free( self );
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



void DaoxCanvasPolygon_Add( DaoxCanvasPolygon *self, float x, float y )
{
	assert( self->ctype == daox_type_canvas_polygon );
	DaoxPath_LineTo( self->path, x, y );
	DaoxPath_Close( self->path );
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











DaoxCanvas* DaoxCanvas_New()
{
	DaoxCanvas *self = (DaoxCanvas*) dao_calloc( 1, sizeof(DaoxCanvas) );
	DaoxSceneNode_Init( (DaoxSceneNode*) self, daox_type_canvas );
	self->nodes = DList_New( DAO_DATA_VALUE );
	self->actives = DList_New( DAO_DATA_VALUE );
	self->brushes = DList_New( DAO_DATA_VALUE );
	self->transform = DaoxMatrix3D_Identity();

	self->auxPath = DaoxPath_New();
	DaoGC_IncRC( (DaoValue*) self->auxPath );

#warning TODO
	self->pathCache = DaoxPathCache_New();
	return self;
}
void DaoxCanvas_Delete( DaoxCanvas *self )
{
	DaoxSceneNode_Free( (DaoxSceneNode*) self );
	DList_Delete( self->nodes );
	DList_Delete( self->actives );
	DList_Delete( self->brushes );
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
	DaoxBrush *brush = DaoxCanvas_GetOrPushBrush( self );

	GC_Assign( & node->brush, brush );
	DaoxCanvas_UpdatePathMesh( self, node );

	if( self->actives->size ){
		DaoxCanvasNode *activeNode = (DaoxCanvasNode*) DList_Back( self->actives );
		if( activeNode->children == NULL ) activeNode->children = DList_New( DAO_DATA_VALUE );
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

DaoxCanvasPolygon* DaoxCanvas_AddPolygon( DaoxCanvas *self )
{
	DaoxCanvasPolygon *node = DaoxCanvasPolygon_New();
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
DaoxCanvasImage* DaoxCanvas_AddImage( DaoxCanvas *self, DaoxImage *image, float x, float y )
{
	DaoxBrush *brush = DaoxCanvas_GetOrPushBrush( self );
	DaoxCanvasPath *node = DaoxCanvasImage_New();
	DaoxTexture *texture = DaoxTexture_New();

	node->translation.x = x;
	node->translation.y = y;
	node->obbox.O.x = node->obbox.O.y = 0;
	node->obbox.X.x = node->obbox.X.y = 0;
	node->obbox.Y.x = node->obbox.Y.y = 0;
	node->obbox.X.x = image->width;
	node->obbox.Y.y = image->height;

	DaoxTexture_SetImage( texture, image );
	DaoxCanvas_AddNode( self, node );
	GC_Assign( & node->texture, texture );
	return node;
}






static void ITEM_SetVisible( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasNode *self = (DaoxCanvasNode*) p[0];
	self->visible = p[1]->xEnum.value;
	self->moved = 1;
}
static void ITEM_Scale( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasNode *self = (DaoxCanvasNode*) p[0];
	self->scale *= p[1]->xFloat.value;
	self->changed = 1;
}
static void ITEM_Rotate( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasNode *self = (DaoxCanvasNode*) p[0];
	float angle = M_PI * p[1]->xFloat.value / 180.0;
	float relative = p[2]->xInteger.value;
	float cos1 = self->rotation.x;
	float sin1 = self->rotation.y;
	float cos2 = cos( angle );
	float sin2 = sin( angle );
	if( relative ){
		self->rotation.x = cos1 * cos2 - sin1 * sin2;
		self->rotation.y = sin1 * cos2 + cos1 * sin2;
	}else{
		self->rotation.x = cos2;
		self->rotation.y = sin2;
	}
	self->moved = 1;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void ITEM_Move( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasNode *self = (DaoxCanvasNode*) p[0];
	float x = p[1]->xFloat.value;
	float y = p[2]->xFloat.value;
	float relative = p[3]->xInteger.value;
	if( relative ){
		self->translation.x += x;
		self->translation.y += y;
	}else{
		self->translation.x = x;
		self->translation.y = y;
	}
	self->moved = 1;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}

static void DaoxCanvasNode_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	daoint i, n;
	DaoxCanvasNode *self = (DaoxCanvasNode*) p;
	if( self->children == NULL ) return;
	DList_Append( lists, self->children );
}

static DaoFuncItem DaoxCanvasNodeMeths[]=
{
	{ ITEM_SetVisible,
		"SetVisible( self: CanvasNode, visible: enum<false,true> )"
	},
	{ ITEM_Scale,
		"Scale( self: CanvasNode, ratio: float ) => CanvasNode"
	},
	{ ITEM_Rotate,
		"Rotate( self: CanvasNode, degree: float, relative = 0 ) => CanvasNode"
	},
	{ ITEM_Move,
		"Move( self: CanvasNode, x: float, y: float, relative = 0 ) => CanvasNode"
	},
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasNode_Typer =
{
	"CanvasNode", NULL, NULL, (DaoFuncItem*) DaoxCanvasNodeMeths, {NULL}, {NULL},
	(FuncPtrDel)DaoxCanvasNode_Delete, DaoxCanvasNode_GetGCFields
};









static void LINE_SetData( DaoxCanvasLine *self, DaoValue *p[] )
{
	float x1 = p[0]->xFloat.value;
	float y1 = p[1]->xFloat.value;
	float x2 = p[2]->xFloat.value;
	float y2 = p[3]->xFloat.value;
	DaoxCanvasLine_Set( self, x1, y1, x2, y2 );
}
static void LINE_Set( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasLine *self = (DaoxCanvasLine*) p[0];
	LINE_SetData( self, p + 1 );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static DaoFuncItem DaoxCanvasLineMeths[]=
{
	{ LINE_Set, "Set( self: CanvasLine, x1 = 0.0, y1 = 0.0, x2 = 1.0, y2 = 1.0 ) => CanvasLine" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasLine_Typer =
{
	"CanvasLine", NULL, NULL, (DaoFuncItem*) DaoxCanvasLineMeths,
	{ & DaoxCanvasNode_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasNode_Delete, DaoxCanvasNode_GetGCFields
};





#if 0
static void RECT_SetData( DaoxCanvasRect *self, DaoValue *p[] )
{
	float x1 = p[0]->xFloat.value;
	float y1 = p[1]->xFloat.value;
	float x2 = p[2]->xFloat.value;
	float y2 = p[3]->xFloat.value;
	float rx = p[4]->xFloat.value;
	float ry = p[5]->xFloat.value;
	DaoxCanvasRect_Set( self, x1, y1, x2, y2, rx, ry );
}
static void RECT_Set( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasRect *self = (DaoxCanvasRect*) p[0];
	RECT_SetData( self, p + 1 );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
#endif
static DaoFuncItem DaoxCanvasRectMeths[]=
{
	//{ RECT_Set,   "Set( self: CanvasRect, x1 = 0.0, y1 = 0.0, x2 = 1.0, y2 = 1.0 ) => CanvasRect" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasRect_Typer =
{
	"CanvasRect", NULL, NULL, (DaoFuncItem*) DaoxCanvasRectMeths,
	{ & DaoxCanvasNode_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasNode_Delete, DaoxCanvasNode_GetGCFields
};






static void CIRCLE_SetData( DaoxCanvasCircle *self, DaoValue *p[] )
{
	float x = p[0]->xFloat.value;
	float y = p[1]->xFloat.value;
	float r = p[2]->xFloat.value;
	DaoxCanvasCircle_Set( self, x, y, r );
}
static void CIRCLE_Set( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasCircle *self = (DaoxCanvasCircle*) p[0];
	CIRCLE_SetData( self, p + 1 );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static DaoFuncItem DaoxCanvasCircleMeths[]=
{
	{ CIRCLE_Set,  "Set( self: CanvasCircle, cx = 0.0, cy = 0.0, r = 1.0 ) => CanvasCircle" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasCircle_Typer =
{
	"CanvasCircle", NULL, NULL, (DaoFuncItem*) DaoxCanvasCircleMeths,
	{ & DaoxCanvasNode_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasNode_Delete, DaoxCanvasNode_GetGCFields
};





static void ELLIPSE_SetData( DaoxCanvasEllipse *self, DaoValue *p[] )
{
	float x1 = p[0]->xFloat.value;
	float y1 = p[1]->xFloat.value;
	float x2 = p[2]->xFloat.value;
	float y2 = p[3]->xFloat.value;
	DaoxCanvasEllipse_Set( self, x1, y1, x2, y2 );
}
static void ELLIPSE_Set( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasEllipse *self = (DaoxCanvasEllipse*) p[0];
	ELLIPSE_SetData( self, p + 1 );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static DaoFuncItem DaoxCanvasEllipseMeths[]=
{
	{ ELLIPSE_Set,  "Set( self: CanvasEllipse, cx = 0.0, cy = 0.0, rx = 1.0, ry = 1.0 ) => CanvasEllipse" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasEllipse_Typer =
{
	"CanvasEllipse", NULL, NULL, (DaoFuncItem*) DaoxCanvasEllipseMeths,
	{ & DaoxCanvasNode_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasNode_Delete, DaoxCanvasNode_GetGCFields
};





static void POLYGON_Add( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasPolygon *self = (DaoxCanvasPolygon*) p[0];
	float x = p[1]->xFloat.value;
	float y = p[2]->xFloat.value;
	DaoxCanvasPolygon_Add( self, x, y );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static DaoFuncItem DaoxCanvasPolygonMeths[]=
{
	{ POLYGON_Add,   "Add( self: CanvasPolygon, x: float, y: float ) => CanvasPolygon" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasPolygon_Typer =
{
	"CanvasPolygon", NULL, NULL, (DaoFuncItem*) DaoxCanvasPolygonMeths,
	{ & DaoxCanvasNode_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasNode_Delete, DaoxCanvasNode_GetGCFields
};





static void PATH_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *path = DaoxPath_New();
	DaoProcess_PutValue( proc, (DaoValue*) path );
}
static void PATH_MoveTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	float x = p[1]->xFloat.value;
	float y = p[2]->xFloat.value;
	DaoxPath_MoveTo( self, x, y );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void PATH_LineTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	float x = p[1]->xFloat.value;
	float y = p[2]->xFloat.value;
	DaoxPath_LineTo( self, x, y );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void PATH_ArcTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	float x = p[1]->xFloat.value;
	float y = p[2]->xFloat.value;
	float d = p[3]->xFloat.value;
	DaoxPath_ArcTo( self, x, y, d );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void PATH_ArcBy( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	float x = p[1]->xFloat.value;
	float y = p[2]->xFloat.value;
	float d = p[3]->xFloat.value;
	DaoxPath_ArcBy( self, x, y, d );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void PATH_QuadTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	float cx = p[1]->xFloat.value;
	float cy = p[2]->xFloat.value;
	float x = p[3]->xFloat.value;
	float y = p[4]->xFloat.value;
	DaoxPath_QuadTo( self, cx, cy, x, y );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void PATH_CubicTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	float cx = p[1]->xFloat.value;
	float cy = p[2]->xFloat.value;
	float x = p[3]->xFloat.value;
	float y = p[4]->xFloat.value;
	DaoxPath_CubicTo( self, cx, cy, x, y );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void PATH_CubicTo2( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	float cx0 = p[1]->xFloat.value;
	float cy0 = p[2]->xFloat.value;
	float cx = p[3]->xFloat.value;
	float cy = p[4]->xFloat.value;
	float x = p[5]->xFloat.value;
	float y = p[6]->xFloat.value;
	DaoxPath_CubicTo2( self, cx0, cy0, cx, cy, x, y );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void PATH_Close( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_Close( self );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}

static void PATH_LineRelTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 1 );
	PATH_LineTo( proc, p, N );
}
static void PATH_ArcRelTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 1 );
	PATH_ArcTo( proc, p, N );
}
static void PATH_ArcRelBy( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 1 );
	PATH_ArcBy( proc, p, N );
}
static void PATH_QuadRelTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 1 );
	PATH_QuadTo( proc, p, N );
}
static void PATH_CubicRelTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 1 );
	PATH_CubicTo( proc, p, N );
}
static void PATH_CubicRelTo2( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 1 );
	PATH_CubicTo2( proc, p, N );
}
static void PATH_LineAbsTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 0 );
	PATH_LineTo( proc, p, N );
}
static void PATH_ArcAbsTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 0 );
	PATH_ArcTo( proc, p, N );
}
static void PATH_ArcAbsBy( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 0 );
	PATH_ArcBy( proc, p, N );
}
static void PATH_QuadAbsTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 0 );
	PATH_QuadTo( proc, p, N );
}
static void PATH_CubicAbsTo( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 0 );
	PATH_CubicTo( proc, p, N );
}
static void PATH_CubicAbsTo2( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPath *self = (DaoxPath*) p[0];
	DaoxPath_SetRelativeMode( self, 0 );
	PATH_CubicTo2( proc, p, N );
}
static DaoFuncItem DaoxCanvasPathMeths[]=
{
#if 0
#endif
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasPath_Typer =
{
	"CanvasPath", NULL, NULL, (DaoFuncItem*) DaoxCanvasPathMeths,
	{ & DaoxCanvasNode_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasNode_Delete, DaoxCanvasNode_GetGCFields
};




static DaoFuncItem DaoxCanvasTextMeths[]=
{
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasText_Typer =
{
	"CanvasText", NULL, NULL, (DaoFuncItem*) DaoxCanvasTextMeths,
	{ & DaoxCanvasNode_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasNode_Delete, DaoxCanvasNode_GetGCFields
};



static DaoFuncItem DaoxCanvasImageMeths[]=
{
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasImage_Typer =
{
	"CanvasImage", NULL, NULL, (DaoFuncItem*) DaoxCanvasImageMeths,
	{ & DaoxCanvasNode_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasNode_Delete, DaoxCanvasNode_GetGCFields
};





static void CANVAS_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = DaoxCanvas_New();
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void CANVAS_SetViewport( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	self->viewport.left = p[1]->xFloat.value;
	self->viewport.right = p[2]->xFloat.value;
	self->viewport.bottom = p[3]->xFloat.value;
	self->viewport.top = p[4]->xFloat.value;
}
static void CANVAS_GetViewport( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoProcess_NewFloat( proc, self->viewport.left );
	DaoProcess_NewFloat( proc, self->viewport.right );
	DaoProcess_NewFloat( proc, self->viewport.bottom );
	DaoProcess_NewFloat( proc, self->viewport.top );
	DaoProcess_PutTuple( proc, -4 );
}
static void CANVAS_SetBackground( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxColor color;
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	color.red   = p[1]->xFloat.value;
	color.green = p[2]->xFloat.value;
	color.blue  = p[3]->xFloat.value;
	color.alpha = p[4]->xFloat.value;
	DaoxCanvas_SetBackground( self, color );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void CANVAS_AddGroup( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxCanvasLine *item = DaoxCanvas_AddGroup( self );
	DaoProcess_PutValue( proc, (DaoValue*) item );
}
static void CANVAS_AddLine( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	float x1 = p[1]->xFloat.value, y1 = p[2]->xFloat.value;
	float x2 = p[3]->xFloat.value, y2 = p[4]->xFloat.value;
	DaoxCanvasLine *item = DaoxCanvas_AddLine( self, x1, y1, x2, y2 );
	DaoProcess_PutValue( proc, (DaoValue*) item );
}
static void CANVAS_AddRect( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	float x1 = p[1]->xFloat.value, y1 = p[2]->xFloat.value;
	float x2 = p[3]->xFloat.value, y2 = p[4]->xFloat.value;
	float rx = p[5]->xFloat.value, ry = p[6]->xFloat.value;
	DaoxCanvasRect *item = DaoxCanvas_AddRect( self, x1, y1, x2, y2, rx, ry );
	DaoProcess_PutValue( proc, (DaoValue*) item );
}
static void CANVAS_AddCircle( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	float x = p[1]->xFloat.value, y = p[2]->xFloat.value;
	float r = p[3]->xFloat.value;
	DaoxCanvasCircle *item = DaoxCanvas_AddCircle( self, x, y, r );
	DaoProcess_PutValue( proc, (DaoValue*) item );
}
static void CANVAS_AddEllipse( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	float x = p[1]->xFloat.value, y = p[2]->xFloat.value;
	float rx = p[3]->xFloat.value, ry = p[4]->xFloat.value;
	DaoxCanvasEllipse *item = DaoxCanvas_AddEllipse( self, x, y, rx, ry );
	DaoProcess_PutValue( proc, (DaoValue*) item );
}
static void CANVAS_AddPolygon( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxCanvasPolygon *item = DaoxCanvas_AddPolygon( self );
	DaoProcess_PutValue( proc, (DaoValue*) item );
}
static void CANVAS_AddText( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DString *text = DaoValue_TryGetString( p[1] );
	float x = p[2]->xFloat.value;
	float y = p[3]->xFloat.value;
	float a = p[4]->xFloat.value;
	DaoxCanvasText *item = DaoxCanvas_AddText( self, DString_GetData( text ), x, y, a );
	if( item == NULL ){
		DaoProcess_RaiseError( proc, NULL, "no font is set" );
		return;
	}
	DaoProcess_PutValue( proc, (DaoValue*) item );
}
static void CANVAS_AddText2( DaoProcess *proc, DaoValue *p[], int N )
{
	float a = p[3]->xFloat.value;
	char *text = DaoValue_TryGetChars( p[1] );
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxPath *path = (DaoxPath*) p[2];
	DaoxCanvasText *item = DaoxCanvas_AddPathText( self, text, path, a );
	if( item == NULL ){
		DaoProcess_RaiseError( proc, NULL, "no font is set" );
		return;
	}
	DaoProcess_PutValue( proc, (DaoValue*) item );
}
static void CANVAS_AddImage( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxImage *image = (DaoxImage*) p[1];
	float x = p[2]->xFloat.value;
	float y = p[3]->xFloat.value;
	DaoxCanvasImage *item = DaoxCanvas_AddImage( self, image, x, y );
	DaoProcess_PutValue( proc, (DaoValue*) item );
}


static void CANVAS_AddPath( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxPath *path = (DaoxPath*) p[1];
	DaoxCanvasPath *item = DaoxCanvas_AddPath( self, path );
	DaoProcess_PutValue( proc, (DaoValue*) item );
}
static void CANVAS_PushBrush( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxBrush *brush;
	int index = N > 1 ? p[1]->xInteger.value : -1;
	brush = DaoxCanvas_PushBrush( self, index );
	DaoProcess_PutValue( proc, (DaoValue*) brush );
}
static void CANVAS_PopBrush( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxCanvas_PopBrush( self );
}



static void BRUSH_SetStrokeWidth( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxBrush *self = (DaoxBrush*) p[0];
	self->strokeStyle.width = p[1]->xFloat.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void BRUSH_SetStrokeColor( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxBrush *self = (DaoxBrush*) p[0];
	self->strokeColor.red   = p[1]->xFloat.value;
	self->strokeColor.green = p[2]->xFloat.value;
	self->strokeColor.blue  = p[3]->xFloat.value;
	self->strokeColor.alpha = p[4]->xFloat.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void BRUSH_SetFillColor( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxBrush *self = (DaoxBrush*) p[0];
	self->fillColor.red   = p[1]->xFloat.value;
	self->fillColor.green = p[2]->xFloat.value;
	self->fillColor.blue  = p[3]->xFloat.value;
	self->fillColor.alpha = p[4]->xFloat.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void BRUSH_SetLineCap( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxBrush *self = (DaoxBrush*) p[0];
	self->strokeStyle.cap = p[1]->xEnum.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void BRUSH_SetJunction( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxBrush *self = (DaoxBrush*) p[0];
	self->strokeStyle.junction = p[1]->xEnum.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void BRUSH_SetDash( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxBrush *self = (DaoxBrush*) p[0];
	DaoArray *array = (DaoArray*) p[1];
	DaoxBrush_SetDashPattern( self, DaoArray_ToFloat32( array ), array->size );
	DaoArray_FromFloat32( array );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void BRUSH_SetStrokeGradient( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxBrush *self = (DaoxBrush*) p[0];
	if( self->strokeGradient == NULL ){
		self->strokeGradient = DaoxGradient_New( DAOX_GRADIENT_BASE );
		DaoGC_IncRC( (DaoValue*) self->strokeGradient );
	}
	DaoProcess_PutValue( proc, (DaoValue*) self->strokeGradient );
}
static void BRUSH_SetFillGradient( DaoProcess *proc, DaoValue *p[], int N, int type )
{
	DaoxBrush *self = (DaoxBrush*) p[0];
	DaoxGradient *grad = DaoxGradient_New( type );
	GC_Assign( & self->fillGradient, grad );
	DaoProcess_PutValue( proc, (DaoValue*) self->fillGradient );
}
static void BRUSH_SetLinearGradient( DaoProcess *proc, DaoValue *p[], int N )
{
	BRUSH_SetFillGradient( proc, p, N, DAOX_GRADIENT_LINEAR );
}
static void BRUSH_SetRadialGradient( DaoProcess *proc, DaoValue *p[], int N )
{
	BRUSH_SetFillGradient( proc, p, N, DAOX_GRADIENT_RADIAL );
}
static void BRUSH_SetPathGradient( DaoProcess *proc, DaoValue *p[], int N )
{
	BRUSH_SetFillGradient( proc, p, N, DAOX_GRADIENT_PATH );
}
static void BRUSH_SetFont( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxBrush *self = (DaoxBrush*) p[0];
	DaoxFont *font = (DaoxFont*) p[1];
	DaoxBrush_SetFont( self, font, p[2]->xFloat.value );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void CANVAS_Test( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	int i, n = self->nodes->size;
	for(i=0; i<n; i++){
		DaoxCanvasPath *item = self->nodes->items.pCanvasNode[i];
		//DaoxCanvasNode_UpdateData( item, self );
	}
}

static void DaoxCanvas_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	daoint i, n;
	DaoxCanvas *self = (DaoxCanvas*) p;
	if( self->nodes == NULL ) return;
	DList_Append( lists, self->nodes );
}

static DaoFuncItem DaoxCanvasMeths[]=
{
	{ CANVAS_New,         "Canvas()" },

	{ CANVAS_SetViewport,   "SetViewport( self: Canvas, left: float, right: float, bottom: float, top: float )" },

	{ CANVAS_GetViewport,   "GetViewport( self: Canvas ) => tuple<left:float,right:float,bottom:float,top:float>" },

	{ CANVAS_SetBackground,  "SetBackground( self: Canvas, red: float, green: float, blue: float, alpha = 1.0 ) => Canvas" },

	{ CANVAS_PushBrush,   "PushBrush( self: Canvas ) => Brush" },
	{ CANVAS_PushBrush,   "PushBrush( self: Canvas, index: int ) => invar<Brush>" },

	{ CANVAS_PopBrush,    "PopBrush( self: Canvas )" },

	{ CANVAS_AddGroup,   "AddGroup( self: Canvas ) => CanvasNode" },
	{ CANVAS_AddCircle,    "AddCircle( self: Canvas, x: float, y: float, r: float ) => CanvasCircle" },
	{ CANVAS_AddLine,   "AddLine( self: Canvas, x1: float, y1: float, x2: float, y2: float ) => CanvasLine" },

	{ CANVAS_AddRect,   "AddRect( self: Canvas, x1: float, y1: float, x2: float, y2: float, rx = 0.0, ry = 0.0 ) => CanvasRect" },


	{ CANVAS_AddEllipse,   "AddEllipse( self: Canvas, x: float, y: float, rx: float, ry: float ) => CanvasEllipse" },

	{ CANVAS_AddPolygon,   "AddPolygon( self: Canvas ) => CanvasPolygon" },



	{ CANVAS_AddImage,     "AddImage( self: Canvas, image: Image, x: float, y: float ) => CanvasImage" },

#if 0
	{ CANVAS_Test,         "Test( self: Canvas )" },
#endif
	{ CANVAS_AddPath,      "AddPath( self: Canvas, path: Path ) => CanvasPath" },
	{ CANVAS_AddText,      "AddText( self: Canvas, text: string, x: float, y: float, degrees = 0.0 ) => CanvasText" },

	{ CANVAS_AddText2,     "AddText( self: Canvas, text: string, path: Path, degrees = 0.0 ) => CanvasText" },
	{ NULL, NULL }
};

extern DaoTypeBase DaoxSceneNode_Typer;
DaoTypeBase DaoxCanvas_Typer =
{
	"Canvas", NULL, NULL, (DaoFuncItem*) DaoxCanvasMeths,
	{ & DaoxSceneNode_Typer, NULL }, {NULL},
	(FuncPtrDel)DaoxCanvas_Delete, DaoxCanvas_GetGCFields
};


static DaoFuncItem DaoxBrushMeths[]=
{
	{ BRUSH_SetStrokeWidth,  "SetStrokeWidth( self: Brush, width = 1.0 ) => Brush" },

	{ BRUSH_SetStrokeColor,  "SetStrokeColor( self: Brush, red: float, green: float, blue: float, alpha = 1.0 ) => Brush" },

	{ BRUSH_SetFillColor,  "SetFillColor( self: Brush, red: float, green: float, blue: float, alpha = 1.0 ) => Brush" },

	{ BRUSH_SetLineCap, "SetLineCap( self: Brush, cap: enum<none,flat,sharp,round> = $none ) => Brush" },
	{ BRUSH_SetJunction, "SetJunction( self: Brush, junction: enum<none,flat,sharp,round> = $sharp ) => Brush" },

	{ BRUSH_SetDash, "SetDashPattern( self: Brush, pattern = [3.0,2.0] ) => Brush" },

	{ BRUSH_SetStrokeGradient, "SetStrokeGradient( self: Brush ) => ColorGradient" },

	{ BRUSH_SetLinearGradient, "SetLinearGradient( self: Brush ) => LinearGradient" },

	{ BRUSH_SetRadialGradient, "SetRadialGradient( self: Brush ) => RadialGradient" },

	//{ BRUSH_SetPathGradient,   "SetPathGradient( self: Brush ) => PathGradient" },

	{ BRUSH_SetFont,      "SetFont( self: Brush, font: Font, size = 12.0 ) => Brush" },
	{ NULL, NULL }
};


DaoTypeBase DaoxBrush_Typer =
{
	"Brush", NULL, NULL, (DaoFuncItem*) DaoxBrushMeths, {NULL}, {NULL},
	(FuncPtrDel)DaoxBrush_Delete, NULL
};



static DaoFuncItem DaoxPathMeths[]=
{
	{ PATH_New,    "Path() => Path" },

	{ PATH_MoveTo,    "MoveTo( self: Path, x: float, y: float ) => Path" },

	{ PATH_LineRelTo,    "LineTo( self: Path, x: float, y: float ) => Path" },

	{ PATH_ArcRelTo,     "ArcTo( self: Path, x: float, y: float, degrees: float ) => Path" },

	{ PATH_ArcRelBy,     "ArcBy( self: Path, cx: float, cy: float, degrees: float ) => Path" },

	{ PATH_QuadRelTo,   "QuadTo( self: Path, cx: float, cy: float, x: float, y: float ) => Path" },

	{ PATH_CubicRelTo,   "CubicTo( self: Path, cx: float, cy: float, x: float, y: float ) => Path" },

	{ PATH_CubicRelTo2,  "CubicTo( self: Path, cx0: float, cy0: float, cx: float, cy: float, x: float, y: float ) => Path" },

	{ PATH_LineAbsTo,    "LineAbsTo( self: Path, x: float, y: float ) => Path" },

	{ PATH_ArcAbsTo,     "ArcAbsTo( self: Path, x: float, y: float, degrees: float ) => Path" },

	{ PATH_ArcAbsBy,     "ArcAbsBy( self: Path, cx: float, cy: float, degrees: float ) => Path" },

	{ PATH_QuadAbsTo,   "QuadAbsTo( self: Path, cx: float, cy: float, x: float, y: float ) => Path" },

	{ PATH_CubicAbsTo,   "CubicAbsTo( self: Path, cx: float, cy: float, x: float, y: float ) => Path" },

	{ PATH_CubicAbsTo2,  "CubicAbsTo( self: Path, cx0: float, cy0: float, cx: float, cy: float, x: float, y: float ) => Path" },

	{ PATH_Close,     "Close( self: Path ) => Path" },
	{ NULL, NULL }
};
DaoTypeBase DaoxPath_Typer =
{
	"Path", NULL, NULL, (DaoFuncItem*) DaoxPathMeths, { NULL }, { NULL },
	(FuncPtrDel)DaoxPath_Delete, NULL
};

DaoType* daox_type_path = NULL;


DAO_DLL int DaoTriangulator_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );
DAO_DLL int DaoFont_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );
DAO_DLL int DaoImage_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );
DAO_DLL int DaoGLUT_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );

DAO_DLL int DaoVectorGraphics_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns )
{
	DaoFont_OnLoad( vmSpace, ns );
	DaoImage_OnLoad( vmSpace, ns );

	DaoNamespace_DefineType( ns, "tuple<red:float,green:float,blue:float,alpha:float>", "Color" );

	daox_type_gradient = DaoNamespace_WrapType( ns, & DaoxGradient_Typer, 0 );
	daox_type_linear_gradient = DaoNamespace_WrapType( ns, & DaoxLinearGradient_Typer, 0 );
	daox_type_radial_gradient = DaoNamespace_WrapType( ns, & DaoxRadialGradient_Typer, 0 );
	daox_type_path_gradient = DaoNamespace_WrapType( ns, & DaoxPathGradient_Typer, 0 );
	daox_type_path = DaoNamespace_WrapType( ns, & DaoxPath_Typer, 0 );

	daox_type_brush = DaoNamespace_WrapType( ns, & DaoxBrush_Typer, 0 );
	daox_type_canvas = DaoNamespace_WrapType( ns, & DaoxCanvas_Typer, 0 );
	daox_type_canvas_node = DaoNamespace_WrapType( ns, & DaoxCanvasNode_Typer, 0 );
	daox_type_canvas_line = DaoNamespace_WrapType( ns, & DaoxCanvasLine_Typer, 0 );
	daox_type_canvas_rect = DaoNamespace_WrapType( ns, & DaoxCanvasRect_Typer, 0 );
	daox_type_canvas_circle = DaoNamespace_WrapType( ns, & DaoxCanvasCircle_Typer, 0 );
	daox_type_canvas_ellipse = DaoNamespace_WrapType( ns, & DaoxCanvasEllipse_Typer, 0 );
	daox_type_canvas_polygon = DaoNamespace_WrapType( ns, & DaoxCanvasPolygon_Typer, 0 );
	daox_type_canvas_path = DaoNamespace_WrapType( ns, & DaoxCanvasPath_Typer, 0 );
	daox_type_canvas_text = DaoNamespace_WrapType( ns, & DaoxCanvasText_Typer, 0 );
	daox_type_canvas_image = DaoNamespace_WrapType( ns, & DaoxCanvasImage_Typer, 0 );

	DaoTriangulator_OnLoad( vmSpace, ns );
	return 0;
}
