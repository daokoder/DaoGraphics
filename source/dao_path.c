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
#include <assert.h>
#include "dao_path.h"
#include "dao_canvas.h"
#include "daoStdtype.h"



DaoxVectorD4 DaoxVectorD4_XYZW( double x, double y, double z, double w )
{
	DaoxVectorD4 res;
	res.x = x;
	res.y = y;
	res.z = z;
	res.w = w;
	return res;
}

DaoxMatrixD3X3 DaoxMatrixD3X3_InitRows( DaoxVectorD3 V1, DaoxVectorD3 V2, DaoxVectorD3 V3 )
{
	DaoxMatrixD3X3 res;
	res.V.V[0] = V1;
	res.V.V[1] = V2;
	res.V.V[2] = V3;
	return res;
}
DaoxMatrixD3X3 DaoxMatrixD3X3_InitColumns( DaoxVectorD3 V1, DaoxVectorD3 V2, DaoxVectorD3 V3 )
{
	DaoxMatrixD3X3 res;
	res.A.A11 = V1.x;  res.A.A12 = V2.x;  res.A.A13 = V3.x;
	res.A.A21 = V1.y;  res.A.A22 = V2.y;  res.A.A23 = V3.y;
	res.A.A31 = V2.z;  res.A.A32 = V2.z;  res.A.A33 = V3.z;
	return res;
}
double DaoxMatrixD3X3_Determinant( DaoxMatrixD3X3 *self )
{
	double det1 = self->A.A21 * self->A.A32 - self->A.A31 * self->A.A22;
	double det2 = self->A.A21 * self->A.A33 - self->A.A31 * self->A.A23;
	double det3 = self->A.A22 * self->A.A33 - self->A.A32 * self->A.A23;
	return self->A.A11 * det3 - self->A.A12 * det2 + self->A.A13 * det1;
}



DaoxMatrixD4X4 DaoxMatrixD4X4_InitRows( DaoxVectorD4 V1, DaoxVectorD4 V2, DaoxVectorD4 V3, DaoxVectorD4 V4 )
{
	DaoxMatrixD4X4 res;
	res.V.V[0] = V1;
	res.V.V[1] = V2;
	res.V.V[2] = V3;
	res.V.V[3] = V4;
	return res;
}
DaoxMatrixD4X4 DaoxMatrixD4X4_InitColumns( DaoxVectorD4 V1, DaoxVectorD4 V2, DaoxVectorD4 V3, DaoxVectorD4 V4 )
{
	DaoxMatrixD4X4 res;
	res.A.A11 = V1.x;  res.A.A12 = V2.x;  res.A.A13 = V3.x;  res.A.A14 = V4.x;
	res.A.A21 = V1.y;  res.A.A22 = V2.y;  res.A.A23 = V3.y;  res.A.A24 = V4.y;
	res.A.A31 = V1.z;  res.A.A32 = V2.z;  res.A.A33 = V3.z;  res.A.A34 = V4.z;
	res.A.A41 = V1.w;  res.A.A42 = V2.w;  res.A.A43 = V3.w;  res.A.A44 = V4.w;
	return res;
}
DaoxMatrixD4X4 DaoxMatrixD4X4_MulMatrix( DaoxMatrixD4X4 *self, DaoxMatrixD4X4 *other )
{
	DaoxMatrixD4X4 res;
	double (*A)[4] = self->M;
	double (*B)[4] = other->M;
	double (*C)[4] = res.M;
	int i, j, k;
	for(i=0; i<4; ++i){
		for(j=0; j<4; ++j){
			double sum = 0.0;
			for(k=0; k<4; ++k) sum += A[i][k] * B[k][j];
			C[i][j] = sum;
		}
	}
	return res;
}
DaoxVectorD4 DaoxMatrixD4X4_MulVector( DaoxMatrixD4X4 *self, DaoxVectorD4 *vector )
{
	DaoxVectorD4 res;
	double (*A)[4] = self->M;
	double *B = & vector->x;
	double *C = & res.x;
	int i, k;
	for(i=0; i<4; ++i){
		double sum = 0.0;
		for(k=0; k<4; ++k) sum += A[i][k] * B[k];
		C[i] = sum;
	}
	return res;
}


typedef struct DaoxLine      DaoxLine;

struct DaoxLine
{
	DaoxVector2D  start;
	DaoxVector2D  end;
};

int DaoxLine_Intersect( DaoxVector2D A, DaoxVector2D B, DaoxVector2D C, DaoxVector2D D, float *S, float *T )
{
	float BxAx = B.x - A.x;
	float ByAy = B.y - A.y;
	float CxAx = C.x - A.x;
	float CyAy = C.y - A.y;
	float DxCx = D.x - C.x;
	float DyCy = D.y - C.y;
	float K = BxAx * DyCy - ByAy * DxCx;

	if( K == 0.0 ) return 0;

	*S = (CxAx * DyCy - CyAy * DxCx) / K;
	*T = (CxAx * ByAy - CyAy * BxAx) / K;

	if( *S < 0 || *S > 1.0 ) return 0;
	if( *T < 0 || *T > 1.0 ) return 0;

	return 1;
}






DaoxPathSegment* DaoxPathSegment_New( DaoxPathComponent *component )
{
	DaoxPathSegment* self = (DaoxPathSegment*) dao_calloc(1,sizeof(DaoxPathSegment));
	self->convexness = 1; /* need for refinement of open path; */
	return self;
}
void DaoxPathSegment_Delete( DaoxPathSegment *self )
{
	if( self->first ) DaoxPathSegment_Delete( self->first );
	if( self->second ) DaoxPathSegment_Delete( self->second );
	dao_free( self );
}
void DaoxPathSegment_Copy( DaoxPathSegment *self, DaoxPathSegment *other )
{
	self->bezier = other->bezier;
	self->subStart = other->subStart;
	self->subEnd = other->subEnd;
	memmove( & self->P1, & other->P1, 4*sizeof(DaoxVector2D) );
}

DaoxPathComponent* DaoxPathComponent_New( DaoxPath *path )
{
	DaoxPathComponent* self = (DaoxPathComponent*) dao_calloc(1,sizeof(DaoxPathComponent));
	self->first = self->last = DaoxPathSegment_New( self );
	self->first->subStart = 1;
	self->first->subEnd = 1;
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
		segment->subStart = 1;
		segment->subEnd = 1;
	}else{
		segment = DaoxPathSegment_New( self );
		segment->subStart = 1;
		segment->subEnd = 1;
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
	self->refinedFirst = self->refinedLast = NULL;
	DaoxPathComponent_PushSegment( self ); // XXX free:
}
void DaoxPathComponent_Copy( DaoxPathComponent *self, DaoxPathComponent *other )
{
	DaoxPathSegment *segment = other->first;
	DaoxPathComponent_Reset( self );
	do {
		DaoxPathSegment *segment2 = DaoxPathComponent_PushSegment( self );
		DaoxPathSegment_Copy( segment2, segment );
		segment = segment->next;
	} while( segment && segment != other->first );
	if( other->last->next == other->first ) self->last->next = self->first;
}
void DaoxPathComponent_Delete( DaoxPathComponent *self )
{
	DaoxPathComponent_Reset( self );
	dao_free( self );
}






DaoxPath* DaoxPath_New()
{
	DaoxPath *self = (DaoxPath*) dao_calloc(1,sizeof(DaoxPath));
	DaoCstruct_Init( (DaoCstruct*)self, daox_type_path );
	self->first = self->last = DaoxPathComponent_New( self );
	return self;
}
void DaoxPath_Delete( DaoxPath *self )
{
	DNode *it;
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
	dao_free( self );
}
DaoxPathComponent* DaoxPath_PushComponent( DaoxPath *self )
{
	DaoxPathComponent *com = NULL;
	self->hashed = 0;
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
	self->hashed = 0;
	for(com=self->first; com; com=com->next){
		DaoxPathComponent_Reset( com );
		com->next = self->freeComponents;
		self->freeComponents = com;
	}
	self->mode = DAOX_PATH_CMD_ABS;
	self->first = self->last = NULL;
	DaoxPath_PushComponent( self );
}
void DaoxPath_Copy( DaoxPath *self, DaoxPath *other )
{
	DaoxPathComponent *com;
	DaoxPath_Reset( self );
	self->mode = other->mode;
	self->hash = other->hash;
	self->hashed = other->hashed;
	for(com=other->first; com; com=com->next){
		DaoxPathComponent *com2 = DaoxPath_PushComponent( self );
		DaoxPathComponent_Copy( com2, com );
	}
}
void DaoxPath_SetRelativeMode( DaoxPath *self, int relative )
{
	self->mode = relative ? DAOX_PATH_CMD_REL : DAOX_PATH_CMD_ABS;
}
void DaoxPath_MoveTo( DaoxPath *self, float x, float y )
{
	DaoxPathComponent *com;
	self->hashed = 0;
	if( self->last->last->bezier == 0 ){
		self->last->last->P1.x = x;
		self->last->last->P1.y = y;
		return;
	}
	com = DaoxPath_PushComponent( self );
	com->last->P1.x = x;
	com->last->P1.y = y;
}
/* If MoveTo() is not called, line from (0,0). */
void DaoxPath_LineTo( DaoxPath *self, float x, float y )
{
	DaoxVector2D start;
	DaoxPathSegment *segment = NULL;
	self->hashed = 0;
	if( self->last->last->bezier == 0 ){
		start = self->last->last->P1;
		segment = self->last->last;
	}else{
		start = self->last->last->P2;
		segment = DaoxPathComponent_PushSegment( self->last );
		segment->P1 = start;
	}
	if( self->mode == DAOX_PATH_CMD_REL ){
		x += start.x;
		y += start.y;
	}
	segment->bezier = 1;
	segment->P2.x = x;
	segment->P2.y = y;
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
	DaoxPathSegment *segment = NULL, *first = NULL;
	double degrees2 = M_PI * degrees / 180.0;
	double dx, dy, R, dA, sine, cosine, dL, dR;
	int i, K;

	self->hashed = 0;
	if( self->last->last->bezier == 0 ){
		start = self->last->last->P1;
		segment = self->last->last;
	}else{
		start = self->last->last->P2;
		segment = DaoxPathComponent_PushSegment( self->last );
		segment->P1 = start;
	}
	first = segment;
	if( self->mode == DAOX_PATH_CMD_ABS ){
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
	first->subStart = 1;
	first->subEnd = 1;
	if( self->last->last != first ){
		self->last->last->subStart = 0;
		self->last->last->subEnd = 1;
		first->subEnd = 0;
	}
	segment = first->next;
	while( segment && segment != self->last->last ){
		segment->subStart = 0;
		segment->subEnd = 0;
		segment = segment->next;
	}
}
void DaoxPath_ArcTo2( DaoxPath *self, float x, float y, float degrees, float deg2 )
{
	DaoxVector2D start;
	double degrees2 = M_PI * degrees / 180.0;
	double cx, cy, dx, dy, R, dR, dL;

	self->hashed = 0;
	if( self->last->last->bezier == 0 ){
		start = self->last->last->P1;
	}else{
		start = self->last->last->P2;
	}
	if( self->mode == DAOX_PATH_CMD_ABS ){
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
	if( self->mode == DAOX_PATH_CMD_ABS ){
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
#define DAOX_PATH_QUAD_TO_CUBIC 1
void DaoxPath_QuadTo( DaoxPath *self, float cx, float cy, float x, float y )
{
	DaoxPathSegment *segment = NULL;
	DaoxVector2D start = {0.0,0.0};

	self->hashed = 0;
	if( self->last->last->bezier == 0 ){
		start = self->last->last->P1;
		segment = self->last->last;
	}else{
		start = self->last->last->P2;
		segment = DaoxPathComponent_PushSegment( self->last );
		segment->P1 = start;
	}
	if( self->mode == DAOX_PATH_CMD_REL ){
		cx += start.x;
		cy += start.y;
		x += start.x;
		y += start.y;
	}
#ifndef DAOX_PATH_QUAD_TO_CUBIC
	segment->bezier = 2;
	segment->C1.x = cx;
	segment->C1.y = cy;
	segment->P2.x = x;
	segment->P2.y = y;
	segment->C2 = segment->C1;
#else
	segment->bezier = 3;
	segment->P2.x = x;
	segment->P2.y = y;
	segment->C1 = DaoxVector2D_Interpolate( segment->P1, DaoxVector2D_XY( cx, cy ), 2.0/3.0 );
	segment->C2 = DaoxVector2D_Interpolate( segment->P2, DaoxVector2D_XY( cx, cy ), 2.0/3.0 );
#endif
}
void DaoxPath_CubicTo( DaoxPath *self, float cx, float cy, float x, float y )
{
	DaoxPathSegment *segment = NULL;
	DaoxVector2D control = self->last->last->C2;
	DaoxVector2D start = self->last->last->P2;

	self->hashed = 0;
	assert( self->last->last->bezier >= 2 );
	control.x = 2.0 * start.x - control.x;
	control.y = 2.0 * start.y - control.y;
	if( self->mode == DAOX_PATH_CMD_REL ){
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
}
void DaoxPath_CubicTo2( DaoxPath *self, float cx1, float cy1, float cx2, float cy2, float x2, float y2 )
{
	DaoxPathSegment *segment = NULL;
	DaoxVector2D start = {0.0,0.0};

	self->hashed = 0;
	if( self->last->last->bezier == 0 ){
		start = self->last->last->P1;
		segment = self->last->last;
	}else{
		start = self->last->last->P2;
		segment = DaoxPathComponent_PushSegment( self->last );
		segment->P1 = start;
	}
	if( self->mode == DAOX_PATH_CMD_REL ){
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
}
void DaoxPath_Close( DaoxPath *self )
{
	DaoxPathSegment *last = self->last->last;
	DaoxPathSegment *segment = NULL;
	self->hashed = 0;
	if( self->last->first == self->last->last ){
		if( self->last->first->bezier == 0 ) return;  /* no component data; */
	}
	if( DaoxVector2D_Dist( self->last->last->P2, self->last->first->P1 ) < 1E-16 ){
		self->last->last->next = self->last->first;
		return;
	}
	segment = DaoxPathComponent_PushSegment( self->last );
	segment->bezier = 1;
	segment->P1 = last->P2;
	segment->P2 = self->last->first->P1;
	segment->next = self->last->first;
}






DaoxVector2D DaoxLine_Intersect2( DaoxLine *self, DaoxLine *other )
{
	float S = 0, T = 0;
	int i = DaoxLine_Intersect( self->start, self->end, other->start, other->end, & S, & T );
	return DaoxVector2D_Interpolate( self->start, self->end, S );
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
	self->first->subStart = self->subStart;
	self->first->subEnd = 0;
	self->second->subStart = 0;
	self->second->subEnd = self->subEnd;

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
	self->first->C2 = self->first->C1;
	self->second->C2 = self->second->C1;
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
void DaoxPathSegment_SubSegments( DaoxPathSegment *self, DArray *segments, float maxlen, float maxdiff )
{
	DaoxPathSegment *segment;
	int i, start = segments->size;

	if( start ){
		DaoxPathSegment *segment = DArray_Get( segments, start-1 );
		segment->next = (DaoxPathSegment*) (daoint) start;
	}

	segment = (DaoxPathSegment*) DArray_Push( segments );
	*segment = *self;
	segment->first = segment->second = segment->next = NULL;
	for(i=start; i<segments->size; ){
		DaoxPathSegment segment1 = {0};
		DaoxPathSegment segment2 = {0};
		DaoxPathSegment *segment = DArray_Get( segments, i );
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

		seg = (DaoxPathSegment*) DArray_Push( segments );
		segment = DArray_Get( segments, i );
		*segment = segment1;
		*seg = segment2;
		segment->next = (DaoxPathSegment*) (daoint)(segments->size - 1);
		seg->next = next;
	}
}
double DaoxPathSegment_Length( DaoxPathSegment *self, float factor )
{
	DaoxPathSegment *res;
	DaoxPathSegment segment = *self;
	DaoxPathSegment segment1 = {0};
	DaoxPathSegment segment2 = {0};
	double minlen = DaoxVector2D_Dist( self->P1, self->P2 );
	double maxlen = DaoxPathSegment_MaxLength( self );
	double len;

	if( factor < 0.0 ) return 0.0;
	if( maxlen < 1E-6 ||  maxlen < 1.001*minlen ){
		if( factor <= 1.0 ) return factor * maxlen;
		return maxlen;
	}

	segment1.next = segment2.next = NULL;
	segment.first = & segment1;
	segment.second = & segment2;
	DaoxPathSegment_Divide( & segment, 0.5 );

	len = DaoxPathSegment_Length( & segment1, 2.0*factor );
	if( factor > 0.5 ) len += DaoxPathSegment_Length( & segment2, 2.0*factor - 1.0 );
	return len;
}


void DaoxPathComponent_Refine( DaoxPathComponent *self, float maxlen, float maxdiff )
{
	DaoxPathSegment *first = self->refinedFirst ? self->refinedFirst : self->first;
	DaoxPathSegment *segment = first;
	do {
		DaoxPathSegment_Refine( segment, maxlen, maxdiff );
		if( segment->refined == 0 ) DaoxPathSegment_Divide( segment, 0.5 );
		segment = segment->next;
	} while( segment && segment != first );
}



double DaoxPathSegment_LocateByDistance( DaoxPathSegment *self, float dist, DaoxVector3D *pos )
{
	DaoxPathSegment *res;
	DaoxPathSegment segment = *self;
	DaoxPathSegment segment1 = {0};
	DaoxPathSegment segment2 = {0};
	double minlen = DaoxVector2D_Dist( self->P1, self->P2 );
	double maxlen = DaoxPathSegment_MaxLength( self );
	double len;

	pos->x = self->P2.x; /* in case for drawing text allow a path; */
	pos->y = self->P2.y;
	pos->z = -1.0; /* marking for not found; */
	if( dist < -EPSILON ) return 0.0;
	if( maxlen < 1E-6 ||  maxlen < 1.001*minlen ){
		if( dist <= maxlen ){
			double factor = dist / (minlen + EPSILON);
			DaoxVector2D mid = DaoxVector2D_Interpolate( self->P1, self->P2, factor );
			pos->x = mid.x;
			pos->y = mid.y;
			pos->z = factor;
			return dist;
		}
		return maxlen;
	}

	segment1.next = segment2.next = NULL;
	segment.first = & segment1;
	segment.second = & segment2;
	DaoxPathSegment_Divide( & segment, 0.5 );

	len = DaoxPathSegment_LocateByDistance( & segment1, dist, pos );
	if( pos->z > -EPSILON ){
		pos->z = 0.5 * pos->z;
		return len;
	}
	len = len + DaoxPathSegment_LocateByDistance( & segment2, dist - len, pos );
	if( pos->z > -EPSILON ){
		pos->z = 0.5 + 0.5 * pos->z;
		return len;
	}
	return len;
}
DaoxPathSegment* DaoxPath_LocateByDistance( DaoxPath *self, float dist, DaoxVector3D *pos )
{
	DaoxPathComponent *com;
	double len, offset = 0.0;

	for(com=self->first; com; com=com->next){
		DaoxPathSegment *first = com->first;
		DaoxPathSegment *segment = first;
		if( com->first->bezier == 0 ) continue;
		do {
			len = DaoxPathSegment_LocateByDistance( segment, dist - offset, pos );
			if( pos->z >= -EPSILON ) return segment; /* found; */
			offset += len;
			segment = segment->next;
		} while( segment && segment != first );
	}
	return NULL;
}
DaoxPathSegment DaoxPath_LocateByPercentage( DaoxPath *self, float percentage, float *p )
{
	DaoxPathSegment seg = {0};
	return seg;
}


DaoxOBBox2D DaoxPathSegment_GetOBBox( DaoxPathSegment *self )
{
	DaoxOBBox2D  obbox;
	DaoxVector2D points[4];

	points[0] = self->P1;
	points[1] = self->P2;
	if( self->bezier >= 2 ) points[2] = self->C1;
	if( self->bezier >= 3 ) points[3] = self->C2;

	DaoxOBBox2D_ResetBox( & obbox, points, self->bezier + 1 );
	return obbox;
}



void DaoxPathComponent_RetrieveSegment( DaoxPathComponent *self, DaoxPathSegment *segment )
{
	int loop = self->last->next == self->first;
	if( segment->refined == 0 ){
		segment->next = NULL;
		if( self->refinedFirst == NULL ){
			self->refinedFirst = self->refinedLast = segment;
		}else{
			self->refinedLast->next = segment;
			self->refinedLast = segment;
			if( loop ) segment->next = self->refinedFirst;
		}
	}else if( segment->refined ){
		DaoxPathComponent_RetrieveSegment( self, segment->first );
		DaoxPathComponent_RetrieveSegment( self, segment->second );
	}
}
void DaoxPathComponent_RetrieveRefined( DaoxPathComponent *self )
{
	DaoxPathSegment *segment = self->first;
	self->refinedFirst = self->refinedLast = NULL;
	do {
		DaoxPathComponent_RetrieveSegment( self, segment );
		segment = segment->next;
	} while( segment && segment != self->first );
	if( self->last->next == self->first ) self->refinedLast->next = self->refinedFirst;
}
int DaoxPathSegment_CheckConvexness2( DaoxPathSegment *self, DaoxVector2D point )
{
	double area1, area2, epsilon = 0.0;
	DaoxVector2D C = self->C1;
	if( self->bezier == 1 ) return 0;
	if( self->bezier == 3 ) C = DaoxVector2D_Interpolate( self->C1, self->C2, 0.5 );
	area1 = DaoxTriangle_Area( self->P1, self->P2, point );
	area2 = DaoxTriangle_Area( self->P1, self->P2, C );
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
	//printf( "DaoxPath_Refine: %15f %15f %i\n", maxlen, maxdiff, self->first->first->bezier );
	for(com=self->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		DaoxPathComponent_Refine( com, maxlen, maxdiff );
	}
}





void DaoxPathStyle_Init( DaoxPathStyle *self )
{
	memset( self, 0, sizeof(DaoxPathStyle) );
	self->junction = DAOX_JUNCTION_FLAT;
	self->cap = DAOX_LINECAP_NONE;
	self->width = 1.0;
}
void DaoxPathStyle_SetDashes( DaoxPathStyle *self, int count, float lens[] )
{
	if( count > sizeof(self->dashes) ) count = sizeof(self->dashes);
	memcpy( self->dashes, lens, count*sizeof(float) );
	self->dash = count; 
}



DaoxPathMesh* DaoxPathMesh_New()
{
	DaoxPathMesh *self = (DaoxPathMesh*) dao_calloc(1,sizeof(DaoxPathMesh));
	DaoCstruct_Init( (DaoCstruct*)self, daox_type_path_mesh );
	DaoxPathStyle_Init( & self->strokeStyle );
	self->fillPoints    = DArray_New( sizeof(DaoxVector3D) );
	self->fillTriangles = DArray_New( sizeof(DaoxTriangle) );
	self->fillBeziers   = DArray_New( sizeof(DaoxBezierPoint) );
	self->strokePoints    = DArray_New( sizeof(DaoxVector3D) );
	self->strokeTriangles = DArray_New( sizeof(DaoxTriangle) );
	self->strokeBeziers   = DArray_New( sizeof(DaoxBezierPoint) );
	self->workPoints    = self->fillPoints;
	self->workTriangles = self->fillTriangles;
	self->workBeziers   = self->fillBeziers;
	self->path = DaoxPath_New();
	GC_IncRC( self->path );
	return self;
}
void DaoxPathMesh_Delete( DaoxPathMesh *self )
{
	DaoCstruct_Free( (DaoCstruct*) self );
	if( self->path ) GC_DecRC( self->path );
	DArray_Delete( self->fillPoints );
	DArray_Delete( self->fillTriangles );
	DArray_Delete( self->fillBeziers );
	DArray_Delete( self->strokePoints );
	DArray_Delete( self->strokeTriangles );
	DArray_Delete( self->strokeBeziers );
	dao_free( self );
}
void DaoxPathMesh_Reset( DaoxPathMesh *self, DaoxPath *path, DaoxPathStyle *style )
{
	if( style ) self->strokeStyle = *style;
	if( path ) GC_Assign( & self->path, path );
	self->fillPoints->size = 0;
	self->fillTriangles->size = 0;
	self->fillBeziers->size = 0;
	self->strokePoints->size = 0;
	self->strokeTriangles->size = 0;
	self->strokeBeziers->size = 0;
	self->workPoints    = self->fillPoints;
	self->workTriangles = self->fillTriangles;
	self->workBeziers   = self->fillBeziers;
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
void DaoxPathSegment_TryDivideLoop( DaoxPathSegment *self, int index, DList *segments )
{
	DaoxVectorD4 dets = DaoxPathSegment_ComputeDeterminantVector( self );
	double d1 = dets.y;
	double d2 = dets.z;
	double d3 = dets.w;
	double delta = 3*d2*d2 - 4*d1*d3;
	double discr = d1*d1*delta;
	double epsilon = 1E-3;

	if( DaoxPathSegment_MaxLength( self ) < 1E-2 ) return;
	if( discr > -epsilon ) return; /* Not a loop cubic bezier; */

	DaoxPathSegment_Divide( self, 0.5 );
	DList_Append( segments, self->first );
	DList_Append( segments, self->second );
	segments->items.pVoid[index] = NULL;
}
void DaoxPathMesh_HandleSegment( DaoxPathMesh *self, DaoxPathSegment *segment, double start, double end )
{
	DaoxPathSegment orientedSegment = *segment;
	DaoxMatrixD4X4 M3INV, F, M3F;
	DaoxBezierPoint *P0, *P1, *P2, *P3, *P4, *P5;
	DaoxVectorD4 M3I[4] = { {1,0,0,0}, {1,1./3.,0,0}, {1,2./3.,1./3.,0}, {1,1,1,1} };
	DaoxVectorD4 dets;
	DaoxVector2D SP2, SC1, norm;
	double d0, d1, d2, d3, delta, dot;
	double epsilon = 0.01;
	float len1, len2, len3;
	float at1 = 0.5;
	float at2 = 0.5;
	int i;

	if( segment->bezier < 2 ) return;

	SC1 = segment->C1;
	if( segment->bezier == 3 ) SC1 = DaoxVector2D_Interpolate( SC1, segment->C2, 0.5 );
	SP2 = DaoxVector2D_Sub( & segment->P2, & segment->P1 );
	SC1 = DaoxVector2D_Sub( & SC1, & segment->P1 );
	norm = DaoxMatrix3D_RotateVector( SP2, -0.5*M_PI );
	dot = DaoxVector2D_Dot( & SC1, & norm );
	if( (dot * segment->convexness) > 0 ){
		double tmp = start;
		/* Reverse the segment, so that the right side is inside: */
		start = end;
		end = tmp;
		orientedSegment.P1 = segment->P2;
		orientedSegment.P2 = segment->P1;
		if( segment->bezier == 3 ){
			orientedSegment.C1 = segment->C2;
			orientedSegment.C2 = segment->C1;
		}
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

	len1 = DaoxVector2D_Dist( segment->P1, segment->C1 );
	if( segment->bezier == 2 ){
		len2 = DaoxVector2D_Dist( segment->P2, segment->C1 );
		at1 = len1 / (len1 + len2 + EPSILON);
		DArray_Reserve( self->workBeziers, self->workBeziers->size + 3 );
		P0 = (DaoxBezierPoint*) DArray_Push( self->workBeziers );
		P1 = (DaoxBezierPoint*) DArray_Push( self->workBeziers );
		P2 = (DaoxBezierPoint*) DArray_Push( self->workBeziers );
		P0->pos = segment->P1;
		P1->pos = segment->P2;
		P2->pos = segment->C1;
		P0->klm.x = P0->klm.y = P0->klm.z = 0.0;
		P1->klm.x = P1->klm.y = P1->klm.z = 1.0;
		P2->klm.x = P2->klm.z = 0.5;  P2->klm.y = 0.0;
		P0->offset = start;
		P1->offset = end;
		P2->offset = (1.0 - at1) * start + at1 * end;
		if( d3 < 0.0 ){
			P0->klm.x = - P0->klm.x;  P0->klm.y = - P0->klm.y;
			P1->klm.x = - P1->klm.x;  P1->klm.y = - P1->klm.y;
			P2->klm.x = - P2->klm.x;  P2->klm.y = - P2->klm.y;
		}
		return;
	}

	if( fabs(d1) > epsilon && delta >= 0.0 ){ /* Serpentine, or Cusp with inflection at infinity: */
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
	}else if( fabs(d1) > epsilon && delta < 0.0 ){ /* Loop: */
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
	}else if( fabs(d1) <= epsilon && fabs(d2) > epsilon ){ /* Cusp with cusp at infinity */
		double tl = d3, sl = 3*d2;
		F.A.A11 = tl;
		F.A.A12 = tl*tl*tl;
		F.A.A13 = 1.0;
		F.A.A21 = -sl;
		F.A.A22 = -3*sl*tl*tl;
		F.A.A32 = 3*sl*sl*tl;
		F.A.A42 = -sl*sl*sl;
	}else if( fabs(d1) <= epsilon && fabs(d2) <= epsilon && d3 != 0.0 ){ /* quadratic */
	}else{ /* d1 == d2 == d3 == 0.0, line */
		return;
	}
	len2 = DaoxVector2D_Dist( segment->C1, segment->C2 );
	len3 = DaoxVector2D_Dist( segment->C2, segment->P2 );
	at1 = len1 / (len1 + len2 + len3 + EPSILON);
	at2 = (len1 + len2) / (len1 + len2 + len3 + EPSILON);

	/* Reserve first, so that DArray_Push will return 6 valid pointers: */
	DArray_Reserve( self->workBeziers, self->workBeziers->size + 6 );
	P0 = (DaoxBezierPoint*) DArray_Push( self->workBeziers );
	P3 = (DaoxBezierPoint*) DArray_Push( self->workBeziers );
	P2 = (DaoxBezierPoint*) DArray_Push( self->workBeziers );
	P4 = (DaoxBezierPoint*) DArray_Push( self->workBeziers );
	P5 = (DaoxBezierPoint*) DArray_Push( self->workBeziers );
	P1 = (DaoxBezierPoint*) DArray_Push( self->workBeziers );
	P0->pos = segment->P1;
	P1->pos = segment->C1;
	P2->pos = segment->C2;
	P3->pos = segment->P2;
	P0->offset = start;
	P1->offset = (1.0 - at1) * start + at1 * end;
	P2->offset = (1.0 - at2) * start + at2 * end;
	P3->offset = end;

	M3F = DaoxMatrixD4X4_MulMatrix( & M3INV, & F );
	P0->klm.x = M3F.A.A11;  P0->klm.y = M3F.A.A12;  P0->klm.z = M3F.A.A13;
	P1->klm.x = M3F.A.A21;  P1->klm.y = M3F.A.A22;  P1->klm.z = M3F.A.A23;
	P2->klm.x = M3F.A.A31;  P2->klm.y = M3F.A.A32;  P2->klm.z = M3F.A.A33;
	P3->klm.x = M3F.A.A41;  P3->klm.y = M3F.A.A42;  P3->klm.z = M3F.A.A43;
	if( fabs(d1) > epsilon && delta >= 0.0 ){ /* serpentine curve: */
		if( d1 < 0.0 ){
			P0->klm.x = - P0->klm.x;  P0->klm.y = - P0->klm.y;
			P1->klm.x = - P1->klm.x;  P1->klm.y = - P1->klm.y;
			P2->klm.x = - P2->klm.x;  P2->klm.y = - P2->klm.y;
			P3->klm.x = - P3->klm.x;  P3->klm.y = - P3->klm.y;
		}
	}else if( fabs(d1) > epsilon && delta < 0.0 ){ /* Loop: */
		double H0 = d3*d1 - d2*d2; /* H(s,t)/(36*s^2) for t/s=0; */
		double H1 = (d3*d1 - d2*d2) + d1*d2 - d1*d1; /* H(s,t)/(36*s^2) for t/s=1; */
		double H = fabs( H0 ) > fabs( H1 ) ? H0 : H1;
		if( d1*H > 0.0 ){
			P0->klm.x = - P0->klm.x;  P0->klm.y = - P0->klm.y;
			P1->klm.x = - P1->klm.x;  P1->klm.y = - P1->klm.y;
			P2->klm.x = - P2->klm.x;  P2->klm.y = - P2->klm.y;
			P3->klm.x = - P3->klm.x;  P3->klm.y = - P3->klm.y;
		}
	}else if( fabs(d1) <= epsilon && fabs(d2) <= epsilon && d3 != 0.0 ){ /* quadratic */
		P0->klm.x = P0->klm.y = P0->klm.z = 0.0;
		P1->klm.x = P1->klm.z = 1.0/3.0; P1->klm.y = 0.0;
		P2->klm.x = P2->klm.z = 2.0/3.0; P2->klm.y = 1.0/3.0;
		P3->klm.x = P3->klm.y = P3->klm.z = 1.0;
		if( d3 < 0.0 ){
			P0->klm.x = - P0->klm.x;  P0->klm.y = - P0->klm.y;
			P1->klm.x = - P1->klm.x;  P1->klm.y = - P1->klm.y;
			P2->klm.x = - P2->klm.x;  P2->klm.y = - P2->klm.y;
			P3->klm.x = - P3->klm.x;  P3->klm.y = - P3->klm.y;
		}
	}
	*P4 = *P0;
	*P5 = *P2;
}

void DaoxPathMesh_Preprocess( DaoxPathMesh *self, DaoxTriangulator *triangulator )
{
	DList *segments;
	DaoxVector3D *point, *point2;
	DArray *boxes;
	DArray *points;
	DaoxPathComponent *com;
	DaoxPathSegment *seg, *first;
	daoint i, j, count = 0;
	float len, maxlen = 100.0;
	float maxdiff = 0.1;

	if( self->path == NULL ) return;
	if( self->fillPoints->size ) return;

	self->path->length = 0;
	DaoxPathMesh_Reset( self, NULL, NULL );
	DaoxOBBox2D_ResetBox( & self->path->obbox, NULL, 0 );

	if( self->path->first->first->bezier == 0 ) return;

	self->workPoints    = self->fillPoints;
	self->workTriangles = self->fillTriangles;
	self->workBeziers   = self->fillBeziers;

	segments = DList_New(0);
	boxes = DArray_New( sizeof(DaoxOBBox2D) );

	DaoxTriangulator_Reset( triangulator );
	DaoxPath_Refine( self->path, maxlen, maxdiff );
	for(com=self->path->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		DaoxPathComponent_RetrieveRefined( com );

		seg = first = com->refinedFirst ? com->refinedFirst : com->first;
		do {
			/* for stroke gradient; */
			self->path->length += DaoxPathSegment_Length( seg, 1.0 );
			seg = seg->next;
		} while( seg && seg != first );

		/* To skip open path components: */
		if( com->refinedLast == NULL || com->refinedLast->next == NULL ) continue;
		seg = com->refinedFirst;
		do {
			DList_PushBack( segments, seg );
			seg = seg->next;
		} while( seg && seg != com->refinedFirst );
	}
	//printf( "Before intersection refinement: %i\n", (int)segments->size );
	for(i=0; i<segments->size; ++i){
		DaoxPathSegment *S1 = (DaoxPathSegment*) segments->items.pVoid[i];
		DaoxOBBox2D *box = DArray_Push( boxes );
		*box = DaoxPathSegment_GetOBBox( S1 );
		if( S1 == NULL || S1->bezier != 3 ) continue;
		DaoxPathSegment_TryDivideLoop( S1, i, segments );
	}
	//printf( "Before intersection refinement: %i\n", (int)segments->size );
	for(i=0; i<segments->size; ++i){
		DaoxPathSegment *S1 = (DaoxPathSegment*) segments->items.pVoid[i];
		DaoxOBBox2D *B1 = (DaoxOBBox2D*) DArray_Get( boxes, i );
		double len1;
		if( S1 == NULL || S1->bezier == 0 ) continue;
		if( (len1 = DaoxPathSegment_MaxLength( S1 )) < 1.0 ) continue;
		for(j=i+1; j<segments->size; ++j){
			DaoxPathSegment *S2 = (DaoxPathSegment*) segments->items.pVoid[j];
			DaoxOBBox2D *B2 = (DaoxOBBox2D*) DArray_Get( boxes, j );
			double len2;
			if( S2 == NULL || S2->bezier == 0 ) continue;
			if( (len2 = DaoxPathSegment_MaxLength( S2 )) < 1.0 ) continue;
			if( DaoxVector2D_Dist( S1->P1, S2->P2 ) < EPSILON ) continue;
			if( DaoxVector2D_Dist( S1->P2, S2->P1 ) < EPSILON ) continue;
			if( DaoxOBBox2D_Intersect( B1, B2 ) >= 0 ){
				DaoxOBBox2D *box1, *box2;
				float R1 = 0.5, R2 = 0.5;
				if( len1 > len2 ){
					DaoxPathSegment_Divide( S1, R1 );
					DList_Append( segments, S1->first );
					DList_Append( segments, S1->second );
					box1 = DArray_Push( boxes );
					box2 = DArray_Push( boxes );
					*box1 = DaoxPathSegment_GetOBBox( S1->first );
					*box2 = DaoxPathSegment_GetOBBox( S1->second );
					segments->items.pVoid[i] = NULL;
					break;
				}else if( len2 > len1 ){
					DaoxPathSegment_Divide( S2, R2 );
					DList_Append( segments, S2->first );
					DList_Append( segments, S2->second );
					box1 = DArray_Push( boxes );
					box2 = DArray_Push( boxes );
					*box1 = DaoxPathSegment_GetOBBox( S2->first );
					*box2 = DaoxPathSegment_GetOBBox( S2->second );
					segments->items.pVoid[j] = NULL;
				}
			}
		}
		if( segments->items.pVoid[i] == NULL ) continue;
	}

#if 0
	printf( "After intersection refinement: %i\n", (int)segments->size );
	fflush( stdout );
#endif

	points = DArray_New( sizeof(DaoxVector2D) );
	for(com=self->path->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		seg = com->refinedFirst;
		do {
			DArray_PushVector2D( points, & seg->P1 );
			if( seg->next == NULL ) DArray_PushVector2D( points, & seg->P2 );
			if( com->first->bezier >= 2 ){
				DArray_PushVector2D( points, & seg->C1 );
				if( com->first->bezier == 3 ) DArray_PushVector2D( points, & seg->C2 );
			}
			seg = seg->next;
		} while( seg && seg != com->refinedFirst );
	}
	DaoxOBBox2D_ResetBox( & self->path->obbox, points->data.vectors2d, points->size );

	DArray_Delete( boxes );
	if( self->strokeStyle.fill == 0 ){
		DList_Delete( segments );
		return;
	}

	segments->size = 0;
	for(com=self->path->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		DaoxPathComponent_RetrieveRefined( com );
		if( com->refinedLast == NULL || com->refinedLast->next == NULL ) continue;
		seg = com->refinedFirst;
		do {
			DaoxTriangulator_PushPoint( triangulator, seg->P1.x, seg->P1.y );
			DList_PushBack( segments, seg );
			seg = seg->next;
		} while( seg && seg != com->refinedFirst );
		DaoxTriangulator_CloseContour( triangulator );
	}
	//printf( "DaoxPath_Segment 2: %i\n", (int) triangulator->vertices->size );
	DaoxTriangulator_Triangulate( triangulator );
	/* Check local convexness with triangulation to handle possible presence of holes: */
	for(i=0; i<triangulator->triangles->size; ++i){
		DaoxTriangle *triangle = triangulator->triangles->data.triangles + i;
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
	for(com=self->path->first; com; com=com->next){
		if( com->first->bezier == 0 ) continue;
		if( com->refinedLast == NULL || com->refinedLast->next == NULL ) continue;
		seg = com->refinedFirst;
		do {
			if( seg->bezier >= 2 ) DaoxPathMesh_HandleSegment( self, seg, 0.0, 0.0 );
			DaoxTriangulator_PushPoint( triangulator, seg->P1.x, seg->P1.y );
			if( seg->convexness < 0 && seg->bezier >= 2 ){
				/* Push control points for locally concave bezier curves: */
				DaoxTriangulator_PushPoint( triangulator, seg->C1.x, seg->C1.y );
				if( seg->bezier == 3 ){
					DaoxTriangulator_PushPoint( triangulator, seg->C2.x, seg->C2.y );
				}
			}
			seg = seg->next;
		} while( seg && seg != com->refinedFirst );
		DaoxTriangulator_CloseContour( triangulator );
	}
	DaoxTriangulator_Triangulate( triangulator );
	for(i=0; i<triangulator->points->size; i+=1){
		DaoxVector2D *pt = triangulator->points->data.vectors2d + i;
		point = DArray_PushVector3D( self->fillPoints, NULL );
		point->x = pt->x;
		point->y = pt->y;
		point->z = 0.0;
	}
	for(i=0; i<triangulator->triangles->size; i+=1){
		DaoxTriangle *triangle = triangulator->triangles->data.triangles + i;
		DaoxTriangle *triangle2 = DArray_PushTriangle( self->fillTriangles, NULL );
		int I = triangle->index[0], J = triangle->index[1], K = triangle->index[2];
		DaoxVector2D P1 = triangulator->points->data.vectors2d[I];
		DaoxVector2D P2 = triangulator->points->data.vectors2d[J];
		DaoxVector2D P3 = triangulator->points->data.vectors2d[K];
		if( DaoxTriangle_Area( P1, P2, P3 ) > 0.0 ){
			*triangle2 = *triangle;
		}else{
			triangle2->index[0] = I;
			triangle2->index[1] = K;
			triangle2->index[2] = J;
		}
	}
	//printf( "DaoxPath_Preprocess: %i %i\n", self->fillPoints->size, self->workBeziers->size );
	//self->workBeziers->size = 0;

	DArray_Delete( points );
	DList_Delete( segments );
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
static DaoxVector2D DaoxLine_MeanNorm( DaoxVector2D A, DaoxVector2D B, DaoxVector2D C )
{
	DaoxVector2D BA = DaoxVector2D_Sub( & B, & A );
	DaoxVector2D CB = DaoxVector2D_Sub( & C, & B );
	DaoxVector2D NAB = DaoxMatrix3D_RotateVector( BA, -0.5*M_PI );
	DaoxVector2D NBC = DaoxMatrix3D_RotateVector( CB, -0.5*M_PI );
	DaoxVector2D NABC = DaoxVector2D_Add( & NAB, & NBC );
	return NABC;
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
	DaoxLine Q01, Q12, NC1;
	DaoxVector2D C1X;

	if( DaoxVector2D_Dist( P1, C1 ) < EPSILON || DaoxVector2D_Dist( C1, P2 ) < EPSILON ){
		C1 = DaoxVector2D_Interpolate( P1, P2, 0.5 );
	}

	Q01 = DaoxLine_Copy( P1, C1, displacement );
	Q12 = DaoxLine_Copy( C1, P2, displacement );
	NC1.start = C1;
	NC1.end = DaoxLine_MeanNorm( P1, C1, P2 );
	NC1.end = DaoxVector2D_Add( & NC1.start, & NC1.end );
	C1  = DaoxLine_Intersect2( & Q01, & NC1 );
	C1X = DaoxLine_Intersect2( & Q12, & NC1 );
	segment.P1 = Q01.start;
	segment.P2 = Q12.end;
	segment.C1 = DaoxVector2D_Interpolate( C1, C1X, 0.5 );
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
	DaoxLine Q01, Q12, Q23, NC1, NC2;
	DaoxVector2D C1X, C2X;

	//printf( "DaoxPathSegment_CubicStroke: %f\n", DaoxPathSegment_Length( self ) );
	if( DaoxVector2D_Dist( P1, C1 ) < EPSILON ) C1 = DaoxVector2D_Interpolate( P1, P2, 0.2 );
	if( DaoxVector2D_Dist( C2, P2 ) < EPSILON ) C2 = DaoxVector2D_Interpolate( P1, P2, 0.8 );
	Q01 = DaoxLine_Copy( P1, C1, displacement );
	Q12 = DaoxLine_Copy( C1, C2, displacement );
	Q23 = DaoxLine_Copy( C2, P2, displacement );
	NC1.start = C1;
	NC2.start = C2;
	NC1.end = DaoxLine_MeanNorm( P1, C1, C2 );
	NC2.end = DaoxLine_MeanNorm( C1, C2, P2 );
	NC1.end = DaoxVector2D_Add( & NC1.start, & NC1.end );
	NC2.end = DaoxVector2D_Add( & NC2.start, & NC2.end );
	C1  = DaoxLine_Intersect2( & Q01, & NC1 );
	C1X = DaoxLine_Intersect2( & Q12, & NC1 );
	C2  = DaoxLine_Intersect2( & Q23, & NC2 );
	C2X = DaoxLine_Intersect2( & Q12, & NC2 );
	segment.P1 = Q01.start;
	segment.P2 = Q23.end;
	segment.C1 = DaoxVector2D_Interpolate( C1, C1X, 0.5 );
	segment.C2 = DaoxVector2D_Interpolate( C2, C2X, 0.5 );
	//printf( "DaoxPathSegment_CubicStroke: %f\n", DaoxPathSegment_Length( self ) );
	return segment;
}
void DaoxPathMesh_AddSubStroke( DaoxPathMesh *self, DaoxPathSegment *seg, double start, double end, int newStart )
{
	DaoxPathSegment left = *seg, right = *seg;
	DaoxVector3D *point1 = NULL, *point2 = NULL;
	float width2 = 0.5 * self->strokeStyle.width;
	int P1, P2, P3, P4, C1, C2;
	int added = 0;

	if( seg->bezier == 0 ) return;
	if( self->strokePoints->size < 2 || newStart ){
		DArray_Reserve( self->strokePoints, self->strokePoints->size + 2 );
		point1 = (DaoxVector3D*) DArray_Push( self->strokePoints );
		point2 = (DaoxVector3D*) DArray_Push( self->strokePoints );
		added = 1;
	}
	P1 = self->strokePoints->size - 2;
	P2 = self->strokePoints->size - 1;
	P3 = self->strokePoints->size + seg->bezier - 1;
	P4 = self->strokePoints->size + seg->bezier;
	if( seg->bezier == 1 ){
		left = DaoxPathSegment_LinearStroke( seg, - width2 );
		right = DaoxPathSegment_LinearStroke( seg, width2 );

		if( added ){
			point1->x = left.P1.x;   point1->y = left.P1.y;   point1->z = start;
			point2->x = right.P1.x;  point2->y = right.P1.y;  point2->z = start;
		}
		DArray_PushTriangleIJK( self->strokeTriangles, P1, P2, P3 );
		DArray_PushTriangleIJK( self->strokeTriangles, P4, P3, P2 );
	}else if( seg->bezier == 2 ){
		double L01 = DaoxVector2D_Dist( seg->P1, seg->C1 );
		double L12 = DaoxVector2D_Dist( seg->C1, seg->P2 );
		double at = L01 / (L01 + L12 + EPSILON);
		int side = DaoxLine_CheckPoint( seg->P1, seg->P2, seg->C1 );

		left = DaoxPathSegment_QuadraticStroke( seg, - width2 );
		right = DaoxPathSegment_QuadraticStroke( seg, width2 );
		left.convexness = - side;
		right.convexness = side;

		DaoxPathMesh_HandleSegment( self, & left, start, end );
		DaoxPathMesh_HandleSegment( self, & right, start, end );

		if( added ){
			point1->x = left.P1.x;   point1->y = left.P1.y;   point1->z = start;
			point2->x = right.P1.x;  point2->y = right.P1.y;  point2->z = start;
		}

		C1 = self->strokePoints->size;
		DArray_PushTriangleIJK( self->strokeTriangles, P1, P2, C1 );
		DArray_PushTriangleIJK( self->strokeTriangles, P4, P3, C1 );

		point1 = (DaoxVector3D*) DArray_Push( self->strokePoints );
		point1->z = (1 - at) * start + at * end;
		if( side > 0 ){
			point1->x = left.C1.x;
			point1->y = left.C1.y;
			DArray_PushTriangleIJK( self->strokeTriangles, P2, P4, C1 );
		}else{
			point1->x = right.C1.x;
			point1->y = right.C1.y;
			DArray_PushTriangleIJK( self->strokeTriangles, P3, P1, C1 );
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

		DaoxPathMesh_HandleSegment( self, & left, start, end );
		DaoxPathMesh_HandleSegment( self, & right, start, end );

		if( added ){
			point1->x = left.P1.x;   point1->y = left.P1.y;   point1->z = start;
			point2->x = right.P1.x;  point2->y = right.P1.y;  point2->z = start;
		}

		C1 = self->strokePoints->size;
		C2 = self->strokePoints->size + 1;
		DArray_PushTriangleIJK( self->strokeTriangles, P1, P2, C1 );
		DArray_PushTriangleIJK( self->strokeTriangles, P4, P3, C2 );

		DArray_Reserve( self->strokePoints, self->strokePoints->size + 2 );
		point1 = (DaoxVector3D*) DArray_Push( self->strokePoints );
		point2 = (DaoxVector3D*) DArray_Push( self->strokePoints );
		point1->z = (1 - at1) * start + at1 * end;
		point2->z = (1 - at2) * start + at2 * end;
		if( side > 0 ){
			point1->x = left.C1.x;
			point1->y = left.C1.y;
			point2->x = left.C2.x;
			point2->y = left.C2.y;
			DArray_PushTriangleIJK( self->strokeTriangles, P2, P4, C2 );
			DArray_PushTriangleIJK( self->strokeTriangles, C2, C1, P2 );
		}else{
			point1->x = right.C1.x;
			point1->y = right.C1.y;
			point2->x = right.C2.x;
			point2->y = right.C2.y;
			DArray_PushTriangleIJK( self->strokeTriangles, P3, P1, C2 );
			DArray_PushTriangleIJK( self->strokeTriangles, C1, C2, P1 );
		}
	}
	DArray_Reserve( self->strokePoints, self->strokePoints->size + 2 );
	point1 = (DaoxVector3D*) DArray_Push( self->strokePoints );
	point2 = (DaoxVector3D*) DArray_Push( self->strokePoints );
	point1->x = left.P2.x;   point1->y = left.P2.y;   point1->z = end;
	point2->x = right.P2.x;  point2->y = right.P2.y;  point2->z = end;
}
double DaoxPathMesh_AddStroke( DaoxPathMesh *self, DaoxPathSegment *seg, double offset, DArray *segments )
{
	DaoxPathSegment *S;
	float width = self->strokeStyle.width;
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
			offset = DaoxPathMesh_AddStroke( self, & A, offset, segments );
			offset = DaoxPathMesh_AddStroke( self, & B, offset, segments );
			return offset;
		}
	}

	if( (maxlen - minlen) > 0.1*width ) maxdiff = 0.1*width / maxlen;

	segments->size = 0;
	DaoxPathSegment_SubSegments( seg, segments, maxlen + 1E-6, maxdiff + 1E-6 );
	S = (DaoxPathSegment*) DArray_Get( segments, 0 );
	while( S ){
		double max = DaoxPathSegment_MaxLength( S );
		double min = DaoxVector2D_Dist( S->P1, S->P2 );
		double len = 0.5 * (max + min);
		DaoxPathMesh_AddSubStroke( self, S, offset, offset + len, newStart );
		S = S->next ? DArray_Get( segments, (daoint) S->next ) : NULL;
		offset += len;
		newStart = 0;
	}
	return offset;
}
DaoxLine DaoxPathSegment_GetStartTangent( DaoxPathSegment *self )
{
	DaoxLine line;
	line.start = self->P1;
	line.end = self->P2;
	if( self->bezier >= 2 ) line.end = self->C1;
	return line;
}
DaoxLine DaoxPathSegment_GetEndTangent( DaoxPathSegment *self )
{
	DaoxLine line;
	line.start = self->P1;
	line.end = self->P2;
	switch( self->bezier ){
	case 2 : line.start = self->C1; break;
	case 3 : line.start = self->C2; break;
	}
	return line;
}
void DaoxPathMesh_PushQuad( DaoxPathMesh *self, DaoxVector2D A, DaoxVector2D B, DaoxVector2D C, DaoxVector2D D, float offset )
{
	int IA = self->strokePoints->size;
	int IB = IA + 1, IC = IA + 2, ID = IA + 3;
	DaoxVector3D *P1, *P2, *P3, *P4;

	DArray_Reserve( self->strokePoints, self->strokePoints->size + 4 );
	P1 = (DaoxVector3D*) DArray_Push( self->strokePoints );
	P2 = (DaoxVector3D*) DArray_Push( self->strokePoints );
	P3 = (DaoxVector3D*) DArray_Push( self->strokePoints );
	P4 = (DaoxVector3D*) DArray_Push( self->strokePoints );
	P1->x = A.x;  P1->y = A.y;  P1->z = offset;
	P2->x = B.x;  P2->y = B.y;  P2->z = offset;
	P3->x = C.x;  P3->y = C.y;  P3->z = offset;
	P4->x = D.x;  P4->y = D.y;  P4->z = offset;
	DArray_PushTriangleIJK( self->strokeTriangles, IA, IB, IC );
	DArray_PushTriangleIJK( self->strokeTriangles, IA, IC, ID );
}
void DaoxPathMesh_AddJunction( DaoxPathMesh *self, DaoxPathSegment *seg, float offset )
{
	float S = 0, T = 0;
	float w2 = 0.5 * self->strokeStyle.width;
	DaoxVector3D zero3 = {0.0, 0.0, 0.0};
	DaoxBezierPoint *P1, *P2, *P3, *P4, *P5, *P6;
	DaoxLine first = DaoxPathSegment_GetEndTangent( seg );
	DaoxLine second = DaoxPathSegment_GetStartTangent( seg->next );
	DaoxLine L1 = DaoxLine_Copy( first.start, first.end, -w2 );
	DaoxLine R1 = DaoxLine_Copy( first.start, first.end, w2 );
	DaoxLine L2 = DaoxLine_Copy( second.start, second.end, -w2 );
	DaoxLine R2 = DaoxLine_Copy( second.start, second.end, w2 );
	DaoxPathSegment round = {0};
	float angle = acos( DaoxTriangle_AngleCosine( first.end, L1.end, L2.start ) );

	DArray_Reserve( self->strokeBeziers, self->strokeBeziers->size + 6 );
	P1 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
	P2 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
	P3 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
	P1->klm = P2->klm = P3->klm = zero3;
	P1->offset = P2->offset = P3->offset = offset;
	P1->pos = seg->P2;

	if( self->strokeStyle.junction == DAOX_JUNCTION_SHARP ){
		P4 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
		P5 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
		P6 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
		P4->klm = P5->klm = P6->klm = zero3;
		P4->offset = P5->offset = P6->offset = offset;
	}
	DaoxLine_Intersect( L1.start, L1.end, L2.start, L2.end, & S, & T );
	if( S <= 1.0 && T >= 0.0 ){
		float dx = R2.start.x - R1.end.x;
		float dy = R2.start.y - R1.end.y;
		P2->pos = R1.end;
		P3->pos = R2.start;
		round.P1 = R1.end;
		if( self->strokeStyle.junction == DAOX_JUNCTION_SHARP ){
			P4->pos = DaoxLine_Intersect2( & R1, & R2 );
			P5->pos = P3->pos;
			P6->pos = P2->pos;
		}else if( self->strokeStyle.junction == DAOX_JUNCTION_ROUND ){
			DaoxPathSegment_MakeArc( & round, dx, dy, w2, angle );
		}
	}else{
		float dx = L1.end.x - L2.start.x;
		float dy = L1.end.y - L2.start.y;
		P2->pos = L2.start;
		P3->pos = L1.end;
		round.P1 = L2.start;
		if( self->strokeStyle.junction == DAOX_JUNCTION_SHARP ){
			P4->pos = DaoxLine_Intersect2( & L1, & L2 );
			P5->pos = P3->pos;
			P6->pos = P2->pos;
		}else if( self->strokeStyle.junction == DAOX_JUNCTION_ROUND ){
			DaoxPathSegment_MakeArc( & round, dx, dy, w2, angle );
		}
	}
	round.convexness = 1;
	DaoxPathMesh_HandleSegment( self, & round, offset, offset );
	//printf( ">>>>>>>>>>>>>>>>>>>>>>> %f %f\n", S, T );
}
void DaoxPathMesh_AddCap( DaoxPathMesh *self, DaoxPathComponent *com, float offset, float offset2 )
{
	float cap = self->strokeStyle.cap;
	float w2 = 0.5 * self->strokeStyle.width;
	DaoxVector3D zero3 = {0.0, 0.0, 0.0};
	DaoxBezierPoint *P1, *P2, *P3, *P4, *P5, *P6;
	DaoxVector2D H1, H2, T1, T2;
	DaoxLine head, tail;
	DaoxLine first = DaoxPathSegment_GetStartTangent( com->first );
	DaoxLine second = DaoxPathSegment_GetEndTangent( com->last );
	DaoxLine R1 = DaoxLine_Copy( first.start, first.end, w2 );
	DaoxLine L2 = DaoxLine_Copy( second.start, second.end, -w2 );
	float dx1 = 2.0*(first.start.x - R1.start.x);
	float dy1 = 2.0*(first.start.y - R1.start.y);
	float dx2 = 2.0*(second.end.x - L2.end.x);
	float dy2 = 2.0*(second.end.y - L2.end.y);

	H1 = R1.start;
	T1 = L2.end;
	H2.x = H1.x + dx1;
	H2.y = H1.y + dy1;
	T2.x = T1.x + dx2;
	T2.y = T1.y + dy2;

	if( cap == DAOX_LINECAP_FLAT ){
		head = DaoxLine_Copy( H1, H2, -self->strokeStyle.width );
		tail = DaoxLine_Copy( T1, T2, -self->strokeStyle.width );
		DaoxPathMesh_PushQuad( self, H1, H2, head.end, head.start, offset );
		DaoxPathMesh_PushQuad( self, T1, T2, tail.end, tail.start, offset2 );
	}else if( cap == DAOX_LINECAP_SHARP ){
		head = DaoxLine_Copy( H1, H2, -w2 );
		tail = DaoxLine_Copy( T1, T2, -w2 );
		DArray_Reserve( self->strokeBeziers, self->strokeBeziers->size + 6 );
		P1 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
		P2 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
		P3 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
		P4 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
		P5 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
		P6 = (DaoxBezierPoint*) DArray_Push( self->strokeBeziers );
		P1->klm = P2->klm = P3->klm = zero3;
		P4->klm = P5->klm = P6->klm = zero3;
		P1->offset = P2->offset = P3->offset = offset;
		P4->offset = P5->offset = P6->offset = offset2;
		P1->pos = H1;  P2->pos = H2;
		P3->pos.x = 0.5*(head.start.x + head.end.x);
		P3->pos.y = 0.5*(head.start.y + head.end.y);
		P4->pos = T1;  P5->pos = T2;
		P6->pos.x = 0.5*(tail.start.x + tail.end.x);
		P6->pos.y = 0.5*(tail.start.y + tail.end.y);
	}else if( cap == DAOX_LINECAP_ROUND ){
		DaoxPathSegment round = {0};
		round.convexness = 1;
		round.P1 = R1.start;
		DaoxPathSegment_MakeArc( & round, dx1, dy1, w2, -M_PI );
		DaoxPathMesh_HandleSegment( self, & round, offset, offset );
		round.P1 = L2.end;
		DaoxPathSegment_MakeArc( & round, dx2, dy2, w2, -M_PI );
		DaoxPathMesh_HandleSegment( self, & round, offset2, offset2 );
	}
}
void DaoxPathMesh_ComputeStroke( DaoxPathMesh *self )
{
	DArray *segments = DArray_New( sizeof(DaoxPathSegment) );
	DaoxPathComponent *com;
	float width = self->strokeStyle.width;
	float junction = self->strokeStyle.junction;
	float cap = self->strokeStyle.cap;
	double offset = 0;

	self->workPoints    = self->strokePoints;
	self->workTriangles = self->strokeTriangles;
	self->workBeziers   = self->strokeBeziers;
	for(com=self->path->first; com; com=com->next){
		DaoxPathSegment *seg;
		double offset2 = offset;
		float w2 = 0.5 * width;
		if( com->first->bezier == 0 ) continue;
		seg = com->refinedFirst;
		do {
			offset = DaoxPathMesh_AddStroke( self, seg, offset, segments );
			//printf( "%5i %5i %f\n", seg->bezier, seg->subEnd, offset );
			if( junction && seg->subEnd && seg->next != NULL ){
				DaoxPathMesh_AddJunction( self, seg, offset );
			}
			seg = seg->next;
		} while( seg && seg != com->refinedFirst );
		if( cap && com->last->next != com->first ){
			DaoxPathMesh_AddCap( self, com, offset2, offset );
		}
	}
	DArray_Delete( segments );
	//printf( "DaoxPathMesh_ComputeStroke: %i\n", self->strokePoints->size );
}




DaoxPathCache* DaoxPathCache_New()
{
	DaoxPath *path = DaoxPath_New();
	DaoxPathCache *self = (DaoxPathCache*) dao_calloc( 1, sizeof(DaoxPathCache) );
	DaoCstruct_Init( (DaoCstruct*)self, daox_type_path_cache );

	self->paths = DHash_New(0,DAO_DATA_LIST);
	self->meshes = DHash_New(0,DAO_DATA_LIST);
	self->triangulator = DaoxTriangulator_New();
	GC_IncRC( path );

	DaoxPath_Reset( path );
	DaoxPath_MoveTo( path, 0, 0 );
	DaoxPath_LineTo( path, DAOX_PATH_UNIT, 0 );
	self->unitLine = DaoxPathCache_FindPath( self, path );

	DaoxPath_Reset( path );
	DaoxPath_MoveTo( path, 0, 0 );
	DaoxPath_LineTo( path, DAOX_PATH_UNIT, 0 );
	DaoxPath_LineTo( path, DAOX_PATH_UNIT, DAOX_PATH_UNIT );
	DaoxPath_LineTo( path, 0, DAOX_PATH_UNIT );
	DaoxPath_Close( path );
	self->unitRect = DaoxPathCache_FindPath( self, path );

	DaoxPath_Reset( path );
	DaoxPath_MoveTo( path, -DAOX_PATH_UNIT, 0 );
	DaoxPath_ArcTo2( path,  DAOX_PATH_UNIT, 0, 180, 180 );
	DaoxPath_ArcTo2( path, -DAOX_PATH_UNIT, 0, 180, 180 );
	DaoxPath_Close( path );
	self->unitCircle1 = DaoxPathCache_FindPath( self, path );

	DaoxPath_Reset( path );
	DaoxPath_MoveTo( path, -DAOX_PATH_UNIT, 0 );
	DaoxPath_ArcTo2( path,  DAOX_PATH_UNIT, 0, 180, 60 );
	DaoxPath_ArcTo2( path, -DAOX_PATH_UNIT, 0, 180, 60 );
	DaoxPath_Close( path );
	self->unitCircle2 = DaoxPathCache_FindPath( self, path );

	DaoxPath_Reset( path );
	DaoxPath_MoveTo( path, -DAOX_PATH_UNIT, 0 );
	DaoxPath_ArcTo2( path,  DAOX_PATH_UNIT, 0, 180, 20 );
	DaoxPath_ArcTo2( path, -DAOX_PATH_UNIT, 0, 180, 20 );
	DaoxPath_Close( path );
	self->unitCircle3 = DaoxPathCache_FindPath( self, path );
	GC_DecRC( path );
	return self;
}
void DaoxPathCache_Delete( DaoxPathCache *self )
{
	DaoCstruct_Free( (DaoCstruct*) self );
	DaoxTriangulator_Delete( self->triangulator );
	DMap_Delete( self->paths );
	DMap_Delete( self->meshes );
	dao_free( self );
}

void DaoxPathSegment_Convert( DaoxPathSegment *self, int buffer[9] )
{
	float *values = & self->P1.x;
	int i;

	buffer[0] = self->bezier;
	for(i=0; i<8; ++i) buffer[i+1] = DAOX_RESOLUTION * values[i];
}
uint_t DaoxPathSegment_Hash( DaoxPathSegment *self, uint_t hash )
{
	int buffer[9];
	DaoxPathSegment_Convert( self, buffer );
	return Dao_Hash( buffer, 9*sizeof(int), hash );
}
uint_t DaoxPath_Hash( DaoxPath *self )
{
	DaoxPathComponent *com;
	uint_t hash = 0;

	if( self->hashed ) return self->hash;

	for(com=self->first; com; com=com->next){
		DaoxPathSegment *first = com->first;
		DaoxPathSegment *segment = first;
		if( com->first->bezier == 0 ) continue;
		do {
			hash = DaoxPathSegment_Hash( segment, hash );
			segment = segment->next;
		} while( segment && segment != first );
	}
	self->hash = hash;
	self->hashed = 1;
	return hash;
}
int DaoxPathSegment_Compare( DaoxPathSegment *self, DaoxPathSegment *other )
{
	int first[9];
	int second[9];
	int i;

	if( self->bezier != other->bezier ) return self->bezier < other->bezier ? -1 : 1;
	if( self->bezier == 0 ) return 0;
	DaoxPathSegment_Convert( self, first );
	DaoxPathSegment_Convert( other, second );
	for(i=1; i<9; ++i){
		if( first[i] != second[i] ) return first[i] < second[i] ? -1 : 1;
	}
	return 0;
}
int DaoxPathComponent_Compare( DaoxPathComponent *self, DaoxPathComponent *other )
{
	DaoxPathSegment *first1 = self->first;
	DaoxPathSegment *first2 = other->first;
	DaoxPathSegment *seg1 = first1;
	DaoxPathSegment *seg2 = first2;
	if( seg1 != NULL && seg2 != NULL ){
		do {
			int cmp = DaoxPathSegment_Compare( seg1, seg2 );
			if( cmp ) return cmp;
			seg1 = seg1->next;
			seg2 = seg2->next;
		} while( seg1 && seg1 != first1 && seg2 && seg2 != first2 );
	}
	if( seg1 == NULL && seg2 != NULL ) return -1;
	if( seg1 != NULL && seg2 == NULL ) return  1;
	return 0;
}
int DaoxPath_Compare( DaoxPath *self, DaoxPath *other )
{
	DaoxPathComponent *com1 = self->first;
	DaoxPathComponent *com2 = other->first;
	for(; com1 && com2; com1=com1->next, com2=com2->next){
		int cmp = DaoxPathComponent_Compare( com1, com2 );
		if( cmp ) return cmp;
	}
	if( com1 == NULL && com2 != NULL ) return -1;
	if( com1 != NULL && com2 == NULL ) return  1;
	return 0;
}
void DaoxPathStyle_Convert( DaoxPathStyle *self, int buffer[DAOX_MAX_DASH+5] )
{
	int i;
	buffer[0] = self->fill;
	buffer[1] = self->cap;
	buffer[2] = self->dash;
	buffer[3] = self->junction;
	buffer[4] = DAOX_RESOLUTION * self->width;
	for(i=0; i<self->dash; ++i){
		buffer[5+i] = DAOX_RESOLUTION * self->dashes[i];
	}
}
uint_t DaoxPathStyle_Hash( DaoxPathStyle *self, uint_t hash )
{
	int buffer[DAOX_MAX_DASH+5];
	DaoxPathStyle_Convert( self, buffer );
	return Dao_Hash( buffer, (5+self->dash)*sizeof(int), hash );
}
int DaoxPathStyle_Compare( DaoxPathStyle *self, DaoxPathStyle *other )
{
	int i, first[DAOX_MAX_DASH+5];
	int second[DAOX_MAX_DASH+5];
	if( self->dash != other->dash ) return self->dash < other->dash ? -1 : 1;
	DaoxPathStyle_Convert( self, first );
	DaoxPathStyle_Convert( other, second );
	for(i=0; i<self->dash+5; ++i){
		if( first[i] != second[i] ) return first[i] < second[i] ? -1 : 1;
	}
	return 0;
}

DaoxPath* DaoxPathCache_FindPath( DaoxPathCache *self, DaoxPath *path )
{
	uint_t hash = DaoxPath_Hash( path );
	DNode *it = DMap_Find( self->paths, IntToPointer(hash) );
	DaoxPath *cached;

	if( it != NULL ){
		DList *paths = (DList*) it->value.pVoid;
		int i;
		for(i=0; i<paths->size; ++i){
			cached = (DaoxPath*) paths->items.pValue[i];
			if( cached == path ) return cached;
			if( cached->hash != hash ) continue;
			if( DaoxPath_Compare( cached, path ) == 0 ) return cached;
		}
	}else{
		DList *ls = DList_New(DAO_DATA_VALUE);
		it = DMap_Insert( self->paths, IntToPointer(hash), ls );
		DList_Delete( ls );
	}
	self->pathCount += 1;
	cached = DaoxPath_New();
	cached->cached = 1;
	DaoxPath_Copy( cached, path );
	DList_Append( it->value.pList, cached );
	return cached;
}
DaoxPathMesh* DaoxPathCache_FindMesh( DaoxPathCache *self, DaoxPath *path, DaoxPathStyle *style )
{
	uint_t hash = DaoxPathStyle_Hash( style, DaoxPath_Hash( path ) );
	DNode *it = DMap_Find( self->meshes, IntToPointer(hash) );
	DaoxPathMesh *mesh;

	if( it != NULL ){
		DList *meshs = (DList*) it->value.pVoid;
		int i, cmp;
		for(i=0; i<meshs->size; ++i){
			DaoxPathMesh *cached = (DaoxPathMesh*) meshs->items.pValue[i];
			if( cached->hash != hash ) continue;
			cmp = DaoxPath_Compare( cached->path, path );
			if( cmp != 0 ) continue;
			if( DaoxPathStyle_Compare( & cached->strokeStyle, style ) == 0 ) return cached;
		}
	}
	if( it == NULL ){
		DList *ls = DList_New(DAO_DATA_VALUE);
		it = DMap_Insert( self->meshes, IntToPointer(hash), ls );
		DList_Delete( ls );
	}
	self->meshCount += 1;
	if( path->cached == 0 ) path = DaoxPathCache_FindPath( self, path );
	mesh = DaoxPathMesh_New();
	mesh->hash = hash;
	DList_Append( it->value.pList, mesh );
	DaoxPathMesh_Reset( mesh, path, style );
	DaoxPathMesh_Preprocess( mesh, self->triangulator );
	if( style->width > 1E-9 ) DaoxPathMesh_ComputeStroke( mesh );
#ifdef DEBUG
	printf( "(%p) Cached paths: %i; Cached meshes: %i\n", self, self->pathCount, self->meshCount );
#endif
	return mesh;
}
