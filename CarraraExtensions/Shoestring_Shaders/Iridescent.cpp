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
#include "Iridescent.h"
#include "copyright.h"
#include "Shoestringshaders.h"

MCCOMErr Iridescent::ExtensionDataChanged()
{
	// get the vector corresponding to the angles
	// start out pointing along X.
	ref_vec[0] = .5;
	ref_vec[1] = 0;
	ref_vec[2] = 0;

	//Y axis rotation
	cos_rot = cos (IridescentPublicData.pitch * DEG_RAD);
	sin_rot = sin (IridescentPublicData.pitch * DEG_RAD);

	rot_matrix[0][0] = cos_rot;
	rot_matrix[0][1] = 0;
	rot_matrix[0][2] = sin_rot;
	rot_matrix[1][0] = 0;
	rot_matrix[1][1] = 1;
	rot_matrix[1][2] = 0;
	rot_matrix[2][0] = -1 * sin_rot;
	rot_matrix[2][1] = 0;
	rot_matrix[2][2] = cos_rot;

	ref_vec = Matrix_mult(ref_vec, rot_matrix);

	// Z axis rotation- yaw
	cos_rot = cos (IridescentPublicData.yaw * DEG_RAD);
	sin_rot = sin (IridescentPublicData.yaw * DEG_RAD);

	rot_matrix[0][0] = cos_rot;
	rot_matrix[0][1] = -1 * sin_rot;
	rot_matrix[0][2] = 0;
	rot_matrix[1][0] = sin_rot;
	rot_matrix[1][1] = cos_rot;
	rot_matrix[1][2] = 0;
	rot_matrix[2][0] = 0;
	rot_matrix[2][1] = 0;
	rot_matrix[2][2] = 1;

	ref_vec = Matrix_mult(ref_vec, rot_matrix);

	ref_vec.Normalize();

	MCCOMErr result = MC_S_OK;

	if (IridescentPublicData.fMixShade)
	result = IridescentPublicData.fMixShade->QueryInterface(IID_I3DShShader, (void**)&fMixShader);
	
	Imp_Out = 0;

	if (fMixShader != NULL)
		Imp_Out = fMixShader->GetImplementedOutput();  

	return result;

}

Iridescent::Iridescent()		// Initialize the public data
{
	IridescentPublicData.Colors[0].Color.red = 0.5;
	IridescentPublicData.Colors[0].Color.green = 0;
	IridescentPublicData.Colors[0].Color.blue = 1;
	IridescentPublicData.Colors[1].Color.red = 0;
	IridescentPublicData.Colors[1].Color.green = 0;
	IridescentPublicData.Colors[1].Color.blue = 1;
	IridescentPublicData.Colors[2].Color.red = 0;
	IridescentPublicData.Colors[2].Color.green = 1;
	IridescentPublicData.Colors[2].Color.blue = 1;
	IridescentPublicData.Colors[3].Color.red = 0;
	IridescentPublicData.Colors[3].Color.green = 1;
	IridescentPublicData.Colors[3].Color.blue = 0;
	IridescentPublicData.Colors[4].Color.red = 1;
	IridescentPublicData.Colors[4].Color.green = 1;
	IridescentPublicData.Colors[4].Color.blue = 0;
	IridescentPublicData.Colors[5].Color.red = 1;
	IridescentPublicData.Colors[5].Color.green = 0.5;
	IridescentPublicData.Colors[5].Color.blue = 0;
	IridescentPublicData.Colors[6].Color.red = 1;
	IridescentPublicData.Colors[6].Color.green = 0;
	IridescentPublicData.Colors[6].Color.blue = 0;
	for (int32 index = 0; index <= 6; index++){
		IridescentPublicData.Colors[index].fCenter = index * 15;
		IridescentPublicData.Colors[index].fFalloff = 3;
		}
	IridescentPublicData.yaw = 0;
	IridescentPublicData.pitch = 0;
	IridescentPublicData.last_is_rest = 0;
	IridescentPublicData.grayscale = 0;
	IridescentPublicData.inv_gray = 0;
	IridescentPublicData.copy_falloff = 1;
	IridescentPublicData.fMixShade = NULL;
	IridescentPublicData.Mode = kSmooth_mode;
	IridescentPublicData.VecMode = kUseRefVec;
	//IridescentPublicData.objectname = NULL;

	last_luma = 0;
}
	
void* Iridescent::GetExtensionDataBuffer()
{
	return ((void*) &(IridescentPublicData));
}

boolean Iridescent::IsEqualTo(I3DExShader* aShader)		// Compare two Iridescent shaders
{
	int32 equality = 1;	
	for (int32 index = 0; index <= 6; index++)
		if (IridescentPublicData.Colors[index].Color.red != ((Iridescent*)aShader)->IridescentPublicData.Colors[index].Color.red ||
			IridescentPublicData.Colors[index].Color.green != ((Iridescent*)aShader)->IridescentPublicData.Colors[index].Color.green ||
			IridescentPublicData.Colors[index].Color.blue != ((Iridescent*)aShader)->IridescentPublicData.Colors[index].Color.blue ||
			IridescentPublicData.Colors[index].fFalloff != ((Iridescent*)aShader)->IridescentPublicData.Colors[index].fFalloff ||
			IridescentPublicData.Colors[index].fCenter != ((Iridescent*)aShader)->IridescentPublicData.Colors[index].fCenter)
				equality = 0;
	if 	(IridescentPublicData.yaw != ((Iridescent*)aShader)->IridescentPublicData.yaw ||
			IridescentPublicData.pitch != ((Iridescent*)aShader)->IridescentPublicData.pitch ||
			IridescentPublicData.last_is_rest != ((Iridescent*)aShader)->IridescentPublicData.last_is_rest ||
			IridescentPublicData.copy_falloff != ((Iridescent*)aShader)->IridescentPublicData.copy_falloff ||
			IridescentPublicData.grayscale!= ((Iridescent*)aShader)->IridescentPublicData.grayscale ||
			IridescentPublicData.inv_gray!= ((Iridescent*)aShader)->IridescentPublicData.inv_gray ||
			IridescentPublicData.Mode != ((Iridescent*)aShader)->IridescentPublicData.Mode ||
			IridescentPublicData.copy_falloff != ((Iridescent*)aShader)->IridescentPublicData.copy_falloff ||
			IridescentPublicData.fMixShade != ((Iridescent*)aShader)->IridescentPublicData.fMixShade ||
			IridescentPublicData.objectname != ((Iridescent*)aShader)->IridescentPublicData.objectname
			)
				equality = 0;
	return equality;
		  
}

MCCOMErr Iridescent::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fNeedsNormal = true;
	theFlags.fConstantChannelsMask = kNoChannel;

	return MC_S_OK;
}

EShaderOutput Iridescent::GetImplementedOutput()
{
	return  kUsesGetColor;	
}

#if (VERSIONNUMBER >= 0x040000)
boolean Iridescent::WantsTransform()
{
	return 0;
}
#endif

#if (VERSIONNUMBER == 0x010000)
MCCOMErr Iridescent::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Iridescent::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Iridescent::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x040000)
real Iridescent::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	fullArea = false;

	TMCColorRGBA hold;
	hold.red = 0; 
	hold.blue = 0;
	hold.green = 0;
	//result.alpha = 1;

	if (IridescentPublicData.VecMode == kUseObject)
	{
		TTransform3D sped;
		TVector3 light_loc;
		TVector3 light_vec;

		light_vec = ref_vec;

	//first figure out the scene
		if (MCVerify(shadingIn.fInstance))
			if (shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree) == MC_S_OK)		
				tree->GetScene(&scene);	// Find the scene
		// temp variables

		if (MCVerify(scene))
		{
			//now that you have a scene, use gettreeelementbyname to find your object (object name from text box)
			scene->GetTreeElementByName( &objecttree, IridescentPublicData.objectname );
			if (MCVerify(objecttree))
			{
				//get the tree's global position
				objecttree->GetGlobalTransform3D(sped);
				
				light_loc = sped.fTranslation;
				light_vec = light_loc - shadingIn.fPoint;
				light_vec.Normalize(); 
			}
		}
		ref_vec = light_vec;		// swap to the light vector
	}

	// calculate angle of surface vs angle of ref vector (light incidence)
	angle = ( acos (( ref_vec * shadingIn.fGNormal) / (ref_vec.GetMagnitude() * shadingIn.fGNormal.GetMagnitude())));

	real32 degree = angle * RAD_DEG;

	// Add in mix shader, if requested
	if (IridescentPublicData.Mode == kShader_mode)
		{
			if (!fMixShader)  // if the mix shader is null, just return black.
			{
				luma = 0;
			}
			else
			{
				if (Imp_Out & kUsesGetColor)		// get color implemented
				{
					fMixShader->GetColor(Color, fullArea, shadingIn);
					luma = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11);
				}
				else if (Imp_Out & kUsesGetValue)  //  or get value implemented
					fMixShader->GetValue(luma, fullArea, shadingIn);
				else 
					luma = 0;		// no useful data back from mix shader, return black
			}
		luma = luma > 1 ? last_luma : luma;

		last_luma = luma;	// in case it returns over 1.0, a bad value, preserve the last good value

		angle = ((angle + (luma - 0.5)) > 0 ?(angle + (luma - 0.5)) : 0); // pin it at zero so the pattern won't repeat.
		}

	if (IridescentPublicData.grayscale)
	{
		real32 luma_val = angle/PI;
		if (IridescentPublicData.inv_gray)
			luma_val = 1 - luma_val;
		result.R = luma_val;
		result.G = luma_val;
		result.B = luma_val;
		#if (VERSIONNUMBER == 0x010000)
		return MC_S_OK;
		#elif (VERSIONNUMBER == 0x020000)
		return MC_S_OK;
		#elif (VERSIONNUMBER >= 0x030000)
		return 1.0f;
		#endif
	}
	else
	{
		// calculate how much of each color applies for this point	
		for (int32 index = 0; index <= 6; index++)
			weight[index] = Calc_weight(IridescentPublicData.copy_falloff ? 
			IridescentPublicData.Colors[0].fFalloff : IridescentPublicData.Colors[index].fFalloff, angle, 
			IridescentPublicData.Colors[index].fCenter * DEG_RAD);

		// Override for filling last color, if selected.
		if (IridescentPublicData.last_is_rest & (angle >= IridescentPublicData.Colors[6].fCenter * DEG_RAD))
			weight[6] = 1;

		// initialize colors
		result.R = 0;
		result.G = 0;
		result.B = 0;

		// sum up the colors participation
		// index selects each color, sums up
		for (int32 index = 0; index <= 6; index++) {
			result.R = result.R + weight[index] * IridescentPublicData.Colors[index].Color.red;
			result.G = result.G + weight[index] * IridescentPublicData.Colors[index].Color.green;
			result.B = result.B + weight[index] * IridescentPublicData.Colors[index].Color.blue;
		}

		result.R = (result.R > 1) ? 1 : result.R;
		result.G = (result.G > 1) ? 1 : result.G;
		result.B = (result.B > 1) ? 1 : result.B;


#if (VERSIONNUMBER == 0x010000)
return MC_S_OK;
#elif (VERSIONNUMBER == 0x020000)
return MC_S_OK;
#elif (VERSIONNUMBER >= 0x030000)
return 1.0f;
#endif
	}
}

// Matrix multiply
TVector3 Iridescent::Matrix_mult(TVector3 vector, real32 rot_matrix[3][3])
{
TVector3 temp;
temp[0] = 0;
temp[1] = 0;
temp[2] = 0;

	for (int32 row = 0; row <= 2; row++)
		for (int32 col = 0; col <= 2; col++)
			temp[row] += vector[col] * rot_matrix[col][row];  // flip rot_matrix to walk down the columns 1st
	return temp;
}

// Calculate the weight
real32 Iridescent::Calc_weight(real32 falloff, real32 angle, real32 center)
{
real32 vector;

	vector = 1 - falloff * fabs (angle - center);
	return   (vector > 0) ? vector :  0;
}