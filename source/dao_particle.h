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

#ifndef __DAO_PARTICLE__
#define __DAO_PARTICLE__

#include "dao_scene.h"

typedef struct DaoxParticle   DaoxParticle;
typedef struct DaoxParticles  DaoxParticles;
typedef struct DaoxEmitter    DaoxEmitter;


struct DaoxParticle
{
	int           index;
	float         life;
	float         lifeSpan;
	float         mass;
	DaoxVector3D  position;
	DaoxVector3D  velocity;
	DaoxVector3D  scale;
};

struct DaoxParticles
{
	float          timeout;
	DArray        *particles;
	DaoxMeshUnit  *data;
};


/*
// Spherical emitter:
//
// -- Randomized normal vector as initial velocity direction;
// -- Randomized tangent vector as gradient vector for perlin-like noise;
*/
struct DaoxEmitter
{
	DaoxModel  base;

	DaoxMeshUnit  *emitter;

	DList  *clusters;
	int     active;

	float  dtime;
	float  spawnRate;
	float  lifeSpan;
	float  radialVelocity;
	float  tangentVelocity;

	DaoxVector3D  gravity;
};
extern DaoType *daox_type_emitter;

DaoxEmitter* DaoxEmitter_New();
void DaoxEmitter_Delete( DaoxEmitter *self );

void DaoxEmitter_Update( DaoxEmitter *self, float dtime );
void DaoxEmitter_UpdateView( DaoxEmitter *self, DaoxVector3D campos );


#endif
