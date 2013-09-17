// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_TRIGGER_H__
#define __GAME_TRIGGER_H__

extern const idEventDef EV_Enable;
extern const idEventDef EV_Disable;

/*
===============================================================================

  Trigger base.

===============================================================================
*/

class idTrigger : public idEntity {
public:
	CLASS_PROTOTYPE( idTrigger );

	static void			DrawDebugInfo( void );

						idTrigger();
	void				Spawn( void );

	void				Save( idSave_I *savefile ) const;
	void				Restore( idRestore_I *savefile );

	virtual void		Enable( void );
	virtual void		Disable( void );

	void				CallScript( void ) const;

protected:

	void				Event_Enable( void );
	void				Event_Disable( void );

	idScriptContext		scriptContext;
//	const function_t *	scriptFunction;
};


/*
===============================================================================

  Trigger which can be activated multiple times.

===============================================================================
*/

class idTrigger_Multi : public idTrigger {
public:
	CLASS_PROTOTYPE( idTrigger_Multi );

						idTrigger_Multi( void );

	void				Spawn( void );

	void				Save( idSave_I *savefile ) const;
	void				Restore( idRestore_I *savefile );

private:
	float				wait;
	float				random;
	float				delay;
	float				random_delay;
	int					nextTriggerTime;
	idStr				requires;
	int					removeItem;
	bool				touchClient;
	bool				touchOther;
	bool				triggerFirst;
	bool				triggerWithSelf;

	bool				CheckFacing( idEntity *activator );
	void				TriggerAction( idEntity *activator );
	void				Event_TriggerAction( idEntity *activator );
	void				Event_Trigger( idEntity *activator );
	void				Event_Touch( idEntity *other, trace_t *trace );
};

/*
===============================================================================

  Trigger which can be activated by USE key

===============================================================================
*/

extern const idEventDef EV_SetDescription;
extern const idEventDef EV_Door_Lock;
extern const idEventDef EV_Door_IsLocked;

class idTrigger_Use : public idTrigger {
public:
	CLASS_PROTOTYPE( idTrigger_Use );

						idTrigger_Use( void );

	void				Spawn( void );

	void				Save( idSave_I *savefile ) const;
	void				Restore( idRestore_I *savefile );
	virtual bool		ConfirmUseDescr( use_descr_t* Desc );

private:
	float				wait;
	float				random;
	float				delay;
	float				random_delay;
	int					nextTriggerTime;
	int					removeItem;
	bool				triggerFirst;
	bool				triggerWithSelf;
	bool				fire_use;
	bool				hidden_use;
	bool				disable_while_use;
	bool				locked;

	idStr				requires;
	idStr				objectName;
	idStr				actionName;

	void				TriggerAction( idEntity *activator );
	void				Event_TriggerAction( idEntity *activator );
	void				Event_Trigger( idEntity *activator );
	void				Event_Use( idEntity *other, trace_t *trace );
	void				Event_SetDescription( const char* text );
	void				Event_Lock( float f );
	void				Event_Locked( idEventReturn *returnContext ); //return f
};

/*
===============================================================================

  Trigger which can only be activated by an entity with a specific name.

===============================================================================
*/

class idTrigger_EntityName : public idTrigger {
public:
	CLASS_PROTOTYPE( idTrigger_EntityName );

						idTrigger_EntityName( void );

	void				Save( idSave_I *savefile ) const;
	void				Restore( idRestore_I *savefile );

	void				Spawn( void );

private:
	float				wait;
	float				random;
	float				delay;
	float				random_delay;
	int					nextTriggerTime;
	bool				triggerFirst;
	idStr				entityName;

	void				TriggerAction( idEntity *activator );
	void				Event_TriggerAction( idEntity *activator );
	void				Event_Trigger( idEntity *activator );
	void				Event_Touch( idEntity *other, trace_t *trace );
};

/*
===============================================================================

  Trigger which repeatedly fires targets.

===============================================================================
*/

class idTrigger_Timer : public idTrigger {
public:
	CLASS_PROTOTYPE( idTrigger_Timer );

						idTrigger_Timer( void );

	void				Save( idSave_I *savefile ) const;
	void				Restore( idRestore_I *savefile );

	void				Spawn( void );

	virtual void		Enable( void );
	virtual void		Disable( void );

private:
	float				random;
	float				wait;
	bool				on;
	float				delay;
	idStr				onName;
	idStr				offName;

	void				Event_Timer( void );
	void				Event_Use( idEntity *activator );
};



/*
===============================================================================

  Trigger which fires targets after being activated a specific number of times.

===============================================================================
*/

class idTrigger_Count : public idTrigger {
public:
	CLASS_PROTOTYPE( idTrigger_Count );

						idTrigger_Count( void );

	void				Save( idSave_I *savefile ) const;
	void				Restore( idRestore_I *savefile );

	void				Spawn( void );

private:
	int					goal;
	int					count;
	float				delay;

	void				Event_Trigger( idEntity *activator );
	void				Event_TriggerAction( idEntity *activator );
};


/*
===============================================================================

  Trigger which hurts touching entities.

===============================================================================
*/

class idTrigger_Hurt : public idTrigger {
public:
	CLASS_PROTOTYPE( idTrigger_Hurt );

						idTrigger_Hurt( void );

	void				Save( idSave_I *savefile ) const;
	void				Restore( idRestore_I *savefile );

	void				Spawn( void );

private:
	bool				on;
	float				delay;
	int					nextTime;

	void				Event_Touch( idEntity *other, trace_t *trace );
	void				Event_Toggle( idEntity *activator );
};


/*
===============================================================================

  Trigger which fades the player view.

===============================================================================
*/

class idTrigger_Fade : public idTrigger {
public:

	CLASS_PROTOTYPE( idTrigger_Fade );

private:
	void				Event_Trigger( idEntity *activator );
};


/*
===============================================================================

  Trigger which continuously tests whether other entities are touching it.

===============================================================================
*/

class idTrigger_Touch : public idEntity {
public:

	CLASS_PROTOTYPE( idTrigger_Touch );

						idTrigger_Touch( void );

	void				Spawn( void );
	virtual void		Think( void );

	void				Save( idSave_I *savefile );
	void				Restore( idRestore_I *savefile );

	virtual void		Enable( void );
	virtual void		Disable( void );

	void				TouchEntities( void );

private:
	idClipModel *		clipModel;

	void				Event_Trigger( idEntity *activator );
	void				CallScript( void ) const;

	void				Event_Enable( void );
	void				Event_Disable( void );

	const function_t *	scriptFunction;
};

#endif /* !__GAME_TRIGGER_H__ */
