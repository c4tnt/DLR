/*
===============================================================================

  c4tnt: Area entity set. 
  
  Area_Marker - Allows mark some level areas, invisible object
  Area_HUBTransfer - HUB system entity. Will transfer all from some location in current map to another map. 
	Trigger this to begin the transfer
  Area_Count - Special counter. Will count specific entitys in Area_Marker. It activate tergets or call scripts, 
	when something happens
  Area_Weather - Creates some weather in specified area (Can generate rain, snow, gale wind)
  Area_Script -  It allows to use areas in a script directly (Setup eventlist, register signals and much more)	
===============================================================================
*/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

const idEventDef EV_FindAreas( "<findAreas>", NULL );

/*
===============================================================================

  idAreaMarker

===============================================================================
*/
idStrPool		idAreaMarker::globalFilters;

CLASS_DECLARATION( idEntity, idAreaMarker )
	EVENT( EV_Activate,		idAreaMarker::Event_Activate )
END_CLASS


/*
================
idAreaMarker::idAreaMarker
================
*/
idAreaMarker::idAreaMarker( void ) {
	clipModel = NULL;
}

/*
================
idAreaMarker::Spawn
================
*/
void idAreaMarker::Spawn( void ) {
	const idKeyValue *kvptr;

	// get the clip model
	clipModel = new idClipModel( GetPhysics()->GetClipModel() );

	// remove the collision model from the physics object
	GetPhysics()->SetClipModel( NULL, 1.0f );
	targets.Clear();
	callbacks.Clear();
	
	kvptr = spawnArgs.MatchPrefix( "filter" );
	
	while (kvptr) {
		Attach(NULL, kvptr->GetValue() );	
		kvptr = spawnArgs.MatchPrefix( "filter", kvptr );
	}

	BecomeActive( TH_THINK );
}

/*
==============================
idAreaMarker::ActivateTargets

"activator" should be set to the entity that initiated the firing.
==============================
*/
void idAreaMarker::ActivateTargets( idEntity *activator ) const {
	idEntity	*ent;
	int			i;
	
	for( i = 0; i < targets.Num(); i++ ) {
		ent = targets[ i ].GetEntity();
		if ( !ent ) {
			continue;
		}
		if ( Owned( ent ) )
			continue;

		if ( ent->RespondsTo( EV_Activate ) || ent->HasSignal( SIG_TRIGGER ) ) {
			ent->Signal( SIG_TRIGGER );
			ent->ProcessEvent( &EV_Activate, NULL, activator );
		} 		
		ent->ActivateGUIS();
	}
}

/*
================
idAreaMarker::Save
================
*/
void idAreaMarker::Save( idSave_I *savefile ) {
	int i;

	savefile->WriteClipModel( clipModel );
	savefile->WriteInt( targets.Num() );
	for( i = 0; i < targets.Num(); i++ ) {
		targets[ i ].Save( savefile );
	}
}

/*
================
idAreaMarker::Restore
================
*/
void idAreaMarker::Restore( idRestore_I *savefile ) {
	int i, num;

	savefile->ReadClipModel( clipModel );
	savefile->ReadInt( num );
	targets.SetNum( num );
	for( i = 0; i < num; i++ ) {
		targets[ i ].Restore( savefile );
	}
}

/*
================
idAreaMarker::TouchEntities
================
*/
void idAreaMarker::TouchEntities( void ) {
	int numClipModels, i, j;
	idBounds bounds;
	idClipModel *cm, *clipModelList[ MAX_GENTITIES ];
	idEntity *entity;

	if ( clipModel == NULL ) {
		return;
	}

	bounds.FromTransformedBounds( clipModel->GetBounds(), clipModel->GetOrigin(), clipModel->GetAxis() );
	numClipModels = gameLocal.clip.ClipModelsTouchingBounds( bounds, -1, clipModelList, MAX_GENTITIES );

	for( i = 0; i < targets.Num(); i++ ) {
		for ( j = 0; j < numClipModels; j++)
		{
			cm = clipModelList[ j ];
			if ( !cm ) {
				continue;
			}
			if ( !cm->IsTraceModel() ) {
				clipModelList[ j ] = clipModelList[ numClipModels - 1 ]; //Cutout
				j--;
				numClipModels--;
				continue;
			}
			entity = cm->GetEntity();
			if ( !entity ) {
				clipModelList[ j ] = clipModelList[ numClipModels - 1 ]; //Cutout
				j--;
				numClipModels--;
				continue;
			}
			if ( targets[i].GetEntity() == entity )
			{
				if ( !gameLocal.clip.ContentsModel( cm->GetOrigin(), cm, cm->GetAxis(), -1,
						clipModel->Handle(), clipModel->GetOrigin(), clipModel->GetAxis() ) ) {
					targets.RemoveIndex(i);
					i--;
					//Call LEAVE script here
					Exit(entity);
				}
				clipModelList[ j ] = clipModelList[ numClipModels - 1 ]; //Cutout
				j--;
				numClipModels--;
				break;
			}
		}
		if (j == numClipModels) //Not found
		{
			targets.RemoveIndex(i);
			i--;
			//Call LEAVE script here
			Exit(entity);
		}
	}
	for ( i = 0; i < numClipModels; i++ ) {
		cm = clipModelList[ i ];

		if ( !cm ) {
			continue;
		}

		if ( !cm->IsTraceModel() ) {
			continue;
		}

		entity = cm->GetEntity();

		if ( !entity ) {
			continue;
		}

		if ( !gameLocal.clip.ContentsModel( cm->GetOrigin(), cm, cm->GetAxis(), -1,
								clipModel->Handle(), clipModel->GetOrigin(), clipModel->GetAxis() ) ) {
			continue;
		}
	
		if ( Enter(entity) ) {
			idEntityPtr<idEntity> &entityPtr = targets.Alloc();
			entityPtr = entity;
		}
	}
	SendChangedNotify( true );
}

/*
================
idAreaMarker::Attach
================
*/
int idAreaMarker::Attach( idAreaDependant* obj, const char* filter, ecmp_method cmp_method, int UserData ) {
	
	_area_callback_t *T = &callbacks.Alloc();
	
	T->filter = globalFilters.AllocString( filter ); 
	T->owner = obj;
	T->cmp_method = cmp_method;
	T->UserData = UserData;
	T->_UF = false;

	return callbacks.Num() - 1;
}

/*
================
idAreaMarker::Update
================
*/
bool idAreaMarker::Update( int Handle, WORD updBits, const char* filter, ecmp_method cmp_method, int UserData ) {
	if ( Handle < 0 || Handle >= callbacks.Num() )
		return false;

	if (updBits & 1) {
		if (callbacks[Handle].filter)
			globalFilters.FreeString(callbacks[Handle].filter);
		callbacks[Handle].filter = globalFilters.AllocString( filter ); 
	}

	if (updBits & 2) callbacks[Handle].cmp_method = cmp_method;
	if (updBits & 4) callbacks[Handle].UserData = UserData;
	callbacks[Handle]._UF = true;

	return true;
}

/*
================
idAreaMarker::Detach
================
*/
void idAreaMarker::Detach( int Handle ) {
	if ( Handle < 0 || Handle >= callbacks.Num() )
		return;

	if (callbacks[Handle].filter)
		globalFilters.FreeString(callbacks[Handle].filter);

	callbacks.RemoveIndex(Handle);
}

/*
================
idAreaMarker::GetCount
================
*/
int idAreaMarker::GetCount( int Handle ) const {
int i, count;

	if ( Handle < 0 || Handle >= callbacks.Num() )
		return 0;

	count = 0;

	for( i = 0; i < targets.Num(); i++ ) {
		if ( Filter(Handle,targets[i].GetEntity()) ) {
			count++;		
		}
	}
	return count;
}

/*
================
idAreaMarker::GetCount
================
*/
int idAreaMarker::GetCount( idAreaDependant* ob ) const {
int i, j, count;

	count = 0;
	for( j = 0; j < callbacks.Num(); j++ ) {
		if (callbacks[j].owner == ob) {
			for ( i = 0; i < targets.Num(); i++ ) {
				if ( Filter(j,targets[i].GetEntity()) ) {
					count++;		
				}
			}
		}
	}
	return count;
}

/*
================
idAreaMarker::DetachAll
================
*/
void idAreaMarker::DetachAll( idAreaDependant* obj ) {
	int i;

	for (i = 0; i < callbacks.Num(); i++)
	{
		if (callbacks[i].owner = obj)
		{
			if (callbacks[i].filter)
				globalFilters.FreeString(callbacks[i].filter);
			callbacks.RemoveIndex(i);
			i--;
		}
	}
}

/*
================
idAreaMarker::Filter
================
*/
bool idAreaMarker::Filter( int Handle, idEntity* ent ) const {
	idTypeInfo *clstype;

	if ( Handle < 0 || Handle >= callbacks.Num() )
		return false;

	switch ( callbacks[Handle].cmp_method )	{
	case CMPM_BYNAME:
		if (!callbacks[Handle].filter->fCmp(ent->GetName()))
			return true;	
		break;
	case CMPM_BYCLASS:
		clstype = idClass::GetClass(*callbacks[Handle].filter);
		if (clstype && ent->IsType(*clstype))
			return true;	
		break;
	default:
		if (callbacks[Handle].owner){
			gameLocal.Error("Bad cmp type for %s in the area_marker %s \n", callbacks[Handle].owner->GetName(), this->GetName());
		}else{
			gameLocal.Error("Bad cmp type in the area_marker %s \n", callbacks[Handle].owner->GetName(), this->GetName());
		}
	}
	return false;	
}
/*
================
idAreaMarker::Enter
================
*/
bool idAreaMarker::Enter( idEntity* ent ) {

	int i;
	bool Allow;

	if (!ent)
		return false;

	Allow = false;

	for (i = 0; i < callbacks.Num(); i++)
	{
		if ( Filter(i,ent) ) {
			if (callbacks[i].owner)	{
				callbacks[i].owner->AreaEnterNotify(this, ent); 
				callbacks[i]._UF = true;
			}
			Allow = true;
		}
	}
	
	if (Allow) {
		gameLocal.Printf("entity %s enters the area_marker %s \n", ent->GetName(), this->GetName());
	}
	return Allow;
}

/*
================
idAreaMarker::Exit
================
*/
void idAreaMarker::Exit( idEntity* ent ) {
	
	int i;

	if (!ent)
		return;

	for (i = 0; i < callbacks.Num(); i++)
	{
		if ( Filter(i,ent) ) {
			if (callbacks[i].owner)	{
				callbacks[i].owner->AreaExitNotify(this, ent); 
				callbacks[i]._UF = true;
			}
		}
	}
	gameLocal.Printf("entity %s leaves the area_marker %s\n", ent->GetName(), this->GetName());
}

/*
================
idAreaMarker::SendChangedNotify
================
*/
void idAreaMarker::SendChangedNotify( bool Send ) {
	
	int i, j;

	for (i = 0; i < callbacks.Num(); i++)
	{
		if (callbacks[i]._UF) {
			if (callbacks[i].owner && Send) {
				callbacks[i].owner->AreaChangedNotify( this );
				gameLocal.Printf("send update signal for %s from the area_marker %s\n", callbacks[i].owner->GetName(), this->GetName());
			}
			for (j = i; j < callbacks.Num(); j++)
			{
				if ( callbacks[j].owner == callbacks[i].owner ) callbacks[i]._UF = false;
			}
		}
	}
}

/*
================
idAreaMarker::Owned
================
*/
bool idAreaMarker::Owned( idEntity* ent ) const {
	
	int i;

	if (!ent)
		return false;

	for (i = 0; i < callbacks.Num(); i++)
	{
		if ( Filter(i,ent) ) {
			if (!callbacks[i].owner)	{
				return false;
			}
		}
	}
	return true;
}

/*
================
idAreaMarker::Think
================
*/
void idAreaMarker::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		TouchEntities();
	}
	idEntity::Think();
}

/*
================
idAreaMarker::Event_Activate
================
*/
void idAreaMarker::Event_Activate( idEntity *activator ) {
	ActivateTargets( activator );
}

/*
===============================================================================

  idAreaDependant
  Hierarchy class, does nothing.
	
===============================================================================
*/

CLASS_DECLARATION( idEntity, idAreaDependant )
	EVENT( EV_FindAreas,		idAreaDependant::Event_Register )
END_CLASS

/*
================
idAreaDependant::~idAreaDependant
================
*/
idAreaDependant::~idAreaDependant( ) {
	int i;
	for ( i = 0; i < areas.Num(); i++ ) {
		if (areas[i].GetEntity()) {
			areas[i].GetEntity()->DetachAll( this );
		}
	}
	areas.Clear();
}
/*
================
idAreaDependant::Event_Register
================
*/
void idAreaDependant::Event_Register( ) {
	const idKeyValue *kvptr, *kvptr2;
	idStr filterprefix;
	idStr classprefix;
	idEntity* ent;
	idAreaMarker* armPtr;

	areas.Clear();
	kvptr = spawnArgs.MatchPrefix( "area" );
	while (kvptr) {
		filterprefix = kvptr->GetKey();
		filterprefix.Append("_filter");
		classprefix = kvptr->GetKey();
		classprefix.Append("_class");
		
		ent = gameLocal.FindEntity(kvptr->GetValue());
		kvptr = spawnArgs.MatchPrefix( "area", kvptr );
		if (!ent)
			continue;

		if (ent->IsType(idAreaMarker::Type)) {
			idEntityPtr<idAreaMarker> &areaPtr = areas.Alloc();
			armPtr = (idAreaMarker*)ent;
			areaPtr = armPtr;
			kvptr2 = spawnArgs.MatchPrefix( filterprefix );
			while (kvptr2) {
				//Register with specified filter
				armPtr->Attach( this, kvptr2->GetValue() );
				kvptr2 = spawnArgs.MatchPrefix( filterprefix, kvptr2 );
			}
			kvptr2 = spawnArgs.MatchPrefix( "filter" );
			while (kvptr2) {
				//Register with specified filter
				armPtr->Attach( this, kvptr2->GetValue() );
				kvptr2 = spawnArgs.MatchPrefix( "filter", kvptr2 );
			}
			kvptr2 = spawnArgs.MatchPrefix( classprefix );
			while (kvptr2) {
				//Register with specified filter
				armPtr->Attach( this, kvptr2->GetValue(),CMPM_BYCLASS );
				kvptr2 = spawnArgs.MatchPrefix( classprefix, kvptr2 );
			}
			kvptr2 = spawnArgs.MatchPrefix( "class" );
			while (kvptr2) {
				//Register with specified filter
				armPtr->Attach( this, kvptr2->GetValue(),CMPM_BYCLASS );
				kvptr2 = spawnArgs.MatchPrefix( "class", kvptr2 );
			}
		}else{
			gameLocal.Warning( "trigger '%s' at (%s) tries to use '%s' as areaMarker", name.c_str(), GetPhysics()->GetOrigin().ToString(0), ent->GetName() );
		}
	}
}


/*
===============================================================================

  idArea_Count

  rem: from idTrigger_Timer
	
===============================================================================
*/

CLASS_DECLARATION( idAreaDependant, idAreaCount )
	EVENT( EV_Activate,		idAreaCount::Event_Activate )
	EVENT( EV_TriggerAction,	idAreaCount::Event_TriggerAction )
	EVENT( EV_Enable,		idAreaCount::Event_Enable )
	EVENT( EV_Disable,		idAreaCount::Event_Disable )
END_CLASS

/*
================
idAreaCount::idAreaCount
================
*/
idAreaCount::idAreaCount( void ) {
	wait = 0.0f;
	random = 0.0f;
	delay = 0.0f;
	random_delay = 0.0f;
	nextTriggerTime = 0;
	scriptFunction = NULL;
	flagged = false;
}

/*
================
idAreaCount::Save
================
*/
void idAreaCount::Save( idSave_I *savefile ) const {
	if ( scriptFunction ) {
		savefile->WriteString( scriptFunction->Name() );
	} else {
		savefile->WriteString( "" );
	}
	savefile->WriteFloat( wait );
	savefile->WriteFloat( random );
	savefile->WriteFloat( delay );
	savefile->WriteFloat( random_delay );
	savefile->WriteInt( nextTriggerTime );
}

/*
================
idAreaCount::Restore
================
*/
void idAreaCount::Restore( idRestore_I *savefile ) {
	idStr funcname;
	savefile->ReadString( funcname );
	if ( funcname.Length() ) {
		scriptFunction = gameLocal.program.FindFunction( funcname );
		if ( scriptFunction == NULL ) {
			gameLocal.Warning( "idTrigger_Multi '%s' at (%s) calls unknown function '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), funcname.c_str() );
		}
	} else {
		scriptFunction = NULL;
	}
	savefile->ReadFloat( wait );
	savefile->ReadFloat( random );
	savefile->ReadFloat( delay );
	savefile->ReadFloat( random_delay );
	savefile->ReadInt( nextTriggerTime );
}

bool idAreaCount::AreaChangedNotify(idAreaMarker* Area) {
float ecount;

	if (!on)
		return false;

	if ( nextTriggerTime > gameLocal.time ) 
		return false;

	ecount = Area->GetCount( this ); //Get count for self triggers
	if (flip == (min <= ecount && max >= ecount)) {
		flagged = false;
		return false;				 //Unsatisfacted
	}
	
	if (!flagged) {
		//Trigger here
		nextTriggerTime = gameLocal.time + 1;

		if ( delay > 0 ) {
			// don't allow it to trigger again until our delay has passed
			nextTriggerTime += SEC2MS( delay + random_delay * gameLocal.random.CRandomFloat() );
			PostEventSec( &EV_TriggerAction, NULL, delay );
		} else {
			TriggerAction( );
		}		
		flagged = true;
	}
	
	return true;
}

/*
================
idAreaCount::Spawn

Fires own targets while conditions is sufficient in some area
================
*/
void idAreaCount::Spawn( void ) {

	idStr funcname = spawnArgs.GetString( "call", "" );

	if ( funcname.Length() ) {
		scriptFunction = gameLocal.program.FindFunction( funcname );
		if ( scriptFunction == NULL ) {
			gameLocal.Warning( "trigger '%s' at (%s) calls unknown function '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), funcname.c_str() );
		}
	} else {
		scriptFunction = NULL;
	}

	spawnArgs.GetFloat( "wait", "0.5", wait );
	spawnArgs.GetFloat( "random", "0", random );
	spawnArgs.GetFloat( "delay", "0", delay );
	spawnArgs.GetFloat( "random_delay", "0", random_delay );
	spawnArgs.GetBool( "start_on", "1", on );
	spawnArgs.GetFloat( "min_count", "0", min );
	spawnArgs.GetFloat( "max_count", "1.5", max );
	spawnArgs.GetBool( "flip_interval", "0", flip );
	spawnArgs.GetBool( "initial_state", "0", flagged );
	
	if ( random && ( random >= wait ) && ( wait >= 0 ) ) {
		random = wait - 1;
		gameLocal.Warning( "idAreaCount '%s' at (%s) has random >= wait", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	if ( random_delay && ( random_delay >= delay ) && ( delay >= 0 ) ) {
		random_delay = delay - 1;
		gameLocal.Warning( "idAreaCount '%s' at (%s) has random_delay >= delay", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	nextTriggerTime = 0;

	if ( gameLocal.GameState() == GAMESTATE_STARTUP ) {
		PostEventMS( &EV_FindAreas, NULL, 0 );
	} else {
		// not during spawn, so it's ok to get the areas
		Event_Register();
	}
}

/*
================
idAreaCount::TriggerAction
================
*/
void idAreaCount::TriggerAction( ) {
	ActivateTargets( this );
	CallScript();

	if ( wait >= 0 ) {
		nextTriggerTime = gameLocal.time + SEC2MS( wait + random * gameLocal.random.CRandomFloat() );
	} else {
		// we can't just remove (this) here, because this is a touch function
		// called while looping through area links...
		nextTriggerTime = gameLocal.time + 1;
		PostEventMS( &EV_Remove, NULL, 0 );
	}
}

/*
================
idAreaCount::CallScript
================
*/
void idAreaCount::CallScript( void ) const {
	idThread *thread;

	if ( scriptFunction ) {
		thread = new idThread( scriptFunction );
		thread->DelayedStart( 0 );
	}
}

/*
================
idAreaCount::GetScriptFunction
================
*/
const function_t *idAreaCount::GetScriptFunction( void ) const {
	return scriptFunction;
}

/*
================
idAreaCount::Event_TriggerAction
================
*/
void idAreaCount::Event_TriggerAction( ) {
	TriggerAction( );
}
/*
================
idAreaCount::Enable
================
*/
void idAreaCount::Event_Enable( void ) {
	// if off, turn it on
	if ( !on ) {
		on = true;
	}
}

/*
================
idAreaCount::Disable
================
*/
void idAreaCount::Event_Disable( void ) {
	// if on, turn it off
	if ( on ) {
		on = false;
	}
}

/*
================
idAreaCount::Event_Activate
================
*/
void idAreaCount::Event_Activate( idEntity *activator ) {
	// if on, turn it off
	if ( on ) {
		on = false;
	} else {
		// turn it on
		on = true;
	}
}
