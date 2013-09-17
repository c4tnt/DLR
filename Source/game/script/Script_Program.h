// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __SCRIPT_PROGRAM_H__
#define __SCRIPT_PROGRAM_H__

class idScriptObject;
class idEventDef;
class idVarDef;
class idTypeDef;
class idEntity;
class idThread;
class idSave_I;
class idRestore_I;

#define MAX_STRING_LEN		128
#define MAX_GLOBALS			196608			// in bytes
#define MAX_STRINGS			1024
#define MAX_FUNCS			3072
#define MAX_STATEMENTS		81920			// statement_t - 18 bytes last I checked
#define MAX_DEFFERED_STATEMENTS 4			// c4tnt: max deffered operations on a single var

const char * const RESULT_STRING = "<RESULT>";
const char * const RETURN_STRING = "<RETURN>";

typedef enum {
	ev_error = -1, ev_void, ev_scriptevent, ev_namespace, ev_string, ev_float, ev_vector, ev_entity, ev_field, ev_function, ev_virtualfunction, ev_pointer, ev_object, ev_jumpoffset, ev_argsize, ev_boolean, ev_list ,ev_array
} etype_t;

//c4tnt: type definition layers. 
typedef enum {
	etl_deflevel, etl_objlevel, etl_varlevel, etl_end = etl_varlevel
} etypelayer_t;
const char etl_names[3][13]={"DEF level","Object level","VAR level"};

class function_t {
public:
						function_t();

	size_t				Allocated( void ) const;
	void				SetName( const char *name );
	const char			*Name( void ) const;
	void				Clear( void );

private:
	idStr 				name;
public:
	const idEventDef	*eventdef;
	idVarDef			*def;
	const idTypeDef		*type;
	int 				firstStatement;
	int 				numStatements;
	int 				parmTotal;
	int 				locals; 			// total ints of parms + locals
	int					filenum; 			// source file defined in
	idList<int>			parmSize;
};

typedef union eval_s {
	const char			*stringPtr;
	float				_float;
	float				vector[ 3 ];
	function_t			*function;
	int 				_int;
	int 				entity;
} eval_t;

typedef struct proxy_s {
	etype_t				proxyFor;
	union {
		int					_objidx;
		void*				_objptr;
	};
	eval_t				_idx;
} proxy_t;

typedef struct opcode_s {
	char		*name;
	char		*opname;
	int			priority;
	int			constMask;	//c4tnt: fix for a "const = const" bug. In runtime. 1 - a is variable. 2 - b is variable
	bool		rightAssociative;
	idVarDef	*type_a;
	idVarDef	*type_b;
	idVarDef	*type_c;
} opcode_t;

/***********************************************************************

idTypeDef

Contains type information for variables and functions.

***********************************************************************/

class idTypeDef {
private:
	etype_t						type;
	etypelayer_t				layer; //c4tnt: type definition layer
	idStr 						name;
	int							size;
	
	// function types are more complex
	idTypeDef					*auxType;					// return type
	idList<idTypeDef *>			parmTypes;
	idStrList					parmNames;
	idList<const function_t *>	functions;

public:
	idVarDef					*def;						// a def that points to this type

						idTypeDef( const idTypeDef &other );
						idTypeDef( etype_t etype, idVarDef *edef, const char *ename, int esize, idTypeDef *aux, etypelayer_t layer = etl_deflevel);
	void				operator=( const idTypeDef& other );
	size_t				Allocated( void ) const;

	bool				Inherits( const idTypeDef *basetype ) const;
	etypelayer_t		GetLayer( void ) const;
	bool				CheckLayer( const etypelayer_t layer ) const;
	bool				MatchesType( const idTypeDef &matchtype ) const;
	bool				MatchesVirtualFunction( const idTypeDef &matchfunc ) const;
	void				AddFunctionParm( idTypeDef *parmtype, const char *name );
	void				AddField( idTypeDef *fieldtype, const char *name );

	void				SetName( const char *newname );
	const char			*Name( void ) const;

	etype_t				Type( void ) const;
	int					Size( void ) const;

	idTypeDef			*SuperClass( void ) const;
	
	idTypeDef			*ReturnType( void ) const;
	void				SetReturnType( idTypeDef *type );

	idTypeDef			*FieldType( void ) const;
	void				SetFieldType( idTypeDef *type );

	idTypeDef			*PointerType( void ) const;
	void				SetPointerType( idTypeDef *type );

	idTypeDef			*ListType( void ) const;	//c4tnt: added for lists and arrays
	void				SetListType( idTypeDef *type );

	idTypeDef			*RightType( void );

	int					NumParameters( void ) const;
	idTypeDef			*GetParmType( int parmNumber ) const;
	const char			*GetParmName( int parmNumber ) const;
	bool				CheckParmFmt( const char* fmt ) const;

	int					NumFunctions( void ) const;
	int					GetFunctionNumber( const function_t *func ) const;
	const function_t	*GetFunction( int funcNumber ) const;
	void				AddFunction( const function_t *func );
};

/***********************************************************************

idScriptObject

In-game representation of objects in scripts.  Use the idScriptVariable template
(below) to access variables.

***********************************************************************/

class idScriptObject {
private:
	idTypeDef					*type;
	
public:
	byte						*data;

								idScriptObject();
								~idScriptObject();

	void						Save( idSave_I *savefile ) const;			// archives object for save game file
	void						Restore( idRestore_I *savefile );			// unarchives object from save game file

	void						Free( void );
	bool						SetType( const char *typeName );
	void						ClearObject( void );
	bool						HasObject( void ) const;
	idTypeDef					*GetTypeDef( void ) const;
	const char					*GetTypeName( void ) const;
	const function_t			*GetConstructor( void ) const;
	const function_t			*GetDestructor( void ) const;
	const function_t			*GetFunction( const char *name ) const;

	byte						*GetVariable( const char *name, etype_t etype ) const;
};

/***********************************************************************

idScriptVariable

Helper template that handles looking up script variables stored in objects.
If the specified variable doesn't exist, or is the wrong data type, idScriptVariable
will cause an error.

***********************************************************************/

template<class type, etype_t etype, class returnType>
class idScriptVariable {
private:
	type				*data;

public:
						idScriptVariable();
	bool				IsLinked( void ) const;
	void				Unlink( void );
	void				LinkTo( idScriptObject &obj, const char *name );
	idScriptVariable	&operator=( const returnType &value );
						operator returnType() const;
};

template<class type, etype_t etype, class returnType>
ID_INLINE idScriptVariable<type, etype, returnType>::idScriptVariable() {
	data = NULL;
}

template<class type, etype_t etype, class returnType>
ID_INLINE bool idScriptVariable<type, etype, returnType>::IsLinked( void ) const {
	return ( data != NULL );
}

template<class type, etype_t etype, class returnType>
ID_INLINE void idScriptVariable<type, etype, returnType>::Unlink( void ) {
	data = NULL;
}

template<class type, etype_t etype, class returnType>
ID_INLINE void idScriptVariable<type, etype, returnType>::LinkTo( idScriptObject &obj, const char *name ) {
	data = ( type * )obj.GetVariable( name, etype );
	if ( !data ) {
		gameLocal.Warning( "Missing '%s' field in script object '%s'", name, obj.GetTypeName() );
	}
}

template<class type, etype_t etype, class returnType>
ID_INLINE idScriptVariable<type, etype, returnType> &idScriptVariable<type, etype, returnType>::operator=( const returnType &value ) {
	// check if we attempt to access the object before it's been linked
	assert( data );

	// make sure we don't crash if we don't have a pointer
	if ( data ) {
		*data = ( type )value;
	}
	return *this;
}

template<class type, etype_t etype, class returnType>
ID_INLINE idScriptVariable<type, etype, returnType>::operator returnType() const {
	// check if we attempt to access the object before it's been linked
	assert( data );

	// make sure we don't crash if we don't have a pointer
	if ( data ) {
		return ( const returnType )*data;
	} else {
		// reasonably safe value
		return ( const returnType )0;
	}
}

/***********************************************************************

Script object variable access template instantiations

These objects will automatically handle looking up of the current value
of a variable in a script object.  They can be stored as part of a class
for up-to-date values of the variable, or can be used in functions to
sample the data for non-dynamic values.

***********************************************************************/

typedef idScriptVariable<bool, ev_boolean, bool>			idScriptBool;
typedef idScriptVariable<float, ev_float, float>			idScriptFloat;
typedef idScriptVariable<float, ev_float, int>				idScriptInt;
typedef idScriptVariable<idVec3, ev_vector, idVec3>			idScriptVector;
typedef idScriptVariable<idStr, ev_string, const char *>	idScriptString;
//c4tnt
typedef idScriptVariable<idStr, ev_list, const int *>		idScriptList;
typedef idScriptVariable<idStr, ev_array, const int *>		idScriptArray;

/***********************************************************************

idCompileError

Causes the compiler to exit out of compiling the current function and
display an error message with line and file info.

***********************************************************************/

class idCompileError : public idException {
public:
	idCompileError( const char *text ) : idException( text ) {}
};

/***********************************************************************

idVarDef

Define the name, type, and location of variables, functions, and objects
defined in script.

***********************************************************************/

typedef union varEval_s {
	idScriptObject			**objectPtrPtr;
	char					*stringPtr;
	float					*floatPtr;
	idVec3					*vectorPtr;
	function_t				*functionPtr;
	int 					*intPtr;
	byte					*bytePtr;
	int 					*entityNumberPtr;
	int						virtualFunction;
	int						jumpOffset;
	int						stackOffset;		// offset in stack for local variables
	int						argSize;
	varEval_s				*evalPtr;
	int						ptrOffset;
	proxy_t					*_proxy;
} varEval_t;

class idVarDefName;

typedef enum {
	lValue, rValue
} lvrv_t;


class idVarDef {
	friend class idVarDefName;

public:
	int						num;
	varEval_t				value;
	idVarDef *				scope; 			// function, namespace, or object the var was defined in
	int						numUsers;		// number of users if this is a constant

	typedef enum {
		uninitialized, initializedVariable, initializedConstant, stackVariable
	} initialized_t;

	initialized_t			initialized;

public:
							idVarDef( idTypeDef *typeptr = NULL );
							~idVarDef();

	const char *			Name( void ) const;
	const char *			GlobalName( void ) const;

	void					SetTypeDef( idTypeDef *_type ) { typeDef = _type; }
	idTypeDef *				TypeDef( void ) const { return typeDef;  }
	etype_t					Type( void ) const { return ( typeDef != NULL ) ? typeDef->Type() : ev_void; }

	int						DepthOfScope( const idVarDef *otherScope ) const;

	void					SetFunction( function_t *func );
	void					SetObject( idScriptObject *object );
	void					SetValue( const eval_t &value, bool constant );
	void					SetString( const char *string, bool constant );

	idVarDef *				Next( void ) const { return next; }		// next var def with same name

	void					PrintInfo( idFile *file, int instructionPointer ) const;
	void					PrintInfo( idStr  &str, int instructionPointer ) const;

private:
	idTypeDef *				typeDef;
	idVarDefName *			name;		// name of this var
	idVarDef *				next;		// next var with the same name
};

/***********************************************************************

  idVarDefName

***********************************************************************/

class idVarDefName {
public:
							idVarDefName( void ) { defs = NULL; }
							idVarDefName( const char *n ) { name = n; defs = NULL; }

	const char *			Name( void ) const { return name; }
	idVarDef *				GetDefs( void ) const { return defs; }

	void					AddDef( idVarDef *def );
	void					RemoveDef( idVarDef *def );

private:
	idStr					name;
	idVarDef *				defs;
};

/***********************************************************************

  Variable and type defintions

***********************************************************************/

extern	idTypeDef	type_void;
extern	idTypeDef	type_scriptevent;
extern	idTypeDef	type_namespace;
extern	idTypeDef	type_string;
extern	idTypeDef	type_float;
extern	idTypeDef	type_vector;
extern	idTypeDef	type_entity;
extern  idTypeDef	type_field; //c4tnt:depricated
extern	idTypeDef	type_function;
extern	idTypeDef	type_virtualfunction;
extern  idTypeDef	type_pointer;
extern	idTypeDef	type_object;
extern	idTypeDef	type_jumpoffset;	// only used for jump opcodes
extern	idTypeDef	type_argsize;		// only used for function call and thread opcodes
extern	idTypeDef	type_boolean;
extern	idTypeDef	type_list;			//c4tnt: array and list external definition
extern	idTypeDef	type_array;

extern	idVarDef	def_void;
extern	idVarDef	def_scriptevent;
extern	idVarDef	def_namespace;
extern	idVarDef	def_string;
extern	idVarDef	def_float;
extern	idVarDef	def_vector;
extern	idVarDef	def_entity;
extern	idVarDef	def_field;
extern	idVarDef	def_function;
extern	idVarDef	def_virtualfunction;
extern	idVarDef	def_pointer;
extern	idVarDef	def_object;
extern	idVarDef	def_jumpoffset;		// only used for jump opcodes
extern	idVarDef	def_argsize;		// only used for function call and thread opcodes
extern	idVarDef	def_boolean;
extern	idVarDef	def_list;
extern	idVarDef	def_array;

typedef struct statement_s {
	unsigned short	op;
	idVarDef		*a;
	idVarDef		*b;
	idVarDef		*c;
	unsigned short	linenumber;
	unsigned short	file;
} statement_t;

/***********************************************************************

idProgram

Handles compiling and storage of script data.  Multiple idProgram objects
would represent seperate programs with no knowledge of each other.  Scripts
meant to access shared data and functions should all be compiled by a
single idProgram.

***********************************************************************/

class idProgram {
private:
	idStrList									fileList;
	idStr 										filename;
	int											filenum;

	int											numVariables;
	byte										variables[ MAX_GLOBALS ];
	idStaticList<byte,MAX_GLOBALS>				variableDefaults;
	idStaticList<function_t,MAX_FUNCS>			functions;
	idStaticList<statement_t,MAX_STATEMENTS>	statements;
	idList<idTypeDef *>							types;
	idList<idVarDefName *>						varDefNames;
	idHashIndex									varDefNameHash;
	idList<idVarDef *>							varDefs;

	idVarDef									*sysDef;

	int											top_functions;
	int											top_statements;
	int											top_types;
	int											top_defs;
	int											top_files;

	void										CompileStats( void );

public:
//	idVarDef									*returnDef;
//	idVarDef									*returnStringDef;

												idProgram();
												~idProgram();

	// save games
	void										Save( idSave_I *savefile ) const;
	bool										Restore( idRestore_I *savefile );
	int											CalculateChecksum( void ) const;		// Used to insure program code has not
																						//    changed between savegames

	void										Startup( const char *defaultScript );
	void										Restart( void );
	bool										CompileText( const char *source, const char *text, bool console, bool allowCVars = false );
	const function_t							*CompileFunction( const char *functionName, const char *text );
	void										CompileFile( const char *filename, bool allowCVars = false );
	void										BeginCompilation( void );
	void										FinishCompilation( void );
	void										DisassembleStatement( idFile *file, int instructionPointer ) const;
	void										DisassembleStatement( idStr &str, int instructionPointer ) const;
	int											Disassemble( const idStr *fname = NULL, bool toConsole = false) const; //c4tnt:updated
	void										FreeData( void );

	const char									*GetFilename( int num );
	int											GetFilenum( const char *name );
	int											GetLineNumberForStatement( int index );
	const char									*GetFilenameForStatement( int index );

	idTypeDef									*AllocType( idTypeDef &type );
	idTypeDef									*AllocType( etype_t etype, idVarDef *edef, const char *ename, int esize, idTypeDef *aux, etypelayer_t vlayer = etl_deflevel );
	idTypeDef									*GetType( idTypeDef &type, bool allocate );
	idTypeDef									*FindType( const char *name );

	idVarDef									*AllocDef( idTypeDef *type, const char *name, idVarDef *scope, bool constant);
	idVarDef									*AllocDefferedDef( idTypeDef *type, const char *name, idVarDef *scope, bool constant);
	idVarDef									*GetDef( const idTypeDef *type, const char *name, const idVarDef *scope ) const;
	void										FreeDef( idVarDef *d, const idVarDef *scope );
	idVarDef									*FindFreeResultDef( idTypeDef *type, const char *name, idVarDef *scope, const idVarDef *a, const idVarDef *b );
	idVarDef									*GetDefList( const char *name ) const;
	void										AddDefToNameList( idVarDef *def, const char *name );

	//c4tnt:				Code from the idCompiler. This functions allows to create a new args on a stack
	idVarDef				*FindImmediate( const idTypeDef *type, const eval_t *eval, const char *string ) const;
	idVarDef				*GetImmediate( idTypeDef *type, const eval_t *eval, const char *string );

	function_t									*FindFunction( const char *name ) const;						// returns NULL if function not found
	function_t									*FindFunction( const char *name, const idTypeDef *type ) const;	// returns NULL if function not found
	const idVarDef								*FindFunctionDef( const char *name ) const;						// returns NULL if function not found
	const idVarDef								*FindFunctionDef( const char *name, const idTypeDef *type ) const;	// returns NULL if function not found
	function_t									&AllocFunction( idVarDef *def );
	function_t									*GetFunction( int index );
	int											GetFunctionIndex( const function_t *func );

	void										SetEntity( const char *name, idEntity *ent );

	statement_t									*AllocStatement( void );
	statement_t									&GetStatement( int index );
	int											NumStatements( void ) { return statements.Num(); }
	
	// c4tnt:	Export\Import statements for optimizer
	void										SaveStatements( int frompos, idList<statement_t> &copybuffer );
	void										LoadStatements( idList<statement_t> &copybuffer );
	void										DropStatements( int frompos );

	int											NumFilenames( void ) { return fileList.Num( ); }
};

/*
================
idProgram::GetStatement
================
*/
ID_INLINE statement_t &idProgram::GetStatement( int index ) {
	return statements[ index ];
}

/*
================
idProgram::GetFunction
================
*/
ID_INLINE function_t *idProgram::GetFunction( int index ) {
	return &functions[ index ];
}

/*
================
idProgram::GetFunctionIndex
================
*/
ID_INLINE int idProgram::GetFunctionIndex( const function_t *func ) {
	return func - &functions[0];
}

/*
================
idProgram::GetFilename
================
*/
ID_INLINE const char *idProgram::GetFilename( int num ) {
	return fileList[ num ];
}

/*
================
idProgram::GetLineNumberForStatement
================
*/
ID_INLINE int idProgram::GetLineNumberForStatement( int index ) {
	return statements[ index ].linenumber;
}

/*
================
idProgram::GetFilenameForStatement
================
*/
ID_INLINE const char *idProgram::GetFilenameForStatement( int index ) {
	return GetFilename( statements[ index ].file );
}

#endif /* !__SCRIPT_PROGRAM_H__ */
