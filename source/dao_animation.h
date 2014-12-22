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


#ifndef __DAO_ANIMATION_H__
#define __DAO_ANIMATION_H__

#include "dao_path.h"


typedef struct DaoxAnimation  DaoxAnimation;
typedef struct DaoxKeyFrame   DaoxKeyFrame;

enum DaoxAnimationChannel
{
	DAOX_ANIMATE_SX ,
	DAOX_ANIMATE_SY ,
	DAOX_ANIMATE_SZ ,
	DAOX_ANIMATE_RX ,
	DAOX_ANIMATE_RY ,
	DAOX_ANIMATE_RZ ,
	DAOX_ANIMATE_TX ,
	DAOX_ANIMATE_TY ,
	DAOX_ANIMATE_TZ ,
	DAOX_ANIMATE_TL ,
	DAOX_ANIMATE_TF
};

enum DaoxAnimationInterpolation
{
	DAOX_ANIMATE_LINEAR ,
	DAOX_ANIMATE_BEZIER ,
	DAOX_ANIMATE_HERMITE , 
	DAOX_ANIMATE_BSPLINE 
};


struct DaoxKeyFrame
{
	float         time;
	short         curve;
	float         scalar;
	DaoxVector3D  vector;
	DaoxMatrix4D  matrix;
	DaoxVector3D  tangent1;
	DaoxVector3D  tangent2;
};


struct DaoxAnimation
{
	DAO_CSTRUCT_COMMON;

	float         time;
	float         dtime;
	short         channel;
	int           keyFrame1;
	int           keyFrame2;
	DArray       *keyFrames;
	DaoxMatrix4D  transform;
};
extern DaoType* daox_type_animation;


DaoxAnimation* DaoxAnimation_New();
void DaoxAnimation_Delete( DaoxAnimation *self );

void DaoxAnimation_Update( DaoxAnimation *self, float dtime );


#endif
