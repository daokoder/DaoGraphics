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


#ifndef __DAO_COLLADA_H__
#define __DAO_COLLADA_H__


#include "dao_resource.h"
#include "daoLexer.h"



typedef struct DaoxObjParser      DaoxObjParser;

struct DaoxObjParser
{
	DaoLexer  *lexer;
	DaoLexer  *lexer2;
	DArray    *integers;
	DArray    *vlist;
	DArray    *vtlist;
	DArray    *vnlist;
	DArray    *flist;
	DMap      *faceVertMap;
};

DaoxObjParser* DaoxObjParser_New();
void DaoxObjParser_Delete( DaoxObjParser *self );

DaoxScene* DaoxResource_LoadObjFile( DaoxResource *self, DString *file, DString *path );



typedef struct DaoxIntTuple   DaoxIntTuple;
typedef struct DaoxIntTuples  DaoxIntTuples;
typedef struct DaoxColladaParser  DaoxColladaParser;


struct DaoxIntTuple
{
	int  index;
	int  reduced;
	int  offset;
	int  values[7];
};


struct DaoxIntTuples
{
	DaoxIntTuple  *tuples;
	uint_t        *counts;
	uint_t         size;
	uint_t         capacity;
	uint_t         dim;
	uint_t         max;
};



struct DaoxColladaParser
{
	DaoxResource  *resource;
	DaoXmlParser  *xmlParser;
	DaoXmlDOM     *xmlDOM;

	DString  *path;
	DString  *string;
	DMap     *tags;

	DArray  *integers;
	DArray  *integers2;
	DArray  *floats;
	DArray  *floats2;
	DArray  *floats3;
	DArray  *positions;
	DArray  *normals;
	DArray  *tangents;
	DArray  *texcoords;

	DaoxIntTuples  *tuples;
	DaoxIntTuples  *tuples2;
};
DaoxColladaParser* DaoxColladaParser_New( DaoxResource *resource );
void DaoxColladaParser_Delete( DaoxColladaParser *self );


DaoxScene* DaoxResource_LoadColladaFile( DaoxResource *self, DString *file, DString *path );


#endif
