// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __DEBRISSYS_H__
#define __DEBRISSYS_H__

/*
===============================================================================


===============================================================================
*/

// Full quality models
#define HQ_DEBRIS_POLY	1024
// Simplified models
#define MQ_DEBRIS_POLY	1024
// Flats and sprites
#define LQ_DEBRIS_POLY	1024

// We have static decals, models and rest physics here.
typedef struct junkcluster_s {
	idVec3						origin;
	float						radius;
	idLinkList<DebriNode>		list;
	idEntityPtr<idEntity>		bind;		//Debris can be binded to some entity
	idEntityPtr<idEntity>		own;		//Debris owner. 
} junkcluster_t;

class DebriNode {
public:
	DebriNode( void );
	DebriNode~( void );
	
private:
	int							LodBias;	//Offset the LOD evaluation for this node
	idClipModel *				clipModel;	//All debris will have an a clipModel
	idPhysics_RigidBody			physicsObj;	//And a physics

};

typedef struct floatFragment_s {
	struct floatFragment_s	 *	next;
	int							privateStartTime;	// start time for this particular particle
	int							index;				// particle index in system, 0 <= index < stage->totalParticles
	idRandom					random;
	idVec3						origin;
	idMat3						axis;
} floatFragment_t;

class GlobalDebris {
public:
								GlobalDebris( void );

	// creats an entity covering the entire world that will call back each rendering
	void						Init( void );
	void						Shutdown( void );

	// drop a brass
	bool						DropBrass( const idDict &args, idEntity *owner, const idVec3 &start, const idMat3 &axis );
	// free old smokes
	void						FreeSmokes( void );

private:
	bool						initialized;

	idStaticList<renderEntity_t, HQ_DEBRIS_MODELS> HQRenderEntity; // used to present a model to the renderer
	int							renderEntityHandle;		// handle to static renderer model

	static const int			MAX_SMOKE_PARTICLES = 10000;
	singleSmoke_t				smokes[MAX_SMOKE_PARTICLES];

	idList<activeSmokeStage_t>	activeStages;
	singleSmoke_t *				freeSmokes;
	int							numActiveSmokes;
	int							currentParticleTime;	// don't need to recalculate if == view time

	bool						UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView );
	static bool					ModelCallback( renderEntity_s *renderEntity, const renderView_t *renderView );
};

#endif /* !__DEBRISSYS_H__ */
