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

#include "Key.h"
#include "copyright.h"
#include "Shoestringshaders.h"

MCCOMErr Key::ExtensionDataChanged()
{
	MCCOMErr result = MC_S_OK;

	if (KeyData.fInputShader)
	result = KeyData.fInputShader->QueryInterface(IID_I3DShShader, (void**)&fInputShader);
	
	Imp_Out = 0;

	if (fInputShader != NULL)
		Imp_Out = fInputShader->GetImplementedOutput();  

		red_range.x = KeyData.Red_range.x/100.0 + KeyData.KeyColor.R;
		green_range.x = KeyData.Grn_range.x/100.0 + KeyData.KeyColor.G;
		blue_range.x = KeyData.Blu_range.x/100.0 + KeyData.KeyColor.B; 
		red_range.y = KeyData.Red_range.y/100.0 + KeyData.KeyColor.R;
		green_range.y = KeyData.Grn_range.y/100.0 + KeyData.KeyColor.G;
		blue_range.y = KeyData.Blu_range.y/100.0 + KeyData.KeyColor.B; 

	#if (VERSIONNUMBER < 0x040000)  // keep it in keymode if pre-alpha 
	KeyData.Mode = kKey_mode;
	#endif


	return result;
}
Key::Key()		// Initialize the public data
{
	KeyData.Red_range[0] = -1;
	KeyData.Grn_range[0] = -1;
	KeyData.Blu_range[0] = -1;

	KeyData.Red_range[1] = 1;
	KeyData.Grn_range[1] = 1;
	KeyData.Blu_range[1] = 1;

	KeyData.fInputShader = NULL;
	KeyData.Mode = kKey_mode;

	KeyData.KeyColor.R = 0;
	KeyData.KeyColor.G = 1;
	KeyData.KeyColor.B = 0;
}
	
void* Key::GetExtensionDataBuffer()
{
	return ((void*) &(KeyData));
}

boolean Key::IsEqualTo(I3DExShader* aShader)		// Compare two Key shaders
{
	int32 equality = 1;	
	if (KeyData.Red_range != ((Key*)aShader)->KeyData.Red_range ||
		KeyData.Grn_range != ((Key*)aShader)->KeyData.Grn_range ||
		KeyData.Blu_range != ((Key*)aShader)->KeyData.Blu_range ||
		KeyData.KeyColor != ((Key*)aShader)->KeyData.KeyColor ||
		KeyData.Mode != ((Key*)aShader)->KeyData.Mode ||
		KeyData.fInputShader != ((Key*)aShader)->KeyData.fInputShader 
		)
			equality = 0;

	return equality;
		  
}

MCCOMErr Key::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fConstantChannelsMask = kNoChannel;

	return MC_S_OK;
}

EShaderOutput Key::GetImplementedOutput()
{
	return  kUsesGetColor;	
}
  
#if (VERSIONNUMBER >= 0x040000)
boolean Key::WantsTransform()
{
	return 0;
}
#endif

#if (VERSIONNUMBER == 0x010000)
MCCOMErr Key::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Key::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Key::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x040000)
real Key::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	fullArea = false;

	// initialize colors
	result.R = 1;
	result.G = 0;
	result.B = 0;
	result.A = 1;

	hold = result; 

	if (!fInputShader)  // if the shader is null, just return black.
			return MC_S_OK;
	else
	{
		if (Imp_Out && kUsesGetColor)		// get color implemented
		{
			fInputShader->GetColor(Color, fullArea, shadingIn);
			if ((Color.R >= red_range.x) && (Color.R <= red_range.y) &&      // matches, replace with black
				(Color.G >= green_range.x) && (Color.G <= green_range.y)&&
				(Color.B >= blue_range.x) && (Color.B <= blue_range.y)){
					if (KeyData.Mode == kKey_mode) {
						result.R = 0;
						result.G = 0;
						result.B = 0;
						#if (VERSIONNUMBER >= 0x040000)
						result.A = Color.A;
						#endif
						}
					else {// alpha mode! clear the alpha to get transparent
						result = Color;
						result.A = 0;
					}
				}
			else {
					if (KeyData.Mode == kKey_mode) {
						result.R = 1;
						result.G = 1;
						result.B = 1;
						#if (VERSIONNUMBER >= 0x040000)
						result.A = Color.A;
						#endif
					}
					else {
						result = Color;
						result.A = 1;
					}
			}
		}
		else
			return MC_S_OK;
	}



#if (VERSIONNUMBER < 0x030000)
return MC_S_OK;
#else 
return 1.0f;
#endif
}