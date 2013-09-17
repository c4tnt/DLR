// Copyright (C) 2004 Id Software, Inc.
//
/*
sys_event.cpp

Event are used for scheduling tasks and for linking script commands.

*/

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

//#define CREATE_EVENT_CODE

/***********************************************************************

  idEventDef

***********************************************************************/

idEventDef *idEventDef::eventDefList[MAX_EVENTS];
int idEventDef::numEventDefs = 0;

static bool eventError = false;
static char eventErrorMsg[ 128 ];

/*
================
idEventDef::idEventDef
================
*/
idEventDef::idEventDef( const char *command, const char *formatspec, char returnType ) {
	idEventDef		*ev;
	int				i;
	unsigned int	bits;

	assert( command );
	assert( !idEvent::initialized );

	// Allow NULL to indicate no args, but always store it as ""
	// so we don't have to check for it.
	if ( !formatspec ) {
		formatspec = "";
	}
	
	this->name = command;
	this->formatspec = formatspec;
	this->returnType = returnType;

	numargs = strlen( formatspec );
	assert( numargs <= D_EVENT_MAXARGS );
	if ( numargs > D_EVENT_MAXARGS ) {
		eventError = true;
		sprintf( eventErrorMsg, "idEventDef::idEventDef : Too many args for '%s' event.", name );
		return;
	}

	// make sure the format for the args is valid, calculate the formatspecindex, and the offsets for each arg
	bits = 0;
	argsize = 0;
	memset( argOffset, 0, sizeof( argOffset ) );
	for( i = 0; i < numargs; i++ ) {
		argOffset[ i ] = argsize;
		switch( formatspec[ i ] ) {
		case D_EVENT_FLOAT :
			bits |= 1 << i;
			argsize += sizeof( float );
			break;

		case D_EVENT_INTEGER :
			argsize += sizeof( int );
			break;

		case D_EVENT_VECTOR :
			argsize += sizeof( idVec3 );
			break;

		case D_EVENT_STRING :
			argsize += MAX_STRING_LEN;
			break;

		case D_EVENT_ENTITY :
			argsize += sizeof( idEntityPtr<idEntity> );
			break;

		case D_EVENT_ENTITY_NULL :
			argsize += sizeof( idEntityPtr<idEntity> );
			break;

		case D_EVENT_TRACE :
			argsize += sizeof( trace_t ) + MAX_STRING_LEN + sizeof( bool );
			break;

		case D_EVENT_LIST:
			break;

		default :
			eventError = true;
			sprintf( eventErrorMsg, "idEventDef::idEventDef : Invalid arg format '%s' string for '%s' event.", formatspec, name );
			return;
			break;
		}
	}

	// calculate the formatspecindex
	formatspecIndex = ( 1 << ( numargs + D_EVENT_MAXARGS ) ) | bits;

	// go through the list of defined events and check for duplicates
	// and mismatched format strings
	eventnum = numEventDefs;
	for( i = 0; i < eventnum; i++ ) {
		ev = eventDefList[ i ];
		if ( strcmp( command, ev->name ) == 0 ) {
			if ( strcmp( formatspec, ev->formatspec ) != 0 ) {
				eventError = true;
				sprintf( eventErrorMsg, "idEvent '%s' defined twice with same name but differing format strings ('%s'!='%s').",
					command, formatspec, ev->formatspec );
				return;
			}

			if ( ev->returnType != returnType ) {
				eventError = true;
				sprintf( eventErrorMsg, "idEvent '%s' defined twice with same name but differing return types ('%c'!='%c').",
					command, returnType, ev->returnType );
				return;
			}
			// Don't bother putting the duplicate event in list.
			eventnum = ev->eventnum;
			return;
		}
	}

	ev = this;

	if ( numEventDefs >= MAX_EVENTS ) {
		eventError = true;
		sprintf( eventErrorMsg, "numEventDefs >= MAX_EVENTS" );
		return;
	}
	eventDefList[numEventDefs] = ev;
	numEventDefs++;
}

/*
================
idEventDef::NumEventCommands
================
*/
int	idEventDef::NumEventCommands( void ) {
	return numEventDefs;
}

/*
================
idEventDef::GetEventCommand
================
*/
const idEventDef *idEventDef::GetEventCommand( int eventnum ) {
	return eventDefList[ eventnum ];
}

/*
================
idEventDef::FindEvent
================
*/
const idEventDef *idEventDef::FindEvent( const char *name ) {
	idEventDef	*ev;
	int			num;
	int			i;

	assert( name );

	num = numEventDefs;
	for( i = 0; i < num; i++ ) {
		ev = eventDefList[ i ];
		if ( strcmp( name, ev->name ) == 0 ) {
			return ev;
		}
	}

	return NULL;
}

/***********************************************************************

  idEvent

***********************************************************************/

static idLinkList<idEvent> FreeEvents;
static idLinkList<idEvent> EventQueue;
static idEvent EventPool[ MAX_EVENTS ];

bool idEvent::initialized = false;

idDynamicBlockAlloc<byte, 16 * 1024, 256>	idEvent::eventDataAllocator;

/*
================
idEvent::~idEvent()
================
*/
idEvent::~idEvent() {
	Free();
}

/*
================
idEvent::Alloc
================
*/
idEvent *idEvent::Alloc( const idEventDef *evdef, idEventReturn *RC, int numargs, va_list args ) {
	idEvent		*ev;
	size_t		size;
	const char	*format;
	idEventArg	*arg;
	byte		*dataPtr;
	int			i;
	const char	*materialName;

	if ( FreeEvents.IsListEmpty() ) {
		gameLocal.Error( "idEvent::Alloc : No more free events" );
	}

	ev = FreeEvents.Next();
	ev->eventNode.Remove();

	ev->eventdef = evdef;
	ev->returnContext = RC;

	if ( numargs != evdef->GetNumArgs() ) {
		gameLocal.Error( "idEvent::Alloc : Wrong number of args for '%s' event.", evdef->GetName() );
	}

	size = evdef->GetArgSize();
	if ( size ) {
		ev->data = eventDataAllocator.Alloc( size );
		memset( ev->data, 0, size );
	} else {
		ev->data = NULL;
	}

	format = evdef->GetArgFormat();
	for( i = 0; i < numargs; i++ ) {
		arg = va_arg( args, idEventArg * );
		if ( format[ i ] != arg->type ) {
			// when NULL is passed in for an entity, it gets cast as an integer 0, so don't give an error when it happens
			if ( !( ( ( format[ i ] == D_EVENT_TRACE ) || ( format[ i ] == D_EVENT_ENTITY ) ) && ( arg->type == 'd' ) && ( arg->value == 0 ) ) ) {
				gameLocal.Error( "idEvent::Alloc : Wrong type passed in for arg # %d on '%s' event.", i, evdef->GetName() );
			}
		}

		dataPtr = &ev->data[ evdef->GetArgOffset( i ) ];

		switch( format[ i ] ) {
		case D_EVENT_FLOAT :
		case D_EVENT_INTEGER :
			*reinterpret_cast<int *>( dataPtr ) = arg->value;
			break;

		case D_EVENT_VECTOR :
			if ( arg->value ) {
				*reinterpret_cast<idVec3 *>( dataPtr ) = *reinterpret_cast<const idVec3 *>( arg->value );
			}
			break;

		case D_EVENT_STRING :
			if ( arg->value ) {
				idStr::Copynz( reinterpret_cast<char *>( dataPtr ), reinterpret_cast<const char *>( arg->value ), MAX_STRING_LEN );
			}
			break;

		case D_EVENT_ENTITY :
		case D_EVENT_ENTITY_NULL :
			*reinterpret_cast< idEntityPtr<idEntity> * >( dataPtr ) = reinterpret_cast<idEntity *>( arg->value );
			break;

		case D_EVENT_TRACE :
			if ( arg->value ) {
				*reinterpret_cast<bool *>( dataPtr ) = true;
				*reinterpret_cast<trace_t *>( dataPtr + sizeof( bool ) ) = *reinterpret_cast<const trace_t *>( arg->value );

				// save off the material as a string since the pointer won't be valid in save games.
				// since we save off the entire trace_t structure, if the material is NULL here,
				// it will be NULL when we process it, so we don't need to save off anything in that case.
				if ( reinterpret_cast<const trace_t *>( arg->value )->c.material ) {
					materialName = reinterpret_cast<const trace_t *>( arg->value )->c.material->GetName();
					idStr::Copynz( reinterpret_cast<char *>( dataPtr + sizeof( bool ) + sizeof( trace_t ) ), materialName, MAX_STRING_LEN );
				}
			} else {
				*reinterpret_cast<bool *>( dataPtr ) = false;
			}
			break;
		case D_EVENT_LIST:
			break;
		default :
			gameLocal.Error( "idEvent::Alloc : Invalid arg format '%s' string for '%s' event.", format, evdef->GetName() );
			break;
		}
	}

	return ev;
}

/*
================
idEvent::CopyArgs
================
*/
void idEvent::CopyArgs( const idEventDef *evdef, int numargs, va_list args, int data[ D_EVENT_MAXARGS ] ) {
	int			i;
	const char	*format;
	idEventArg	*arg;

	format = evdef->GetArgFormat();
	if ( numargs != evdef->GetNumArgs() ) {
		gameLocal.Error( "idEvent::CopyArgs : Wrong number of args for '%s' event.", evdef->GetName() );
	}

	for( i = 0; i < numargs; i++ ) {
		arg = va_arg( args, idEventArg * );
		if ( format[ i ] != arg->type ) {
			// when NULL is passed in for an entity, it gets cast as an integer 0, so don't give an error when it happens
			if ( !( ( ( format[ i ] == D_EVENT_TRACE ) || ( format[ i ] == D_EVENT_ENTITY ) ) && ( arg->type == 'd' ) && ( arg->value == 0 ) ) ) {
				gameLocal.Error( "idEvent::CopyArgs : Wrong type passed in for arg # %d on '%s' event.", i, evdef->GetName() );
			}
		}

		data[ i ] = arg->value;
	}
}

/*
================
idEvent::Free
================
*/
void idEvent::Free( void ) {
	if ( data ) {
		eventDataAllocator.Free( data );
		data = NULL;
	}
	
	if ( returnContext ) {
		returnContext->Checkout(EVRS_EXECUTED);
	}

	eventdef	= NULL;
	time		= 0;
	object		= NULL;
	typeinfo	= NULL;
	returnContext = NULL;

	eventNode.SetOwner( this );
	eventNode.AddToEnd( FreeEvents );
}

/*
================
idEvent::Schedule
================
*/
void idEvent::Schedule( idClass *obj, const idTypeInfo *type, int time ) {
	idEvent *event;

	assert( initialized );
	if ( !initialized ) {
		return;
	}

	object = obj;
	typeinfo = type;

	// wraps after 24 days...like I care. ;)
	this->time = gameLocal.time + time;

	eventNode.Remove();

	event = EventQueue.Next();
	while( ( event != NULL ) && ( this->time >= event->time ) ) {
		event = event->eventNode.Next();
	}

	if ( event ) {
		eventNode.InsertBefore( event->eventNode );
	} else {
		eventNode.AddToEnd( EventQueue );
	}
}

/*
================
idEvent::CancelEvents
================
*/
void idEvent::CancelEvents( const idClass *obj, const idEventDef *evdef ) {
	idEvent *event;
	idEvent *next;

	if ( !initialized ) {
		return;
	}

	for( event = EventQueue.Next(); event != NULL; event = next ) {
		next = event->eventNode.Next();
		if ( event->object == obj ) {
			if ( !evdef || ( evdef == event->eventdef ) ) {
				event->Free();
			}
		}
	}
}

/*
================
idEvent::ClearEventList
================
*/
void idEvent::ClearEventList( void ) {
	int i;

	//
	// initialize lists
	//
	FreeEvents.Clear();
	EventQueue.Clear();
   
	// 
	// add the events to the free list
	//
	for( i = 0; i < MAX_EVENTS; i++ ) {
		EventPool[ i ].Free();
	}
}

/*
================
idEvent::ServiceEvents
================
*/
void idEvent::ServiceEvents( void ) {
	idEvent		*event;
	int			num;
	int			args[ D_EVENT_MAXARGS ];
	int			offset;
	int			i;
	int			numargs;
	const char	*formatspec;
	trace_t		**tracePtr;
	const idEventDef *ev;
	byte		*data;
	const char  *materialName;

	num = 0;

	while( !EventQueue.IsListEmpty() ) {
		event = EventQueue.Next();
		assert( event );

		if ( event->time > gameLocal.time ) {
			break;
		}

		// copy the data into the local args array and set up pointers
		ev = event->eventdef;
		formatspec = ev->GetArgFormat();
		numargs = ev->GetNumArgs();
		for( i = 0; i < numargs; i++ ) {
			offset = ev->GetArgOffset( i );
			data = event->data;
			switch( formatspec[ i ] ) {
			case D_EVENT_FLOAT :
			case D_EVENT_INTEGER :
				args[ i ] = *reinterpret_cast<int *>( &data[ offset ] );
				break;

			case D_EVENT_VECTOR :
				*reinterpret_cast<idVec3 **>( &args[ i ] ) = reinterpret_cast<idVec3 *>( &data[ offset ] );
				break;

			case D_EVENT_STRING :
				*reinterpret_cast<const char **>( &args[ i ] ) = reinterpret_cast<const char *>( &data[ offset ] );
				break;

			case D_EVENT_ENTITY :
			case D_EVENT_ENTITY_NULL :
				*reinterpret_cast<idEntity **>( &args[ i ] ) = reinterpret_cast< idEntityPtr<idEntity> * >( &data[ offset ] )->GetEntity();
				break;

			case D_EVENT_TRACE :
				tracePtr = reinterpret_cast<trace_t **>( &args[ i ] );
				if ( *reinterpret_cast<bool *>( &data[ offset ] ) ) {
					*tracePtr = reinterpret_cast<trace_t *>( &data[ offset + sizeof( bool ) ] );

					if ( ( *tracePtr )->c.material != NULL ) {
						// look up the material name to get the material pointer
						materialName = reinterpret_cast<const char *>( &data[ offset + sizeof( bool ) + sizeof( trace_t ) ] );
						( *tracePtr )->c.material = declManager->FindMaterial( materialName, true );
					}
				} else {
					*tracePtr = NULL;
				}
				break;
			case D_EVENT_LIST:
				break;
			default:
				gameLocal.Error( "idEvent::ServiceEvents : Invalid arg format '%s' string for '%s' event.", formatspec, ev->GetName() );
			}
		}

		// the event is removed from its list so that if then object
		// is deleted, the event won't be freed twice
		event->eventNode.Remove();
		assert( event->object );
		event->object->ProcessEventArgPtr( ev, args, event->returnContext );


#if 0
		// event functions may never leave return values on the FPU stack
		// enable this code to check if any event call left values on the FPU stack
		if ( !sys->FPU_StackIsEmpty() ) {
			gameLocal.Error( "idEvent::ServiceEvents %d: %s left a value on the FPU stack\n", num, ev->GetName() );
		}
#endif

		// return the event to the free list
		event->Free();

		// Don't allow ourselves to stay in here too long.  An abnormally high number
		// of events being processed is evidence of an infinite loop of events.
		num++;
		if ( num > MAX_EVENTS ) {
			gameLocal.Error( "Event overflow.  Possible infinite loop in script." );
		}
	}
	if ( num > MAX_EVENTSPERFRAME ) {
		gameLocal.Warning( "Event overflow warning. Events count: %d", num );
	}

}

/*
================
idEvent::Init
================
*/
void idEvent::Init( void ) {
	gameLocal.Printf( "Initializing event system\n" );

	if ( eventError ) {
		gameLocal.Error( "%s", eventErrorMsg );
	}

#ifdef CREATE_EVENT_CODE
	void CreateEventCallbackHandler();
	CreateEventCallbackHandler();
	gameLocal.Error( "Wrote event callback handler" );
#endif

	if ( initialized ) {
		gameLocal.Printf( "...already initialized\n" );
		ClearEventList();
		return;
	}

	ClearEventList();

	eventDataAllocator.Init();

	gameLocal.Printf( "...%i event definitions\n", idEventDef::NumEventCommands() );

	// the event system has started
	initialized = true;
}

/*
================
idEvent::Shutdown
================
*/
void idEvent::Shutdown( void ) {
	gameLocal.Printf( "Shutdown event system\n" );

	if ( !initialized ) {
		gameLocal.Printf( "...not started\n" );
		return;
	}

	ClearEventList();
	
	eventDataAllocator.Shutdown();

	// say it is now shutdown
	initialized = false;
}

/*
================
idEvent::Save
================
*/
void idEvent::Save( idSave_I *savefile ) {
	char *str;
	int i, size;
	idEvent	*event;
	byte *dataPtr;
	bool validTrace;
	const char	*format;

	savefile->WriteInt( EventQueue.Num() );

	event = EventQueue.Next();
	while( event != NULL ) {
		savefile->WriteInt( event->time );
		savefile->WriteString( event->eventdef->GetName() );
		savefile->WriteString( event->typeinfo->classname );
		savefile->WriteObject( event->object );
		savefile->WriteInt( event->eventdef->GetArgSize() );
//		savefile->Write( event->data, event->eventdef->GetArgSize() );
		format = event->eventdef->GetArgFormat();
		for ( i = 0, size = 0; i < event->eventdef->GetNumArgs(); ++i) {
			dataPtr = &event->data[ event->eventdef->GetArgOffset( i ) ];
			switch( format[ i ] ) {
				case D_EVENT_FLOAT :
					savefile->WriteFloat( *reinterpret_cast<float *>( dataPtr ) );
					size += sizeof( float );
					break;
				case D_EVENT_INTEGER :
				case D_EVENT_ENTITY :
				case D_EVENT_ENTITY_NULL :
					savefile->WriteInt( *reinterpret_cast<int *>( dataPtr ) );
					size += sizeof( int );
					break;
				case D_EVENT_VECTOR :
					savefile->WriteVec3( *reinterpret_cast<idVec3 *>( dataPtr ) );
					size += sizeof( idVec3 );
					break;
				case D_EVENT_TRACE :
					validTrace = *reinterpret_cast<bool *>( dataPtr );
					savefile->WriteBool( validTrace );
					size += sizeof( bool );
					if ( validTrace ) {
						size += sizeof( trace_t );
						const trace_t &t = *reinterpret_cast<trace_t *>( dataPtr + sizeof( bool ) );
						SaveTrace( savefile, t );
						if ( t.c.material ) {
							size += MAX_STRING_LEN;
							str = reinterpret_cast<char *>( dataPtr + sizeof( bool ) + sizeof( trace_t ) );
							savefile->WriteString( str );
						}
					}
					break;
				default:
					break;
			}
		}
		assert( size == event->eventdef->GetArgSize() );
		event = event->eventNode.Next();
	}
}

/*
================
idEvent::Restore
================
*/
void idEvent::Restore( idRestore_I *savefile ) {
	char    *str;
	int		num, argsize, i, j, size;
	idStr	r_str;

	idStr	name;
	byte *dataPtr;
	idEvent	*event;
	const char	*format;

	savefile->ReadInt( num );

	for ( i = 0; i < num; i++ ) {
		if ( FreeEvents.IsListEmpty() ) {
			gameLocal.Error( "idEvent::Restore : No more free events" );
		}

		event = FreeEvents.Next();
		event->eventNode.Remove();
		event->eventNode.AddToEnd( EventQueue );

		savefile->ReadInt( event->time );

		// read the event name
		savefile->ReadString( name );
		event->eventdef = idEventDef::FindEvent( name );
		if ( !event->eventdef ) {
			savefile->Error( "idEvent::Restore: unknown event '%s'", name.c_str() );
		}

		// read the classtype
		savefile->ReadString( name );
		event->typeinfo = idClass::GetClass( name );
		if ( !event->typeinfo ) {
			savefile->Error( "idEvent::Restore: unknown class '%s' on event '%s'", name.c_str(), event->eventdef->GetName() );
		}

		savefile->ReadObject( event->object );

		// read the args
		savefile->ReadInt( argsize );
		if ( argsize != event->eventdef->GetArgSize() ) {
			savefile->Error( "idEvent::Restore: arg size (%d) doesn't match saved arg size(%d) on event '%s'", event->eventdef->GetArgSize(), argsize, event->eventdef->GetName() );
		}
		if ( argsize ) {
			event->data = eventDataAllocator.Alloc( argsize );
			//savefile->Read( event->data, argsize );
			format = event->eventdef->GetArgFormat();
			assert( format );
			for ( j = 0, size = 0; j < event->eventdef->GetNumArgs(); ++j) {
				dataPtr = &event->data[ event->eventdef->GetArgOffset( j ) ];
				switch( format[ j ] ) {
					case D_EVENT_FLOAT :
						savefile->ReadFloat( *reinterpret_cast<float *>( dataPtr ) );
						size += sizeof( float );
						break;
					case D_EVENT_INTEGER :
					case D_EVENT_ENTITY :
					case D_EVENT_ENTITY_NULL :
						savefile->ReadInt( *reinterpret_cast<int *>( dataPtr ) );
						size += sizeof( int );
						break;
					case D_EVENT_VECTOR :
						savefile->ReadVec3( *reinterpret_cast<idVec3 *>( dataPtr ) );
						size += sizeof( idVec3 );
						break;
					case D_EVENT_TRACE :
						savefile->ReadBool( *reinterpret_cast<bool *>( dataPtr ) );
						size += sizeof( bool );
						if ( *reinterpret_cast<bool *>( dataPtr ) ) {
							size += sizeof( trace_t );
							trace_t &t = *reinterpret_cast<trace_t *>( dataPtr + sizeof( bool ) );
							RestoreTrace( savefile,  t) ;
							if ( t.c.material ) {
								size += MAX_STRING_LEN;
								str = reinterpret_cast<char *>( dataPtr + sizeof( bool ) + sizeof( trace_t ) );
								savefile->ReadString( r_str );
								memset( str, 0, MAX_STRING_LEN );
								idStr::Copynz( str, r_str.c_str(), MAX_STRING_LEN );
							}
						}
						break;
					default:
						break;
				}
			}
			assert( size == event->eventdef->GetArgSize() );

		} else {
			event->data = NULL;
		}
	}
}

/*
 ================
 idEvent::ReadTrace
 
 idRestoreGame has a ReadTrace procedure, but unfortunately idEvent wants the material
 string name at the of the data structure rather than in the middle
 ================
 */
void idEvent::RestoreTrace( idRestore_I *savefile, trace_t &trace ) {
	savefile->ReadFloat( trace.fraction );
	savefile->ReadVec3( trace.endpos );
	savefile->ReadMat3( trace.endAxis );
	savefile->ReadInt( (int&)trace.c.type );
	savefile->ReadVec3( trace.c.point );
	savefile->ReadVec3( trace.c.normal );
	savefile->ReadFloat( trace.c.dist );
	savefile->ReadInt( trace.c.contents );
	savefile->ReadInt( (int&)trace.c.material );
	savefile->ReadInt( trace.c.contents );
	savefile->ReadInt( trace.c.modelFeature );
	savefile->ReadInt( trace.c.trmFeature );
	savefile->ReadInt( trace.c.id );
}

/*
 ================
 idEvent::WriteTrace

 idSaveGame has a WriteTrace procedure, but unfortunately idEvent wants the material
 string name at the of the data structure rather than in the middle
================
 */
void idEvent::SaveTrace( idSave_I *savefile, const trace_t &trace ) {
	savefile->WriteFloat( trace.fraction );
	savefile->WriteVec3( trace.endpos );
	savefile->WriteMat3( trace.endAxis );
	savefile->WriteInt( trace.c.type );
	savefile->WriteVec3( trace.c.point );
	savefile->WriteVec3( trace.c.normal );
	savefile->WriteFloat( trace.c.dist );
	savefile->WriteInt( trace.c.contents );
	savefile->WriteInt( (int&)trace.c.material );
	savefile->WriteInt( trace.c.contents );
	savefile->WriteInt( trace.c.modelFeature );
	savefile->WriteInt( trace.c.trmFeature );
	savefile->WriteInt( trace.c.id );
}

/***********************************************************************

  idEventReturn
  c4tnt

***********************************************************************/

static idLinkList<idEventReturn> FreeEverets;
static idEventReturn ReturnCxPool[ MAX_RETURNS ];

bool idEventReturn::initialized = false;

idDynamicBlockAlloc<byte, 1024, 256>	idEventReturn::ReturnDataAllocator;

/*
================
idEventReturn::Init
================
*/
void idEventReturn::Init( void ) {
	gameLocal.Printf( "Initializing event return system\n" );

	if ( initialized ) {
		gameLocal.Printf( "...already initialized\n" );
		return;
	}
	ReturnDataAllocator.Init();
	ClearReturns();

	// the event system has started
	initialized = true;
}

/*
================
idEventReturn::Shutdown
================
*/
void idEventReturn::Shutdown( void ) {
	gameLocal.Printf( "Shutdown event return system\n" );

	if ( !initialized ) {
		gameLocal.Printf( "...not started\n" );
		return;
	}

	ClearReturns();
	ReturnDataAllocator.Shutdown();

	// say it is now shutdown
	initialized = false;
}

/*
================
idEventReturn::SetName
================
*/
void	idEventReturn::SetName( const char* _name ) {
	strncpy( name, _name, 127 );
	name[127] = 0;
}
/*
================
idEventReturn::Alloc
================
*/
idEventReturn *idEventReturn::Alloc( idClass* _creator ) {
	idEventReturn	*evr;

	if ( !initialized ) {
		gameLocal.Printf( "EVR disabled\n" );
		return NULL;
	}

	if ( FreeEverets.IsListEmpty() ) {
		gameLocal.Error( "idEventReturn::Alloc : No more free RCs" );
	}

	evr = FreeEverets.Next();
	evr->ereturnNode.Remove();

	evr->e_type = D_EVENT_VOID;
	evr->ReturnData = NULL;
	evr->flags = EVRS_ZERO;
	evr->creator = _creator;
	evr->tries = 0;
	evr->allocated = 0;
	if (_creator) {
		strncpy( evr->name, _creator->GetClassname(), 127 );
		evr->name[127] = 0;
	} else {
		strncpy( evr->name, "<no creator>", 127 );
		evr->name[127] = 0;
	}
	return evr;
}

/*
================
idEventReturn::Alloc
================
*/
idEventReturn *idEventReturn::Alloc( idClass* _creator, const char* _name ) {
	idEventReturn	*evr;

	if ( !initialized ) {
		gameLocal.Printf( "EVR disabled\n" );
		return NULL;
	}

	if ( FreeEverets.IsListEmpty() ) {
		gameLocal.Error( "idEventReturn::Alloc : No more free RCs" );
	}

	evr = FreeEverets.Next();
	evr->ereturnNode.Remove();

	evr->e_type = D_EVENT_VOID;
	evr->ReturnData = NULL;
	evr->flags = EVRS_ZERO;
	evr->creator = _creator;
	evr->tries = 0;
	evr->allocated = 0;
	if (_name) {
		strncpy( evr->name, _name, 127 );
		evr->name[127] = 0;
	} else if (_creator) {
		strncpy( evr->name, _creator->GetClassname(), 127 );
		evr->name[127] = 0;
	} else {
		strncpy( evr->name, "<no creator>", 127 );
		evr->name[127] = 0;
	}

	return evr;
}

void idEventReturn::ClearReturns( void ) {
int i;
	//
	// initialize lists
	//
	FreeEverets.Clear();
   
	// 
	// add the events to the free list
	//
	for( i = 0; i < MAX_RETURNS; i++ ) {
		ReturnCxPool[ i ].Free();
	}
}

void idEventReturn::ListReturns( void ) {
int i;
int free;

	if ( !initialized ) {
		gameLocal.Printf( "EVR disabled\n" );
		return;
	}
   
	// 
	// add the events to the free list
	//
	gameLocal.Printf( "------------------Delayed return list-----------------\n" );
	free = 0;
	for( i = 0; i < MAX_RETURNS; i++ ) {
		if ( !idEventReturn::isFreed( &ReturnCxPool[ i ] ) ) {
			gameLocal.Printf( "%d: (%s) with flags:\n", i, ReturnCxPool[ i ].Name() );
			ReturnCxPool[ i ].PrintFlags();
			gameLocal.Printf( "\n" );
		} else {
			free++;
		}
	}
	gameLocal.Printf( "--------------Delayed return list end-----------------\n" );
	gameLocal.Printf( "free returns %d\n", free );
}

/*
================
idEvent::CancelEvents
================
*/
void idEventReturn::FreeObj( const idClass *obj ) {
int i;
	if ( !initialized ) {
		return;
	}

	for( i = 0; i < MAX_RETURNS; i++ ) {
		if (ReturnCxPool[ i ].creator == obj) ReturnCxPool[ i ].Free();
	}
}

/*
================
idEventReturn::Invalidate
================
*/
void idEventReturn::Invalidate( void ) {
	if (ReturnData)	ReturnDataAllocator.Free( ReturnData );
	ReturnData = NULL;
	e_size = 0;
	e_type = D_EVENT_VOID;
	allocated = 0;
	Flag(0, ~(EVRS_HASRETURN|EVRS_FAILED));
}

/*
================
idEventReturn::ReturnString
================
*/
void idEventReturn::ReturnString( const char *text ) {
	int szLen = strlen(text);
	e_type = D_EVENT_STRING;
	_return(text,sizeof(char)*szLen+1);
	((char*)ReturnData)[szLen] = 0;
}

/*
================
idEventReturn::ReturnFloat
================
*/
void idEventReturn::ReturnFloat( float value ) {
	e_type = D_EVENT_FLOAT;
	_return(&value,sizeof(float));
}

/*
================
idEventReturn::ReturnInt
================
*/
void idEventReturn::ReturnInt( int value ) {
	e_type = D_EVENT_INTEGER;
	_return(&value,sizeof(int));
}

/*
================
idEventReturn::ReturnVector
================
*/
void idEventReturn::ReturnVector( idVec3 const &vec ) {
	e_type = D_EVENT_VECTOR;
	_return(&vec,sizeof(idVec3));
}

/*
================
idEventReturn::_return
================
*/
void idEventReturn::_return(const void* data, const int size ) {
	if ( ReturnData ) {
		if ( size > allocated ) {
			ReturnDataAllocator.Free( ReturnData );
			ReturnData = NULL;
			allocated = 0;
			e_size = 0;
		}
		if ( data ) {
			if ( !ReturnData ) {
				ReturnData = ReturnDataAllocator.Alloc( size );
				allocated = size;
			}
			e_size = size;
			memcpy( ReturnData, data, size );
			Flag(EVRS_HASRETURN, ~EVRS_HASRETURN);
		}
	} else {
		if ( data ) {
			ReturnData = ReturnDataAllocator.Alloc( size );
			allocated = size;
			e_size = size;
			memcpy( ReturnData, data, size );
			Flag(EVRS_HASRETURN, ~EVRS_HASRETURN);
		}
	}
}

/*
================
idEventReturn::Load
Push return value to the interpreter
================
*/
bool idEventReturn::Load( idInterpreter* target, const char _type ) {
	
	float floatVal;
	idToken token;

	if ( _type == D_EVENT_VOID || _type == D_EVENT_CONTEXT ) return true;

	if ( Test( EVRS_FAILED, EVRS_FAILED ) ) {
		// return failsafe var.
		switch (_type) {
			case D_EVENT_INTEGER:
				gameLocal.Warning( "idEventReturn::LOAD int unsupported by script", e_type );
				return false;
			case D_EVENT_FLOAT:
				target->Push( 0.0f );
				return true;
			case D_EVENT_VECTOR:
				target->Push( idVec3( ) );
				return true;
			case D_EVENT_STRING:
				target->PushString( "" );
				return true;
			case D_EVENT_ENTITY:
				target->Push( (int)0 );
				return true;
			case D_EVENT_ENTITY_NULL:
				target->Push( (int)0 );
				return true;
			case D_EVENT_LIST:
				target->Push( (int)0 );
				return true;
			default:
				gameLocal.Error( "idEventReturn::Load unknown export type: %c", e_type );
				return false;
		}
	}

	if ( e_type == D_EVENT_VOID || e_type == D_EVENT_CONTEXT ) return false;
	if ( !ReturnData ) return false;

	switch (_type) {
		case D_EVENT_INTEGER:
			gameLocal.Warning( "idEventReturn::LOAD int unsupported by script", e_type );
			return false;
		case D_EVENT_FLOAT:
			switch (e_type) {
				case D_EVENT_INTEGER:
					floatVal = (float)*(int*)ReturnData;
					target->Push( floatVal );
					break;
				case D_EVENT_FLOAT:
					floatVal = *(float*)ReturnData;
					target->Push( floatVal );
					break;
				case D_EVENT_VECTOR:
					floatVal = ((idVec3*)ReturnData)->Length();
					target->Push( floatVal );
					break;
				case D_EVENT_STRING:
					token = (char*)ReturnData;
					target->Push( token.GetFloatValue() );
					break;
				default:
					gameLocal.Warning( "idEventReturn::Bad internal type: %c", e_type );
					return false;
			}
			break;
		case D_EVENT_VECTOR:
			switch (e_type) {
				case D_EVENT_INTEGER:
					floatVal = (float)*(int*)ReturnData;
					target->Push( idVec3( floatVal, floatVal, floatVal ) );
					break;
				case D_EVENT_FLOAT:
					floatVal = *(float*)ReturnData;
					target->Push( idVec3( floatVal, floatVal, floatVal ) );
					break;
				case D_EVENT_VECTOR:
					target->Push( *(idVec3*)ReturnData );
					break;
				default:
					gameLocal.Warning( "idEventReturn::Bad internal type: %c", e_type );
					return false;
			}
			break;
		case D_EVENT_STRING:
			switch (e_type) {
				case D_EVENT_INTEGER:
					target->PushString( va( "%d", (int*)ReturnData ) );
					break;
				case D_EVENT_FLOAT:
					target->PushString( va( "%f", (float*)ReturnData ) );
					break;
				case D_EVENT_VECTOR:
					target->PushString( ( *(idVec3*)ReturnData ).ToString() );
					break;
				case D_EVENT_STRING:
					target->PushString( (char*)ReturnData );
					break;
				default:
					gameLocal.Warning( "idEventReturn::Bad internal type: %c", e_type );
					return false;
			}
			break;
		case D_EVENT_ENTITY:
			if ( e_type != D_EVENT_ENTITY && e_type != D_EVENT_ENTITY_NULL ) {
				gameLocal.Warning( "idEventReturn::Bad internal type: %c", e_type );
				return false;
			}
			if ( *(int*)ReturnData == 0 ) {
				gameLocal.Error( "idEventReturn::NULL entity was returned", e_type );
			} else {
				target->Push( *(int*)ReturnData );
			}
			break;
		case D_EVENT_ENTITY_NULL:
			if ( e_type != D_EVENT_ENTITY && e_type != D_EVENT_ENTITY_NULL ) {
				gameLocal.Warning( "idEventReturn::Bad internal type: %c", e_type );
				return false;
			}
			target->Push( *(int*)ReturnData );
			break;
		case D_EVENT_LIST:
			gameLocal.Error( "idEventReturn::Trying to return idList", e_type );
			break;
		default:
			gameLocal.Error( "idEventReturn::Load unknown export type: %c", e_type );
			return false;
	}
	return true;
}

/*
================
idEventReturn::Fails
================
*/
void idEventReturn::Fails(void) {
	Flag(EVRS_FAILED, ~EVRS_FAILED);
	Checkout( EVRS_EXECUTED );
}
/*
================
idEventReturn::ReturnEntity
================
*/
void idEventReturn::ReturnEntity( idEntity *ent ) {
int EntityNumber;

	e_type = D_EVENT_ENTITY;

	EntityNumber = ( ent )?(ent->entityNumber + 1):0;
	_return(&EntityNumber,sizeof(int));
}

/*
================
idEventReturn::FinallyString
================
*/
void idEventReturn::FinallyString( const char *text ) {
	ReturnString( text );
	Checkout( EVRS_EXECUTED );
}

/*
================
idEventReturn::FinallyFloat
================
*/
void idEventReturn::FinallyFloat( float value ) {
	ReturnFloat( value );
	Checkout( EVRS_EXECUTED );
}

/*
================
idEventReturn::FinallyInt
================
*/
void idEventReturn::FinallyInt( int value ) {
	ReturnInt( value );
	Checkout( EVRS_EXECUTED );
}

/*
================
idEventReturn::FinallyVector
================
*/
void idEventReturn::FinallyVector( idVec3 const &vec ) {
	ReturnVector( vec );
	Checkout( EVRS_EXECUTED );
}

/*
================
idEventReturn::FinallyEntity
================
*/
void idEventReturn::FinallyEntity( idEntity *ent ) {
	ReturnEntity( ent );
	Checkout( EVRS_EXECUTED );
}

int idEventReturn::GetReturnedEntNum( void ) {
idToken token;
	switch (e_type) {
		case D_EVENT_INTEGER:
			return (*(int*)ReturnData); 
		case D_EVENT_FLOAT:
			return ((int)*(float*)ReturnData); 
		case D_EVENT_VECTOR:
			return ((idVec3*)ReturnData)->Length();
		case D_EVENT_STRING:
			token = (char*)ReturnData;
			return token.GetIntValue();
		default:
			return 0;
	}
}

int idEventReturn::GetReturnedInt( void ) {
idToken token;
	switch (e_type) {
		case D_EVENT_INTEGER:
			return (*(int*)ReturnData); 
		case D_EVENT_FLOAT:
			return ((int)*(float*)ReturnData); 
		case D_EVENT_VECTOR:
			return ((idVec3*)ReturnData)->Length();
		case D_EVENT_STRING:
			token = (char*)ReturnData;
			return token.GetIntValue();
		default:
			return 0;
	}
}

float idEventReturn::GetReturnedFloat( void ) {
idToken token;
	switch (e_type) {
		case D_EVENT_INTEGER:
			return ((float)*(int*)ReturnData); 
		case D_EVENT_FLOAT:
			return (*(float*)ReturnData); 
		case D_EVENT_VECTOR:
			return ((idVec3*)ReturnData)->Length();
		case D_EVENT_STRING:
			token = (char*)ReturnData;
			return token.GetFloatValue();
		default:
			return 0.0f;
	}
}

const idVec3 idEventReturn::GetReturnedVec( void ) {
idToken token;
	switch (e_type) {
		case D_EVENT_INTEGER:
			return idVec3(*(int*)ReturnData, *(int*)ReturnData, *(int*)ReturnData); 
		case D_EVENT_FLOAT:
			return idVec3(*(float*)ReturnData, *(float*)ReturnData, *(float*)ReturnData);  
		case D_EVENT_VECTOR:
			return (*(idVec3*)ReturnData);
		default:
			return idVec3();
	}
}

const char* idEventReturn::GetReturnedString( void ) {
	switch (e_type) {
		case D_EVENT_INTEGER:
			return va("%d",(*(int*)ReturnData)); 
		case D_EVENT_FLOAT:
			return va("%f",(*(float*)ReturnData)); 
		case D_EVENT_VECTOR:
			return ((idVec3*)ReturnData)->ToString();
		case D_EVENT_STRING:
			return (char*)ReturnData;
		default:
			return "";
	}
}

/*
================
idEventReturn::Checkout
Check this RC lifetime
================
*/
bool idEventReturn::Checkout( int fl, bool noRemove ) {
	flags |= fl;

	if (Test(EVRS_EXECUTED|EVRS_HANDLED,EVRS_EXECUTED|EVRS_HANDLED))
	{
		if (!noRemove) Free();
		return false;
	}
	return true;
}

/*
================
idEventReturn::Flag
Xor flag
================
*/
void idEventReturn::Flag( int xor, int msk ) {
	flags = (flags & msk) ^ xor;
}

/*
================
idEventReturn::Test
return a tested flags
================
*/
bool idEventReturn::Test( int xor, int msk ) {
	return ( ((flags & msk) ^ xor) == 0);
}

/*
================
idEventReturn::PrintFlags
Just debug
================
*/
void idEventReturn::PrintFlags( void ) {
	if (flags & EVRS_EXECUTED) gameLocal.Printf("EVRS_EXECUTED ");
	if (flags & EVRS_HASRETURN) gameLocal.Printf("EVRS_HASRETURN ");
	if (flags & EVRS_HANDLED) gameLocal.Printf("EVRS_HANDLED ");
	if (flags & EVRS_AGAIN) gameLocal.Printf("EVRS_AGAIN ");
	if (flags & EVRS_FAILED) gameLocal.Printf("EVRS_FAILED ");
	if (e_type) gameLocal.Printf( " %c", e_type );
}

/*
================
idEventReturn::Free
================
*/
void idEventReturn::Free( void ) {
	if (ReturnData)
	{
		ReturnDataAllocator.Free( ReturnData );
		ReturnData = NULL;
	}
	allocated = 0;
	e_size = 0;
	e_type = D_RETURN_NSPEC;

	ereturnNode.SetOwner( this );
	ereturnNode.AddToEnd( FreeEverets );
}

/*
================
idEventReturn::isFreed
================
*/
bool idEventReturn::isFreed( const idEventReturn *obj ) {
	return ( obj->ereturnNode.ListHead() == &FreeEverets );
}


#ifdef CREATE_EVENT_CODE
/*
================
CreateEventCallbackHandler
================
*/
void CreateEventCallbackHandler( void ) {
	int num;
	int count;
	int i, j, k;
	char argString[ D_EVENT_MAXARGS + 1 ];
	idStr string1;
	idStr string2;
	idFile *file;

	file = fileSystem->OpenFileWrite( "Callbacks.cpp" );

	file->Printf( "// generated file - see CREATE_EVENT_CODE\n\n" );

	for( i = 1; i <= D_EVENT_MAXARGS; i++ ) {

		file->Printf( "\t/*******************************************************\n\n\t\t%d args\n\n\t*******************************************************/\n\n", i );

		for ( j = 0; j < ( 1 << i ); j++ ) {
			for ( k = 0; k < i; k++ ) {
				argString[ k ] = j & ( 1 << k ) ? 'f' : 'i';
			}
			argString[ i ] = '\0';
			
			string1.Empty();
			string2.Empty();

			for( k = 0; k < i; k++ ) {
				if ( j & ( 1 << k ) ) {
					string1 += "const float";
					string2 += va( "*( float * )&data[ %d ]", k );
				} else {
					string1 += "const int";
					string2 += va( "data[ %d ]", k );
				}

				if ( k < i - 1 ) {
					string1 += ", ";
					string2 += ", ";
				}
			}

			file->Printf( "\tcase %d :\n\t\ttypedef void ( idClass::*eventCallback_%s_t )( %s );\n", ( 1 << ( i + D_EVENT_MAXARGS ) ) + j, argString, string1.c_str() );
			file->Printf( "\t\t( this->*( eventCallback_%s_t )callback )( %s );\n\t\tbreak;\n\n", argString, string2.c_str() );

		}
	}

	fileSystem->CloseFile( file );
}

#endif
