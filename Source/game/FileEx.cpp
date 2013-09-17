#include "precompiled.h"
#pragma hdrstop


BufferToFile::~BufferToFile( void ) {

}

unsigned int BufferToFile::Timestamp( void ) {
	return 0;
}

int BufferToFile::Tell( void ) {
	return 0;
}

int BufferToFile::Seek( long offset, fsOrigin_t origin ) {
	return 0;
}

int BufferToFile::Printf( const char *fmt, ... ) {
	return 0;
}

int BufferToFile::VPrintf( const char *fmt, va_list arg ) {
	return 0;
}

int BufferToFile::WriteFloatString( const char *fmt, ... ) {
	return 0;
}
