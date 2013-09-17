// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_SI_H__
#define __GAME_SI_H__

/*
===================================================================================

	SI special types

===================================================================================
*/

typedef struct {
	idVec3		origin;
	idVec3		dir;
	int			localTime;
	int			localLATime;
	idMat3		ax;
} damagePoints_t;

typedef struct {
	bool Internal; // Shared edge
	bool Collapsed; // Single edge, but have a neighbours
	bool CrossSurf; // Single edge, but have a neighbours on another surface
	idList<int>	tris;
	int			vert[2];
	int			surf;
} idxedge_t;
/*
===================================================================================

	Pressured steam pipe

===================================================================================
*/


class idSIPipe : public idEntity {
public:
	CLASS_PROTOTYPE( idSIPipe );

							idSIPipe ();
							~idSIPipe ();

	void					Spawn( void );

	void					Save( idSave_I *savefile ) const;
	void					Restore( idRestore_I *savefile );

	void					AddVapor(const idVec3 &pos,const idVec3 &velocity);

	virtual void			Think( void );

	virtual void			Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );
	virtual bool			Pain( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );
	virtual void			Hide( void );
	virtual void			Show( void );

	virtual void			AddDamageEffect( const trace_t &collision, const idVec3 &velocity, const char *damageDefName );

private:
	bool					On;
	bool					invencible;
	int						smokeTime;
	const idDeclParticle *	smoke;
	const idDeclParticle *	cursmoke;
	idList<damagePoints_t>	vaporPoints;
	idVec3					particleFwd;

	void					RemoveVapors();

	void					Event_Activate( idEntity *activator );
};

/*
===================================================================================

	Energetical (Flat with beams on a edges)

===================================================================================
*/

class idEnergetical : public idEntity {
public:
	CLASS_PROTOTYPE( idEnergetical );

							idEnergetical ();
							~idEnergetical ();

	void					Spawn( void );

	void					Save( idSave_I *savefile ) const;
	void					Restore( idRestore_I *savefile );

	virtual void			Think( void );

	virtual void			Hide( void );
	virtual void			Show( void );

	virtual void			AddDamageEffect( const trace_t &collision, const idVec3 &velocity, const char *damageDefName );

	void					Debug_Edges();
	void					makeEdges( const idRenderModel *renderModel );

private:
	bool					On;
	idList<idxedge_t*>		EdgeList;
	void					Event_Activate( idEntity *activator );
};

#endif /* !__GAME_SI_H__ */
