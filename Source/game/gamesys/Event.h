// Copyright (C) 2004 Id Software, Inc.
//
/*
sys_event.h

Event are used for scheduling tasks and for linking script commands.
*/
#ifndef __SYS_EVENT_H__
#define __SYS_EVENT_H__

#define D_EVENT_MAXARGS				8			// if changed, enable the CREATE_EVENT_CODE define in Event.cpp to generate switch statement for idClass::ProcessEventArgPtr.
												// running the game will then generate c:\doom\base\events.txt, the contents of which should be copied into the switch statement.

#define D_EVENT_VOID				( ( char )0 )
#define D_EVENT_INTEGER				'd'
#define D_EVENT_FLOAT				'f'
#define D_EVENT_VECTOR				'v'
#define D_EVENT_STRING				's'
#define D_EVENT_ENTITY				'e'
#define	D_EVENT_ENTITY_NULL			'E'			// event can handle NULL entity pointers
#define D_EVENT_TRACE				't'
#define D_EVENT_LIST				'L'			// variate list
#define D_EVENT_CONTEXT				'*'			// this event doesn't return anything, but wants have an access to RC
#define D_RETURN_NSPEC				' '			// this event can return everything

#define MAX_EVENTS					4096
#define MAX_RETURNS					4096		// Maybe we need more than MAX_EVENTS, because returns are shared with a scripts
#define MAX_EVENTSPERFRAME			2048

class idClass;
class idTypeInfo;

class idEventDef {
private:
	const char					*name;
	const char					*formatspec;
	unsigned int				formatspecIndex;
	int							returnType;
	int							numargs;
	size_t						argsize;
	int							argOffset[ D_EVENT_MAXARGS ];
	int							eventnum;
	const idEventDef *			next;

	static idEventDef *			eventDefList[MAX_EVENTS];
	static int					numEventDefs;

public:
								idEventDef( const char *command, const char *formatspec = NULL, char returnType = 0 );
								
	const char					*GetName( void ) const;
	const char					*GetArgFormat( void ) const;
	unsigned int				GetFormatspecIndex( void ) const;
	char						GetReturnType( void ) const;
	int							GetEventNum( void ) const;
	int							GetNumArgs( void ) const;
	size_t						GetArgSize( void ) const;
	int							GetArgOffset( int arg ) const;

	static int					NumEventCommands( void );
	static const idEventDef		*GetEventCommand( int eventnum );
	static const idEventDef		*FindEvent( const char *name );
};

class idSave_I;
class idRestore_I;
class idInterpreter;

//c4tnt. Four-pass return system:
// 1. Event caller create the return context and a event
// 2. Event caller awaits when event fill this class and set 'Returned' flag
// 3. Event caller take off the return data and set 'Handled' flag
// 4. Engine remove this return context when 'Handled' flag is set

//Flag details
//    EVRS_EXECUTED - Function must set this flag when event was finished. Caller never set this flag
//    EVRS_HASRETURN - Function set this flag when he has a return data. Caller never set this flag
//    EVRS_HANDLED - Function never set this flag. Caller set this flag only when this RC don't need anymore
//    EVRS_AGAIN - function can set this flag for restarting himself at the next frame. Caller set this flag only when this RC don't need anymore
//    EVRS_FAILED - function doesn't executed. This flag interrupts all multiframed events and used in failed { ... } script construction.
//
//Single event:
//1. Caller creates RC
//2. Function uses this RC and marks it as EXECUTED
//3. Caller takes out returned value from RC and marks this RC as HANDLED
//4. Manager removes RC 
//
//Sleepy event:
//1. Caller creates RC
//2. Function uses RC and don't marks it as EXECUTED
//3. Function schedule event and gives own RC to this event
//4. Caller sees, that RC is not EXECUTED and sets status of the own thread to "Wait RC"
//5. Scheduled event marks RC as EXECUTED some time later
//6. Thread  wakes up and continue execution as a single event
//
//Multiframed event:
//1. Caller creates RC
//2. Function uses RC and don't marks it as EXECUTED but marks it as AGAIN
//3. Caller seeing, that RC is not EXECUTED but marked as AGAIN will stops thread executing and continue this at the next frame
//5. Function can mark own RC as EXECUTED for break the execution loop or can clear AGAIN and become a sleepy event
//6. When event becomes executed, caller will process this as single event
//
//Note: you can detect a first run of multiframed event by checking an "AGAIN" flag

#define	EVRS_ZERO 0
#define	EVRS_EXECUTED BIT(0)
#define	EVRS_HASRETURN BIT(1)
#define	EVRS_HANDLED BIT(2)
#define	EVRS_AGAIN BIT(3)
#define	EVRS_FAILED BIT(4)
#define	EVRS_ALL BIT(5)-1

class idEventReturn {
private:

	char						e_type;
	char						name[128];
	int							allocated;
	int							e_size;
	byte						*ReturnData;
	int							flags;
	unsigned int				tries;		//Each event call will incrase this value. Used for a multiframed actions
	idLinkList<idEventReturn>	ereturnNode;
	idClass						*creator;		//Link to a creator. Only for auto removing

	static idDynamicBlockAlloc<byte, 1024, 256> ReturnDataAllocator;
	static bool					initialized;

	void						_return( const void* Data, const int size );

public:

	void						Invalidate( void ); //Undo the return.
	
	void						ReturnString( const char *text ); //Set return data
	void						ReturnFloat( float value );
	void						ReturnInt( int value );
	void						ReturnVector( idVec3 const &vec );
	void						ReturnEntity( idEntity *ent );

	void						FinallyString( const char *text ); //Set return data and end execution
	void						FinallyFloat( float value );
	void						FinallyInt( int value );
	void						FinallyVector( idVec3 const &vec );
	void						FinallyEntity( idEntity *ent );

	void						Fails(void);

	int							GetReturnedInt( void );
	float						GetReturnedFloat( void );
	const idVec3				GetReturnedVec( void );
	const char*					GetReturnedString( void );
	int							GetReturnedEntNum( void );

	void						Free( void );
	bool						Checkout( int Status, bool noRemove = false );
	void						Flag( int xor, int msk );
	bool						Test( int xor, int msk );
	void						PrintFlags( void );
	bool						Load( idInterpreter* target, const char _type );
	const char*					Name( void );
	void						SetName( const char* _name );

	static idEventReturn		*Alloc( idClass* _creator = NULL );
	static idEventReturn		*Alloc( idClass* _creator, const char* _name );
	static void					Init( void );
	static void					Shutdown( void );
	static void					ClearReturns( void );
	static void					ListReturns( void );
	static void					FreeObj( const idClass *obj );
	static bool					isFreed( const idEventReturn *obj );

	static void					Save( idSave_I *savefile );					// archives object for save game file
	static void					Restore( idRestore_I *savefile );				// unarchives object from save game file
};

ID_INLINE 	const char*	idEventReturn::Name( void ) {
	return name;
}

class idEvent {
private:
	const idEventDef			*eventdef;
	idEventReturn				*returnContext; //Can be NULL
	byte						*data;
	int							time;
	idClass						*object;
	const idTypeInfo			*typeinfo;

	idLinkList<idEvent>			eventNode;

	static idDynamicBlockAlloc<byte, 16 * 1024, 256> eventDataAllocator;

public:
	static bool					initialized;

								~idEvent();

	static idEvent				*Alloc( const idEventDef *evdef, idEventReturn *RC, int numargs, va_list args );
	static void					CopyArgs( const idEventDef *evdef, int numargs, va_list args, int data[ D_EVENT_MAXARGS ]  );
	
	void						Free( void );
	void						Schedule( idClass *object, const idTypeInfo *cls, int time );
	byte						*GetData( void );

	static void					CancelEvents( const idClass *obj, const idEventDef *evdef = NULL );
	static void					ClearEventList( void );
	static void					ServiceEvents( void );
	static void					Init( void );
	static void					Shutdown( void );

	// save games
	static void					Save( idSave_I *savefile );					// archives object for save game file
	static void					Restore( idRestore_I *savefile );				// unarchives object from save game file
	static void					SaveTrace( idSave_I *savefile, const trace_t &trace );
	static void					RestoreTrace( idRestore_I *savefile, trace_t &trace );
};

/*
================
idEvent::GetData
================
*/
ID_INLINE byte *idEvent::GetData( void ) {
	return data;
}

/*
================
idEventDef::GetName
================
*/
ID_INLINE const char *idEventDef::GetName( void ) const {
	return name;
}

/*
================
idEventDef::GetArgFormat
================
*/
ID_INLINE const char *idEventDef::GetArgFormat( void ) const {
	return formatspec;
}

/*
================
idEventDef::GetFormatspecIndex
================
*/
ID_INLINE unsigned int idEventDef::GetFormatspecIndex( void ) const {
	return formatspecIndex;
}

/*
================
idEventDef::GetReturnType
================
*/
ID_INLINE char idEventDef::GetReturnType( void ) const {
	return returnType;
}

/*
================
idEventDef::GetNumArgs
================
*/
ID_INLINE int idEventDef::GetNumArgs( void ) const {
	return numargs;
}

/*
================
idEventDef::GetArgSize
================
*/
ID_INLINE size_t idEventDef::GetArgSize( void ) const {
	return argsize;
}

/*
================
idEventDef::GetArgOffset
================
*/
ID_INLINE int idEventDef::GetArgOffset( int arg ) const {
	assert( ( arg >= 0 ) && ( arg < D_EVENT_MAXARGS ) );
	return argOffset[ arg ];
}

/*
================
idEventDef::GetEventNum
================
*/
ID_INLINE int idEventDef::GetEventNum( void ) const {
	return eventnum;
}

#endif /* !__SYS_EVENT_H__ */
