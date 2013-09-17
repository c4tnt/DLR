#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

CLASS_DECLARATION( idEntity, idSIPipe )
	EVENT( EV_Activate,			idSIPipe::Event_Activate )
END_CLASS

/*
===============
idSIPipe::idSIPipe
===============
*/
idSIPipe::idSIPipe() {
	smokeTime = 0;
	smoke = NULL;
	health = 0;
	On = false;
	invencible = false;
	vaporPoints.Clear();
	particleFwd.x = 0;
	particleFwd.y = 1;
	particleFwd.z = 0;
	cursmoke = NULL;
}

/*
===============
idSIPipe::~idSIPipe
===============
*/
idSIPipe::~idSIPipe() {
	RemoveVapors();
}

/*
===============
idSIPipe::RemoveVapors()
===============
*/

void idSIPipe::RemoveVapors() {
	vaporPoints.Clear();
}

/*
===============
idSIPipe::Spawn
===============
*/
void idSIPipe::Spawn( void ) {
	bool solid;
	bool hidden;
	int i;

	solid = spawnArgs.GetBool( "solid" );
	hidden = spawnArgs.GetBool( "hide" );
	health = spawnArgs.GetInt( "health", "5" );

	particleFwd = spawnArgs.GetVector( "particleOrient", "0 1 0" );

	spawnArgs.SetBool( "bleed", 1 );

	vaporPoints.Clear();

	const char *smokeName = spawnArgs.GetString( "smoke" );
	if ( *smokeName != '\0' ) {
		smoke = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, smokeName ) );
	} else {
		smoke = NULL;
	}

	if ( solid && !hidden ) {
		GetPhysics()->SetContents( CONTENTS_SOLID );
	} else {
		GetPhysics()->SetContents( 0 );
	}

	fl.takedamage = true;

	if (health <= 0)
		invencible = true;

	if ( spawnArgs.GetBool( "start_off" ) ) {
		for ( i = 0; i < vaporPoints.Num(); i++ )
		{	
			vaporPoints[i].localTime = 0;
		}
		On = false;
	} else if ( smoke ) {
		if (hidden)
		{
			for ( i = 0; i < vaporPoints.Num(); i++ )
			{	
				vaporPoints[i].localTime = 0;
			}
		}else{
			for ( i = 0; i < vaporPoints.Num(); i++ )
			{	
				vaporPoints[i].localTime = gameLocal.time;
			}
			BecomeActive( TH_UPDATEPARTICLES );
		}
		On = true;
	}
}

/*
===============
idSIPipe::Save
===============
*/
void idSIPipe::Save( idSave_I *savefile ) const {
int i;	
	savefile->WriteInt( smokeTime );
	savefile->WriteParticle( smoke );
	savefile->WriteBool( On );
	savefile->WriteBool( invencible );
	savefile->WriteInt( health );

	savefile->WriteInt(vaporPoints.Num());
	for ( i = 0; i < vaporPoints.Num(); i++ ) {
		savefile->WriteVec3(vaporPoints[i].origin);
		savefile->WriteVec3(vaporPoints[i].dir);
		savefile->WriteInt(vaporPoints[i].localTime);
		savefile->WriteInt(vaporPoints[i].localLATime);
		savefile->WriteMat3(vaporPoints[i].ax);
	}
}

/*
===============
idSIPipe::Restore
===============
*/
void idSIPipe::Restore( idRestore_I *savefile ) {
int num;
int i;
damagePoints_t* dpt;

	savefile->ReadInt( smokeTime );
	savefile->ReadParticle( smoke );
	savefile->ReadBool( On );
	savefile->ReadBool( invencible );
	savefile->ReadInt( health );

	savefile->ReadInt( num );
	vaporPoints.SetNum( num );
	for ( i = 0; i < num; i++ ) {
		dpt = &vaporPoints.Alloc();
		savefile->ReadVec3(dpt->origin);
		savefile->ReadVec3(dpt->dir);
		savefile->ReadInt(dpt->localTime);
		savefile->ReadInt(dpt->localLATime);
		savefile->ReadMat3(dpt->ax);
	}
}

/*
===============
idSIPipe::Think
===============
*/

void idSIPipe::Think(void)
{
int i;
bool bi = true;
int vpnum;

	idEntity::Think();
	if ( CheckDormant() || smoke == NULL) {
		return;
	}
	if ( ( thinkFlags & TH_UPDATEPARTICLES) && !IsHidden() ) {
		vpnum = vaporPoints.Num();
		for ( i = 0; i < vpnum; i++ )
		{	
			if ( !gameLocal.smokeParticles->EmitSmoke( smoke, vaporPoints[i].localTime, gameLocal.random.CRandomFloat(),vaporPoints[i].origin, vaporPoints[i].ax, vaporPoints[i].localLATime)) 
			{
				bi = false;
				if ( On ) {
					vaporPoints[i].localTime = gameLocal.time;
				} else {
					vaporPoints[i].localTime = 0;
				}
			}
			if ( On ) vaporPoints[i].localLATime = gameLocal.time;
		}
	}
	if (bi && !On) BecomeInactive( TH_UPDATEPARTICLES );
}

/*
================
idSIPipe::Hide
================
*/
void idSIPipe::Hide( void ) {
	idEntity::Hide();
	GetPhysics()->SetContents( 0 );
}

/*
================
idSIPipe::Show
================
*/
void idSIPipe::Show( void ) {
int i;

	idEntity::Show();
	if ( spawnArgs.GetBool( "solid" ) ) {
		GetPhysics()->SetContents( CONTENTS_SOLID );
	}

	if (!( thinkFlags & TH_UPDATEPARTICLES ) && On) {
		BecomeActive( TH_UPDATEPARTICLES );
		for ( i = 0; i < vaporPoints.Num(); i++ )
		{	
			vaporPoints[i].localTime = gameLocal.time;
		}
	}
}

/*
============
idSIPipe::AddDamageEffect
============
*/
void idSIPipe::AddDamageEffect( const trace_t &collision, const idVec3 &velocity, const char *damageDefName ) {

	const char *decal, *sound, *typeName;
	surfTypes_t materialType;
	
	AddVapor(collision.c.point,collision.c.normal);

	if ( collision.c.material != NULL ) {
		materialType = collision.c.material->GetSurfaceType();
	} else {
		materialType = SURFTYPE_METAL;
	}

	// get material type name
	typeName = gameLocal.sufaceTypeNames[ materialType ];

	// play impact sound
	sound = spawnArgs.GetString( va( "snd_%s", typeName ) );
	if ( *sound == '\0' ) {
		sound = spawnArgs.GetString( "snd_metal" );
	}
	if ( *sound == '\0' ) {
		sound = spawnArgs.GetString( "snd_impact" );
	}
	if ( *sound != '\0' ) {
		StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_BODY, 0, false, NULL );
	}

	// project decal
	decal = spawnArgs.GetString( va( "mtr_detonate_%s", typeName ) );
	if ( *decal == '\0' ) {
		decal = spawnArgs.GetString( "mtr_detonate" );
	}
	if ( *decal != '\0' ) {
		
		gameLocal.ProjectDecal( collision.c.point, -collision.c.normal, 8.0f, true, spawnArgs.GetFloat( "decal_size", "6.0" ), decal );
	}
}

/*
============
idSIPipe::Pain
============
*/
bool idSIPipe::Pain( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
	return true;
}

/*
============
idSIPipe::Killed
============
*/
void idSIPipe::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
	On = false;
	StopSound( SND_CHANNEL_ANY, false );
	health = spawnArgs.GetInt( "health", "5" );
	if (!invencible)
	{
		RemoveVapors();
		Hide();
	}
}

/*
===============
idSIPipe::Event_Activate
===============
*/

void idSIPipe::Event_Activate( idEntity *activator ) {
int i;

	if ( thinkFlags & TH_UPDATEPARTICLES ) {
		On = false;
		return;
	} else {
		if (!IsHidden() ) {
			BecomeActive( TH_UPDATEPARTICLES );
		}
		for ( i = 0; i < vaporPoints.Num(); i++ )
		{	
			vaporPoints[i].localTime = gameLocal.time;
		}
		On = true;
	}
}

/*
===============
idSIPipe::AddVapor
===============
*/

void idSIPipe::AddVapor(const idVec3 &pos,const idVec3 &velocity)
{
	damagePoints_t* ptr;
	idMat3 axis;
	idVec3 tmpv;

	tmpv = velocity;
	tmpv.Normalize();

	axis = velocity.ToMat3();
	tmpv = axis[2];
	axis[2] = axis[0];
	axis[0] = -tmpv;

	ptr = &vaporPoints.Alloc();
	ptr->ax = axis;
	ptr->dir = velocity;
	ptr->localTime = gameLocal.time;
	ptr->origin = pos;
}



CLASS_DECLARATION( idEntity, idEnergetical )
	EVENT( EV_Activate,			idEnergetical::Event_Activate )
END_CLASS

/*
===============
idEnergetical::idEnergetical
===============
*/
idEnergetical::idEnergetical() {
	On = false;
}

/*
===============
idEnergetical::~idEnergetical
===============
*/
idEnergetical::~idEnergetical() {
return;
}

/*
===============
idEnergetical::Spawn
===============
*/
void idEnergetical::Spawn( void ) {
	bool solid;
	bool hidden;

	solid = spawnArgs.GetBool( "solid" );
	hidden = spawnArgs.GetBool( "hide" );

	if ( solid && !hidden ) {
		GetPhysics()->SetContents( CONTENTS_SOLID );
	} else {
		GetPhysics()->SetContents( 0 );
	}

	fl.takedamage = false;

	makeEdges(renderEntity.hModel);

	if ( spawnArgs.GetBool( "start_off" ) ) {
		On = false;
	} else {
		On = true;
		BecomeActive( TH_THINK );
	}
}

/*
===============
idEnergetical::Save
===============
*/
void idEnergetical::Save( idSave_I *savefile ) const {

	savefile->WriteBool( On );

}

/*
===============
idEnergetical::Restore
===============
*/
void idEnergetical::Restore( idRestore_I *savefile ) {

	savefile->ReadBool( On );

}

/*
===============
idEnergetical::Think
===============
*/

void idEnergetical::Think(void)
{

	idEntity::Think();
	if ( CheckDormant()) {
		return;
	}
	Debug_Edges();
	if (!On) BecomeInactive( TH_THINK );
}

/*
================
idEnergetical::Hide
================
*/
void idEnergetical::Hide( void ) {
	idEntity::Hide();
	GetPhysics()->SetContents( 0 );
}

/*
================
idEnergetical::Show
================
*/
void idEnergetical::Show( void ) {

	idEntity::Show();
	if ( spawnArgs.GetBool( "solid" ) ) {
		GetPhysics()->SetContents( CONTENTS_SOLID );
	}
	BecomeActive( TH_THINK );
}

/*
============
idEnergetical::AddDamageEffect
============
*/
void idEnergetical::AddDamageEffect( const trace_t &collision, const idVec3 &velocity, const char *damageDefName ) {

	const char *decal, *sound, *typeName;
	surfTypes_t materialType;
	
	if ( collision.c.material != NULL ) {
		materialType = collision.c.material->GetSurfaceType();
	} else {
		materialType = SURFTYPE_METAL;
	}

	// get material type name
	typeName = gameLocal.sufaceTypeNames[ materialType ];

	// play impact sound
	sound = spawnArgs.GetString( va( "snd_%s", typeName ) );
	if ( *sound == '\0' ) {
		sound = spawnArgs.GetString( "snd_metal" );
	}
	if ( *sound == '\0' ) {
		sound = spawnArgs.GetString( "snd_impact" );
	}
	if ( *sound != '\0' ) {
		StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_BODY, 0, false, NULL );
	}

	// project decal
	decal = spawnArgs.GetString( va( "mtr_detonate_%s", typeName ) );
	if ( *decal == '\0' ) {
		decal = spawnArgs.GetString( "mtr_detonate" );
	}
	if ( *decal != '\0' ) {
		
		gameLocal.ProjectDecal( collision.c.point, -collision.c.normal, 8.0f, true, spawnArgs.GetFloat( "decal_size", "6.0" ), decal );
	}
}

/*
===============
idEnergetical::Debug_Edges
===============
*/

void idEnergetical::Debug_Edges( ) {
	int i;
	const modelSurface_t *surf;
	const idDrawVert* v;
	const idDrawVert* v2;
	idxedge_t* curEdge;
	idVec3 org;

	if ( !renderEntity.hModel ) {
		return;
	}
	org = renderEntity.origin;

	for ( i = 0; i < EdgeList.Num();  i++ ) {
		curEdge = EdgeList[i];
		surf = renderEntity.hModel->Surface( curEdge->surf );
		v = &surf->geometry->verts[ curEdge->vert[0] ];
		v2 = &surf->geometry->verts[ curEdge->vert[1] ];
		if (!curEdge->Internal) {
			if (curEdge->CrossSurf) {
				gameRenderWorld->DebugLine( idVec4(1, 1, 1, 1), v2->xyz + org, v->xyz + org, 0, false );
			}else{
				if (!curEdge->Collapsed) {
					gameRenderWorld->DebugLine( idVec4(0, 1, 1, 1), v2->xyz + org, v->xyz + org, 0, false );
				}else{
					gameRenderWorld->DebugLine( idVec4(1, 1, 0, 1), v2->xyz + org, v->xyz + org, 0, false );
				}
			}
		}else{
			gameRenderWorld->DebugLine( idVec4(1, 0, 0, 1), v2->xyz + org, v->xyz + org, 0, false );
		}
	}
}

/*
===============
idEnergetical::makeEdges
===============
*/

void idEnergetical::makeEdges( const idRenderModel *renderModel ) {
	int i, j, k, l;
	int idx1, idx2;
	const modelSurface_t *surf;
	const idDrawVert *v;
	const idDrawVert *v2;
	const idDrawVert *v3;
	const idDrawVert *v4;
	bool  collapse;
	bool  kill;
	idxedge_t* curEdge;
	idxedge_t* curEdge2;

	if ( !renderModel ) {
		return;
	}

	// Extract edges from a model
	for ( i = 0; i < renderModel->NumSurfaces(); i++ ) {
		surf = renderModel->Surface( i );

		for ( j = 0; j < surf->geometry->numIndexes; j += 3 ) {
			idx1 = surf->geometry->indexes[ j ];
			for ( k = 0; k < 3; k++ ) {
				idx2 = surf->geometry->indexes[ j + 2 - k ];
				v = &surf->geometry->verts[ idx1 ];
				v2 = &surf->geometry->verts[ idx2 ];
				collapse = false;
				kill = false;
				for ( l = 0; l < EdgeList.Num(); l++ )
				{
					curEdge = EdgeList[l];
					if (curEdge->surf == i)	{
						v3 = &surf->geometry->verts[ curEdge->vert[0] ];
						v4 = &surf->geometry->verts[ curEdge->vert[1] ];
						if ((curEdge->vert[0] == idx1 && curEdge->vert[1] == idx2) || (curEdge->vert[0] == idx2 && curEdge->vert[1] == idx1)) {
							curEdge->Internal = true;
							kill = true;
						}
						if ((v->xyz.Compare(v3->xyz,0.1f) && v2->xyz.Compare(v4->xyz,0.1f)) || (v->xyz.Compare(v4->xyz,0.1f) && v2->xyz.Compare(v3->xyz,0.1f))) {
							curEdge->Collapsed = true;
							collapse = true;
						}
					}
				}
				if (!kill) {
					curEdge = new idxedge_t;
					curEdge->Internal = false;
					curEdge->Collapsed = collapse;
					curEdge->CrossSurf = false;
					curEdge->vert[0] = idx1;
					curEdge->vert[1] = idx2;
					curEdge->surf = i;
					EdgeList.Append(curEdge);
				}
				idx1 = idx2;
			}
		}
	}
	for ( i = 0; i < EdgeList.Num(); i++ ) {
		curEdge = EdgeList[i];
		v = &renderModel->Surface( curEdge->surf )->geometry->verts[ curEdge->vert[0] ];
		v2 = &renderModel->Surface( curEdge->surf )->geometry->verts[ curEdge->vert[1] ];
		for ( j = i+1; j < EdgeList.Num(); j++ ) {
			curEdge2 = EdgeList[j];
			if (curEdge2->surf != curEdge->surf) {
				v3 = &renderModel->Surface( curEdge2->surf )->geometry->verts[ curEdge2->vert[0] ];
				v4 = &renderModel->Surface( curEdge2->surf )->geometry->verts[ curEdge2->vert[1] ];
				if ((v->xyz.Compare(v3->xyz,0.1f) && v2->xyz.Compare(v4->xyz,0.1f)) || (v->xyz.Compare(v4->xyz,0.1f) && v2->xyz.Compare(v3->xyz,0.1f))) {
					curEdge->CrossSurf = !curEdge->Collapsed;
					curEdge2->CrossSurf = !curEdge2->Collapsed;
				}
			}
		}
	}
}

/*
===============
idEnergetical::Event_Activate
===============
*/

void idEnergetical::Event_Activate( idEntity *activator ) {

	On = !On;
	return;
}

