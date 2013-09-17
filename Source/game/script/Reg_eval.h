#ifndef __SCRIPT_RE_H__
#define __SCRIPT_RE_H__

//Simple evaluator class.
//Can evaluate only numerical formulas
//Use it for fast damage\stats\weapon calculations

typedef enum {
	RE_DEC,			// (A)--
	RE_INC,			// (A)++
	RE_ADD,			// (A) = A + B
	RE_SUB,			// (A) = A - B
	RE_MUL,			// (A) = A * B
	RE_DIV,			// (A) = A / B
	RE_I,			// (A) = int(A)
	RE_IF,			// (A) = A?B:C
	RE_MAX,			// (A) = max(A,B)
	RE_MIN,			// (A) = min(A,B)
	RE_DUP			// (A, A) = A
} _re_opcode;

typedef enum {
	RGT_FCONST,		// only float ops
	RGT_REG,
	RGT_DYN			// dynamic reg
} _re_rgtype;

typedef struct _re_stackvar_s {
	union {
		float	VarF;
		int		RgRef;
	};
	_re_rgtype type;
} _re_stackvar_t;

///////////////////////////////////////////////////
//
//    idRegEvaluator
//
///////////////////////////////////////////////////

class idRegEvaluator {

public:
	idRegEvaluator();
	~idRegEvaluator();

	bool Parse( const char* cmd );
	bool Eval( const char* cmd );

	int  SetRegister( const char* name );
	int  CopyRegister( const char* name, idRegEvaluator &from );
	
	bool SetRegisterClampI( int Handle, int min, int max );
	bool SetRegisterClampF( int Handle, float min, float max );
	bool ClearRegisterClamp( int Handle );
	
	bool SetRegisterDataI( int Handle, int Data );
	bool SetRegisterDataF( int Handle, float Data );
	
	int  GetRegisterDataI( int Handle );
	float GetRegisterDataF( int Handle );

private:	
	

};


#endif