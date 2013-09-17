// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"


/*
================
idScriptContext::idScriptContext()
================
*/
idScriptContext::idScriptContext() {
	localstackUsed = 0;
	Reset();
}

/*
====================
idScriptContext::Apply

Set the interpreter stack
====================
*/
void idScriptContext::Apply( idInterpreter *to ) const {
int i;
	if (!to)
		return;

	to->Reset();

	memcpy( to->localstack, localstack.Ptr(), localstackUsed );
	
	// Apply the localstack patches here
	for (i = 0; i < localstackPatches.Num(); i++) {
		const spatch_t &vpath = localstackPatches[i];
		memcpy(&to->localstack[vpath.stackbase], vpath.src, vpath.size);
	}

	to->localstackBase = localstackBase;
	to->localstackUsed = localstackUsed;
	to->maxLocalstackUsed = localstackUsed;
	to->instructionPointer = instructionPointer;
	to->currentFunction = currentFunction;

	memcpy( to->callStack, callStack.Ptr(), callStack.MemoryUsed() );

	to->callStackDepth = callStack.Num();
	to->maxStackDepth = to->callStackDepth;
}

/*
================
idScriptContext::EnterObjectFunction

Calls a function on a script object.

NOTE: If this is called from within a event called by this interpreter, the function arguments will be invalid after calling this function.
================
*/
void idScriptContext::SetEnterObjectFunction( idEntity *self, const function_t *func ) {
	Push( self->entityNumber + 1 );
	SetEnterFunction( func );
}

/*
====================
idScriptContext::EnterFunction

Returns the new program statement counter

NOTE: If this is called from within a event called by this interpreter, the function arguments will be invalid after calling this function.
====================
*/
void idScriptContext::SetEnterFunction( const function_t *func ) {
	int 		c;
	prstack_t	*stack;

	stack = &callStack.Alloc();

	stack->s			= instructionPointer + 1;	// point to the next instruction to execute
	stack->f			= currentFunction;
	stack->stackbase	= localstackBase;

	if ( !func ) {
		gameLocal.Error( "NULL function" );
	}

	currentFunction = func;
	assert( !func->eventdef );
	NextInstruction( func->firstStatement );

	// allocate space on the stack for locals
	// parms are already on stack
	c = func->locals - func->parmTotal;
	assert( c >= 0 );

	if ( localstackUsed + c > LOCALSTACK_SIZE ) {
		gameLocal.Error( "EnterFuncton: locals stack overflow\n" );
	}
	
	localstack.AssureSize( localstackUsed + c );

	// initialize local stack variables to zero
	memset( &localstack[ localstackUsed ], 0, c );

	localstackUsed += c;
	localstackBase = localstackUsed - func->locals;
}

/*
================
idScriptContext::Reset
================
*/
void idScriptContext::Reset( void ) {
	localstackUsed = 0;
	localstackBase = 0;
	currentFunction = 0;

	callStack.Clear();
	localstack.Clear();
	localstackPatches.Clear();

	NextInstruction( 0 );
}

bool idScriptContext::CompileParamL( idLexer& lexer ) {

	idToken token, token2, token3, token4;
	const idVarDef *funcdef;
	const idTypeDef* func_type;
	const idTypeDef* argtype;
	idEntity*		ent;
	int argc;
	int argn;
	bool semicolon;

	semicolon = false;
	
	Reset();

	while( 1 ) {

		if (!lexer.ReadToken( &token )) {
			return Callable();
		}
		if ( token == ";" ) {
			semicolon = false;
			continue;
		}

		if (token.type != TT_NAME) {
			gameLocal.Warning( "JIT: Unexpected token %s", token.c_str());
			Reset();
			return false;
		}

		if (semicolon) {
			gameLocal.Warning( "JIT: Unexpected token %s", token.c_str());
			Reset();
			return false;
		}

		while( lexer.CheckTokenString( "::" ) ) {
			if ( !lexer.ReadToken( &token2 ) ) {
				gameLocal.Warning( "JIT: Expecting function name following '::' in CompileParamL" );
				Reset();
				return false;
			}
			token += "::" + token2;
		}

		funcdef = gameLocal.program.FindFunctionDef( token );
		if ( !funcdef || !funcdef->TypeDef()) {
			gameLocal.Warning( "JIT: Function '%s' not found", token.c_str() );
			Reset();
			return false;
		}
		
		func_type = funcdef->TypeDef();
		argc = func_type->NumParameters();
		
		if (lexer.CheckTokenString( "(" )) {
			// Get Args	
			for (argn = 0; argn < argc; argn++) {
				argtype = func_type->GetParmType(argn);
				
				if (argn) {
					if (!lexer.ExpectTokenString( "," )) {
						Reset();
						return false;
					}
				}

				switch ( argtype->Type() )
				{
				case ev_boolean:
					Push((int)lexer.ParseBool());
					break;
				case ev_vector:
					{
						lexer.ReadToken(&token2);
						if (token2.type != TT_LITERAL) {
							gameLocal.Warning( "JIT: Couldn't read vector. '%s' is not in the form of 'x y z'", token2.c_str());
							Reset();
							return false;
						}
						idLexer lex( token2, token.Length(), "<JIT>", LEXFL_NOERRORS );
						//x y z form
						Push(lex.ParseFloat());
						Push(lex.ParseFloat());
						Push(lex.ParseFloat());
					}
					break;
				case ev_float:
					Push(lexer.ParseFloat());
					break;
				case ev_string:
					lexer.ReadToken(&token2);
					if (token2.type == TT_STRING || token2.type == TT_LITERAL) {
						PushString(token2.c_str());
					}else{
						gameLocal.Warning( "JIT: %s is not a string", token2.c_str());
						Reset();
						return false;
					}
					break;
				case ev_entity:
					if (lexer.CheckTokenString( "$" )) {
						//Map entity reference
						lexer.ReadToken(&token2);
						if (token2.type != TT_NAME) {
							gameLocal.Warning( "JIT: %s is not a entity name", token2.c_str());
							Reset();
							return false;
						}
						ent = gameLocal.FindEntity( token2.c_str() );
						if (!ent) {
							gameLocal.Warning( "JIT: '%s' entity not found", token2.c_str());
							Push((int)0);
						} else {
							Push((int)(ent->entityNumber + 1));
						}
					} else {
						lexer.ReadToken(&token2);
						gameLocal.Warning( "JIT: %s is not a entity name", token2.c_str());
						Reset();
						return false;
					}
					break;
				default:
					gameLocal.Warning( "JIT: CompileParamL unsupported function parameter type (%s) on arg (%s)", func_type->GetParmType(argn)->Name(), func_type->GetParmName(argn) );
					Reset();
					return false;
				}
			}
			if (!lexer.ExpectTokenString( ")" )) {
				Reset();
				return false;
			}
		} else {
			if (argc > 0) { 
				gameLocal.Warning( "JIT: Expecting '('" );
				Reset();
				return false;
			}
		}
		// Call function here
		SetEnterFunction( funcdef->value.functionPtr );
		semicolon = true;
	}

	return Callable();
}

bool idScriptContext::CompileParamS( const char* str ) {
	
	idLexer lexer( LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOFATALERRORS ); 
	
	idToken token, token2, token3, token4;

	lexer.LoadMemory( str, idStr::Length(str), "<JIT>" );
	return CompileParamL(lexer);
}

/*
================
idScriptContext::Save
================
*/
void idScriptContext::Save( idSave_I *savefile ) const {
	int i;

	savefile->WriteInt( callStack.Num() );
	for( i = 0; i < callStack.Num(); i++ ) {
		savefile->WriteInt( callStack[i].s );
		if ( callStack[i].f ) {
			savefile->WriteInt( gameLocal.program.GetFunctionIndex( callStack[i].f ) );
		} else {
			savefile->WriteInt( -1 );
		}
		savefile->WriteInt( callStack[i].stackbase );
	}

	savefile->WriteInt( localstackUsed );
	savefile->Write( localstack.Ptr(), localstackUsed );
	savefile->WriteInt( localstackBase );

	if ( currentFunction ) {
		savefile->WriteInt( gameLocal.program.GetFunctionIndex( currentFunction ) );
	} else {
		savefile->WriteInt( -1 );
	}
	savefile->WriteInt( instructionPointer );
}

/*
================
idScriptContext::Restore
================
*/
void idScriptContext::Restore( idRestore_I *savefile ) {
	int i;
	idStr funcname;
	int func_index;
	prstack_t* st;
	int callsz;

	savefile->ReadInt( callsz );
	for( i = 0; i < callsz; i++ ) {
		st = &callStack.Alloc();
		savefile->ReadInt( st->s );

		savefile->ReadInt( func_index );
		if ( func_index >= 0 ) {
			st->f = gameLocal.program.GetFunction( func_index );
		} else {
			st->f = NULL;
		}

		savefile->ReadInt( st->stackbase );
	}

	savefile->ReadInt( localstackUsed );
	localstack.AssureSize( localstackUsed );
	savefile->Read( localstack.Ptr(), localstackUsed );

	savefile->ReadInt( localstackBase );
	savefile->ReadInt( func_index );

	if ( func_index >= 0 ) {
		currentFunction = gameLocal.program.GetFunction( func_index );
	} else {
		currentFunction = NULL;
	}
	savefile->ReadInt( instructionPointer );
}

/*
================
idInterpreter::idInterpreter()
================
*/
idInterpreter::idInterpreter() {
	localstackUsed = 0;
	terminateOnExit = true;
	debug = 0;
	memset( localstack, 0, sizeof( localstack ) );
	memset( callStack, 0, sizeof( callStack ) );
	Reset();
}

/*
================
idInterpreter::Save
================
*/
void idInterpreter::Save( idSave_I *savefile ) const {
	int i;

	savefile->WriteInt( callStackDepth );
	for( i = 0; i < callStackDepth; i++ ) {
		savefile->WriteInt( callStack[i].s );
		if ( callStack[i].f ) {
			savefile->WriteInt( gameLocal.program.GetFunctionIndex( callStack[i].f ) );
		} else {
			savefile->WriteInt( -1 );
		}
		savefile->WriteInt( callStack[i].stackbase );
	}
	savefile->WriteInt( maxStackDepth );

	savefile->WriteInt( localstackUsed );
	savefile->WriteRLE( &localstack, localstackUsed );

	savefile->WriteInt( localstackBase );
	savefile->WriteInt( maxLocalstackUsed );

	if ( currentFunction ) {
		savefile->WriteInt( gameLocal.program.GetFunctionIndex( currentFunction ) );
	} else {
		savefile->WriteInt( -1 );
	}
	savefile->WriteInt( instructionPointer );

	savefile->WriteInt( popParms );

	if ( multiFrameEvent ) {
		savefile->WriteString( multiFrameEvent->GetName() );
	} else {
		savefile->WriteString( "" );
	}
	savefile->WriteObject( eventEntity );

	savefile->WriteObject( thread );

	savefile->WriteBool( doneProcessing );
	savefile->WriteBool( threadDying );
	savefile->WriteBool( terminateOnExit );
	savefile->WriteBool( debug );
}

/*
================
idInterpreter::Restore
================
*/
void idInterpreter::Restore( idRestore_I *savefile ) {
	int i;
	idStr funcname;
	int func_index;

	savefile->ReadInt( callStackDepth );
	for( i = 0; i < callStackDepth; i++ ) {
		savefile->ReadInt( callStack[i].s );

		savefile->ReadInt( func_index );
		if ( func_index >= 0 ) {
			callStack[i].f = gameLocal.program.GetFunction( func_index );
		} else {
			callStack[i].f = NULL;
		}

		savefile->ReadInt( callStack[i].stackbase );
	}
	savefile->ReadInt( maxStackDepth );

	savefile->ReadInt( localstackUsed );
	if (!savefile->ReadRLE( &localstack, localstackUsed )) {
		gameLocal.Error( "idInterpreter::Restore: decompression failed." );
	}

	savefile->ReadInt( localstackBase );
	savefile->ReadInt( maxLocalstackUsed );

	savefile->ReadInt( func_index );
	if ( func_index >= 0 ) {
		currentFunction = gameLocal.program.GetFunction( func_index );
	} else {
		currentFunction = NULL;
	}
	savefile->ReadInt( instructionPointer );

	savefile->ReadInt( popParms );

	savefile->ReadString( funcname );
	if ( funcname.Length() ) {
		multiFrameEvent = idEventDef::FindEvent( funcname );
	}

	savefile->ReadObject( reinterpret_cast<idClass *&>( eventEntity ) );
	savefile->ReadObject( reinterpret_cast<idClass *&>( thread ) );

	savefile->ReadBool( doneProcessing );
	savefile->ReadBool( threadDying );
	savefile->ReadBool( terminateOnExit );
	savefile->ReadBool( debug );
}

/*
================
idInterpreter::Reset
================
*/
void idInterpreter::Reset( void ) {
	callStackDepth = 0;
	localstackUsed = 0;
	localstackBase = 0;

	maxLocalstackUsed = 0;
	maxStackDepth = 0;

	popParms = 0;
	multiFrameEvent = NULL;
	eventEntity = NULL;

	currentFunction = 0;
	NextInstruction( 0 );

	threadDying 	= false;
	doneProcessing	= true;
}

/*
================
idInterpreter::GetRegisterValue

Returns a string representation of the value of the register.  This is 
used primarily for the debugger and debugging

//FIXME:  This is pretty much wrong.  won't access data in most situations.
================
*/
bool idInterpreter::GetRegisterValue( const char *name, idStr &out, int scopeDepth ) {
	varEval_t		reg;
	idVarDef		*d;
	char			funcObject[ 1024 ];
	char			*funcName;
	const idVarDef	*scope;
	const idTypeDef	*field;
	const idScriptObject *obj;
	const function_t *func;

	out.Empty();
	
	if ( scopeDepth == -1 ) {
		scopeDepth = callStackDepth;
	}	
	
	if ( scopeDepth == callStackDepth ) {
		func = currentFunction;
	} else {
		func = callStack[ scopeDepth ].f;
	}
	if ( !func ) {
		return false;
	}

	idStr::Copynz( funcObject, func->Name(), sizeof( funcObject ) );
	funcName = strstr( funcObject, "::" );
	if ( funcName ) {
		*funcName = '\0';
		scope = gameLocal.program.GetDef( NULL, funcObject, &def_namespace );
		funcName += 2;
	} else {
		funcName = funcObject;
		scope = &def_namespace;
	}

	// Get the function from the object
	d = gameLocal.program.GetDef( NULL, funcName, scope );
	if ( !d ) {
		return false;
	}
	
	// Get the variable itself and check various namespaces
	d = gameLocal.program.GetDef( NULL, name, d );
	if ( !d ) {
		if ( scope == &def_namespace ) {
			return false;
		}
		
		d = gameLocal.program.GetDef( NULL, name, scope );
		if ( !d ) {
			d = gameLocal.program.GetDef( NULL, name, &def_namespace );
			if ( !d ) {
				return false;
			}
		}
	}
		
	reg = GetVariable( d );
	switch( d->Type() ) {
	case ev_float:
		if ( reg.floatPtr ) {
			out = va("%g", *reg.floatPtr );
		} else {
			out = "0";
		}
		return true;
		break;

	case ev_vector:
		if ( reg.vectorPtr ) {				
			out = va( "%g,%g,%g", reg.vectorPtr->x, reg.vectorPtr->y, reg.vectorPtr->z );
		} else {
			out = "0,0,0";
		}
		return true;
		break;

	case ev_boolean:
		if ( reg.intPtr ) {
			out = va( "%d", *reg.intPtr );
		} else {
			out = "0";
		}
		return true;
		break;

	case ev_field:
		if ( scope == &def_namespace ) {
			// should never happen, but handle it safely anyway
			return false;
		}

		field = scope->TypeDef()->GetParmType( reg.ptrOffset )->FieldType();
		obj   = *reinterpret_cast<const idScriptObject **>( &localstack[ callStack[ callStackDepth ].stackbase ] );
		if ( !field || !obj ) {
			return false;
		}
								
		switch ( field->Type() ) {
		case ev_boolean:
			out = va( "%d", *( reinterpret_cast<int *>( &obj->data[ reg.ptrOffset ] ) ) );
			return true;

		case ev_float:
			out = va( "%g", *( reinterpret_cast<float *>( &obj->data[ reg.ptrOffset ] ) ) );
			return true;

		default:
			return false;
		}
		break;

	case ev_string:
		if ( reg.stringPtr ) {
			out = "\"";
			out += reg.stringPtr;
			out += "\"";
		} else {
			out = "\"\"";
		}
		return true;

	default:
		return false;
	}
}

/*
================
idInterpreter::GetRegisterVar

Returns a string representation of the value of the register.  This is 
used primarily for the debugger and debugging

================
*/
bool idInterpreter::GetRegisterVar( idVarDef* d, idStr &out ) {
	varEval_t		reg;
	idEntity*		ent;
	idScriptObject*	obj;

	out.Empty();
	
	reg = GetVariable( d );
	switch( d->Type() ) {
	case ev_float:
		if ( reg.floatPtr ) {
			out = va("%g", *reg.floatPtr );
		} else {
			out = "0";
		}
		return true;
		break;

	case ev_vector:
		if ( reg.vectorPtr ) {				
			out = va( "%g,%g,%g", reg.vectorPtr->x, reg.vectorPtr->y, reg.vectorPtr->z );
		} else {
			out = "0,0,0";
		}
		return true;
		break;

	case ev_boolean:
		if ( reg.intPtr ) {
			out = va( "%d", *reg.intPtr );
		} else {
			out = "0";
		}
		return true;
		break;

	case ev_string:
		if ( reg.stringPtr ) {
			out = "\"";
			out += reg.stringPtr;
			out += "\"";
		} else {
			out = "\"\"";
		}
		return true;

	case ev_entity:
		ent = GetEntity( *reg.entityNumberPtr );
		if ( !ent ) {
			out = va( "NULL(%d)", *reg.entityNumberPtr );
		} else {
			out = va( "%s(%d)", ent->GetName(), *reg.entityNumberPtr );
		}
		return true;

	case ev_object:
		obj = GetScriptObject( *reg.entityNumberPtr );
		if ( !obj ) {
			out = va( "NULL(%d)", *reg.entityNumberPtr );
		} else {
			out = va( "%s(%d)", obj->GetTypeName(), *reg.entityNumberPtr );
		}
		return true;

	default:
		return false;
	}
}

/*
================
idInterpreter::GetCallstackDepth
================
*/
int idInterpreter::GetCallstackDepth( void ) const {
	return callStackDepth;
}

/*
================
idInterpreter::GetCallstack
================
*/
const prstack_t *idInterpreter::GetCallstack( void ) const {
	return &callStack[ 0 ];
}

/*
================
idInterpreter::GetCurrentFunction
================
*/
const function_t *idInterpreter::GetCurrentFunction( void ) const {
	return currentFunction;
}

/*
================
idInterpreter::GetThread
================
*/
idThread *idInterpreter::GetThread( void ) const {
	return thread;
}

/*
================
idInterpreter::GetThread
================
*/
idProgram *idInterpreter::GetModule( void ) const {
	return module;
}

/*
================
idInterpreter::SetThread
================
*/
void idInterpreter::SetThread( idThread *pThread ) {
	thread = pThread;
}

/*
================
idInterpreter::SetThread
================
*/
void idInterpreter::SetModule( idProgram *pModule ) {
	module = pModule;
}

/*
================
idInterpreter::CurrentLine
================
*/
int idInterpreter::CurrentLine( void ) const {
	if ( instructionPointer < 0 ) {
		return 0;
	}
	return gameLocal.program.GetLineNumberForStatement( instructionPointer );
}

/*
================
idInterpreter::CurrentFile
================
*/
const char *idInterpreter::CurrentFile( void ) const {
	if ( instructionPointer < 0 ) {
		return "";
	}
	return gameLocal.program.GetFilenameForStatement( instructionPointer );
}

/*
============
idInterpreter::StackTrace
============
*/
void idInterpreter::StackTrace( void ) const {
	const function_t	*f;
	int 				i;
	int					top;

	if ( callStackDepth == 0 ) {
		gameLocal.Printf( "<NO STACK>\n" );
		return;
	}

	top = callStackDepth;
	if ( top >= MAX_STACK_DEPTH ) {
		top = MAX_STACK_DEPTH - 1;
	}
	
	if ( !currentFunction ) {
		gameLocal.Printf( "<NO FUNCTION>\n" );
	} else {
		gameLocal.Printf( "%12s : %s\n", gameLocal.program.GetFilename( currentFunction->filenum ), currentFunction->Name() );
	}

	for( i = top; i >= 0; i-- ) {
		f = callStack[ i ].f;
		if ( !f ) {
			gameLocal.Printf( "<NO FUNCTION>\n" );
		} else {
			gameLocal.Printf( "%12s : %s\n", gameLocal.program.GetFilename( f->filenum ), f->Name() );
		}
	}
}

/*
============
idInterpreter::Error

Aborts the currently executing function
============
*/
void idInterpreter::Error( char *fmt, ... ) const {
	va_list argptr;
	char	text[ 1024 ];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	StackTrace();

	if ( ( instructionPointer >= 0 ) && ( instructionPointer < gameLocal.program.NumStatements() ) ) {
		statement_t &line = gameLocal.program.GetStatement( instructionPointer );
		common->Error( "%s(%d): Thread '%s': %s\n", gameLocal.program.GetFilename( line.file ), line.linenumber, thread->GetThreadName(), text );
	} else {
		common->Error( "Thread '%s': %s\n", thread->GetThreadName(), text );
	}
}

/*
============
idInterpreter::Warning

Prints file and line number information with warning.
============
*/
void idInterpreter::Warning( char *fmt, ... ) const {
	va_list argptr;
	char	text[ 1024 ];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	if ( ( instructionPointer >= 0 ) && ( instructionPointer < gameLocal.program.NumStatements() ) ) {
		statement_t &line = gameLocal.program.GetStatement( instructionPointer );
		common->Warning( "%s(%d): Thread '%s': %s", gameLocal.program.GetFilename( line.file ), line.linenumber, thread->GetThreadName(), text );
	} else {
		common->Warning( "Thread '%s' : %s", thread->GetThreadName(), text );
	}
}

/*
================
idInterpreter::DisplayInfo
================
*/
void idInterpreter::DisplayInfo( void ) const {
	const function_t *f;
	int i;

	gameLocal.Printf( " Stack depth: %d bytes, %d max\n", localstackUsed, maxLocalstackUsed );
	gameLocal.Printf( "  Call depth: %d, %d max\n", callStackDepth, maxStackDepth );
	gameLocal.Printf( "  Call Stack: " );

	if ( callStackDepth == 0 ) {
		gameLocal.Printf( "<NO STACK>\n" );
	} else {
		if ( !currentFunction ) {
			gameLocal.Printf( "<NO FUNCTION>\n" );
		} else {
			gameLocal.Printf( "%12s : %s\n", gameLocal.program.GetFilename( currentFunction->filenum ), currentFunction->Name() );
		}

		for( i = callStackDepth; i > 0; i-- ) {
			gameLocal.Printf( "              " );
			f = callStack[ i ].f;
			if ( !f ) {
				gameLocal.Printf( "<NO FUNCTION>\n" );
			} else {
				gameLocal.Printf( "%12s : %s\n", gameLocal.program.GetFilename( f->filenum ), f->Name() );
			}
		}
	}
}

/*
====================
idInterpreter::ThreadCall

Copys the args from the calling thread's stack
====================
*/
void idInterpreter::ThreadCall( idInterpreter *source, const function_t *func, int args ) {
	Reset();

	memcpy( localstack, &source->localstack[ source->localstackUsed - args ], args );

	localstackUsed = args;
	localstackBase = 0;

	maxLocalstackUsed = localstackUsed;
	EnterFunction( func, false );

	thread->SetThreadName( currentFunction->Name() );
}

/*
================
idInterpreter::EnterObjectFunction

Calls a function on a script object.

NOTE: If this is called from within a event called by this interpreter, the function arguments will be invalid after calling this function.
================
*/
void idInterpreter::EnterObjectFunction( idEntity *self, const function_t *func, bool clearStack ) {
	if ( clearStack ) {
		Reset();
	}
	if ( popParms ) {
		PopParms( popParms );
		popParms = 0;
	}
	Push( self->entityNumber + 1 );
	EnterFunction( func, false );
}

/*
====================
idInterpreter::EnterFunction

Returns the new program statement counter

NOTE: If this is called from within a event called by this interpreter, the function arguments will be invalid after calling this function.
====================
*/
void idInterpreter::EnterFunction( const function_t *func, bool clearStack ) {
	int 		c;
	prstack_t	*stack;

	if ( clearStack ) {
		Reset();
	}
	if ( popParms ) {
		PopParms( popParms );
		popParms = 0;
	}

	if ( callStackDepth >= MAX_STACK_DEPTH ) {
		Error( "call stack overflow" );
	}

	stack = &callStack[ callStackDepth ];

	stack->s			= instructionPointer + 1;	// point to the next instruction to execute
	stack->f			= currentFunction;
	stack->stackbase	= localstackBase;

	callStackDepth++;
	if ( callStackDepth > maxStackDepth ) {
		maxStackDepth = callStackDepth;
	}

	if ( !func ) {
		Error( "NULL function" );
	}

	if ( debug ) {
		if ( currentFunction ) {
			gameLocal.Printf( "%d: call '%s' from '%s'(line %d)%s\n", gameLocal.time, func->Name(), currentFunction->Name(), 
				gameLocal.program.GetStatement( instructionPointer ).linenumber, clearStack ? " clear stack" : "" );
		} else {
            gameLocal.Printf( "%d: call '%s'%s\n", gameLocal.time, func->Name(), clearStack ? " clear stack" : "" );
		}
	}

	currentFunction = func;
	assert( !func->eventdef );
	NextInstruction( func->firstStatement );

	// allocate space on the stack for locals
	// parms are already on stack
	c = func->locals - func->parmTotal;
	assert( c >= 0 );

	if ( localstackUsed + c > LOCALSTACK_SIZE ) {
		Error( "EnterFuncton: locals stack overflow\n" );
	}

	// initialize local stack variables to zero
	memset( &localstack[ localstackUsed ], 0, c );

	localstackUsed += c;
	localstackBase = localstackUsed - func->locals;

	if ( localstackUsed > maxLocalstackUsed ) {
		maxLocalstackUsed = localstackUsed ;
	}
}

/*
====================
idInterpreter::LeaveFunctionSafe
====================
*/
void idInterpreter::LeaveFunctionSafe( etype_t rtype ) {
	prstack_t *stack;

	if ( callStackDepth <= 0 ) {
		Error( "prog stack underflow" );
	}

	// return value
	if ( rtype != ev_void ) {
		Warning( "It seems there is no situable return value in function %s. Defaulting...", currentFunction->Name() );
		switch( rtype ) {
		case ev_boolean :
		case ev_float :
			PopParms( currentFunction->locals );
			Push( (float)0 );
			break;
		case ev_string :
			PopParms( currentFunction->locals );
			PushString( "" );
			break;
		case ev_vector :
			PopParms( currentFunction->locals );
			Push( vec3_zero );
			break;
		case ev_entity :
		case ev_object :
			PopParms( currentFunction->locals );
			Push( (int)0 );
			break;
		default :
			Error("Can't recover after the bad return value. Look for any warnings before. r_type is %d", rtype );
		}
	} else {
		// remove locals from the stack
		PopParms( currentFunction->locals );
	}

	if ( debug ) {
		statement_t &line = gameLocal.program.GetStatement( instructionPointer );
		gameLocal.Printf( "%d: %s(%d): exit %s", gameLocal.time, gameLocal.program.GetFilename( line.file ), line.linenumber, currentFunction->Name() );
		if ( callStackDepth > 1 ) {
			gameLocal.Printf( " return to %s(line %d)\n", callStack[ callStackDepth - 1 ].f->Name(), gameLocal.program.GetStatement( callStack[ callStackDepth - 1 ].s ).linenumber );
		} else {
			gameLocal.Printf( " done\n" );
		}
	}

	// up stack
	callStackDepth--;
	stack = &callStack[ callStackDepth ]; 
	currentFunction = stack->f;
	localstackBase = stack->stackbase;
	NextInstruction( stack->s );

	if ( callStackDepth <= 0 ) {
		// all done
		doneProcessing = true;
		threadDying = true;
		currentFunction = 0;
	}
}

/*
====================
idInterpreter::LeaveFunction
====================
*/
void idInterpreter::LeaveFunction( idVarDef *returnDef ) {
	prstack_t *stack;
	varEval_t ret;
	const char* _sptr;

	if ( callStackDepth <= 0 ) {
		Error( "prog stack underflow" );
	}

	// return value
	if ( returnDef ) {
		switch( returnDef->Type() ) {
		case ev_string :
			_sptr = GetString( returnDef );
			PopParms( currentFunction->locals );
			PushString( _sptr );					//c4tnt: new calling conversation return
			break;

		case ev_vector :
			ret = GetVariable( returnDef );
			PopParms( currentFunction->locals );
			Push( *ret.vectorPtr );					//c4tnt: new calling conversation return
			break;

		default :
			ret = GetVariable( returnDef );
			PopParms( currentFunction->locals );
			Push( *ret.intPtr );					//c4tnt: new calling conversation return
		}
	} else {
		// remove locals from the stack
		PopParms( currentFunction->locals );
	}

	if ( debug ) {
		statement_t &line = gameLocal.program.GetStatement( instructionPointer );
		gameLocal.Printf( "%d: %s(%d): exit %s", gameLocal.time, gameLocal.program.GetFilename( line.file ), line.linenumber, currentFunction->Name() );
		if ( callStackDepth > 1 ) {
			gameLocal.Printf( " return to %s(line %d)\n", callStack[ callStackDepth - 1 ].f->Name(), gameLocal.program.GetStatement( callStack[ callStackDepth - 1 ].s ).linenumber );
		} else {
			gameLocal.Printf( " done\n" );
		}
	}

	// up stack
	callStackDepth--;
	stack = &callStack[ callStackDepth ]; 
	currentFunction = stack->f;
	localstackBase = stack->stackbase;
	NextInstruction( stack->s );

	if ( callStackDepth <= 0 ) {
		// all done
		doneProcessing = true;
		threadDying = true;
		currentFunction = 0;
	}
}

int idInterpreter::CheckEvent( const function_t *func, int argsize ) {
	varEval_t			var;
	int 				start;
	const idEventDef	*evdef;
	idEntity			*ent;

	if ( !func ) {
		Error( "NULL function" );
	}

	assert( func->eventdef );
	evdef = func->eventdef;

	start = localstackUsed - argsize;
	var.intPtr = ( int * )&localstack[ start ];
	ent = GetEntity( *var.entityNumberPtr );

	if ( !ent ) {
		return CHECK_EVENT_NO_ENT;
	}

	if ( !ent->RespondsTo( *evdef ) ) {
		return CHECK_EVENT_NOT_RESPOND;
	}

	return CHECK_EVENT_RESPOND;
}
/*
================
idInterpreter::CallEvent
================
*/
void idInterpreter::CallEvent( const function_t *func, int argsize, idEventReturn *RC ) {
	int 				i;
	int					j;
	varEval_t			var;
	int 				pos;
	int 				start;
	int					data[ D_EVENT_MAXARGS ];
	const idEventDef	*evdef;
	const char			*format;

	if ( !func ) {
		Error( "NULL function" );
	}

	assert( func->eventdef );
	evdef = func->eventdef;

	start = localstackUsed - argsize;
	var.intPtr = ( int * )&localstack[ start ];
	eventEntity = GetEntity( *var.entityNumberPtr );

	if ( !eventEntity || !eventEntity->RespondsTo( *evdef ) ) {
		if ( developer.GetBool() ) {
			// give a warning in developer mode
			if (eventEntity) {
				Warning( "Function '%s' not supported on entity '%s'", evdef->GetName(), eventEntity->name.c_str() );
			}else{
				Warning( "Function '%s' called without a entity", evdef->GetName() );
			}

		}
		if (RC) {
			RC->Fails(); //Event call failed
		}

		PopParms( argsize );

		// always return a safe value when an object doesn't exist
		switch( evdef->GetReturnType() ) {
		case D_EVENT_INTEGER :
			Push( (float)0 );
			break;

		case D_EVENT_FLOAT :
			Push( (float)0 );
			break;

		case D_EVENT_VECTOR :
			Push( vec3_zero );
			break;

		case D_EVENT_STRING :
			PushString( "" );
			break;

		case D_EVENT_ENTITY :
		case D_EVENT_ENTITY_NULL :
			Push( (int)0 );
			break;

		case D_EVENT_TRACE :
		default:
			// unsupported data type
			break;
		}

		eventEntity = NULL;
		return;
	}

	format = evdef->GetArgFormat();
	for( j = 0, i = 0, pos = type_object.Size(); ( pos < argsize ) || ( format[ i ] != 0 ); i++ ) {
		switch( format[ i ] ) {
		case D_EVENT_INTEGER :
			var.intPtr = ( int * )&localstack[ start + pos ];
			data[ i ] = int( *var.floatPtr );
			break;

		case D_EVENT_FLOAT :
			var.intPtr = ( int * )&localstack[ start + pos ];
			( *( float * )&data[ i ] ) = *var.floatPtr;
			break;

		case D_EVENT_VECTOR :
			var.intPtr = ( int * )&localstack[ start + pos ];
			( *( idVec3 ** )&data[ i ] ) = var.vectorPtr;
			break;

		case D_EVENT_STRING :
			( *( const char ** )&data[ i ] ) = ( char * )&localstack[ start + pos ];
			break;

		case D_EVENT_ENTITY :
			var.intPtr = ( int * )&localstack[ start + pos ];
			( *( idEntity ** )&data[ i ] ) = GetEntity( *var.entityNumberPtr );
			if ( !( *( idEntity ** )&data[ i ] ) ) {
				Warning( "Entity not found for event '%s'. Terminating thread.", evdef->GetName() );
				threadDying = true;
				PopParms( argsize );
				if (RC) {
					RC->Fails(); //Event call failed
				}
				return;
			}
			break;

		case D_EVENT_ENTITY_NULL :
			var.intPtr = ( int * )&localstack[ start + pos ];
			( *( idEntity ** )&data[ i ] ) = GetEntity( *var.entityNumberPtr );
			break;

		case D_EVENT_LIST:
			Error( "list type not supported yet (from script for '%s' event)", evdef->GetName() );
			break;

		case D_EVENT_TRACE :
			Error( "trace type not supported from script for '%s' event.", evdef->GetName() );
			break;

		default :
			Error( "Invalid arg format string for '%s' event.", evdef->GetName() );
			break;
		}

		pos += func->parmSize[ j++ ];
	}

	popParms = argsize;

	eventEntity->ProcessEventArgPtr( evdef, data, RC );
	 
	if ( !multiFrameEvent ) {
		if ( popParms ) {
			PopParms( popParms );
		}
		eventEntity = NULL;
	} else {
		doneProcessing = true;
	}
	popParms = 0;
}

/*
================
idInterpreter::BeginMultiFrameEvent
================
*/
bool idInterpreter::BeginMultiFrameEvent( idEntity *ent, const idEventDef *event ) { 
	if ( eventEntity != ent ) {
		Error( "idInterpreter::BeginMultiFrameEvent called with wrong entity" );
	}
	if ( multiFrameEvent ) {
		if ( multiFrameEvent != event ) {
			Error( "idInterpreter::BeginMultiFrameEvent called with wrong event" );
		}
		return false;
	}

	multiFrameEvent = event;
	return true;
}

/*
================
idInterpreter::EndMultiFrameEvent
================
*/
void idInterpreter::EndMultiFrameEvent( idEntity *ent, const idEventDef *event ) {
	if ( multiFrameEvent != event ) {
		Error( "idInterpreter::EndMultiFrameEvent called with wrong event" );
	}

	multiFrameEvent = NULL;
}

/*
================
idInterpreter::MultiFrameEventInProgress
================
*/
bool idInterpreter::MultiFrameEventInProgress( void ) const {
	return multiFrameEvent != NULL;
}

/*
================
idInterpreter::GetEventReturnType
================
*/
char idInterpreter::GetEventReturnType( const function_t *func ) {
	const idEventDef	*evdef;

	if ( !func ) {
		Error( "NULL function" );
	}

	assert( func->eventdef );
	evdef = func->eventdef;
	return evdef->GetReturnType();
}

/*
================
idInterpreter::CallSysEvent
================
*/
void idInterpreter::CallSysEvent( const function_t *func, int argsize, idEventReturn *RC ) {
	int 				i;
	int					j;
	varEval_t			source;
	int 				pos;
	int 				start;
	int					data[ D_EVENT_MAXARGS ];
	const idEventDef	*evdef;
	const char			*format;

	if ( !func ) {
		Error( "NULL function" );
	}

	assert( func->eventdef );
	evdef = func->eventdef;

	start = localstackUsed - argsize;

	format = evdef->GetArgFormat();
	for( j = 0, i = 0, pos = 0; ( pos < argsize ) || ( format[ i ] != 0 ); i++ ) {
		switch( format[ i ] ) {
		case D_EVENT_INTEGER :
			source.intPtr = ( int * )&localstack[ start + pos ];
			*( int * )&data[ i ] = int( *source.floatPtr );
			break;

		case D_EVENT_FLOAT :
			source.intPtr = ( int * )&localstack[ start + pos ];
			*( float * )&data[ i ] = *source.floatPtr;
			break;

		case D_EVENT_VECTOR :
			source.intPtr = ( int * )&localstack[ start + pos ];
			*( idVec3 ** )&data[ i ] = source.vectorPtr;
			break;

		case D_EVENT_STRING :
			*( const char ** )&data[ i ] = ( char * )&localstack[ start + pos ];
			break;

		case D_EVENT_ENTITY :
			source.intPtr = ( int * )&localstack[ start + pos ];
			*( idEntity ** )&data[ i ] = GetEntity( *source.entityNumberPtr );
			if ( !*( idEntity ** )&data[ i ] ) {
				if (RC) RC->Fails();
				Warning( "Entity not found for event '%s'. Terminating thread.", evdef->GetName() );
				threadDying = true;
				PopParms( argsize );
				return;
			}
			break;

		case D_EVENT_ENTITY_NULL :
			source.intPtr = ( int * )&localstack[ start + pos ];
			*( idEntity ** )&data[ i ] = GetEntity( *source.entityNumberPtr );
			break;

		case D_EVENT_LIST:
			Error( "list type not supported yet (from script for '%s' event)", evdef->GetName() );
			break;

		case D_EVENT_TRACE :
			Error( "trace type not supported from script for '%s' event.", evdef->GetName() );
			break;

		default :
			Error( "Invalid arg format string for '%s' event.", evdef->GetName() );
			break;
		}

		pos += func->parmSize[ j++ ];
	}

	popParms = argsize;
	thread->ProcessEventArgPtr( evdef, data, RC );
	if ( popParms ) {
		PopParms( popParms );
	}
	popParms = 0;
}


/*
====================
idInterpreter::Execute
====================
*/
bool idInterpreter::Execute( void ) {
	varEval_t	var_a;
	varEval_t	var_b;
	varEval_t	var_c;
	varEval_t	var;
	statement_t	*st;
	int 		runaway;
	idThread	*newThread;
	float		floatVal;
	idScriptObject *obj;
	const function_t *func;
	idEventReturn *RC; //c4tnt: Local tracking context 
	char		evRT;
	char*		strPtr;
	bool		didTrace;
	idStr		debug_s;

	if ( threadDying || !currentFunction ) {
		return true;
	}

	if ( multiFrameEvent ) {
		// move to previous instruction and call it again
		instructionPointer--;
	}

	runaway = g_runaway.GetInteger();

	doneProcessing = false;
	didTrace = false;
	while( !doneProcessing && !threadDying ) {
		instructionPointer++;

		if ( !--runaway ) {
			if ( didTrace ) {
				Error( "runaway loop error, last execution in %s", currentFunction->Name() );
				return true;
			} else {
				gameLocal.Printf( "^3----------Disasm trace----------^0\n" );
				didTrace = true;
				runaway = g_unrolldisasm.GetInteger();
				if ( runaway <= 0 ) {
					Error( "runaway loop error, last execution in %s", currentFunction->Name() );
					return true;
				}
			}
		}

		st = &gameLocal.program.GetStatement( instructionPointer );
		
		

		if ( didTrace ) {
			opcode_t* op = &idCompiler::opcodes[ st->op ];
			gameLocal.Printf("%6d: ^1%15s^0\n", instructionPointer, op->opname);

			if ( st->a ) {
				debug_s.Clear();
				st->a->PrintInfo( debug_s, instructionPointer );
				gameLocal.Printf( "\t\t^2A ^3%s^0", debug_s.c_str() );
				if ( GetRegisterVar( st->a, debug_s ) ) {
					gameLocal.Printf( "\t[^4%s^0]", debug_s.c_str() );
				}
				gameLocal.Printf( "\n" );
			}
			
			if ( st->b ) {
				debug_s.Clear();
				st->b->PrintInfo( debug_s, instructionPointer );
				gameLocal.Printf( "\t\t^2B ^3%s^0", debug_s.c_str() );
				if ( GetRegisterVar( st->b, debug_s ) ) {
					gameLocal.Printf( "\t[^4%s^0]", debug_s.c_str() );
				}
				gameLocal.Printf( "\n" );
			}
		}

		switch( st->op ) {
		case OP_NOOP:
			break;
		case OP_RETURN:
			LeaveFunction( st->a );
			break;
		case OP_DEFRETURN:
			LeaveFunctionSafe( (etype_t)st->a->value.jumpOffset );
			break;
		case OP_ASSERT:
			if ( st->a == 0 || *GetVariable( st->a ).intPtr == 0) {
				Error( "Function '%s' assertion failed", currentFunction->Name() );
			}
			break;
		case OP_TRACEFALL:
			var_a = GetVariable( st->a );
			didTrace = true;
			runaway = (int)*var_a.floatPtr;
			break;

		case OP_THREAD:
			newThread = new idThread( this, st->a->value.functionPtr, st->b->value.argSize );
			newThread->Start();
			floatVal = newThread->GetThreadNum();

			// return the thread number to the script
			// gameLocal.program.ReturnFloat( newThread->GetThreadNum() );
			PopParms( st->b->value.argSize );
			Push( floatVal );
			break;

		case OP_OBJTHREAD:
			var_a = GetVariable( st->a );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( obj ) {
				func = obj->GetTypeDef()->GetFunction( st->b->value.virtualFunction );
				assert( st->c->value.argSize == func->parmTotal );
				newThread = new idThread( this, GetEntity( *var_a.entityNumberPtr ), func, func->parmTotal );
				newThread->Start();

				// return the thread number to the script
				// gameLocal.program.ReturnFloat( newThread->GetThreadNum() );
				floatVal = newThread->GetThreadNum();
			} else {
				// return a null thread to the script
				// gameLocal.program.ReturnFloat( 0.0f );
				floatVal = 0.0f;
			}
			PopParms( st->c->value.argSize );
			Push( floatVal );
			break;

		case OP_CALL:
			EnterFunction( st->a->value.functionPtr, false );
			break;

		case OP_VPCALL:
			evRT = GetEventReturnType( st->a->value.functionPtr ); //Get return type
			RC = idEventReturn::Alloc( thread, st->a->value.functionPtr->Name() ); 

			switch ( CheckEvent( st->a->value.functionPtr, st->b->value.argSize ) )
			{
			case CHECK_EVENT_RESPOND:
				CallEvent( st->a->value.functionPtr, st->b->value.argSize, RC );
				break;
			case CHECK_EVENT_NOT_RESPOND:
				CallSysEvent( st->a->value.functionPtr, st->b->value.argSize - type_object.Size(), RC );	//strip 'self' arg
				PopParms( type_object.Size() );	//exclude 'self' from the stack
				break;
			case CHECK_EVENT_NO_ENT:
				PopParms( st->b->value.argSize );	// pop all args
				RC->Fails(); // and fail this call
			}

			if (RC->Test(0, EVRS_EXECUTED))
			{
				//Multiframed SYS?
				if (st->a->value.functionPtr) {
					Error( "SysEvent not executed: %s", st->a->value.functionPtr->Name());
				}else{
					Error( "SysEvent not executed: unknown event" );
				}
			}

			if ( evRT == D_EVENT_INTEGER ) evRT = D_EVENT_FLOAT;	// Type cast for script

			if ( didTrace ) {
				if ( RC->Test(EVRS_FAILED, EVRS_FAILED) ) {
					gameLocal.Printf( "^4Call failed^0\n" );
				}
			}

			if ( !RC->Load( this, evRT ) ) {
				// No return value
				Error( "Value not returned from obj-event" );
			}
			RC->Checkout( EVRS_HANDLED );	// Kill RCs;

			break;

		case OP_EVENTCALL:

			evRT = GetEventReturnType( st->a->value.functionPtr ); //Get return type
			RC = idEventReturn::Alloc( thread, st->a->value.functionPtr->Name() ); 
			
			CallEvent( st->a->value.functionPtr, st->b->value.argSize, RC );
			
			if ( RC->Test(0, EVRS_EXECUTED) ) {
				// Multiframed event
				if (st->a->value.functionPtr) {
					Error( "Event not executed or multiframed: %s", st->a->value.functionPtr->Name());
				}else{
					Error( "Event not executed or multiframed: unknown event" );
				}
			}
			
			if ( evRT == D_EVENT_INTEGER ) evRT = D_EVENT_FLOAT;	// Type cast for script

			if ( didTrace ) {
				if ( RC->Test(EVRS_FAILED, EVRS_FAILED) ) {
					gameLocal.Printf( "^4Call failed^0\n" );
				}
			}

			if ( !RC->Load( this, evRT ) ) {
				// No return value
				Error( "Value not returned from obj-event %s", st->a->value.functionPtr->Name() );
			}
			RC->Checkout( EVRS_HANDLED );	// Kill RCs;
			break;

		case OP_OBJECTCALL:	
			var_a = GetVariable( st->a );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( obj ) {
				func = obj->GetTypeDef()->GetFunction( st->b->value.virtualFunction );
				EnterFunction( func, false );
			} else {
				// we don't know, how many data is returned from this function
				PopParms( st->c->value.argSize & 0xFFFF );
				Pushz( 0, ( st->c->value.argSize >> 16 ) & 0xFFFF );
				if ( didTrace ) {
					gameLocal.Printf( "^4Objcall without object, unrolled^0\n" );
				}
			}
			break;

		case OP_SYSCALL:

			evRT = GetEventReturnType( st->a->value.functionPtr ); //Get return type
			RC = idEventReturn::Alloc( thread, st->a->value.functionPtr->Name() ); 
			CallSysEvent( st->a->value.functionPtr, st->b->value.argSize, RC );
			if (RC->Test(0, EVRS_EXECUTED))
			{
				//Multiframed SYS?
				if (st->a->value.functionPtr) {
					Error( "SysEvent not executed: %s", st->a->value.functionPtr->Name());
				}else{
					Error( "SysEvent not executed: unknown event" );
				}
			}

			if ( evRT == D_EVENT_INTEGER ) evRT = D_EVENT_FLOAT;	// Type cast for script

			if ( didTrace ) {
				if ( RC->Test(EVRS_FAILED, EVRS_FAILED) ) {
					gameLocal.Printf( "^4Call failed^0\n" );
				}
			}

			if ( !RC->Load( this, evRT ) ) {
				// No return value
				Error( "Value not returned from sys obj-event" );
			}
			RC->Checkout( EVRS_HANDLED );	// Kill RCs;
			break;

		case OP_IFNOT:
			var_a = GetVariable( st->a );
			if ( *var_a.intPtr == 0 ) {
				NextInstruction( instructionPointer + st->b->value.jumpOffset );
			}
			break;

		case OP_IF:
			var_a = GetVariable( st->a );
			if ( *var_a.intPtr != 0 ) {
				NextInstruction( instructionPointer + st->b->value.jumpOffset );
			}
			break;

		case OP_GOTO:
			NextInstruction( instructionPointer + st->a->value.jumpOffset );
			break;

		case OP_ADD_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = *var_a.floatPtr + *var_b.floatPtr;
			break;

		case OP_ADD_V:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.vectorPtr = *var_a.vectorPtr + *var_b.vectorPtr;
			break;

		case OP_ADD_S:
			SetString( st->c, GetString( st->a ) );
			AppendString( st->c, GetString( st->b ) );
			break;

		case OP_ADD_FS:
			var_a = GetVariable( st->a );
			SetString( st->c, FloatToString( *var_a.floatPtr ) );
			AppendString( st->c, GetString( st->b ) );
			break;

		case OP_ADD_SF:
			var_b = GetVariable( st->b );
			SetString( st->c, GetString( st->a ) );
			AppendString( st->c, FloatToString( *var_b.floatPtr ) );
			break;

		case OP_ADD_VS:
			var_a = GetVariable( st->a );
			SetString( st->c, var_a.vectorPtr->ToString() );
			AppendString( st->c, GetString( st->b ) );
			break;

		case OP_ADD_SV:
			var_b = GetVariable( st->b );
			SetString( st->c, GetString( st->a ) );
			AppendString( st->c, var_b.vectorPtr->ToString() );
			break;

		case OP_SUB_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = *var_a.floatPtr - *var_b.floatPtr;
			break;

		case OP_SUB_V:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.vectorPtr = *var_a.vectorPtr - *var_b.vectorPtr;
			break;

		case OP_MUL_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = *var_a.floatPtr * *var_b.floatPtr;
			break;

		case OP_MUL_V:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = *var_a.vectorPtr * *var_b.vectorPtr;
			break;

		case OP_MUL_FV:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.vectorPtr = *var_a.floatPtr * *var_b.vectorPtr;
			break;

		case OP_MUL_VF:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.vectorPtr = *var_a.vectorPtr * *var_b.floatPtr;
			break;

		case OP_DIV_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );

			if ( *var_b.floatPtr == 0.0f ) {
				Warning( "Divide by zero" );
				*var_c.floatPtr = idMath::INFINITY;
			} else {
				*var_c.floatPtr = *var_a.floatPtr / *var_b.floatPtr;
			}
			break;

		case OP_MOD_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable ( st->c );

			if ( *var_b.floatPtr == 0.0f ) {
				Warning( "Divide by zero" );
				*var_c.floatPtr = *var_a.floatPtr;
			} else {
				*var_c.floatPtr = static_cast<int>( *var_a.floatPtr ) % static_cast<int>( *var_b.floatPtr );
			}
			break;

		case OP_BITAND:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = static_cast<int>( *var_a.floatPtr ) & static_cast<int>( *var_b.floatPtr );
			break;

		case OP_BITOR:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = static_cast<int>( *var_a.floatPtr ) | static_cast<int>( *var_b.floatPtr );
			break;

		case OP_GE:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.floatPtr >= *var_b.floatPtr );
			break;

		case OP_LE:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.floatPtr <= *var_b.floatPtr );
			break;

		case OP_GT:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.floatPtr > *var_b.floatPtr );
			break;

		case OP_LT:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.floatPtr < *var_b.floatPtr );
			break;

		case OP_AND:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.floatPtr != 0.0f ) && ( *var_b.floatPtr != 0.0f );
			break;

		case OP_AND_BOOLF:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.intPtr != 0 ) && ( *var_b.floatPtr != 0.0f );
			break;

		case OP_AND_FBOOL:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.floatPtr != 0.0f ) && ( *var_b.intPtr != 0 );
			break;

		case OP_AND_BOOLBOOL:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.intPtr != 0 ) && ( *var_b.intPtr != 0 );
			break;

		case OP_OR:	
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.floatPtr != 0.0f ) || ( *var_b.floatPtr != 0.0f );
			break;

		case OP_OR_BOOLF:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.intPtr != 0 ) || ( *var_b.floatPtr != 0.0f );
			break;

		case OP_OR_FBOOL:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.floatPtr != 0.0f ) || ( *var_b.intPtr != 0 );
			break;
			
		case OP_OR_BOOLBOOL:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.intPtr != 0 ) || ( *var_b.intPtr != 0 );
			break;
			
		case OP_NOT_BOOL:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.intPtr == 0 );
			break;

		case OP_NOT_F:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.floatPtr == 0.0f );
			break;

		case OP_NOT_V:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.vectorPtr == vec3_zero );
			break;

		case OP_NOT_S:
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( strlen( GetString( st->a ) ) == 0 );
			break;

		case OP_NOT_ENT:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( GetEntity( *var_a.entityNumberPtr ) == NULL );
			break;

		case OP_NEG_F:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = -*var_a.floatPtr;
			break;

		case OP_NEG_V:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.vectorPtr = -*var_a.vectorPtr;
			break;

		case OP_INT_F:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = static_cast<int>( *var_a.floatPtr );
			break;

		case OP_EQ_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.floatPtr == *var_b.floatPtr );
			break;

		case OP_EQ_V:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.vectorPtr == *var_b.vectorPtr );
			break;

		case OP_EQ_S:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( idStr::Cmp( GetString( st->a ), GetString( st->b ) ) == 0 );
			break;

		case OP_EQ_E:
		case OP_EQ_EO:
		case OP_EQ_OE:
		case OP_EQ_OO:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.entityNumberPtr == *var_b.entityNumberPtr );
			break;

		case OP_NE_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.floatPtr != *var_b.floatPtr );
			break;

		case OP_NE_V:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.vectorPtr != *var_b.vectorPtr );
			break;

		case OP_NE_S:
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( idStr::Cmp( GetString( st->a ), GetString( st->b ) ) != 0 );
			break;

		case OP_NE_E:
		case OP_NE_EO:
		case OP_NE_OE:
		case OP_NE_OO:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( *var_a.entityNumberPtr != *var_b.entityNumberPtr );
			break;

		case OP_UADD_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.floatPtr += *var_a.floatPtr;
			break;

		case OP_UADD_V:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.vectorPtr += *var_a.vectorPtr;
			break;

		case OP_USUB_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.floatPtr -= *var_a.floatPtr;
			break;

		case OP_USUB_V:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.vectorPtr -= *var_a.vectorPtr;
			break;

		case OP_UMUL_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.floatPtr *= *var_a.floatPtr;
			break;

		case OP_UMUL_V:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.vectorPtr *= *var_a.floatPtr;
			break;

		case OP_UDIV_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );

			if ( *var_a.floatPtr == 0.0f ) {
				Warning( "Divide by zero" );
				*var_b.floatPtr = idMath::INFINITY;
			} else {
				*var_b.floatPtr = *var_b.floatPtr / *var_a.floatPtr;
			}
			break;

		case OP_UDIV_V:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );

			if ( *var_a.floatPtr == 0.0f ) {
				Warning( "Divide by zero" );
				var_b.vectorPtr->Set( idMath::INFINITY, idMath::INFINITY, idMath::INFINITY );
			} else {
				*var_b.vectorPtr = *var_b.vectorPtr / *var_a.floatPtr;
			}
			break;

		case OP_UMOD_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );

			if ( *var_a.floatPtr == 0.0f ) {
				Warning( "Divide by zero" );
				*var_b.floatPtr = *var_a.floatPtr;
			} else {
				*var_b.floatPtr = static_cast<int>( *var_b.floatPtr ) % static_cast<int>( *var_a.floatPtr );
			}
			break;

		case OP_UOR_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.floatPtr = static_cast<int>( *var_b.floatPtr ) | static_cast<int>( *var_a.floatPtr );
			break;

		case OP_UAND_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.floatPtr = static_cast<int>( *var_b.floatPtr ) & static_cast<int>( *var_a.floatPtr );
			break;

		case OP_UINC_F:
			var_a = GetVariable( st->a );
			( *var_a.floatPtr )++;
			break;

		case OP_UINCP_F:
			var_a = GetVariable( st->a );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( obj ) {
				var.bytePtr = &obj->data[ st->b->value.ptrOffset ];
				( *var.floatPtr )++;
			}
			break;

		case OP_UDEC_F:
			var_a = GetVariable( st->a );
			( *var_a.floatPtr )--;
			break;

		case OP_UDECP_F:
			var_a = GetVariable( st->a );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( obj ) {
				var.bytePtr = &obj->data[ st->b->value.ptrOffset ];
				( *var.floatPtr )--;
			}
			break;

		case OP_XST_S:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( idStr::Cmp( GetString( st->a ), "" ) );
			break;

		case OP_XST_E:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( GetEntity( *var_a.entityNumberPtr ) != NULL );
			break;

		case OP_XST_V:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( var_a.vectorPtr->LengthSqr() == 0 );
			break;

		case OP_XST_O:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ( GetScriptObject( *var_a.entityNumberPtr ) != NULL );
			break;

		case OP_COMP_F:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			*var_c.floatPtr = ~static_cast<int>( *var_a.floatPtr );
			break;

		case OP_STORE_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.floatPtr = *var_a.floatPtr;
			break;

		case OP_STORE_ENT:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.entityNumberPtr = *var_a.entityNumberPtr;
			break;

		case OP_STORE_BOOL:	
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.intPtr = *var_a.intPtr;
			break;

		case OP_STORE_OBJENT:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( !obj ) {
				*var_b.entityNumberPtr = 0;
			} else if ( !obj->GetTypeDef()->Inherits( st->b->TypeDef() ) ) {
				//Warning( "object '%s' cannot be converted to '%s'", obj->GetTypeName(), st->b->TypeDef()->Name() );
				*var_b.entityNumberPtr = 0;
			} else {
				*var_b.entityNumberPtr = *var_a.entityNumberPtr;
			}
			break;

		case OP_STORE_OBJ:
		case OP_STORE_ENTOBJ:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.entityNumberPtr = *var_a.entityNumberPtr;
			break;

		case OP_STORE_A:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.intPtr = *var_a.intPtr;
			break;

		case OP_STORE_S:
			SetString( st->b, GetString( st->a ) );
			break;

		case OP_STORE_V:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.vectorPtr = *var_a.vectorPtr;
			break;

		case OP_STORE_FTOS:
			var_a = GetVariable( st->a );
			SetString( st->b, FloatToString( *var_a.floatPtr ) );
			break;

		case OP_STORE_BTOS:
			var_a = GetVariable( st->a );
			SetString( st->b, *var_a.intPtr ? "true" : "false" );
			break;

		case OP_STORE_VTOS:
			var_a = GetVariable( st->a );
			SetString( st->b, var_a.vectorPtr->ToString() );
			break;

		case OP_STORE_FTOBOOL:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			if ( *var_a.floatPtr != 0.0f ) {
				*var_b.intPtr = 1;
			} else {
				*var_b.intPtr = 0;
			}
			break;

		case OP_STORE_BOOLTOF:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			*var_b.floatPtr = static_cast<float>( *var_a.intPtr );
			break;

		case OP_STOREP_F:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->floatPtr ) {
				var_a = GetVariable( st->a );
				*var_b.evalPtr->floatPtr = *var_a.floatPtr;
			}
			break;

		case OP_STOREP_ENT:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->entityNumberPtr ) {
				var_a = GetVariable( st->a );
				*var_b.evalPtr->entityNumberPtr = *var_a.entityNumberPtr;
			}
			break;

		case OP_STOREP_FLD:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->intPtr ) {
				var_a = GetVariable( st->a );
				*var_b.evalPtr->intPtr = *var_a.intPtr;
			}
			break;

		case OP_STOREP_BOOL:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->intPtr ) {
				var_a = GetVariable( st->a );
				*var_b.evalPtr->intPtr = *var_a.intPtr;
			}
			break;

		case OP_STOREP_S:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->stringPtr ) {
				idStr::Copynz( var_b.evalPtr->stringPtr, GetString( st->a ), MAX_STRING_LEN );
			}
			break;

		case OP_STOREP_V:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->vectorPtr ) {
				var_a = GetVariable( st->a );
				*var_b.evalPtr->vectorPtr = *var_a.vectorPtr;
			}
			break;
		
		case OP_STOREP_FTOS:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->stringPtr ) {
				var_a = GetVariable( st->a );
				idStr::Copynz( var_b.evalPtr->stringPtr, FloatToString( *var_a.floatPtr ), MAX_STRING_LEN );
			}
			break;

		case OP_STOREP_BTOS:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->stringPtr ) {
				var_a = GetVariable( st->a );
				if ( *var_a.floatPtr != 0.0f ) {
					idStr::Copynz( var_b.evalPtr->stringPtr, "true", MAX_STRING_LEN );
				} else {
					idStr::Copynz( var_b.evalPtr->stringPtr, "false", MAX_STRING_LEN );
				}
			}
			break;

		case OP_STOREP_VTOS:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->stringPtr ) {
				var_a = GetVariable( st->a );
				idStr::Copynz( var_b.evalPtr->stringPtr, var_a.vectorPtr->ToString(), MAX_STRING_LEN );
			}
			break;

		case OP_STOREP_FTOBOOL:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->intPtr ) {
				var_a = GetVariable( st->a );
				if ( *var_a.floatPtr != 0.0f ) {
					*var_b.evalPtr->intPtr = 1;
				} else {
					*var_b.evalPtr->intPtr = 0;
				}
			}
			break;

		case OP_STOREP_BOOLTOF:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->floatPtr ) {
				var_a = GetVariable( st->a );
				*var_b.evalPtr->floatPtr = static_cast<float>( *var_a.intPtr );
			}
			break;

		case OP_STOREP_OBJ:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->entityNumberPtr ) {
				var_a = GetVariable( st->a );
				*var_b.evalPtr->entityNumberPtr = *var_a.entityNumberPtr;
			}
			break;

		case OP_STOREP_A:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->intPtr ) {
				var_a = GetVariable( st->a );
				*var_b.evalPtr->intPtr = *var_a.intPtr;
			}
			break;

		case OP_STOREP_OBJENT:
			var_b = GetVariable( st->b );
			if ( var_b.evalPtr && var_b.evalPtr->entityNumberPtr ) {
				var_a = GetVariable( st->a );
				obj = GetScriptObject( *var_a.entityNumberPtr );
				if ( !obj ) {
					*var_b.evalPtr->entityNumberPtr = 0;

				// st->b points to type_pointer, which is just a temporary that gets its type reassigned, so we store the real type in st->c
				// so that we can do a type check during run time since we don't know what type the script object is at compile time because it
				// comes from an entity
				} else if ( !obj->GetTypeDef()->Inherits( st->c->TypeDef() ) ) {
					Warning( "object '%s' cannot be converted to '%s'", obj->GetTypeName(), st->c->TypeDef()->Name() );
					*var_b.evalPtr->entityNumberPtr = 0;
				} else {
					*var_b.evalPtr->entityNumberPtr = *var_a.entityNumberPtr;
				}
			}
			break;

		case OP_ADDRESS_O:
		case OP_ADDRESS_E:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( obj ) {
				var_c.evalPtr->bytePtr = &obj->data[ st->b->value.ptrOffset ];
			} else {
				var_c.evalPtr->bytePtr = NULL;
			}
			break;

		case OP_INDIRECT_F:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( obj ) {
				var.bytePtr = &obj->data[ st->b->value.ptrOffset ];
				*var_c.floatPtr = *var.floatPtr;
			} else {
				*var_c.floatPtr = 0.0f;
			}
			break;

		case OP_INDIRECT_ENT:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( obj ) {
				var.bytePtr = &obj->data[ st->b->value.ptrOffset ];
				*var_c.entityNumberPtr = *var.entityNumberPtr;
			} else {
				*var_c.entityNumberPtr = 0;
			}
			break;

		case OP_INDIRECT_BOOL:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( obj ) {
				var.bytePtr = &obj->data[ st->b->value.ptrOffset ];
				*var_c.intPtr = *var.intPtr;
			} else {
				*var_c.intPtr = 0;
			}
			break;

		case OP_INDIRECT_S:
			var_a = GetVariable( st->a );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( obj ) {
				var.bytePtr = &obj->data[ st->b->value.ptrOffset ];
				SetString( st->c, var.stringPtr );
			} else {
				SetString( st->c, "" );
			}
			break;

		case OP_INDIRECT_V:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( obj ) {
				var.bytePtr = &obj->data[ st->b->value.ptrOffset ];
				*var_c.vectorPtr = *var.vectorPtr;
			} else {
				var_c.vectorPtr->Zero();
			}
			break;

		case OP_INDIRECT_OBJ:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( !obj ) {
				*var_c.entityNumberPtr = 0;
			} else {
				var.bytePtr = &obj->data[ st->b->value.ptrOffset ];
				*var_c.entityNumberPtr = *var.entityNumberPtr;
			}
			break;
		
		case OP_INDIRECT_A:
			var_a = GetVariable( st->a );
			var_c = GetVariable( st->c );
			obj = GetScriptObject( *var_a.entityNumberPtr );
			if ( obj ) {
				var.bytePtr = &obj->data[ st->b->value.ptrOffset ];
				*var_c.intPtr = *var.intPtr;
			} else {
				*var_c.intPtr = 0.0f; //NULL is a special array state - uninitialized array
			}
			break;

		case OP_PUSH_F:
			var_a = GetVariable( st->a );
			Push( *var_a.floatPtr );
			break;

		case OP_PUSH_FTOS:
			var_a = GetVariable( st->a );
			PushString( FloatToString( *var_a.floatPtr ) );
			break;

		case OP_PUSH_BTOF:
			var_a = GetVariable( st->a );
			floatVal = *var_a.intPtr;
			Push( floatVal );
			break;

		case OP_PUSH_FTOB:
			var_a = GetVariable( st->a );
			if ( *var_a.floatPtr != 0.0f ) {
				Push( (int)1 );
			} else {
				Push( (int)0 );
			}
			break;

		case OP_PUSH_VTOS:
			var_a = GetVariable( st->a );
			PushString( var_a.vectorPtr->ToString() );
			break;

		case OP_PUSH_BTOS:
			var_a = GetVariable( st->a );
			PushString( *var_a.intPtr ? "true" : "false" );
			break;

		case OP_PUSH_ENT:
			var_a = GetVariable( st->a );
			Push( *var_a.entityNumberPtr );
			break;

		case OP_PUSH_S:
			PushString( GetString( st->a ) );
			break;

		case OP_PUSH_V:
			var_a = GetVariable( st->a );
			Push( *var_a.vectorPtr );
			break;

		case OP_PUSH_OBJ:
			var_a = GetVariable( st->a );
			Push( *var_a.entityNumberPtr );
			break;

		case OP_PUSH_OBJENT:
			var_a = GetVariable( st->a );
			Push( *var_a.entityNumberPtr );
			break;

		case OP_PUSH_A:
			var_a = GetVariable( st->a );
			Push( *var_a.intPtr );
			break;

		case OP_RPOP_F:
			var_c = GetVariable( st->c );
			Pop( *var_c.floatPtr );
			break;
		case OP_RPOP_V:
			var_c = GetVariable( st->c );
			Pop( *var_c.vectorPtr );
			break;
		case OP_RPOP_S:
			strPtr = GetStringWrite( st->c );
			if ( strPtr ) {
				PopString( strPtr );
			} else {
				Error( "String write failure" );
			}
			break;
		case OP_RPOP_ENT:
			var_c = GetVariable( st->c );
			Pop( *var_c.entityNumberPtr );
			break;
		case OP_RPOP_OBJENT:
			var_c = GetVariable( st->c );
			Pop( *var_c.entityNumberPtr );
			break;
		case OP_RPOP_OBJ:
			var_c = GetVariable( st->c );
			Pop( *var_c.entityNumberPtr );
			break;
		case OP_RPOP_FTOB:
			var_c = GetVariable( st->c );
			Pop( floatVal );
			if ( floatVal ) {
				*var_c.intPtr = 1;
			} else {
				*var_c.intPtr = 0;
			}
			break;
		
		case OP_RPOP_A:					// Do this ))) !
			break;
		
		case OP_BREAK:
		case OP_CONTINUE:
			break;

		case OP_INDEX_PTR:		//c4tnt: array 
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			Error( "Array(%i) ptr requested by idx %i", *var_a.intPtr, *var_b.floatPtr );
			break;
		case OP_INDEX_F:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			Error( "Array(%i) float requested by idx %i", *var_a.intPtr, *var_b.floatPtr );
			break;
		case OP_INDEX_V:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			Error( "Array(%i) vector requested by idx %i", *var_a.intPtr, *var_b.floatPtr );
			break;
		case OP_INDEX_S:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			Error( "Array(%i) string requested by idx %i", *var_a.intPtr, *var_b.floatPtr );
			break;
		case OP_INDEX_ENT:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			Error( "Array(%i) entity requested by idx %i", *var_a.intPtr, *var_b.floatPtr );
			break;
		case OP_INDEX_BOOL:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			Error( "Array(%i) boolean requested by idx %i", *var_a.intPtr, *var_b.floatPtr );
			break;
		case OP_INDEX_OBJ:
			var_a = GetVariable( st->a );
			var_b = GetVariable( st->b );
			Error( "Array(%i) object requested by idx %i", *var_a.intPtr, *var_b.floatPtr );
			break;

		case OP_MKARR_F:
			var_a = GetVariable( st->a );
			Error( "Float array[%f] creation", *var_a.floatPtr );
			break;
		case OP_MKARR_V:
			var_a = GetVariable( st->a );
			Error( "Vector array[%f] creation", *var_a.floatPtr );
			break;
		case OP_MKARR_S:
			var_a = GetVariable( st->a );
			Error( "String array[%f] creation", *var_a.floatPtr );
			break;
		case OP_MKARR_ENT:
			var_a = GetVariable( st->a );
			Error( "Entity array[%f] creation", *var_a.floatPtr );
			break;
		case OP_MKARR_BOOL:
			var_a = GetVariable( st->a );
			Error( "Boolean array[%f] creation", *var_a.floatPtr );
			break;
		case OP_MKARR_OBJ:
			var_a = GetVariable( st->a );
			Error( "Object array[%f] creation", *var_a.floatPtr );
			break;
		case OP_RMARR:
			var_a = GetVariable( st->a );
			Error( "Array(%i) have been removed ", *var_a.intPtr );
			break;

		case OP_ALLOC_OBJ:
			Error( "Floating object allocation", *var_a.floatPtr );
			break;
		case OP_DEALLOC_OBJ:
			Error( "Floating object freeing", *var_a.floatPtr );
			break;

		default:
			Error( "Bad opcode %i", st->op );
			break;
		}

		if ( didTrace ) {
			if ( st->c ) {
				debug_s.Clear();
				st->c->PrintInfo( debug_s, instructionPointer );
				gameLocal.Printf( "\t\t^2C ^3%s^0", debug_s.c_str() );
				if ( GetRegisterVar( st->c, debug_s ) ) {
					gameLocal.Printf( "\t[^4%s^0]", debug_s.c_str() );
				}
				gameLocal.Printf( "\n" );
			}
		}

	}

	return threadDying;
}