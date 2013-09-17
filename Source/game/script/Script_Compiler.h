// Copyright (C) 2004 Id Software, Inc.
//
#ifndef __SCRIPT_COMPILER_H__
#define __SCRIPT_COMPILER_H__

// These opcodes are no longer necessary:
// OP_PUSH_OBJ:
// OP_PUSH_OBJENT:

enum {
	OP_NOOP,
	OP_RETURN,
	OP_DEFRETURN,

	OP_ASSERT,
	OP_TRACEFALL,

	OP_UINC_F,
	OP_UINCP_F,
	OP_UDEC_F,
	OP_UDECP_F,
	OP_XST_S,
	OP_XST_E,
	OP_XST_V,
	OP_XST_O,

	OP_COMP_F,

	OP_MUL_F,
	OP_MUL_V,
	OP_MUL_FV,
	OP_MUL_VF,
	OP_DIV_F,
	OP_MOD_F,
	OP_ADD_F,
	OP_ADD_V,
	OP_ADD_S,
	OP_ADD_FS,
	OP_ADD_SF,
	OP_ADD_VS,
	OP_ADD_SV,
	OP_SUB_F,
	OP_SUB_V,
	
	OP_EQ_F,
	OP_EQ_V,
	OP_EQ_S,
	OP_EQ_E,
	OP_EQ_EO,
	OP_EQ_OE,
	OP_EQ_OO,
	
	OP_NE_F,
	OP_NE_V,
	OP_NE_S,
	OP_NE_E,
	OP_NE_EO,
	OP_NE_OE,
	OP_NE_OO,
	
	OP_LE,
	OP_GE,
	OP_LT,
	OP_GT,

	OP_INDIRECT_F,
	OP_INDIRECT_V,
	OP_INDIRECT_S,
	OP_INDIRECT_ENT,
	OP_INDIRECT_BOOL,
	OP_INDIRECT_OBJ,
	OP_INDIRECT_A,
	
	OP_ADDRESS_O,
	OP_ADDRESS_E,

	OP_EVENTCALL,
	OP_OBJECTCALL,
	OP_SYSCALL,
	OP_VPCALL,

	OP_STORE_F,
	OP_STORE_V,
	OP_STORE_S,
	OP_STORE_ENT,
	OP_STORE_BOOL,
	OP_STORE_OBJENT,
	OP_STORE_OBJ,
	OP_STORE_ENTOBJ,
	OP_STORE_A,

	OP_STORE_FTOS,
	OP_STORE_BTOS,
	OP_STORE_VTOS,
	OP_STORE_FTOBOOL,
	OP_STORE_BOOLTOF,

	OP_STOREP_F,
	OP_STOREP_V,
	OP_STOREP_S,
	OP_STOREP_ENT,
	OP_STOREP_FLD,
	OP_STOREP_BOOL,
	OP_STOREP_OBJ,
	OP_STOREP_OBJENT,
	OP_STOREP_A,

	OP_STOREP_FTOS,
	OP_STOREP_BTOS,
	OP_STOREP_VTOS,
	OP_STOREP_FTOBOOL,
	OP_STOREP_BOOLTOF,

	OP_RPOP_F,
	OP_RPOP_V,
	OP_RPOP_S,
	OP_RPOP_ENT,
	OP_RPOP_OBJ,
	OP_RPOP_OBJENT,
	OP_RPOP_FTOB,
	OP_RPOP_A,

	OP_UMUL_F,
	OP_UMUL_V,
	OP_UDIV_F,
	OP_UDIV_V,
	OP_UMOD_F,
	OP_UADD_F,
	OP_UADD_V,
	OP_USUB_F,
	OP_USUB_V,
	OP_UAND_F,
	OP_UOR_F,

	OP_NOT_BOOL,
	OP_NOT_F,
	OP_NOT_V,
	OP_NOT_S,
	OP_NOT_ENT,

	OP_NEG_F,
	OP_NEG_V,

	OP_INT_F,
	OP_IF,
	OP_IFNOT,

	OP_CALL,
	OP_THREAD,
	OP_OBJTHREAD,

	OP_PUSH_F,
	OP_PUSH_V,
	OP_PUSH_S,
	OP_PUSH_ENT,
	OP_PUSH_OBJ,
	OP_PUSH_OBJENT,
	OP_PUSH_A,
	OP_PUSH_FTOS,
	OP_PUSH_BTOF,
	OP_PUSH_FTOB,
	OP_PUSH_VTOS,
	OP_PUSH_BTOS,

	OP_GOTO,

	OP_AND,
	OP_AND_BOOLF,
	OP_AND_FBOOL,
	OP_AND_BOOLBOOL,
	OP_OR,
	OP_OR_BOOLF,
	OP_OR_FBOOL,
	OP_OR_BOOLBOOL,
	
	OP_BITAND,
	OP_BITOR,

	OP_BREAK,			// placeholder op.  not used in final code
	OP_CONTINUE,		// placeholder op.  not used in final code

	OP_INDEX_F,			//c4tnt: array 
	OP_INDEX_V,
	OP_INDEX_S,
	OP_INDEX_ENT,
	OP_INDEX_BOOL,
	OP_INDEX_OBJ,
	OP_INDEX_PTR,

	OP_MKARR_F,
	OP_MKARR_V,
	OP_MKARR_S,
	OP_MKARR_ENT,
	OP_MKARR_BOOL,
	OP_MKARR_OBJ,
	OP_RMARR,

	OP_ALLOC_OBJ,
	OP_DEALLOC_OBJ,

	NUM_OPCODES
};

class idCompiler {
private:
//	static bool		punctuationValid[ 256 ];
	static char		*punctuation[];

//	static idParser	parser;
	idParser		*parserPtr;
	idToken			token;
					
	idTypeDef		*immediateType;
	eval_t			immediate;
					
	bool			eof;
	bool			console;
	bool			callthread;
	bool			registerCVars;
	int				braceDepth;
	int				loopDepth;
	int				currentLineNumber;
	int				currentFileNumber;
	int				errorCount;
	idProgram		*CompileTarget;
					
	idVarDef		*scope;				// the function being parsed, or NULL
	const idVarDef	*basetype;			// for accessing fields

	float			Divide( float numerator, float denominator );
	void			Error( const char *error, ... ) const id_attribute((format(printf,2,3)));
	void			Warning( const char *message, ... ) const id_attribute((format(printf,2,3)));
	idVarDef		*OptimizeOpcode( const opcode_t *op, idVarDef *var_a, idVarDef *var_b );
	idVarDef		*ParseNew( void );
	idVarDef		*EmitOpcode( const opcode_t *op, idVarDef *var_a, idVarDef *var_b, idTypeDef *type_c = NULL);
	idVarDef		*EmitOpcode( int op, idVarDef *var_a, idVarDef *var_b, idTypeDef *type_c = NULL );
	void			EmitOpcode3( const opcode_t *op, idVarDef *var_a, idVarDef *var_b, idVarDef *var_c );
	void			EmitOpcode3( int op, idVarDef *var_a, idVarDef *var_b, idVarDef *var_c );
	opcode_t		*FindSituableOpcode( opcode_t *start, idTypeDef *t_a, idTypeDef *t_b, etype_t t_c );
	bool			EmitPush( idVarDef *expression, const idTypeDef *funcArg );
	void			NextToken( void );
	void			ExpectToken( const char *string );
	bool			CheckToken( const char *string );
	void			ParseName( idStr &name );
	void			SkipOutOfFunction( void );
	void			SkipToSemicolon( void );
	idTypeDef		*CheckType( etypelayer_t vlay = etl_deflevel );
	idTypeDef		*ParseType( etypelayer_t layer = etl_deflevel );
	idVarDef		*VirtualFunctionConstant( idVarDef *func );
	idVarDef		*SizeConstant( int size );
	idVarDef		*SizeConstantWReturn( int size, int _return );
	idVarDef		*JumpConstant( int value );
	idVarDef		*JumpDef( int jumpfrom, int jumpto );
	idVarDef		*JumpTo( int jumpto );
	idVarDef		*JumpFrom( int jumpfrom );
	idVarDef		*ParseImmediate( void );
	idVarDef		*EmitFunctionParms( int op, idVarDef *func, int startarg, int startsize, idVarDef *object );
	idVarDef		*EmitROP( idTypeDef *returnType, const char* _fname );
	idVarDef		*EmitCAST( idTypeDef *returnType, idVarDef* inVarDef, const char* _fname );
	idVarDef		*ParseFunctionCall( idVarDef *func );
	idVarDef		*ParseArrayListInd( idVarDef *arrayDef );
	idVarDef		*ParseObjectCall( idVarDef *object, idVarDef *func );
	idVarDef		*ParseEventCall( idVarDef *object, idVarDef *func );
	idVarDef		*ParseVPCall( idVarDef *object, idVarDef *func );
	idVarDef		*ParseSysObjectCall( idVarDef *func );
	idVarDef		*LookupDef( const char *name, const idVarDef *baseobj );
	idVarDef		*ParseValue( );
	idVarDef		*GetTerm( );
	idVarDef		*EmitArrayCreation( idVarDef *arrayDef );
	idVarDef		*EmitArrayCreation( idTypeDef *arrayTDef, idVarDef *num );

	bool			TypeMatches( etype_t type1, etype_t type2 ) const;
	idVarDef		*GetExpression( int priority );
	idTypeDef		*GetTypeForEventArg( char argType );
	void			PatchLoop( int start, int continuePos );
	void			ParseTracefall( void );
	void			ParseReturnStatement( void );
	void			ParseWhileStatement( void );
	void			ParseForStatement( void );
	void			ParseDoWhileStatement( void );
	void			ParseIfStatement( void );
	void			ParseCVarStatement( void );
	void			ParseStatement( void );
	void			ParseObjectDef( const char *objname );
	idTypeDef		*ParseFunction( idTypeDef *returnType, const char *name );
	void			ParseFunctionDef( idTypeDef *returnType, const char *name );
	void			ParseVariableDef( idTypeDef *type, const char *name );
	void			ParseEventDef( idTypeDef *type, const char *name );
	void			ParseDefs( void );
	void			ParseNamespace( idVarDef *newScope );

public :
	static opcode_t	opcodes[];

					idCompiler(idProgram* Target, idParser* parser, bool allowCVars);
	void			CompileFile( bool console );
	
};

#endif /* !__SCRIPT_COMPILER_H__ */
