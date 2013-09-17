#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

idDynamicBlockAlloc<byte, 1024, 256>	ctRelationship::relation_alloc;

bool ctRelationship::initialized = false;

void	ctRelationship::InitStorage() {
	gameLocal.Printf( "Initializing relations server\n" );
	if ( initialized ) {
		gameLocal.Printf( "...already initialized\n" );
		return;
	}
	relation_alloc.Init();
	// the event system has started
	initialized = true;
}

void	ctRelationship::FreeStorage() {
	gameLocal.Printf( "Shutdown relations server\n" );

	if ( !initialized ) {
		gameLocal.Printf( "...not started\n" );
		return;
	}
	relation_alloc.Shutdown();
	// say it is now shutdown
	initialized = false;
}

void	ctRelationship::Clear() {
int j;
relation_ptr_t *rel;
relation_ptr_t *next_rel;

	j =	RegisteredEntitys.Num();
	while ( j > 0 )	{
		j--;
		if ( RegisteredEntitys[j].head ) {
			rel = RegisteredEntitys[j].head;
			while ( rel != NULL ) {
				if ( rel->Self[0]->entptr == RegisteredEntitys[j].entptr ) {
					next_rel = rel->NextRelation[0];
					RemoveRel( rel );
				} else if ( rel->Self[1]->entptr == RegisteredEntitys[j].entptr ) {
					next_rel = rel->NextRelation[1];
					RemoveRel( rel );
				}
				rel = next_rel;
			}
		}
	}
	RegisteredEntitys.Clear();
}

void	ctRelationship::List() {
int j;
relation_ptr_t *rel;
relation_ptr_t *next_rel;
int side;
idEntity* me;
idEntity* you;

	j =	RegisteredEntitys.Num();
	while ( j > 0 )	{
		j--;
		if ( RegisteredEntitys[j].head ) {
			rel = RegisteredEntitys[j].head;
			while ( rel != NULL ) {
				side = RelSide( rel, RegisteredEntitys[j].entptr.GetSpawnId() );
				if ( side == -1 ) {
					gameLocal.Printf( "rel: >---list broken here----<\n" );
					break;
				} else {
					next_rel = rel->NextRelation[side];
					if ( rel->Self[side] == NULL ) {
						gameLocal.Printf( "rel: missing self\n" );
					} else if ( rel->Self[1-side] == NULL ) {
						gameLocal.Printf( "rel: missing other\n" );
					} else if ( rel->Self[side]->head != RegisteredEntitys[j].head ) {
						gameLocal.Printf( "rel: wrong head\n" );
					} else {
						me = rel->Self[side]->entptr.GetEntity();
						you = rel->Self[1-side]->entptr.GetEntity();
						if ( me == NULL ) {
							gameLocal.Printf( "rel: missing self entity\n" );
						} else if ( you == NULL ) {
							gameLocal.Printf( "rel: missing other entity\n" );
						} else {
							if ( rel->RelationType[side] == R_NONE ) gameLocal.Printf( "rel: %s have no relations with %s\n", me->GetName(), you->GetName() );
							if ( rel->RelationType[side] == R_ENEMY ) gameLocal.Printf( "rel: %s is enemy of %s\n", me->GetName(), you->GetName() );
							if ( rel->RelationType[side] == R_FRIEND ) gameLocal.Printf( "rel: %s is friend of %s\n", me->GetName(), you->GetName() );
						}
					}
				}
				rel = next_rel;
			}
		}
	}
}

void	ctRelationship::SetRelation( idEntity* source, idEntity* target, relation_e rel ) {

int j;
int side;
relation_ent_t *rel_src;
relation_ent_t *rel_dst;
relation_ptr_t *found_rel;
relation_ptr_t *found_rel2;

	if ( source == NULL || target == NULL || source == target ) return;	//Incorrect relation

	rel_src = NULL;
	rel_dst = NULL;

	j =	RegisteredEntitys.Num();
	while ( j > 0 )	{
		j--;
		if ( RegisteredEntitys[j].entptr == source ) rel_src = &RegisteredEntitys[j];
		if ( RegisteredEntitys[j].entptr == target ) rel_dst = &RegisteredEntitys[j];
	}	
	if ( rel_src == NULL ) {
		if ( rel == R_NONE ) return;			// Relation is non-existent
		rel_src = &RegisteredEntitys.Alloc();
		rel_src->entptr = source;
		rel_src->head = NULL;
		gameLocal.Printf( "rel: add left entity\n" );
	}
	if ( rel_dst == NULL ) {
		if ( rel == R_NONE ) return;			// Relation is non-existent
		rel_dst = &RegisteredEntitys.Alloc();
		rel_dst->entptr = target;
		rel_dst->head = NULL;
		gameLocal.Printf( "rel: add right entity\n" );
	}

	found_rel = FindRel( rel_src, target );
	found_rel2 = FindRel( rel_dst, source );
	
	if ( (found_rel != NULL) && (found_rel2 != NULL) ) {	//Bother
		if ( found_rel != found_rel2 ) {
			gameLocal.Printf( "rel: need merge\n" );
			RemoveRel( found_rel2 );
			InsertRel( rel_dst, found_rel );
		}
		side = RelSide( found_rel, source );
		if ( side != -1 ) {
			gameLocal.Printf( "rel: changed\n" );
			found_rel->RelationType[side] = rel;
		}
		if ( found_rel->RelationType[0] == R_NONE && found_rel->RelationType[1] == R_NONE ) {
			gameLocal.Printf( "rel: collapsed\n" );
			RemoveRel( found_rel );
		}
	} else if ( found_rel ) {	// Check second side links
		// Add to rel_dst
		side = RelSide( found_rel, source );
		if ( side != -1 ) {
			if ( found_rel->RelationType[1-side] == R_NONE && rel == R_NONE ) {
				gameLocal.Printf( "rel: collapsed\n" );
				RemoveRel( found_rel );
			} else {
				found_rel->RelationType[side] = rel;
				InsertRel( rel_dst, found_rel );
				gameLocal.Printf( "rel: insert to left\n" );
			}
		}
	} else if ( found_rel2 ) {
		// Add to rel_src
		side = RelSide( found_rel2, source );
		if ( side != -1 ) {
			if ( found_rel2->RelationType[1-side] == R_NONE && rel == R_NONE ) {
				gameLocal.Printf( "rel: collapsed\n" );
				RemoveRel( found_rel2 );
			} else {
				found_rel2->RelationType[side] = rel;
				InsertRel( rel_src, found_rel2 );
				gameLocal.Printf( "rel: insert to right\n" );
			}
		}
	} else if ( rel != R_NONE ) {
		// Add both
		gameLocal.Printf( "rel: insert both\n" );
		found_rel = AllocRel();
		InsertRel( rel_src, found_rel );
		InsertRel( rel_dst, found_rel );
		side = RelSide( found_rel, source );
		if ( side != -1 ) {
			found_rel->RelationType[side] = rel;
			found_rel->RelationType[1-side] = R_NONE;
		}
	}
}

void ctRelationship::ClearRelations( idEntity* target ) {

	int j;
	relation_ent_t *rel_dst;
	relation_ptr_t *rel;
	relation_ptr_t *next_rel;
	int side;

	if ( !target )
		return;

	// find entity in the entitylist
	j =	RegisteredEntitys.Num();
	rel_dst = NULL;
	while ( j > 0 )	{
		j--;
		if ( RegisteredEntitys[j].entptr == target ) {
			rel_dst = &RegisteredEntitys[j];
			break;
		}
	}	

	if (!rel_dst) return;
	
	rel = rel_dst->head;
	while ( rel != NULL ) {
		side = RelSide( rel, RegisteredEntitys[j].entptr.GetSpawnId() );
		if ( side == -1 ) break;
		next_rel = rel->NextRelation[side];
		RemoveRel( rel );
		rel = next_rel;
	}
	RegisteredEntitys.RemoveIndex(j);
}

int ctRelationship::RelSide ( relation_ptr_t *rel, int spawnid ) {

	if ( !rel )
		return -1;

	if ( rel->Self[0] && rel->Self[0]->entptr.GetSpawnId() == spawnid ) {
		return 0;
	} else if ( rel->Self[1] && rel->Self[1]->entptr.GetSpawnId() == spawnid ) {
		return 1;
	} else {
		return -1;
	}
}

int ctRelationship::RelSide ( relation_ptr_t *rel, idEntity* ent ) {

	if ( !rel )
		return -1;

	if ( rel->Self[0] && rel->Self[0]->entptr == ent ) {
		return 0;
	} else if ( rel->Self[1] && rel->Self[1]->entptr == ent ) {
		return 1;
	} else {
		return -1;
	}
}

bool ctRelationship::InsertRel ( relation_ent_t *rel_list, relation_ptr_t *item ) {

relation_ptr_t *old_head;

	if ( !rel_list )
		return NULL;

	old_head = rel_list->head;
	rel_list->head = item;

	if ( item->Self[0] == NULL || item->Self[0]->entptr == rel_list->entptr ) {
		item->Self[0] = rel_list;
		item->NextRelation[0] = old_head;
		return true;
	} else if ( item->Self[1] == NULL || item->Self[1]->entptr == rel_list->entptr ) {
		item->Self[1] = rel_list;
		item->NextRelation[1] = old_head;
		return true;
	} else {
		//No add
		rel_list->head = old_head;
		return false;
	}
}

relation_ptr_t *ctRelationship::AllocRel (  ) {
relation_ptr_t *rel;
	
	rel = (relation_ptr_t*)relation_alloc.Alloc( sizeof(relation_ptr_t) );
	rel->NextRelation[0] = NULL;
	rel->NextRelation[1] = NULL;
	rel->RelationType[0] = R_NONE;
	rel->RelationType[1] = R_NONE;
	rel->UserData[0]	 = NULL;
	rel->UserData[1]	 = NULL;
	rel->Self[0]		 = NULL;
	rel->Self[1]		 = NULL;
	return rel;

}

relation_ptr_t *ctRelationship::FindRel ( relation_ent_t *rel_list, idEntity *ent_ptr2 ) {

idEntity *ent_ptr;
relation_ptr_t *find_rel;
relation_ptr_t *find_rel_next;

	if ( !rel_list || !rel_list->head )
		return NULL;

	ent_ptr = rel_list->entptr.GetEntity();
	find_rel = rel_list->head;

	while ( find_rel != NULL ) {
		if ( find_rel->Self[0] && find_rel->Self[0]->entptr == ent_ptr ) {			// Might be half-removed
			find_rel_next = find_rel->NextRelation[0];
			if ( find_rel->Self[1] && find_rel->Self[1]->entptr == ent_ptr2 ) {
				return find_rel;
			}
		} else if ( find_rel->Self[1] && find_rel->Self[1]->entptr == ent_ptr ) {	// Might be half-removed
			find_rel_next = find_rel->NextRelation[1];
			if ( find_rel->Self[0] && find_rel->Self[0]->entptr == ent_ptr2 ) {
				return find_rel;
			}
		} else {
			return NULL;
		}
		find_rel = find_rel_next;
	}
	return NULL;
}

void ctRelationship::RemoveRel ( relation_ptr_t *item ) {
int i;
relation_ptr_t	*rem_rel;
relation_ptr_t	*next_rel;
relation_ptr_t	*prew_rel;
idEntity		*eptr;
int				side;
int				prew_side;

	if ( !item )
		return;

	for ( i = 0; i < 2; i++ ) {
		prew_rel = NULL;
		prew_side = -1;
		if ( item->Self[i] ) {
			// Remove from the left side	
			rem_rel = item->Self[i]->head;
			eptr	= item->Self[i]->entptr.GetEntity();
		
			while ( rem_rel != NULL ) {
				side = RelSide( rem_rel, eptr );
				if ( side == -1 ) break;

				next_rel = rem_rel->NextRelation[side];
				if ( rem_rel == item ) {
					// Remove item
					if ( prew_rel ) {
						if ( prew_side == -1 ) break;
						prew_rel->NextRelation[prew_side] = next_rel;
					} else {
						item->Self[i]->head = next_rel;
					}
					rem_rel->NextRelation[side] = NULL;
				}
				prew_rel = rem_rel;
				prew_side = side;
				rem_rel = next_rel;
			}
		}
	}

	if ( item->UserData[0] != NULL ) relation_alloc.Free( (byte*)item->UserData[0] );
	if ( item->UserData[1] != NULL ) relation_alloc.Free( (byte*)item->UserData[1] );
	relation_alloc.Free( (byte*)item );

}