/*  Shoestring Shaders - plug-in for Carrara
    Copyright (C) 2003  Mark DesMarais

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

//#include "math.h"
#include "Coordinate_Shift.h"
#include "copyright.h"
#include "Shoestringshaders.h"

MCCOMErr Coordinate_Shift::ExtensionDataChanged()
{
	MCCOMErr result = MC_S_OK;

	if (Coordinate_ShiftData.fInputShader)
		result = Coordinate_ShiftData.fInputShader->QueryInterface(IID_I3DShShader, (void**)&fInputShader);
	
	Imp_Out = 0;

	if (fInputShader != NULL)
		Imp_Out = fInputShader->GetImplementedOutput();  

	sObject_name = Coordinate_ShiftData.Objectname;

	return result;
}
Coordinate_Shift::Coordinate_Shift()		// Initialize the public data
{
	Coordinate_ShiftData.Mode = kOffset_mode;

	Coordinate_ShiftData.Xoffset = 0;
	Coordinate_ShiftData.Yoffset = 0;
	Coordinate_ShiftData.Zoffset = 0;

	Coordinate_ShiftData.GlobalX = true;
	Coordinate_ShiftData.GlobalY = true;
	Coordinate_ShiftData.GlobalZ = true;

	Coordinate_ShiftData.LocalX = false;
	Coordinate_ShiftData.LocalY = false;
	Coordinate_ShiftData.LocalZ = false;
	
	Coordinate_ShiftData.fInputShader = NULL;
}
	
void* Coordinate_Shift::GetExtensionDataBuffer()
{
	return ((void*) &(Coordinate_ShiftData));
}

boolean Coordinate_Shift::IsEqualTo(I3DExShader* aShader)		// Compare two Coordinate_Shift shaders
{
	int32 equality = 1;	
	if (Coordinate_ShiftData.Xoffset != ((Coordinate_Shift*)aShader)->Coordinate_ShiftData.Xoffset ||
		Coordinate_ShiftData.Yoffset != ((Coordinate_Shift*)aShader)->Coordinate_ShiftData.Yoffset ||
		Coordinate_ShiftData.Zoffset != ((Coordinate_Shift*)aShader)->Coordinate_ShiftData.Zoffset ||
		Coordinate_ShiftData.GlobalX != ((Coordinate_Shift*)aShader)->Coordinate_ShiftData.GlobalX ||
		Coordinate_ShiftData.GlobalY != ((Coordinate_Shift*)aShader)->Coordinate_ShiftData.GlobalY ||
		Coordinate_ShiftData.GlobalZ != ((Coordinate_Shift*)aShader)->Coordinate_ShiftData.GlobalZ ||
		Coordinate_ShiftData.LocalX != ((Coordinate_Shift*)aShader)->Coordinate_ShiftData.LocalX ||
		Coordinate_ShiftData.LocalY != ((Coordinate_Shift*)aShader)->Coordinate_ShiftData.LocalY ||
		Coordinate_ShiftData.LocalZ != ((Coordinate_Shift*)aShader)->Coordinate_ShiftData.LocalZ ||
		Coordinate_ShiftData.Mode != ((Coordinate_Shift*)aShader)->Coordinate_ShiftData.Mode ||
		Coordinate_ShiftData.fInputShader != ((Coordinate_Shift*)aShader)->Coordinate_ShiftData.fInputShader )
		equality = 0;
	return equality;		  
}

MCCOMErr Coordinate_Shift::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fNeedsNormal = true;
	theFlags.fNeedsIsoUV = true;
	theFlags.fNeedsNormal = true;
	theFlags.fNeedsNormalDerivative = true;
	theFlags.fNeedsNormalLoc = true;
	theFlags.fNeedsNormalLocDerivative = true;
	theFlags.fNeedsPixelRatio = true;
	theFlags.fNeedsPoint = true;
	theFlags.fNeedsPointLoc = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput Coordinate_Shift::GetImplementedOutput()
{
	uint32 implemented = kUsesDoShade;
	return (EShaderOutput)implemented;
}
  
#if (VERSIONNUMBER >= 0x040000)
boolean Coordinate_Shift::WantsTransform()
{
	return 0;
}
#endif

MCCOMErr	Coordinate_Shift::DoShade(ShadingOut&  result, ShadingIn &  shadingIn)
{
	fullArea = false;

	// initialize colors

	//result.Clear();

	if (Coordinate_ShiftData.Mode == kObject_mode)  {

		if (MCVerify(shadingIn.fInstance)){
			if (shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree) == MC_S_OK){	
				tree->GetScene(&scene);	
				tree->GetTop(&objecttree);
			}
		}

		if (MCVerify(scene)){ 
			//now that you have a scene, use gettreeelementbyname to find your object (object name from text box)
			scene->GetTreeElementByName( &objecttree, sObject_name );
			if (MCVerify(objecttree)) {
				objecttree->GetGlobalTransform3D(sped); 	//get the tree's global position
				object_loc = sped.fTranslation;	
				offset_vector = object_loc;
			}
		}
	}
	else { // Offset mode 
		offset_vector.x = Coordinate_ShiftData.Xoffset;
		offset_vector.y = Coordinate_ShiftData.Yoffset;
		offset_vector.z = Coordinate_ShiftData.Zoffset;
	}


	if (Coordinate_ShiftData.GlobalX) shadingIn.fPoint.x = shadingIn.fPoint.x - offset_vector.x;
	if (Coordinate_ShiftData.GlobalY) shadingIn.fPoint.y = shadingIn.fPoint.y - offset_vector.y;
	if (Coordinate_ShiftData.GlobalZ) shadingIn.fPoint.z = shadingIn.fPoint.z - offset_vector.z;	
	if (Coordinate_ShiftData.LocalX) shadingIn.fPointLoc.x = shadingIn.fPointLoc.x - offset_vector.x;
	if (Coordinate_ShiftData.LocalY) shadingIn.fPointLoc.y = shadingIn.fPointLoc.y - offset_vector.y;
	if (Coordinate_ShiftData.LocalZ) shadingIn.fPointLoc.z = shadingIn.fPointLoc.z - offset_vector.z;

	if (!fInputShader)  // if the shader is null, just return black.
			return MC_S_OK;
	else {
		if (Imp_Out & kUsesDoShade)	{	// do shade
			fInputShader->DoShade(result, shadingIn); }
		else if (Imp_Out & kUsesGetColor){		// get color implemented
			fInputShader->GetColor(Color, fullArea, shadingIn);
			result.fColor = Color; }
		else if (Imp_Out & kUsesGetValue) { //  or get value implemented
			fInputShader->GetValue(luma, fullArea, shadingIn);
			result.fColor.red = luma;
			result.fColor.green = luma;
			result.fColor.blue = luma;
		}
	}




#if (VERSIONNUMBER == 0x010000)
return MC_S_OK;
#elif (VERSIONNUMBER == 0x020000)
return MC_S_OK;
#elif (VERSIONNUMBER >= 0x030000)
return 1.0f;
#endif
}