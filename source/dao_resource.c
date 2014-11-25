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


#include <string.h>
#include "dao_resource.h"
#include "dao_format.h"
#include "daoVmspace.h"









DaoxResource* DaoxResource_New( DaoVmSpace *vms )
{
	DaoxResource *self = (DaoxResource*) dao_calloc( 1, sizeof(DaoxResource) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_resource );
	self->scenes     = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->lights     = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->cameras    = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->images     = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->textures   = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->effects    = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->materials  = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->geometries = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->terrains   = DHash_New( DAO_DATA_STRING, DAO_DATA_VALUE );
	self->xmlDOM     = DaoXmlDOM_New();
	self->xmlParser  = DaoXmlParser_New();
	self->vmSpace = vms;
	return self;
}
void DaoxResource_Delete( DaoxResource *self )
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
	DMap_Delete( self->terrains );
	DaoXmlDOM_Delete( self->xmlDOM );
	DaoXmlParser_Delete( self->xmlParser );
	dao_free( self );
}
DaoxScene* DaoxResource_GetScene( DaoxResource *self )
{
	if( self->scenes->size == 0 ) return NULL;
	return (DaoxScene*) DMap_First( self->scenes )->value.pVoid;
}

int DaoxResource_SearchFile( DaoxResource *self, DString *fname, DString *search )
{
	DString *tmp;

	if( DaoVmSpace_SearchResource( self->vmSpace, fname, search ) ) return 1;
	tmp = DString_Copy( fname );
	Dao_MakePath( search, tmp );
	if( DaoVmSpace_TestFile( self->vmSpace, tmp ) ){
		DString_Assign( fname, tmp );
		DString_Delete( tmp );
		return 1;
	}
	DString_Delete( tmp );
	return 0;
}
int DaoxResource_ReadFile( DaoxResource *self, DString *fname, DString *source )
{
	return DaoVmSpace_ReadFile( self->vmSpace, fname, source );
}
DaoxImage* DaoxResource_LoadImage( DaoxResource *self, DString *fname, DString *path )
{
	DaoxImage *image = NULL;
	DString *file = DString_Copy( fname );
	DString *source = DString_New();

	if( DaoxResource_SearchFile( self, file, path ) ){
		DNode *it = DMap_Find( self->images, file );
		if( it != NULL ){
			image = (DaoxImage*) it->value.pValue;
		}else if( DaoxResource_ReadFile( self, file, source ) ){
			image = DaoxImage_New();
			DaoxImage_Decode( image, source );
			DMap_Insert( self->images, file, image );
		}
	}
	DString_Delete( source );
	DString_Delete( file );
	return image;
}


static DaoRoutine* Dao_Get_Object_Method( DaoCstruct *cd, DaoObject **obj, const char *name )
{
  DaoRoutine *meth;
  if( cd == NULL ) return NULL;
  *obj = DaoCdata_GetObject( (DaoCdata*) cd );
  if( *obj == NULL ) return NULL;
  return DaoObject_GetMethod( *obj, name );
}
DaoCstruct* DaoxResource_CallMethod( DaoxResource *self, const char *method, DaoType *ctype )
{
	DaoValue *res = NULL;
	DaoObject *obj = NULL;
	DaoRoutine *rout = Dao_Get_Object_Method( (DaoCstruct*) self, & obj, method );
	DaoProcess *proc;

	if( rout == NULL || obj == NULL ) return NULL;
	proc = DaoVmSpace_AcquireProcess( dao_vmspace_graphics );

	rout = DaoRoutine_Resolve( rout, (DaoValue*) obj, NULL, NULL, NULL, 0, 0 );
	if( rout == NULL ) goto Finalize;
	DaoProcess_Call( proc, rout, (DaoValue*) obj, NULL, 0 );
	res = DaoProcess_GetReturned( proc );
	if( res == NULL || res->type != DAO_OBJECT ) return NULL;
	return DaoObject_CastCstruct( (DaoObject*)res, ctype );
Finalize:
	DaoVmSpace_ReleaseProcess( dao_vmspace_graphics, proc );
	return NULL;
}
DaoxScene* DaoxResource_CreateScene( DaoxResource *self )
{
	DaoCstruct *res = DaoxResource_CallMethod( self, "CreateScene", daox_type_scene );
	printf( "DaoxResource_CreateScene %p\n", res );
	if( res ) return (DaoxScene*) res;
	return DaoxScene_New();
}


DaoxMesh* DaoxResource_MakeTerrain( DaoxResource *self, DaoxImage *heightmap )
{
	DaoxMesh *mesh = DaoxMesh_New();
	DaoxMeshUnit *unit = DaoxMesh_AddUnit( mesh );
}
