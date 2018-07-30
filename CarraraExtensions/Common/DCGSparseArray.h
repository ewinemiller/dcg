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
#include "MCBasicTypes.h"
#include "MCClassArray.h"

template<class T> class DCGSparseArray {
public:
	DCGSparseArray<T>(void) {	
		fElemCount = 0;
		hashTableSize = 0;
		sparsity = 500;
		createOnAccess = false;
		BeginIter();};

		~DCGSparseArray<T>(void){};

	inline T& operator[](uint32 index){ 
		MCAssert(index < GetElemCount()); 
		TypeList& list = hashElements[GetHash(index)];
		uint32 listCount = list.listElements.GetElemCount();
		for (uint32 listIndex = 0; listIndex < listCount; listIndex++)
		{
			TypePlusIndex& typePlusIndex = list.listElements[listIndex];
			if (typePlusIndex.index == index)
			{
				return typePlusIndex.element;
			}
		}
		if (createOnAccess)
		{
			TypePlusIndex& retval = list.listElements.AddElem();
			retval.index = index;
			return retval.element;
		}
		else
		{
			return emptyObject;
		}
	};

	inline boolean SetElemCount(uint32 elemCount) {
		if (GetElemCount() > 0){
			ArrayFree();
			BeginIter();
		}
		fElemCount = elemCount;
		BuildHashTable();
		return true;};
	
	inline void SetSparsity(uint32 sparsity) {
		this->sparsity = sparsity;
	}
	
	inline uint32 GetSparsity() {
		return sparsity;
	}

	inline uint32 GetElemCount() const {
		return fElemCount;};

	void GetElem(uint32 index, T& data) {
		data = (*this)[index];
	}

	inline void SetCreateOnAccess(boolean createOnAccess) {
		this->createOnAccess = createOnAccess;
	}

	inline boolean GetCreateOnAccess() const {
		return createOnAccess;
	}

	inline void SetEmptyObject(const T&  data) {
		emptyObject = data;
	}

	inline T& GetEmptyObject() const {
		return emptyObject;
	}

	inline void ArrayFree() {
		fElemCount = 0;
		BuildHashTable();
	}

	inline void BeginIter() {
		iterHashIndex = 0;
		iterListIndex = 0;
	}

	inline T* Next(void) {
		while (iterHashIndex < hashTableSize)
		{
			TypeList& list = hashElements[iterHashIndex];
			uint32 listCount = list.listElements.GetElemCount();
			if (iterListIndex < listCount)
			{
				TypePlusIndex& typePlusIndex = list.listElements[iterListIndex];
				iterListIndex++;
				return &typePlusIndex.element;
			}
			iterHashIndex++;
			iterListIndex = 0;
		}
		return NULL;
	}

	inline real32 GetAverageChainLength(void) {
		if (hashTableSize == 0)
			return 0;

		uint32 elementCount = 0;

		for (uint32 i = 0; i < hashTableSize; i++) {
			TypeList& list = hashElements[i];
			elementCount += list.listElements.GetElemCount();
		}
		return (real32)elementCount/(real32)hashTableSize;
	}

protected:
	T emptyObject;
	uint32 iterHashIndex;
	uint32 iterListIndex;

	struct TypePlusIndex
	{
		T element;
		int32 index;
	};
	struct TypeList
	{
		TMCClassArray<TypePlusIndex> listElements;
	};
	TMCClassArray<TypeList> hashElements;

	uint32 hashTableSize;
	uint32 fElemCount;	
	uint32 sparsity;
	boolean createOnAccess;
	inline uint32 GetHash(uint32 n) {return n % hashTableSize;}
	inline void BuildHashTable() {
		hashTableSize = fElemCount / sparsity;
		hashElements.SetElemCount(hashTableSize);
	}
};