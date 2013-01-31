/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2012,2013, Limin Fu
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


#ifndef __DAO_FONT_H__
#define __DAO_FONT_H__

#include "daoStdtype.h"
#include "dao_triangulator.h"
#include "dao_path.h"


typedef struct DaoxFont   DaoxFont;
typedef struct DaoxGlyph  DaoxGlyph;

typedef struct DaoxGlyphPoint  DaoxGlyphPoint;


struct DaoxGlyphPoint
{
	uchar_t flag;
	short   x, y;
};




struct DaoxFont
{
	DAO_CSTRUCT_COMMON;

	DString  *buffer;
	uchar_t  *fontData;

	int  fontStart;
	int  fontHeight;
	int  lineSpace;

	int  head;  /* font header table; */
	int  cmap;  /* character code mapping table; */
	int  loca;  /* glyph location table; */
	int  glyf;  /* glyph outline table; */
	int  hhea;  /* horizontal header table; */
	int  hmtx;  /* horizontal metrics table; */

	int      enc_map;
	uchar_t  enc_format;
	uchar_t  indexToLocFormat;

	DMap  *glyphs;  /* Glyph index to glyph; */
	DMap  *glyphs2; /* Unicode to glyph; */

	DaoxGlyphPoint  *points;
	DaoxTriangulator  *triangulator;
};
extern DaoType* daox_type_font;



DaoxFont* DaoxFont_New();
int DaoxFont_Open( DaoxFont *self, const char *file );
int DaoxFont_FindTable( DaoxFont *self, const char *tag );
int DaoxFont_FindGlyphIndex( DaoxFont *self, wchar_t ch );

DaoxGlyph* DaoxFont_GetGlyph( DaoxFont *self, int glyph_index );
DaoxGlyph* DaoxFont_GetCharGlyph( DaoxFont *self, wchar_t ch );



struct DaoxGlyph
{
	DAO_CSTRUCT_COMMON;

	wchar_t  codepoint;

	int  advanceWidth;
	int  leftSideBearing;

	DaoxPath  *shape;
};
extern DaoType* daox_type_glyph;

DaoxGlyph* DaoxGlyph_New();
void DaoxGlyph_Delete( DaoxGlyph *self );



#endif
