//

#ifndef __SURFCAGE_H__
#define __SURFCAGE_H__

#define	CAGE_MAXCONTOURS		16

class kContour;

typedef enum {
	KCS_EMPTY						= 0,
	KCS_LOOPED						= BIT(0),	// E(dx) = 0; E(dy) = 0;
	KCS_NOSELFSECTED				= BIT(1),	// Haven't internal intersections
	KCS_PLUSTOINF					= BIT(2),	// Positive internal normals orientation: (V + eN) is outside 
} contourStatus_t;

typedef enum {
	KESS_NONE						= BIT(0),	// no edge data
	KESS_SINGLE						= BIT(1),	// edge has both sides on single surface
	KESS_FOURVERTS					= BIT(2),	// edge has four unshared vertex
	KESS_FLAG						= BIT(3),	// edge has a flag
	KESS_VIRT						= BIT(4)	// virtual edge without references on a model
} edgeSurfStatus_t;

typedef enum {
	KFS_NOSUBTRACT					= BIT(0),	// Prevent subtracting on this surface
	KFS_DISCRETE					= BIT(1),	// Surface won't carved
	KFS_TWOSIDED					= BIT(2),	// Generate backside
	KFS_GROUNDED					= BIT(3)	// Fixed invisible face, this face will break carving and always has a 'connected' status
} flatStatus_t;

typedef enum {
	KES_SMOOTH						= BIT(0),	// Normal smoothing over this edge
	KES_CUTOUT						= BIT(1)	// Cut througth edge
} edgeStatus_t;

typedef struct {
												//reference indexes
	kContour*	contours[CAGE_MAXCONTOURS];		//contour parts
	int			surf;							//surface link
	int			islandmark;						//surface connection marker (-1 - don't checked yet)
	flatStatus_t flags;							//Face flags
	
	idVec3		normal;							//Position
	float		offset;
} kFace_t;

typedef struct kFlatEdge_s {
	idVec2		 direction;		//dx and dy
	edgeStatus_t flags;			//Edge flags
	kFace_t*	 base;			//base surface
	kFlatEdge_s* flip;			//edge on another side of a surface. When NULL, this edge placed only on the basically surface
								//! edge->flip->flip == edge
} kFlatEdge_t;

/*
===============================================================================

	Cage elements:
		kContour (kFlatEdges group with positive circulation)
		kFace
		kGrid (grid base class)  -base class
		kHGrid (Heigthmap grid)  -Only Normal offset component
		kUVGrid (Grid over face) -DNDUDV offset system.
		kFGrid (Grid over face)  -Fourier offset system.

===============================================================================
*/
class kGrid {
public:
	kGrid();
	virtual ~kGrid();

};

class kContour {
public:
	kContour();
	~kContour();
	int Combine (const kContour &contour);
	void Align  (idVec2 PointInside);
	void Normalize ();		//Make loop, route intersections and align.
	void MakeLoop ();		//Attach the end of a contour to the start of a contour
	kFlatEdge_t* Cutout(int ordFrom, int ordTo); //Split out subchain
	int GetOrdinal(kFlatEdge_t*); //Find edge into a contour and return his ordinal
	
	kFlatEdge_t* operator[]( const int index ) const;
	kFlatEdge_t* operator[]( const int index );

	contourStatus_t GetContourStatus(); //Only return status flags, without any checking
	contourStatus_t CheckContourStatus(); //Test the contour to alignment, self-intersection and loops

private:
	contourStatus_t CurrentStatus;
	kFlatEdge_t* EdgesHead;
};
/*
===============================================================================

	Multisurface cage.

===============================================================================
*/

class idSurfCage {
public:
					idSurfCage();
					~idSurfCage();

//	float			operator[]( const int index ) const;
//	float &			operator[]( const int index );

	// *Load* //
	void			AddSurface(const idSurface &aSurf );
	void			Clear( void );

	// *Operations*//
	void			Expand(const idVec3 &center, const float len ); //Move all vertexes outward the center
	void			Contract(const idVec3 &center, const float len ); //Move all vertexes toward the center
	void			Weld( const float eps ); //Weld vertex
	void			Reach( const idVec3 &center ); //Remove all unreachable surfedges
	void			Complete( void ); // Cap holes
	int				CarveRegion( const idSurfCage &another, const idVec3 &center); //Build a Carve edges, returns index

	//*Extract*//
	void			MakeSingle( idSurface *aSurf ); //Makes a single surface
	void			MakeFins( idSurface *aSurf ); //Produce a endcaps surface
	void			MakeExpand( idSurface *aSurf ); //Produce a expanded surface
	void			MakeSection( idSurface *aSurf , kFace_t* cutter); //Produce a flat section (water surface for example)
	void			MakeGridSection( idSurface *aSurf , kGrid& grid); //Produce a flat section from grid

protected:

	// Primary model info. 
	idList<idDrawVert>		verts;			// vertices
	idList<int>				indexes;		// 3 references to vertices for each triangle
	idList<surfaceEdge_t>	edges;			// edges
	idList<int>				edgeIndexes;	// 3 references to edges for each triangle, may be negative for reversed edge

	// Variable info
//	idLinkList<kFace_t>		Faces;	// All contours of the object stored here

};

#endif /* !__SURFCAGE_H__ */
