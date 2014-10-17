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


DaoVmSpace *__daoVmSpace = NULL;


static DaoFuncItem DaoxMeshUnitMeths[]=
{
	{ NULL, NULL }
};
DaoTypeBase DaoxMeshUnit_Typer =
{
	"MeshUnit", NULL, NULL, (DaoFuncItem*) DaoxMeshUnitMeths, {0}, {0},
	(FuncPtrDel)DaoxMeshUnit_Delete, NULL
};

static DaoFuncItem DaoxMeshMeths[]=
{
	{ NULL, NULL }
};
DaoTypeBase DaoxMesh_Typer =
{
	"Mesh", NULL, NULL, (DaoFuncItem*) DaoxMeshMeths, {0}, {0},
	(FuncPtrDel)DaoxMesh_Delete, NULL
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
DaoTypeBase DaoxTexture_Typer =
{
	"Texture", NULL, NULL, (DaoFuncItem*) DaoxTextureMeths, {0}, {0},
	(FuncPtrDel)DaoxTexture_Delete, NULL
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
		printf( ">>>>>>>>>>>> %i %f\n", param->values[0]->xEnum.value, color.green );
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
DaoTypeBase DaoxMaterial_Typer =
{
	"Material", NULL, NULL, (DaoFuncItem*) DaoxMaterialMeths, {0}, {0},
	(FuncPtrDel)DaoxMaterial_Delete, NULL
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
static DaoFuncItem DaoxSceneNodeMeths[]=
{
	{ SNODE_Move,    "Move( self: SceneNode, x: float, y: float, z: float )" },
	{ SNODE_MoveBy,  "MoveBy( self: SceneNode, dx: float, dy: float, dz: float )" },
	{ NULL, NULL }
};
DaoTypeBase DaoxSceneNode_Typer =
{
	"SceneNode", NULL, NULL, (DaoFuncItem*) DaoxSceneNodeMeths, {0}, {0},
	(FuncPtrDel)DaoxSceneNode_Delete, NULL
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
	DaoxCamera_Orient( self, p[1]->xEnum.value + 1 );
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
static DaoFuncItem DaoxCameraMeths[]=
{
	{ CAM_New,     "Camera()" },
	{ CAM_Move,    "Move( self: Camera, x: float, y: float, z: float )" },
	{ CAM_MoveBy,  "MoveBy( self: Camera, dx: float, dy: float, dz: float )" },
	{ CAM_LookAt,  "LookAt( self: Camera, x: float, y: float, z: float )" },
	{ CAM_Rotate,  "Rotate( self: Camera, angle: float )" },
	{ CAM_Orient,  "Orient( self: Camera, worldUpAxis: enum<X,Y,Z> = $Z )" },
	{ CAM_SetFOV,  "SetFOV( self: Camera, angle: float )" },
	{ CAM_SetNearPlane,  "SetNearPlane( self: Camera, dist: float )" },
	{ CAM_SetFarPlane,   "SetFarPlane( self: Camera, dist: float )" },
	{ NULL, NULL }
};
DaoTypeBase DaoxCamera_Typer =
{
	"Camera", NULL, NULL, (DaoFuncItem*) DaoxCameraMeths,
	{ & DaoxSceneNode_Typer, 0 }, {0},
	(FuncPtrDel)DaoxCamera_Delete, NULL
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
	{ & DaoxSceneNode_Typer, 0 }, {0},
	(FuncPtrDel)DaoxLight_Delete, NULL
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
DaoTypeBase DaoxModel_Typer =
{
	"Model", NULL, NULL, (DaoFuncItem*) DaoxModelMeths,
	{ & DaoxSceneNode_Typer, 0 }, {0},
	(FuncPtrDel)DaoxModel_Delete, NULL
};




static void TERRAIN_SetMaterial( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxTerrain *self = (DaoxTerrain*) p[0];
	DaoxMaterial *material = (DaoxMaterial*) p[1];
	DaoxTerrain_SetMaterial( self, material );
}
static DaoFuncItem DaoxTerrainMeths[]=
{
	{ TERRAIN_SetMaterial,
		"SetMaterial( self: Terrain, material: Material, which: enum<first,second> = $first )"
	},
	{ NULL, NULL }
};
DaoTypeBase DaoxTerrain_Typer =
{
	"Terrain", NULL, NULL, (DaoFuncItem*) DaoxTerrainMeths,
	{ & DaoxSceneNode_Typer, 0 }, {0},
	(FuncPtrDel)DaoxTerrain_Delete, NULL
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
static void SCENE_AddTerrain( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxScene *self = (DaoxScene*) p[0];
	DaoxImage *heightmap = (DaoxImage*) p[1];
	DaoxTerrain *terrain = DaoxTerrain_New();
	float width = p[2]->xFloat.value;
	float length = p[3]->xFloat.value;
	float height = p[4]->xFloat.value;

	DaoxTerrain_SetHeightmap( terrain, heightmap );
	DaoxTerrain_SetSize( terrain, width, length, height );
	DaoxTerrain_Rebuild( terrain );
	DaoxScene_AddNode( self, (DaoxSceneNode*) terrain );
	DaoProcess_PutValue( proc, (DaoValue*) terrain );
}
static DaoFuncItem DaoxSceneMeths[] =
{
	{ SCENE_New,         "Scene()" },
	{ SCENE_AddNode,     "AddNode( self: Scene, node: SceneNode )" },
	{ SCENE_AddBox,      "AddBox( self: Scene, xlen = 1.0, ylen = 1.0, zlen = 1.0 ) => Model" },
	{ SCENE_AddTerrain,
		"AddTerrain( self: Scene, heightmap: Image, width = 1.0, length = 1.0, height = 1.0 )"
			"=> Terrain"
	},
	{ NULL, NULL }
};
DaoTypeBase DaoxScene_Typer =
{
	"Scene", NULL, NULL, (DaoFuncItem*) DaoxSceneMeths, {0}, {0},
	(FuncPtrDel)DaoxScene_Delete, NULL
};



static void PAINTER_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxPainter *self = DaoxPainter_New();
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
static DaoFuncItem DaoxPainterMeths[]=
{
	{ PAINTER_New,            "Painter()" },
	{ PAINTER_RenderToImage,  "RenderToImage( self: Painter, canvas: Canvas, image: Image, width: int, height: int )" },
	{ NULL, NULL }
};
DaoTypeBase DaoxPainter_Typer =
{
	"Painter", NULL, NULL, (DaoFuncItem*) DaoxPainterMeths, {0}, {0},
	(FuncPtrDel)DaoxPainter_Delete, NULL
};



static void RENDR_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxRenderer *self = DaoxRenderer_New();
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
	switch( p[1]->xEnum.value ){
	case 0 : self->showAxis = 0; break;
	case 1 : self->showAxis = 1; break;
	}
}
static DaoFuncItem DaoxRendererMeths[]=
{
	{ RENDR_New,         "Renderer()" },
	{ RENDR_SetCurrentCamera,  "SetCurrentCamera( self: Renderer, camera: Camera )" },
	{ RENDR_GetCurrentCamera,  "GetCurrentCamera( self: Renderer ) => Camera" },
	{ RENDR_Enable,  "Enable( self: Renderer, what: enum<none,axis> )" },
	{ NULL, NULL }
};
DaoTypeBase DaoxRenderer_Typer =
{
	"Renderer", NULL, NULL, (DaoFuncItem*) DaoxRendererMeths, {0}, {0},
	(FuncPtrDel)DaoxRenderer_Delete, NULL
};



static void RES_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxSceneResource *self = DaoxSceneResource_New();
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void RES_LoadObjFile( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxScene *scene;
	DaoxSceneResource *self = (DaoxSceneResource*) p[0];
	DString *codePath = proc->activeRoutine->nameSpace->path;
	DString *file = p[1]->xString.value;
	Dao_MakePath( codePath, file );
	scene = DaoxSceneResource_LoadObjFile( self, file->chars );
	DaoProcess_PutValue( proc, (DaoValue*) scene );
}
static DaoFuncItem DaoxResourceMeths[]=
{
	{ RES_New,              "Resource()" },
	{ RES_LoadObjFile,      "LoadObjFile( self: Resource, file: string ) => Scene" },
	{ NULL, NULL }
};
DaoTypeBase DaoxResource_Typer =
{
	"Resource", NULL, NULL, (DaoFuncItem*) DaoxResourceMeths, {0}, {0},
	(FuncPtrDel)DaoxSceneResource_Delete, NULL
};



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



DAO_DLL int DaoVectorGraphics_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );
DAO_DLL int DaoGLUT_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );

DAO_DLL int DaoGraphics_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *nspace )
{
	DaoNamespace *ns;
	__daoVmSpace = vmSpace;
	printf( "DaoGraphics3D_OnLoad\n" );
	ns = DaoVmSpace_GetNamespace( vmSpace, "Graphics" );
	DaoNamespace_AddConst( nspace, ns->name, (DaoValue*) ns, DAO_PERM_PUBLIC );

	daox_type_mesh_unit = DaoNamespace_WrapType( ns, & DaoxMeshUnit_Typer, 0 );
	daox_type_mesh = DaoNamespace_WrapType( ns, & DaoxMesh_Typer, 0 );
	daox_type_texture = DaoNamespace_WrapType( ns, & DaoxTexture_Typer, 0 );
	daox_type_material = DaoNamespace_WrapType( ns, & DaoxMaterial_Typer, 0 );
	daox_type_scene_node = DaoNamespace_WrapType( ns, & DaoxSceneNode_Typer, 0 );
	daox_type_camera = DaoNamespace_WrapType( ns, & DaoxCamera_Typer, 0 );
	daox_type_light = DaoNamespace_WrapType( ns, & DaoxLight_Typer, 0 );
	daox_type_model = DaoNamespace_WrapType( ns, & DaoxModel_Typer, 0 );
	daox_type_terrain = DaoNamespace_WrapType( ns, & DaoxTerrain_Typer, 0 );
	daox_type_scene = DaoNamespace_WrapType( ns, & DaoxScene_Typer, 0 );
	daox_type_painter = DaoNamespace_WrapType( ns, & DaoxPainter_Typer, 0 );
	daox_type_renderer = DaoNamespace_WrapType( ns, & DaoxRenderer_Typer, 0 );
	daox_type_resource = DaoNamespace_WrapType( ns, & DaoxResource_Typer, 0 );
	DaoVectorGraphics_OnLoad( vmSpace, ns );
	DaoGLUT_OnLoad( vmSpace, ns );
	return 0;
}

