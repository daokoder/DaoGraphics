/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2013, Limin Fu
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
#include "dao_collada.h"


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

static DaoFuncItem DaoxTextureMeths[]=
{
	{ NULL, NULL }
};
DaoTypeBase DaoxTexture_Typer =
{
	"Texture", NULL, NULL, (DaoFuncItem*) DaoxTextureMeths, {0}, {0},
	(FuncPtrDel)DaoxTexture_Delete, NULL
};

static DaoFuncItem DaoxMaterialMeths[]=
{
	{ NULL, NULL }
};
DaoTypeBase DaoxMaterial_Typer =
{
	"Material", NULL, NULL, (DaoFuncItem*) DaoxMaterialMeths, {0}, {0},
	(FuncPtrDel)DaoxMaterial_Delete, NULL
};


static DaoFuncItem DaoxSceneNodeMeths[]=
{
	{ NULL, NULL }
};
DaoTypeBase DaoxSceneNode_Typer =
{
	"SceneNode", NULL, NULL, (DaoFuncItem*) DaoxSceneNodeMeths, {0}, {0},
	(FuncPtrDel)DaoxSceneNode_Delete, NULL
};


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
	{ CAM_Move,    "Move( self: Camera, x: float, y: float, z: float )" },
	{ CAM_MoveBy,  "MoveBy( self: Camera, dx: float, dy: float, dz: float )" },
	{ CAM_LookAt,  "LookAt( self: Camera, x: float, y: float, z: float )" },
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



static DaoFuncItem DaoxLightMeths[]=
{
	{ NULL, NULL }
};
DaoTypeBase DaoxLight_Typer =
{
	"Light", NULL, NULL, (DaoFuncItem*) DaoxLightMeths,
	{ & DaoxSceneNode_Typer, 0 }, {0},
	(FuncPtrDel)DaoxLight_Delete, NULL
};

static DaoFuncItem DaoxModelMeths[]=
{
	{ NULL, NULL }
};
DaoTypeBase DaoxModel_Typer =
{
	"Model", NULL, NULL, (DaoFuncItem*) DaoxModelMeths,
	{ & DaoxSceneNode_Typer, 0 }, {0},
	(FuncPtrDel)DaoxModel_Delete, NULL
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
void DaoxScene_Load3DS( DaoxScene *self, const char *file );
static void SCENE_Load3DS( DaoProcess *proc, DaoValue *p[], int N )
{
	int j;
	DaoxCamera *camera = NULL;
	DaoxScene *scene = (DaoxScene*) p[0];
	DaoxScene_Load3DS( scene, p[1]->xString.value->chars );
	return;
	for(j=0; j<scene->nodes->size; ++j){
		DaoxSceneNode *node = (DaoxSceneNode*) scene->nodes->items.pVoid[j];
		if( node->ctype == daox_type_camera ) camera = (DaoxCamera*) node;
	}
	camera = DaoxCamera_New();
	DaoxScene_AddNode( scene, (DaoxSceneNode*) camera );

	camera->fovAngle = 30;
	camera->aspectRatio = 0.8;//WIDTH / (float)HEIGHT;
	DaoxCamera_MoveXYZ( camera, 0, 1000, 50 );
	DaoxCamera_LookAtXYZ( camera, 0, 0, 20 );
	camera->farPlane = 1000;
}
static DaoFuncItem DaoxSceneMeths[]=
{
	{ SCENE_New,         "Scene()" },
	{ SCENE_AddNode,     "AddNode( self: Scene, node: SceneNode )" },
	{ SCENE_Load3DS,     "Load3DS( self: Scene, file: string )" },
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
	{ PAINTER_RenderToImage,  "RenderToImage( self :Painter, canvas :Canvas, image :Image, width :int, height :int )" },
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
static void RENDR_GetCurrentCamera( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxRenderer *self = (DaoxRenderer*) p[0];
	DaoxCamera *cam = DaoxRenderer_GetCurrentCamera( self );
	DaoProcess_PutValue( proc, (DaoValue*) cam );
}
static DaoFuncItem DaoxRendererMeths[]=
{
	{ RENDR_New,         "Renderer()" },
	{ RENDR_GetCurrentCamera,  "GetCurrentCamera( self: Renderer ) => Camera" },
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
static void RES_LoadColladaFile( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxSceneResource *self = (DaoxSceneResource*) p[0];
	const char *file = DaoValue_TryGetChars( p[1] );
	DaoxScene *scene = DaoxSceneResource_LoadColladaFile( self, file );
	DaoProcess_PutValue( proc, (DaoValue*) scene );
}
static DaoFuncItem DaoxResourceMeths[]=
{
	{ RES_New,              "Resource()" },
	{ RES_LoadColladaFile,  "LoadColladaFile( self: Resource, file: string ) => Scene" },
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
DaoType *daox_type_scene = NULL;
DaoType *daox_type_painter = NULL;
DaoType *daox_type_renderer = NULL;
DaoType *daox_type_resource = NULL;



int WIDTH = 900;
int HEIGHT = 600;
void reshape(int width, int height);
void display(void);

DaoxCamera *camera = NULL;
DaoxScene *scene = NULL;
DaoxRenderer *renderer = NULL;


DaoxScene* Test_Collada();

DAO_DLL int DaoVectorGraphics_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );
DAO_DLL int DaoGLUT_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns );

static void Test();

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
	daox_type_scene = DaoNamespace_WrapType( ns, & DaoxScene_Typer, 0 );
	daox_type_painter = DaoNamespace_WrapType( ns, & DaoxPainter_Typer, 0 );
	daox_type_renderer = DaoNamespace_WrapType( ns, & DaoxRenderer_Typer, 0 );
	daox_type_resource = DaoNamespace_WrapType( ns, & DaoxResource_Typer, 0 );
	DaoVectorGraphics_OnLoad( vmSpace, ns );
	DaoGLUT_OnLoad( vmSpace, ns );
	//Test();
	return 0;
}

void Test()
{
	//scene = Test_Collada();

	//DaoVectorGraphics_OnLoad( vmSpace, ns );

	scene = DaoxScene_New();
	camera = DaoxCamera_New();

	camera->fovAngle = 30;
	camera->aspectRatio = WIDTH / (float)HEIGHT;

	DaoxModel *model = DaoxModel_New();
	DaoxMesh *mesh = DaoxMesh_New();
	DaoxMesh_MakeViewFrustumCorners( mesh, camera->fovAngle, camera->aspectRatio, camera->nearPlane );
	//DaoxMesh_MakeBoxObject( mesh );
	DaoxMesh_UpdateTree( mesh, 0 );
	DaoxModel_SetMesh( model, mesh );


	//DaoxSceneNode_MoveXYZ( (DaoxSceneNode*) model, 0, 0, -5 );

	DaoxScene_AddNode( scene, (DaoxSceneNode*) camera );
	//DaoxScene_AddNode( scene, (DaoxSceneNode*) model );

	//DaoxSceneNode_MoveXYZ( (DaoxSceneNode*) model, 4.5, 0, 0 );
	//DaoxSceneNode_MoveXYZ( (DaoxSceneNode*) camera, 0, 8, 30 );
	//DaoxCamera_MoveXYZ( camera, 0, 0, 3 );

	DaoxCamera_MoveXYZ( camera, 0, 1000, 50 );
	DaoxCamera_LookAtXYZ( camera, 0, 0, 20 );
	camera->farPlane = 1000;
#if 0
#endif

#if 0
	// Grumman-F14:
	DaoxCamera_MoveXYZ( camera, 5, 20, 5 );
	DaoxCamera_LookAtXYZ( camera, 0, 0, 2 );
	camera->farPlane = 1000;
#endif

#if 0
	// MP_US_Engi:
	DaoxCamera_MoveXYZ( camera, 0, -200, 100 );
	DaoxCamera_LookAtXYZ( camera, 0, 100, 100 );
	DaoxCamera_LookAtXYZ( camera, 0, 400, 0 );
	camera->farPlane = 10000;
#endif

	DaoxScene_Load3DS( scene, "BTR80.3ds" );
	//DaoxScene_Load3DS( scene, "fels.3ds" );
	//DaoxScene_Load3DS( scene, "teapot.3ds" );
	//DaoxScene_Load3DS( scene, "x29.3ds" );
	//DaoxScene_Load3DS( scene, "ironman.max" );
	//DaoxScene_Load3DS( scene, "Well/Well.3DS" );
	//DaoxScene_Load3DS( scene, "Grumman-F14-Tomcat/f14d.3ds" );
	//DaoxScene_Load3DS( scene, "MP_US_Engi.3ds" );
	//DaoxScene_Load3DS( scene, "rhino.3ds" );
	//DaoxScene_Load3DS( scene, "burger.3ds" );
	//DaoxScene_Load3DS( scene, "fighter1/fighter1.3ds" );
	//DaoxScene_Load3DS( scene, "cube_with_specular_texture.3DS" );
	//DaoxScene_Load3DS( scene, "cube_with_diffuse_texture.3DS" );

	DaoxMatrix4D rotation = DaoxMatrix4D_EulerRotation( 0.5, 0.3, 0.4 );
	DaoxMatrix4D inverse = DaoxMatrix4D_Inverse( & rotation );
	DaoxMatrix4D prod = DaoxMatrix4D_MulMatrix( & rotation, & inverse );
	DaoxMatrix4D_Print( & rotation );
	DaoxMatrix4D_Print( & inverse );
	DaoxMatrix4D_Print( & prod );

	printf( "nodes: %i\n", scene->nodes->size );


	int j;
	for(j=1; j<0; ++j){
		DaoxModel *model2 = DaoxModel_New();
		DaoxModel_SetMesh( model2, mesh );
		DaoxScene_AddNode( scene, (DaoxSceneNode*) model2 );
		DaoxSceneNode_MoveXYZ( (DaoxSceneNode*) model2, (j+1)*1.5, 0, 0 );
	}
	for(j=1; j<scene->nodes->size; ++j){
		DaoxSceneNode *node = (DaoxSceneNode*) scene->nodes->items.pVoid[j];
		if( node->ctype == daox_type_camera ) camera = (DaoxCamera*) node;
	}
	camera->fovAngle = 60;
	camera->aspectRatio = WIDTH / (float)HEIGHT;

	DaoxSceneNode_AddChild( (DaoxSceneNode*) camera, (DaoxSceneNode*) model );

	DaoxVector3D pos = DaoxSceneNode_GetWorldPosition( (DaoxSceneNode*) camera );
	DaoxVector3D_Print( & pos );


	int i, argc = 1;
	char *argv = "Dao Graphics";

#ifdef FREEGLUT
	glutInitContextVersion (3, 2);
	glutInitContextFlags (GLUT_FORWARD_COMPATIBLE
#ifdef DEBUG
			| GLUT_DEBUG
#endif
	);
#endif

	glutInit(&argc, &argv);
	glutInitWindowSize(WIDTH,HEIGHT);
	glutInitWindowPosition(100,100);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE
#ifndef FREEGLUT
			|GLUT_3_2_CORE_PROFILE
#endif
			);

	glutCreateWindow( argv );
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);


	renderer = DaoxRenderer_New();
	//DaoxRenderer_InitShaders( renderer );
	//DaoxRenderer_InitBuffers( renderer );

	glClearColor(0.8f,0.8f,0.8f,1.f);


	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_NORMALIZE);

	//glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

	glViewport(0, 0, WIDTH, HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	glutGet(GLUT_ELAPSED_TIME);
	glutMainLoop();
}
#if 0
#endif


void DaoxCanvas_ApplyMaxFPS( int fps_limit );
void DaoxCanvas_TestFPS();


static float angle = 50.f;
static float angle_step = 0.f;
void do_motion (void)
{
	static GLint prev_time = 0;

	int time = glutGet(GLUT_ELAPSED_TIME);
	angle_step = (time-prev_time)*0.0003;
	angle += angle_step;
	prev_time = time;

	glutPostRedisplay ();
}
void reshape(int width, int height)
{
}
GLuint scene_list = 0;
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DaoxRenderer_Render( renderer, scene, camera );
	glutSwapBuffers();

	DaoxCanvas_ApplyMaxFPS( 3000 );
	DaoxCanvas_TestFPS();
	do_motion();
}
