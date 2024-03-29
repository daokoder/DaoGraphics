/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2013-2016, Limin Fu
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
#include "dao_particle.h"


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

static DaoFunctionEntry DaoxPathMeths[]=
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

DaoTypeCore daoPathCore =
{
	"Path",                                            /* name */
	sizeof(DaoxPath),                                  /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxPathMeths,                                     /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* GetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxPath_Delete,               /* Delete */
	NULL                                               /* HandleGC */
};


static void DaoxPathMesh_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxPathMesh *self = (DaoxPathMesh*) p;
	if( self->path ) DList_Append( values, self->path );
	if( remove ) self->path = NULL;
}

DaoTypeCore daoPathMeshCore =
{
	"PathMesh",                                        /* name */
	sizeof(DaoxPathMesh),                              /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	NULL,                                              /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* GetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxPathMesh_Delete,           /* Delete */
	DaoxPathMesh_HandleGC                              /* HandleGC */
};


static void DaoxPathCache_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
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

DaoTypeCore daoPathCacheCore =
{
	"PathCache",                                       /* name */
	sizeof(DaoxPathCache),                             /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	NULL,                                              /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* GetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxPathCache_Delete,          /* Delete */
	DaoxPathCache_HandleGC                             /* HandleGC */
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

static DaoFunctionEntry DaoxGradientMeths[]=
{
	{ GRAD_AddStop,  "AddStop( self: ColorGradient, at: float, color: Color ) => ColorGradient" },
	{ GRAD_AddStops,  "AddStops( self: ColorGradient, stops: list<tuple<at:float,color:Color>> ) => ColorGradient" },
	{ NULL, NULL }
};


DaoTypeCore daoGradientCore =
{
	"ColorGradient",                                   /* name */
	sizeof(DaoxGradient),                              /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxGradientMeths,                                 /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* GetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxGradient_Delete,           /* Delete */
	NULL                                               /* HandleGC */
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

static DaoFunctionEntry DaoxLinearGradientMeths[]=
{
	{ LGRAD_SetStart,  "SetStart( self: LinearGradient, x: float, y: float )" },
	{ LGRAD_SetEnd,    "SetEnd( self: LinearGradient, x: float, y: float )" },
	{ NULL, NULL }
};


DaoTypeCore daoLinearGradientCore =
{
	"LinearGradient",                                  /* name */
	sizeof(DaoxGradient),                              /* size */
	{ & daoGradientCore, NULL },                       /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxLinearGradientMeths,                           /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* GetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxGradient_Delete,           /* Delete */
	NULL                                               /* HandleGC */
};


static void RGRAD_SetRadius( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxGradient *self = (DaoxGradient*) p[0];
	self->radius = p[1]->xFloat.value;
}

static DaoFunctionEntry DaoxRadialGradientMeths[]=
{
	{ LGRAD_SetStart,  "SetCenter( self: RadialGradient, x: float, y: float )" },
	{ LGRAD_SetEnd,    "SetFocus( self: RadialGradient, x: float, y: float )" },
	{ RGRAD_SetRadius, "SetRadius( self: RadialGradient, r: float )" },
	{ NULL, NULL }
};


DaoTypeCore daoRadialGradientCore =
{
	"RadialGradient",                                  /* name */
	sizeof(DaoxGradient),                              /* size */
	{ & daoGradientCore, NULL },                       /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxRadialGradientMeths,                           /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* GetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxGradient_Delete,           /* Delete */
	NULL                                               /* HandleGC */
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
static void BRUSH_SetFont( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxBrush *self = (DaoxBrush*) p[0];
	DaoxFont *font = (DaoxFont*) p[1];
	DaoxBrush_SetFont( self, font, p[2]->xFloat.value );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}

static DaoFunctionEntry DaoxBrushMeths[]=
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

	{ BRUSH_SetFont,      "SetFont( self: Brush, font: Font, size = 12.0 ) => Brush" },
	{ NULL, NULL }
};

static void DaoxBrush_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
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


DaoTypeCore daoBrushCore =
{
	"Brush",                                           /* name */
	sizeof(DaoxBrush),                                 /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxBrushMeths,                                    /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxBrush_Delete,              /* Delete */
	DaoxBrush_HandleGC                                 /* HandleGC */
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

static void DaoxCanvasNode_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
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

static DaoFunctionEntry DaoxCanvasNodeMeths[]=
{
	{ ITEM_SetVisible,
		"SetVisible( self: CanvasNode, visible = true )"
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


DaoTypeCore daoCanvasNodeCore =
{
	"CanvasNode",                                      /* name */
	sizeof(DaoxCanvasNode),                            /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxCanvasNodeMeths,                               /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxCanvasNode_Delete,         /* Delete */
	DaoxCanvasNode_HandleGC                            /* HandleGC */
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
static DaoFunctionEntry DaoxCanvasLineMeths[]=
{
	{ LINE_Set, "Set( self: CanvasLine, x1 = 0.0, y1 = 0.0, x2 = 1.0, y2 = 1.0 ) => CanvasLine" },
	{ NULL, NULL }
};


DaoTypeCore daoCanvasLineCore =
{
	"CanvasLine",                                      /* name */
	sizeof(DaoxCanvasLine),                            /* size */
	{ & daoCanvasNodeCore, NULL },                     /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxCanvasLineMeths,                               /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxCanvasNode_Delete,         /* Delete */
	DaoxCanvasNode_HandleGC                            /* HandleGC */
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
static DaoFunctionEntry DaoxCanvasRectMeths[]=
{
	//{ RECT_Set,   "Set( self: CanvasRect, x1 = 0.0, y1 = 0.0, x2 = 1.0, y2 = 1.0 ) => CanvasRect" },
	{ NULL, NULL }
};


DaoTypeCore daoCanvasRectCore =
{
	"CanvasRect",                                      /* name */
	sizeof(DaoxCanvasRect),                            /* size */
	{ & daoCanvasNodeCore, NULL },                     /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxCanvasRectMeths,                               /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxCanvasNode_Delete,         /* Delete */
	DaoxCanvasNode_HandleGC                            /* HandleGC */
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
static DaoFunctionEntry DaoxCanvasCircleMeths[]=
{
	{ CIRCLE_Set,  "Set( self: CanvasCircle, cx = 0.0, cy = 0.0, r = 1.0 ) => CanvasCircle" },
	{ NULL, NULL }
};


DaoTypeCore daoCanvasCircleCore =
{
	"CanvasCircle",                                    /* name */
	sizeof(DaoxCanvasCircle),                          /* size */
	{ & daoCanvasNodeCore, NULL },                     /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxCanvasCircleMeths,                             /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxCanvasNode_Delete,         /* Delete */
	DaoxCanvasNode_HandleGC                            /* HandleGC */
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
static DaoFunctionEntry DaoxCanvasEllipseMeths[]=
{
	{ ELLIPSE_Set,  "Set( self: CanvasEllipse, cx = 0.0, cy = 0.0, rx = 1.0, ry = 1.0 ) => CanvasEllipse" },
	{ NULL, NULL }
};


DaoTypeCore daoCanvasEllipseCore =
{
	"CanvasEllipse",                                   /* name */
	sizeof(DaoxCanvasEllipse),                         /* size */
	{ & daoCanvasNodeCore, NULL },                     /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxCanvasEllipseMeths,                            /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxCanvasNode_Delete,         /* Delete */
	DaoxCanvasNode_HandleGC                            /* HandleGC */
};




static DaoFunctionEntry DaoxCanvasPathMeths[]=
{
#if 0
#endif
	{ NULL, NULL }
};


DaoTypeCore daoCanvasPathCore =
{
	"CanvasPath",                                      /* name */
	sizeof(DaoxCanvasPath),                            /* size */
	{ & daoCanvasNodeCore, NULL },                     /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxCanvasPathMeths,                               /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxCanvasNode_Delete,         /* Delete */
	DaoxCanvasNode_HandleGC                            /* HandleGC */
};




static DaoFunctionEntry DaoxCanvasTextMeths[]=
{
	{ NULL, NULL }
};


DaoTypeCore daoCanvasTextCore =
{
	"CanvasText",                                      /* name */
	sizeof(DaoxCanvasText),                            /* size */
	{ & daoCanvasNodeCore, NULL },                     /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxCanvasTextMeths,                               /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxCanvasNode_Delete,         /* Delete */
	DaoxCanvasNode_HandleGC                            /* HandleGC */
};



static DaoFunctionEntry DaoxCanvasImageMeths[]=
{
	{ NULL, NULL }
};


DaoTypeCore daoCanvasImageCore =
{
	"CanvasImage",                                     /* name */
	sizeof(DaoxCanvasImage),                           /* size */
	{ & daoCanvasNodeCore, NULL },                     /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxCanvasImageMeths,                              /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxCanvasNode_Delete,         /* Delete */
	DaoxCanvasNode_HandleGC                            /* HandleGC */
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
static void CANVAS_Remove( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxCanvas *self = (DaoxCanvas*) p[0];
	DaoxCanvasNode *node = (DaoxCanvasNode*) p[1];
	DaoxCanvas_Remove( self, node );
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
	DaoImage *image = (DaoImage*) p[1];
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



static DaoFunctionEntry DaoxCanvasMeths[]=
{
	{ CANVAS_New,         "Canvas()" },

	{ CANVAS_SetViewport,   "SetViewport( self: Canvas, left: float, right: float, bottom: float, top: float )" },

	{ CANVAS_GetViewport,   "GetViewport( self: Canvas ) => tuple<left:float,right:float,bottom:float,top:float>" },

	{ CANVAS_SetBackground,  "SetBackground( self: Canvas, red: float, green: float, blue: float, alpha = 1.0 ) => Canvas" },

	{ CANVAS_PushBrush,   "PushBrush( self: Canvas ) => Brush" },
	{ CANVAS_PushBrush,   "PushBrush( self: Canvas, index: int ) => invar<Brush>" },

	{ CANVAS_PopBrush,    "PopBrush( self: Canvas )" },

	{ CANVAS_Remove,   "Remove( self: Canvas, node: CanvasNode )" },

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

static void DaoxSceneNode_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove );

static void DaoxCanvas_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxCanvas *self = (DaoxCanvas*) p;
	DaoxSceneNode_HandleGC( p, values, lists, maps, remove );
	DList_Append( lists, self->nodes );
	DList_Append( lists, self->brushes );
	DList_Append( values, self->auxPath );
	DList_Append( values, self->pathCache );
	if( remove ) self->auxPath = NULL;
	if( remove ) self->pathCache = NULL;
}


extern DaoTypeCore daoSceneNodeCore;

DaoTypeCore daoCanvasCore =
{
	"Canvas",                                          /* name */
	sizeof(DaoxCanvas),                                /* size */
	{ & daoSceneNodeCore, NULL },                      /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxCanvasMeths,                                   /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxCanvas_Delete,             /* Delete */
	DaoxCanvas_HandleGC                                /* HandleGC */
};






static void MeshUnit_SetMaterial( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxMeshUnit *self = (DaoxMeshUnit*) p[0];
	DaoxMaterial *mat = (DaoxMaterial*) p[1];
	DaoxMeshUnit_SetMaterial( self, mat );
}

static DaoFunctionEntry DaoxMeshUnitMeths[]=
{
	{ MeshUnit_SetMaterial,
		"SetMaterial( self: MeshUnit, material: Material )"
	},
	{ NULL, NULL }
};
static void DaoxMeshUnit_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxMeshUnit *self = (DaoxMeshUnit*) p;
	if( self->mesh ) DList_Append( values, self->mesh );
	if( self->material ) DList_Append( values, self->material );
	if( remove ){
		self->mesh = NULL;
		self->material = NULL;
	}
}

DaoTypeCore daoMeshUnitCore =
{
	"MeshUnit",                                        /* name */
	sizeof(DaoxMeshUnit),                              /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxMeshUnitMeths,                                 /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxMeshUnit_Delete,           /* Delete */
	DaoxMeshUnit_HandleGC                              /* HandleGC */
};


static DaoFunctionEntry DaoxMeshMeths[]=
{
	{ NULL, NULL }
};
static void DaoxMesh_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxMesh *self = (DaoxMesh*) p;
	DList_Append( lists, self->units );
}

DaoTypeCore daoMeshCore =
{
	"Mesh",                                            /* name */
	sizeof(DaoxMesh),                                  /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxMeshMeths,                                     /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxMesh_Delete,               /* Delete */
	DaoxMesh_HandleGC                                  /* HandleGC */
};



static void TEX_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTexture *self = DaoxTexture_New();
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void TEX_SetImage( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTexture *self = (DaoxTexture*) p[0];
	DaoImage *image = (DaoImage*) p[1];
	DaoxTexture_SetImage( self, image );
}

static DaoFunctionEntry DaoxTextureMeths[]=
{
	{ TEX_New,
		"Texture()"
	},
	{ TEX_SetImage,
		"SetImage( self: Texture, image: Image )"
	},
	{ NULL, NULL }
};

static void DaoxTexture_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxTexture *self = (DaoxTexture*) p;
	if( self->image ) DList_Append( values, self->image );
	if( remove ) self->image = NULL;
}

DaoTypeCore daoTextureCore =
{
	"Texture",                                         /* name */
	sizeof(DaoxTexture),                               /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxTextureMeths,                                  /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxTexture_Delete,            /* Delete */
	DaoxTexture_HandleGC                               /* HandleGC */
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
static DaoFunctionEntry DaoxMaterialMeths[]=
{
	{ MAT_New,
		"Material()"
	},
	{ MAT_SetColor,
		"SetColor( self: Material, ...: tuple<enum<ambient,diffuse,specular,emission>,tuple<float,float,float>> )"
	},
	{ MAT_SetTexture,
		"SetTexture( self: Material, texture: Texture, which: enum<diffuse,emission,bump> = $diffuse )"
	},
	{ NULL, NULL }
};

static void DaoxMaterial_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxMaterial *self = (DaoxMaterial*) p;
	if( self->diffuseTexture ) DList_Append( values, self->diffuseTexture );
	if( self->emissionTexture ) DList_Append( values, self->emissionTexture );
	if( self->bumpTexture ) DList_Append( values, self->bumpTexture );
	if( remove ){
		self->diffuseTexture = NULL;
		self->emissionTexture = NULL;
		self->bumpTexture = NULL;
	}
}

DaoTypeCore daoMaterialCore =
{
	"Material",                                        /* name */
	sizeof(DaoxMaterial),                              /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxMaterialMeths,                                 /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxMaterial_Delete,           /* Delete */
	DaoxMaterial_HandleGC                              /* HandleGC */
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
static DaoFunctionEntry DaoxSceneNodeMeths[]=
{
	{ SNODE_Move,    "Move( self: SceneNode, x: float, y: float, z: float )" },
	{ SNODE_MoveBy,  "MoveBy( self: SceneNode, dx: float, dy: float, dz: float )" },
	{ SNODE_Trans,   ".translation( self: SceneNode ) => tuple<x:float,y:float,z:float>" },
	{ NULL, NULL }
};
static void DaoxSceneNode_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxSceneNode *self = (DaoxSceneNode*) p;
	DList_Append( lists, self->children );
	if( self->controller && self->controller->animations ){
		DList_Append( lists, self->controller->animations );
	}
	if( self->parent ) DList_Append( values, self->parent );
	if( remove ){
		self->parent = NULL;
	}
}

DaoTypeCore daoSceneNodeCore =
{
	"SceneNode",                                       /* name */
	sizeof(DaoxSceneNode),                             /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxSceneNodeMeths,                                /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxSceneNode_Delete,          /* Delete */
	DaoxSceneNode_HandleGC                             /* HandleGC */
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
static DaoFunctionEntry DaoxCameraMeths[]=
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

DaoTypeCore daoCameraCore =
{
	"Camera",                                          /* name */
	sizeof(DaoxCamera),                                /* size */
	{ & daoSceneNodeCore, NULL },                      /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxCameraMeths,                                   /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxCamera_Delete,             /* Delete */
	DaoxSceneNode_HandleGC                             /* HandleGC */
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
static DaoFunctionEntry DaoxLightMeths[]=
{
	{ LIGHT_New,
		"Light( litype: enum<ambient,point,directional,spot>, red =1.0, green =1.0, blue =1.0 )"
	},
	{ NULL, NULL }
};

DaoTypeCore daoLightCore =
{
	"Light",                                           /* name */
	sizeof(DaoxLight),                                 /* size */
	{ & daoSceneNodeCore, NULL },                      /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxLightMeths,                                    /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxLight_Delete,              /* Delete */
	DaoxSceneNode_HandleGC                             /* HandleGC */
};





DaoTypeCore daoJointCore =
{
	"Joint",                                           /* name */
	sizeof(DaoxJoint),                                 /* size */
	{ & daoSceneNodeCore, NULL },                      /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	NULL,                                              /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxJoint_Delete,              /* Delete */
	DaoxSceneNode_HandleGC                             /* HandleGC */
};




static void DaoxSkeleton_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxSkeleton *self = (DaoxSkeleton*) p;
	DList_Append( lists, self->joints );
}

DaoTypeCore daoSkeletonCore =
{
	"Skeleton",                                        /* name */
	sizeof(DaoxSkeleton),                              /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	NULL,                                              /* methods */
	NULL,                      NULL,                   /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxSkeleton_Delete,           /* Delete */
	DaoxSkeleton_HandleGC                              /* HandleGC */
};





static void MODEL_SetMaterial( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxModel *self = (DaoxModel*) p[0];
	DaoxMaterial *mat = (DaoxMaterial*) p[1];
	DaoxMesh_SetMaterial( self->mesh, mat );
}
static DaoFunctionEntry DaoxModelMeths[]=
{
	{ MODEL_SetMaterial,
		"SetMaterial( self: Model, material: Material )"
	},
	{ NULL, NULL }
};
static void DaoxModel_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxModel *self = (DaoxModel*) p;
	DaoxSceneNode_HandleGC( p, values, lists, maps, remove );
	if( self->mesh ) DList_Append( values, self->mesh );
	if( self->skeleton ) DList_Append( values, self->skeleton );
	if( remove ){
		self->mesh = NULL;
		self->skeleton = NULL;
	}
}

DaoTypeCore daoModelCore =
{
	"Model",                                           /* name */
	sizeof(DaoxModel),                                 /* size */
	{ & daoSceneNodeCore, NULL },                      /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxModelMeths,                                    /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxModel_Delete,              /* Delete */
	DaoxModel_HandleGC                                 /* HandleGC */
};




static void EMITTER_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxEmitter *self = DaoxEmitter_New();
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void EMITTER_SetMaterial( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxEmitter *self = (DaoxEmitter*) p[0];
	DaoxMaterial *mat = (DaoxMaterial*) p[1];
	DaoxMesh_SetMaterial( self->base.mesh, mat );
	GC_Assign( & self->material, mat );
}
static void EMITTER_Configure( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxEmitter *self = (DaoxEmitter*) p[0];
	int i;
	for(i=1; i<N; ++i){
		DaoTuple *param = (DaoTuple*) p[i];
		float value = param->values[1]->xFloat.value;
		switch( param->values[0]->xEnum.value ){
		case 0 : self->emissionRate = value; break;
		case 1 : self->lifeSpan = value; break;
		case 2 : self->radialVelocity = value; break;
		case 3 : self->gravityStrength = value; break;
		}
	}
}
static DaoFunctionEntry DaoxEmitterMeths[]=
{
	{ EMITTER_New,
		"Emitter()"
	},
	{ EMITTER_SetMaterial,
		"SetMaterial( self: Emitter, material: Material )"
	},
	{ EMITTER_Configure,
		"Configure( self: Emitter, ... : tuple<enum<EmissionRate,LifeSpan,Velocity,Gravity>,float> )"
	},
	{ NULL, NULL }
};

static void DaoxEmitter_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxEmitter *self = (DaoxEmitter*) p;
	DaoxModel_HandleGC( p, values, lists, maps, remove );
	if( self->emitter ) DList_Append( values, self->emitter );
	if( self->material ) DList_Append( values, self->material );
	if( remove ){
		self->emitter = NULL;
		self->material = NULL;
	}
}


DaoTypeCore daoEmitterCore =
{
	"Emitter",                                         /* name */
	sizeof(DaoxEmitter),                               /* size */
	{ & daoModelCore, NULL },                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxEmitterMeths,                                  /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxEmitter_Delete,            /* Delete */
	DaoxEmitter_HandleGC                               /* HandleGC */
};



static void TerrainBlock_SetMaterial( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrainBlock *self = (DaoxTerrainBlock*) p[0];
	DaoxMaterial *mat = (DaoxMaterial*) p[1];
	DaoxMeshUnit_SetMaterial( self->mesh, mat );
}

static DaoFunctionEntry DaoxTerrainBlockMeths[]=
{
	{ TerrainBlock_SetMaterial,
		"SetMaterial( self: TerrainBlock, material: Material )"
	},
	{ NULL, NULL }
};

DaoTypeCore daoTerrainBlockCore =
{
	"TerrainBlock",                                    /* name */
	sizeof(DaoxTerrainBlock),                          /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxTerrainBlockMeths,                             /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxTerrainBlock_Delete,       /* Delete */
	NULL                                               /* HandleGC */
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

static DaoFunctionEntry DaoxTerrainMeths[]=
{
	{ Terrain_GetBlock,
		"GetBlock( self: Terrain, side: int, radius: int, offset: int ) => TerrainBlock"
	},
	{ Terrain_EachBlock,
		"EachBlock( self: Terrain ) [block:TerrainBlock]"
	},
	{ NULL, NULL }
};
static void DaoxTerrain_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxTerrain *self = (DaoxTerrain*) p;
	DaoxModel_HandleGC( p, values, lists, maps, remove );
	DList_Append( lists, self->blocks );
}

DaoTypeCore daoTerrainCore =
{
	"Terrain",                                         /* name */
	sizeof(DaoxTerrain),                               /* size */
	{ & daoModelCore, NULL },                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxTerrainMeths,                                  /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxTerrain_Delete,            /* Delete */
	DaoxTerrain_HandleGC                               /* HandleGC */
};




static void SCENE_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxScene *self = DaoxScene_New();
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void SCENE_SetBackground( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxScene *self = (DaoxScene*) p[0];
	DaoxColor color;
	color.red   = p[1]->xFloat.value;
	color.green = p[2]->xFloat.value;
	color.blue  = p[3]->xFloat.value;
	color.alpha = p[4]->xFloat.value;
	self->background = color;
}
static void SCENE_AddNode( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxScene *self = (DaoxScene*) p[0];
	DaoxSceneNode *node = (DaoxSceneNode*) p[1];
	DaoxScene_AddNode( self, node );
}
static void SCENE_AddBox( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxScene *self = (DaoxScene*) p[0];
	DaoxModel *model = DaoxModel_New();
	DaoxMesh *mesh = DaoxMesh_New();
	float wx = p[1]->xFloat.value;
	float wy = p[2]->xFloat.value;
	float wz = p[3]->xFloat.value;

	DaoxMesh_MakeBox( mesh, wx, wy, wz );
	DaoxMesh_UpdateTree( mesh, 0 );
	DaoxModel_SetMesh( model, mesh );
	DaoxSceneNode_MoveXYZ( (DaoxSceneNode*) model, 0, 0, 0 );
	DaoxScene_AddNode( self, (DaoxSceneNode*) model );
	DaoProcess_PutValue( proc, (DaoValue*) model );
}
static void SCENE_AddSphere( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxScene *self = (DaoxScene*) p[0];
	DaoxModel *model = DaoxModel_New();
	DaoxMesh *mesh = DaoxMesh_New();
	float radius = p[1]->xFloat.value;
	int res = p[2]->xInteger.value;

	DaoxMesh_MakeSphere( mesh, radius, res );
	DaoxMesh_UpdateTree( mesh, 0 );
	DaoxModel_SetMesh( model, mesh );
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
		DaoImage *image = (DaoImage*) p[1];
		float height = p[4]->xFloat.value;
		heightmap = DaoArray_New(DAO_FLOAT);
		_DaoImage_Export( image, heightmap, height / 255.0 );
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
		DaoImage *image = (DaoImage*) p[1];
		float height = p[4]->xFloat.value;
		heightmap = DaoArray_New(DAO_FLOAT);
		_DaoImage_Export( image, heightmap, height / 255.0 );
	}

	DaoxTerrain_SetHeightmap( terrain, heightmap );
	DaoxTerrain_SetHexBlocks( terrain, circles, radius );
	DaoxTerrain_Rebuild( terrain );
	DaoxScene_AddNode( self, (DaoxSceneNode*) terrain );
	DaoProcess_PutValue( proc, (DaoValue*) terrain );
}
static DaoFunctionEntry DaoxSceneMeths[] =
{
	{ SCENE_New,         "Scene()" },
	{ SCENE_SetBackground,  "SetBackground( self: Scene, red: float, green: float, blue: float, alpha = 1.0 )" },
	{ SCENE_AddNode,     "AddNode( self: Scene, node: SceneNode )" },
	{ SCENE_AddBox,      "AddBox( self: Scene, xlen = 1.0, ylen = 1.0, zlen = 1.0 ) => Model" },
	{ SCENE_AddSphere,   "AddSphere( self: Scene, radius = 1.0, resolution = 3 ) => Model" },
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
static void DaoxScene_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxScene *self = (DaoxScene*) p;
	DList_Append( lists, self->nodes );
	if( self->pathCache ) DList_Append( values, self->pathCache );
	if( remove ) self->pathCache = NULL;
}

DaoTypeCore daoSceneCore =
{
	"Scene",                                           /* name */
	sizeof(DaoxScene),                                 /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxSceneMeths,                                    /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxScene_Delete,              /* Delete */
	DaoxScene_HandleGC                                 /* HandleGC */
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
	DaoImage *image = (DaoImage*) p[2];
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
static DaoFunctionEntry DaoxPainterMeths[]=
{
	{ PAINTER_New,            "Painter( contex: Context )" },
	{ PAINTER_RenderToImage,  "RenderToImage( self: Painter, canvas: Canvas, image: Image, width: int, height: int )" },
	{ PAINTER_Paint,  "Paint( self: Painter, canvas: Canvas )" },
	{ NULL, NULL }
};
static void DaoxPainter_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
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

DaoTypeCore daoPainterCore =
{
	"Painter",                                         /* name */
	sizeof(DaoxPainter),                               /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxPainterMeths,                                  /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxPainter_Delete,            /* Delete */
	DaoxPainter_HandleGC                               /* HandleGC */
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
static DaoFunctionEntry DaoxRendererMeths[]=
{
	{ RENDR_New,         "Renderer( contex: Context )" },
	{ RENDR_SetCurrentCamera,  "SetCurrentCamera( self: Renderer, camera: Camera )" },
	{ RENDR_GetCurrentCamera,  "GetCurrentCamera( self: Renderer ) => Camera" },
	{ RENDR_Enable,  "Enable( self: Renderer, what: enum<axis,mesh>, bl = true )" },
	{ RENDR_Render,  "Render( self: Renderer, scene: Scene )" },
	{ NULL, NULL }
};
static void DaoxRenderer_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxRenderer *self = (DaoxRenderer*) p;
	if( self->scene ) DList_Append( values, self->scene );
	if( self->camera ) DList_Append( values, self->camera );
	DList_Append( values, self->shader );
	DList_Append( values, self->buffer );
	DList_Append( values, self->bufferVG );
	DList_Append( values, self->bufferSK );
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
		self->bufferSK = NULL;
		self->context = NULL;
		self->axisMesh = NULL;
		self->worldAxis = NULL;
		self->localAxis = NULL;
	}
}

DaoTypeCore daoRendererCore =
{
	"Renderer",                                        /* name */
	sizeof(DaoxRenderer),                              /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxRendererMeths,                                 /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxRenderer_Delete,           /* Delete */
	DaoxRenderer_HandleGC                              /* HandleGC */
};





DaoTypeCore daoAnimationCore =
{
	"Animation",                                       /* name */
	sizeof(DaoxAnimation),                             /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	NULL,                                              /* methods */
	NULL,                      NULL,                   /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxAnimation_Delete,          /* Delete */
	NULL                                               /* HandleGC */
};





static void RES_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxResource *self = DaoxResource_New( proc->vmSpace );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void RES_RaiseLoadingError( DaoProcess *proc )
{
	DaoProcess_RaiseError( proc, NULL, "Model loading failed!" );
}
static void RES_LoadObjFile( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxResource *self = (DaoxResource*) p[0];
	DString *file = p[1]->xString.value;
	DString *codePath = proc->activeRoutine->nameSpace->path;
	DaoxScene *scene = DaoxResource_LoadObjFile( self, file, codePath );
	if( scene == NULL ){
		RES_RaiseLoadingError( proc );
		return;
	}
	DaoProcess_PutValue( proc, (DaoValue*) scene );
}
static void RES_LoadDaeFile( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxResource *self = (DaoxResource*) p[0];
	DString *file = p[1]->xString.value;
	DString *codePath = proc->activeRoutine->nameSpace->path;
	DaoxScene *scene = DaoxResource_LoadColladaFile( self, file, codePath );
	if( scene == NULL ){
		RES_RaiseLoadingError( proc );
		return;
	}
	DaoProcess_PutValue( proc, (DaoValue*) scene );
}
static DaoFunctionEntry DaoxResourceMeths[]=
{
	{ RES_New,              "Resource()" },
	{ RES_LoadObjFile,      "LoadObjFile( self: Resource, file: string ) => Scene" },
	{ RES_LoadDaeFile,      "LoadDaeFile( self: Resource, file: string ) => Scene" },
	{ NULL, NULL }
};
static void DaoxResource_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
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

DaoTypeCore daoResourceCore =
{
	"Resource",                                        /* name */
	sizeof(DaoxResource),                              /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxResourceMeths,                                 /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxResource_Delete,           /* Delete */
	DaoxResource_HandleGC                              /* HandleGC */
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

static DaoFunctionEntry DaoxTerrainGeneratorMeths[]=
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
static void DaoxTerrainGenerator_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxTerrainGenerator *self = (DaoxTerrainGenerator*) p;
	DList_Append( values, self->terrain );
	if( remove ) self->terrain = NULL;
}

DaoTypeCore daoTerrainGeneratorCore =
{
	"TerrainGenerator",                                /* name */
	sizeof(DaoxTerrainGenerator),                      /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxTerrainGeneratorMeths,                         /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxTerrainGenerator_Delete,   /* Delete */
	DaoxTerrainGenerator_HandleGC                      /* HandleGC */
};



static DaoFunctionEntry DaoxShaderMeths[]=
{
	{ NULL, NULL }
};


DaoTypeCore daoShaderCore =
{
	"Shader",                                          /* name */
	sizeof(DaoxShader),                                /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxShaderMeths,                                   /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxShader_Delete,             /* Delete */
	NULL                                               /* HandleGC */
};



static DaoFunctionEntry DaoxBufferMeths[]=
{
	{ NULL, NULL }
};


DaoTypeCore daoBufferCore =
{
	"Buffer",                                          /* name */
	sizeof(DaoxBuffer),                                /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxBufferMeths,                                   /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxBuffer_Delete,             /* Delete */
	NULL                                               /* HandleGC */
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
static DaoFunctionEntry DaoxContextMeths[]=
{
	{ CTX_New,   "Context( width: int, height: int )" },
	{ CTX_Quit,  "Quit( self: Context )" },
	{ NULL, NULL }
};

static void DaoxContext_HandleGC( DaoValue *p, DList *values, DList *lists, DList *maps, int remove )
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

DaoTypeCore daoContextCore =
{
	"Context",                                         /* name */
	sizeof(DaoxContext),                               /* size */
	{ NULL },                                          /* bases */
	{ NULL },                                          /* casts */
	NULL,                                              /* numbers */
	DaoxContextMeths,                                  /* methods */
	DaoCstruct_CheckGetField,  DaoCstruct_DoGetField,  /* GetField */
	NULL,                      NULL,                   /* SetField */
	NULL,                      NULL,                   /* GetItem */
	NULL,                      NULL,                   /* SetItem */
	NULL,                      NULL,                   /* Unary */
	NULL,                      NULL,                   /* Binary */
	NULL,                      NULL,                   /* Conversion */
	NULL,                      NULL,                   /* ForEach */
	NULL,                                              /* Print */
	NULL,                                              /* Slice */
	NULL,                                              /* Compare */
	NULL,                                              /* Hash */
	NULL,                                              /* Create */
	NULL,                                              /* Copy */
	(DaoDeleteFunction) DaoxContext_Delete,            /* Delete */
	DaoxContext_HandleGC                               /* HandleGC */
};



static void GRAPHICS_Backend( DaoProcess *proc, DaoValue *p[], int N )
{
#   ifdef DAO_GRAPHICS_USE_GLES
	DaoProcess_PutEnum( proc, "OpenGLES" );
#   else
	DaoProcess_PutEnum( proc, "OpenGL" );
#   endif
}

static DaoFunctionEntry globalMeths[]=
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
DaoType *daox_type_emitter = NULL;
DaoType *daox_type_joint = NULL;
DaoType *daox_type_skeleton = NULL;
DaoType *daox_type_model = NULL;
DaoType *daox_type_terrain = NULL;
DaoType *daox_type_scene = NULL;
DaoType *daox_type_painter = NULL;
DaoType *daox_type_renderer = NULL;
DaoType *daox_type_animation = NULL;
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
	DaoNamespace *randns = DaoVmSpace_LinkModule( vmSpace, nspace, "random" );
	DaoNamespace *imns = DaoVmSpace_LinkModule( vmSpace, nspace, "image" );
	DaoNamespace *ns;

	dao_vmspace_graphics = vmSpace;
	ns = DaoVmSpace_GetNamespace( vmSpace, "Graphics" );
	DaoNamespace_AddParent( ns, imns );
	DaoNamespace_AddConst( nspace, ns->name, (DaoValue*) ns, DAO_PERM_PUBLIC );
	DaoNamespace_WrapFunctions( ns, globalMeths );

	DaoFont_OnLoad( vmSpace, ns );
	DaoTriangulator_OnLoad( vmSpace, ns );

	DaoNamespace_DefineType( ns, "tuple<red:float,green:float,blue:float,alpha:float>", "Color" );

	daox_type_path = DaoNamespace_WrapType( ns, & daoPathCore, DAO_CSTRUCT, 0 );
	daox_type_path_mesh = DaoNamespace_WrapType( ns, & daoPathMeshCore, DAO_CSTRUCT, 0 );
	daox_type_path_cache = DaoNamespace_WrapType( ns, & daoPathCacheCore, DAO_CSTRUCT, 0 );

	daox_type_gradient = DaoNamespace_WrapType( ns, & daoGradientCore, DAO_CSTRUCT, 0 );
	daox_type_linear_gradient = DaoNamespace_WrapType( ns, & daoLinearGradientCore, DAO_CSTRUCT, 0 );
	daox_type_radial_gradient = DaoNamespace_WrapType( ns, & daoRadialGradientCore, DAO_CSTRUCT, 0 );

	daox_type_brush = DaoNamespace_WrapType( ns, & daoBrushCore, DAO_CSTRUCT, 0 );
	daox_type_canvas_node = DaoNamespace_WrapType( ns, & daoCanvasNodeCore, DAO_CSTRUCT, 0 );
	daox_type_canvas_line = DaoNamespace_WrapType( ns, & daoCanvasLineCore, DAO_CSTRUCT, 0 );
	daox_type_canvas_rect = DaoNamespace_WrapType( ns, & daoCanvasRectCore, DAO_CSTRUCT, 0 );
	daox_type_canvas_circle = DaoNamespace_WrapType( ns, & daoCanvasCircleCore, DAO_CSTRUCT, 0 );
	daox_type_canvas_ellipse = DaoNamespace_WrapType( ns, & daoCanvasEllipseCore, DAO_CSTRUCT, 0 );
	daox_type_canvas_path = DaoNamespace_WrapType( ns, & daoCanvasPathCore, DAO_CSTRUCT, 0 );
	daox_type_canvas_text = DaoNamespace_WrapType( ns, & daoCanvasTextCore, DAO_CSTRUCT, 0 );
	daox_type_canvas_image = DaoNamespace_WrapType( ns, & daoCanvasImageCore, DAO_CSTRUCT, 0 );

	daox_type_mesh_unit = DaoNamespace_WrapType( ns, & daoMeshUnitCore, DAO_CSTRUCT, 0 );
	daox_type_mesh = DaoNamespace_WrapType( ns, & daoMeshCore, DAO_CSTRUCT, 0 );
	daox_type_texture = DaoNamespace_WrapType( ns, & daoTextureCore, DAO_CSTRUCT, 0 );
	daox_type_material = DaoNamespace_WrapType( ns, & daoMaterialCore, DAO_CSTRUCT, 0 );
	daox_type_scene_node = DaoNamespace_WrapType( ns, & daoSceneNodeCore, DAO_CSTRUCT, 0 );
	daox_type_camera = DaoNamespace_WrapType( ns, & daoCameraCore, DAO_CSTRUCT, 0 );
	daox_type_light = DaoNamespace_WrapType( ns, & daoLightCore, DAO_CSTRUCT, 0 );
	daox_type_joint = DaoNamespace_WrapType( ns, & daoJointCore, DAO_CSTRUCT, 0 );
	daox_type_skeleton = DaoNamespace_WrapType( ns, & daoSkeletonCore, DAO_CSTRUCT, 0 );
	daox_type_model = DaoNamespace_WrapType( ns, & daoModelCore, DAO_CSTRUCT, 0 );
	daox_type_emitter = DaoNamespace_WrapType( ns, & daoEmitterCore, DAO_CSTRUCT, 0 );
	daox_type_terrain = DaoNamespace_WrapType( ns, & daoTerrainCore, DAO_CSTRUCT, 0 );

	daox_type_canvas = DaoNamespace_WrapType( ns, & daoCanvasCore, DAO_CSTRUCT, 0 );
	daox_type_scene = DaoNamespace_WrapType( ns, & daoSceneCore, DAO_CSTRUCT, 0 );
	daox_type_painter = DaoNamespace_WrapType( ns, & daoPainterCore, DAO_CSTRUCT, 0 );
	daox_type_renderer = DaoNamespace_WrapType( ns, & daoRendererCore, DAO_CSTRUCT, 0 );
	daox_type_animation = DaoNamespace_WrapType( ns, & daoAnimationCore, DAO_CSTRUCT, 0 );
	daox_type_resource = DaoNamespace_WrapType( ns, & daoResourceCore, DAO_CSTRUCT, 0 );
	daox_type_terrain_block = DaoNamespace_WrapType( ns, & daoTerrainBlockCore, DAO_CSTRUCT, 0 );
	daox_type_terrain_generator = DaoNamespace_WrapType( ns, & daoTerrainGeneratorCore, DAO_CSTRUCT, 0 );

	daox_type_shader = DaoNamespace_WrapType( ns, & daoShaderCore, DAO_CSTRUCT, 0 );
	daox_type_buffer = DaoNamespace_WrapType( ns, & daoBufferCore, DAO_CSTRUCT, 0 );
	daox_type_context = DaoNamespace_WrapType( ns, & daoContextCore, DAO_CSTRUCT, 0 );

	DaoWindow_OnLoad( vmSpace, ns );
	return 0;
}

