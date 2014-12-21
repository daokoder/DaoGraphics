/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2014, Limin Fu
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


#include "dao_animation.h"


DaoxAnimation* DaoxAnimation_New()
{
	DaoxAnimation *self = (DaoxAnimation*) dao_calloc( 1, sizeof(DaoxAnimation) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_animation );
	self->keyFrames = DArray_New( sizeof(DaoxKeyFrame) );
	self->transform = DaoxMatrix4D_Identity();
	return self;
}
void DaoxAnimation_Delete( DaoxAnimation *self )
{
	DArray_Delete( self->keyFrames );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}

void DaoxAnimation_Update( DaoxAnimation *self, float dtime )
{
	DaoxMatrix4D matrix;
	DaoxVector3D vector;
	DaoxVector3D P0, P1, C0, C1;
	DaoxVector3D scale, rotate, translate;
	DaoxKeyFrame *firstFrame, *secondFrame;
	int imin, first, last, frameCount = self->keyFrames->size;
	float min, endtime, factor = 0.5;

	self->time += dtime;
	self->dtime += dtime;
	if( self->dtime < 1E-3 ) return;
	if( frameCount == 0 ) return;

	endtime = self->keyFrames->data.keyframes[frameCount-1].time;
	if( endtime < EPSILON ) return; // TODO

	while( self->time > endtime ) self->time -= endtime;
	self->dtime = 0;

	imin = -1;
	min = endtime + 1.0;

	first = 0;
	last = frameCount - 1;
	/* Locate the closest frame: */
	while( first <= last ){
		int mid = (first + last) / 2;
		double time = self->keyFrames->data.keyframes[mid].time;
		double diff = fabs( self->time - time );
		if( diff < min ){
			min = diff;
			imin = mid;
		}
		if( self->time > time ){
			first = mid + 1;
		}else{
			last = mid - 1;
		}
	}
	firstFrame = secondFrame = self->keyFrames->data.keyframes + imin;
	if( self->time > secondFrame->time ){
		secondFrame = self->keyFrames->data.keyframes + (imin + 1) % frameCount;
	}else{
		firstFrame = self->keyFrames->data.keyframes + (imin + frameCount - 1) % frameCount;
	}

	P0 = firstFrame->vector;
	P1 = secondFrame->vector;
	C0 = firstFrame->tangent2;
	C1 = secondFrame->tangent1;
	switch( self->channel ){
	case DAOX_ANIMATE_SX : case DAOX_ANIMATE_SY : case DAOX_ANIMATE_SZ :
	case DAOX_ANIMATE_RX : case DAOX_ANIMATE_RY : case DAOX_ANIMATE_RZ :
	case DAOX_ANIMATE_TX : case DAOX_ANIMATE_TY : case DAOX_ANIMATE_TZ :
		P0.x = firstFrame->time;
		P0.y = firstFrame->scalar;
		P1.x = secondFrame->time;
		P1.y = secondFrame->scalar;
		break;
	case DAOX_ANIMATE_TL :
		P0 = firstFrame->vector;
		P1 = secondFrame->vector;
		break;
	}
	factor = (self->time - firstFrame->time) / (secondFrame->time - firstFrame->time);
	if( firstFrame->time > secondFrame->time ){
		factor = self->time / secondFrame->time;
	}
	vector = DaoxVector3D_Interpolate( P0, P1, factor );
	if( self->channel == DAOX_ANIMATE_TF ){
		switch( firstFrame->curve ){
		case DAOX_ANIMATE_LINEAR :
		case DAOX_ANIMATE_BEZIER :
		case DAOX_ANIMATE_HERMITE : 
		case DAOX_ANIMATE_BSPLINE :
			break;
		}
		self->transform = DaoxMatrix4D_Interpolate( & firstFrame->matrix, & secondFrame->matrix, factor );
		return;
	}else{
		switch( firstFrame->curve ){
		case DAOX_ANIMATE_LINEAR :
		case DAOX_ANIMATE_BEZIER :
		case DAOX_ANIMATE_HERMITE : 
		case DAOX_ANIMATE_BSPLINE :
			break;
		}
	}

	scale = DaoxVector3D_XYZ( 1.0, 1.0, 1.0 );
	rotate = translate = DaoxVector3D_XYZ( 0.0, 0.0, 0.0 );
	switch( self->channel ){
	case DAOX_ANIMATE_SX : scale.x = vector.y; break;
	case DAOX_ANIMATE_SY : scale.y = vector.y; break;
	case DAOX_ANIMATE_SZ : scale.z = vector.y; break;
	case DAOX_ANIMATE_RX : rotate.x = vector.y; break;
	case DAOX_ANIMATE_RY : rotate.y = vector.y; break;
	case DAOX_ANIMATE_RZ : rotate.z = vector.y; break;
	case DAOX_ANIMATE_TX : translate.x = vector.y; break;
	case DAOX_ANIMATE_TY : translate.y = vector.y; break;
	case DAOX_ANIMATE_TZ : translate.z = vector.y; break;
	case DAOX_ANIMATE_TL : translate = vector; break;
	case DAOX_ANIMATE_TF : break;
	}
	self->transform = DaoxMatrix4D_Combine( scale, rotate, translate );
}

