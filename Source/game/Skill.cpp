#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

idStrPool		idSkill::skillStrings;

/*
================
idSkill::idSkill
================
*/
idSkill::idSkill( ) {
	Clear( true );
}

idSkill::~idSkill( ) {
	Clear( true );
}
/*
================
idSkill::SetSkill
================
*/
bool idSkill::SetSkill( const char* name, const char* playerclass ) {
	const idKeyValue *kv;
	const idDict* edef;
	reject_entity_t* rj;
	const char* cptr;

	int i, c;

	kv = NULL;

	if ( playerclass ) {
		// search player-specified skill, such as doom_marine::nightmare
		kv = Skills.FindKey( va( "%s::%s", playerclass, name ) );
	}
	
	if (!kv)
		kv = Skills.FindKey( name );

	if (!kv)
		return false;

	edef = gameLocal.FindEntityDefDict( kv->GetValue(), false );

	if (!edef)
		return false;

	Clear( false );


	// Load skill from the 'edef'
	c = edef->GetNumKeyVals();
	for ( i = 0; i < c; i++ ) {
		kv = edef->GetKeyVal( i );

		if ( !kv->GetKey().IcmpPrefix( "replace " ) ) {
			EntityReplace.Set( &kv->GetKey().c_str()[8], kv->GetValue() );
			continue;
		}

		if ( !kv->GetKey().IcmpPrefix( "reject" ) ) {
			rj = &EntityReject.Alloc();
			cptr = kv->GetKey().c_str();
			while ( *cptr ) {
				if ( *cptr == ' ' ) break;
				cptr++;
			}
			while ( *cptr ) {
				if ( *cptr != ' ' ) break;
				cptr++;
			}

			if (!cptr || !*cptr) {
				rj->classname = NULL;
			}else{
				rj->classname = skillStrings.AllocString( cptr );
			}

			cptr = kv->GetValue().c_str();

			if (!cptr || !*cptr) {
				rj->keyname = NULL;
			}else{
				rj->keyname = skillStrings.AllocString( cptr );
			}

			if ( !rj->classname && !rj->keyname )
				gameLocal.Error("All map was deprecated by: '%s' '%s'", kv->GetKey(), kv->GetValue());
			continue;
		}

		if ( cvarSystem->Find( kv->GetKey() ) ) {
			CVarKeys.Set( kv->GetKey(), kv->GetValue() );
			continue;
		}

		SkillKeys.Set( kv->GetKey(), kv->GetValue() );
	}
	return true;
}

void idSkill::debugPrint( void ) const {
	int i;
	const idKeyValue *kv;
	const reject_entity_t* rj;

	gameLocal.Printf( "Skill keys:\n" );
	i = SkillKeys.GetNumKeyVals();
	if ( !i ) {
		gameLocal.Printf( "    <empty>\n" );
	} else {
		while (i > 0) {
			i--;
			kv = SkillKeys.GetKeyVal( i );
			gameLocal.Printf( "    '%s' : '%s' \n", kv->GetKey().c_str(), kv->GetValue().c_str() );
		}
	}

	gameLocal.Printf( "Replaces:\n" );
	i = EntityReplace.GetNumKeyVals();
	if ( !i ) {
		gameLocal.Printf( "    <empty>\n" );
	} else {
		while (i > 0) {
			i--;
			kv = EntityReplace.GetKeyVal( i );
			gameLocal.Printf( "    '%s' => '%s' \n", kv->GetKey().c_str(), kv->GetValue().c_str() );
		}
	}

	gameLocal.Printf( "Rejects:\n" );
	i = EntityReject.Num();
	if ( !i ) {
		gameLocal.Printf( "    <empty>\n" );
	} else {
		while (i > 0) {
			i--;
			rj = &EntityReject[i];
			gameLocal.Printf( "    cls: " );
			if ( !rj->classname ) {
				gameLocal.Printf( "* " );
			}else{
				gameLocal.Printf( "%s", rj->classname->c_str() );
			}
			gameLocal.Printf( " key: " );
			if ( !rj->keyname ) {
				gameLocal.Printf( "* " );
			}else{
				gameLocal.Printf( "%s", rj->keyname->c_str() );
			}
			gameLocal.Printf( "\n" );
		}
	}
}

/*
================
idSkill::ReplaceDefName
================
*/
bool idSkill::ReplaceDefName( const char** name ) const {
	
	if ( !name ) return false;

	const idKeyValue *kv = EntityReplace.FindKey( *name );
	
	if ( !kv ) return false;

	*name = kv->GetValue().c_str();

	return true;
}

/*
================
idSkill::ExplicitEntity
================
*/
bool idSkill::ExplicitEntity( const char* classname, const idDict& keys ) const {
	
	int i, c;
	const reject_entity_t* rj;
	
	c = EntityReject.Num();
	for ( i = 0; i < c; i++ ) {
		rj = &EntityReject[i];
		if ( !rj->classname || !idStr::Icmpf(classname, rj->classname->c_str()) ) {
			//Compare class with wildcards
			if ( !rj->keyname || keys.GetBool( rj->keyname->c_str())) {
				return true;
			}
		}
	}
	return false;
}

/*
================
idSkill::ApplyCvar
================
*/
bool idSkill::ApplyCvar( ) {
	int i, c;
	const idKeyValue* kv;
	idCVar* cvar;

	c = CVarKeys.GetNumKeyVals();
	for ( i = 0; i < c; i++ ) {
		kv = CVarKeys.GetKeyVal( i );
		cvar = cvarSystem->Find( kv->GetKey() );
		if (cvar) {
			cvar->SetString( kv->GetValue() );
		}
	}
	return true;
}

/*
================
idSkill::Init
================
*/
bool idSkill::Init() {
	const idDict* skilllist = gameLocal.FindEntityDefDict( "skills", false );
	
	if ( !skilllist ) {
		return false;
	}

	Skills.Copy( *skilllist );
	return true;
}

/*
================
idSkill::Clear
================
*/
void idSkill::Clear( bool reset ) {
	int i, c;
	const reject_entity_t* rj;

	if ( reset ) Skills.Clear();

	SkillKeys.Clear();
	CVarKeys.Clear();
	EntityReplace.Clear();

	c = EntityReject.Num();
	for ( i = 0; i < c; i++ ) {
		rj = &EntityReject[i];
		if (rj->classname) {
			skillStrings.FreeString( rj->classname );
		}
		if (rj->keyname) {
			skillStrings.FreeString( rj->keyname );
		}
	}

	EntityReject.Clear();
}

void idSkill::Save( idSave_I *savefile ) const {
	int i, c, f;
	const reject_entity_t* rj;
	
	savefile->WriteDict( &SkillKeys );
	savefile->WriteDict( &CVarKeys );
	savefile->WriteDict( &EntityReplace );
	c = EntityReject.Num();
	savefile->WriteInt( c );
	for ( i = 0; i < c; i++ ) {
		rj = &EntityReject[i];
		f = 0;
		if (rj->classname) f |= 1;
		if (rj->keyname) f |= 2;
		savefile->WriteInt( f );
		if (f & 1) savefile->WriteString( rj->classname->c_str() );
		if (f & 2) savefile->WriteString( rj->keyname->c_str() );
	}

}

void idSkill::Restore( idRestore_I *savefile ) {
	int i, c, f;
	reject_entity_t* rj;
	idStr rstr;

	Init();
	Clear( false );

	savefile->ReadDict( &SkillKeys );
	savefile->ReadDict( &CVarKeys );
	savefile->ReadDict( &EntityReplace );
	savefile->ReadInt( c );
	for ( i = 0; i < c; i++ ) {
		savefile->ReadInt( f );
		if (f) {
			rj = &EntityReject.Alloc();
			rj->classname = NULL;
			rj->keyname = NULL;

			if (f & 1) {
				savefile->ReadString( rstr );
				rj->classname = skillStrings.AllocString( rstr.c_str() );
			}
			if (f & 2) {
				savefile->ReadString( rstr );
				rj->keyname = skillStrings.AllocString( rstr.c_str() );
			}
		}
	}
}
