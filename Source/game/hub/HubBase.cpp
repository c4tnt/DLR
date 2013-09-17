#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

#include "TypeInfo.h"

/***********************************************************************

	idHubSave
	
***********************************************************************/
/*
================
idHubSave::idHubSave()
================
*/
idHubSave::idHubSave() {
}

/*
================
idHubSave::~idHubSave()
================
*/
idHubSave::~idHubSave() {
}


/*
================
idHubSave::CallHSave_r
================
*/
void idHubSave::CallHSave_r( const idTypeInfo *cls, const idClass *obj ) {
	if ( cls->super ) {
		CallHSave_r( cls->super, obj );
		if ( cls->super->Save == cls->Save ) {
			// don't call save on this inheritance level since the function was called in the super class
			return;
		}
	}
	( obj->*cls->Save )( this );
}

/*
================
idHubSave::Write
================
*/
void idHubSave::Write( const void *buffer, int len ) {
	LinkedBuffer->Write( buffer, len );
}

/*
================
idHubSave::WriteRLE
Compress buffer with RLE and write out
================
*/
void idHubSave::WriteRLE( const void *buffer, int len ) {
	LinkedBuffer->Write( buffer, len );
}

/*
================
idHubSave::WriteInt
================
*/
void idHubSave::WriteInt( const int value ) {
	LinkedBuffer->Write( &value, sizeof(value) );
}

/*
================
idHubSave::WriteJoint
================
*/
void idHubSave::WriteJoint( const jointHandle_t value ) {
	LinkedBuffer->Write( &value, sizeof(value) );
}

/*
================
idHubSave::WriteShort
================
*/
void idHubSave::WriteShort( const short value ) {
	LinkedBuffer->Write( &value, sizeof(value) );
}

/*
================
idHubSave::WriteUnsignedShort
================
*/
void idHubSave::WriteUnsignedShort( const unsigned short value ) {
	LinkedBuffer->Write( &value, sizeof(value) );
}

/*
================
idHubSave::WriteByte
================
*/
void idHubSave::WriteByte( const byte value ) {
	LinkedBuffer->Write( &value, sizeof(value) );
}

/*
================
idHubSave::WriteSignedChar
================
*/
void idHubSave::WriteSignedChar( const signed char value ) {
	LinkedBuffer->Write( &value, sizeof(value) );
}

/*
================
idHubSave::WriteFloat
================
*/
void idHubSave::WriteFloat( const float value ) {
	LinkedBuffer->Write( &value, sizeof(value) );
}

/*
================
idHubSave::WriteBool
================
*/
void idHubSave::WriteBool( const bool value ) {
char m_byte;

	m_byte = value?0xFF:0x00;
	LinkedBuffer->Write( &m_byte, sizeof(m_byte) );
}

/*
================
idHubSave::WriteComment
================
*/
void idHubSave::WriteComment( const char *string ) {
	int len;

	len = strlen( string );
//	file->Write( string, len );
	WriteByte( 0x00 );
}

/*
================
idHubSave::WriteString
================
*/
void idHubSave::WriteString( const char *string ) {
	int sz = strlen( string );
	LinkedBuffer->Write( &sz, sizeof(sz) );
	LinkedBuffer->Write( string, sz );
}

/*
================
idHubSave::WriteVec2
================
*/
void idHubSave::WriteVec2( const idVec2 &vec ) {
	LinkedBuffer->Write( &vec, sizeof( vec ) );
}

/*
================
idHubSave::WriteVec3
================
*/
void idHubSave::WriteVec3( const idVec3 &vec ) {
	LinkedBuffer->Write( &vec, sizeof( vec ) );
}

/*
================
idHubSave::WriteVec4
================
*/
void idHubSave::WriteVec4( const idVec4 &vec ) {
	LinkedBuffer->Write( &vec, sizeof( vec ) );
}

/*
================
idHubSave::WriteVec6
================
*/
void idHubSave::WriteVec6( const idVec6 &vec ) {
	LinkedBuffer->Write( &vec, sizeof( vec ) );
}

/*
================
idHubSave::WriteBounds
================
*/
void idHubSave::WriteBounds( const idBounds &bounds ) {
	LinkedBuffer->Write( &bounds, sizeof(bounds) );
}

/*
================
idHubSave::WriteBounds
================
*/
void idHubSave::WriteWinding( const idWinding &w )
{
	int i, num;
	num = w.GetNumPoints();
	LinkedBuffer->Write( &num, sizeof(num) );
	for ( i = 0; i < num; i++ ) {
		LinkedBuffer->Write( &w[i], sizeof(idVec5) );
	}
}

/*
================
idHubSave::WriteMat3
================
*/
void idHubSave::WriteMat3( const idMat3 &mat ) {
	LinkedBuffer->Write( &mat, sizeof( mat ) );
}

/*
================
idHubSave::WriteAngles
================
*/
void idHubSave::WriteAngles( const idAngles &angles ) {
	LinkedBuffer->Write( &angles, sizeof( angles ) );
}

/*
================
idHubSave::WriteObject
================
*/
void idHubSave::WriteObject( const idClass *obj ) {
	int index;

/*	index = objects.FindIndex( obj );
	if ( index < 0 ) {
		gameLocal.DPrintf( "idSaveGame::WriteObject - WriteObject FindIndex failed\n" );

		// Use the NULL index
		index = 0;
	}
	WriteInt( index );*/
}

/*
================
idHubSave::WriteStaticObject
================
*/
void idHubSave::WriteStaticObject( const idClass &obj ) {
	CallHSave_r( obj.GetType(), &obj );
}

/*
================
idHubSave::WriteDict
================
*/
void idHubSave::WriteDict( const idDict *dict ) {
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
idHubSave::WriteMaterial
================
*/
void idHubSave::WriteMaterial( const idMaterial *material ) {
	if ( !material ) {
		WriteString( "" );
	} else {
		WriteString( material->GetName() );
	}
}

/*
================
idHubSave::WriteSkin
================
*/
void idHubSave::WriteSkin( const idDeclSkin *skin ) {
	if ( !skin ) {
		WriteString( "" );
	} else {
		WriteString( skin->GetName() );
	}
}

/*
================
idHubSave::WriteParticle
================
*/
void idHubSave::WriteParticle( const idDeclParticle *particle ) {
	if ( !particle ) {
		WriteString( "" );
	} else {
		WriteString( particle->GetName() );
	}
}

/*
================
idHubSave::WriteFX
================
*/
void idHubSave::WriteFX( const idDeclFX *fx ) {
	if ( !fx ) {
		WriteString( "" );
	} else {
		WriteString( fx->GetName() );
	}
}

/*
================
idHubSave::WriteModelDef
================
*/
void idHubSave::WriteModelDef( const idDeclModelDef *modelDef ) {
	if ( !modelDef ) {
		WriteString( "" );
	} else {
		WriteString( modelDef->GetName() );
	}
}

/*
================
idHubSave::WriteSoundShader
================
*/
void idHubSave::WriteSoundShader( const idSoundShader *shader ) {
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
idHubSave::WriteModel
================
*/
void idHubSave::WriteModel( const idRenderModel *model ) {
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
idHubSave::WriteUserInterface
================
*/
void idHubSave::WriteUserInterface( const idUserInterface *ui, bool unique ) {
	const char *name;

	if ( !ui ) {
		WriteString( "" );
	} else {
		name = ui->Name();
		WriteString( name );
		WriteBool( unique );
		WriteDict( &ui->State() );
		
		BufferToFile _tbo(LinkedBuffer);
		if ( ui->WriteToSaveGame( &_tbo ) == false ) {
			gameLocal.Error( "idHubSave::WriteUserInterface: ui failed to write properly\n" );
		}
	}
}

/*
================
idHubSave::WriteRenderEntity
================
*/
void idHubSave::WriteRenderEntity( const renderEntity_t &renderEntity ) {
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
idHubSave::WriteRenderLight
================
*/
void idHubSave::WriteRenderLight( const renderLight_t &renderLight ) {
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
idHubSave::WriteRefSound
================
*/
void idHubSave::WriteRefSound( const refSound_t &refSound ) {
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
idHubSave::WriteRenderView
================
*/
void idHubSave::WriteRenderView( const renderView_t &view ) {
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
idHubSave::WriteUsercmd
===================
*/
void idHubSave::WriteUsercmd( const usercmd_t &usercmd ) {
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
idHubSave::WriteContactInfo
===================
*/
void idHubSave::WriteContactInfo( const contactInfo_t &contactInfo ) {
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
idHubSave::WriteTrace
===================
*/
void idHubSave::WriteTrace( const trace_t &trace ) {
	WriteFloat( trace.fraction );
	if ( trace.fraction < 1.0f ) {
		WriteVec3( trace.endpos );
		WriteMat3( trace.endAxis );
		WriteContactInfo( trace.c );
	}
}



/*
===================
idHubSave::WriteTraceModel
===================
*/
void idHubSave::WriteTraceModel( const idTraceModel &trace ) {
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
idHubSave::WriteClipModel
===================
*/
void idHubSave::WriteClipModel( const idClipModel *clipModel ) {
	if ( clipModel != NULL ) {
		WriteBool( true );
		clipModel->Save( this );
	} else {
		WriteBool( false );
	}
}

/*
===================
idHubSave::WriteSoundCommands
===================
*/
void idHubSave::WriteSoundCommands( void ) {
}

/*
======================
idHubSave::WriteBuildNumber
======================
*/
void idHubSave::WriteBuildNumber( const int value ) {
}

/*
======================
idHubSave::WriteWDT
======================
*/
void idHubSave::WriteWDT( void ) {
}

/***********************************************************************

	idHubRestore
	
***********************************************************************/

/*
================
idHubRestore::idHubRestore
================
*/
idHubRestore::idHubRestore( ) {
}

/*
================
idHubRestore::~idHubRestore()
================
*/
idHubRestore::~idHubRestore() {
}

/*
================
idHubRestore::Error
================
*/
void idHubRestore::Error( const char *fmt, ... ) {
	va_list	argptr;
	char	text[ 1024 ];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	gameLocal.Error( "%s", text );
}

/*
================
idHubRestore::CallHRestore_r
================
*/
void idHubRestore::CallHRestore_r( const idTypeInfo *cls, idClass *obj ) {
}

/*
================
idHubRestore::Read
================
*/
void idHubRestore::Read( void *buffer, int len ) {
}

/*
================
idHubRestore::ReadRLE
Read RLE buffer and decompress
================
*/
bool idHubRestore::ReadRLE( void *buffer, int len ) {
	return true;
}

/*
================
idHubRestore::ReadInt
================
*/
void idHubRestore::ReadInt( int &value ) {
}

/*
================
idHubRestore::ReadJoint
================
*/
void idHubRestore::ReadJoint( jointHandle_t &value ) {
}

/*
================
idHubRestore::ReadShort
================
*/
void idHubRestore::ReadShort( short &value ) {
}

/*
================
idHubRestore::ReadSignedShort
================
*/
void idHubRestore::ReadUnsignedShort( unsigned short &value ) {
}

/*
================
idHubRestore::ReadByte
================
*/
void idHubRestore::ReadByte( byte &value ) {
}

/*
================
idHubRestore::ReadSignedChar
================
*/
void idHubRestore::ReadSignedChar( signed char &value ) {
}

/*
================
idHubRestore::ReadFloat
================
*/
void idHubRestore::ReadFloat( float &value ) {
}

/*
================
idHubRestore::ReadBool
================
*/
void idHubRestore::ReadBool( bool &value ) {
}

/*
================
idHubRestore::ReadComment
================
*/
void idHubRestore::ReadComment( void ) {
	byte bt;

	ReadByte( bt );
	while ( bt ) {
		ReadByte( bt );
	}
}

/*
================
idHubRestore::ReadString
================
*/
void idHubRestore::ReadString( idStr &string ) {
}

/*
================
idHubRestore::ReadVec2
================
*/
void idHubRestore::ReadVec2( idVec2 &vec ) {
}

/*
================
idHubRestore::ReadVec3
================
*/
void idHubRestore::ReadVec3( idVec3 &vec ) {
}

/*
================
idHubRestore::ReadVec4
================
*/
void idHubRestore::ReadVec4( idVec4 &vec ) {
}

/*
================
idHubRestore::ReadVec6
================
*/
void idHubRestore::ReadVec6( idVec6 &vec ) {
}

/*
================
idHubRestore::ReadBounds
================
*/
void idHubRestore::ReadBounds( idBounds &bounds ) {
}

/*
================
idHubRestore::ReadWinding
================
*/
void idHubRestore::ReadWinding( idWinding &w )
{
}

/*
================
idHubRestore::ReadMat3
================
*/
void idHubRestore::ReadMat3( idMat3 &mat ) {
}

/*
================
idHubRestore::ReadAngles
================
*/
void idHubRestore::ReadAngles( idAngles &angles ) {
}

/*
================
idHubRestore::ReadObject
================
*/
void idHubRestore::ReadObject( idClass *&obj ) {
}

/*
================
idHubRestore::ReadStaticObject
================
*/
void idHubRestore::ReadStaticObject( idClass &obj ) {
}

/*
================
idHubRestore::ReadDict
================
*/
void idHubRestore::ReadDict( idDict *dict ) {
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
idHubRestore::ReadMaterial
================
*/
void idHubRestore::ReadMaterial( const idMaterial *&material ) {
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
idHubRestore::ReadSkin
================
*/
void idHubRestore::ReadSkin( const idDeclSkin *&skin ) {
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
idHubRestore::ReadParticle
================
*/
void idHubRestore::ReadParticle( const idDeclParticle *&particle ) {
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
idHubRestore::ReadFX
================
*/
void idHubRestore::ReadFX( const idDeclFX *&fx ) {
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
idHubRestore::ReadSoundShader
================
*/
void idHubRestore::ReadSoundShader( const idSoundShader *&shader ) {
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
idHubRestore::ReadModelDef
================
*/
void idHubRestore::ReadModelDef( const idDeclModelDef *&modelDef ) {
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
idHubRestore::ReadModel
================
*/
void idHubRestore::ReadModel( idRenderModel *&model ) {
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
idHubRestore::ReadUserInterface
================
*/
void idHubRestore::ReadUserInterface( idUserInterface *&ui ) {
	idStr name;

	ReadString( name );
	if ( !name.Length() ) {
		ui = NULL;
	} else {
		bool unique;
		ReadBool( unique );
		ui = uiManager->FindGui( name, true, unique );
		if ( ui ) {
			// needs implementation
		}
	}
}

/*
================
idHubRestore::ReadRenderEntity
================
*/
void idHubRestore::ReadRenderEntity( renderEntity_t &renderEntity ) {
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
idHubRestore::ReadRenderLight
================
*/
void idHubRestore::ReadRenderLight( renderLight_t &renderLight ) {
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
idHubRestore::ReadRefSound
================
*/
void idHubRestore::ReadRefSound( refSound_t &refSound ) {
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
idHubRestore::ReadRenderView
================
*/
void idHubRestore::ReadRenderView( renderView_t &view ) {
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
idHubRestore::ReadUsercmd
=================
*/
void idHubRestore::ReadUsercmd( usercmd_t &usercmd ) {
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
idHubRestore::ReadContactInfo
===================
*/
void idHubRestore::ReadContactInfo( contactInfo_t &contactInfo ) {
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
idHubRestore::ReadTrace
===================
*/
void idHubRestore::ReadTrace( trace_t &trace ) {
	ReadFloat( trace.fraction );
	if ( trace.fraction < 1.0f ) {
		ReadVec3( trace.endpos );
		ReadMat3( trace.endAxis );
		ReadContactInfo( trace.c );
	}
}

/*
===================
idHubRestore::ReadTraceModel
===================
*/
void idHubRestore::ReadTraceModel( idTraceModel &trace ) {
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
idHubRestore::ReadClipModel
=====================
*/
void idHubRestore::ReadClipModel( idClipModel *&clipModel ) {
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
idHubRestore::ReadSoundCommands
=====================
*/
void idHubRestore::ReadSoundCommands( void ) {
}

/*
=====================
idHubRestore::ReadBuildNumber
=====================
*/
void idHubRestore::ReadBuildNumber( void ) {
}

bool idHubRestore::ReadWDT( void ) {
	int wdt;

	if (wdt != SAVE_WDT) return false;
	return true;
}
/*
=====================
idHubRestore::GetBuildNumber
=====================
*/
int idHubRestore::GetBuildNumber( void ) {
	return 0;
}


/*
=====================
idHUB::idHUB
=====================
*/
idHUB::idHUB( ) {
	currentCluster.Clear();
	currentSpawnMark.Clear();
	nextMap.Clear();
	hubState = HUBSYS_NOTREADY;
	sameMap = false;
}

/*
=====================
idHUB::~idHUB
=====================
*/
idHUB::~idHUB( ) {
}

/*
=====================
idHUB::LoadPure
=====================
*/
void idHUB::LoadPure( ) {
	currentCluster.Clear();
	currentSpawnMark.Clear();
	sameMap = false;
	hubState = HUBSYS_INGAME;
}

/*
=====================
idHUB::ChangeLevel
=====================
*/
bool idHUB::ChangeLevel( const char* mapName, const char* spawn_mark ) {
	
	currentSpawnMark = spawn_mark;
	nextMap = mapName;
	sameMap = ( thisMap == nextMap );

	// TODO: Actually it would be better to load the <nextmap> map and assure if there is a requested spawn.

	if ( sameMap ) {
		// TODO: Perform the soft-reloading for a map
	}

	hubState = HUBSYS_SESSION_W;
	
	return true;
}

/*
=====================
idHUB::MapStartup
=====================
*/
bool idHUB::MapStartup( idEntity* worldSpawn, bool ClusterPurge ) {

	const char* cluster;
	int i;

	if (!worldSpawn) {
		return false;
	}

	cluster = worldSpawn->spawnArgs.GetString( "cluster", "" );

	hubState = HUBSYS_PREPARING;
	nextMap.Clear();

	if ( !cluster || !*cluster || ClusterPurge ) {
		// Clear hubinfo
		PurgeHI();
	} else {
		// Find map in the maphash

	}

	hubState = HUBSYS_INGAME;
	return true;
}

/*
=====================
idHUB::LoadBaseMap
=====================
*/
bool idHUB::LoadBaseMap( const idMapFile* mapFile ) {
	
	// So, we are interested only in entities here
	return true;
}


/*
=====================
idHUB::PurgeHI
=====================
*/
void idHUB::PurgeHI( void ) {

}


/*
=====================
idHUB::SaveCurrentMap
=====================
*/
bool idHUB::SaveCurrentMap( ) {
	return false;
}

/*
=====================
idHUB::GetPendingSessionCommand
=====================
*/
bool idHUB::GetPendingSessionCommand( char* out, size_t buffer_sz ) {

	idStr tmp;

	if ( hubState == HUBSYS_SESSION_W ) {
		tmp = "map ";
		tmp += nextMap;
		strncpy( out, tmp.c_str(), buffer_sz );
		return true;
	}
	return false;
};

/*
=====================
idHUB::getState
=====================
*/
HUBSYS_STATE idHUB::getState( void ) {
	return hubState;
};