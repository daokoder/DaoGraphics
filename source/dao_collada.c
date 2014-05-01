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
	self->floats = DaoxPlainArray_New( sizeof(float) );
	self->floats2 = DaoxPlainArray_New( sizeof(float) );
	self->floats3 = DaoxPlainArray_New( sizeof(float) );
	self->integers = DaoxPlainArray_New( sizeof(int) );
	self->integers2 = DaoxPlainArray_New( sizeof(int) );
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
	DaoxPlainArray_Delete( self->floats );
	DaoxPlainArray_Delete( self->floats2 );
	DaoxPlainArray_Delete( self->floats3 );
	DaoxPlainArray_Delete( self->integers );
	DaoxPlainArray_Delete( self->integers2 );
	DString_Delete( self->string );
	DMap_Delete( self->tags );
	dao_free( self );
}




int DString_ParseFloats( DString *self, DaoxPlainArray *values, int append )
{
	double value;
	char *source = self->bytes;
	char *end = source + self->size;
	if( append == 0 ) DaoxPlainArray_ResetSize( values, 0 );
	while( source < end ){
		char *next = NULL;
		value = strtod( source, & next );
		if( next == source ) return values->size;
		DaoxPlainArray_PushFloat( values, value );
		source = next;
	}
	return values->size;
}
int DString_ParseIntegers( DString *self, DaoxPlainArray *values, int append )
{
	daoint value;
	char *source = self->bytes;
	char *end = source + self->size;
	if( append == 0 ) values->size = 0;
	while( source < end ){
		char *next = NULL;
		value = strtol( source, & next, 10 );
		if( next == source ) return values->size;
		DaoxPlainArray_PushInt( values, value );
		source = next;
	}
	return values->size;
}
void* DaoxSceneResource_GetIntanceDef( DaoxSceneResource *self, DMap *lib, DaoXmlNode *node )
{
	DNode *it;
	DString *att = DaoXmlNode_GetAttributeMBS( node, "url" );
	if( att == NULL ) return NULL;
	DString_SetChars( self->collada->string, att->bytes + 1 );
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
	DaoxPlainArray *integers = handle->integers;
	DaoxPlainArray *integers2 = handle->integers2;
	DaoxPlainArray *floats = handle->floats;
	DaoxPlainArray *floats2 = handle->floats2;
	DaoxPlainArray *floats3 = handle->floats3;
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
		if( strcmp( child->name->bytes, "triangles" ) != 0 ) continue;

		unit = DaoxMesh_AddUnit( mesh );
		material = DaoxMaterial_New();
		DaoxMeshUnit_SetMaterial( unit, material );


		att = DaoXmlNode_GetAttributeMBS( child, "material" );
		if( att ) DString_Assign( material->name, att );

		node1 = DaoXmlNode_GetChildWithAttributeMBS( child, "semantic", "VERTEX" );
		node2 = DaoXmlNode_GetChildWithAttributeMBS( child, "semantic", "NORMAL" );
		node3 = DaoXmlNode_GetChildWithAttributeMBS( child, "semantic", "TEXCOORD" );

		att2 = DaoXmlNode_GetAttributeMBS( node1, "offset" );
		if( att2 ) offset1 = strtol( att2->bytes, NULL, 10 );
		if( node2 ){
			att2 = DaoXmlNode_GetAttributeMBS( node2, "offset" );
			if( att2 ) offset2 = strtol( att2->bytes, NULL, 10 );
		}
		if( node3 ){
			att2 = DaoXmlNode_GetAttributeMBS( node3, "offset" );
			if( att2 ) offset3 = strtol( att2->bytes, NULL, 10 );
		}

		att2 = DaoXmlNode_GetAttributeMBS( node1, "source" );
		printf( "%p %s %s\n", att2, att2->bytes, child->name->bytes );
		node4 = DaoXmlNode_GetChildWithAttributeMBS( pred, "id", att2->bytes + 1 );
		printf( "%p\n", node4 );
		node4 = DaoXmlNode_GetChildWithAttributeMBS( node4, "semantic", "POSITION" );
		att2 = DaoXmlNode_GetAttributeMBS( node4, "source" );
		node4 = DaoXmlNode_GetChildWithAttributeMBS( pred, "id", att2->bytes + 1 );
		node4 = DaoXmlNode_GetChildMBS( node4, "float_array" );
		DString_ParseFloats( node4->content, floats, 0 );
		printf( "floats: %i\n", floats->size );

		if( node2 ){
			att2 = DaoXmlNode_GetAttributeMBS( node2, "source" );
			node4 = DaoXmlNode_GetChildWithAttributeMBS( pred, "id", att2->bytes + 1 );
			node4 = DaoXmlNode_GetChildMBS( node4, "float_array" );
			DString_ParseFloats( node4->content, floats2, 0 );
		}

		if( node3 ){
			att2 = DaoXmlNode_GetAttributeMBS( node3, "source" );
			node4 = DaoXmlNode_GetChildWithAttributeMBS( pred, "id", att2->bytes + 1 );
			node4 = DaoXmlNode_GetChildMBS( node4, "float_array" );
			DString_ParseFloats( node4->content, floats3, 0 );
		}

		for(j=0; j<floats->size; j+=3){
			DaoxVertex *vertex = DaoxPlainArray_PushVertex( unit->vertices );
			vertex->point.x = floats->pod.floats[j];
			vertex->point.y = floats->pod.floats[j+1];
			vertex->point.z = floats->pod.floats[j+2];
		}
		integers->size = 0;
		node4 = DaoXmlNode_GetChildMBS( child, "p" );
		if( node4 ) DString_ParseIntegers( node4->content, integers, 0 );

		att2 = DaoXmlNode_GetAttributeMBS( child, "count" );
		if( att2 == NULL ) return 1; // TODO error;
		count = strtol( att2->bytes, NULL, 10 );
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
			int *values = integers->pod.ints + j;
			for(k=0; k<vertexStride; ++k) tuple->values[k] = values[k];
			tuple->index = j;
		}
#if 0
		for(j=0; j<integers->size; j+=vertexStride){
			int *values = integers->pod.ints + j;
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
				DaoxVertex *vertex = unit->vertices->pod.vertices + integers->pod.ints[idx];
				DaoxVertex *vertex2 = unit->vertices->pod.vertices + integers->pod.ints[idx2];
				integers->pod.ints[idx] = integers->pod.ints[idx2];
				j += 1;
				if( j >= tuples->size ) break;
			}
			if( j >= tuples->size ) break;
			if( tuples->tuples[j].values[offset1] == intup->values[offset1] ){
				daoint idx = tuples->tuples[j].index + offset1;
				daoint idx2 = intup->index + offset1;
				DaoxVertex *vertex = DaoxPlainArray_PushVertex( unit->vertices );
				DaoxVertex *vertex2 = unit->vertices->pod.vertices + integers->pod.ints[idx2];
				integers->pod.ints[idx] = unit->vertices->size - 1;
				vertex->point = vertex2->point;
			}
			intup = tuples->tuples + j;
		}
		printf( "final number of vertice: %i\n", unit->vertices->size );
#if 0
		for(j=0; j<integers->size; j+=vertexStride){
			int *values = integers->pod.ints + j;
			printf( "%5i:", j/vertexStride );
			for(k=0; k<vertexStride; ++k) printf( " %5i", values[k] );
			printf( "\n" );
		}
#endif


		for(j=0; j<integers->size; j+=vertexStride){
			int *values = integers->pod.ints + j;
			DaoxVertex *vertex = unit->vertices->pod.vertices + values[offset1];
			if( node2 ){
				float *norms = floats2->pod.floats + 3*values[offset2];
				vertex->norm.x = norms[0];
				vertex->norm.y = norms[1];
				vertex->norm.z = norms[2];
			}
			if( node3 ){
				float *uv = floats3->pod.floats + 2*values[offset3];
				vertex->texUV.x = uv[0];
				vertex->texUV.y = uv[1];
			}
		}
		ii = 0;
		jj = vertexStride;
		kk = 2*vertexStride;
		for(j=0; j<integers->size; j+=shapeStride){
			DaoxTriangle *triangle = DaoxPlainArray_PushTriangle( unit->triangles );
			DaoxVertex *A, *B, *C;
			int *values = integers->pod.ints + j;
			triangle->index[0] = values[ii + offset1];
			triangle->index[1] = values[jj + offset1];
			triangle->index[2] = values[kk + offset1];
			A = unit->vertices->pod.vertices + triangle->index[0];
			B = unit->vertices->pod.vertices + triangle->index[1];
			C = unit->vertices->pod.vertices + triangle->index[2];
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
	DaoxPlainArray *floats = handle->floats;
	DString *att, *att2, *string = handle->string;
	DNode *it = DMap_Find( handle->tags, node->name );
	DMap *table = NULL;
	daoint i, j, k, id = it ? it->value.pInt : 0;
	daoint ii, jj, kk;
	double dvalue;
	void *data;

	printf( "%s %i %i\n", node->name->bytes, id, DAE_INSTANCE_GEOMETRY );
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
		tmpColor.red = floats->pod.floats[0];
		tmpColor.green = floats->pod.floats[1];
		tmpColor.blue = floats->pod.floats[2];
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
		dvalue = strtod( node->content->bytes, NULL );
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
		child = DaoXmlNode_GetChildWithAttributeMBS( pred, "sid", att->bytes );
		child = DaoXmlNode_GetChildMBS( child, "sampler2D" );
		child = DaoXmlNode_GetChildMBS( child, "source" );
		child = DaoXmlNode_GetChildWithAttributeMBS( pred, "sid", child->content->bytes );
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
			if( strncmp( child->name->bytes, "instance_", 9 ) != 0 ) continue;
			DaoXmlDOM_Traverse( self->xmlDOM, child, self, DaoxSceneResource_ColladaVisit );
			node->data = child->data;
			k += 1;
			printf( "%3i: %s %p\n", i, child->name->bytes, child->data );
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
			printf( "%3i: %s %p\n", i, child->name->bytes, child->data );
			if( strncmp( child->name->bytes, "instance_", 9 ) == 0 && child->data != NULL ){
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
		matrix = DaoxMatrix4D_InitColumnMajor( floats->pod.floats );
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
		DString_SetChars( string, att2->bytes + 1 );
		it = DMap_Find( self->materials, string );
		printf( "%p\n", it );
		if( it == NULL ) break; //XXX
		material = (DaoxMaterial*) it->value.pVoid;
		printf( "DAE_INSTANCE_MATERIAL-------------------------- %p %p\n", material, material->texture1 );
		for(i=0; i<model->mesh->units->size; ++i){
			DaoxMeshUnit *unit = (DaoxMeshUnit*) model->mesh->units->items.pVoid[i];
			if( unit->material == NULL ) continue;
			printf( "%i %s, %s  %p %p\n", i, unit->material->name->bytes, att->bytes, unit, unit->material );
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
