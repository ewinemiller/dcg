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
#define  _USE_MATH_DEFINES
//#include "math.h"
#include "vector3.h"
#include "Weave.h"
#include "copyright.h"
#include "Shoestringshaders.h"

void* Weave::GetExtensionDataBuffer()
{
	return ((void*) &Data);
}

Weave::Weave()	// Initialize
{
	for (int32 i = 0 ; i<=4 ; i++)
			for (int32 j = 0 ; j<=4 ; j++)
				Data.OnTop[i][j] = ((i+j) % 2) ? 1:0;
	Data.Wiggle_Intensity = 0;
	Data.Edge_Intensity = 0;
	Data.ThreeD_across = 0;
	Data.ThreeD_along = 0;
	Data.Across_intensity = 1.5;
	Data.Along_intensity = 1.5;
	Data.U_Blank_Width = 1;
	Data.V_Blank_Width = 1;
	Data.Tiles_U = 1;
	Data.Tiles_V = 1;
	for (int32 i = 0 ; i<=4 ; i++)
	{
		Data.UWidths[i] = 1;
		Data.VWidths[i] = 1;
		Data.UShader[i] = NULL;
		Data.VShader[i] = NULL;
	}
	Data.WShader = NULL;
	Data.EShader = NULL;
	Data.BShader = NULL;
	Data.Mode = kBasic_mode;
	Data.Mask = 0;
	Data.Inv_Mask = 0;
	Data.Comp_Along = 0;
	Data.Comp_Across = 0;
}

boolean Weave::IsEqualTo(I3DExShader* aShader)
{
	
	boolean flag;

	flag = true;

	for (int32 i = 0 ; i<=24 ; i++)
		if (Data.OnTop[i] != ((Weave*)aShader)->Data.OnTop[i]) 
				flag = false;
	for (int32 i = 0 ; i<=4 ; i++)
	{
		if ((Data.UShader[i] != ((Weave*)aShader)->Data.UShader[i]) |
			(Data.VShader[i] != ((Weave*)aShader)->Data.VShader[i]))
				flag = false;
	}
	if ((Data.Across_intensity != (((Weave*)aShader)->Data.Across_intensity)) |
		(Data.Along_intensity != (((Weave*)aShader)->Data.Along_intensity)) |
		(Data.Mode != (((Weave*)aShader)->Data.Mode)) |
		(Data.U_Blank_Width != (((Weave*)aShader)->Data.U_Blank_Width)) |
		(Data.V_Blank_Width != (((Weave*)aShader)->Data.V_Blank_Width)) |
		(Data.Tiles_U != (((Weave*)aShader)->Data.Tiles_U)) |
		(Data.Tiles_V != (((Weave*)aShader)->Data.Tiles_V)) |
		(Data.Wiggle_Intensity != (((Weave*)aShader)->Data.Wiggle_Intensity)) |
		(Data.Edge_Intensity != (((Weave*)aShader)->Data.Edge_Intensity)) |
		(Data.ThreeD_across != (((Weave*)aShader)->Data.ThreeD_across)) |
		(Data.ThreeD_along != (((Weave*)aShader)->Data.ThreeD_along)) |
		(Data.Mask != (((Weave*)aShader)->Data.Mask)) |
		(Data.Inv_Mask != (((Weave*)aShader)->Data.Inv_Mask)) |
		(Data.WShader != (((Weave*)aShader)->Data.WShader)) |
		(Data.EShader != (((Weave*)aShader)->Data.EShader)) |
		(Data.BShader != (((Weave*)aShader)->Data.BShader)) |
		(Data.Comp_Along != (((Weave*)aShader)->Data.Comp_Along)) |
		(Data.Comp_Across != (((Weave*)aShader)->Data.Comp_Across)) 
		)  
			flag = false;

	return flag; 
}


MCCOMErr Weave::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fNeedsNormal = true;
	theFlags.fNeedsIsoUV = true;		// IsoUV are normals aligned with the UV axes
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput Weave::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetColor + kUsesGetVector);
}

#if (VERSIONNUMBER >= 0x040000)
boolean Weave::WantsTransform()
{
	return 0;
}
#endif

MCCOMErr Weave::ExtensionDataChanged()
{
	int32 i;
	int32 j;

	MCCOMErr result1 = MC_S_OK;
	MCCOMErr result2 = MC_S_OK;

	for (i = 0 ; i<=4 ; i++)	// Copy the shader data to the local data structures		
		{
			if (Data.UShader[i])
			{
				result1 = Data.UShader[i]->QueryInterface(IID_I3DShShader, (void**)&UShaders[i]);
				if (result1 != MC_S_OK)
					result2 = MC_S_FALSE;
			}
			else 
				UShaders[i] = NULL;
			if (Data.VShader[i])
			{
				result1 = Data.VShader[i]->QueryInterface(IID_I3DShShader, (void**)&VShaders[i]);
				if (result1 != MC_S_OK)
					result2 = MC_S_FALSE;
			}
			else
				VShaders[i] = NULL;
		}

	if (Data.WShader)
	{
        result1 = Data.WShader->QueryInterface(IID_I3DShShader, (void**)&WShaders);
		if (result1 != MC_S_OK)
			result2 = MC_S_FALSE;
	}
	else
		WShaders = NULL;
	if (Data.EShader)
	{
        result1 = Data.EShader->QueryInterface(IID_I3DShShader, (void**)&EShaders);
		if (result1 != MC_S_OK)
			result2 = MC_S_FALSE;
	}
	else
		EShaders = NULL;
	if (Data.BShader)
	{
        result1 = Data.BShader->QueryInterface(IID_I3DShShader, (void**)&BShaders);
		if (result1 != MC_S_OK)
			result2 = MC_S_FALSE;
	}
	else
		BShaders = NULL;

	for (i = 0 ; i<=4 ; i++)			// check imp outs here as it only changes when the shader does
	{
		if (UShaders[i] != NULL)
			UImp_Out[i] = UShaders[i]->GetImplementedOutput();
		else
			UImp_Out[i] = 0;
		if (VShaders[i] != NULL)
			VImp_Out[i] = VShaders[i]->GetImplementedOutput();
		else
			VImp_Out[i] = 0;
	}
	if (WShaders != NULL)
		WShader_Imp = WShaders->GetImplementedOutput();
	else
		WShader_Imp = 0;

	if (EShaders != NULL)
		EShader_Imp = EShaders->GetImplementedOutput(); 
	else
		EShader_Imp = 0;

	if (BShaders != NULL)
		BShader_Imp = BShaders->GetImplementedOutput(); 
	else
		BShader_Imp = 0;

// Set up horizontal ranges

	total_width = 0;
	U_threads.quant = 0;
	V_threads.quant = 0;

	for (i = 0 ; i<=9 ; i++)			// zero out u_range
		U_threads.range[i] = 0;

	for (i = 0 ; i<=4 ; i++)			// Count U threads, break out when you hit a null.
		if (UShaders[i] != NULL) U_threads.quant++; else i = 5;

	for (i = 0 ; i<=9 ; i++)			// zero out v_range
		V_threads.range[i] = 0;

	for (i = 0 ; i<=4 ; i++)			// Count vertical threads, break out when you hit a null.
		if (VShaders[i] != NULL) V_threads.quant++; else i = 5;

	if (Data.Mode == kBasic_mode)		// if in Basic mode, fake it out and only use the 1st shaders.
	{
		UImp_Out[1] = UImp_Out[0];
		VImp_Out[1] = VImp_Out[0];
		U_threads.quant = 2;
		V_threads.quant = 2;
		Data.UWidths[1] = Data.UWidths[0];
		Data.VWidths[1] = Data.VWidths[0];
		for (int32 i = 0 ; i<=1 ; i++)
			for (int32 j = 0 ; j<=1 ; j++)
				Data.OnTop[i][j] = ((i+j) % 2) ? 1:0;
	}


	if (U_threads.quant != 0)						// there are some U threads. . . 
	{
		U_threads.range[0] = Data.U_Blank_Width/2.0;
		total_width = U_threads.range[0];
		for (i = 1 ; i<= U_threads.quant ; i++ )
		{
			U_threads.range[(i*2)-1] = total_width + Data.UWidths[i-1];
			total_width += (Data.UWidths[i-1] + Data.U_Blank_Width);
			U_threads.range[(i*2)] = total_width;
		}
		total_width -= Data.U_Blank_Width/2.0;
		for (i = 0 ; i<=9 ; i++)			// get into 0-1 range
			U_threads.range[i] /= total_width;

		for (i = 0 ; i<U_threads.quant ; i++)			// get into 0-1 range
		{
			U_threads.half[i] = (U_threads.range[(i*2)+1] - U_threads.range[i*2])/2.0;
			U_threads.middle[i] = U_threads.range[i*2] + U_threads.half[i];
		}
	}

// now do the verticals. . .
	total_width = 0;



	if (V_threads.quant != 0)						// there are some V threads. . . 
	{
		V_threads.range[0] = Data.V_Blank_Width/2.0;
		total_width = V_threads.range[0];
		for (i = 1 ; i<= V_threads.quant ; i++ )
		{
			V_threads.range[(i*2)-1] = total_width + Data.VWidths[i-1];
			total_width += (Data.VWidths[i-1] + Data.V_Blank_Width);
			V_threads.range[(i*2)] = total_width;
		}
		total_width -= Data.V_Blank_Width/2.0;
		for (i = 0 ; i<=9 ; i++)			// get into 0-1 range
			V_threads.range[i] /= total_width;

		for (i = 0 ; i<V_threads.quant ; i++)			// get into 0-1 range
		{
			V_threads.half[i] = (V_threads.range[(i*2)+1] - V_threads.range[i*2])/2.0;
			V_threads.middle[i] = V_threads.range[i*2] + V_threads.half[i];
		}
	}

/* Along the thread calculations require that we know how many segments there are, and where their
endpoints are. Due to OnTop, this can vary from row to row, so we need to build a structure that records
what is going on, row by row, from each direction.
*/
// first, along the U bands
	int32 seg_count;
	for (i=0; i<U_threads.quant; i++)	// vband number
	{
		seg_count = 0;		// if seg count still = 0 at the end of the loop, special case; this band alway on top
		U_bands[i].Segs[0].seg_begin = 0;
		U_bands[i].Seg_quant = 0;

		for (j=0; j<V_threads.quant; j++)		// walk along the crossovers
		{
			if (!Data.OnTop[j][i])		// V shader crosses over
			{
				U_bands[i].Segs[seg_count].seg_end = V_threads.range[(j*2)];
				U_bands[i].Segs[seg_count].seg_half = 
								(U_bands[i].Segs[seg_count].seg_end - U_bands[i].Segs[seg_count].seg_begin)/2;
				U_bands[i].Segs[seg_count].seg_mid = U_bands[i].Segs[seg_count].seg_begin + U_bands[i].Segs[seg_count].seg_half;
				U_bands[i].Segs[++seg_count].seg_begin = V_threads.range[(j*2)+1];
				U_bands[i].Seg_quant++;
			}
		}
		// need to handle end of last segment.
		U_bands[i].Segs[seg_count].seg_end = 1;
		U_bands[i].Segs[seg_count].seg_half = (1 - U_bands[i].Segs[seg_count].seg_begin)/2;
		U_bands[i].Segs[seg_count].seg_mid = U_bands[i].Segs[seg_count].seg_begin + U_bands[i].Segs[seg_count].seg_half;
	}

// then along the V bands.
	for (i=0; i<V_threads.quant; i++)	// uband number
	{
		seg_count = 0;		// if seg count still = 0 at the end of the loop, special case; this band alway on top.
		V_bands[i].Segs[0].seg_begin = 0;
		V_bands[i].Seg_quant = 0;

		for (j=0; j<U_threads.quant; j++)		// walk along the crossovers
		{
			if (Data.OnTop[i][j])		// U shader crosses over
			{
				V_bands[i].Segs[seg_count].seg_end = U_threads.range[(j*2)];
				V_bands[i].Segs[seg_count].seg_half = 
								(V_bands[i].Segs[seg_count].seg_end - V_bands[i].Segs[seg_count].seg_begin)/2;
				V_bands[i].Segs[seg_count].seg_mid = V_bands[i].Segs[seg_count].seg_begin + V_bands[i].Segs[seg_count].seg_half;
				V_bands[i].Segs[++seg_count].seg_begin = U_threads.range[(j*2)+1];
				V_bands[i].Seg_quant++;
			}
		}
		// need to handle end of last segment.
		V_bands[i].Segs[seg_count].seg_end = 1;
		V_bands[i].Segs[seg_count].seg_half = (1 - V_bands[i].Segs[seg_count].seg_begin)/2;
		V_bands[i].Segs[seg_count].seg_mid = V_bands[i].Segs[seg_count].seg_begin + V_bands[i].Segs[seg_count].seg_half;
	}

	one_over_U = 1.0/Data.Tiles_U;
	one_over_V = 1.0/Data.Tiles_V;

	Across_intensity = Data.Across_intensity;
	Along_intensity = Data.Along_intensity;

	return result2;
}


#if (VERSIONNUMBER == 0x010000)
MCCOMErr Weave::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Weave::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Weave::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x040000)
real Weave::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#endif


{
	result.R = 0;
	result.G = 0;
	result.B = 0;

	hold = result;

	fullArea = false;

	if ((V_threads.quant == 0) & (U_threads.quant == 0))  
		#if (VERSIONNUMBER == 0x010000)
		return MC_S_OK;
		#elif (VERSIONNUMBER == 0x020000)
		return MC_S_OK;
		#elif (VERSIONNUMBER >= 0x030000)
		return 1.0f;
		#endif  // no threads, just return black.
	else
	{

		real32 luma = 0;	// temp variable for getting value implemented.

		current_u = shadingIn.fUV[0];
		current_v = shadingIn.fUV[1];

		DoWiggle(fullArea, shadingIn);		// large scale randomizer

		// adjust for tiling
		if ( current_u != 0)
			current_u = fmodf(current_u, one_over_U)* Data.Tiles_U;
		if (current_v != 0)
			current_v = fmodf(current_v, one_over_V) * Data.Tiles_V;

		DoEdge(fullArea, shadingIn);		// thread edge randomizer	

		// Horizontal picker
		Threadpicker(Return_values, U_threads, current_u);

		uu_distance = Return_values.distance;
		uu_half = Return_values.half;
		u_shader = Return_values.shader;
		uu_percent = Return_values.percent;

		// Vertical picker
		Threadpicker(Return_values, V_threads, current_v);

		vv_distance = Return_values.distance;
		vv_half = Return_values.half;
		v_shader = Return_values.shader;
		vv_percent = Return_values.percent;

		uv_distance = 0;
		vu_distance = 0;
		uv_half = 1;
		vu_half = 1;
		uv_percent = 1;
		vu_percent = 1;
		if (Data.ThreeD_along | Data.Comp_Along)
		{
			// U thread along values
			Alongthread(Return_values, u_shader, current_v, U_bands);

			uv_distance = Return_values.distance;
			uv_half = Return_values.half;
			uv_percent = Return_values.percent;

			// V thread along values
			Alongthread(Return_values, v_shader, current_u, V_bands);

			vu_distance = Return_values.distance;
			vu_half = Return_values.half;
			vu_percent = Return_values.percent;
		}

		// ok, now decide who is on top!
		if ((v_shader + u_shader) == 0)	// double gap- show the background.
			winner = 0;					
		else if (v_shader == 0)			// threads always trump background
		{
			winner = u_shader;
			u_wins = 1;
		}
		else if (u_shader == 0)
		{
			winner = v_shader;
			u_wins = 0;
		}
		else if (Data.OnTop[v_shader-1][u_shader-1])
		{
			winner = u_shader;
			u_wins = 1;
		}
		else
		{
			winner = v_shader;
			u_wins = 0;
		}

		if (winner == 0)
		{
			if (Data.Mask)
			{
				result.R = Data.Inv_Mask;
				result.G = Data.Inv_Mask;
				result.B = Data.Inv_Mask;
			}
			else
				if (BShader_Imp & (0x2 | 0x10 | 0x4))		// get color implemented
				{
					BShaders->GetColor(Color, fullArea, shadingIn);
					result.R = Color.R;
					result.G = Color.G;
					result.B = Color.B;
					#if (VERSIONNUMBER >= 0x040000)
					result.alpha = Color.alpha;
					#endif
				}
				else if (BShader_Imp & 0x1)  //  or get value implemented
				{
					BShaders->GetValue(luma, fullArea, shadingIn);
					result.R = luma;
					result.G = luma;
					result.B = luma;
				}
		}
		else if (u_wins)
		{
			if (Data.Mask)
			{
				result.R = !Data.Inv_Mask;
				result.G = !Data.Inv_Mask;
				result.B = !Data.Inv_Mask;
			}
			else
			{
				if (Data.Comp_Across)
					shadingIn.fUV[0] = uu_percent;
				if (Data.Comp_Along)
					shadingIn.fUV[1] = uv_percent;
				//if (UImp_Out[winner-1] & kUsesGetValue)  // get value implemented
				//{
				//	if (Data.Mode == kBasic_mode)
				//		UShaders[0]->GetColor(Color, fullArea, shadingIn);
				//	else
				//		UShaders[winner-1]->GetValue(luma, fullArea, shadingIn);
				//	result.R = luma;
				//	result.G = luma;
				//	result.B = luma;
				//}else 
				if (UImp_Out[winner-1])// & kUsesGetColor)		// or get color implemented
				{
					if (Data.Mode == kBasic_mode)
						UShaders[0]->GetColor(Color, fullArea, shadingIn);
					else
						UShaders[winner-1]->GetColor(Color, fullArea, shadingIn);
					result.R = Color.R;
					result.G = Color.G;
					result.B = Color.B;
					#if (VERSIONNUMBER >= 0x040000)
					result.alpha = Color.alpha;
					#endif
				}
				 
				if (Data.ThreeD_across | Data.ThreeD_along)				// do percent calcs for shading
				{ 
					real32 factor;
					// (x^2/A^2 + y^2/B^2)
					factor = 1.0 - (powf(uv_distance , 2.0) / powf(uv_half * Along_intensity , 2.0) + 
									powf(uu_distance , 2.0) / powf(uu_half * Across_intensity , 2.0));				
					result.R *= factor;
					result.G *= factor;
					result.B *= factor;
				}
			}
		}
		else		// v wins
		{

			if (Data.Mask)
			{
				result.R = !Data.Inv_Mask;
				result.G = !Data.Inv_Mask;
				result.B = !Data.Inv_Mask;
			}
			else
			{
				real32 temp = shadingIn.fUV[0];
				shadingIn.fUV[0] = shadingIn.fUV[1];
				shadingIn.fUV[1] = temp;
			
				if (Data.Comp_Across)
					shadingIn.fUV[0] = vv_percent;// swapped
				if (Data.Comp_Along)
					shadingIn.fUV[1] = vu_percent;
				//if (VImp_Out[winner-1] & kUsesGetValue)  //  get value implemented
				//{
				//	if (Data.Mode == kBasic_mode)
				//		VShaders[0]->GetColor(Color, fullArea, shadingIn);
				//	else
				//		VShaders[winner-1]->GetValue(luma, fullArea, shadingIn);
				//	result.R = luma;
				//	result.G = luma;
				//	result.B = luma;
				//}else 
				if (VImp_Out[winner-1])		// or get color implemented
				{
					if (Data.Mode == kBasic_mode)
						VShaders[0]->GetColor(Color, fullArea, shadingIn);
					else
						VShaders[winner-1]->GetColor(Color, fullArea, shadingIn);
					result.R = Color.R;
					result.G = Color.G;
					result.B = Color.B;
					#if (VERSIONNUMBER >= 0x040000)
					result.alpha = Color.alpha;
					#endif
				}
				 
				if (Data.ThreeD_across | Data.ThreeD_along)			// do percent calcs for shading
				{
					real32 factor;
					// (x^2/A^2 + y^2/B^2)
					factor = 1.0 - (powf(vu_distance , 2.0) / powf(vu_half * Along_intensity , 2.0) + 
									powf(vv_distance , 2.0) / powf(vv_half * Across_intensity , 2.0));
					result.R *= factor;
					result.G *= factor;
					result.B *= factor;
				}
			}
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


#if (VERSIONNUMBER <= 0x020000)
MCCOMErr Weave::GetVector(TVector3& result,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x030000)
real Weave::GetVector(TVector3& result,ShadingIn& shadingIn)
#endif

{
	boolean fullArea;

	fullArea = false;

	current_u = shadingIn.fUV[0];
	current_v = shadingIn.fUV[1];

	DoWiggle( fullArea, shadingIn);

		// adjust for tiling
	if ( current_u != 0)
		current_u = fmodf(current_u, one_over_U)* Data.Tiles_U;
	if (current_v != 0)
		current_v = fmodf(current_v, one_over_V) * Data.Tiles_V;

	DoEdge(fullArea, shadingIn);

			// Horizontal picker
	Threadpicker(Return_values, U_threads, current_u);

	uu_distance = Return_values.distance;
	uu_half = Return_values.half;
	u_shader = Return_values.shader;

	// Vertical picker
	Threadpicker(Return_values, V_threads, current_v);

	vv_distance = Return_values.distance;
	vv_half = Return_values.half;
	v_shader = Return_values.shader;

	uv_distance = 0;
	vu_distance = 0;
	uv_half = 1;
	vu_half = 1;
		
	// U thread along values
	Alongthread(Return_values, u_shader, current_v, U_bands);

	uv_distance = Return_values.distance;
	uv_half = Return_values.half;

	// V thread along values
	Alongthread(Return_values, v_shader, current_u, V_bands);

	vu_distance = Return_values.distance;
	vu_half = Return_values.half;

	// ok, now decide who is on top!
	if ((v_shader + u_shader) == 0)	// double gap- show the background. No bump change; we're done.
	{
		result = shadingIn.fNormalLoc;
		#if (VERSIONNUMBER == 0x010000)
		return MC_S_OK;
		#elif (VERSIONNUMBER == 0x020000)
		return MC_S_OK;
		#elif (VERSIONNUMBER >= 0x030000)
		return 1.0f;
		#endif
	}
	else if (v_shader == 0)			// threads always trump background
	{
		winner = u_shader;
		u_wins = 1;
	}
	else if (u_shader == 0)
	{
		winner = v_shader;
		u_wins = 0;
	}
	else if (Data.OnTop[v_shader-1][u_shader-1])
	{
		winner = u_shader;
		u_wins = 1;
	}
	else
	{
		winner = v_shader;
		u_wins = 0;
	}

	TVector3 IsoU;
	TVector3 IsoV;

	IsoU = shadingIn.fIsoU;
	IsoV = shadingIn.fIsoV;

	if (u_wins)
	{
		if (Data.ThreeD_across & Data.ThreeD_along)
		{
			result = VectorRotate(IsoU * (1/Along_intensity)*2, shadingIn.fGNormal, uv_distance);	
			result.Normalize();
			result = VectorRotate(IsoV * (1/Across_intensity)*2, result, -uu_distance);	
			#if (VERSIONNUMBER == 0x010000)
			return MC_S_OK;
			#elif (VERSIONNUMBER == 0x020000)
			return MC_S_OK;
			#elif (VERSIONNUMBER >= 0x030000)
			return 1.0f;
			#endif
		}
		if (Data.ThreeD_across)									// do percent calcs for shading
		{
			result = VectorRotate(IsoV * (1/Across_intensity)*2, shadingIn.fGNormal, -uu_distance);
			#if (VERSIONNUMBER == 0x010000)
			return MC_S_OK;
			#elif (VERSIONNUMBER == 0x020000)
			return MC_S_OK;
			#elif (VERSIONNUMBER >= 0x030000)
			return 1.0f;
			#endif
		}
		if (Data.ThreeD_along)
		{
			result = VectorRotate(IsoU * (1/Along_intensity)*2, shadingIn.fGNormal, uv_distance);	
			#if (VERSIONNUMBER == 0x010000)
			return MC_S_OK;
			#elif (VERSIONNUMBER == 0x020000)
			return MC_S_OK;
			#elif (VERSIONNUMBER >= 0x030000)
			return 1.0f;
			#endif
		}
	}
	else		// v wins
	{
		if (Data.ThreeD_across & Data.ThreeD_along)
		{
			result = VectorRotate(IsoV * (1/Along_intensity)*2, shadingIn.fGNormal, -vu_distance);						
			result.Normalize();
			result = VectorRotate(IsoU * (1/Across_intensity)*2, result, vv_distance);	
			#if (VERSIONNUMBER == 0x010000)
			return MC_S_OK;
			#elif (VERSIONNUMBER == 0x020000)
			return MC_S_OK;
			#elif (VERSIONNUMBER >= 0x030000)
			return 1.0f;
			#endif
		}
		if (Data.ThreeD_across)									// do percent calcs for shading
		{
			result = VectorRotate(IsoU * (1/Across_intensity)*2, shadingIn.fGNormal, vv_distance);	
			#if (VERSIONNUMBER == 0x010000)
			return MC_S_OK;
			#elif (VERSIONNUMBER == 0x020000)
			return MC_S_OK;
			#elif (VERSIONNUMBER >= 0x030000)
			return 1.0f;
			#endif
		}
		if (Data.ThreeD_along)
		{
			result = VectorRotate(IsoV * (1/Along_intensity)*2, shadingIn.fGNormal, -vu_distance);	
			#if (VERSIONNUMBER == 0x010000)
			return MC_S_OK;
			#elif (VERSIONNUMBER == 0x020000)
			return MC_S_OK;
			#elif (VERSIONNUMBER >= 0x030000)
			return 1.0f;
			#endif
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


// Subroutines
boolean Weave::DoWiggle (boolean& fullArea, ShadingIn& shadingIn)  	// big randomizer- wiggle creates a distortion of the threads
// Modifies current_v and current_u
{
	real32 wiggle = 0;
	real32 luma = 0;

	if (Data.Wiggle_Intensity != 0)
	{
		if (WShader_Imp & (0x2 | 0x10 | 0x4))		// get color implemented
		{
			WShaders->GetColor(Color, fullArea, shadingIn);
			luma = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11);
			wiggle = luma * Data.Wiggle_Intensity/100;
		}
		else if (WShader_Imp & 0x1)  //  or get value implemented
		{
			WShaders->GetValue(luma, fullArea, shadingIn);
			wiggle = luma * Data.Wiggle_Intensity/100;
		}
		current_v += wiggle;
		current_u += wiggle;
	}

return (1);
}

boolean Weave::DoEdge(boolean& fullArea, ShadingIn& shadingIn)  	//thread edge randomizer. . . 
// Modifies current_v and current_u
{
	real32 wiggle = 0;
	real32 luma = 0;

	if (Data.Edge_Intensity != 0)
	{
		if (EShader_Imp & (0x2 | 0x10 | 0x4))		// get color implemented
		{
			EShaders->GetColor(Color, fullArea, shadingIn);
			luma = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11);
			wiggle = luma * Data.Edge_Intensity/100;
		}
		else if (EShader_Imp & 0x1)  //  or get value implemented
		{
			EShaders->GetValue(luma, fullArea, shadingIn);
			wiggle = luma * Data.Edge_Intensity/100;
		}
		current_v += wiggle;
		current_u += wiggle;
	}
	return (1);
}

boolean Weave::Threadpicker(Loc_Data &Return_values, Threads &in_Thread, real32 coordinate)
{	
	int32 i = 0;
	real32 lower_bound = 0;
	boolean still_looking = (in_Thread.quant != 0 );
	Return_values.shader = 0;
	Return_values.distance = 0;
	Return_values.half = 1;
	Return_values.percent = 0;
	while (still_looking)											
	{
		if ((coordinate >=lower_bound) & (coordinate < in_Thread.range[i*2]))		// blank
			still_looking = 0;															// point is found!
		else if ((coordinate >= in_Thread.range[i*2]) & (coordinate < in_Thread.range[(i*2)+1]))	// stripe
		{
			Return_values.shader = i+1;							// shaders run 1-5
			still_looking = 0;											// point is found!
			if (Data.ThreeD_across)							// do calcs for shading only if required
				if (coordinate > in_Thread.middle[i])
					Return_values.distance = -1.0 * (coordinate - in_Thread.middle[i]);
				else if (coordinate < in_Thread.middle[i])
					Return_values.distance = (in_Thread.middle[i] - coordinate);
			Return_values.half = in_Thread.half[i];
			Return_values.percent = (coordinate - in_Thread.range[i*2]) / 
									(in_Thread.range[(i*2)+1] - in_Thread.range[i*2]);
		}
		else
		{
			if (i == 5)				// last bit of blank
			{
				Return_values.shader = 0;	
				still_looking = 0;												// point is found!
			}
			else
			{
				lower_bound = in_Thread.range[(i*2)+1];
				i++;
			}
		}
	}
return (1);
}

boolean Weave::Alongthread(Loc_Data &Return_values, int32 shader, real32 coordinate, Bands *in_band)
{
	int32 i = 0;
	real32 temp_mid;
	Return_values.distance = 0;
	Return_values.half = 1;
	Return_values.percent = 0;

	boolean still_looking = ((shader != 0) & (in_band[shader-1].Seg_quant != 0));
	while (still_looking)
		if (( in_band[shader-1].Segs[i].seg_begin <= coordinate) & (in_band[shader-1].Segs[i].seg_end > coordinate))
		{
			still_looking = 0;										// point is in this segment
			// do calculations
			if ((i != 0) & (i != in_band[shader-1].Seg_quant))  // in a bounded segment
			{
				if (coordinate > in_band[shader-1].Segs[i].seg_mid)
					Return_values.distance = -1.0*(coordinate - in_band[shader-1].Segs[i].seg_mid);
				else if (coordinate < in_band[shader-1].Segs[i].seg_mid)
					Return_values.distance = (in_band[shader-1].Segs[i].seg_mid - coordinate);
				else
					Return_values.distance = 0;
				Return_values.half = in_band[shader-1].Segs[i].seg_half;
				Return_values.percent = (coordinate - in_band[shader-1].Segs[i].seg_begin) / 
											(in_band[shader-1].Segs[i].seg_half * 2.0);
			}
			else if (i == 0)			// in the first section- need to wrap around to end segment.
			{
				temp_mid = in_band[shader-1].Segs[0].seg_mid - 
					in_band[shader-1].Segs[in_band[shader-1].Seg_quant].seg_half;	
				if (coordinate > temp_mid)
					Return_values.distance = -1.0*(coordinate - temp_mid);
				else if (coordinate < temp_mid)
					Return_values.distance = (temp_mid - coordinate);
				else
					Return_values.distance = 0;
				Return_values.half = (in_band[shader-1].Segs[0].seg_half + 
					in_band[shader-1].Segs[in_band[shader-1].Seg_quant].seg_half);
				Return_values.percent = (coordinate + in_band[shader-1].Segs[in_band[shader-1].Seg_quant].seg_half * 2.0 ) / 
						((in_band[shader-1].Segs[0].seg_half + 
						in_band[shader-1].Segs[in_band[shader-1].Seg_quant].seg_half) * 2.0);
			}
			else   // in last section, need to wrap to first section
			{
				temp_mid = in_band[shader-1].Segs[i].seg_mid + in_band[shader-1].Segs[0].seg_half;
				if (coordinate > temp_mid)
					Return_values.distance = -1.0 *(coordinate - temp_mid);
				else if (coordinate < temp_mid)
					Return_values.distance = (temp_mid - coordinate);
				else
					Return_values.distance = 0;
				Return_values.half = (in_band[shader-1].Segs[0].seg_half + 
					in_band[shader-1].Segs[in_band[shader-1].Seg_quant].seg_half);
				Return_values.percent = (coordinate - in_band[shader-1].Segs[i].seg_begin) / 
							((in_band[shader-1].Segs[0].seg_half + in_band[shader-1].Segs[i].seg_half) * 2.0);
			}
		}
		else
			if (i < in_band[shader-1].Seg_quant) i++;
			else  still_looking = 0;

	return (1);
}



TVector3 Weave::VectorRotate(TVector3 shaft, TVector3 vector, real32 asin_angle)
{
	real32 cos_rot = cosf(asinf(asin_angle));	
	real32 sin_rot = asin_angle;
	real32 t = 1.0 - cos_rot;
	TVector3 return_vec;


	return_vec[0] = 0;
	return_vec[1] = 0;
	return_vec[2] = 0;
			
	return_vec[0] += (cos_rot + t * shaft[0] * shaft[0]) * vector[0];
	return_vec[0] += (t * shaft[0] * shaft[1] - shaft[2] * sin_rot) * vector[1];
	return_vec[0] += (t * shaft[0] * shaft[2] + shaft[1] * sin_rot) * vector[2];

	return_vec[1] += (t * shaft[0] * shaft[1] + shaft[2] * sin_rot) * vector[0];
	return_vec[1] += (cos_rot + t * shaft[1] * shaft[1]) * vector[1];
	return_vec[1] += (t * shaft[1] * shaft[2] - shaft[0] * sin_rot) * vector[2];

	return_vec[2] += (t * shaft[0] * shaft[2] - shaft[1] * sin_rot) * vector[0];
	return_vec[2] += (t * shaft[1] * shaft[2] + shaft[0] * sin_rot) * vector[1];
	return_vec[2] += (cos_rot + t * shaft[2] * shaft[2]) * vector[2];

	return return_vec;
}

// end of Weave