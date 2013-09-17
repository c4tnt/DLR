// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_LASER_H__
#define __GAME_LASER_H__

/*
===================================================================================

	Laser special types

===================================================================================
*/

struct LaserChunk_t {
	renderEntity_t			renderEntitySegment;
	qhandle_t				modelDefHandleSegment;
	renderEntity_t			renderEntityCap;
	qhandle_t				modelDefHandleCap;
	idVec3					SavedEnd;
};

/*
===================================================================================

	Laser source

===================================================================================
*/

class idLaser : public idEntity {
public:
	CLASS_PROTOTYPE( idLaser );

						idLaser();
						~idLaser();

	void				Spawn( void );

	void				Save( idSave_I *savefile ) const;
	void				Restore( idRestore_I *savefile );

	virtual void		Think( void );

	void				UpdateBeam( const idEntity *targetEnt,const float Length); //Calculates the end beam pos and apply damage effects
	bool				AppendBeam( int IDX, idVec3 &Start, idVec3 &End, bool forceUpdate = false ); //Extend the beam. Can reuse chunk when it is set
	bool				CropBeam( int IDX ); //Crop the beam

	virtual void		WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void		ReadFromSnapshot( const idBitMsgDelta &msg );


private:
	void				Event_MatchTarget( void );
	void				Event_Activate( idEntity *activator );
	void				Event_On( void );
	void				Event_Off( void );
	void				Event_GetTouchEntity( idEventReturn *returnContext );
	void				Event_AimOn( idEntity *ent );

	idEntityPtr<idEntity> targetLaser;
	idEntityPtr<idEntity> latched;
	
	idList<LaserChunk_t> Chunks;
	
	int					MaxChunks;
	float				beamWidth;
	idStr				beamSkin;
	idStr				damageDef;
	bool				enabled;
	idVec3				color;

	const idDeclParticle *	endSmoke;			// null if it doesn't smoke
	int					endSmokeStartTime;
	int					endSmokeLastTime;
	int					nextDamageTime;
	int					DamageTime;
};

#endif /* !__GAME_LASER_H__ */
