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
				which = 1;
			}else{
				which = 2;
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
	"instance_scene" ,
	"bind_material" ,
	"technique" ,
	"technique_common" ,
	"extra"
};
static const char* const collada_main_tags[] =
{
	"library_lights" ,
	"library_cameras" ,
	"library_images" ,
	"library_effects" ,
	"library_materials" ,
	"library_geometries" ,
	"library_visual_scenes" ,
	NULL
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
	self->xmlParser  = DaoXmlParser_New();
	self->xmlDOM     = DaoXmlDOM_New();
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
	self->string = DString_New(1);
	self->tags = DHash_New( DAO_DATA_STRING, 0 );
	for(i=1; i<=DAE_EXTRA; ++i){
		DString tag = DString_WrapChars( collada_tags[i-1] );
		DMap_Insert( self->tags, & tag, (void*) i );
	}
	return self;
}
void DaoxColladaParser_Delete( DaoxColladaParser *self )
{
	DaoXmlParser_Delete( self->xmlParser );
	DaoXmlDOM_Delete( self->xmlDOM );
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
	DString_Delete( self->string );
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
	DaoXmlNode *node = DaoXmlNode_GetChildWithAttributeMBS( mesh, "id", att->chars + 1 );
	return DaoXmlNode_GetChildMBS( node, "float_array" );
}
int DaoxColladaParser_HandleGeometry( DaoxColladaParser *self, DaoXmlNode *node )
{
	DaoxResource *resource = self->resource;
	DaoxMesh *mesh = NULL;
	DaoxMeshUnit *unit = NULL;
	DaoxMaterial *material = NULL;
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
		material = DaoxMaterial_New();
		DaoxMeshUnit_SetMaterial( unit, material );


		att = DaoXmlNode_GetAttributeMBS( child, "material" );
		if( att ) DString_Assign( material->name, att );

		vertInput = DaoXmlNode_GetChildWithAttributeMBS( child, "semantic", "VERTEX" );
		normInput = DaoXmlNode_GetChildWithAttributeMBS( child, "semantic", "NORMAL" );
		tanInput  = DaoXmlNode_GetChildWithAttributeMBS( child, "semantic", "TANGENT" );
		texInput  = DaoXmlNode_GetChildWithAttributeMBS( child, "semantic", "TEXCOORD" );

		if( vertInput ) vertOffset = DaoxColladaParser_GetInputOffset( vertInput );
		if( normInput ) normOffset = DaoxColladaParser_GetInputOffset( normInput );
		if( tanInput )  tanOffset = DaoxColladaParser_GetInputOffset( tanInput );
		if( texInput )  texOffset  = DaoxColladaParser_GetInputOffset( texInput );

		att = DaoXmlNode_GetAttributeMBS( vertInput, "source" );
		vertData = DaoXmlNode_GetChildWithAttributeMBS( meshNode, "id", att->chars + 1 );
		vertData = DaoXmlNode_GetChildWithAttributeMBS( vertData, "semantic", "POSITION" );
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
int DaoxResource_ColladaVisit( void *userdata, DaoXmlNode *node )
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
	DaoxMatrix4D matrix;
	DaoXmlNode *pred = NULL, *child = NULL;
	DArray *floats = self->floats;
	DString *att, *att2, *string = self->string;
	DNode *it = DMap_Find( self->tags, node->name );
	DMap *table = NULL;
	daoint i, j, k, id = it ? it->value.pInt : 0;
	daoint ii, jj, kk;
	double dvalue;
	float fvalue;
	void *data;

	printf( "%s %i %i\n", node->name->chars, id, DAE_INSTANCE_GEOMETRY );
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
			DString path = DString_WrapChars( "." );
			DaoxImage *image = DaoxResource_LoadImage( resource, node->content, & path );
			if( image ) DaoxTexture_SetImage( texture, image );
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
		child = DaoXmlNode_GetChildWithAttributeMBS( pred, "sid", att->chars );
		child = DaoXmlNode_GetChildMBS( child, "sampler2D" );
		child = DaoXmlNode_GetChildMBS( child, "source" );
		child = DaoXmlNode_GetChildWithAttributeMBS( pred, "sid", child->content->chars );
		child = DaoXmlNode_GetChildMBS( child, "surface" );
		child = DaoXmlNode_GetChildMBS( child, "init_from" );
		it = DMap_Find( resource->images, child->content );
		if( it == NULL ) break;
		printf( "DAE_TEXTURE-------------------------- %p\n", material );
		DaoxMaterial_SetTexture( material, (DaoxTexture*) it->value.pVoid, 1 );
		break;
	case DAE_NODE :
		scene = (DaoxScene*) DaoXmlNode_GetAncestorDataMBS( node, "visual_scene", 1 );
		/*
		// The instance_* child node will be parsed first to create the node,
		// and then parse the other children nodes.
		*/
		for(i=0,k=0; i<node->children->size; ++i){
			child = (DaoXmlNode*) node->children->items.pVoid[i];
			if( strncmp( child->name->chars, "instance_", 9 ) != 0 ) continue;
			DaoXmlDOM_Traverse( self->xmlDOM, child, self, DaoxResource_ColladaVisit );
			node->data = child->data;
			k += 1;
			printf( "%3i: %s %p\n", i, child->name->chars, child->data );
		}
		sceneNode = (DaoxSceneNode*) node->data;
		/* Create a group node if there are multiple or none instance_* nodes: */
		if( sceneNode == NULL || k > 1 ){
			node->data = sceneNode = DaoxSceneNode_New();
			//DaoxScene_AddNode( scene, sceneNode );
			printf( "DaoxScene_AddNode: %p %p\n", scene, sceneNode );
		}
		for(i=0; i<node->children->size; ++i){
			child = (DaoXmlNode*) node->children->items.pVoid[i];
			printf( "%3i: %s %p\n", i, child->name->chars, child->data );
			if( strncmp( child->name->chars, "instance_", 9 ) == 0 && child->data != NULL ){
				sceneNode2 = (DaoxSceneNode*) child->data;
				if( sceneNode != sceneNode2 ) DaoxSceneNode_AddChild( sceneNode, sceneNode2 );
				continue;
			}
			DaoXmlDOM_Traverse( self->xmlDOM, child, self, DaoxResource_ColladaVisit );
		}
		sceneNode2 = (DaoxSceneNode*) DaoXmlNode_GetAncestorDataMBS( node, "node", 1 );
		if( sceneNode2 ){
			DaoxSceneNode_AddChild( sceneNode2, sceneNode );
		}else{
			printf( "DaoxScene_AddNode: %p %p\n", scene, sceneNode );
			DaoxScene_AddNode( scene, sceneNode );
		}
		if( sceneNode2 == NULL ) return 0;
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
		if( floats->data.floats[0] > 0.9 ) sceneNode->rotation.x = fvalue; else
		if( floats->data.floats[1] > 0.9 ) sceneNode->rotation.y = fvalue; else
		if( floats->data.floats[2] > 0.9 ) sceneNode->rotation.z = fvalue;
		break;
	case DAE_MATRIX :
		sceneNode = (DaoxSceneNode*) DaoXmlNode_GetAncestorDataMBS( node, "node", 1 );
		if( sceneNode == NULL ) break;

		if( DString_ParseFloats( node->content, floats, 0 ) != 16 ) break; // TODO
		matrix = DaoxMatrix4D_InitColumnMajor( floats->data.floats );
#warning"DAE_MATRIX"
		//sceneNode->transform = DaoxMatrix4D_MulMatrix( & matrix, & sceneNode->transform );
		break;
	case DAE_INSTANCE_CAMERA :
		node->data = camera = DaoxCamera_New();
		data = DaoxColladaParser_GetIntanceDef( self, resource->cameras, node );
		if( data ) DaoxCamera_CopyFrom( camera, (DaoxCamera*) data );
		break;
	case DAE_INSTANCE_LIGHT :
		node->data = light = DaoxLight_New();
		data = DaoxColladaParser_GetIntanceDef( self, resource->lights, node );
		if( data ) DaoxLight_CopyFrom( light, (DaoxLight*) data );
		break;
	case DAE_INSTANCE_EFFECT :
		material = (DaoxMaterial*) DaoXmlNode_GetAncestorDataMBS( node, "material", 1 );
		if( material == NULL ) break;

		data = DaoxColladaParser_GetIntanceDef( self, resource->effects, node );
		if( data ) DaoxMaterial_CopyFrom( material, (DaoxMaterial*) data );
		printf( "DAE_INSTANCE_EFFECT-------------------------- %p %p\n", material, material->texture1 );
		break;
	case DAE_INSTANCE_MATERIAL :
		model = (DaoxModel*) DaoXmlNode_GetAncestorDataMBS( node, "instance_geometry", 3 );
		if( model == NULL || model->mesh == NULL ) break; // XXX;
		if( (att = DaoXmlNode_GetAttributeMBS( node, "symbol" )) == NULL ) goto ErrorMissingID;
		if( (att2 = DaoXmlNode_GetAttributeMBS( node, "target" )) == NULL ) goto ErrorMissingID;
		DString_SetChars( string, att2->chars + 1 );
		it = DMap_Find( resource->materials, string );
		printf( "%p\n", it );
		if( it == NULL ) break; //XXX
		material = (DaoxMaterial*) it->value.pVoid;
		printf( "DAE_INSTANCE_MATERIAL-------------------------- %p %p\n", material, material->texture1 );
		for(i=0; i<model->mesh->units->size; ++i){
			DaoxMeshUnit *unit = (DaoxMeshUnit*) model->mesh->units->items.pVoid[i];
			if( unit->material == NULL ) continue;
			printf( "%i %s, %s  %p %p\n", i, unit->material->name->chars, att->chars, unit, unit->material );
			if( DString_EQ( unit->material->name, att ) == 0 ) continue;
			DaoxMaterial_CopyFrom( unit->material, material );
		}
		break;
	case DAE_GEOMETRY :
		DaoxColladaParser_HandleGeometry( self, node );
		return 0;
	case DAE_INSTANCE_GEOMETRY :
		scene = (DaoxScene*) DaoXmlNode_GetAncestorDataMBS( node, "visual_scene", 2 );
		if( scene == NULL ) break; // XXX;
		//node->data = model = DaoxModel_New(); // TODO
		//DaoxScene_AddNode( scene, (DaoxSceneNode*) model );
		data = DaoxColladaParser_GetIntanceDef( self, resource->geometries, node );
		if( data == NULL ) break; //XXX
		node->data = model = DaoxModel_New();
		DaoxModel_SetMesh( model, (DaoxMesh*) data );
		DaoxScene_AddNode( scene, (DaoxSceneNode*) model );
		printf( "DAE_INSTANCE_GEOMETRY: %p\n", node->data );
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


DaoxScene* DaoxResource_LoadColladaSource( DaoxResource *self, DString *source, DString *path )
{
	DaoxColladaParser *parser = DaoxColladaParser_New( self );
	DaoXmlDOM *dom = parser->xmlDOM;
	int i;

	parser->path = path;
	DaoXmlParser_Parse( parser->xmlParser, parser->xmlDOM, source );

	/* The parsing order the sections are rearranged for convenience: */
	for(i=0; collada_main_tags[i]; ++i){
		DaoXmlNode *node = DaoXmlNode_GetChildMBS( dom->root, collada_main_tags[i] );
		DaoXmlDOM_Traverse( dom, node, parser, DaoxResource_ColladaVisit );
	}
	DaoxColladaParser_Delete( parser );
	return DaoxResource_GetScene( self );
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


