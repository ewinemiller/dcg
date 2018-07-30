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
#include "Transforms.h"

#ifndef __CTRANSFORMER__
#define __CTRANSFORMER__

#if CP_PRAGMA_ONCE
#pragma once
#endif

//#define ONE_OVER_288 0.0034722222222222222222222222222222 

//const static real32 ONE_OVER_288 = 1 / 288;

class cTransformer{
	public:
		cTransformer();
		~cTransformer();
//		void LocalToGlobal(TTransform3D* transform,TVector3* LocalPos,TVector3* GlobalPos);
		void LocalToGlobal(const TTransform3D& transform,const TVector3& LocalPos,TVector3& GlobalPos);
		void LocalToGlobalVector(const TTransform3D& transform,const TVector3& LocalPos,TVector3& GlobalPos);
//		void GlobalToLocal(TTransform3D* transform, TVector3* GlobalPos,TVector3* LocalPos);
		void GlobalToLocal(const TTransform3D& transform, const TVector3& GlobalPos,TVector3& LocalPos);
		void GlobalToLocalInv(const TTransform3D& transform, const TVector3& GlobalPos,TVector3& LocalPos);
		void GlobalToLocalVector(const TTransform3D& transform, const TVector3& GlobalPos,TVector3& LocalPos);
	};

#endif