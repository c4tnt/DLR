// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __SCRIPT_INTERPRETER_H__
#define __SCRIPT_INTERPRETER_H__

#define MAX_STACK_DEPTH 	64
//#define LOCALSTACK_SIZE 	6144
#define LOCALSTACK_SIZE 	0x2000

#define CHECK_EVENT_RESPOND		0
#define CHECK_EVENT_NO_ENT		1
#define CHECK_EVENT_NOT_RESPOND	2

typedef struct prstack_s {
	int 				s;
	const function_t	*f;
	int 				stackbase;
} prstack_t;

typedef struct spatch_s {
	int 				stackbase;
	void*				src;
	int					size;
} spatch_t;

class idInterpreter;

class idScriptContext {

private:
	idList<prstack_t>	callStack;
	idList<byte>		localstack;
	idList<spatch_t>	localstackPatches;

	int 				localstackUsed;
	int 				localstackBase;
	const function_t	*currentFunction;

	void				NextInstruction( int position );
	int 				instructionPointer;

	idEntity			*Self;
	idEntity			*Activator;
	idEntity			*Target;

public:
	void				Reset( void );
	void				PopParms( int numParms );
	void				PushString( const char *string );
	template <class _T>	void Push( const _T &value );
	void				Push( void* mem, int size );

	void				SetEnterFunction( const function_t *func );
	void				SetEnterObjectFunction( idEntity *self, const function_t *func );

	void				Save( idSave_I *savefile ) const;				// archives object for save game file
	void				Restore( idRestore_I *savefile );				// unarchives object from save game file
						idScriptContext();

	//Compilers
	bool				CompileParamS( const char* str );
	bool				CompileParamL( idLexer& lex );
	void				Apply( idInterpreter *to ) const;
	bool				Callable() const;
};

/*
====================
idScriptContext::Callable
====================
*/
ID_INLINE bool idScriptContext::Callable( ) const {
	return ( currentFunction != NULL );
}

/*
====================
idScriptContext::PopParms
====================
*/
ID_INLINE void idScriptContext::PopParms( int numParms ) {
	// pop our parms off the stack
	if ( localstackUsed < numParms ) {
		gameLocal.Error( "locals stack underflow\n" );
	}

	localstackUsed -= numParms;
}

/*
====================
idScriptContext::Push _T
====================
*/
template <class _T>
ID_INLINE void idScriptContext::Push( const _T &value ) {
	if ( localstackUsed + sizeof( _T ) > LOCALSTACK_SIZE ) {
		gameLocal.Error( "Push: locals stack overflow\n" );
	}
	
	localstack.AssureSize(localstackUsed + sizeof( _T ));

	*( _T * )&localstack[ localstackUsed ]	= value;
	localstackUsed += sizeof( _T );
}

/*
====================
idScriptContext::Push (memory)
====================
*/
ID_INLINE void idScriptContext::Push( void* mem, int size ) {
	if ( localstackUsed + size > LOCALSTACK_SIZE ) {
		gameLocal.Error( "Push: locals stack overflow\n" );
	}
	localstack.AssureSize(localstackUsed + size);
	memcpy(&localstack[ localstackUsed ], mem, size);
	localstackUsed += size;
}
/*
====================
idScriptContext::PushString
====================
*/
ID_INLINE void idScriptContext::PushString( const char *string ) {
	if ( localstackUsed + MAX_STRING_LEN > LOCALSTACK_SIZE ) {
		gameLocal.Error( "PushString: locals stack overflow\n" );
	}
	localstack.AssureSize( localstackUsed + MAX_STRING_LEN );
	idStr::Copynz( ( char * )&localstack[ localstackUsed ], string, MAX_STRING_LEN );
	localstackUsed += MAX_STRING_LEN;
}

/*
====================
idScriptContext::NextInstruction
====================
*/
ID_INLINE void idScriptContext::NextInstruction( int position ) {
	// Before we execute an instruction, we increment instructionPointer,
	// therefore we need to compensate for that here.
	instructionPointer = position - 1;
}


class idInterpreter {
	friend class idScriptContext;
private:
	prstack_t			callStack[ MAX_STACK_DEPTH ];
	int 				callStackDepth;
	int 				maxStackDepth;

	byte				localstack[ LOCALSTACK_SIZE ];
	int 				localstackUsed;
	int 				localstackBase;
	int 				maxLocalstackUsed;

	const function_t	*currentFunction;
	int 				instructionPointer;

	int					popParms;
	const idEventDef	*multiFrameEvent;
	idEntity			*eventEntity;

	idThread			*thread;
	idProgram			*module;

	const char			*FloatToString( float value );
	void				AppendString( idVarDef *def, const char *from );
	void				SetString( idVarDef *def, const char *from );
	const char			*GetString( idVarDef *def );
	char				*GetStringWrite( idVarDef *def );
	varEval_t			GetVariable( idVarDef *def );
	idEntity			*GetEntity( int entnum ) const;
	idScriptObject		*GetScriptObject( int entnum ) const;
	void				NextInstruction( int position );

	void				LeaveFunction( idVarDef *returnDef );
	void				LeaveFunctionSafe( etype_t rtype );
	int					CheckEvent( const function_t *func, int argsize );
	void				CallEvent( const function_t *func, int argsize, idEventReturn *RC = NULL );
	void				CallSysEvent( const function_t *func, int argsize, idEventReturn *RC = NULL );
	char				GetEventReturnType( const function_t *func );

	bool				GetRegisterVar( idVarDef* d, idStr &out );

public:
	bool				doneProcessing;
	bool				threadDying;
	bool				terminateOnExit;
	bool				debug;

						idInterpreter();

	// save games
	void				Save( idSave_I *savefile ) const;				// archives object for save game file
	void				Restore( idRestore_I *savefile );				// unarchives object from save game file

	void				SetThread( idThread *pThread );
	void				SetModule( idProgram *pModule );

	void				StackTrace( void ) const;

	int					CurrentLine( void ) const;
	const char			*CurrentFile( void ) const;

	void				Error( char *fmt, ... ) const id_attribute((format(printf,2,3)));
	void				Warning( char *fmt, ... ) const id_attribute((format(printf,2,3)));
	void				DisplayInfo( void ) const;

	bool				BeginMultiFrameEvent( idEntity *ent, const idEventDef *event );
	void				EndMultiFrameEvent( idEntity *ent, const idEventDef *event );
	bool				MultiFrameEventInProgress( void ) const;

	void				ThreadCall( idInterpreter *source, const function_t *func, int args );
	void				EnterFunction( const function_t *func, bool clearStack );
	void				EnterObjectFunction( idEntity *self, const function_t *func, bool clearStack );

	bool				Execute( void );
	void				Reset( void );

	//c4tnt:			now these operations are public
	void				PopParms( int numParms );
	void				PushString( const char *string );
	template <class _T>	void Push( const _T &value );
	void				Push( void* mem, int size );
	void				Pushz( char _v, int size );
	template <class _T>	void Pop( _T &value );
	void				PopString( char *string );
	
	bool				GetRegisterValue( const char *name, idStr &out, int scopeDepth );
	int					GetCallstackDepth( void ) const;

	const prstack_t		*GetCallstack( void ) const;
	const function_t	*GetCurrentFunction( void ) const;
	idThread			*GetThread( void ) const;
	idProgram			*GetModule( void ) const;

};

/*
====================
idInterpreter::PopParms
====================
*/
ID_INLINE void idInterpreter::PopParms( int numParms ) {
	// pop our parms off the stack
	if ( localstackUsed < numParms ) {
		Error( "locals stack underflow\n" );
	}

	localstackUsed -= numParms;
}

/*
====================
idInterpreter::Push _T
====================
*/
template <class _T>
ID_INLINE void idInterpreter::Push( const _T &value ) {
	if ( localstackUsed + sizeof( _T ) > LOCALSTACK_SIZE ) {
		Error( "Push: locals stack overflow\n" );
	}
	*( _T * )&localstack[ localstackUsed ]	= value;
	localstackUsed += sizeof( _T );
}

/*
====================
idInterpreter::Pop _T
====================
*/
template <class _T>
ID_INLINE void idInterpreter::Pop( _T &value ) {
	if ( localstackUsed - sizeof( _T ) < 0 ) {
		Error( "Pop: locals stack underflow\n" );
	}
	localstackUsed -= sizeof( _T );
	value = *( _T * )&localstack[ localstackUsed ];
}

/*
====================
idInterpreter::PopString
====================
*/
ID_INLINE void idInterpreter::PopString( char *string ) {
	if ( localstackUsed - MAX_STRING_LEN < 0 ) {
		Error( "PopString: locals stack underflow\n" );
	}
	localstackUsed -= MAX_STRING_LEN;
	idStr::Copynz( string, ( char * )&localstack[ localstackUsed ], MAX_STRING_LEN );
}

/*
====================
idInterpreter::Push (memory)
====================
*/
ID_INLINE void idInterpreter::Push( void* mem, int size ) {
	if ( localstackUsed + size > LOCALSTACK_SIZE ) {
		Error( "Push: locals stack overflow\n" );
	}
	memcpy(&localstack[ localstackUsed ], mem, size);
	localstackUsed += size;
}

/*
====================
idInterpreter::Pushz (memory)
====================
*/
ID_INLINE void idInterpreter::Pushz( char _v, int size ) {
	if ( localstackUsed + size > LOCALSTACK_SIZE ) {
		Error( "Push: locals stack overflow\n" );
	}
	memset(&localstack[ localstackUsed ], _v, size);
	localstackUsed += size;
}

/*
====================
idInterpreter::PushString
====================
*/
ID_INLINE void idInterpreter::PushString( const char *string ) {
	if ( localstackUsed + MAX_STRING_LEN > LOCALSTACK_SIZE ) {
		Error( "PushString: locals stack overflow\n" );
	}
	idStr::Copynz( ( char * )&localstack[ localstackUsed ], string, MAX_STRING_LEN );
	localstackUsed += MAX_STRING_LEN;
}

/*
====================
idInterpreter::FloatToString
====================
*/
ID_INLINE const char *idInterpreter::FloatToString( float value ) {
	static char	text[ 32 ];

	if ( value == ( float )( int )value ) {
		sprintf( text, "%d", ( int )value );
	} else {
		sprintf( text, "%f", value );
	}
	return text;
}

/*
====================
idInterpreter::AppendString
====================
*/
ID_INLINE void idInterpreter::AppendString( idVarDef *def, const char *from ) {
	if ( def->initialized == idVarDef::stackVariable ) {
		idStr::Append( ( char * )&localstack[ localstackBase + def->value.stackOffset ], MAX_STRING_LEN, from );
	} else {
		idStr::Append( def->value.stringPtr, MAX_STRING_LEN, from );
	}
}

/*
====================
idInterpreter::SetString
====================
*/
ID_INLINE void idInterpreter::SetString( idVarDef *def, const char *from ) {
	if ( def->initialized == idVarDef::stackVariable ) {
		idStr::Copynz( ( char * )&localstack[ localstackBase + def->value.stackOffset ], from, MAX_STRING_LEN );
	} else {
		idStr::Copynz( def->value.stringPtr, from, MAX_STRING_LEN );
	}
}

/*
====================
idInterpreter::GetString
====================
*/
ID_INLINE const char *idInterpreter::GetString( idVarDef *def ) {
	if ( def->initialized == idVarDef::stackVariable ) {
		return ( char * )&localstack[ localstackBase + def->value.stackOffset ];
	} else {
		return def->value.stringPtr;
	}
}

/*
====================
idInterpreter::GetStringWrite
====================
*/
ID_INLINE char *idInterpreter::GetStringWrite( idVarDef *def ) {
	if ( def->initialized == idVarDef::stackVariable ) {
		return ( char * )&localstack[ localstackBase + def->value.stackOffset ];
	} else {
		return NULL;
	}
}

/*
====================
idInterpreter::GetVariable
====================
*/
ID_INLINE varEval_t idInterpreter::GetVariable( idVarDef *def ) {
	if ( def->initialized == idVarDef::stackVariable ) {
		varEval_t val;
		val.intPtr = ( int * )&localstack[ localstackBase + def->value.stackOffset ];
		return val;
	} else {
		return def->value;
	}
}

/*
================
idInterpreter::GetEntity
================
*/
ID_INLINE idEntity *idInterpreter::GetEntity( int entnum ) const{
	assert( entnum <= MAX_GENTITIES );
	if ( ( entnum > 0 ) && ( entnum <= MAX_GENTITIES ) ) {
		return gameLocal.entities[ entnum - 1 ];
	}
	return NULL;
}

/*
================
idInterpreter::GetScriptObject
================
*/
ID_INLINE idScriptObject *idInterpreter::GetScriptObject( int entnum ) const {
	idEntity *ent;

	assert( entnum <= MAX_GENTITIES );
	if ( ( entnum > 0 ) && ( entnum <= MAX_GENTITIES ) ) {
		ent = gameLocal.entities[ entnum - 1 ];
		if ( ent && ent->scriptObject.data ) {
			return &ent->scriptObject;
		}
	}
	return NULL;
}

/*
====================
idInterpreter::NextInstruction
====================
*/
ID_INLINE void idInterpreter::NextInstruction( int position ) {
	// Before we execute an instruction, we increment instructionPointer,
	// therefore we need to compensate for that here.
	instructionPointer = position - 1;
}

#endif /* !__SCRIPT_INTERPRETER_H__ */
