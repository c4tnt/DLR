#ifndef __HUB_BASE_H__
#define	__HUB_BASE_H__

class idHubSave : public idSave_I {
public:
							idHubSave();
							~idHubSave();

	virtual void			Write( const void *buffer, int len );
	virtual void			WriteRLE( const void *buffer, int len );
	virtual void			WriteInt( const int value );
	virtual void			WriteJoint( const jointHandle_t value );
	virtual void			WriteShort( const short value );
	virtual void			WriteUnsignedShort( const unsigned short value );
	virtual void			WriteByte( const byte value );
	virtual void			WriteSignedChar( const signed char value );
	virtual void			WriteFloat( const float value );
	virtual void			WriteBool( const bool value );
	virtual void			WriteString( const char *string );
	virtual void			WriteVec2( const idVec2 &vec );
	virtual void			WriteVec3( const idVec3 &vec );
	virtual void			WriteVec4( const idVec4 &vec );
	virtual void			WriteVec6( const idVec6 &vec );
	virtual void			WriteWinding( const idWinding &winding );
	virtual void			WriteBounds( const idBounds &bounds );
	virtual void			WriteMat3( const idMat3 &mat );
	virtual void			WriteAngles( const idAngles &angles );
	virtual void			WriteObject( const idClass *obj );
	virtual void			WriteStaticObject( const idClass &obj );
	virtual void			WriteDict( const idDict *dict );
	virtual void			WriteMaterial( const idMaterial *material );
	virtual void			WriteSkin( const idDeclSkin *skin );
	virtual void			WriteParticle( const idDeclParticle *particle );
	virtual void			WriteFX( const idDeclFX *fx );
	virtual void			WriteSoundShader( const idSoundShader *shader );
	virtual void			WriteModelDef( const class idDeclModelDef *modelDef );
	virtual void			WriteModel( const idRenderModel *model );
	virtual void			WriteUserInterface( const idUserInterface *ui, bool unique );
	virtual void			WriteRenderEntity( const renderEntity_t &renderEntity );
	virtual void			WriteRenderLight( const renderLight_t &renderLight );
	virtual void			WriteRefSound( const refSound_t &refSound );
	virtual void			WriteRenderView( const renderView_t &view );
	virtual void			WriteUsercmd( const usercmd_t &usercmd );
	virtual void			WriteContactInfo( const contactInfo_t &contactInfo );
	virtual void			WriteTrace( const trace_t &trace );
	virtual void			WriteTraceModel( const idTraceModel &trace );
	virtual void			WriteClipModel( const class idClipModel *clipModel );
	virtual void			WriteSoundCommands( void );
	virtual void			WriteBuildNumber( const int value );

	virtual	void			WriteWDT( void );
	virtual void			WriteComment( const char *string );
	
private:
	void					CallHSave_r( const idTypeInfo *cls, const idClass *obj );
	BufferBase*				LinkedBuffer;
};

class idHubRestore : public idRestore_I {
public:
							idHubRestore();
							~idHubRestore();

	void					Error( const char *fmt, ... ) id_attribute((format(printf,2,3)));

	virtual void			Read( void *buffer, int len );
	virtual bool			ReadRLE( void *buffer, int len );
	virtual void			ReadInt( int &value );
	virtual void			ReadJoint( jointHandle_t &value );
	virtual void			ReadShort( short &value );
	virtual void			ReadUnsignedShort( unsigned short &value );
	virtual void			ReadByte( byte &value );
	virtual void			ReadSignedChar( signed char &value );
	virtual void			ReadFloat( float &value );
	virtual void			ReadBool( bool &value );
	virtual void			ReadString( idStr &string );
	virtual void			ReadVec2( idVec2 &vec );
	virtual void			ReadVec3( idVec3 &vec );
	virtual void			ReadVec4( idVec4 &vec );
	virtual void			ReadVec6( idVec6 &vec );
	virtual void			ReadWinding( idWinding &winding );
	virtual void			ReadBounds( idBounds &bounds );
	virtual void			ReadMat3( idMat3 &mat );
	virtual void			ReadAngles( idAngles &angles );
	virtual void			ReadObject( idClass *&obj );
	virtual void			ReadStaticObject( idClass &obj );
	virtual void			ReadDict( idDict *dict );
	virtual void			ReadMaterial( const idMaterial *&material );
	virtual void			ReadSkin( const idDeclSkin *&skin );
	virtual void			ReadParticle( const idDeclParticle *&particle );
	virtual void			ReadFX( const idDeclFX *&fx );
	virtual void			ReadSoundShader( const idSoundShader *&shader );
	virtual void			ReadModelDef( const idDeclModelDef *&modelDef );
	virtual void			ReadModel( idRenderModel *&model );
	virtual void			ReadUserInterface( idUserInterface *&ui );
	virtual void			ReadRenderEntity( renderEntity_t &renderEntity );
	virtual void			ReadRenderLight( renderLight_t &renderLight );
	virtual void			ReadRefSound( refSound_t &refSound );
	virtual void			ReadRenderView( renderView_t &view );
	virtual void			ReadUsercmd( usercmd_t &usercmd );
	virtual void			ReadContactInfo( contactInfo_t &contactInfo );
	virtual void			ReadTrace( trace_t &trace );
	virtual void			ReadTraceModel( idTraceModel &trace );
	virtual void			ReadClipModel( idClipModel *&clipModel );
	virtual void			ReadSoundCommands( void );
	virtual void			ReadBuildNumber( void );
	virtual int				GetBuildNumber( void );

	virtual	bool			ReadWDT( void );
	virtual void			ReadComment( void );


private:
	
	void					CallHRestore_r( const idTypeInfo *cls, idClass *obj );
};


// c4tnt: not really by ID ;)
class idHUBEntry {
public:
							idHUBEntry();
							~idHUBEntry();
private:

	idStr					mapName;
	idHashIndex				entityHash;		// Saved map entity list
	
};

typedef struct player_exit_state_s {
	
	idStr	spawn_marker;
	idVec3	spawn_offset;

} player_exit_state_t;

enum HUBSYS_STATE {
	HUBSYS_NOTREADY,
	HUBSYS_PREPARING,
	HUBSYS_INGAME,
	HUBSYS_SESSION_W
};

class idHUB {
public:

							idHUB();
							~idHUB();

/*
    ________________________________________ CROSSMAP INTERACTION _____________________________________________
*/

							// Transfer entity
	bool					Transfer( idEntity *ent, char* mapName, char* targetSpot );

							// Transfer Global Character immediately
	bool					MoveGlobalChar( char* charName, char* mapName, char* targetSpot );

							// Transfer global character when player exits map
							// mapName = NULL -> transfer with player
							// targetSpot = NULL -> transfer to player spawn spot ( can be NULL only when mapName = NULL )
	bool					MoveGlobalCharDelayed( char* charName, char* mapName, char* targetSpot );
	
							// Returns name of the map with specified character
	char*					WhereIsChar( char* charName );

							// Get transfered entity count ( NULL = Loaded map )
	int						TransfersNum( char* mapName ) const;

							// Send event, sheduled on time
	bool					SheduleEvent( idEvent* ev, int time, char* mapName );	

							// Send startup event
	bool					SheduleLoadEvent( idEvent* ev, char* mapName );

/*
    __________________________________________ MAP MANAGERMENT ________________________________________________
*/

							// Load pure hubinfo
	void					LoadPure( );

							// Forced level change. Save clusterinfo for the current map and proceed to the next map.
	bool					ChangeLevel( const char* mapName, const char* spawn_mark );

							// Call this during map loading
	bool					MapStartup( idEntity* worldSpawn, bool ClusterPurge = true );

							// Load original map state, used for HUB compression
	bool					LoadBaseMap( const idMapFile* mapFile );

							// Update hubinfo for the current map immediately
	bool					SaveCurrentMap( );

							// Load all sheduled events to the event system
	bool					SendEvents();

							// Returns spawn point for the object. For clients ID = [0... MAX_CLIENTS], 
							// for global characters and transfer ID > MAX_CLIENTS. Get transfer ID from the transfer list.
							// Works only after level load
	idEntity *				GetSpawnPoint( int ID ) const;
	idEntity *				GetSpawnPointChar( char* charName ) const;

/*
	_____________________________________________ CONNECTIVITY ________________________________________________
*/

	bool					GetPendingSessionCommand( char* out, size_t buffer_sz );
	HUBSYS_STATE			getState( void );

/*
    _____________________________________________ SAVE\LOAD ___________________________________________________
*/
							// Save and load. Only for savegames
	void					Save( idSave_I *savefile ) const;
	void					Restore( idRestore_I *savefile );

private:

							// Remove all saved hubinfo
	void					PurgeAll();
	void					PurgeHI();
private:

	idStr					currentCluster;
	idStr					currentSpawnMark;

							// Load_map and pending session cmd						

	idStr					nextMap;
	idStr					thisMap;
	HUBSYS_STATE			hubState;
	bool					sameMap;
	bool					Loaded;

							// HIDB

};
#endif