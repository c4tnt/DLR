#ifndef __FILEEX_H__
#define __FILEEX_H__

class BufferToFile: public idFile {

public:
							BufferToFile( BufferBase* buffer ) { _buffer = buffer; _name.Clear(); }
							BufferToFile( BufferBase* buffer, const char* name ) { _buffer = buffer; _name = name; }

	virtual					~BufferToFile( void );
							// Get the name of the file.
	virtual const char *	GetName( void ) { return _name.c_str(); } 
							// Get the full file path.
	virtual const char *	GetFullPath( void ) { return _name.c_str(); }
							// Read data from the file to the buffer.
	virtual int				Read( void *buffer, int len ) { return _buffer->Read( buffer, len ); } 
							// Write data from the buffer to the file.
	virtual int				Write( const void *buffer, int len ) { return _buffer->Write( buffer, len ); } 
							// Returns the length of the file.
	virtual int				Length( void ) { return _buffer->Size(); }
							// Return a time value for reload operations.
	virtual unsigned int	Timestamp( void );
							// Returns offset in file.
	virtual int				Tell( void );
							// Forces flush on files being writting to.
	virtual void			ForceFlush( void ) {}
							// Causes any buffered data to be written to the file.
	virtual void			Flush( void ) {}
							// Seek on a file.
	virtual int				Seek( long offset, fsOrigin_t origin );
							// Go back to the beginning of the file.
	virtual void			Rewind( void ) { _buffer->ResetIO( false ); }
							// Like fprintf.
	virtual int				Printf( const char *fmt, ... ) id_attribute((format(printf,2,3)));
							// Like fprintf but with argument pointer
	virtual int				VPrintf( const char *fmt, va_list arg );
							// Write a string with high precision floating point numbers to the file.
	virtual int				WriteFloatString( const char *fmt, ... ) id_attribute((format(printf,2,3)));

#if (API_VERS >= 3)

		// Endian portable alternatives to Read(...)
	virtual int				ReadInt( int &value ) { return 0; }
	virtual int				ReadUnsignedInt( unsigned int &value ) { return 0; }
	virtual int				ReadShort( short &value ) { return 0; }
	virtual int				ReadUnsignedShort( unsigned short &value ) { return 0; }
	virtual int				ReadChar( char &value ) { return 0; }
	virtual int				ReadUnsignedChar( unsigned char &value ) { return 0; }
	virtual int				ReadFloat( float &value ) { return 0; }
	virtual int				ReadBool( bool &value ) { return 0; }
	virtual int				ReadString( idStr &string ) { return 0; }
	virtual int				ReadVec2( idVec2 &vec ) { return 0; }
	virtual int				ReadVec3( idVec3 &vec ) { return 0; }
	virtual int				ReadVec4( idVec4 &vec ) { return 0; }
	virtual int				ReadVec6( idVec6 &vec ) { return 0; }
	virtual int				ReadMat3( idMat3 &mat ) { return 0; }
	
	// Endian portable alternatives to Write(...)
	virtual int				WriteInt( const int value ) { return 0; }
	virtual int				WriteUnsignedInt( const unsigned int value ) { return 0; }
	virtual int				WriteShort( const short value ) { return 0; }
	virtual int				WriteUnsignedShort( unsigned short value ) { return 0; }
	virtual int				WriteChar( const char value ) { return 0; }
	virtual int				WriteUnsignedChar( const unsigned char value ) { return 0; }
	virtual int				WriteFloat( const float value ) { return 0; }
	virtual int				WriteBool( const bool value ) { return 0; }
	virtual int				WriteString( const char *string ) { return 0; }
	virtual int				WriteVec2( const idVec2 &vec ) { return 0; }
	virtual int				WriteVec3( const idVec3 &vec ) { return 0; }
	virtual int				WriteVec4( const idVec4 &vec ) { return 0; }
	virtual int				WriteVec6( const idVec6 &vec ) { return 0; }
	virtual int				WriteMat3( const idMat3 &mat ) { return 0; }

#endif
private:
	
	BufferBase*				_buffer;
	idStr					_name;
};
#endif