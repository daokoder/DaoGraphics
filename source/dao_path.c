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


#include <math.h>
#include <assert.h>
#include "daoStdtype.h"
#include "dao_path.h"
#include "dao_canvas.h"





DaoxPathMesh* DaoxPathMesh_New()
{
	DaoxPathMesh *self = (DaoxPathMesh*) calloc(1,sizeof(DaoxPathMesh));
	self->points    = DaoxPlainArray_New( sizeof(DaoxVector3D) );
	self->triangles = DaoxPlainArray_New( sizeof(DaoxTriangle) );
	self->patches   = DaoxPlainArray_New( sizeof(DaoxTexturedPoint) );
	return self;
}
void DaoxPathMesh_Delete( DaoxPathMesh *self )
{
	DaoxPlainArray_Delete( self->points );
	DaoxPlainArray_Delete( self->triangles );
	DaoxPlainArray_Delete( self->patches );
	free( self );
}
void DaoxPathMesh_Reset( DaoxPathMesh *self )
{
	self->points->size = 0;
	self->triangles->size = 0;
	self->patches->size = 0;
}





DaoxPathSegment* DaoxPathSegment_New( DaoxPathComponent *component )
{
	DaoxPathSegment* self = (DaoxPathSegment*) calloc(1,sizeof(DaoxPathSegment));
	self->convexness = 1; /* need for refinement of open path; */
	return self;
}
void DaoxPathSegment_Delete( DaoxPathSegment *self )
{
	if( self->first ) DaoxPathSegment_Delete( self->first );
	if( self->second ) DaoxPathSegment_Delete( self->second );
	dao_free( self );
}

DaoxPathComponent* DaoxPathComponent_New( DaoxPath *path )
{
	DaoxPathComponent* self = (DaoxPathComponent*) calloc(1,sizeof(DaoxPathComponent));
	self->first = self->last = DaoxPathSegment_New( self );
	self->path = path;
	return self;
}
DaoxPathSegment* DaoxPathComponent_PushSegment( DaoxPathComponent *self )
{
	DaoxPathSegment *segment = NULL;
	if( self->last && self->last->bezier == 0 ) return self->last;
	if( self->path->freeSegments ){
		segment = self->path->freeSegments;
		self->path->freeSegments = segment->next;
		segment->bezier = 0;
		segment->refined = 0;
	}else{
		segment = DaoxPathSegment_New( self );
	}
	if( self->first == NULL ){
		self->first = self->last = segment;
		return segment;
	}
	self->last->next = segment;
	self->last = segment;
	return segment;
}
void DaoxPathComponent_Reset( DaoxPathComponent *self )
{
	DaoxPathSegment *segment = self->first;
	do {
		segment->next = self->path->freeSegments;
		self->path->freeSegments = segment;
		segment = segment->next;
	} while( segment && segment != self->first );
	self->first = self->last = NULL;
	self->refined.first = self->refined.last = NULL;
	DaoxPathComponent_PushSegment( self ); // XXX free:
}
void DaoxPathComponent_Delete( DaoxPathComponent *self )
{
	DaoxPathComponent_Reset( self );
	dao_free( self );
}






DaoxPath* DaoxPath_New()
{
	DaoxPath *self = (DaoxPath*) calloc(1,sizeof(DaoxPath));
	DaoCstruct_Init( (DaoCstruct*)self, daox_type_path );
	self->first = self->last = DaoxPathComponent_New( self );
	self->mesh = DaoxPathMesh_New();
	return self;
}
void DaoxPath_Delete( DaoxPath *self )
{
	DaoCstruct_Free( (DaoCstruct*) self );
	DaoxPath_Reset( self );
	while( self->freeComponents ){
		DaoxPathComponent *com = self->freeComponents;
		self->freeComponents = self->freeComponents->next;
		DaoxPathComponent_Delete( com );
	}
	while( self->freeSegments ){
		DaoxPathSegment *segment = self->freeSegments;
		self->freeSegments = self->freeSegments->next;
		DaoxPathSegment_Delete( segment );
	}
	DaoxPathMesh_Delete( self->mesh );
	free( self );
}
DaoxPathComponent* DaoxPath_PushComponent( DaoxPath *self )
{
	DaoxPathComponent *com = NULL;
	if( self->last && self->last->first->bezier == 0 ) return self->last;
	if( self->freeComponents ){
		com = self->freeComponents;
		self->freeComponents = com->next;
	}else{
		com = DaoxPathComponent_New( self );
	}
	if( self->first == NULL ){
		self->first = self->last = com;
		return com;
	}
	self->last->next = com;
	self->last = com;
	return com;
}
void DaoxPath_Reset( DaoxPath *self )
{
	DaoxPathComponent *com;
	for(com=self->first; com; com=com->next){
		DaoxPathComponent_Reset( com );
		com->next = self->freeComponents;
		self->freeComponents = com;
	}
	self->cmdRelative = 0;
	self->first = self->last = NULL;
	DaoxPath_PushComponent( self );
}
void DaoxPath_SetRelativeMode( DaoxPath *self, int relative )
{
	self->cmdRelative = relative;
}
void DaoxPath_MoveTo( DaoxPath *self, float x, float y )
{
	DaoxPathComponent *com;
	if( self->last->last->bezier == 0 ){
		self->last->last->P1.x = x;
		self->last->last->P1.y = y;
		return;
	}
	com = DaoxPathComponent_New( self );
	com->last->P1.x = x;
	com->last->P1.y = y;
	self->last->next = com;
	self->last = com;
}
/* If MoveTo() is not called, line from (0,0). */
DaoxPathSegment* DaoxPath_LineTo( DaoxPath *self, float x, float y )
{
	DaoxVector2D start;
	DaoxPathSegment *segment = NULL;
	if( self->last->last->bezier == 0 ){
		start = self->last->last->P1;
		segment = self->last->last;
	}else{
		start = self->last->last->P2;
		segment = DaoxPathComponent_PushSegment( self->last );
		segment->P1 = start;
	}
	if( self->cmdRelative ){
		x += start.x;
		y += start.y;
	}
	segment->bezier = 1;
	segment->P2.x = x;
	segment->P2.y = y;
	return segment;
}
/*
// Quoted from: http://en.wikipedia.org/wiki/Bezier_spline
// We may compose a circle of radius R from an arbitrary number of cubic Bezier curves.
// Let the arc start at point A and end at point B, placed at equal distances above 
// and below the x-axis, spanning an arc of angle theta=2*phi:
//     AX = cos(phi)
//     AY = sin(phi)
//     BX =  AX
//     BY = -AY
// The control points may be written as:
//     CAX = (4-AX)/3
//     CAY = (1-AX)*(3-AX)/(3*AY)
//     CBX =  CAX
//     CBY = -CAY
*/
void DaoxPathSegment_MakeArc( DaoxPathSegment *self, float dx, float dy, float R, float degree )
{
	double angle = 0.0;
	double AX = cos( 0.5 * degree );
	double AY = - sin( 0.5 * degree );
	double BX = AX, BY = - AY;
	double CAX = (4.0 - AX) / 3.0;
	double CAY = (1.0 - AX)*(3.0 - AX) / (3.0 * AY);
	double CBX = CAX, CBY = - CAY;
	double CAX2 = CAX - AX;  /* control point relative to A; */
	double CAY2 = CAY - AY;
	double CBX2 = CBX - BX;  /* control point relative to B; */
	double CBY2 = CBY - BY;
	double sine, cosine;

	self->bezier = 3;
	self->refined = 0;
	self->P2.x = self->P1.x + dx;
	self->P2.y = self->P1.y + dy;

	if( fabs( dx ) < 1E-16 ){
		angle = dy >= 0.0 ? 0.5*M_PI : 1.5*M_PI;
	}else{
		angle = atan( dy / fabs( dx ) );
		if( dx < 0.0 ) angle = M_PI - angle;
	}
	if( degree > 0.0 ){
		sine = R * sin( angle - 0.5 * M_PI );
		cosine = R * cos( angle - 0.5 * M_PI );
	}else{
		sine = R * sin( angle + 0.5 * M_PI );
		cosine = R * cos( angle + 0.5 * M_PI );
	}
	self->C1.x = self->P1.x + CAX2 * cosine - CAY2 * sine;
	self->C1.y = self->P1.y + CAX2 * sine + CAY2 * cosine;
	self->C2.x = self->P2.x + CBX2 * cosine - CBY2 * sine;
	self->C2.y = self->P2.y + CBX2 * sine + CBY2 * cosine;
}
void DaoxPath_ArcBy2( DaoxPath *self, float cx, float cy, float degrees, float deg2 )
{
	DaoxVector2D point, start, next, end, center; /* A: start; B: end; C: center; */
	DaoxPathSegment *segment = NULL;
	double degrees2 = M_PI * degrees / 180.0;
	double dx, dy, R, dA, sine, cosine, dL, dR;
	int i, K;

	if( self->last->last->bezier == 0 ){
		start = self->last->last->P1;
		segment = self->last->last;
	}else{
		start = self->last->last->P2;
		segment = DaoxPathComponent_PushSegment( self->last );
		segment->P1 = start;
	}
	if( self->cmdRelative == 0 ){
		cx -= start.x;
		cy -= start.y;
	}

	R = sqrt( cx * cx + cy * cy );
	if( fabs( degrees ) < deg2 ){
		cosine = cos( degrees2 );
		sine = sin( degrees2 );
		dx = - sine * (-cy);
		dy = cosine * (-cx);
		DaoxPathSegment_MakeArc( segment, dx, dy, R, degrees2 );
		return;
	}
	center.x = start.x + cx;
	center.y = start.y + cy;

	printf( "degrees = %15f:  %15f  %15f\n", degrees, start.x, start.y );
	printf( "degrees = %15f:  %15f  %15f\n", degrees, center.x, center.y );

	/* Make start relative to the center: */
	start.x -= center.x;
	start.y -= center.y;
	point = start;

	K = 1 + fabs( degrees ) / deg2;
	dA = degrees2 / K;
	dL = 1.0 / (double)K;
	for(i=0; i<K; ++i){
		if( segment == NULL ){
			segment = DaoxPathComponent_PushSegment( self->last );
			segment->P1.x = point.x + center.x;
			segment->P1.y = point.y + center.y;
		}
		sine = sin( (i + 1) * dA );
		cosine = cos( (i + 1) * dA );
		next.x = start.x * cosine - start.y * sine;
		next.y = start.x * sine + start.y * cosine;
		DaoxPathSegment_MakeArc( segment, next.x - point.x, next.y - point.y, R, dA );
		point = next;
		segment = NULL;
	}
}
void DaoxPath_ArcTo2( DaoxPath *self, float x, float y, float degrees, float deg2 )
{
	DaoxVector2D start;
	double degrees2 = M_PI * degrees / 180.0;
	double cx, cy, dx, dy, R, dR, dL;

	if( self->last->last->bezier == 0 ){
		start = self->last->last->P1;
	}else{
		start = self->last->last->P2;
	}
	if( self->cmdRelative == 0 ){
		x -= start.x;
		y -= start.y;
	}

	cx = 0.5 * x;
	cy = 0.5 * y;
	dx = - cx;
	dy = - cy;
	dL = x * x + y * y;
	R = 0.5 * dL / (1.0 - cos(degrees2) );
	dR = sqrt( R - 0.25 * dL );
	R = sqrt( R );
	dL = 0.5 * sqrt( dL );
	if( degrees > 180.0 ){
		cx += - dR * dy / dL;
		cy += + dR * dx / dL;
	}else if( degrees < - 180.0 ){
		cx += + dR * dy / dL;
		cy += - dR * dx / dL;
	}else if( degrees > 0.0 ){
		cx += + dR * dy / dL;
		cy += - dR * dx / dL;
	}else{
		cx += - dR * dy / dL;
		cy += + dR * dx / dL;
	}
	if( self->cmdRelative == 0 ){
		cx += start.x;
		cy += start.y;
	}

	DaoxPath_ArcBy2( self, cx, cy, degrees, deg2 );
}
void DaoxPath_ArcTo( DaoxPath *self, float x, float y, float degrees )
{
	DaoxPath_ArcTo2( self, x, y, degrees, 30.0 );
}
void DaoxPath_ArcBy( DaoxPath *self, float cx, float cy, float degrees )
{
	DaoxPath_ArcBy2( self, cx, cy, degrees, 30.0 );
}
DaoxPathSegment* DaoxPath_QuadTo( DaoxPath *self, float cx, float cy, float x, float y )
{
	DaoxPathSegment *segment = NULL;
	DaoxVector2D start = {0.0,0.0};

	if( self->last->last->bezier == 0 ){
		start = self->last->last->P1;
		segment = self->last->last;
	}else{
		start = self->last->last->P2;
		segment = DaoxPathComponent_PushSegment( self->last );
		segment->P1 = start;
	}
	if( self->cmdRelative ){
		cx += start.x;
		cy += start.y;
		x += start.x;
		y += start.y;
	}
	segment->bezier = 2;
	segment->C1.x = cx;
	segment->C1.y = cy;
	segment->P2.x = x;
	segment->P2.y = y;
	segment->C2 = segment->C1;
	return segment;
}
DaoxPathSegment* DaoxPath_CubicTo( DaoxPath *self, float cx, float cy, float x, float y )
{
	DaoxPathSegment *segment = NULL;
	DaoxVector2D control = self->last->last->C2;
	DaoxVector2D start = self->last->last->P2;

	assert( self->last->last->bezier >= 2 );
	control.x = 2.0 * start.x - control.x;
	control.y = 2.0 * start.y - control.y;
	if( self->cmdRelative ){
		cx += start.x + x;
		cy += start.y + y;
		x += start.x;
		y += start.y;
	}
	segment = DaoxPathComponent_PushSegment( self->last );
	segment->bezier = 3;
	segment->P1 = start;
	segment->C1 = control;
	segment->C2.x = cx;
	segment->C2.y = cy;
	segment->P2.x = x;
	segment->P2.y = y;
	return segment;
}
DaoxPathSegment* DaoxPath_CubicTo2( DaoxPath *self, float cx1, float cy1, float cx2, float cy2, float x2, float y2 )
{
	DaoxPathSegment *segment = NULL;
	DaoxVector2D start = {0.0,0.0};

	if( self->last->last->bezier == 0 ){
		start = self->last->last->P1;
		segment = self->last->last;
	}else{
		start = self->last->last->P2;
		segment = DaoxPathComponent_PushSegment( self->last );
		segment->P1 = start;
	}
	if( self->cmdRelative ){
		cx1 += start.x;
		cy1 += start.y;
		cx2 += start.x + x2;
		cy2 += start.y + y2;
		x2 += start.x;
		y2 += start.y;
	}
	segment->bezier = 3;
	segment->C1.x = cx1;
	segment->C1.y = cy1;
	segment->C2.x = cx2;
	segment->C2.y = cy2;
	segment->P2.x = x2;
	segment->P2.y = y2;
	return segment;
}
DaoxPathSegment* DaoxPath_Close( DaoxPath *self )
{
	DaoxPathSegment *last = self->last->last;
	DaoxPathSegment *segment = NULL;
	if( self->last->first == self->last->last ){
		if( self->last->first->bezier == 0 ) return NULL;  /* no component data; */
	}
	if( DaoxVector2D_Dist( self->last->last->P2, self->last->first->P1 ) < 1E-16 ){
		self->last->last->next = self->last->first;
		return self->last->last;
	}
	segment = DaoxPathComponent_PushSegment( self->last );
	segment->bezier = 1;
	segment->P1 = last->P2;
	segment->P2 = self->last->first->P1;
	segment->next = self->last->first;
	return segment;
}






DaoxVector2D Interpolate( DaoxVector2D start, DaoxVector2D end, float S )
{
	DaoxVector2D res;
	res.x = (1.0 - S) * start.x + S * end.x;
	res.y = (1.0 - S) * start.y + S * end.y;
	return res;
}
DaoxVector2D DaoxLine_Intersect2( DaoxLine *self, DaoxLine *other )
{
	float S = 0, T = 0;
	int i = DaoxLine_Intersect( self->start, self->end, other->start, other->end, & S, & T );
	return Interpolate( self->start, self->end, S );
}



DaoxMatrixD4X4 DaoxPathSegment_CubicPowerBasis( DaoxPathSegment *self )
{
	DaoxMatrixD4X4 M3 = { {{1,0,0,0}, {-3,3,0,0}, {3,-6,3,0}, {-1,3,-3,1}} };
	DaoxMatrixD4X4 BS = { {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}} };

	BS.A.A11 = self->P1.x;  BS.A.A12 = self->P1.y;
	BS.A.A21 = self->C1.x;  BS.A.A22 = self->C1.y;
	BS.A.A31 = self->C2.x;  BS.A.A32 = self->C2.y;
	BS.A.A41 = self->P2.x;  BS.A.A42 = self->P2.y;

	/* Convert to power basis: */
	return DaoxMatrixD4X4_MulMatrix( & M3, & BS );
}

DaoxVector2D DaoxPathSegment_Tangent( DaoxPathSegment *self, DaoxMatrixD4X4 *dCS, double T )
{
	double T2 = T * T;
	DaoxVector2D tangent;
	tangent.x = dCS->A.A21 + dCS->A.A31 * T + dCS->A.A41 * T2;
	tangent.y = dCS->A.A22 + dCS->A.A32 * T + dCS->A.A42 * T2;
	return tangent;
}
double DaoxPathSegment_Curvature( DaoxPathSegment *self, DaoxMatrixD4X4 *dCS, DaoxMatrixD4X4 *ddCS, double T )
{
	double T2 = T * T;
	double dx = dCS->A.A21 + dCS->A.A31 * T + dCS->A.A41 * T2;
	double dy = dCS->A.A22 + dCS->A.A32 * T + dCS->A.A42 * T2;
	double ddx = ddCS->A.A31 + dCS->A.A41 * T;
	double ddy = ddCS->A.A32 + dCS->A.A42 * T;
	return fabs( dx*ddy + dy*ddx ) / (pow( dx*dx + dy*dy, 1.5 ) + EPSILON);
}
DaoxVectorD2 DaoxPathSegment_Interpolate( DaoxPathSegment *self, double T )
{
	DaoxVectorD2 point;
	double S0 = (1.0 - T) * (1.0 - T) * (1.0 - T);
	double S1 = 3.0 * (1.0 - T) * (1.0 - T) * T;
	double S2 = 3.0 * (1.0 - T) * T * T;
	double S3 = T * T * T;
	point.x = S0 * self->P1.x + S1 * self->C1.x + S2 * self->C2.x + S3 * self->P2.x;
	point.y = S0 * self->P1.y + S1 * self->C1.y + S2 * self->C2.y + S3 * self->P2.y;
	return point;
}
void DaoxPathSegment_InitSubSegments( DaoxPathSegment *self )
{
	if( self->first == NULL ) self->first = DaoxPathSegment_New( NULL );
	if( self->second == NULL ) self->second = DaoxPathSegment_New( NULL );
	self->first->bezier = self->second->bezier = self->bezier;
	self->first->convexness = self->second->convexness = self->convexness;
	self->first->refined = self->second->refined = 0;
	self->refined = 1;

	self->first->P1 = self->P1;
	self->second->P2 = self->P2;
}
void DaoxPathSegment_DivideLinear( DaoxPathSegment *self, float at )
{
	DaoxPathSegment_InitSubSegments( self );

	self->first->P2 = self->second->P1 = DaoxVector2D_Interpolate( self->P1, self->P2, at );
}
void DaoxPathSegment_DivideQuadratic( DaoxPathSegment *self, float at )
{
	DaoxPathSegment_InitSubSegments( self );
	self->first->C1 = DaoxVector2D_Interpolate( self->P1, self->C1, at );
	self->second->C1 = DaoxVector2D_Interpolate( self->C1, self->P2, at );
	self->first->P2 = DaoxVector2D_Interpolate( self->first->C1, self->second->C1, at );
	self->second->P1 = self->first->P2;
}
void DaoxPathSegment_DivideCubic( DaoxPathSegment *self, float at )
{
	DaoxVector2D Q1 = DaoxVector2D_Interpolate( self->C1, self->C2, at );

	DaoxPathSegment_InitSubSegments( self );

	self->first->C1 = DaoxVector2D_Interpolate( self->P1, self->C1, at );
	self->first->C2 = DaoxVector2D_Interpolate( self->first->C1, Q1, at );

	self->second->C2 = DaoxVector2D_Interpolate( self->C2, self->P2, at );
	self->second->C1 = DaoxVector2D_Interpolate( Q1, self->second->C2, at );

	self->first->P2 = DaoxVector2D_Interpolate( self->first->C2, self->second->C1, at );
	self->second->P1 = self->first->P2;
}
void DaoxPathSegment_Divide( DaoxPathSegment *self, float at )
{
	if( self->bezier == 2 ){
		DaoxPathSegment_DivideQuadratic( self, at );
	}else if( self->bezier == 3 ){
		DaoxPathSegment_DivideCubic( self, at );
	}else{
		DaoxPathSegment_DivideLinear( self, at );
	}
}


int DaoxPathSegment_TryDivideQuadratic( DaoxPathSegment *self, float maxlen, float maxdiff )
{
	float PC = DaoxVector2D_Dist( self->P1, self->C1 );
	float CP = DaoxVector2D_Dist( self->C1, self->P2 );
	float PP = DaoxVector2D_Dist( self->P1, self->P2 );
	float length = 0.5 * (PC + CP + PP);
	float delta = (PC + CP - PP) / (PP + 1E-16);

	if( length < maxlen && (delta < maxdiff || length*delta < 1) ){
		self->refined = 0;
		if( self->first ) self->first->refined = 0;
		if( self->second ) self->second->refined = 0;
		return 0;
	}
	/* Interpolate between 0.5 and PC/(PC+CP), in case that PC/(PC+CP) become 0 or 1: */
	DaoxPathSegment_DivideQuadratic( self, 0.1 * 0.5 + 0.9 * PC / (PC + CP) );
	return 1;
}
int DaoxPathSegment_TryDivideCubic( DaoxPathSegment *self, float maxlen, float maxdiff )
{
	float PC = DaoxVector2D_Dist( self->P1, self->C1 );
	float CC = DaoxVector2D_Dist( self->C1, self->C2 );
	float CP = DaoxVector2D_Dist( self->C2, self->P2 );
	float PP = DaoxVector2D_Dist( self->P1, self->P2 );
	float delta = (PC + CC + CP - PP) / (PP + 1E-16);
	float length = 0.5 * (PC + CC + CP + PP);

	if( length < maxlen && (delta < maxdiff || length*delta < 1) ){
		self->refined = 0;
		if( self->first ) self->first->refined = 0;
		if( self->second ) self->second->refined = 0;
		return 0;
	}
	/* Interpolate between 0.5 and PC/(PC+CP), in case that PC/(PC+CP) become 0 or 1: */
	DaoxPathSegment_DivideCubic( self, 0.1 * 0.5 + 0.9 * PC / (PC + CP) );
	return 1;
}
void DaoxPathSegment_RefineLinear( DaoxPathSegment *self, float maxlen, float maxdiff )
{
	if( self->refined ){
		DaoxPathSegment_RefineLinear( self->first, maxlen, maxdiff );
		DaoxPathSegment_RefineLinear( self->second, maxlen, maxdiff );
		return;
	}
}
void DaoxPathSegment_RefineQuadratic( DaoxPathSegment *self, float maxlen, float maxdiff )
{
	if( self->refined ){
		DaoxPathSegment_RefineQuadratic( self->first, maxlen, maxdiff );
		DaoxPathSegment_RefineQuadratic( self->second, maxlen, maxdiff );
		return;
	}
	if( DaoxPathSegment_TryDivideQuadratic( self, maxlen, maxdiff ) == 0 ) return;

	DaoxPathSegment_RefineQuadratic( self->first, maxlen, maxdiff );
	DaoxPathSegment_RefineQuadratic( self->second, maxlen, maxdiff );
}
void DaoxPathSegment_RefineCubic( DaoxPathSegment *self, float maxlen, float maxdiff )
{
	if( self->refined ){
		DaoxPathSegment_RefineCubic( self->first, maxlen, maxdiff );
		DaoxPathSegment_RefineCubic( self->second, maxlen, maxdiff );
		return;
	}
	if( DaoxPathSegment_TryDivideCubic( self, maxlen, maxdiff ) == 0 ) return;

	DaoxPathSegment_RefineCubic( self->first, maxlen, maxdiff );
	DaoxPathSegment_RefineCubic( self->second, maxlen, maxdiff );
}

void DaoxPathSegment_Refine( DaoxPathSegment *self, float maxlen, float maxdiff )
{
	if( self->bezier == 1 ){
		DaoxPathSegment_RefineLinear( self, maxlen, maxdiff );
	}else if( self->bezier == 2 ){
		DaoxPathSegment_RefineQuadratic( self, maxlen, maxdiff );
	}else if( self->bezier == 3 ){
		DaoxPathSegment_RefineCubic( self, maxlen, maxdiff );
	}
}

double DaoxPathSegment_MaxLength( DaoxPathSegment *self )
{
	double dist = 0.0;
	switch( self->bezier ){
	case 1:
		dist += DaoxVector2D_Dist( self->P1, self->P2 );
		break;
	case 2:
		dist += DaoxVector2D_Dist( self->P1, self->C1 );
		dist += DaoxVector2D_Dist( self->C1, self->P2 );
		break;
	case 3:
		dist += DaoxVector2D_Dist( self->P1, self->C1 );
		dist += DaoxVector2D_Dist( self->C1, self->C2 );
		dist += DaoxVector2D_Dist( self->C2, self->P2 );
		break;
	}
	return dist;
}
void DaoxPathSegment_SubSegments( DaoxPathSegment *self, DaoxPlainArray *segments, float maxlen, float maxdiff )
{
	DaoxPathSegment *segment;
	int i;

	segment = (DaoxPathSegment*) DaoxPlainArray_Push( segments );
	*segment = *self;
	segment->first = segment->second = segment->next = NULL;
	for(i=0; i<segments->size; ){
		DaoxPathSegment segment1 = {0};
		DaoxPathSegment segment2 = {0};
		DaoxPathSegment *segment = DaoxPlainArray_Get( segments, i );
		DaoxPathSegment *seg, *next = segment->next;
		double max = DaoxPathSegment_MaxLength( segment );
		double min = DaoxVector2D_Dist( segment->P1, segment->P2 );
		if( max < maxlen && ((max - min) < maxdiff*max || max < 1E-6) ){
			i += 1;
			continue;
		}

		segment1.next = segment2.next = NULL;
		segment->first = & segment1;
		segment->second = & segment2;
		DaoxPathSegment_Divide( segment, 0.5 );

		seg = (DaoxPathSegment*) DaoxPlainArray_Push( segments );
		segment = DaoxPlainArray_Get( segments, i );
		*segment = segment1;
		*seg = segment2;
		segment->next = (DaoxPathSegment*) (daoint)(segments->size - 1);
		seg->next = next;
	}
}
double DaoxPathSegment_Length( DaoxPathSegment *self )
{
	DaoxPlainArray *segments = DaoxPlainArray_New( sizeof(DaoxPathSegment) );
	double res = 0.0;
	int i;

	DaoxPathSegment_SubSegments( self, segments, 1E6, 1E-3 );

	//printf( "DaoxPathSegment_Length: %3i\n", segments->size );
	for(i=0; i<segments->size; ++i){
		DaoxPathSegment *segment = DaoxPlainArray_Get( segments, i );
		double max = DaoxPathSegment_MaxLength( segment );
		double min = DaoxVector2D_Dist( segment->P1, segment->P2 );
		res += 0.5 * (min + max);
	}
	DaoxPlainArray_Delete( segments );
	return res;
}


void DaoxPathComponent_Refine( DaoxPathComponent *self, float maxlen, float maxdiff )
{
	DaoxPathSegment *first = self->refined.first ? self->refined.first : self->first;
	DaoxPathSegment *segment = first;
	do {
		DaoxPathSegment_Refine( segment, maxlen, maxdiff );
		if( segment->refined == 0 ) DaoxPathSegment_Divide( segment, 0.5 );
		segment = segment->next;
	} while( segment && segment != first );
}


DaoxPathSegment* DaoxPathSegment_LocateByDistance( DaoxPathSegment *self, float distance, float offset, float *p )
{
	return NULL;
#if 0
	if( distance < offset || distance > (offset + self->length) ) return NULL;
	if( p ) *p = (distance - offset) / self->length;
	if( self->refined == 0 ) return self;
	if( distance <= (offset + self->first->length) ){
		return DaoxPathSegment_LocateByDistance( self->first, distance, offset, p );
	}
	return DaoxPathSegment_LocateByDistance( self->second, distance, offset + self->first->length, p );
#endif
}

DaoxPathSegment* DaoxPathComponent_LocateByDistance( DaoxPathComponent *self, float distance, float offset, float *p )
{
	DaoxPathSegment *first = self->first;
	DaoxPathSegment *segment = first;
#if 0
	do {
		DaoxPathSegment *seg = DaoxPathSegment_LocateByDistance( segment, distance, offset, p );
		if( seg ) return seg;
		offset += segment->length;
		segment = segment->next;
	} while( segment && segment != first );
#endif
	return NULL;
}

DaoxPathSegment* DaoxPath_LocateByDistance( DaoxPath *self, float distance, float *p )
{
	float offset = 0.0;
	DaoxPathSegment *seg;
	DaoxPathComponent *com;
	if( distance < 0.0 ) return NULL;
	for(com=self->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		seg = DaoxPathComponent_LocateByDistance( com, distance, offset, p );
		if( seg ) return seg;
		//offset += com->length;
	}
	return NULL;
}
DaoxPathSegment* DaoxPath_LocateByPercentage( DaoxPath *self, float percentage, float *p )
{
}


DaoxOBBox2D DaoxPathSegment_GetOBBox( DaoxPathSegment *self )
{
	DaoxOBBox2D  obbox;
	DaoxVector2D P1 = self->P1;
	DaoxVector2D P2 = self->P2;
	DaoxVector2D xaxis = DaoxVector2D_Sub( & P2, & P1 );
	DaoxVector2D yaxis;
	DaoxVector2D xaxis1, xaxis2, yaxis1, yaxis2;
	DaoxVector2D delta;
	DaoxVector2D points[12];
	double len = sqrt( DaoxVector2D_Norm2( & xaxis ) );
	double xmin, xmax, ymin = 0.0, ymax = 0.0;
	double eps1 = 1E-4 * len;
	double eps2 = 1E-3 * len;
	int i, count = 0;

	if( eps1 > 1E-4 ) eps1 = 1E-4;
	if( eps2 > 1E-3 ) eps2 = 1E-3;

#if 0
	eps1 = 1E-5 * len;
	eps2 = 1E-5 * len;
	if( eps1 > 1E-4 ) eps1 = 1E-4;
	if( eps2 > 1E-3 ) eps2 = 1E-3;
#endif

	xaxis = DaoxVector2D_Normalize( & xaxis );
	yaxis.x = - xaxis.y;
	yaxis.y = xaxis.x;
	delta = DaoxVector2D_Scale( & xaxis, eps1 );
	P1 = DaoxVector2D_Add( & P1, & delta );
	delta = DaoxVector2D_Sub( & P2, & P1 );
	xmin = 0.0;
	xmax = DaoxVector2D_Dot( & delta, & xaxis );
	if( self->bezier >= 2 ) points[count++] = self->C1;
	if( self->bezier >= 3 ) points[count++] = self->C2;
	for(i=0; i<count; ++i){
		DaoxVector2D C = DaoxVector2D_Sub( & points[i], & P1 );
		double dotx = DaoxVector2D_Dot( & C, & xaxis );
		double doty = DaoxVector2D_Dot( & C, & yaxis );
		if( dotx < xmin ) xmin = dotx;
		if( dotx > xmax ) xmax = dotx;
		if( doty < ymin ) ymin = doty;
		if( doty > ymax ) ymax = doty;
	}
	ymax += eps2;
	ymin -= eps2;
	xaxis1 = DaoxVector2D_Scale( & xaxis, xmin );
	yaxis1 = DaoxVector2D_Scale( & yaxis, ymin );
	xaxis2 = DaoxVector2D_Scale( & xaxis, xmax - xmin );
	yaxis2 = DaoxVector2D_Scale( & yaxis, ymax - ymin );
	obbox.O = P1;
	obbox.O = DaoxVector2D_Add( & obbox.O, & xaxis1 );
	obbox.O = DaoxVector2D_Add( & obbox.O, & yaxis1 );
	obbox.X = DaoxVector2D_Add( & obbox.O, & xaxis2 );
	obbox.Y = DaoxVector2D_Add( & obbox.O, & yaxis2 );
	return obbox;
}



void DaoxPathComponent_RetrieveSegment( DaoxPathComponent *self, DaoxPathSegment *segment )
{
	if( segment->refined == 0 ){
		segment->next = NULL;
		if( self->refined.first == NULL ){
			self->refined.first = self->refined.last = segment;
		}else{
			self->refined.last->next = segment;
			self->refined.last = segment;
		}
	}else if( segment->refined ){
		DaoxPathComponent_RetrieveSegment( self, segment->first );
		DaoxPathComponent_RetrieveSegment( self, segment->second );
	}
}
void DaoxPathComponent_RetrieveRefined( DaoxPathComponent *self )
{
	DaoxPathSegment *segment = self->first;
	self->refined.first = self->refined.last = NULL;
	do {
		DaoxPathComponent_RetrieveSegment( self, segment );
		segment = segment->next;
	} while( segment && segment != self->first );
	if( self->last->next == self->first ) self->refined.last->next = self->refined.first;
}
int DaoxPathSegment_CheckConvexness2( DaoxPathSegment *self, DaoxVector2D point )
{
	double area1, area2, epsilon = 0.0;
	if( self->bezier == 1 ) return 0;
	area1 = DaoxTriangle_Area( self->P1, self->P2, point );
	area2 = DaoxTriangle_Area( self->P1, self->P2, self->C1 );
	if( area1 < epsilon ) area2 = - area2;
	if( area2 > epsilon ) return -1;
	if( area2 < epsilon ) return 1;
	return 0;
}
void DaoxPathSegment_SetConvexness( DaoxPathSegment *self, int convexness )
{
	self->convexness = convexness;
	if( self->refined ){
		DaoxPathSegment_SetConvexness( self->first, convexness );
		DaoxPathSegment_SetConvexness( self->second, convexness );
	}
}
void DaoxPathSegment_CheckConvexness( DaoxPathSegment *self, DaoxVector2D point )
{
	int convexness = DaoxPathSegment_CheckConvexness2( self, point );
	DaoxPathSegment_SetConvexness( self, convexness );
}

void DaoxPath_ImportPath( DaoxPath *self, DaoxPath *path, DaoxMatrix3D *transform )
{
	DaoxPathComponent *com, *com2;
	DaoxPathSegment *seg, *seg2;
	for(com=path->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		com2 = DaoxPath_PushComponent( self );
		seg = com->first;
		do {
			seg2 = DaoxPathComponent_PushSegment( com2 );
			seg2->refined = 0;
			seg2->bezier = seg->bezier;
			seg2->P1 = DaoxMatrix3D_MulVector( transform, & seg->P1, 1 );
			seg2->P2 = DaoxMatrix3D_MulVector( transform, & seg->P2, 1 );
			seg2->C1 = DaoxMatrix3D_MulVector( transform, & seg->C1, 1 );
			seg2->C2 = DaoxMatrix3D_MulVector( transform, & seg->C2, 1 );
			seg = seg->next;
		} while( seg && seg != com->first );
		if( com->last->next == com->first ) com2->last->next = com2->first;
	}
}

void DaoxPath_Refine( DaoxPath *self, float maxlen, float maxdiff )
{
	DaoxPathComponent *com;
	printf( "DaoxPath_Refine: %15f %15f %i\n", maxlen, maxdiff, self->first->first->bezier );
	for(com=self->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		DaoxPathComponent_Refine( com, maxlen, maxdiff );
		//self->length += com->length;
	}
}

DaoxVectorD4 DaoxPathSegment_ComputeDeterminantVector( DaoxPathSegment *self )
{
	DaoxMatrixD4X4 CS = DaoxPathSegment_CubicPowerBasis( self );
	DaoxVectorD3 C0, C1, C2, C3; /* M3*B */
	DaoxMatrixD3X3 mat0, mat1, mat2, mat3;
	DaoxVectorD4 dets;

	C0.x = CS.A.A11;  C0.y = CS.A.A12;  C0.z = CS.A.A13;
	C1.x = CS.A.A21;  C1.y = CS.A.A22;  C1.z = CS.A.A23;
	C2.x = CS.A.A31;  C2.y = CS.A.A32;  C2.z = CS.A.A33;
	C3.x = CS.A.A41;  C3.y = CS.A.A42;  C3.z = CS.A.A43;

	mat0 = DaoxMatrixD3X3_InitRows( C3, C2, C1 );
	mat1 = DaoxMatrixD3X3_InitRows( C3, C2, C0 );
	mat2 = DaoxMatrixD3X3_InitRows( C3, C1, C0 );
	mat3 = DaoxMatrixD3X3_InitRows( C2, C1, C0 );

	dets.x =   DaoxMatrixD3X3_Determinant( & mat0 );
	dets.y = - DaoxMatrixD3X3_Determinant( & mat1 );
	dets.z =   DaoxMatrixD3X3_Determinant( & mat2 );
	dets.w = - DaoxMatrixD3X3_Determinant( & mat3 );
	return dets;
}
void DaoxPathSegment_TryDivideLoop( DaoxPathSegment *self, int index, DArray *segments )
{
	DaoxVectorD4 dets = DaoxPathSegment_ComputeDeterminantVector( self );
	double d0 = dets.x;
	double d1 = dets.y;
	double d2 = dets.z;
	double d3 = dets.w;
	double delta = 3*d2*d2 - 4*d1*d3;
	double td, sd, te, se, at = 0.0;
	double root, tolerance = 1E-6;

	if( DaoxPathSegment_MaxLength( self ) < 1E-2 ) return;
	if( fabs( d1 ) < EPSILON || delta >= 0.0 ) return; /* Not a loop cubic bezier; */
	root = sqrt( - delta );
	td = d2 + root;
	sd = 2*d1;
	te = d2 - root;
	se = 2*d1;
	at = td / sd;
	if( at < tolerance || at > (1.0 - tolerance) ) at = te / se;
	if( at < tolerance || at > (1.0 - tolerance) ) return; /* double point not on the segement; */

	DaoxPathSegment_Divide( self, at );
	DArray_Append( segments, self->first );
	DArray_Append( segments, self->second );
	segments->items.pVoid[index] = NULL;
}
void DaoxPathMesh_HandleSegment( DaoxPathMesh *self, DaoxPathSegment *segment, double start, double end )
{
	DaoxPathSegment orientedSegment = *segment;
	DaoxMatrixD4X4 M3INV, F, M3F;
	DaoxTexturedPoint *P0, *P1, *P2, *P3, *P4, *P5;
	DaoxVectorD4 M3I[4] = { {1,0,0,0}, {1,1./3.,0,0}, {1,2./3.,1./3.,0}, {1,1,1,1} };
	DaoxVectorD4 dets;
	DaoxVector2D SP2, SC1, norm;
	double d0, d1, d2, d3, delta, dot;
	int i;

	if( segment->bezier < 2 ) return;

	SP2 = DaoxVector2D_Sub( & segment->P2, & segment->P1 );
	SC1 = DaoxVector2D_Sub( & segment->C1, & segment->P1 );
	norm = DaoxMatrix3D_RotateVector( SP2, -0.5*M_PI );
	dot = DaoxVector2D_Dot( & SC1, & norm );
	if( (dot * segment->convexness) > 0 ){
		/* Reverse the segment, so that the right side is inside: */
		orientedSegment.P1 = segment->P2;
		orientedSegment.P2 = segment->P1;
		orientedSegment.C1 = segment->C2;
		orientedSegment.C2 = segment->C1;
		orientedSegment.convexness = - segment->convexness;
	}
	segment = & orientedSegment;
	dets = DaoxPathSegment_ComputeDeterminantVector( segment );
	d0 = dets.x;
	d1 = dets.y;
	d2 = dets.z;
	d3 = dets.w;
	delta = 3*d2*d2 - 4*d1*d3;

	//printf( "DaoxPath_HandleSegment %i\n", segment->bezier );
	P0 = P1 = P2 = P3 = P4 = P5 = NULL;

	for(i=0; i<4; ++i) M3INV.V.V[i] = M3I[i];
	memset( & F, 0, sizeof(DaoxMatrixD4X4) );
	F.A.A14 = 1.0;

	if( segment->bezier == 2 ){
		DaoxPlainArray_Reserve( self->patches, self->patches->size + 3 );
		P0 = (DaoxTexturedPoint*) DaoxPlainArray_Push( self->patches );
		P1 = (DaoxTexturedPoint*) DaoxPlainArray_Push( self->patches );
		P2 = (DaoxTexturedPoint*) DaoxPlainArray_Push( self->patches );
		P0->point = segment->P1;
		P1->point = segment->P2;
		P2->point = segment->C1;
		P0->klm.x = P0->klm.y = P0->klm.z = 0.0;
		P1->klm.x = P1->klm.y = P1->klm.z = 1.0;
		P2->klm.x = P2->klm.z = 0.5;  P2->klm.y = 0.0;
		P0->offset = start;
		P1->offset = 0.5*(start + end);
		P2->offset = end;
		return;
	}

	if( d1 != 0.0 && delta >= 0.0 ){ /* Serpentine, or Cusp with inflection at infinity: */
		double root = sqrt( delta / 3.0 );
		double tl = d2 + root, sl = 2*d1;
		double tm = d2 - root, sm = 2*d1;
		double tn = 1.0, sn = 0.0;
		double scalel = 1.0 / sqrt( tl*tl + sl*sl );
		double scalem = 1.0 / sqrt( tm*tm + sm*sm );
		tl *= scalel;
		sl *= scalel;
		tm *= scalem;
		sm *= scalem;
		F.A.A11 = tl*tm;
		F.A.A12 = tl*tl*tl;
		F.A.A13 = tm*tm*tm;
		F.A.A21 = -sm*tl - sl*tm;
		F.A.A22 = -3*sl*tl*tl;
		F.A.A23 = -3*sm*tm*tm;
		F.A.A31 = sl*sm;
		F.A.A32 = 3*sl*sl*tl;
		F.A.A33 = 3*sm*sm*tm;
		F.A.A42 = -sl*sl*sl;
		F.A.A43 = -sm*sm*sm;
	}else if( d1 != 0.0 && delta < 0.0 ){ /* Loop: */
		double root = sqrt( - delta );
		double td = d2 + root, sd = 2*d1;
		double te = d2 - root, se = 2*d1;
		double scaled = 1.0 / sqrt( td*td + sd*sd );
		double scalee = 1.0 / sqrt( te*te + se*se );
		td *= scaled;
		sd *= scaled;
		te *= scalee;
		se *= scalee;
		F.A.A11 = td*te;
		F.A.A12 = td*td*te;
		F.A.A13 = td*te*te;
		F.A.A21 = -se*td - sd*te;
		F.A.A22 = -se*td*td - 2*sd*te*td;
		F.A.A23 = -sd*te*te - 2*se*td*te;
		F.A.A31 = sd*se;
		F.A.A32 = te*sd*sd + 2*se*td*sd;
		F.A.A33 = td*se*se + 2*sd*te*se;
		F.A.A42 = -sd*sd*se;
		F.A.A43 = -sd*se*se;
	}else if( d1 == 0.0 && d2 != 0.0 ){ /* Cusp with cusp at infinity */
		double tl = d3, sl = 3*d2;
		F.A.A11 = tl;
		F.A.A12 = tl*tl*tl;
		F.A.A13 = 1.0;
		F.A.A21 = -sl;
		F.A.A22 = -3*sl*tl*tl;
		F.A.A32 = 3*sl*sl*tl;
		F.A.A42 = -sl*sl*sl;
	}else if( d1 == 0.0 && d2 == 0.0 && d3 != 0.0 ){ /* quadratic */
	}else{ /* d1 == d2 == d3 == 0.0, line */
		return;
	}

	/* Reserve first, so that DaoxPlainArray_Push will return 6 valid pointers: */
	DaoxPlainArray_Reserve( self->patches, self->patches->size + 6 );
	P0 = (DaoxTexturedPoint*) DaoxPlainArray_Push( self->patches );
	P3 = (DaoxTexturedPoint*) DaoxPlainArray_Push( self->patches );
	P2 = (DaoxTexturedPoint*) DaoxPlainArray_Push( self->patches );
	P4 = (DaoxTexturedPoint*) DaoxPlainArray_Push( self->patches );
	P5 = (DaoxTexturedPoint*) DaoxPlainArray_Push( self->patches );
	P1 = (DaoxTexturedPoint*) DaoxPlainArray_Push( self->patches );
	P0->point = segment->P1;
	P1->point = segment->C1;
	P2->point = segment->C2;
	P3->point = segment->P2;
	P0->offset = P1->offset = start;
	P2->offset = P2->offset = end;

	M3F = DaoxMatrixD4X4_MulMatrix( & M3INV, & F );
	P0->klm.x = M3F.A.A11;  P0->klm.y = M3F.A.A12;  P0->klm.z = M3F.A.A13;
	P1->klm.x = M3F.A.A21;  P1->klm.y = M3F.A.A22;  P1->klm.z = M3F.A.A23;
	P2->klm.x = M3F.A.A31;  P2->klm.y = M3F.A.A32;  P2->klm.z = M3F.A.A33;
	P3->klm.x = M3F.A.A41;  P3->klm.y = M3F.A.A42;  P3->klm.z = M3F.A.A43;
	if( d1 != 0.0 && delta > 0.0 ){ /* serpentine curve: */
		if( d1 < 0.0 ){
			P0->klm.x = - P0->klm.x;  P0->klm.y = - P0->klm.y;
			P1->klm.x = - P1->klm.x;  P1->klm.y = - P1->klm.y;
			P2->klm.x = - P2->klm.x;  P2->klm.y = - P2->klm.y;
			P3->klm.x = - P3->klm.x;  P3->klm.y = - P3->klm.y;
		}
	}else if( d1 != 0.0 && delta < 0.0 ){ /* Loop: */
		double H0 = d3*d1 - d2*d2; /* H(s,t)/(36*s^2) for t/s=0; */
		double H1 = (d3*d1 - d2*d2) + d1*d2 - d1*d1; /* H(s,t)/(36*s^2) for t/s=1; */
		double H = fabs( H0 ) > fabs( H1 ) ? H0 : H1;
		if( d1*H > 0.0 ){
			P0->klm.x = - P0->klm.x;  P0->klm.y = - P0->klm.y;
			P1->klm.x = - P1->klm.x;  P1->klm.y = - P1->klm.y;
			P2->klm.x = - P2->klm.x;  P2->klm.y = - P2->klm.y;
			P3->klm.x = - P3->klm.x;  P3->klm.y = - P3->klm.y;
		}
	}else if( d1 == 0.0 && d2 == 0.0 && d3 != 0.0 ){ /* quadratic */
		P0->klm.x = P0->klm.y = P0->klm.z = 0.0;
		P1->klm.x = P1->klm.z = 0.5;  P1->klm.y = 0.0;
		P2->klm.x = P2->klm.z = 0.5;  P2->klm.y = 0.0;
		P3->klm.x = P3->klm.y = P3->klm.z = 1.0;
	}
	*P4 = *P0;
	*P5 = *P2;
}

void DaoxPath_Preprocess( DaoxPath *self, DaoxTriangulator *triangulator )
{
	DArray *segments;
	DaoxVector3D *point, *point2;
	DaoxPlainArray *boxes;
	DaoxPlainArray *points;
	DaoxPathComponent *com;
	DaoxPathSegment *seg;
	daoint i, j, count = 0;
	float len, maxlen = 100.0;
	float maxdiff = 0.1;

	self->length = 0;
	DaoxPathMesh_Reset( self->mesh );
	DaoxOBBox2D_ResetBox( & self->obbox, NULL, 0 );
	if( self->first->first->bezier == 0 ) return;

	segments = DArray_New(0);
	boxes = DaoxPlainArray_New( sizeof(DaoxOBBox2D) );

	DaoxTriangulator_Reset( triangulator );
	DaoxPath_Refine( self, maxlen, maxdiff );
	for(com=self->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		DaoxPathComponent_RetrieveRefined( com );
		if( com->refined.last == NULL || com->refined.last->next == NULL ) continue;
		seg = com->refined.first;
		do {
			DArray_PushBack( segments, seg );
			self->length += DaoxPathSegment_Length( seg );
			seg = seg->next;
		} while( seg && seg != com->refined.first );
	}
	printf( "Before intersection refinement: %i\n", (int)segments->size );
	for(i=0; i<segments->size; ++i){
		DaoxPathSegment *S1 = (DaoxPathSegment*) segments->items.pVoid[i];
		DaoxOBBox2D *box = DaoxPlainArray_Push( boxes );
		*box = DaoxPathSegment_GetOBBox( S1 );
		if( S1 == NULL || S1->bezier != 3 ) continue;
		DaoxPathSegment_TryDivideLoop( S1, i, segments );
	}
	printf( "Before intersection refinement: %i\n", (int)segments->size );
	for(i=0; i<segments->size; ++i){
		DaoxPathSegment *S1 = (DaoxPathSegment*) segments->items.pVoid[i];
		DaoxOBBox2D *B1 = (DaoxOBBox2D*) DaoxPlainArray_Get( boxes, i );
		double len1;
		if( S1 == NULL || S1->bezier == 0 ) continue;
		if( (len1 = DaoxPathSegment_MaxLength( S1 )) < 1.0 ) continue;
		for(j=i+1; j<segments->size; ++j){
			DaoxPathSegment *S2 = (DaoxPathSegment*) segments->items.pVoid[j];
			DaoxOBBox2D *B2 = (DaoxOBBox2D*) DaoxPlainArray_Get( boxes, j );
			double len2;
			if( S2 == NULL || S2->bezier == 0 ) continue;
			if( (len2 = DaoxPathSegment_MaxLength( S2 )) < 1.0 ) continue;
			if( DaoxOBBox2D_Intersect( B1, B2 ) >= 0 ){
				DaoxOBBox2D *box1, *box2;
				float R1 = 0.5, R2 = 0.5;
				if( len1 > len2 ){
					DaoxPathSegment_Divide( S1, R1 );
					DArray_Append( segments, S1->first );
					DArray_Append( segments, S1->second );
					box1 = DaoxPlainArray_Push( boxes );
					box2 = DaoxPlainArray_Push( boxes );
					*box1 = DaoxPathSegment_GetOBBox( S1->first );
					*box2 = DaoxPathSegment_GetOBBox( S1->second );
					segments->items.pVoid[i] = NULL;
					break;
				}else if( len2 > len1 ){
					DaoxPathSegment_Divide( S2, R2 );
					DArray_Append( segments, S2->first );
					DArray_Append( segments, S2->second );
					box1 = DaoxPlainArray_Push( boxes );
					box2 = DaoxPlainArray_Push( boxes );
					*box1 = DaoxPathSegment_GetOBBox( S2->first );
					*box2 = DaoxPathSegment_GetOBBox( S2->second );
					segments->items.pVoid[j] = NULL;
				}
			}
		}
		if( segments->items.pVoid[i] == NULL ) continue;
	}
#if 0
#endif
	printf( "After intersection refinement: %i\n", (int)segments->size );
	segments->size = 0;
	for(com=self->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		DaoxPathComponent_RetrieveRefined( com );
		if( com->refined.last == NULL || com->refined.last->next == NULL ) continue;
		seg = com->refined.first;
		do {
			DaoxTriangulator_PushPoint( triangulator, seg->P1.x, seg->P1.y );
			DArray_PushBack( segments, seg );
			seg = seg->next;
		} while( seg && seg != com->refined.first );
		DaoxTriangulator_CloseContour( triangulator );
	}
	printf( "DaoxPath_Segment 2: %i\n", (int) triangulator->vertices->size );
	DaoxTriangulator_Triangulate( triangulator );
	/* Check local convexness with triangulation to handle possible presence of holes: */
	for(i=0; i<triangulator->triangles->size; ++i){
		DaoxTriangle *triangle = triangulator->triangles->pod.triangles + i;
		int C = triangle->index[0];
		int A = triangle->index[1];
		int B = triangle->index[2];
		DaoxPathSegment *SC = (DaoxPathSegment*) segments->items.pVoid[C];
		DaoxPathSegment *SA = (DaoxPathSegment*) segments->items.pVoid[A];
		DaoxPathSegment *SB = (DaoxPathSegment*) segments->items.pVoid[B];
		if( SC->next == SA ) DaoxPathSegment_CheckConvexness( SC, SB->P1 );
		if( SA->next == SB ) DaoxPathSegment_CheckConvexness( SA, SC->P1 );
		if( SB->next == SC ) DaoxPathSegment_CheckConvexness( SB, SA->P1 );
	}
	DaoxTriangulator_Reset( triangulator );
	for(com=self->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		if( com->refined.last == NULL || com->refined.last->next == NULL ) continue;
		seg = com->refined.first;
		do {
			if( seg->bezier >= 2 ) DaoxPathMesh_HandleSegment( self->mesh, seg, 0.0, 0.0 );
			DaoxTriangulator_PushPoint( triangulator, seg->P1.x, seg->P1.y );
			if( seg->convexness < 0 && seg->bezier >= 2 ){
				/* Push control points for locally concave bezier curves: */
				DaoxTriangulator_PushPoint( triangulator, seg->C1.x, seg->C1.y );
				if( seg->bezier == 3 ){
					DaoxTriangulator_PushPoint( triangulator, seg->C2.x, seg->C2.y );
				}
			}
			seg = seg->next;
		} while( seg && seg != com->refined.first );
		DaoxTriangulator_CloseContour( triangulator );
	}
	DaoxTriangulator_Triangulate( triangulator );
	for(i=0; i<triangulator->points->size; i+=1){
		DaoxVector2D *pt = triangulator->points->pod.vectors2d + i;
		point = (DaoxVector3D*) DaoxPlainArray_Push( self->mesh->points );
		point->x = pt->x;
		point->y = pt->y;
		point->z = 0.0;
	}
	for(i=0; i<triangulator->triangles->size; i+=1){
		DaoxTriangle *triangle = triangulator->triangles->pod.triangles + i;
		DaoxTriangle *triangle2 = DaoxPlainArray_PushTriangle( self->mesh->triangles );
		*triangle2 = *triangle;
	}

	points = DaoxPlainArray_New( sizeof(DaoxVector2D) );
	for(com=self->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		seg = com->refined.first;
		do {
			DaoxVector2D *point = DaoxPlainArray_PushVector2D( points );
			*point = seg->P1;
			if( seg->next == NULL ){
				point = DaoxPlainArray_PushVector2D( points );
				*point = seg->P2;
			}
			if( com->first->bezier >= 2 ){
				point = DaoxPlainArray_PushVector2D( points );
				*point = seg->C1;
				if( com->first->bezier == 3 ){
					point = DaoxPlainArray_PushVector2D( points );
					*point = seg->C2;
				}
			}
			seg = seg->next;
		} while( seg && seg != com->refined.first );
	}
	DaoxOBBox2D_ResetBox( & self->obbox, points->pod.vectors2d, points->size );
	printf( "DaoxPath_Preprocess: %i %i\n", self->mesh->points->size, self->mesh->patches->size );
	//self->mesh->patches->size = 0;
	
	DaoxPlainArray_Delete( boxes );
	DaoxPlainArray_Delete( points );
	DArray_Delete( segments );
}

int DaoxLine_CheckPoint( DaoxVector2D start, DaoxVector2D end, DaoxVector2D point )
{
	DaoxVector2D forward = DaoxVector2D_Sub( & end, & start );
	DaoxVector2D right = DaoxMatrix3D_RotateVector( forward, -0.5*M_PI );
	DaoxVector2D point2 = DaoxVector2D_Sub( & point, & start );
	double dot = DaoxVector2D_Dot( & point2, & right );
	if( dot < 0.0 ) return -1;
	return dot > 0.0 ? 1 : 0;
}
DaoxLine DaoxLine_Copy( DaoxVector2D start, DaoxVector2D end, float displacement )
{
	DaoxVector2D forward = DaoxVector2D_Sub( & end, & start );
	DaoxVector2D right = DaoxMatrix3D_RotateVector( forward, -0.5*M_PI );
	DaoxVector2D norm = DaoxVector2D_Normalize( & right );
	DaoxVector2D delta = DaoxVector2D_Scale( & norm, displacement );
	DaoxLine line;
	line.start = DaoxVector2D_Add( & start, & delta );
	line.end = DaoxVector2D_Add( & end, & delta );
	return line;
}
DaoxPathSegment DaoxPathSegment_LinearStroke( DaoxPathSegment *self, float displacement )
{
	DaoxPathSegment segment = *self;
	DaoxVector2D forward = DaoxVector2D_Sub( & self->P2, & self->P1 );
	DaoxVector2D right = DaoxMatrix3D_RotateVector( forward, -0.5*M_PI );
	DaoxVector2D norm = DaoxVector2D_Normalize( & right );
	DaoxVector2D delta = DaoxVector2D_Scale( & norm, displacement );
	segment.P1 = DaoxVector2D_Add( & self->P1, & delta );
	segment.P2 = DaoxVector2D_Add( & self->P2, & delta );
	return segment;
}
DaoxPathSegment DaoxPathSegment_QuadraticStroke( DaoxPathSegment *self, float displacement )
{
	DaoxPathSegment segment = *self;
	DaoxVector2D P1 = self->P1;
	DaoxVector2D P2 = self->P2;
	DaoxVector2D C1 = self->C1;
	DaoxLine Q01, Q12;

	if( DaoxVector2D_Dist( P1, C1 ) < EPSILON || DaoxVector2D_Dist( C1, P2 ) < EPSILON ){
		C1 = Interpolate( P1, P2, 0.5 );
	}

	Q01 = DaoxLine_Copy( P1, C1, displacement );
	Q12 = DaoxLine_Copy( C1, P2, displacement );
	segment.P1 = Q01.start;
	segment.P2 = Q12.end;
	segment.C1 = DaoxLine_Intersect2( & Q01, & Q12 );
	segment.C2 = segment.C1;
	return segment;
}
DaoxPathSegment DaoxPathSegment_CubicStroke( DaoxPathSegment *self, float displacement )
{
	DaoxPathSegment segment = *self;
	DaoxVector2D P1 = self->P1;
	DaoxVector2D P2 = self->P2;
	DaoxVector2D C1 = self->C1;
	DaoxVector2D C2 = self->C2;
	DaoxLine Q01, Q12, Q23;

	if( DaoxVector2D_Dist( P1, C1 ) < EPSILON ) C1 = Interpolate( P1, P2, 0.2 );
	if( DaoxVector2D_Dist( C2, P2 ) < EPSILON ) C2 = Interpolate( P1, P2, 0.8 );
	Q01 = DaoxLine_Copy( P1, C1, displacement );
	Q12 = DaoxLine_Copy( C1, C2, displacement );
	Q23 = DaoxLine_Copy( C2, P2, displacement );
	segment.P1 = Q01.start;
	segment.P2 = Q23.end;
	segment.C1 = DaoxLine_Intersect2( & Q01, & Q12 );
	segment.C2 = DaoxLine_Intersect2( & Q23, & Q12 );
	//printf( "DaoxPathSegment_CubicStroke: %f\n", DaoxPathSegment_Length( self ) );
	return segment;
}
void DaoxPathMesh_AddSubStroke( DaoxPathMesh *strokes, DaoxPathSegment *seg, double start, double end, float width, int newStart )
{
	DaoxPathSegment left = *seg, right = *seg;
	DaoxVector3D *point1, *point2;
	float width2 = 0.5 * width;
	int P1, P2, P3, P4, C1, C2;
	int added = 0;

	if( seg->bezier == 0 ) return;
	if( strokes->points->size < 2 || newStart ){
		DaoxPlainArray_Reserve( strokes->points, strokes->points->size + 2 );
		point1 = (DaoxVector3D*) DaoxPlainArray_Push( strokes->points );
		point2 = (DaoxVector3D*) DaoxPlainArray_Push( strokes->points );
		added = 1;
	}
	P1 = strokes->points->size - 2;
	P2 = strokes->points->size - 1;
	P3 = strokes->points->size + seg->bezier - 1;
	P4 = strokes->points->size + seg->bezier;
	if( seg->bezier == 1 ){
		left = DaoxPathSegment_LinearStroke( seg, - width2 );
		right = DaoxPathSegment_LinearStroke( seg, width2 );

		if( added ){
			point1->x = left.P1.x;   point1->y = left.P1.y;   point1->z = start;
			point2->x = right.P1.x;  point2->y = right.P1.y;  point2->z = start;
		}
		DaoxPlainArray_PushTriangleIJK( strokes->triangles, P1, P2, P3 );
		DaoxPlainArray_PushTriangleIJK( strokes->triangles, P4, P3, P2 );
	}else if( seg->bezier == 2 ){
		double L01 = DaoxVector2D_Dist( seg->P1, seg->C1 );
		double L12 = DaoxVector2D_Dist( seg->C1, seg->P2 );
		double at = L01 / (L01 + L12 + EPSILON);
		int side = DaoxLine_CheckPoint( seg->P1, seg->P2, seg->C1 );

		left = DaoxPathSegment_QuadraticStroke( seg, - width2 );
		right = DaoxPathSegment_QuadraticStroke( seg, width2 );
		left.convexness = - side;
		right.convexness = side;

		DaoxPathMesh_HandleSegment( strokes, & left, start, end );
		DaoxPathMesh_HandleSegment( strokes, & right, start, end );

		if( added ){
			point1->x = left.P1.x;   point1->y = left.P1.y;   point1->z = start;
			point2->x = right.P1.x;  point2->y = right.P1.y;  point2->z = start;
		}

		C1 = strokes->points->size;
		DaoxPlainArray_PushTriangleIJK( strokes->triangles, P1, P2, C1 );
		DaoxPlainArray_PushTriangleIJK( strokes->triangles, P4, P3, C1 );

		point1 = (DaoxVector3D*) DaoxPlainArray_Push( strokes->points );
		point1->z = (1 - at) * start + at * end;
		if( side > 0 ){
			point1->x = left.C1.x;
			point1->y = left.C1.y;
			DaoxPlainArray_PushTriangleIJK( strokes->triangles, P2, P4, C1 );
		}else{
			point1->x = right.C1.x;
			point1->y = right.C1.y;
			DaoxPlainArray_PushTriangleIJK( strokes->triangles, P3, P1, C1 );
		}
	}else if( seg->bezier == 3 ){
		double L01 = DaoxVector2D_Dist( seg->P1, seg->C1 );
		double L12 = DaoxVector2D_Dist( seg->C1, seg->C2 );
		double L23 = DaoxVector2D_Dist( seg->C2, seg->P2 );
		double at1 = L01 / (L01 + L12 + L23 + EPSILON);
		double at2 = (L01 + L12) / (L01 + L12 + L23 + EPSILON);
		int side = DaoxLine_CheckPoint( seg->P1, seg->P2, seg->C1 );

		if( side == 0 ) side = DaoxLine_CheckPoint( seg->P1, seg->P2, seg->C2 );

		left = DaoxPathSegment_CubicStroke( seg, - width2 );
		right = DaoxPathSegment_CubicStroke( seg, width2 );
		left.convexness = - side;
		right.convexness = side;

		DaoxPathMesh_HandleSegment( strokes, & left, start, end );
		DaoxPathMesh_HandleSegment( strokes, & right, start, end );

		if( added ){
			point1->x = left.P1.x;   point1->y = left.P1.y;   point1->z = start;
			point2->x = right.P1.x;  point2->y = right.P1.y;  point2->z = start;
		}

		C1 = strokes->points->size;
		C2 = strokes->points->size + 1;
		DaoxPlainArray_PushTriangleIJK( strokes->triangles, P1, P2, C1 );
		DaoxPlainArray_PushTriangleIJK( strokes->triangles, P4, P3, C2 );

		DaoxPlainArray_Reserve( strokes->points, strokes->points->size + 2 );
		point1 = (DaoxVector3D*) DaoxPlainArray_Push( strokes->points );
		point2 = (DaoxVector3D*) DaoxPlainArray_Push( strokes->points );
		point1->z = (1 - at1) * start + at1 * end;
		point2->z = (1 - at2) * start + at2 * end;
		if( side > 0 ){
			point1->x = left.C1.x;
			point1->y = left.C1.y;
			point2->x = left.C2.x;
			point2->y = left.C2.y;
			DaoxPlainArray_PushTriangleIJK( strokes->triangles, P2, P4, C2 );
			DaoxPlainArray_PushTriangleIJK( strokes->triangles, C2, C1, P2 );
		}else{
			point1->x = right.C1.x;
			point1->y = right.C1.y;
			point2->x = right.C2.x;
			point2->y = right.C2.y;
			DaoxPlainArray_PushTriangleIJK( strokes->triangles, P3, P1, C2 );
			DaoxPlainArray_PushTriangleIJK( strokes->triangles, C1, C2, P1 );
		}
	}
	DaoxPlainArray_Reserve( strokes->points, strokes->points->size + 2 );
	point1 = (DaoxVector3D*) DaoxPlainArray_Push( strokes->points );
	point2 = (DaoxVector3D*) DaoxPlainArray_Push( strokes->points );
	point1->x = left.P2.x;   point1->y = left.P2.y;   point1->z = end;
	point2->x = right.P2.x;  point2->y = right.P2.y;  point2->z = end;
}
double DaoxPathMesh_AddStroke( DaoxPathMesh *strokes, DaoxPathSegment *seg, double offset, float width, int refine, DaoxPlainArray *segments )
{
	DaoxPathSegment *S;
	double maxlen = DaoxPathSegment_MaxLength( seg );
	double minlen = DaoxVector2D_Dist( seg->P1, seg->P2 );
	double maxdiff = 0.25;
	int newStart = 1;

	if( seg->bezier == 3 && maxlen > 1.0 ){
		float S = 0, T = 0;
		DaoxLine_Intersect( seg->P1, seg->P2, seg->C1, seg->C2, & S, & T );
		if( (S >= 0 && S <= 1) || (T >= 0 && T <= 1) ){
			DaoxPathSegment A, B, C = *seg;
			C.first = & A;
			C.second = & B;
			DaoxPathSegment_Divide( & C, 0.5 );
			offset = DaoxPathMesh_AddStroke( strokes, & A, offset, width, refine, segments );
			offset = DaoxPathMesh_AddStroke( strokes, & B, offset, width, refine, segments );
			return offset;
		}
	}

	if( refine ) maxlen = 2.0;
	if( (maxlen - minlen) > width ) maxdiff = width / maxlen;
	
	segments->size = 0;
	DaoxPathSegment_SubSegments( seg, segments, maxlen, maxdiff );
	S = (DaoxPathSegment*) DaoxPlainArray_Get( segments, 0 );
	while( S ){
		double max = DaoxPathSegment_MaxLength( S );
		double min = DaoxVector2D_Dist( S->P1, S->P2 );
		double len = 0.5 * (max + min);
		DaoxPathMesh_AddSubStroke( strokes, S, offset, offset + len, width, newStart );
		S = S->next ? DaoxPlainArray_Get( segments, (daoint) S->next ) : NULL;
		offset += len;
		newStart = 0;
	}
	return offset;
}
void DaoxPath_ComputeStroke( DaoxPath *self, DaoxPathMesh *strokes, float width, int refine )
{
	DaoxPlainArray *segments = DaoxPlainArray_New( sizeof(DaoxPathSegment) );
	DaoxPathComponent *com;
	double offset = 0;

	DaoxPathMesh_Reset( strokes );
	for(com=self->first; com; com=com->next){
		DaoxPathSegment *seg;
		if( com->first->bezier == 0 ) continue;
		seg = com->refined.first;
		do {
			offset = DaoxPathMesh_AddStroke( strokes, seg, offset, width, refine, segments );
			seg = seg->next;
		} while( seg && seg != com->refined.first );
	}
	DaoxPlainArray_Delete( segments );
	printf( "DaoxPath_ComputeStroke: %i\n", strokes->points->size );
}
