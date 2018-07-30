/*  DCG Importer - plug-in for Carrara
    Copyright (C) 2012 Eric Winemiller

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
#include "copyright.h"
#include "SequencedObjPrim.h"
#include "SequencedObjImp.h"
#include "comsafeutilities.h"
#include "IShUtilities.h"


void Extension3DInit(IMCUnknown* utilities)
{
}

void Extension3DCleanup()
{
}


TBasicUnknown* MakeCOMObject(const MCCLSID& classId)	// This method instanciate
{														// the object COM
	TBasicUnknown* res = NULL;
	
	if (classId == CLSID_SequencedObjPrim) 
	{
		res = new SequencedObjPrim; 
	}

	if (classId == CLSID_SequencedObjImp) 
	{
		res = new SequencedObjImp; 
	}

	return res;
}
