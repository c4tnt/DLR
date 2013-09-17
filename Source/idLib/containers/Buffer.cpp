#include "../precompiled.h"
#pragma hdrstop


/*
=====================
BufferBase::BufferBase
=====================
*/
BufferBase::BufferBase ( ) {
	ptr = NULL;
	ioptr = NULL;
	buffSize = 0;
	usedSize = 0;
}

BufferBase::~BufferBase ( ) {
	if (ptr) {
		delete [] ptr;
	}
	ptr = NULL;
	ioptr = NULL;
}

int BufferBase::AssureIO ( int Size ) const {
	int readsize;
	
	if ( !ioptr ) return 0;
	if ( !ptr ) return 0;

	readsize = ioptr - ptr;
	if ( usedSize >= readsize ) {
		return ((usedSize - readsize) >= Size)?Size:(usedSize - readsize);
	} else {
		return 0;
	}
}

bool BufferBase::AssureIO ( int Size, bool Expand ) {
	int realsize;
	
	if ( !ioptr ) ioptr = ptr;
	if ( !ptr ) {
		realsize = Size;
	} else {
		realsize = (ioptr - ptr) + Size;
	}
	if ( usedSize < realsize ) {	
		// Need to expand
		if ( Expand ) {
			AssureSize( realsize );
			return true;
		} else {
			return false;
		}
	} else {
		return true;
	}
}

void BufferBase::AssureSize ( int Size ) {
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

void BufferBase::SetupSize ( int Size, bool erase ) {
	int copysize;
	byte* oldPtr;

	if (ptr) {
		// Convert effective address to offset
		DWORD offset = ioptr - ptr;

		oldPtr = ptr;
		if ( Size > 0 ) {
			ptr = new byte[Size];
			if ( erase ) {
				memset( ptr, 0x00, Size );
				usedSize = 0;
				buffSize = Size;
			}else{
				if (usedSize > buffSize) usedSize = buffSize;
				copysize = (Size > usedSize)?usedSize:Size; //Minimum from the buffSize and Size
				memcpy( ptr, oldPtr, copysize );
			}
			if ( offset > usedSize ) offset = usedSize;
			// Restore ptr effective address
			ioptr = ptr + offset;
		} else {
			Size = 0;
			ptr = NULL;
			ioptr = NULL;
		}
		buffSize = Size;
		if ( usedSize > buffSize ) usedSize = buffSize; //Fixup usedSize
		delete [] oldPtr;
	} else {
		if ( Size > 0 ) {
			ptr = new byte[Size];
			memset( ptr, 0x00, Size );
			usedSize = 0;
			buffSize = Size;
			ioptr = ptr;
		} else {
			ptr = NULL;
			ioptr = NULL;
		}
	}
}

void BufferBase::CropSize ( void ) {
	SetupSize( usedSize );
}


/*
================
BufferBase::FRead

Read from file
================
*/

void BufferBase::FRead ( idFile *file ) {
	int Sz;

#if API_VERS >= 3
		file->ReadInt( Sz );
#else
		file->Read( &Sz, sizeof( Sz ) );
#endif
	AssureSize( Sz );
	if ( Sz > 0 ) {
		file->Read( ptr, Sz );
	}
	ioptr = ptr;
}

/*
================
BufferBase::FWrite

Write from file
================
*/

void BufferBase::FWrite ( idFile *file ) const {

#if API_VERS >= 3
		file->WriteInt( usedSize );
#else
		file->Write( &usedSize, sizeof( usedSize ) );
#endif
	if ( usedSize > 0 ) {
		file->Write( ptr, usedSize );
	}
}

/*
================
BufferBase::Write

Write to the archive
================
*/

ID_INLINE int BufferBase::Write ( const void *buffer, int len ) {
	AssureIO( len, true );
	memcpy( ioptr, buffer, len );
	ioptr += len;
	return len;
}

/*
================
BufferBase::Read

Read from the archive
================
*/

ID_INLINE int BufferBase::Read ( void *buffer, int len ) {

	int rd;
	rd = AssureIO( len );

	if ( rd ) {
		memcpy( buffer, ioptr, rd );
		ioptr += rd;
		return rd;
	}
	return 0;
}
/*
================
BufferBase::SetGranularity

Sets the base size of the array and resizes the array to match.
================
*/
void BufferBase::SetGranularity( int newgranularity ) {
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

