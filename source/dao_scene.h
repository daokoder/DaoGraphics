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

/*
// 3D Graphics
//
*/

#ifndef __DAO_GRAPHICS3D_H__
#define __DAO_GRAPHICS3D_H__


#include "dao_common.h"
#include "dao_font.h"
#include "dao_image.h"
#include "dao_mesh.h"



typedef struct DaoxTexture   DaoxTexture;


typedef struct DaoxSceneNode   DaoxSceneNode;
typedef struct DaoxCamera      DaoxCamera;
typedef struct DaoxLight       DaoxLight;
typedef struct DaoxModel       DaoxModel;
typedef struct DaoxScene       DaoxScene;




enum DaoxLightType
{
	DAOX_LT_AMBIENT ,
	DAOX_LT_POINT ,
	DAOX_LT_DIRECTIONAL ,
	DAOX_LT_SPOT
};

enum DaoxLightingModel
{
	DAOX_LM_DEFAULT ,
	DAOX_LM_CONSTANT ,
	DAOX_LM_LAMBERT ,
	DAOX_LM_BLINN ,
	DAOX_LM_PHONG
};






struct DaoxTexture
{
	DAO_CSTRUCT_COMMON;

	uint_t      tid;
	DString    *file;
	DaoxImage  *image;
};
extern DaoType *daox_type_texture;

DaoxTexture* DaoxTexture_New();
void DaoxTexture_Delete( DaoxTexture *self );
void DaoxTexture_SetImage( DaoxTexture *self, DaoxImage *image );
void DaoxTexture_glInitTexture( DaoxTexture *self );





struct DaoxMaterial
{
	DAO_CSTRUCT_COMMON;

	DaoxColor  ambient;
	DaoxColor  diffuse;
	DaoxColor  specular;
	DaoxColor  emission;

	uint_t     lighting;
	uint_t     shininess;
	uint_t     shinStrength;
	uint_t     transparency;
	uint_t     transFalloff;
	uint_t     reflectBlur;

	DaoxTexture  *texture1;
	DaoxTexture  *texture2;
	DString      *name;
};
extern DaoType *daox_type_material;

DaoxMaterial* DaoxMaterial_New();
void DaoxMaterial_Delete( DaoxMaterial *self );

void DaoxMaterial_CopyFrom( DaoxMaterial *self, DaoxMaterial *other );
void DaoxMaterial_SetTexture( DaoxMaterial *self, DaoxTexture *texture );





struct DaoxSceneNode
{
	DAO_CSTRUCT_COMMON;

	uchar_t  renderable;

	DaoxOBBox3D     obbox;     /* local coordinates; */
	DaoxMatrix4D    transform; /* local to parent */

	DaoxSceneNode  *parent;
	DArray         *children;
};
extern DaoType *daox_type_scene_node;

DaoxSceneNode* DaoxSceneNode_New();
void DaoxSceneNode_Delete( DaoxSceneNode *self );

void DaoxSceneNode_Init( DaoxSceneNode *self, DaoType *type );
void DaoxSceneNode_Free( DaoxSceneNode *self );

void DaoxSceneNode_AddChild( DaoxSceneNode *self, DaoxSceneNode *child );

void DaoxSceneNode_MoveByXYZ( DaoxSceneNode *self, float dx, float dy, float dz );
void DaoxSceneNode_MoveXYZ( DaoxSceneNode *self, float x, float y, float z );
void DaoxSceneNode_MoveBy( DaoxSceneNode *self, DaoxVector3D delta );
void DaoxSceneNode_Move( DaoxSceneNode *self, DaoxVector3D pos );

void DaoxSceneNode_ApplyTransform( DaoxSceneNode *self, DaoxMatrix4D matrix );

DaoxMatrix4D DaoxSceneNode_GetWorldTransform( DaoxSceneNode *self );

DaoxVector3D DaoxSceneNode_GetWorldPosition( DaoxSceneNode *self );



/*
// The camera always located at the origin in its local coordinates;
// And it always looks at the opposite direction of the local z-axis (0,0,-1),
// with the up direction pointing to the direction of the local y-axis (0,1,0),
// and the right direction pointing to the direction of the local x-axis (1,0,0),
//
// Its location and orientation can only be adjusted by transformation (::objectToParent).
*/
struct DaoxCamera
{
	DaoxSceneNode  base;
	DaoxVector3D   viewTarget;
	float          fovAngle;
	float          aspectRatio;   // Width to height ratio of the view field;
	float          nearPlane;
	float          farPlane;
};
extern DaoType *daox_type_camera;

DaoxCamera* DaoxCamera_New();
void DaoxCamera_Delete( DaoxCamera *self );

void DaoxCamera_CopyFrom( DaoxCamera *self, DaoxCamera *other );

void DaoxCamera_Move( DaoxCamera *self, DaoxVector3D pos );
void DaoxCamera_MoveBy( DaoxCamera *self, DaoxVector3D delta );
void DaoxCamera_LookAt( DaoxCamera *self, DaoxVector3D pos );
void DaoxCamera_MoveXYZ( DaoxCamera *self, float x, float y, float z );
void DaoxCamera_MoveByXYZ( DaoxCamera *self, float dx, float dy, float dz );
void DaoxCamera_LookAtXYZ( DaoxCamera *self, float x, float y, float z );



struct DaoxLight
{
	DaoxSceneNode  base;
	DaoxVector3D   targetPosition;
	DaoxColor      intensity;
	uint_t         lightType;
};
extern DaoType *daox_type_light;

DaoxLight* DaoxLight_New();
void DaoxLight_Delete( DaoxLight *self );

void DaoxLight_CopyFrom( DaoxLight *self, DaoxLight *other );

void DaoxLight_Move( DaoxLight *self, DaoxVector3D pos );
void DaoxLight_PointAt( DaoxLight *self, DaoxVector3D pos );
void DaoxLight_MoveXYZ( DaoxLight *self, float x, float y, float z );
void DaoxLight_PointAtXYZ( DaoxLight *self, float x, float y, float z );


struct DaoxModel
{
	DaoxSceneNode  base;
	DaoxMesh      *mesh;

	/*
	// The memory of these arrays is freed after the model has been invisible
	// for sufficiently long time.
	*/
	DaoxPlainArray  *offsets; /* offsets in the vertex buffer object; */
	DArray          *points; /* vertex positions, global coordinates; */
	DArray          *vnorms; /* vertex normals, global coordinates; */
	DArray          *tnorms; /* triangle normals, global coordinates; */
	uint_t           viewFrame;
};
extern DaoType *daox_type_model;

DaoxModel* DaoxModel_New();
void DaoxModel_Delete( DaoxModel *self );
void DaoxModel_SetMesh( DaoxModel *self, DaoxMesh *mesh );
void DaoxModel_TransformMesh( DaoxModel *self );




typedef struct DaoxViewFrustum  DaoxViewFrustum;

struct DaoxViewFrustum
{
	float  left;
	float  right;
	float  top;
	float  bottom;
	float  near;
	float  far;

	DaoxMatrix4D  worldToCamera;
	DaoxVector3D  cameraPosition;   /* world coordinates; */
	DaoxVector3D  viewDirection;    /* world coordinates; */
	DaoxVector3D  nearViewCenter;   /* world coordinates; */
	DaoxVector3D  farViewCenter;    /* world coordinates; */
	DaoxVector3D  topLeftEdge;      /* world coordinates; */
	DaoxVector3D  topRightEdge;     /* world coordinates; */
	DaoxVector3D  bottomLeftEdge;   /* world coordinates; */
	DaoxVector3D  bottomRightEdge;  /* world coordinates; */
	DaoxVector3D  leftPlaneNorm;    /* world coordinates; */
	DaoxVector3D  rightPlaneNorm;   /* world coordinates; */
	DaoxVector3D  topPlaneNorm;     /* world coordinates; */
	DaoxVector3D  bottomPlaneNorm;  /* world coordinates; */
};

void DaoxViewFrustum_Init( DaoxViewFrustum *self, DaoxCamera *camera );
int  DaoxViewFrustum_Visible( DaoxViewFrustum *self, DaoxOBBox3D *box );
DaoxViewFrustum DaoxViewFrustum_Transform( DaoxViewFrustum *self, DaoxMatrix4D *transform );
double DaoxViewFrustum_Difference( DaoxViewFrustum *self, DaoxViewFrustum *other );




struct DaoxScene
{
	DAO_CSTRUCT_COMMON;

	DArray  *nodes;
	DArray  *lights;
	DMap    *materialNames;
	DMap    *textureNames;
	DArray  *materials;
	DArray  *textures;
};
extern DaoType *daox_type_scene;

DaoxScene* DaoxScene_New();
void DaoxScene_Delete( DaoxScene *self );

void DaoxScene_AddNode( DaoxScene *self, DaoxSceneNode *node );
void DaoxScene_AddMaterial( DaoxScene *self, DString *name, DaoxMaterial *material );




#endif
