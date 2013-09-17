#ifndef __LIQUID_H__
#define __LIQUID_H__

/*
===============================================================================

  idLiquid

	Base class for all liquid object.  The entity part of the liquid is
	responsible for spawning splashes and sounds to match.

	The physics portion is as usual, responsible for the physics.
===============================================================================
*/

class idRenderModelLiquid;

class idLiquid : public idEntity {
public:
	CLASS_PROTOTYPE( idLiquid );

	void				Spawn( void );

	void				Save( idSave_I *savefile ) const;
	void				Restore( idRestore_I *savefile );

	virtual impactEffect_t Collide( const trace_t &collision, const idVec3 &velocity );

	const char*			getSubmergedMtr() const;

private:

	void				SetupModel( const idRenderModel *renderModel );


	void				Event_Touch( idEntity *other, trace_t *trace );

	idPhysics_Liquid	physicsObj;

	const idDeclParticle *splash[3];
	const idDeclParticle *waves;

	idStr				smokeName;
	idStr				soundName;
	idStr				underwaterMtr;
};

#endif // __LIQUID_H__