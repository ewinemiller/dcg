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

#include "ToonOverrideCache.h"
#include "ComUtilities.h"
#include "I3DShScene.h"
#include "I3DShInstance.h"
#include "I3dShRenderFeature.h"
#include "MCCountedPtrHelper.h"

cOverrideCache::cOverrideCache(){

	}

cOverrideCache::~cOverrideCache() {

	}

boolean cOverrideCache::Init(I3DShScene* scene, WireRendererPublicData fData, uint32 plMaxElements) {
	TMCCountedPtr<ToonEnabled> ToonEnabledGenData;
	lMaxElements = plMaxElements;
	cache.SetElemCount(plMaxElements);
	return true;

	}

ToonSettings cOverrideCache::ItemSettings(uint32 plIndex){
	return cache[plIndex];
	}


boolean cOverrideCache::CleanUp() {
	cache.SetElemCount(0);
	return true;
	}

MCCOMErr cOverrideCache::GetToonOverriden(const MCIID& riid, ToonEnabled** toonenabled, I3DShInstance *instance) 
{
	TMCCountedGetHelper<ToonEnabled> result(toonenabled);

	TMCCountedPtr<I3DShTreeElement> treeitem;
	TMCCountedPtr<I3DShTreeElement> nextitem;
	TMCCountedPtr<I3DShDataComponent> dataitem;
	TMCCountedPtr<ToonEnabled> ToonEnabledGenData;
	long i;
	long lDataComponents;
	
	if (instance->QueryInterface(IID_I3DShTreeElement, (void**)&treeitem)==MC_S_OK)
	{
		do
		{
			//check this item
			lDataComponents = treeitem -> GetDataComponentsCount();
			for (i = 0;i < lDataComponents;i++){
				treeitem -> GetDataComponentByIndex(&dataitem, i);
				if (dataitem->QueryInterface(riid, (void**)&ToonEnabledGenData)==MC_S_OK) 
				{
					result = ToonEnabledGenData;
					return MC_S_OK;
				}
				dataitem = NULL;
			}
			//check the parent
			treeitem -> GetTop(&nextitem);
			treeitem = NULL;
			treeitem = nextitem;
			nextitem = NULL;

		} while (treeitem != NULL);
	}
	return MC_S_FALSE;
}

