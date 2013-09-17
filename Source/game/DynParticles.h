// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __DYNPARTICLES_H__
#define __DYNPARTICLES_H__

/*
===============================================================================

	c4tnt

	Extended dynamic particle system. 

===============================================================================
*/

typedef struct _dynprt_s _dynprt_t;

enum _dynprt_spawn_method {
	DSM_PARTICLE,				// Spawn additional particle
	DSM_SPLASH,					// Spawn decal splash
	DSM_DECAL,					// Spawn static decal
	DSM_SELF					// Change self
}

enum _dynprt_spawn_direction {
	DSD_FOLLOW,					// Particles will follow the parent direction
	DSD_SPRAY,					// Particles will sprayed 
	DSD_NEGSPRAY,				// Particles will sprayed back
	DSD_ORTOXPLODE,				// Particles will directed ortogonally for the previous direction
	DSD_XPLODE,					// Particles will directed randomly
	DSD_SLIDE					// Particles will slide on surface
}

enum _dynprt_spawn_trigger {
	DST_NEARWALL,				// Near the blocking wall <near_dist arg>
	DST_NEARWATER,				// Near the water <near_dist arg>
	DST_FAR,					// When so far from the spawner <far_dist arg>
	DST_TOUCHWALL,				// When touching the wall <no arg>
	DST_TOUCHWATER,				// When touching the water <no arg>
	DST_TOUCHFLOOR,				// When touching the floor <no arg> (special top-down trace)
	DST_NEARFLOOR,				// When particle is nearby the floor <near_dist>
	DST_TIME,					// When time elapsed <time>
	DST_PREVIOUS				// Trigger when previous event in chain was triggered
}

typedef struct _dynprt_event_s {
	
	_dynprt_t*					Spawn;	// Spawn particle type ( NULL for nothing )
	int							Count;	// Count
	
	_dynprt_spawn_direction		director;	// New particle direction
	float						spray_angle;
	float						new_speed;
	float						new_aspeed;
	
	_dynprt_spawn_trigger		trigger;		// Event reason
	float						trig_arg;

	idAngles					angular_speed_mod;
	idVec3						linear_speed_mod;

	struct _dynprt_event_s*		chained_event;

} _dynprt_event_t;

class idDynamicParticleCloud {
public:
								idDynamicParticleCloud( void );

private:

	idBounds					AABB;				// For AABB collision and clipping

};

class idDynamicParticles {
public:
								idDynamicParticles( void );

	// creats an entity covering the entire world that will call back each rendering
	void						Init( void );
	void						Shutdown( void );

private:

};

#endif /* !__SMOKEPARTICLES_H__ */
