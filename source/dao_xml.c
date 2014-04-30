/*
// Dao Standard Module
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
#include "dao_xml.h"

DaoXmlNode* DaoXmlNode_New()
{
	DaoXmlNode *self = (DaoXmlNode*) dao_calloc( 1, sizeof(DaoXmlNode) );
	self->parent = NULL;
	self->children = DArray_New(0);
	self->name = DString_New(1);
	self->content = DString_New(1);
	self->attributes = DHash_New(D_STRING,D_STRING);
	return self;
}
void DaoXmlNode_Delete( DaoXmlNode *self )
{
	daoint i;
	for(i=0; i<self->children->size; ++i){
		DaoXmlNode_Delete( (DaoXmlNode*) self->children->items.pVoid[i] );
	}
	DArray_Delete( self->children );
	DString_Delete( self->name );
	DString_Delete( self->content );
	DMap_Delete( self->attributes );
	dao_free( self );
}

DString* DaoXmlNode_GetAttribute( DaoXmlNode *self, DString *name )
{
	DNode *it = DMap_Find( self->attributes, name );
	if( it ) return it->value.pString;
	return NULL;
}
DString* DaoXmlNode_GetAttributeMBS( DaoXmlNode *self, const char *name )
{
	DString name2 = DString_WrapChars( name );
	return DaoXmlNode_GetAttribute( self, & name2 );
}
DaoXmlNode* DaoXmlNode_GetAncestor( DaoXmlNode *self, DString *name, int level )
{
	DaoXmlNode *node = self;
	for(; level > 0 && node != NULL; --level) node = node->parent;
	if( node && DString_EQ( node->name, name ) ) return node;
	return NULL;
}
DaoXmlNode* DaoXmlNode_GetAncestorMBS( DaoXmlNode *self, const char *name, int level )
{
	DString name2 = DString_WrapChars( name );
	return DaoXmlNode_GetAncestor( self, & name2, level );
}
DaoXmlNode* DaoXmlNode_GetChild( DaoXmlNode *self, DString *name )
{
	daoint i;
	for(i=0; i<self->children->size; ++i){
		DaoXmlNode *child = (DaoXmlNode*) self->children->items.pVoid[i];
		if( DString_EQ( child->name, name ) ) return child;
	}
	return NULL;
}
DaoXmlNode* DaoXmlNode_GetChildMBS( DaoXmlNode *self, const char *name )
{
	DString name2 = DString_WrapChars( name );
	return DaoXmlNode_GetChild( self, & name2 );
}
DaoXmlNode* DaoXmlNode_GetChildWithAttribute( DaoXmlNode *self, DString *key, DString *value )
{
	daoint i;
	for(i=0; i<self->children->size; ++i){
		DaoXmlNode *child = (DaoXmlNode*) self->children->items.pVoid[i];
		DString *att = DaoXmlNode_GetAttribute(  child, key );
		if( att && DString_EQ( att, value ) ) return child;
	}
	return NULL;
}
DaoXmlNode* DaoXmlNode_GetChildWithAttributeMBS( DaoXmlNode *self, const char *key, const char *value )
{
	DString key2 = DString_WrapChars( key );
	DString value2 = DString_WrapChars( value );
	return DaoXmlNode_GetChildWithAttribute( self, & key2, & value2 );
}
void* DaoXmlNode_GetAncestorData( DaoXmlNode *self, DString *name, int level )
{
	DaoXmlNode *ancestor = DaoXmlNode_GetAncestor( self, name, level );
	if( ancestor ) return ancestor->data;
	return NULL;
}
void* DaoXmlNode_GetAncestorDataMBS( DaoXmlNode *self, const char *name, int level )
{
	DString name2 = DString_WrapChars( name );
	return DaoXmlNode_GetAncestorData( self, & name2, level );
}





DaoXmlDOM* DaoXmlDOM_New()
{
	DaoXmlDOM *self = (DaoXmlDOM*) dao_calloc( 1, sizeof(DaoXmlDOM) );
	self->root = NULL;
	self->caches = DArray_New(0);
	return self;
}

DaoXmlNode* DaoXmlDOM_NewNode( DaoXmlDOM *self )
{
	if( self->caches->size ){
		DaoXmlNode *node = DArray_Back( self->caches );
		DArray_PopBack( self->caches );
		return node;
	}
	return DaoXmlNode_New();
}
void DaoXmlDOM_CacheNode( DaoXmlDOM *self, DaoXmlNode *node )
{
	daoint i;
	for(i=0; i<node->children->size; ++i){
		DaoXmlDOM_CacheNode( self, (DaoXmlNode*) node->children->items.pVoid[i] );
	}
	node->id = 0;
	node->parent = NULL;
	node->data = NULL;
	node->children->size = 0;
	node->name->size = 0;
	/* Most nodes will have empty contents, and some may have long contents,
	// so it is better to clear the string: */
	DString_Clear( node->content );
	DMap_Reset( node->attributes );
	DArray_Append( self->caches, node );
}
void DaoXmlDOM_Reset( DaoXmlDOM *self )
{
	if( self->root ) DaoXmlDOM_CacheNode( self, self->root );
	self->root = NULL;
}
void DaoXmlDOM_Delete( DaoXmlDOM *self )
{
	daoint i;
	DaoXmlDOM_Reset( self );
	for(i=0; i<self->caches->size; ++i){
		DaoXmlNode_Delete( (DaoXmlNode*) self->caches->items.pVoid[i] );
	}
	DArray_Delete( self->caches );
	dao_free( self );
}
void DaoXmlDOM_TraverseNode( DaoXmlDOM *self, DaoXmlNode *node, void *visitor, DaoXmlNode_Visit visit )
{
	daoint i;
	if( visit( visitor, node ) == 0 ) return;
	for(i=0; i<node->children->size; ++i){
		DaoXmlNode *child = (DaoXmlNode*) node->children->items.pVoid[i];
		DaoXmlDOM_TraverseNode( self, child, visitor, visit );
	}
}
void DaoXmlDOM_Traverse( DaoXmlDOM *self, DaoXmlNode *root, void *visitor, DaoXmlNode_Visit visit )
{
	if( root == NULL ) root = self->root;
	if( root == NULL ) return;

	DaoXmlDOM_TraverseNode( self, root, visitor, visit );
}





enum DaoXmlParserState
{
	DAO_XML_
};

DaoXmlParser* DaoXmlParser_New()
{
	DaoXmlParser *self = (DaoXmlParser*) dao_calloc( 1, sizeof(DaoXmlParser) );
	self->key = DString_New(1);
	self->value = DString_New(1);
	self->escape = DString_New(1);
	self->escapes = DHash_New(D_STRING,D_STRING);
	DString_SetChars( self->key, "lt" );
	DString_SetChars( self->value, "<" );
	DMap_Insert( self->escapes, self->key, self->value );
	DString_SetChars( self->key, "gt" );
	DString_SetChars( self->value, ">" );
	DMap_Insert( self->escapes, self->key, self->value );
	DString_SetChars( self->key, "amp" );
	DString_SetChars( self->value, "&" );
	DMap_Insert( self->escapes, self->key, self->value );
	DString_SetChars( self->key, "apos" );
	DString_SetChars( self->value, "'" );
	DMap_Insert( self->escapes, self->key, self->value );
	DString_SetChars( self->key, "quot" );
	DString_SetChars( self->value, "\"" );
	DMap_Insert( self->escapes, self->key, self->value );
	return self;
}
void DaoXmlParser_Delete( DaoXmlParser *self )
{
	DString_Delete( self->key );
	DString_Delete( self->value );
	DString_Delete( self->escape );
	DMap_Delete( self->escapes );
	dao_free( self );
}

int DaoXmlParser_SkipWhiteSpaces( DaoXmlParser *self )
{
	while( self->source < self->end && isspace( *self->source ) ) self->source += 1;
	return self->source >= self->end;
}
int DaoXmlParser_ParseChar( DaoXmlParser *self, const char ch )
{
	if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
	if( self->source > self->end ) return 1;
	if( self->source[0] != ch ) return 1;
	self->source += 1;
	return 0;
}
int DaoXmlParser_ParseIdentifier( DaoXmlParser *self, DString *string )
{
	char ch = *self->source;
	DString_Reset( string, 0 );
	if( !isalpha( ch ) && ch != ':' && ch != '_' ) return 1;
	while( isalnum( ch ) || ch == ':' || ch == '_' || ch == '-' || ch == '.' ){
		DString_AppendChar( string, *self->source );
		self->source += 1;
		if( self->source >= self->end ) break;
		ch = *self->source;
	}
	return 0;
}
int DaoXmlParser_ParseEscapedChar( DaoXmlParser *self, DString *escape )
{
	DString_Reset( escape, 0 );
	if( (self->source + 2) > self->end ) return 1;
	if( *self->source != '&' ) return 1;
	self->source += 1;
	while( self->source < self->end && *self->source != ';' ){
		DString_AppendChar( escape, *self->source );
		self->source += 1;
	}
	self->source += 1;
	return 0;
}
int DaoXmlParser_ParseFormatedChar( DaoXmlParser *self, DString *string )
{
	DNode *it;
	if( *self->source != '&' ){
		DString_AppendChar( string, *self->source );
		self->source += 1;
		return 0;
	}
	if( DaoXmlParser_ParseEscapedChar( self, self->escape ) ) return 1;
	it = DMap_Find( self->escapes, self->escape );
	if( it == NULL ) return 1;
	DString_Append( string, it->value.pString );
	return 0;
}
int DaoXmlParser_ParseQuotedString( DaoXmlParser *self, DString *string )
{
	DNode *it;
	char quote;
	DString_Reset( string, 0 );
	if( (self->source + 2) > self->end ) return 1;
	if( self->source[0] != '\'' && self->source[0] != '"' ) return 1;
	quote = self->source[0];
	self->source += 1;
	while( self->source < self->end && *self->source != quote ){
		if( DaoXmlParser_ParseFormatedChar( self, string ) ) return 1;
	}
	self->source += 1;
	return 0;
}
int DaoXmlParser_ParseAttribute( DaoXmlParser *self, DString *key, DString *value )
{
	if( DaoXmlParser_ParseIdentifier( self, key ) ) return 1;
	if( DaoXmlParser_ParseChar( self, '=' ) ) return 1;
	if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
	if( DaoXmlParser_ParseQuotedString( self, value ) ) return 1;
	return 0;
}
int DaoXmlParser_ParseDeclaration( DaoXmlParser *self, DString *string )
{
	if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
	if( (self->source + 7) > self->end ) return 1;

	if( self->source[0] != '<' || self->source[1] != '?' ) return 1;
	self->source += 2;

	if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
	if( strncmp( self->source, "xml", 3 ) && strncmp( self->source, "XML", 3 ) ) return 1;
	self->source += 3;
	if( self->source >= self->end || !isspace( *self->source ) ) return 1;
	self->source += 1;

	if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
	while( self->source < self->end && isalpha( *self->source ) ){
		if( DaoXmlParser_ParseAttribute( self, self->key, self->value ) ) return 1;
		if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
	}
	if( (self->source + 2) > self->end ) return 1;
	if( self->source[0] != '?' || self->source[1] != '>' ) return 1;
	self->source += 2;
	return 0;
}
int DaoXmlParser_ParseNode( DaoXmlParser *self, DaoXmlDOM *dom, DaoXmlNode *node );
int DaoXmlParser_ParseNodeContent( DaoXmlParser *self, DaoXmlDOM *dom, DaoXmlNode *node )
{
	DaoXmlNode *child;
	while( self->source < self->end ){
		if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
		if( self->source >= self->end ) return 1;
		if( *self->source == '<' ){
			char *current = self->source;
			self->source += 1;
			if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
			if( *self->source == '/' ){
				self->source = current;
				return 0;
			}else if( *self->source == '!' ){
				self->source += 1;
				if( strncmp( self->source, "[CDATA[", 7 ) == 0 ){
					self->source = strstr( self->source + 7, "]]>" );
					if( self->source == NULL ) return 1;
					self->source += 3;
					continue;
				}else if( strncmp( self->source, "--", 2 ) == 0 ){
					self->source = strstr( self->source + 7, "-->" );
					if( self->source == NULL ) return 1;
					self->source += 3;
					continue;
				}
			}

			child = DaoXmlDOM_NewNode( dom );
			child->parent = node;
			DArray_Append( node->children, child );
			self->source = current;
			DaoXmlParser_ParseNode( self, dom, child );
		}else{
			while( self->source < self->end && *self->source != '<' ){
				if( DaoXmlParser_ParseFormatedChar( self, node->content ) ) return 1;
			}
		}
	}
	return 0;
}
int DaoXmlParser_ParseNode( DaoXmlParser *self, DaoXmlDOM *dom, DaoXmlNode *node )
{
	if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
	if( self->source >= self->end ) return 1;
	if( *self->source != '<' ) return 1;
	self->source += 1;

	if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
	if( DaoXmlParser_ParseIdentifier( self, node->name ) ) return 1;

	if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
	while( self->source < self->end && isalpha( *self->source ) ){
		DaoXmlParser_ParseAttribute( self, self->key, self->value );
		DMap_Insert( node->attributes, self->key, self->value );
		if( DaoXmlParser_SkipWhiteSpaces( self ) ) return 1;
	}
	if( self->source >= self->end ) return 1;
	if( *self->source == '/' ){
		self->source += 1;
		if( self->source >= self->end ) return 1;
		if( *self->source != '>' ) return 1;
		self->source += 1;
		return 0;
	}
	if( *self->source != '>' ) return 1;
	self->source += 1;

	if( DaoXmlParser_ParseNodeContent( self, dom, node ) ) return 1;
	if( DaoXmlParser_ParseChar( self, '<' ) ) return 1;
	if( DaoXmlParser_ParseChar( self, '/' ) ) return 1;
	if( DaoXmlParser_ParseIdentifier( self, self->value ) ) return 1;
	if( DString_EQ( node->name, self->value ) == 0 ) return 1;
	if( DaoXmlParser_ParseChar( self, '>' ) ) return 1;
	return 0;
}
int DaoXmlParser_Parse( DaoXmlParser *self, DaoXmlDOM *dom, DString *source )
{
	self->source = source->bytes;
	self->end = self->source + source->size;
	self->error = NULL;
	DaoXmlDOM_Reset( dom );

	dom->root = DaoXmlDOM_NewNode( dom );

	if( DaoXmlParser_ParseDeclaration( self, self->value ) ) return 1;
	if( DaoXmlParser_ParseNode( self, dom, dom->root ) ) return 1;
	return 0;
}




void DaoXmlNode_TestVisit( void *userdata, DaoXmlNode *parent, DaoXmlNode *node )
{
	printf( "%10s -> %-10s:  %s\n", parent ? parent->name->bytes : "", node->name->bytes, node->content->bytes );
}

#if 0
DAO_DLL int DaoOnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns )
{
	FILE *fin = fopen( "test.xml", "r" );
	DString *xml = DString_New(1);
	DaoXmlDOM *dom = DaoXmlDOM_New();
	DaoXmlParser *parser = DaoXmlParser_New();
	DaoFile_ReadAll( fin, xml, 1 );

	DaoXmlParser_Parse( parser, dom, xml );

	DaoXmlDOM_Traverse( dom, NULL, NULL, DaoXmlNode_TestVisit );

	DString_Delete( xml );
	DaoXmlDOM_Delete( dom );
	DaoXmlParser_Delete( parser );
	return 0;
}
#endif
