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


#ifndef __DAO_COLLADA_H__
#define __DAO_COLLADA_H__


#include "dao_resource.h"


typedef struct DaoxIntTuple   DaoxIntTuple;
typedef struct DaoxIntTuples  DaoxIntTuples;
typedef struct DaoxColladaParser  DaoxColladaParser;


struct DaoxIntTuple
{
	int  index;
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
	DMap     *tags;
	DString  *string;

	DaoxPlainArray  *integers;
	DaoxPlainArray  *integers2;
	DaoxPlainArray  *floats;
	DaoxPlainArray  *floats2;
	DaoxPlainArray  *floats3;
	DaoxIntTuples   *tuples;
};
DaoxColladaParser* DaoxColladaParser_New();
void DaoxColladaParser_Delete( DaoxColladaParser *self );


DaoxScene* DaoxSceneResource_LoadColladaSource( DaoxSceneResource *self, DString *source );
DaoxScene* DaoxSceneResource_LoadColladaFile( DaoxSceneResource *self, const char *file );


#endif
