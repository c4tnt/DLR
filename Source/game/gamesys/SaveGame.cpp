// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

#include "TypeInfo.h"

/*
Save game related helper classes.

Save games are implemented in two classes, idSaveGame and idRestoreGame, that implement write/read functions for 
common types.  They're passed in to each entity and object for them to archive themselves.  Each class
implements save/restore functions for it's own data.  When restoring, all the objects are instantiated,
then the restore function is called on each, superclass first, then subclasses.

Pointers are restored by saving out an object index for each unique object pointer and adding them to a list of
objects that are to be saved.  Restore instantiates all the objects in the list before calling the Restore function
on each object so that the pointers returned are valid.  No object's restore function should rely on any other objects
being fully instantiated until after the restore process is complete.  Post restore fixup should be done by posting
events with 0 delay.

The savegame header will have the Game Name, Version, Map Name, and Player Persistent Info.

Changes in version make savegames incompatible, and the game will start from the beginning of the level with
the player's persistent info.

Changes to classes that don't need to break compatibilty can use the build number as the savegame version.
Later versions are responsible for restoring from previous versions by ignoring any unused data and initializing
variables that weren't in previous versions with safe information.

At the head of the save game is enough information to restore the player to the beginning of the level should the
file be unloadable in some way (for example, due to script changes).
*/

/*
================
idSaveGame::idSaveGame()
================
*/
idSaveGame::idSaveGame( idFile *savefile ) {

	file = savefile;
	dictHash = NULL;
	// Put NULL at the start of the list so we can skip over it.
	objects.Clear();
	objects.Append( NULL );
}

/*
================
idSaveGame::~idSaveGame()
================
*/
idSaveGame::~idSaveGame() {
	if ( objects.Num() ) {
		Close();
	}
}

/*
================
idSaveGame::Close
================
*/
void idSaveGame::Close( void ) {
	int i;

	for( i = 1; i < objects.Num(); i++ ) {
		CallSave_r( objects[ i ]->GetType(), objects[ i ] );
	}

	objects.Clear();
	if ( dictHash ) {
		dictHash->Clear();
		delete dictHash;
	}

#ifdef ID_DEBUG_MEMORY
	idStr gameState = file->GetName();
	gameState.StripFileExtension();
	WriteGameState_f( idCmdArgs( va( "test %s_save", gameState.c_str() ), false ) );
#endif
}

/*
================
idSaveGame::WriteObjectList
================
*/
void idSaveGame::WriteObjectList( void ) {
	int i;
	idStrList classHash;
	bool SmallIndexes = false;

	for( i = 1; i < objects.Num(); i++ ) {
		classHash.AddUnique( objects[ i ]->GetClassname() );
	}

	WriteInt( classHash.Num() );
	if ( classHash.Num() < 255 ) SmallIndexes = true;

	for( i = 0; i < classHash.Num(); i++ ) {
		WriteString( classHash[ i ] );
	}

	WriteInt( objects.Num() - 1 );
	
	for( i = 1; i < objects.Num(); i++ ) {
		if (SmallIndexes) {
			WriteByte( (byte)classHash.FindIndex( objects[ i ]->GetClassname() ) );
		} else {
			WriteUnsignedShort( (unsigned short)classHash.FindIndex( objects[ i ]->GetClassname() ) );
		}
	}
}

/*
================
idSaveGame::SetupDictHash
================
*/
void idSaveGame::SetupDictHash( void ) {

	if ( dictHash ) {
		dictHash->Clear();
	} else {
		dictHash = new idStrPool;
	}

}

/*
================
idSaveGame::CallSave_r
================
*/
void idSaveGame::CallSave_r( const idTypeInfo *cls, const idClass *obj ) {

#ifdef SAVEFILE_COMMENTS
	WriteComment( va( "<-### %s ###->", cls->classname ) ); 
#endif

	if ( cls->super ) {
		CallSave_r( cls->super, obj );
		if ( cls->super->Save == cls->Save ) {
			// don't call save on this inheritance level since the function was called in the super class
			return;
		}
	}
	
	( obj->*cls->Save )( this );

	WriteWDT();

#ifdef SAVEFILE_COMMENTS
	WriteComment( va( "<-### END %s ###->", cls->classname ) ); 
#endif
}

/*
================
idSaveGame::AddObject
================
*/
void idSaveGame::AddObject( const idClass *obj ) {
	objects.AddUnique( obj );
}

/*
================
idSaveGame::Write
================
*/
void idSaveGame::Write( const void *buffer, int len ) {
	file->Write( buffer, len );
}

/*
================
idSaveGame::WriteRLE
Compress buffer with RLE and write out
================
*/
void idSaveGame::WriteRLE( const void *buffer, int len ) {

	archive.ResetIO( true );
//	WriteComment( "Here is Uncompressed data:" );
//	file->Write( buffer, len );
	archive.CompressLCS( buffer, len );
//	WriteComment( "Here is Compressed data:" );
	archive.Save( file );
//	b = new byte[ len ];	
//	WriteComment( "Here is Decompressed data:" );
//	archive.ResetIO( false );
//	memset( b, 0xFF, len );
//	archive.DecompressLCS( b, len );
//	file->Write( b, len );
//	delete [] b;
//	WriteComment( "End data:" );
}

/*
================
idSaveGame::WriteInt
================
*/
void idSaveGame::WriteInt( const int value ) {

#if (API_VERS >= 3)
	file->WriteInt( value );
#else
	file->Write( &value, sizeof( value ) );
#endif
}

/*
================
idSaveGame::WriteJoint
================
*/
void idSaveGame::WriteJoint( const jointHandle_t value ) {
#if (API_VERS >= 3)
	file->WriteInt( (int&)value );
#else
	file->Write( &value, sizeof( value ) );
#endif
}

/*
================
idSaveGame::WriteShort
================
*/
void idSaveGame::WriteShort( const short value ) {
#if (API_VERS >= 3)
	file->WriteShort( value );
#else
	file->Write( &value, sizeof( value ) );
#endif
}

/*
================
idSaveGame::WriteUnsignedShort
================
*/
void idSaveGame::WriteUnsignedShort( const unsigned short value ) {
	file->Write( &value, sizeof( value ) );
}

/*
================
idSaveGame::WriteByte
================
*/
void idSaveGame::WriteByte( const byte value ) {
	file->Write( &value, sizeof( value ) );
}

/*
================
idSaveGame::WriteSignedChar
================
*/
void idSaveGame::WriteSignedChar( const signed char value ) {
	file->Write( &value, sizeof( value ) );
}

/*
================
idSaveGame::WriteFloat
================
*/
void idSaveGame::WriteFloat( const float value ) {

#if (API_VERS >= 3)
	file->WriteFloat( value );
#else
	file->Write( &value, sizeof( value ) );
#endif
}

/*
================
idSaveGame::WriteBool
================
*/
void idSaveGame::WriteBool( const bool value ) {
char m_byte;

//#if (API_VERS >= 3)
//	file->WriteBool( value );
//#else
	m_byte = value?0xFF:0x00;
	file->Write( &m_byte, 1 );
//#endif
}

/*
================
idSaveGame::WriteComment
================
*/
void idSaveGame::WriteComment( const char *string ) {
	int len;

	len = strlen( string );
	file->Write( string, len );
	WriteByte( 0x00 );
}

/*
================
idSaveGame::WriteString
================
*/
void idSaveGame::WriteString( const char *string ) {
	int len;
	int idx;

	len = strlen( string );
	if ( dictHash ) {
		idx = dictHash->FindString( string );
		if ( idx < 0 ) {
			WriteInt( len );
			file->Write( string, len );
			dictHash->AllocString( string );
		} else {
			WriteInt( -idx - 1 );			
		}
	} else {
		WriteInt( len );
		file->Write( string, len );
	}
}

/*
================
idSaveGame::WriteVec2
================
*/
void idSaveGame::WriteVec2( const idVec2 &vec ) {
#if (API_VERS >= 3)
	file->WriteVec2( vec );
#else
	file->Write( &vec, sizeof( vec ) );
#endif
}

/*
================
idSaveGame::WriteVec3
================
*/
void idSaveGame::WriteVec3( const idVec3 &vec ) {
#if (API_VERS >= 3)
	file->WriteVec3( vec );
#else
	file->Write( &vec, sizeof( vec ) );
#endif
}

/*
================
idSaveGame::WriteVec4
================
*/
void idSaveGame::WriteVec4( const idVec4 &vec ) {
#if (API_VERS >= 3)
	file->WriteVec4( vec );
#else
	file->Write( &vec, sizeof( vec ) );
#endif
}

/*
================
idSaveGame::WriteVec6
================
*/
void idSaveGame::WriteVec6( const idVec6 &vec ) {
#if (API_VERS >= 3)
	file->WriteVec6( vec );
#else
	file->Write( &vec, sizeof( vec ) );
#endif
}

/*
================
idSaveGame::WriteBounds
================
*/
void idSaveGame::WriteBounds( const idBounds &bounds ) {
#if (API_VERS >= 3)
	idBounds b = bounds;
	LittleRevBytes( &b, sizeof(float), sizeof(b)/sizeof(float) );
	file->Write( &b, sizeof( b ) );
#else
	file->Write( &bounds, sizeof( bounds ) );
#endif
}

/*
================
idSaveGame::WriteBounds
================
*/
void idSaveGame::WriteWinding( const idWinding &w )
{
	int i, num;
	num = w.GetNumPoints();
	file->Write( &num, sizeof(num) );
	for ( i = 0; i < num; i++ ) {
		file->Write( &w[i], sizeof(idVec5) );
	}
}


/*
================
idSaveGame::WriteMat3
================
*/
void idSaveGame::WriteMat3( const idMat3 &mat ) {
	file->Write( &mat, sizeof( mat ) );
}

/*
================
idSaveGame::WriteAngles
================
*/
void idSaveGame::WriteAngles( const idAngles &angles ) {
	file->Write( &angles, sizeof( angles ) );
}

/*
================
idSaveGame::WriteObject
================
*/
void idSaveGame::WriteObject( const idClass *obj ) {
	int index;

	index = objects.FindIndex( obj );
	if ( index < 0 ) {
		gameLocal.DPrintf( "idSaveGame::WriteObject - WriteObject FindIndex failed\n" );

		// Use the NULL index
		index = 0;
	}
	WriteInt( index );
}

/*
================
idSaveGame::WriteStaticObject
================
*/
void idSaveGame::WriteStaticObject( const idClass &obj ) {
	CallSave_r( obj.GetType(), &obj );
}

/*
================
idSaveGame::WriteDict
================
*/
void idSaveGame::WriteDict( const idDict *dict ) {
	int num;
	int i;
	const idKeyValue *kv;

	if ( !dict ) {
		WriteInt( -1 );
	} else {
		num = dict->GetNumKeyVals();
		WriteInt( num );
		for( i = 0; i < num; i++ ) {
			kv = dict->GetKeyVal( i );
			WriteString( kv->GetKey() );
			WriteString( kv->GetValue() );
		}
	}
}

/*
================
idSaveGame::WriteMaterial
================
*/
void idSaveGame::WriteMaterial( const idMaterial *material ) {
	if ( !material ) {
		WriteString( "" );
	} else {
		WriteString( material->GetName() );
	}
}

/*
================
idSaveGame::WriteSkin
================
*/
void idSaveGame::WriteSkin( const idDeclSkin *skin ) {
	if ( !skin ) {
		WriteString( "" );
	} else {
		WriteString( skin->GetName() );
	}
}

/*
================
idSaveGame::WriteParticle
================
*/
void idSaveGame::WriteParticle( const idDeclParticle *particle ) {
	if ( !particle ) {
		WriteString( "" );
	} else {
		WriteString( particle->GetName() );
	}
}

/*
================
idSaveGame::WriteFX
================
*/
void idSaveGame::WriteFX( const idDeclFX *fx ) {
	if ( !fx ) {
		WriteString( "" );
	} else {
		WriteString( fx->GetName() );
	}
}

/*
================
idSaveGame::WriteModelDef
================
*/
void idSaveGame::WriteModelDef( const idDeclModelDef *modelDef ) {
	if ( !modelDef ) {
		WriteString( "" );
	} else {
		WriteString( modelDef->GetName() );
	}
}

/*
================
idSaveGame::WriteSoundShader
================
*/
void idSaveGame::WriteSoundShader( const idSoundShader *shader ) {
	const char *name;

	if ( !shader ) {
		WriteString( "" );
	} else {
		name = shader->GetName();
		WriteString( name );
	}
}

/*
================
idSaveGame::WriteModel
================
*/
void idSaveGame::WriteModel( const idRenderModel *model ) {
	const char *name;

	if ( !model ) {
		WriteString( "" );
	} else {
		name = model->Name();
		WriteString( name );
	}
}

/*
================
idSaveGame::WriteUserInterface
================
*/
void idSaveGame::WriteUserInterface( const idUserInterface *ui, bool unique ) {
	const char *name;

	if ( !ui ) {
		WriteString( "" );
	} else {
		name = ui->Name();
		WriteString( name );
		WriteBool( unique );
//		WriteDict( &ui->State() );
		if ( ui->WriteToSaveGame( file ) == false ) {
			gameLocal.Error( "idSaveGame::WriteUserInterface: ui failed to write properly\n" );
		}
	}
}

/*
================
idSaveGame::WriteRenderEntity
================
*/
void idSaveGame::WriteRenderEntity( const renderEntity_t &renderEntity ) {
	int i;

	WriteModel( renderEntity.hModel );

	WriteInt( renderEntity.entityNum );
	WriteInt( renderEntity.bodyId );

	WriteBounds( renderEntity.bounds );

	// callback is set by class's Restore function

	WriteInt( renderEntity.suppressSurfaceInViewID );
	WriteInt( renderEntity.suppressShadowInViewID );
	WriteInt( renderEntity.suppressShadowInLightID );
	WriteInt( renderEntity.allowSurfaceInViewID );

	WriteVec3( renderEntity.origin );
	WriteMat3( renderEntity.axis );

	WriteMaterial( renderEntity.customShader );
	WriteMaterial( renderEntity.referenceShader );
	WriteSkin( renderEntity.customSkin );

	if ( renderEntity.referenceSound != NULL ) {
		WriteInt( renderEntity.referenceSound->Index() );
	} else {
		WriteInt( 0 );
	}

	for( i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ ) {
		WriteFloat( renderEntity.shaderParms[ i ] );
	}

	for( i = 0; i < MAX_RENDERENTITY_GUI; i++ ) {
		WriteUserInterface( renderEntity.gui[ i ], renderEntity.gui[ i ] ? renderEntity.gui[ i ]->IsUniqued() : false );
	}

	WriteFloat( renderEntity.modelDepthHack );

	WriteBool( renderEntity.noSelfShadow );
	WriteBool( renderEntity.noShadow );
	WriteBool( renderEntity.noDynamicInteractions );
	WriteBool( renderEntity.weaponDepthHack );

	WriteInt( renderEntity.forceUpdate );
}

/*
================
idSaveGame::WriteRenderLight
================
*/
void idSaveGame::WriteRenderLight( const renderLight_t &renderLight ) {
	int i;

	WriteMat3( renderLight.axis );
	WriteVec3( renderLight.origin );

	WriteInt( renderLight.suppressLightInViewID );
	WriteInt( renderLight.allowLightInViewID );
	WriteBool( renderLight.noShadows );
	WriteBool( renderLight.noSpecular );
	WriteBool( renderLight.pointLight );
	WriteBool( renderLight.parallel );

	WriteVec3( renderLight.lightRadius );
	WriteVec3( renderLight.lightCenter );

	if ( !renderLight.pointLight ) {
		WriteVec3( renderLight.target );
		WriteVec3( renderLight.right );
		WriteVec3( renderLight.up );
		WriteVec3( renderLight.start );
		WriteVec3( renderLight.end );
	}

	// only idLight has a prelightModel and it's always based on the entityname, so we'll restore it there
	// WriteModel( renderLight.prelightModel );

	WriteInt( renderLight.lightId );

	WriteMaterial( renderLight.shader );

	
	for( i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ ) {
		WriteFloat( renderLight.shaderParms[ i ] );
	}

	if ( renderLight.referenceSound != NULL ) {
		WriteInt( renderLight.referenceSound->Index() );
	} else {
		WriteInt( 0 );
	}
	
}

/*
================
idSaveGame::WriteRefSound
================
*/
void idSaveGame::WriteRefSound( const refSound_t &refSound ) {
	if ( refSound.referenceSound ) {
		WriteInt( refSound.referenceSound->Index() );
	} else {
		WriteInt( 0 );
	}
	WriteVec3( refSound.origin );
	WriteInt( refSound.listenerId );
	WriteSoundShader( refSound.shader );
	WriteFloat( refSound.diversity );
	WriteBool( refSound.waitfortrigger );


	WriteFloat( refSound.parms.minDistance );
	WriteFloat( refSound.parms.maxDistance );
	WriteFloat( refSound.parms.volume );
	WriteFloat( refSound.parms.shakes );
	WriteInt( refSound.parms.soundShaderFlags );
	WriteInt( refSound.parms.soundClass );

}

/*
================
idSaveGame::WriteRenderView
================
*/
void idSaveGame::WriteRenderView( const renderView_t &view ) {
	int i;

	WriteInt( view.viewID );
	WriteInt( view.x );
	WriteInt( view.y );
	WriteInt( view.width );
	WriteInt( view.height );

	WriteFloat( view.fov_x );
	WriteFloat( view.fov_y );
	WriteVec3( view.vieworg );
	WriteMat3( view.viewaxis );

	WriteBool( view.cramZNear );

	WriteInt( view.time );

	for( i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ ) {
		WriteFloat( view.shaderParms[ i ] );
	}
}

/*
===================
idSaveGame::WriteUsercmd
===================
*/
void idSaveGame::WriteUsercmd( const usercmd_t &usercmd ) {
	WriteInt( usercmd.gameFrame );
	WriteInt( usercmd.gameTime );
	WriteInt( usercmd.duplicateCount );
	WriteByte( usercmd.buttons );
	WriteSignedChar( usercmd.forwardmove );
	WriteSignedChar( usercmd.rightmove );
	WriteSignedChar( usercmd.upmove );
	WriteShort( usercmd.angles[0] );
	WriteShort( usercmd.angles[1] );
	WriteShort( usercmd.angles[2] );
	WriteShort( usercmd.mx );
	WriteShort( usercmd.my );
	WriteSignedChar( usercmd.impulse );
	WriteByte( usercmd.flags );
	WriteInt( usercmd.sequence );
}

/*
===================
idSaveGame::WriteContactInfo
===================
*/
void idSaveGame::WriteContactInfo( const contactInfo_t &contactInfo ) {
	WriteShort( (short)contactInfo.type );
	WriteVec3( contactInfo.point );
	WriteVec3( contactInfo.normal );
	WriteFloat( contactInfo.dist );
	WriteInt( contactInfo.contents );
	WriteMaterial( contactInfo.material );
	WriteInt( contactInfo.modelFeature );
	WriteInt( contactInfo.trmFeature );
	WriteInt( contactInfo.entityNum );
	WriteInt( contactInfo.id );
}

/*
===================
idSaveGame::WriteTrace
===================
*/
void idSaveGame::WriteTrace( const trace_t &trace ) {
	WriteFloat( trace.fraction );
	if ( trace.fraction < 1.0f ) {
		WriteVec3( trace.endpos );
		WriteMat3( trace.endAxis );
		WriteContactInfo( trace.c );
	}
}


/*
===================
idRestoreGame::WriteTraceModel
===================
*/
void idSaveGame::WriteTraceModel( const idTraceModel &trace ) {
	int j, k;
	
	WriteInt( (int&)trace.type );
	WriteInt( trace.numVerts );
	for ( j = 0; j < trace.numVerts; j++ ) {
		WriteVec3( trace.verts[j] );
	}
	WriteInt( trace.numEdges );
	for ( j = 0; j <= (trace.numEdges); j++ ) {
		WriteInt( trace.edges[j].v[0] );
		WriteInt( trace.edges[j].v[1] );
		WriteVec3( trace.edges[j].normal );
	}
	WriteInt( trace.numPolys );
	for ( j = 0; j < trace.numPolys; j++ ) {
		WriteVec3( trace.polys[j].normal );
		WriteFloat( trace.polys[j].dist );
		WriteBounds( trace.polys[j].bounds );
		WriteInt( trace.polys[j].numEdges );
		for ( k = 0; k < trace.polys[j].numEdges; k++ ) {
			WriteInt( trace.polys[j].edges[k] );
		}
	}
	WriteVec3( trace.offset );
	WriteBounds( trace.bounds );
	WriteBool( trace.isConvex );
}

/*
===================
idSaveGame::WriteClipModel
===================
*/
void idSaveGame::WriteClipModel( const idClipModel *clipModel ) {
	if ( clipModel != NULL ) {
		WriteBool( true );
		clipModel->Save( this );
	} else {
		WriteBool( false );
	}
}

/*
===================
idSaveGame::WriteSoundCommands
===================
*/
void idSaveGame::WriteSoundCommands( void ) {
	gameSoundWorld->WriteToSaveGame( file );
}

/*
======================
idSaveGame::WriteBuildNumber
======================
*/
void idSaveGame::WriteBuildNumber( const int value ) {
#if API_VERS >= 3
	file->WriteInt( BUILD_NUMBER );
#else
	file->Write( &BUILD_NUMBER, sizeof( BUILD_NUMBER ) );
#endif
}

/*
======================
idSaveGame::WriteWDT
======================
*/
void idSaveGame::WriteWDT( void ) {
	int wdt = SAVE_WDT;

	file->Write( &wdt, sizeof( int ) );
}
/***********************************************************************

	idRestoreGame
	
***********************************************************************/

/*
================
idRestoreGame::RestoreGame
================
*/
idRestoreGame::idRestoreGame( idFile *savefile ) {
	file = savefile;
	debugTabs = 0;
	dictHash = NULL;
}

/*
================
idRestoreGame::~idRestoreGame()
================
*/
idRestoreGame::~idRestoreGame() {
	DeleteDictHash();
}

/*
================
void idRestoreGame::CreateObjects
================
*/
void idRestoreGame::CreateObjects( void ) {
	int i, num, cnum;
	unsigned short s;
	byte _b;
	idTypeInfo *type;
	idStrList classHash;
	bool SmallIndexes;

	ReadInt( cnum );
	SmallIndexes = (cnum < 256);
	classHash.SetGranularity( 1 );
	classHash.SetNum( cnum );
	for( i = 0; i < cnum; i++ ) {
		ReadString( classHash[ i ] );
	}

	ReadInt( num );
	// create all the objects
	objects.SetNum( num + 1 );
	memset( objects.Ptr(), 0, sizeof( objects[ 0 ] ) * objects.Num() );

	for( i = 1; i < objects.Num(); i++ ) {
		if ( SmallIndexes ) {
			ReadByte( _b );
			s = _b;
		} else {
			ReadUnsignedShort( s );
		}
		if ( s < 0 || s > cnum ) {
			Error( "idRestoreGame::CreateObjects: Bad class index '%d'", s );
		}
		type = idClass::GetClass( classHash[(int)s] );
		if ( !type ) {
			Error( "idRestoreGame::CreateObjects: Unknown class '%s'", classHash[s].c_str() );
		}
		objects[ i ] = type->CreateInstance();
#ifdef ID_DEBUG_MEMORY
		InitTypeVariables( objects[i], type->classname, 0xce );
#endif
	}
}

/*
================
void idRestoreGame::PresentObjects
================
*/
void idRestoreGame::PresentObjects( void ) {
	int i;
	// regenerate render entities and render lights because are not saved
	for( i = 1; i < objects.Num(); i++ ) {
		if ( objects[ i ]->IsType( idEntity::Type ) ) {
			idEntity *ent = static_cast<idEntity *>( objects[ i ] );
			ent->UpdateVisuals();
			ent->Present();
		}
	}
}

/*
================
void idRestoreGame::SetupDictHash
================
*/
void idRestoreGame::SetupDictHash ( void ) {

	if ( dictHash ) {
		dictHash->Clear();
	} else {
		dictHash = new idStrPool;
	}

}

/*
================
void idRestoreGame::RestoreObjects
================
*/
void idRestoreGame::RestoreObjects( void ) {
	int i;

	// restore all the objects
	for( i = 1; i < objects.Num(); i++ ) {
		debugTabs = 1;
		if ( developer.GetBool() ) {
			gameLocal.Printf( "+Class:%s\n", objects[ i ]->GetClassname() );
		}
		CallRestore_r( objects[ i ]->GetType(), objects[ i ] );
		if ( developer.GetBool() ) {
			gameLocal.Printf( "-Class:%s\n", objects[ i ]->GetClassname() );
		}
	}

#ifdef ID_DEBUG_MEMORY
	idStr gameState = file->GetName();
	gameState.StripFileExtension();
	WriteGameState_f( idCmdArgs( va( "test %s_restore", gameState.c_str() ), false ) );
	//CompareGameState_f( idCmdArgs( va( "test %s_save", gameState.c_str() ) ) );
	gameLocal.Error( "dumped game states" );
#endif
}

/*
====================
void idRestoreGame::DeleteDictHash
====================
*/
void idRestoreGame::DeleteDictHash( void ) {
	if ( dictHash ) {
		dictHash->Clear();
		delete dictHash;
	}
}

/*
====================
void idRestoreGame::DeleteObjects
====================
*/
void idRestoreGame::DeleteObjects( void ) {

	// Remove the NULL object before deleting
	objects.RemoveIndex( 0 );

	objects.DeleteContents( true );
}

/*
================
idRestoreGame::Error
================
*/
void idRestoreGame::Error( const char *fmt, ... ) {
	va_list	argptr;
	char	text[ 1024 ];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	objects.DeleteContents( true );

	gameLocal.Error( "%s", text );
}

/*
================
idRestoreGame::CallRestore_r
================
*/
void idRestoreGame::CallRestore_r( const idTypeInfo *cls, idClass *obj ) {
	int i;

#ifdef SAVEFILE_COMMENTS
	ReadComment();
#endif

	if ( cls->super ) {
		debugTabs++;
		CallRestore_r( cls->super, obj );
		debugTabs--;
		if ( cls->super->Restore == cls->Restore ) {
			// don't call save on this inheritance level since the function was called in the super class
			return;
		}
	}

	
	if ( developer.GetBool() ) {
		for (i = 0; i < debugTabs; i++) gameLocal.Printf( "    " );
		gameLocal.Printf( "Class c:%s o:%s ... ", cls->classname, obj->GetClassname() );
	}
	
	( obj->*cls->Restore )( (idRestore_I*) this );

	if (!ReadWDT()) {
		gameLocal.Error("Watchdog damaged after c:%s o:%s (offset: %X)", cls->classname, obj->GetClassname(), file->Tell());
	}

#ifdef SAVEFILE_COMMENTS
	ReadComment();
#endif

	if ( developer.GetBool() ) gameLocal.Printf( "OK\n", obj->GetClassname() );
}

/*
================
idRestoreGame::Read
================
*/
void idRestoreGame::Read( void *buffer, int len ) {
	file->Read( buffer, len );
}

/*
================
idRestoreGame::ReadRLE
Read RLE buffer and decompress
================
*/
bool idRestoreGame::ReadRLE( void *buffer, int len ) {
	archive.Restore( file );
//	return archive.Read( buffer, len );
	return archive.DecompressLCS( buffer, len );
}

/*
================
idRestoreGame::ReadInt
================
*/
void idRestoreGame::ReadInt( int &value ) {

#if API_VERS >= 3
	file->ReadInt( value );
#else
	file->Read( &value, sizeof( value ) );
#endif
}

/*
================
idRestoreGame::ReadJoint
================
*/
void idRestoreGame::ReadJoint( jointHandle_t &value ) {
#if API_VERS >= 3
	file->ReadInt( (int&)value );
#else
	file->Read( &value, sizeof( value ) );
#endif
}

/*
================
idRestoreGame::ReadShort
================
*/
void idRestoreGame::ReadShort( short &value ) {
#if API_VERS >= 3
	file->ReadShort( value );
#else
	file->Read( &value, sizeof( value ) );
#endif
}

/*
================
idRestoreGame::ReadSignedShort
================
*/
void idRestoreGame::ReadUnsignedShort( unsigned short &value ) {
	file->Read( &value, sizeof( value ) );
}

/*
================
idRestoreGame::ReadByte
================
*/
void idRestoreGame::ReadByte( byte &value ) {
	file->Read( &value, sizeof( value ) );
}

/*
================
idRestoreGame::ReadSignedChar
================
*/
void idRestoreGame::ReadSignedChar( signed char &value ) {
	file->Read( &value, sizeof( value ) );
}

/*
================
idRestoreGame::ReadFloat
================
*/
void idRestoreGame::ReadFloat( float &value ) {
#if API_VERS >= 3
	file->ReadFloat( value );
#else
	file->Read( &value, sizeof( value ) );
#endif
}

/*
================
idRestoreGame::ReadBool
================
*/
void idRestoreGame::ReadBool( bool &value ) {
char m_byte;

//#if API_VERS >= 3
//	file->ReadBool( value );
//#else
	file->Read( &m_byte, 1 );
	value = (m_byte != 0x00);
//#endif
}

/*
================
idRestoreGame::ReadComment
================
*/
void idRestoreGame::ReadComment( void ) {
	byte bt;

	ReadByte( bt );
	while ( bt ) {
		ReadByte( bt );
	}
}

/*
================
idRestoreGame::ReadString
================
*/
void idRestoreGame::ReadString( idStr &string ) {
	int len_idx;

	ReadInt( len_idx );
	if ( len_idx < 0 ) {
		if ( !dictHash ) {
			gameLocal.Error( "idRestoreGame::ReadString compression index used, but compression is disabled" );
		}
		len_idx = -len_idx - 1;
		if ( len_idx > dictHash->Num() ) {
			gameLocal.Error( "idRestoreGame::ReadString compression error 1" );
		}
		string = (*dictHash)[len_idx]->c_str();
	} else {
		string.Fill( ' ', len_idx );
		file->Read( &string[ 0 ], len_idx );
		if ( dictHash ) {
			dictHash->AllocString( string );
		}
	}
}

/*
================
idRestoreGame::ReadVec2
================
*/
void idRestoreGame::ReadVec2( idVec2 &vec ) {
#if API_VERS >= 3
	file->ReadVec2( vec );
#else
	file->Read( &vec, sizeof( vec ) );
#endif
}

/*
================
idRestoreGame::ReadVec3
================
*/
void idRestoreGame::ReadVec3( idVec3 &vec ) {
#if API_VERS >= 3
	file->ReadVec3( vec );
#else
	file->Read( &vec, sizeof( vec ) );
#endif
}

/*
================
idRestoreGame::ReadVec4
================
*/
void idRestoreGame::ReadVec4( idVec4 &vec ) {
#if API_VERS >= 3
	file->ReadVec4( vec );
#else
	file->Read( &vec, sizeof( vec ) );
#endif
}

/*
================
idRestoreGame::ReadVec6
================
*/
void idRestoreGame::ReadVec6( idVec6 &vec ) {
#if API_VERS >= 3
	file->ReadVec6( vec );
#else
	file->Read( &vec, sizeof( vec ) );
#endif
}

/*
================
idRestoreGame::ReadBounds
================
*/
void idRestoreGame::ReadBounds( idBounds &bounds ) {
	file->Read( &bounds, sizeof( bounds ) );
	LittleRevBytes( &bounds, sizeof(float), sizeof(bounds)/sizeof(float) );
}

/*
================
idRestoreGame::ReadWinding
================
*/
void idRestoreGame::ReadWinding( idWinding &w )
{
	int i, num;
#if API_VERS >= 3
	file->ReadInt( num );
#else
	file->Read( &num, sizeof(num) );
#endif
	w.SetNumPoints( num );
	for ( i = 0; i < num; i++ ) {
		file->Read( &w[i], sizeof(idVec5) );
		LittleRevBytes(&w[i], sizeof(float), sizeof(idVec5)/sizeof(float) );
	}
}

/*
================
idRestoreGame::ReadMat3
================
*/
void idRestoreGame::ReadMat3( idMat3 &mat ) {
#if API_VERS >= 3
	file->ReadMat3( mat );
#else
	file->Read( &mat, sizeof( mat ) );
#endif
}

/*
================
idRestoreGame::ReadAngles
================
*/
void idRestoreGame::ReadAngles( idAngles &angles ) {
	file->Read( &angles, sizeof( angles ) );
	LittleRevBytes(&angles, sizeof(float), sizeof(idAngles)/sizeof(float) );
}

/*
================
idRestoreGame::ReadObject
================
*/
void idRestoreGame::ReadObject( idClass *&obj ) {
	int index;
	idStr iStr;

	ReadInt( index );

	if ( ( index < 0 ) || ( index >= objects.Num() ) ) {
		Error( "idRestoreGame::ReadObject: invalid object index" );
	}
	obj = objects[ index ];
}

/*
================
idRestoreGame::ReadStaticObject
================
*/
void idRestoreGame::ReadStaticObject( idClass &obj ) {
	if ( developer.GetBool() ) {
		if (debugTabs > 1) gameLocal.Printf( "\n" );
		gameLocal.Printf( "RSO Class o:%s ... \n", obj.GetClassname() );
	}

	CallRestore_r( obj.GetType(), &obj );

	if ( developer.GetBool() ) {
		gameLocal.Printf( "End RSO..." );
	}

}

/*
================
idRestoreGame::ReadDict
================
*/
void idRestoreGame::ReadDict( idDict *dict ) {
	int num;
	int i;
	idStr key;
	idStr value;

	ReadInt( num );

	if ( num < 0 ) {
		dict = NULL;
	} else {
		dict->Clear();
		for( i = 0; i < num; i++ ) {
			ReadString( key );
			ReadString( value );
			dict->Set( key, value );
		}
	}
}

/*
================
idRestoreGame::ReadMaterial
================
*/
void idRestoreGame::ReadMaterial( const idMaterial *&material ) {
	idStr name;

	ReadString( name );
	if ( !name.Length() ) {
		material = NULL;
	} else {
		material = declManager->FindMaterial( name );
	}
}

/*
================
idRestoreGame::ReadSkin
================
*/
void idRestoreGame::ReadSkin( const idDeclSkin *&skin ) {
	idStr name;

	ReadString( name );
	if ( !name.Length() ) {
		skin = NULL;
	} else {
		skin = declManager->FindSkin( name );
	}
}

/*
================
idRestoreGame::ReadParticle
================
*/
void idRestoreGame::ReadParticle( const idDeclParticle *&particle ) {
	idStr name;

	ReadString( name );
	if ( !name.Length() ) {
		particle = NULL;
	} else {
		particle = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, name ) );
	}
}

/*
================
idRestoreGame::ReadFX
================
*/
void idRestoreGame::ReadFX( const idDeclFX *&fx ) {
	idStr name;

	ReadString( name );
	if ( !name.Length() ) {
		fx = NULL;
	} else {
		fx = static_cast<const idDeclFX *>( declManager->FindType( DECL_FX, name ) );
	}
}

/*
================
idRestoreGame::ReadSoundShader
================
*/
void idRestoreGame::ReadSoundShader( const idSoundShader *&shader ) {
	idStr name;

	ReadString( name );
	if ( !name.Length() ) {
		shader = NULL;
	} else {
		shader = declManager->FindSound( name );
	}
}

/*
================
idRestoreGame::ReadModelDef
================
*/
void idRestoreGame::ReadModelDef( const idDeclModelDef *&modelDef ) {
	idStr name;

	ReadString( name );
	if ( !name.Length() ) {
		modelDef = NULL;
	} else {
		modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, name, false ) );
	}
}

/*
================
idRestoreGame::ReadModel
================
*/
void idRestoreGame::ReadModel( idRenderModel *&model ) {
	idStr name;

	ReadString( name );
	if ( !name.Length() ) {
		model = NULL;
	} else {
		model = renderModelManager->FindModel( name );
	}
}

/*
================
idRestoreGame::ReadUserInterface
================
*/
void idRestoreGame::ReadUserInterface( idUserInterface *&ui ) {
	idStr name;

	ReadString( name );
	if ( !name.Length() ) {
		ui = NULL;
	} else {
		bool unique;
		ReadBool( unique );
		ui = uiManager->FindGui( name, true, unique );
		if ( ui ) {
			if ( ui->ReadFromSaveGame( file ) == false ) {
				Error( "idSaveGame::ReadUserInterface: ui failed to read properly\n" );
			} else {
				ui->StateChanged( gameLocal.time );
			}
		}
	}
}

/*
================
idRestoreGame::ReadRenderEntity
================
*/
void idRestoreGame::ReadRenderEntity( renderEntity_t &renderEntity ) {
	int i;
	int index;

	ReadModel( renderEntity.hModel );

	ReadInt( renderEntity.entityNum );
	ReadInt( renderEntity.bodyId );

	ReadBounds( renderEntity.bounds );

	// callback is set by class's Restore function
	renderEntity.callback = NULL;
	renderEntity.callbackData = NULL;

	ReadInt( renderEntity.suppressSurfaceInViewID );
	ReadInt( renderEntity.suppressShadowInViewID );
	ReadInt( renderEntity.suppressShadowInLightID );
	ReadInt( renderEntity.allowSurfaceInViewID );

	ReadVec3( renderEntity.origin );
	ReadMat3( renderEntity.axis );

	ReadMaterial( renderEntity.customShader );
	ReadMaterial( renderEntity.referenceShader );
	ReadSkin( renderEntity.customSkin );

	ReadInt( index );
	renderEntity.referenceSound = gameSoundWorld->EmitterForIndex( index );

	for( i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ ) {
		ReadFloat( renderEntity.shaderParms[ i ] );
	}

	for( i = 0; i < MAX_RENDERENTITY_GUI; i++ ) {
		ReadUserInterface( renderEntity.gui[ i ] );
	}

	// idEntity will restore "cameraTarget", which will be used in idEntity::Present to restore the remoteRenderView
	renderEntity.remoteRenderView = NULL;

	renderEntity.joints = NULL;
	renderEntity.numJoints = 0;

	ReadFloat( renderEntity.modelDepthHack );

	ReadBool( renderEntity.noSelfShadow );
	ReadBool( renderEntity.noShadow );
	ReadBool( renderEntity.noDynamicInteractions );
	ReadBool( renderEntity.weaponDepthHack );

	ReadInt( renderEntity.forceUpdate );
}

/*
================
idRestoreGame::ReadRenderLight
================
*/
void idRestoreGame::ReadRenderLight( renderLight_t &renderLight ) {
	int index;
	int i;

	ReadMat3( renderLight.axis );
	ReadVec3( renderLight.origin );

	ReadInt( renderLight.suppressLightInViewID );
	ReadInt( renderLight.allowLightInViewID );
	ReadBool( renderLight.noShadows );
	ReadBool( renderLight.noSpecular );
	ReadBool( renderLight.pointLight );
	ReadBool( renderLight.parallel );

	ReadVec3( renderLight.lightRadius );
	ReadVec3( renderLight.lightCenter );

	if ( !renderLight.pointLight ) {
		ReadVec3( renderLight.target );
		ReadVec3( renderLight.right );
		ReadVec3( renderLight.up );
		ReadVec3( renderLight.start );
		ReadVec3( renderLight.end );
	}

	// only idLight has a prelightModel and it's always based on the entityname, so we'll restore it there
	// ReadModel( renderLight.prelightModel );
	renderLight.prelightModel = NULL;

	ReadInt( renderLight.lightId );

	ReadMaterial( renderLight.shader );

	for( i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ ) {
		ReadFloat( renderLight.shaderParms[ i ] );
	}

	ReadInt( index );
	renderLight.referenceSound = gameSoundWorld->EmitterForIndex( index );
}

/*
================
idRestoreGame::ReadRefSound
================
*/
void idRestoreGame::ReadRefSound( refSound_t &refSound ) {
	int		index;
	ReadInt( index );

	refSound.referenceSound = gameSoundWorld->EmitterForIndex( index );
	ReadVec3( refSound.origin );
	ReadInt( refSound.listenerId );
	ReadSoundShader( refSound.shader );
	ReadFloat( refSound.diversity );
	ReadBool( refSound.waitfortrigger );

	ReadFloat( refSound.parms.minDistance );
	ReadFloat( refSound.parms.maxDistance );
	ReadFloat( refSound.parms.volume );
	ReadFloat( refSound.parms.shakes );
	ReadInt( refSound.parms.soundShaderFlags );
	ReadInt( refSound.parms.soundClass );
}

/*
================
idRestoreGame::ReadRenderView
================
*/
void idRestoreGame::ReadRenderView( renderView_t &view ) {
	int i;

	ReadInt( view.viewID );
	ReadInt( view.x );
	ReadInt( view.y );
	ReadInt( view.width );
	ReadInt( view.height );

	ReadFloat( view.fov_x );
	ReadFloat( view.fov_y );
	ReadVec3( view.vieworg );
	ReadMat3( view.viewaxis );

	ReadBool( view.cramZNear );

	ReadInt( view.time );

	for( i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ ) {
		ReadFloat( view.shaderParms[ i ] );
	}
}

/*
=================
idRestoreGame::ReadUsercmd
=================
*/
void idRestoreGame::ReadUsercmd( usercmd_t &usercmd ) {
	ReadInt( usercmd.gameFrame );
	ReadInt( usercmd.gameTime );
	ReadInt( usercmd.duplicateCount );
	ReadByte( usercmd.buttons );
	ReadSignedChar( usercmd.forwardmove );
	ReadSignedChar( usercmd.rightmove );
	ReadSignedChar( usercmd.upmove );
	ReadShort( usercmd.angles[0] );
	ReadShort( usercmd.angles[1] );
	ReadShort( usercmd.angles[2] );
	ReadShort( usercmd.mx );
	ReadShort( usercmd.my );
	ReadSignedChar( usercmd.impulse );
	ReadByte( usercmd.flags );
	ReadInt( usercmd.sequence );
}

/*
===================
idRestoreGame::ReadContactInfo
===================
*/
void idRestoreGame::ReadContactInfo( contactInfo_t &contactInfo ) {
	ReadShort( (short &)contactInfo.type );
	ReadVec3( contactInfo.point );
	ReadVec3( contactInfo.normal );
	ReadFloat( contactInfo.dist );
	ReadInt( contactInfo.contents );
	ReadMaterial( contactInfo.material );
	ReadInt( contactInfo.modelFeature );
	ReadInt( contactInfo.trmFeature );
	ReadInt( contactInfo.entityNum );
	ReadInt( contactInfo.id );
}

/*
===================
idRestoreGame::ReadTrace
===================
*/
void idRestoreGame::ReadTrace( trace_t &trace ) {
	ReadFloat( trace.fraction );
	if ( trace.fraction < 1.0f ) {
		ReadVec3( trace.endpos );
		ReadMat3( trace.endAxis );
		ReadContactInfo( trace.c );
	}
}

/*
 ===================
 idRestoreGame::ReadTraceModel
 ===================
 */
void idRestoreGame::ReadTraceModel( idTraceModel &trace ) {
	int j, k;
	
	ReadInt( (int&)trace.type );
	ReadInt( trace.numVerts );
	memset( &trace.verts, 0, sizeof( trace.verts ) );
	for ( j = 0; j < trace.numVerts; j++ ) {
		ReadVec3( trace.verts[j] );
	}
	ReadInt( trace.numEdges );
	memset( &trace.edges, 0, sizeof( trace.edges ) );
	for ( j = 0; j <= trace.numEdges; j++ ) {
		ReadInt( trace.edges[j].v[0] );
		ReadInt( trace.edges[j].v[1] );
		ReadVec3( trace.edges[j].normal );
	}
	ReadInt( trace.numPolys );
	memset( &trace.polys, 0, sizeof( trace.polys ) );
	for ( j = 0; j < trace.numPolys; j++ ) {
		ReadVec3( trace.polys[j].normal );
		ReadFloat( trace.polys[j].dist );
		ReadBounds( trace.polys[j].bounds );
		ReadInt( trace.polys[j].numEdges );
		for ( k = 0; k < trace.polys[j].numEdges; k++ ) {
			ReadInt( trace.polys[j].edges[k] );
		}
	}
	ReadVec3( trace.offset );
	ReadBounds( trace.bounds );
	ReadBool( trace.isConvex );
}

/*
=====================
idRestoreGame::ReadClipModel
=====================
*/
void idRestoreGame::ReadClipModel( idClipModel *&clipModel ) {
	bool restoreClipModel;

	ReadBool( restoreClipModel );
	if ( restoreClipModel ) {
		clipModel = new idClipModel();
		clipModel->Restore( this );
	} else {
		clipModel = NULL;
	}
}

/*
=====================
idRestoreGame::ReadSoundCommands
=====================
*/
void idRestoreGame::ReadSoundCommands( void ) {
	gameSoundWorld->StopAllSounds();
	gameSoundWorld->ReadFromSaveGame( file );

}

/*
=====================
idRestoreGame::ReadBuildNumber
=====================
*/
void idRestoreGame::ReadBuildNumber( void ) {
#if API_VERS >= 3
	file->ReadInt( buildNumber );
#else
	file->Read( &buildNumber, sizeof( buildNumber ) );
#endif
}

bool idRestoreGame::ReadWDT( void ) {
	int wdt;

	file->Read( &wdt, sizeof( int ) );
	
	if (wdt != SAVE_WDT) return false;
	return true;
}
/*
=====================
idRestoreGame::GetBuildNumber
=====================
*/
int idRestoreGame::GetBuildNumber( void ) {
	return buildNumber;
}


/*
=====================
RLECompactor::RLECompactor
=====================
*/
RLECompactor::RLECompactor ( ) {
	ptr = NULL;
	buffSize = 0;
	usedSize = 0;
	granularity = 1024;
}

RLECompactor::~RLECompactor ( ) {
	if (ptr) {
		delete [] ptr;
	}
}

bool RLECompactor::AssureIO ( int Size ) const {
	int realsize;
	
	if ( !ioptr ) return false;
	if ( !ptr ) return false;

	realsize = (ioptr - ptr) + Size;

	return ( usedSize >= realsize );
}

bool RLECompactor::AssureIO ( int Size, bool Expand ) {
	int realsize;
	
	if ( !ioptr ) ioptr = ptr;
	if ( !ptr ) {
		realsize = Size;
	} else {
		realsize = (ioptr - ptr) + Size;
	}
	if ( usedSize < realsize ) {	
		// Need to expand
		if ( Expand ) AssureSize( realsize );
		return false;
	} else {
		return true;
	}
}

void RLECompactor::AssureSize ( int Size ) {
	int newsize;

	if ( buffSize < Size ) {
		newsize = Size + granularity - 1;
		newsize -= newsize % granularity;
		if ( newsize != buffSize ) {
			SetupSize( newsize, false );
		}
	}
	usedSize = Size;
}

void RLECompactor::SetupSize ( int Size, bool erase ) {
	int copysize;
	int iopos;
	byte* oldPtr;

	if (ptr) {
		oldPtr = ptr;
		iopos = (ioptr - ptr);
		if ( Size > 0 ) {
			ptr = new byte[Size];
			if ( erase ) {
				memset( ptr, 0x00, Size );
				usedSize = 0;
				buffSize = Size;
				ioptr = ptr;
			}else{
				if (usedSize > buffSize) usedSize = buffSize;
				copysize = (Size > usedSize)?usedSize:Size; //Minimum from the buffSize and Size
				ioptr = ptr + ((iopos > Size)?Size:iopos);	//Restore the ioptr to the old position
				memcpy( ptr, oldPtr, copysize );
			}
		} else {
			Size = 0;
			ptr = NULL;
			ioptr = ptr;
		}
		buffSize = Size;
		if ( usedSize > buffSize ) usedSize = buffSize; //Fixup buffSize
		delete [] oldPtr;
	} else {
		if ( Size > 0 ) {
			ptr = new byte[Size];
			ioptr = ptr;
			memset( ptr, 0x00, Size );
			usedSize = 0;
			buffSize = Size;
		}
	}
}

void RLECompactor::CropSize ( void ) {
	SetupSize( usedSize );
}

bool RLECompactor::CompressRLE ( const void *buffer, int len ) {
	SetupSize( len );
	usedSize = len;
	memcpy( ptr, buffer, len );
	return true;
}

bool RLECompactor::DecompressRLE ( void *buffer, int len ) {
	if ( len != buffSize )
		return false;

	memcpy( buffer, ptr, len );
	return true;
}

/* 
	Compress-decompress pair for LCS
*/

bool RLECompactor::CompressLCS ( const void *buffer, int len ) {
	const char* p;
	const char* endP;
	const char* b;
	const char* Raw;
	unsigned char Wr;
	int Cycles;
	int Raws;

	p = (const char*)buffer;
	endP = p + len;
	b = (const char*)buffer;
	Raw = NULL;

	while ( p <= endP ) {
		if ( (*p != *b) || (p == endP) ) {
			Cycles = (p - b);
			if (Cycles > 4) {
				if (Raw) {
					//Emit RAW
					Raws = (b - Raw);
					while ( Raws > 255 ) {
						Wr = 0x00;	
						Write( &Wr, sizeof(Wr) );
						Wr = 255;	
						Write( &Wr, sizeof(Wr) );
						Raws -= 255;
						Write( Raw, 0xFF );
					}
					if ( Raws > 0 ) {
						Wr = 0x00;	
						Write( &Wr, sizeof(Wr) );
						Wr = (Raws & 0xFF);	
						Write( &Wr, sizeof(Wr) );
						Write( Raw, Raws );
					}
					Raw = NULL;
				}
				//Emit cycle
				while ( Cycles > 255 ) {
					Wr = 255;	
					Write( &Wr, sizeof(Wr) );
					Cycles -= 255;
					Wr = *b;
					Write( &Wr, sizeof(Wr) );
				}
				if ( Cycles > 4 ) {
					Wr = (Cycles & 0xFF);	
					Write( &Wr, sizeof(Wr) );
					Wr = *b;
					Write( &Wr, sizeof(Wr) );
				} else {
					p -= Cycles;
				}
			} else {
				if (!Raw) Raw = b;
			}
			b = p;
		}
		p++;
	}

	if (Raw) {
		//Emit RAW
		Raws = (endP - Raw);
		while ( Raws > 255 ) {
			Wr = 0x00;	
			Write( &Wr, sizeof(Wr) );
			Wr = 255;	
			Write( &Wr, sizeof(Wr) );
			Raws -= 255;
			Write( Raw, 0xFF );
		}
		if ( Raws > 0 ) {
			Wr = 0x00;	
			Write( &Wr, sizeof(Wr) );
			Wr = (Raws & 0xFF);	
			Write( &Wr, sizeof(Wr) );
			Write( Raw, Raws );
		}
		Raw = NULL;
	}

	return true;
}

bool RLECompactor::DecompressLCS ( void *buffer, int len ) {
	char* p;
	const char* endP;
	unsigned char Wr;
	unsigned char B;

	p = (char*)buffer;
	endP = p + len;

	while ( Read( &Wr, sizeof(Wr) ) && p <= endP) {
		if ( Wr == 0x00 ) {
			if (!Read( &Wr, sizeof(Wr) ) ) return false; 
			if ( (p + Wr) > endP ) return false; //Overflow
			Read( p, Wr );
			p += Wr;
		} else {
			if ( (p + Wr) > endP ) return false; //Overflow
			if (!Read( &B, sizeof(B) ) ) return false;
			memset( p, B, Wr );
			p += Wr;
		}
	}

	if ( p < endP ) {
		return false;
	}

	return true;
}

/* 
	Compress-decompress pair for LCS_F
*/

bool RLECompactor::CompressLCS_F ( idFile *savefile ) {
	const char* p;
	const char* endP;
	const char* b;
	const char* Raw;
	unsigned char Wr;
	int Cycles;
	int Raws;

	p = (const char*)ptr;
	endP = p + usedSize;
	b = (const char*)ptr;
	Raw = NULL;

	while ( p <= endP ) {
		if ( (*p != *b) || (p == endP) ) {
			Cycles = (p - b);
			if (Cycles > 4) {
				if (Raw) {
					//Emit RAW
					Raws = (b - Raw);
					while ( Raws > 255 ) {
						Wr = 0x00;	
						savefile->Write( &Wr, sizeof(Wr) );
						Wr = 255;	
						savefile->Write( &Wr, sizeof(Wr) );
						Raws -= 255;
						savefile->Write( Raw, 0xFF );
					}
					if ( Raws > 0 ) {
						Wr = 0x00;	
						savefile->Write( &Wr, sizeof(Wr) );
						Wr = (Raws & 0xFF);	
						savefile->Write( &Wr, sizeof(Wr) );
						savefile->Write( Raw, Raws );
					}
					Raw = NULL;
				}
				//Emit cycle
				while ( Cycles > 255 ) {
					Wr = 255;	
					savefile->Write( &Wr, sizeof(Wr) );
					Cycles -= 255;
					Wr = *b;
					savefile->Write( &Wr, sizeof(Wr) );
				}
				if ( Cycles > 4 ) {
					Wr = (Cycles & 0xFF);	
					savefile->Write( &Wr, sizeof(Wr) );
					Wr = *b;
					savefile->Write( &Wr, sizeof(Wr) );
				} else {
					p -= Cycles;
				}
			} else {
				if (!Raw) Raw = b;
			}
			b = p;
		}
		p++;
	}

	if (Raw) {
		//Emit RAW
		Raws = (endP - Raw);
		while ( Raws > 255 ) {
			Wr = 0x00;	
			savefile->Write( &Wr, sizeof(Wr) );
			Wr = 255;	
			savefile->Write( &Wr, sizeof(Wr) );
			Raws -= 255;
			savefile->Write( Raw, 0xFF );
		}
		if ( Raws > 0 ) {
			Wr = 0x00;	
			savefile->Write( &Wr, sizeof(Wr) );
			Wr = (Raws & 0xFF);	
			savefile->Write( &Wr, sizeof(Wr) );
			savefile->Write( Raw, Raws );
		}
		Raw = NULL;
	}

	return true;
}

bool RLECompactor::DecompressLCS_F ( idFile *savefile, int len ) {
	char* p;
	const char* endP;
	unsigned char Wr;
	unsigned char B;

	p = (char*)ptr;
	
	if ( len > 0 ) {
		endP = p + len;
		AssureSize( len );
	} else {
		endP = p + usedSize;
	}

	while ( p <= endP) {
		savefile->Read( &Wr, sizeof(Wr) );
		if ( Wr == 0x00 ) {
			savefile->Read( &Wr, sizeof(Wr) ); 
			if ( (p + Wr) > endP ) return false; //Overflow
			savefile->Read( p, Wr );
			p += Wr;
		} else {
			if ( (p + Wr) > endP ) return false; //Overflow
			savefile->Read( &B, sizeof(B) );
			memset( p, B, Wr );
			p += Wr;
		}
	}
	return true;
}

/*
================
RLECompactor::Read

Read from file
================
*/

void RLECompactor::Restore ( idFile *savefile ) {
	int Sz;

#if API_VERS >= 3
		savefile->ReadInt( Sz );
#else
		savefile->Read( &Sz, sizeof( Sz ) );
#endif
	AssureSize( Sz );
	if ( Sz > 0 ) {
		savefile->Read( ptr, Sz );
	}
	ioptr = ptr;
}

/*
================
RLECompactor::SetGranularity

Write from file
================
*/

void RLECompactor::Save ( idFile *savefile ) const {

#if API_VERS >= 3
		savefile->WriteInt( usedSize );
#else
		savefile->Write( &usedSize, sizeof( usedSize ) );
#endif
	if ( usedSize > 0 ) {
		savefile->Write( ptr, usedSize );
	}
}

/*
================
RLECompactor::Write

Write to the archive
================
*/

ID_INLINE void RLECompactor::Write ( const void *buffer, int len ) {
	AssureIO( len, true );
	memcpy( ioptr, buffer, len );
	ioptr += len;
}

/*
================
RLECompactor::Write

Read from the archive
================
*/

ID_INLINE bool RLECompactor::Read ( void *buffer, int len ) {

	if ( AssureIO( len ) ) {
		memcpy( buffer, ioptr, len );
		ioptr += len;
		return true;
	}
	return false;
}
/*
================
RLECompactor::SetGranularity

Sets the base size of the array and resizes the array to match.
================
*/
void RLECompactor::SetGranularity( int newgranularity ) {
	int newsize;

	assert( newgranularity > 0 );
	granularity = newgranularity;

	if ( ptr ) {
		// resize it to the closest level of granularity
		newsize = usedSize + granularity - 1;
		newsize -= newsize % granularity;
		if ( newsize != buffSize ) {
			SetupSize( newsize );
		}
	}
}

