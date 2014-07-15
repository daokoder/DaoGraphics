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

#include "dao_scene.h"


#if defined(__APPLE__)
//#  include <OpenGL/gl.h>
#  include <OpenGL/gl3.h>
#  include <GLUT/glut.h>
#else
//#  include <GL/gl.h>
#  include <GL/gl3.h>
#  include <GL/glut.h>
#endif





void DaoxTexture_glFreeTexture( DaoxTexture *self );

DaoxTexture* DaoxTexture_New()
{
	DaoxTexture *self = (DaoxTexture*) dao_malloc( sizeof(DaoxTexture) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_texture );
	self->tid = 0;
	self->image = NULL;
	self->file = DString_New(1);
	return self;
}
void DaoxTexture_Delete( DaoxTexture *self )
{
	if( self->tid ) DaoxTexture_glFreeTexture( self );
	DaoCstruct_Free( (DaoCstruct*) self );
	DaoGC_DecRC( (DaoValue*) self->image );
	DString_Delete( self->file );
	dao_free( self );
}
void DaoxTexture_SetImage( DaoxTexture *self, DaoxImage *image )
{
	if( self->tid ) DaoxTexture_glFreeTexture( self );
	GC_Assign( & self->image, image );
}
void DaoxTexture_LoadImage( DaoxTexture *self, const char *file )
{
	DaoxImage *image = self->image;
	int ok = 0;
	if( image == NULL || image->refCount > 1 ){
		image = DaoxImage_New();
		DaoxTexture_SetImage( self, image );
	}
	if( ok == 0 ) ok = DaoxImage_LoadPNG( self->image, file );
	if( ok == 0 ) ok = DaoxImage_LoadBMP( self->image, file );
}
void DaoxTexture_glFreeTexture( DaoxTexture *self )
{
	GLuint tid = self->tid;
	if( tid == 0 ) return;
	glDeleteTextures( 1, & tid );
	self->tid = 0;
}
void DaoxTexture_glInitTexture( DaoxTexture *self )
{
	uchar_t *data;
	int W, H;
	GLuint tid = 0;

	if( self->tid ) return;
	if( self->file->size ){
		if( self->image == NULL || self->image->imageData == NULL ){
			DaoxTexture_LoadImage( self, self->file->chars );
		}
	}
	if( self->image == NULL ) return;
	data = self->image->imageData;
	W = self->image->width;
	H = self->image->height;

	if( W == 0 || H == 0 ) return;

	glGenTextures( 1, & tid );
	self->tid = tid;

	glBindTexture(GL_TEXTURE_2D, self->tid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	if( self->image->depth == DAOX_IMAGE_BIT24 ){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}else if( self->image->depth == DAOX_IMAGE_BIT32 ){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}





DaoxMaterial* DaoxMaterial_New()
{
	DaoxMaterial *self = (DaoxMaterial*) dao_calloc( 1, sizeof(DaoxMaterial) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_material );
	self->ambient = daox_black_color;
	self->diffuse = daox_black_color;
	self->specular = daox_black_color;
	self->emission = daox_black_color;
	self->name = DString_New(1);
	return self;
}
void DaoxMaterial_Delete( DaoxMaterial *self )
{
	DString_Delete( self->name );
	DaoGC_DecRC( (DaoValue*) self->texture1 );
	DaoGC_DecRC( (DaoValue*) self->texture2 );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}
void DaoxMaterial_CopyFrom( DaoxMaterial *self, DaoxMaterial *other )
{
	/* Do not copy name! */
	memcpy( & self->ambient, & other->ambient, 4*sizeof(DaoxColor) );
	memcpy( & self->lighting, & other->lighting, 6*sizeof(uint_t) );
	GC_Assign( & self->texture1, other->texture1 );
	GC_Assign( & self->texture2, other->texture2 );
}
void DaoxMaterial_SetTexture( DaoxMaterial *self, DaoxTexture *texture, int which )
{
	switch( which ){
	case 1 : GC_Assign( & self->texture1, texture ); break;
	case 2 : GC_Assign( & self->texture2, texture ); break;
	}
}





void DaoxViewFrustum_Normalize( DaoxViewFrustum *self )
{
	DaoxVector3D vector;

	self->viewDirection = DaoxVector3D_Normalize( & self->viewDirection ); 
	self->topLeftEdge = DaoxVector3D_Normalize( & self->topLeftEdge ); 
	self->topRightEdge = DaoxVector3D_Normalize( & self->topRightEdge ); 
	self->bottomLeftEdge = DaoxVector3D_Normalize( & self->bottomLeftEdge ); 
	self->bottomRightEdge = DaoxVector3D_Normalize( & self->bottomRightEdge ); 
	self->leftPlaneNorm = DaoxVector3D_Normalize( & self->leftPlaneNorm ); 
	self->rightPlaneNorm = DaoxVector3D_Normalize( & self->rightPlaneNorm ); 
	self->topPlaneNorm = DaoxVector3D_Normalize( & self->topPlaneNorm ); 
	self->bottomPlaneNorm = DaoxVector3D_Normalize( & self->bottomPlaneNorm ); 

	vector = DaoxVector3D_Scale( & self->viewDirection, self->near + 0.11 );
	self->axisOrigin = DaoxVector3D_Add( & self->cameraPosition, & vector );
}
/*
// View direction: -z;
// Up direction:    y;
// Right direction: x;
*/
void DaoxViewFrustum_Init( DaoxViewFrustum *self, DaoxCamera *camera )
{
	DaoxVector3D cameraPosition = {0.0, 0.0, 0.0};
	DaoxVector3D viewDirection = {0.0, 0.0, -1.0};
	DaoxVector3D nearViewCenter = {0.0, 0.0, 0.0};
	DaoxVector3D farViewCenter = {0.0, 0.0, 0.0};
	DaoxVector3D topLeft = cameraPosition;
	DaoxVector3D topRight = cameraPosition;
	DaoxVector3D bottomLeft = cameraPosition;
	DaoxVector3D bottomRight = cameraPosition;
	DaoxVector3D leftPlaneNorm;
	DaoxVector3D rightPlaneNorm;
	DaoxVector3D topPlaneNorm;
	DaoxVector3D bottomPlaneNorm;
	DaoxMatrix4D objectToWorld = DaoxSceneNode_GetWorldTransform( & camera->base );
	float xtan = tan( 0.5 * camera->fovAngle * M_PI / 180.0 );

	self->right = camera->nearPlane * xtan;
	self->left = - self->right;
	self->top = self->right / camera->aspectRatio;
	self->bottom = - self->top;
	self->near = camera->nearPlane;
	self->far = camera->farPlane;

	topLeft.x = self->left;
	topLeft.y = self->top;
	topLeft.z = - camera->nearPlane;

	topRight.x = self->right;
	topRight.y = self->top;
	topRight.z = - camera->nearPlane;

	bottomLeft.x = self->left;
	bottomLeft.y = self->bottom;
	bottomLeft.z = - camera->nearPlane;

	bottomRight.x = self->right;
	bottomRight.y = self->bottom;
	bottomRight.z = - camera->nearPlane;

	nearViewCenter.z = - camera->nearPlane;
	farViewCenter.z = - camera->farPlane;

	leftPlaneNorm = DaoxVector3D_Cross( & topLeft, & bottomLeft );
	rightPlaneNorm = DaoxVector3D_Cross( & bottomRight, & topRight );
	topPlaneNorm = DaoxVector3D_Cross( & topRight, & topLeft );
	bottomPlaneNorm = DaoxVector3D_Cross( & bottomLeft, & bottomRight );

	self->cameraPosition = DaoxMatrix4D_MulVector( & objectToWorld, & cameraPosition, 1.0 );
	self->nearViewCenter = DaoxMatrix4D_MulVector( & objectToWorld, & nearViewCenter, 1.0 );
	self->farViewCenter = DaoxMatrix4D_MulVector( & objectToWorld, & farViewCenter, 1.0 );
	self->viewDirection = DaoxMatrix4D_MulVector( & objectToWorld, & viewDirection, 0.0 );
	self->topLeftEdge = DaoxMatrix4D_MulVector( & objectToWorld, & topLeft, 0.0 );
	self->topRightEdge = DaoxMatrix4D_MulVector( & objectToWorld, & topRight, 0.0 );
	self->bottomLeftEdge = DaoxMatrix4D_MulVector( & objectToWorld, & bottomLeft, 0.0 );
	self->bottomRightEdge = DaoxMatrix4D_MulVector( & objectToWorld, & bottomRight, 0.0 );
	self->leftPlaneNorm = DaoxMatrix4D_MulVector( & objectToWorld, & leftPlaneNorm, 0.0 );
	self->rightPlaneNorm = DaoxMatrix4D_MulVector( & objectToWorld, & rightPlaneNorm, 0.0 );
	self->topPlaneNorm = DaoxMatrix4D_MulVector( & objectToWorld, & topPlaneNorm, 0.0 );
	self->bottomPlaneNorm = DaoxMatrix4D_MulVector( & objectToWorld, & bottomPlaneNorm, 0.0 );
	DaoxViewFrustum_Normalize( self );
}
DaoxViewFrustum DaoxViewFrustum_Transform( DaoxViewFrustum *self, DaoxMatrix4D *matrix )
{
	DaoxViewFrustum frustum = *self;
	frustum.cameraPosition = DaoxMatrix4D_MulVector( matrix, & self->cameraPosition, 1.0 );
	frustum.nearViewCenter = DaoxMatrix4D_MulVector( matrix, & self->nearViewCenter, 1.0 );
	frustum.farViewCenter = DaoxMatrix4D_MulVector( matrix, & self->farViewCenter, 1.0 );
	frustum.viewDirection = DaoxMatrix4D_MulVector( matrix, & self->viewDirection, 0.0 );
	frustum.topLeftEdge = DaoxMatrix4D_MulVector( matrix, & self->topLeftEdge, 0.0 );
	frustum.topRightEdge = DaoxMatrix4D_MulVector( matrix, & self->topRightEdge, 0.0 );
	frustum.bottomLeftEdge = DaoxMatrix4D_MulVector( matrix, & self->bottomLeftEdge, 0.0 );
	frustum.bottomRightEdge = DaoxMatrix4D_MulVector( matrix, & self->bottomRightEdge, 0.0 );
	frustum.leftPlaneNorm = DaoxMatrix4D_MulVector( matrix, & self->leftPlaneNorm, 0.0 );
	frustum.rightPlaneNorm = DaoxMatrix4D_MulVector( matrix, & self->rightPlaneNorm, 0.0 );
	frustum.topPlaneNorm = DaoxMatrix4D_MulVector( matrix, & self->topPlaneNorm, 0.0 );
	frustum.bottomPlaneNorm = DaoxMatrix4D_MulVector( matrix, & self->bottomPlaneNorm, 0.0 );
	DaoxViewFrustum_Normalize( & frustum );
	return frustum;
}
double DaoxViewFrustum_Difference( DaoxViewFrustum *self, DaoxViewFrustum *other )
{
	double d1, d2, d3, d4, d5, max = 0.0;
	d1 = DaoxVector3D_Difference( & self->cameraPosition, & other->cameraPosition);
	d2 = DaoxVector3D_Difference( & self->leftPlaneNorm, & other->leftPlaneNorm);
	d3 = DaoxVector3D_Difference( & self->rightPlaneNorm, & other->rightPlaneNorm);
	d4 = DaoxVector3D_Difference( & self->topPlaneNorm, & other->topPlaneNorm);
	d5 = DaoxVector3D_Difference( & self->bottomPlaneNorm, & other->bottomPlaneNorm);
	if( d1 > max ) max = d1;
	if( d2 > max ) max = d2;
	if( d3 > max ) max = d3;
	if( d4 > max ) max = d4;
	if( d5 > max ) max = d5;
	return max;
}
void DaoxViewFrustum_Print( DaoxViewFrustum *self )
{
	printf( "DaoxViewFrustum:\n" );
	DaoxVector3D_Print( & self->cameraPosition );
	DaoxVector3D_Print( & self->leftPlaneNorm );
	DaoxVector3D_Print( & self->rightPlaneNorm );
	DaoxVector3D_Print( & self->topPlaneNorm );
	DaoxVector3D_Print( & self->bottomPlaneNorm );
}


/*
// This function take a plane (passing "point" with normal "norm"),
// and a line segment (connecting P1 and P2) as parameter. It returns:
// -1, if the line segment P1P2 lies in the negative side of the plane;
// 0,  if the line segment cross the plane;
// 1,  if the line segment lies in the positive side of the plane;
*/
static int CheckLine( DaoxVector3D point, DaoxVector3D norm, DaoxVector3D P1, DaoxVector3D P2 )
{
	double dot1, dot2;
	P1 = DaoxVector3D_Sub( & P1, & point );
	P2 = DaoxVector3D_Sub( & P2, & point );
	dot1 = DaoxVector3D_Dot( & P1, & norm );
	dot2 = DaoxVector3D_Dot( & P2, & norm );
	if( dot1 * dot2 <= EPSILON ) return 0;
	if( dot1 < 0.0 ) return -1;
	return 1;
}
static int CheckBox( DaoxVector3D point, DaoxVector3D norm, DaoxOBBox3D *box )
{
	DaoxVector3D dX, dY, dZ, XY, YZ, ZX, XYZ;
	int C1, C2, C3, C4;

	dX = DaoxVector3D_Sub( & box->X, & box->O );
	dY = DaoxVector3D_Sub( & box->Y, & box->O );
	dZ = DaoxVector3D_Sub( & box->Z, & box->O );

	XY = DaoxVector3D_Add( & box->X, & dY );
	C1 = CheckLine( point, norm, box->Z, XY );
	if( C1 == 0 ) return 0;

	YZ = DaoxVector3D_Add( & box->Y, & dZ );
	C2 = CheckLine( point, norm, box->X, YZ );
	if( C2 == 0 ) return 0;

	ZX = DaoxVector3D_Add( & box->Z, & dX );
	C3 = CheckLine( point, norm, box->Y, ZX );
	if( C3 == 0 ) return 0;

	XYZ = DaoxVector3D_Add( & ZX, & dY );
	C4 = CheckLine( point, norm, box->O, XYZ );

	return C4;
}
int  DaoxViewFrustum_SphereCheck( DaoxViewFrustum *self, DaoxOBBox3D *box )
{
	DaoxVector3D C = DaoxVector3D_Sub( & box->C, & self->cameraPosition );
	double D0 = DaoxVector3D_Dot( & C, & self->viewDirection );
	double D1, D2, D3, D4, margin = box->R + EPSILON;
	if( D0 > (self->far + margin) ) return -1;
	if( D0 < (self->near - margin) ) return -1;
	if( (D1 = DaoxVector3D_Dot( & C, & self->leftPlaneNorm )) > margin ) return -1;
	if( (D2 = DaoxVector3D_Dot( & C, & self->rightPlaneNorm )) > margin ) return -1;
	if( (D3 = DaoxVector3D_Dot( & C, & self->topPlaneNorm )) > margin ) return -1;
	if( (D4 = DaoxVector3D_Dot( & C, & self->bottomPlaneNorm )) > margin ) return -1;
	if( D0 > (self->near + margin) && D0 < (self->far - margin) && D1 < -margin && D2 < -margin && D3 < -margin && D4 < -margin ) return 1;
	return 0;
}
int  DaoxViewFrustum_Visible( DaoxViewFrustum *self, DaoxOBBox3D *box )
{
	int C1, C2, C3, C4, C5, C6;
	int C0 = DaoxViewFrustum_SphereCheck( self, box );
	if( C0 != 0 ) return C0;

	C1 = C2 = C3 = C4 = C5 = C6 = 0;
	if( (C1 = CheckBox( self->nearViewCenter, self->viewDirection, box )) < 0 ) return -1;
	if( (C2 = CheckBox( self->farViewCenter, self->viewDirection, box )) > 0 ) return -1;
	if( (C3 = CheckBox( self->cameraPosition, self->leftPlaneNorm, box )) > 0 ) return -1;
	if( (C4 = CheckBox( self->cameraPosition, self->rightPlaneNorm, box )) > 0 ) return -1;
	if( (C5 = CheckBox( self->cameraPosition, self->topPlaneNorm, box )) > 0 ) return -1;
	if( (C6 = CheckBox( self->cameraPosition, self->bottomPlaneNorm, box )) > 0 ) return -1;
	if( C1 >= 0 && C2 <= 0 && C3 <= 0 && C4 <= 0 && C5 <= 0 && C6 <= 0 ) return 1;
	return 0;
}




typedef struct DaoxPointable  DaoxPointable;

struct DaoxPointable
{
	DaoxSceneNode  base;
	DaoxVector3D   targetPosition;
};




void DaoxSceneNode_Init( DaoxSceneNode *self, DaoType *type )
{
	DaoCstruct_Init( (DaoCstruct*) self, type );
	self->renderable = 0;
	self->parent = NULL;
	self->children = DList_New( DAO_DATA_VALUE );
	self->transform = DaoxMatrix4D_Identity();
}
void DaoxSceneNode_Free( DaoxSceneNode *self )
{
	DaoCstruct_Free( (DaoCstruct*) self );
	DaoGC_DecRC( (DaoValue*) self->parent );
	DList_Delete( self->children );
}
DaoxSceneNode* DaoxSceneNode_New()
{
	DaoxSceneNode *self = (DaoxSceneNode*) dao_calloc( 1, sizeof(DaoxSceneNode) );
	DaoxSceneNode_Init( self, daox_type_scene_node );
	return self;
}
void DaoxSceneNode_Delete( DaoxSceneNode *self )
{
	DaoxSceneNode_Free( self );
	dao_free( self );
}
void DaoxSceneNode_ApplyTransform( DaoxSceneNode *self, DaoxMatrix4D mat )
{
	self->transform = DaoxMatrix4D_MulMatrix( & mat, & self->transform );
}
void DaoxSceneNode_MoveByXYZ( DaoxSceneNode *self, float dx, float dy, float dz )
{
	DaoxMatrix4D M = DaoxMatrix4D_Translation( dx, dy, dz );
	DaoxSceneNode_ApplyTransform( self, M );
}
void DaoxSceneNode_MoveXYZ( DaoxSceneNode *self, float x, float y, float z )
{
	DaoxMatrix4D M = self->transform;
	DaoxSceneNode_MoveByXYZ( self, x - M.B1, y - M.B2, z - M.B3 );
}
void DaoxSceneNode_MoveBy( DaoxSceneNode *self, DaoxVector3D delta )
{
	DaoxSceneNode_MoveByXYZ( self, delta.x, delta.y, delta.z );
}
void DaoxSceneNode_Move( DaoxSceneNode *self, DaoxVector3D pos )
{
	DaoxSceneNode_MoveXYZ( self, pos.x, pos.y, pos.z );
}
DaoxMatrix4D DaoxSceneNode_GetWorldTransform( DaoxSceneNode *self )
{
	DaoxMatrix4D transform = self->transform;
	DaoxSceneNode *node = self;
	while( node->parent ){
		node = node->parent;
		transform = DaoxMatrix4D_MulMatrix( & node->transform, & transform );
	}
	return transform;
}
DaoxVector3D DaoxSceneNode_GetWorldPosition( DaoxSceneNode *self )
{
	DaoxMatrix4D transform = DaoxSceneNode_GetWorldTransform( self );
	DaoxVector3D vec = {0.0, 0.0, 0.0};
	return DaoxMatrix4D_MulVector( & transform, & vec, 1.0 );
}
void DaoxSceneNode_AddChild( DaoxSceneNode *self, DaoxSceneNode *child )
{
	GC_Assign( & child->parent, self );
	DList_Append( self->children, child );
}





void DaoxPointable_PointAt( DaoxPointable *self, DaoxVector3D pos )
{
	double angle;
	DaoxVector3D axis, newPointDirection;
	DaoxVector3D sourcePosition = {0.0,0.0,0.0};
	DaoxVector3D pointDirection = {0.0,0.0,-1.0};
	DaoxMatrix4D rotation = DaoxMatrix4D_RotationOnly( & self->base.transform );
	DaoxMatrix4D translation = DaoxMatrix4D_TranslationOnly( & self->base.transform );
	DaoxMatrix4D rot;

	if( DaoxVector3D_Dist( & self->targetPosition, & pos ) < 1E-9 ) return;
	self->targetPosition = pos;
	sourcePosition = DaoxMatrix4D_MulVector( & self->base.transform, & sourcePosition, 1.0 );
	pointDirection = DaoxMatrix4D_MulVector( & rotation, & pointDirection, 1.0 );
	newPointDirection = DaoxVector3D_Sub( & pos, & sourcePosition );

	pointDirection = DaoxVector3D_Normalize( & pointDirection );
	newPointDirection = DaoxVector3D_Normalize( & newPointDirection );
	axis = DaoxVector3D_Cross( & newPointDirection, & pointDirection );
	if( DaoxVector3D_Norm2( & axis ) > 1E-9 ){
		angle = DaoxVector3D_Angle( & newPointDirection, & pointDirection );
		rot = DaoxMatrix4D_AxisRotation( axis, -angle );
		rot = DaoxMatrix4D_MulMatrix( & rot, & rotation );
	}else{
		rot = rotation;
	}
	self->base.transform = DaoxMatrix4D_MulMatrix( & translation, & rot );
}
void DaoxPointable_PointAtXYZ( DaoxPointable *self, float x, float y, float z )
{
	DaoxVector3D pos;
	pos.x = x;
	pos.y = y;
	pos.z = z;
	DaoxPointable_PointAt( self, pos );
}
void DaoxPointable_Move( DaoxPointable *self, DaoxVector3D pos )
{
	double angle;
	DaoxVector3D axis, newPointDirection;
	DaoxVector3D sourcePosition = {0.0,0.0,0.0};
	DaoxVector3D pointDirection = {0.0,0.0,-1.0};
	DaoxMatrix4D rotation = DaoxMatrix4D_RotationOnly( & self->base.transform );
	DaoxMatrix4D translation = DaoxMatrix4D_Translation( pos.x, pos.y, pos.z );
	DaoxMatrix4D rot;

	sourcePosition = DaoxMatrix4D_MulVector( & self->base.transform, & sourcePosition, 1.0 );

	if( DaoxVector3D_Dist( & sourcePosition, & pos ) < 1E-9 ) return;

	pointDirection = DaoxMatrix4D_MulVector( & rotation, & pointDirection, 1.0 );
	newPointDirection = DaoxVector3D_Sub( & self->targetPosition, & pos );

	pointDirection = DaoxVector3D_Normalize( & pointDirection );
	newPointDirection = DaoxVector3D_Normalize( & newPointDirection );
	axis = DaoxVector3D_Cross( & newPointDirection, & pointDirection );
	if( DaoxVector3D_Norm2( & axis ) > 1E-9 ){
		angle = DaoxVector3D_Angle( & newPointDirection, & pointDirection );
		rot = DaoxMatrix4D_AxisRotation( axis, -angle );
		rot = DaoxMatrix4D_MulMatrix( & rot, & rotation );
	}else{
		rot = rotation;
	}
	self->base.transform = DaoxMatrix4D_MulMatrix( & translation, & rot );

	DaoxPointable_PointAt( self, self->targetPosition );
}
void DaoxPointable_Move2( DaoxPointable *self, DaoxVector3D pos )
{
	DaoxSceneNode_Move( (DaoxSceneNode*) self, pos );
	DaoxPointable_PointAt( self, self->targetPosition );
}
void DaoxPointable_MoveXYZ( DaoxPointable *self, float x, float y, float z )
{
	DaoxSceneNode_MoveXYZ( (DaoxSceneNode*) self, x, y, z );
	DaoxPointable_PointAt( self, self->targetPosition );
}
void DaoxPointable_MoveBy( DaoxPointable *self, DaoxVector3D delta )
{
	DaoxSceneNode_MoveBy( (DaoxSceneNode*) self, delta );
	DaoxPointable_PointAt( self, self->targetPosition );
}
void DaoxPointable_MoveByXYZ( DaoxPointable *self, float dx, float dy, float dz )
{
	DaoxSceneNode_MoveByXYZ( (DaoxSceneNode*) self, dx, dy, dz );
	DaoxPointable_PointAt( self, self->targetPosition );
}





DaoxCamera* DaoxCamera_New()
{
	DaoxCamera *self = (DaoxCamera*) dao_calloc( 1, sizeof(DaoxCamera) );
	DaoxSceneNode_Init( (DaoxSceneNode*) self, daox_type_camera );
	self->viewTarget.x =  0;
	self->viewTarget.y =  0;
	self->viewTarget.z = -1;
	self->fovAngle = 90.0;
	self->aspectRatio = 1.2;
	self->nearPlane = 0.5;
	self->farPlane = 100.0;
	return self;
}
void DaoxCamera_Delete( DaoxCamera *self )
{
	DaoxSceneNode_Free( (DaoxSceneNode*) self );
	dao_free( self );
}
void DaoxCamera_CopyFrom( DaoxCamera *self, DaoxCamera *other )
{
	self->viewTarget = other->viewTarget;
	self->fovAngle = other->fovAngle;
	self->aspectRatio = other->aspectRatio;
	self->nearPlane = other->nearPlane;
	self->farPlane = other->farPlane;
}

DaoxVector3D DaoxCamera_GetPosition( DaoxCamera *self )
{
	DaoxVector3D position = {0.0,0.0,0.0};
	return DaoxMatrix4D_MulVector( & self->base.transform, & position, 1.0 );
}
DaoxVector3D DaoxCamera_GetDirection( DaoxCamera *self, DaoxVector3D *localDirection )
{
	DaoxMatrix4D rotation = DaoxMatrix4D_RotationOnly( & self->base.transform );
	return DaoxMatrix4D_MulVector( & rotation, localDirection, 1.0 );
}
DaoxVector3D DaoxCamera_GetViewDirection( DaoxCamera *self )
{
	DaoxVector3D direction = {0.0,0.0,-1.0};
	return DaoxCamera_GetDirection( self, & direction );
}
DaoxVector3D DaoxCamera_GetUpDirection( DaoxCamera *self )
{
	DaoxVector3D direction = {0.0,1.0,0.0};
	return DaoxCamera_GetDirection( self, & direction );
}
DaoxVector3D DaoxCamera_GetRightDirection( DaoxCamera *self )
{
	DaoxVector3D direction = {1.0,0.0,0.0};
	return DaoxCamera_GetDirection( self, & direction );
}

void DaoxCamera_Move( DaoxCamera *self, DaoxVector3D pos )
{
	DaoxPointable_Move( (DaoxPointable*) self, pos );
}
void DaoxCamera_MoveXYZ( DaoxCamera *self, float x, float y, float z )
{
	DaoxPointable_MoveXYZ( (DaoxPointable*) self, x, y, z );
}
void DaoxCamera_MoveBy( DaoxCamera *self, DaoxVector3D delta )
{
	DaoxVector3D position = DaoxMatrix4D_MulVector( & self->base.transform, & delta, 1.0 );;
	DaoxPointable_Move( (DaoxPointable*) self, position );
	DaoxPointable_PointAt( (DaoxPointable*) self, self->viewTarget );
}
void DaoxCamera_MoveByXYZ( DaoxCamera *self, float dx, float dy, float dz )
{
	DaoxVector3D delta;
	delta.x = dx;
	delta.y = dy;
	delta.z = dz;
	DaoxCamera_MoveBy( self, delta );
}
void DaoxCamera_RotateBy( DaoxCamera *self, float alpha )
{
	DaoxVector3D cameraPosition = DaoxCamera_GetPosition( self );
	DaoxVector3D cameraDirection = DaoxCamera_GetViewDirection( self );
	DaoxMatrix4D rotation = DaoxMatrix4D_RotationOnly( & self->base.transform );
	DaoxMatrix4D translation = DaoxMatrix4D_TranslationOnly( & self->base.transform );
	DaoxMatrix4D rot = DaoxMatrix4D_AxisRotation( cameraDirection, alpha );
	rot = DaoxMatrix4D_MulMatrix( & rot, & rotation );
	self->base.transform = DaoxMatrix4D_MulMatrix( & translation, & rot );
}
void DaoxCamera_Orient( DaoxCamera *self, int xyz )
{
	DaoxVector3D xaxis = { 1.0, 0.0, 0.0 };
	DaoxVector3D yaxis = { 0.0, 1.0, 0.0 };
	DaoxVector3D zaxis = { 0.0, 0.0, 1.0 };
	DaoxVector3D upaxis = xyz == 1 ? xaxis : (xyz == 2 ? yaxis : zaxis);
	DaoxVector3D cameraUp = DaoxCamera_GetUpDirection( self );
	DaoxVector3D cameraDirection = DaoxCamera_GetViewDirection( self );
	DaoxVector3D projection = DaoxVector3D_ProjectToPlane( & upaxis, & cameraDirection );
	DaoxVector3D cross = DaoxVector3D_Cross( & cameraUp, & projection );
	float angle = DaoxVector3D_Angle( & cameraUp, & projection );
	float dot = DaoxVector3D_Dot( & cross, & cameraDirection );
	DaoxCamera_RotateBy( self, 2*M_PI - angle );
	printf( "DaoxCamera_AdjustToHorizon: %g %g %i\n", dot, angle, xyz );
	DaoxVector3D_Print( & cameraDirection );
	DaoxVector3D_Print( & cameraUp );
	DaoxVector3D_Print( & projection );
	DaoxVector3D_Print( & cross );
}
void DaoxCamera_LookAt( DaoxCamera *self, DaoxVector3D pos )
{
	DaoxPointable_PointAt( (DaoxPointable*) self, pos );
}
void DaoxCamera_LookAtXYZ( DaoxCamera *self, float x, float y, float z )
{
	DaoxPointable_PointAtXYZ( (DaoxPointable*) self, x, y, z );
}





DaoxLight* DaoxLight_New()
{
	DaoxLight *self = (DaoxLight*) dao_calloc( 1, sizeof(DaoxLight) );
	DaoxSceneNode_Init( (DaoxSceneNode*) self, daox_type_light );
	self->targetPosition.x = 0;
	self->targetPosition.y = -1E16;
	self->targetPosition.z = 0;
	self->intensity.alpha = 1.0;
	return self;
}
void DaoxLight_Delete( DaoxLight *self )
{
	DaoxSceneNode_Free( (DaoxSceneNode*) self );
	dao_free( self );
}
void DaoxLight_CopyFrom( DaoxLight *self, DaoxLight *other )
{
	self->targetPosition = other->targetPosition;
	self->intensity = other->intensity;
}

void DaoxLight_Move( DaoxLight *self, DaoxVector3D pos )
{
	DaoxPointable_Move( (DaoxPointable*) self, pos );
}
void DaoxLight_PointAt( DaoxLight *self, DaoxVector3D pos )
{
	DaoxPointable_PointAt( (DaoxPointable*) self, pos );
}
void DaoxLight_MoveXYZ( DaoxLight *self, float x, float y, float z )
{
	DaoxPointable_MoveXYZ( (DaoxPointable*) self, x, y, z );
}
void DaoxLight_PointAtXYZ( DaoxLight *self, float x, float y, float z )
{
	DaoxPointable_PointAtXYZ( (DaoxPointable*) self, x, y, z );
}





DaoxModel* DaoxModel_New()
{
	DaoxModel *self = (DaoxModel*) dao_calloc( 1, sizeof(DaoxModel) );
	DaoxSceneNode_Init( (DaoxSceneNode*) self, daox_type_model );
	return self;
}
static void DList_DeleteVector3DLists( DList *self )
{
	daoint i;
	for(i=0; i<self->size; ++i) DArray_Delete( self->items.pArray[i] );
	DList_Delete( self );
}
void DaoxModel_Delete( DaoxModel *self )
{
	DaoxSceneNode_Free( (DaoxSceneNode*) self );
	dao_free( self );
}
void DaoxModel_SetMesh( DaoxModel *self, DaoxMesh *mesh )
{
	GC_Assign( & self->mesh, mesh );
	self->base.obbox = mesh->obbox;
}



DaoxTerrainPoint* DaoxTerrainPoint_New()
{
	DaoxTerrainPoint *self = (DaoxTerrainPoint*) dao_calloc( 1, sizeof(DaoxTerrainPoint) );
	return self;
}
void DaoxTerrainPoint_Delete( DaoxTerrainPoint *self )
{
	dao_free( self );
}


DaoxTerrainPatch* DaoxTerrainPatch_New()
{
	DaoxTerrainPatch *self = (DaoxTerrainPatch*) dao_calloc( 1, sizeof(DaoxTerrainPatch) );
	return self;
}
void DaoxTerrainPatch_Delete( DaoxTerrainPatch *self )
{
	dao_free( self );
}
double DaoxTerrainPatch_Size( DaoxTerrainPatch *self )
{
	if( self->points[0] == NULL || self->points[1] == NULL ) return 0.0;
	return DaoxVector3D_Dist( & self->points[0]->pos, & self->points[1]->pos );
}




DaoxTerrain* DaoxTerrain_New()
{
	DaoxTerrain *self = (DaoxTerrain*) dao_calloc( 1, sizeof(DaoxTerrain) );
	DaoxSceneNode_Init( (DaoxSceneNode*) self, daox_type_terrain );
	self->triangles = DArray_New( sizeof(DaoxTriangle) );
	self->vertices = DList_New(0);
	self->pointCache = DList_New(0);
	self->patchCache = DList_New(0);
	self->width = self->length = self->height = 1.0;
	self->depth = 1.0;
	return self;
}
void DaoxTerrain_CachePatch( DaoxTerrain *self, DaoxTerrainPatch *patch )
{
	int i;
	for(i=0; i<4; ++i){
		if( patch->subs[i] == NULL ) continue;
		DaoxTerrain_CachePatch( self, patch->subs[i] );
		DList_Append( self->patchCache, patch->subs[i] );
		patch->subs[i] = NULL;
	}
	if( patch->center ){
		patch->center->refCount -= 1;
		if( patch->center->refCount == 0 ) DList_Append( self->pointCache, patch->center );
	}
	for(i=0; i<4; ++i){
		DaoxTerrainPoint *point = patch->points[i];
		if( point == NULL ) continue;
		point->refCount -= 1;
		if( point->refCount == 0 ) DList_Append( self->pointCache, point );
		patch->points[i] = NULL;
	}
}
void DaoxTerrain_Delete( DaoxTerrain *self )
{
	int i;
	if( self->patchTree ) DaoxTerrain_CachePatch( self, self->patchTree );
	for(i=0; i<self->pointCache->size; ++i) dao_free( self->pointCache->items.pVoid[i] );
	for(i=0; i<self->patchCache->size; ++i) dao_free( self->patchCache->items.pVoid[i] );
	DArray_Delete( self->triangles );
	DList_Delete( self->vertices );
	DList_Delete( self->pointCache );
	DList_Delete( self->patchCache );
	DaoxSceneNode_Free( (DaoxSceneNode*) self );
	dao_free( self );
}

void DaoxTerrain_SetSize( DaoxTerrain *self, float width, float length, float height )
{
	self->width = width;
	self->length = length;
	self->height = height;
}
void DaoxTerrain_SetHeightmap( DaoxTerrain *self, DaoxImage *heightmap )
{
	GC_Assign( & self->heightmap, heightmap );
}
void DaoxTerrain_SetMaterial( DaoxTerrain *self, DaoxMaterial *material )
{
	GC_Assign( & self->material, material );
}

float DaoxTerrain_GetPixel( DaoxTerrain *self, float x, float y )
{
	uchar_t *imgdata = self->heightmap->imageData;
	float hmx = ((x + 0.5*self->width) * (self->heightmap->width-1)) / self->width;
	float hmy = ((y + 0.5*self->length) * (self->heightmap->height-1)) / self->length;
	int hmx1 = (int) hmx, hmx2 = hmx1 + (hmx > hmx1);
	int hmy1 = (int) hmy, hmy2 = hmy1 + (hmy > hmy1);
	int widthStep = self->heightmap->widthStep;
	int pixelBytes = 1 + self->heightmap->depth;
	int pix0 = *(imgdata + hmy1 * widthStep + hmx1 * pixelBytes);
	int pix1 = *(imgdata + hmy1 * widthStep + hmx2 * pixelBytes);
	int pix2 = *(imgdata + hmy2 * widthStep + hmx2 * pixelBytes);
	int pix3 = *(imgdata + hmy2 * widthStep + hmx1 * pixelBytes);
	float alpha = hmx - hmx1, alpha2 = 1.0 - alpha;
	float beta = hmy - hmy1, beta2 = 1.0 - beta;
	float pix = alpha2 * beta2 * pix0 + alpha * beta * pix2;
	pix += alpha * beta2 * pix1 + alpha2 * beta * pix3;
	return pix;
}
float DaoxTerrain_GetHeight( DaoxTerrain *self, float x, float y )
{
	float pix = DaoxTerrain_GetPixel( self, x, y );
	return (self->height * pix) / 255.0;
}
static DaoxTerrainPoint* DaoxTerrain_MakePoint( DaoxTerrain *self, float x, float y )
{
	DaoxTerrainPoint *point = NULL;

	if( self->pointCache->size ){
		point = (DaoxTerrainPoint*) DList_PopBack( self->pointCache );
	}else{
		point = DaoxTerrainPoint_New();
	}
	point->activeIndex = 0;
	point->divLevel = 0;
	point->refCount = 0;
	point->pos.x = x;
	point->pos.y = y;
	point->pos.z = DaoxTerrain_GetHeight( self, x, y );
	point->norm.x = point->norm.y = point->norm.z = 0.0;
	point->east = point->west = NULL;
	point->south = point->north = NULL;
	return point;
}
static DaoxTerrainPatch* DaoxTerrain_MakePatch( DaoxTerrain *self, DaoxTerrainPoint *points[4], DList *pointList, int level )
{
	int i, j, ihmx, ihmy;
	int mapWidth = self->heightmap->width;
	int mapHeight = self->heightmap->height;
	int pixelBytes = 1 + self->heightmap->depth;
	float left = points[2]->pos.x;
	float right = points[0]->pos.x;
	float top = points[0]->pos.y;
	float bottom = points[2]->pos.y;
	float xcenter = 0.5*(left + right);
	float ycenter = 0.5*(top + bottom);
	float min = 0.0, max = 0.0;
	float hmx1, hmx2, hmy1, hmy2;
	float H0, H1, H2, H3;
	DaoxTerrainPatch *patch = NULL;

	if( self->patchCache->size ){
		patch = (DaoxTerrainPatch*) DList_PopBack( self->patchCache );
	}else{
		patch = DaoxTerrainPatch_New();
	}
	patch->center = DaoxTerrain_MakePoint( self, xcenter, ycenter );
	patch->center->divLevel = level + 1;
	patch->center->refCount += 1;
	DList_Append( pointList, patch->center );
	for(i=0; i<4; ++i){
		patch->subs[i] = NULL;
		patch->points[i] = points[i];
		points[i]->refCount += 1;
	}

	H0 = DaoxTerrain_GetPixel( self, points[0]->pos.x, points[0]->pos.y );
	H1 = DaoxTerrain_GetPixel( self, points[1]->pos.x, points[1]->pos.y );
	H2 = DaoxTerrain_GetPixel( self, points[2]->pos.x, points[2]->pos.y );
	H3 = DaoxTerrain_GetPixel( self, points[3]->pos.x, points[3]->pos.y );

	hmx1 = ((points[2]->pos.x + 0.5*self->width) * (mapWidth-1)) / self->width;
	hmx2 = ((points[3]->pos.x + 0.5*self->width) * (mapWidth-1)) / self->width;
	hmy1 = ((points[2]->pos.y + 0.5*self->length) * (mapHeight-1)) / self->length;
	hmy2 = ((points[1]->pos.y + 0.5*self->length) * (mapHeight-1)) / self->length;
	ihmx = (int) hmx1;
	ihmy = (int) hmy1;
	for(j=ihmy + (hmy1>ihmy); j<=hmy2; ++j){
		uchar_t *imgdata = self->heightmap->imageData + j * self->heightmap->widthStep;
		float beta = (j - hmy1) / (hmy2 - hmy1 + EPSILON);
		float beta2 = 1.0 - beta;
		for(i=ihmx + (hmx1>ihmx); i<=hmx2; ++i){
			float pixel = imgdata[ i * pixelBytes ];
			float alpha = (i - hmx1) / (hmx2 - hmx1 + EPSILON);
			float alpha2 = 1.0 - alpha;
			float expected;
			expected  = alpha2 * beta2 * H0 + alpha * beta * H2;
			expected += alpha * beta2 * H1 + alpha2 * beta * H3;
			pixel -= expected;
			if( pixel < min ) min = pixel;
			if( pixel > max ) max = pixel;
		}
	}
	patch->minHeight = (self->height * min) / 255.0;
	patch->maxHeight = (self->height * max) / 255.0;
	return patch;
}
void DaoxTerrain_Refine( DaoxTerrain *self, DaoxTerrainPatch *patch, float maxHeightDiff, DList *pointList )
{
	int i, j, level = patch->center->divLevel;
	float left = patch->points[2]->pos.x;
	float right = patch->points[0]->pos.x;
	float top = patch->points[0]->pos.y;
	float bottom = patch->points[2]->pos.y;
	float xcenter = patch->center->pos.x;
	float ycenter = patch->center->pos.y;
	DaoxTerrainPoint *mid01, *mid12, *mid23, *mid30, *points[4];
	DaoxTerrainPoint *center = patch->center;

	mid01 = patch->points[0];
	while( mid01 != patch->points[1] && mid01->divLevel != level ) mid01 = mid01->west;
	if( mid01 == patch->points[1] ){
		mid01 = DaoxTerrain_MakePoint( self, xcenter, top );
		mid01->divLevel = level;
		mid01->east = patch->points[0];
		mid01->west = patch->points[1];
		patch->points[0]->west = mid01;
		patch->points[1]->east = mid01;
		DList_Append( pointList, mid01 );
	}

	mid12 = patch->points[1];
	while( mid12 != patch->points[2] && mid12->divLevel != level ) mid12 = mid12->south;
	if( mid12 == patch->points[2] ){
		mid12 = DaoxTerrain_MakePoint( self, left, ycenter );
		mid12->divLevel = level;
		mid12->north = patch->points[1];
		mid12->south = patch->points[2];
		patch->points[1]->south = mid12;
		patch->points[2]->north = mid12;
		DList_Append( pointList, mid12 );
	}

	mid23 = patch->points[2];
	while( mid23 != patch->points[3] && mid23->divLevel != level ) mid23 = mid23->east;
	if( mid23 == patch->points[3] ){
		mid23 = DaoxTerrain_MakePoint( self, xcenter, bottom );
		mid23->divLevel = level;
		mid23->west = patch->points[2];
		mid23->east = patch->points[3];
		patch->points[2]->east = mid23;
		patch->points[3]->west = mid23;
		DList_Append( pointList, mid23 );
	}

	mid30 = patch->points[3];
	while( mid30 != patch->points[0] && mid30->divLevel != level ) mid30 = mid30->north;
	if( mid30 == patch->points[0] ){
		mid30 = DaoxTerrain_MakePoint( self, right, ycenter );
		mid30->divLevel = level;
		mid30->south = patch->points[3];
		mid30->north = patch->points[0];
		patch->points[3]->north = mid30;
		patch->points[0]->south = mid30;
		DList_Append( pointList, mid30 );
	}
	center->east = mid30;
	center->west = mid12;
	center->south = mid23;
	center->north = mid01;
	mid12->east = center;
	mid30->west = center;
	mid01->south = center;
	mid23->north = center;

	points[0] = patch->points[0];
	points[1] = mid01;
	points[2] = center;
	points[3] = mid30;
	patch->subs[0] = DaoxTerrain_MakePatch( self, points, pointList, level );

	points[0] = mid01;
	points[1] = patch->points[1];
	points[2] = mid12;
	points[3] = center;
	patch->subs[1] = DaoxTerrain_MakePatch( self, points, pointList, level );

	points[0] = center;
	points[1] = mid12;
	points[2] = patch->points[2];
	points[3] = mid23;
	patch->subs[2] = DaoxTerrain_MakePatch( self, points, pointList, level );

	points[0] = mid30;
	points[1] = center;
	points[2] = mid23;
	points[3] = patch->points[3];
	patch->subs[3] = DaoxTerrain_MakePatch( self, points, pointList, level );

	for(i=0; i<4; ++i){
		DaoxTerrainPatch *sub = patch->subs[i];
		double width = DaoxTerrainPatch_Size( sub );
		double diff = sub->maxHeight - sub->minHeight;
		if( diff > maxHeightDiff || diff > (width+1.0) ){
			DaoxTerrain_Refine( self, sub, maxHeightDiff, pointList );
		}
	}
}
static void DaoxTerrain_UpdatePointNeighbors( DaoxTerrain *self, DaoxTerrainPatch *patch )
{
	int i, j, level = patch->center->divLevel;
	DaoxTerrainPoint *mid01, *mid12, *mid23, *mid30;
	DaoxTerrainPoint *center = patch->center;

	if( patch->subs[0] ){
		for(i=0; i<4; ++i) DaoxTerrain_UpdatePointNeighbors( self, patch->subs[i] );
		return;
	}

	mid01 = patch->points[0];
	while( mid01 != patch->points[1] && mid01->divLevel != level ) mid01 = mid01->west;
	if( mid01 != patch->points[1] ){
		center->north = mid01;
		mid01->south = center;
	}

	mid12 = patch->points[1];
	while( mid12 != patch->points[2] && mid12->divLevel != level ) mid12 = mid12->south;
	if( mid12 != patch->points[2] ){
		center->west = mid12;
		mid12->east = center;
	}

	mid23 = patch->points[2];
	while( mid23 != patch->points[3] && mid23->divLevel != level ) mid23 = mid23->east;
	if( mid23 != patch->points[3] ){
		center->south = mid23;
		mid23->north = center;
	}

	mid30 = patch->points[3];
	while( mid30 != patch->points[0] && mid30->divLevel != level ) mid30 = mid30->north;
	if( mid30 != patch->points[0] ){
		center->east = mid30;
		mid30->west = center;
	}
}
static void DaoxTerrain_UpdateNormals( DaoxTerrain *self, DaoxTerrainPoint *center, DaoxTerrainPoint *first, DaoxTerrainPoint *second )
{
	DaoxVector3D A = DaoxVector3D_Sub( & first->pos, & center->pos );
	DaoxVector3D B = DaoxVector3D_Sub( & second->pos, & first->pos );
	DaoxVector3D N = DaoxVector3D_Cross( & A, & B );
	N = DaoxVector3D_Normalize( & N );
	center->norm = DaoxVector3D_Add( & center->norm, & N );
	first->norm = DaoxVector3D_Add( & first->norm, & N );
	second->norm = DaoxVector3D_Add( & second->norm, & N );
}
static void DaoxTerrain_ComputeNormals( DaoxTerrain *self, DaoxTerrainPatch *patch )
{
	int i;
	if( patch->subs[0] == NULL ){
		DaoxTerrainPoint *first;
		for(first = patch->points[0]; first != patch->points[1]; first = first->west){
			DaoxTerrain_UpdateNormals( self, patch->center, first, first->west );
		}
		for(first = patch->points[1]; first != patch->points[2]; first = first->south){
			DaoxTerrain_UpdateNormals( self, patch->center, first, first->south );
		}
		for(first = patch->points[2]; first != patch->points[3]; first = first->east){
			DaoxTerrain_UpdateNormals( self, patch->center, first, first->east );
		}
		for(first = patch->points[3]; first != patch->points[0]; first = first->north){
			DaoxTerrain_UpdateNormals( self, patch->center, first, first->north );
		}
		return;
	}
	for(i=0; i<4; ++i) DaoxTerrain_ComputeNormals( self, patch->subs[i] );
}
void DaoxTerrain_Rebuild( DaoxTerrain *self, float maxHeightDiff )
{
	int i;
	float width = self->width;
	float length = self->length;
	DaoxTerrainPatch *patch = self->patchTree;
	DaoxTerrainPoint *first, *base;
	DaoxTerrainPoint *points[4];
	DList *pointList;

	if( self->patchTree ){
		/* Caching base points: */
		DList_Append( self->pointCache, self->baseCenter );
		for(first = patch->points[0]; first != patch->points[1]; first = first->west){
			if( first->west->north ) DList_Append( self->pointCache, first->west->north );
		}
		for(first = patch->points[1]; first != patch->points[2]; first = first->south){
			if( first->south->west ) DList_Append( self->pointCache, first->south->west );
		}
		for(first = patch->points[2]; first != patch->points[3]; first = first->east){
			if( first->east->south ) DList_Append( self->pointCache, first->east->south );
		}
		for(first = patch->points[3]; first != patch->points[0]; first = first->north){
			if( first->north->east ) DList_Append( self->pointCache, first->north->east );
		}
		DaoxTerrain_CachePatch( self, self->patchTree );
	}
	printf( "Patch Cache: %i\n", self->patchCache->size );
	
	self->patchTree = NULL;
	if( self->heightmap == NULL ) return;

	points[0] = DaoxTerrain_MakePoint( self, +0.5*width, +0.5*length );
	points[1] = DaoxTerrain_MakePoint( self, -0.5*width, +0.5*length );
	points[2] = DaoxTerrain_MakePoint( self, -0.5*width, -0.5*length );
	points[3] = DaoxTerrain_MakePoint( self, +0.5*width, -0.5*length );

	points[1]->east = points[0];
	points[0]->west = points[1];

	points[2]->east = points[3];
	points[3]->west = points[2];

	points[0]->south = points[3];
	points[3]->north = points[0];

	points[1]->south = points[2];
	points[2]->north = points[1];

	pointList = DList_New(0);
	self->patchTree = patch = DaoxTerrain_MakePatch( self, points, pointList, 0 );
	self->baseCenter = DaoxTerrain_MakePoint( self, 0.0, 0.0 );
	self->baseCenter->pos.z = - self->depth;

	for(i=0; i<4; ++i) DList_Append( pointList, points[i] );
	if( (self->patchTree->maxHeight - self->patchTree->minHeight) > maxHeightDiff ){
		DaoxTerrain_Refine( self, self->patchTree, maxHeightDiff, pointList );
	}
	DaoxTerrain_UpdatePointNeighbors( self, self->patchTree );

	printf( "tree: %p\n", self->patchTree );
	/* Making base points: */
	for(first = patch->points[0]; first != patch->points[1]; first = first->west){
		base = DaoxTerrain_MakePoint( self, 0.0, 0.0 );
		base->pos = first->west->pos;
		base->pos.z = - self->depth;
		first->west->north = base;
		//printf( "make base point: %p %p %p\n", first, first->west, base );
	}
	for(first = patch->points[1]; first != patch->points[2]; first = first->south){
		base = DaoxTerrain_MakePoint( self, 0.0, 0.0 );
		base->pos = first->south->pos;
		base->pos.z = - self->depth;
		first->south->west = base;
	}
	for(first = patch->points[2]; first != patch->points[3]; first = first->east){
		base = DaoxTerrain_MakePoint( self, 0.0, 0.0 );
		base->pos = first->east->pos;
		base->pos.z = - self->depth;
		first->east->south = base;
	}
	for(first = patch->points[3]; first != patch->points[0]; first = first->north){
		base = DaoxTerrain_MakePoint( self, 0.0, 0.0 );
		base->pos = first->north->pos;
		base->pos.z = - self->depth;
		first->north->east = base;
	}
	DaoxTerrain_ComputeNormals( self, self->patchTree );
	for(i=0; i<pointList->size; ++i){
		DaoxTerrainPoint *point = (DaoxTerrainPoint*) pointList->items.pVoid[i];
		point->norm = DaoxVector3D_Normalize( & point->norm );
	}
	DList_Delete( pointList );
}
static void DaoxTerrain_TryActivatePoint( DaoxTerrain *self, DaoxTerrainPoint *point )
{
	if( point->activeIndex == 0 ){
		DList_Append( self->vertices, point );
		point->activeIndex = self->vertices->size;
	}
}
static void DaoxTerrain_MakeTriangle( DaoxTerrain *self, DaoxTerrainPoint *center, DaoxTerrainPoint *first, DaoxTerrainPoint *second )
{
	DaoxTriangle triangle;
	triangle.index[0] = center->activeIndex - 1;
	triangle.index[1] = first->activeIndex - 1;
	triangle.index[2] = second->activeIndex - 1;
	DArray_PushTriangle( self->triangles, & triangle );
}
static void DaoxTerrain_MakeBaseTriangle( DaoxTerrain *self, DaoxTerrainPoint *first, DaoxTerrainPoint *second, DaoxTerrainPoint *baseFirst, DaoxTerrainPoint *baseSecond, DaoxTerrainPoint *baseCenter )
{
	DaoxTerrain_TryActivatePoint( self, baseFirst );
	DaoxTerrain_TryActivatePoint( self, baseSecond );
	DaoxTerrain_MakeTriangle( self, baseFirst, second, first );
	DaoxTerrain_MakeTriangle( self, second, baseFirst, baseSecond );
	DaoxTerrain_MakeTriangle( self, baseCenter, baseSecond, baseFirst );
}
static int DaoxTerrain_PatchVisible( DaoxTerrain *self, DaoxTerrainPatch *patch, DaoxViewFrustum *frustum )
{
	DaoxOBBox3D obbox;
	DaoxMatrix4D terrainToWorld = DaoxSceneNode_GetWorldTransform( & self->base );
	float margin = 1.0; // XXX:

	if( frustum == NULL ) return 1;
	if( DaoxTerrainPatch_Size( patch ) < 0.01*(self->width + self->length) ) return 1;

	obbox.C = patch->center->pos;
	obbox.O = patch->points[2]->pos;
	obbox.X = patch->points[3]->pos;
	obbox.Y = patch->points[1]->pos;
	obbox.Z = patch->points[2]->pos;
	obbox.O.z += patch->minHeight - margin;
	obbox.X.z += patch->minHeight - margin;
	obbox.Y.z += patch->minHeight - margin;
	obbox.Z.z += patch->maxHeight + margin;
	obbox.C.z += 0.5*(patch->minHeight + patch->maxHeight);
	obbox.R = DaoxVector3D_Dist( & obbox.O, & obbox.C );
	obbox = DaoxOBBox3D_Transform( & obbox, & terrainToWorld );
	return DaoxViewFrustum_Visible( frustum, & obbox );
}
static int DaoxTerrain_PatchHasFineView( DaoxTerrain *self, DaoxTerrainPatch *patch, DaoxViewFrustum *frustum )
{
	DaoxMatrix4D toWorld = DaoxSceneNode_GetWorldTransform( & self->base );
	DaoxVector3D P0 = DaoxMatrix4D_MulVector( & toWorld, & patch->points[0]->pos, 1.0 );
	float mindist, viewDiff;
	int i;

	if( frustum == NULL ) return 0;
	mindist = DaoxVector3D_Dist( & P0, & frustum->cameraPosition );
	for(i=1; i<3; ++i){
		DaoxVector3D P = DaoxMatrix4D_MulVector( & toWorld, & patch->points[i]->pos, 1.0 );
		float dist = DaoxVector3D_Dist( & P, & frustum->cameraPosition );
		if( dist < mindist ) mindist = dist;
	}
	if( mindist <= frustum->near ) return 0;

	viewDiff = patch->maxHeight - patch->minHeight;
	viewDiff = frustum->ratio * frustum->near * viewDiff / mindist;
	return viewDiff < 5.0;
}
static void DaoxTerrain_ActivateVertices( DaoxTerrain *self, DaoxTerrainPatch *patch, DaoxViewFrustum *frustum )
{
	int i;

	patch->visible = DaoxTerrain_PatchVisible( self, patch, frustum ) >= 0;
	if( ! patch->visible ){
		/* For building coarse but complete mesh: */
		for(i=0; i<4; ++i) DaoxTerrain_TryActivatePoint( self, patch->points[i] );
		return;
	}

	DaoxTerrain_TryActivatePoint( self, patch->center );
	for(i=0; i<4; ++i) DaoxTerrain_TryActivatePoint( self, patch->points[i] );

	patch->smooth = DaoxTerrain_PatchHasFineView( self, patch, frustum );
	if( patch->subs[0] == NULL || patch->smooth ) return;

	for(i=0; i<4; ++i) DaoxTerrain_ActivateVertices( self, patch->subs[i], frustum );
}
static void DaoxTerrain_GenerateTriangles( DaoxTerrain *self, DaoxTerrainPatch *patch, DaoxViewFrustum *frustum )
{
	int i, level = patch->center->divLevel;

	if( ! patch->visible ){
		/* For building coarse but complete mesh: */
		DaoxTerrain_MakeTriangle( self, patch->points[0], patch->points[1], patch->points[2] );
		DaoxTerrain_MakeTriangle( self, patch->points[0], patch->points[2], patch->points[3] );
		return;
	}
	if( patch->subs[0] == NULL || patch->smooth ){
		int active1, active2, active3, active4;
		DaoxTerrainPoint *first, *second = NULL;
		DaoxTerrainPoint *east = patch->center->east;
		DaoxTerrainPoint *west = patch->center->west;
		DaoxTerrainPoint *south = patch->center->south;
		DaoxTerrainPoint *north = patch->center->north;

		while( east && east->divLevel != level ) east = east->east;
		while( west && west->divLevel != level ) west = west->west;
		while( south && south->divLevel != level ) south = south->south;
		while( north && north->divLevel != level ) north = north->north;
		active1 = east && east->activeIndex > 0;
		active2 = west && west->activeIndex > 0;
		active3 = south && south->activeIndex > 0;
		active4 = north && north->activeIndex > 0;

		if( active1 == 0 && active2 == 0 && active3 == 0 && active4 == 0 ){
			DaoxTerrain_MakeTriangle( self, patch->points[0], patch->points[1], patch->points[2] );
			DaoxTerrain_MakeTriangle( self, patch->points[0], patch->points[2], patch->points[3] );
			return;
		}

		for(first = patch->points[0]; first != patch->points[1]; first = second){
			second = first->west;
			while( second->activeIndex == 0 ) second = second->west;
			DaoxTerrain_MakeTriangle( self, patch->center, first, second );
		}
		for(first = patch->points[1]; first != patch->points[2]; first = second){
			second = first->south;
			while( second->activeIndex == 0 ) second = second->south;
			DaoxTerrain_MakeTriangle( self, patch->center, first, second );
		}
		for(first = patch->points[2]; first != patch->points[3]; first = second){
			second = first->east;
			while( second->activeIndex == 0 ) second = second->east;
			DaoxTerrain_MakeTriangle( self, patch->center, first, second );
		}
		for(first = patch->points[3]; first != patch->points[0]; first = second){
			second = first->north;
			while( second->activeIndex == 0 ) second = second->north;
			DaoxTerrain_MakeTriangle( self, patch->center, first, second );
		}
		return;
	}
	for(i=0; i<4; ++i) DaoxTerrain_GenerateTriangles( self, patch->subs[i], frustum );
}
void DaoxTerrain_UpdateView( DaoxTerrain *self, DaoxViewFrustum *frustum )
{
	DaoxTerrainPatch *patch = self->patchTree;
	DaoxTerrainPoint *first, *baseFirst, *baseSecond;
	DaoxTerrainPoint *baseCenter;
	daoint i, j;

	for(i=0; i<self->vertices->size; ++i){
		DaoxTerrainPoint *point = (DaoxTerrainPoint*) self->vertices->items.pVoid[i];
		point->activeIndex = 0;
	}
	self->vertices->size = 0;
	self->triangles->size = 0;

	if( self->patchTree == NULL ) DaoxTerrain_Rebuild( self, self->height / 16.0 );
	DaoxTerrain_ActivateVertices( self, self->patchTree, frustum );
	DaoxTerrain_GenerateTriangles( self, self->patchTree, frustum );

	patch = self->patchTree;
	baseCenter = self->baseCenter;
	DaoxTerrain_TryActivatePoint( self, self->baseCenter );

	printf( "tree: %p\n", self->patchTree );
	for(first = patch->points[0]; first != patch->points[1]; ){
		DaoxTerrainPoint *second = first->west;
		while( second->activeIndex == 0 ) second = second->west;
		baseFirst = first == patch->points[0] ? patch->points[0]->east : first->north;
		baseSecond = second->north;
		//printf( "use base point: %p %p %p\n", first, second, baseSecond );
		DaoxTerrain_MakeBaseTriangle( self, first, second, baseFirst, baseSecond, baseCenter );
		first = second;
	}
	for(first = patch->points[1]; first != patch->points[2]; ){
		DaoxTerrainPoint *second = first->south;
		while( second->activeIndex == 0 ) second = second->south;
		baseFirst = first == patch->points[1] ? patch->points[1]->north : first->west;
		baseSecond = second->west;
		DaoxTerrain_MakeBaseTriangle( self, first, second, baseFirst, baseSecond, baseCenter );
		first = second;
	}
	for(first = patch->points[2]; first != patch->points[3]; ){
		DaoxTerrainPoint *second = first->east;
		while( second->activeIndex == 0 ) second = second->east;
		baseFirst = first == patch->points[2] ? patch->points[2]->west : first->south;
		baseSecond = second->south;
		DaoxTerrain_MakeBaseTriangle( self, first, second, baseFirst, baseSecond, baseCenter );
		first = second;
	}
	for(first = patch->points[3]; first != patch->points[0]; ){
		DaoxTerrainPoint *second = first->north;
		while( second->activeIndex == 0 ) second = second->north;
		baseFirst = first == patch->points[3] ? patch->points[3]->south : first->east;
		baseSecond = second->east;
		DaoxTerrain_MakeBaseTriangle( self, first, second, baseFirst, baseSecond, baseCenter );
		first = second;
	}
}






DaoxScene* DaoxScene_New()
{
	int i;
	DaoxColor colors[6];
	DaoxMaterial *material;
	DaoxScene *self = (DaoxScene*) dao_malloc( sizeof(DaoxScene) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_scene );
	self->nodes = DList_New( DAO_DATA_VALUE );
	self->lights = DList_New(0);
	self->materialNames = DMap_New( DAO_DATA_STRING, 0 );
	self->textureNames = DMap_New( DAO_DATA_STRING, 0 );
	self->materials = DList_New( DAO_DATA_VALUE );
	self->textures = DList_New( DAO_DATA_VALUE );

	colors[0] = daox_black_color;
	colors[1] = daox_white_color;
	colors[2] = daox_red_color;
	colors[3] = daox_green_color;
	colors[4] = daox_blue_color;
	colors[5] = daox_gray_color;
	for(i=0; i<6; ++i){
		material = DaoxMaterial_New();
		material->ambient = material->diffuse = material->specular = colors[i];
		DList_Append( self->materials, material );
	}
	return self;
}
void DaoxScene_Delete( DaoxScene *self )
{
	printf( "DaoxScene_Delete: %p\n", self );
	DaoCstruct_Free( (DaoCstruct*) self );
	DList_Delete( self->nodes );
	DList_Delete( self->lights );
	DList_Delete( self->materials );
	DList_Delete( self->textures );
	DMap_Delete( self->materialNames );
	DMap_Delete( self->textureNames );
	dao_free( self );
}

void DaoxScene_AddNode( DaoxScene *self, DaoxSceneNode *node )
{
	DList_Append( self->nodes, node );
	if( node->ctype == daox_type_light ) DList_Append( self->lights, node );
}
void DaoxScene_AddMaterial( DaoxScene *self, DString *name, DaoxMaterial *material )
{
	MAP_Insert( self->materialNames, name, self->materials->size );
	DList_Append( self->materials, material );
}



