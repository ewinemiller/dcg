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
#ifndef __DCGSharedCacheClient__
#define __DCGSharedCacheClient__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCClassArray.h"
#include "IShSMP.h"

#ifdef _DEBUG 
//#include "Windows.h"
//#include <stdio.h>
#endif

template<class T, class S, class K> class DCGSharedCache;

template<class T, class S, class K> class DCGSharedCacheClient {


private:
	DCGSharedCache<T, S, K>& globalCache;


public:
	DCGSharedCacheClient<T, S, K>(DCGSharedCache<T, S, K>& globalCache) : globalCache(globalCache) {
		cacheElement = NULL;
	};

	~DCGSharedCacheClient<T, S, K>(void){
	};

	virtual void fillElement(T& newElement, const S& dataSource) = 0;
	virtual void emptyElement(T& oldElement) = 0;

protected:
	K globalStorageKey;
	T* cacheElement;

	void releaseCache() {
		if (cacheElement != NULL) {
			globalCache.stopUsing(this);
			cacheElement = NULL;
		}
	}

	void getCache(const S& dataSource) {
		cacheElement = globalCache.getCacheItem(globalStorageKey, this, dataSource);

	}

};

template<class T, class S, class K> class DCGSharedCache {

TMCCriticalSection* myCS;

public:
	DCGSharedCache<T, S, K>(void) {	
	};


	~DCGSharedCache<T, S, K>(void){
	};

private:
	struct TypePlus
	{
		T* element;
		K key;
		boolean active;
		TMCArray<DCGSharedCacheClient<T,S, K>*> usedBy;

		TypePlus(void) {
			element = NULL;
		};

	};
	TMCClassArray<TypePlus> elements;
	
	T* fillCachedItem(const K& key, DCGSharedCacheClient<T, S, K>* usedBy, const S& dataSource, TypePlus& element) {
		element.element = new T();
		usedBy->fillElement(*element.element, dataSource);
		element.key = key;
		element.usedBy.AddElem(usedBy);
		element.active = true;
		return element.element;
	}

public:
	void init() {
		myCS= NewCS();
	};

	void cleanup() {
		elements.SetElemCount(0);
		DeleteCS(myCS);
	};

	void stopUsing(DCGSharedCacheClient<T, S, K>* usedBy) {
		CWhileInCS cs(myCS);
		uint32 elementCount = elements.GetElemCount();
		if (elementCount > 0) {
			boolean hasActive = false;
			for (int32 elementIndex = elementCount - 1; elementIndex >= 0; elementIndex--) {
				TypePlus& element = elements[elementIndex];
				uint32 usedByCount = element.usedBy.GetElemCount();
				uint32 newUsedByCount = 0;

				for (uint32 usedByIndex = 0; usedByIndex < usedByCount; usedByIndex++) {
					DCGSharedCacheClient<T, S, K>* &currentUsedBy = element.usedBy[usedByIndex];
					if (currentUsedBy == usedBy){
#ifdef _DEBUG
//						char temp[80];
//						sprintf_s(temp, 80, "releasing  0x%x used by 0x%x on thread 0x%x\n\0", &element.element, usedBy, GetCurrentThreadId());
//						OutputDebugStringA(temp);
#endif
						currentUsedBy = NULL;
					}
					else if (currentUsedBy != NULL) {
						newUsedByCount++;
					}
				}
				if (newUsedByCount == 0) {
					if  (element.active == static_cast<boolean>(true)) {
#ifdef _DEBUG
//						char temp[80];
//						sprintf_s(temp, 80, "destroying 0x%x on thread 0x%x\n\0", &element.element, GetCurrentThreadId());
//						OutputDebugStringA(temp);
#endif
						//nobody is using this anymore, destroy it
						element.active = false;
						element.usedBy.SetElemCount(0);
						usedBy->emptyElement(*element.element);
						delete element.element;
						element.element = NULL;
					}
				}
				else {
					hasActive = true;
				}
			}
			if (!hasActive) {
				//if nothing is active, let's clear out cached elements completely
				elements.SetElemCount(0);
			}
		}

	};

	T* getCacheItem(const K& key, DCGSharedCacheClient<T, S, K>* usedBy, const S& dataSource) {
		//look outside a critical section to see if we find it quickly 
		//with no contention
		uint32 elementCount = elements.GetElemCount();
		for (uint32 elementIndex = 0; elementIndex < elementCount; elementIndex++) {
			TypePlus& element = elements[elementIndex];
			if (element.active && element.key == key) {
				uint32 usedByCount = element.usedBy.GetElemCount();
				for (uint32 usedByIndex = 0; usedByIndex < usedByCount; usedByIndex++) {
					if (element.usedBy[usedByIndex] == usedBy){
						//found and this object already cares about it
						return element.element;
					}
				}
				//found, but I'm not on the list that cares
				//so exit out and drop into our critical section
				elementIndex = elementCount;
			}
		}
		//not found, start critical section, search again
		//add if we don't find it
		CWhileInCS cs(myCS);
		elementCount = elements.GetElemCount();
		uint32 emptySlot = elementCount;
		for (uint32 elementIndex = 0; elementIndex < elementCount; elementIndex++) {
			TypePlus& element = elements[elementIndex];
			if (element.active && element.key == key) {
				uint32 usedByCount = element.usedBy.GetElemCount();
				for (uint32 usedByIndex = 0; usedByIndex < usedByCount; usedByIndex++) {
					//found and this object already cares about it
					if (element.usedBy[usedByIndex] == usedBy){
						return element.element;
					}
				} 
				//found, but add me to the list of folks who want to 
				//retain this
#ifdef _DEBUG
//				char temp[80];
//				sprintf_s(temp, 80, "found      0x%x used by 0x%x on thread 0x%x\n\0", &element.element, usedBy, GetCurrentThreadId());
//				OutputDebugStringA(temp);
#endif
				element.usedBy.AddElem(usedBy);
				return element.element;
			}
			else if (!element.active) {
				emptySlot = elementIndex;
			}
		}
		//we didn't find anything, use an empty element or add an element
		//and fill it by calling back into usedBy
		if (emptySlot == elementCount) {
			elements.AddElemCount(1);
		}
#ifdef _DEBUG
//		char temp[80];
//		sprintf_s(temp, 80, "creating   0x%x used by 0x%x on thread 0x%x\n\0", &elements[emptySlot].element, usedBy, GetCurrentThreadId());
//		OutputDebugStringA(temp);
#endif
		return fillCachedItem(key, usedBy, dataSource, elements[emptySlot]);
	};


};

#endif