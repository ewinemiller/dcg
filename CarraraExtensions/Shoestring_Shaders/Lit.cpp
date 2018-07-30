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
#include "Lit.h"
#include "copyright.h"
#include "Shoestringshaders.h"
#include "I3DShLightSource.h"
#include "I3DExLight.h"
#include "IExtension.h"

MCCOMErr Lit::ExtensionDataChanged()
{
	return MC_S_OK;
}

Lit::Lit()		// Initialize the public data
{
	LitPublicData.inv_gray = 0;
	LitPublicData.Angle[0] = 20;
	LitPublicData.Angle[1] = 160;
	LitPublicData.color = 0;
	LitPublicData.Point_mode = kSurfPoint;
	LitPublicData.shine_thru = 0;
	LitPublicData.New_mode = 0;
	last_luma = 0;
}
	
void* Lit::GetExtensionDataBuffer()
{
	return ((void*) &(LitPublicData));
}

boolean Lit::IsEqualTo(I3DExShader* aShader)		// Compare two Lit shaders
{
	int32 equality = 1;	

	if 	(
			LitPublicData.inv_gray!= ((Lit*)aShader)->LitPublicData.inv_gray ||
			LitPublicData.color!= ((Lit*)aShader)->LitPublicData.color ||
			LitPublicData.lightname!= ((Lit*)aShader)->LitPublicData.lightname ||
			LitPublicData.Point_mode!= ((Lit*)aShader)->LitPublicData.Point_mode ||
			LitPublicData.shine_thru!= ((Lit*)aShader)->LitPublicData.shine_thru ||
			LitPublicData.New_mode != ((Lit*)aShader)->LitPublicData.New_mode ||
			LitPublicData.Angle!= ((Lit*)aShader)->LitPublicData.Angle 
			)
				equality = 0;
	return equality;
		  
}

MCCOMErr Lit::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fNeedsNormal = true;
	theFlags.fConstantChannelsMask = kNoChannel;

	return MC_S_OK;
}

EShaderOutput Lit::GetImplementedOutput()
{
	return  (EShaderOutput)(kUsesGetColor + kUsesGetValue);	
}

#if (VERSIONNUMBER >= 0x040000)
boolean Lit::WantsTransform()
{
	return 0;
}
#endif

real Lit::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	TMCColorRGBA temp;
	real hold;
	hold = 0;
	result = hold;
	real32 luma = 0;
	real32 luma_angle = 1.0;

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
//	TMCColorRGBA light_color;
	TVector3 identity;
	TVector3 compare;
	real32 cone_angle;
	TVector3 shaded_location;
	TVector3 hotpoint;
	TTreeTransform scale;

	TVector3 NonUniformScale = scale.GetXYZScaling();
	real UniformScale = scale.GetUniformScaling();

	hotpoint = TVector3::kZero;
	shaded_location = TVector3::kZero;

	if (MCVerify(scene)) {
		//now that you have a scene, use gettreeelementbyname to find your object (object name from text box)
		scene->GetTreeElementByName( &temp_tree, LitPublicData.lightname );
		if (MCVerify(temp_tree)){
			tree->GetGlobalTransform3D(sped);   // while I'm at at it, get the loc of the object being shaded.
			shaded_location = sped.fTranslation;
			tree->GetHotPoint(hotpoint);			// and its hotpoint (moved to global coords)
			tree->GetGlobalTreeTransform(scale);
			hotpoint.x =  (hotpoint.x * NonUniformScale.x * UniformScale) + shaded_location.x;
			hotpoint.y =  (hotpoint.y * NonUniformScale.y * UniformScale) + shaded_location.y;
			hotpoint.z =  (hotpoint.z * NonUniformScale.z * UniformScale) + shaded_location.z;
		}

		find_point = (LitPublicData.Point_mode == kSurfPoint) ? shadingIn.fPoint : shaded_location;
		find_point = (LitPublicData.Point_mode == kHotPoint) ? hotpoint : find_point;

		TMCCountedPtr<I3DShLightsource> lightTree;
		TVector3 dir_to_light;
		real distance;
		TStandardLight st_light;
		real brightness;

		if (MCVerify(temp_tree)) {
			// ok, lets see if we can't find out what kind of a light we are dealing with!!
			temp_tree->QueryInterface(IID_I3DShLightsource, (void**)&lightTree);
			if (lightTree){																				// bulb
				lightTree->GetLightGlobalTransform(&sped);
				light_loc = sped.fTranslation;
				light_vec = light_loc - find_point;
				light_vec.Normalize(); 
				lightTree->GetDirection(find_point, dir_to_light, distance);
				lightTree->GetStandardLight(st_light);
				lightTree->GetLightParameter(1651665268, &brightness);				// 'brit' gets brightness
				//lightTree->GetLightParameter(1668246639, &light_color);				// 'colo' gets color

				real32 light_dist_max = st_light.fFalloffDistanceMax;
				real32 light_dist_min = st_light.fFalloffDistanceMin;
				real32 light_cone_max = st_light.fConeAngleDegreesMax;
				real32 light_cone_min = st_light.fConeAngleDegreesMin;
				int32 light_type = st_light.fLightType;						//kDistantLight,kPointLight,kSpotLight

				TVector3 normal = shadingIn.fGNormal;				// angle of surface to light
				real32 surface_angle = ( acos (( dir_to_light * normal) / 
								(dir_to_light.GetMagnitude() * normal.GetMagnitude())));
				if (LitPublicData.shine_thru)							// backside, modify to swap.
					surface_angle = (surface_angle >= PI2) ? (PI2 - (surface_angle - PI2)) :  surface_angle + PI2;

				if (LitPublicData.New_mode) {
					if ((surface_angle <= LitPublicData.Angle.y) &&
						(surface_angle >= LitPublicData.Angle.x)){	// if it is facing the light. . . .
						if (light_type == kDistantLight) {	// distant lights don't lose brightness, don't have cones.
							luma = 1.0;
						}
						else {
							// check distance for spot and bulb
							if (distance <= light_dist_max){
								if (distance <= light_dist_min) luma = 1.0;
								else luma = 1.0 - (distance - light_dist_min)/
													(light_dist_max - light_dist_min);	// bulb is done here

								// check angle to make sure we are in the cone.
								if (light_type == kSpotLight){				
									luma_angle = 0;
									const TVector3 compare = sped.TransformVector(TVector3::kNegativeZ); // get the vector
									cone_angle = ( acos (( dir_to_light * compare) / 
														(dir_to_light.GetMagnitude() * compare.GetMagnitude()))); 
									cone_angle = cone_angle * RAD_DEG;  // convert to degrees cuz thats how the limits are
									if (cone_angle > (180 - light_cone_min)){
										if (cone_angle > (180 - light_cone_max))
											luma_angle = 1.0;
										else
											luma_angle = 1.0 - (180 - cone_angle - light_cone_max)/
													(light_cone_min - light_cone_max);
									}					
								}
								else luma_angle = 1.0;
							}
						}
						real32 angle = ( 1 - ((surface_angle - LitPublicData.Angle.x)/
							(LitPublicData.Angle.y - LitPublicData.Angle.x))); 
						luma = luma * luma_angle * angle ;
						temp = st_light.fDiffuse/brightness * luma;						
						}
					}
				else {								// Legacy mode
					if (surface_angle <= LitPublicData.Angle.y){	// if it is facing the light. . . .
						if (light_type == kDistantLight) {			// distant lights don't lose brightness, don't have cones.
							luma = 1.0;
						}
						else {
							// check distance for spot and bulb
							if (distance <= light_dist_max){
								if (distance <= light_dist_min) luma = 1.0;
								else luma = 1.0 - (distance - light_dist_min)/
													(light_dist_max - light_dist_min);	// bulb is done here

								// check angle to make sure we are in the cone.
								if (light_type == kSpotLight){				
									luma_angle = 0;
									const TVector3 compare = sped.TransformVector(TVector3::kNegativeZ); // get the vector
									cone_angle = ( acos (( dir_to_light * compare) / 
														(dir_to_light.GetMagnitude() * compare.GetMagnitude()))); 
									cone_angle = cone_angle * RAD_DEG;  // convert to degrees cuz thats how the limits are
									if (cone_angle > (180 - light_cone_min)){
										if (cone_angle > (180 - light_cone_max))
											luma_angle = 1.0;
										else
											luma_angle = 1.0 - (180 - cone_angle - light_cone_max)/
													(light_cone_min - light_cone_max);
									}					
								}
								else luma_angle = 1.0;
							}
						}
						real32 angle = ( 1 - ((surface_angle - LitPublicData.Angle.x)/
							(LitPublicData.Angle.y - LitPublicData.Angle.x))); 
						luma = luma * luma_angle * angle ;
						temp = st_light.fDiffuse/brightness * luma;						
						}
					}

			}
		}
	}
	if (LitPublicData.inv_gray)
		luma = 1 - luma;

	//	if (luma > 1)
	//	luma = 1;
	//if (luma < 0)
	//	luma = 0;

	if (!LitPublicData.color){
		result = luma;

	}


return 1.0f;
}

#if (VERSIONNUMBER == 0x010000)
MCCOMErr Lit::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Lit::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Lit::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x040000)
real Lit::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	fullArea = false;
	TMCColorRGBA hold;
	hold.kBlack;
	result = hold;
	real32 luma = 0;
	real32 luma_angle = 1.0;

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
//	TMCColorRGBA light_color;
	TVector3 identity;
	TVector3 compare;
	real32 cone_angle;
	TVector3 shaded_location;
	TVector3 hotpoint;
	TTreeTransform scale;

	TVector3 NonUniformScale = scale.GetXYZScaling();
	real UniformScale = scale.GetUniformScaling();

	hotpoint = TVector3::kZero;
	shaded_location = TVector3::kZero;

	if (MCVerify(scene)) {
		//now that you have a scene, use gettreeelementbyname to find your object (object name from text box)
		scene->GetTreeElementByName( &temp_tree, LitPublicData.lightname );
		if (MCVerify(temp_tree)){
			tree->GetGlobalTransform3D(sped);   // while I'm at at it, get the loc of the object being shaded.
			shaded_location = sped.fTranslation;
			tree->GetHotPoint(hotpoint);			// and its hotpoint (moved to global coords)
			tree->GetGlobalTreeTransform(scale);
			hotpoint.x =  (hotpoint.x * NonUniformScale.x * UniformScale) + shaded_location.x;
			hotpoint.y =  (hotpoint.y * NonUniformScale.y * UniformScale) + shaded_location.y;
			hotpoint.z =  (hotpoint.z * NonUniformScale.z * UniformScale) + shaded_location.z;
		}

		find_point = (LitPublicData.Point_mode == kSurfPoint) ? shadingIn.fPoint : shaded_location;
		find_point = (LitPublicData.Point_mode == kHotPoint) ? hotpoint : find_point;

		TMCCountedPtr<I3DShLightsource> lightTree;
		TVector3 dir_to_light;
		real distance;
		TStandardLight st_light;
		real brightness;

		if (MCVerify(temp_tree)) {
			// ok, lets see if we can't find out what kind of a light we are dealing with!!
			temp_tree->QueryInterface(IID_I3DShLightsource, (void**)&lightTree);
			if (lightTree){																				// bulb
				lightTree->GetLightGlobalTransform(&sped);
				light_loc = sped.fTranslation;
				light_vec = light_loc - find_point;
				light_vec.Normalize(); 
				lightTree->GetDirection(find_point, dir_to_light, distance);
				lightTree->GetStandardLight(st_light);
				lightTree->GetLightParameter(1651665268, &brightness);				// 'brit' gets brightness
				//lightTree->GetLightParameter(1668246639, &light_color);				// 'colo' gets color

				real32 light_dist_max = st_light.fFalloffDistanceMax;
				real32 light_dist_min = st_light.fFalloffDistanceMin;
				real32 light_cone_max = st_light.fConeAngleDegreesMax;
				real32 light_cone_min = st_light.fConeAngleDegreesMin;
				int32 light_type = st_light.fLightType;						//kDistantLight,kPointLight,kSpotLight

				TVector3 normal = shadingIn.fGNormal;				// angle of surface to light
				real32 surface_angle = ( acos (( dir_to_light * normal) / 
								(dir_to_light.GetMagnitude() * normal.GetMagnitude())));
				if (LitPublicData.shine_thru)							// backside, modify to swap.
					surface_angle = (surface_angle >= PI2) ? (PI2 - (surface_angle - PI2)) :  surface_angle + PI2;

				if (LitPublicData.New_mode) {
					if ((surface_angle <= LitPublicData.Angle.y) &&
						(surface_angle >= LitPublicData.Angle.x)){	// if it is facing the light. . . .
						if (light_type == kDistantLight) {	// distant lights don't lose brightness, don't have cones.
							luma = 1.0;
						}
						else {
							// check distance for spot and bulb
							if (distance <= light_dist_max){
								if (distance <= light_dist_min) luma = 1.0;
								else luma = 1.0 - (distance - light_dist_min)/
													(light_dist_max - light_dist_min);	// bulb is done here

								// check angle to make sure we are in the cone.
								if (light_type == kSpotLight){				
									luma_angle = 0;
									const TVector3 compare = sped.TransformVector(TVector3::kNegativeZ); // get the vector
									cone_angle = ( acos (( dir_to_light * compare) / 
														(dir_to_light.GetMagnitude() * compare.GetMagnitude()))); 
									cone_angle = cone_angle * RAD_DEG;  // convert to degrees cuz thats how the limits are
									if (cone_angle > (180 - light_cone_min)){
										if (cone_angle > (180 - light_cone_max))
											luma_angle = 1.0;
										else
											luma_angle = 1.0 - (180 - cone_angle - light_cone_max)/
													(light_cone_min - light_cone_max);
									}					
								}
								else luma_angle = 1.0;
							}
						}
						real32 angle = ( 1 - ((surface_angle - LitPublicData.Angle.x)/
							(LitPublicData.Angle.y - LitPublicData.Angle.x))); 
						luma = luma * luma_angle * angle ;
						result = st_light.fDiffuse/brightness * luma;						
						}
					}
				else {								// Legacy mode
					if (surface_angle <= LitPublicData.Angle.y){	// if it is facing the light. . . .
						if (light_type == kDistantLight) {			// distant lights don't lose brightness, don't have cones.
							luma = 1.0;
						}
						else {
							// check distance for spot and bulb
							if (distance <= light_dist_max){
								if (distance <= light_dist_min) luma = 1.0;
								else luma = 1.0 - (distance - light_dist_min)/
													(light_dist_max - light_dist_min);	// bulb is done here

								// check angle to make sure we are in the cone.
								if (light_type == kSpotLight){				
									luma_angle = 0;
									const TVector3 compare = sped.TransformVector(TVector3::kNegativeZ); // get the vector
									cone_angle = ( acos (( dir_to_light * compare) / 
														(dir_to_light.GetMagnitude() * compare.GetMagnitude()))); 
									cone_angle = cone_angle * RAD_DEG;  // convert to degrees cuz thats how the limits are
									if (cone_angle > (180 - light_cone_min)){
										if (cone_angle > (180 - light_cone_max))
											luma_angle = 1.0;
										else
											luma_angle = 1.0 - (180 - cone_angle - light_cone_max)/
													(light_cone_min - light_cone_max);
									}					
								}
								else luma_angle = 1.0;
							}
						}
						real32 angle = ( 1 - ((surface_angle - LitPublicData.Angle.x)/
							(LitPublicData.Angle.y - LitPublicData.Angle.x))); 
						luma = luma * luma_angle * angle ;
						result = st_light.fDiffuse/brightness * luma;						
						}
					}

			}
		}
	}
	if (LitPublicData.inv_gray)
		luma = 1 - luma;
	if (!LitPublicData.color){
		result.R = luma;
		result.G = luma;
		result.B = luma;
		result.A = 1;
	}

#if (VERSIONNUMBER == 0x010000)
return MC_S_OK;
#elif (VERSIONNUMBER == 0x020000)
return MC_S_OK;
#elif (VERSIONNUMBER >= 0x030000)
return 1.0f;
#endif
}