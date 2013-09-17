// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_SKILL_H__
#define __GAME_SKILL_H__

typedef struct reject_entity_s {
	const idPoolStr* classname;	// Reject entity by class
	const idPoolStr* keyname;	// Reject entity when 'key' is nonzero
} reject_entity_t;
/*
===================================================================================

	Skill settings

===================================================================================
*/


class idSkill {
public:
	idSkill();
	~idSkill();

	//Set skill
	bool					SetSkill( const char* name, const char* playerclass = NULL);

	void					SetKey( const char *key, const char *value );

	const char*				GetKey( const char *key, const char *defaultString ) const;
	float					GetFloatKey( const char *key, const char *defaultString ) const;
	int						GetIntKey( const char *key, const char *defaultString ) const;
	bool					GetBoolKey( const char *key, const char *defaultString ) const;

	bool					ReplaceDefName( const char** name ) const;	//Apply def replacement
	bool					ExplicitEntity( const char* classname, const idDict& keys ) const;	//Explicit entity while spawning
	bool					ApplyCvar( );
	void					debugPrint( void ) const;

	bool					Init();
	void					Clear( bool reset );
	// save games
	void					Save( idSave_I *savefile ) const;					// archives object for save game file
	void					Restore( idRestore_I *savefile );					// unarchives object from save game file

private:

	idDict					Skills;
	idDict					SkillKeys;
	idDict					CVarKeys;
	idDict					EntityReplace;		//Entity def replacement
	idList<reject_entity_t> EntityReject;

	static idStrPool		skillStrings;
};

/*
================
idSkill::GetFloatKey
================
*/
ID_INLINE float idSkill::GetFloatKey( const char *key, const char *defaultString ) const {
	return SkillKeys.GetFloat( key, defaultString );
}

/*
================
idSkill::GetIntKey
================
*/
ID_INLINE int idSkill::GetIntKey( const char *key, const char *defaultString ) const {
	return SkillKeys.GetInt( key, defaultString );
}

/*
================
idSkill::GetBoolKey
================
*/
ID_INLINE bool idSkill::GetBoolKey( const char *key, const char *defaultString ) const {
	return SkillKeys.GetBool( key, defaultString );
}

/*
================
idSkill::GetKey
================
*/
ID_INLINE const char* idSkill::GetKey( const char *key, const char *defaultString ) const {
	const idKeyValue *kv = SkillKeys.FindKey( key );
	if ( kv ) {
		return kv->GetValue().c_str();
	}
	return defaultString;
}

/*
================
idSkill::SetKey
================
*/
ID_INLINE void idSkill::SetKey( const char *key, const char *value ) {
	SkillKeys.Set( key, value );
}
#endif /* !__GAME_SKILL_H__ */
