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
