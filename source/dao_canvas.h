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

/*
// Vector Graphics Frontend:
//
// TODO: SVG Tiny backend;
//
// TODO:
// * multiple line, rect and ellipse in one item;
// * type for color and transformation matrix?
// * locating item by point;
*/

#ifndef __DAO_CANVAS_H__
#define __DAO_CANVAS_H__

#include "dao_common.h"
#include "dao_triangulator.h"
#include "dao_font.h"
#include "dao_image.h"
#include "dao_scene.h"
#include "daoStdtype.h"
#include "daoValue.h"

#define DAOX_MAX_DASH            16
#define DAOX_MAX_GRADIENT_STOPS  64

typedef struct DaoxBrush      DaoxBrush;
typedef struct DaoxGradient   DaoxGradient;
typedef struct DaoxCanvasNode DaoxCanvasNode;
typedef struct DaoxCanvas     DaoxCanvas;


enum DaoxGradientTypes
{
	DAOX_GRADIENT_BASE ,
	DAOX_GRADIENT_LINEAR ,
	DAOX_GRADIENT_RADIAL ,
	DAOX_GRADIENT_STROKE ,
	DAOX_GRADIENT_PATH
};

struct DaoxGradient
{
	DAO_CSTRUCT_COMMON;

	int  gradient;

	DArray  *stops;  /* <float> */
	DArray  *colors; /* <DaoxColor> */

	DaoxVector2D  points[2];
	float         radius;
};
DAO_DLL DaoType *daox_type_gradient;
DAO_DLL DaoType *daox_type_linear_gradient;
DAO_DLL DaoType *daox_type_radial_gradient;
DAO_DLL DaoType *daox_type_path_gradient;




struct DaoxBrush
{
	DAO_CSTRUCT_COMMON;

	uchar_t  dash;
	uchar_t  linecap;
	uchar_t  junction;
	float    fontSize;
	float    strokeWidth;
	float    dashPattern[DAOX_MAX_DASH];

	DaoxColor  strokeColor;
	DaoxColor  fillColor;

	DaoxGradient  *strokeGradient;
	DaoxGradient  *fillGradient;
	DaoxFont      *font;
};
DAO_DLL DaoType *daox_type_brush;





/*
// Base for all canvas item types:
*/
struct DaoxCanvasNode
{
	DAO_CSTRUCT_COMMON;

	uint_t   hash;
	uchar_t  visible;
	uchar_t  dataChanged;  /* geometry changed; */
	uchar_t  stateChanged; /* orientation, stroke width etc. changed; */
	float    magnitude;    /* scale on the raw data, no effects on the strokes and children; */

	DaoxOBBox2D       obbox;        /* local space; */
	DaoxVector2D      scale;        /* local space; */
	DaoxVector2D      rotation;     /* local space (cos,sin); */
	DaoxVector2D      translation;  /* parent space; */
	DaoxBrush        *brush;

	DaoxCanvasNode   *parent;     /* parent item; */
	DList            *children;   /* children items; */

	DaoxPath      *path;     /* may be filled with filling color; */
	DaoxPathMesh  *strokes;  /* may be filled with stroking color; */

	union {
		DArray       *points;  /* polyline or polygon item; */
		DaoxTexture  *texture; /* image item; */
		DString      *text;    /* text item; */
	} data;
};



/*
// A line is always defined locally by (0,0)-(1,0).
// Its actual length and orientation are determined by its transformations.
*/
typedef  DaoxCanvasNode  DaoxCanvasLine;

/*
// A rectable is always defined locally by (0,0)-(1,0)-(1,1)-(0,1).
// Its actual shape and orientation are determined by its transformations.
*/
typedef  DaoxCanvasNode  DaoxCanvasRect;

/*
// A circle is always defined locally as unit circle located at (0,0).
// Its actual shape and orientation are determined by its transformations.
*/
typedef  DaoxCanvasNode  DaoxCanvasCircle;

/*
// An ellipse is always defined locally as unit circle located at (0,0).
// Its actual shape and orientation are determined by its transformations.
*/
typedef  DaoxCanvasNode  DaoxCanvasEllipse;


typedef DaoxCanvasNode  DaoxCanvasPolyline;
typedef DaoxCanvasNode  DaoxCanvasPolygon;
typedef DaoxCanvasNode  DaoxCanvasPath;
typedef DaoxCanvasNode  DaoxCanvasImage;
typedef DaoxCanvasNode  DaoxCanvasText;



DAO_DLL DaoType *daox_type_canvas_node;
DAO_DLL DaoType *daox_type_canvas_line;
DAO_DLL DaoType *daox_type_canvas_rect;
DAO_DLL DaoType *daox_type_canvas_circle;
DAO_DLL DaoType *daox_type_canvas_ellipse;
DAO_DLL DaoType *daox_type_canvas_polyline;
DAO_DLL DaoType *daox_type_canvas_polygon;
DAO_DLL DaoType *daox_type_canvas_path;
DAO_DLL DaoType *daox_type_canvas_text;
DAO_DLL DaoType *daox_type_canvas_image;





struct DaoxCanvas
{
	DaoxSceneNode  base;

	DaoxAABBox2D  viewport;
	DaoxMatrix3D  transform;

	DaoxColor  background;

	DList  *nodes;
	DList  *actives;
	DList  *brushes;

	DMap  *rects;
	DMap  *ellipses;
	DMap  *glyphs;

	/*
	// Map hash values of canvas items to lists of canvas items.
	*/
	DMap  *meshMap;  /* DMap<int,DList<DaoxCanvasNode*>>; */

	DaoxPath  *unitLine;
	DaoxPath  *unitRect;
	DaoxPath  *unitCircle1;
	DaoxPath  *unitCircle2;
	DaoxPath  *unitCircle3;

	DaoxTriangulator  *triangulator;
};
DAO_DLL DaoType *daox_type_canvas;




#ifdef __cplusplus
extern "C"{
#endif




DAO_DLL DaoxGradient* DaoxGradient_New( int type );
DAO_DLL void DaoxGradient_Delete( DaoxGradient *self );
DAO_DLL void DaoxGradient_Add( DaoxGradient *self, float stop, DaoxColor color );
DAO_DLL void DaoxGradient_Copy( DaoxGradient *self, DaoxGradient *other );




DAO_DLL DaoxBrush* DaoxBrush_New();
DAO_DLL void DaoxBrush_Delete( DaoxBrush *self );
DAO_DLL void DaoxBrush_Copy( DaoxBrush *self, DaoxBrush *other );

DAO_DLL void DaoxBrush_SetStrokeWidth( DaoxBrush *self, float width );
DAO_DLL void DaoxBrush_SetStrokeColor( DaoxBrush *self, DaoxColor color );
DAO_DLL void DaoxBrush_SetFillColor( DaoxBrush *self, DaoxColor color );
DAO_DLL void DaoxBrush_SetDashPattern( DaoxBrush *self, float pat[], int n );
DAO_DLL void DaoxBrush_SetFont( DaoxBrush *self, DaoxFont *font, float size );




DAO_DLL DaoxCanvasNode* DaoxCanvasNode_New( DaoType *type );
DAO_DLL void DaoxCanvasNode_Delete( DaoxCanvasNode *self );

DAO_DLL void DaoxCanvasNode_MarkDataChanged( DaoxCanvasNode *self );
DAO_DLL void DaoxCanvasNode_MarkStateChanged( DaoxCanvasNode *self );
DAO_DLL void DaoxCanvasNode_Update( DaoxCanvasNode *self, DaoxCanvas *canvas );
DAO_DLL DaoxMatrix3D DaoxCanvasNode_GetLocalTransform( DaoxCanvasNode *self );



DAO_DLL void DaoxCanvasLine_Set( DaoxCanvasLine *self, float x1, float y1, float x2, float y2 );

//DAO_DLL void DaoxCanvasRect_Set( DaoxCanvasRect *self, float x1, float y1, float x2, float y2, float rx, float ry );

DAO_DLL void DaoxCanvasCircle_Set( DaoxCanvasCircle *self, float x, float y, float r );

DAO_DLL void DaoxCanvasEllipse_Set( DaoxCanvasEllipse *self, float x, float y, float rx, float ry );

DAO_DLL void DaoxCanvasPolyline_Add( DaoxCanvasPolyline *self, float x, float y );

DAO_DLL void DaoxCanvasPolygon_Add( DaoxCanvasPolygon *self, float x, float y );








DAO_DLL DaoxCanvas* DaoxCanvas_New();
DAO_DLL void DaoxCanvas_Delete( DaoxCanvas *self );

DAO_DLL void DaoxCanvas_SetViewport( DaoxCanvas *self, float left, float right, float bottom, float top );
DAO_DLL float DaoxCanvas_Scale( DaoxCanvas *self );
DAO_DLL void DaoxCanvas_SetBackground( DaoxCanvas *self, DaoxColor color );

DAO_DLL DaoxBrush* DaoxCanvas_PushBrush( DaoxCanvas *self, int index );
DAO_DLL void DaoxCanvas_PopBrush( DaoxCanvas *self );


DAO_DLL DaoxCanvasLine* DaoxCanvas_AddLine( DaoxCanvas *self, float x1, float y1, float x2, float y2 );

DAO_DLL DaoxCanvasRect* DaoxCanvas_AddRect( DaoxCanvas *self, float x1, float y1, float x2, float y2, float rx, float ry );

DAO_DLL DaoxCanvasCircle* DaoxCanvas_AddCircle( DaoxCanvas *self, float x, float y, float r );

DAO_DLL DaoxCanvasEllipse* DaoxCanvas_AddEllipse( DaoxCanvas *self, float x, float y, float rx, float ry );

DAO_DLL DaoxCanvasPolyline* DaoxCanvas_AddPolyline( DaoxCanvas *self );

DAO_DLL DaoxCanvasPolygon* DaoxCanvas_AddPolygon( DaoxCanvas *self );

DAO_DLL DaoxCanvasPath* DaoxCanvas_AddPath( DaoxCanvas *self, DaoxPath *path );

DAO_DLL DaoxCanvasText* DaoxCanvas_AddText( DaoxCanvas *self, const char *text, float x, float y, float degrees );

DAO_DLL DaoxCanvasText* DaoxCanvas_AddPathText( DaoxCanvas *self, const char *text, DaoxPath *path, float degrees );

DAO_DLL DaoxCanvasImage* DaoxCanvas_AddImage( DaoxCanvas *self, DaoxImage *image, float x, float y );




#ifdef __cplusplus
}
#endif

#endif
