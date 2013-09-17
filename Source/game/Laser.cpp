#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

#define	LASER_TRACE_STEP	96
#define MAX_LASER_BOUNCE	128
/*
===============================================================================

	idLaser

===============================================================================
*/

const idEventDef EV_Laser_On( "On", NULL );
const idEventDef EV_Laser_Off( "Off", NULL );
const idEventDef EV_Laser_GetTouchEntity( "getTouchEntity", NULL, 'E' );
const idEventDef EV_Laser_Aim( "Aim", "E" );

CLASS_DECLARATION( idEntity, idLaser )
	EVENT( EV_PostSpawn,			idLaser::Event_MatchTarget )
	EVENT( EV_Activate,				idLaser::Event_Activate )
	EVENT( EV_Laser_On,				idLaser::Event_On )
	EVENT( EV_Laser_Off,			idLaser::Event_Off )
	EVENT( EV_Laser_GetTouchEntity, idLaser::Event_GetTouchEntity )
	EVENT( EV_Laser_Aim,			idLaser::Event_AimOn )
END_CLASS

/*
===============
idLaser::idLaser
===============
*/
idLaser::idLaser() {
		targetLaser = NULL;
		latched = NULL;
		enabled = true;
		color.Set(0,0,0);
		endSmoke = NULL;
		endSmokeStartTime = 0;
		endSmokeLastTime = 0;
		nextDamageTime = 0;
		DamageTime = 0;
}

/*
===============
idLaser::~idLaser
===============
*/
idLaser::~idLaser() {
	CropBeam(0);
}

/*
===============
idLaser::Save
===============
*/
void idLaser::Save( idSave_I *savefile ) const {

	targetLaser.Save( savefile );
	latched.Save( savefile );
	savefile->WriteInt(MaxChunks);
	savefile->WriteFloat(beamWidth);
	savefile->WriteString(beamSkin);
	savefile->WriteString(damageDef);
	savefile->WriteBool(enabled);
	savefile->WriteVec3(color);
	savefile->WriteParticle( endSmoke );
	savefile->WriteInt( endSmokeStartTime );
	savefile->WriteInt( DamageTime );
	savefile->WriteInt( nextDamageTime );
}

/*
===============
idLaser::Restore
===============
*/
void idLaser::Restore( idRestore_I *savefile ) {
	targetLaser.Restore( savefile );
	latched.Restore( savefile );
	savefile->ReadInt(MaxChunks);
	savefile->ReadFloat(beamWidth);
	savefile->ReadString(beamSkin);
	savefile->ReadString(damageDef);
	savefile->ReadBool(enabled);
	savefile->ReadVec3(color);
	savefile->ReadParticle( endSmoke );
	savefile->ReadInt( endSmokeStartTime );
	savefile->ReadInt( DamageTime );
	savefile->ReadInt( nextDamageTime );

	CropBeam(0);
}

/*
===============
idLaser::Spawn
===============
*/
void idLaser::Spawn( void ) {

const char* smokeName;

	Chunks.Clear();
	spawnArgs.GetFloat( "width", "0.1", beamWidth );
	DamageTime = SEC2MS(spawnArgs.GetFloat( "damage_delay", "0.2" ));
	spawnArgs.GetString( "skin", "", beamSkin );
	spawnArgs.GetString( "def_damage", "damage_moverCrush", damageDef );
	color = spawnArgs.GetVector( "_color", "1 1 1"); //Read laser color
	if ( spawnArgs.GetBool( "start_off" ) ) {
		enabled = false;
		renderEntity.shaderParms[SHADERPARM_PARTICLE_STOPTIME] = MS2SEC( 1 );
		UpdateVisuals();
	} else {
		enabled = true;
	}

	smokeName = spawnArgs.GetString( "smoke_end" );
	if ( *smokeName != '\0' ) {
		endSmoke = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, smokeName ) );
	} else {
		endSmoke = NULL;
	}

	endSmokeStartTime = 0;
	endSmokeLastTime = 0;
	nextDamageTime = 0;

	PostEventMS( &EV_PostSpawn, NULL, 0 );

}

/*
================
idLaser::Think
================
*/
void idLaser::Think( void ) {
	idEntity *targetEnt;

	targetEnt = targetLaser.GetEntity();

	RunPhysics();
	if ( enabled ) {
		if ( endSmokeStartTime == 0 )
				endSmokeStartTime = gameLocal.time;

		UpdateBeam(targetEnt,spawnArgs.GetFloat( "length", "10000"));

		if ( latched.IsValid() && damageDef[0] != '/0') {
			//Damage touching
			if ( nextDamageTime < gameLocal.time ) {
				latched.GetEntity()->Damage( NULL, NULL, vec3_origin, damageDef.c_str(), 1.0f, INVALID_JOINT );
				nextDamageTime = gameLocal.time + DamageTime;
			}
		}
	}else{
		if ( CropBeam(0) ) UpdateVisuals();
		BecomeInactive( TH_THINK );
		endSmokeStartTime = 0;
		latched = NULL;
	}
	Present();
}

/*
================
idLaser::UpdateBeam
================
*/
void idLaser::UpdateBeam( const idEntity *targetEnt, const float Length ) {

	idVec3 origin; 
	const idMat3& ax = GetPhysics()->GetAxis(); 
	idVec3 end; 
	idVec3 dir; 
	idVec3 tmp; 
	int count;
	int retraces;
	bool needUpdate;
	bool canReflect;
	idEntity* ContactEnt;
	trace_t tr;
	idBounds  LaserSize = idBounds(vec3_origin).Expand(beamWidth);
	
	origin = GetPhysics()->GetOrigin();
	if ( targetEnt ) {
		dir = targetEnt->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();
		dir.Normalize();
	}else{
		dir = ax.Transpose()[0];
		dir.Normalize();
	}

	count = 0;
	needUpdate = false;
	
	while (count < MAX_LASER_BOUNCE) {

		end = origin + dir * LASER_TRACE_STEP;
		retraces = Length / LASER_TRACE_STEP;
		tmp = origin;
		while (!gameLocal.clip.TraceBounds( tr, tmp, end, LaserSize, MASK_SHOT_RENDERMODEL, this ) && retraces > 0) {
			tmp = end;
			end = tmp + dir * LASER_TRACE_STEP;
			retraces--;
		}

		needUpdate |= AppendBeam(count, origin, tr.endpos, needUpdate); 
		canReflect = false;
		if (tr.c.entityNum < ENTITYNUM_MAX_NORMAL) {
			ContactEnt = gameLocal.entities[tr.c.entityNum];
		}else{
			ContactEnt = NULL;
		}
		if (tr.c.material != NULL && tr.c.material->GetSurfaceType() == SURFTYPE_GLASS) {
			canReflect = true;
		}
		if (ContactEnt != NULL && ContactEnt->IsType(idBrittleFracture::Type)) {
			canReflect = true;
		}
		
		if ((tr.fraction < 1.0f) && canReflect) {
			//Reflect
			origin = tr.endpos;
			dir = dir - tr.c.normal * (2 * (tr.c.normal * dir));
			dir.Normalize();
		} else {
			//Drop
			needUpdate |= CropBeam(count+1); 

			if ( !(latched == ContactEnt) ) {
				nextDamageTime = 0;
				latched = ContactEnt;
			}

			//Emit new smokes here
			dir = -dir;
			tmp = gameLocal.GetGravity();
			tmp.Normalize();

			idMat3 naxis( tmp, dir.Cross(tmp) , dir );
			if ( !gameLocal.smokeParticles->EmitSmoke( endSmoke, endSmokeStartTime, gameLocal.random.RandomFloat(), tr.endpos, naxis, endSmokeLastTime ) ) {
				endSmokeStartTime = gameLocal.time;
			}
			if ( needUpdate ) {
				endSmokeLastTime = 0;
			} else {
				endSmokeLastTime = gameLocal.time;
			}

			break;
		}
		count++;
	}
	if ( needUpdate ) UpdateVisuals();
}

/*
================
idLaser::AppendBeam
================
*/
bool idLaser::AppendBeam( int IDX, idVec3 &Start, idVec3 &End, bool forceUpdate ) {
idVec3 dir;


	if ( IDX >= Chunks.Num() ) {
		//Locate a new model
		LaserChunk_t amodel;
		memset( &amodel, 0, sizeof( amodel ) );
		dir = End - Start;
		dir.Normalize();

		amodel.renderEntitySegment.origin = Start;
		amodel.renderEntitySegment.axis = dir.ToMat3();
		amodel.renderEntitySegment.shaderParms[ SHADERPARM_BEAM_WIDTH ] = beamWidth;
		amodel.renderEntitySegment.shaderParms[ SHADERPARM_BEAM_END_X ] = End.x;
		amodel.renderEntitySegment.shaderParms[ SHADERPARM_BEAM_END_Y ] = End.y;
		amodel.renderEntitySegment.shaderParms[ SHADERPARM_BEAM_END_Z ] = End.z;
		amodel.renderEntitySegment.shaderParms[ SHADERPARM_RED ] = color.x;
		amodel.renderEntitySegment.shaderParms[ SHADERPARM_GREEN ] = color.y;
		amodel.renderEntitySegment.shaderParms[ SHADERPARM_BLUE ] = color.z;
		amodel.renderEntitySegment.shaderParms[ SHADERPARM_ALPHA ] = 1.0f;
		amodel.renderEntitySegment.shaderParms[ SHADERPARM_DIVERSITY] = gameLocal.random.CRandomFloat() * 0.75;
		amodel.renderEntitySegment.hModel = renderModelManager->FindModel( "_beam" );
		amodel.renderEntitySegment.callback = NULL;
		amodel.renderEntitySegment.numJoints = 0;
		amodel.renderEntitySegment.joints = NULL;
		amodel.renderEntitySegment.bounds.Clear();
		amodel.renderEntitySegment.customShader = declManager->FindMaterial( beamSkin );
		amodel.modelDefHandleSegment = gameRenderWorld->AddEntityDef( &amodel.renderEntitySegment );

		Chunks.Append( amodel ); 

		return true;
	}else{
		//Use old model
		LaserChunk_t* current;
		
		current = &Chunks[IDX];
		dir = End - Start;
		dir.Normalize();

		if (current->modelDefHandleSegment != -1) {
			if ((current->SavedEnd == End) && !forceUpdate) return false;
			current->renderEntitySegment.origin = Start;
			current->renderEntitySegment.axis = dir.ToMat3();
			current->renderEntitySegment.shaderParms[ SHADERPARM_BEAM_WIDTH ] = beamWidth;
			current->renderEntitySegment.shaderParms[ SHADERPARM_BEAM_END_X ] = End.x;
			current->renderEntitySegment.shaderParms[ SHADERPARM_BEAM_END_Y ] = End.y;
			current->renderEntitySegment.shaderParms[ SHADERPARM_BEAM_END_Z ] = End.z;
			current->SavedEnd = End;
			current->renderEntitySegment.shaderParms[ SHADERPARM_RED ] = color.x;
			current->renderEntitySegment.shaderParms[ SHADERPARM_GREEN ] = color.y;
			current->renderEntitySegment.shaderParms[ SHADERPARM_BLUE ] = color.z;
			current->renderEntitySegment.shaderParms[ SHADERPARM_ALPHA ] = 1.0f;
			current->renderEntitySegment.shaderParms[ SHADERPARM_DIVERSITY] = gameLocal.random.CRandomFloat() * 0.75;
			gameRenderWorld->UpdateEntityDef( current->modelDefHandleSegment, &current->renderEntitySegment );
			return true;
		}else{
			current->renderEntitySegment.origin = Start;
			current->renderEntitySegment.axis = dir.ToMat3();
			current->renderEntitySegment.shaderParms[ SHADERPARM_BEAM_WIDTH ] = beamWidth;
			current->renderEntitySegment.shaderParms[ SHADERPARM_BEAM_END_X ] = End.x;
			current->renderEntitySegment.shaderParms[ SHADERPARM_BEAM_END_Y ] = End.y;
			current->renderEntitySegment.shaderParms[ SHADERPARM_BEAM_END_Z ] = End.z;
			current->SavedEnd = End;
			current->renderEntitySegment.shaderParms[ SHADERPARM_RED ] = color.x;
			current->renderEntitySegment.shaderParms[ SHADERPARM_GREEN ] = color.y;
			current->renderEntitySegment.shaderParms[ SHADERPARM_BLUE ] = color.z;
			current->renderEntitySegment.shaderParms[ SHADERPARM_ALPHA ] = 1.0f;
			current->renderEntitySegment.shaderParms[ SHADERPARM_DIVERSITY] = gameLocal.random.CRandomFloat() * 0.75;
			current->modelDefHandleSegment = gameRenderWorld->AddEntityDef( &current->renderEntitySegment );
			return true;
		}

		return false;
	}
}

/*
================
idLaser::CropBeam
================
*/
bool idLaser::CropBeam( int IDX ) {
int i;
LaserChunk_t* current;

	if ( IDX >= Chunks.Num() ) return false;
	for (i = IDX; i < Chunks.Num(); i++) {
		current = &Chunks[i];
		if (current->modelDefHandleSegment != -1) {
			gameRenderWorld->FreeEntityDef( current->modelDefHandleSegment );
		}
	}
	while ( IDX < Chunks.Num() )
	{
		Chunks.RemoveIndex(IDX);
	}
	
	return true;
}

/*
================
idLaser::Event_Activate
================
*/
void idLaser::Event_Activate( idEntity *activator ) {
	if ( IsHidden() ) {
		Show();
		enabled = true;
		BecomeActive( TH_THINK );
	} else {
		enabled = !enabled;
		if (enabled) BecomeActive( TH_THINK );
	}
}

/*
================
idLaser::Event_MatchTarget
================
*/
void idLaser::Event_MatchTarget( void ) {
	int i;

	idEntity *targetEnt = NULL;

	for( i = 0; i < targets.Num(); i++ ) {
		targetEnt = targets[ i ].GetEntity();
		if ( targetEnt ) {
			break;
		}
	}

	if ( !targetEnt ) {
		targetLaser = NULL;
	}else{
		targetLaser = targetEnt;
	}
	
	if ( enabled ) {
		BecomeActive( TH_THINK );
		Show();
		UpdateBeam(targetEnt,spawnArgs.GetFloat( "length", "10000"));
	}
}

/*
================
idLaser::Event_On
================
*/
void idLaser::Event_On( void ) {
	enabled = true;
	BecomeActive( TH_THINK );
}

/*
================
idLaser::Event_Off
================
*/
void idLaser::Event_Off( void ) {
	enabled = false;
}

/*
================
idLaser::Event_GetTouchEntity
================
*/
void idLaser::Event_GetTouchEntity( idEventReturn *returnContext ) {
	returnContext->FinallyEntity( latched.GetEntity());
}

/*
================
idLaser::Event_AimOn
================
*/
void idLaser::Event_AimOn( idEntity *ent ) {
	idEntity *targetEnt = NULL;
	int i;

	if (!ent) {
		for( i = 0; i < targets.Num(); i++ ) {
			targetEnt = targets[ i ].GetEntity();
			if ( targetEnt ) {
				break;
			}
		}

		if ( !targetEnt ) {
			targetLaser = NULL;
		}else{
			targetLaser = targetEnt;
		}	
	} else {
		targetLaser = ent;
	}
}

/*
================
idLaser::WriteToSnapshot
c4tnt: not ready yet
================
*/
void idLaser::WriteToSnapshot( idBitMsgDelta &msg ) const {
	GetPhysics()->WriteToSnapshot( msg );
	WriteBindToSnapshot( msg );
	WriteColorToSnapshot( msg );
}

/*
================
idLaser::ReadFromSnapshot
c4tnt: not ready yet
================
*/
void idLaser::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	GetPhysics()->ReadFromSnapshot( msg );
	ReadBindFromSnapshot( msg );
	ReadColorFromSnapshot( msg );
	if ( msg.HasChanged() ) {
		UpdateVisuals();
	}
}
