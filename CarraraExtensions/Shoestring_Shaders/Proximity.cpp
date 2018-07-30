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
#include "Proximity.h"
#include "copyright.h"
#include "Shoestringshaders.h"
#include "I3DShObject.h"
#include "I3DShScene.h"
#include "I3DShGroup.h"
#include "PublicUtilities.h"

MCCOMErr Proximity::ExtensionDataChanged()
{
	MCCOMErr result = MC_S_OK;

	if (ProximityPublicData.fMixShade)
	result = ProximityPublicData.fMixShade->QueryInterface(IID_I3DShShader, (void**)&fMixShader);
	
	Imp_Out = 0;


	if (fMixShader != NULL)
		Imp_Out = fMixShader->GetImplementedOutput();  

	sObject_name = ProximityPublicData.objectname;
	length = sObject_name.Length();
	FadeLength = ProximityPublicData.Limit-ProximityPublicData.DistanceFade;

	return result;
}

Proximity::Proximity()		// Initialize the public data
{

	ProximityPublicData.Use_shader = 0;
	ProximityPublicData.Shader_factor = 1;
	ProximityPublicData.Limit = 10;
	ProximityPublicData.Offset = 0;
	ProximityPublicData.Point_mode = kSurfPoint;
	ProximityPublicData.X_axis = 1;
	ProximityPublicData.Y_axis = 1;
	ProximityPublicData.Z_axis = 1;
	ProximityPublicData.Single = 1;
	ProximityPublicData.DistanceFade = 0;
	last_luma = 0;
}
	
void* Proximity::GetExtensionDataBuffer()
{
	return ((void*) &(ProximityPublicData));
}

boolean Proximity::IsEqualTo(I3DExShader* aShader)		// Compare two Proximity shaders
{
	int32 equality = 1;	
	if ((ProximityPublicData.fMixShade != ((Proximity*)aShader)->ProximityPublicData.fMixShade) ||
	(ProximityPublicData.Limit != ((Proximity*)aShader)->ProximityPublicData.Limit) ||
	(ProximityPublicData.objectname != ((Proximity*)aShader)->ProximityPublicData.objectname) ||
	(ProximityPublicData.Offset != ((Proximity*)aShader)->ProximityPublicData.Offset) ||
	(ProximityPublicData.Shader_factor != ((Proximity*)aShader)->ProximityPublicData.Shader_factor) ||
	(ProximityPublicData.Point_mode != ((Proximity*)aShader)->ProximityPublicData.Point_mode) ||
	(ProximityPublicData.X_axis != ((Proximity*)aShader)->ProximityPublicData.X_axis) ||
	(ProximityPublicData.Y_axis != ((Proximity*)aShader)->ProximityPublicData.Y_axis) ||
	(ProximityPublicData.Z_axis != ((Proximity*)aShader)->ProximityPublicData.Z_axis) ||
	(ProximityPublicData.Use_shader != ((Proximity*)aShader)->ProximityPublicData.Use_shader))
		equality = 0;
	return equality;	  
}

MCCOMErr Proximity::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fNeedsPoint = true;
	theFlags.fNeedsPointLoc = true;
	theFlags.fConstantChannelsMask = kNoChannel;

	return MC_S_OK;
}

EShaderOutput Proximity::GetImplementedOutput()
{
	return  kUsesGetValue;		
}

#if (VERSIONNUMBER == 0x010000)
MCCOMErr Proximity::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Proximity::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x030000)
real Proximity::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	fullArea = false;
	real distance = 0;

	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShScene> scene;
	TMCCountedPtr<I3DShTreeElement> objecttree;
	TVector3 shaded_location;
	TTransform3D sped;
	TVector3 hotpoint;
	TTreeTransform scale;

	TVector3 NonUniformScale = scale.GetXYZScaling();
	real UniformScale = scale.GetUniformScaling();

	result = 0;
	if (length == 0)
		return 0;
	TMCString255 temp;
	//first figure out the scene
	if (MCVerify(shadingIn.fInstance)){
		if (shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree) == MC_S_OK){	
			tree->GetScene(&scene);	// Find the scene
			tree->GetGlobalTransform3D(sped);   // while I'm at at it, get the loc of the object being shaded.
			tree->GetFullName(temp);
			
			shaded_location = sped.fTranslation;
			tree->GetHotPoint(hotpoint);			// and its hotpoint (moved to global coords)
			tree->GetGlobalTreeTransform(scale);
			hotpoint.x =  (hotpoint.x * NonUniformScale.x * UniformScale) + shaded_location.x;
			hotpoint.y =  (hotpoint.y * NonUniformScale.y * UniformScale) + shaded_location.y;
			hotpoint.z =  (hotpoint.z * NonUniformScale.z * UniformScale) + shaded_location.z;
		}
	}
	else
		return 0;

	find_point = (ProximityPublicData.Point_mode == kSurfPoint) ? shadingIn.fPoint : shaded_location;
	find_point = (ProximityPublicData.Point_mode == kHotPoint) ? hotpoint : find_point;

	TVector3 object_loc;
	TVector3 temp_point;
	temp_point = find_point;
	tree->GetTop(&objecttree);

	if (ProximityPublicData.Single)	{	// only looking for a single instance
		if (MCVerify(scene)){ 
			//now that you have a scene, use gettreeelementbyname to find your object (object name from text box)
			scene->GetTreeElementByName( &objecttree, sObject_name );
			if (MCVerify(objecttree)) {
				objecttree->GetGlobalTransform3D(sped); 	//get the tree's global position
				object_loc = sped.fTranslation;	
				if (!ProximityPublicData.X_axis) temp_point.x = object_loc.x;
				if (!ProximityPublicData.Y_axis) temp_point.y = object_loc.y;
				if (!ProximityPublicData.Z_axis) temp_point.z = object_loc.z;
				distance = object_loc.GetDistance(temp_point);
			}
		}
	}
	else {										// time to walk the tree!
		real32 shortest_distance = 10000000;		// hopefully large enough not to cause problems?
		TMCCountedPtr<I3DShGroup> group;
		scene->GetTreeRoot(&group);
		group->QueryInterface(IID_I3DShTreeElement, (void **) &objecttree);
		TTreeElementIterator iter(objecttree);	// this may need to be set to root (probably does) to make it work. . . 
		TMCString1023 sName;
		objecttree->GetName(sName);
		for (I3DShTreeElement* currTree = iter.First() ; iter.More() ; currTree = iter.Next()) {
			
			currTree->GetName(sName);
			sName.SubString(0, length);
			if (sName == sObject_name)	{			// Object is a match!
				currTree->GetGlobalTransform3D(sped);
				object_loc = sped.fTranslation;
				if (!ProximityPublicData.X_axis) temp_point.x = object_loc.x;
				if (!ProximityPublicData.Y_axis) temp_point.y = object_loc.y;
				if (!ProximityPublicData.Z_axis) temp_point.z = object_loc.z;
				distance = object_loc.GetDistance(temp_point);
				if (distance < shortest_distance) shortest_distance = distance;
			}
			temp_point = find_point;		
		}
		distance = shortest_distance;
	}

	distance = distance - ProximityPublicData.Offset;
	// Add in mix shader, if requested
	if (ProximityPublicData.Use_shader) {
		if (!fMixShader){  // if the mix shader is null, just return black.
			luma = 0;
		}
		else {
			TMCColorRGBA Color;
			if (Imp_Out & (kUsesGetColor | kUsesDoShade )){		// get color implemented
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

		distance = distance + (ProximityPublicData.Shader_factor * luma)/ProximityPublicData.Limit;
	}

	if (distance > ProximityPublicData.Limit)
		result = 0;
	else if (ProximityPublicData.DistanceFade == 0)			// legacy mode
		result = 1 - (distance/ProximityPublicData.Limit);
	else if (distance < FadeLength)
		result = 1;
	else
		result = 1 -((distance-FadeLength) / ProximityPublicData.DistanceFade);

	if (result > 1.0) result = 1.0;
	if (result < 0.0) result = 0.0;
#if (VERSIONNUMBER == 0x010000)
return MC_S_OK;
#elif (VERSIONNUMBER == 0x020000)
return MC_S_OK;
#elif (VERSIONNUMBER >= 0x030000)
return 1.0f;
#endif
}
