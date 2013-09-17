#ifndef __GAME_AREA_H__
#define __GAME_AREA_H__


/*
===============================================================================

  events

===============================================================================
*/
const idEventDef EV_TriggerAction( "<triggerAction>", "e" );

/*
===============================================================================

  idAreaMarker

===============================================================================
*/
class idAreaDependant;

typedef enum {
		CMPM_BYNAME,
		CMPM_BYCLASS
} ecmp_method;

typedef struct _area_callback_s {
	idAreaDependant* owner;		//Class-owner
	const idPoolStr* filter;	//Filter string
	ecmp_method cmp_method;		//Filtering method
	int UserData;				//Trigger-specified data. 
	bool _UF;					//Changed flag, for internal use
} _area_callback_t;

class idAreaMarker : public idEntity {
public:
	CLASS_PROTOTYPE( idAreaMarker );

						idAreaMarker( void );

	void				Spawn( void );
	virtual void		Think( void );

	void				Save( idSave_I *savefile );
	void				Restore( idRestore_I *savefile );
	
	int					Attach( idAreaDependant* obj, const char* filter, ecmp_method cmp_method = CMPM_BYNAME, int UserData = 0 ); //Connect events reciever
	bool				Update( int Handle, WORD updBits, const char* filter, ecmp_method cmp_method = CMPM_BYNAME, int UserData = 0 ); //Change filter
	void				Detach( int Handle ); //Disconnect single event
	void				DetachAll( idAreaDependant* obj ); //Disconnect event reciever

	void				TouchEntities( void );

	virtual void		ActivateTargets( idEntity *activator ) const;

	int					GetCount (int Handle) const;
	int					GetCount (idAreaDependant* ob) const;
	bool				Filter( int Handle, idEntity* ent ) const;

private:
	idClipModel *		clipModel;
	idList< _area_callback_t > callbacks;		

	bool				Enter( idEntity* ent );
	void				Exit( idEntity* ent );
	void				Event_Activate( idEntity *activator );
	bool				CheckEntity( idEntity* ent );
	void				SendChangedNotify( bool Send = true );
	bool				Owned( idEntity* ent ) const;

	static idStrPool	globalFilters;

};

/*
===============================================================================

idArea_Dependant

===============================================================================
*/

class idAreaDependant : public idEntity {
public:
	CLASS_PROTOTYPE( idAreaDependant );
	
	virtual			~idAreaDependant ();
	virtual bool	AreaChangedNotify(idAreaMarker* Area) { return false; }; //Calls when something changes in Area
	virtual bool	AreaEnterNotify(idAreaMarker* Area, idEntity* ent) { return false; }; //Calls when entity entering the area
	virtual bool	AreaExitNotify(idAreaMarker* Area, idEntity* ent) { return false; }; //Calls when entity leaving the area

protected:
	void idAreaDependant::Event_Register( );
	idList< idEntityPtr<idAreaMarker> >	areas;		// to attach a multiple targets

private:


};

#endif

/*
===============================================================================

  Trigger, area counter

===============================================================================
*/

class idAreaCount : public idAreaDependant {
public:
	CLASS_PROTOTYPE( idAreaCount );

						idAreaCount( void );

	void				Save( idSave_I *savefile ) const;
	void				Restore( idRestore_I *savefile );
	virtual bool		AreaChangedNotify(idAreaMarker* Area);

	void				Spawn( void );
	const function_t *	GetScriptFunction( void ) const;


private:
	float				wait;
	
	float				min;
	float				max;
	bool				flagged;
	bool				flip;

	float				random;
	float				delay;
	float				random_delay;
	int					nextTriggerTime;
	bool				on;
	const function_t *	scriptFunction;

	void				TriggerAction( );
	void				Event_TriggerAction( );
	void				Event_Enable( void );
	void				Event_Disable( void );
	void				Event_Activate( idEntity *activator );
	void				CallScript( void ) const;

};
