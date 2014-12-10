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


#include "dao_opengl.h"
#include "dao_painter.h"
#include "dao_renderer.h"
#include "dao_resource.h"
#include "dao_format.h"
#include "dao_terrain.h"


DaoVmSpace *dao_vmspace_graphics = NULL;


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


static void DaoxPathMesh_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxPathMesh *self = (DaoxPathMesh*) p;
	if( self->path ) DList_Append( values, self->path );
	if( remove ) self->path = NULL;
}
DaoTypeBase DaoxPathMesh_Typer =
{
	"PathMesh", NULL, NULL, NULL, { NULL }, { NULL },
	(FuncPtrDel)DaoxPathMesh_Delete, DaoxPathMesh_GetGCFields
};


static void DaoxPathCache_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DNode *it;
	DaoxPathCache *self = (DaoxPathCache*) p;
	for(it=DMap_First(self->paths); it; it=DMap_Next(self->paths,it)){
		DList_Append( lists, it->value.pVoid );
	}
	for(it=DMap_First(self->meshes); it; it=DMap_Next(self->meshes,it)){
		DList_Append( lists, it->value.pVoid );
	}
}
DaoTypeBase DaoxPathCache_Typer =
{
	"PathCache", NULL, NULL, NULL, { NULL }, { NULL },
	(FuncPtrDel)DaoxPathCache_Delete, DaoxPathCache_GetGCFields
};





static void DaoxColor_FromDaoValues( DaoxColor *self, DaoValue *values[] )
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
	DaoxGradient *grad = DaoxGradient_New( DAOX_GRADIENT_BASE );
	GC_Assign( & self->strokeGradient, grad );
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

static void DaoxBrush_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxBrush *self = (DaoxBrush*) p;
	if( self->strokeGradient ) DList_Append( values, self->strokeGradient );
	if( self->fillGradient ) DList_Append( values, self->fillGradient );
	if( self->texture ) DList_Append( values, self->texture );
	if( self->font ) DList_Append( values, self->font );
	if( remove ){
		self->strokeGradient = NULL;
		self->fillGradient = NULL;
		self->texture = NULL;
		self->font = NULL;
	}
}

DaoTypeBase DaoxBrush_Typer =
{
	"Brush", NULL, NULL, (DaoFuncItem*) DaoxBrushMeths, {NULL}, {NULL},
	(FuncPtrDel)DaoxBrush_Delete, DaoxBrush_GetGCFields
};




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
	DaoxCanvasNode *self = (DaoxCanvasNode*) p;
	if( self->children ) DList_Append( lists, self->children );
	if( self->parent ) DList_Append( values, self->parent );
	if( self->brush ) DList_Append( values, self->brush );
	if( self->path ) DList_Append( values, self->path );
	if( self->mesh ) DList_Append( values, self->mesh );
	if( remove ){
		self->parent = NULL;
		self->brush = NULL;
		self->path = NULL;
		self->mesh = NULL;
	}
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



void DaoxCanvasLine_Set( DaoxCanvasLine *self, float x1, float y1, float x2, float y2 );
void DaoxCanvasRect_Set( DaoxCanvasRect *self, float x1, float y1, float x2, float y2 );
void DaoxCanvasCircle_Set( DaoxCanvasCircle *self, float x, float y, float r );
void DaoxCanvasEllipse_Set( DaoxCanvasEllipse *self, float x, float y, float rx, float ry );

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
	DaoxCanvas *self = DaoxCanvas_New( NULL );
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
	float w = p[4]->xFloat.value;
	DaoxCanvasImage *item = DaoxCanvas_AddImage( self, image, x, y, w );
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

	{ CANVAS_AddPath,      "AddPath( self: Canvas, path: Path ) => CanvasPath" },
	{ CANVAS_AddText,      "AddText( self: Canvas, text: string, x: float, y: float, degrees = 0.0 ) => CanvasText" },

	{ CANVAS_AddText2,     "AddText( self: Canvas, text: string, path: Path, degrees = 0.0 ) => CanvasText" },

	{ CANVAS_AddImage,     "AddImage( self: Canvas, image: Image, x: float, y: float, w: float ) => CanvasImage" },
	{ NULL, NULL }
};

extern DaoTypeBase DaoxSceneNode_Typer;
static void DaoxSceneNode_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove );

static void DaoxCanvas_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxCanvas *self = (DaoxCanvas*) p;
	DaoxSceneNode_GetGCFields( p, values, lists, maps, remove );
	DList_Append( lists, self->nodes );
	DList_Append( lists, self->brushes );
	DList_Append( values, self->auxPath );
	DList_Append( values, self->pathCache );
	if( remove ) self->auxPath = NULL;
	if( remove ) self->pathCache = NULL;
}

DaoTypeBase DaoxCanvas_Typer =
{
	"Canvas", NULL, NULL, (DaoFuncItem*) DaoxCanvasMeths,
	{ & DaoxSceneNode_Typer, NULL }, {NULL},
	(FuncPtrDel)DaoxCanvas_Delete, DaoxCanvas_GetGCFields
};






static void MeshUnit_SetMaterial( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxMeshUnit *self = (DaoxMeshUnit*) p[0];
	DaoxMaterial *mat = (DaoxMaterial*) p[1];
	DaoxMeshUnit_SetMaterial( self, mat );
}

static DaoFuncItem DaoxMeshUnitMeths[]=
{
	{ MeshUnit_SetMaterial,
		"SetMaterial( self: MeshUnit, material: Material )"
	},
	{ NULL, NULL }
};
static void DaoxMeshUnit_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxMeshUnit *self = (DaoxMeshUnit*) p;
	if( self->mesh ) DList_Append( values, self->mesh );
	if( self->material ) DList_Append( values, self->material );
	if( remove ){
		self->mesh = NULL;
		self->material = NULL;
	}
}
DaoTypeBase DaoxMeshUnit_Typer =
{
	"MeshUnit", NULL, NULL, (DaoFuncItem*) DaoxMeshUnitMeths, {NULL}, {NULL},
	(FuncPtrDel)DaoxMeshUnit_Delete, DaoxMeshUnit_GetGCFields
};

static DaoFuncItem DaoxMeshMeths[]=
{
	{ NULL, NULL }
};
static void DaoxMesh_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxMesh *self = (DaoxMesh*) p;
	DList_Append( lists, self->units );
}
DaoTypeBase DaoxMesh_Typer =
{
	"Mesh", NULL, NULL, (DaoFuncItem*) DaoxMeshMeths, {NULL}, {NULL},
	(FuncPtrDel)DaoxMesh_Delete, DaoxMesh_GetGCFields
};


static void TEX_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTexture *self = DaoxTexture_New();
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void TEX_SetImage( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTexture *self = (DaoxTexture*) p[0];
	DaoxImage *image = (DaoxImage*) p[1];
	DaoxTexture_SetImage( self, image );
}

static DaoFuncItem DaoxTextureMeths[]=
{
	{ TEX_New,
		"Texture()"
	},
	{ TEX_SetImage,
		"SetImage( self: Texture, image: Image )"
	},
	{ NULL, NULL }
};

static void DaoxTexture_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxTexture *self = (DaoxTexture*) p;
	if( self->image ) DList_Append( values, self->image );
	if( remove ) self->image = NULL;
}
DaoTypeBase DaoxTexture_Typer =
{
	"Texture", NULL, NULL, (DaoFuncItem*) DaoxTextureMeths, {NULL}, {NULL},
	(FuncPtrDel)DaoxTexture_Delete, DaoxTexture_GetGCFields
};


static void MAT_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxMaterial *self = DaoxMaterial_New();
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void MAT_SetColor( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxMaterial *self = (DaoxMaterial*) p[0];
	int i;
	for(i=1; i<N; ++i){
		DaoTuple *param = (DaoTuple*) p[i];
		DaoTuple *value = (DaoTuple*) param->values[1];
		DaoxColor color = { 0.0, 0.0, 0.0, 1.0 };
		color.red   = value->values[0]->xFloat.value;
		color.green = value->values[1]->xFloat.value;
		color.blue  = value->values[2]->xFloat.value;
		switch( param->values[0]->xEnum.value ){
		case 0: self->ambient  = color; break;
		case 1: self->diffuse  = color; break;
		case 2: self->specular = color; break;
		case 3: self->emission = color; break;
		}
	}
}
static void MAT_SetTexture( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxMaterial *self = (DaoxMaterial*) p[0];
	DaoxTexture *texture = (DaoxTexture*) p[1];
	DaoxMaterial_SetTexture( self, texture, p[2]->xEnum.value + 1 );
}
static DaoFuncItem DaoxMaterialMeths[]=
{
	{ MAT_New,
		"Material()"
	},
	{ MAT_SetColor,
		"SetColor( self: Material, ...: tuple<enum<ambient,diffuse,specular,emission>,tuple<float,float,float>> )"
	},
	{ MAT_SetTexture,
		"SetTexture( self: Material, texture: Texture, which: enum<first,second> = $first )"
	},
	{ NULL, NULL }
};

static void DaoxMaterial_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxMaterial *self = (DaoxMaterial*) p;
	if( self->texture1 ) DList_Append( values, self->texture1 );
	if( self->texture2 ) DList_Append( values, self->texture2 );
	if( remove ){
		self->texture1 = NULL;
		self->texture2 = NULL;
	}
}
DaoTypeBase DaoxMaterial_Typer =
{
	"Material", NULL, NULL, (DaoFuncItem*) DaoxMaterialMeths, {NULL}, {NULL},
	(FuncPtrDel)DaoxMaterial_Delete, DaoxMaterial_GetGCFields
};


static void SNODE_Move( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxSceneNode *self = (DaoxSceneNode*) p[0];
	float x = p[1]->xFloat.value;
	float y = p[2]->xFloat.value;
	float z = p[3]->xFloat.value;
	DaoxSceneNode_MoveXYZ( self, x, y, z );
}
static void SNODE_MoveBy( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxSceneNode *self = (DaoxSceneNode*) p[0];
	float dx = p[1]->xFloat.value;
	float dy = p[2]->xFloat.value;
	float dz = p[3]->xFloat.value;
	DaoxSceneNode_MoveByXYZ( self, dx, dy, dz );
}
static void SNODE_Trans( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxSceneNode *self = (DaoxSceneNode*) p[0];
	DaoTuple *res = DaoProcess_PutTuple( proc, 3 );
	res->values[0]->xFloat.value = self->translation.x;
	res->values[1]->xFloat.value = self->translation.y;
	res->values[2]->xFloat.value = self->translation.z;
}
static DaoFuncItem DaoxSceneNodeMeths[]=
{
	{ SNODE_Move,    "Move( self: SceneNode, x: float, y: float, z: float )" },
	{ SNODE_MoveBy,  "MoveBy( self: SceneNode, dx: float, dy: float, dz: float )" },
	{ SNODE_Trans,   ".translation( self: SceneNode ) => tuple<x:float,y:float,z:float>" },
	{ NULL, NULL }
};
static void DaoxSceneNode_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxSceneNode *self = (DaoxSceneNode*) p;
	DList_Append( lists, self->children );
	if( self->parent ) DList_Append( values, self->parent );
	if( remove ){
		self->parent = NULL;
	}
}
DaoTypeBase DaoxSceneNode_Typer =
{
	"SceneNode", NULL, NULL, (DaoFuncItem*) DaoxSceneNodeMeths, {NULL}, {NULL},
	(FuncPtrDel)DaoxSceneNode_Delete, DaoxSceneNode_GetGCFields
};


static void CAM_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCamera *self = DaoxCamera_New();
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void CAM_Move( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCamera *self = (DaoxCamera*) p[0];
	float x = p[1]->xFloat.value;
	float y = p[2]->xFloat.value;
	float z = p[3]->xFloat.value;
	DaoxCamera_MoveXYZ( self, x, y, z );
}
static void CAM_MoveBy( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCamera *self = (DaoxCamera*) p[0];
	float dx = p[1]->xFloat.value;
	float dy = p[2]->xFloat.value;
	float dz = p[3]->xFloat.value;
	DaoxCamera_MoveByXYZ( self, dx, dy, dz );
}
static void CAM_LookAt( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCamera *self = (DaoxCamera*) p[0];
	float x = p[1]->xFloat.value;
	float y = p[2]->xFloat.value;
	float z = p[3]->xFloat.value;
	DaoxCamera_LookAtXYZ( self, x, y, z );
}
static void CAM_Rotate( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCamera *self = (DaoxCamera*) p[0];
	float angle = p[1]->xFloat.value;
	DaoxCamera_RotateBy( self, angle );
}
static void CAM_Orient( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCamera *self = (DaoxCamera*) p[0];
	int dir = p[1]->xEnum.value + 1;
	int rev = p[2]->xBoolean.value;
	DaoxCamera_Orient( self, rev ? -dir : dir );
}
static void CAM_SetFOV( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCamera *self = (DaoxCamera*) p[0];
	float angle = p[1]->xFloat.value;
	self->fovAngle = angle;
}
static void CAM_SetNearPlane( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCamera *self = (DaoxCamera*) p[0];
	float dist = p[1]->xFloat.value;
	self->nearPlane = dist;
}
static void CAM_SetFarPlane( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCamera *self = (DaoxCamera*) p[0];
	float dist = p[1]->xFloat.value;
	self->farPlane = dist;
}
static void CAM_FocusPos( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCamera *self = (DaoxCamera*) p[0];
	DaoTuple *res = DaoProcess_PutTuple( proc, 3 );
	res->values[0]->xFloat.value = self->viewTarget.x;
	res->values[1]->xFloat.value = self->viewTarget.y;
	res->values[2]->xFloat.value = self->viewTarget.z;
}
static void CAM_FOV( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCamera *self = (DaoxCamera*) p[0];
	DaoProcess_PutFloat( proc, self->fovAngle );
}
static DaoFuncItem DaoxCameraMeths[]=
{
	{ CAM_New,     "Camera()" },
	{ CAM_Move,    "Move( self: Camera, x: float, y: float, z: float )" },
	{ CAM_MoveBy,  "MoveBy( self: Camera, dx: float, dy: float, dz: float )" },
	{ CAM_LookAt,  "LookAt( self: Camera, x: float, y: float, z: float )" },
	{ CAM_Rotate,  "Rotate( self: Camera, angle: float )" },
	{ CAM_Orient,  "Orient( self: Camera, worldUpAxis: enum<X,Y,Z> = $Z, reverse = false )" },
	{ CAM_SetFOV,  "SetFOV( self: Camera, angle: float )" },
	{ CAM_SetNearPlane,  "SetNearPlane( self: Camera, dist: float )" },
	{ CAM_SetFarPlane,   "SetFarPlane( self: Camera, dist: float )" },
	{ CAM_FocusPos, ".focus( self: Camera ) => tuple<x:float,y:float,z:float>" },
	{ CAM_FOV, ".fov( self: Camera ) => float" },
	{ NULL, NULL }
};
DaoTypeBase DaoxCamera_Typer =
{
	"Camera", NULL, NULL, (DaoFuncItem*) DaoxCameraMeths,
	{ & DaoxSceneNode_Typer, NULL }, {NULL},
	(FuncPtrDel)DaoxCamera_Delete, DaoxSceneNode_GetGCFields
};



static void LIGHT_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxLight *self = DaoxLight_New();
	self->lightType = p[0]->xEnum.value;
	self->intensity.red = p[1]->xFloat.value;
	self->intensity.green = p[2]->xFloat.value;
	self->intensity.blue = p[3]->xFloat.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static DaoFuncItem DaoxLightMeths[]=
{
	{ LIGHT_New,
		"Light( litype: enum<ambient,point,directional,spot>, red =1.0, green =1.0, blue =1.0 )"
	},
	{ NULL, NULL }
};
DaoTypeBase DaoxLight_Typer =
{
	"Light", NULL, NULL, (DaoFuncItem*) DaoxLightMeths,
	{ & DaoxSceneNode_Typer, NULL }, {NULL},
	(FuncPtrDel)DaoxLight_Delete, DaoxSceneNode_GetGCFields
};

static void MOD_SetMaterial( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxModel *self = (DaoxModel*) p[0];
	DaoxMaterial *mat = (DaoxMaterial*) p[1];
	DaoxMesh_SetMaterial( self->mesh, mat );
}
static DaoFuncItem DaoxModelMeths[]=
{
	{ MOD_SetMaterial,
		"SetMaterial( self: Model, material: Material )"
	},
	{ NULL, NULL }
};
static void DaoxModel_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxModel *self = (DaoxModel*) p;
	DaoxSceneNode_GetGCFields( p, values, lists, maps, remove );
	if( self->mesh ) DList_Append( values, self->mesh );
	if( remove ){
		self->mesh = NULL;
	}
}
DaoTypeBase DaoxModel_Typer =
{
	"Model", NULL, NULL, (DaoFuncItem*) DaoxModelMeths,
	{ & DaoxSceneNode_Typer, NULL }, {NULL},
	(FuncPtrDel)DaoxModel_Delete, DaoxModel_GetGCFields
};




static void TerrainBlock_SetMaterial( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrainBlock *self = (DaoxTerrainBlock*) p[0];
	DaoxMaterial *mat = (DaoxMaterial*) p[1];
	DaoxMeshUnit_SetMaterial( self->mesh, mat );
}

static DaoFuncItem DaoxTerrainBlockMeths[]=
{
	{ TerrainBlock_SetMaterial,
		"SetMaterial( self: TerrainBlock, material: Material )"
	},
	{ NULL, NULL }
};
DaoTypeBase DaoxTerrainBlock_Typer =
{
	"TerrainBlock", NULL, NULL, (DaoFuncItem*) DaoxTerrainBlockMeths, {NULL}, {NULL},
	(FuncPtrDel)DaoxTerrainBlock_Delete, NULL
};




static void Terrain_GetBlock( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrain *self = (DaoxTerrain*) p[0];
	int side = p[1]->xInteger.value;
	int radius = p[2]->xInteger.value;
	int offset = p[3]->xInteger.value;
	DaoxTerrainBlock *unit = DaoxTerrain_GetBlock( self, side, radius, offset );

	if( unit == NULL ){
		DaoProcess_RaiseError( proc, "Index", "out of range" );
		return;
	}
	DaoProcess_PutValue( proc, (DaoValue*) unit );
}
static void Terrain_SetTileType( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrain *self = (DaoxTerrain*) p[0];
	int side = p[1]->xInteger.value;
	int radius = p[2]->xInteger.value;
	int offset = p[3]->xInteger.value;
	DaoxTerrainBlock *unit = DaoxTerrain_GetBlock( self, side, radius, offset );
	//unit->type = type;
}
static void Terrain_EachBlock( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrain *self = (DaoxTerrain*) p[0];
	DaoVmCode *sect = DaoProcess_InitCodeSection( proc, 1 );
	DaoxTerrainBlock *unit;
	int entry;

	if( sect == NULL ) return;
	entry = proc->topFrame->entry;
	for(unit=self->first; unit!=NULL; unit=unit->next){
		if( sect->b >0 ) DaoProcess_SetValue( proc, sect->a, (DaoValue*) unit );
		proc->topFrame->entry = entry;
		DaoProcess_Execute( proc );
		if( proc->status == DAO_PROCESS_ABORTED ) break;
	}
	DaoProcess_PopFrame( proc );
}

static DaoFuncItem DaoxTerrainMeths[]=
{
	{ Terrain_GetBlock,
		"GetBlock( self: Terrain, side: int, radius: int, offset: int ) => TerrainBlock"
	},
	{ Terrain_EachBlock,
		"EachBlock( self: Terrain ) [block:TerrainBlock]"
	},
	{ NULL, NULL }
};
static void DaoxTerrain_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxTerrain *self = (DaoxTerrain*) p;
	DaoxModel_GetGCFields( p, values, lists, maps, remove );
	DList_Append( lists, self->blocks );
}
DaoTypeBase DaoxTerrain_Typer =
{
	"Terrain", NULL, NULL, (DaoFuncItem*) DaoxTerrainMeths,
	{ & DaoxModel_Typer, NULL }, {NULL},
	(FuncPtrDel)DaoxTerrain_Delete, DaoxTerrain_GetGCFields
};




static void SCENE_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxScene *self = DaoxScene_New();
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void SCENE_AddNode( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxScene *self = (DaoxScene*) p[0];
	DaoxSceneNode *node = (DaoxSceneNode*) p[1];
	DaoxScene_AddNode( self, node );
}
static void SCENE_AddBox( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxMatrix4D transform = DaoxMatrix4D_Identity();
	DaoxScene *self = (DaoxScene*) p[0];
	DaoxModel *model = DaoxModel_New();
	DaoxMesh *mesh = DaoxMesh_New();
	DaoxMesh_MakeBoxObject( mesh );
	DaoxMesh_UpdateTree( mesh, 0 );
	DaoxModel_SetMesh( model, mesh );
	model->base.scale.x = p[1]->xFloat.value;
	model->base.scale.y = p[2]->xFloat.value;
	model->base.scale.z = p[3]->xFloat.value;
	DaoxSceneNode_MoveXYZ( (DaoxSceneNode*) model, 0, 0, 0 );
	DaoxScene_AddNode( self, (DaoxSceneNode*) model );
	DaoProcess_PutValue( proc, (DaoValue*) model );
}
static void SCENE_AddRectTerrain( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrain *terrain = DaoxTerrain_New();
	DaoxScene *self = (DaoxScene*) p[0];
	DaoArray *heightmap = (DaoArray*) p[1];
	float width = p[2]->xFloat.value;
	float length = p[3]->xFloat.value;

	if( heightmap->type != DAO_ARRAY ){
		DaoxImage *image = (DaoxImage*) p[1];
		float height = p[4]->xFloat.value;
		heightmap = DaoArray_New(DAO_FLOAT);
		DaoxImage_Export( image, heightmap, height / 255.0 );
	}

	DaoxTerrain_SetHeightmap( terrain, heightmap );
	DaoxTerrain_SetRectAutoBlocks( terrain, width, length );
	DaoxTerrain_Rebuild( terrain );
	DaoxScene_AddNode( self, (DaoxSceneNode*) terrain );
	DaoProcess_PutValue( proc, (DaoValue*) terrain );
}
static void SCENE_AddHexTerrain( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrain *terrain = DaoxTerrain_New();
	DaoxScene *self = (DaoxScene*) p[0];
	DaoArray *heightmap = (DaoArray*) p[1];
	int circles = p[2]->xInteger.value;
	float radius = p[3]->xFloat.value;

	if( heightmap->type != DAO_ARRAY ){
		DaoxImage *image = (DaoxImage*) p[1];
		float height = p[4]->xFloat.value;
		heightmap = DaoArray_New(DAO_FLOAT);
		DaoxImage_Export( image, heightmap, height / 255.0 );
	}

	DaoxTerrain_SetHeightmap( terrain, heightmap );
	DaoxTerrain_SetHexBlocks( terrain, circles, radius );
	DaoxTerrain_Rebuild( terrain );
	DaoxScene_AddNode( self, (DaoxSceneNode*) terrain );
	DaoProcess_PutValue( proc, (DaoValue*) terrain );
}
static DaoFuncItem DaoxSceneMeths[] =
{
	{ SCENE_New,         "Scene()" },
	{ SCENE_AddNode,     "AddNode( self: Scene, node: SceneNode )" },
	{ SCENE_AddBox,      "AddBox( self: Scene, xlen = 1.0, ylen = 1.0, zlen = 1.0 ) => Model" },
	{ SCENE_AddRectTerrain,
		"AddRectTerrain( self: Scene, heightmap: array<float>, width = 1.0, length = 1.0 )"
			"=> Terrain"
	},
	{ SCENE_AddRectTerrain,
		"AddRectTerrain( self: Scene, heightmap: Image, width = 1.0, length = 1.0, height = 1.0 )"
			"=> Terrain"
	},
	{ SCENE_AddHexTerrain,
		"AddHexTerrain( self: Scene, heightmap: array<float>, circles = 2, radius = 1.0 )"
			"=> Terrain"
	},
	{ SCENE_AddHexTerrain,
		"AddHexTerrain( self: Scene, heightmap: Image, circles = 1, radius = 1.0, height = 1.0 )"
			"=> Terrain"
	},
	{ NULL, NULL }
};
static void DaoxScene_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxScene *self = (DaoxScene*) p;
	DList_Append( lists, self->nodes );
	if( self->pathCache ) DList_Append( values, self->pathCache );
	if( remove ) self->pathCache = NULL;
}
DaoTypeBase DaoxScene_Typer =
{
	"Scene", NULL, NULL, (DaoFuncItem*) DaoxSceneMeths, {NULL}, {NULL},
	(FuncPtrDel)DaoxScene_Delete, DaoxScene_GetGCFields
};



static void PAINTER_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxContext *ctx = (DaoxContext*) DaoValue_CastCstruct( p[0], daox_type_context );
	DaoxPainter *self = DaoxPainter_New( ctx );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void PAINTER_RenderToImage( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPainter *self = (DaoxPainter*) p[0];
	DaoxCanvas *canvas = (DaoxCanvas*) p[1];
	DaoxImage *image = (DaoxImage*) p[2];
	int width = p[3]->xInteger.value;
	int height = p[4]->xInteger.value;
	DaoxPainter_PaintCanvasImage( self, canvas, canvas->viewport, image, width, height );
}
static void PAINTER_Paint( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPainter *self = (DaoxPainter*) p[0];
	DaoxCanvas *canvas = (DaoxCanvas*) p[1];
	DaoxPainter_Paint( self, canvas, canvas->viewport );
}
static DaoFuncItem DaoxPainterMeths[]=
{
	{ PAINTER_New,            "Painter( contex: Context )" },
	{ PAINTER_RenderToImage,  "RenderToImage( self: Painter, canvas: Canvas, image: Image, width: int, height: int )" },
	{ PAINTER_Paint,  "Paint( self: Painter, canvas: Canvas )" },
	{ NULL, NULL }
};
static void DaoxPainter_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxPainter *self = (DaoxPainter*) p;
	DList_Append( values, self->shader );
	DList_Append( values, self->buffer );
	DList_Append( values, self->context );
	if( remove ){
		self->shader = NULL;
		self->buffer = NULL;
		self->context = NULL;
	}
}
DaoTypeBase DaoxPainter_Typer =
{
	"Painter", NULL, NULL, (DaoFuncItem*) DaoxPainterMeths, {0}, {0},
	(FuncPtrDel)DaoxPainter_Delete, DaoxPainter_GetGCFields
};



static void RENDR_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxContext *ctx = (DaoxContext*) DaoValue_CastCstruct( p[0], daox_type_context );
	DaoxRenderer *self = DaoxRenderer_New( ctx );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void RENDR_SetCurrentCamera( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxRenderer *self = (DaoxRenderer*) p[0];
	DaoxCamera *cam = (DaoxCamera*) p[1];
	DaoxRenderer_SetCurrentCamera( self, cam );
}
static void RENDR_GetCurrentCamera( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxRenderer *self = (DaoxRenderer*) p[0];
	DaoxCamera *cam = DaoxRenderer_GetCurrentCamera( self );
	DaoProcess_PutValue( proc, (DaoValue*) cam );
}
static void RENDR_Enable( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxRenderer *self = (DaoxRenderer*) p[0];
	int bl = p[2]->xBoolean.value;
	switch( p[1]->xEnum.value ){
	case 0 : self->showAxis = bl; break;
	case 1 : self->showMesh = bl; break;
	}
}
static void RENDR_Render( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxRenderer *self = (DaoxRenderer*) p[0];
	DaoxScene *scene = (DaoxScene*) p[1];
	DaoxRenderer_Render( self, scene, NULL );
}
static DaoFuncItem DaoxRendererMeths[]=
{
	{ RENDR_New,         "Renderer( contex: Context )" },
	{ RENDR_SetCurrentCamera,  "SetCurrentCamera( self: Renderer, camera: Camera )" },
	{ RENDR_GetCurrentCamera,  "GetCurrentCamera( self: Renderer ) => Camera" },
	{ RENDR_Enable,  "Enable( self: Renderer, what: enum<axis,mesh>, bl = true )" },
	{ RENDR_Render,  "Render( self: Renderer, scene: Scene )" },
	{ NULL, NULL }
};
static void DaoxRenderer_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxRenderer *self = (DaoxRenderer*) p;
	if( self->scene ) DList_Append( values, self->scene );
	if( self->camera ) DList_Append( values, self->camera );
	DList_Append( values, self->shader );
	DList_Append( values, self->buffer );
	DList_Append( values, self->bufferVG );
	DList_Append( values, self->context );
	DList_Append( values, self->axisMesh );
	DList_Append( values, self->worldAxis );
	DList_Append( values, self->localAxis );
	if( remove ){
		self->scene = NULL;
		self->camera = NULL;
		self->shader = NULL;
		self->buffer = NULL;
		self->bufferVG = NULL;
		self->context = NULL;
		self->axisMesh = NULL;
		self->worldAxis = NULL;
		self->localAxis = NULL;
	}
}
DaoTypeBase DaoxRenderer_Typer =
{
	"Renderer", NULL, NULL, (DaoFuncItem*) DaoxRendererMeths, {0}, {0},
	(FuncPtrDel)DaoxRenderer_Delete, DaoxRenderer_GetGCFields
};



static void RES_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxResource *self = DaoxResource_New( proc->vmSpace );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void RES_LoadObjFile( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxResource *self = (DaoxResource*) p[0];
	DString *file = p[1]->xString.value;
	DString *codePath = proc->activeRoutine->nameSpace->path;
	DaoxScene *scene = DaoxResource_LoadObjFile( self, file, codePath );
	DaoProcess_PutValue( proc, (DaoValue*) scene );
}
static DaoFuncItem DaoxResourceMeths[]=
{
	{ RES_New,              "Resource()" },
	{ RES_LoadObjFile,      "LoadObjFile( self: Resource, file: string ) => Scene" },
	{ NULL, NULL }
};
static void DaoxResource_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxResource *self = (DaoxResource*) p;
	DList_Append( maps, self->scenes );
	DList_Append( maps, self->lights );
	DList_Append( maps, self->cameras );
	DList_Append( maps, self->images );
	DList_Append( maps, self->textures );
	DList_Append( maps, self->effects );
	DList_Append( maps, self->materials );
	DList_Append( maps, self->geometries );
	DList_Append( maps, self->terrains );
}
DaoTypeBase DaoxResource_Typer =
{
	"Resource", NULL, NULL, (DaoFuncItem*) DaoxResourceMeths, {0}, {0},
	(FuncPtrDel)DaoxResource_Delete, DaoxResource_GetGCFields
};




static void TerrainGenerator_New( DaoProcess *proc, DaoValue *p[], int N )
{
	int shape = p[0]->xEnum.value;
	int circles = p[1]->xInteger.value;
	float radius = p[2]->xFloat.value;

	DaoxTerrainGenerator *self = DaoxTerrainGenerator_New();
	DaoxTerrain_SetHexBlocks( self->terrain, circles, radius );
	DaoxTerrain_InitBlocks( self->terrain );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void TerrainGenerator_New2( DaoProcess *proc, DaoValue *p[], int N )
{
	int shape = p[0]->xEnum.value;
	int rows = p[1]->xInteger.value;
	int cols = p[2]->xInteger.value;
	float radius = p[3]->xFloat.value;

	DaoxTerrainGenerator *self = DaoxTerrainGenerator_New();
	DaoxTerrain_SetRectBlocks( self->terrain, rows, cols, radius );
	DaoxTerrain_InitBlocks( self->terrain );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void TerrainParams_Configure( DaoxTerrainParams *self, DaoValue **p, int N )
{
	int i;
	for(i=0; i<N; ++i){
		DaoTuple *param = (DaoTuple*) p[i];
		switch( param->values[0]->xEnum.value ){
		case 0 : self->faultScale = param->values[1]->xFloat.value; break;
		case 1 : self->amplitude = param->values[1]->xFloat.value; break;
		case 2 : self->resolution = param->values[1]->xFloat.value; break;
		}
	}
}
static void TerrainGenerator_Configure( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrainGenerator *self = (DaoxTerrainGenerator*) p[0];
	TerrainParams_Configure( & self->params, p+1, N-1 );
}
DaoxTerrainParams* DaoxTerrainParams_New();
static void TerrainGenerator_Configure2( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrainGenerator *self = (DaoxTerrainGenerator*) p[0];
	int side = p[1]->xInteger.value;
	int radius = p[2]->xInteger.value;
	int offset = p[3]->xInteger.value;
	DaoxTerrainBlock *unit = DaoxTerrain_GetBlock( self->terrain, side, radius, offset );

	if( unit == NULL ){
		DaoProcess_RaiseError( proc, "Index", "out of range" );
		return;
	}
	if( unit->params == NULL ) unit->params = DaoxTerrainParams_New();
	TerrainParams_Configure( unit->params, p+4, N-4 );
}
static void TerrainGenerator_Generate( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrainGenerator *self = (DaoxTerrainGenerator*) p[0];
	int iterations = p[1]->xInteger.value;
	int seed = p[2]->xInteger.value;

	DaoxTerrainGenerator_Generate( self, iterations, seed );
}
static void TerrainGenerator_Update( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrainGenerator *self = (DaoxTerrainGenerator*) p[0];
	int iterations = p[1]->xInteger.value;

	DaoxTerrainGenerator_Update( self, iterations );
}
static void TerrainGenerator_GetTerrain( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrainGenerator *self = (DaoxTerrainGenerator*) p[0];
	DaoProcess_PutValue( proc, (DaoValue*) self->terrain );
}
static void TerrainGenerator_ExportRectTerrain( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrainGenerator *self = (DaoxTerrainGenerator*) p[0];
	DaoxTerrain *terrain = DaoxTerrain_New();
	int rows = p[1]->xInteger.value;
	int cols = p[2]->xInteger.value;
	float radius = p[3]->xFloat.value;

	DaoxTerrain_SetRectBlocks( terrain, rows, cols, radius );
	DaoxTerrain_Export( self->terrain, terrain );
	DaoProcess_PutValue( proc, (DaoValue*) terrain );
}
static void TerrainGenerator_ExportHexTerrain( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrainGenerator *self = (DaoxTerrainGenerator*) p[0];
	DaoxTerrain *terrain = DaoxTerrain_New();
	int circles = p[1]->xInteger.value;
	float radius = p[2]->xFloat.value;

	DaoxTerrain_SetHexBlocks( terrain, circles, radius );
	DaoxTerrain_Export( self->terrain, terrain );
	DaoProcess_PutValue( proc, (DaoValue*) terrain );
}

static DaoFuncItem DaoxTerrainGeneratorMeths[]=
{
	{ TerrainGenerator_New,
		"TerrainGenerator( shape: enum<hexagon>, circles = 2, radius = 1.0 )"
	},
	{ TerrainGenerator_New2,
		"TerrainGenerator( shape: enum<rectangle>, rows = 2, columns = 2, blocksize = 1.0 )"
	},
	{ TerrainGenerator_Configure,
		"Configure( self: TerrainGenerator, ... : tuple<enum<scale,amplitude,resolution>,float> )"
	},
	{ TerrainGenerator_Configure2,
		"ConfigureBlock( self: TerrainGenerator, side: int, dist: int, offset: int,"
			"... : tuple<enum<scale,amplitude,resolution>,float> )"
	},
	{ TerrainGenerator_Generate,
		"Generate( self: TerrainGenerator, iterations = 20, seed = 0 )"
	},
	{ TerrainGenerator_Update,
		"Update( self: TerrainGenerator, iterations = 20 )"
	},
	{ TerrainGenerator_GetTerrain,
		"GetTerrain( self: TerrainGenerator ) => Terrain"
	},
	{ TerrainGenerator_ExportRectTerrain,
		"ExportRectTerrain( self: TerrainGenerator, rows: int, columns: int, blocksize: float ) => Terrain"
	},
	{ TerrainGenerator_ExportHexTerrain,
		"ExportHexTerrain( self: TerrainGenerator, circles: int, radius: float ) => Terrain"
	},
	{ NULL, NULL }
};
static void DaoxTG_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxTerrainGenerator *self = (DaoxTerrainGenerator*) p;
	DList_Append( values, self->terrain );
	if( remove ) self->terrain = NULL;
}
DaoTypeBase DaoxTerrainGenerator_Typer =
{
	"TerrainGenerator", NULL, NULL, (DaoFuncItem*) DaoxTerrainGeneratorMeths,
	{ NULL }, { NULL }, (FuncPtrDel)DaoxTerrainGenerator_Delete, DaoxTG_GetGCFields
};



static DaoFuncItem DaoxShaderMeths[]=
{
	{ NULL, NULL }
};

DaoTypeBase DaoxShader_Typer =
{
	"Shader", NULL, NULL, (DaoFuncItem*) DaoxShaderMeths, { NULL }, { NULL },
	(FuncPtrDel)DaoxShader_Delete, NULL
};



static DaoFuncItem DaoxBufferMeths[]=
{
	{ NULL, NULL }
};

DaoTypeBase DaoxBuffer_Typer =
{
	"Buffer", NULL, NULL, (DaoFuncItem*) DaoxBufferMeths, { NULL }, { NULL },
	(FuncPtrDel)DaoxBuffer_Delete, NULL
};



static void CTX_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxContext *self = DaoxContext_New();
	self->deviceWidth  = p[0]->xInteger.value;
	self->deviceHeight = p[1]->xInteger.value;
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void CTX_Quit( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxContext *self = (DaoxContext*) p[0];
	DaoxContext_Clear( self );
}
static DaoFuncItem DaoxContextMeths[]=
{
	{ CTX_New,   "Context( width: int, height: int )" },
	{ CTX_Quit,  "Quit( self: Context )" },
	{ NULL, NULL }
};

static void DaoxContext_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxContext *self = (DaoxContext*) p;
	if( remove && (self->shaders->size + self->buffers->size + self->textures->size) ){
		fprintf( stderr, "WARNING: Automatic deletion of graphics contexts may be unsafe!\n" );
		fprintf( stderr, "-------  Please manually quit them first!\n" );
	}
	DList_Append( lists, self->shaders );
	DList_Append( lists, self->buffers );
	DList_Append( lists, self->textures );
}
DaoTypeBase DaoxContext_Typer =
{
	"Context", NULL, NULL, (DaoFuncItem*) DaoxContextMeths, { NULL }, { NULL },
	(FuncPtrDel)DaoxContext_Delete, DaoxContext_GetGCFields
};



static void GRAPHICS_Backend( DaoProcess *proc, DaoValue *p[], int N )
{
#   ifdef DAO_GRAPHICS_USE_GLES
	DaoProcess_PutEnum( proc, "OpenGLES" );
#   else
	DaoProcess_PutEnum( proc, "OpenGL" );
#   endif
}

static DaoFuncItem globalMeths[]=
{
	{ GRAPHICS_Backend,
		"Backend() => enum<OpenGL,OpenGLES>"
	},
	{ NULL, NULL }
};


DaoType* daox_type_path = NULL;
DaoType *daox_type_path_mesh = NULL;
DaoType *daox_type_path_cache = NULL;

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
DaoType *daox_type_canvas_path = NULL;
DaoType *daox_type_canvas_text = NULL;
DaoType *daox_type_canvas_image = NULL;


DaoType *daox_type_mesh = NULL;
DaoType *daox_type_mesh_unit = NULL;
DaoType *daox_type_texture = NULL;
DaoType *daox_type_material = NULL;
DaoType *daox_type_scene_node = NULL;
DaoType *daox_type_camera = NULL;
DaoType *daox_type_light = NULL;
DaoType *daox_type_model = NULL;
DaoType *daox_type_terrain = NULL;
DaoType *daox_type_scene = NULL;
DaoType *daox_type_painter = NULL;
DaoType *daox_type_renderer = NULL;
DaoType *daox_type_resource = NULL;
DaoType *daox_type_terrain_block = NULL;
DaoType *daox_type_terrain_generator = NULL;

DaoType *daox_type_shader = NULL;
DaoType *daox_type_buffer = NULL;
DaoType *daox_type_context = NULL;


DaoxCamera *camera = NULL;
DaoxLight *light = NULL;
DaoxScene *scene = NULL;
DaoxTerrainGenerator *terrain = NULL;
DaoxRenderer *renderer = NULL;

DAO_DLL void DaoGraphics_InitTest()
{
	scene = DaoxScene_New();
	light = DaoxLight_New();
	camera = DaoxCamera_New();
	DaoxScene_AddNode( scene, (DaoxSceneNode*) light );
	DaoxScene_AddNode( scene, (DaoxSceneNode*) camera );
	light->intensity.red = 1.0;
	light->intensity.green = 1.0;
	light->intensity.blue = 1.0;
	DaoxLight_MoveXYZ( light, 0, 0, 50 );
	DaoxCamera_MoveXYZ( camera, 0, 50, 50 );
	DaoxCamera_LookAtXYZ( camera, 0, 0, 0 );

	terrain = DaoxTerrainGenerator_New();
	DaoxTerrain_SetHexBlocks( terrain->terrain, 1, 10.0 );
	DaoxTerrain_InitBlocks( terrain->terrain );
	DaoxTerrainGenerator_Generate( terrain, 10, 1 );
	DaoxScene_AddNode( scene, (DaoxSceneNode*) terrain->terrain );

	renderer = DaoxRenderer_New( NULL );

	DaoGC_IncRC( (DaoValue*) scene );
	DaoGC_IncRC( (DaoValue*) renderer );
}
DAO_DLL void DaoGraphics_DrawTest()
{
	DaoxRenderer_Render( renderer, scene, camera );
}


DAO_DLL int DaoTriangulator_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );
DAO_DLL int DaoFont_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );
DAO_DLL int DaoImage_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );
DAO_DLL int DaoWindow_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );

DAO_DLL int DaoGraphics_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *nspace )
{
	DaoNamespace *ns;
	dao_vmspace_graphics = vmSpace;
	ns = DaoVmSpace_GetNamespace( vmSpace, "Graphics" );
	DaoNamespace_AddConst( nspace, ns->name, (DaoValue*) ns, DAO_PERM_PUBLIC );
	DaoNamespace_WrapFunctions( ns, globalMeths );

	DaoFont_OnLoad( vmSpace, ns );
	DaoImage_OnLoad( vmSpace, ns );
	DaoTriangulator_OnLoad( vmSpace, ns );

	DaoNamespace_DefineType( ns, "tuple<red:float,green:float,blue:float,alpha:float>", "Color" );

	daox_type_path = DaoNamespace_WrapType( ns, & DaoxPath_Typer, 0 );
	daox_type_path_mesh = DaoNamespace_WrapType( ns, & DaoxPathMesh_Typer, 0 );
	daox_type_path_cache = DaoNamespace_WrapType( ns, & DaoxPathCache_Typer, 0 );

	daox_type_gradient = DaoNamespace_WrapType( ns, & DaoxGradient_Typer, 0 );
	daox_type_linear_gradient = DaoNamespace_WrapType( ns, & DaoxLinearGradient_Typer, 0 );
	daox_type_radial_gradient = DaoNamespace_WrapType( ns, & DaoxRadialGradient_Typer, 0 );
	daox_type_path_gradient = DaoNamespace_WrapType( ns, & DaoxPathGradient_Typer, 0 );

	daox_type_brush = DaoNamespace_WrapType( ns, & DaoxBrush_Typer, 0 );
	daox_type_canvas_node = DaoNamespace_WrapType( ns, & DaoxCanvasNode_Typer, 0 );
	daox_type_canvas_line = DaoNamespace_WrapType( ns, & DaoxCanvasLine_Typer, 0 );
	daox_type_canvas_rect = DaoNamespace_WrapType( ns, & DaoxCanvasRect_Typer, 0 );
	daox_type_canvas_circle = DaoNamespace_WrapType( ns, & DaoxCanvasCircle_Typer, 0 );
	daox_type_canvas_ellipse = DaoNamespace_WrapType( ns, & DaoxCanvasEllipse_Typer, 0 );
	daox_type_canvas_path = DaoNamespace_WrapType( ns, & DaoxCanvasPath_Typer, 0 );
	daox_type_canvas_text = DaoNamespace_WrapType( ns, & DaoxCanvasText_Typer, 0 );
	daox_type_canvas_image = DaoNamespace_WrapType( ns, & DaoxCanvasImage_Typer, 0 );

	daox_type_mesh_unit = DaoNamespace_WrapType( ns, & DaoxMeshUnit_Typer, 0 );
	daox_type_mesh = DaoNamespace_WrapType( ns, & DaoxMesh_Typer, 0 );
	daox_type_texture = DaoNamespace_WrapType( ns, & DaoxTexture_Typer, 0 );
	daox_type_material = DaoNamespace_WrapType( ns, & DaoxMaterial_Typer, 0 );
	daox_type_scene_node = DaoNamespace_WrapType( ns, & DaoxSceneNode_Typer, 0 );
	daox_type_camera = DaoNamespace_WrapType( ns, & DaoxCamera_Typer, 0 );
	daox_type_light = DaoNamespace_WrapType( ns, & DaoxLight_Typer, 0 );
	daox_type_model = DaoNamespace_WrapType( ns, & DaoxModel_Typer, 0 );
	daox_type_terrain = DaoNamespace_WrapType( ns, & DaoxTerrain_Typer, 0 );

	daox_type_canvas = DaoNamespace_WrapType( ns, & DaoxCanvas_Typer, 0 );
	daox_type_scene = DaoNamespace_WrapType( ns, & DaoxScene_Typer, 0 );
	daox_type_painter = DaoNamespace_WrapType( ns, & DaoxPainter_Typer, 0 );
	daox_type_renderer = DaoNamespace_WrapType( ns, & DaoxRenderer_Typer, 0 );
	daox_type_resource = DaoNamespace_WrapType( ns, & DaoxResource_Typer, 0 );
	daox_type_terrain_block = DaoNamespace_WrapType( ns, & DaoxTerrainBlock_Typer, 0 );
	daox_type_terrain_generator = DaoNamespace_WrapType( ns, & DaoxTerrainGenerator_Typer, 0 );

	daox_type_shader = DaoNamespace_WrapType( ns, & DaoxShader_Typer, 0 );
	daox_type_buffer = DaoNamespace_WrapType( ns, & DaoxBuffer_Typer, 0 );
	daox_type_context = DaoNamespace_WrapType( ns, & DaoxContext_Typer, 0 );

	DaoWindow_OnLoad( vmSpace, ns );
	return 0;
}

