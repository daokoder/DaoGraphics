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
#include "dao_path.h"


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
	DaoxVectorD4 Svec, vec1, vec2, vec3 ,vec4;
	DaoxMatrixD4X4 Cmat, Mmat = { {{-1,3,-3,1}, {3,-6,3,0}, {-3,3,0,0}, {1,0,0,0}} };
	DaoxKeyFrame *keyFrames = self->keyFrames->data.keyframes;
	DaoxKeyFrame *prevFrame, *nextFrame;
	int imin, first, last, frameCount = self->keyFrames->size;
	float min, endtime, factor = 0.5;

	self->time += dtime;
	self->dtime += dtime;

	if( self->dtime < 1E-3 ) return;
	if( frameCount == 0 ) return;

	endtime = keyFrames[frameCount-1].time;
	if( endtime < 1E-3 ) return;

	self->time  = self->time - endtime * (int)(self->time / endtime);
	self->dtime = 0.0;

	first = 0;
	last = frameCount - 1;
	min = endtime + 1.0;
	imin = 0;
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
	prevFrame = nextFrame = self->keyFrames->data.keyframes + imin;
	if( self->time > nextFrame->time ){
		nextFrame = self->keyFrames->data.keyframes + (imin + 1) % frameCount;
	}else{
		prevFrame = self->keyFrames->data.keyframes + (imin + frameCount - 1) % frameCount;
	}

	factor = (self->time - prevFrame->time) / (nextFrame->time - prevFrame->time);
	if( prevFrame->time > nextFrame->time ){
		factor = self->time / nextFrame->time;
	}

	if( self->channel == DAOX_ANIMATE_TF ){
		// XXX: assuming only rotation and translation:
		DaoxQuaternion Q1 = DaoxQuaternion_FromRotationMatrix( & prevFrame->matrix );
		DaoxQuaternion Q2 = DaoxQuaternion_FromRotationMatrix( & nextFrame->matrix );
		DaoxQuaternion Q = DaoxQuaternion_Slerp( & Q1, & Q2, factor );

		self->transform = DaoxMatrix4D_FromQuaternion( & Q );
		self->transform.B1 = (1.0 - factor)*prevFrame->matrix.B1 + factor*nextFrame->matrix.B1;
		self->transform.B2 = (1.0 - factor)*prevFrame->matrix.B2 + factor*nextFrame->matrix.B2;
		self->transform.B3 = (1.0 - factor)*prevFrame->matrix.B3 + factor*nextFrame->matrix.B3;

#if 0
		self->transform = DaoxMatrix4D_Interpolate( & prevFrame->matrix, & nextFrame->matrix, factor );
#endif
		return;
	}

	P0 = prevFrame->vector;
	C0 = prevFrame->tangent2;
	C1 = nextFrame->tangent1;
	P1 = nextFrame->vector;
	switch( self->channel ){
	case DAOX_ANIMATE_SX : case DAOX_ANIMATE_SY : case DAOX_ANIMATE_SZ :
	case DAOX_ANIMATE_RX : case DAOX_ANIMATE_RY : case DAOX_ANIMATE_RZ :
	case DAOX_ANIMATE_TX : case DAOX_ANIMATE_TY : case DAOX_ANIMATE_TZ :
		P0.x = prevFrame->time;
		P0.y = prevFrame->scalar;
		P1.x = nextFrame->time;
		P1.y = nextFrame->scalar;
		break;
	case DAOX_ANIMATE_TL :
		P0 = prevFrame->vector;
		P1 = nextFrame->vector;
		break;
	}
	if( prevFrame->curve == DAOX_ANIMATE_HERMITE ){
		DaoxVector3D T0D3 = DaoxVector3D_Scale( & C0, 1.0/3.0 );
		DaoxVector3D T1D3 = DaoxVector3D_Scale( & C1, 1.0/3.0 );
		C0 = DaoxVector3D_Add( & P0, & T0D3 );
		C1 = DaoxVector3D_Sub( & P1, & T1D3 );
	}
	switch( prevFrame->curve ){
	case DAOX_ANIMATE_LINEAR :
		vector = DaoxVector3D_Interpolate( P0, P1, factor );
		break;
	case DAOX_ANIMATE_BEZIER :
	case DAOX_ANIMATE_HERMITE : 
		Svec.w = 1.0;
		Svec.z = factor;
		Svec.y = factor * factor;
		Svec.x = Svec.y * factor;
		vec1 = DaoxVectorD4_XYZW( P0.x, P0.y, P0.z, 1.0 );
		vec2 = DaoxVectorD4_XYZW( C0.x, C0.y, C0.z, 1.0 );
		vec3 = DaoxVectorD4_XYZW( C1.x, C1.y, C1.z, 1.0 );
		vec4 = DaoxVectorD4_XYZW( P1.x, P1.y, P1.z, 1.0 );
		Cmat = DaoxMatrixD4X4_InitColumns( vec1, vec2, vec3, vec4 );
		Svec = DaoxMatrixD4X4_MulVector( & Mmat, & Svec );
		Svec = DaoxMatrixD4X4_MulVector( & Cmat, & Svec );
		vector.x = Svec.x;
		vector.y = Svec.y;
		vector.z = Svec.z;
		break;
	case DAOX_ANIMATE_BSPLINE : // TODO:
	default:
		vector = DaoxVector3D_Interpolate( P0, P1, factor );
		break;
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

