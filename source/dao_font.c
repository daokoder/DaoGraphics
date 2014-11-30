/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2012-2014, Limin Fu
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


#include <math.h>
#include <string.h>
#include "dao_font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

typedef unsigned long  ulong_t;



enum DaoxTTPlatformIDs
{
   DAOTT_PLATFORM_ID_UNICODE   = 0,
   DAOTT_PLATFORM_ID_MAC       = 1,
   DAOTT_PLATFORM_ID_ISO       = 2,
   DAOTT_PLATFORM_ID_MICROSOFT = 3
};

enum DaoxTTUnicodeEncodings
{
	DAOTT_UNICODE_DEFAULT = 0,
	DAOTT_UNICODE_1_1     = 1,
	DAOTT_UNICODE_ISO     = 2,
	DAOTT_UNICODE_2_0     = 3
};




short daox_tt_short( uchar_t *data )
{
	return (data[0]<<8) + data[1];
}
ushort_t daox_tt_ushort( uchar_t *data )
{
	return (data[0]<<8) + data[1];
}
long daox_tt_long( uchar_t *data )
{
	return (data[0]<<24) + (data[1]<<16) + (data[2]<<8) + data[3];
}
ulong_t daox_tt_ulong( uchar_t *data )
{
	return (data[0]<<24) + (data[1]<<16) + (data[2]<<8) + data[3];
}



DaoxGlyph* DaoxGlyph_New()
{
	DaoxGlyph *self = (DaoxGlyph*) dao_calloc(1,sizeof(DaoxGlyph));
	DaoCstruct_Init( (DaoCstruct*)self, daox_type_glyph );
	self->codepoint = 0;
	self->shape = DaoxPath_New();
	DaoGC_IncRC( (DaoValue*) self->shape );
	return self;
}
void DaoxGlyph_Delete( DaoxGlyph *self )
{
	DaoCstruct_Free( (DaoCstruct*) self );
	DaoGC_DecRC( (DaoValue*) self->shape );
	dao_free( self );
}




DaoxFont* DaoxFont_New()
{
	DaoxFont *self = (DaoxFont*) dao_calloc( 1, sizeof(DaoxFont) );
	DaoCstruct_Init( (DaoCstruct*)self, daox_type_font );
	self->buffer = DString_New(0);
	self->glyphs = DMap_New(0,0);
	return self;
}
void DaoxFont_Delete( DaoxFont *self )
{
	DNode *it;
	for(it=DMap_First(self->glyphs); it; it=DMap_Next(self->glyphs,it)){
		DaoxGlyph_Delete( (DaoxGlyph*) it->value.pVoid );
	}
	DMap_Delete( self->glyphs );
	DString_Delete( self->buffer );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}

void DaoxFont_ResetGlyphs( DaoxFont *self )
{
	DNode *it;
	for(it=DMap_First(self->glyphs); it; it=DMap_Next(self->glyphs,it)){
		DaoxGlyph *glyph = (DaoxGlyph*) it->value.pVoid;
		// TODO: reset glyph shape;
	}
}
int DaoxFont_Init( DaoxFont *self, DString *ttfData )
{
	DString_Assign( self->buffer, ttfData );
	if( stbtt_InitFont( & self->info, self->buffer->chars, 0 ) == 0 ) return 0;
	stbtt_GetFontVMetrics( & self->info, & self->ascent, & self->descent, & self->lineSpace );
	self->fontHeight = self->ascent + self->descent;
	self->lineSpace += self->fontHeight;
	return 1;
}
DaoxGlyph* DaoxFont_LoadGlyph( DaoxFont *self, size_t codepoint )
{
	DaoxGlyph *glyph = DaoxGlyph_New();
	stbtt_vertex *vertices = NULL;
	int id = stbtt_FindGlyphIndex( & self->info, codepoint );
	int i, num_verts = stbtt_GetGlyphShape( & self->info, id, & vertices );

	stbtt_GetGlyphHMetrics( & self->info, id, & glyph->advanceWidth, & glyph->leftSideBearing );

	DMap_Insert( self->glyphs, (void*) codepoint, glyph );
	DaoxPath_SetRelativeMode( glyph->shape, 0 );
	for(i=0; i<num_verts; ++i){
		stbtt_vertex_type x = vertices[i].x;
		stbtt_vertex_type y = vertices[i].y;
		stbtt_vertex_type cx = vertices[i].cx;
		stbtt_vertex_type cy = vertices[i].cy;
		switch( vertices[i].type ){
		case STBTT_vmove :
			if( i ) DaoxPath_Close( glyph->shape );
			DaoxPath_MoveTo( glyph->shape, x, y );
			break;
		case STBTT_vline :
			DaoxPath_LineTo( glyph->shape, x, y );
			break;
		case STBTT_vcurve :
			DaoxPath_QuadTo( glyph->shape, cx, cy, x, y );
			break;
		}
	}
	if( i ) DaoxPath_Close( glyph->shape );

	return glyph;
}

DaoxGlyph* DaoxFont_GetGlyph( DaoxFont *self, size_t codepoint )
{
	DNode *node = DMap_Find( self->glyphs, (void*) codepoint );

	if( node ) return (DaoxGlyph*) node->value.pVoid;
	return DaoxFont_LoadGlyph( self, codepoint );
}


int DaoxFont_Open( DaoxFont *self, const char *file )
{
	FILE *fin = fopen( file, "r" );

	DaoxFont_ResetGlyphs( self );
	DString_Reset( self->buffer, 0 );

	if( fin == NULL ) return 0;

	DaoFile_ReadAll( fin, self->buffer, 1 );
	return DaoxFont_Init( self, self->buffer );
}




static void FONT_New( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxFont *self = DaoxFont_New();
	DaoProcess_PutValue( proc, (DaoValue*) self );
	if( N ) DaoxFont_Open( self, DaoValue_TryGetChars( p[0] ) );
}
static void FONT_Open( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoxFont *self = (DaoxFont*) p[0];
	DaoProcess_PutInteger( proc, DaoxFont_Open( self, DaoValue_TryGetChars( p[1] ) ) );
}
static DaoFuncItem DaoxFontMeths[]=
{
	{ FONT_New,     "Font( file = \"\" )" },
	{ FONT_Open,    "Open( self: Font, file: string ) => int" },
	{ NULL, NULL }
};

DaoTypeBase DaoxFont_Typer =
{
	"Font", NULL, NULL, (DaoFuncItem*) DaoxFontMeths, { NULL }, { NULL },
	(FuncPtrDel)DaoxFont_Delete, NULL
};

static DaoFuncItem DaoxGlyphMeths[]=
{
	{ NULL, NULL }
};
DaoTypeBase DaoxGlyph_Typer =
{
	"Glyph", NULL, NULL, (DaoFuncItem*) DaoxGlyphMeths, { NULL }, { NULL },
	(FuncPtrDel)DaoxGlyph_Delete, NULL
};

DaoType* daox_type_font = NULL;
DaoType* daox_type_glyph = NULL;

DAO_DLL int DaoFont_OnLoad( DaoVmSpace *vmSpace, DaoNamespace *ns )
{
	daox_type_font = DaoNamespace_WrapType( ns, & DaoxFont_Typer, 0 );
	daox_type_glyph = DaoNamespace_WrapType( ns, & DaoxGlyph_Typer, 0 );
	return 0;
}
