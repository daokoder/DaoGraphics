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
#include "daoVmspace.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"


typedef struct DaoxWindow DaoxWindow;

struct DaoxWindow
{
	DAO_CSTRUCT_COMMON;

	GLFWwindow *handle;

	DaoxContext  *context;
	DaoxPainter  *painter;
	DaoxRenderer *renderer;
	DaoxCanvas   *widget;
	DaoValue     *model;

	DString *title;

	float width;
	float height;
	int   visible;

	float cursorPosX;
	float cursorPosY;
	DaoxCanvasText *fpsLabel;
};
DaoType *daox_type_window = NULL;

DaoxWindow* DaoxWindow_New()
{
	DaoxWindow *self = (DaoxWindow*) dao_calloc(1,sizeof(DaoxWindow));
	DaoCstruct_Init( (DaoCstruct*)self, daox_type_window );
	self->context = DaoxContext_New();
	GC_IncRC( self->context );
	self->title = DString_NewChars( "Untitled" );
	self->width = 300.0;
	self->height = 200.0;
	return self;
}
void DaoxWindow_Delete( DaoxWindow *self )
{
	//if( self->handle ) glfwDestroyWindow( self->handle );
	DaoCstruct_Free( (DaoCstruct*) self );
	GC_DecRC( self->context );
	GC_DecRC( self->painter );
	GC_DecRC( self->renderer );
	GC_DecRC( self->widget );
	GC_DecRC( self->model );
	DString_Delete( self->title );
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
void DaoxCanvas_Zoom( DaoxCanvas *self, int zoomin )
{
	DaoxAABBox2D box = self->viewport;
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
	DaoxCanvas_SetViewport( self, box.left, box.right, box.bottom, box.top );
}
void DaoxWindow_KeyCallback( GLFWwindow *window, int key, int scode, int action, int mods )
{
	DaoxWindow *self = (DaoxWindow*) glfwGetWindowUserPointer( window );
	DaoxCanvas *canvas;
	DaoxScene *scene;

	if( self == NULL ) return;
	canvas = (DaoxCanvas*) DaoValue_CastCstruct( self->model, daox_type_canvas );
	scene = (DaoxScene*) DaoValue_CastCstruct( self->model, daox_type_scene );

	if( key == GLFW_KEY_ESCAPE && glfwGetKey( window, key ) == GLFW_RELEASE ){
		self->visible = 0;
		return;
	}
	if( canvas ){
		switch( key ){
		case GLFW_KEY_EQUAL :
			if( mods & GLFW_MOD_SHIFT ) DaoxCanvas_Zoom( canvas, 1 );
			break;
		case GLFW_KEY_MINUS :
			DaoxCanvas_Zoom( canvas, 0 );
			break;
		case GLFW_KEY_TAB :
			if( glfwGetKey( window, key ) == GLFW_PRESS ){
				self->painter->showMesh = ! self->painter->showMesh;
			}
			break;
		}
	}else if( scene && self->renderer->camera ){
		DaoxCamera *camera = self->renderer->camera;
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
			break;
		case GLFW_KEY_TAB :
			if( glfwGetKey( window, key ) == GLFW_PRESS ){
				self->renderer->showMesh = ! self->renderer->showMesh;
				self->renderer->showAxis = ! self->renderer->showAxis;
			}
			break;
		}
	}
}

void DaoxWindow_RotateCanvas( DaoxWindow *self, DaoxCanvas *canvas, int x, int y )
{
	DaoxMatrix3D rotate = {1.0,0.0,0.0,0.0,1.0,0.0};
	DaoxVector2D start, end, center = {0.0,0.0};
	double W2 = 0.5 * self->width;
	double H2 = 0.5 * self->height;
	double area, cosine, sine;

	start.x = self->cursorPosX - W2;
	start.y = self->cursorPosY - H2;
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
	DaoxMatrix3D_Multiply( & canvas->transform, rotate );
}
void DaoxWindow_MoveCanvas( DaoxWindow *self, DaoxCanvas *canvas, int x, int y )
{
	DaoxAABBox2D box = canvas->viewport;
	float xscale = (box.right - box.left) / self->width;
	float yscale = (box.top - box.bottom) / self->height;
	int dx = x - self->cursorPosX;
	int dy = y - self->cursorPosY;
	box.left   -= dx * xscale;
	box.right  -= dx * xscale;
	box.bottom += dy * yscale;
	box.top    += dy * yscale;
	// TODO: Use canvas bounding box to do it;
	//if( box.left > 0.9*self->width ) return;
	//if( box.right < 0.1*self->width ) return;
	//if( box.bottom > 0.9*self->height ) return;
	//if( box.top < 0.1*self->height ) return;
	DaoxCanvas_SetViewport( canvas, box.left, box.right, box.bottom, box.top );
}
void DaoxWindow_MourseBottonCallback( GLFWwindow *window, int botton, int state, int mod )
{
}
void DaoxWindow_CursorEnterCallback( GLFWwindow *window, int bl )
{
	DaoxWindow *self = (DaoxWindow*) glfwGetWindowUserPointer( window );
	double x = 0, y = 0;

	if( self == NULL ) return;

	glfwGetCursorPos( window, & x, & y );
	self->cursorPosX = x;
	self->cursorPosY = y;
}
void DaoxWindow_CursorMoveCallback( GLFWwindow *window, double x, double y )
{
	int left = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT );
	int right = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT );
	DaoxWindow *self = (DaoxWindow*) glfwGetWindowUserPointer( window );
	DaoxCanvas *canvas;
	DaoxScene *scene;

	if( self == NULL ) return;
	canvas = (DaoxCanvas*) DaoValue_CastCstruct( self->model, daox_type_canvas );
	scene = (DaoxScene*) DaoValue_CastCstruct( self->model, daox_type_scene );

	if( canvas ){
		if( left == GLFW_PRESS ){
			DaoxWindow_RotateCanvas( self, canvas, x, y );
		}else if( right == GLFW_PRESS ){
			DaoxWindow_MoveCanvas( self, canvas, x, y );
		}
	}else if( scene && self->renderer->camera ){
		DaoxCamera *camera = self->renderer->camera;
		DaoxVector3D pos = DaoxCamera_GetPosition( camera );
		float dist = DaoxVector3D_Dist( & pos, & camera->viewTarget );
		float delta = dist / 4;
		float dx = (x - self->cursorPosX) / self->width;
		float dy = (y - self->cursorPosY) / self->height;
		float dd = sqrt( dx*dx + dy*dy );
		if( dd < 0.1 ) DaoxCamera_MoveByXYZ( camera, delta * dx, delta * dy, 0 );
	}
	self->cursorPosX = x;
	self->cursorPosY = y;
}
void DaoxWindow_FocusCallback( GLFWwindow *window, int bl )
{
	DaoxWindow *self = (DaoxWindow*) glfwGetWindowUserPointer( window );
	double x = 0, y = 0;

	if( self == NULL ) return;

	glfwGetCursorPos( window, & x, & y );
	self->cursorPosX = x;
	self->cursorPosY = y;
}
void DaoxWindow_CloseCallback( GLFWwindow *window )
{
	DaoxWindow *self = (DaoxWindow*) glfwGetWindowUserPointer( window );

	if( self == NULL ) return;
	DaoxContext_Clear( self->context );
}

static void WIN_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxWindow *self = DaoxWindow_New();
	DString_Assign( self->title, p[2]->xString.value );
	self->width  = self->context->deviceWidth  = p[0]->xInteger.value;
	self->height = self->context->deviceHeight = p[1]->xInteger.value;
	self->handle = glfwCreateWindow( self->width, self->height, self->title->chars, NULL, NULL);
	if( self->handle == NULL ){
		DaoProcess_RaiseError( proc, NULL, "Failed to create window" );
		return;
	}
	glfwSetWindowUserPointer( self->handle, self );
	glfwSetWindowCloseCallback( self->handle, DaoxWindow_CloseCallback );
	glfwHideWindow( self->handle );
	glfwMakeContextCurrent( self->handle );
	DaoProcess_PutValue( proc, (DaoValue*) self );
}
static void WIN_Ctx( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxWindow *self = (DaoxWindow*) p[0];
	DaoProcess_PutValue( proc, (DaoValue*) self->context );
}
static void WIN_Show( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxWindow *self = (DaoxWindow*) p[0];
	DaoxCanvas *canvas = (DaoxCanvas*) DaoValue_CastCstruct( p[1], daox_type_canvas );
	DaoxScene *scene = (DaoxScene*) DaoValue_CastCstruct( p[1], daox_type_scene );
	DaoxFont *font = DaoxFont_GetDefault();
	int fpsLimit = p[2]->xInteger.value;
	int fpsTest = p[3]->xBoolean.value;
	double fpsTestTime = 0.0;
	double timeInterval = 0.0;
	double lastFrameStart = 0.0;
	float currentFPS = 0.0;
	size_t fpsCount = 0;
	char chars[32];

	if( fpsTest && self->widget == NULL ){
		char *fpsText = "FPS:       ";
		DaoxColor bgcolor = {0.0,0.0,0.0,0.0};
		DaoxBrush *brush;
		self->widget = DaoxCanvas_New( NULL );
		self->widget->viewport.right = self->width;
		self->widget->viewport.top = self->height;
		DaoGC_IncRC( (DaoValue*) self->widget );
		DaoxCanvas_SetBackground( self->widget, bgcolor );
		brush = DaoxCanvas_PushBrush( self->widget, 0 );
		brush->strokeColor.blue = 1.0;
		brush->fillColor.blue = 1.0;
		brush->fillColor.alpha = 1.0;
		brush->fontSize = 20;
		self->fpsLabel = DaoxCanvas_AddText( self->widget, fpsText, 10, self->height - 20, 0 );
	}
	if( self->painter == NULL && (canvas != NULL || self->widget != NULL) ){
		self->painter = DaoxPainter_New( self->context );
		DaoGC_IncRC( (DaoValue*) self->painter );
	}
	if( self->renderer == NULL && scene != NULL ){
		self->renderer = DaoxRenderer_New( self->context );
		DaoGC_IncRC( (DaoValue*) self->renderer );
	}
	if( canvas != NULL ){
		float dm = sqrt(self->width*self->width + self->height*self->height );
		float cx = 0.5*(canvas->viewport.left + canvas->viewport.right);
		float cy = 0.5*(canvas->viewport.top + canvas->viewport.bottom);
		float w = canvas->viewport.right - canvas->viewport.left;
		float h = canvas->viewport.top - canvas->viewport.bottom;
		float d = sqrt(w*w + h*h);
		w = 0.5 * self->width * d / dm;
		h = 0.5 * self->height * d / dm;
		canvas->viewport.left = cx - w;
		canvas->viewport.right = cx + w;
		canvas->viewport.bottom = cy - h;
		canvas->viewport.top = cy + h;
	}
	GC_Assign( & self->model, p[1] );
	self->visible = 1;

	glfwShowWindow( self->handle );
	glfwMakeContextCurrent( self->handle );
	glfwSetKeyCallback( self->handle, DaoxWindow_KeyCallback );
	glfwSetCursorPosCallback( self->handle, DaoxWindow_CursorMoveCallback );
	glfwSetCursorEnterCallback( self->handle, DaoxWindow_CursorEnterCallback );
	glfwSetWindowFocusCallback( self->handle, DaoxWindow_FocusCallback );

#ifdef SAVE_RENDERED_SCENE
	char name[50];
	int frame = 1;
	DaoxImage *image = DaoxImage_New();
	image->depth = DAOX_IMAGE_BIT32;
	DaoxImage_Resize( image, self->width, self->height );
#endif
	while( self->visible && ! glfwWindowShouldClose( self->handle ) ){
		double frameStartTime = 0.0;
		double frameEndTime = 0.0;
		frameStartTime = glfwGetTime();
		if( canvas ) DaoxPainter_Paint( self->painter, canvas, canvas->viewport );
		if( scene ){
#ifdef SAVE_RENDERED_SCENE
			DaoxScene_Update( scene, 1.0/30.0 );
			glReadBuffer( GL_BACK );
			glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
			glPixelStorei( GL_PACK_ROW_LENGTH, image->width );
			DaoxRenderer_Render( self->renderer, scene, scene->camera );
			glReadPixels( 0, 0, self->width, self->height, GL_RGBA, GL_UNSIGNED_BYTE, image->imageData );
			sprintf( name, "rama_attack_frame_%03i.png", frame );
			DaoxImage_SavePNG( image, name );
			frame += 1;
			if( frame > 155 ) break;
#else
			DaoxScene_Update( scene, frameStartTime - lastFrameStart );
			DaoxRenderer_Render( self->renderer, scene, scene->camera );
#endif
		}
		lastFrameStart = frameStartTime;
		if( fpsTest ){
			if( fpsCount % 10 == 0 ){
				int i, n = sprintf( chars, "%.1f", currentFPS );
				for(i=0; i<n; ++i){
					DaoxGlyph *glyph = DaoxFont_GetGlyph( font, chars[i] );
					DaoxCanvasNode *chnode = self->fpsLabel->children->items.pCanvasNode[i+5];
					DaoxPath *path = DaoxPathCache_FindPath( self->widget->pathCache, glyph->shape );
					GC_Assign( & chnode->path, path );
					DaoxCanvasNode_MarkDataChanged( chnode );
				}
			}
			DaoxPainter_Paint( self->painter, self->widget, self->widget->viewport );
		}
		glfwSwapBuffers( self->handle );
		glfwPollEvents();

		if( fpsTest == 0 ) continue;
		frameEndTime = glfwGetTime();
		timeInterval = frameEndTime - frameStartTime;
		if( timeInterval < 1.0/fpsLimit ) usleep( 1000000 * (1.0/fpsLimit -  timeInterval) );

		fpsCount += 1;
		currentFPS = fpsCount / (frameEndTime - fpsTestTime);
		if( frameEndTime > (fpsTestTime + 3) ){
			fpsTestTime = frameEndTime - 1.0;
			fpsCount = currentFPS; /* Frame count estimation in the past second; */
		}
	}
	glfwHideWindow( self->handle );
}
static void WIN_Hide( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxWindow *self = (DaoxWindow*) p[0];
	self->visible = 0;
	glfwHideWindow( self->handle );
}
static void WIN_Quit( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxWindow *self = (DaoxWindow*) p[0];
	self->visible = 0;
	glfwHideWindow( self->handle );
	DaoxContext_Clear( self->context );
}

static DaoFuncItem DaoxWindowMeths[]=
{
	{ WIN_New,   "Window( width = 300, height = 200, title = '' )" },
	{ WIN_Ctx,   "GetContext( self: Window ) => Context" },
	{ WIN_Show,  "Show( self: Window, canvas: Canvas, fps = 30, test_fps = false )" },
	{ WIN_Show,  "Show( self: Window, scene: Scene, fps = 30, test_fps = false )" },
	{ WIN_Hide,  "Hide( self: Window )" },
	{ WIN_Quit,  "Quit( self: Window )" },
	{ NULL, NULL }
};

static void DaoxWindow_GetGCFields( void *p, DList *values, DList *lists, DList *maps, int remove )
{
	DaoxWindow *self = (DaoxWindow*) p;
	DList_Append( values, self->context );
	if( self->painter ) DList_Append( values, self->painter );
	if( self->renderer ) DList_Append( values, self->renderer );
	if( self->widget ) DList_Append( values, self->widget );
	if( self->model ) DList_Append( values, self->model );
	if( remove ){
		self->context = NULL;
		self->painter = NULL;
		self->renderer = NULL;
		self->widget = NULL;
		self->model = NULL;
	}
}
DaoTypeBase DaoxWindow_Typer =
{
	"Window", NULL, NULL, (DaoFuncItem*) DaoxWindowMeths, { NULL }, { NULL },
	(FuncPtrDel)DaoxWindow_Delete, DaoxWindow_GetGCFields
};



DAO_DLL int DaoWindow_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns )
{
	dao_vmspace_graphics = vmSpace;
	if( glfwInit() == 0 ) return 1;
	glfwWindowHint(GLFW_VISIBLE, 0); 
	glfwWindowHint(GLFW_SAMPLES, 8); 
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1); 
	glfwWindowHint(GLFW_DEPTH_BITS, 24); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef MACOSX
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	daox_type_window = DaoNamespace_WrapType( ns, & DaoxWindow_Typer, 0 );
	return 0;
}
