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
#include "Translucent.h"
#include "copyright.h"
#include "Shoestringshaders.h"

MCCOMErr Translucent::ExtensionDataChanged()
{
	/*
	Code from http://en.wikipedia.org/wiki/B%E9zier_curve#Application_in_computer_graphics
	cp is a 4 element array where:
	cp[0] is the starting point, or A in the above diagram
	cp[1] is the first control point, or B
	cp[2] is the second control point, or C
	cp[3] is the end point, or D
	*/

	cp[0] = 0;
	cp[1] = TranslucentPublicData.Ease.x;
	cp[2] = TranslucentPublicData.Ease.y;
	cp[3] = 1;
        
    /* calculate the polynomial coefficients */
    
    cy = 3.0 * (cp[1] - cp[0]);
    by = 3.0 * (cp[2] - cp[1]) - cy;
    ay = cp[3] - cp[0] - cy - by;
        
	MCCOMErr result = MC_S_OK;

	return result;
}

Translucent::Translucent()		// Initialize the public data
{
	TranslucentPublicData.inv_gray = 0;
	TranslucentPublicData.Mode = kLinear_mode;
	TranslucentPublicData.Angle[0] = 20;
	TranslucentPublicData.Angle[1] = 160;
	TranslucentPublicData.Ease.x = 0;
	TranslucentPublicData.Ease.y = 100;
	TranslucentPublicData.Point_mode = kSurfPoint;

	last_luma = 0;
}
	
void* Translucent::GetExtensionDataBuffer()
{
	return ((void*) &(TranslucentPublicData));
}

boolean Translucent::IsEqualTo(I3DExShader* aShader)		// Compare two Translucent shaders
{
	int32 equality = 1;	

	if 	(
			TranslucentPublicData.lightname!= ((Translucent*)aShader)->TranslucentPublicData.lightname ||
			TranslucentPublicData.inv_gray!= ((Translucent*)aShader)->TranslucentPublicData.inv_gray ||
			TranslucentPublicData.Mode != ((Translucent*)aShader)->TranslucentPublicData.Mode ||
			TranslucentPublicData.Angle != ((Translucent*)aShader)->TranslucentPublicData.Angle ||
			TranslucentPublicData.Ease != ((Translucent*)aShader)->TranslucentPublicData.Ease ||
			TranslucentPublicData.Point_mode != ((Translucent*)aShader)->TranslucentPublicData.Point_mode ||
			TranslucentPublicData.cameraname != ((Translucent*)aShader)->TranslucentPublicData.cameraname
			)
				equality = 0;
	return equality;
		  
}

MCCOMErr Translucent::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fNeedsNormal = true;
	theFlags.fConstantChannelsMask = kNoChannel;

	return MC_S_OK;
}

EShaderOutput Translucent::GetImplementedOutput()
{
	return  kUsesGetColor;	
}

#if (VERSIONNUMBER >= 0x040000)
boolean Translucent::WantsTransform()
{
	return 0;
}
#endif

#if (VERSIONNUMBER == 0x010000)
MCCOMErr Translucent::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Translucent::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Translucent::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x040000)
real Translucent::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#endif

{
	fullArea = false;

	hold.red = 0; 
	hold.blue = 0;
	hold.green = 0;

	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShScene> scene;
	TMCCountedPtr<I3DShTreeElement> temp_tree;

	//first figure out the scene
	if (MCVerify(shadingIn.fInstance))
		if (shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree) == MC_S_OK)		
			tree->GetScene(&scene);	// Find the scene
	// temp variables
	TTransform3D sped;
	TVector3 light_loc;
	TVector3 light_vec;
	TVector3 camera_loc;
	TVector3 camera_vec;

	TVector3 shaded_location;
	TVector3 hotpoint;
	TTreeTransform scale;

	TVector3 NonUniformScale = scale.GetXYZScaling();
	real UniformScale = scale.GetUniformScaling();

	if (MCVerify(scene))
	{
		//now that you have a scene, use gettreeelementbyname to find your object (object name from text box)
		scene->GetTreeElementByName( &temp_tree, TranslucentPublicData.lightname );
		if (MCVerify(temp_tree)){
			tree->GetGlobalTransform3D(sped);   // while I'm at at it, get the loc of the object being shaded.
			shaded_location = sped.fTranslation;
			tree->GetHotPoint(hotpoint);			// and its hotpoint (moved to global coords)
			tree->GetGlobalTreeTransform(scale);
			hotpoint.x =  (hotpoint.x * NonUniformScale.x * UniformScale) + shaded_location.x;
			hotpoint.y =  (hotpoint.y * NonUniformScale.y * UniformScale) + shaded_location.y;
			hotpoint.z =  (hotpoint.z * NonUniformScale.z * UniformScale) + shaded_location.z;
		}

	find_point = (TranslucentPublicData.Point_mode == kSurfPoint) ? shadingIn.fPoint : shaded_location;
	find_point = (TranslucentPublicData.Point_mode == kHotPoint) ? hotpoint : find_point;



		if (MCVerify(temp_tree))
		{
			//get the tree's global position
			temp_tree->GetGlobalTransform3D(sped);
			light_loc = sped.fTranslation;
			light_vec = light_loc - find_point;
			light_vec.Normalize(); 
		}
		scene->GetTreeElementByName( &temp_tree, TranslucentPublicData.cameraname );
		if (MCVerify(temp_tree))
		{
			temp_tree->GetGlobalTransform3D(sped);
			camera_loc = sped.fTranslation;
			camera_vec = camera_loc - find_point;
			camera_vec.Normalize(); 
		}
	}

	angle = ( acos (( camera_vec * light_vec) / (camera_vec.GetMagnitude() * light_vec.GetMagnitude())));

	if (angle <= TranslucentPublicData.Angle.x){
		angle = 0; }
	else if (angle >= TranslucentPublicData.Angle.y){
		angle = 1; }
	else {
		angle = (angle - TranslucentPublicData.Angle.x) /
				(TranslucentPublicData.Angle.y - TranslucentPublicData.Angle.x); 
		if (TranslucentPublicData.Mode == kBezier_mode) {
			real32 tSquared = angle * angle;
			real32 tCubed = tSquared * angle;        
			angle = (ay * tCubed) + (by * tSquared) + (cy * angle) + cp[0];
		}
	}

	real32 luma_val = angle;
	if (TranslucentPublicData.inv_gray)
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