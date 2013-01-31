/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2012, Limin Fu
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


#if defined(__APPLE__)
#  include <OpenGL/gl3.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl3.h>
#  include <GL/glut.h>
#endif

#include <math.h>
#include <stdio.h>
#include "dao_opengl.h"
#include "dao_painter.h"
#include "dao_renderer.h"


static float window_width = 300.0;
static float window_height = 200.0;
static float fps_limit = 10.0;

static int fps_count = 0;
static int current_time = 0;
static int last_update = 0;
static int count_reset = 0;
static int test_fps = 0;


DaoxPainter  *daox_current_painter = NULL;
DaoxRenderer *daox_current_renderer = NULL;
DaoxScene    *daox_current_scene = NULL;
DaoxCanvas   *daox_current_canvas = NULL;


static DaoRoutine* Dao_Get_Object_Method( DaoCstruct *cd, DaoObject **obj, const char *name )
{
  DaoRoutine *meth;
  if( cd == NULL ) return NULL;
  *obj = DaoCdata_GetObject( cd );
  if( *obj == NULL ) return NULL;
  return DaoObject_GetMethod( *obj, name );
}

void DaoCstruct_CallMethod( DaoCstruct *cdata, const char *method )
{
	DaoObject *obj = NULL;
	DaoRoutine *rout = Dao_Get_Object_Method( cdata, & obj, method );
	DaoProcess *proc;

	if( rout == NULL || obj == NULL ) return;
	proc = DaoVmSpace_AcquireProcess( __daoVmSpace );

	rout = DaoRoutine_Resolve( rout, (DaoValue*) obj, & daox_current_renderer, 1 );
	if( rout == NULL ) goto Finalize;
	DaoProcess_Call( proc, rout, (DaoValue*) obj, & daox_current_renderer, 1 );
Finalize:
	DaoVmSpace_ReleaseProcess( __daoVmSpace, proc );
}

int DaoCstruct_CallKeyboardMethod( DaoCstruct *cdata, const char *method, int key, int x, int y )
{
	DaoObject *obj = NULL;
	DaoRoutine *rout = Dao_Get_Object_Method( cdata, & obj, method );
	DaoProcess *proc;
	DaoValue **params;

	if( rout == NULL || obj == NULL ) return 0;
	proc = DaoVmSpace_AcquireProcess( __daoVmSpace );

	DaoProcess_NewInteger( proc, (daoint) key );
	DaoProcess_NewInteger( proc, (daoint) x );
	DaoProcess_NewInteger( proc, (daoint) y );
	params = DaoProcess_GetLastValues( proc, 3 );
	rout = DaoRoutine_Resolve( rout, (DaoValue*) obj, params, 3 );
	if( rout == NULL ) goto Finalize;
	DaoProcess_Call( proc, rout, (DaoValue*) obj, params, 3 );
Finalize:
	DaoVmSpace_ReleaseProcess( __daoVmSpace, proc );
	return rout != NULL;
}

void DaoxCanvas_UpdateScene( DaoxCanvas *canvas )
{
	DaoCstruct_CallMethod( (DaoCstruct*) canvas, "Update" );
}

void DaoxCanvas_glutIdle(void)
{
	if( daox_current_canvas ) DaoCstruct_CallMethod( (DaoCstruct*) daox_current_canvas, "Idle" );
}
void DaoxCanvas_ApplyMaxFPS( int fps_limit )
{
	int interval;
	current_time = glutGet(GLUT_ELAPSED_TIME);
	interval = current_time - last_update;
	if( interval < 1000.0 / fps_limit ) usleep( 2000 * (1000.0 / fps_limit - interval) );
	last_update = current_time;
}
void DaoxCanvas_TestFPS()
{
	fps_count += 1;

	if( last_update ){
		printf( "FPS: %9.1f\n", 1000.0*fps_count / (float)(current_time - count_reset) );
		fflush( stdout );
	}
	if( (current_time - count_reset) > 5000 ){
		fps_count = (int)(1000.0 * fps_count / (float)(current_time - count_reset));
		count_reset = current_time - 1000;
	}
}
void DaoxCanvas_glutDisplay(void)
{
	if( daox_current_painter && daox_current_canvas ){
		DaoxCanvas_UpdateScene( daox_current_canvas );
		DaoxPainter_Paint( daox_current_painter, daox_current_canvas, daox_current_canvas->viewport );
	}
	if( daox_current_renderer && daox_current_scene ){
		DaoxCamera *camera = DaoxRenderer_GetCurrentCamera( daox_current_renderer );
		camera->aspectRatio = window_width / (float) window_height;
		DaoCstruct_CallMethod( (DaoCstruct*) daox_current_scene, "Update" );
		DaoxRenderer_Render( daox_current_renderer, daox_current_scene, NULL );
	}

	glutSwapBuffers();

	DaoxCanvas_ApplyMaxFPS( fps_limit );

	if( test_fps ) DaoxCanvas_TestFPS();
}

void DaoxCanvas_glutReshape( int width, int height )
{
}

void DaoxCanvas_Zoom( int zoomin )
{
	DaoxAABBox2D box = daox_current_canvas->viewport;
	float width, height, dw, dh;
	width = box.right - box.left;
	height = box.top - box.bottom;
	dw = 0.0;
	dh = 0.0;
	if( zoomin ){
		dw = width / 6;
		dh = height / 6;
	}else{
		dw = - width / 4;
		dh = - height / 4;
	}
	box.left   += dw;
	box.right  -= dw;
	box.bottom += dh;
	box.top    -= dh;
	DaoxCanvas_SetViewport( daox_current_canvas, box.left, box.right, box.bottom, box.top );
}
void DaoxCanvas_glutKeyboard( unsigned char key, int x, int y )
{
	if( daox_current_canvas == NULL ) return;
	if( DaoCstruct_CallKeyboardMethod( daox_current_canvas, "OnKeyboard", key, x, y ) ) return;

	if( key == '+' ){
		DaoxCanvas_Zoom( 1 );
	}else if( key == '-' ){
		DaoxCanvas_Zoom( 0 );
	}else if( key == GLUT_KEY_UP || key == GLUT_KEY_DOWN || key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT ){
		if( daox_current_renderer && daox_current_scene ){
		}
	}
}

void DaoxCanvas_glutSpecialKeyboard( int key, int x, int y )
{
	if( daox_current_renderer ){
		DaoxCamera *camera = DaoxRenderer_GetCurrentCamera( daox_current_renderer );
		float dx = 0.0;
		float dy = 0.0;
		float dz = 0.0;
		float delta = 10*camera->nearPlane;
		switch( key ){
		case GLUT_KEY_UP    : dz = - delta; break;
		case GLUT_KEY_DOWN  : dz = + delta; break;
		case GLUT_KEY_LEFT  : dx = - delta; break;
		case GLUT_KEY_RIGHT : dx = + delta; break;
		}
		DaoxCamera_MoveByXYZ( camera, dx, dy, dz );
	}
	if( daox_current_canvas == NULL ) return;
	DaoCstruct_CallKeyboardMethod( daox_current_canvas, "OnKeyboard", key, x, y );
}

enum ActionType
{
	ROTATION ,
	MOVING
};

static int last_x = 0;
static int last_y = 0;
static int action_type = ROTATION;

void DaoxCanvas_glutButton( int button, int state, int x, int y )
{
	last_x = x;
	last_y = y;
	action_type = button == GLUT_LEFT_BUTTON ? ROTATION : MOVING;
}

void DaoxCanvas_Rotate( int x, int y )
{
	DaoxMatrix3D rotate = {1.0,0.0,0.0,0.0,1.0,0.0};
	DaoxAABBox2D box = daox_current_canvas->viewport;
	DaoxVector2D start, end, center = {0.0,0.0};
	double W2 = 0.5 * window_width;
	double H2 = 0.5 * window_height;
	double area, cosine, sine;

	start.x = last_x - W2;
	start.y = last_y - H2;
	end.x = x - W2;
	end.y = y - H2;

	area = DaoxTriangle_Area( center, start, end );
	cosine = DaoxTriangle_AngleCosine( center, start, end );
	sine = sqrt( 1.0 - cosine * cosine );


	rotate.A11 = rotate.A22 = cosine;
	if( area < 0.0 ){
		rotate.A12 = - sine;
		rotate.A21 =   sine;
	}else{
		rotate.A12 =   sine;
		rotate.A21 = - sine;
	}
	DaoxMatrix3D_Multiply( & daox_current_canvas->transform, rotate );

Done:
	last_x = x;
	last_y = y;
}
void DaoxCanvas_Move( int x, int y )
{
	DaoxAABBox2D box = daox_current_canvas->viewport;
	float xscale = (box.right - box.left) / window_width;
	float yscale = (box.top - box.bottom) / window_height;
	box.left   -= (x - last_x) * xscale;
	box.right  -= (x - last_x) * xscale;
	box.bottom += (y - last_y) * xscale;
	box.top    += (y - last_y) * xscale;
	last_x = x;
	last_y = y;
	if( box.left > 0.9*window_width ) return;
	if( box.right < 0.1*window_width ) return;
	if( box.bottom > 0.9*window_height ) return;
	if( box.top < 0.1*window_height ) return;
	DaoxCanvas_SetViewport( daox_current_canvas, box.left, box.right, box.bottom, box.top );
}
void DaoxCanvas_glutDrag( int x, int y )
{
	if( action_type == ROTATION ){
		DaoxCanvas_Rotate( x, y );
	}else{
		DaoxCanvas_Move( x, y );
	}
}

void DaoxCanvas_glutMove( int x, int y )
{
}

void DaoxCanvas_glutInit(int width, int height, const char *title)
{
	int i, argc = 1;
	char *argv = "Dao Graphics";

	glutInit( &argc, &argv );

	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_STENCIL|GLUT_MULTISAMPLE|GLUT_3_2_CORE_PROFILE);
	//glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_STENCIL|GLUT_MULTISAMPLE);

	glutInitWindowPosition(0,0);
	glutInitWindowSize(width,height);
	glutCreateWindow(title);

	glutDisplayFunc(DaoxCanvas_glutDisplay);
	//glutIdleFunc(DaoxCanvas_glutIdle);
	glutIdleFunc(DaoxCanvas_glutDisplay);
	glutReshapeFunc(DaoxCanvas_glutReshape);
	glutKeyboardFunc(DaoxCanvas_glutKeyboard);
	glutSpecialFunc(DaoxCanvas_glutSpecialKeyboard);
	glutMouseFunc(DaoxCanvas_glutButton);
	glutMotionFunc(DaoxCanvas_glutDrag);
	glutPassiveMotionFunc(DaoxCanvas_glutMove);

	//glEnable(GL_CULL_FACE);
}



static void GLUT_Init( DaoProcess *proc, DaoValue *p[], int N )
{
	char *title = DaoValue_TryGetMBString( p[2] );
	window_width = DaoValue_TryGetInteger( p[0] );
	window_height = DaoValue_TryGetInteger( p[1] );
	fps_limit = DaoValue_TryGetInteger( p[3] );
	test_fps = DaoValue_TryGetInteger( p[4] );
	DaoxCanvas_glutInit( window_width, window_height, title );
}
static void GLUT_SetGraphics( DaoProcess *proc, DaoValue *p[], int N )
{
	daox_current_painter = (DaoxCanvas*) p[0];
	daox_current_canvas = (DaoxCanvas*) p[1];
	daox_current_canvas->defaultWidth = window_width;
	daox_current_canvas->defaultHeight = window_height;
}

static void GLUT_Paint( DaoProcess *proc, DaoValue *p[], int N )
{
	GLUT_SetGraphics( proc, p, N );
	DaoxCanvas_SetViewport( daox_current_canvas,
			-window_width/2, window_width/2, -window_height/2, window_height/2 );
	glutMainLoop();
}
static void GLUT_Render( DaoProcess *proc, DaoValue *p[], int N )
{
	daox_current_renderer = (DaoxRenderer*) p[0];
	daox_current_scene = (DaoxScene*) p[1];
	if( daox_current_renderer->buffer.vertexOffset == 0 ){
		DaoxModel *model = DaoxModel_New();
		DaoxMesh *mesh = DaoxMesh_New();
		DaoxCamera *cam = DaoxRenderer_GetCurrentCamera( daox_current_renderer );
		cam->aspectRatio = window_width / (float) window_height;
		DaoxMesh_MakeViewFrustumCorners( mesh, cam->fovAngle, cam->aspectRatio, cam->nearPlane );
		//DaoxMesh_MakeBoxObject( mesh );
		DaoxMesh_UpdateTree( mesh, 0 );
		DaoxModel_SetMesh( model, mesh );
		DaoxSceneNode_AddChild( (DaoxSceneNode*) cam, (DaoxSceneNode*) model );

		DaoxRenderer_InitShaders( daox_current_renderer );
		DaoxRenderer_InitBuffers( daox_current_renderer );
	}
	glutMainLoop();
}
static DaoFuncItem DaoxGLUTMeths[]=
{
	{ GLUT_Init,      "glutInit( width = 300, height = 200, title = '', fps=10, test_fps=0 )" },
	{ GLUT_Paint,     "glutDisplay( painter : Painter, canvas : Canvas )" },
	{ GLUT_Render,    "glutDisplay( renderer : Renderer, scene : Scene )" },
	{ GLUT_SetGraphics,  "glutSetGraphics( painter : Painter, canvas : Canvas )" },
	{ NULL, NULL }
};


DAO_DLL int DaoGLUT_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns )
{
	__daoVmSpace = vmSpace;
	DaoNamespace_WrapFunctions( ns, DaoxGLUTMeths );
	return 0;
}
