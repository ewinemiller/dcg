/*  Shader Gel and Strobe - plug-in for Carrara
    Copyright (C) 2003 Michael Clarke

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
#include "cognito.h"
#include "ShaderGel.h"
#include "Strobe.h"

// IID for CognitoExt, so I can find it...
 
#if (VERSIONNUMBER >= 0x050000)
const MCGUID IID_CognitoDataExt(R_IID_CognitoDataExt);
#else
const MCGUID IID_CognitoDataExt={R_IID_CognitoDataExt};
#endif

// MCX Methods

void Extension3DInit(IMCUnknown* utilities) {
	
}

void Extension3DCleanup() {
	
}


TBasicUnknown* MakeCOMObject(const MCCLSID& classId) {
	
	TBasicUnknown* res = NULL;

	if (classId == CLSID_ShaderGel) {
		
		res = new ShaderGel(); 
		
	}

	if (classId == CLSID_Strobe) {
		
		res = new Strobe(); 
		
	}

	if (classId == CLSID_MCStrobe) {
		
		res = new MCStrobe(); 
		
	}

	return res;
}

