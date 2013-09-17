// Copyright (C) 2004 Id Software, Inc.
//

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

#ifdef __DEBRISSYS_H__

static const char *GlobalDebris_SnapshotName = "_AllDebris_Snapshot_";

/*
================
GlobalDebris::GlobalDebris
================
*/
GlobalDebris::GlobalDebris( void ) {
}

/*
================
GlobalDebris::Init
================
*/
void GlobalDebris::Init( void ) {
}

/*
================
GlobalDebris::Shutdown
================
*/
void GlobalDebris::Shutdown( void ) {
}

/*
================
GlobalDebris::FreeSmokes
================
*/
void GlobalDebris::FreeSmokes( void ) {
}

/*
================
GlobalDebris::EmitSmoke

Called by game code to drop another particle into the list
Added properly support for multistage particles (c4tnt)
================
*/
bool GlobalDebris::EmitSmoke( const idDeclParticle *smoke, const int systemStartTime, const float diversity, const idVec3 &origin, const idMat3 &axis,const int lastActive ) {
	return true;
}


/*
================
GlobalDebris::UpdateRenderEntity
================
*/
bool GlobalDebris::UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView ) {
	return true;
}

/*
================
GlobalDebris::ModelCallback
================
*/
bool GlobalDebris::ModelCallback( renderEntity_s *renderEntity, const renderView_t *renderView ) {
	return true;
}
#endif