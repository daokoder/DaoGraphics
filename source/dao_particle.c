/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2014-2016, Limin Fu
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

#include "dao_particle.h"


DaoxParticles* DaoxParticles_New( DaoxMeshUnit *data )
{
	DaoxParticles *self = (DaoxParticles*) dao_calloc( 1, sizeof(DaoxParticles) );
	self->particles = DArray_New( sizeof(DaoxParticle) );
	self->data = data;
	return self;
}
void DaoxParticles_Delete( DaoxParticles *self )
{
	DArray_Delete( self->particles );
	dao_free( self );
}
void DaoxParticles_Reset( DaoxParticles *self )
{
	self->timeout = 0;
	self->particles->size = 0;
	self->data->vertices->size = 0;
	self->data->triangles->size = 0;
}


DaoxEmitter* DaoxEmitter_New()
{
	DaoxMeshFrame *frame = DaoxMeshFrame_New();
	DaoxEmitter *self = (DaoxEmitter*) dao_calloc( 1, sizeof(DaoxEmitter) );
	DaoxModel_Init( (DaoxModel*) self, daox_type_emitter, DaoxMesh_New() );
	self->dtime = 0.0;
	self->emissionRate = 50;
	self->lifeSpan = 3;
	self->radialVelocity = 1;
	self->tangentVelocity = 0;
	self->gravityStrength = 1.0;
	self->gravity = DaoxVector3D_XYZ( 0.0, 1.0, 0.0 );
	self->clusters = DList_New(0);
	self->emitter = DaoxMeshUnit_New();
	GC_IncRC( self->emitter );
	DaoxMeshFrame_MakeSphere( frame, 0.1, 5 );
	DaoxMeshFrame_Export( frame, self->emitter );
	DaoxMeshFrame_Delete( frame );
	return self;
}
void DaoxEmitter_Delete( DaoxEmitter *self )
{
	DaoxModel_Free( (DaoxModel*) self );
	GC_DecRC( self->emitter );
	GC_DecRC( self->material );
	dao_free( self );
}

DaoxParticle* DaoxEmitter_AddParticle( DaoxEmitter *self, DaoxParticles *cluster, DaoxVector3D pos )
{
	int i, offset = cluster->data->vertices->size;
	DaoRandGenerator *randgen = self->randGenerator;
	DaoxParticle *particle = (DaoxParticle*) DArray_Push( cluster->particles );
	DaoxVertex *vertex1, *vertex2, *vertex3, *vertex4;
	DaoxVector3D X = DaoxVector3D_XYZ( 1.0, 0.0, 0.0 );
	DaoxVector3D Z = DaoxVector3D_XYZ( 0.0, 0.0, 1.0 );
	DaoxVector3D norm;
	float lifeSpan, r1, r2;

	if( randgen == NULL ) return NULL;

	particle->scale = 0.0;
	particle->position = pos;

	DArray_Reserve( cluster->data->vertices, offset + 4 );
	vertex1 = DArray_PushVertex( cluster->data->vertices, NULL );
	vertex2 = DArray_PushVertex( cluster->data->vertices, NULL );
	vertex3 = DArray_PushVertex( cluster->data->vertices, NULL );
	vertex4 = DArray_PushVertex( cluster->data->vertices, NULL );
	vertex1->pos.x = pos.x - 0.5;  vertex1->pos.y = pos.y - 0.5;  vertex1->pos.z = pos.z;
	vertex2->pos.x = pos.x + 0.5;  vertex2->pos.y = pos.y - 0.5;  vertex2->pos.z = pos.z;
	vertex3->pos.x = pos.x + 0.5;  vertex3->pos.y = pos.y + 0.5;  vertex3->pos.z = pos.z;
	vertex4->pos.x = pos.x - 0.5;  vertex4->pos.y = pos.y + 0.5;  vertex4->pos.z = pos.z;
	vertex1->tex.x = 0.0;  vertex1->tex.y = 0.0;
	vertex2->tex.x = 1.0;  vertex2->tex.y = 0.0;
	vertex3->tex.x = 1.0;  vertex3->tex.y = 1.0;
	vertex4->tex.x = 0.0;  vertex4->tex.y = 1.0;

	norm = DaoxTriangle_Normal( & vertex1->pos, & vertex2->pos, & vertex3->pos );
	vertex1->norm = vertex2->norm = vertex3->norm = vertex4->norm = norm;

	DArray_PushTriangleIJK( cluster->data->triangles, offset, offset+1, offset+2 );
	DArray_PushTriangleIJK( cluster->data->triangles, offset, offset+2, offset+3 );

	r1 = _DaoRandGenerator_GetUniform( randgen );
	r2 = _DaoRandGenerator_GetUniform( randgen );
	for(i=0; i<4; ++i){
		DaoxVertex *vertex = cluster->data->vertices->data.vertices + offset + i;
		vertex->tan.x = r1;
		vertex->tan.y = r2;
	}

	lifeSpan = self->lifeSpan * (0.8 + 0.2*_DaoRandGenerator_GetNormal( randgen ));
	if( lifeSpan < 0.5*self->lifeSpan ) lifeSpan = 0.5*self->lifeSpan;
	if( lifeSpan > 2.0*self->lifeSpan ) lifeSpan = 2.0*self->lifeSpan;
	if( self->life < lifeSpan ) lifeSpan = self->life;
	if( lifeSpan > cluster->timeout ) cluster->timeout = lifeSpan;
	particle->lifeSpan = lifeSpan;
	particle->life = 0;
	return particle;
}
void DaoxEmitter_Update( DaoxEmitter *self, float dtime )
{
	DaoxMatrix4D objToWorld = DaoxSceneNode_GetWorldTransform( (DaoxSceneNode*) self );
	DaoxMatrix4D worldToObj = DaoxMatrix4D_Inverse( & objToWorld );
	DaoxVector3D gravity = DaoxMatrix4D_Rotate( & worldToObj, & self->gravity );
	DaoxVector3D dv = DaoxVector3D_Scale( & gravity, dtime * self->gravityStrength );
	DaoRandGenerator *randgen = self->randGenerator;
	DaoxParticles *cluster = NULL;
	float fvelocity = dtime;
	int i, j, k;

	if( randgen == NULL ) return;
	self->dtime += dtime;
	self->life += dtime;
	for(i=0; i<self->active; ++i){
		DaoxParticles *cluster = (DaoxParticles*) self->clusters->items.pVoid[i];
		if( cluster->timeout < self->dtime ){
			if( (i+1) < self->active ){
				DaoxParticles *p = (DaoxParticles*) self->clusters->items.pVoid[self->active-1];
				self->clusters->items.pVoid[self->active-1] = cluster;
				self->clusters->items.pVoid[i] = p;
				i -= 1;
			}
			DaoxParticles_Reset( cluster );
			self->active -= 1;
			continue;
		}
		cluster->timeout -= self->dtime;
		for(j=0; j<cluster->particles->size; ++j){
			DaoxParticle *particle = cluster->particles->data.particles + j;
			DaoxVertex *vertices = cluster->data->vertices->data.vertices + 4*j;
			DaoxVector3D dx;
			float dist, factor;

			particle->life += self->dtime;
			factor = (particle->lifeSpan - particle->life) / particle->lifeSpan;
			factor *= particle->life / particle->lifeSpan;
			factor = sqrt( factor );

			dx = DaoxVector3D_Scale( & particle->velocity, fvelocity );

			particle->position = DaoxVector3D_Add( & particle->position, & dx );
			particle->velocity = DaoxVector3D_Add( & particle->velocity, & dv );
			dist = sqrt( DaoxVector3D_Norm2( & particle->position ) );
			particle->scale = 1.0 + sqrt(sqrt( dist ));
			particle->scale *= factor;
		}
	}
	cluster = NULL;
	if( self->active > 0 ){
		/*
		// Existing particle clusters could be used for the new particles, if:
		// -- The new particles will NOT delay the deletion of the existing
		//    particles in the clusters;
		// -- The particle cluster does not have too many particles already;
		*/
		cluster = self->clusters->items.pVoid[ self->active-1 ];
		if( cluster->timeout < 0.9 * self->lifeSpan ) cluster = NULL;
		if( cluster && cluster->particles->size >= 128 ) cluster = NULL;
	}
	if( cluster == NULL ){
		if( self->active >= self->clusters->size ){
			DaoxMeshUnit *unit = DaoxMesh_AddUnit( self->base.mesh );
			DaoxMeshUnit_SetMaterial( unit, self->material );
			cluster = DaoxParticles_New( unit );
			DList_Append( self->clusters, cluster );
		}
		cluster = self->clusters->items.pVoid[ self->active ++ ];
	}
	k = self->emissionRate * self->dtime;
	/*
	// dtime may be bigger for the first a few frames, without this adjustment,
	// many particles may be created at once at the beginning, and deleted at once
	// after their life span reached.
	*/
	if( self->life < self->lifeSpan ) k *= self->life / self->lifeSpan;
	if( k ) self->dtime = 0.0;
	while( k ){
		int source = self->emitter->vertices->size * _DaoRandGenerator_GetUniform( randgen );
		DaoxVertex *vertex = self->emitter->vertices->data.vertices + source;
		DaoxParticle *particle = DaoxEmitter_AddParticle( self, cluster, vertex->pos );
		particle->velocity = DaoxVector3D_Scale( & vertex->norm, self->radialVelocity );
		k -= 1;
	}
	DaoxMesh_UpdateTree( self->base.mesh, 1024 );
	DaoxMesh_ResetBoundingBox( self->base.mesh );
}
void DaoxEmitter_UpdateView( DaoxEmitter *self, DaoxVector3D campos )
{
	DaoRandGenerator *randgen = self->randGenerator;
	int i, j, k;
	for(i=0; i<self->active; ++i){
		DaoxParticles *cluster = (DaoxParticles*) self->clusters->items.pVoid[i];
		for(j=0; j<cluster->particles->size; ++j){
			DaoxParticle *particle = cluster->particles->data.particles + j;
			DaoxVertex *vertices = cluster->data->vertices->data.vertices + 4*j;
			DaoxVector3D camdir, dy, dx, O = DaoxVector3D_XYZ( 0, 0, 0 );
			DaoxVector3D N, P0, P1, P2, P3;

			if( particle->life > particle->lifeSpan ){
				for(k=0; k<4; ++k){
					DaoxVertex *vertex = vertices + k;
					vertex->pos = particle->position;
					vertex->norm = DaoxVector3D_XYZ( 0, 0, 0 );
				}
				continue;
			}
			camdir = DaoxVector3D_Sub( & particle->position, & campos );
			camdir = DaoxVector3D_Normalize( & camdir );
			if( randgen ){
				camdir.x += 0.1*(_DaoRandGenerator_GetUniform( randgen ) - 0.5);
				camdir.y += 0.1*(_DaoRandGenerator_GetUniform( randgen ) - 0.5);
				camdir.z += 0.1*(_DaoRandGenerator_GetUniform( randgen ) - 0.5);
				camdir = DaoxVector3D_Normalize( & camdir );
			}
			dx = DaoxVector3D_XYZ( camdir.y, camdir.z, camdir.x );
			dx = DaoxVector3D_Cross( & camdir, & dx );
			dx = DaoxVector3D_Normalize( & dx );
			dx = DaoxVector3D_Scale( & dx, particle->scale );

			dy = DaoxVector3D_Cross( & dx, & camdir );
			dy = DaoxVector3D_Normalize( & dy );
			dy = DaoxVector3D_Scale( & dy, particle->scale );

			P0 = DaoxVector3D_Sub( &  O, & dx );
			P0 = DaoxVector3D_Sub( & P0, & dy );
			P1 = DaoxVector3D_Add( &  O, & dx );
			P1 = DaoxVector3D_Sub( & P1, & dy );
			P2 = DaoxVector3D_Add( &  O, & dx );
			P2 = DaoxVector3D_Add( & P2, & dy );
			P3 = DaoxVector3D_Sub( &  O, & dx );
			P3 = DaoxVector3D_Add( & P3, & dy );

			vertices[0].pos = DaoxVector3D_Add( & particle->position, & P0 );
			vertices[1].pos = DaoxVector3D_Add( & particle->position, & P1 );
			vertices[2].pos = DaoxVector3D_Add( & particle->position, & P2 );
			vertices[3].pos = DaoxVector3D_Add( & particle->position, & P3 );

			N = DaoxTriangle_Normal( & vertices[0].pos, & vertices[1].pos, & vertices[2].pos );
			for(k=0; k<4; ++k){
				vertices[k].norm = N;
				vertices[k].tan = particle->position;
				vertices[k].tan.z = (particle->lifeSpan - particle->life) / particle->lifeSpan;
			}
		}
	}
}
