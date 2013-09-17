#ifndef __BUFFER_H__
#define __BUFFER_H__

/*
===============================================================================

	File-like buffer for read and write. Can be linked as idFile

===============================================================================
*/

class BufferBase {
public:
							BufferBase ( );
	virtual					~BufferBase ( );

	virtual void			SetupSize ( int Size, bool erase = false );		//Setup internal size
	virtual void			CropSize ( void );								//Crop internal buffer to actual size

	virtual void			FRead ( idFile *file );
	virtual void			FWrite ( idFile *file ) const;

	virtual bool			AssureIO ( int Size, bool Expand );				//Assure that we can write (Size) bytes after the ioptr
	virtual int				AssureIO ( int Size ) const;						//Assure that we can write (Size) bytes after the ioptr
	virtual void			AssureSize ( int Size );						//Assure that buffer can recieve (Size) bytes
	virtual int				Read ( void *buffer, int len );					//Read data from the internal buffer
	virtual int				Write ( const void *buffer, int len );			//Write data to the internal buffer
	virtual void			ResetIO ( bool clear );							//Reset size and ioptr

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

ID_INLINE int BufferBase::GetGranularity( void ) const {
	return granularity;
}

ID_INLINE int BufferBase::Size() const {
	return usedSize; 
}

ID_INLINE int BufferBase::FullSize() const { 
	return buffSize; 
}

ID_INLINE void BufferBase::ResetIO( bool clear ) { 
	ioptr = ptr;
	if ( clear ) usedSize = 0;
}


/*
===============================================================================

	File-like buffer with inline compression

===============================================================================
*/

class BufferLCS: public BufferBase {

							BufferLCS ( );
	virtual					~BufferLCS ( );

	virtual void			SetupSize ( int Size, bool erase = false );		//Setup internal size
	virtual void			CropSize ( void );								//Crop internal buffer to actual size

	virtual void			FRead ( idFile *file );
	virtual void			FWrite ( idFile *file ) const;

	virtual bool			AssureIO ( int Size, bool Expand );				//Assure that we can write (Size) bytes after the ioptr
	virtual int				AssureIO ( int Size ) const;						//Assure that we can write (Size) bytes after the ioptr
	virtual void			AssureSize ( int Size );						//Assure that buffer can recieve (Size) bytes
	virtual int				Read ( void *buffer, int len );					//Read data from the internal buffer
	virtual int				Write ( const void *buffer, int len );			//Write data to the internal buffer
	virtual void			ResetIO ( bool clear );							//Reset size and ioptr

};


#endif