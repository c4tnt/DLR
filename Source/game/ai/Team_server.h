#ifndef __AI_TS_H__
#define __AI_TS_H__

typedef enum {
	// must update these in script/doom_defs.script if changed
	INVISIBLE_V = 0,		
	VISIBLE_V = 1,
	LASTKNOWN_V = 2,
	LOST_V = 3
} enemyVision_e;

typedef enum {
	ENT_NULL = 0,
	ENT_ENTITY = 1,
	ENT_AI = 2,
	ENT_PLAYER = 3
} entityType_e;

typedef enum {
	R_NONE = 0,			// Not set
	R_ENEMY = 1,		// Enemy, try to attack his
	R_FRIEND = 2,		// Friendly, share enemys with
	R_NEUTRAL = 3,		// Ignore while not attacking
	R_MAXRELS = 3			
} relation_e;

struct relation_ptr_s;

typedef struct relation_ent_s {
	idEntityPtr<idEntity>		entptr;				// Entity
	relation_ptr_s*				head;				// Relation list head
} relation_ent_t;

// Relationships internal data
typedef struct relation_data_s {

	idVec3				lastVisiblePos;				// Last visible position
	idVec3				lastVisibleEyeOffset;		// Last visible eye heigth 
	idVec3				lastVisibleReachablePos;	// Last visible and reachable
	idVec3				lastReachablePos;			// Last reachable

} relation_data_t;

typedef struct relation_ptr_s {
	relation_ent_s*		Self[2];			// Self entity index
	relation_e			RelationType[2];	// Relation type, used only SAT and SET
	relation_ptr_s*		NextRelation[2];	// Next relation 
	relation_data_t*	UserData[2];		// User data ptr
} relation_ptr_t;

class ctRelationship {
public:
	
	void	Clear();											// Clear all
	void	SetRelation( idEntity* source, idEntity* target, relation_e rel );
	void	ClearRelations( idEntity* target );					// Removes all private entity relations
	bool	AddEntity( idEntity* target );						// Add entity with a clear relationship
	relation_e	GetRelation( idEntity* source, idEntity* target );
	relation_data_t*	GetRelationData( idEntity* source, idEntity* target );	//Yeah, you can modify this datablock througth ptr
	
	bool	isEnemy( idEntity* source, idEntity* target );
	bool	isAlly( idEntity* source, idEntity* target );
	bool	isNeutral( idEntity* source, idEntity* target );
	bool	isTwosided( idEntity* source, idEntity* target );	
	bool	isConflicted( idEntity* source, idEntity* target );	// Enemy/Ally conflict, AI can resolve this
	
	void	FreeEntity( idEntity* ent );
	void	List();

	static void	ctRelationship::InitStorage();
	static void	ctRelationship::FreeStorage();

private:
	
	relation_ptr_t *FindRel ( relation_ent_t *rel_list, idEntity *ent_ptr2 );
	relation_ptr_t *AllocRel ( );
	bool InsertRel ( relation_ent_t *rel_list, relation_ptr_t *item );
	void RemoveRel ( relation_ptr_t *item );
	int RelSide ( relation_ptr_t *rel, int spawnid );
	int RelSide ( relation_ptr_t *rel, idEntity* ent );
	relation_e DefaultRelation ( idEntity* source, idEntity* target );

	idList< relation_ent_t > RegisteredEntitys;	// Entitys
	static idDynamicBlockAlloc<byte, 1024, 256> relation_alloc;
	static bool initialized;
};

#endif