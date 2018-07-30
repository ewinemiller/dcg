/*  Project Gemini - plug-in for Carrara
    Copyright (C) 2002 Eric Winemiller

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
#include "GeminiDeformer.h"
#include "Eraser.h"
#include "GeminiCommand.h"
#include "copyright.h"
#include "dcgdebug.h"

void Extension3DInit(IMCUnknown* utilities)  
{
} 
     
void Extension3DCleanup()  
{
	// Perform any nec clean-up here
}
  
TBasicUnknown* MakeCOMObject(const MCCLSID& classId)	  
{								
	TBasicUnknown* res = NULL;
	if (classId == CLSID_GeminiDeformer) 
	{
		res = new GeminiDeformer; 
	}

	if (classId == CLSID_Eraser) 
	{
		res = new Eraser; 
	}

	if (classId == CLSID_GeminiCommand) 
	{
		res = new GeminiCommand; 
	}

	return res;
}
