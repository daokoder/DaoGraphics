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


#include "dao_format.h"
#include "dao_xml.h"
#include "dao_opengl.h"





DaoxObjParser* DaoxObjParser_New()
{
	DaoxObjParser *self = (DaoxObjParser*) dao_calloc( 1, sizeof(DaoxObjParser) );
	self->lexer = DaoLexer_New();
	self->lexer2 = DaoLexer_New();
	self->integers = DArray_New( sizeof(int) );
	self->vlist = DArray_New( sizeof(DaoxVector3D) );
	self->vtlist = DArray_New( sizeof(DaoxVector3D) );
	self->vnlist = DArray_New( sizeof(DaoxVector3D) );
	self->flist = DArray_New( sizeof(int) );
	self->faceVertMap = DMap_New( DAO_DATA_COMPLEX, 0 );
	return self;
}
void DaoxObjParser_Delete( DaoxObjParser *self )
{
	DaoLexer_Delete( self->lexer );
	DaoLexer_Delete( self->lexer2 );
	DArray_Delete( self->integers );
	DArray_Delete( self->vlist );
	DArray_Delete( self->vtlist );
	DArray_Delete( self->vnlist );
	DArray_Delete( self->flist );
	DMap_Delete( self->faceVertMap );
	dao_free( self );
}

static int DaoxToken_CheckKeyword( DaoToken *token, const char *keyword )
{
	if( token->type != DTOK_IDENTIFIER ) return 0;
	return strcmp( token->string.chars, keyword ) == 0;
}
static int DaoxToken_CheckKeywords( DaoToken *token, const char *keywords )
{
	if( token->type != DTOK_IDENTIFIER ) return 0;
	return DString_Match( & token->string, keywords, NULL, NULL );
}

static DaoxMeshUnit* DaoxObjParser_ConstructMeshUnit( DaoxObjParser *self, DaoxMesh *mesh )
{
	DNode *it;
	DaoxVertex vertex;
	DaoxVector3D vector;
	DaoxMeshUnit *unit = DaoxMesh_AddUnit( mesh );
	dao_complex buffer = {0.0, 0.0};
	uint_t *quadints = (uint_t*) & buffer;
	daoint i, j, k, N = self->flist->size;
	int tcount = 0;

	for(i=0; i<N; ){
		int count = self->flist->data.ints[i];
		int smooth = self->flist->data.ints[i+1];
		quadints[1] = quadints[2] = 0;
		quadints[3] = smooth;
		self->integers->size = 0;
		//printf( "i = %i; count = %i; smooth = %i\n", i, count, smooth );
		for(i+=2, j=0; j<count; i+=3, ++j){
			quadints[0] = self->flist->data.ints[i];   /* vertex index; */
			quadints[1] = self->flist->data.ints[i+1]; /* texture index; */
			quadints[2] = self->flist->data.ints[i+2]; /* norm index; */
			//printf( "%i: %i/%i/%i\n", i, quadints[0], quadints[1], quadints[2] );
			if( smooth != 0 && quadints[2] == 0 ){
				it = DMap_Find( self->faceVertMap, quadints );
				if( it != NULL ){
					DArray_PushInt( self->integers, it->value.pInt );
					DArray_PushInt( self->integers, quadints[2] );
					continue;
				}
			}

			memset( & vertex, 0, sizeof(DaoxVertex) );
			vertex.pos = self->vlist->data.vectors3d[ quadints[0]-1 ];
			if( quadints[1] ){
				vector = self->vtlist->data.vectors3d[ quadints[1]-1 ];
				vertex.tex.x = vector.x;
				vertex.tex.y = vector.y;
			}
			if( quadints[2] ){
				vector = self->vnlist->data.vectors3d[ quadints[2]-1 ];
				vertex.norm = vector;
			}
			MAP_Insert( self->faceVertMap, quadints, unit->vertices->size );
			DArray_PushInt( self->integers, unit->vertices->size );
			DArray_PushInt( self->integers, quadints[2] );
			DArray_PushVertex( unit->vertices, & vertex );
		}
		for(j=4; j<self->integers->size; j+=2){
			DaoxTriangle *triangle = DArray_PushTriangle( unit->triangles, NULL );
			DaoxVector3D A, B, C, AB, BC, facenorm;
			int hasnorms[3];

			tcount += 1;
			triangle->index[0] = self->integers->data.ints[0];
			triangle->index[1] = self->integers->data.ints[j-2];
			triangle->index[2] = self->integers->data.ints[j];
			hasnorms[0] = self->integers->data.ints[1];
			hasnorms[1] = self->integers->data.ints[j-1];
			hasnorms[2] = self->integers->data.ints[j+1];
			if( hasnorms[0] || hasnorms[1] || hasnorms[2] ) continue;

			A = unit->vertices->data.vertices[ triangle->index[0] ].pos;
			B = unit->vertices->data.vertices[ triangle->index[1] ].pos;
			C = unit->vertices->data.vertices[ triangle->index[2] ].pos;
			facenorm = DaoxTriangle_Normal( & A, & B, & C );
			for(k=0; k<3; ++k){
				DaoxVector3D *norm = &unit->vertices->data.vertices[triangle->index[k]].norm;
				if( hasnorms[k] ) continue;
				norm->x += facenorm.x;
				norm->y += facenorm.y;
				norm->z += facenorm.z;
			}
		}
	}
	for(i=0; i<unit->vertices->size; ++i){
		DaoxVector3D *norm = & unit->vertices->data.vertices[i].norm;
		*norm = DaoxVector3D_Normalize( norm );
	}
	self->flist->size = 0;
	DMap_Reset( self->faceVertMap );
	return unit;
}

int DaoxResource_LoadObjMtlSource( DaoxResource *self, DaoxObjParser *parser, DString *source, DString *path )
{
	DNode *it;
	DaoToken **tokens;
	DaoxVector3D vector;
	DaoxImage *image;
	DaoxTexture *texture = NULL;
	DaoxMaterial *material = NULL;
	DString *string = DString_New();
	double numbers[4] = {0.0};
	daoint i, j, k, N, N1;

	DaoLexer_Tokenize( parser->lexer2, source->chars, 0 );
	tokens = parser->lexer2->tokens->items.pToken;
	N = parser->lexer2->tokens->size;
	N1 = N - 1;
	for(i=0; i<N; ){
		DaoToken *token = tokens[i];
		if( DaoxToken_CheckKeyword( token, "newmtl" ) ){
			if( i >= N1 ) goto InvalidFormat;
			DString_Reset( string, 0 );
			while( (++i) < N && tokens[i]->line == token->line ) {
				DString_Append( string, & tokens[i]->string );
			}
			material = DaoxMaterial_New();
			DString_Assign( material->name, string );
			DMap_Insert( self->materials, material->name, material );
		}else if( DaoxToken_CheckKeyword( token, "Ns" ) ){
			i += 2;
		}else if( DaoxToken_CheckKeyword( token, "Ni" ) ){
			i += 2;
		}else if( DaoxToken_CheckKeywords( token, "^ (Ka|Kd|Ks|Ke) $" ) ){
			const char *keys = "KaKdKsKe";
			const char *s = strstr( keys, token->string.chars );
			int ctype = (s - keys) / 2;
			DaoxColor *color = & material->diffuse;
			k = 0;
			while( (++i) < N && tokens[i]->line == token->line ) {
				DaoToken *tok = tokens[i];
				if( tok->type < DTOK_DIGITS_DEC ) goto InvalidFormat;
				if( tok->type > DTOK_NUMBER_SCI ) goto InvalidFormat;
				if( k >= 3 ) goto InvalidFormat;
				numbers[k++] = DaoToken_ToFloat( tok );
			}
			switch( ctype ){
			case 0 : color = & material->ambient; break;
			case 1 : color = & material->diffuse; break;
			case 2 : color = & material->specular; break;
			case 3 : color = & material->emission; break;
			}
			color->red = numbers[0];
			color->green = numbers[1];
			color->blue = numbers[2];
		}else if( DaoxToken_CheckKeywords( token, "^ (map_Kd|map_Bump) $" ) ){
			int which = 0;
			if( strcmp( token->string.chars, "map_Kd" ) == 0 ){
				which = DAOX_DIFFUSE_TEXTURE;
			}else{
				which = DAOX_BUMP_TEXTURE;
			}
			DString_Reset( string, 0 );
			while( (++i) < N && tokens[i]->line == token->line ) {
				DString_Append( string, & tokens[i]->string );
			}
			image = DaoxResource_LoadImage( self, string, path );
			texture = DaoxTexture_New();
			if( image ) DaoxTexture_SetImage( texture, image );
			DaoxMaterial_SetTexture( material, texture, which );
		}else{
			i += 1;
		}
	}
	DString_Delete( string );
	return 1;
InvalidFormat:
	printf( "ERROR: invalid format!\n" );
	DString_Delete( string );
	return 0;
}
int DaoxResource_LoadObjMtlFile( DaoxResource *self, DaoxObjParser *parser, const char *file )
{
	int res;
	FILE *fin = fopen( file, "r" );
	DString *source = DString_New();
	DString *path = DString_New();
	DString_SetChars( path, file );
	DString_Change( path, "%\\", "/", 0 );
	DString_Change( path, "[^/]+ $", "", 1 );
	DaoFile_ReadAll( fin, source, 1 );
	res = DaoxResource_LoadObjMtlSource( self, parser, source, path );
	DString_Delete( source );
	DString_Delete( path );
	return res;
}

DaoxScene* DaoxResource_LoadObjSource( DaoxResource *self, DString *source, DString *path )
{
	DNode *it;
	DaoToken **tokens;
	DaoxVertex vertex;
	DaoxVector3D vector;
	DaoxMesh *mesh = NULL;
	DaoxModel *model = NULL;
	DaoxMeshUnit *unit = NULL;
	DaoxMaterial *material = NULL;
	DaoxScene *scene = DaoxScene_New();
	DaoxObjParser *parser = DaoxObjParser_New();
	DString *string = DString_New();
	DString *source2 = DString_New();
	DArray *vlist = parser->vlist;
	DArray *vtlist = parser->vtlist;
	DArray *vnlist = parser->vnlist;
	dao_complex com = {0.0, 0.0};
	uint_t *integers = (uint_t*) & com;
	double numbers[4] = {0.0};
	daoint vcount = 0, vtcount = 0, vncount = 0;
	daoint i, j, k, N, N1, tcount = 0;
	int smooth = 1;

	DaoLexer_Tokenize( parser->lexer, source->chars, 0 );
	tokens = parser->lexer->tokens->items.pToken;
	N = parser->lexer->tokens->size;
	N1 = N - 1;
	for(i=0; i<N; ){
		DaoToken *token = tokens[i];
		if( DaoxToken_CheckKeyword( token, "mtllib" ) ){
			DString_Reset( string, 0 );
			while( (++i) < N && tokens[i]->line == token->line ) {
				DString_Append( string, & tokens[i]->string );
			}
			if( DaoxResource_SearchFile( self, string, path ) ){
				if( DaoxResource_ReadFile( self, string, source2 ) ){
					DaoxResource_LoadObjMtlSource( self, parser, source2, path );
				}
			}
		}else if( DaoxToken_CheckKeyword( token, "v" ) ){
			k = 0;
			while( (++i) < N && tokens[i]->line == token->line ) {
				DaoToken *tok = tokens[i];
				int sign = 2*(tokens[i-1]->type != DTOK_SUB) - 1;
				if( tok->type == DTOK_SUB ) continue;
				if( tok->type < DTOK_DIGITS_DEC ) goto InvalidFormat;
				if( tok->type > DTOK_NUMBER_SCI ) goto InvalidFormat;
				if( k >= 4 ) goto InvalidFormat;
				numbers[k++] = sign * DaoToken_ToFloat( tok );
			}
			vector.x = numbers[0];
			vector.y = numbers[1];
			vector.z = numbers[2];
			//printf( "v: " ); DaoxVector3D_Print( & vector );
			DArray_PushVector3D( parser->vlist, & vector );
			vcount += 1;
		}else if( DaoxToken_CheckKeyword( token, "vt" ) ){
			k = 0;
			while( (++i) < N && tokens[i]->line == token->line ) {
				DaoToken *tok = tokens[i];
				int sign = 2*(tokens[i-1]->type != DTOK_SUB) - 1;
				if( tok->type == DTOK_SUB ) continue;
				if( tok->type < DTOK_DIGITS_DEC ) goto InvalidFormat;
				if( tok->type > DTOK_NUMBER_SCI ) goto InvalidFormat;
				if( k >= 3 ) goto InvalidFormat;
				numbers[k++] = sign * DaoToken_ToFloat( tok );
			}
			vector.x = numbers[0];
			vector.y = numbers[1];
			vector.z = 0.0;
			//printf( "vt: " ); DaoxVector3D_Print( & vector );
			DArray_PushVector3D( parser->vtlist, & vector );
			vtcount += 1;
		}else if( DaoxToken_CheckKeyword( token, "vn" ) ){
			k = 0;
			while( (++i) < N && tokens[i]->line == token->line ) {
				DaoToken *tok = tokens[i];
				int sign = 2*(tokens[i-1]->type != DTOK_SUB) - 1;
				if( tok->type == DTOK_SUB ) continue;
				if( tok->type < DTOK_DIGITS_DEC ) goto InvalidFormat;
				if( tok->type > DTOK_NUMBER_SCI ) goto InvalidFormat;
				if( k >= 3 ) goto InvalidFormat;
				numbers[k++] = sign * DaoToken_ToFloat( tok );
			}
			vector.x = numbers[0];
			vector.y = numbers[1];
			vector.z = numbers[2];
			DArray_PushVector3D( parser->vnlist, & vector );
			vncount += 1;
		}else if( token->type == DTOK_IDENTIFIER && token->string.size == 1
				&& (token->string.chars[0] == 'o' || token->string.chars[0] == 'g') ){
			if( model && parser->flist->size ){
				unit = DaoxObjParser_ConstructMeshUnit( parser, mesh );
				DaoxMeshUnit_SetMaterial( unit, material );
				DaoxMesh_UpdateTree( mesh, 0 ); 
				DaoxMesh_ResetBoundingBox( mesh );
				DaoxMesh_UpdateNormTangents( mesh, 0, 1 );
				DaoxModel_SetMesh( model, mesh );
				DaoxScene_AddNode( scene, (DaoxSceneNode*) model );
				model = NULL;
			}
			DString_Reset( string, 0 );
			while( (++i) < N && tokens[i]->line == token->line ) {
				DString_Append( string, & tokens[i]->string );
			}
			//printf( "node: %s %i\n", string->chars, i );
			if( model == NULL ){
				model = DaoxModel_New();
				mesh = DaoxMesh_New();
				material = NULL;
			}
			vcount = vtcount = vncount = tcount = 0;
		}else if( DaoxToken_CheckKeyword( token, "usemtl" ) ){
			if( model && parser->flist->size ){
				unit = DaoxObjParser_ConstructMeshUnit( parser, mesh );
				DaoxMeshUnit_SetMaterial( unit, material );
				DaoxMesh_UpdateTree( mesh, 0 ); 
				DaoxMesh_ResetBoundingBox( mesh );
			}
			DString_Reset( string, 0 );
			while( (++i) < N && tokens[i]->line == token->line ) {
				DString_Append( string, & tokens[i]->string );
			}
			it = DMap_Find( self->materials, string );
			//printf( ">> %s %p\n", string->chars, it );
			if( it ) material = (DaoxMaterial*) it->value.pVoid;
		}else if( DaoxToken_CheckKeyword( token, "s" ) ){
			smooth = DaoToken_ToInteger( tokens[i+1] ); /* XXX: s off */
			i += 2;
		}else if( DaoxToken_CheckKeyword( token, "f" ) ){
			int offset = parser->flist->size;
			DArray_PushInt( parser->flist, 0 );
			DArray_PushInt( parser->flist, smooth );
			k = 0;
			//printf( "\n" );
			while( (++i) < N && tokens[i]->line == token->line ) {
				DaoToken *tok = tokens[i];
				if( tok->type != DTOK_DIGITS_DEC ) goto InvalidFormat;
				if( k >= 3 ) goto InvalidFormat;
				integers[k++] = DaoToken_ToInteger( tok );
				if( i >= N1 || tokens[i+1]->type != DTOK_DIV ){
					parser->flist->data.ints[offset] += 1;
					for(j=0; j<3; ++j) DArray_PushInt( parser->flist, integers[j] );
					//printf( "%i/%i/%i\n", integers[0], integers[1], integers[2] );
					k = integers[1] = integers[2] = 0;
				}else if( tokens[++i]->type == DTOK_DIV ){
					if( (i+1) >= N ) goto InvalidFormat;
					if( tokens[i+1]->type == DTOK_DIV ){
						integers[k++] = 0;
						i += 1;
					}
				}else{
					goto InvalidFormat;
				}
			}
		}else{
			i += 1;
		}
		fflush( stdout );
	}
	if( model ){
		//printf( ">> %i %i %i %i\n", vcount, vtcount, vncount, tcount );
		unit = DaoxObjParser_ConstructMeshUnit( parser, mesh );
		DaoxMeshUnit_SetMaterial( unit, material );
		DaoxMesh_UpdateTree( mesh, 0 ); 
		DaoxMesh_ResetBoundingBox( mesh );
		DaoxMesh_UpdateNormTangents( mesh, 0, 1 );
		DaoxModel_SetMesh( model, mesh );
		DaoxScene_AddNode( scene, (DaoxSceneNode*) model );
	}
	//printf( "nodes: %i\n", scene->nodes->size );
	DaoxObjParser_Delete( parser );
	DString_Delete( source2 );
	DString_Delete( string );
	return scene;
InvalidFormat:
	printf( "ERROR: invalid format at line %i!\n", tokens[i]->line );
	DaoxObjParser_Delete( parser );
	DString_Delete( source2 );
	DString_Delete( string );
	return NULL;
}
DaoxScene* DaoxResource_LoadObjFile( DaoxResource *self, DString *file, DString *path )
{
	DaoxScene *scene = NULL;
	DString *source = DString_New();

	file = DString_Copy( file );
	//printf( "DaoxResource_LoadObjFile: %s %s\n", file->chars, path->chars );
	if( DaoxResource_SearchFile( self, file, path ) ){
		if( DaoxResource_ReadFile( self, file, source ) ){
			DString_Change( file, "[^/\\]* $", "", 0 );
			scene = DaoxResource_LoadObjSource( self, source, file );
		}
	}
	DString_Delete( source );
	DString_Delete( file );
	return scene;
}






enum DaoxColladaTags
{
	DAE_UNSUPPORTED ,
	DAE_ASSET ,
	DAE_LIBRARY_LIGHTS ,
	DAE_LIBRARY_CAMERAS ,
	DAE_LIBRARY_IMAGES ,
	DAE_LIBRARY_EFFECTS ,
	DAE_LIBRARY_MATERIALS ,
	DAE_LIBRARY_GEOMETRIES ,
	DAE_LIBRARY_CONTROLLERS ,
	DAE_LIBRARY_VISUAL_SCENES ,
	DAE_VISUAL_SCENE ,
	DAE_SCENE ,
	DAE_LIGHT ,
	DAE_DIRECTIONAL ,
	DAE_POINT ,
	DAE_SPOT ,
	DAE_COLOR ,
	DAE_CAMERA ,
	DAE_OPTICS ,
	DAE_PERSPECTIVE ,
	DAE_YFOV ,
	DAE_ASPECT_RATIO ,
	DAE_ZNEAR ,
	DAE_ZFAR ,
	DAE_MATERIAL ,
	DAE_EFFECT ,
	DAE_NEWPARAM ,
	DAE_SURFACE ,
	DAE_INIT_FROM ,
	DAE_SAMPLER2D ,
	DAE_SOURCE ,
	DAE_CONSTANT ,
	DAE_LAMBERT ,
	DAE_BLINN ,
	DAE_PHONG ,
	DAE_EMISSION ,
	DAE_AMBIENT ,
	DAE_DIFFUSE ,
	DAE_TEXTURE ,
	DAE_SPECULAR ,
	DAE_SHININESS ,
	DAE_REFLECTIVE ,
	DAE_REFLECTIVITY ,
	DAE_TRANSPARENT ,
	DAE_TRANSPARENCY ,
	DAE_INDEX_OF_REFRACTION ,
	DAE_IMAGE ,
	DAE_GEOMETRY ,
	DAE_CONTROLLER ,
	DAE_MESH ,
	DAE_FLOAT_ARRAY ,
	DAE_VERTICES ,
	DAE_TRIANGLES ,
	DAE_POLYLIST ,
	DAE_NODE ,
	DAE_TRANSLATE ,
	DAE_ROTATE ,
	DAE_MATRIX ,
	DAE_INSTANCE_CAMERA ,
	DAE_INSTANCE_LIGHT ,
	DAE_INSTANCE_EFFECT ,
	DAE_INSTANCE_MATERIAL ,
	DAE_INSTANCE_GEOMETRY ,
	DAE_INSTANCE_CONTROLLER ,
	DAE_INSTANCE_SCENE ,
	DAE_BIND_MATERIAL ,
	DAE_TECHNIQUE ,
	DAE_TECH_COMMON ,
	DAE_EXTRA
};

static const char* const collada_tags[] =
{
	"asset" ,
	"library_lights" ,
	"library_cameras" ,
	"library_images" ,
	"library_effects" ,
	"library_materials" ,
	"library_geometries" ,
	"library_controllers" ,
	"library_visual_scenes" ,
	"visual_scene" ,
	"scene" ,
	"light" ,
	"directional" ,
	"point",
	"spot",
	"color" ,
	"camera" ,
	"optics" ,
	"perspective" ,
	"yfov" ,
	"aspect_ratio" ,
	"znear" ,
	"zfar" ,
	"material" ,
	"effect" ,
	"newparam" ,
	"surface" ,
	"init_from" ,
	"sampler2D" ,
	"source" ,
	"constant" ,
	"blinn" ,
	"phong" ,
	"lambert" ,
	"emission" ,
	"ambient" ,
	"diffuse" ,
	"texture" ,
	"specular" ,
	"shininess" ,
	"reflective" ,
	"reflectivity" ,
	"transparent" ,
	"transparency" ,
	"index_of_refraction" ,
	"image" ,
	"geometry" ,
	"controller" ,
	"mesh" ,
	"float_array" ,
	"vertices" ,
	"triangles" ,
	"polylist" ,
	"node" ,
	"translate" ,
	"rotate" ,
	"matrix" ,
	"instance_camera" ,
	"instance_light" ,
	"instance_effect" ,
	"instance_material" ,
	"instance_geometry" ,
	"instance_controller" ,
	"instance_scene" ,
	"bind_material" ,
	"technique" ,
	"technique_common" ,
	"extra"
};

static const char* const collada_channel_tags[] =
{
	"scale.X" ,
	"scale.Y" ,
	"scale.Z" ,
	"rotateX.ANGLE" ,
	"rotateY.ANGLE" ,
	"rotateZ.ANGLE" ,
	"location.X" ,
	"location.Y" ,
	"location.Z" ,
	"translate",
	"transform"
};



DaoxIntTuples* DaoxIntTuples_New()
{
	DaoxIntTuples *self = (DaoxIntTuples*) dao_calloc( 1, sizeof(DaoxIntTuples) );
	return self;
}
void DaoxIntTuples_Delete( DaoxIntTuples *self )
{
	if( self->tuples ) dao_free( self->tuples );
	if( self->counts ) dao_free( self->counts );
	dao_free( self );
}
void DaoxIntTuples_Resize( DaoxIntTuples *self, int size )
{
	if( size <= self->capacity ){
		self->size = size;
		return;
	}
	self->size = self->capacity = size;
	self->tuples = (DaoxIntTuple*) dao_realloc( self->tuples, self->size*sizeof(DaoxIntTuple) );
	self->counts = (uint_t*) dao_realloc( self->counts, self->size*sizeof(uint_t) );
}
int DaoxIntTuples_Compare( DaoxIntTuples *self, DaoxIntTuple *first, DaoxIntTuple *second )
{
	int i;
	if( first->index == second->index ) return 0;
	for(i=0; i<self->dim; ++i){
		int v1 = first->values[i];
		int v2 = second->values[i];
		if( v1 != v2 ) return v1 < v2 ? -1 : 1;
	}
	return first->index < second->index ? -1 : 1;
}
/* TODO */
void DaoxIntTuples_RadixRangeSort( DaoxIntTuples *self, int first, int last, int radix )
{
	int i;
	for(i=0; i<self->max; ++i) self->counts[i] = 0;
}
void DaoxIntTuples_RadixSort( DaoxIntTuples *self )
{
	self->max = 0;
	if( self->size <= 1 ) return;
	DaoxIntTuples_RadixRangeSort( self, 0, self->size-1, 0 );
}
void DaoxIntTuples_QuickSort( DaoxIntTuples *self, DaoxIntTuple items[], int first, int last )
{
	DaoxIntTuple pivot, tmp;
	int lower = first + 1;
	int upper = last;

	if( first >= last ) return;
	tmp = items[first];
	items[first] = items[ (first+last)/2 ];
	items[ (first+last)/2 ] = tmp;
	pivot = items[ first ];

	while( lower <= upper ){
		while( lower < last && DaoxIntTuples_Compare( self, & items[lower], & pivot ) < 0 ) lower ++;
		while( upper > first && DaoxIntTuples_Compare( self, & pivot, & items[upper] ) < 0 ) upper --;
		if( lower < upper ){
			tmp = items[lower];
			items[lower] = items[upper];
			items[upper] = tmp;
			upper --;
		}
		lower ++;
	}
	tmp = items[first];
	items[first] = items[upper];
	items[upper] = tmp;
	if( first+1 < upper ) DaoxIntTuples_QuickSort( self, items, first, upper-1 );
	if( upper+1 < last ) DaoxIntTuples_QuickSort( self, items, upper+1, last );
}





DaoxColladaParser* DaoxColladaParser_New( DaoxResource *resource )
{
	size_t i;
	DaoxColladaParser *self = (DaoxColladaParser*) dao_calloc( 1, sizeof(DaoxColladaParser) );
	self->resource = resource;
	self->parser  = DaoXmlParser_New();
	self->dom     = DaoXmlDOM_New();
	self->tuples = DaoxIntTuples_New();
	self->tuples2 = DaoxIntTuples_New();
	self->floats = DArray_New( sizeof(float) );
	self->floats2 = DArray_New( sizeof(float) );
	self->floats3 = DArray_New( sizeof(float) );
	self->integers = DArray_New( sizeof(int) );
	self->integers2 = DArray_New( sizeof(int) );
	self->positions = DArray_New( sizeof(DaoxVector3D) );
	self->normals   = DArray_New( sizeof(DaoxVector3D) );
	self->tangents  = DArray_New( sizeof(DaoxVector3D) );
	self->texcoords = DArray_New( sizeof(DaoxVector2D) );
	self->skinparams = DArray_New( sizeof(DaoxSkinParam) );
	self->indexfloats = DArray_New( sizeof(DaoxIndexFloat) );
	self->string = DString_New(1);
	self->tags = DHash_New( DAO_DATA_STRING, 0 );
	self->channels = DHash_New( DAO_DATA_STRING, 0 );
	self->materials = DHash_New( DAO_DATA_STRING, 0 );
	for(i=1; i<=DAE_EXTRA; ++i){
		DString tag = DString_WrapChars( collada_tags[i-1] );
		DMap_Insert( self->tags, & tag, (void*) i );
	}
	for(i=0; i<=DAOX_ANIMATE_TF; ++i){
		DString tag = DString_WrapChars( collada_channel_tags[i] );
		DMap_Insert( self->channels, & tag, (void*) i );
	}
	return self;
}
void DaoxColladaParser_Delete( DaoxColladaParser *self )
{
	DaoXmlParser_Delete( self->parser );
	DaoXmlDOM_Delete( self->dom );
	DaoxIntTuples_Delete( self->tuples );
	DaoxIntTuples_Delete( self->tuples2 );
	DArray_Delete( self->floats );
	DArray_Delete( self->floats2 );
	DArray_Delete( self->floats3 );
	DArray_Delete( self->integers );
	DArray_Delete( self->integers2 );
	DArray_Delete( self->positions );
	DArray_Delete( self->normals );
	DArray_Delete( self->tangents );
	DArray_Delete( self->texcoords );
	DArray_Delete( self->skinparams );
	DArray_Delete( self->indexfloats );
	DString_Delete( self->string );
	DMap_Delete( self->materials );
	DMap_Delete( self->channels );
	DMap_Delete( self->tags );
	dao_free( self );
}




int DString_ParseFloats( DString *self, DArray *values, int append )
{
	double value;
	char *source = self->chars;
	char *end = source + self->size;
	if( append == 0 ) DArray_Reset( values, 0 );
	while( source < end ){
		char *next = NULL;
		value = strtod( source, & next );
		if( next == source ) return values->size;
		DArray_PushFloat( values, value );
		source = next;
	}
	return values->size;
}
int DString_ParseIntegers( DString *self, DArray *values, int append )
{
	daoint value;
	char *source = self->chars;
	char *end = source + self->size;
	if( append == 0 ) values->size = 0;
	while( source < end ){
		char *next = NULL;
		value = strtol( source, & next, 10 );
		if( next == source ) return values->size;
		DArray_PushInt( values, value );
		source = next;
	}
	return values->size;
}
int DaoxColladaParser_FillArrayOfVector2D( DArray *vectors2d, DArray *floats )
{
	int i;
	DArray_Reset( vectors2d, floats->size/2 );
	for(i=0; i<vectors2d->size; ++i){
		DaoxVector2D *vec = & vectors2d->data.vectors2d[i];
		vec->x = floats->data.floats[2*i];
		vec->y = floats->data.floats[2*i+1];
	}
	return 1;
}
int DaoxColladaParser_FillArrayOfVector3D( DArray *vectors3d, DArray *floats )
{
	int i;
	DArray_Reset( vectors3d, floats->size/3 );
	for(i=0; i<vectors3d->size; ++i){
		DaoxVector3D *vec = & vectors3d->data.vectors3d[i];
		vec->x = floats->data.floats[3*i];
		vec->y = floats->data.floats[3*i+1];
		vec->z = floats->data.floats[3*i+2];
	}
	return 1;
}
void* DaoxColladaParser_GetIntanceDef( DaoxColladaParser *self, DMap *lib, DaoXmlNode *node )
{
	DNode *it;
	DString *att = DaoXmlNode_GetAttributeMBS( node, "url" );
	if( att == NULL ) return NULL;
	DString_SetChars( self->string, att->chars + 1 );
	it = DMap_Find( lib, self->string );
	if( it ) return it->value.pVoid;
	return NULL;
}
int DaoxColladaParser_GetInputOffset( DaoXmlNode *input )
{
	DString *att = DaoXmlNode_GetAttributeMBS( input, "offset" );
	if( att ) return strtol( att->chars, NULL, 10 );
	return 0;
}
DaoXmlNode* DaoxColladaParser_GetInputDataNode( DaoXmlNode *mesh, DaoXmlNode *input )
{
	DString *att = DaoXmlNode_GetAttributeMBS( input, "source" );
	DaoXmlNode *node = DaoXmlNode_GetChildByAttributeMBS( mesh, "id", att->chars + 1 );
	return DaoXmlNode_GetChildMBS( node, "float_array" );
}
DaoxMaterial* DaoxColladaParser_GetMaterial( DaoxColladaParser *self, DString *name )
{
	DNode *it = DMap_Find( self->materials, name );
	if( it == NULL ){
		DaoxMaterial *material = DaoxMaterial_New();
		it = DMap_Insert( self->materials, name, material );
	}
	return (DaoxMaterial*) it->value.pValue;
}
int DaoxColladaParser_Parse( void *userdata, DaoXmlNode *node );
int DaoxColladaParser_HandleGeometry( DaoxColladaParser *self, DaoXmlNode *node )
{
	DaoxResource *resource = self->resource;
	DaoxMesh *mesh = NULL;
	DaoxMeshUnit *unit = NULL;
	DaoXmlNode *node2, *meshNode = NULL, *child = NULL;
	DArray *integers = self->integers;
	DArray *integers2 = self->integers2;
	DArray *floats = self->floats;
	DaoxIntTuples *tuples = self->tuples;
	DaoxIntTuples *tuples2 = self->tuples2;
	DString *att, *string = self->string;
	daoint i, j, k, ii, jj, kk, count;

	printf( "DaoxColladaParser_HandleGeometry\n" );

	if( (child = DaoXmlNode_GetChildMBS( node, "mesh" )) == NULL ) return 0; // TODO
	if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
	node->data = mesh = DaoxMesh_New();
	printf( "DaoxMesh_New: %p\n", node->data );
	DMap_Insert( resource->geometries, att, node->data );
	meshNode = child;
	for(i=0; i<meshNode->children->size; ++i){
		DaoXmlNode *vertInput = NULL, *normInput = NULL, *tanInput = NULL, *texInput = NULL;
		DaoXmlNode *vertData = NULL, *normData = NULL, *tanData = NULL, *texData = NULL;
		int vertOffset = 0, normOffset = 0, tanOffset = 0, texOffset = 0;
		int offset1 = 0, offset2 = 1, offset3 = 2;
		int vertexStride = 1;
		int shapeStride = 3;
		int meshtype = 0;

		child = (DaoXmlNode*) meshNode->children->items.pVoid[i];
		if( strcmp( child->name->chars, "triangles" ) == 0 ){
			meshtype = DAE_TRIANGLES;
		}else if( strcmp( child->name->chars, "polylist" ) == 0 ){
			meshtype = DAE_POLYLIST;
		}
		if( meshtype == 0 ) continue;

		unit = DaoxMesh_AddUnit( mesh );

		att = DaoXmlNode_GetAttributeMBS( child, "material" );
		if( att ){
			DaoxMaterial *material = DaoxColladaParser_GetMaterial( self, att );
			DaoxMeshUnit_SetMaterial( unit, material );
		}

		vertInput = DaoXmlNode_GetChildByAttributeMBS( child, "semantic", "VERTEX" );
		normInput = DaoXmlNode_GetChildByAttributeMBS( child, "semantic", "NORMAL" );
		tanInput  = DaoXmlNode_GetChildByAttributeMBS( child, "semantic", "TANGENT" );
		texInput  = DaoXmlNode_GetChildByAttributeMBS( child, "semantic", "TEXCOORD" );

		if( vertInput ) vertOffset = DaoxColladaParser_GetInputOffset( vertInput );
		if( normInput ) normOffset = DaoxColladaParser_GetInputOffset( normInput );
		if( tanInput )  tanOffset = DaoxColladaParser_GetInputOffset( tanInput );
		if( texInput )  texOffset  = DaoxColladaParser_GetInputOffset( texInput );

		att = DaoXmlNode_GetAttributeMBS( vertInput, "source" );
		vertData = DaoXmlNode_GetChildByAttributeMBS( meshNode, "id", att->chars + 1 );
		vertData = DaoXmlNode_GetChildByAttributeMBS( vertData, "semantic", "POSITION" );
		vertData = DaoxColladaParser_GetInputDataNode( meshNode, vertData );
		if( normInput ) normData = DaoxColladaParser_GetInputDataNode( meshNode, normInput );
		if( tanInput )  tanData  = DaoxColladaParser_GetInputDataNode( meshNode, tanInput );
		if( texInput )  texData  = DaoxColladaParser_GetInputDataNode( meshNode, texInput );

		DString_ParseFloats( vertData->content, floats, 0 );
		DaoxColladaParser_FillArrayOfVector3D( self->positions, floats );
		if( normInput ){
			DString_ParseFloats( normData->content, floats, 0 );
			DaoxColladaParser_FillArrayOfVector3D( self->normals, floats );
		}
		if( tanInput ){
			DString_ParseFloats( tanData->content, floats, 0 );
			DaoxColladaParser_FillArrayOfVector3D( self->tangents, floats );
		}
		if( texInput ){
			DString_ParseFloats( texData->content, floats, 0 );
			DaoxColladaParser_FillArrayOfVector2D( self->texcoords, floats );
		}

		att = DaoXmlNode_GetAttributeMBS( child, "count" );
		if( att == NULL ) return 1; // TODO error;
		count = strtol( att->chars, NULL, 10 );

		node2 = DaoXmlNode_GetChildMBS( child, "vcount" );
		if( node2 ) DString_ParseIntegers( node2->content, integers2, 0 );

		node2 = DaoXmlNode_GetChildMBS( child, "p" );
		if( node2 ) DString_ParseIntegers( node2->content, integers, 0 );

		vertexStride = vertOffset;
		if( normInput && normOffset > vertexStride ) vertexStride = normOffset;
		if( tanInput  && tanOffset  > vertexStride ) vertexStride = tanOffset;
		if( texInput  && texOffset  > vertexStride ) vertexStride = texOffset;
		vertexStride += 1;

		/* Find vertices with common position, normal and texture coordinates: */
		DaoxIntTuples_Resize( tuples, integers->size / vertexStride );
		DaoxIntTuples_Resize( tuples2, integers->size / vertexStride );
		tuples->dim = vertexStride;
		tuples2->dim = vertexStride;
		for(j=0; j<tuples->size; j+=1){
			DaoxIntTuple *tuple = tuples->tuples + j;
			int *values = integers->data.ints + j * vertexStride;
			for(k=0; k<vertexStride; ++k) tuple->values[k] = values[k];
			tuple->index = tuple->reduced = j;
		}
		DaoxIntTuples_QuickSort( tuples, tuples->tuples, 0, tuples->size-1 );
		for(j=1; j<tuples->size; ++j){
			DaoxIntTuple *tuple1 = tuples->tuples + j-1;
			DaoxIntTuple *tuple2 = tuples->tuples + j;
			if( DaoxIntTuples_Compare( tuples, tuple1, tuple2 ) == 0 ){
				tuple2->reduced = tuple1->reduced;
			}
		}
		for(j=0; j<tuples->size; ++j){
			DaoxIntTuple *tuple = tuples->tuples + j;
			tuples2->tuples[tuple->index] = *tuple;
		}
		for(j=0; j<tuples->size; ++j){
			DaoxIntTuple *tuple = tuples2->tuples + j;
			DaoxVertex *vertex;
			if( tuple->reduced != tuple->index ) continue;
			tuple->offset = unit->vertices->size;
			vertex = DArray_PushVertex( unit->vertices, NULL );
			vertex->pos = self->positions->data.vectors3d[ tuple->values[ vertOffset ] ];
			if( normInput ){
				vertex->norm = self->normals->data.vectors3d[ tuple->values[ normOffset ] ];
			}
			if( tanInput ){
				vertex->tan = self->tangents->data.vectors3d[ tuple->values[ tanOffset ] ];
			}
			if( texInput ){
				vertex->tex = self->texcoords->data.vectors2d[ tuple->values[ texOffset ] ];
			}
			if( self->skinparams->size == self->positions->size ){
				DaoxSkinParam *param = (DaoxSkinParam*) DArray_Push( unit->skinParams );
				*param = self->skinparams->data.skinparams[ tuple->values[vertOffset] ];
			}
		}

		if( meshtype == DAE_TRIANGLES ){
			for(j=0; j<tuples->size; j+=3){
				DaoxIntTuple *tuple = tuples2->tuples + j;
				DaoxTriangle *triangle = DArray_PushTriangle( unit->triangles, NULL );
				triangle->index[0] = tuples2->tuples[ tuple[0].reduced ].offset;
				triangle->index[1] = tuples2->tuples[ tuple[1].reduced ].offset;
				triangle->index[2] = tuples2->tuples[ tuple[2].reduced ].offset;
			}
		}else if( meshtype == DAE_POLYLIST ){
			int offset = 0;
			for(j=0; j<integers2->size; ++j){
				DaoxIntTuple *first = tuples2->tuples + offset;
				int m = integers2->data.ints[j] - 1;
				for(k=1; k<m; ++k){
					DaoxIntTuple *second = tuples2->tuples + offset + k;
					DaoxIntTuple *third  = tuples2->tuples + offset + k + 1;
					DaoxTriangle *triangle = DArray_PushTriangle( unit->triangles, NULL );
					triangle->index[0] = tuples2->tuples[ first->reduced ].offset;
					triangle->index[1] = tuples2->tuples[ second->reduced ].offset;
					triangle->index[2] = tuples2->tuples[ third->reduced ].offset;
				}
				offset += m + 1;
			}
		}
		DaoxMeshUnit_UpdateNormTangents( unit, normInput == NULL, tanInput == NULL );
	}
	DaoxMesh_UpdateTree( mesh, 0 );
	DaoxMesh_ResetBoundingBox( mesh );
	DaoxOBBox3D_Print( & mesh->obbox );
	return 1;

ErrorMissingID:
	/*TODO: Error*/
	return 0;
}
int DaoxColladaParser_HandleController( DaoxColladaParser *self, DaoXmlNode *node )
{
	DaoxResource *resource = self->resource;
	DaoxSkeleton *skeleton = NULL;
	DaoxModel *model = NULL;
	DaoxMesh *mesh = NULL;
	DaoxMeshUnit *unit = NULL;
	DaoxMaterial *material = NULL;
	DaoXmlNode *node2, *skinNode = NULL, *child = NULL;
	DaoXmlNode *jointInput = NULL, *bindInput = NULL, *weightInput = NULL;
	DaoXmlNode *namesNode, *nodeVertexWeights = NULL;
	DArray *integers = self->integers;
	DArray *integers2 = self->integers2;
	DArray *floats = self->floats;
	DaoxIntTuples *tuples = self->tuples;
	DaoxIntTuples *tuples2 = self->tuples2;
	DString *att, *string = self->string;
	daoint i, j, k, ii, jj, kk, count;
	int jointInputOffset, weightInputOffset;
	int boneCount;

	printf( "DaoxColladaParser_HandleController\n" );

	if( (skinNode = DaoXmlNode_GetChildMBS( node, "skin" )) == NULL ) return 0; // TODO
	if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;

	skeleton = DaoxSkeleton_New();

	child = DaoXmlNode_GetChildMBS( node, "bind_shape_matrix" );
	if( child ){
		DString_ParseFloats( child->content, floats, 0 );
		skeleton->bindMat = DaoxMatrix4D_InitRowMajor( floats->data.floats );
	}

	if( (child = DaoXmlNode_GetChildMBS( skinNode, "joints" )) == NULL ) return 0; // TODO

	jointInput = DaoXmlNode_GetChildByAttributeMBS( child, "semantic", "JOINT" );
	bindInput  = DaoXmlNode_GetChildByAttributeMBS( child, "semantic", "INV_BIND_MATRIX" );

	if( jointInput == NULL || bindInput == NULL ) return 0; // TODO

	att = DaoXmlNode_GetAttributeMBS( jointInput, "source" );
	node2 = DaoXmlNode_GetChildByAttributeMBS( skinNode, "id", att->chars + 1 );
	namesNode = DaoXmlNode_GetChildMBS( node2, "Name_array" );

	att = DaoXmlNode_GetAttributeMBS( namesNode, "count" );
	boneCount = strtol( att->chars, NULL, 10 );
	DArray_Resize( skeleton->skinMats, boneCount );

	node2 = DaoxColladaParser_GetInputDataNode( skinNode, bindInput );
	DString_ParseFloats( node2->content, floats, 0 );
	for(i=0; i<boneCount; ++i){
		float *mat = floats->data.floats + 16*i;
		skeleton->skinMats->data.matrices4d[i] = DaoxMatrix4D_InitRowMajor( mat );
	}

	nodeVertexWeights = DaoXmlNode_GetChildMBS( skinNode, "vertex_weights" );
	jointInput = DaoXmlNode_GetChildByAttributeMBS( nodeVertexWeights, "semantic", "JOINT" );
	weightInput = DaoXmlNode_GetChildByAttributeMBS( nodeVertexWeights, "semantic", "WEIGHT" );

	jointInputOffset = DaoxColladaParser_GetInputOffset( jointInput );
	weightInputOffset = DaoxColladaParser_GetInputOffset( weightInput );

	node2 = DaoxColladaParser_GetInputDataNode( skinNode, weightInput );
	DString_ParseFloats( node2->content, floats, 0 );

	node2 = DaoXmlNode_GetChildMBS( nodeVertexWeights, "vcount" );
	DString_ParseIntegers( node2->content, integers, 0 );

	node2 = DaoXmlNode_GetChildMBS( nodeVertexWeights, "v" );
	DString_ParseIntegers( node2->content, integers2, 0 );

	DArray_Resize( self->skinparams, integers->size );
	memset( self->skinparams->data.base, 0, self->skinparams->size*sizeof(DaoxSkinParam) );
	for(i=0,k=0; i<integers->size; ++i){
		DaoxSkinParam *param = & self->skinparams->data.skinparams[i];
		double sum = 0.0;
		int m = integers->data.ints[i];
		self->indexfloats->size = 0;
		for(j=0; j<m; ++j){
			int *ids = integers2->data.ints + 2*(k+j);
			int joint = ids[jointInputOffset];
			float weight = floats->data.floats[ids[weightInputOffset]];
			DArray_PushIndexFloat( self->indexfloats, joint, -weight );
		}
		DArray_SortIndexFloats( self->indexfloats );
		if( m > 4 ) m = 4;
		for(j=0; j<m; ++j){
			DaoxIndexFloat *jw = self->indexfloats->data.indexfloats + j;
			param->joints[j] = jw->index;
			param->weights[j] = - jw->value;
			sum += - jw->value;
		}
		if( m != integers->data.ints[i] ){
			for(j=0; j<m; ++j) param->weights[j] /= sum + EPSILON;
		}
		k += integers->data.ints[i];
	}

	if( (att = DaoXmlNode_GetAttributeMBS( skinNode, "source" )) == NULL ) return 0;//TODO
	node2 = DaoXmlNode_GetChildByAttributeMBS( self->libGeometries, "id", att->chars+1 );
	DaoXmlDOM_Traverse( self->dom, node2, self, DaoxColladaParser_Parse );

	node->data = model = DaoxModel_New();
	model->skeleton = skeleton;
	GC_IncRC( model->skeleton );
	DaoxModel_SetMesh( model, (DaoxMesh*) node2->data );

	self->skinparams->size = 0;

	return 1;

ErrorMissingID:
	/*TODO: Error*/
	return 0;
}
int DaoxColladaParser_Parse( void *userdata, DaoXmlNode *node )
{
	DaoxColladaParser *self = (DaoxColladaParser*) userdata;
	DaoxResource *resource = self->resource;
	DaoxScene *scene = NULL;
	DaoxSceneNode *sceneNode = NULL;
	DaoxSceneNode *sceneNode2 = NULL;
	DaoxTexture *texture = NULL;
	DaoxMesh *mesh = NULL;
	DaoxModel *model = NULL;
	DaoxLight *light = NULL;
	DaoxCamera *camera = NULL;
	DaoxMaterial *material = NULL;
	DaoxColor *color = NULL;
	DaoxColor tmpColor;
	DaoxVector3D vector;
	DaoxVector3D vector2;
	DaoxVector3D *pvector;
	DaoxMatrix4D matrix;
	DaoXmlNode *pred = NULL, *child = NULL;
	DaoXmlNode *node2;
	DArray *floats = self->floats;
	DString *att, *att2, *string = self->string;
	DNode *it = DMap_Find( self->tags, node->name );
	DMap *table = NULL;
	daoint i, j, k, id = it ? it->value.pInt : 0;
	daoint ii, jj, kk;
	int instances;
	double dvalue;
	float fvalue;
	void *data;

	if( node->data != NULL ) return 1;

	//printf( "%s %i %i\n", node->name->chars, id, DAE_INSTANCE_GEOMETRY );
	node->id = id;
	switch( id ){
	case DAE_UNSUPPORTED :
	case DAE_ASSET :
	case DAE_LIBRARY_LIGHTS :
	case DAE_LIBRARY_IMAGES :
	case DAE_LIBRARY_MATERIALS :
	case DAE_LIBRARY_GEOMETRIES :
	case DAE_LIBRARY_VISUAL_SCENES :
		break;
	case DAE_VISUAL_SCENE :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
		printf( "DAE_VISUAL_SCENE: %p %p\n", scene, att );
		node->data = scene = DaoxResource_CreateScene( resource );
		DMap_Insert( resource->scenes, att, scene );
		break;
	case DAE_SCENE :
		break;
	case DAE_LIGHT :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
		node->data = DaoxLight_New();
		DMap_Insert( resource->lights, att, node->data );
		break;
	case DAE_CAMERA :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
		node->data = DaoxCamera_New();
		DMap_Insert( resource->cameras, att, node->data );
		break;
	case DAE_IMAGE :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
		node->data = DaoxTexture_New();
		DMap_Insert( resource->images, att, node->data );
		break;
	case DAE_EFFECT :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
		node->data = DaoxMaterial_New();
		DMap_Insert( resource->effects, att, node->data );
		break;
	case DAE_MATERIAL :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
		node->data = DaoxMaterial_New();
		DMap_Insert( resource->materials, att, node->data );
		break;
	case DAE_AMBIENT :
	case DAE_DIRECTIONAL :
	case DAE_POINT :
	case DAE_SPOT :
		light = (DaoxLight*) DaoXmlNode_GetAncestorDataMBS( node, "light", 2 );
		if( light == NULL ) break;
		switch( id ){
		case DAE_AMBIENT     : light->lightType = DAOX_LT_AMBIENT; break;
		case DAE_DIRECTIONAL : light->lightType = DAOX_LT_DIRECTIONAL; break;
		case DAE_POINT       : light->lightType = DAOX_LT_POINT; break;
		case DAE_SPOT        : light->lightType = DAOX_LT_SPOT; break;
		}
		break;
	case DAE_COLOR :
		if( DString_ParseFloats( node->content, floats, 0 ) < 3 ) break;
		tmpColor.red = floats->data.floats[0];
		tmpColor.green = floats->data.floats[1];
		tmpColor.blue = floats->data.floats[2];
		tmpColor.alpha = 1.0;

		if( (light = (DaoxLight*) DaoXmlNode_GetAncestorDataMBS( node, "light", 3 )) ){
			light->intensity = tmpColor;
		}else if( (material = (DaoxMaterial*) DaoXmlNode_GetAncestorDataMBS( node, "effect", 5 )) ){
			switch( node->parent->id ){
			case DAE_EMISSION : material->emission = tmpColor; break;
			case DAE_AMBIENT  : material->ambient  = tmpColor; break;
			case DAE_DIFFUSE  : material->diffuse  = tmpColor; break;
			case DAE_SPECULAR : material->specular = tmpColor; break;
			}
		}
		break;
	case DAE_SHININESS :
		material = (DaoxMaterial*) DaoXmlNode_GetAncestorDataMBS( node, "effect", 4 );
		if( material == NULL ) break;
		node2 = DaoXmlNode_GetChildMBS( node, "float" );
		if( node2 ) material->shininess = strtod( node2->content->chars, NULL );
		break;
	case DAE_OPTICS :
		break;
	case DAE_PERSPECTIVE :
		break;
	case DAE_YFOV :
	case DAE_ASPECT_RATIO :
	case DAE_ZNEAR :
	case DAE_ZFAR :
		camera = (DaoxCamera*) DaoXmlNode_GetAncestorDataMBS( node, "camera", 4 );
		if( camera == NULL ) break;
		dvalue = strtod( node->content->chars, NULL );
		switch( id ){
		case DAE_YFOV :         camera->fovAngle = dvalue; break;
		case DAE_ASPECT_RATIO : camera->aspectRatio = dvalue; break;
		case DAE_ZNEAR :        camera->nearPlane = dvalue; break;
		case DAE_ZFAR :         camera->farPlane = dvalue; break;
		}
		break;
	case DAE_INIT_FROM :
		if( (texture = (DaoxTexture*) DaoXmlNode_GetAncestorDataMBS( node, "image", 1 )) ){
			/* Always assuming relative path for simplicity: */
			DaoxImage *image = DaoxResource_LoadImage( resource, node->content, self->path );
			if( image ) DaoxTexture_SetImage( texture, image );
			node->data = texture;
		}else{
			char *id = node->content->chars;
			node2 = DaoXmlNode_GetChildByAttributeMBS( self->libImages, "id", id );
			if( node2 == NULL ) break; //TODO;
			if( node2->data == NULL ){
				DaoXmlDOM_Traverse( self->dom, node2, self, DaoxColladaParser_Parse );
			}
			node->data = node2->data;
		}
		break;
	case DAE_NEWPARAM :
		break;
	case DAE_CONSTANT :
	case DAE_LAMBERT :
	case DAE_BLINN :
	case DAE_PHONG :
		material = (DaoxMaterial*) DaoXmlNode_GetAncestorDataMBS( node, "effect", 3 );
		if( material == NULL ) break;
		switch( id ){
		case DAE_CONSTANT : material->lighting = DAOX_LM_CONSTANT; break;
		case DAE_LAMBERT  : material->lighting = DAOX_LM_LAMBERT; break;
		case DAE_BLINN    : material->lighting = DAOX_LM_BLINN; break;
		case DAE_PHONG    : material->lighting = DAOX_LM_PHONG; break;
		}
		break;
	case DAE_TEXTURE :
		att = DaoXmlNode_GetAttributeMBS( node, "texture" );
		att2 = DaoXmlNode_GetAttributeMBS( node, "texcoord" );

		material = (DaoxMaterial*) DaoXmlNode_GetAncestorDataMBS( node, "effect", 5 );
		if( material == NULL ) break;

		pred = DaoXmlNode_GetAncestorMBS( node, "profile_COMMON", 4 );
		if( pred == NULL ) break;
		child = DaoXmlNode_GetChildByAttributeMBS( pred, "sid", att->chars );
		child = DaoXmlNode_GetChildMBS( child, "sampler2D" );
		child = DaoXmlNode_GetChildMBS( child, "source" );
		child = DaoXmlNode_GetChildByAttributeMBS( pred, "sid", child->content->chars );
		child = DaoXmlNode_GetChildMBS( child, "surface" );
		child = DaoXmlNode_GetChildMBS( child, "init_from" );
		if( child->data == NULL ) break;
		kk = -1;
		switch( node->parent->id ){
		case DAE_EMISSION : kk = DAOX_EMISSION_TEXTURE; break;
		case DAE_DIFFUSE  : kk = DAOX_DIFFUSE_TEXTURE; break;
		}
		if( kk >= 0 ) DaoxMaterial_SetTexture( material, (DaoxTexture*) child->data, kk );
		break;
	case DAE_NODE :
		sceneNode = NULL;
		att = DaoXmlNode_GetAttributeMBS( node, "type" );
		if( strcmp( att->chars, "JOINT" ) == 0 ){
			node->data = sceneNode = (DaoxSceneNode*) DaoxJoint_New();
			sceneNode2 = (DaoxSceneNode*) DaoXmlNode_GetAncestorDataMBS( node, "node", 1 );
			if( sceneNode2 ){
				DaoxSceneNode_AddChild( sceneNode2, sceneNode );
			}else{
				DaoxScene_AddNode( self->currentScene, sceneNode );
			}
			break;
		}
		/*
		// The instance_* child node will be parsed first to create the node,
		// and then parse the other children nodes.
		*/
		for(i=0,instances=0; i<node->children->size; ++i){
			child = (DaoXmlNode*) node->children->items.pVoid[i];
			if( strncmp( child->name->chars, "instance_", 9 ) != 0 ) continue;
			DaoXmlDOM_Traverse( self->dom, child, self, DaoxColladaParser_Parse );
			if( child->data != NULL ){
				sceneNode = (DaoxSceneNode*) child->data;
				instances += 1;
			}
		}
		/* Create a group node if there are multiple or none instance_* nodes: */
		if( instances != 1 ) sceneNode = DaoxSceneNode_New();
		node->data = sceneNode;
		for(i=0; i<node->children->size; ++i){
			child = (DaoXmlNode*) node->children->items.pVoid[i];
			if( strncmp( child->name->chars, "instance_", 9 ) != 0 ){
				DaoXmlDOM_Traverse( self->dom, child, self, DaoxColladaParser_Parse );
				continue;
			}
			if( child->data != NULL ){
				sceneNode2 = (DaoxSceneNode*) child->data;
				if( sceneNode != sceneNode2 ) DaoxSceneNode_AddChild( sceneNode, sceneNode2 );
			}
		}
		sceneNode2 = (DaoxSceneNode*) DaoXmlNode_GetAncestorDataMBS( node, "node", 1 );
		if( sceneNode2 ){
			DaoxSceneNode_AddChild( sceneNode2, sceneNode );
		}else{
			DaoxScene_AddNode( self->currentScene, sceneNode );
		}
		return 0; /* No longer need to visit/parse the children nodes! */
	case DAE_TRANSLATE :
		sceneNode = (DaoxSceneNode*) DaoXmlNode_GetAncestorDataMBS( node, "node", 1 );
		if( sceneNode == NULL ) break;
		if( DString_ParseFloats( node->content, floats, 0 ) != 3 ) break; // TODO
		sceneNode->translation.x = floats->data.floats[0];
		sceneNode->translation.y = floats->data.floats[1];
		sceneNode->translation.z = floats->data.floats[2];
		break;
	case DAE_ROTATE :
		sceneNode = (DaoxSceneNode*) DaoXmlNode_GetAncestorDataMBS( node, "node", 1 );
		if( sceneNode == NULL ) break;
		if( DString_ParseFloats( node->content, floats, 0 ) != 4 ) break; // TODO
		fvalue = floats->data.floats[3] * M_PI / 180.0;
		pvector = & sceneNode->rotation;
		if( sceneNode->ctype == daox_type_joint ){
			DaoxJoint *joint = (DaoxJoint*) sceneNode;
			att = DaoXmlNode_GetAttributeMBS( node, "sid" );
			if( DString_FindChars( att, "Orient", 0 ) != DAO_NULLPOS ){
				pvector = & joint->orientation;
			}
		}
		if( floats->data.floats[0] > 0.9 ) pvector->x = fvalue; else
		if( floats->data.floats[1] > 0.9 ) pvector->y = fvalue; else
		if( floats->data.floats[2] > 0.9 ) pvector->z = fvalue;
		break;
	case DAE_MATRIX :
		sceneNode = (DaoxSceneNode*) DaoXmlNode_GetAncestorDataMBS( node, "node", 1 );
		if( sceneNode == NULL ) break;

		if( DString_ParseFloats( node->content, floats, 0 ) != 16 ) break; // TODO
		/*
		// Though the collada specification says the matrix is in column order,
		// but it means only for operations (vector*matrix vs matrix*vector).
		// The matrix element should be interpreted as if it is row order!
		// <matrix>
		//   1.0 0.0 0.0 2.0
		//   0.0 1.0 0.0 3.0
		//   0.0 0.0 1.0 4.0
		//   0.0 0.0 0.0 1.0
		// </matrix>
		*/
		matrix = DaoxMatrix4D_InitRowMajor( floats->data.floats );
		if( sceneNode->controller == NULL ) sceneNode->controller = DaoxController_New();
		sceneNode->controller->transform = matrix;
		break;
	case DAE_INSTANCE_CAMERA :
		att = DaoXmlNode_GetAttributeMBS( node, "url" );
		node2 = DaoXmlNode_GetChildByAttributeMBS( self->libCameras, "id", att->chars+1 );
		DaoXmlDOM_Traverse( self->dom, node2, self, DaoxColladaParser_Parse );
		node->data = node2->data;
		if( node->data == NULL ) break; // TODO
		break;
	case DAE_INSTANCE_LIGHT :
		att = DaoXmlNode_GetAttributeMBS( node, "url" );
		node2 = DaoXmlNode_GetChildByAttributeMBS( self->libLights, "id", att->chars+1 );
		DaoXmlDOM_Traverse( self->dom, node2, self, DaoxColladaParser_Parse );
		node->data = node2->data;
		if( node->data == NULL ) break; // TODO
		break;
	case DAE_INSTANCE_EFFECT :
		material = (DaoxMaterial*) DaoXmlNode_GetAncestorDataMBS( node, "material", 1 );
		if( material == NULL ) break;
		att = DaoXmlNode_GetAttributeMBS( node, "url" );
		node2 = DaoXmlNode_GetChildByAttributeMBS( self->libEffects, "id", att->chars+1 );
		DaoXmlDOM_Traverse( self->dom, node2, self, DaoxColladaParser_Parse );
		node->data = node2->data;
		if( node->data ) DaoxMaterial_CopyFrom( material, (DaoxMaterial*) node->data );
		break;
	case DAE_INSTANCE_MATERIAL :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "symbol" )) == NULL ) goto ErrorMissingID;
		if( (att2 = DaoXmlNode_GetAttributeMBS( node, "target" )) ==NULL ) goto ErrorMissingID;
		node2 = DaoXmlNode_GetChildByAttributeMBS( self->libMaterials, "id", att2->chars+1 );
		DaoXmlDOM_Traverse( self->dom, node2, self, DaoxColladaParser_Parse );
		if( node2->data == NULL ) break; // TODO
		material = (DaoxMaterial*) node2->data;
		it = DMap_Find( self->materials, att );
		if( it == NULL ) break; // TODO
		DaoxMaterial_CopyFrom( (DaoxMaterial*) it->value.pValue, material );
		break;
	case DAE_GEOMETRY :
		DaoxColladaParser_HandleGeometry( self, node );
		return 0;
	case DAE_CONTROLLER :
		DaoxColladaParser_HandleController( self, node );
		return 0;
	case DAE_INSTANCE_GEOMETRY :
		att = DaoXmlNode_GetAttributeMBS( node, "url" );
		node2 = DaoXmlNode_GetChildByAttributeMBS( self->libGeometries, "id", att->chars+1 );
		DaoXmlDOM_Traverse( self->dom, node2, self, DaoxColladaParser_Parse );
		if( node2->data == NULL ) break; //XXX
		node->data = model = DaoxModel_New();
		DaoxModel_SetMesh( model, (DaoxMesh*) node2->data );
		DaoxScene_AddNode( self->currentScene, (DaoxSceneNode*) model );
		break;
	case DAE_INSTANCE_CONTROLLER :
		att = DaoXmlNode_GetAttributeMBS( node, "url" );
		node2 = DaoXmlNode_GetChildByAttributeMBS( self->libControllers, "id", att->chars+1 );
		DaoxColladaParser_HandleController( self, node2 );
		node->data = node2->data;
		if( node->data == NULL ) break; // TODO
		DaoxScene_AddNode( self->currentScene, (DaoxSceneNode*) node->data );
		break;
	case DAE_INSTANCE_SCENE :
		break;
	case DAE_TECH_COMMON : break;
	case DAE_EXTRA : return 0;
	default : break;
	}
	return 1;

ErrorMissingID:
	/*TODO: Error*/
	return 0;
}
int DaoxColladaParser_AttachJoint( void *userdata, DaoXmlNode *node )
{
	DaoxColladaParser *self = (DaoxColladaParser*) userdata;
	DString *att;
	if( self->jointName == NULL ) return 0;
	if( self->currentModel == NULL ) return 0;
	if( self->currentModel->skeleton == NULL ) return 0;
	if( strcmp( node->name->chars, "node" ) != 0 ) return 1;
	att = DaoXmlNode_GetAttributeMBS( node, "sid" );
	if( att == NULL ) return 1;
	if( DString_EQ( att, self->jointName ) == 0 ) return 1;
	DList_Append( self->currentModel->skeleton->joints, node->data );
	return 0;
}
int DaoxColladaParser_AttachJoints( void *userdata, DaoXmlNode *node )
{
	DaoxColladaParser *self = (DaoxColladaParser*) userdata;
	DaoxResource *resource = self->resource;
	DaoxModel *model = (DaoxModel*) node->data;
	DaoXmlNode *node2, *skinNode;
	DString *att, *names, *name;
	int i, j, offset = 0;

	if( model == NULL ) return 0; // TODO
	if( strcmp( node->name->chars, "instance_controller" ) != 0 ) return 1;

	att = DaoXmlNode_GetAttributeMBS( node, "url" );
	node2 = DaoXmlNode_GetChildByAttributeMBS( self->libControllers, "id", att->chars+1 );
	skinNode = DaoXmlNode_GetChildMBS( node2, "skin" );

	if( (node2 = DaoXmlNode_GetChildMBS( skinNode, "joints" )) == NULL ) return 0; // TODO
	node2 = DaoXmlNode_GetChildByAttributeMBS( node2, "semantic", "JOINT" );

	if( node2 == NULL ) return 0; // TODO

	att = DaoXmlNode_GetAttributeMBS( node2, "source" );
	node2 = DaoXmlNode_GetChildByAttributeMBS( skinNode, "id", att->chars + 1 );
	node2 = DaoXmlNode_GetChildMBS( node2, "Name_array" );
	names = DString_Copy( node2->content );
	DString_Change( names, "%s+", " ", 0 );
	DString_Trim( names, 1, 1, 0 );
	printf( "bones: %s\n", names->chars );
	name = DString_New();
	self->currentModel = model;
	node2 = self->libVisualScenes;
	while( offset < names->size ){
		int pos = DString_FindChar( names, ' ', offset );
		if( pos == DAO_NULLPOS ) pos = names->size;
		DString_SubString( names, name, offset, pos - offset );
		self->jointName = name;
		DaoXmlDOM_Traverse( self->dom, node2, self, DaoxColladaParser_AttachJoint );
		offset = pos + 1;
	}
	DString_Delete( names );
	DString_Delete( name );
	return 1;
}
int DaoXml_FindNodeByID( void *userdata, DaoXmlNode *node )
{
	DString *name = (DString*) userdata;
	DString *att = DaoXmlNode_GetAttributeMBS( node, "id" );
	return att != NULL && DString_EQ( att, name );
}
DaoXmlNode*  DaoXmlNode_GetSource( DaoXmlNode *host, DaoXmlNode *sampler, const char *semantic )
{
	DaoXmlNode *node = DaoXmlNode_GetChildByAttributeMBS( sampler, "semantic", semantic );
	if( node ){ 
		DString *att = DaoXmlNode_GetAttributeMBS( node, "source" );
		return  DaoXmlNode_GetChildByAttributeMBS( host, "id", att->chars+1 );
	}
	return NULL;
}
void DaoxColladaParser_ParseAnimation( DaoxColladaParser *self, DaoXmlNode *node )
{
	DaoValue *dvalue;
	DaoxSceneNode *sceneNode;
	DaoxAnimation *animation;
	DaoxKeyFrame staticFrame;
	DaoXmlNode *vscenes = self->libVisualScenes;
	DaoXmlNode *channelNode = DaoXmlNode_GetChildMBS( node, "channel" );
	DaoXmlNode *inputNode, *outputNode, *interNode;
	DaoXmlNode *intanNode, *outtanNode;
	DaoXmlNode *samplerNode, *targetNode;
	DaoXmlNode *node2;
	DString *att;
	DNode *it;
	int channel;
	int i, pos;

	if( channelNode == NULL ) return; // TODO

	att = DaoXmlNode_GetAttributeMBS( channelNode, "target" );
	pos = DString_FindChar( att, '/', 0 );
	DString_SubString( att, self->string, 0, pos );

	targetNode = DaoXmlDOM_Search( self->dom, vscenes, self->string, DaoXml_FindNodeByID );

	if( targetNode == NULL || targetNode->data == NULL ) return; // TODO
	printf( "animation: %s %p\n", self->string->chars, targetNode->data );

	DString_SubString( att, self->string, pos+1, -1 );
	it = DMap_Find( self->channels, self->string );
	if( it == NULL ) return;
	channel = it->value.pInt;

	dvalue = (DaoValue*) targetNode->data;
	sceneNode = (DaoxSceneNode*) DaoValue_CastCstruct( dvalue, daox_type_scene_node );
	printf( "animation: %s %p\n", self->string->chars, sceneNode );
	if( sceneNode == NULL ) return;

	att = DaoXmlNode_GetAttributeMBS( channelNode, "source" );
	samplerNode = DaoXmlNode_GetChildByAttributeMBS( node, "id", att->chars+1 );

	inputNode = DaoXmlNode_GetSource( node, samplerNode, "INPUT" );
	outputNode = DaoXmlNode_GetSource( node, samplerNode, "OUTPUT" );
	interNode = DaoXmlNode_GetSource( node, samplerNode, "INTERPOLATION" );
	intanNode = DaoXmlNode_GetSource( node, samplerNode, "IN_TANGENT" );
	outtanNode = DaoXmlNode_GetSource( node, samplerNode, "OUT_TANGENT" );

	if( inputNode == NULL || outputNode == NULL || interNode == NULL ) return; //TODO

	printf( "animation: %s %p\n", self->string->chars, interNode );

	animation = DaoxAnimation_New();
	animation->channel = channel;
	if( sceneNode->controller == NULL ) sceneNode->controller = DaoxController_New();
	if( sceneNode->controller->animations == NULL ){
		sceneNode->controller->animations = DList_New( DAO_DATA_VALUE );
	}
	DList_Append( sceneNode->controller->animations, animation );

	node2 = DaoXmlNode_GetChildMBS( inputNode, "float_array" );
	DString_ParseFloats( node2->content, self->floats, 0 );
	DArray_Resize( animation->keyFrames, self->floats->size );
	memset( & staticFrame, 0, sizeof(DaoxKeyFrame) );
	staticFrame.matrix = DaoxMatrix4D_Identity();
	for(i=0; i<animation->keyFrames->size; ++i){
		animation->keyFrames->data.keyframes[i] = staticFrame;
		animation->keyFrames->data.keyframes[i].time = self->floats->data.floats[i];
	}
	node2 = DaoXmlNode_GetChildMBS( outputNode, "float_array" );
	DString_ParseFloats( node2->content, self->floats, 0 );
	// TODO: check size;
	for(i=0; i<animation->keyFrames->size; ++i){
		DaoxKeyFrame *frame = animation->keyFrames->data.keyframes + i;
		float *floats;
		switch( channel ){
		case DAOX_ANIMATE_SX : case DAOX_ANIMATE_SY : case DAOX_ANIMATE_SZ :
		case DAOX_ANIMATE_TX : case DAOX_ANIMATE_TY : case DAOX_ANIMATE_TZ :
			frame->scalar = self->floats->data.floats[i];
			break;
		case DAOX_ANIMATE_RX : case DAOX_ANIMATE_RY : case DAOX_ANIMATE_RZ :
			frame->scalar = self->floats->data.floats[i] * M_PI/180.0;
			break;
		case DAOX_ANIMATE_TL :
			floats = self->floats->data.floats + 3*i;
			frame->vector = DaoxVector3D_XYZ( floats[0], floats[1], floats[2] );
			break;
		case DAOX_ANIMATE_TF :
			floats = self->floats->data.floats + 16*i;
			frame->matrix = DaoxMatrix4D_InitRowMajor( floats );
			break;
		}
	}
	node2 = DaoXmlNode_GetChildMBS( interNode, "Name_array" );
	DString_Assign( self->string, node2->content );
	DString_Change( self->string, "LINEAR", "0", 0 );
	DString_Change( self->string, "BEZIER", "1", 0 );
	DString_Change( self->string, "HERMITE", "2", 0 );
	DString_Change( self->string, "BSPLINE", "3", 0 );
	DString_ParseIntegers( self->string, self->integers, 0 );
	for(i=0; i<animation->keyFrames->size; ++i){
		DaoxKeyFrame *frame = animation->keyFrames->data.keyframes + i;
		frame->curve = self->integers->data.ints[i];
	}
	if( intanNode && outtanNode ){
		int stride;
		node2 = DaoXmlNode_GetChildMBS( intanNode, "float_array" );
		DString_ParseFloats( node2->content, self->floats, 0 );
		stride = self->floats->size / animation->keyFrames->size;
		if( stride == 6 ){
			// time, x, time, y, time, z;
			// TODO: interpolate separately?
			self->floats->size = self->floats->size / 2;
			for(i=0; i<self->floats->size; ++i){
				self->floats->data.floats[i] = self->floats->data.floats[2*i+1];
			}
			stride = 3;
		}
		for(i=0; i<animation->keyFrames->size; ++i){
			DaoxKeyFrame *frame = animation->keyFrames->data.keyframes + i;
			float *floats = self->floats->data.floats + i*stride;
			switch( stride ){
			case 3: frame->tangent1.z = floats[2];
			case 2: frame->tangent1.y = floats[1];
			case 1: frame->tangent1.x = floats[0];
			}
			if( channel >= DAOX_ANIMATE_RX && channel <= DAOX_ANIMATE_RZ ){
				frame->tangent1.y *= M_PI / 180.0;
			}
		}
		node2 = DaoXmlNode_GetChildMBS( outtanNode, "float_array" );
		DString_ParseFloats( node2->content, self->floats, 0 );
		stride = self->floats->size / animation->keyFrames->size;
		if( stride == 6 ){
			// time, x, time, y, time, z;
			// TODO: interpolate separately?
			self->floats->size = self->floats->size / 2;
			for(i=0; i<self->floats->size; ++i){
				self->floats->data.floats[i] = self->floats->data.floats[2*i+1];
			}
			stride = 3;
		}
		for(i=0; i<animation->keyFrames->size; ++i){
			DaoxKeyFrame *frame = animation->keyFrames->data.keyframes + i;
			float *floats = self->floats->data.floats + i*stride;
			switch( stride ){
			case 3: frame->tangent2.z = floats[2];
			case 2: frame->tangent2.y = floats[1];
			case 1: frame->tangent2.x = floats[0];
			}
			if( channel >= DAOX_ANIMATE_RX && channel <= DAOX_ANIMATE_RZ ){
				frame->tangent2.y *= M_PI / 180.0;
			}
		}
	}
}

DaoxScene* DaoxResource_LoadColladaSource( DaoxResource *self, DString *source, DString *path )
{
	DaoxColladaParser *parser = DaoxColladaParser_New( self );
	DaoXmlDOM *dom = parser->dom;
	DaoxScene *scene = NULL;
	DString *att;
	int i;

	parser->path = path;
	DaoXmlParser_Parse( parser->parser, parser->dom, source );

	parser->libLights = DaoXmlNode_GetChildMBS( dom->root, "library_lights" );
	parser->libCameras = DaoXmlNode_GetChildMBS( dom->root, "library_cameras" );
	parser->libImages  = DaoXmlNode_GetChildMBS( dom->root, "library_images" );
	parser->libEffects = DaoXmlNode_GetChildMBS( dom->root, "library_effects" );
	parser->libMaterials = DaoXmlNode_GetChildMBS( dom->root, "library_materials" );
	parser->libGeometries = DaoXmlNode_GetChildMBS( dom->root, "library_geometries" );
	parser->libControllers = DaoXmlNode_GetChildMBS( dom->root, "library_controllers" );
	parser->libAnimations = DaoXmlNode_GetChildMBS( dom->root, "library_animations" );
	parser->libVisualScenes = DaoXmlNode_GetChildMBS( dom->root, "library_visual_scenes" );

	for(i=0; i<dom->root->children->size; ++i){
		DaoXmlNode *node = (DaoXmlNode*) dom->root->children->items.pVoid[i];
		if( strcmp( node->name->chars, "scene" ) != 0 ) continue;
		parser->currentScene = DaoxResource_CreateScene( self );

		node = DaoXmlNode_GetChildMBS( node, "instance_visual_scene" );

		att = DaoXmlNode_GetAttributeMBS( node, "url" );
		node = DaoXmlNode_GetChildByAttributeMBS( parser->libVisualScenes, "id", att->chars+1);
		DaoXmlDOM_Traverse( dom, node, parser, DaoxColladaParser_Parse );
		DaoXmlDOM_Traverse( dom, node, parser, DaoxColladaParser_AttachJoints );
	}
	if( parser->libAnimations ){
		for(i=0; i<parser->libAnimations->children->size; ++i){
			DaoXmlNode *node = (DaoXmlNode*) parser->libAnimations->children->items.pVoid[i];
			if( strcmp( node->name->chars, "animation" ) != 0 ) continue;
			DaoxColladaParser_ParseAnimation( parser, node );
		}
	}
	scene = parser->currentScene;
	DaoxColladaParser_Delete( parser );
	printf( "Scene: %i nodes; %i lights; %p\n", scene->nodes->size, scene->lights->size, scene->camera );
	return scene;
}
DaoxScene* DaoxResource_LoadColladaFile( DaoxResource *self, DString *file, DString *path )
{
	DaoxScene *scene = NULL;
	DString *source = DString_New();

	file = DString_Copy( file );
	//printf( "DaoxResource_LoadColladaFile: %s %s\n", file->chars, path->chars );
	if( DaoxResource_SearchFile( self, file, path ) ){
		if( DaoxResource_ReadFile( self, file, source ) ){
			DString_Change( file, "[^/\\]* $", "", 0 );
			scene = DaoxResource_LoadColladaSource( self, source, file );
		}
	}
	DString_Delete( source );
	DString_Delete( file );
	return scene;
}


