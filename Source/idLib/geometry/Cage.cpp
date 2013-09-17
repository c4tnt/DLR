// Copyright (C) 2004 Id Software, Inc.
//

#include "../precompiled.h"
#pragma hdrstop


/*
=============
kContour::
=============
*/

kContour::kContour(){
	EdgesHead = NULL;
	CurrentStatus = KCS_EMPTY ; //No flags
}

kContour::~kContour(){
}

int kContour::Combine (const kContour &contour){
	return 0;
}

void kContour::Align  (idVec2 PointInside){
}

void kContour::Normalize() {
}

void kContour::MakeLoop() {
}

kFlatEdge_t* kContour::Cutout(int ordFrom, int ordTo){
	return NULL;
}

int kContour::GetOrdinal(kFlatEdge_t*){
	return 0;
}
	
kFlatEdge_t* kContour::operator[]( const int index ) const {
	return NULL;
}

kFlatEdge_t* kContour::operator[]( const int index ) {
	return NULL;
}

contourStatus_t kContour::GetContourStatus() {
	return KCS_EMPTY;
}
contourStatus_t kContour::CheckContourStatus() {
	return KCS_EMPTY;
}

/*
=============
idSurfCage::AddSurface
=============
*/
idSurfCage::idSurfCage() {

}

idSurfCage::~idSurfCage() {

}

void idSurfCage::AddSurface(const idSurface &aSurf) {

}

void idSurfCage::Clear( void ) {

}

void idSurfCage::Expand( const idVec3& center, const float len ) {

}

void idSurfCage::Contract( const idVec3& center, const float len ) {

}

void idSurfCage::Weld( const float eps ) {

}

void idSurfCage::Reach( const idVec3& center ) {

}

void idSurfCage::Complete( void ) {

}

int	 idSurfCage::CarveRegion( const idSurfCage& another, const idVec3& center) {
	return 0;
}

void idSurfCage::MakeSingle( idSurface *aSurf ) {

}

void idSurfCage::MakeFins( idSurface *aSurf ) {

}

