/*  Carrara plug-in utilities
    Copyright (C) 2000 Eric Winemiller

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    
    http://digitalcarversguild.com/
    email: ewinemiller @ digitalcarversguild.com (remove space)
*/
#include "cTransformer.h"

cTransformer::cTransformer() {
	}

cTransformer::~cTransformer() {
	}

// Tool to get the Global Coordinates from the Local Coordinates 
/*void cTransformer::LocalToGlobal(TTransform3D* transform,TVector3* LocalPos,TVector3* GlobalPos)
{
	// Axes rotations
	(*GlobalPos)[0] = transform->fRotationAndScale[0][0]*(*LocalPos)[0]
					+transform->fRotationAndScale[0][1]*(*LocalPos)[1]
					+transform->fRotationAndScale[0][2]*(*LocalPos)[2];

	(*GlobalPos)[1] = transform->fRotationAndScale[1][0]*(*LocalPos)[0]
					+transform->fRotationAndScale[1][1]*(*LocalPos)[1]
					+transform->fRotationAndScale[1][2]*(*LocalPos)[2];

	(*GlobalPos)[2] = transform->fRotationAndScale[2][0]*(*LocalPos)[0]
					+transform->fRotationAndScale[2][1]*(*LocalPos)[1]
					+transform->fRotationAndScale[2][2]*(*LocalPos)[2];

	// Unit conversion : Points Units to 3D Units 
	(*GlobalPos)[0] *= ONE_OVER_288;  
	(*GlobalPos)[1] *= ONE_OVER_288;  
	(*GlobalPos)[2] *= ONE_OVER_288;
	
	// Origin translation
	(*GlobalPos)[0] += transform->fTranslation[0];
	(*GlobalPos)[1] += transform->fTranslation[1];
	(*GlobalPos)[2] += transform->fTranslation[2];
}
*/
void cTransformer::LocalToGlobal(const TTransform3D& transform,const TVector3& LocalPos,TVector3& GlobalPos)
{
	//stuff in locals so that same item can be passed for local and global
	TVector3 working = LocalPos;

	// Axes rotations
	GlobalPos.x = transform.fRotationAndScale[0][0]*working.x
					+transform.fRotationAndScale[0][1]*working.y
					+transform.fRotationAndScale[0][2]*working.z;

	GlobalPos.y = transform.fRotationAndScale[1][0]*working.x
					+transform.fRotationAndScale[1][1]*working.y
					+transform.fRotationAndScale[1][2]*working.z;

	GlobalPos.z = transform.fRotationAndScale[2][0]*working.x
					+transform.fRotationAndScale[2][1]*working.y
					+transform.fRotationAndScale[2][2]*working.z;

	// Origin translation
	GlobalPos += transform.fTranslation;
}
/*void cTransformer::LocalToGlobalNoScale2(TTransform3D* transform,TVector3* LocalPos,TVector3* GlobalPos)
{
	double x, y, z;

	// Origin translation
	x = (*LocalPos)[0] - transform->fTranslation[0];
	y = (*LocalPos)[1] - transform->fTranslation[1];
	z = (*LocalPos)[2] - transform->fTranslation[2];

	// Axes rotations
	(*GlobalPos)[0] = transform->fRotationAndScale[0][0]*x
					+transform->fRotationAndScale[0][1]*y
					+transform->fRotationAndScale[0][2]*z;

	(*GlobalPos)[1] = transform->fRotationAndScale[1][0]*x
					+transform->fRotationAndScale[1][1]*y
					+transform->fRotationAndScale[1][2]*z;

	(*GlobalPos)[2] = transform->fRotationAndScale[2][0]*x
					+transform->fRotationAndScale[2][1]*y
					+transform->fRotationAndScale[2][2]*z;

}
*/
// Tool to get the Global Coordinates from the Local Coordinates for a Vector
void cTransformer::LocalToGlobalVector(const TTransform3D& transform,const TVector3& LocalPos,TVector3& GlobalPos)
{
	//stuff in locals so that same item can be passed for local and global
	TVector3 working = LocalPos;

	// Axes rotations
	GlobalPos.x = transform.fRotationAndScale[0][0]*working.x
					+transform.fRotationAndScale[0][1]*working.y
					+transform.fRotationAndScale[0][2]*working.z;

	GlobalPos.y = transform.fRotationAndScale[1][0]*working.x
					+transform.fRotationAndScale[1][1]*working.y
					+transform.fRotationAndScale[1][2]*working.z;

	GlobalPos.z = transform.fRotationAndScale[2][0]*working.x
					+transform.fRotationAndScale[2][1]*working.y
					+transform.fRotationAndScale[2][2]*working.z;
}

// Tool to get the Local Coordinates from the Global Coordinates             
/*
void cTransformer::GlobalToLocal(TTransform3D* transform,TVector3* GlobalPos,TVector3* LocalPos)
{
	double x, y, z;

	x = (*GlobalPos)[0];
	y = (*GlobalPos)[1];
	z = (*GlobalPos)[2];

	  // Origin translation
	x -= transform->fTranslation[0];
	y -= transform->fTranslation[1];
	z -= transform->fTranslation[2];

   // Unit conversion : 3D Units to Points Units
	x *= 288.0;
	y *= 288.0;
	z *= 288.0;
 
  // Axes rotations
	(*LocalPos)[0] = transform->fRotationAndScale[0][0]*x
					+transform->fRotationAndScale[1][0]*y
					+transform->fRotationAndScale[2][0]*z;

	(*LocalPos)[1] = transform->fRotationAndScale[0][1]*x
					+transform->fRotationAndScale[1][1]*y
					+transform->fRotationAndScale[2][1]*z;

	(*LocalPos)[2] = transform->fRotationAndScale[0][2]*x
					+transform->fRotationAndScale[1][2]*y
					+transform->fRotationAndScale[2][2]*z;
}
*/
void cTransformer::GlobalToLocal(const TTransform3D& transform,const TVector3& GlobalPos, TVector3& LocalPos)
{
	//stuff in locals so that same item can be passed for local and global
	TVector3 working = GlobalPos;

	// Origin translation
	working -= transform.fTranslation;

	// Axes rotations
	LocalPos.x = transform.fRotationAndScale[0][0]*working.x
					+transform.fRotationAndScale[1][0]*working.y
					+transform.fRotationAndScale[2][0]*working.z;

	LocalPos.y = transform.fRotationAndScale[0][1]*working.x
					+transform.fRotationAndScale[1][1]*working.y
					+transform.fRotationAndScale[2][1]*working.z;

	LocalPos.z = transform.fRotationAndScale[0][2]*working.x
					+transform.fRotationAndScale[1][2]*working.y
					+transform.fRotationAndScale[2][2]*working.z;
}

void cTransformer::GlobalToLocalInv(const TTransform3D& transform,const TVector3& GlobalPos, TVector3& LocalPos)
{
	//stuff in locals so that same item can be passed for local and global
	TVector3 working = GlobalPos;

	// Origin translation
	working -= transform.fTranslation;

	// Axes rotations
	LocalPos.x = transform.fRotationAndScale[0][0]*working.x
					+transform.fRotationAndScale[0][1]*working.y
					+transform.fRotationAndScale[0][2]*working.z;

	LocalPos.y = transform.fRotationAndScale[1][0]*working.x
					+transform.fRotationAndScale[1][1]*working.y
					+transform.fRotationAndScale[1][2]*working.z;

	LocalPos.z = transform.fRotationAndScale[2][0]*working.x
					+transform.fRotationAndScale[2][1]*working.y
					+transform.fRotationAndScale[2][2]*working.z;
}
// Tool to get the Local Coordinates from the Global Coordinates for a Vector
void cTransformer::GlobalToLocalVector(const TTransform3D& transform, const TVector3& GlobalPos,TVector3& LocalPos)
{
	//stuff in locals so that same item can be passed for local and global
	TVector3 working = GlobalPos;

	// Axes rotations
	LocalPos.x = transform.fRotationAndScale[0][0]*working.x
					+transform.fRotationAndScale[1][0]*working.y
					+transform.fRotationAndScale[2][0]*working.z;

	LocalPos.y = transform.fRotationAndScale[0][1]*working.x
					+transform.fRotationAndScale[1][1]*working.y
					+transform.fRotationAndScale[2][1]*working.z;

	LocalPos.z = transform.fRotationAndScale[0][2]*working.x
					+transform.fRotationAndScale[1][2]*working.y
					+transform.fRotationAndScale[2][2]*working.z;
}
