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


#include <string.h>
#include "dao_resource.h"
#include "dao_format.h"









DaoxSceneResource* DaoxSceneResource_New()
{
	DaoxSceneResource *self = (DaoxSceneResource*) dao_calloc( 1, sizeof(DaoxSceneResource) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_resource );
	self->scenes = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->lights = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->cameras = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->images = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->textures = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->effects = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->materials = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->geometries = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->xmlDOM = DaoXmlDOM_New();
	self->xmlParser = DaoXmlParser_New();
	self->collada = DaoxColladaParser_New();
	return self;
}
void DaoxSceneResource_Delete( DaoxSceneResource *self )
{
	DaoCstruct_Free( (DaoCstruct*) self );
	DMap_Delete( self->scenes );
	DMap_Delete( self->lights );
	DMap_Delete( self->cameras );
	DMap_Delete( self->images );
	DMap_Delete( self->textures );
	DMap_Delete( self->effects );
	DMap_Delete( self->materials );
	DMap_Delete( self->geometries );
	DaoXmlDOM_Delete( self->xmlDOM );
	DaoXmlParser_Delete( self->xmlParser );
	DaoxColladaParser_Delete( self->collada );
	dao_free( self );
}
DaoxScene* DaoxSceneResource_GetScene( DaoxSceneResource *self )
{
	if( self->scenes->size == 0 ) return NULL;
	return (DaoxScene*) DMap_First( self->scenes )->value.pVoid;
}



static DaoRoutine* Dao_Get_Object_Method( DaoCstruct *cd, DaoObject **obj, const char *name )
{
  DaoRoutine *meth;
  if( cd == NULL ) return NULL;
  *obj = DaoCdata_GetObject( (DaoCdata*) cd );
  if( *obj == NULL ) return NULL;
  return DaoObject_GetMethod( *obj, name );
}
DaoCstruct* DaoxSceneResource_CallMethod( DaoxSceneResource *self, const char *method, DaoType *ctype )
{
	DaoValue *res = NULL;
	DaoObject *obj = NULL;
	DaoRoutine *rout = Dao_Get_Object_Method( (DaoCstruct*) self, & obj, method );
	DaoProcess *proc;

	if( rout == NULL || obj == NULL ) return NULL;
	proc = DaoVmSpace_AcquireProcess( __daoVmSpace );

	rout = DaoRoutine_Resolve( rout, (DaoValue*) obj, NULL, NULL, NULL, 0, 0 );
	if( rout == NULL ) goto Finalize;
	DaoProcess_Call( proc, rout, (DaoValue*) obj, NULL, 0 );
	res = DaoProcess_GetReturned( proc );
	if( res == NULL || res->type != DAO_OBJECT ) return NULL;
	return DaoObject_CastCstruct( (DaoObject*)res, ctype );
Finalize:
	DaoVmSpace_ReleaseProcess( __daoVmSpace, proc );
	return NULL;
}
DaoxScene* DaoxSceneResource_CreateScene( DaoxSceneResource *self )
{
	DaoCstruct *res = DaoxSceneResource_CallMethod( self, "CreateScene", daox_type_scene );
	printf( "DaoxSceneResource_CreateScene %p\n", res );
	if( res ) return (DaoxScene*) res;
	return DaoxScene_New();
}



DaoxScene* Test_Collada()
{
	FILE *fin = fopen( "duck.dae", "r" );
	//FILE *fin = fopen( "cube_triangulate.dae", "r" );
	//FILE *fin = fopen( "Seymour/Seymour_triangulate.dae", "r" );
	DString *source = DString_New(1);
	DaoxSceneResource *resource = DaoxSceneResource_New();
	DaoFile_ReadAll( fin, source, 1 );
	DaoxSceneResource_LoadColladaSource( resource, source );
	return DaoxSceneResource_GetScene( resource );
	DaoxSceneResource_Delete( resource );
	DString_Delete( source );
	exit(0);
}
