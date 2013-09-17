// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

// simple types.  function types are dynamically allocated
// c4tnt: etl_ flags was added
idTypeDef	type_void( ev_void, &def_void, "void", 0, NULL, etl_varlevel);
idTypeDef	type_scriptevent( ev_scriptevent, &def_scriptevent, "scriptevent", sizeof( void * ), NULL, etl_deflevel);
idTypeDef	type_namespace( ev_namespace, &def_namespace, "namespace", sizeof( void * ), NULL, etl_deflevel);
idTypeDef	type_string( ev_string, &def_string, "string", MAX_STRING_LEN, NULL, etl_varlevel);
idTypeDef	type_float( ev_float, &def_float, "float", sizeof( float ), NULL, etl_varlevel);
idTypeDef	type_vector( ev_vector, &def_vector, "vector", sizeof( idVec3 ), NULL, etl_varlevel);
idTypeDef	type_entity( ev_entity, &def_entity, "entity", sizeof( int * ), NULL, etl_varlevel);					// stored as entity number pointer
idTypeDef	type_field( ev_field, &def_field, "field", sizeof( void * ), NULL, etl_objlevel); 
idTypeDef	type_function( ev_function, &def_function, "function", sizeof( void * ), &type_void, etl_objlevel);
idTypeDef	type_virtualfunction( ev_virtualfunction, &def_virtualfunction, "virtual function", sizeof( int ), NULL, etl_objlevel);
idTypeDef	type_pointer( ev_pointer, &def_pointer, "pointer", sizeof( void * ), NULL, etl_objlevel);
idTypeDef	type_object( ev_object, &def_object, "object", sizeof( int * ), NULL, etl_deflevel);		// stored as entity number pointer
idTypeDef	type_jumpoffset( ev_jumpoffset, &def_jumpoffset, "<jump>", sizeof( int ), NULL, etl_deflevel); // only used for jump opcodes
idTypeDef	type_argsize( ev_argsize, &def_argsize, "<argsize>", sizeof( int ), NULL, etl_deflevel);	// only used for function call and thread opcodes
idTypeDef	type_boolean( ev_boolean, &def_boolean, "boolean", sizeof( int ), NULL, etl_varlevel);
idTypeDef	type_list( ev_list, &def_list, "list", sizeof( int * ), NULL, etl_varlevel); //c4tnt: added an array and a collection data types - stored as id on the data pool
idTypeDef	type_array( ev_array, &def_array, "array", sizeof( int * ), NULL, etl_varlevel); 

idVarDef	def_void( &type_void );
idVarDef	def_scriptevent( &type_scriptevent );
idVarDef	def_namespace( &type_namespace );
idVarDef	def_string( &type_string );
idVarDef	def_float( &type_float );
idVarDef	def_vector( &type_vector );
idVarDef	def_entity( &type_entity );
idVarDef	def_field( &type_field ); 
idVarDef	def_function( &type_function );
idVarDef	def_virtualfunction( &type_virtualfunction );
idVarDef	def_pointer( &type_pointer );
idVarDef	def_object( &type_object );
idVarDef	def_jumpoffset( &type_jumpoffset );		// only used for jump opcodes
idVarDef	def_argsize( &type_argsize );
idVarDef	def_boolean( &type_boolean );
idVarDef	def_list( &type_list );	//c4tnt: added an array and a collection data types
idVarDef	def_array( &type_array );

/***********************************************************************

  function_t

***********************************************************************/

/*
================
function_t::function_t
================
*/
function_t::function_t() {
	Clear();
}

/*
================
function_t::Allocated
================
*/
size_t function_t::Allocated( void ) const {
	return name.Allocated() + parmSize.Allocated();
}

/*
================
function_t::SetName
================
*/
void function_t::SetName( const char *name ) {
	this->name = name;
}

/*
================
function_t::Name
================
*/
const char *function_t::Name( void ) const {
	return name;
}

/*
================
function_t::Clear
================
*/
void function_t::Clear( void ) {
	eventdef		= NULL;
	def				= NULL;
	type			= NULL;
	firstStatement	= 0;
	numStatements	= 0;
	parmTotal		= 0;
	locals			= 0;
	filenum			= 0;
	name.Clear();
	parmSize.Clear();
}

/***********************************************************************

  idTypeDef

***********************************************************************/

/*
================
c4tnt: note - vlayer added
idTypeDef::idTypeDef
================
*/
idTypeDef::idTypeDef( etype_t etype, idVarDef *edef, const char *ename, int esize, idTypeDef *aux, etypelayer_t vlayer) {
	name		= ename;
	type		= etype;
	def			= edef;
	size		= esize;
	auxType		= aux;
	layer		= vlayer;

	parmTypes.SetGranularity( 1 );
	parmNames.SetGranularity( 1 );
	functions.SetGranularity( 1 );
}

/*
================
idTypeDef::idTypeDef
================
*/
idTypeDef::idTypeDef( const idTypeDef &other ) {
	*this = other;
}

/*
================
c4tnt: note - vlayer added
idTypeDef::operator=
================
*/
void idTypeDef::operator=( const idTypeDef& other ) {
	type		= other.type;
	layer		= other.layer;
	def			= other.def;
	name		= other.name;
	size		= other.size;
	auxType		= other.auxType;
	parmTypes	= other.parmTypes;
	parmNames	= other.parmNames;
	functions	= other.functions;
}

/*
================
idTypeDef::Allocated
================
*/
size_t idTypeDef::Allocated( void ) const {
	size_t memsize;
	int i;

	memsize = name.Allocated() + parmTypes.Allocated() + parmNames.Allocated() + functions.Allocated();
	for( i = 0; i < parmTypes.Num(); i++ ) {
		memsize += parmNames[ i ].Allocated();
	}

	return memsize;
}

/*
================
idTypeDef::Inherits

Returns true if basetype is an ancestor of this type.
================
*/
bool idTypeDef::Inherits( const idTypeDef *basetype ) const {
	idTypeDef *superType;

	if ( type != ev_object ) {
		return false;
	}

	if ( this == basetype ) {
		return true;
	}
	for( superType = auxType; superType != NULL; superType = superType->auxType ) {
		if ( superType == basetype ) {
			return true;
		}
	}

	return false;
}

/*
================
c4tnt: variable creation layer checker
idTypeDef::CheckLayer

Returns true if type level >= requested type level
================
*/
bool idTypeDef::CheckLayer( const etypelayer_t vlayer ) const {

	if (layer < vlayer) return false;

	return true;
}
/*
================
c4tnt: variable creation layer checker
idTypeDef::GetLayer

Returns type layer id
================
*/
etypelayer_t idTypeDef::GetLayer( void ) const {
	if (layer < 0) return etl_deflevel;
	if (layer > etl_end) return etl_end;
	return layer;
}

/*
================
idTypeDef::MatchesType

Returns true if both types' base types and parameters match
================
*/
bool idTypeDef::MatchesType( const idTypeDef &matchtype ) const {
	int i;

	if ( this == &matchtype ) {
		return true;
	}

	if ( ( type != matchtype.type ) || ( auxType != matchtype.auxType ) ) {
		return false;
	}

	if ( parmTypes.Num() != matchtype.parmTypes.Num() ) {
		return false;
	}

	for( i = 0; i < matchtype.parmTypes.Num(); i++ ) {
		if ( parmTypes[ i ] != matchtype.parmTypes[ i ] ) {
			return false;
		}
	}

	return true;
}

/*
================
idTypeDef::MatchesVirtualFunction

Returns true if both functions' base types and parameters match
================
*/
bool idTypeDef::MatchesVirtualFunction( const idTypeDef &matchfunc ) const {
	int i;

	if ( this == &matchfunc ) {
		return true;
	}

	if ( ( type != matchfunc.type ) || ( auxType != matchfunc.auxType ) ) {
		return false;
	}

	if ( parmTypes.Num() != matchfunc.parmTypes.Num() ) {
		return false;
	}

	if ( parmTypes.Num() > 0 ) {
		if ( !parmTypes[ 0 ]->Inherits( matchfunc.parmTypes[ 0 ] ) ) {
			return false;
		}
	}

	for( i = 1; i < matchfunc.parmTypes.Num(); i++ ) {
		if ( parmTypes[ i ] != matchfunc.parmTypes[ i ] ) {
			return false;
		}
	}

	return true;
}

/*
================
idTypeDef::AddFunctionParm

Adds a new parameter for a function type.
================
*/
void idTypeDef::AddFunctionParm( idTypeDef *parmtype, const char *name ) {
	if ( type != ev_function ) {
		throw idCompileError( "idTypeDef::AddFunctionParm : tried to add parameter on non-function type" );
	}

	parmTypes.Append( parmtype );
	idStr &parmName = parmNames.Alloc();
	parmName = name;
}

/*
================
idTypeDef::AddField

Adds a new field to an object type.
================
*/
void idTypeDef::AddField( idTypeDef *fieldtype, const char *name ) {
	if ( type != ev_object ) {
		throw idCompileError( "idTypeDef::AddField : tried to add field to non-object type" );
	}

	parmTypes.Append( fieldtype );
	idStr &parmName = parmNames.Alloc();
	parmName = name;

	if ( fieldtype->FieldType()->Inherits( &type_object ) ) {
		size += type_object.Size();
	} else {
		size += fieldtype->FieldType()->Size();
	}
}

/*
================
idTypeDef::SetName
================
*/
void idTypeDef::SetName( const char *newname ) {
	name = newname;
}

/*
================
idTypeDef::Name
================
*/
const char *idTypeDef::Name( void ) const {
	return name;
}

/*
================
idTypeDef::Type
================
*/
etype_t idTypeDef::Type( void ) const {
	return type;
}

/*
================
idTypeDef::Size
================
*/
int idTypeDef::Size( void ) const {
	return size;
}

/*
================
idTypeDef::SuperClass

If type is an object, then returns the object's superclass
================
*/
idTypeDef *idTypeDef::SuperClass( void ) const {
	if ( type != ev_object ) {
		throw idCompileError( "idTypeDef::SuperClass : tried to get superclass of a non-object type" );
	}

	return auxType;
}

/*
================
idTypeDef::ReturnType

If type is a function, then returns the function's return type
================
*/
idTypeDef *idTypeDef::ReturnType( void ) const {
	if ( type != ev_function ) {
		throw idCompileError( "idTypeDef::ReturnType: tried to get return type on non-function type" );
	}

	return auxType;
}

/*
================
idTypeDef::SetReturnType

If type is a function, then sets the function's return type
================
*/
void idTypeDef::SetReturnType( idTypeDef *returntype ) {
	if ( type != ev_function ) {
		throw idCompileError( "idTypeDef::SetReturnType: tried to set return type on non-function type" );
	}

	auxType = returntype;
}

/*
================
idTypeDef::RightType

returns a return type for fields and functions
================
*/
idTypeDef *idTypeDef::RightType( void ) {
	if ( type == ev_field || type == ev_function || type == ev_pointer || type == ev_list && type == ev_array) {
		return auxType;
	}

	return this;
}

/*
================
idTypeDef::FieldType

If type is a field, then returns it's type
================
*/
idTypeDef *idTypeDef::FieldType( void ) const {
	if ( type != ev_field ) {
		throw idCompileError( "idTypeDef::FieldType: tried to get field type on non-field type" );
	}

	return auxType;
}

/*
================
idTypeDef::SetFieldType

If type is a field, then sets the function's return type
================
*/
void idTypeDef::SetFieldType( idTypeDef *fieldtype ) {
	if ( type != ev_field ) {
		throw idCompileError( "idTypeDef::SetFieldType: tried to set return type on non-function type" );
	}

	auxType = fieldtype;
}

/*
================
idTypeDef::PointerType

If type is a pointer, then returns the type it points to
================
*/
idTypeDef *idTypeDef::PointerType( void ) const {
	if ( type != ev_pointer ) {
		throw idCompileError( "idTypeDef::PointerType: tried to get pointer type on non-pointer" );
	}

	return auxType;
}

/*
================
idTypeDef::SetPointerType

If type is a pointer, then sets the pointer's type
================
*/
void idTypeDef::SetPointerType( idTypeDef *pointertype ) {
	if ( type != ev_pointer ) {
		throw idCompileError( "idTypeDef::SetPointerType: tried to set type on non-pointer" );
	}

	auxType = pointertype;
}

/*
================
idTypeDef::ListType

If type is a list or array, then returns the type it points to
================
*/
idTypeDef *idTypeDef::ListType( void ) const {
	if ( type != ev_list && type != ev_array ) {
		throw idCompileError( "idTypeDef::ListType: tried to get list type on non-list" );
	}

	return auxType;
}

/*
================
c4tnt
idTypeDef::SetListType

If type is a pointer, then sets the pointer's type
================
*/
void idTypeDef::SetListType( idTypeDef *listtype ) {
	if ( type != ev_list && type != ev_array ) {
		throw idCompileError( "idTypeDef::SetListType: tried to set type on non-list" );
	}

	auxType = listtype;
}

/*
================
idTypeDef::NumParameters
================
*/
int idTypeDef::NumParameters( void ) const {
	return parmTypes.Num();
}

/*
================
idTypeDef::GetParmType
================
*/
idTypeDef *idTypeDef::GetParmType( int parmNumber ) const {
	assert( parmNumber >= 0 );
	assert( parmNumber < parmTypes.Num() );
	return parmTypes[ parmNumber ];
}

/*
================
idTypeDef::GetParmName
================
*/
const char *idTypeDef::GetParmName( int parmNumber ) const {
	assert( parmNumber >= 0 );
	assert( parmNumber < parmTypes.Num() );
	return parmNames[ parmNumber ];
}


bool idTypeDef::CheckParmFmt( const char* fmt ) const {

int arg;
int argc;
const char* pos;

	if (!fmt)
		return false;
	
	argc = parmTypes.Num();
	pos = fmt;

	for ( arg = 0; arg < argc; arg++ ) {
		switch (*pos)
		{
		case 'b':
		case 'B':
			if ( parmTypes[ arg ]->Type() != ev_boolean )
				return false;
			break;
		case 'f':
		case 'F':
			if ( parmTypes[ arg ]->Type() != ev_float )
				return false;
			break;
		case 's':
		case 'S':
			if ( parmTypes[ arg ]->Type() != ev_string )
				return false;
			break;
		case 'e':
		case 'E':
			if ( parmTypes[ arg ]->Type() != ev_entity )
				return false;
			break;
		case 'v':
		case 'V':
			if ( parmTypes[ arg ]->Type() != ev_vector )
				return false;
			break;
		case 'l':
		case 'L':
			if ( parmTypes[ arg ]->Type() != ev_list )
				return false;
			break;
		case 'a':
		case 'A':
			if ( parmTypes[ arg ]->Type() != ev_array )
				return false;
			break;
		case 0:
			return false;
		}
		pos++;
	}
	if (*pos)
		return false;
	
	return true;
}

/*
================
idTypeDef::NumFunctions
================
*/
int idTypeDef::NumFunctions( void ) const {
	return functions.Num();
}

/*
================
idTypeDef::GetFunctionNumber
================
*/
int idTypeDef::GetFunctionNumber( const function_t *func ) const {
	int i;

	for( i = 0; i < functions.Num(); i++ ) {
		if ( functions[ i ] == func ) {
			return i;
		}
	}
	return -1;
}

/*
================
idTypeDef::GetFunction
================
*/
const function_t *idTypeDef::GetFunction( int funcNumber ) const {
	assert( funcNumber >= 0 );
	assert( funcNumber < functions.Num() );
	return functions[ funcNumber ];
}

/*
================
idTypeDef::AddFunction
================
*/
void idTypeDef::AddFunction( const function_t *func ) {
	int i;

	for( i = 0; i < functions.Num(); i++ ) {
		if ( !strcmp( functions[ i ]->def->Name(), func->def->Name() ) ) {
			if ( func->def->TypeDef()->MatchesVirtualFunction( *functions[ i ]->def->TypeDef() ) ) {
				functions[ i ] = func;
				return;
			}
		}
	}
	functions.Append( func );
}

/***********************************************************************

  idVarDef

***********************************************************************/

/*
================
idVarDef::idVarDef()
================
*/
idVarDef::idVarDef( idTypeDef *typeptr ) {
	typeDef		= typeptr;
	num			= 0;
	scope		= NULL;
	numUsers	= 0;
	initialized = idVarDef::uninitialized;
	memset( &value, 0, sizeof( value ) );
	name		= NULL;
	next		= NULL;
}

/*
============
idVarDef::~idVarDef
============
*/
idVarDef::~idVarDef() {
	if ( name ) {
		name->RemoveDef( this );
	}
}

/*
============
idVarDef::Name
============
*/
const char *idVarDef::Name( void ) const {
	return name->Name();
}

/*
============
idVarDef::GlobalName
============
*/
const char *idVarDef::GlobalName( void ) const {
	if ( scope != &def_namespace ) {
		return va( "%s::%s", scope->GlobalName(), name->Name() );
	} else {
		return name->Name();
	}
}

/*
============
idVarDef::DepthOfScope
============
*/
int idVarDef::DepthOfScope( const idVarDef *otherScope ) const {
	const idVarDef *def;
	int depth;

	depth = 1;
	for( def = otherScope; def != NULL; def = def->scope ) {
		if ( def == scope ) {
			return depth;
		}
		depth++;
	}

	return 0;
}

/*
============
idVarDef::SetFunction
============
*/
void idVarDef::SetFunction( function_t *func ) {
	assert( typeDef );
	initialized = initializedConstant;
	assert( typeDef->Type() == ev_function );
	value.functionPtr = func;
}

/*
============
idVarDef::SetObject
============
*/
void idVarDef::SetObject( idScriptObject *object ) {
	assert( typeDef );
	initialized = initialized;
	assert( typeDef->Inherits( &type_object ) );
	*value.objectPtrPtr = object;
}

/*
============
idVarDef::SetValue
============
*/
void idVarDef::SetValue( const eval_t &_value, bool constant ) {
	assert( typeDef );
	if ( constant ) {
		initialized = initializedConstant;
	} else {
		initialized = initializedVariable;
	}

	switch( typeDef->Type() ) {
	case ev_pointer :
	case ev_boolean :
	case ev_field :
		*value.intPtr = _value._int;
		break;

	case ev_jumpoffset :
		value.jumpOffset = _value._int;
		break;

	case ev_argsize :
		value.argSize = _value._int;
		break;

	case ev_entity :
		*value.entityNumberPtr = _value.entity;
		break;

	case ev_string :
		idStr::Copynz( value.stringPtr, _value.stringPtr, MAX_STRING_LEN );
		break;

	case ev_float :
		*value.floatPtr = _value._float;
		break;

	case ev_vector :
		value.vectorPtr->x = _value.vector[ 0 ];
		value.vectorPtr->y = _value.vector[ 1 ];
		value.vectorPtr->z = _value.vector[ 2 ];
		break;

	case ev_function :
		value.functionPtr = _value.function;
		break;

	case ev_virtualfunction :
		value.virtualFunction = _value._int;
		break;

	case ev_object :
		*value.entityNumberPtr = _value.entity;
		break;

	default :
		throw idCompileError( va( "weird type on '%s'", Name() ) );
		break;
	}
}

/*
============
idVarDef::SetString
============
*/
void idVarDef::SetString( const char *string, bool constant ) {
	if ( constant ) {
		initialized = initializedConstant;
	} else {
		initialized = initializedVariable;
	}
	
	assert( typeDef && ( typeDef->Type() == ev_string ) );
	idStr::Copynz( value.stringPtr, string, MAX_STRING_LEN );
}

/*
============
idVarDef::PrintInfo
c4tnt: idStr version
============
*/
void idVarDef::PrintInfo( idStr &str, int instructionPointer ) const {
	statement_t	*jumpst;
	int			jumpto;
	etype_t		etype;
	int			i;
	int			len;
	const char	*ch;
	idStr		tmps;

	if ( initialized == initializedConstant ) {
		str += "const ";
	}

	etype = typeDef->Type();
	switch( etype ) {
	case ev_jumpoffset :
		jumpto = instructionPointer + value.jumpOffset;
		jumpst = &gameLocal.program.GetStatement( jumpto );

		sprintf( tmps, "address %d [%s(%d)]", jumpto, gameLocal.program.GetFilename( jumpst->file ), jumpst->linenumber );
		str += tmps;
		break;

	case ev_function :
		if ( value.functionPtr->eventdef ) {
			sprintf( tmps, "event %s", GlobalName() );
		} else {
			sprintf( tmps, "function %s", GlobalName() );
		}
		str += tmps;
		break;

	case ev_field :
		sprintf( tmps, "field %d", value.ptrOffset );
		str += tmps;
		break;

	case ev_argsize:
		sprintf( tmps, "args %d", value.argSize );
		str += tmps;
		break;

	case ev_array:
		
		if ( initialized == stackVariable ) {
			sprintf( tmps, "array s[%d] <%s>", value.stackOffset, typeDef->ListType()->Name() );
		} else {
			sprintf( tmps, "array g[%d] <%s>", num, typeDef->ListType()->Name() );
		}
		str += tmps;
		break;

	default:
		sprintf( tmps, "%s ", typeDef->Name() );
		str += tmps;

		if ( initialized == initializedConstant ) {
			switch( etype ) {
			case ev_string :
				str += "\"";
				len = strlen( value.stringPtr );
				ch = value.stringPtr;
				for( i = 0; i < len; i++, ch++ ) {
					if ( idStr::CharIsPrintable( *ch ) ) {
						sprintf( tmps, "%c", *ch );
					} else if ( *ch == '\n' ) {
						sprintf( tmps, "\\n" );
					} else {
						sprintf( tmps, "\\x%.2x", static_cast<int>( *ch ) );
					}
					str += tmps;
				}
				str += "\"";
				break;

			case ev_vector :
				sprintf( tmps, "'%s'", value.vectorPtr->ToString() );
				str += tmps;
				break;

			case ev_float :
				sprintf( tmps, "%f", *value.floatPtr );
				str += tmps;
				break;

			case ev_virtualfunction :
				sprintf( tmps, "vtable[ %d ]", value.virtualFunction );
				str += tmps;
				break;

			default :
				sprintf( tmps, "%d", *value.intPtr );
				str += tmps;
				break;
			}
		} else if ( initialized == stackVariable ) {
			sprintf( tmps, "stack[%d]", value.stackOffset );
			str += tmps;
		} else if (!strcmp(Name(),RETURN_STRING)) {
			str += RETURN_STRING;
		} else {
			sprintf( tmps, "global[%d]", num );
			str += tmps;
		}
		break;
	}
}

/*
============
idVarDef::PrintInfo
============
*/
void idVarDef::PrintInfo( idFile *file, int instructionPointer ) const {
	statement_t	*jumpst;
	int			jumpto;
	etype_t		etype;
	int			i;
	int			len;
	const char	*ch;

	if ( initialized == initializedConstant ) {
		file->Printf( "const " );
	}

	etype = typeDef->Type();
	switch( etype ) {
	case ev_jumpoffset :
		jumpto = instructionPointer + value.jumpOffset;
		jumpst = &gameLocal.program.GetStatement( jumpto );
		file->Printf( "address %d [%s(%d)]", jumpto, gameLocal.program.GetFilename( jumpst->file ), jumpst->linenumber );
		break;

	case ev_function :
		if ( value.functionPtr->eventdef ) {
			file->Printf( "event %s", GlobalName() );
		} else {
			file->Printf( "function %s", GlobalName() );
		}
		break;

	case ev_field :
		file->Printf( "field %d", value.ptrOffset );
		break;

	case ev_argsize:
		file->Printf( "args %d", value.argSize );
		break;

	case ev_array:
		
		if ( initialized == stackVariable ) {
			file->Printf( "array on stack[%d] <%s>", value.stackOffset, typeDef->ListType()->Name() );
		} else {
			file->Printf( "array on global[%d] <%s>", num, typeDef->ListType()->Name() );
		}

		break;

	default:
		file->Printf( "%s ", typeDef->Name() );
		if ( initialized == initializedConstant ) {
			switch( etype ) {
			case ev_string :
				file->Printf( "\"" );
				len = strlen( value.stringPtr );
				ch = value.stringPtr;
				for( i = 0; i < len; i++, ch++ ) {
					if ( idStr::CharIsPrintable( *ch ) ) {
						file->Printf( "%c", *ch );
					} else if ( *ch == '\n' ) {
						file->Printf( "\\n" );
					} else {
						file->Printf( "\\x%.2x", static_cast<int>( *ch ) );
					}
				}
				file->Printf( "\"" );
				break;

			case ev_vector :
				file->Printf( "'%s'", value.vectorPtr->ToString() );
				break;

			case ev_float :
                file->Printf( "%f", *value.floatPtr );
				break;

			case ev_virtualfunction :
				file->Printf( "vtable[ %d ]", value.virtualFunction );
				break;

			default :
				file->Printf( "%d", *value.intPtr );
				break;
			}
		} else if ( initialized == stackVariable ) {
			file->Printf( "stack[%d]", value.stackOffset );
		} else if (!strcmp(Name(),RETURN_STRING)) {
			file->Printf( RETURN_STRING );
		} else {
			file->Printf( "global[%d]", num );
		}
		break;
	}
}

/***********************************************************************

  idVarDef

***********************************************************************/

/*
============
idVarDefName::AddDef
============
*/
void idVarDefName::AddDef( idVarDef *def ) {
	assert( def->next == NULL );
	def->name = this;
	def->next = defs;
	defs = def;
}

/*
============
idVarDefName::RemoveDef
============
*/
void idVarDefName::RemoveDef( idVarDef *def ) {
	if ( defs == def ) {
		defs = def->next;
	} else {
		for ( idVarDef *d = defs; d->next != NULL; d = d->next ) {
			if ( d->next == def ) {
				d->next = def->next;
				break;
			}
		}
	}
	def->next = NULL;
	def->name = NULL;
}

/***********************************************************************

  idScriptObject

***********************************************************************/

/*
============
idScriptObject::idScriptObject
============
*/
idScriptObject::idScriptObject() {
	data = NULL;
	type = &type_object;
}

/*
============
idScriptObject::~idScriptObject
============
*/
idScriptObject::~idScriptObject() {
	Free();
}

/*
============
idScriptObject::Free
============
*/
void idScriptObject::Free( void ) {
	if ( data ) {
		Mem_Free( data );
	}

	data = NULL;
	type = &type_object;
}

/*
================
idScriptObject::Save
================
*/
void idScriptObject::Save( idSave_I *savefile ) const {
	size_t size;

	if ( type == &type_object && data == NULL ) {
		// Write empty string for uninitialized object
		savefile->WriteString( "" );
	} else {
		savefile->WriteString( type->Name() );
		size = type->Size();
		savefile->WriteRLE( data, size );	//c4tnt: Some compression
	}
}

/*
================
idScriptObject::Restore
================
*/
void idScriptObject::Restore( idRestore_I *savefile ) {
	idStr typeName;

	savefile->ReadString( typeName );

	// Empty string signals uninitialized object
	if ( typeName.Length() == 0 ) {
		return;
	}

	if ( !SetType( typeName ) ) {
		savefile->Error( "idScriptObject::Restore: failed to restore object of type '%s'.", typeName.c_str() );
	}

	if ( !savefile->ReadRLE( data, type->Size() ) ) {
		savefile->Error( "idScriptObject::Restore: couldn't decompress object '%s' from save game.", typeName.c_str() );
	}
}

/*
============
idScriptObject::SetType

Allocates an object and initializes memory.
============
*/
bool idScriptObject::SetType( const char *typeName ) {
	size_t size;
	idTypeDef *newtype;

	// lookup the type
	newtype = gameLocal.program.FindType( typeName );

	// only allocate memory if the object type changes
	if ( newtype != type ) {	
		Free();
		if ( !newtype ) {
			gameLocal.Warning( "idScriptObject::SetType: Unknown type '%s'", typeName );
			return false;
		}

		if ( !newtype->Inherits( &type_object ) ) {
			gameLocal.Warning( "idScriptObject::SetType: Can't create object of type '%s'.  Must be an object type.", newtype->Name() );
			return false;
		}

		// set the type
		type = newtype;

		// allocate the memory
		size = type->Size();
		data = ( byte * )Mem_Alloc( size );
	}

	// init object memory
	ClearObject();

	return true;
}

/*
============
idScriptObject::ClearObject

Resets the memory for the script object without changing its type.
============
*/
void idScriptObject::ClearObject( void ) {
	size_t size;

	if ( type != &type_object ) {
		// init object memory
		size = type->Size();
		memset( data, 0, size );
	}
}

/*
============
idScriptObject::HasObject
============
*/
bool idScriptObject::HasObject( void ) const {
	return ( type != &type_object );
}

/*
============
idScriptObject::GetTypeDef
============
*/
idTypeDef *idScriptObject::GetTypeDef( void ) const {
	return type;
}

/*
============
idScriptObject::GetTypeName
============
*/
const char *idScriptObject::GetTypeName( void ) const {
	return type->Name();
}

/*
============
idScriptObject::GetConstructor
============
*/
const function_t *idScriptObject::GetConstructor( void ) const {
	const function_t *func;

	func = GetFunction( "init" );
	return func;
}

/*
============
idScriptObject::GetDestructor
============
*/
const function_t *idScriptObject::GetDestructor( void ) const {
	const function_t *func;

	func = GetFunction( "destroy" );
	return func;
}

/*
============
idScriptObject::GetFunction
============
*/
const function_t *idScriptObject::GetFunction( const char *name ) const {
	const function_t *func;

	if ( type == &type_object ) {
		return NULL;
	}

	func = gameLocal.program.FindFunction( name, type );
	return func;
}

/*
============
idScriptObject::GetVariable
============
*/
byte *idScriptObject::GetVariable( const char *name, etype_t etype ) const {
	int				i;
	int				pos;
	const idTypeDef	*t;
	const idTypeDef	*parm;

	if ( type == &type_object ) {
		return NULL;
	}

	t = type;
	do {
		if ( t->SuperClass() != &type_object ) {
			pos = t->SuperClass()->Size();
		} else {
			pos = 0;
		}
		for( i = 0; i < t->NumParameters(); i++ ) {
			parm = t->GetParmType( i );
			if ( !strcmp( t->GetParmName( i ), name ) ) {
				if ( etype != parm->FieldType()->Type() ) {
					return NULL;
				}
				return &data[ pos ];
			}

			if ( parm->FieldType()->Inherits( &type_object ) ) {
				pos += type_object.Size();
			} else {
				pos += parm->FieldType()->Size();
			}
		}
		t = t->SuperClass();
	} while( t && ( t != &type_object ) );

	return NULL;
}

/***********************************************************************

  idProgram

***********************************************************************/

/*
============
idProgram::AllocType
============
*/
idTypeDef *idProgram::AllocType( idTypeDef &type ) {
	idTypeDef *newtype;

	newtype	= new idTypeDef( type ); 
	types.Append( newtype );

	return newtype;
}

/*
============
idProgram::AllocType
============
*/
idTypeDef *idProgram::AllocType( etype_t etype, idVarDef *edef, const char *ename, int esize, idTypeDef *aux, etypelayer_t vlayer ) {
	idTypeDef *newtype;

	newtype	= new idTypeDef( etype, edef, ename, esize, aux, vlayer );
	types.Append( newtype );

	return newtype;
}

/*
============
idProgram::GetType

Returns a preexisting complex type that matches the parm, or allocates
a new one and copies it out.
============
*/
idTypeDef *idProgram::GetType( idTypeDef &type, bool allocate ) {
	int i;

	//FIXME: linear search == slow
	for( i = types.Num() - 1; i >= 0; i-- ) {
		if ( types[ i ]->MatchesType( type ) && !strcmp( types[ i ]->Name(), type.Name() ) ) {
			return types[ i ];
		}
	}

	if ( !allocate ) {
		return NULL;
	}

	// allocate a new one
	return AllocType( type );
}

/*
============
idProgram::FindType

Returns a preexisting complex type that matches the name, or returns NULL if not found
============
*/
idTypeDef *idProgram::FindType( const char *name ) {
	idTypeDef	*check;
	int			i;

	for( i = types.Num() - 1; i >= 0; i-- ) {
		check = types[ i ];
		if ( !strcmp( check->Name(), name ) ) {
			return check;
		}
	}

	return NULL;
}

/*
============
idProgram::GetDefList
============
*/
idVarDef *idProgram::GetDefList( const char *name ) const {
	int i, hash;

	hash = varDefNameHash.GenerateKey( name, true );
	for ( i = varDefNameHash.First( hash ); i != -1; i = varDefNameHash.Next( i ) ) {
		if ( idStr::Cmp( varDefNames[i]->Name(), name ) == 0 ) {
			return varDefNames[i]->GetDefs();
		}
	}
	return NULL;
}

/*
============
idProgram::AddDefToNameList
============
*/
void idProgram::AddDefToNameList( idVarDef *def, const char *name ) {
	int i, hash;

	hash = varDefNameHash.GenerateKey( name, true );
	for ( i = varDefNameHash.First( hash ); i != -1; i = varDefNameHash.Next( i ) ) {
		if ( idStr::Cmp( varDefNames[i]->Name(), name ) == 0 ) {
			break;
		}
	}
	if ( i == -1 ) {
		i = varDefNames.Append( new idVarDefName( name ) );
		varDefNameHash.Add( hash, i );
	}
	varDefNames[i]->AddDef( def );
}

/*
============
idProgram::AllocDef
============
*/
idVarDef *idProgram::AllocDef( idTypeDef *type, const char *name, idVarDef *scope, bool constant ) {
	idVarDef	*def;
	idStr		element;
	idVarDef	*def_x;
	idVarDef	*def_y;
	idVarDef	*def_z;

	// allocate a new def
	def = new idVarDef( type );
	def->scope		= scope;
	def->numUsers	= 1;
	def->num		= varDefs.Append( def );

	// add the def to the list with defs with this name and set the name pointer
	AddDefToNameList( def, name );

	if ( ( type->Type() == ev_vector ) || ( ( type->Type() == ev_field ) && ( type->FieldType()->Type() == ev_vector ) ) ) {
		//
		// vector
		//
		if ( !strcmp( name, RESULT_STRING ) ) {
			// <RESULT> vector defs don't need the _x, _y and _z components
			assert( scope->Type() == ev_function );
			def->value.stackOffset	= scope->value.functionPtr->locals;
			def->initialized		= idVarDef::stackVariable;
			scope->value.functionPtr->locals += type->Size();
		} else if ( scope->TypeDef()->Inherits( &type_object ) ) {
			idTypeDef	newtype( ev_field, NULL, "float field", 0, &type_float );
			idTypeDef	*type = GetType( newtype, true );

			// set the value to the variable's position in the object
			def->value.ptrOffset = scope->TypeDef()->Size();

			// make automatic defs for the vectors elements
			// origin can be accessed as origin_x, origin_y, and origin_z
			sprintf( element, "%s_x", def->Name() );
			def_x = AllocDef( type, element, scope, constant );

			sprintf( element, "%s_y", def->Name() );
			def_y = AllocDef( type, element, scope, constant );
			def_y->value.ptrOffset = def_x->value.ptrOffset + type_float.Size();

			sprintf( element, "%s_z", def->Name() );
			def_z = AllocDef( type, element, scope, constant );
			def_z->value.ptrOffset = def_y->value.ptrOffset + type_float.Size();
		} else {
			// make automatic defs for the vectors elements
			// origin can be accessed as origin_x, origin_y, and origin_z
			sprintf( element, "%s_x", def->Name() );
			def_x = AllocDef( &type_float, element, scope, constant );

			sprintf( element, "%s_y", def->Name() );
			def_y = AllocDef( &type_float, element, scope, constant );

			sprintf( element, "%s_z", def->Name() );
			def_z = AllocDef( &type_float, element, scope, constant );

			// point the vector def to the x coordinate
			def->value			= def_x->value;
			def->initialized	= def_x->initialized;
		}
	} else if ( scope->TypeDef()->Inherits( &type_object ) ) {
		//
		// object variable
		//
		// set the value to the variable's position in the object
		def->value.ptrOffset = scope->TypeDef()->Size();
	} else if ( scope->Type() == ev_function ) {
		//
		// stack variable
		//
		// since we don't know how many local variables there are,
		// we have to have them go backwards on the stack
		def->value.stackOffset	= scope->value.functionPtr->locals;
		def->initialized		= idVarDef::stackVariable;

		if ( type->Inherits( &type_object ) ) {
			// objects only have their entity number on the stack, not the entire object
			scope->value.functionPtr->locals += type_object.Size();
		} else {
			scope->value.functionPtr->locals += type->Size();
		}
	} else {
		//
		// global variable
		//
		def->value.bytePtr = &variables[ numVariables ];
		numVariables += def->TypeDef()->Size();
		if ( numVariables > sizeof( variables ) ) {
			throw idCompileError( va( "Exceeded global memory size (%d bytes)", sizeof( variables ) ) );
		}

		memset( def->value.bytePtr, 0, def->TypeDef()->Size() );
	}

	return def;
}

/*
============
idProgram::AllocDef
============
*/
idVarDef *idProgram::AllocDefferedDef( idTypeDef *type, const char *name, idVarDef *scope, bool constant ) {
	idVarDef	*def;
	idStr		element;

	if ( !strcmp( name, RESULT_STRING ) ) {
		return NULL;
	}

	// allocate a new def
	def = new idVarDef( type );
	def->scope		= scope;
	def->numUsers	= 1;
	def->num		= varDefs.Append( def );

	// add the def to the list with defs with this name and set the name pointer
	AddDefToNameList( def, name );

	if ( ( type->Type() == ev_vector ) || ( ( type->Type() == ev_field ) && ( type->FieldType()->Type() == ev_vector ) ) ) {
		//
		// vector
		//
		assert( scope->Type() == ev_function );
		def->value.stackOffset	= 0;
		def->initialized		= idVarDef::stackVariable;
	} else if ( scope->TypeDef()->Inherits( &type_object ) ) {
		//
		// object variable
		//
		// set the value to the variable's position in the object
		def->value.ptrOffset = scope->TypeDef()->Size();
	} else if ( scope->Type() == ev_function ) {
		//
		// stack variable
		//
		// since we don't know how many local variables there are,
		// we have to have them go backwards on the stack
		def->value.stackOffset	= 0; //scope->value.functionPtr->locals;
		def->initialized		= idVarDef::stackVariable;

	} else {
		//
		// global variable
		//
		def->value.bytePtr = NULL;
	}

	return def;
}

/*
============
idProgram::GetDef

If type is NULL, it will match any type
============
*/
idVarDef *idProgram::GetDef( const idTypeDef *type, const char *name, const idVarDef *scope ) const {
	idVarDef		*def;
	idVarDef		*bestDef;
	int				bestDepth;
	int				depth;

	bestDepth = 0;
	bestDef = NULL;
	for( def = GetDefList( name ); def != NULL; def = def->Next() ) {
		if ( def->scope->Type() == ev_namespace ) {
			depth = def->DepthOfScope( scope );
			if ( !depth ) {
				// not in the same namespace
				continue;
			}
		} else if ( def->scope != scope ) {
			// in a different function
			continue;
		} else {
			depth = 1;
		}

		if ( !bestDef || ( depth < bestDepth ) ) {
			bestDepth = depth;
			bestDef = def;
		}
	}

	// see if the name is already in use for another type
	if ( bestDef && type && ( bestDef->TypeDef() != type ) ) {
		throw idCompileError( va( "Type mismatch on redeclaration of %s", name ) );
	}

	return bestDef;
}

/*
============
idProgram::FreeDef
============
*/
void idProgram::FreeDef( idVarDef *def, const idVarDef *scope ) {
	idVarDef *e;
	int i;

	if ( def->Type() == ev_vector ) {
		idStr name;

		sprintf( name, "%s_x", def->Name() );
		e = GetDef( NULL, name, scope );
		if ( e ) {
			FreeDef( e, scope );
		}

		sprintf( name, "%s_y", def->Name() );
		e = GetDef( NULL, name, scope );
		if ( e ) {
			FreeDef( e, scope );
		}

		sprintf( name, "%s_z", def->Name() );
		e = GetDef( NULL, name, scope );
		if ( e ) {
			FreeDef( e, scope );
		}
	}

	varDefs.RemoveIndex( def->num );
	for( i = def->num; i < varDefs.Num(); i++ ) {
		varDefs[ i ]->num = i;
	}

	delete def;
}

/*
============
idProgram::FindFreeResultDef
============
*/
idVarDef *idProgram::FindFreeResultDef( idTypeDef *type, const char *name, idVarDef *scope, const idVarDef *a, const idVarDef *b ) {
	idVarDef *def;
	
	for( def = GetDefList( name ); def != NULL; def = def->Next() ) {
		if ( def == a || def == b ) {
			continue;
		}
		if ( def->TypeDef() != type ) {
			continue;
		}
		if ( def->scope != scope ) {
			continue;
		}
		if ( def->numUsers <= 1 ) {
			continue;
		}
		return def;
	}

	return AllocDef( type, name, scope, false );
}

/*
================
idProgram::FindFunction

Searches for the specified function in the currently loaded script.  A full namespace should be
specified if not in the global namespace.

Returns 0 if function not found.
Returns >0 if function found.
================
*/
function_t *idProgram::FindFunction( const char *name ) const {
	int			start;
	int			pos;
	idVarDef	*namespaceDef;
	idVarDef	*def;

	assert( name );

	idStr fullname = name;
	start = 0;
	namespaceDef = &def_namespace;
	do {
		pos = fullname.Find( "::", true, start );
		if ( pos < 0 ) {
			break;
		}

		idStr namespaceName = fullname.Mid( start, pos - start );
		def = GetDef( NULL, namespaceName, namespaceDef );
		if ( !def ) {
			// couldn't find namespace
			return NULL;
		}
		namespaceDef = def;

		// skip past the ::
		start = pos + 2;
	} while( def->Type() == ev_namespace );

	idStr funcName = fullname.Right( fullname.Length() - start );
	def = GetDef( NULL, funcName, namespaceDef );
	if ( !def ) {
		// couldn't find function
		return NULL;
	}

	if ( ( def->Type() == ev_function ) && ( def->value.functionPtr->eventdef == NULL ) ) {
		return def->value.functionPtr;
	}

	// is not a function, or is an eventdef
	return NULL;
}

/*
================
idProgram::FindFunction

Searches for the specified object function in the currently loaded script.

Returns 0 if function not found.
Returns >0 if function found.
================
*/
function_t *idProgram::FindFunction( const char *name, const idTypeDef *type ) const {
	const idVarDef	*tdef;
	const idVarDef	*def;

	// look for the function
	def = NULL;
	for( tdef = type->def; tdef != &def_object; tdef = tdef->TypeDef()->SuperClass()->def ) {
		def = GetDef( NULL, name, tdef );
		if ( def ) {
			return def->value.functionPtr;
		}
	}

	return NULL;
}

/*
================
idProgram::FindFunctionDef

Searches for the specified function in the currently loaded script.  A full namespace should be
specified if not in the global namespace.

Returns 0 if function not found.
Returns >0 if function found.
================
*/
const idVarDef *idProgram::FindFunctionDef( const char *name ) const {
	int			start;
	int			pos;
	idVarDef	*namespaceDef;
	idVarDef	*def;

	assert( name );

	idStr fullname = name;
	start = 0;
	namespaceDef = &def_namespace;
	do {
		pos = fullname.Find( "::", true, start );
		if ( pos < 0 ) {
			break;
		}

		idStr namespaceName = fullname.Mid( start, pos - start );
		def = GetDef( NULL, namespaceName, namespaceDef );
		if ( !def ) {
			// couldn't find namespace
			return NULL;
		}
		namespaceDef = def;

		// skip past the ::
		start = pos + 2;
	} while( def->Type() == ev_namespace );

	idStr funcName = fullname.Right( fullname.Length() - start );
	def = GetDef( NULL, funcName, namespaceDef );
	if ( !def ) {
		// couldn't find function
		return NULL;
	}

	if ( ( def->Type() == ev_function ) && ( def->value.functionPtr->eventdef == NULL ) ) {
		return def;
	}

	// is not a function, or is an eventdef
	return NULL;
}

/*
================
idProgram::FindFunctionDef

Searches for the specified object function in the currently loaded script.

Returns 0 if function not found.
Returns >0 if function found.
================
*/
const idVarDef *idProgram::FindFunctionDef( const char *name, const idTypeDef *type ) const {
	const idVarDef	*tdef;
	const idVarDef	*def;

	// look for the function
	def = NULL;
	for( tdef = type->def; tdef != &def_object; tdef = tdef->TypeDef()->SuperClass()->def ) {
		def = GetDef( NULL, name, tdef );
		if ( def ) {
			return def;
		}
	}

	return NULL;
}
/*
================
idProgram::AllocFunction
================
*/
function_t &idProgram::AllocFunction( idVarDef *def ) {
	if ( functions.Num() >= functions.Max() ) {
		throw idCompileError( va( "Exceeded maximum allowed number of functions (%d)", functions.Max() ) );
	}

	// fill in the dfunction
	function_t &func	= *functions.Alloc();
	func.eventdef		= NULL;
	func.def			= def;
	func.type			= def->TypeDef();
	func.firstStatement	= 0;
	func.numStatements	= 0;
	func.parmTotal		= 0;
	func.locals			= 0;
	func.filenum		= filenum;
	func.parmSize.SetGranularity( 1 );
	func.SetName( def->GlobalName() );

	def->SetFunction( &func );

	return func;
}

/*
================
idProgram::SetEntity
================
*/
void idProgram::SetEntity( const char *name, idEntity *ent ) {
	idVarDef	*def;
	idStr		defName( "$" );

	defName += name;

	def = GetDef( &type_entity, defName, &def_namespace );
	if ( def && ( def->initialized != idVarDef::stackVariable ) ) {
		// 0 is reserved for NULL entity
		if ( !ent ) {
			*def->value.entityNumberPtr = 0;
		} else {
			*def->value.entityNumberPtr = ent->entityNumber + 1;
		}
	}
}

/*
================
idProgram::AllocStatement
================
*/
statement_t *idProgram::AllocStatement( void ) {
	if ( statements.Num() >= statements.Max() ) {
		throw idCompileError( va( "Exceeded maximum allowed number of statements (%d)", statements.Max() ) );
	}
	return statements.Alloc();
}

/*
==============
idProgram::BeginCompilation

called before compiling a batch of files, clears the pr struct
==============
*/
void idProgram::BeginCompilation( void ) {
	statement_t	*statement;

	FreeData();

	try {
		// make the first statement a return for a "NULL" function
		statement = AllocStatement();
		statement->linenumber	= 0;
		statement->file 		= 0;
		statement->op			= OP_RETURN;
		statement->a			= NULL;
		statement->b			= NULL;
		statement->c			= NULL;

		// define NULL
		//AllocDef( &type_void, "<NULL>", &def_namespace, true );

		// define the return def
		//returnDef = AllocDef( &type_vector, RETURN_STRING, &def_namespace, false );

		// define the return def for strings
		//returnStringDef = AllocDef( &type_string, RETURN_STRING, &def_namespace, false );

		// define the sys object
		sysDef = AllocDef( &type_void, "sys", &def_namespace, true );
	}

	catch( idCompileError &err ) {
		gameLocal.Error( "%s", err.error );
	}
}

/*
==============
idProgram::DisassembleStatement
c4tnt: idStr version
==============
*/
void idProgram::DisassembleStatement( idStr &str, int instructionPointer ) const {
	opcode_t			*op;
	const statement_t	*statement;
	idStr				tmps;

	statement = &statements[ instructionPointer ];
	op = &idCompiler::opcodes[ statement->op ];
	
	sprintf(tmps, "%6d: %15s\t", instructionPointer, op->opname);
	str += tmps;

	if ( statement->a ) {
		str += "\ta: ";
		statement->a->PrintInfo( str, instructionPointer );
	}

	if ( statement->b ) {
		str += "\tb: ";
		statement->b->PrintInfo( str, instructionPointer );
	}

	if ( statement->c ) {
		str += "\tc: ";
		statement->c->PrintInfo( str, instructionPointer );
	}

	str += "\n";
}

/*
==============
idProgram::DisassembleStatement
==============
*/
void idProgram::DisassembleStatement( idFile *file, int instructionPointer ) const {
	opcode_t			*op;
	const statement_t	*statement;

	statement = &statements[ instructionPointer ];
	op = &idCompiler::opcodes[ statement->op ];
	file->Printf( "%20s(%d):\t%6d: %15s\t", fileList[ statement->file ].c_str(), statement->linenumber, instructionPointer, op->opname );

	if ( statement->a ) {
		file->Printf( "\ta: " );
		statement->a->PrintInfo( file, instructionPointer );
	}

	if ( statement->b ) {
		file->Printf( "\tb: " );
		statement->b->PrintInfo( file, instructionPointer );
	}

	if ( statement->c ) {
		file->Printf( "\tc: " );
		statement->c->PrintInfo( file, instructionPointer );
	}

	file->Printf( "\n" );
}

/*
==============
idProgram::Disassemble
==============
*/
int idProgram::Disassemble( const idStr *fname, bool toConsole ) const {//c4tnt: function names filter added
	int					i;
	int					instructionPointer;
	int					count;
	const function_t	*func;
	idFile				*file;
	idStr				tmps;

	if (!toConsole)	file = fileSystem->OpenFileByMode( "script/disasm.txt", FS_WRITE );

	count = 0;
	for( i = 0; i < functions.Num(); i++ ) {
		func = &functions[ i ];
		if ( func->eventdef ) {
			// skip eventdefs
			continue;
		}

		if ( ( fname == NULL ) || ( !fname->Icmpn(func->Name(),fname->Length() ) ) )
		{
			if (!toConsole) {
				file->Printf( "\nfunction %s() %d stack used, %d parms, %d locals {\n", func->Name(), func->locals, func->parmTotal, func->locals - func->parmTotal );
				for( instructionPointer = 0; instructionPointer < func->numStatements; instructionPointer++ ) {
					DisassembleStatement( file, func->firstStatement + instructionPointer );
				}
				file->Printf( "}\n" );
			}else{
				gameLocal.Printf( "\nfunction %s() %d stack used, %d parms, %d locals {\n", func->Name(), func->locals, func->parmTotal, func->locals - func->parmTotal );
				tmps.Clear();
				for( instructionPointer = 0; instructionPointer < func->numStatements; instructionPointer++ ) {
					DisassembleStatement( tmps, func->firstStatement + instructionPointer );
				}
				gameLocal.Printf( "%s}\n", tmps.c_str() );
			}
			count++;
		}
	}

	if (!toConsole) fileSystem->CloseFile( file );
	return count;
}

/*
==============
idProgram::FinishCompilation

Called after all files are compiled to check for errors
==============
*/
void idProgram::FinishCompilation( void ) {
	int	i;

	top_functions	= functions.Num();
	top_statements	= statements.Num();
	top_types		= types.Num();
	top_defs		= varDefs.Num();
	top_files		= fileList.Num();

	variableDefaults.Clear();
	variableDefaults.SetNum( numVariables );

	for( i = 0; i < numVariables; i++ ) {
		variableDefaults[ i ] = variables[ i ];
	}
}

/*
==============
idProgram::CompileStats

called after all files are compiled to report memory usage.
==============
*/
void idProgram::CompileStats( void ) {
	int	memused;
	int	memallocated;
	int	numdefs;
	int	stringspace;
	int funcMem;
	int	i;

	gameLocal.Printf( "---------- Compile stats ----------\n" );
	gameLocal.DPrintf( "Files loaded:\n" );

	stringspace = 0;
	for( i = 0; i < fileList.Num(); i++ ) {
		gameLocal.DPrintf( "   %s\n", fileList[ i ].c_str() );
		stringspace += fileList[ i ].Allocated();
	}
	stringspace += fileList.Size();

	numdefs = varDefs.Num();
	memused = varDefs.Num() * sizeof( idVarDef );
	memused += types.Num() * sizeof( idTypeDef );
	memused += stringspace;

	for( i = 0; i < types.Num(); i++ ) {
		memused += types[ i ]->Allocated();
	}

	funcMem = functions.MemoryUsed();
	for( i = 0; i < functions.Num(); i++ ) {
		funcMem += functions[ i ].Allocated();
	}

	memallocated = funcMem + memused + sizeof( idProgram );

	memused += statements.MemoryUsed();
	memused += functions.MemoryUsed();	// name and filename of functions are shared, so no need to include them
	memused += sizeof( variables );

	gameLocal.Printf( "\nMemory usage:\n" );
	gameLocal.Printf( "     Strings: %d, %d bytes\n", fileList.Num(), stringspace );
	gameLocal.Printf( "  Statements: %d, %d bytes\n", statements.Num(), statements.MemoryUsed() );
	gameLocal.Printf( "   Functions: %d, %d bytes\n", functions.Num(), funcMem );
	gameLocal.Printf( "   Variables: %d bytes\n", numVariables );
	gameLocal.Printf( "    Mem used: %d bytes\n", memused );
	gameLocal.Printf( " Static data: %d bytes\n", sizeof( idProgram ) );
	gameLocal.Printf( "   Allocated: %d bytes\n", memallocated );
	gameLocal.Printf( " Thread size: %d bytes\n\n", sizeof( idThread ) );
}

/*
================
idProgram::CompileText
================
*/
bool idProgram::CompileText( const char *source, const char *text, bool console, bool allowCVars ) {
	int			i;
	idVarDef	*def;
	idStr		ospath;
	idParser	parser;
	idTimer compile_time;

	// use a full os path for GetFilenum since it calls OSPathToRelativePath to convert filenames from the parser
	ospath = fileSystem->RelativePathToOSPath( source );
	filenum = GetFilenum( ospath );

	idCompiler	compiler(this, &parser, allowCVars);

	compile_time.Start();
	try {
		parser.SetFlags( LEXFL_ALLOWMULTICHARLITERALS );
		
		parser.LoadMemory( text, strlen( text ), filename );	//Load file
		parser.AddDefine( SCRIPT_DLRHEAD " " SCRIPT_DLRVER ); //DLR Marker
		parser.Immediate_include ( SCRIPT_DEFAULTDEFS, true );	//Include doom_main

		compiler.CompileFile( console );

		// check to make sure all functions prototyped have code
		for( i = 0; i < varDefs.Num(); i++ ) {
			def = varDefs[ i ];
			if ( ( def->Type() == ev_function ) && ( ( def->scope->Type() == ev_namespace ) || def->scope->TypeDef()->Inherits( &type_object ) ) ) {
				if ( !def->value.functionPtr->eventdef && !def->value.functionPtr->firstStatement ) {
					throw idCompileError( va( "function %s was not defined\n", def->GlobalName() ) );
				}
			}
		}
		
	}
	
	catch( idCompileError &err ) {
		compile_time.Stop();
		if ( console ) {
			gameLocal.Printf( "%s\n", err.error );
			return false;
		} else {
			gameLocal.Error( "%s\n", err.error );
		}
	};

	compile_time.Stop();
	if ( !console ) {
		gameLocal.Printf( "Compiled '%s': %.1f ms\n", source, compile_time.Milliseconds() );
		CompileStats();
	}

	return true;
}

/*
================
idProgram::CompileFunction
================
*/
const function_t *idProgram::CompileFunction( const char *functionName, const char *text ) {
	bool result;

	result = CompileText( functionName, text, false );
	gameLocal.Error( "after compile text.\n" );

	if ( g_disasm.GetBool() ) {
		Disassemble();
	}

	if ( !result ) {
		gameLocal.Error( "Compile failed." );
	}

	return FindFunction( functionName );
}

/*
================
idProgram::CompileFile
================
*/
void idProgram::CompileFile( const char *filename, bool allowCVars ) {
	char *src;
	bool result;

	if ( fileSystem->ReadFile( filename, ( void ** )&src, NULL ) < 0 ) {
		gameLocal.Error( "Couldn't load %s\n", filename );
	}

	result = CompileText( filename, src, false, allowCVars );

	fileSystem->FreeFile( src );

	if ( g_disasm.GetBool() ) {
		Disassemble();
	}

	if ( !result ) {
		gameLocal.Error( "Compile failed in file %s.", filename );
	}
}

/*
================
idProgram::FreeData
================
*/
void idProgram::FreeData( void ) {
	int i;

	// free the defs
	varDefs.DeleteContents( true );
	varDefNames.DeleteContents( true );
	varDefNameHash.Free();

//	returnDef		= NULL;
//	returnStringDef = NULL;
	sysDef			= NULL;

	// free any special types we've created
	types.DeleteContents( true );

	filenum = 0;

	numVariables = 0;
	memset( variables, 0, sizeof( variables ) );

	// clear all the strings in the functions so that it doesn't look like we're leaking memory.
	for( i = 0; i < functions.Num(); i++ ) {
		functions[ i ].Clear();
	}

	filename.Clear();
	fileList.Clear();
	statements.Clear();
	functions.Clear();

	top_functions	= 0;
	top_statements	= 0;
	top_types		= 0;
	top_defs		= 0;
	top_files		= 0;

	filename = "";
}

/*
================
idProgram::Startup
================
*/
void idProgram::Startup( const char *defaultScript ) {
	gameLocal.Printf( "Initializing scripts\n" );

	// make sure all data is freed up
	idThread::Restart();

	// get ready for loading scripts
	BeginCompilation();

	// load the default script
	if ( defaultScript && *defaultScript ) {
		CompileFile( defaultScript, true );
	}
//	gameLocal.Error( "after compile file.\n" );

	FinishCompilation();
//	gameLocal.Error( "after finish compilation.\n" );
}

/*
================
idProgram::Save
================
*/
void idProgram::Save( idSave_I *savefile ) const {
	int i;
	int currentFileNum = top_files;

	savefile->WriteInt( (fileList.Num() - currentFileNum) );
	while ( currentFileNum < fileList.Num() ) {
		savefile->WriteString( fileList[ currentFileNum ] );
		currentFileNum++;
	}

	for ( i = 0; i < variableDefaults.Num(); i++ ) {
		if ( variables[i] != variableDefaults[i] ) {
			savefile->WriteInt( i );
			savefile->WriteByte( variables[i] );
		}
	}
	// Mark the end of the diff with default variables with -1
	savefile->WriteInt( -1 );

	savefile->WriteInt( numVariables );
	for ( i = variableDefaults.Num(); i < numVariables; i++ ) {
		savefile->WriteByte( variables[i] );
	}

	int checksum = CalculateChecksum();
	savefile->WriteInt( checksum );
}

/*
================
idProgram::Restore
================
*/
bool idProgram::Restore( idRestore_I *savefile ) {
	int i, num, index;
	bool result = true;
	idStr scriptname;

	savefile->ReadInt( num );
	for ( i = 0; i < num; i++ ) {
		savefile->ReadString( scriptname );
		CompileFile( scriptname );
	}

	savefile->ReadInt( index );
	while( index >= 0 ) {
		savefile->ReadByte( variables[index] );
		savefile->ReadInt( index );
	}

	savefile->ReadInt( num );
	for ( i = variableDefaults.Num(); i < num; i++ ) {
		savefile->ReadByte( variables[i] );
	}

	int saved_checksum, checksum;

	savefile->ReadInt( saved_checksum );
	checksum = CalculateChecksum();

	if ( saved_checksum != checksum ) {
		result = false;
	}

	return result;
}

/*
================
idProgram::CalculateChecksum
================
*/
int idProgram::CalculateChecksum( void ) const {
	int i, result;

	typedef struct {
		unsigned short	op;
		int				a;
		int				b;
		int				c;
		unsigned short	linenumber;
		unsigned short	file;
	} statementBlock_t;

	statementBlock_t	*statementList = new statementBlock_t[ statements.Num() ];

	memset( statementList, 0, ( sizeof(statementBlock_t) * statements.Num() ) );

	// Copy info into new list, using the variable numbers instead of a pointer to the variable
	for( i = 0; i < statements.Num(); i++ ) {
		statementList[i].op = statements[i].op;

		if ( statements[i].a ) {
			statementList[i].a = statements[i].a->num;
		} else {
			statementList[i].a = -1;
		}
		if ( statements[i].b ) {
			statementList[i].b = statements[i].b->num;
		} else {
			statementList[i].b = -1;
		}
		if ( statements[i].c ) {
			statementList[i].c = statements[i].c->num;
		} else {
			statementList[i].c = -1;
		}

		statementList[i].linenumber = statements[i].linenumber;
		statementList[i].file = statements[i].file;
	}

	result = MD4_BlockChecksum( statementList, ( sizeof(statementBlock_t) * statements.Num() ) );

	delete [] statementList;

	return result;
}

/*
==============
idProgram::Restart

Restores all variables to their initial value
==============
*/
void idProgram::Restart( void ) {
	int i;

	idThread::Restart();

	//
	// since there may have been a script loaded by the map or the user may
	// have typed "script" from the console, free up any types and vardefs that
	// have been allocated after the initial startup
	//
	for( i = top_types; i < types.Num(); i++ ) {
		delete types[ i ];
	}
	types.SetNum( top_types, false );

	for( i = top_defs; i < varDefs.Num(); i++ ) {
		delete varDefs[ i ];
	}
	varDefs.SetNum( top_defs, false );

	for( i = top_functions; i < functions.Num(); i++ ) {
		functions[ i ].Clear();
	}
	functions.SetNum( top_functions	);

	statements.SetNum( top_statements );
	fileList.SetNum( top_files, false );
	filename.Clear();
	
	// reset the variables to their default values
	numVariables = variableDefaults.Num();
	for( i = 0; i < numVariables; i++ ) {
		variables[ i ] = variableDefaults[ i ];
	}
}

/*
================
idProgram::GetFilenum
================
*/
int idProgram::GetFilenum( const char *name ) {
	if ( filename == name ) {
		return filenum;
	}

	idStr strippedName;
	strippedName = fileSystem->OSPathToRelativePath( name );
	if ( !strippedName.Length() ) {
		// not off the base path so just use the full path
		filenum = fileList.AddUnique( name );
	} else {
		filenum = fileList.AddUnique( strippedName );
	}

	// save the unstripped name so that we don't have to strip the incoming name every time we call GetFilenum
	filename = name;

	return filenum;
}

/*
================
idProgram::idProgram
================
*/
idProgram::idProgram() {
	FreeData();
}

/*
================
idProgram::~idProgram
================
*/
idProgram::~idProgram() {
	FreeData();
}


/*
============
idProgram::FindImmediate

tries to find an existing immediate with the same value
============
*/
idVarDef *idProgram::FindImmediate( const idTypeDef *type, const eval_t *eval, const char *string ) const {
	idVarDef	*def;
	etype_t		etype;

	etype = type->Type();

	// check for a constant with the same value
	for( def = GetDefList( "<IMMEDIATE>" ); def != NULL; def = def->Next() ) {
		if ( def->TypeDef() != type ) {
			continue;
		}

		switch( etype ) {
		case ev_field :
			if ( *def->value.intPtr == eval->_int ) {
				return def;
			}
			break;

		case ev_argsize :
			if ( def->value.argSize == eval->_int ) {
				return def;
			}
			break;

		case ev_jumpoffset :
			if ( def->value.jumpOffset == eval->_int ) {
				return def;
			}
			break;

		case ev_entity :
			if ( *def->value.intPtr == eval->entity ) {
				return def;
			}
			break;

		case ev_string :
			if ( idStr::Cmp( def->value.stringPtr, string ) == 0 ) {
				return def;
			}
			break;

		case ev_float :
			if ( *def->value.floatPtr == eval->_float ) {
				return def;
			}
			break;

		case ev_virtualfunction :
			if ( def->value.virtualFunction == eval->_int ) {
				return def;
			}
			break;


		case ev_vector :
			if ( ( def->value.vectorPtr->x == eval->vector[ 0 ] ) && 
				( def->value.vectorPtr->y == eval->vector[ 1 ] ) && 
				( def->value.vectorPtr->z == eval->vector[ 2 ] ) ) {
				return def;
			}
			break;

		default :
			gameLocal.Error( "weird immediate type" );
			break;
		}
	}

	return NULL;
}

/*
============
idProgram::GetImmediate

returns an existing immediate with the same value, or allocates a new one
============
*/
idVarDef *idProgram::GetImmediate( idTypeDef *type, const eval_t *eval, const char *string ) {
	idVarDef *def;

	def = FindImmediate( type, eval, string );
	if ( def ) {
		def->numUsers++;
	} else {
		// allocate a new def
		def = AllocDef( type, "<IMMEDIATE>", &def_namespace, true );
		if ( type->Type() == ev_string ) {
			def->SetString( string, true );
		} else {
			def->SetValue( *eval, true );
		}
	}

	return def;
}

/*
================
idProgram::SaveStatements
================
*/

void idProgram::SaveStatements( int frompos, idList<statement_t> &copybuffer ) {
	int i;
	if ( frompos < 0 ) return;

	for ( i = frompos; i < statements.Num(); i++ ) {
		copybuffer.Append( statements[i] );
	}
	statements.SetNum(frompos);
}

/*
================
idProgram::DropStatements
================
*/

void idProgram::DropStatements( int frompos ) {
	if ( frompos < 0 ) return;
	statements.SetNum(frompos);
}

/*
================
idProgram::LoadStatement
================
*/

void idProgram::LoadStatements( idList<statement_t> &copybuffer ) {
	int i;

	for ( i = 0; i < copybuffer.Num(); i++ ) {
		statements.Append( copybuffer[i] );
	}
}