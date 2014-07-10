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


#include "dao_collada.h"
#include "dao_xml.h"


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
	DAE_VISUAL_SCENE ,
	DAE_NODE ,
	DAE_MATRIX ,
	DAE_INSTANCE_CAMERA ,
	DAE_INSTANCE_LIGHT ,
	DAE_INSTANCE_EFFECT ,
	DAE_INSTANCE_MATERIAL ,
	DAE_INSTANCE_GEOMETRY ,
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
	"visual_scene" ,
	"node" ,
	"matrix" ,
	"instance_camera" ,
	"instance_light" ,
	"instance_effect" ,
	"instance_material" ,
	"instance_geometry" ,
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
	for(i=0; i<self->dim; ++i){
		int v1 = first->values[i];
		int v2 = second->values[i];
		if( v1 != v2 ) return v1 < v2 ? -1 : 1;
	}
	return 0;
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





DaoxColladaParser* DaoxColladaParser_New()
{
	size_t i;
	DaoxColladaParser *self = (DaoxColladaParser*) dao_calloc( 1, sizeof(DaoxColladaParser) );
	self->tuples = DaoxIntTuples_New();
	self->floats = DArray_New( sizeof(float) );
	self->floats2 = DArray_New( sizeof(float) );
	self->floats3 = DArray_New( sizeof(float) );
	self->integers = DArray_New( sizeof(int) );
	self->integers2 = DArray_New( sizeof(int) );
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
	DaoxIntTuples_Delete( self->tuples );
	DArray_Delete( self->floats );
	DArray_Delete( self->floats2 );
	DArray_Delete( self->floats3 );
	DArray_Delete( self->integers );
	DArray_Delete( self->integers2 );
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
void* DaoxSceneResource_GetIntanceDef( DaoxSceneResource *self, DMap *lib, DaoXmlNode *node )
{
	DNode *it;
	DString *att = DaoXmlNode_GetAttributeMBS( node, "url" );
	if( att == NULL ) return NULL;
	DString_SetChars( self->collada->string, att->chars + 1 );
	it = DMap_Find( lib, self->collada->string );
	if( it ) return it->value.pVoid;
	return NULL;
}
int DaoxSceneResource_HandleColladaGeometry( DaoxSceneResource *self, DaoXmlNode *node )
{
	DaoxMesh *mesh = NULL;
	DaoxMeshUnit *unit = NULL;
	DaoxMaterial *material = NULL;
	DaoxColladaParser *handle = self->collada;
	DaoXmlNode *node1, *node2, *node3, *node4;
	DaoXmlNode *pred = NULL, *child = NULL;
	DArray *integers = handle->integers;
	DArray *integers2 = handle->integers2;
	DArray *floats = handle->floats;
	DArray *floats2 = handle->floats2;
	DArray *floats3 = handle->floats3;
	DaoxIntTuples *tuples = handle->tuples;
	DaoxIntTuple *intup = NULL;
	DString *att, *att2, *string = handle->string;
	daoint i, j, k, ii, jj, kk, count;

	printf( "DaoxSceneResource_HandleColladaGeometry\n" );

	if( (child = DaoXmlNode_GetChildMBS( node, "mesh" )) == NULL ) return 0; // TODO
	if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
	node->data = mesh = DaoxMesh_New();
	printf( "DaoxMesh_New: %p\n", node->data );
	DMap_Insert( self->geometries, att, node->data );
	pred = child;
	for(i=0; i<pred->children->size; ++i){
		int offset1 = 0, offset2 = 1, offset3 = 2;
		int vertexStride = 1;
		int shapeStride = 3;

		child = (DaoXmlNode*) pred->children->items.pVoid[i];
		if( strcmp( child->name->chars, "triangles" ) != 0 ) continue;

		unit = DaoxMesh_AddUnit( mesh );
		material = DaoxMaterial_New();
		DaoxMeshUnit_SetMaterial( unit, material );


		att = DaoXmlNode_GetAttributeMBS( child, "material" );
		if( att ) DString_Assign( material->name, att );

		node1 = DaoXmlNode_GetChildWithAttributeMBS( child, "semantic", "VERTEX" );
		node2 = DaoXmlNode_GetChildWithAttributeMBS( child, "semantic", "NORMAL" );
		node3 = DaoXmlNode_GetChildWithAttributeMBS( child, "semantic", "TEXCOORD" );

		att2 = DaoXmlNode_GetAttributeMBS( node1, "offset" );
		if( att2 ) offset1 = strtol( att2->chars, NULL, 10 );
		if( node2 ){
			att2 = DaoXmlNode_GetAttributeMBS( node2, "offset" );
			if( att2 ) offset2 = strtol( att2->chars, NULL, 10 );
		}
		if( node3 ){
			att2 = DaoXmlNode_GetAttributeMBS( node3, "offset" );
			if( att2 ) offset3 = strtol( att2->chars, NULL, 10 );
		}

		att2 = DaoXmlNode_GetAttributeMBS( node1, "source" );
		printf( "%p %s %s\n", att2, att2->chars, child->name->chars );
		node4 = DaoXmlNode_GetChildWithAttributeMBS( pred, "id", att2->chars + 1 );
		printf( "%p\n", node4 );
		node4 = DaoXmlNode_GetChildWithAttributeMBS( node4, "semantic", "POSITION" );
		att2 = DaoXmlNode_GetAttributeMBS( node4, "source" );
		node4 = DaoXmlNode_GetChildWithAttributeMBS( pred, "id", att2->chars + 1 );
		node4 = DaoXmlNode_GetChildMBS( node4, "float_array" );
		DString_ParseFloats( node4->content, floats, 0 );
		printf( "floats: %i\n", floats->size );

		if( node2 ){
			att2 = DaoXmlNode_GetAttributeMBS( node2, "source" );
			node4 = DaoXmlNode_GetChildWithAttributeMBS( pred, "id", att2->chars + 1 );
			node4 = DaoXmlNode_GetChildMBS( node4, "float_array" );
			DString_ParseFloats( node4->content, floats2, 0 );
		}

		if( node3 ){
			att2 = DaoXmlNode_GetAttributeMBS( node3, "source" );
			node4 = DaoXmlNode_GetChildWithAttributeMBS( pred, "id", att2->chars + 1 );
			node4 = DaoXmlNode_GetChildMBS( node4, "float_array" );
			DString_ParseFloats( node4->content, floats3, 0 );
		}

		for(j=0; j<floats->size; j+=3){
			DaoxVertex *vertex = DArray_PushVertex( unit->vertices, NULL );
			vertex->point.x = floats->data.floats[j];
			vertex->point.y = floats->data.floats[j+1];
			vertex->point.z = floats->data.floats[j+2];
		}
		integers->size = 0;
		node4 = DaoXmlNode_GetChildMBS( child, "p" );
		if( node4 ) DString_ParseIntegers( node4->content, integers, 0 );

		att2 = DaoXmlNode_GetAttributeMBS( child, "count" );
		if( att2 == NULL ) return 1; // TODO error;
		count = strtol( att2->chars, NULL, 10 );
		vertexStride = offset1;
		if( node2 && offset2 > vertexStride ) vertexStride = offset2;
		if( node3 && offset3 > vertexStride ) vertexStride = offset3;
		vertexStride += 1;
		shapeStride = integers->size / count;
		printf( "stride: %i %i\n", vertexStride, shapeStride );
		if( shapeStride != 3*vertexStride ) return 1; // TODO error;

		DaoxIntTuples_Resize( tuples, integers->size / vertexStride );
		tuples->dim = vertexStride;
		for(j=0; j<integers->size; j+=vertexStride){
			DaoxIntTuple *tuple = tuples->tuples + j/vertexStride;
			int *values = integers->data.ints + j;
			for(k=0; k<vertexStride; ++k) tuple->values[k] = values[k];
			tuple->index = j;
		}
#if 0
		for(j=0; j<integers->size; j+=vertexStride){
			int *values = integers->data.ints + j;
			printf( "%5i:", j/vertexStride );
			for(k=0; k<vertexStride; ++k) printf( " %5i", values[k] );
			printf( "\n" );
		}
		for(j=0; j<tuples->size; ++j){
			DaoxIntTuple *tuple = tuples->tuples + j;
			printf( "%5i: %5i", j, tuple->index );
			for(k=0; k<vertexStride; ++k) printf( " %5i", tuple->values[k] );
			printf( "\n" );
		}
#endif
		DaoxIntTuples_QuickSort( tuples, tuples->tuples, 0, tuples->size-1 );
#if 0
		for(j=0; j<tuples->size; ++j){
			DaoxIntTuple *tuple = tuples->tuples + j;
			printf( "%5i: %5i", j, tuple->index );
			for(k=0; k<vertexStride; ++k) printf( " %5i", tuple->values[k] );
			printf( "\n" );
		}
#endif
		intup = tuples->tuples;
		for(j=1; j<tuples->size; ){
			while( DaoxIntTuples_Compare( tuples, tuples->tuples+j, intup ) == 0 ){
				daoint idx = tuples->tuples[j].index + offset1;
				daoint idx2 = intup->index + offset1;
				DaoxVertex *vertex = unit->vertices->data.vertices + integers->data.ints[idx];
				DaoxVertex *vertex2 = unit->vertices->data.vertices + integers->data.ints[idx2];
				integers->data.ints[idx] = integers->data.ints[idx2];
				j += 1;
				if( j >= tuples->size ) break;
			}
			if( j >= tuples->size ) break;
			if( tuples->tuples[j].values[offset1] == intup->values[offset1] ){
				daoint idx = tuples->tuples[j].index + offset1;
				daoint idx2 = intup->index + offset1;
				DaoxVertex *vertex = DArray_PushVertex( unit->vertices, NULL );
				DaoxVertex *vertex2 = unit->vertices->data.vertices + integers->data.ints[idx2];
				integers->data.ints[idx] = unit->vertices->size - 1;
				vertex->point = vertex2->point;
			}
			intup = tuples->tuples + j;
		}
		printf( "final number of vertice: %i\n", unit->vertices->size );
#if 0
		for(j=0; j<integers->size; j+=vertexStride){
			int *values = integers->data.ints + j;
			printf( "%5i:", j/vertexStride );
			for(k=0; k<vertexStride; ++k) printf( " %5i", values[k] );
			printf( "\n" );
		}
#endif


		for(j=0; j<integers->size; j+=vertexStride){
			int *values = integers->data.ints + j;
			DaoxVertex *vertex = unit->vertices->data.vertices + values[offset1];
			if( node2 ){
				float *norms = floats2->data.floats + 3*values[offset2];
				vertex->norm.x = norms[0];
				vertex->norm.y = norms[1];
				vertex->norm.z = norms[2];
			}
			if( node3 ){
				float *uv = floats3->data.floats + 2*values[offset3];
				vertex->texUV.x = uv[0];
				vertex->texUV.y = uv[1];
			}
		}
		ii = 0;
		jj = vertexStride;
		kk = 2*vertexStride;
		for(j=0; j<integers->size; j+=shapeStride){
			DaoxTriangle *triangle = DArray_PushTriangle( unit->triangles, NULL );
			DaoxVertex *A, *B, *C;
			int *values = integers->data.ints + j;
			triangle->index[0] = values[ii + offset1];
			triangle->index[1] = values[jj + offset1];
			triangle->index[2] = values[kk + offset1];
			A = unit->vertices->data.vertices + triangle->index[0];
			B = unit->vertices->data.vertices + triangle->index[1];
			C = unit->vertices->data.vertices + triangle->index[2];
			//triangle->norm = DaoxTriangle_Normal( A, B, C );
			if( node2 == NULL ){
			}
		}
	}
	DaoxMesh_UpdateTree( mesh, 0 );
	DaoxMesh_ResetBoundingBox( mesh );
	return 1;

ErrorMissingID:
	/*TODO: Error*/
	return 0;
}
int DaoxSceneResource_ColladaVisit( void *userdata, DaoXmlNode *node )
{
	DaoxSceneResource *self = (DaoxSceneResource*) userdata;
	DaoxColladaParser *handle = self->collada;
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
	DArray *floats = handle->floats;
	DString *att, *att2, *string = handle->string;
	DNode *it = DMap_Find( handle->tags, node->name );
	DMap *table = NULL;
	daoint i, j, k, id = it ? it->value.pInt : 0;
	daoint ii, jj, kk;
	double dvalue;
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
		break;
	case DAE_LIBRARY_VISUAL_SCENES :
		node->data = scene = DaoxSceneResource_CreateScene( self );
		att = DaoXmlNode_GetAttributeMBS( node, "id" );
		printf( "DAE_LIBRARY_VISUAL_SCENE: %p %p\n", scene, att );
		if( att ) DMap_Insert( self->scenes, att, scene );
		break;
	case DAE_SCENE :
		break;
	case DAE_LIGHT :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
		node->data = DaoxLight_New();
		DMap_Insert( self->lights, att, node->data );
		break;
	case DAE_CAMERA :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
		node->data = DaoxCamera_New();
		DMap_Insert( self->cameras, att, node->data );
		break;
	case DAE_IMAGE :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
		node->data = DaoxTexture_New();
		DMap_Insert( self->images, att, node->data );
		break;
	case DAE_EFFECT :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
		node->data = DaoxMaterial_New();
		DMap_Insert( self->effects, att, node->data );
		break;
	case DAE_MATERIAL :
		if( (att = DaoXmlNode_GetAttributeMBS( node, "id" )) == NULL ) goto ErrorMissingID;
		node->data = DaoxMaterial_New();
		DMap_Insert( self->materials, att, node->data );
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
			DString_Assign( texture->file, node->content );
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
		it = DMap_Find( self->images, child->content );
		if( it == NULL ) break;
		printf( "DAE_TEXTURE-------------------------- %p\n", material );
		DaoxMaterial_SetTexture( material, (DaoxTexture*) it->value.pVoid );
		break;
	case DAE_VISUAL_SCENE :
		node->data = scene = DaoxSceneResource_CreateScene( self );
		att = DaoXmlNode_GetAttributeMBS( node, "id" );
		printf( "DAE_VISUAL_SCENE: %p %p\n", scene, att );
		if( att ) DMap_Insert( self->scenes, att, scene );
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
			DaoXmlDOM_Traverse( self->xmlDOM, child, self, DaoxSceneResource_ColladaVisit );
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
			DaoXmlDOM_Traverse( self->xmlDOM, child, self, DaoxSceneResource_ColladaVisit );
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
	case DAE_MATRIX :
		sceneNode = (DaoxSceneNode*) DaoXmlNode_GetAncestorDataMBS( node, "node", 1 );
		if( sceneNode == NULL ) break;

		if( DString_ParseFloats( node->content, floats, 0 ) != 16 ) break; // TODO
		matrix = DaoxMatrix4D_InitColumnMajor( floats->data.floats );
		sceneNode->transform = DaoxMatrix4D_MulMatrix( & matrix, & sceneNode->transform );
		break;
	case DAE_INSTANCE_CAMERA :
		node->data = camera = DaoxCamera_New();
		data = DaoxSceneResource_GetIntanceDef( self, self->cameras, node );
		if( data ) DaoxCamera_CopyFrom( camera, (DaoxCamera*) data );
		break;
	case DAE_INSTANCE_LIGHT :
		node->data = light = DaoxLight_New();
		data = DaoxSceneResource_GetIntanceDef( self, self->lights, node );
		if( data ) DaoxLight_CopyFrom( light, (DaoxLight*) data );
		break;
	case DAE_INSTANCE_EFFECT :
		material = (DaoxMaterial*) DaoXmlNode_GetAncestorDataMBS( node, "material", 1 );
		if( material == NULL ) break;

		data = DaoxSceneResource_GetIntanceDef( self, self->effects, node );
		if( data ) DaoxMaterial_CopyFrom( material, (DaoxMaterial*) data );
		printf( "DAE_INSTANCE_EFFECT-------------------------- %p %p\n", material, material->texture1 );
		break;
	case DAE_INSTANCE_MATERIAL :
		model = (DaoxModel*) DaoXmlNode_GetAncestorDataMBS( node, "instance_geometry", 3 );
		if( model == NULL || model->mesh == NULL ) break; // XXX;
		if( (att = DaoXmlNode_GetAttributeMBS( node, "symbol" )) == NULL ) goto ErrorMissingID;
		if( (att2 = DaoXmlNode_GetAttributeMBS( node, "target" )) == NULL ) goto ErrorMissingID;
		DString_SetChars( string, att2->chars + 1 );
		it = DMap_Find( self->materials, string );
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
		DaoxSceneResource_HandleColladaGeometry( self, node );
		return 0;
	case DAE_INSTANCE_GEOMETRY :
		scene = (DaoxScene*) DaoXmlNode_GetAncestorDataMBS( node, "visual_scene", 2 );
		if( scene == NULL ) break; // XXX;
		//node->data = model = DaoxModel_New(); // TODO
		//DaoxScene_AddNode( scene, (DaoxSceneNode*) model );
		data = DaoxSceneResource_GetIntanceDef( self, self->geometries, node );
		if( data == NULL ) break; //XXX
		node->data = model = DaoxModel_New();
		DaoxModel_SetMesh( model, (DaoxMesh*) data );
		DaoxScene_AddNode( scene, (DaoxSceneNode*) model );
		printf( "DAE_INSTANCE_GEOMETRY: %p\n", node->data );
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


DaoxScene* DaoxSceneResource_LoadColladaSource( DaoxSceneResource *self, DString *source )
{
	int i;

	DaoXmlParser_Parse( self->xmlParser, self->xmlDOM, source );

	/* The parsing order the sections are rearranged for convenience: */
	for(i=0; collada_main_tags[i]; ++i){
		DaoXmlNode *node = DaoXmlNode_GetChildMBS( self->xmlDOM->root, collada_main_tags[i] );
		DaoXmlDOM_Traverse( self->xmlDOM, node, self, DaoxSceneResource_ColladaVisit );
	}
	return DaoxSceneResource_GetScene( self );
}
DaoxScene* DaoxSceneResource_LoadColladaFile( DaoxSceneResource *self, const char *file )
{
	FILE *fin = fopen( file, "r" );
	DString *source = DString_New(1);
	DaoFile_ReadAll( fin, source, 1 );
	DaoxSceneResource_LoadColladaSource( self, source );
	DString_Delete( source );
	return DaoxSceneResource_GetScene( self );
}




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
	complex16 buffer = {0.0, 0.0};
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
			vertex.point = self->vlist->data.vectors3d[ quadints[0]-1 ];
			if( quadints[1] ){
				vector = self->vtlist->data.vectors3d[ quadints[1]-1 ];
				vertex.texUV.x = vector.x;
				vertex.texUV.y = vector.y;
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

			A = unit->vertices->data.vertices[ triangle->index[0] ].point;
			B = unit->vertices->data.vertices[ triangle->index[1] ].point;
			C = unit->vertices->data.vertices[ triangle->index[2] ].point;
			AB = DaoxVector3D_Sub( & B, & A );
			BC = DaoxVector3D_Sub( & C, & B );
			facenorm = DaoxVector3D_Cross( & AB, & BC );
			facenorm = DaoxVector3D_Normalize( & facenorm );
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
	printf( "triangle count: %i\n", tcount );
	return unit;
}

int DaoxSceneResource_LoadObjMtlSource( DaoxSceneResource *self, DaoxObjParser *parser, DString *source, DString *path )
{
	DNode *it;
	DaoToken **tokens;
	DaoxVector3D vector;
	DaoxTexture *texture = NULL;
	DaoxMaterial *material = NULL;
	DString *string = DString_New();
	double numbers[4] = {0.0};
	daoint i, j, k, N, N1;

	printf( "DaoxSceneResource_LoadObjSource\n" );
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
			printf( "new material: %s\n", string->chars );
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
			printf( "ctype %i %s\n", ctype, token->string.chars );
			k = 0;
			while( (++i) < N && tokens[i]->line == token->line ) {
				DaoToken *tok = tokens[i];
				if( tok->type < DTOK_DIGITS_DEC ) goto InvalidFormat;
				if( tok->type > DTOK_NUMBER_SCI ) goto InvalidFormat;
				if( k >= 3 ) goto InvalidFormat;
				numbers[k++] = DaoToken_ToDouble( tok );
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
		}else if( DaoxToken_CheckKeyword( token, "map_Kd" ) ){
			DString_Reset( string, 0 );
			while( (++i) < N && tokens[i]->line == token->line ) {
				DString_Append( string, & tokens[i]->string );
			}
			Dao_MakePath( path, string );
			DString_Change( string, "%\\", "/", 0 );
			texture = DaoxTexture_New();
			DaoxMaterial_SetTexture( material, texture );
			DaoxTexture_LoadImage( texture, string->chars );
			printf( "texture: %s %i\n", string->chars, texture->image->imageSize );
		}else{
			i += 1;
		}
	}
	return 1;
InvalidFormat:
	printf( "ERROR: invalid format!\n" );
	return 0;
}
int DaoxSceneResource_LoadObjMtlFile( DaoxSceneResource *self, DaoxObjParser *parser, const char *file )
{
	int res;
	FILE *fin = fopen( file, "r" );
	DString *source = DString_New(1);
	DString *path = DString_New(1);
	DString_SetChars( path, file );
	DString_Change( path, "%\\", "/", 0 );
	DString_Change( path, "[^/]+ $", "", 1 );
	DaoFile_ReadAll( fin, source, 1 );
	res = DaoxSceneResource_LoadObjMtlSource( self, parser, source, path );
	DString_Delete( source );
	DString_Delete( path );
	return res;
}

DaoxScene* DaoxSceneResource_LoadObjSource( DaoxSceneResource *self, DString *source, DString *path )
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
	DArray *vlist = parser->vlist;
	DArray *vtlist = parser->vtlist;
	DArray *vnlist = parser->vnlist;
	complex16 com = {0.0, 0.0};
	uint_t *integers = (uint_t*) & com;
	double numbers[4] = {0.0};
	daoint vcount = 0, vtcount = 0, vncount = 0;
	daoint i, j, k, N, N1, tcount = 0;
	int smooth = 1;

	printf( "DaoxSceneResource_LoadObjSource\n" );
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
			Dao_MakePath( path, string );
			printf( "%s\n", string->chars );
			DaoxSceneResource_LoadObjMtlFile( self, parser, string->chars );
		}else if( DaoxToken_CheckKeyword( token, "v" ) ){
			k = 0;
			while( (++i) < N && tokens[i]->line == token->line ) {
				DaoToken *tok = tokens[i];
				int sign = 2*(tokens[i-1]->type != DTOK_SUB) - 1;
				if( tok->type == DTOK_SUB ) continue;
				if( tok->type < DTOK_DIGITS_DEC ) goto InvalidFormat;
				if( tok->type > DTOK_NUMBER_SCI ) goto InvalidFormat;
				if( k >= 4 ) goto InvalidFormat;
				numbers[k++] = sign * DaoToken_ToDouble( tok );
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
				numbers[k++] = sign * DaoToken_ToDouble( tok );
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
				numbers[k++] = sign * DaoToken_ToDouble( tok );
			}
			vector.x = numbers[0];
			vector.y = numbers[1];
			vector.z = numbers[2];
			DArray_PushVector3D( parser->vnlist, & vector );
			vncount += 1;
		}else if( token->type == DTOK_IDENTIFIER && token->string.size == 1
				&& (token->string.chars[0] == 'o' || token->string.chars[0] == 'g') ){
			if( model && parser->flist->size ){
				printf( ">> %i %i %i\n", vcount, vtcount, tcount );
				unit = DaoxObjParser_ConstructMeshUnit( parser, mesh );
				DaoxMeshUnit_SetMaterial( unit, material );
				DaoxMesh_UpdateTree( mesh, 0 ); 
				DaoxMesh_ResetBoundingBox( mesh );
				DaoxModel_SetMesh( model, mesh );
				DaoxScene_AddNode( scene, (DaoxSceneNode*) model );
				model = NULL;
			}
			DString_Reset( string, 0 );
			while( (++i) < N && tokens[i]->line == token->line ) {
				DString_Append( string, & tokens[i]->string );
			}
			printf( "node: %s %i\n", string->chars, i );
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
			printf( ">> %s %p\n", string->chars, it );
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
		printf( ">> %i %i %i %i\n", vcount, vtcount, vncount, tcount );
		unit = DaoxObjParser_ConstructMeshUnit( parser, mesh );
		DaoxMeshUnit_SetMaterial( unit, material );
		DaoxMesh_UpdateTree( mesh, 0 ); 
		DaoxMesh_ResetBoundingBox( mesh );
		DaoxOBBox3D_Print( & mesh->obbox );
		DaoxModel_SetMesh( model, mesh );
		DaoxScene_AddNode( scene, (DaoxSceneNode*) model );
	}
	printf( "nodes: %i\n", scene->nodes->size );
	return scene;
InvalidFormat:
	printf( "ERROR: invalid format at line %i!\n", tokens[i]->line );
	return NULL;
}
DaoxScene* DaoxSceneResource_LoadObjFile( DaoxSceneResource *self, const char *file )
{
	DaoxScene *scene;
	FILE *fin = fopen( file, "r" );
	DString *source = DString_New(1);
	DString *path = DString_New(1);
	DString_SetChars( path, file );
	DString_Change( path, "%\\", "/", 0 );
	DString_Change( path, "[^/]+ $", "", 1 );
	printf( "%s\n", path->chars );
	DaoFile_ReadAll( fin, source, 1 );
	scene = DaoxSceneResource_LoadObjSource( self, source, path );
	DString_Delete( source );
	DString_Delete( path );
	return scene;
}
