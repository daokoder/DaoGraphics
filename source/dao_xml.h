/*
// Dao Standard Module
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



#ifndef __DAO_XML_H__
#define __DAO_XML_H__


#include "dao_common.h"
#include "daoStdtype.h"
#include "daoValue.h"


typedef struct DaoXmlNode    DaoXmlNode;
typedef struct DaoXmlDOM     DaoXmlDOM;
typedef struct DaoXmlParser  DaoXmlParser;


/* return 0 to skip traversing the children nodes: */
typedef int (*DaoXmlNode_Visit)( void *userdata, DaoXmlNode *node );


struct DaoXmlNode
{
	DaoXmlNode  *parent;
	DList       *children;

	DString     *name;
	DString     *content;
	DMap        *attributes;

	uint_t       id;
	void        *data;
};
DaoXmlNode* DaoXmlNode_New();
void DaoXmlNode_Delete( DaoXmlNode *self );

DString* DaoXmlNode_GetAttribute( DaoXmlNode *self, DString *name );
DString* DaoXmlNode_GetAttributeMBS( DaoXmlNode *self, const char *name );
DaoXmlNode* DaoXmlNode_GetAncestor( DaoXmlNode *self, DString *name, int level );
DaoXmlNode* DaoXmlNode_GetAncestorMBS( DaoXmlNode *self, const char *name, int level );
DaoXmlNode* DaoXmlNode_GetChild( DaoXmlNode *self, DString *name );
DaoXmlNode* DaoXmlNode_GetChildMBS( DaoXmlNode *self, const char *name );
DaoXmlNode* DaoXmlNode_GetChildByAttribute( DaoXmlNode *self, DString *key, DString *value );
DaoXmlNode* DaoXmlNode_GetChildByAttributeMBS( DaoXmlNode *self, const char *key, const char *value );

void* DaoXmlNode_GetAncestorData( DaoXmlNode *self, DString *name, int level );
void* DaoXmlNode_GetAncestorDataMBS( DaoXmlNode *self, const char *name, int level );


struct DaoXmlDOM
{
	DaoXmlNode  *root;
	DList       *caches;
};
DaoXmlDOM* DaoXmlDOM_New();
void DaoXmlDOM_Delete( DaoXmlDOM *self );

void DaoXmlDOM_Traverse( DaoXmlDOM *self, DaoXmlNode *root, void *visitor, DaoXmlNode_Visit visit );



struct DaoXmlParser
{
	uchar_t  state;

	DString  *key;
	DString  *value;
	DString  *escape;
	DMap     *escapes;

	char  *source;
	char  *end;
	char  *error;
};

DaoXmlParser* DaoXmlParser_New();
void DaoXmlParser_Delete( DaoXmlParser *self );

int DaoXmlParser_Parse( DaoXmlParser *self, DaoXmlDOM *dom, DString *source );


#endif
