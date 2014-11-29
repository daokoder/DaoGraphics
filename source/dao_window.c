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
#include <stdio.h>
#include "dao_opengl.h"
#include "dao_painter.h"
#include "dao_renderer.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"


typedef struct DaoxWindow DaoxWindow;

struct DaoxWindow
{
	DAO_CSTRUCT_COMMON;

	GLFWwindow *handle;

	DaoxPainter  *painter;
	DaoxRenderer *renderer;
	DaoValue     *model;

	DString *title;

	int visible;

	float width;
	float height;

	int test_fps;
	int fps_count;
	int count_reset;
	float fps_limit;
	double current_time;
	double last_update;

};
DaoType *daox_type_window = NULL;

DaoxWindow* DaoxWindow_New()
{
	DaoxWindow *self = (DaoxWindow*) dao_calloc(1,sizeof(DaoxWindow));
	DaoCstruct_Init( (DaoCstruct*)self, daox_type_window );
	self->title = DString_NewChars( "Untitled" );
	self->width = 300.0;
	self->height = 200.0;
	self->fps_limit = 30.0;
	return self;
}
void DaoxWindow_Delete( DaoxWindow *self )
{
	DString_Delete( self->title );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}


void DaoxCamera_Zoom( DaoxCamera *camera, int zoomin )
{
	DaoxVector3D pos = DaoxCamera_GetPosition( camera );
	float dist = DaoxVector3D_Dist( & pos, & camera->viewTarget );
	float delta = dist / 4;
	if( zoomin ){
		camera->fovAngle *= 0.9;
	}else{
		float scaled = 1.1 * camera->fovAngle;
		float interpolated = 0.9 * camera->fovAngle + 0.1 * 179;
		camera->fovAngle = scaled < interpolated ? scaled : interpolated;
	}   
}
void DaoxWindow_KeyCallback( GLFWwindow *window, int key, int scode, int action, int mods )
{
	DaoxWindow *self = (DaoxWindow*) glfwGetWindowUserPointer( window );
	DaoxCanvas *canvas = (DaoxCanvas*) DaoValue_CastCstruct( self->model, daox_type_canvas );
	DaoxScene *scene = (DaoxScene*) DaoValue_CastCstruct( self->model, daox_type_scene );

	if( key == GLFW_KEY_ESCAPE ){
		self->visible = 0;
		return;
	}
	if( canvas ){
	}else if( scene ){
		DaoxCamera *camera = scene->camera;
		DaoxVector3D pos = DaoxCamera_GetPosition( camera );
		float dist = DaoxVector3D_Dist( & pos, & camera->viewTarget );
		float delta = dist / 8;
		float dx = 0.0;
		float dy = 0.0;
		float dz = 0.0;
		switch( key ){
		case GLFW_KEY_EQUAL :
			if( mods & GLFW_MOD_SHIFT ) DaoxCamera_Zoom( camera, 1 );
			break;
		case GLFW_KEY_MINUS :
			DaoxCamera_Zoom( camera, 0 );
			break;
		case GLFW_KEY_UP :
		case GLFW_KEY_DOWN :
		case GLFW_KEY_LEFT :
		case GLFW_KEY_RIGHT :
			switch( key ){
			case GLFW_KEY_UP    : dy = + delta; break;
			case GLFW_KEY_DOWN  : dy = - delta; break;
			case GLFW_KEY_LEFT  : dx = - delta; break;
			case GLFW_KEY_RIGHT : dx = + delta; break;
			}
			DaoxCamera_MoveByXYZ( camera, dx, dy, dz );
		}
	}
}


static void WIN_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxWindow *self = DaoxWindow_New();
	DString_Assign( self->title, p[2]->xString.value );
	self->width = p[0]->xInteger.value;
	self->height = p[1]->xInteger.value;
	self->handle = glfwCreateWindow( self->width, self->height, self->title->chars, NULL, NULL);
	if( self->handle == NULL ) DaoProcess_RaiseError( proc, NULL, "Failed to create window" );
	glfwMakeContextCurrent( self->handle );
	glfwSetWindowUserPointer( self->handle, self );
	glfwSetKeyCallback( self->handle, DaoxWindow_KeyCallback );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void WIN_Show( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxWindow *self = (DaoxWindow*) p[0];
	DaoxCanvas *canvas = (DaoxCanvas*) DaoValue_CastCstruct( p[1], daox_type_canvas );
	DaoxScene *scene = (DaoxScene*) DaoValue_CastCstruct( p[1], daox_type_scene );
	int entry;

	if( canvas != NULL && self->painter == NULL ){
		self->painter = DaoxPainter_New();
		DaoGC_IncRC( (DaoValue*) self->painter );
	}
	if( scene != NULL && self->renderer == NULL ){
		self->renderer = DaoxRenderer_New();
		DaoGC_IncRC( (DaoValue*) self->renderer );
	}
	self->model = p[1];
	self->visible = 1;

	glfwMakeContextCurrent( self->handle );
	while( self->visible && ! glfwWindowShouldClose( self->handle ) ){
		if( canvas ) DaoxPainter_Paint( self->painter, canvas, canvas->viewport );
		if( scene )  DaoxRenderer_Render( self->renderer, scene, scene->camera );
		glfwSwapBuffers( self->handle );
		glfwPollEvents();
	}
}
static void WIN_Hide( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxWindow *self = (DaoxWindow*) p[0];
	self->visible = 0;
}

static DaoFuncItem DaoxWindowMeths[]=
{
	{ WIN_New,   "Window( width = 300, height = 200, title = '' )" },
	{ WIN_Show,  "Show( self: Window, canvas: Canvas, fps = 30, test_fps = false )" },
	{ WIN_Show,  "Show( self: Window, scene: Scene, fps = 30, test_fps = false )" },
	{ WIN_Hide,  "Hide( self: Window )" },
	{ NULL, NULL }
};

DaoTypeBase DaoxWindow_Typer =
{
	"Window", NULL, NULL, (DaoFuncItem*) DaoxWindowMeths, { NULL }, { NULL },
	(FuncPtrDel)DaoxWindow_Delete, NULL
};



DAO_DLL int DaoWindow_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns )
{
	dao_vmspace_graphics = vmSpace;
	if( glfwInit() == 0 ) return 1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef MACOSX
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	daox_type_window = DaoNamespace_WrapType( ns, & DaoxWindow_Typer, 0 );
	return 0;
}
