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


#ifndef __DAO_RESOURCE_H__
#define __DAO_RESOURCE_H__

#include "dao_scene.h"
#include "dao_xml.h"


typedef struct DaoxSceneResource  DaoxSceneResource;




struct DaoxSceneResource
{
	DAO_CSTRUCT_COMMON;

	DMap  *scenes;
	DMap  *lights;
	DMap  *cameras;
	DMap  *images;   /* also stored as texture for convenience; */
	DMap  *textures;
	DMap  *effects;
	DMap  *materials;
	DMap  *geometries;
	DMap  *terrains;

	DaoXmlDOM     *xmlDOM;
	DaoXmlParser  *xmlParser;
};
extern DaoType *daox_type_resource;

DaoxSceneResource* DaoxSceneResource_New();
void DaoxSceneResource_Delete( DaoxSceneResource *self );

DaoxScene* DaoxSceneResource_GetScene( DaoxSceneResource *self );

DaoxScene* DaoxSceneResource_CreateScene( DaoxSceneResource *self );

DaoxMesh* DaoxSceneResource_MakeTerrain( DaoxSceneResource *self, DaoxImage *heightmap );

#endif
