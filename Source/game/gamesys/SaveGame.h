// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __SAVEGAME_H__
#define __SAVEGAME_H__

#define SAVE_WDT	0xABAB
//#define SAVEFILE_COMMENTS
/*
	Save/Load interfaces
*/

class idSave_I 
{
public:
	virtual	void			Write( const void *buffer, int len ) = 0;
	virtual	void			WriteRLE( const void *buffer, int len ) = 0;
	virtual	void			WriteInt( const int value ) = 0;
	virtual	void			WriteJoint( const jointHandle_t value ) = 0; 
	virtual	void			WriteShort( const short value ) = 0;
	virtual	void			WriteUnsignedShort( const unsigned short value ) = 0;
	virtual	void			WriteByte( const byte value ) = 0;
	virtual	void			WriteSignedChar( const signed char value ) = 0;
	virtual	void			WriteFloat( const float value ) = 0;
	virtual	void			WriteBool( const bool value ) = 0;
	virtual	void			WriteString( const char *string ) = 0;
	virtual	void			WriteVec2( const idVec2 &vec ) = 0;
	virtual	void			WriteVec3( const idVec3 &vec ) = 0;
	virtual	void			WriteVec4( const idVec4 &vec ) = 0;
	virtual	void			WriteVec6( const idVec6 &vec ) = 0;
	virtual	void			WriteWinding( const idWinding &winding ) = 0;
	virtual	void			WriteBounds( const idBounds &bounds ) = 0;
	virtual	void			WriteMat3( const idMat3 &mat ) = 0;
	virtual	void			WriteAngles( const idAngles &angles ) = 0;
	virtual	void			WriteObject( const idClass *obj ) = 0; 
	virtual	void			WriteStaticObject( const idClass &obj ) = 0;
	virtual	void			WriteDict( const idDict *dict ) = 0;
	virtual	void			WriteMaterial( const idMaterial *material ) = 0;
	virtual	void			WriteSkin( const idDeclSkin *skin ) = 0;
	virtual	void			WriteParticle( const idDeclParticle *particle ) = 0;
	virtual	void			WriteFX( const idDeclFX *fx ) = 0;
	virtual	void			WriteSoundShader( const idSoundShader *shader ) = 0;
	virtual	void			WriteModelDef( const class idDeclModelDef *modelDef ) = 0;
	virtual	void			WriteModel( const idRenderModel *model ) = 0;
	virtual	void			WriteUserInterface( const idUserInterface *ui, bool unique ) = 0;
	virtual	void			WriteRenderEntity( const renderEntity_t &renderEntity ) = 0;
	virtual	void			WriteRenderLight( const renderLight_t &renderLight ) = 0;
	virtual	void			WriteRefSound( const refSound_t &refSound ) = 0;
	virtual	void			WriteRenderView( const renderView_t &view ) = 0;
	virtual	void			WriteUsercmd( const usercmd_t &usercmd ) = 0;
	virtual	void			WriteContactInfo( const contactInfo_t &contactInfo ) = 0;
	virtual	void			WriteTrace( const trace_t &trace ) = 0;
	virtual	void			WriteTraceModel( const idTraceModel &trace ) = 0;
	virtual	void			WriteClipModel( const class idClipModel *clipModel ) = 0;
	virtual	void			WriteSoundCommands( void ) = 0;
	virtual	void			WriteBuildNumber( const int value ) = 0;
	virtual	void			WriteWDT( void ) = 0;
	virtual	void			WriteComment( const char *string ) = 0;
};

class idRestore_I 
{
public:
	virtual void			Error( const char *fmt, ... ) = 0 id_attribute((format(printf,2,3)));

	virtual	void			Read( void *buffer, int len ) = 0;
	virtual	bool			ReadRLE( void *buffer, int len ) = 0;
	virtual	void			ReadInt( int &value ) = 0;
	virtual	void			ReadJoint( jointHandle_t &value ) = 0;
	virtual	void			ReadShort( short &value ) = 0;
	virtual	void			ReadUnsignedShort( unsigned short &value ) = 0;
	virtual	void			ReadByte( byte &value ) = 0;
	virtual	void			ReadSignedChar( signed char &value ) = 0;
	virtual	void			ReadFloat( float &value ) = 0;
	virtual	void			ReadBool( bool &value ) = 0;
	virtual	void			ReadString( idStr &string ) = 0;
	virtual	void			ReadVec2( idVec2 &vec ) = 0;
	virtual	void			ReadVec3( idVec3 &vec ) = 0;
	virtual	void			ReadVec4( idVec4 &vec ) = 0;
	virtual	void			ReadVec6( idVec6 &vec ) = 0;
	virtual	void			ReadWinding( idWinding &winding ) = 0;
	virtual	void			ReadBounds( idBounds &bounds ) = 0;
	virtual	void			ReadMat3( idMat3 &mat ) = 0;
	virtual	void			ReadAngles( idAngles &angles ) = 0;
	virtual	void			ReadObject( idClass *&obj ) = 0;
	virtual	void			ReadStaticObject( idClass &obj ) = 0;
	virtual	void			ReadDict( idDict *dict ) = 0;
	virtual	void			ReadMaterial( const idMaterial *&material ) = 0;
	virtual	void			ReadSkin( const idDeclSkin *&skin ) = 0;
	virtual	void			ReadParticle( const idDeclParticle *&particle ) = 0;
	virtual	void			ReadFX( const idDeclFX *&fx ) = 0;
	virtual	void			ReadSoundShader( const idSoundShader *&shader ) = 0;
	virtual	void			ReadModelDef( const idDeclModelDef *&modelDef ) = 0;
	virtual	void			ReadModel( idRenderModel *&model ) = 0;
	virtual	void			ReadUserInterface( idUserInterface *&ui ) = 0;
	virtual	void			ReadRenderEntity( renderEntity_t &renderEntity ) = 0;
	virtual	void			ReadRenderLight( renderLight_t &renderLight ) = 0;
	virtual	void			ReadRefSound( refSound_t &refSound ) = 0;
	virtual	void			ReadRenderView( renderView_t &view ) = 0;
	virtual	void			ReadUsercmd( usercmd_t &usercmd ) = 0;
	virtual	void			ReadContactInfo( contactInfo_t &contactInfo ) = 0;
	virtual	void			ReadTrace( trace_t &trace ) = 0;
	virtual	void			ReadTraceModel( idTraceModel &trace ) = 0;
	virtual	void			ReadClipModel( idClipModel *&clipModel ) = 0;
	virtual	void			ReadSoundCommands( void ) = 0;
	virtual	void			ReadBuildNumber( void ) = 0;
	virtual int				GetBuildNumber( void ) = 0;
	virtual	bool			ReadWDT( void ) = 0;
	virtual	void			ReadComment( void ) = 0;

};

/*

Save game related helper classes.

*/

const int INITIAL_RELEASE_BUILD_NUMBER = 1262;

class RLECompactor {
public:
	RLECompactor ( );
	~RLECompactor ( );

	void					SetupSize ( int Size, bool erase = false );	//Setup internal size
	void					CropSize ( void );							//Crop internal buffer to actual size

	bool					CompressRLE ( const void *buffer, int len );	//Compress data from the external buffer to the internal buffer
	bool					DecompressRLE ( void *buffer, int len );		//Decompress data from the internal buffer to the external buffer
	bool					CompressLCS ( const void *buffer, int len );	//Compress (Long chains only) data from the external buffer to the internal buffer
	bool					DecompressLCS ( void *buffer, int len );		//Decompress (Long chains only) data from the internal buffer to the external buffer

	bool					CompressLCS_F ( idFile *savefile );				//Compress (Long chains only) data from the internal buffer to file
	bool					DecompressLCS_F ( idFile *savefile, int len );			//Decompress (Long chains only) data from file to the internal buffer
	void					Restore ( idFile *savefile );
	void					Save ( idFile *savefile ) const;

	bool					AssureIO ( int Size, bool Expand );				//Assure that we can write (Size) bytes after the ioptr
	bool					AssureIO ( int Size ) const;					//Assure that we can write (Size) bytes after the ioptr
	void					AssureSize ( int Size );						//Assure that buffer can recieve (Size) bytes
	bool					Read ( void *buffer, int len );					//Read data from the internal buffer
	void					Write ( const void *buffer, int len );			//Write data to the internal buffer
	void					ResetIO ( bool clear );							//Reset size and ioptr

	byte*					Ptr();										//Get internal buffer
	int						Size() const;								//Get actual size
	int						FullSize() const;							//Get real size
	void					SetGranularity( int newgranularity );		// set new granularity
	int						GetGranularity( void ) const;				// get the current granularity

private:
	int						buffSize;
	int						usedSize;
	int						granularity;
	byte*					ptr;
	byte*					ioptr;										// for file-like operations
};

ID_INLINE int RLECompactor::GetGranularity( void ) const {
	return granularity;
}

ID_INLINE int RLECompactor::Size() const {
	return usedSize; 
}

ID_INLINE int RLECompactor::FullSize() const { 
	return buffSize; 
}

ID_INLINE void RLECompactor::ResetIO( bool clear ) { 
	ioptr = ptr;
	if ( clear ) usedSize = 0;
}

class idSaveGame : public idSave_I {
public:
							idSaveGame( idFile *savefile );
							~idSaveGame();

	void					Close( void );

	void					AddObject( const idClass *obj );
	void					WriteObjectList( void );
	void					SetupDictHash ( void );

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
	idFile *				file;
	idStrPool *				dictHash;
	idList<const idClass *>	objects;
	RLECompactor			archive;

	void					CallSave_r( const idTypeInfo *cls, const idClass *obj );
};

class idRestoreGame : public idRestore_I {
public:
							idRestoreGame( idFile *savefile );
							~idRestoreGame();

	void					CreateObjects( void );
	void					RestoreObjects( void );
	void					PresentObjects( void );
	void					DeleteObjects( void );
	void					SetupDictHash ( void );
	void					DeleteDictHash ( void );

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
	int						buildNumber;

	int						debugTabs;
	idStrPool *				dictHash;
	
	idFile *				file;

	RLECompactor			archive;

	idList<idClass *>		objects;

	void					CallRestore_r( const idTypeInfo *cls, idClass *obj );
};

#endif /* !__SAVEGAME_H__*/
