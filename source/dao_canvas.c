/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2012,2013, Limin Fu
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


#define UNIT 128.0


DaoType *daox_type_color_gradient = NULL;
DaoType *daox_type_linear_gradient = NULL;
DaoType *daox_type_radial_gradient = NULL;
DaoType *daox_type_path_gradient = NULL;
DaoType *daox_type_canvas_state = NULL;
DaoType *daox_type_canvas = NULL;

DaoType *daox_type_canvas_item = NULL;
DaoType *daox_type_canvas_line = NULL;
DaoType *daox_type_canvas_rect = NULL;
DaoType *daox_type_canvas_circle = NULL;
DaoType *daox_type_canvas_ellipse = NULL;
DaoType *daox_type_canvas_polyline = NULL;
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







DaoxColorGradient* DaoxColorGradient_New( int type )
{
	DaoxColorGradient *self = (DaoxColorGradient*) dao_calloc(1,sizeof(DaoxColorGradient));
	DaoType *ctype = daox_type_color_gradient;
	switch( type ){
	case DAOX_GRADIENT_BASE : ctype = daox_type_color_gradient; break;
	case DAOX_GRADIENT_LINEAR : ctype = daox_type_linear_gradient; break;
	case DAOX_GRADIENT_RADIAL : ctype = daox_type_radial_gradient; break;
	case DAOX_GRADIENT_PATH : ctype = daox_type_path_gradient; break;
	}
	DaoCstruct_Init( (DaoCstruct*)self, ctype );
	self->stops = DaoxPlainArray_New( sizeof(float) );
	self->colors = DaoxPlainArray_New( sizeof(DaoxColor) );
	self->gradient = type;
	return self;
}
void DaoxColorGradient_Delete( DaoxColorGradient *self )
{
	DaoxPlainArray_Delete( self->stops );
	DaoxPlainArray_Delete( self->colors );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}
void DaoxColorGradient_Add( DaoxColorGradient *self, float stop, DaoxColor color )
{
	DaoxColor *C = (DaoxColor*) DaoxPlainArray_Push( self->colors );
	DaoxPlainArray_PushFloat( self->stops, stop );
	*C = color;
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
	DaoxColorGradient *self = (DaoxColorGradient*) p[0];
	DaoxColor color = {0.0,0.0,0.0,0.0};
	DaoxColor_FromDaoValues( & color, p[2]->xTuple.items );
	DaoxColorGradient_Add( self, p[1]->xFloat.value, color );
}
static void GRAD_AddStops( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxColorGradient *self = (DaoxColorGradient*) p[0];
	DaoList *stops = (DaoList*) p[1];
	int i, n = DaoList_Size( stops );
	for(i=0; i<n; ++i){
		DaoTuple *item = (DaoTuple*) DaoList_GetItem( stops, i );
		DaoxColor color = {0.0,0.0,0.0,0.0};
		DaoxColor_FromDaoValues( & color, item->items[1]->xTuple.items );
		DaoxColorGradient_Add( self, item->items[0]->xFloat.value, color );
	}
}

static DaoFuncItem DaoxColorGradientMeths[]=
{
	{ GRAD_AddStop,  "AddStop( self : ColorGradient, at: float, color: Color ) => ColorGradient" },
	{ GRAD_AddStops,  "AddStops( self : ColorGradient, stops: list<tuple<at:float,color:Color>> ) => ColorGradient" },
	{ NULL, NULL }
};

DaoTypeBase DaoxColorGradient_Typer =
{
	"ColorGradient", NULL, NULL, (DaoFuncItem*) DaoxColorGradientMeths, {0}, {0},
	(FuncPtrDel)DaoxColorGradient_Delete, NULL
};


static void LGRAD_SetStart( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxColorGradient *self = (DaoxColorGradient*) p[0];
	self->points[0].x = p[1]->xFloat.value;
	self->points[0].y = p[2]->xFloat.value;
}
static void LGRAD_SetEnd( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxColorGradient *self = (DaoxColorGradient*) p[0];
	self->points[1].x = p[1]->xFloat.value;
	self->points[1].y = p[2]->xFloat.value;
}

static DaoFuncItem DaoxLinearGradientMeths[]=
{
	{ LGRAD_SetStart,  "SetStart( self : LinearGradient, x : float, y : float )" },
	{ LGRAD_SetEnd,    "SetEnd( self : LinearGradient, x : float, y : float )" },
	{ NULL, NULL }
};

DaoTypeBase DaoxLinearGradient_Typer =
{
	"LinearGradient", NULL, NULL, (DaoFuncItem*) DaoxLinearGradientMeths,
	{ & DaoxColorGradient_Typer, 0}, {0},
	(FuncPtrDel)DaoxColorGradient_Delete, NULL
};


static void RGRAD_SetRadius( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxColorGradient *self = (DaoxColorGradient*) p[0];
	self->radius = p[1]->xFloat.value;
}

static DaoFuncItem DaoxRadialGradientMeths[]=
{
	{ LGRAD_SetStart,  "SetCenter( self : RadialGradient, x : float, y : float )" },
	{ LGRAD_SetEnd,    "SetFocus( self : RadialGradient, x : float, y : float )" },
	{ RGRAD_SetRadius, "SetRadius( self : RadialGradient, r : float )" },
	{ NULL, NULL }
};

DaoTypeBase DaoxRadialGradient_Typer =
{
	"RadialGradient", NULL, NULL, (DaoFuncItem*) DaoxRadialGradientMeths,
	{ & DaoxColorGradient_Typer, 0}, {0},
	(FuncPtrDel)DaoxColorGradient_Delete, NULL
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
	{ & DaoxColorGradient_Typer, 0}, {0},
	(FuncPtrDel)DaoxColorGradient_Delete, NULL
};









DaoxCanvasState* DaoxCanvasState_New()
{
	DaoxCanvasState *self = (DaoxCanvasState*) dao_calloc(1,sizeof(DaoxCanvasState));
	DaoCstruct_Init( (DaoCstruct*)self, daox_type_canvas_state );
	self->linecap = DAOX_LINECAP_NONE;
	self->junction = DAOX_JUNCTION_FLAT;
	self->strokeWidth = 1.0;
	self->strokeColor.alpha = 1.0;
	self->fontSize = 12.0;
	return self;
}
void DaoxCanvasState_Delete( DaoxCanvasState *self )
{
	if( self->strokeGradient ) DaoxColorGradient_Delete( self->strokeGradient );
	if( self->font ) DaoGC_DecRC( (DaoValue*) self->font );
	if( self->parent ) DaoGC_DecRC( (DaoValue*) self->parent );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}
void DaoxCanvasState_Copy( DaoxCanvasState *self, DaoxCanvasState *other )
{
	DaoGC_ShiftRC( (DaoValue*) other->font, (DaoValue*) self->font );
	DaoGC_ShiftRC( (DaoValue*) other->parent, (DaoValue*) self->parent );
	self->font = other->font;
	self->parent = other->parent;
	self->dash = other->dash;
	self->junction = other->junction;
	self->fontSize = other->fontSize;
	self->strokeWidth = other->strokeWidth;
	self->strokeColor = other->strokeColor;
	self->fillColor = other->fillColor;
	memcpy( self->dashPattern, other->dashPattern, other->dash*sizeof(float) );
}
void DaoxCanvasState_SetDashPattern( DaoxCanvasState *self, float pat[], int n )
{
	if( n > 10 ) n = 10;
	self->dash = n;
	memcpy( self->dashPattern, pat, n*sizeof(float) );
}
void DaoxCanvasState_SetFont( DaoxCanvasState *self, DaoxFont *font, float size )
{
	DaoGC_ShiftRC( (DaoValue*) font, (DaoValue*) self->font );
	self->font = font;
	self->fontSize = size;
}
void DaoxCanvasState_SetParentItem( DaoxCanvasState *self, DaoxCanvasItem *item )
{
	DaoGC_ShiftRC( (DaoValue*) item, (DaoValue*) self->parent );
	self->parent = item;
}




void DaoxCanvasItem_Init( DaoxCanvasItem *self, DaoType *type )
{
	memset( self, 0, sizeof(DaoxCanvasItem) );
	DaoCstruct_Init( (DaoCstruct*)self, type );
	self->scale = 1.0;
	self->visible = 1;
	self->dataChanged = 1;
	self->stateChanged = 1;
	self->transform = DaoxMatrix3D_Identity();
}
void DaoxCanvasItem_Free( DaoxCanvasItem *self )
{
	if( self->strokes ){
		if( self->path == NULL || self->path->strokes == NULL )
			DaoxPathMesh_Delete( self->strokes );
	}
	if( self->path ) DaoGC_DecRC( (DaoValue*) self->path );
	DaoCstruct_Free( (DaoCstruct*) self );
	DaoGC_DecRC( (DaoValue*) self->parent );
	DaoGC_DecRC( (DaoValue*) self->state );
}



DaoxCanvasItem* DaoxCanvasItem_New( DaoType *type )
{
	DaoxCanvasItem *self = (DaoxCanvasItem*) dao_calloc( 1, sizeof(DaoxCanvasItem) );
	DaoxCanvasItem_Init( self, type );
	return self;
}
void DaoxCanvasItem_Delete( DaoxCanvasItem *self )
{
#warning"=================================GC for DaoxCanvasGroup field"
	if( self->children ) DArray_Delete( self->children );
	DaoxCanvasItem_Free( self );
	dao_free( self );
}
void DaoxCanvasItem_MarkDataChanged( DaoxCanvasItem *self )
{
	self->dataChanged = 1;
	self->stateChanged = 1;
	if( self->parent ) DaoxCanvasItem_MarkStateChanged( self->parent );
}
void DaoxCanvasItem_MarkStateChanged( DaoxCanvasItem *self )
{
	self->stateChanged = 1;
	if( self->parent ) DaoxCanvasItem_MarkStateChanged( self->parent );
}
static void DaoxPlainArray_PushOBBoxVertexPoints2D( DaoxPlainArray *self, DaoxOBBox2D *box )
{
	DaoxVector2D *point1, *point2, *point3, *point4;
	DaoxVector2D dY = DaoxVector2D_Sub( & box->Y, & box->O );

	DaoxPlainArray_Reserve( self, self->size + 4 );
	point1 = DaoxPlainArray_PushVector2D( self );
	point2 = DaoxPlainArray_PushVector2D( self );
	point3 = DaoxPlainArray_PushVector2D( self );
	point4 = DaoxPlainArray_PushVector2D( self );
	*point1 = box->O;
	*point2 = box->X;
	*point3 = box->Y;
	*point4 = DaoxVector2D_Add( & box->X, & dY );
}
void DaoxCanvasItem_Update( DaoxCanvasItem *self, DaoxCanvas *canvas )
{
	DaoxPlainArray *points;
	daoint i;

	if( self->stateChanged == 0 ) return;

	if( self->dataChanged && self->state ){
		DaoxPath *path = NULL;
		DaoType *ctype = self->ctype;
		DaoxCanvasState *state = self->state;
		DaoxColorGradient *strokeGradient = state->strokeGradient;
		float strokeWidth = state->strokeWidth / (self->scale + EPSILON);
		float strokeAlpha = state->strokeColor.alpha;
		int refine = state->dash || strokeGradient != NULL;
		int junction = state->junction;
		int cap = state->linecap;

		if( ctype == daox_type_canvas_polyline || ctype == daox_type_canvas_polygon ){
			if( self->path == NULL ){
				self->path = path = DaoxPath_New();
				DaoGC_IncRC( (DaoValue*) self->path );
			}
			DaoxPath_Reset( self->path );
			if( self->ctype == daox_type_canvas_polyline ){
				for(i=0; i<self->data.points->size; i+=2){
					DaoxVector2D *points = self->data.points->pod.vectors2d + i;
					DaoxPath_MoveTo( self->path, points[0].x, points[0].y );
					DaoxPath_LineTo( self->path, points[1].x, points[1].y );
				}
			}else{
				if( self->data.points->size ){
					DaoxVector2D point = self->data.points->pod.vectors2d[0];
					DaoxPath_MoveTo( self->path, point.x, point.y );
				}
				for(i=1; i<self->data.points->size; ++i){
					DaoxVector2D point = self->data.points->pod.vectors2d[i];
					DaoxPath_LineTo( self->path, point.x, point.y );
				}
				DaoxPath_Close( self->path );
			}
			DaoxPath_Preprocess( path, canvas->triangulator );
		}
		path = self->path;
		if( path ){
			if( path->first->refined.first == NULL )
				DaoxPath_Preprocess( path, canvas->triangulator );

			if( strokeWidth > EPSILON && (strokeAlpha > EPSILON || strokeGradient != NULL) ){
				if( path->strokes == NULL ) path->strokes = DMap_New(0,0);
				self->strokes = DaoxPath_GetStrokes( path, strokeWidth, cap, junction, refine );
			}
		}
		self->dataChanged = 0;
	}
	self->stateChanged = self->dataChanged;

	points = DaoxPlainArray_New( sizeof(DaoxVector2D) );
	if( self->path ){
		DaoxOBBox2D obbox = DaoxOBBox2D_Scale( & self->path->obbox, self->scale );
		if( self->strokes && self->strokes->points->size ){
			float strokeWidth = self->state ? self->state->strokeWidth : 0;
			strokeWidth *= self->scale + EPSILON;
			obbox = DaoxOBBox2D_CopyWithMargin( & obbox, 0.5*strokeWidth );
		}
		DaoxPlainArray_PushOBBoxVertexPoints2D( points, & obbox );
	}
	if( self->children ){
		for(i=0; i<self->children->size; ++i){
			DaoxCanvasItem *it = (DaoxCanvasItem*) self->children->items.pVoid[i];
			DaoxCanvasItem_Update( it, canvas );
			DaoxPlainArray_PushOBBoxVertexPoints2D( points, & it->obbox );
		}
	}
	DaoxOBBox2D_ResetBox( & self->obbox, points->pod.vectors2d, points->size );
	DaoxPlainArray_Delete( points );
}






DaoxCanvasLine* DaoxCanvasLine_New()
{
	return DaoxCanvasItem_New( daox_type_canvas_line );
}
void DaoxCanvasLine_Delete( DaoxCanvasLine *self )
{
	DaoxCanvasItem_Delete( self );
}


DaoxCanvasRect* DaoxCanvasRect_New()
{
	return DaoxCanvasItem_New( daox_type_canvas_rect );
}
void DaoxCanvasRect_Delete( DaoxCanvasRect *self )
{
	DaoxCanvasItem_Delete( self );
}


DaoxCanvasCircle* DaoxCanvasCircle_New()
{
	return DaoxCanvasItem_New( daox_type_canvas_circle );
}
void DaoxCanvasCircle_Delete( DaoxCanvasCircle *self )
{
	DaoxCanvasItem_Delete( self );
}


DaoxCanvasEllipse* DaoxCanvasEllipse_New()
{
	return DaoxCanvasItem_New( daox_type_canvas_ellipse );
}
void DaoxCanvasEllipse_Delete( DaoxCanvasEllipse *self )
{
	DaoxCanvasItem_Delete( self );
}


DaoxCanvasPolyline* DaoxCanvasPolyline_New()
{
	DaoxCanvasPolyline *self = (DaoxCanvasPolyline*)dao_calloc( 1, sizeof(DaoxCanvasPolyline));
	DaoxCanvasItem_Init( self,  daox_type_canvas_polyline );
	self->data.points = DaoxPlainArray_New( sizeof(DaoxVector2D) );
	return self;
}
void DaoxCanvasPolyline_Delete( DaoxCanvasPolyline *self )
{
	DaoxPlainArray_Delete( self->data.points );
	DaoxCanvasItem_Free( self );
	dao_free( self );
}


DaoxCanvasPolygon* DaoxCanvasPolygon_New()
{
	DaoxCanvasPolygon *self = (DaoxCanvasPolygon*)dao_calloc( 1, sizeof(DaoxCanvasPolygon));
	DaoxCanvasItem_Init( self,  daox_type_canvas_polygon );
	self->data.points = DaoxPlainArray_New( sizeof(DaoxVector2D) );
	return self;
}
void DaoxCanvasPolygon_Delete( DaoxCanvasPolygon *self )
{
	DaoxPlainArray_Delete( self->data.points );
	DaoxCanvasItem_Free( self );
	dao_free( self );
}


DaoxCanvasPath* DaoxCanvasPath_New()
{
	DaoxCanvasPath *self = (DaoxCanvasPath*)dao_calloc( 1, sizeof(DaoxCanvasPath));
	DaoxCanvasItem_Init( self,  daox_type_canvas_path );
	return self;
}
void DaoxCanvasPath_Delete( DaoxCanvasPath *self )
{
	DaoxCanvasItem_Free( self );
	dao_free( self );
}


DaoxCanvasImage* DaoxCanvasImage_New()
{
	DaoxCanvasImage *self = (DaoxCanvasImage*)dao_calloc( 1, sizeof(DaoxCanvasImage));
	DaoxCanvasItem_Init( self,  daox_type_canvas_image );
	return self;
}
void DaoxCanvasImage_Delete( DaoxCanvasImage *self )
{
	DaoxCanvasItem_Free( self );
	DaoGC_DecRC( (DaoValue*) self->data.texture );
	dao_free( self );
}


DaoxCanvasText* DaoxCanvasText_New()
{
	DaoxCanvasText *self = (DaoxCanvasText*) dao_calloc( 1, sizeof(DaoxCanvasText) );
	DaoxCanvasItem_Init( self, daox_type_canvas_text );
	self->data.text = DString_New(0);
	return self;
}
void DaoxCanvasText_Delete( DaoxCanvasText *self )
{
	DaoxCanvasItem_Free( self );
	DString_Delete( self->data.text );
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
	self->scale = r / UNIT;
	self->transform.A11 = cosine;
	self->transform.A12 = - sine;
	self->transform.A21 = sine;
	self->transform.A22 = cosine;
	self->transform.B1 = x1;
	self->transform.B2 = y1;
	self->dataChanged = 1;
}

void DaoxCanvasRect_Set( DaoxCanvasRect *self, float x1, float y1, float x2, float y2 )
{
	float w = fabs( x2 - x1 );
	float h = fabs( y2 - y1 );
	assert( self->ctype == daox_type_canvas_rect );
	self->transform = DaoxMatrix3D_Identity();
	self->transform.B1 = x1;
	self->transform.B2 = y1;
	self->dataChanged = 1;
	self->scale = w / UNIT;
}

void DaoxCanvasCircle_Set( DaoxCanvasCircle *self, float x, float y, float r )
{
	assert( self->ctype == daox_type_canvas_circle );
	self->scale = r;
	self->transform = DaoxMatrix3D_Identity();
	self->transform.B1 = x;
	self->transform.B2 = y;
	self->dataChanged = 1;
}

void DaoxCanvasEllipse_Set( DaoxCanvasEllipse *self, float x, float y, float rx, float ry )
{
	assert( self->ctype == daox_type_canvas_ellipse );
	self->scale = rx / UNIT;
	self->transform = DaoxMatrix3D_Identity();
	self->transform.B1 = x;
	self->transform.B2 = y;
	self->dataChanged = 1;
}



void DaoxCanvasPolyline_Add( DaoxCanvasPolyline *self, float x, float y )
{
	assert( self->ctype == daox_type_canvas_polyline );
	DaoxPlainArray_PushVectorXY( self->data.points, x, y );
	self->dataChanged = 1;
}

void DaoxCanvasPolygon_Add( DaoxCanvasPolygon *self, float x, float y )
{
	assert( self->ctype == daox_type_canvas_polygon );
	DaoxPlainArray_PushVectorXY( self->data.points, x, y );
	self->dataChanged = 1;
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
	self->triangulator = DaoxTriangulator_New();
	self->items = DArray_New(D_VALUE);
	self->states = DArray_New(D_VALUE);
	self->rects = DMap_New(0,D_VALUE);
	self->ellipses = DMap_New(0,D_VALUE);
	self->transform = DaoxMatrix3D_Identity();

	self->unitLine = DaoxPath_New();
	self->unitLine->strokes = DMap_New(0,0);
	DaoxPath_MoveTo( self->unitLine, 0, 0 );
	DaoxPath_LineTo( self->unitLine, UNIT, 0 );
	DaoGC_IncRC( (DaoValue*) self->unitLine );
	DaoxPath_Preprocess( self->unitLine, self->triangulator );

	self->unitRect = DaoxPath_New();
	self->unitRect->strokes = DMap_New(0,0);
	DaoxPath_MoveTo( self->unitRect, 0, 0 );
	DaoxPath_LineTo( self->unitRect, UNIT, 0 );
	DaoxPath_LineTo( self->unitRect, UNIT, UNIT );
	DaoxPath_LineTo( self->unitRect, 0, UNIT );
	DaoxPath_Close( self->unitRect );
	DaoGC_IncRC( (DaoValue*) self->unitRect );
	DaoxPath_Preprocess( self->unitRect, self->triangulator );

	self->unitCircle1 = DaoxPath_New();
	self->unitCircle1->strokes = DMap_New(0,0);
	DaoxPath_MoveTo( self->unitCircle1, -UNIT, 0 );
	DaoxPath_ArcTo2( self->unitCircle1,  UNIT, 0, 180, 180 );
	DaoxPath_ArcTo2( self->unitCircle1, -UNIT, 0, 180, 180 );
	DaoxPath_Close( self->unitCircle1 );
	DaoxPath_Preprocess( self->unitCircle1, self->triangulator );

	self->unitCircle2 = DaoxPath_New();
	self->unitCircle2->strokes = DMap_New(0,0);
	DaoxPath_MoveTo( self->unitCircle2, -UNIT, 0 );
	DaoxPath_ArcTo2( self->unitCircle2,  UNIT, 0, 180, 60 );
	DaoxPath_ArcTo2( self->unitCircle2, -UNIT, 0, 180, 60 );
	DaoxPath_Close( self->unitCircle2 );
	DaoxPath_Preprocess( self->unitCircle2, self->triangulator );

	self->unitCircle3 = DaoxPath_New();
	self->unitCircle3->strokes = DMap_New(0,0);
	DaoxPath_MoveTo( self->unitCircle3, -UNIT, 0 );
	DaoxPath_ArcTo2( self->unitCircle3,  UNIT, 0, 180, 20 );
	DaoxPath_ArcTo2( self->unitCircle3, -UNIT, 0, 180, 20 );
	DaoxPath_Close( self->unitCircle3 );
	DaoxPath_Preprocess( self->unitCircle3, self->triangulator );
	return self;
}
void DaoxCanvas_Delete( DaoxCanvas *self )
{
	DaoxSceneNode_Free( (DaoxSceneNode*) self );
	DaoxTriangulator_Delete( self->triangulator );
	DaoGC_DecRC( (DaoValue*) self->unitLine );
	DaoGC_DecRC( (DaoValue*) self->unitRect );
	DaoGC_DecRC( (DaoValue*) self->unitCircle1 );
	DaoGC_DecRC( (DaoValue*) self->unitCircle2 );
	DaoGC_DecRC( (DaoValue*) self->unitCircle3 );
	DArray_Delete( self->items );
	DArray_Delete( self->states );
	DMap_Delete( self->ellipses );
	DMap_Delete( self->rects );
	dao_free( self );
}


void DaoxCanvas_SetViewport( DaoxCanvas *self, float left, float right, float bottom, float top )
{
	self->viewport.left = left;
	self->viewport.right = right;
	self->viewport.bottom = bottom;
	self->viewport.top = top;
}
float DaoxCanvas_Scale( DaoxCanvas *self )
{
	DaoxAABBox2D box = self->viewport;
	float xscale = fabs( box.right - box.left ) / (self->defaultWidth + 1);
	float yscale = fabs( box.top - box.bottom ) / (self->defaultHeight + 1);
	return 0.5 * (xscale + yscale);
}
void DaoxCanvas_SetBackground( DaoxCanvas *self, DaoxColor color )
{
	self->background = color;
}
DaoxCanvasState* DaoxCanvas_GetCurrentState( DaoxCanvas *self )
{
	if( self->states->size == 0 ) return NULL;
	return (DaoxCanvasState*) self->states->items.pVoid[self->states->size-1];
}
DaoxCanvasState* DaoxCanvas_GetOrPushState( DaoxCanvas *self )
{
	if( self->states->size == 0 ) DaoxCanvas_PushState( self );
	return DaoxCanvas_GetCurrentState( self );
}
DaoxCanvasState* DaoxCanvas_PushState( DaoxCanvas *self )
{
	DaoxCanvasState *prev = DaoxCanvas_GetCurrentState( self );
	DaoxCanvasState *state = DaoxCanvasState_New();
	if( prev ) DaoxCanvasState_Copy( state, prev );
	DArray_PushBack( self->states, state );
	return state;
}
void DaoxCanvas_PopState( DaoxCanvas *self )
{
	DArray_PopBack( self->states );
}

void DaoxCanvas_SetStrokeWidth( DaoxCanvas *self, float width )
{
	DaoxCanvasState *state = DaoxCanvas_GetOrPushState( self );
	state->strokeWidth = width;
}
void DaoxCanvas_SetStrokeColor( DaoxCanvas *self, DaoxColor color )
{
	DaoxCanvasState *state = DaoxCanvas_GetOrPushState( self );
	state->strokeColor = color;
}
void DaoxCanvas_SetFillColor( DaoxCanvas *self, DaoxColor color )
{
	DaoxCanvasState *state = DaoxCanvas_GetOrPushState( self );
	state->fillColor = color;
}
void DaoxCanvas_SetDashPattern( DaoxCanvas *self, float pat[], int n )
{
	DaoxCanvasState *state = DaoxCanvas_GetOrPushState( self );
	if( n > 10 ) n = 10;
	state->dash = n;
	memcpy( state->dashPattern, pat, n*sizeof(float) );
}
void DaoxCanvas_SetFont( DaoxCanvas *self, DaoxFont *font, float size )
{
	DaoxCanvasState *state = DaoxCanvas_GetOrPushState( self );
	DaoGC_ShiftRC( (DaoValue*) font, (DaoValue*) state->font );
	state->font = font;
	state->fontSize = size;
}

void DaoxCanvas_AddItem( DaoxCanvas *self, DaoxCanvasItem *item )
{
	DaoxCanvasState *state = DaoxCanvas_GetOrPushState( self );
	if( state->parent ){
		if( state->parent->children == NULL ) state->parent->children = DArray_New(D_VALUE);
		DArray_PushBack( state->parent->children, item );
		DaoxCanvasItem_MarkDataChanged( state->parent );
	}else{
		DArray_PushBack( self->items, item );
	}
	DaoGC_ShiftRC( (DaoValue*) state, (DaoValue*) item->state );
	item->state = DaoxCanvas_GetOrPushState( self );
}

DaoxCanvasItem* DaoxCanvas_AddGroup( DaoxCanvas *self )
{
	DaoxCanvasLine *item = DaoxCanvasItem_New( daox_type_canvas_item );
	DaoxCanvas_AddItem( self, item );
	return item;
}

DaoxCanvasLine* DaoxCanvas_AddLine( DaoxCanvas *self, float x1, float y1, float x2, float y2 )
{
	DaoxCanvasLine *item = DaoxCanvasLine_New();
	DaoxCanvasLine_Set( item, x1, y1, x2, y2 );
	DaoxCanvas_AddItem( self, item );
	DaoGC_IncRC( (DaoValue*) self->unitLine );
	item->path = self->unitLine;
	return item;
}

DaoxCanvasRect* DaoxCanvas_AddRect( DaoxCanvas *self, float x1, float y1, float x2, float y2, float rx, float ry )
{
	float w = fabs(x2 - x1);
	float h = fabs(y2 - y1);
	float r = h / (w + EPSILON);
	float rx2 = fabs(rx) / (w + EPSILON);
	float ry2 = fabs(ry) / (h + EPSILON);
	size_t ratio1 = 0xffff * r;
	size_t ratio2 = 0xff * rx2;
	size_t ratio3 = 0xff * ry2;
	size_t key = ratio1 | (ratio2<<16) | (ratio3<<24);
	DNode *it = DMap_Find( self->rects, (void*) key );
	DaoxCanvasRect *item = DaoxCanvasRect_New();
	DaoxPath *path = NULL;

	if( it == NULL ){
		path = DaoxPath_New();
		path->strokes = DMap_New(0,0);
		DaoxPath_SetRelativeMode( path, 1 );
		if( fabs( rx2 ) > 1E-3 && fabs( ry2 ) > 1E-3 ){
			r *= UNIT;
			ry2 *= r;
			rx2 *= UNIT;
			DaoxPath_MoveTo( path, 0.0, ry2 );
			DaoxPath_CubicTo2( path, 0.0, -0.55*ry2, -0.55*rx2, 0.0, rx2, -ry2 );
			DaoxPath_LineTo( path, UNIT - 2.0*rx2, 0.0 );
			DaoxPath_CubicTo2( path, 0.55*rx2, 0.0, 0.0, -0.55*ry2, rx2, ry2 );
			DaoxPath_LineTo( path, 0.0, r - 2.0*ry2 );
			DaoxPath_CubicTo2( path, 0.0, 0.55*ry2, 0.55*rx2, 0.0, -rx2, ry2 );
			DaoxPath_LineTo( path, -(UNIT - 2.0*rx2), 0.0 );
			DaoxPath_CubicTo2( path, -0.55*rx2, 0.0, 0.0, 0.55*ry2, -rx2, -ry2 );
		}else{
			DaoxPath_MoveTo( path, 0.0, 0.0 );
			DaoxPath_LineTo( path, UNIT, 0.0 );
			DaoxPath_LineTo( path, 0.0, r );
			DaoxPath_LineTo( path, -UNIT, 0.0 );
		}
		DaoxPath_Close( path );
		DaoxPath_Preprocess( path, self->triangulator );
	}else{
		path = (DaoxPath*) it->value.pVoid;
	}

	DaoxCanvasRect_Set( item, x1, y1, x2, y2 );
	DaoxCanvas_AddItem( self, item );
	DaoGC_IncRC( (DaoValue*) path );
	item->path = path;
	return item;
}

DaoxCanvasCircle* DaoxCanvas_AddCircle( DaoxCanvas *self, float x, float y, float r )
{
	daoint R1E6 = (daoint)( r * 1E6 );
	DaoxCanvasCircle *item = DaoxCanvasCircle_New();
	DaoxPath *circle = self->unitCircle3;

	if( r < 8.0 ){
		circle = self->unitCircle1;
	}else if( r < 32.0 ){
		circle = self->unitCircle2;
	}

	DaoxCanvasCircle_Set( item, x, y, r );
	DaoxCanvas_AddItem( self, item );
	DaoGC_IncRC( (DaoValue*) circle );
	item->path = circle;
	return item;
}

DaoxCanvasEllipse* DaoxCanvas_AddEllipse( DaoxCanvas *self, float x, float y, float rx, float ry )
{
	float ratio = ry / (rx + EPSILON);
	size_t key = 0xffff * ratio;
	DNode *it = DMap_Find( self->ellipses, (void*) key );
	DaoxCanvasEllipse *item = DaoxCanvasEllipse_New();
	DaoxPath *path = NULL;

	if( it == NULL ){
		DaoxMatrix3D mat = DaoxMatrix3D_Identity();
		mat.A22 = ratio;
		path = DaoxPath_New();
		path->strokes = DMap_New(0,0);
		DaoxPath_ImportPath( path, self->unitCircle3, & mat );
		DaoxPath_Preprocess( path, self->triangulator );
	}else{
		path = (DaoxPath*) it->value.pVoid;
	}

	DaoxCanvasEllipse_Set( item, x, y, rx, ry );
	DaoxCanvas_AddItem( self, item );
	DaoGC_IncRC( (DaoValue*) path );
	item->path = path;
	return item;
}

DaoxCanvasPolyline* DaoxCanvas_AddPolyline( DaoxCanvas *self )
{
	DaoxCanvasPolyline *item = DaoxCanvasPolyline_New();
	DaoxCanvas_AddItem( self, item );
	return item;
}

DaoxCanvasPolygon* DaoxCanvas_AddPolygon( DaoxCanvas *self )
{
	DaoxCanvasPolygon *item = DaoxCanvasPolygon_New();
	DaoxCanvas_AddItem( self, item );
	return item;
}

DaoxCanvasPath* DaoxCanvas_AddPath( DaoxCanvas *self, DaoxPath *path )
{
	DaoxCanvasPath *item = DaoxCanvasPath_New();
	DaoGC_IncRC( (DaoValue*) path );
	item->path = path;
	DaoxCanvas_AddItem( self, item );
	return item;
}
void DaoxCanvas_AddCharItems( DaoxCanvas *self, DaoxCanvasText *textItem, const wchar_t *text, float x, float y, float degrees )
{
	DaoxGlyph *glyph;
	DaoxCanvasState *state;
	DaoxCanvasText *chitem;
	DaoxPath *textPath = textItem->path;
	DaoxFont *font = textItem->state->font;
	DaoxMatrix3D transform = {1.0,0.0,0.0,0.0,1.0,0.0};
	DaoxMatrix3D rotation = {0.0,0.0,0.0,0.0,0.0,0.0};
	float width = textItem->state->strokeWidth;
	float size = textItem->state->fontSize;
	float scale = size / (float)font->fontHeight;
	float offset, advance, angle = degrees * M_PI / 180.0;

	rotation.A11 = cos( angle );
	rotation.A12 = - sin( angle );
	rotation.A21 = - rotation.A12;
	rotation.A22 = rotation.A11;
	DaoxMatrix3D_Multiply( & transform, rotation );

	state = DaoxCanvas_PushState( self );
	DaoxCanvasState_SetParentItem( state, textItem );

	offset = x;
	while( *text ){
		DaoxAABBox2D bounds = {0.0,0.0,0.0,0.0};
		wchar_t ch = *text++;
		glyph = DaoxFont_GetCharGlyph( font, ch );
		if( glyph == NULL ) break;

		bounds.right = glyph->advanceWidth;
		bounds.top = font->lineSpace;
		bounds = DaoxAABBox2D_Transform( & bounds, & rotation );
		advance = bounds.right - bounds.left;

		if( textItem->children == NULL ) textItem->children = DArray_New(D_VALUE);

		chitem = DaoxCanvasPath_New();
		DaoxCanvas_AddItem( self, chitem );
		DaoGC_IncRC( (DaoValue*) glyph->shape );
		chitem->path = glyph->shape;
		chitem->scale = scale;

		if( textPath ){
			float p = 0.0, adv = 0.5 * (scale * advance + width);
			DaoxPathSegment seg1 = DaoxPath_LocateByDistance( textPath, offset+adv, &p );
			DaoxPathSegment seg2 = DaoxPath_LocateByDistance( textPath, offset, &p );
			if( seg1.bezier == 0 ) seg1 = seg2;
			if( seg2.bezier ){
				float dx = seg1.P2.x - seg1.P1.x;
				float dy = seg1.P2.y - seg1.P1.y;
				DaoxMatrix3D_RotateXAxisTo( & transform, dx, dy );
				DaoxMatrix3D_Multiply( & transform, rotation );
				transform.B1 = (1.0 - p) * seg2.P1.x + p * seg2.P2.x;
				transform.B2 = (1.0 - p) * seg2.P1.y + p * seg2.P2.y;
			}
		}else{
			transform.B1 = offset;
			transform.B2 = y;
		}
		chitem->transform = transform;
		offset += scale * advance + width;
		chitem = NULL;
	}
	DaoxCanvas_PopState( self );
}
DaoxCanvasText* DaoxCanvas_AddText( DaoxCanvas *self, const wchar_t *text, float x, float y, float degrees )
{
	DaoxCanvasPath *item;
	DaoxCanvasState *state;
	if( self->states->size == 0 ) return NULL;
	state = DaoxCanvas_GetOrPushState( self );
	if( state->font == NULL ) return NULL;

	item = DaoxCanvasText_New();
	DaoxCanvas_AddItem( self, item );
	DaoxCanvas_AddCharItems( self, item, text, x, y, degrees );
	return item;
}
DaoxCanvasText* DaoxCanvas_AddPathText( DaoxCanvas *self, const wchar_t *text, DaoxPath *path, float degrees )
{
	DaoxCanvasPath *item;
	DaoxCanvasState *state;
	if( self->states->size == 0 ) return NULL;
	state = DaoxCanvas_GetOrPushState( self );
	if( state->font == NULL ) return NULL;

	item = DaoxCanvasText_New();
	DaoxCanvas_AddItem( self, item );

	DaoxPath_Preprocess( path, self->triangulator );
	DaoGC_ShiftRC( (DaoValue*) path, (DaoValue*) item->path );
	item->path = path;
	item->visible = 0;

	DaoxCanvas_AddCharItems( self, item, text, 0, 0, degrees );
	return item;
}
DaoxCanvasImage* DaoxCanvas_AddImage( DaoxCanvas *self, DaoxImage *image, float x, float y )
{
	DaoxCanvasState *state = DaoxCanvas_GetOrPushState( self );
	DaoxCanvasPath *item = DaoxCanvasImage_New();
	DaoxTexture *texture = DaoxTexture_New();

	item->transform = DaoxMatrix3D_Identity();
	item->transform.B1 = x;
	item->transform.B2 = y;

	DaoxTexture_SetImage( texture, image );
	DaoxCanvas_AddItem( self, item );
	DaoGC_ShiftRC( (DaoValue*) texture, (DaoValue*) item->data.texture );
	item->data.texture = texture;
	return item;
}








static void ITEM_SetVisible( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasItem *self = (DaoxCanvasItem*) p[0];
	self->visible = p[1]->xEnum.value;
}
static void ITEM_SetTransform( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasItem *self = (DaoxCanvasItem*) p[0];
	DaoArray *array = (DaoArray*) p[1];
	daoint n = array->size;
	if( n != 4 && n != 6 ){
		DaoProcess_RaiseException( proc, DAO_ERROR_PARAM, "need matrix with 4 or 6 elements" );
		return;
	}
	DaoxMatrix3D_Set( & self->transform, array->data.f, n );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void ITEM_MulTransform( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxMatrix3D transform;
	DaoxCanvasItem *self = (DaoxCanvasItem*) p[0];
	DaoArray *array = (DaoArray*) p[1];
	daoint n = array->size;
	if( n != 4 && n != 6 ){
		DaoProcess_RaiseException( proc, DAO_ERROR_PARAM, "need matrix with 4 or 6 elements" );
		return;
	}
	DaoxMatrix3D_Set( & transform, array->data.f, n );
	DaoxMatrix3D_Multiply( & self->transform, transform );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}

static void DaoxCanvasItem_GetGCFields( void *p, DArray *values, DArray *arrays, DArray *maps, int remove )
{
	daoint i, n;
	DaoxCanvasItem *self = (DaoxCanvasItem*) p;
	if( self->children == NULL ) return;
	DArray_Append( arrays, self->children );
}

static DaoFuncItem DaoxCanvasItemMeths[]=
{
	{ ITEM_SetVisible,  "SetVisible( self : CanvasItem, visible :enum<false,true> )" },
	{ ITEM_SetTransform, "SetTransform( self : CanvasItem, transform : array<float> ) => CanvasItem" },
	{ ITEM_MulTransform, "MulTransform( self : CanvasItem, transform : array<float> ) => CanvasItem" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasItem_Typer =
{
	"CanvasItem", NULL, NULL, (DaoFuncItem*) DaoxCanvasItemMeths, {0}, {0},
	(FuncPtrDel)DaoxCanvasItem_Delete, DaoxCanvasItem_GetGCFields
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
	{ LINE_Set, "Set( self : CanvasLine, x1 = 0F, y1 = 0F, x2 = 1F, y2 = 1F ) => CanvasLine" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasLine_Typer =
{
	"CanvasLine", NULL, NULL, (DaoFuncItem*) DaoxCanvasLineMeths,
	{ & DaoxCanvasItem_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasItem_Delete, DaoxCanvasItem_GetGCFields
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
	//{ RECT_Set,   "Set( self : CanvasRect, x1 = 0F, y1 = 0F, x2 = 1F, y2 = 1F ) => CanvasRect" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasRect_Typer =
{
	"CanvasRect", NULL, NULL, (DaoFuncItem*) DaoxCanvasRectMeths,
	{ & DaoxCanvasItem_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasItem_Delete, DaoxCanvasItem_GetGCFields
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
	{ CIRCLE_Set,  "Set( self : CanvasCircle, cx = 0F, cy = 0F, r = 1F ) => CanvasCircle" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasCircle_Typer =
{
	"CanvasCircle", NULL, NULL, (DaoFuncItem*) DaoxCanvasCircleMeths,
	{ & DaoxCanvasItem_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasItem_Delete, DaoxCanvasItem_GetGCFields
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
	{ ELLIPSE_Set,  "Set( self : CanvasEllipse, cx = 0F, cy = 0F, rx = 1F, ry = 1F ) => CanvasEllipse" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasEllipse_Typer =
{
	"CanvasEllipse", NULL, NULL, (DaoFuncItem*) DaoxCanvasEllipseMeths,
	{ & DaoxCanvasItem_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasItem_Delete, DaoxCanvasItem_GetGCFields
};





static void POLYLINE_Add( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasPolyline *self = (DaoxCanvasPolyline*) p[0];
	float x = p[1]->xFloat.value;
	float y = p[2]->xFloat.value;
	DaoxCanvasPolyline_Add( self, x, y );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static DaoFuncItem DaoxCanvasPolylineMeths[]=
{
	{ POLYLINE_Add,  "Add( self : CanvasPolyline, x: float, y: float ) => CanvasPolyline" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasPolyline_Typer =
{
	"CanvasPolyline", NULL, NULL, (DaoFuncItem*) DaoxCanvasPolylineMeths,
	{ & DaoxCanvasItem_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasItem_Delete, DaoxCanvasItem_GetGCFields
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
	{ POLYGON_Add,   "Add( self : CanvasPolygon, x : float, y : float ) => CanvasPolygon" },
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasPolygon_Typer =
{
	"CanvasPolygon", NULL, NULL, (DaoFuncItem*) DaoxCanvasPolygonMeths,
	{ & DaoxCanvasItem_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasItem_Delete, DaoxCanvasItem_GetGCFields
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
	{ & DaoxCanvasItem_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasItem_Delete, DaoxCanvasItem_GetGCFields
};




static DaoFuncItem DaoxCanvasTextMeths[]=
{
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasText_Typer =
{
	"CanvasText", NULL, NULL, (DaoFuncItem*) DaoxCanvasTextMeths,
	{ & DaoxCanvasItem_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasItem_Delete, DaoxCanvasItem_GetGCFields
};



static DaoFuncItem DaoxCanvasImageMeths[]=
{
	{ NULL, NULL }
};

DaoTypeBase DaoxCanvasImage_Typer =
{
	"CanvasImage", NULL, NULL, (DaoFuncItem*) DaoxCanvasImageMeths,
	{ & DaoxCanvasItem_Typer, NULL }, { NULL },
	(FuncPtrDel)DaoxCanvasItem_Delete, DaoxCanvasItem_GetGCFields
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
static void CANVAS_AddPolyline( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxCanvasPolyline *item = DaoxCanvas_AddPolyline( self );
	float x1 = p[1]->xFloat.value, y1 = p[2]->xFloat.value;
	float x2 = p[3]->xFloat.value, y2 = p[4]->xFloat.value;
	DaoxCanvasPolyline_Add( item, x1, y1 );
	DaoxCanvasPolyline_Add( item, x2, y2 );
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
	DaoxCanvasText *item = DaoxCanvas_AddText( self, DString_GetWCS( text ), x, y, a );
	if( item == NULL ){
		DaoProcess_RaiseException( proc, DAO_ERROR, "no font is set" );
		return;
	}
	DaoProcess_PutValue( proc, (DaoValue*) item );
}
static void CANVAS_AddText2( DaoProcess *proc, DaoValue *p[], int N )
{
	float a = p[3]->xFloat.value;
	wchar_t *text = DaoValue_TryGetWCString( p[1] );
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxPath *path = (DaoxPath*) p[2];
	DaoxCanvasText *item = DaoxCanvas_AddPathText( self, text, path, a );
	if( item == NULL ){
		DaoProcess_RaiseException( proc, DAO_ERROR, "no font is set" );
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

#if 0
#warning"DaoxCanvas_glDrawSceneImage"
//void DaoxCanvas_glDrawSceneImage( DaoxCanvas *canvas, DaoxAABBox2D viewport, DaoxImage *image, int width, int height );

static void CANVAS_RenderToImage( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxImage *image = (DaoxImage*) p[1];
	float width = p[2]->xFloat.value;
	float height = p[3]->xFloat.value;
	//DaoxCanvas_glDrawSceneImage( self, self->viewport, image, width, height );
}
#endif
static void CANVAS_AddPath( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxPath *path = (DaoxPath*) p[1];
	DaoxCanvasPath *item = DaoxCanvas_AddPath( self, path );
	DaoProcess_PutValue( proc, (DaoValue*) item );
}
static void CANVAS_PushState( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxCanvasState *state = DaoxCanvas_PushState( self );
	DaoProcess_PutValue( proc, (DaoValue*) state );
}
static void CANVAS_PopState( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxCanvas_PopState( self );
}



static void STATE_SetStrokeWidth( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasState *self = (DaoxCanvasState*) p[0];
	self->strokeWidth = p[1]->xFloat.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void STATE_SetStrokeColor( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasState *self = (DaoxCanvasState*) p[0];
	self->strokeColor.red   = p[1]->xFloat.value;
	self->strokeColor.green = p[2]->xFloat.value;
	self->strokeColor.blue  = p[3]->xFloat.value;
	self->strokeColor.alpha = p[4]->xFloat.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void STATE_SetFillColor( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasState *self = (DaoxCanvasState*) p[0];
	self->fillColor.red   = p[1]->xFloat.value;
	self->fillColor.green = p[2]->xFloat.value;
	self->fillColor.blue  = p[3]->xFloat.value;
	self->fillColor.alpha = p[4]->xFloat.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void STATE_SetLineCap( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasState *self = (DaoxCanvasState*) p[0];
	self->linecap = p[1]->xEnum.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void STATE_SetJunction( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasState *self = (DaoxCanvasState*) p[0];
	self->junction = p[1]->xEnum.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void STATE_SetDash( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasState *self = (DaoxCanvasState*) p[0];
	DaoArray *array = (DaoArray*) p[1];
	DaoxCanvasState_SetDashPattern( self, array->data.f, array->size );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void STATE_SetStrokeGradient( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasState *self = (DaoxCanvasState*) p[0];
	if( self->strokeGradient == NULL ){
		self->strokeGradient = DaoxColorGradient_New( DAOX_GRADIENT_BASE );
		DaoGC_IncRC( (DaoValue*) self->strokeGradient );
	}
	DaoProcess_PutValue( proc, (DaoValue*) self->strokeGradient );
}
static void STATE_SetFillGradient( DaoProcess *proc, DaoValue *p[], int N, int type )
{
	DaoxCanvasState *self = (DaoxCanvasState*) p[0];
	if( self->fillGradient == NULL ){
		self->fillGradient = DaoxColorGradient_New( type );
		DaoGC_IncRC( (DaoValue*) self->fillGradient );
	}
	DaoProcess_PutValue( proc, (DaoValue*) self->fillGradient );
}
static void STATE_SetLinearGradient( DaoProcess *proc, DaoValue *p[], int N )
{
	STATE_SetFillGradient( proc, p, N, DAOX_GRADIENT_LINEAR );
}
static void STATE_SetRadialGradient( DaoProcess *proc, DaoValue *p[], int N )
{
	STATE_SetFillGradient( proc, p, N, DAOX_GRADIENT_RADIAL );
}
static void STATE_SetPathGradient( DaoProcess *proc, DaoValue *p[], int N )
{
	STATE_SetFillGradient( proc, p, N, DAOX_GRADIENT_PATH );
}
static void STATE_SetFont( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasState *self = (DaoxCanvasState*) p[0];
	DaoxFont *font = (DaoxFont*) p[1];
	DaoxCanvasState_SetFont( self, font, p[2]->xFloat.value );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void STATE_SetParentItem( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvasState *self = (DaoxCanvasState*) p[0];
	DaoxCanvasItem *item = (DaoxCanvasItem*) p[1];
	DaoGC_ShiftRC( (DaoValue*) item, (DaoValue*) self->parent );
	self->parent = item;
}
static void CANVAS_Test( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	int i, n = self->items->size;
	for(i=0; i<n; i++){
		DaoxCanvasPath *item = (DaoxCanvasItem*) self->items->items.pVoid[i];
		//DaoxCanvasItem_UpdateData( item, self );
	}
}

static void DaoxCanvas_GetGCFields( void *p, DArray *values, DArray *arrays, DArray *maps, int remove )
{
	daoint i, n;
	DaoxCanvas *self = (DaoxCanvas*) p;
	if( self->items == NULL ) return;
	DArray_Append( arrays, self->items );
}

static DaoFuncItem DaoxCanvasMeths[]=
{
	{ CANVAS_New,         "Canvas()" },

	{ CANVAS_SetViewport,   "SetViewport( self: Canvas, left: float, right: float, bottom: float, top: float )" },

	{ CANVAS_GetViewport,   "GetViewport( self: Canvas ) => tuple<left:float,right:float,bottom:float,top:float>" },

	{ CANVAS_SetBackground,  "SetBackground( self : Canvas, red: float, green: float, blue: float, alpha = 1F ) => Canvas" },

	{ CANVAS_PushState,   "PushState( self: Canvas ) => CanvasState" },

	{ CANVAS_PopState,    "PopState( self: Canvas )" },

	{ CANVAS_AddGroup,   "AddGroup( self: Canvas ) => CanvasItem" },
	{ CANVAS_AddCircle,    "AddCircle( self: Canvas, x: float, y: float, r: float ) => CanvasCircle" },
	{ CANVAS_AddLine,   "AddLine( self: Canvas, x1: float, y1: float, x2: float, y2: float ) => CanvasLine" },

	{ CANVAS_AddRect,   "AddRect( self: Canvas, x1: float, y1: float, x2: float, y2: float, rx = 0F, ry = 0F ) => CanvasRect" },


	{ CANVAS_AddEllipse,   "AddEllipse( self: Canvas, x: float, y: float, rx: float, ry: float ) => CanvasEllipse" },

	{ CANVAS_AddPolyline,  "AddPolyline( self: Canvas, x1: float, y1: float, x2: float, y2: float ) => CanvasPolyline" },

	{ CANVAS_AddPolygon,   "AddPolygon( self: Canvas ) => CanvasPolygon" },



	{ CANVAS_AddImage,     "AddImage( self: Canvas, image: Image, x :float, y :float ) => CanvasImage" },

#if 0
	{ CANVAS_RenderToImage,  "RenderToImage( self: Canvas, image: Image, width :float, height :float )" },
	{ CANVAS_Test,         "Test( self: Canvas )" },
#endif
	{ CANVAS_AddPath,      "AddPath( self: Canvas, path : Path ) => CanvasPath" },
	{ CANVAS_AddText,      "AddText( self: Canvas, text : string, x :float, y :float, degrees = 0F ) => CanvasText" },

	{ CANVAS_AddText2,     "AddText( self: Canvas, text : string, path :Path, degrees = 0F ) => CanvasText" },
	{ NULL, NULL }
};

extern DaoTypeBase DaoxSceneNode_Typer;
DaoTypeBase DaoxCanvas_Typer =
{
	"Canvas", NULL, NULL, (DaoFuncItem*) DaoxCanvasMeths,
	{ & DaoxSceneNode_Typer, 0 }, {0},
	(FuncPtrDel)DaoxCanvas_Delete, DaoxCanvas_GetGCFields
};


static DaoFuncItem DaoxCanvasStateMeths[]=
{
	{ STATE_SetStrokeWidth,  "SetStrokeWidth( self : CanvasState, width = 1F ) => CanvasState" },

	{ STATE_SetStrokeColor,  "SetStrokeColor( self : CanvasState, red: float, green: float, blue: float, alpha = 1F ) => CanvasState" },

	{ STATE_SetFillColor,  "SetFillColor( self : CanvasState, red: float, green: float, blue: float, alpha = 1F ) => CanvasState" },

	{ STATE_SetLineCap, "SetLineCap( self : CanvasState, cap: enum<none,flat,sharp,round> = $none ) => CanvasState" },
	{ STATE_SetJunction, "SetJunction( self : CanvasState, junction: enum<none,flat,sharp,round> = $sharp ) => CanvasState" },

	{ STATE_SetDash, "SetDashPattern( self : CanvasState, pattern = [3F,2F] ) => CanvasState" },

	{ STATE_SetStrokeGradient, "SetStrokeGradient( self : CanvasState ) => ColorGradient" },

	{ STATE_SetLinearGradient, "SetLinearGradient( self : CanvasState ) => LinearGradient" },

	{ STATE_SetRadialGradient, "SetRadialGradient( self : CanvasState ) => RadialGradient" },

	//{ STATE_SetPathGradient,   "SetPathGradient( self : CanvasState ) => PathGradient" },

	{ STATE_SetFont,      "SetFont( self: CanvasState, font : Font, size = 12F ) => CanvasState" },
	{ STATE_SetParentItem,   "SetParentItem( self: CanvasState, item : CanvasItem )" },
	{ NULL, NULL }
};


DaoTypeBase DaoxCanvasState_Typer =
{
	"CanvasState", NULL, NULL, (DaoFuncItem*) DaoxCanvasStateMeths, {0}, {0},
	(FuncPtrDel)DaoxCanvasState_Delete, NULL
};



static DaoFuncItem DaoxPathMeths[]=
{
	{ PATH_New,    "Path() => Path" },

	{ PATH_MoveTo,    "MoveTo( self : Path, x : float, y : float ) => Path" },

	{ PATH_LineRelTo,    "LineTo( self : Path, x : float, y : float ) => Path" },

	{ PATH_ArcRelTo,     "ArcTo( self : Path, x : float, y : float, degrees : float ) => Path" },

	{ PATH_ArcRelBy,     "ArcBy( self : Path, cx : float, cy : float, degrees : float ) => Path" },

	{ PATH_QuadRelTo,   "QuadTo( self : Path, cx : float, cy : float, x : float, y : float ) => Path" },

	{ PATH_CubicRelTo,   "CubicTo( self : Path, cx : float, cy : float, x : float, y : float ) => Path" },

	{ PATH_CubicRelTo2,  "CubicTo( self : Path, cx0 : float, cy0 : float, cx : float, cy : float, x : float, y : float ) => Path" },

	{ PATH_LineAbsTo,    "LineAbsTo( self : Path, x : float, y : float ) => Path" },

	{ PATH_ArcAbsTo,     "ArcAbsTo( self : Path, x : float, y : float, degrees : float ) => Path" },

	{ PATH_ArcAbsBy,     "ArcAbsBy( self : Path, cx : float, cy : float, degrees : float ) => Path" },

	{ PATH_QuadAbsTo,   "QuadAbsTo( self : Path, cx : float, cy : float, x : float, y : float ) => Path" },

	{ PATH_CubicAbsTo,   "CubicAbsTo( self : Path, cx : float, cy : float, x : float, y : float ) => Path" },

	{ PATH_CubicAbsTo2,  "CubicAbsTo( self : Path, cx0 : float, cy0 : float, cx : float, cy : float, x : float, y : float ) => Path" },

	{ PATH_Close,     "Close( self : Path ) => Path" },
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

	DaoNamespace_TypeDefine( ns, "tuple<red:float,green:float,blue:float,alpha:float>", "Color" );

	daox_type_color_gradient = DaoNamespace_WrapType( ns, & DaoxColorGradient_Typer, 0 );
	daox_type_linear_gradient = DaoNamespace_WrapType( ns, & DaoxLinearGradient_Typer, 0 );
	daox_type_radial_gradient = DaoNamespace_WrapType( ns, & DaoxRadialGradient_Typer, 0 );
	daox_type_path_gradient = DaoNamespace_WrapType( ns, & DaoxPathGradient_Typer, 0 );
	daox_type_path = DaoNamespace_WrapType( ns, & DaoxPath_Typer, 0 );

	daox_type_canvas_state = DaoNamespace_WrapType( ns, & DaoxCanvasState_Typer, 0 );
	daox_type_canvas = DaoNamespace_WrapType( ns, & DaoxCanvas_Typer, 0 );
	daox_type_canvas_item = DaoNamespace_WrapType( ns, & DaoxCanvasItem_Typer, 0 );
	daox_type_canvas_line = DaoNamespace_WrapType( ns, & DaoxCanvasLine_Typer, 0 );
	daox_type_canvas_rect = DaoNamespace_WrapType( ns, & DaoxCanvasRect_Typer, 0 );
	daox_type_canvas_circle = DaoNamespace_WrapType( ns, & DaoxCanvasCircle_Typer, 0 );
	daox_type_canvas_ellipse = DaoNamespace_WrapType( ns, & DaoxCanvasEllipse_Typer, 0 );
	daox_type_canvas_polyline = DaoNamespace_WrapType( ns, & DaoxCanvasPolyline_Typer, 0 );
	daox_type_canvas_polygon = DaoNamespace_WrapType( ns, & DaoxCanvasPolygon_Typer, 0 );
	daox_type_canvas_path = DaoNamespace_WrapType( ns, & DaoxCanvasPath_Typer, 0 );
	daox_type_canvas_text = DaoNamespace_WrapType( ns, & DaoxCanvasText_Typer, 0 );
	daox_type_canvas_image = DaoNamespace_WrapType( ns, & DaoxCanvasImage_Typer, 0 );

	DaoTriangulator_OnLoad( vmSpace, ns );
	return 0;
}
