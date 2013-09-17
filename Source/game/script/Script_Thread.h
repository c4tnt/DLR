// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __SCRIPT_THREAD_H__
#define __SCRIPT_THREAD_H__

extern const idEventDef EV_Thread_Execute;
extern const idEventDef EV_Thread_SetCallback;
extern const idEventDef EV_Thread_TerminateThread;
extern const idEventDef EV_Thread_Pause;
extern const idEventDef EV_Thread_Wait;
extern const idEventDef EV_Thread_WaitFrame;
extern const idEventDef EV_Thread_WaitFor;
extern const idEventDef EV_Thread_WaitForThread;
extern const idEventDef EV_Thread_Print;
extern const idEventDef EV_Thread_PrintLn;
extern const idEventDef EV_Thread_Say;
extern const idEventDef EV_Thread_Assert;
extern const idEventDef EV_Thread_Trigger;
extern const idEventDef EV_Thread_SetCvar;
extern const idEventDef EV_Thread_GetCvar;
extern const idEventDef EV_Thread_Random;
extern const idEventDef EV_Thread_GetTime;
extern const idEventDef EV_Thread_KillThread;
extern const idEventDef EV_Thread_SetThreadName;
extern const idEventDef EV_Thread_GetEntity;
extern const idEventDef EV_Thread_Spawn;
extern const idEventDef EV_Thread_SetSpawnArg;
extern const idEventDef EV_Thread_SpawnString;
extern const idEventDef EV_Thread_SpawnFloat;
extern const idEventDef EV_Thread_SpawnVector;
extern const idEventDef EV_Thread_AngToForward;
extern const idEventDef EV_Thread_AngToRight;
extern const idEventDef EV_Thread_AngToUp;
extern const idEventDef EV_Thread_Sine;
extern const idEventDef EV_Thread_Cosine;
extern const idEventDef EV_Thread_Normalize;
extern const idEventDef EV_Thread_VecLength;
extern const idEventDef EV_Thread_VecDotProduct;
extern const idEventDef EV_Thread_VecCrossProduct;

extern const idEventDef EV_Thread_Vector;
extern const idEventDef EV_Thread_VecLerp;
extern const idEventDef EV_Thread_VecProjectPlane;
extern const idEventDef EV_Thread_VecTrunc;

extern const idEventDef EV_Thread_OnSignal;
extern const idEventDef EV_Thread_ClearSignal;
extern const idEventDef EV_Thread_SetCamera;
extern const idEventDef EV_Thread_FirstPerson;
extern const idEventDef EV_Thread_TraceFraction;
extern const idEventDef EV_Thread_TracePos;
extern const idEventDef EV_Thread_FadeIn;
extern const idEventDef EV_Thread_FadeOut;
extern const idEventDef EV_Thread_FadeTo;
extern const idEventDef EV_Thread_Restart;

class idThread : public idClass {
private:
	static idThread				*currentThread;

	idThread					*waitingForThread;
	int							waitingFor;
	int							waitingUntil;
	idInterpreter				interpreter;

	int 						threadNum;
	idStr 						threadName;

	int							lastExecuteTime;
	int							creationTime;

	bool						manualControl;

	static int					threadIndex;
	static idList<idThread *>	threadList;

	// c4tnt: sys hidden args here
	idDict						spawnArgs;
	idDict						textArgs;
	static trace_t				trace;

	// c4tnt: could not find any reference for this.
	//int							param_size;
	//const function_t *			param_func_ptr;

	void						Init( void );
	void						Pause( void );

	void						Event_Execute( void );
	void						Event_SetThreadName( const char *name );

	//
	// script callable Events
	//
	void						Event_TerminateThread( int num );
	void						Event_Pause( void );
	void						Event_Wait( float time );
	void						Event_WaitFrame( void );
	void						Event_WaitFor( idEntity *ent );
	void						Event_WaitForThread( int num );
	void						Event_Print( const char *text );
	void						Event_PrintLn( const char *text );
	void						Event_Say( const char *text );
	void						Event_Assert( float value );
	void						Event_Trigger( idEntity *ent );
	void						Event_SetCvar( const char *name, const char *value ) const;
	void						Event_GetCvar( idEventReturn *returnContext, const char *name ) const; //return s
	void						Event_SetSkillvar( const char *name, const char *value ) const;
	void						Event_GetSkillvar( idEventReturn *returnContext, const char *name, const char *defstr ) const; //return s
	void						Event_Random( idEventReturn *returnContext, float range ) const; //return f
	void						Event_GetTime( idEventReturn *returnContext ); //return f
	void						Event_KillThread( const char *name );
	void						Event_GetEntity( idEventReturn *returnContext, const char *name ); //return e
	void						Event_Spawn( idEventReturn *returnContext, const char *classname ); //return e
	void						Event_CopySpawnArgs( idEntity *ent );
	void						Event_SetSpawnArg( const char *key, const char *value );
	void						Event_ResetSpawnArgs( );
	void						Event_SelectSpawnDef( const char *classname );
	void						Event_SpawnString( idEventReturn *returnContext, const char *key, const char *defaultvalue ); //return s
	void						Event_SpawnFloat( idEventReturn *returnContext, const char *key, float defaultvalue ); //return f
	void						Event_SpawnVector( idEventReturn *returnContext, const char *key, idVec3 &defaultvalue ); //return v
	void						Event_ClearPersistantArgs( void );
	void 						Event_SetPersistantArg( const char *key, const char *value );
	void 						Event_GetPersistantString( idEventReturn *returnContext, const char *key ); //return s
	void 						Event_GetPersistantFloat( idEventReturn *returnContext, const char *key ); //return f
	void 						Event_GetPersistantVector( idEventReturn *returnContext, const char *key ); //return v
	void						Event_AngToForward( idEventReturn *returnContext, idAngles &ang ); //return v
	void						Event_AngToRight( idEventReturn *returnContext, idAngles &ang ); //return v
	void						Event_AngToUp( idEventReturn *returnContext, idAngles &ang ); //return v
	void						Event_GetSine( idEventReturn *returnContext, float angle ); //return f
	void						Event_GetCosine( idEventReturn *returnContext, float angle ); //return f
	void						Event_GetSquareRoot( idEventReturn *returnContext, float theSquare ); //return f
	void						Event_VecNormalize( idEventReturn *returnContext, idVec3 &vec ); //return v
	void						Event_VecLength( idEventReturn *returnContext, idVec3 &vec ); //return f
	void						Event_VecDotProduct( idEventReturn *returnContext, idVec3 &vec1, idVec3 &vec2 ); //return f
	void						Event_VecCrossProduct( idEventReturn *returnContext, idVec3 &vec1, idVec3 &vec2 ); //return v
	void						Event_VecToAngles( idEventReturn *returnContext, idVec3 &vec ); //return v

	void						Event_Vector( idEventReturn *returnContext, float x, float y, float z );
	void						Event_VecLerp( idEventReturn *returnContext, idVec3 &zero, idVec3 &one, float part );
	void						Event_VecProjectPlane( idEventReturn *returnContext, idVec3 &vec, idVec3 &normal );
	void						Event_VecTrunc( idEventReturn *returnContext, idVec3 &vec, float max );
	void						Event_VecSnap( idEventReturn *returnContext, idVec3 &vec );

	void						Event_OnSignal( int signal, idEntity *ent, const char *func );
	void						Event_ClearSignalThread( int signal, idEntity *ent );
	void						Event_SetCamera( idEntity *ent );
	void						Event_FirstPerson( void );
	void						Event_Trace( idEventReturn *returnContext, const idVec3 &start, const idVec3 &end, const idVec3 &mins, const idVec3 &maxs, int contents_mask, idEntity *passEntity ); //return f
	void						Event_TracePoint( idEventReturn *returnContext, const idVec3 &start, const idVec3 &end, int contents_mask, idEntity *passEntity ); //return f
	void						Event_GetTraceFraction( idEventReturn *returnContext ); //return f
	void						Event_GetTraceEndPos( idEventReturn *returnContext ); //return v
	void						Event_GetTraceNormal( idEventReturn *returnContext ); //return v
	void						Event_GetTraceEntity( idEventReturn *returnContext ); //return e
	void						Event_GetTraceJoint( idEventReturn *returnContext ); //return s
	void						Event_GetTraceBody( idEventReturn *returnContext ); //return s
	void						Event_FadeIn( idVec3 &color, float time );
	void						Event_FadeOut( idVec3 &color, float time );
	void						Event_FadeTo( idVec3 &color, float alpha, float time );
	void						Event_SetShaderParm( int parmnum, float value );
	void						Event_StartMusic( const char *name );
	void						Event_ChangeMusic( const char *text, float fadeout );
	void						Event_Warning( const char *text );
	void						Event_Error( const char *text );
	void 						Event_StrLen( idEventReturn *returnContext, const char *string ); //return d
	void 						Event_StrLeft( idEventReturn *returnContext, const char *string, int num ); //return s
	void 						Event_StrRight( idEventReturn *returnContext, const char *string, int num ); //return s
	void 						Event_StrSkip( idEventReturn *returnContext, const char *string, int num ); //return s
	void 						Event_StrMid( idEventReturn *returnContext, const char *string, int start, int num ); //return s
	void						Event_StrToFloat( idEventReturn *returnContext, const char *string ); //return f
	void						Event_RadiusDamage( const idVec3 &origin, idEntity *inflictor, idEntity *attacker, idEntity *ignore, const char *damageDefName, float dmgPower );
	void						Event_IsClient( idEventReturn *returnContext ); //return f
	void 						Event_IsMultiplayer( idEventReturn *returnContext ); //return f
	void 						Event_GetFrameTime( idEventReturn *returnContext ); //return f
	void 						Event_GetTicsPerSecond( idEventReturn *returnContext ); //return f
	void						Event_CacheSoundShader( const char *soundName );
	void						Event_DebugLine( const idVec3 &color, const idVec3 &start, const idVec3 &end, const float lifetime );
	void						Event_DebugArrow( const idVec3 &color, const idVec3 &start, const idVec3 &end, const int size, const float lifetime );
	void						Event_DebugCircle( const idVec3 &color, const idVec3 &origin, const idVec3 &dir, const float radius, const int numSteps, const float lifetime );
	void						Event_DebugBounds( const idVec3 &color, const idVec3 &mins, const idVec3 &maxs, const float lifetime );
	void						Event_DrawText( const char *text, const idVec3 &origin, float scale, const idVec3 &color, const int align, const float lifetime );
	void						Event_HUDMSG_All( const char *text, const idVec3 &origin, const idVec3 &box, const float lifetime );
	void						Event_InfluenceActive( idEventReturn *returnContext ); //return d
	void						Event_Luck( idEventReturn *returnContext, const float chance ); //return d
	void						Event_Text_Align( const float align );
	void						Event_Text_Scale( const float scale );
	void						Event_Text_Spacing( const float spacing );
	void						Event_Text_Font( const char* font );
	void						Event_Text_Color( const idVec3 &color, const float _alpha );
	void						Event_SpawnParticle( const char *particleName, const idVec3 &origin );
	void						Event_SetGameoverSeq( const char *funcName );

public:							
								CLASS_PROTOTYPE( idThread );
								
								idThread();
								idThread( idEntity *self, const function_t *func );
								idThread( const function_t *func );
								idThread( idInterpreter *source, const function_t *func, int args );
								idThread( idInterpreter *source, idEntity *self, const function_t *func, int args );
								idThread( const idScriptContext *scon, const char* threadName);

	virtual						~idThread();

								// tells the thread manager not to delete this thread when it ends
	void						ManualDelete( void );

	// save games
	void						Save( idSave_I *savefile ) const;				// archives object for save game file
	void						Restore( idRestore_I *savefile );				// unarchives object from save game file

	void						EnableDebugInfo( void ) { interpreter.debug = true; };
	void						DisableDebugInfo( void ) { interpreter.debug = false; };

	void						WaitMS( int time );
	void						WaitSec( float time );
	void						WaitFrame( void );
								
								// NOTE: If this is called from within a event called by this thread, the function arguments will be invalid after calling this function.
	void						CallFunction( const function_t	*func, bool clearStack );

								// NOTE: If this is called from within a event called by this thread, the function arguments will be invalid after calling this function.
	void						CallFunction( idEntity *obj, const function_t *func, bool clearStack );

								// c4tnt:Working with stack
	void						ParamI( const int &parm );
	void						ParamF( const float &parm );
	void						ParamS( const char* parm );
	void						ParamV( const idVec3 &parm );
	void						ParamE( const idEntity *parm );
	bool						CompileParamD( const idDict &parms, const idTypeDef* func_type );

	void						DisplayInfo();
	static idThread				*GetThread( int num );
	static void					ListThreads_f( const idCmdArgs &args );
	static void					Restart( void );
	static void					ObjectMoveDone( int threadnum, idEntity *obj );
								
	static idList<idThread*>&	GetThreads ( void );
	
	bool						IsDoneProcessing ( void );
	bool						IsDying			 ( void );	
								
	void						End( void );
	static void					KillThread( const char *name );
	static void					KillThread( int num );
	bool						Execute( void );
	void						ManualControl( void ) { manualControl = true; CancelEvents( &EV_Thread_Execute ); };
	void						DoneProcessing( void ) { interpreter.doneProcessing = true; };
	void						ContinueProcessing( void ) { interpreter.doneProcessing = false; };
	bool						ThreadDying( void ) { return interpreter.threadDying; };
	void						EndThread( void ) { interpreter.threadDying = true; };
	bool						IsWaiting( void );
	void						ClearWaitFor( void );
	bool						IsWaitingFor( idEntity *obj );
	void						ObjectMoveDone( idEntity *obj );
	void						ThreadCallback( idThread *thread );
	void						DelayedStart( int delay );
	bool						Start( void );
	idThread					*WaitingOnThread( void );
	void						SetThreadNum( int num );
	int 						GetThreadNum( void );
	void						SetThreadName( const char *name );
	const char					*GetThreadName( void );

	void						Error( const char *fmt, ... ) const id_attribute((format(printf,2,3)));
	void						Warning( const char *fmt, ... ) const id_attribute((format(printf,2,3)));
								
	static idThread				*CurrentThread( void );
	static int					CurrentThreadNum( void );
	static bool					BeginMultiFrameEvent( idEntity *ent, const idEventDef *event );
	static void					EndMultiFrameEvent( idEntity *ent, const idEventDef *event );
};

/*
================
idThread::WaitingOnThread
================
*/
ID_INLINE idThread *idThread::WaitingOnThread( void ) {
	return waitingForThread;
}

/*
================
idThread::SetThreadNum
================
*/
ID_INLINE void idThread::SetThreadNum( int num ) {
	threadNum = num;
}

/*
================
idThread::GetThreadNum
================
*/
ID_INLINE int idThread::GetThreadNum( void ) {
	return threadNum;
}

/*
================
idThread::GetThreadName
================
*/
ID_INLINE const char *idThread::GetThreadName( void ) {
	return threadName.c_str();
}

/*
================
idThread::GetThreads
================
*/
ID_INLINE idList<idThread*>& idThread::GetThreads ( void ) {
	return threadList;
}	

/*
================
idThread::IsDoneProcessing
================
*/
ID_INLINE bool idThread::IsDoneProcessing ( void ) {
	return interpreter.doneProcessing;
}

/*
================
idThread::IsDying
================
*/
ID_INLINE bool idThread::IsDying ( void ) {
	return interpreter.threadDying;
}

#endif /* !__SCRIPT_THREAD_H__ */
