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


#ifndef __WEAVE__
#define __WEAVE__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "WeaveDef.h"
#include "BasicShader.h"
//Alan
#include "ShaderBase.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "MCColorRGBA.h"
#include "copyright.h"


// Define the Weave CLSID ( see the WeaveDef.h file to get R_CLSID_Weave value )

extern const MCGUID CLSID_Weave;

struct WeaveShaderPublicData
{
	int32	Mode;
	boolean	OnTop[5][5];
	int32	Wiggle_Intensity;
	int32	Edge_Intensity;
	boolean	ThreeD_across;					// flat or 3-D.
	boolean	ThreeD_along;					// flat or 3-D.
	real32	Across_intensity;
	real32	Along_intensity;
	int32	U_Blank_Width;				
	int32	V_Blank_Width;				
	int32	Tiles_U;				
	int32	Tiles_V;				
	int32	UWidths[5];		// 
	int32	VWidths[5];		// 
	boolean	Mask;
	boolean	Inv_Mask;
	TMCCountedPtr<IShParameterComponent> WShader;		// wiggle
	TMCCountedPtr<IShParameterComponent> EShader;		// edge
	TMCCountedPtr<IShParameterComponent> BShader;		// background 
	TMCCountedPtr<IShParameterComponent> UShader[5];		// U shaders
	TMCCountedPtr<IShParameterComponent> VShader[5];		// V shaders
	boolean	Comp_Along;
	boolean	Comp_Across;
};

//Alan class Weave : public TBasicShader
class Weave : public ShaderBase
{
public :

	WeaveShaderPublicData Data;

	Weave();

	STANDARD_RELEASE;

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(WeaveShaderPublicData);}

#if (VERSIONNUMBER <= 0x020000)
	virtual MCCOMErr		MCCOMAPI    GetColor	(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);
	virtual MCCOMErr		MCCOMAPI	GetVector		(TVector3& result,ShadingIn& shadingIn);
#elif (VERSIONNUMBER == 0x030000)
	virtual real 		MCCOMAPI		GetColor	(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);
	virtual real		MCCOMAPI	GetVector		(TVector3& result,ShadingIn& shadingIn);
#elif (VERSIONNUMBER >= 0x040000)
	virtual real 		MCCOMAPI		GetColor	(TMCColorRGBA& result,boolean& fullArea, ShadingIn& shadingIn);
	virtual real		MCCOMAPI	GetVector		(TVector3& result,ShadingIn& shadingIn);
	virtual boolean		MCCOMAPI    WantsTransform	();  
#endif


protected:

	TMCCountedPtr<I3DShShader> UShaders[5];
	TMCCountedPtr<I3DShShader> VShaders[5];
	TMCCountedPtr<I3DShShader> WShaders;
	TMCCountedPtr<I3DShShader> EShaders;
	TMCCountedPtr<I3DShShader> BShaders;

private:

	// Structures
		struct Loc_Data				// structure for returning data
		{
			int32 shader;
			real32 distance;
			real32 half;
			real32 percent;
		};

		struct Threads				// holds data about threads
		{
			int32		quant;
			real32	range[11];		//portion covered
			real32	middle[5];		// mid point
			real32	half[5];		// half of width
		};

		struct Segments
		{
			real32	seg_begin;
			real32	seg_end;
			real32	seg_mid;
			real32	seg_half;
		};

		struct Bands
		{
			int32			Seg_quant;
            Segments	Segs[6];		// max number of segments in a line
		};

// Subroutines
		boolean			DoWiggle (boolean& fullArea, ShadingIn& shadingIn);
		boolean			DoEdge (boolean& fullArea, ShadingIn& shadingIn);
		boolean			Threadpicker(Loc_Data &Return_values, Threads &in_Thread, real32 coordinate);
		boolean 			Alongthread(Loc_Data &Return_values, int32 shader, real32 coordinate, Bands *in_band);
		TVector3		VectorRotate(TVector3 shaft, TVector3 vector, real32 asin_angle);

// variables
#if (VERSIONNUMBER <= 0x030000)
	TMCColorRGB Color;
	TMCColorRGB hold;
#elif (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA Color;
	TMCColorRGBA hold;
#endif		
		real32			total_width;		// 
		uint32			UImp_Out[5];	// keep track of what the implemented outputs are
		uint32			VImp_Out[5];	// keep track of what the implemented outputs are
		uint32			WShader_Imp;
		uint32			EShader_Imp;
		uint32			BShader_Imp;

		real32			current_u;
		real32			current_v;
		real32			one_over_U;
		real32			one_over_V;
		int32			u_shader;
		int32			v_shader;
		real32			uu_distance;
		real32			vv_distance;
		real32			uv_distance;
		real32			vu_distance;
		real32			uu_half;
		real32			vv_half;
		real32			uv_half;
		real32			vu_half;
		real32			uu_percent;
		real32			vv_percent;
		real32			uv_percent;
		real32			vu_percent;
		int32			winner;
		boolean			u_wins;
		real32			Across_intensity;
		real32			Along_intensity;

		Threads		U_threads;
		Threads		V_threads;

		Loc_Data Return_values;

		Bands		U_bands[5];
		Bands		V_bands[5];

		enum
		{
			kBasic_mode	= IDTYPE('B', 'S', 'C', 'M'),			//IDs from .rsr
			kAdvanced_mode	= IDTYPE('A', 'D', 'V', 'M')
		};
};
                           
#endif // __Weave__
