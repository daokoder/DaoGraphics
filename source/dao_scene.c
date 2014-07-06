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
	if( image == NULL || image->refCount > 1 ){
		image = DaoxImage_New();
		DaoxTexture_SetImage( self, image );
	}
	DaoxImage_LoadBMP( self->image, file );
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
void DaoxMaterial_SetTexture( DaoxMaterial *self, DaoxTexture *texture )
{
	GC_Assign( & self->texture1, texture );
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
	float scale, scale2;

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

	scale = 1.0 / sqrt( DaoxVector3D_Norm2( & leftPlaneNorm ) );
	scale2 = 1.0 / sqrt( DaoxVector3D_Norm2( & topPlaneNorm ) );
	leftPlaneNorm = DaoxVector3D_Scale( & leftPlaneNorm, scale );
	rightPlaneNorm = DaoxVector3D_Scale( & rightPlaneNorm, scale );
	topPlaneNorm = DaoxVector3D_Scale( & topPlaneNorm, scale2 );
	bottomPlaneNorm = DaoxVector3D_Scale( & bottomPlaneNorm, scale2 );

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
	if( dot1 * dot2 <= 0.0 ) return 0;
	if( dot1 < 0.0 ) return -1;
	return 1;
}
static int CheckBox( DaoxVector3D point, DaoxVector3D norm, DaoxOBBox3D *box )
{
	DaoxVector3D dX, dY, dZ, XY, YZ, ZX, XYZ;
	int ch, check;

	dX = DaoxVector3D_Sub( & box->X, & box->O );
	dY = DaoxVector3D_Sub( & box->Y, & box->O );
	dZ = DaoxVector3D_Sub( & box->Z, & box->O );

	XY = DaoxVector3D_Add( & box->X, & dY );
	check = CheckLine( point, norm, box->Z, XY );
	if( check == 0 ) return 0;

	YZ = DaoxVector3D_Add( & box->Y, & dZ );
	ch = CheckLine( point, norm, box->X, YZ );
	if( (check * ch) <= 0 ) return 0;

	ZX = DaoxVector3D_Add( & box->Z, & dX );
	ch = CheckLine( point, norm, box->Y, ZX );
	if( (check * ch) <= 0 ) return 0;

	XYZ = DaoxVector3D_Add( & ZX, & dY );
	ch = CheckLine( point, norm, box->O, XYZ );
	if( (check * ch) <= 0 ) return 0;

	return check;
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
	if( C1 > 0 && C2 < 0 && C3 < 0 && C4 < 0 && C5 < 0 && C6 < 0 ) return 1;
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
	if( self->ctype == daox_type_model ){
		DaoxModel *model = (DaoxModel*) self;
		daoint i;
		for(i=0; i<model->points->size; ++i){
			DArray *points = model->points->items.pArray[i];
			DArray_Reset( points, 0 );
		}
	}
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
	double dot, angle, angle2;
	DaoxVector3D axis, newPointDirection;
	DaoxVector3D xaxis = {1.0,0.0,0.0};
	DaoxVector3D yaxis = {0.0,1.0,0.0};
	DaoxVector3D sourcePosition = {0.0,0.0,0.0};
	DaoxVector3D pointDirection = {0.0,0.0,-1.0};
	DaoxVector3D rightDirection = {1.0,0.0,0.0};
	DaoxMatrix4D rotation = DaoxMatrix4D_RotationOnly( & self->base.transform );
	DaoxMatrix4D translation = DaoxMatrix4D_TranslationOnly( & self->base.transform );
	DaoxMatrix4D rot, rot2;

	self->targetPosition = pos;
	sourcePosition = DaoxMatrix4D_MulVector( & self->base.transform, & sourcePosition, 1.0 );
	pointDirection = DaoxMatrix4D_MulVector( & rotation, & pointDirection, 1.0 );
	rightDirection = DaoxMatrix4D_MulVector( & rotation, & rightDirection, 1.0 );
	newPointDirection = DaoxVector3D_Sub( & pos, & sourcePosition );

	pointDirection = DaoxVector3D_Normalize( & pointDirection );
	newPointDirection = DaoxVector3D_Normalize( & newPointDirection );

	dot = DaoxVector3D_Dot( & newPointDirection, & yaxis );
	angle = DaoxVector3D_Angle( & newPointDirection, & yaxis );
	angle -= DaoxVector3D_Angle( & pointDirection, & yaxis );
	rot = DaoxMatrix4D_AxisRotation( rightDirection, - angle );
	if( fabs( fabs(dot) - 1.0 ) < EPSILON ) goto Final;

	pointDirection.y = newPointDirection.y = 0;
	angle2 = DaoxVector3D_Angle( & newPointDirection, & xaxis );
	angle2 -= DaoxVector3D_Angle( & pointDirection, & xaxis );
	rot2 = DaoxMatrix4D_AxisRotation( yaxis, angle2 );
	rot = DaoxMatrix4D_MulMatrix( & rot, & rot2 );
	rot = DaoxMatrix4D_MulMatrix( & rot, & rotation );

Final:
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

void DaoxCamera_Move( DaoxCamera *self, DaoxVector3D pos )
{
	DaoxPointable_Move( (DaoxPointable*) self, pos );
}
void DaoxCamera_MoveBy( DaoxCamera *self, DaoxVector3D delta )
{
	DaoxPointable_MoveBy( (DaoxPointable*) self, delta );
}
void DaoxCamera_LookAt( DaoxCamera *self, DaoxVector3D pos )
{
	DaoxPointable_PointAt( (DaoxPointable*) self, pos );
}
void DaoxCamera_MoveXYZ( DaoxCamera *self, float x, float y, float z )
{
	DaoxPointable_MoveXYZ( (DaoxPointable*) self, x, y, z );
}
void DaoxCamera_MoveByXYZ( DaoxCamera *self, float dx, float dy, float dz )
{
	DaoxPointable_MoveByXYZ( (DaoxPointable*) self, dx, dy, dz );
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
	self->points = DList_New(0);
	self->vnorms = DList_New(0);
	self->tnorms = DList_New(0);
	self->offsets = DArray_New( sizeof(int) );
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
	DArray_Delete( self->offsets );
	DList_DeleteVector3DLists( self->points );
	DList_DeleteVector3DLists( self->vnorms );
	DList_DeleteVector3DLists( self->tnorms );
	DaoxSceneNode_Free( (DaoxSceneNode*) self );
	dao_free( self );
}
void DaoxModel_SetMesh( DaoxModel *self, DaoxMesh *mesh )
{
	GC_Assign( & self->mesh, mesh );
	self->base.obbox = mesh->obbox;
}
void DaoxModel_TransformMesh( DaoxModel *self )
{
	DaoxMatrix4D transform = DaoxSceneNode_GetWorldTransform( & self->base );
	daoint i, j;

	if( self->mesh == NULL ) return;
	while( self->points->size < self->mesh->units->size ){
		DList_Append( self->points, DArray_New( sizeof(DaoxVector3D) ) );
		DList_Append( self->vnorms, DArray_New( sizeof(DaoxVector3D) ) );
		DList_Append( self->tnorms, DArray_New( sizeof(DaoxVector3D) ) );
	}
	for(i=0; i<self->mesh->units->size; ++i){
		DaoxMeshUnit *unit = self->mesh->units->items.pMeshUnit[i];
		DArray *points = self->points->items.pArray[i];
		DArray *vnorms = self->vnorms->items.pArray[i];
		DArray *tnorms = self->tnorms->items.pArray[i];
		if( points->size == unit->vertices->size
				&& vnorms->size == unit->vertices->size
				&& tnorms->size == unit->triangles->size ) continue;
		DArray_Reset( points, unit->vertices->size );
		DArray_Reset( vnorms, unit->vertices->size );
		DArray_Reset( tnorms, unit->triangles->size );
		for(j=0; j<unit->vertices->size; ++j){
			DaoxVertex *vertex = & unit->vertices->data.vertices[j];
			points->data.vectors3d[j] = DaoxMatrix4D_MulVector( & transform, & vertex->point, 1.0 );
			vnorms->data.vectors3d[j] = DaoxMatrix4D_MulVector( & transform, & vertex->norm, 0.0 );
		}
		for(j=0; j<unit->triangles->size; ++j){
			DaoxTriangle *triangle = & unit->triangles->data.triangles[j];
			DaoxVector3D *A = points->data.vectors3d + triangle->index[0];
			DaoxVector3D *B = points->data.vectors3d + triangle->index[1];
			DaoxVector3D *C = points->data.vectors3d + triangle->index[2];
			tnorms->data.vectors3d[j] = DaoxTriangle_Normal( A, B, C );
		}
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









typedef struct DaoxBinaryParser DaoxBinaryParser;

struct DaoxBinaryParser
{
	uchar_t  *source;
	uchar_t  *end;
	uchar_t  *error;
};

uint_t DaoxBinaryParser_DecodeUInt16LE( DaoxBinaryParser *self )
{
	uint_t res;
	if( (self->source + 2) > self->end ){
		self->source = self->error;
		return 0;
	}
	res = (self->source[1]<<8)|(self->source[0]);
	self->source += 2;
	return res;
}
uint_t DaoxBinaryParser_DecodeUInt32LE( DaoxBinaryParser *self )
{
	uint_t res;
	if( (self->source + 4) > self->end ){
		self->source = self->error;
		return 0;
	}
	res = (self->source[3]<<24)|(self->source[2]<<16)|(self->source[1]<<8)|(self->source[0]);
	self->source += 4;
	return res;
}
uint_t DaoxBinaryParser_DecodeUInt32BE( DaoxBinaryParser *self )
{
	uint_t res;
	if( (self->source + 4) > self->end ){
		self->source = self->error;
		return 0;
	}
	res = (self->source[0]<<24)|(self->source[1]<<16)|(self->source[2]<<8)|(self->source[3]);
	self->source += 4;
	return res;
}
/*
// IEEE 754 single-precision binary floating-point format:
//   sign(1)--exponent(8)--fraction(23)--
//   S  EEEEEEEE  FFFFFFFFFFFFFFFFFFFFFFF
//   31       22                        0
//
//   value = (-1)^S  *  ( 1 + \sigma_0^22 (b_i * 2^{-(23-i)}) )  *  2^{E-127}
*/
double DaoxBinaryParser_DecodeFloatLE( DaoxBinaryParser *self )
{
	double value = 1.0;
	uint_t bits = DaoxBinaryParser_DecodeUInt32LE( self );
	uint_t negative = bits & (1<<31);
	int i, expon;

	if( (self->source + 4) > self->end ){
		self->source = self->error;
		return 0;
	}
	if( bits == 0 ) return 0;
	if( bits == (0x7FF<<20) ) return INFINITY;
	if( bits == ((0x7FF<<20)|1) ) return NAN;

	bits = (bits<<1)>>1;
	expon = (bits>>23) - 127;
	for(i=0; i<23; ++i){
		if( (bits>>i)&0x1 ){
			int e = -(23-i);
			if( e >= 0 )
				value += pow( 2, e );
			else
				value += 1.0 / pow( 2, -e );
		}
	}
	if( expon >= 0 )
		value *= pow( 2, expon );
	else
		value /= pow( 2, -expon );
	if( negative ) value = -value;
	return value;
}
char* DaoxBinaryParser_DecodeString( DaoxBinaryParser *self )
{
	char *s = (char*) self->source;
	while( *self->source ) self->source += 1;
	self->source += 1;
	return s;
}
enum ColorType
{
	AMBIENT, DIFFUSE, SPECULAR
};
void DaoxScene_Parse3DS( DaoxScene *self, DString *source )
{
	DaoxColor tmpColor;
	DaoxVector3D vector;
	DaoxVector3D vector2;
	DaoxMatrix4D matrix;
	DaoxColor *color = NULL;
	DaoxImage *image = NULL;
	DaoxTexture *texture = NULL;
	DaoxMeshUnit *unit = NULL;
	DaoxMesh *mesh = NULL;
	DaoxModel *model = NULL;
	DaoxLight *light = NULL;
	DaoxCamera *camera = NULL;
	DaoxMaterial *material = NULL;
	DaoxBinaryParser parser0;
	DaoxBinaryParser *parser = & parser0;
	DArray *vertices = DArray_New( sizeof(DaoxVertex) );
	DArray *triangles = DArray_New( sizeof(DaoxTriangle) );
	DList *integers = DList_New(0);
	DList *chunkids = DList_New(0);
	DList *chunkends = DList_New(0);
	DList *faceCounts = DList_New(0);
	DString *string = DString_New(1);
	DNode *it = NULL;
	int colortype = AMBIENT;
	int i, j, k, m, n;
	float x, y, z, floats[16];
	float amount;

	parser->source = (uchar_t*) source->chars;
	parser->end = parser->source + source->size;
	parser->error = parser->end + 1;
	while( parser->source < parser->end ){
		uint_t chunkid = DaoxBinaryParser_DecodeUInt16LE( parser );
		uint_t chunklen = DaoxBinaryParser_DecodeUInt32LE( parser );
		uint_t parentid = (daoint) DList_Back( chunkids );
		if( parser->source >= parser->end ) break;
		while( chunkends->size && ((uchar_t*)DList_Back( chunkends )) < parser->source ){
			DList_PopBack( chunkends );
			DList_PopBack( chunkids );
		}
		DList_PushBack( chunkends, parser->source + (chunklen - 6) );
		DList_PushBack( chunkids, (void*)(size_t)chunkid );
		printf( "chunk: %6X -> %6X %6i\n", parentid, chunkid, chunklen );
		switch( chunkid ){
		case 0x4D4D : /* Main Chunk */
		case 0x3D3D : /* 3D Editor Chunk */
			break;
		case 0x4000 : /* Object Block */
			DaoxBinaryParser_DecodeString( parser );
			break;
		case 0x4100 : /* Triangular Mesh */
			if( model ){
				DaoxMesh_UpdateTree( mesh, 0 );
				DaoxMesh_ResetBoundingBox( mesh );
				DaoxModel_SetMesh( model, mesh );
				DaoxScene_AddNode( self, (DaoxSceneNode*) model );
			}
			printf( "mesh\n" );
			model = DaoxModel_New();
			mesh = DaoxMesh_New();
			break;
		case 0x4110 : /* Vertices List */
			m = DaoxBinaryParser_DecodeUInt16LE( parser );
			printf( "vertices: %i %p\n", m, unit );
			DList_Resize( faceCounts, m, 0 );
			DArray_Reset( vertices, 0 );
			for(i=0; i<m; ++i){
				DaoxVertex *vertex = DArray_PushVertex( vertices );
				vertex->point.x = DaoxBinaryParser_DecodeFloatLE( parser );
				vertex->point.y = DaoxBinaryParser_DecodeFloatLE( parser );
				vertex->point.z = DaoxBinaryParser_DecodeFloatLE( parser );
				vertex->norm.x = vertex->norm.y = vertex->norm.z = 0.0;
				vertex->texUV.x = vertex->texUV.y = 0.0;
				faceCounts->items.pInt[i] = 0;
				//printf( "%9.3f %9.3f %9.3f\n", vertex->point.x, vertex->point.y, vertex->point.z );
			}
			break;
		case 0x4120 : /* Faces Description */
			m = DaoxBinaryParser_DecodeUInt16LE( parser );
			DArray_Reset( triangles, 0 );
			printf( "triangles: %i %p\n", m, unit );
			for(i=0; i<m; ++i){
				DaoxTriangle *triangle = DArray_PushTriangle( triangles );
				DaoxVertex *A, *B, *C;
				DaoxVector3D AB, BC, N;
				float norm;
				triangle->index[0] = DaoxBinaryParser_DecodeUInt16LE( parser );
				triangle->index[1] = DaoxBinaryParser_DecodeUInt16LE( parser );
				triangle->index[2] = DaoxBinaryParser_DecodeUInt16LE( parser );
				DaoxBinaryParser_DecodeUInt16LE( parser );
				A = & vertices->data.vertices[ triangle->index[0] ];
				B = & vertices->data.vertices[ triangle->index[1] ];
				C = & vertices->data.vertices[ triangle->index[2] ];
				AB = DaoxVector3D_Sub( & B->point, & A->point );
				BC = DaoxVector3D_Sub( & C->point, & B->point );
				N = DaoxVector3D_Cross( & AB, & BC );
				norm = DaoxVector3D_Norm2( & N );
				N = DaoxVector3D_Scale( & N, 1.0 / sqrt( norm ) );
				A->norm = DaoxVector3D_Add( & A->norm, & N );
				B->norm = DaoxVector3D_Add( & B->norm, & N );
				C->norm = DaoxVector3D_Add( & C->norm, & N );
				faceCounts->items.pInt[ triangle->index[0] ] += 1;
				faceCounts->items.pInt[ triangle->index[1] ] += 1;
				faceCounts->items.pInt[ triangle->index[2] ] += 1;
				//printf( "%6i%6i%6i\n", triangle->index[0], triangle->index[1], triangle->index[2] );
			}
			for(i=0; i<vertices->size; ++i){
				DaoxVertex *V = & vertices->data.vertices[i];
				int count = faceCounts->items.pInt[i];
				V->norm = DaoxVector3D_Scale( & V->norm, 1.0 / count );
			}
			break;
		case 0x4130 : /* Faces Material */
			DString_SetChars( string, DaoxBinaryParser_DecodeString( parser ) );
			it = DMap_Find( self->materialNames, string );
			m = DaoxBinaryParser_DecodeUInt16LE( parser );
			k = it ? it->value.pInt : 0;
			unit = DaoxMesh_AddUnit( mesh );
			material = self->materials->items.pMaterial[k];
			DaoxMeshUnit_SetMaterial( unit, material );
			if( integers->size < vertices->size ) DList_Resize( integers, vertices->size, 0 );
			memset( integers->items.pInt, 0, vertices->size*sizeof(daoint) );
			for(i=0; i<m; ++i){
				int id = DaoxBinaryParser_DecodeUInt16LE( parser );
				DaoxTriangle *triangle = DArray_PushTriangle( unit->triangles );
				DaoxTriangle *triangle2 = triangles->data.triangles + id;
				for(j=0; j<3; ++j){
					if( integers->items.pInt[ triangle2->index[j] ] == 0 ){
						DaoxVertex *vertex = DArray_PushVertex( unit->vertices );
						DaoxVertex *vertex2 = vertices->data.vertices + triangle2->index[j];
						integers->items.pInt[ triangle2->index[j] ] = unit->vertices->size;
						*vertex = *vertex2;
					}
					triangle->index[j] = integers->items.pInt[ triangle2->index[j] ] - 1;
				}
			}
			break;
		case 0x4140 : /* Mapping Coordinates List */
			m = DaoxBinaryParser_DecodeUInt16LE( parser );
			for(i=0; i<m; ++i){
				DaoxVertex *vertex = & vertices->data.vertices[i];
				x = DaoxBinaryParser_DecodeFloatLE( parser );
				y = DaoxBinaryParser_DecodeFloatLE( parser );
				if( i >= vertices->size ) continue;
				vertex->texUV.x = x;
				vertex->texUV.y = y;
			}
			break;
		case 0x4160 : /* Local Coordinates System */
			matrix = DaoxMatrix4D_Identity();
			matrix.A11 = DaoxBinaryParser_DecodeFloatLE( parser );
			matrix.A21 = DaoxBinaryParser_DecodeFloatLE( parser );
			matrix.A31 = DaoxBinaryParser_DecodeFloatLE( parser );
			matrix.A12 = DaoxBinaryParser_DecodeFloatLE( parser );
			matrix.A22 = DaoxBinaryParser_DecodeFloatLE( parser );
			matrix.A32 = DaoxBinaryParser_DecodeFloatLE( parser );
			matrix.A13 = DaoxBinaryParser_DecodeFloatLE( parser );
			matrix.A23 = DaoxBinaryParser_DecodeFloatLE( parser );
			matrix.A33 = DaoxBinaryParser_DecodeFloatLE( parser );
			vector.x = DaoxBinaryParser_DecodeFloatLE( parser );
			vector.y = DaoxBinaryParser_DecodeFloatLE( parser );
			vector.z = DaoxBinaryParser_DecodeFloatLE( parser );
			DaoxMatrix4D_Print( & matrix );
			DaoxVector3D_Print( & vector );
#warning "=========================================== transform"
			for(i=0; i<vertices->size; ++i){
				DaoxVertex *vertex = & vertices->data.vertices[i];
				DaoxVector3D point = DaoxVector3D_Sub( & vertex->point, & vector );
				vertex->point = DaoxMatrix4D_MulVector( & matrix, & point, 1.0 );
			}
			matrix.B1 = vector.x;
			matrix.B2 = vector.y;
			matrix.B3 = vector.z;
			model->base.transform = matrix;
			break;
		case 0x4600 : /* Light */
			light = DaoxLight_New();
			DaoxScene_AddNode( self, (DaoxSceneNode*) light );
			vector.x = DaoxBinaryParser_DecodeFloatLE( parser );
			vector.y = DaoxBinaryParser_DecodeFloatLE( parser );
			vector.z = DaoxBinaryParser_DecodeFloatLE( parser );
			DaoxLight_Move( light, vector );
			printf( "light: " );
			DaoxVector3D_Print( & vector );
			break;
		case 0x4700 :
			camera = DaoxCamera_New();
			DaoxScene_AddNode( self, (DaoxSceneNode*) camera );
			vector.x = DaoxBinaryParser_DecodeFloatLE( parser );
			vector.y = DaoxBinaryParser_DecodeFloatLE( parser );
			vector.z = DaoxBinaryParser_DecodeFloatLE( parser );
			DaoxCamera_Move( camera, vector );
			vector2.x = DaoxBinaryParser_DecodeFloatLE( parser );
			vector2.y = DaoxBinaryParser_DecodeFloatLE( parser );
			vector2.z = DaoxBinaryParser_DecodeFloatLE( parser );
			DaoxCamera_LookAt( camera, vector2 );
			x = DaoxBinaryParser_DecodeFloatLE( parser ); /* rotation angle; */
			y = DaoxBinaryParser_DecodeFloatLE( parser ); /* camera lens; */
			camera->farPlane = 10.0 * sqrt( DaoxVector3D_Dist2( & vector, & vector2 ) );
			printf( "camera: " );
			DaoxVector3D_Print( & vector );
			break;
		case 0xAFFF : /* Material Block */
			material = DaoxMaterial_New();
			DList_Append( self->materials, material );
			break;
		case 0xA000 : /* Material name */
			DString_SetChars( string, DaoxBinaryParser_DecodeString( parser ) );
			MAP_Insert( self->materialNames, string, self->materials->size - 1 );
			break;
		case 0xA010 : /* ambient color */
			colortype = AMBIENT;
			break;
		case 0xA020 : /* diffuse color */
			colortype = DIFFUSE;
			break;
		case 0xA030 : /* specular color */
			colortype = SPECULAR;
			break;
		case 0x0010 : /* RGB color (float format) */
		case 0x0011 : /* RGB color (24bits) */
			switch( parentid ){
			case 0x4600 : color = & light->intensity; break;
			case 0xA010 : color = & material->ambient; break;
			case 0xA020 : color = & material->diffuse; break;
			case 0xA030 : color = & material->specular; break;
			default : color = & tmpColor; break;
			}
			color->alpha = 1.0;
			if( chunkid == 0x0010 ){
				color->red = DaoxBinaryParser_DecodeFloatLE( parser );
				color->green = DaoxBinaryParser_DecodeFloatLE( parser );
				color->blue = DaoxBinaryParser_DecodeFloatLE( parser );
			}else{
				color->red = parser->source[0] / 255.0;
				color->green = parser->source[1] / 255.0;
				color->blue = parser->source[2] / 255.0;
				parser->source += chunklen - 6;
			}
			break;
		case 0xA040 : /* shininess */
		case 0xA041 : /* shin. strength */
		case 0xA050 : /* transparency */
		case 0xA052 : /* trans. falloff */
		case 0xA053 : /* reflect blur */
		case 0xA100 : /* material type */
		case 0xA084 : /* self illum */
		case 0xB000 : /* Keyframer Chunk */
			parser->source += chunklen - 6;
			break;
		case 0xA200 :
			texture = DaoxTexture_New();
			MAP_Insert( self->textureNames, string, self->textures->size );
			DList_Append( self->textures, texture );
			DaoxMaterial_SetTexture( material, texture );
			break;
		case 0xA300 :
			DString_SetChars( string, DaoxBinaryParser_DecodeString( parser ) );
			DaoxTexture_LoadImage( texture, string->chars );
			printf( "texture: %s %i %i\n", string->chars, texture->image->width, texture->image->height );
			break;
		case 0x0030 :
			amount = DaoxBinaryParser_DecodeUInt16LE( parser ) / 100.0;
			printf( "%f\n", amount );
			//if( parentid == 0xA200 )
			break;
		default :
			parser->source += chunklen - 6;
			break;
		}
	}
	if( model ){
		DaoxMesh_UpdateTree( mesh, 0 );
		DaoxMesh_ResetBoundingBox( mesh );
		DaoxModel_SetMesh( model, mesh );
		DaoxScene_AddNode( self, (DaoxSceneNode*) model );
	}
	DArray_Delete( vertices );
	DArray_Delete( triangles );
	DList_Delete( integers );
	DList_Delete( chunkids );
	DList_Delete( chunkends );
	DList_Delete( faceCounts );
	DString_Delete( string );
}
void DaoxScene_Load3DS( DaoxScene *self, const char *file )
{
	DString *source = DString_New(1);
	FILE *fin = fopen( file, "r" );
	if( fin == NULL ) return; // TODO: Error;
	DaoFile_ReadAll( fin, source, 1 );
	DaoxScene_Parse3DS( self, source );
	DString_Delete( source );
}


