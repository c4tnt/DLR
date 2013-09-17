// Copyright (C) 2004 Id Software, Inc.
//
/*
game_worldspawn.cpp

Worldspawn class.  Each map has one worldspawn which handles global spawnargs.

*/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

/*
================
idWorldspawn

Every map should have exactly one worldspawn.
================
*/
CLASS_DECLARATION( idEntity, idWorldspawn )
	EVENT( EV_Remove,				idWorldspawn::Event_Remove )
	EVENT( EV_SafeRemove,			idWorldspawn::Event_Remove )
END_CLASS

/*
================
idWorldspawn::Spawn
================
*/
void idWorldspawn::Spawn( void ) {
	idStr				scriptname;
	idStr				scriptfile;
	idThread			*thread;
	const function_t	*func;
	const idKeyValue	*kv;

	assert( gameLocal.world == NULL );
	gameLocal.world = this;

	g_gravity.SetFloat( spawnArgs.GetFloat( "gravity", va( "%f", DEFAULT_GRAVITY ) ) );

	// disable stamina on hell levels
	//if ( spawnArgs.GetBool( "no_stamina" ) ) {
	//	max_stamina = 0.0f ;
	//}

	// load script
	// c4tnt: now loading map script from the script/maps folder. mapname.script also loaded
	scriptname = gameLocal.GetMapName();
	scriptname.SetFileExtension( ".script" );
	scriptfile = "script/";
	scriptfile += scriptname;

	gameLocal.Printf( "Loading map script '%s'... ", scriptfile.c_str() );
	if ( fileSystem->ReadFile( scriptfile, NULL, NULL ) > 0 ) {
		gameLocal.program.CompileFile( scriptfile );
		gameLocal.Printf( "loaded\n" );
	} else {
		gameLocal.Printf( "failed\n" );
	}

	gameLocal.Printf( "Loading map script '%s'... ", scriptname.c_str() );
	if ( fileSystem->ReadFile( scriptname, NULL, NULL ) > 0 ) {
		gameLocal.program.CompileFile( scriptname );
		gameLocal.Printf( "loaded\n" );
	} else {
		gameLocal.Printf( "failed\n" );
	}

	// call the main function by default
	func = gameLocal.program.FindFunction( "main" );
	if ( func != NULL ) {
		thread = new idThread( func );
		thread->DelayedStart( 0 );
	}

	// call any functions specified in worldspawn
	kv = spawnArgs.MatchPrefix( "call" );
	while( kv != NULL ) {
		func = gameLocal.program.FindFunction( kv->GetValue() );
		if ( func == NULL ) {
			gameLocal.Error( "Function '%s' not found in script for '%s' key on worldspawn", kv->GetValue().c_str(), kv->GetKey().c_str() );
		}

		thread = new idThread( func );
		thread->DelayedStart( 0 );
		kv = spawnArgs.MatchPrefix( "call", kv );
	}
}

/*
=================
idWorldspawn::Save
=================
*/
void idWorldspawn::Save( idRestore_I *savefile ) {
}

/*
=================
idWorldspawn::Restore
=================
*/
void idWorldspawn::Restore( idRestore_I *savefile ) {
	assert( gameLocal.world == this );

	g_gravity.SetFloat( spawnArgs.GetFloat( "gravity", va( "%f", DEFAULT_GRAVITY ) ) );

	// disable stamina on hell levels
	//if ( spawnArgs.GetBool( "no_stamina" ) ) {
	//	pm_stamina.SetFloat( 0.0f );
	//}
}

/*
================
idWorldspawn::~idWorldspawn
================
*/
idWorldspawn::~idWorldspawn() {
	if ( gameLocal.world == this ) {
		gameLocal.world = NULL;
	}
}

/*
================
idWorldspawn::Event_Remove
================
*/
void idWorldspawn::Event_Remove( void ) {
	gameLocal.Error( "Tried to remove world" );
}
