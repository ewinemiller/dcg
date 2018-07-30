/*  Wirerender utilities for Carrara
    Copyright (C) 2004 Eric Winemiller

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
#ifndef __TOONSETTINGSCACHE__
#define __TOONSETTINGSCACHE__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ToonEnabled.h"
#include "I3DShTreeElement.h"
#include "WireRendererData.h"
#include "ToonSettings.h"

class cOverrideCache{
	public:
		cOverrideCache();
		~cOverrideCache();
		boolean Init(I3DShScene* scene, WireRendererPublicData fData, uint32 plMaxElements);
		boolean CleanUp();
		ToonSettings ItemSettings(uint32 plIndex);
		MCCOMErr MCCOMAPI  GetToonOverriden(const MCIID& riid, ToonEnabled** toonenabled, I3DShInstance *instance) ;
		TMCClassArray<ToonSettings> cache;

	private:
		uint32 lMaxElements;
	
	};

#endif