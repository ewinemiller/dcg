/*  Anything Grows - plug-in for Carrara
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
#include "AnythingGrowsDeformer.h"
#include "AnythingGrowsPrimitive.h"
#include "LockMasterShader.h"
#include "Billboard.h"
#include "Copyright.h"

void Extension3DInit(IMCUnknown* utilities)  
{
	// Perform your dll initialization here
} 
     
void Extension3DCleanup()  
{
	// Perform any nec clean-up here
}
  
TBasicUnknown* MakeCOMObject(const MCCLSID& classId)	  
{														 
	TBasicUnknown* res = NULL;
	if (classId == CLSID_AnythingGrowsDeformer) {
		res = new AnythingGrowsDeformer; 
		}
	if (classId == CLSID_AnythingGrowsPrimitive) {
		res = new AnythingGrowsPrimitive;
		}
	 
	if (classId == CLSID_LockMasterShader) {
		res = new LockMasterShader;
		}

	if (classId == CLSID_Billboard)
	{
		res = new Billboard;
	}

	return res;
}
