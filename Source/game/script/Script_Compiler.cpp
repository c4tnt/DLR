// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

#define ARRAY_IND_PRIORITY	1
#define FUNCTION_PRIORITY	2
#define INT_PRIORITY		2
#define NOT_PRIORITY		5
#define TILDE_PRIORITY		5
#define TOP_PRIORITY		7

//bool idCompiler::punctuationValid[ 256 ];
char *idCompiler::punctuation[] = {
	"+=", "-=", "*=", "/=", "%=", "&=", "|=", "++", "--",
	"&&", "||", "<=", ">=", "==", "!=", "::", ";",  ",",
	"~",  "!",  "*",  "/",  "%",  "(",   ")",  "-", "+",
	"=",  "[",  "]",  ".",  "<",  ">" ,  "&",  "|", ":",  NULL
};

opcode_t idCompiler::opcodes[] = {
	{ "<NOOP>", "NOOP", -1, 0, false, &def_void, &def_void, &def_void }, //This is a real noop? :)
	{ "<RETURN>", "RETURN", -1, 0, false, &def_void, &def_void, &def_void },
	{ "<DEFRETURN>", "DEFRETURN", -1, 0, false, &def_jumpoffset, &def_void, &def_void },

	{ "<ASSERT>", "ASSERT", -1, 0, false, &def_float, &def_void, &def_void },
	{ "<TRACEFALL>", "TRACEFALL", -1, 0, false, &def_float, &def_void, &def_void },

	{ "++", "UINC_F", 1, 6, true, &def_float, &def_void, &def_void },
	{ "++", "UINCP_F", 1, 0, true, &def_object, &def_field, &def_float },
	{ "--", "UDEC_F", 1, 6, true, &def_float, &def_void, &def_void },
	{ "--", "UDECP_F", 1, 0, true, &def_object, &def_field, &def_float },
	{ "?", "XST_S", 1, 0, true, &def_string, &def_void, &def_float },
	{ "?", "XST_E", 1, 0, true, &def_entity, &def_void, &def_float },
	{ "?", "XST_V", 1, 0, true, &def_vector, &def_void, &def_float },
	{ "?", "XST_O", 1, 0, true, &def_object, &def_void, &def_float },

	{ "~", "COMP_F", -1, 0, false, &def_float, &def_void, &def_float },
	
	{ "*", "MUL_F", 3, 0, false, &def_float, &def_float, &def_float },
	{ "*", "MUL_V", 3, 0, false, &def_vector, &def_vector, &def_float },
	{ "*", "MUL_FV", 3, 0, false, &def_float, &def_vector, &def_vector },
	{ "*", "MUL_VF", 3, 0, false, &def_vector, &def_float, &def_vector },
	
	{ "/", "DIV", 3, 0, false, &def_float, &def_float, &def_float },
	{ "%", "MOD_F",	3, 0, false, &def_float, &def_float, &def_float },
	
	{ "+", "ADD_F", 4, 0, false, &def_float, &def_float, &def_float },
	{ "+", "ADD_V", 4, 0, false, &def_vector, &def_vector, &def_vector },
	{ "+", "ADD_S", 4, 0, false, &def_string, &def_string, &def_string },
	{ "+", "ADD_FS", 4, 0, false, &def_float, &def_string, &def_string },
	{ "+", "ADD_SF", 4, 0, false, &def_string, &def_float, &def_string },
	{ "+", "ADD_VS", 4, 0, false, &def_vector, &def_string, &def_string },
	{ "+", "ADD_SV", 4, 0, false, &def_string, &def_vector, &def_string },
	
	{ "-", "SUB_F", 4, 0, false, &def_float, &def_float, &def_float },
	{ "-", "SUB_V", 4, 0, false, &def_vector, &def_vector, &def_vector },
	
	{ "==", "EQ_F", 5, 0, false, &def_float, &def_float, &def_float },
	{ "==", "EQ_V", 5, 0, false, &def_vector, &def_vector, &def_float },
	{ "==", "EQ_S", 5, 0, false, &def_string, &def_string, &def_float },
	{ "==", "EQ_E", 5, 0, false, &def_entity, &def_entity, &def_float },
	{ "==", "EQ_EO", 5, 0, false, &def_entity, &def_object, &def_float },
	{ "==", "EQ_OE", 5, 0, false, &def_object, &def_entity, &def_float },
	{ "==", "EQ_OO", 5, 0, false, &def_object, &def_object, &def_float },
	
	{ "!=", "NE_F", 5, 0, false, &def_float, &def_float, &def_float },
	{ "!=", "NE_V", 5, 0, false, &def_vector, &def_vector, &def_float },
	{ "!=", "NE_S", 5, 0, false, &def_string, &def_string, &def_float },
    { "!=", "NE_E", 5, 0, false, &def_entity, &def_entity, &def_float },
	{ "!=", "NE_EO", 5, 0, false, &def_entity, &def_object, &def_float },
	{ "!=", "NE_OE", 5, 0, false, &def_object, &def_entity, &def_float },
	{ "!=", "NE_OO", 5, 0, false, &def_object, &def_object, &def_float },
	
	{ "<=", "LE", 5, 0, false, &def_float, &def_float, &def_float },
	{ ">=", "GE", 5, 0, false, &def_float, &def_float, &def_float },
	{ "<", "LT", 5, 0, false, &def_float, &def_float, &def_float },
	{ ">", "GT", 5, 0, false, &def_float, &def_float, &def_float },

	{ ".", "INDIRECT_F", 1, 0, false, &def_object, &def_field, &def_float },
	{ ".", "INDIRECT_V", 1, 0, false, &def_object, &def_field, &def_vector },
	{ ".", "INDIRECT_S", 1, 0, false, &def_object, &def_field, &def_string },
	{ ".", "INDIRECT_E", 1, 0, false, &def_object, &def_field, &def_entity },
	{ ".", "INDIRECT_BOOL", 1, 0, false, &def_object, &def_field, &def_boolean },
	{ ".", "INDIRECT_OBJ", 1, 0, false, &def_object, &def_field, &def_object },
	{ ".", "INDIRECT_A", 1, 0, false, &def_object, &def_field, &def_array },

	{ ".", "ADDRESS_O", 1, 0, false, &def_object, &def_field, &def_pointer },
	{ ".", "ADDRESS_E", 1, 0, false, &def_entity, &def_field, &def_pointer },

	{ ".", "EVENTCALL", 2, 0, false, &def_entity, &def_function, &def_void },			// Call object's event
	{ ".", "OBJECTCALL", 2, 0, false, &def_object, &def_function, &def_void },			// Call object's function
	{ ".", "SYSCALL", 2, 0, false, &def_void, &def_function, &def_void },				// Call sys event
	{ "<.>", "VPCALL", 2, 0, false, &def_object, &def_function, &def_void },			// Call sys event or object's event if possable

	{ "=", "STORE_F", 6, 1, true, &def_float, &def_float, &def_float },
	{ "=", "STORE_V", 6, 1, true, &def_vector, &def_vector, &def_vector },
	{ "=", "STORE_S", 6, 1, true, &def_string, &def_string, &def_string },
	{ "=", "STORE_ENT", 6, 1, true, &def_entity, &def_entity, &def_entity },
	{ "=", "STORE_BOOL", 6, 1, true, &def_boolean, &def_boolean, &def_boolean },
	{ "=", "STORE_OBJENT", 6, 1, true, &def_object, &def_entity, &def_object },
	{ "=", "STORE_OBJ", 6, 1, true, &def_object, &def_object, &def_object },
	{ "=", "STORE_ENTOBJ", 6, 1, true, &def_entity, &def_object, &def_object },
	{ "=", "STORE_A", 6, 1, true, &def_array, &def_array, &def_array },
	
	{ "=", "STORE_FTOS", 6, 1, true, &def_string, &def_float, &def_string },
	{ "=", "STORE_BTOS", 6, 1, true, &def_string, &def_boolean, &def_string },
	{ "=", "STORE_VTOS", 6, 1, true, &def_string, &def_vector, &def_string },
	{ "=", "STORE_FTOBOOL", 6, 1, true, &def_boolean, &def_float, &def_boolean },
	{ "=", "STORE_BOOLTOF", 6, 1, true, &def_float, &def_boolean, &def_float },

	{ "=", "STOREP_F", 6, 0, true, &def_pointer, &def_float, &def_void },
	{ "=", "STOREP_V", 6, 0, true, &def_pointer, &def_vector, &def_void },
	{ "=", "STOREP_S", 6, 0, true, &def_pointer, &def_string, &def_void },
	{ "=", "STOREP_ENT", 6, 0, true, &def_pointer, &def_entity, &def_void },
	{ "=", "STOREP_FLD", 6, 0, true, &def_pointer, &def_field, &def_void },
	{ "=", "STOREP_BOOL", 6, 0, true, &def_pointer, &def_boolean, &def_void },
	{ "=", "STOREP_OBJ", 6, 0, true, &def_pointer, &def_object, &def_void },
	{ "=", "STOREP_OBJENT", 6, 0, true, &def_pointer, &def_object, &def_void },
	{ "=", "STOREP_A", 6, 0, true, &def_pointer, &def_array, &def_void },

	{ "<=>", "STOREP_FTOS", 6, 0, true, &def_pointer, &def_float, &def_string },
	{ "<=>", "STOREP_BTOS", 6, 0, true, &def_pointer, &def_boolean, &def_string },
	{ "<=>", "STOREP_VTOS", 6, 0, true, &def_pointer, &def_vector, &def_string },
	{ "<=>", "STOREP_FTOBOOL", 6, 0, true, &def_pointer, &def_float, &def_boolean },
	{ "<=>", "STOREP_BOOLTOF", 6, 0, true, &def_pointer, &def_boolean, &def_float },
	
	{ "<ROP>", "RPOP_F", 6, 0, true, &def_void, &def_void, &def_float },
	{ "<ROP>", "RPOP_V", 6, 0, true, &def_void, &def_void, &def_vector },
	{ "<ROP>", "RPOP_S", 6, 0, true, &def_void, &def_void, &def_string },
	{ "<ROP>", "RPOP_ENT", 6, 0, true, &def_void, &def_void, &def_entity },
	{ "<ROP>", "RPOP_OBJ", 6, 0, true, &def_void, &def_void, &def_object },
	{ "<ROP>", "RPOP_OBJENT", 6, 0, true, &def_void, &def_void, &def_object },
	{ "<ROP>", "RPOP_FTOB", 6, 0, true, &def_void, &def_void, &def_boolean },
	{ "<ROP>", "RPOP_A", 6, 0, true, &def_void, &def_void, &def_array },

	{ "*=", "UMUL_F", 6, 1, true, &def_float, &def_float, &def_void },
	{ "*=", "UMUL_V", 6, 1, true, &def_vector, &def_float, &def_void },
	{ "/=", "UDIV_F", 6, 1, true, &def_float, &def_float, &def_void },
	{ "/=", "UDIV_V", 6, 1, true, &def_vector, &def_float, &def_void },
	{ "%=", "UMOD_F", 6, 1, true, &def_float, &def_float, &def_void },
	{ "+=", "UADD_F", 6, 1, true, &def_float, &def_float, &def_void },
	{ "+=", "UADD_V", 6, 1, true, &def_vector, &def_vector, &def_void },
	{ "-=", "USUB_F", 6, 1, true, &def_float, &def_float, &def_void },
	{ "-=", "USUB_V", 6, 1, true, &def_vector, &def_vector, &def_void },
	{ "&=", "UAND_F", 6, 1, true, &def_float, &def_float, &def_void },
	{ "|=", "UOR_F", 6, 1, true, &def_float, &def_float, &def_void },
	
	{ "!", "NOT_BOOL", -1, 0, false, &def_boolean, &def_void, &def_float },
	{ "!", "NOT_F", -1, 0, false, &def_float, &def_void, &def_float },
	{ "!", "NOT_V", -1, 0, false, &def_vector, &def_void, &def_float },
	{ "!", "NOT_S", -1, 0, false, &def_vector, &def_void, &def_float },
	{ "!", "NOT_ENT", -1, 0, false, &def_entity, &def_void, &def_float },

	{ "<NEG_F>", "NEG_F", -1, 0, false, &def_float, &def_void, &def_float },
	{ "<NEG_V>", "NEG_V", -1, 0, false, &def_vector, &def_void, &def_vector },

	{ "int", "INT_F", -1, 0, false, &def_float, &def_void, &def_float },
	
	{ "<IF>", "IF", -1, 0, false, &def_float, &def_jumpoffset, &def_void },
	{ "<IFNOT>", "IFNOT", -1, 0, false, &def_float, &def_jumpoffset, &def_void },
	
	// calls returns REG_RETURN
	{ "<CALL>", "CALL", -1, 0, false, &def_function, &def_argsize, &def_void },
	{ "<THREAD>", "THREAD", -1, 0, false, &def_function, &def_argsize, &def_void },
	{ "<THREAD>", "OBJTHREAD", -1, 0, false, &def_function, &def_argsize, &def_void },
	
	{ "<PUSH>", "PUSH_F", -1, 0, false, &def_float, &def_float, &def_void },
	{ "<PUSH>", "PUSH_V", -1, 0, false, &def_vector, &def_vector, &def_void },
	{ "<PUSH>", "PUSH_S", -1, 0, false, &def_string, &def_string, &def_void },
	{ "<PUSH>", "PUSH_ENT", -1, 0, false, &def_entity, &def_entity, &def_void },
	{ "<PUSH>", "PUSH_OBJ", -1, 0, false, &def_object, &def_object, &def_void },
	{ "<PUSH>", "PUSH_OBJENT", -1, 0, false, &def_entity, &def_object, &def_void },
	{ "<PUSH>", "PUSH_A", -1, 0, false, &def_array, &def_array, &def_void },
	{ "<PUSH>", "PUSH_FTOS", -1, 0, false, &def_string, &def_float, &def_void },
	{ "<PUSH>", "PUSH_BTOF", -1, 0, false, &def_float, &def_boolean, &def_void },
	{ "<PUSH>", "PUSH_FTOB", -1, 0, false, &def_boolean, &def_float, &def_void },
	{ "<PUSH>", "PUSH_VTOS", -1, 0, false, &def_string, &def_vector, &def_void },
	{ "<PUSH>", "PUSH_BTOS", -1, 0, false, &def_string, &def_boolean, &def_void },
	
	{ "<GOTO>", "GOTO", -1, 0, false, &def_jumpoffset, &def_void, &def_void },
	
	{ "&&", "AND", 7, 0, false, &def_float, &def_float, &def_float },
	{ "&&", "AND_BOOLF", 7, 0, false, &def_boolean, &def_float, &def_float },
	{ "&&", "AND_FBOOL", 7, 0, false, &def_float, &def_boolean, &def_float },
	{ "&&", "AND_BOOLBOOL", 7, 0, false, &def_boolean, &def_boolean, &def_float },
	{ "||", "OR", 7, 0, false, &def_float, &def_float, &def_float },
	{ "||", "OR_BOOLF", 7, 0, false, &def_boolean, &def_float, &def_float },
	{ "||", "OR_FBOOL", 7, 0, false, &def_float, &def_boolean, &def_float },
	{ "||", "OR_BOOLBOOL", 7, 0, false, &def_boolean, &def_boolean, &def_float },
	
	{ "&", "BITAND", 3, 0, false, &def_float, &def_float, &def_float },
	{ "|", "BITOR", 3, 0, false, &def_float, &def_float, &def_float },

	{ "<BREAK>", "BREAK", -1, 0, false, &def_float, &def_void, &def_void },
	{ "<CONTINUE>", "CONTINUE", -1, 0, false, &def_float, &def_void, &def_void },

	//c4tnt: basic array operations
	{ "<INDEX>", "ARRINDEX_F", -1, 0, false, &def_array, &def_float, &def_float }, //Array index as R-value
	{ "<INDEX>", "ARRINDEX_V", -1, 0, false, &def_array, &def_float, &def_vector },
	{ "<INDEX>", "ARRINDEX_S", -1, 0, false, &def_array, &def_float, &def_string },
	{ "<INDEX>", "ARRINDEX_E", -1, 0, false, &def_array, &def_float, &def_entity },
	{ "<INDEX>", "ARRINDEX_BOOL", -1, 0, false, &def_array, &def_float, &def_boolean },
	{ "<INDEX>", "ARRINDEX_OBJ", -1, 0, false, &def_array, &def_float, &def_object },
	{ "<INDEX>", "ARRINDEX_PTR", -1, 0, false, &def_array, &def_float, &def_pointer }, //Array index as L-value

	{ "<MKARR>", "MKARR_F", -1, 0, false, &def_float, &def_void, &def_array },
	{ "<MKARR>", "MKARR_V", -1, 0, false, &def_float, &def_void, &def_array },
	{ "<MKARR>", "MKARR_S", -1, 0, false, &def_float, &def_void, &def_array },
	{ "<MKARR>", "MKARR_E", -1, 0, false, &def_float, &def_void, &def_array },
	{ "<MKARR>", "MKARR_BOOL", -1, 0, false, &def_float, &def_void, &def_array },
	{ "<MKARR>", "MKARR_OBJ", -1, 0, false, &def_float, &def_void, &def_array },
	{ "<RMARR>", "RMARR", -1, 0, false, &def_array, &def_void, &def_void },

	//c4tnt: object managerment operations
	{ "<ALLOCOBJ>", "ALLOC_OBJ", -1, 0, false, &def_argsize, &def_void, &def_object },
	{ "<DEALLOCOBJ>", "DEALLOC_OBJ", -1, 0, false, &def_object, &def_void, &def_void },
	{ NULL }
};

/*
================
idCompiler::idCompiler()
================
*/
idCompiler::idCompiler(idProgram* Target, idParser* parser, bool allowCVars) {

	// make sure we have the right # of opcodes in the table
	assert( ( sizeof( opcodes ) / sizeof( opcodes[ 0 ] ) ) == ( NUM_OPCODES + 1 ) );

	eof	= true;
	parserPtr = parser;

	callthread			= false;
	loopDepth			= 0;
	eof					= false;
	braceDepth			= 0;
	immediateType		= NULL;
	basetype			= NULL;
	currentLineNumber	= 0;
	currentFileNumber	= 0;
	errorCount			= 0;
	console				= false;
	scope				= &def_namespace;
	CompileTarget		= Target; //Set target
	registerCVars		= allowCVars;

	memset( &immediate, 0, sizeof( immediate ) );
//	memset( punctuationValid, 0, sizeof( punctuationValid ) );
	
//	for( ptr = punctuation; *ptr != NULL; ptr++ ) {
//		id = parserPtr->GetPunctuationId( *ptr );
//		if ( ( id >= 0 ) && ( id < 256 ) ) {
//			punctuationValid[ id ] = true;
//		}
//	}
}

/*
============
idCompiler::Error

Aborts the current file load
============
*/
void idCompiler::Error( const char *message, ... ) const {
	va_list	argptr;
	char	string[ 1024 ];

	va_start( argptr, message );
	vsprintf( string, message, argptr );
	va_end( argptr );

	throw idCompileError( string );
}

/*
============
idCompiler::Warning

Prints a warning about the current line
============
*/
void idCompiler::Warning( const char *message, ... ) const {
	va_list	argptr;
	char	string[ 1024 ];

	va_start( argptr, message );
	vsprintf( string, message, argptr );
	va_end( argptr );

	parserPtr->Warning( "%s", string );
}

/*
============
idCompiler::VirtualFunctionConstant

Creates a def for an index into a virtual function table
============
*/
ID_INLINE idVarDef *idCompiler::VirtualFunctionConstant( idVarDef *func ) {
	eval_t eval;

	memset( &eval, 0, sizeof( eval ) );
	eval._int = func->scope->TypeDef()->GetFunctionNumber( func->value.functionPtr );
	if ( eval._int < 0 ) {
		Error( "Function '%s' not found in scope '%s'", func->Name(), func->scope->Name() );
	}
    
	return CompileTarget->GetImmediate( &type_virtualfunction, &eval, "" );
}

/*
============
idCompiler::SizeConstant

Creates a def for a size constant
============
*/
ID_INLINE idVarDef *idCompiler::SizeConstant( int size ) {
	eval_t eval;

	memset( &eval, 0, sizeof( eval ) );
	eval._int = size;
	return CompileTarget->GetImmediate( &type_argsize, &eval, "" );
}

/*
============
idCompiler::SizeConstantWReturn

Creates a def for a size constant
============
*/
ID_INLINE idVarDef *idCompiler::SizeConstantWReturn( int size, int _return ) {
	eval_t eval;

	memset( &eval, 0, sizeof( eval ) );
	eval._int = ((_return & 0xFFFF)<<16)|(size & 0xFFFF);
	return CompileTarget->GetImmediate( &type_argsize, &eval, "" );
}

/*
============
idCompiler::JumpConstant

Creates a def for a jump constant
============
*/
ID_INLINE idVarDef *idCompiler::JumpConstant( int value ) {
	eval_t eval;

	memset( &eval, 0, sizeof( eval ) );
	eval._int = value;
	return CompileTarget->GetImmediate( &type_jumpoffset, &eval, "" );
}

/*
============
idCompiler::JumpDef

Creates a def for a relative jump from one code location to another
============
*/
ID_INLINE idVarDef *idCompiler::JumpDef( int jumpfrom, int jumpto ) {
	return JumpConstant( jumpto - jumpfrom );
}

/*
============
idCompiler::JumpTo

Creates a def for a relative jump from current code location
============
*/
ID_INLINE idVarDef *idCompiler::JumpTo( int jumpto ) {
	return JumpDef( CompileTarget->NumStatements(), jumpto );
}

/*
============
idCompiler::JumpFrom

Creates a def for a relative jump from code location to current code location
============
*/
ID_INLINE idVarDef *idCompiler::JumpFrom( int jumpfrom ) {
	return JumpDef( jumpfrom, CompileTarget->NumStatements() );
}

/*
============
idCompiler::Divide
============
*/
ID_INLINE float idCompiler::Divide( float numerator, float denominator ) {
	if ( denominator == 0 ) {
		Error( "Divide by zero" );
		return 0;
	}

	return numerator / denominator;
}

/*
============
idCompiler::OptimizeOpcode

try to optimize when the operator works on constants only
============
*/
idVarDef *idCompiler::OptimizeOpcode( const opcode_t *op, idVarDef *var_a, idVarDef *var_b ) {
	eval_t		c;
	idTypeDef	*type;

	if ( var_a && var_a->initialized != idVarDef::initializedConstant ) {
		return NULL;
	}
	if ( var_b && var_b->initialized != idVarDef::initializedConstant ) {
		return NULL;
	}

	idVec3 &vec_c = *reinterpret_cast<idVec3 *>( &c.vector[ 0 ] );

	memset( &c, 0, sizeof( c ) );
	switch( op - opcodes ) {
		case OP_ADD_F:		c._float = *var_a->value.floatPtr + *var_b->value.floatPtr; type = &type_float; break;
		case OP_ADD_V:		vec_c = *var_a->value.vectorPtr + *var_b->value.vectorPtr; type = &type_vector; break;
		case OP_SUB_F:		c._float = *var_a->value.floatPtr - *var_b->value.floatPtr; type = &type_float; break;
		case OP_SUB_V:		vec_c = *var_a->value.vectorPtr - *var_b->value.vectorPtr; type = &type_vector; break;
		case OP_MUL_F:		c._float = *var_a->value.floatPtr * *var_b->value.floatPtr; type = &type_float; break;
		case OP_MUL_V:		c._float = *var_a->value.vectorPtr * *var_b->value.vectorPtr; type = &type_float; break;
		case OP_MUL_FV:		vec_c = *var_b->value.vectorPtr * *var_a->value.floatPtr; type = &type_vector; break;
		case OP_MUL_VF:		vec_c = *var_a->value.vectorPtr * *var_b->value.floatPtr; type = &type_vector; break;
		case OP_DIV_F:		c._float = Divide( *var_a->value.floatPtr, *var_b->value.floatPtr ); type = &type_float; break;
		case OP_MOD_F:		c._float = (int)*var_a->value.floatPtr % (int)*var_b->value.floatPtr; type = &type_float; break;
		case OP_BITAND:		c._float = ( int )*var_a->value.floatPtr & ( int )*var_b->value.floatPtr; type = &type_float; break;
		case OP_BITOR:		c._float = ( int )*var_a->value.floatPtr | ( int )*var_b->value.floatPtr; type = &type_float; break;
		case OP_GE:			c._float = *var_a->value.floatPtr >= *var_b->value.floatPtr; type = &type_float; break;
		case OP_LE:			c._float = *var_a->value.floatPtr <= *var_b->value.floatPtr; type = &type_float; break;
		case OP_GT:			c._float = *var_a->value.floatPtr > *var_b->value.floatPtr; type = &type_float; break;
		case OP_LT:			c._float = *var_a->value.floatPtr < *var_b->value.floatPtr; type = &type_float; break;
		case OP_AND:		c._float = *var_a->value.floatPtr && *var_b->value.floatPtr; type = &type_float; break;
		case OP_OR:			c._float = *var_a->value.floatPtr || *var_b->value.floatPtr; type = &type_float; break;
		case OP_NOT_BOOL:	c._int = !*var_a->value.intPtr; type = &type_boolean; break;
		case OP_NOT_F:		c._float = !*var_a->value.floatPtr; type = &type_float; break;
		case OP_NOT_V:		c._float = !var_a->value.vectorPtr->x && !var_a->value.vectorPtr->y && !var_a->value.vectorPtr->z; type = &type_float; break;
		case OP_NEG_F:		c._float = -*var_a->value.floatPtr; type = &type_float; break;
		case OP_NEG_V:		vec_c = -*var_a->value.vectorPtr; type = &type_vector; break;
		case OP_INT_F:		c._float = ( int )*var_a->value.floatPtr; type = &type_float; break;
		case OP_EQ_F:		c._float = ( *var_a->value.floatPtr == *var_b->value.floatPtr ); type = &type_float; break;
		case OP_EQ_V:		c._float = var_a->value.vectorPtr->Compare( *var_b->value.vectorPtr ); type = &type_float; break;
		case OP_EQ_E:		c._float = ( *var_a->value.intPtr == *var_b->value.intPtr ); type = &type_float; break;
		case OP_NE_F:		c._float = ( *var_a->value.floatPtr != *var_b->value.floatPtr ); type = &type_float; break;
		case OP_NE_V:		c._float = !var_a->value.vectorPtr->Compare( *var_b->value.vectorPtr ); type = &type_float; break;
		case OP_NE_E:		c._float = ( *var_a->value.intPtr != *var_b->value.intPtr ); type = &type_float; break;
		case OP_UADD_F:		c._float = *var_b->value.floatPtr + *var_a->value.floatPtr; type = &type_float; break;
		case OP_USUB_F:		c._float = *var_b->value.floatPtr - *var_a->value.floatPtr; type = &type_float; break;
		case OP_UMUL_F:		c._float = *var_b->value.floatPtr * *var_a->value.floatPtr; type = &type_float; break;
		case OP_UDIV_F:		c._float = Divide( *var_b->value.floatPtr, *var_a->value.floatPtr ); type = &type_float; break;
		case OP_UMOD_F:		c._float = ( int ) *var_b->value.floatPtr % ( int )*var_a->value.floatPtr; type = &type_float; break;
		case OP_UOR_F:		c._float = ( int )*var_b->value.floatPtr | ( int )*var_a->value.floatPtr; type = &type_float; break;
		case OP_UAND_F: 	c._float = ( int )*var_b->value.floatPtr & ( int )*var_a->value.floatPtr; type = &type_float; break;
		case OP_UINC_F:		c._float = *var_a->value.floatPtr + 1; type = &type_float; break;
		case OP_UDEC_F:		c._float = *var_a->value.floatPtr - 1; type = &type_float; break;
		case OP_COMP_F:		c._float = ( float )~( int )*var_a->value.floatPtr; type = &type_float; break;
		default:			type = NULL; break;
	}

	if ( !type ) {
		return NULL;
	}

	if ( var_a ) {
		var_a->numUsers--;
		if ( var_a->numUsers <= 0 ) {
			CompileTarget->FreeDef( var_a, NULL );
		}
	}
	if ( var_b ) {
		var_b->numUsers--;
		if ( var_b->numUsers <= 0 ) {
			CompileTarget->FreeDef( var_b, NULL );
		}
	}

	return CompileTarget->GetImmediate( type, &c, "" );
}

/*
============
idCompiler::EmitOpNew

Emits allocation opcode, returning the var it places it's value in
============
*/
idVarDef *idCompiler::ParseNew( void ) {
	idVarDef	*var_c;
	idVarDef	*t;
	idTypeDef	*alloc;


	if (CheckToken( "[" )) { //make an array

		t = GetExpression( TOP_PRIORITY );
		ExpectToken( "]" );
		if (t->Type() != ev_float)
		{
			Error ("Invalid type '%s' in new[]", t->Name());
			return 0;
		}
		ExpectToken( "(" );
		alloc = ParseType(etl_varlevel);
		ExpectToken( ")" );

		idTypeDef	newarr( ev_array, NULL, "", 0, alloc, etl_varlevel); //c4tnt: array
		alloc = CompileTarget->GetType(newarr,true);
		var_c = EmitArrayCreation( alloc , t );
	}else{
		ExpectToken( "(" ) ;
		alloc = ParseType(etl_varlevel);
		ExpectToken( ")" );

		switch( alloc->Type() ) {
		case ev_object:
			if (alloc->Size() <= 0)
			{
				Error( "Zero-length object ('%s') in new()", alloc->Name());
			}
			var_c = EmitOpcode( OP_ALLOC_OBJ, SizeConstant(alloc->Size()), 0 );
			var_c->SetTypeDef( alloc );
			break;
		default :
			Error( "Invalid type '%s' in new()", alloc->Name());
			break;
		}
	}

	return var_c;
}

/*
============
idCompiler::EmitOpcode

Emits a primitive statement, returning the var it places it's value in
============
*/
idVarDef *idCompiler::EmitOpcode( const opcode_t *op, idVarDef *var_a, idVarDef *var_b, idTypeDef *type_c ) {
	statement_t	*statement;
	idVarDef	*var_c;
	const char  *err_side;
	var_c = OptimizeOpcode( op, var_a, var_b );
	if ( var_c ) {
		return var_c;
	}

	err_side = NULL;
	if ( op->constMask & 1 ) {
		if ( var_b == NULL || var_b->initialized == idVarDef::initializedConstant ) {
			if ( op->constMask & 4 ) {
				err_side = "Right";
			} else {
				err_side = "Left";
			}
		}
	}

	if ( op->constMask & 2 ) {
		if ( var_a == NULL || var_a->initialized == idVarDef::initializedConstant ) {
			if ( op->constMask & 4 ) {
				err_side = "Left";
			} else {
				err_side = "Right";
			}
		}
	}
	
	if ( err_side ) {
		Error( "%s variable on op %s is NULL or constant, expecting variable", err_side, op->name );
	}

	if ( var_a && !strcmp( var_a->Name(), RESULT_STRING ) ) {
		var_a->numUsers++;
	}
	if ( var_b && !strcmp( var_b->Name(), RESULT_STRING ) ) {
		var_b->numUsers++;
	}
	
	statement = CompileTarget->AllocStatement();
	statement->linenumber	= currentLineNumber;
	statement->file 		= currentFileNumber;
	
	if ( op->rightAssociative ) {
		if ( ( op->type_c == op->type_a ) || ( op->type_c == &def_void ) ) {
			var_c = NULL;
		} else {
			if ( type_c ) {
				if ( type_c->Type() != op->type_c->Type() ) {
					Error( "---STOP:Field and opcode type mismatch on op: %s---", op->name );
				}
				var_c = CompileTarget->FindFreeResultDef( type_c, RESULT_STRING, scope, var_a, var_b );
			} else {
				var_c = CompileTarget->FindFreeResultDef( op->type_c->TypeDef(), RESULT_STRING, scope, var_a, var_b );
			}
			// set user count back to 1, a result def needs to be used twice before it can be reused
			var_c->numUsers = 1;
		}
	} else if ( op->type_c == &def_void ) {
		// ifs, gotos, and assignments don't need vars allocated
		var_c = NULL;
	} else {
		// allocate result space
		// try to reuse result defs as much as possible
		if ( type_c ) {
			if ( type_c->Type() != op->type_c->Type() ) {
				Error( "---STOP:Field and opcode type mismatch on op: %s---", op->name );
			}
			var_c = CompileTarget->FindFreeResultDef( type_c, RESULT_STRING, scope, var_a, var_b );
		} else {
			var_c = CompileTarget->FindFreeResultDef( op->type_c->TypeDef(), RESULT_STRING, scope, var_a, var_b );
		}
		// set user count back to 1, a result def needs to be used twice before it can be reused
		var_c->numUsers = 1;
	}

	statement->op	= op - opcodes;
	statement->a	= var_a;
	statement->b	= var_b;
	statement->c	= var_c;
	
	if ( op->rightAssociative ) {
		if ( !var_c ) return var_a;
	}

	return var_c;
}

/*
============
idCompiler::EmitOpcode3

Emits a primitive statement, returning the var it places it's value in
============
*/
void idCompiler::EmitOpcode3( const opcode_t *op, idVarDef *var_a, idVarDef *var_b, idVarDef *var_c ) {
	statement_t	*statement;
	const char  *err_side;

	err_side = NULL;
	if ( op->constMask & 1 ) {
		if ( var_b == NULL || var_b->initialized == idVarDef::initializedConstant ) {
			if ( op->constMask & 4 ) {
				err_side = "Right";
			} else {
				err_side = "Left";
			}
		}
	}

	if ( op->constMask & 2 ) {
		if ( var_a == NULL || var_a->initialized == idVarDef::initializedConstant ) {
			if ( op->constMask & 4 ) {
				err_side = "Left";
			} else {
				err_side = "Right";
			}
		}
	}
	
	if ( err_side ) {
		Error( "%s variable on op %s is NULL or constant, expecting variable", err_side, op->name );
	}

	if ( var_a && !strcmp( var_a->Name(), RESULT_STRING ) ) {
		var_a->numUsers++;
	}
	if ( var_b && !strcmp( var_b->Name(), RESULT_STRING ) ) {
		var_b->numUsers++;
	}
	
	statement = CompileTarget->AllocStatement();
	statement->linenumber	= currentLineNumber;
	statement->file 		= currentFileNumber;
	statement->op	= op - opcodes;
	statement->a	= var_a;
	statement->b	= var_b;
	statement->c	= var_c;
}

/*
============
idCompiler::EmitOpcode

Emits a primitive statement, returning the var it places it's value in
============
*/
ID_INLINE idVarDef *idCompiler::EmitOpcode( int op, idVarDef *var_a, idVarDef *var_b, idTypeDef *type_c ) {
	return EmitOpcode( &opcodes[ op ], var_a, var_b, type_c );
}

/*
============
idCompiler::EmitOpcode3

Emits a primitive statement, returning the var it places it's value in
============
*/
ID_INLINE void idCompiler::EmitOpcode3( int op, idVarDef *var_a, idVarDef *var_b, idVarDef *var_c ) {
	EmitOpcode3( &opcodes[ op ], var_a, var_b, var_c );
}

/*
============
idCompiler::EmitPush

Emits an opcode to push the variable onto the stack.
============
*/
bool idCompiler::EmitPush( idVarDef *expression, const idTypeDef *funcArg ) {
	opcode_t *op;
	opcode_t *out;

	out = NULL;
	for( op = &opcodes[ OP_PUSH_F ]; op->name && !strcmp( op->name, "<PUSH>" ); op++ ) {
		if ( ( funcArg->Type() == op->type_a->Type() ) && ( expression->Type() == op->type_b->Type() ) ) {
			out = op;
			break;
		}
	}

	if ( !out ) {
		if ( ( expression->TypeDef() != funcArg ) && !expression->TypeDef()->Inherits( funcArg ) ) {
			return false;
		}

		out = &opcodes[ OP_PUSH_ENT ];
	}

	EmitOpcode( out, expression, 0 );

	return true;
}

/*
==============
idCompiler::NextToken

Sets token, immediateType, and possibly immediate
==============
*/
void idCompiler::NextToken( void ) {
	int i;

	// reset our type
	immediateType = NULL;
	memset( &immediate, 0, sizeof( immediate ) );

	// Save the token's line number and filename since when we emit opcodes the current 
	// token is always the next one to be read 
	currentLineNumber = token.line;
	currentFileNumber = CompileTarget->GetFilenum( parserPtr->GetFileName() );

	if ( !parserPtr->ReadToken( &token ) ) {
		eof = true;
		return;
	}

	if ( currentFileNumber != CompileTarget->GetFilenum( parserPtr->GetFileName() ) ) {
		if ( ( braceDepth > 0 ) && ( token != "}" ) ) {
			// missing a closing brace.  try to give as much info as possible.
			if ( scope->Type() == ev_function ) {
				Error( "Unexpected end of file inside function '%s'.  Missing closing braces.", scope->Name() );
			} else if ( scope->Type() == ev_object ) {
				Error( "Unexpected end of file inside object '%s'.  Missing closing braces.", scope->Name() );
			} else if ( scope->Type() == ev_namespace ) {
				Error( "Unexpected end of file inside namespace '%s'.  Missing closing braces.", scope->Name() );
			} else {
				Error( "Unexpected end of file inside braced section" );
			}
		}
	}

	switch( token.type ) {
	case TT_STRING:
		// handle quoted strings as a unit
		immediateType = &type_string;
		return;

	case TT_LITERAL: {
		if ( token.Length() == 1 ) {
			// handle quoted charcode unit
			immediateType = &type_float;
			immediate._float = (float)(unsigned char)token[0];
		} else {
			// handle quoted vectors as a unit
			idLexer lex( token, token.Length(), parserPtr->GetFileName(), LEXFL_NOERRORS );
			idToken token2;
			immediateType = &type_vector;
			for( i = 0; i < 3; i++ ) {
				if ( !lex.ReadToken( &token2 ) ) {
					Error( "Couldn't read vector. '%s' is not in the form of 'x y z'", token.c_str() );
				}
				if ( token2.type == TT_PUNCTUATION && token2 == "-" ) {
					if ( !lex.CheckTokenType( TT_NUMBER, 0, &token2 ) ) {
						Error( "expected a number following '-' but found '%s' in vector '%s'", token2.c_str(), token.c_str() );
					}
					immediate.vector[ i ] = -token2.GetFloatValue();
				} else if ( token2.type == TT_NUMBER ) {
					immediate.vector[ i ] = token2.GetFloatValue();
				} else {
					Error( "vector '%s' is not in the form of 'x y z'.  expected float value, found '%s'", token.c_str(), token2.c_str() );
				}
			}
		}
		return;
	}

	case TT_NUMBER:
		immediateType = &type_float;
		immediate._float = token.GetFloatValue();
		return;

	case TT_PUNCTUATION:
		// entity names
		if ( token == "$" ) {
			immediateType = &type_entity;
			parserPtr->ReadToken( &token );
			return;
		}

		if ( token == "{" ) {
			braceDepth++;
			return;
		}

		if ( token == "}" ) {
			braceDepth--;
			return;
		}

//		if ( punctuationValid[ token.subtype ] ) {
			return;
//		}

//		Error( "Unknown punctuation (compiler)'%s'", token.c_str() );
		break;

	case TT_NAME:
		return;

	default:
		Error( "Unknown token '%s'", token.c_str() );
	}
}

/*
=============
idCompiler::ExpectToken

Issues an Error if the current token isn't equal to string
Gets the next token
=============
*/
void idCompiler::ExpectToken( const char *string ) {
	if ( token != string ) {
		Error( "expected '%s', found '%s'", string, token.c_str() );
	}

	NextToken();
}

/*
=============
idCompiler::CheckToken

Returns true and gets the next token if the current token equals string
Returns false and does nothing otherwise
=============
*/
bool idCompiler::CheckToken( const char *string ) {
	if ( token != string ) {
		return false;
	}
		
	NextToken();
	
	return true;
}

/*
============
idCompiler::ParseName

Checks to see if the current token is a valid name
============
*/
void idCompiler::ParseName( idStr &name ) {
	if ( token.type != TT_NAME ) {
		Error( "'%s' is not a name", token.c_str() );
	}

	name = token;
	NextToken();
}

/*
============
idCompiler::SkipOutOfFunction

For error recovery, pops out of nested braces
============
*/
void idCompiler::SkipOutOfFunction( void ) {
	while( braceDepth ) {
		parserPtr->SkipBracedSection( false );
		braceDepth--;
	}
	NextToken();
}

/*
============
idCompiler::SkipToSemicolon

For error recovery
============
*/
void idCompiler::SkipToSemicolon( void ) {
	do {
		if ( CheckToken( ";" ) ) {
			return;
		}

		NextToken();
	} while( !eof );
}

/*
============
idCompiler::CheckType

Parses a variable type, including functions types
============
*/
idTypeDef *idCompiler::CheckType( etypelayer_t vlay ) {
	idTypeDef *type;
	
	if ( token == "float" ) {
		type = &type_float;
	} else if ( token == "vector" ) {
		type = &type_vector;
	} else if ( token == "entity" ) {
		type = &type_entity;
	} else if ( token == "string" ) {
		type = &type_string;
	} else if ( token == "void" ) {
		type = &type_void;
	} else if ( token == "object" ) {
		type = &type_object;
	} else if ( token == "list" ) {
		type = &type_list;
	} else if ( token == "array" ) {
		type = &type_array;
	} else if ( token == "boolean" ) {
		type = &type_boolean;
	} else if ( token == "namespace" ) {
		type = &type_namespace;
	} else if ( token == "scriptEvent" ) {
		type = &type_scriptevent;
	} else {
		type = CompileTarget->FindType( token.c_str() );
		if ( type && !type->Inherits( &type_object ) ) {
			type = NULL;
		}
	}
	if ( ( type != NULL ) && ( !type->CheckLayer(vlay) ) ) {
		Error( "Layered type \"%s\" (%s) can't be used at %s", type->Name(),etl_names[type->GetLayer()], etl_names[vlay]);
	}

	return type;
}

/*
============
idCompiler::ParseType

Parses a variable type, including functions types
============
*/
idTypeDef *idCompiler::ParseType( etypelayer_t layer ) {
	idTypeDef *type;
	idTypeDef *subtype;
	
	type = CheckType(layer);
	if ( !type ) {
		Error( "\"%s\" is not a type", token.c_str() );
	}

	if ( ( type == &type_scriptevent ) && ( scope != &def_namespace ) ) {
		Error( "scriptEvents can only defined in the global namespace" );
	}

	if ( ( type == &type_namespace ) && ( scope->Type() != ev_namespace ) ) {
		Error( "A namespace may only be defined globally, or within another namespace" );
	}

	if ( ( type == &type_array ) ) //c4tnt: Array type parsing
	{

		NextToken();
		if (!CheckToken( "<" )) {
			Error( "Array without a type. Use array<type> construction" );
		}
		subtype = ParseType(etl_varlevel);
		ExpectToken( ">" );

		idTypeDef	newarr( ev_array, NULL, "array", type->Size(), subtype, etl_varlevel); //c4tnt: array
		type = CompileTarget->GetType(newarr,true); 

		return type;
	}

	NextToken();
	
	return type;
}

/*
============
idCompiler::ParseImmediate

Looks for a preexisting constant
============
*/
idVarDef *idCompiler::ParseImmediate( void ) {
	idVarDef *def;

	def = CompileTarget->GetImmediate( immediateType, &immediate, token.c_str() );
	NextToken();

	return def;
}

/*
============
idCompiler::EmitArrayCreation
============
*/
idVarDef *idCompiler::EmitArrayCreation( idVarDef *arrayDef ) {
	idVarDef		*e;

	//emit MKARR for arrays
	if ( CheckToken( "[" ) ) {
		e = GetExpression( TOP_PRIORITY );
		ExpectToken( "]" );
		return EmitArrayCreation( arrayDef->TypeDef(), e );
	}else{
		return NULL; 
	}
}

/*
============
idCompiler::EmitArrayCreation (counted)
============
*/
idVarDef *idCompiler::EmitArrayCreation( idTypeDef *arrayTDef, idVarDef *num ) {
	int resultOp;
	idVarDef		*r;

	if (arrayTDef->Type() != ev_array)
	{
		Error ("'%s' in not an array",arrayTDef->Name());
		return 0;
	}

	switch( arrayTDef->ListType()->Type()) {
	case ev_boolean :
		resultOp = OP_MKARR_BOOL;
		break;
	case ev_float :
		resultOp = OP_MKARR_F;
		break;
	case ev_vector :
		resultOp = OP_MKARR_V;
		break;
	case ev_entity :
		resultOp = OP_MKARR_ENT;
		break;
	case ev_object :
		resultOp = OP_MKARR_OBJ;
		break;
	case ev_string :
		resultOp = OP_MKARR_S;
		break;
	default :
		Error( "Invalid type of array '%s'", arrayTDef->Name() );
		// shut up compiler
		resultOp = OP_MKARR_F;
		break;
	}
	r = EmitOpcode( resultOp, num, 0 );
	r->SetTypeDef(arrayTDef); //set subtype for an result array
	return r;
}
/*
============
idCompiler::EmitFunctionParms
============
*/
idVarDef *idCompiler::EmitFunctionParms( int op, idVarDef *func, int startarg, int startsize, idVarDef *object ) {
	idVarDef		*e;
	const idTypeDef	*type;
	const idTypeDef	*funcArg;
	idVarDef		*returnDef;
	idTypeDef		*returnType;
	int 			arg;
	int 			size;
	int				resultOp;
	idVarDef		*resultDef = NULL;

	type = func->TypeDef();
	if ( func->Type() != ev_function ) {
		Error( "'%s' is not a function", func->Name() );
	}

	returnType = func->TypeDef()->ReturnType();

	// copy the parameters to the global parameter variables
	arg = startarg;
	size = startsize;
	if ( !CheckToken( ")" ) ) {
		do {
			if ( arg >= type->NumParameters() ) {
				Error( "too many parameters" );
			}

			e = GetExpression( TOP_PRIORITY );

			funcArg = type->GetParmType( arg );
			if ( !EmitPush( e, funcArg ) ) {
				Error( "type mismatch on parm %i of call to '%s': %i vs %i ", arg + 1, func->Name(), funcArg->Type(), e->Type() );
			}

			if ( funcArg->Type() == ev_object ) {
				size += type_object.Size();
			} else {
				size += funcArg->Size();
			}

			arg++;
		} while( CheckToken( "," ) );
	
		ExpectToken( ")" );
	}

	if ( arg < type->NumParameters() ) {
		Error( "too few parameters for function '%s'", func->Name() );
	}

	if ( op == OP_CALL ) {
		EmitOpcode( op, func, 0 );
	} else if ( ( op == OP_OBJECTCALL ) || ( op == OP_OBJTHREAD ) ) {
		//c4tnt:changed
		EmitOpcode3( op, object, VirtualFunctionConstant( func ), SizeConstantWReturn( func->value.functionPtr->parmTotal, returnType->Size() ) );
	} else {
		EmitOpcode( op, func, SizeConstant( size ) );
	}

	return EmitROP( returnType, func->Name() );
}

/*
============
idCompiler::EmitROP
============
*/
idVarDef *idCompiler::EmitROP( idTypeDef *returnType, const char* _fname ) {

	int				resultOp;

	if (!returnType)
		return &def_void; //VOID;

	switch( returnType->Type() ) {
	case ev_void :
		return &def_void; //VOID;
		break;

	case ev_string :
		resultOp = OP_RPOP_S;
		break;

	case ev_boolean :
		resultOp = OP_RPOP_FTOB;
		break;

	case ev_float :
		resultOp = OP_RPOP_F;
		break;

	case ev_vector :
		resultOp = OP_RPOP_V;
		break;

	case ev_entity :
		resultOp = OP_RPOP_ENT;
		break;

	case ev_object :
		resultOp = OP_RPOP_OBJ;
		break;

	case ev_array :
		resultOp = OP_RPOP_A;
		break;

	default :
		Error( "Invalid return type for function '%s'", _fname );
		break;
	}

	return EmitOpcode( resultOp, 0, 0 );
}

/*
============
idCompiler::EmitCAST
============
*/
idVarDef *idCompiler::EmitCAST( idTypeDef *returnType, idVarDef* inVarDef, const char* _fname ) {

	int				resultOp;
	idVarDef		*resultDef = NULL;
	etype_t 	type_a;
	etype_t 	type_b;
	opcode_t	*op;

	if (!returnType)
		return &def_void; //VOID;

	type_a = inVarDef->Type();
	type_b = returnType->Type();

	if ( TypeMatches( type_a, type_b ) ) {
		return inVarDef; //No type casting
	}

	// allocate result space
	// try to reuse result defs as much as possible
	resultDef = CompileTarget->FindFreeResultDef( returnType, RESULT_STRING, scope, inVarDef, NULL );
	// set user count back to 0, a result def needs to be used twice before it can be reused
	resultDef->numUsers = 0;

	if ( !resultDef ) {
		Error( "Problems while allocating result def for %s", _fname );
	}

	for( op = opcodes; op->name; op++ ) {
		if ( !strcmp( op->name, "=" ) ) {
			break;
		}
	}

	assert( op->name );

	while( !TypeMatches( type_a, op->type_a->Type() ) || !TypeMatches( type_b, op->type_b->Type() ) ) {
		op++;
		if ( !op->name || strcmp( op->name, "=" ) ) {
			Error( "type mismatch for %s", _fname );
		}
	}

	EmitOpcode( op, inVarDef, resultDef );
	return resultDef;
}

/*
============
idCompiler::ParseFunctionCall
============
*/
idVarDef *idCompiler::ParseArrayListInd( idVarDef *arrayDef ) {

	idVarDef *indexDef;
	int resultOp;

	assert( arrayDef );

	if ( arrayDef->Type() != ev_array ) {
		Error( "'%s' is not an array", arrayDef->Name() );
	}

	switch( arrayDef->TypeDef()->ListType()->Type()) {

	case ev_boolean :
		resultOp = OP_INDEX_BOOL;
		break;

	case ev_float :
		resultOp = OP_INDEX_F;
		break;

	case ev_vector :
		resultOp = OP_INDEX_V;
		break;

	case ev_entity :
		resultOp = OP_INDEX_ENT;
		break;

	case ev_object :
		resultOp = OP_INDEX_OBJ;
		break;

	case ev_string :
		resultOp = OP_INDEX_S;
		break;

	default :
		Error( "Invalid type of array '%s'", arrayDef->Name() );
		// shut up compiler
		resultOp = OP_INDEX_F;
		break;
	}
	
	indexDef = GetExpression( TOP_PRIORITY );
	if ( indexDef->Type() != ev_float ) {
		Error( "'%s' index type mismatch", indexDef->Name() );
	}
	
	ExpectToken( "]" );
	return EmitOpcode(resultOp, arrayDef, indexDef );
}
/*
============
idCompiler::ParseFunctionCall
============
*/
idVarDef *idCompiler::ParseFunctionCall( idVarDef *funcDef ) {
	assert( funcDef );

	if ( funcDef->Type() != ev_function ) {
		Error( "'%s' is not a function", funcDef->Name() );
	}

	if ( funcDef->initialized == idVarDef::uninitialized ) {
		Error( "Function '%s' has not been defined yet", funcDef->GlobalName() );
	}

	assert( funcDef->value.functionPtr );
	if ( callthread ) {
		if ( ( funcDef->initialized != idVarDef::uninitialized ) && funcDef->value.functionPtr->eventdef ) {
			Error( "Built-in functions cannot be called as threads" );
		}
		callthread = false;
		return EmitFunctionParms( OP_THREAD, funcDef, 0, 0, NULL );
	} else {
		if ( ( funcDef->initialized != idVarDef::uninitialized ) && funcDef->value.functionPtr->eventdef ) {
			if ( ( scope->Type() != ev_namespace ) && ( scope->scope->Type() == ev_object ) ) {
				// get the local object pointer
				idVarDef *thisdef = CompileTarget->GetDef( scope->scope->TypeDef(), "self", scope );
				if ( !thisdef ) {
					Error( "No 'self' within scope" );
				}

				return ParseVPCall( thisdef, funcDef );
			} else {
				//c4tnt fixup. Now all sys functions can be called directly by name, but only from topmost scope 
				//Error( "Built-in functions cannot be called without an object" ); -off

				return ParseSysObjectCall( funcDef );
			}
		}

		return EmitFunctionParms( OP_CALL, funcDef, 0, 0, NULL );
	}
}

/*
============
idCompiler::ParseObjectCall
============
*/
idVarDef *idCompiler::ParseObjectCall( idVarDef *object, idVarDef *func ) {
	EmitPush( object, object->TypeDef() );
	if ( callthread ) {
		callthread = false;
		return EmitFunctionParms( OP_OBJTHREAD, func, 1, type_object.Size(), object );
	} else {
		return EmitFunctionParms( OP_OBJECTCALL, func, 1, 0, object );
	}
}

/*
============
idCompiler::ParseEventCall
============
*/
idVarDef *idCompiler::ParseEventCall( idVarDef *object, idVarDef *funcDef ) {
int AcqOpcode;

	if ( callthread ) {
		Error( "Cannot call built-in functions as a thread" );
	}

	if ( funcDef->Type() != ev_function ) {
		Error( "'%s' is not a function", funcDef->Name() );
	}

	if ( !funcDef->value.functionPtr->eventdef ) {
		Error( "\"%s\" cannot be called with object notation", funcDef->Name() );
	}

//	if ( object->Type() == ev_object ) {
//		EmitPush( object, &type_entity );
//	} else {
	EmitPush( object, object->TypeDef() );
	return EmitFunctionParms( OP_EVENTCALL, funcDef, 0, type_object.Size(), NULL );
}

/*
============
idCompiler::ParseVPCall
============
*/
idVarDef *idCompiler::ParseVPCall( idVarDef *object, idVarDef *funcDef ) {
int AcqOpcode;

	if ( callthread ) {
		Error( "Cannot call built-in functions as a thread" );
	}

	if ( funcDef->Type() != ev_function ) {
		Error( "'%s' is not a function", funcDef->Name() );
	}

	if ( !funcDef->value.functionPtr->eventdef ) {
		Error( "\"%s\" cannot be called with object notation", funcDef->Name() );
	}

//	if ( object->Type() == ev_object ) {
//		EmitPush( object, &type_entity );
//	} else {
	EmitPush( object, object->TypeDef() );
	return EmitFunctionParms( OP_VPCALL, funcDef, 0, type_object.Size(), NULL );
}

/*
============
idCompiler::ParseSysObjectCall
============
*/
idVarDef *idCompiler::ParseSysObjectCall( idVarDef *funcDef ) {
int AcqOpcode;

	if ( callthread ) {
		Error( "Cannot call built-in functions as a thread" );
	}

	if ( funcDef->Type() != ev_function ) {
		Error( "'%s' is not a function", funcDef->Name() );
	}

	if ( !funcDef->value.functionPtr->eventdef ) {
		Error( "\"%s\" cannot be called with object notation", funcDef->Name() );
	}

	if ( !idThread::Type.RespondsTo( *funcDef->value.functionPtr->eventdef ) ) {
		Error( "\"%s\" is not callable as a 'sys' function", funcDef->Name() );
	}

	return EmitFunctionParms( OP_SYSCALL, funcDef, 0, 0, NULL );
}

/*
============
idCompiler::LookupDef
============
*/
idVarDef *idCompiler::LookupDef( const char *name, const idVarDef *baseobj ) {
	idVarDef	*def;
	idVarDef	*field;
	etype_t		type_b;
	etype_t		type_c;
	etype_t		type_c_ptr;
	opcode_t	*op;
	bool		ExportAsL;

	// check if we're accessing a field
	if ( baseobj && ( baseobj->Type() == ev_object ) ) {
		const idVarDef *tdef;

		def = NULL;
		for( tdef = baseobj; tdef != &def_object; tdef = tdef->TypeDef()->SuperClass()->def ) {
			def = CompileTarget->GetDef( NULL, name, tdef );
			if ( def ) {
				break;
			}
		}
	} else {
		// first look through the defs in our scope
		def = CompileTarget->GetDef( NULL, name, scope );
		if ( !def ) {
			// if we're in a member function, check types local to the object
			if ( ( scope->Type() != ev_namespace ) && ( scope->scope->Type() == ev_object ) ) {
				// get the local object pointer
				idVarDef *thisdef = CompileTarget->GetDef( scope->scope->TypeDef(), "self", scope );

				field = LookupDef( name, scope->scope->TypeDef()->def );
				if ( !field ) {
					Error( "Unknown value \"%s\"", name );
				}

				// type check
				type_b = field->Type();
				if ( field->Type() == ev_function ) {
					type_c = field->TypeDef()->ReturnType()->Type();
				} else {
					type_c = field->TypeDef()->FieldType()->Type();	// field access gets type from field
	                if ( CheckToken( "++" ) ) {
						if ( type_c != ev_float ) {
							Error( "Invalid type for ++" );
						}
						def = EmitOpcode( OP_UINCP_F, thisdef, field );
						return def;
					} else if ( CheckToken( "--" ) ) {
						if ( type_c != ev_float ) {
							Error( "Invalid type for --" );
						}
						def = EmitOpcode( OP_UDECP_F, thisdef, field );
						return def;
					}
				}

//				ExportAsL = (ord == lValue && type_c != ev_entity && type_c != ev_object && field->Type() != ev_function ); //c4tnt: we don't know, will be object used as lValue or not

//				if ( ExportAsL ) { 
//					op = &opcodes[ OP_ADDRESS_O ];
//					type_c_ptr = ev_pointer;
//				}else{
					op = &opcodes[ OP_INDIRECT_F ];
					type_c_ptr = type_c;
//				}

				while( ( op->type_a->Type() != ev_object ) 
					|| ( type_b != op->type_b->Type() ) || ( type_c_ptr != op->type_c->Type() ) ) {
					if ( ( op->priority == FUNCTION_PRIORITY ) && ( op->type_a->Type() == ev_object ) && ( op->type_c->Type() == ev_void ) && 
						( type_c != op->type_c->Type() ) ) {
						// catches object calls that return a value
						break;
					}
					op++;
					if ( !op->name || strcmp( op->name, "." ) ) {
						Error( "no valid opcode to access type '%s'", field->TypeDef()->SuperClass()->Name() );
					}
				}

				if ( ( op - opcodes ) == OP_OBJECTCALL ) {
					ExpectToken( "(" );
					def = ParseObjectCall( thisdef, field );
				} else {
					// emit the conversion opcode
					def = EmitOpcode( op, thisdef, field );

					// field access gets type from field
//					if ( ExportAsL ) {
//						idTypeDef ptr(type_pointer);
//						ptr.SetPointerType( field->TypeDef()->FieldType() );
						
//						def->SetTypeDef( CompileTarget->GetType(ptr,true));
//					}else{
						def->SetTypeDef( field->TypeDef()->FieldType() );
//					}
				}
			}
		}
	}

	return def;
}

/*
============
idCompiler::ParseValue

Returns the def for the current token
============
*/
idVarDef *idCompiler::ParseValue( ) {
	idVarDef	*def;
	idVarDef	*namespaceDef;
	idStr		name;
	
	if ( immediateType == &type_entity ) {
		// if an immediate entity ($-prefaced name) then create or lookup a def for it.
		// when entities are spawned, they'll lookup the def and point it to them.
		def = CompileTarget->GetDef( &type_entity, "$" + token, &def_namespace );
		if ( !def ) {
			def = CompileTarget->AllocDef( &type_entity, "$" + token, &def_namespace, true );
		}
		NextToken();
		return def;
	} else if ( immediateType ) {
		// if the token is an immediate, allocate a constant for it
		return ParseImmediate();
	}

	ParseName( name );
	def = LookupDef( name, basetype );
	if ( !def ) {
		if ( basetype ) {
			Error( "%s is not a member of %s", name.c_str(), basetype->TypeDef()->Name() );
		} else {
			Error( "Unknown value \"%s\"", name.c_str() );
		}
	// if namespace, then look up the variable in that namespace
	} else if ( def->Type() == ev_namespace ) {
		while( def->Type() == ev_namespace ) {
			ExpectToken( "::" );
			ParseName( name );
			namespaceDef = def;
			def = CompileTarget->GetDef( NULL, name, namespaceDef );
			if ( !def ) {
				Error( "Unknown value \"%s::%s\"", namespaceDef->GlobalName(), name.c_str() );
			}
		}
		//def = LookupDef( name, basetype );
	}

	return def;
}

/*
============
idCompiler::GetTerm
============
*/
idVarDef *idCompiler::GetTerm( ) {
	idVarDef	*e;
	int 		op;
	
	if ( !immediateType && CheckToken( "~" ) ) {
		e = GetExpression( TILDE_PRIORITY );
		switch( e->Type() ) {
		case ev_float :
			op = OP_COMP_F;
			break;

		default :
			Error( "type mismatch for ~" );

			// shut up compiler
			op = OP_COMP_F;
			break;
		}

		return EmitOpcode( op, e, 0 );
	}

	if ( !immediateType && CheckToken( "!" ) ) {
		e = GetExpression( NOT_PRIORITY );
		switch( e->Type() ) {
		case ev_boolean :
			op = OP_NOT_BOOL;
			break;

		case ev_float :
			op = OP_NOT_F;
			break;

		case ev_string :
			op = OP_NOT_S;
			break;

		case ev_vector :
			op = OP_NOT_V;
			break;

		case ev_entity :
			op = OP_NOT_ENT;
			break;

		case ev_function :
			Error( "Invalid type for !" );

			// shut up compiler
			op = OP_NOT_F;
			break;

		case ev_object :
			op = OP_NOT_ENT;
			break;

		default :
			Error( "type mismatch for !" );

			// shut up compiler
			op = OP_NOT_F;
			break;
		}

		return EmitOpcode( op, e, 0 );
	}

	// check for negation operator
	if ( !immediateType && CheckToken( "-" ) ) {
		// constants are directly negated without an instruction
		if ( immediateType == &type_float ) {
			immediate._float = -immediate._float;
			return ParseImmediate();
		} else if ( immediateType == &type_vector ) {
			immediate.vector[0] = -immediate.vector[0];
			immediate.vector[1] = -immediate.vector[1];
			immediate.vector[2] = -immediate.vector[2];
			return ParseImmediate();
		} else {
			e = GetExpression( NOT_PRIORITY );
			switch( e->Type() ) {
			case ev_float :
				op = OP_NEG_F;
				break;

			case ev_vector :
				op = OP_NEG_V;
				break;
			default :
				Error( "type mismatch for -" );

				// shut up compiler
				op = OP_NEG_F;
				break;
			}
			return EmitOpcode( &opcodes[ op ], e, 0 );
		}
	}
	
	if ( CheckToken( "int" ) ) {

		ExpectToken( "(" );

		e = GetExpression( INT_PRIORITY );
		if ( e->Type() != ev_float ) {
			Error( "type mismatch for int()" );
		}

		ExpectToken( ")" );

		return EmitOpcode( OP_INT_F, e, 0 );
	}

// c4tnt: array, list and obect creation
	if ( CheckToken( "new" ) ) {
		return ParseNew();
	}
// c4tnt--end

	if ( CheckToken( "thread" ) ) {
		callthread = true;
		e = GetExpression( FUNCTION_PRIORITY );

		if ( callthread ) {
			Error( "Invalid thread call" );
		}

		// threads return the thread number
		return EmitROP( &type_float, "thread" );
	}
	
	if ( !immediateType && CheckToken( "(" ) ) {
		e = GetExpression( TOP_PRIORITY );
		ExpectToken( ")" );

		return e;
	}
	
	return ParseValue( );
}

/*
==============
idCompiler::TypeMatches
==============
*/
bool idCompiler::TypeMatches( etype_t type1, etype_t type2 ) const {
	if ( type1 == type2 ) {
		return true;
	}

	//if ( ( type1 == ev_entity ) && ( type2 == ev_object ) ) {
	//	return true;
	//}
		
	//if ( ( type2 == ev_entity ) && ( type1 == ev_object ) ) {
	//	return true;
	//}

	return false;
}

/*
==============
idCompiler::FindSituableOpcode
c4tnt: try to find some situable opcodes for a specified in and out types. 
All deffered 'in' variables will specified after this operation.
==============
*/
opcode_t	*idCompiler::FindSituableOpcode( opcode_t *start, idTypeDef *t_a, idTypeDef *t_b, etype_t t_c ) {
	opcode_t		*op;
	
	op = start;
	
	while( !TypeMatches( t_a->Type(), op->type_a->Type() ) || !TypeMatches( t_b->Type(), op->type_b->Type() ) ||
		( ( t_c != ev_void ) && ( op->priority != FUNCTION_PRIORITY ) && !TypeMatches( t_c, op->type_c->Type() ) ) ) {
			op++;
			if ( !op->name || strcmp( op->name, start->name ) ) {
				return NULL;		
	//			Error( "type mismatch for '%s'", start->name );
			}
	}
	return op;
}

/*
==============
idCompiler::GetExpression
==============
*/
idVarDef *idCompiler::GetExpression( int priority ) {
	opcode_t		*op;
	opcode_t		*oldop;
	opcode_t		*preset; //change current op to this
	idVarDef		*e;
	idVarDef		*e2;
	const idVarDef	*oldtype;
	etype_t 		type_a;
	etype_t 		type_b;
	idTypeDef*		type_c;
	
	if ( priority == 0 ) {
		return GetTerm( );
	}
		
	e = GetExpression( priority - 1 );
	if ( token == ";" ) {
		// save us from searching through the opcodes unneccesarily
		return e;
	}

	while( 1 ) {
		if ( ( priority == ARRAY_IND_PRIORITY ) && CheckToken( "[" ) ) {
			return ParseArrayListInd ( e );
		}
		if ( ( priority == FUNCTION_PRIORITY ) && CheckToken( "(" ) ) {
			return ParseFunctionCall( e );
		}

		// has to be a punctuation
		if ( immediateType ) {
			break;
		}

		for( op = opcodes; op->name; op++ ) {
			if ( ( op->priority == priority ) && CheckToken( op->name ) ) {
				break;
			}
		}

		if ( !op->name ) {
			// next token isn't at this priority level
			break;
		}
		preset = NULL; //no preset

		// unary operators act only on the left operand
		if ( op->type_b == &def_void ) {
			e = EmitOpcode( op, e, 0 );
			return e;
		}

		// preserve our base type
		oldtype = basetype;

		// field access needs scope from object
		if ( ( op->name[ 0 ] == '.' ) && e->TypeDef()->Inherits( &type_object ) ) {
			// save off what type this field is part of
			basetype = e->TypeDef()->def;
		}

		if ( op->rightAssociative ) {
			// if last statement is an indirect, change it to an address of
			if ( CompileTarget->NumStatements() > 0 ) {
				statement_t &statement = CompileTarget->GetStatement( CompileTarget->NumStatements() - 1 );
				if ( ( statement.op >= OP_INDIRECT_F ) && ( statement.op < OP_ADDRESS_O ) ) {
					statement.op = OP_ADDRESS_O;
					type_pointer.SetPointerType( e->TypeDef() );
					e->SetTypeDef( CompileTarget->GetType(type_pointer,true) );
				}else if ( ( statement.op >= OP_INDEX_F ) && ( statement.op < OP_INDEX_PTR ) ) {
					statement.op = OP_INDEX_PTR;
					type_pointer.SetPointerType( e->TypeDef() );
					e->SetTypeDef( CompileTarget->GetType(type_pointer,true) );
				}
			}

			e2 = GetExpression( priority );
		} else {
			e2 = GetExpression( priority - 1 );
		}

		// restore type
		basetype = oldtype;
			
		// type check
		type_a = e->Type();
		type_b = e2->Type();

		// field access gets type from field
		if ( op->name[ 0 ] == '.' ) {
			type_c = e2->TypeDef()->RightType();
		} else {
			type_c = NULL;
		}

		oldop = op;
		op = NULL;
		
		if (!op) {
			// Try to find rvalue
			if ( type_c ) {
				op = FindSituableOpcode( oldop , e->TypeDef() , e2->TypeDef() , type_c->Type() );
			} else {
				op = FindSituableOpcode( oldop , e->TypeDef() , e2->TypeDef() , ev_void );
			}

			if (!op) {
				Error( "type mismatch for '%s': %s vs %s", oldop->name, e->TypeDef()->Name(), e2->TypeDef()->Name() );
			}
		}

		switch( op - opcodes ) {
		case OP_SYSCALL :
			ExpectToken( "(" );
			e = ParseSysObjectCall( e2 );
			break;

		case OP_OBJECTCALL :
			ExpectToken( "(" );
			if ( ( e2->initialized != idVarDef::uninitialized ) && e2->value.functionPtr->eventdef ) {
				e = ParseEventCall( e, e2 );
			} else {
				e = ParseObjectCall( e, e2 );
			}
			break;
		
		case OP_EVENTCALL :
			ExpectToken( "(" );
			if ( ( e2->initialized != idVarDef::uninitialized ) && e2->value.functionPtr->eventdef ) {
				e = ParseEventCall( e, e2 );
			} else {
				e = ParseObjectCall( e, e2 );
			}
			break;

		default:
			if ( callthread ) {
				Error( "Expecting function call after 'thread'" );
			}

			if ( ( type_a == ev_pointer ) && ( type_b != e->TypeDef()->PointerType()->Type() ) ) {
				// FIXME: need to make a general case for this
				if ( ( op - opcodes == OP_STOREP_F ) && ( e->TypeDef()->PointerType()->Type() == ev_boolean ) ) {
					// copy from float to boolean pointer
					op = &opcodes[ OP_STOREP_FTOBOOL ];
				} else if ( ( op - opcodes == OP_STOREP_BOOL ) && ( e->TypeDef()->PointerType()->Type() == ev_float ) ) {
					// copy from boolean to float pointer
					op = &opcodes[ OP_STOREP_BOOLTOF ];
				} else if ( ( op - opcodes == OP_STOREP_F ) && ( e->TypeDef()->PointerType()->Type() == ev_string ) ) {
					// copy from float to string pointer
					op = &opcodes[ OP_STOREP_FTOS ];
				} else if ( ( op - opcodes == OP_STOREP_BOOL ) && ( e->TypeDef()->PointerType()->Type() == ev_string ) ) {
					// copy from boolean to string pointer
					op = &opcodes[ OP_STOREP_BTOS ];
				} else if ( ( op - opcodes == OP_STOREP_V ) && ( e->TypeDef()->PointerType()->Type() == ev_string ) ) {
					// copy from vector to string pointer
					op = &opcodes[ OP_STOREP_VTOS ];
				} else if ( ( op - opcodes == OP_STOREP_ENT ) && ( e->TypeDef()->PointerType()->Type() == ev_object ) ) {
					// store an entity into an object pointer
					op = &opcodes[ OP_STOREP_OBJENT ];
				} else {
					Error( "type mismatch for '%s'", op->name );
				}
			}
			
			if ( op->rightAssociative ) {
				e = EmitOpcode( op, e2, e, type_c );
			} else {
				e = EmitOpcode( op, e, e2, type_c );
			}

			if ( op - opcodes == OP_STOREP_OBJENT ) {
				// statement.b points to type_pointer, which is just a temporary that gets its type reassigned, so we store the real type in statement.c
				// so that we can do a type check during run time since we don't know what type the script object is at compile time because it
				// comes from an entity
				statement_t &statement = CompileTarget->GetStatement( CompileTarget->NumStatements() - 1 );
				statement.c = type_pointer.PointerType()->def;
			}

			break;
		}
	}

	return e;
}

/*
================
idCompiler::PatchLoop
================
*/
void idCompiler::PatchLoop( int start, int continuePos ) {
	int			i;
	statement_t	*pos;

	pos = &CompileTarget->GetStatement( start );
	for( i = start; i < CompileTarget->NumStatements(); i++, pos++ ) {
		if ( pos->op == OP_BREAK ) {
			pos->op = OP_GOTO;
			pos->a = JumpFrom( i );
		} else if ( pos->op == OP_CONTINUE ) {
			pos->op = OP_GOTO;
			pos->a = JumpDef( i, continuePos );
		}
	}
}

/*
================
idCompiler::ParseReturnStatement
================
*/
void idCompiler::ParseReturnStatement( void ) {
	idVarDef	*e;
	idVarDef	*_r;
	etype_t 	type_a;
	etype_t 	type_b;
	opcode_t	*op;

	if ( CheckToken( ";" ) ) {
		if ( scope->TypeDef()->ReturnType()->Type() != ev_void ) {
			Error( "expecting return value" );
		}

		EmitOpcode( OP_RETURN, 0, 0 );
		return;
	}

	e = GetExpression( TOP_PRIORITY );
	ExpectToken( ";" );

	_r = EmitCAST( scope->TypeDef()->ReturnType(), e, scope->TypeDef()->Name() );
	EmitOpcode( OP_RETURN, _r, 0 );
	return;
}
	
/*
================
idCompiler::ParseTracefall
================
*/
void idCompiler::ParseTracefall( void ) {
	idVarDef	*e;
	idVarDef	*_r;
	etype_t 	type_a;
	etype_t 	type_b;
	opcode_t	*op;

	if ( CheckToken( ";" ) ) {
		EmitOpcode( OP_TRACEFALL, 0, 0 );
		return;
	}

	e = GetExpression( TOP_PRIORITY );
	ExpectToken( ";" );

	EmitOpcode( OP_TRACEFALL, e, 0 );
	return;
}

/*
================
idCompiler::ParseWhileStatement
================
*/
void idCompiler::ParseWhileStatement( void ) {
	idVarDef	*e;
	int			patch1;
	int			patch2;

	loopDepth++;

	ExpectToken( "(" );
	
	patch2 = CompileTarget->NumStatements();
	e = GetExpression( TOP_PRIORITY );
	ExpectToken( ")" );

	if ( ( e->initialized == idVarDef::initializedConstant ) ) {
		if ( *e->value.intPtr != 0 ) {
			ParseStatement();
			EmitOpcode( OP_GOTO, JumpTo( patch2 ), 0 );
			// fixup breaks and continues
			PatchLoop( patch2, patch2 );
		} else {
			patch1 = CompileTarget->NumStatements();
			ParseStatement();
			CompileTarget->DropStatements( patch1 );	// Eat all skipped statements
		}
	} else {
		patch1 = CompileTarget->NumStatements();
        EmitOpcode( OP_IFNOT, e, 0 );
		ParseStatement();
		EmitOpcode( OP_GOTO, JumpTo( patch2 ), 0 );
		CompileTarget->GetStatement( patch1 ).b = JumpFrom( patch1 );
		// fixup breaks and continues
		PatchLoop( patch2, patch2 );
	}

	loopDepth--;
}

/*
================
idCompiler::ParseForStatement

c4tnt: Form of for statement with a conter was changed!
Form of for statement with a counter:

	a = 0;
start:					<< patch4

	if ( !( a < 10 ) ) goto end;		<< patch1

process:
	
	statements;
	a = a + 1;
	goto start;			<< goto patch4

end:

Form of for statement without a counter:

	a = 0;
start:					<< patch2
	if ( !( a < 10 ) ) {
		goto end;		<< patch1
	}

process:
	statements;
	goto start;			<< goto patch2

end:
================
*/
void idCompiler::ParseForStatement( void ) {
	idVarDef	*e;
	int			start;
	int			patch1;
	int			patch2;
	int			patch3;
	
	int			constantcondition = 0;

	idList<statement_t>	copybuffer;

	loopDepth++;

	start = CompileTarget->NumStatements();

	ExpectToken( "(" );
	
	// init
	if ( !CheckToken( ";" ) ) {
		do {
			GetExpression( TOP_PRIORITY );
		} while( CheckToken( "," ) );

		ExpectToken( ";" );
	}

	// condition
	patch2 = CompileTarget->NumStatements();

	e = GetExpression( TOP_PRIORITY );
	ExpectToken( ";" );

	if ( e->initialized == idVarDef::initializedConstant ) {
		if ( *e->value.intPtr ) {
			constantcondition = 1;
		} else {
			constantcondition = -1;
		}
	}

	patch1 = CompileTarget->NumStatements();

	if ( constantcondition >= 0 )
	{
		if ( !constantcondition )
			EmitOpcode( OP_IFNOT, e, 0 );

		// counter
		if ( !CheckToken( ")" ) ) {
			patch3 = CompileTarget->NumStatements();

			do {
				GetExpression( TOP_PRIORITY );
			} while( CheckToken( "," ) );
			
			ExpectToken( ")" );

			copybuffer.Clear();
			CompileTarget->SaveStatements( patch3, copybuffer );
			ParseStatement();
			patch3 = CompileTarget->NumStatements();
			CompileTarget->LoadStatements( copybuffer );

		} else {
			ParseStatement();
			patch3 = patch2;
		}

		// goto patch2
		EmitOpcode( OP_GOTO, JumpTo( patch2 ), 0 );

		// fixup patch1
		if ( !constantcondition )
			CompileTarget->GetStatement( patch1 ).b = JumpFrom( patch1 );

		// fixup breaks and continues
		PatchLoop( start, patch3 );
	
	} else {
		if ( !CheckToken( ")" ) ) {
			do {
				GetExpression( TOP_PRIORITY );
			} while( CheckToken( "," ) );
			ExpectToken( ")" );
			ParseStatement();
			CompileTarget->DropStatements( patch1 );	// Eat all skipped statements
		} else {
			ParseStatement();
			CompileTarget->DropStatements( patch1 );	// Eat all skipped statements
		}
	}

	loopDepth--;
}

/*
================
idCompiler::ParseDoWhileStatement
================
*/
void idCompiler::ParseDoWhileStatement( void ) {
	idVarDef	*e;
	int			patch1;

	loopDepth++;

	patch1 = CompileTarget->NumStatements();
	ParseStatement();
	ExpectToken( "while" );
	ExpectToken( "(" );
	e = GetExpression( TOP_PRIORITY );
	ExpectToken( ")" );
	ExpectToken( ";" );

	EmitOpcode( OP_IF, e, JumpTo( patch1 ) );

	// fixup breaks and continues
	PatchLoop( patch1, patch1 );

	loopDepth--;
}

/*
================
idCompiler::ParseIfStatement
================
*/
void idCompiler::ParseIfStatement( void ) {
	idVarDef	*e;
	int			patch1;
	int			patch2;

	ExpectToken( "(" );
	e = GetExpression( TOP_PRIORITY );
	ExpectToken( ")" );

	if ( e->initialized == idVarDef::initializedConstant ) {
		if ( *e->value.intPtr ) {
			ParseStatement();
			if ( CheckToken( "else" ) ) {					// Remove else block
				patch1 = CompileTarget->NumStatements();
				ParseStatement();
				CompileTarget->DropStatements( patch1 );
			}
		} else {
			patch1 = CompileTarget->NumStatements();		// Remove primary if block
			ParseStatement();
			CompileTarget->DropStatements( patch1 );
			if ( CheckToken( "else" ) ) {					
				ParseStatement();
			}
		}
	} else {
		patch1 = CompileTarget->NumStatements();
		EmitOpcode( OP_IFNOT, e, 0 );

		ParseStatement();
		
		if ( CheckToken( "else" ) ) {
			patch2 = CompileTarget->NumStatements();
			EmitOpcode( OP_GOTO, 0, 0 );
			CompileTarget->GetStatement( patch1 ).b = JumpFrom( patch1 );
			ParseStatement();
			CompileTarget->GetStatement( patch2 ).a = JumpFrom( patch2 );
		} else {
			CompileTarget->GetStatement( patch1 ).b = JumpFrom( patch1 );
		}
	}
}

/*
================
idCompiler::ParseCvarStatement
================
*/
void idCompiler::ParseCVarStatement( void ) {
	idStr 		name;
	idTypeDef	*type;

	if ( scope->Type() != ev_namespace ) {
		Error( "Cvars cannot be defined within functions or other objects" );
	}

	type = ParseType();
	ParseName( name );

	if (CheckToken( "(" ) ) {		// function prototype/declaration, build a syscmd
		ParseFunctionDef( type, name.c_str() );
		Warning ( "Defined function _cvar" );
	} else {						// probably is a variable, build a cvar
		if ( type == &type_boolean ) {
			ParseVariableDef( type, name.c_str() );
			ExpectToken( ";" );
			Warning ( "Defined bool _cvar" );
		} else if ( type == &type_vector ) {
			ParseVariableDef( type, name.c_str() );
			ExpectToken( ";" );
			Warning ( "Defined vector _cvar" );
		} else if ( type == &type_float ) {
			ParseVariableDef( type, name.c_str() );
			ExpectToken( ";" );
			Warning ( "Defined float _cvar" );
		} else if ( type == &type_string ) {
			ParseVariableDef( type, name.c_str() );
			ExpectToken( ";" );
			Warning ( "Defined string _cvar" );
		} else if ( type == &type_entity ) {
			ParseVariableDef( type, name.c_str() );
			ExpectToken( ";" );
			Warning ( "Defined entity _cvar" );
		} else {
			Error ( "Wrong type of _cvar" );
		}
	}
}

/*
============
idCompiler::ParseStatement
============
*/
void idCompiler::ParseStatement( void ) {
	if ( CheckToken( ";" ) ) {
		// skip semicolons, which are harmless and ok syntax
		return;
	}

	if ( CheckToken( "{" ) ) {
		do {
			ParseStatement();
		} while( !CheckToken( "}" ) );

		return;
	} 

	if ( CheckToken( "tracefall" ) ) {
		ParseTracefall();
		return;
	}

	if ( CheckToken( "return" ) ) {
		ParseReturnStatement();
		return;
	}
	
	if ( CheckToken( "while" ) ) {
		ParseWhileStatement();
		return;
	}

	if ( CheckToken( "for" ) ) {
		ParseForStatement();
		return;
	}

	if ( CheckToken( "do" ) ) {
		ParseDoWhileStatement();
		return;
	}

	if ( CheckToken( "break" ) ) {
		ExpectToken( ";" );
		if ( !loopDepth ) {
			Error( "cannot break outside of a loop" );
		}
		EmitOpcode( OP_BREAK, 0, 0 );
		return;
	}

	if ( CheckToken( "continue" ) ) {
		ExpectToken( ";" );
		if ( !loopDepth ) {
			Error( "cannot contine outside of a loop" );
		}
		EmitOpcode( OP_CONTINUE, 0, 0 );
		return;
	}

	if ( CheckType() != NULL ) {
		ParseDefs();
		return;
	}

	if ( CheckToken( "if" ) ) {
		ParseIfStatement();
		return;
	}

	GetExpression( TOP_PRIORITY );
	ExpectToken(";");
}

/*
================
idCompiler::ParseObjectDef
================
*/
void idCompiler::ParseObjectDef( const char *objname ) {
	idTypeDef	*objtype;
	idTypeDef	*type;
	idTypeDef	*parentType;
	idTypeDef	*fieldtype;
	idStr		name;
	const char  *fieldname;
	idTypeDef	newtype( ev_field, NULL, "", 0, NULL, etl_deflevel); //c4tnt: object fields has OBJ level!
	idVarDef	*oldscope;
	int			num;
	int			i;

	oldscope = scope;
	if ( scope->Type() != ev_namespace ) {
		Error( "Objects cannot be defined within functions or other objects" );
	}

	// make sure it doesn't exist before we create it
	if ( CompileTarget->FindType( objname ) != NULL ) {
		Error( "'%s' : redefinition; different basic types", objname );
	}

	// base type
	if ( !CheckToken( ":" ) ) {
		parentType = &type_object;
	} else {
		parentType = ParseType();
		if ( !parentType->Inherits( &type_object ) ) {
			Error( "Objects may only inherit from objects." );
		}
	}
	
	objtype = CompileTarget->AllocType( ev_object, NULL, objname, parentType == &type_object ? 0 : parentType->Size(), parentType, etl_varlevel ); //c4tnt: object sample will have OBJ level
	objtype->def = CompileTarget->AllocDef( objtype, objname, scope, true );
	scope = objtype->def;

	// inherit all the functions
	num = parentType->NumFunctions();
	for( i = 0; i < parentType->NumFunctions(); i++ ) {
		const function_t *func = parentType->GetFunction( i );
		objtype->AddFunction( func );
	}

	ExpectToken( "{" );

	do {
		if ( CheckToken( ";" ) ) {
			// skip semicolons, which are harmless and ok syntax
			continue;
		}

		fieldtype = ParseType(etl_objlevel); //c4tnt: fixup - prevent creating a non-object-level inside object
		newtype.SetFieldType( fieldtype );

		fieldname = va( "%s field", fieldtype->Name() );
		newtype.SetName( fieldname );

		ParseName( name );

		// check for a function prototype or declaraction
		if ( CheckToken( "(" ) ) {
			ParseFunctionDef( newtype.FieldType(), name );
		} else {
			type = CompileTarget->GetType( newtype, true );
			assert( !type->def );
			CompileTarget->AllocDef( type, name, scope, true );
			objtype->AddField( type, name );
			ExpectToken( ";" );
		}
	} while( !CheckToken( "}" ) );

	scope = oldscope;

	ExpectToken( ";" );
}

/*
============
idCompiler::ParseFunction

parse a function type
============
*/
idTypeDef *idCompiler::ParseFunction( idTypeDef *returnType, const char *name ) {
	idTypeDef	newtype( ev_function, NULL, name, type_function.Size(), returnType, etl_objlevel ); //c4tnt: functions have OBJ level
	idTypeDef	*type;
	
	if ( scope->Type() != ev_namespace ) {
		// create self pointer
		newtype.AddFunctionParm( scope->TypeDef(), "self" );
	}

	if ( !CheckToken( ")" ) ) {
		idStr parmName;
		do {
			type = ParseType();
			ParseName( parmName );
			newtype.AddFunctionParm( type, parmName );
		} while( CheckToken( "," ) );

		ExpectToken( ")" );
	}

	return CompileTarget->GetType( newtype, true );
}

/*
================
idCompiler::ParseFunctionDef
================
*/
void idCompiler::ParseFunctionDef( idTypeDef *returnType, const char *name ) {
	idTypeDef		*type;
	idVarDef		*def;
	const idVarDef	*parm;
	idVarDef		*oldscope;
	int 			i;
	int 			numParms;
	const idTypeDef	*parmType;
	function_t		*func;
	statement_t		*pos;

	if ( ( scope->Type() != ev_namespace ) && !scope->TypeDef()->Inherits( &type_object ) ) {
		Error( "Functions may not be defined within other functions" );
	}

	type = ParseFunction( returnType, name );
	def = CompileTarget->GetDef( type, name, scope );
	if ( !def ) {
		def = CompileTarget->AllocDef( type, name, scope, true );
		type->def = def;

		func = &CompileTarget->AllocFunction( def );
		if ( scope->TypeDef()->Inherits( &type_object ) ) {
			scope->TypeDef()->AddFunction( func );
		}
	} else {
		func = def->value.functionPtr;
		assert( func );
		if ( func->firstStatement ) {
			Error( "%s redeclared", def->GlobalName() );
		}
		// c4tnt: disallow eventdef redefinition
		if ( func->eventdef ) {
			Error( "%s already declared as built-in event", def->GlobalName() );
		}
	}

	// check if this is a prototype or declaration
	if ( !CheckToken( "{" ) ) {
		// it's just a prototype, so get the ; and move on
		ExpectToken( ";" );
		return;
	}

	// calculate stack space used by parms
	numParms = type->NumParameters();
	func->parmSize.SetNum( numParms );
	for( i = 0; i < numParms; i++ ) {
		parmType = type->GetParmType( i );
		if ( parmType->Inherits( &type_object ) ) {
			func->parmSize[ i ] = type_object.Size();
		} else {
			func->parmSize[ i ] = parmType->Size();
		}
		func->parmTotal += func->parmSize[ i ];
	}

	// define the parms
	for( i = 0; i < numParms; i++ ) {
		if ( CompileTarget->GetDef( type->GetParmType( i ), type->GetParmName( i ), def ) ) {
			Error( "'%s' defined more than once in function parameters", type->GetParmName( i ) );
		}
		parm = CompileTarget->AllocDef( type->GetParmType( i ), type->GetParmName( i ), def, false );
	}

	oldscope = scope;
	scope = def;

	func->firstStatement = CompileTarget->NumStatements();

	// check if we should call the super class constructor
	if ( oldscope->TypeDef()->Inherits( &type_object ) && !idStr::Icmp( name, "init" ) ) {
		idTypeDef *superClass;
		function_t *constructorFunc = NULL;

		// find the superclass constructor
		for( superClass = oldscope->TypeDef()->SuperClass(); superClass != &type_object; superClass = superClass->SuperClass() ) {
			constructorFunc = CompileTarget->FindFunction( va( "%s::init", superClass->Name() ) );
			if ( constructorFunc ) {
				break;
			}
		}

		// emit the call to the constructor
		if ( constructorFunc ) {
			idVarDef *selfDef = CompileTarget->GetDef( type->GetParmType( 0 ), type->GetParmName( 0 ), def );
			assert( selfDef );
			EmitPush( selfDef, selfDef->TypeDef() );
			EmitOpcode( &opcodes[ OP_CALL ], constructorFunc->def, 0 );
		}
	}

	// parse regular statements
	while( !CheckToken( "}" ) ) {
		ParseStatement();
	}

	// check if we should call the super class destructor
	if ( oldscope->TypeDef()->Inherits( &type_object ) && !idStr::Icmp( name, "destroy" ) ) {
		idTypeDef *superClass;
		function_t *destructorFunc = NULL;

		// find the superclass destructor
		for( superClass = oldscope->TypeDef()->SuperClass(); superClass != &type_object; superClass = superClass->SuperClass() ) {
			destructorFunc = CompileTarget->FindFunction( va( "%s::destroy", superClass->Name() ) );
			if ( destructorFunc ) {
				break;
			}
		}

		if ( destructorFunc ) {
			if ( func->firstStatement < CompileTarget->NumStatements() ) {
			// change all returns to point to the call to the destructor
				pos = &CompileTarget->GetStatement( func->firstStatement );
				for( i = func->firstStatement; i < CompileTarget->NumStatements(); i++, pos++ ) {
					if ( pos->op == OP_RETURN ) {
						pos->op = OP_GOTO;
						pos->a = JumpDef( i, CompileTarget->NumStatements() );
					}
				}
			}

			// emit the call to the destructor
			idVarDef *selfDef = CompileTarget->GetDef( type->GetParmType( 0 ), type->GetParmName( 0 ), def );
			assert( selfDef );
			EmitPush( selfDef, selfDef->TypeDef() );
			EmitOpcode( &opcodes[ OP_CALL ], destructorFunc->def, 0 );
		}
	}

// Disabled code since it caused a function to fall through to the next function when last statement is in the form "if ( x ) { return; }"
#if 0
	// don't bother adding a return opcode if the "return" statement was used.
	if ( ( func->firstStatement == CompileTarget->NumStatements() ) || ( CompileTarget->GetStatement( CompileTarget->NumStatements() - 1 ).op != OP_RETURN ) ) {
		// emit an end of statements opcode
		EmitOpcode( OP_RETURN, 0, 0 );
	}
#else
	if ( returnType->Type() == ev_void ) {
		// always emit the return opcode
		EmitOpcode( OP_RETURN, 0, 0 );
	} else {
		EmitOpcode( OP_DEFRETURN, JumpConstant( returnType->Type() ), 0 );	// Throw safe return
	}
#endif

	// record the number of statements in the function
	func->numStatements = CompileTarget->NumStatements() - func->firstStatement;

	scope = oldscope;
}

/*
================
idCompiler::ParseVariableDef
================
*/
void idCompiler::ParseVariableDef( idTypeDef *type, const char *name ) {
	idVarDef	*def, *def2;
	bool		negate;

	def = CompileTarget->GetDef( type, name, scope );
	if ( def ) {
		Error( "%s redeclared", name );
	}
	
	def = CompileTarget->AllocDef( type, name, scope, false );

	// check for an initialization
	
	if ( type->Type() == ev_array) //a sort of an array
	{
		def2 = EmitArrayCreation(def);
		if (def2 != NULL) {
			EmitOpcode( OP_STORE_A, def2, def );
		}
	}

	if ( CheckToken( "=" ) ) {
		// if a local variable in a function then write out interpreter code to initialize variable
		if ( scope->Type() == ev_function ) {
			def2 = GetExpression( TOP_PRIORITY );
			if ( ( type == &type_float ) && ( def2->TypeDef() == &type_float ) ) {
				EmitOpcode( OP_STORE_F, def2, def );
			} else if ( ( type == &type_vector ) && ( def2->TypeDef() == &type_vector ) ) {
				EmitOpcode( OP_STORE_V, def2, def );
			} else if ( ( type == &type_string ) && ( def2->TypeDef() == &type_string ) ) {
				EmitOpcode( OP_STORE_S, def2, def );
			} else if ( ( type == &type_entity ) && ( ( def2->TypeDef() == &type_entity ) || ( def2->TypeDef()->Inherits( &type_object ) ) ) ) {
				EmitOpcode( OP_STORE_ENT, def2, def );
			} else if ( type->Inherits( &type_object ) ) {
				if ( def2->TypeDef() == &type_entity ) {
					EmitOpcode( OP_STORE_OBJENT, def2, def );
				} else if ( def2->TypeDef()->Inherits( type ) ) {
					EmitOpcode( OP_STORE_OBJ, def2, def );
				} else {
					Error( "bad initialization for '%s', %s is not inherited from the %s", name, def2->TypeDef()->Name(), type->Name() );
				}
			} else if ( ( type == &type_boolean ) && ( def2->TypeDef() == &type_boolean ) ) {
				EmitOpcode( OP_STORE_BOOL, def2, def );
			} else if ( ( type == &type_string ) && ( def2->TypeDef() == &type_float ) ) {
				EmitOpcode( OP_STORE_FTOS, def2, def );
			} else if ( ( type == &type_string ) && ( def2->TypeDef() == &type_boolean ) ) {
				EmitOpcode( OP_STORE_BTOS, def2, def );
			} else if ( ( type == &type_string ) && ( def2->TypeDef() == &type_vector ) ) {
				EmitOpcode( OP_STORE_VTOS, def2, def );
			} else if ( ( type == &type_boolean ) && ( def2->TypeDef() == &type_float ) ) {
				EmitOpcode( OP_STORE_FTOBOOL, def2, def );
			} else if ( ( type == &type_float ) && ( def2->TypeDef() == &type_boolean ) ) {
				EmitOpcode( OP_STORE_BOOLTOF, def2, def );
			} else {
				Error( "bad initialization for '%s'", name );
			}
		} else {
			// global variables can only be initialized with immediate values
			negate = false;
			if ( token.type == TT_PUNCTUATION && token == "-" ) {
				negate = true;
				NextToken();
				if ( immediateType != &type_float ) {
					Error( "wrong immediate type for '-' on variable '%s'", name );
				}
			}

			if ( immediateType != type ) {
				Error( "wrong immediate type for '%s'", name );
			}

			// global variables are initialized at start up
			if ( type == &type_string ) {
				def->SetString( token, false );
			} else {
				if ( negate ) {
					immediate._float = -immediate._float;
				}
				def->SetValue( immediate, false );
			}
			NextToken();
		}
	} else if ( type == &type_string ) {
		// local strings on the stack are initialized in the interpreter
		if ( scope->Type() != ev_function ) {
			def->SetString( "", false );
		}
	} else if ( type->Inherits( &type_object ) ) {
		if ( scope->Type() != ev_function ) {
			def->SetObject( NULL );
		}
	}
}

/*
================
idCompiler::GetTypeForEventArg
================
*/
idTypeDef *idCompiler::GetTypeForEventArg( char argType ) {
	idTypeDef *type;

	switch( argType ) {
	case D_EVENT_INTEGER :
		// this will get converted to int by the interpreter
		type = &type_float;
		break;

	case D_EVENT_FLOAT :
		type = &type_float;
		break;

	case D_EVENT_VECTOR :
		type = &type_vector;
		break;

	case D_EVENT_STRING :
		type = &type_string;
		break;

	case D_EVENT_ENTITY :
	case D_EVENT_ENTITY_NULL :
		type = &type_entity;
		break;

	case D_EVENT_CONTEXT :
	case D_EVENT_VOID :
		type = &type_void;
		break;

	case D_EVENT_TRACE :
		// This data type isn't available from script
		type = NULL;
		break;
	case D_EVENT_LIST :
		type = &type_list;
		break;

	default:
		// probably a typo
		type = NULL;
		break;
	}
	
	return type;
}

/*
================
idCompiler::ParseEventDef
================
*/
void idCompiler::ParseEventDef( idTypeDef *returnType, const char *name ) {
	const idTypeDef	*expectedType;
	idTypeDef		*argType;
	idTypeDef		*type;
	int 			i;
	int				num;
	const char		*format;
	const idEventDef *ev;
	idStr			parmName;

	ev = idEventDef::FindEvent( name );
	if ( !ev ) {
		Error( "Unknown event '%s'", name );
	}

	// set the return type
	expectedType = GetTypeForEventArg( ev->GetReturnType() );
	if ( !expectedType ) {
		Error( "Invalid return type '%c' in definition of '%s' event.", ev->GetReturnType(), name );
	}
	if ( returnType != expectedType ) {
		Error( "Return type doesn't match internal return type '%s'", expectedType->Name() );
	}

	idTypeDef newtype( ev_function, NULL, name, type_function.Size(), returnType );

	ExpectToken( "(" );

	format = ev->GetArgFormat();
	num = strlen( format );
	for( i = 0; i < num; i++ ) {
		expectedType = GetTypeForEventArg( format[ i ] );
		if ( !expectedType || ( expectedType == &type_void ) ) {
			Error( "Invalid parameter '%c' in definition of '%s' event.", format[ i ], name );
		}

		argType = ParseType();
		ParseName( parmName );
		if ( argType != expectedType ) {
			Error( "The type of parm %d ('%s') does not match the internal type '%s' in definition of '%s' event.", 
				i + 1, parmName.c_str(), expectedType->Name(), name );
		}

		newtype.AddFunctionParm( argType, "" );

		if ( i < num - 1 ) {
			if ( CheckToken( ")" ) ) {
				Error( "Too few parameters for event definition.  Internal definition has %d parameters.", num );
			}
			ExpectToken( "," );
		}
	}
	if ( !CheckToken( ")" ) ) {
		Error( "Too many parameters for event definition.  Internal definition has %d parameters.", num );
	}
	ExpectToken( ";" );

	type = CompileTarget->FindType( name );
	if ( type ) {
		if ( !newtype.MatchesType( *type ) || ( type->def->value.functionPtr->eventdef != ev ) ) {
			Error( "Type mismatch on redefinition of '%s'", name );
		}
	} else {
		type = CompileTarget->AllocType( newtype );
		type->def = CompileTarget->AllocDef( type, name, &def_namespace, true );

		function_t &func	= CompileTarget->AllocFunction( type->def );
		func.eventdef		= ev;
		func.parmSize.SetNum( num );
		for( i = 0; i < num; i++ ) {
			argType = newtype.GetParmType( i );
			func.parmTotal		+= argType->Size();
			func.parmSize[ i ]	= argType->Size();
		}

		// mark the parms as local
		func.locals	= func.parmTotal;
	}
}

/*
================
idCompiler::ParseDefs

Called at the outer layer and when a local statement is hit
================
*/
void idCompiler::ParseDefs( void ) {
	idStr 		name;
	idTypeDef	*type;
	idVarDef	*def;
	idVarDef	*oldscope;

	if ( CheckToken( ";" ) ) {
		// skip semicolons, which are harmless and ok syntax
		return;
	}

	if ( CheckToken( "_cvar" ) ) {
		ParseCVarStatement();
		return;
	}

	type = ParseType();
	if ( type == &type_scriptevent ) {
		type = ParseType();
		ParseName( name );
		ParseEventDef( type, name );
		return;
	}
    
	ParseName( name );

	if ( type == &type_namespace ) {
		def = CompileTarget->GetDef( type, name, scope );
		if ( !def ) {
			def = CompileTarget->AllocDef( type, name, scope, true );
		}
		ParseNamespace( def );
	} else if ( CheckToken( "::" ) ) {
		def = CompileTarget->GetDef( NULL, name, scope );
		if ( !def ) {
			Error( "Unknown object name '%s'", name.c_str() );
		}
		ParseName( name );
		oldscope = scope;
		scope = def;

		ExpectToken( "(" );
		ParseFunctionDef( type, name.c_str() );
		scope = oldscope;
	} else if ( type == &type_object ) {
		ParseObjectDef( name.c_str() );
	} else if ( CheckToken( "(" ) ) {		// check for a function prototype or declaraction
		ParseFunctionDef( type, name.c_str() );
	} else {
		ParseVariableDef( type, name.c_str() );
		while( CheckToken( "," ) ) {
			ParseName( name );
			ParseVariableDef( type, name.c_str() );
		}
		ExpectToken( ";" );
	}
}

/*
================
idCompiler::ParseNamespace

Parses anything within a namespace definition
================
*/
void idCompiler::ParseNamespace( idVarDef *newScope ) {
	idVarDef *oldscope;

	oldscope = scope;
	if ( newScope != &def_namespace ) {
		ExpectToken( "{" );
	}

	while( !eof ) {
		scope		= newScope;
		callthread	= false;

		if ( ( newScope != &def_namespace ) && CheckToken( "}" ) ) {
			break;
		}

		ParseDefs();
	}

	scope = oldscope;
}

/*
============
idCompiler::CompileFile

compiles the 0 terminated text, adding definitions to the program structure
============
*/
void idCompiler::CompileFile( bool toConsole ) {

	scope				= &def_namespace;
	basetype			= NULL;
	callthread			= false;
	loopDepth			= 0;
	eof					= false;
	braceDepth			= 0;
	immediateType		= NULL;
	currentLineNumber	= 0;
	console				= toConsole;
	
	if (!CompileTarget)
		Error( "NULL compile target" );

	memset( &immediate, 0, sizeof( immediate ) );

// c4tnt: Fixup punctuations, very temporary hack, it will be moved to a Lexer
//	char	**ptr;
//	int		id;
//	for( ptr = punctuation; *ptr != NULL; ptr++ ) {
//		id = parserPtr->GetPunctuationId( *ptr );
//		if ( ( id >= 0 ) && ( id < 256 ) ) {
//			punctuationValid[ id ] = true;
//		}
//	}
	
//c4tnt: Huge deprecation here, see clear sdk

	try {
		// read first token
		NextToken();
		while( !eof ) {
			// parse from global namespace
			ParseNamespace( &def_namespace );
		}
	}
		
	catch( idCompileError &err ) {
		idStr error;

		if ( console ) {
			// don't print line number of an error if were calling script from the console using the "script" command
			sprintf( error, "Error: %s\n", err.error );
		} else {
			sprintf( error, "Error: file %s, line %d: %s\n", CompileTarget->GetFilename( currentFileNumber ), currentLineNumber, err.error );
		}

		parserPtr->FreeSource();

		throw idCompileError( error );
	}

	parserPtr->FreeSource();
}
