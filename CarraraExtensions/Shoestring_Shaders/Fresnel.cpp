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
#include "Fresnel.h"
#include "copyright.h"
#include "Shoestringshaders.h"
#include "I3DRenderingModule.h"
#include "IMFPart.h"
#include "IMFSliderPart.h"
#include "PMapTypes.h"
#include "MFAttributes.h"
#include <float.h>

		const Medium_Preset Medium_Preset_data[Num_Medium_Presets]={
	    //     						nr			ni			mur			disp	  //**** = Value Certain, ? Value uncertain .
		{/* Water */				1.33283	,	0.00001	,	1.0		,	0.01794	},//****
		{/*Air 	*/					1.00027	,  	0.0    	,	1.0		,	0.01119 },//****
		{/*Glass	*/				1.51673	,  	0.00001	,	1.0		,	0.0089 	},//****
		{/*Diamond	*/				2.414	,  	0.00001	,	0.999979,	0.044  	},//****
		{/*Lead Crystal	*/			1.805	,  	0.00001	,	1.0		,	0.032   },//****
		{/*Quartz	*/				1.56	,  	0.00001	,	1.0		,	0.02   	},//****
		{/*Plastic (Polycarb') */	1.58523	,  	0.00001	,	1.0		,	0.025776},//****
		{/*Silver.	*/				0.121  	,  	3.5		,	0.999974,	0.01136 },//****
		{/*Gold	*/					0.25	,  	2.84	,	0.999962,  -1.08 	},//****
		{/*Aluminium	*/			0.93	,  	5.85	,	1.000022,   10.0 	},//****
		{/*Copper	*/				0.46818	,  	2.80877	,	0.99999	,  	0.95525	},//****
		{/*Bronze	*/				1.18	,  	2.5		,	1.0		,  	0.8    	},//*???
		{/*Iron	*/					2.93	,  	2.0		,	300.0	,  -0.1088 	},//*???
		{/*Ferrite	*/				1.51673	,  	2.0		,	2000.0	,  -1.0   	},//*???
	    {/*Alcohol	*/				1.36	,  	0.00001	,	1.0		,  	0.02   	},//*???
	    {/*Amber	*/				1.54	,	0.00001	,	1.0		,  	0.09   	},//????
	    {/*Amethyst	*/				1.554	,	0.05	,	0.94	,  	0.013  	},//*???
	    {/*Emerald	*/				1.605	,	0.04	,	1.0		,  	0.044   },//*???
	    {/*Ice	*/					1.309	,	0.1		,	1.0		,  	0.04   	},//*???
	    {/*Ruby	*/					1.779	,	0.4		,	1.0		,  	0.044  	},//*???
	    {/*Salt	*/					1.516	,	0.1		,	1.0		,  	0.04   	},//*???
	    {/*Topaz	*/				1.62	,	0.1		,	0.94	,  	0.014  	},//*???
		{/*Mylar	*/				1.65	,	3.5		,	0.999974, 	0.02   	},//*???
		{/*Nickel	*/				1.39    ,	7.1		,	4.000022,  -1.182 	},//****
		{/*Platinum	*/				2.33	,	3.5		,	0.999974, 	0.02   	},//*???
		{/*Titanium	*/				2.16    ,	7.1		,	1.000022,  -1.182 	},//****
		{/*Plexiglass 	*/			1.488   ,	1.0		,	1.000022,  	0.08 	},//????
		{/*Obsidian	*/				1.5    	,	2.5		,	1.000022,  	0.8	 	},//????
		{/*Sea Water	*/			1.37    ,	0.00001	,	1.0000,		0.04	},//????
		{/*Fresh Water	*/			1.34    ,	0.00001	,	1.0000,		0.02	},//????
		{/*Dirty Water	*/			1.34    ,	0.00001	,	1.0000,		0.01	} //????
		};
		

MCCOMErr Fresnel::ExtensionDataChanged()
{
	if  (   fabs(FresnelPublicData.n1r              -   FresnelShadowData.n1r             )>=nsdel|| 
			fabs(FresnelPublicData.n1i              -   FresnelShadowData.n1i             )>=nsdel||
			fabs(FresnelPublicData.n2r              -   FresnelShadowData.n2r             )>=nsdel||
			fabs(FresnelPublicData.n2i              -   FresnelShadowData.n2i             )>=nsdel||
			fabs(FresnelPublicData.mu1r             -   FresnelShadowData.mu1r            )>=nsdel||
			fabs(FresnelPublicData.mu2r             -   FresnelShadowData.mu2r            )>=nsdel||
			fabs(FresnelPublicData.Pper             -   FresnelShadowData.Pper            )>=nsdel||
			FresnelPublicData.TRMode              !=   FresnelShadowData.TRMode                  ||
			fabs(FresnelPublicData.n2rslider        -   FresnelShadowData.n2rslider       )>=nsdel||
			fabs(FresnelPublicData.n2islider        -   FresnelShadowData.n2islider       )>=nsdel||
			fabs(FresnelPublicData.mu2rslider       -   FresnelShadowData.mu2rslider      )>=nsdel||
			fabs(FresnelPublicData.lambdaslider     -   FresnelShadowData.lambdaslider    )>=lsdel||
			fabs(FresnelPublicData.disp             -   FresnelShadowData.disp            )>=nsdel||
			fabs(FresnelPublicData.n2rd             -   FresnelShadowData.n2rd            )>=nsdel||
			fabs(FresnelPublicData.n2id             -   FresnelShadowData.n2id            )>=nsdel||
			FresnelPublicData.Preset_n1           !=   FresnelShadowData.Preset_n1               ||
			FresnelPublicData.Preset_n2           !=   FresnelShadowData.Preset_n2               ||     
			FresnelPublicData.Preset_illum        !=   FresnelShadowData.Preset_illum            ||
			FresnelPublicData.Color_Object        !=   FresnelShadowData.Color_Object            ||          
     		fabs(FresnelPublicData.illuminant       -   FresnelShadowData.illuminant      )>de
	)
	{
	if (  (FresnelPublicData.illuminant != FresnelShadowData.illuminant) & (FresnelPublicData.Preset_illum == FresnelShadowData.Preset_illum))
		{	FresnelPublicData.Preset_illum=Other_Illuminant;
		 	FresnelPrivateData.BlackBody=true;
	   		real32 X;real32 Y;real32 Z;real32 R;real32 G; real32 B;
	    	FresnelPrivateData.Color_System=whitepoint (FresnelPrivateData.Monitor_ColorSystem, FresnelPublicData.illuminant);
	    	XYZColor WhitePointMon,WhitePoint;
	    	XYZReferenceWhite RefMon,Ref;
	    	FresnelPublicData.xWhite=FresnelPrivateData.Color_System.xWhite;
	    	FresnelPublicData.yWhite=FresnelPrivateData.Color_System.yWhite;
	    	X=FresnelPrivateData.Color_System.xWhite*FresnelPrivateData.Color_System.YWhite/FresnelPrivateData.Color_System.yWhite;
	    	Y=FresnelPrivateData.Color_System.YWhite;
	    	Z=(( 1.0 - FresnelPrivateData.Color_System.xWhite - FresnelPrivateData.Color_System.yWhite ) * FresnelPrivateData.Color_System.YWhite)/FresnelPrivateData.Color_System.yWhite;
			Ref.Xr=X;
			Ref.Yr=Y;
			Ref.Zr=Z;
	    	RefMon.Xr=FresnelPrivateData.Monitor_ColorSystem.xWhite*FresnelPrivateData.Monitor_ColorSystem.YWhite/FresnelPrivateData.Monitor_ColorSystem.yWhite;;
	    	RefMon.Yr=FresnelPrivateData.Monitor_ColorSystem.YWhite;;
	    	RefMon.Zr=(( 1.0 - FresnelPrivateData.Monitor_ColorSystem.xWhite - FresnelPrivateData.Monitor_ColorSystem.yWhite ) * FresnelPrivateData.Monitor_ColorSystem.YWhite)/FresnelPrivateData.Monitor_ColorSystem.yWhite;;
	    	WhitePoint.X=X;
	    	WhitePoint.Y=Y;
	    	WhitePoint.Z=Z;
	    	WhitePointMon.X=RefMon.Xr;
	    	WhitePointMon.Y=RefMon.Yr;
	    	WhitePointMon.Z=RefMon.Zr;
	    	FresnelPrivateData.WhitePoint=XYZ2Lab(WhitePoint,RefMon);
	 		XYZSpectralColor Color;
	 		Color.SX=WhitePoint.X;
	 		Color.SY=WhitePoint.Y;
	 		Color.SZ=WhitePoint.Z;
			FresnelPrivateData.SolutionFound=false;
	 		FresnelPrivateData.partitioncount=0;
	 		Color.SX=Color.SX/Color.SY;Color.SY=Color.SY/Color.SY;Color.SZ=Color.SZ/Color.SY;
			
		 	FresnelPrivateData.disableKGR=true; 
			LabParameter W;
			W.Plane=KRB; 
			W.u=0;W.v=0;
			R=0;
			G=0;
			B=0;
	 		W=Lab_Gamut_Project ( FresnelPrivateData.Monitor_ColorSystem,Color ,0.5,FresnelPrivateData.WhitePoint.a,FresnelPrivateData.WhitePoint.b,7);
	 		FresnelPrivateData.disableKGR=false; 

			if ( W.Plane==KRB ) {R=W.u;B=W.v;G=0.0;};if ( W.Plane==KBG ) {B=W.u;G=W.v;R=0.0;};if ( W.Plane==KGR ) {G=W.u;R=W.v;B=0.0;};
			if ( W.Plane==GCY ) {G=1.0;B=W.u;R=W.v;};if ( W.Plane==RYM ) {R=1.0;G=W.u;B=W.v;};if ( W.Plane==BMC ) {B=1.0;R=W.u;G=W.v;};
			FresnelPrivateData.WhitePointRGB.R=R;
			FresnelPrivateData.WhitePointRGB.G=G;
			FresnelPrivateData.WhitePointRGB.B=B;
	 		gamma_correct_rgb (FresnelPrivateData.Monitor_ColorSystem, &R, &G, &B);
	 		FresnelPublicData.illuminant_Color.R=R;
	 		FresnelPublicData.illuminant_Color.G=G;
	 		FresnelPublicData.illuminant_Color.B=B;
	 		
		}	

	if  ( (FresnelPublicData.Preset_illum != FresnelShadowData.Preset_illum) & 
			(FresnelPublicData.Preset_illum >= 1) & 
			(FresnelPublicData.Preset_illum < Other_Illuminant) &
			(FresnelShadowData.Preset_illum >= 1) & 
			(FresnelShadowData.Preset_illum <= Other_Illuminant)
	    )
	    { 	FresnelPrivateData.BlackBody=false;
	   		real32 X;real32 Y;real32 Z;real32 R;real32 G; real32 B; 
	   		FresnelPrivateData.Color_System=FresnelPrivateData.Monitor_ColorSystem;
	    	FresnelPrivateData.Color_System=whitepoint (FresnelPrivateData.Monitor_ColorSystem, FresnelPrivateData.illum_Preset_data[FresnelPublicData.Preset_illum]);
	    	XYZColor WhitePointMon,WhitePoint;
	    	XYZReferenceWhite RefMon,Ref;
	    	FresnelPublicData.xWhite=FresnelPrivateData.Color_System.xWhite;
	    	FresnelPublicData.yWhite=FresnelPrivateData.Color_System.yWhite;
	    	X=FresnelPrivateData.Color_System.xWhite*FresnelPrivateData.Color_System.YWhite/FresnelPrivateData.Color_System.yWhite;
	    	Y=FresnelPrivateData.Color_System.YWhite;
	    	Z=(( 1.0 - FresnelPrivateData.Color_System.xWhite - FresnelPrivateData.Color_System.yWhite ) * FresnelPrivateData.Color_System.YWhite)/FresnelPrivateData.Color_System.yWhite;
			XYZtoCorColorTemp(X,Y,Z, &FresnelPublicData.illuminant);
			Ref.Xr=X;
			Ref.Yr=Y;
			Ref.Zr=Z;
	    	RefMon.Xr=FresnelPrivateData.Monitor_ColorSystem.xWhite*FresnelPrivateData.Monitor_ColorSystem.YWhite/FresnelPrivateData.Monitor_ColorSystem.yWhite;;
	    	RefMon.Yr=FresnelPrivateData.Monitor_ColorSystem.YWhite;;
	    	RefMon.Zr=(( 1.0 - FresnelPrivateData.Monitor_ColorSystem.xWhite - FresnelPrivateData.Monitor_ColorSystem.yWhite ) * FresnelPrivateData.Monitor_ColorSystem.YWhite)/FresnelPrivateData.Monitor_ColorSystem.yWhite;;
	    	WhitePoint.X=X;
	    	WhitePoint.Y=Y;
	    	WhitePoint.Z=Z;
	    	WhitePointMon.X=RefMon.Xr;
	    	WhitePointMon.Y=RefMon.Yr;
	    	WhitePointMon.Z=RefMon.Zr;
	    	FresnelPrivateData.WhitePoint=XYZ2Lab(WhitePoint,RefMon);
	 		XYZSpectralColor Color;
	 		Color.SX=WhitePoint.X;
	 		Color.SY=WhitePoint.Y;
	 		Color.SZ=WhitePoint.Z;
	 		FresnelPrivateData.SolutionFound=false;
	 		FresnelPrivateData.partitioncount=0;
	 		Color.SX=Color.SX/Color.SY;Color.SY=Color.SY/Color.SY;Color.SZ=Color.SZ/Color.SY;

		 	FresnelPrivateData.disableKGR=true; 
			LabParameter W;
			W.Plane=KRB; 
			W.u=0;W.v=0;
			R=0;
			G=0;
			B=0;
	 		W=Lab_Gamut_Project ( FresnelPrivateData.Monitor_ColorSystem,Color ,0.5,FresnelPrivateData.WhitePoint.a,FresnelPrivateData.WhitePoint.b,7);
	 		FresnelPrivateData.disableKGR=false; 

			if ( W.Plane==KRB ) {R=W.u;B=W.v;G=0.0;};if ( W.Plane==KBG ) {B=W.u;G=W.v;R=0.0;};if ( W.Plane==KGR ) {G=W.u;R=W.v;B=0.0;};
			if ( W.Plane==GCY ) {G=1.0;B=W.u;R=W.v;};if ( W.Plane==RYM ) {R=1.0;G=W.u;B=W.v;};if ( W.Plane==BMC ) {B=1.0;R=W.u;G=W.v;};

			FresnelPrivateData.WhitePointRGB.R=R;
			FresnelPrivateData.WhitePointRGB.G=G;
			FresnelPrivateData.WhitePointRGB.B=B;
	 		gamma_correct_rgb (FresnelPrivateData.Monitor_ColorSystem, &R, &G, &B);
	 		FresnelPublicData.illuminant_Color.R=R;
	 		FresnelPublicData.illuminant_Color.G=G;
	 		FresnelPublicData.illuminant_Color.B=B;
	 		
	    };
	if ( (fabs(FresnelPublicData.lambdaslider - FresnelShadowData.lambdaslider) >=0.001) || (FresnelPublicData.illuminant != FresnelShadowData.illuminant) || (FresnelPublicData.Color_Object != FresnelShadowData.Color_Object))
		{	real32 R; real32 G; real32 B;real32 GLf=FresnelPrivateData.GLf;real32 L,a,b;
	   		FresnelPrivateData.SolutionFound=false;
			if(FresnelPrivateData.BlackBody==true)
				{FresnelPrivateData.Color_System=whitepoint (FresnelPrivateData.Monitor_ColorSystem, FresnelPublicData.illuminant);
				 Calc_Spectral_Color ( FresnelPrivateData.Color_System,FresnelPrivateData.Monitor_ColorSystem, &R, &G,&B,&L,&a,&b,FresnelPublicData.lambdaslider, FresnelPublicData.illuminant,GLf,7);
				}
			else
				{FresnelPrivateData.Color_System=whitepoint (FresnelPrivateData.Monitor_ColorSystem, FresnelPrivateData.illum_Preset_data[FresnelPublicData.Preset_illum]);
				 Calc_Spectral_Color ( FresnelPrivateData.Color_System,FresnelPrivateData.Monitor_ColorSystem, &R, &G,&B,&L,&a,&b,FresnelPublicData.lambdaslider, FresnelPublicData.illuminant,GLf,7);
				}
			
			FresnelPublicData.Spectral_Red    =R;
			FresnelPublicData.Spectral_Green  =G;
			FresnelPublicData.Spectral_Blue   =B;
	 		gamma_correct_rgb (FresnelPrivateData.Monitor_ColorSystem, &R, &G, &B);
			FresnelPublicData.Spectral_Color.R=R;
	 		FresnelPublicData.Spectral_Color.G=G;
	 		FresnelPublicData.Spectral_Color.B=B;

		};



	//If a value is entered in FresnelShadowData.n2r  , set the delta slider FresnelShadowData.n2rslider back to zero.
	if (FresnelPublicData.n2r != FresnelShadowData.n2r  ) {
		FresnelPublicData.n2rslider  = 0.0; 
		FresnelPrivateData.n2r_internal = FresnelPublicData.n2r;
		};
	//n2rslider is a delta on FresnelShadowData.n2r  . 
	if(FresnelPublicData.n2rslider !=FresnelShadowData.n2rslider ) {
		FresnelPublicData.n2r = FresnelPrivateData.n2r_internal + FresnelPublicData.n2rslider ;
		};
	//If a value is entered in FresnelShadowData.n2i   , set the delta slider FresnelShadowData.n2islider back to zero. 
	if(FresnelPublicData.n2i !=FresnelShadowData.n2i   ) {
		FresnelPublicData.n2islider  =0.0;
		FresnelPrivateData.n2i_internal=FresnelPublicData.n2i;
		};
	//n2islider is a delta on FresnelShadowData.n2i   . 
	if(FresnelPublicData.n2islider !=FresnelShadowData.n2islider ) {
		FresnelPublicData.n2i = FresnelPrivateData.n2i_internal + FresnelPublicData.n2islider;
		};
	//If a value is entered in FresnelShadowData.n2i   , set the delta slider FresnelShadowData.n2islider back to zero. 
	if(FresnelPublicData.mu2r !=FresnelShadowData.mu2r) {
		FresnelPublicData.mu2rslider = 0.0;
		FresnelPrivateData.mu2r_internal=FresnelPublicData.mu2r;
		};
	//mu2rslider is a delta on FresnelShadowData.mu2r. 
	if(FresnelPublicData.mu2rslider!=FresnelShadowData.mu2rslider) {
		FresnelPublicData.mu2r = FresnelPrivateData.mu2r_internal + FresnelPublicData.mu2rslider;
		};
		

	//Calculate FresnelShadowData.n2rd and FresnelShadowData.n2id for given FresnelShadowData.dispersion and wavelength.
	//And the R/T coefficients.
	if (fabs(FresnelPublicData.n1r              -   FresnelShadowData.n1r         )   >=nsdel||
		fabs(FresnelPublicData.n1i              -   FresnelShadowData.n1i         )   >=nsdel||
		fabs(FresnelPublicData.mu1r             -   FresnelShadowData.mu1r        )   >=nsdel||
		fabs(FresnelPublicData.n2rslider        -   FresnelShadowData.n2rslider   )   >=nsdel||
		fabs(FresnelPublicData.n2r              -   FresnelShadowData.n2r         )   >=nsdel||
		fabs(FresnelPublicData.n2i              -   FresnelShadowData.n2i         )   >=nsdel||
		fabs(FresnelPublicData.mu2r             -   FresnelShadowData.mu2r        )   >=nsdel||
		fabs(FresnelPublicData.n2rslider        -   FresnelShadowData.n2rslider   )   >=nsdel||
		fabs(FresnelPublicData.n2islider        -   FresnelShadowData.n2islider   )   >=nsdel||
		fabs(FresnelPublicData.mu2rslider       -   FresnelShadowData.mu2rslider  )   >=nsdel||
		fabs(FresnelPublicData.lambdaslider     -   FresnelShadowData.lambdaslider)   >=lsdel||
		fabs(FresnelPublicData.disp             -   FresnelShadowData.disp        )   >=nsdel||
		fabs(FresnelPublicData.n2rd             -   FresnelShadowData.n2rd        )   >=nsdel||
		fabs(FresnelPublicData.n2id             -   FresnelShadowData.n2id        )   >=nsdel||
		fabs(FresnelPublicData.Pper             -   FresnelShadowData.Pper        )   >=nsdel||
		FresnelPublicData.Preset_n1           !=   FresnelShadowData.Preset_n1              ||      
		FresnelPublicData.Preset_n2           !=   FresnelShadowData.Preset_n2                    
 		) 
		{
		FresnelPublicData.n2rd=ndispersion(FresnelPublicData.lambdaslider,
						FresnelPublicData.n2r,FresnelPublicData.disp);
		FresnelPublicData.n2id=ndispersion(2.0*nD-FresnelPublicData.lambdaslider,FresnelPublicData.n2i,
						(FresnelPublicData.n2i/FresnelPublicData.n2r)*FresnelPublicData.disp);
		
	boolean match=false; //Set the n1 Preset displayed from the PMap n1r.. etc values . Tolerance on matching is epsilon
	for ( int32 i=0; i < Other_Value;i++)
		{	if ((fabs(FresnelPublicData.n1r  -  Medium_Preset_data[i].nr  ) <= epsilon ) & 
				(fabs(FresnelPublicData.n1i  -  Medium_Preset_data[i].ni  ) <= epsilon ) &
			 	(fabs(FresnelPublicData.mu1r -  Medium_Preset_data[i].mur ) <= epsilon ) )
			 		{	FresnelPublicData.Preset_n1=i;
			 			match=true;
			 			break;
			 		};
		};
	if ( match==false){FresnelPublicData.Preset_n1=Other_Value;};//Found the bug.

	match=false;    //Set the n2 Preset displayed from the PMap n1r.. etc values. Tolerance on matching is epsilon
	for ( int32 i=0; i < Other_Value;i++)
		{	if ((fabs(FresnelPublicData.n2r  -  Medium_Preset_data[i].nr  ) <= epsilon ) & 
				(fabs(FresnelPublicData.n2i  -  Medium_Preset_data[i].ni  ) <= epsilon ) &
			 	(fabs(FresnelPublicData.mu2r -  Medium_Preset_data[i].mur ) <= epsilon ) &
			 	(fabs(FresnelPublicData.disp -  Medium_Preset_data[i].disp) <= epsilon ))
			 		{	FresnelPublicData.Preset_n2=i;
			 			match=true;
			 			break;
			 		};
		};
	if ( match==false){FresnelPublicData.Preset_n2=Other_Value;};
		if(FresnelPublicData.TRMode == kUseTransparency)   
			{   for (int32 index = 0; index < CoeffSize; index++) //Compute perpendicular and parallel 
																//polarisation transmission coefficients.
					{complex thetai;
      		   		   	SET_COMPLEX (&thetai ,      index*PI/CoeffScale ,      0.0);
						FresnelPrivateData.Tperc.dat[index] = Re(Tper(thetai));
						FresnelPrivateData.Tparc.dat[index] = Re(Tpar(thetai));
					}
			}
		if(FresnelPublicData.TRMode == kUseReflection)	        
			{
			for (int32 index = 0; index < CoeffSize; index++) //Compute perpendicular and parallel 
															//polarisation reflection coefficients.
					{complex thetai;
     		       	SET_COMPLEX (&thetai ,      index*PI/CoeffScale ,      0.0);
					FresnelPrivateData.Rperc.dat[index] = Re(Rper(thetai));
					FresnelPrivateData.Rparc.dat[index] = Re(Rpar(thetai));
				}	        
			};
		};
		


	 FresnelShadowData.n1r              = FresnelPublicData.n1r            ; 
     FresnelShadowData.n1i              = FresnelPublicData.n1i            ;
     FresnelShadowData.n2r              = FresnelPublicData.n2r            ;
     FresnelShadowData.n2i              = FresnelPublicData.n2i            ;
     FresnelShadowData.mu1r             = FresnelPublicData.mu1r           ;
     FresnelShadowData.mu2r             = FresnelPublicData.mu2r           ;
     FresnelShadowData.Pper             = FresnelPublicData.Pper           ;
     FresnelPrivateData.Ppar            = 100-FresnelPublicData.Pper       ;
     FresnelShadowData.TRMode           = FresnelPublicData.TRMode         ;
     FresnelShadowData.n2rslider        = FresnelPublicData.n2rslider      ;
     FresnelShadowData.n2islider        = FresnelPublicData.n2islider      ;
     FresnelShadowData.mu2rslider       = FresnelPublicData.mu2rslider     ;
     FresnelShadowData.lambdaslider     = FresnelPublicData.lambdaslider   ;
     FresnelShadowData.disp             = FresnelPublicData.disp           ;
     FresnelShadowData.n2rd             = FresnelPublicData.n2rd           ;
     FresnelShadowData.n2id             = FresnelPublicData.n2id           ;
     FresnelShadowData.Preset_n1	    = FresnelPublicData.Preset_n1      ;          
	 FresnelShadowData.Preset_n2        = FresnelPublicData.Preset_n2      ;          
	 FresnelShadowData.Preset_illum     = FresnelPublicData.Preset_illum   ;    
	 FresnelShadowData.Color_Object     = FresnelPublicData.Color_Object   ;          
     FresnelShadowData.illuminant       = FresnelPublicData.illuminant     ;   
	}

	MCCOMErr result = MC_S_OK;

	if (FresnelPublicData.fMixShade)
		result = FresnelPublicData.fMixShade->QueryInterface(IID_I3DShShader, (void**)&fMixShader);

	FresnelPrivateData.Imp_Out = 0;
	if (fMixShader != NULL)
		FresnelPrivateData.Imp_Out = fMixShader->GetImplementedOutput();  

	return result;
}

MCCOMErr Fresnel::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	//if (message == kMsg_CUIP_ComponentAttached)
	//	{TestPass();
	//	// Need to return FALSE if we want kMsg_CUIP_ComponentAttached to be called
	//	return MC_S_FALSE;
	//	} // TBasicShader::HandleEvent( message, source, data);
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;
	TMCCountedPtr<IMFPart> parent_Part;
	TMCCountedPtr<IMFPart> text_Part;
	TMCCountedPtr<IMFPart> preset_Part;
	TMCCountedPtr<IMFSliderPart> Slider_Part;

	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();
	parent_Part = sourcePart->FindParentPartByID('NTOP');
	
	if(sourceID=='Diss' && message ==3)
	{	sourcePart->SetAttribute( TMFBooleanAttribute(kCustomDisplayRange_Token, true) );
		sourcePart->SetAttribute( TMFReal32Attribute (kSnpValue_Token,			 0.0) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMinValue_Token,			 -10000.0) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMaxValue_Token,			  10000.0) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMinValueDisplayed_Token,	   -0.25) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMaxValueDisplayed_Token,	    0.25) );
	};
	if (message == 3) { 
		//n2rslider is a delta on FresnelShadowData.n2r  .
		if (sourceID == 'N2rs') {
			FresnelPublicData.n2r = FresnelPrivateData.n2r_internal + FresnelPublicData.n2rslider;
			text_Part = parent_Part->FindChildPartByID('N2Re');
			text_Part->SetValue(&FresnelPublicData.n2r, kReal32ValueType, 1, 0);
			text_Part->ProcessUpdatesPart();
		}
		else if (sourceID == 'N2is') {
			FresnelPublicData.n2i = FresnelPrivateData.n2i_internal + FresnelPublicData.n2islider;
			text_Part = parent_Part->FindChildPartByID('N2Im');
			text_Part->SetValue(&FresnelPublicData.n2i, kReal32ValueType, 1, 0);
			text_Part->ProcessUpdatesPart();		
		}
		else if (sourceID == 'N2Ms') {
			FresnelPublicData.mu2r = FresnelPrivateData.mu2r_internal + FresnelPublicData.mu2rslider;
			text_Part = parent_Part->FindChildPartByID('N2Mr');
			text_Part->SetValue(&FresnelPublicData.mu2r, kReal32ValueType, 1, 0);
			text_Part->ProcessUpdatesPart();		
		}
		}
	if(message==5)
		{
		if (sourceID == 'PRS1') { //The User has selected a New Preset for Medium 1.
		int32 Preset_Value1;
		preset_Part = parent_Part->FindChildPartByID('PRS1');
		preset_Part->GetValue(&Preset_Value1, kInt32ValueType);
		if(Preset_Value1!=Other_Value)
			{
			FresnelShadowData.Preset_n1=Other_Value;	//Makes connected values recalculate	
			FresnelPublicData.Preset_n1=Preset_Value1;		
			FresnelPublicData.n1r  =  Medium_Preset_data[FresnelPublicData.Preset_n1].nr  ;
			FresnelPublicData.n1i  =  Medium_Preset_data[FresnelPublicData.Preset_n1].ni  ;
			FresnelPublicData.mu1r =  Medium_Preset_data[FresnelPublicData.Preset_n1].mur ;
			preset_Part->ProcessUpdatesPart();
			Fresnel::ExtensionDataChanged();
			}
		}else
		{
		if (sourceID == 'PRS2') { //The User has selected a New Preset for Medium 2.
		int32 Preset_Value2;
		preset_Part = parent_Part->FindChildPartByID('PRS2');
		preset_Part->GetValue(&Preset_Value2, kInt32ValueType);
		if(Preset_Value2!=Other_Value)
			{
			FresnelShadowData.Preset_n1=Other_Value;		
			FresnelPublicData.Preset_n2=Preset_Value2;		
			FresnelPublicData.n2r  =  Medium_Preset_data[FresnelPublicData.Preset_n2].nr  ;
			FresnelPublicData.n2i  =  Medium_Preset_data[FresnelPublicData.Preset_n2].ni  ;
			FresnelPublicData.mu2r =  Medium_Preset_data[FresnelPublicData.Preset_n2].mur ;
			FresnelPublicData.disp =  Medium_Preset_data[FresnelPublicData.Preset_n2].disp;
			FresnelPublicData.n2rslider=0.0;
			FresnelPublicData.n2islider=0.0;
			FresnelPublicData.mu2rslider=0.0;
			preset_Part->ProcessUpdatesPart();
			Fresnel::ExtensionDataChanged();
			}
		}
		}

	}
	return MC_S_OK;
}

Fresnel::Fresnel()		// Initialize the public data
{   FresnelPublicData.Object_Name="Rendering Camera";
	FresnelPublicData.fMixShade = NULL;
	FresnelPublicData.Mode = kSmooth_mode;
    //Default is Air->Glass
	FresnelPublicData.n1r          = 1.0;                    //Incident medium real RI.
    FresnelPublicData.n1i          = 0.0;                    //Incident medium imaginary RI. (Absorption).
    FresnelPublicData.n2r          = 1.51673;                //Target medium real RI. Typical Glass (10^-5).
    FresnelPublicData.n2i          = 0.00001;                //Target medium imaginary RI. (Absorption).
    FresnelPublicData.mu1r         = 1.0;                    //Incident medium real magnetic permittivity.
    FresnelPublicData.mu2r         = 1.0;                    //Target medium real magnetic permittivity.
    FresnelPublicData.Pper         = 50.0;                   //Proportion perpendicular polarization.
	FresnelPublicData.TRMode       = kUseReflection;         //Select Transparency or Reflection. kUseTransparency=Transparency On, kUseRflection=Reflection On.
    FresnelPublicData.n2rslider    = 0.0;
    FresnelPublicData.n2islider    = 0.0;
    FresnelPublicData.mu2rslider   = 0.0;
    FresnelPublicData.lambdaslider = 589.3;                 //Most published RFI measured at 589.3nm, Sodium D line.
    FresnelPublicData.disp         = 0.017;                 //Dispersion of Crown glass.
    FresnelPublicData.n2rd         = 1.51673;               //Target medium real RI. Typical Glass (1.5). This value will be updated for the FresnelShadowData.dispersion and wavelength chosen.
    FresnelPublicData.n2id         = 0.00001;;              //Target medium imaginary RI. (Absorption). This value will be updated for the FresnelShadowData.dispersion and wavelength chosen.
    FresnelPublicData.Preset_illum = D65;
    FresnelShadowData.Preset_illum = Other_Illuminant;
    FresnelPublicData.Color_Object = false;
	FresnelPrivateData.first_pass = 1;
	FresnelPrivateData.last_luma = 0;
	FresnelPrivateData.GLf = 0.9;
	
		 	
	for (int32 i = D30; i<Num_Illumination_Presets ; i++)
	{
	 	FresnelPrivateData.illum_Preset_data[i] = 3000.0 + 500*(i-D30);
	};
	 	FresnelPrivateData.illum_Preset_data[D93] = 9300.0;
 	
                  			  	 		/* Name                  xRed    	yRed    	YRed,      xGreen  		yGreen  	YGreen   		xBlue    		yBlue  		YBlue       White/Black point   Gamma   */
	const colourSystem    AdobeRGB    =  { "Adobe RGB",          0.64,    	0.33   		,0.297361 ,0.21,   		0.71   		,0.627355 		,0.15,   		0.06   		,0.075285 ,  IlluminantD65,      2.2           };
	const colourSystem    sRGBSystem  =  { "sRGB     ",          0.64,    	0.33   		,0.212656 ,0.3 ,   		0.6   		,0.715158 		,0.15,   		0.06   		,0.072186 ,  IlluminantD65,      sRGB          };
	const colourSystem    NTSCsystem  =  { "NTSC",               0.67,    	0.33   		,0.298839 ,0.21,   		0.71   		,0.586811 		,0.14,   		0.08   		,0.114350 ,  IlluminantC,        GAMMA_REC709  };
	const colourSystem    SMPTEsystem =  { "SMPTE",              0.630,   	0.340  		,0.212395 ,0.310,  		0.595  		,0.701049 		,0.155,  		0.070  		,0.086556 ,  IlluminantD65,      GAMMA_REC709  };
	const colourSystem    CIEsystem   =  { "CIE",                0.7350,  	0.2650 		,0.176204 ,0.2740, 		0.7170 		,0.812985 		,0.1670, 		0.0090 		,0.010811 ,  IlluminantE,        GAMMA_REC709  };
	const colourSystem    MyMonitor   =  { "Apple 17",		     0.6420800, 0.356442 	,0.265300 ,0.314654,  	0.588295  	,0.638300  		,0.150953, 		0.100531  	,0.096650 ,  MyWhitePoint,       2.2		   };
	const colourSystem    AppleRGB    =  { "Apple RGB",		     0.6250,  	0.340  		,0.244634 ,0.280,  		0.595  		,0.672034 		,0.1550, 		0.0700 		,0.083332 ,  IlluminantD65,      1.8		   };
	const colourSystem    Source      =  { "Source CS",		     0.7350,  	0.2650 		,0.176204 ,0.2740, 		0.7170 		,0.812985 		,0.1670, 		0.0090 		,0.010811 ,  IlluminantD50,      2.2		   };
	const colourSystem    Destination =  { "Destination CS",     0.6420800, 0.356442 	,0.265300 ,0.314654,  	0.588295  	,0.638300  		,0.150953, 		0.100531  	,0.096650 ,  IlluminantD50,      2.2		   };
	const colourSystem    MyMonitorsRGB= { "Apple 17",		     0.642000,  0.358000 	,0.222885 ,0.318000,  	0.596000  	,0.630570  		,0.153000, 		0.103000  	,0.135747,   MyWhitePoint,       sRGB		   };
	const colourSystem    EoviaSystem  = { "EoviasRGB",          0.625012,  0.339979   	,0.244545 ,0.279967 ,   0.594953   	,0.672102 		,0.154908,		0.069936	,0.083296 ,  Eovia        ,      sRGB          };
        						 		/* Name                  xRed    	yRed    	YRed,      xGreen  		yGreen  	YGreen   		xBlue    		yBlue  		YBlue       White/Black point   Gamma   */	 	
	FresnelPrivateData.Source_ColorSystem=Source;
	FresnelPrivateData.Destination_ColorSystem=MyMonitor;
	FresnelPrivateData.Monitor_ColorSystem=sRGBSystem;
	FresnelPrivateData.CIE_ColorSystem=CIEsystem;
	FresnelPrivateData.Adobe_ColorSystem=AdobeRGB;
	FresnelPrivateData.SolutionAbort=false;	 
	FresnelPrivateData.disableKGR=false;
	FresnelPrivateData.Eovia_ColorSystem=EoviaSystem;
	x1=0.290;
	y1=0.478;
	x2=0.293;
	y2=0.480;
};
	
void* Fresnel::GetExtensionDataBuffer()
{
	return ((void*) &(FresnelPublicData));
}

boolean Fresnel::IsEqualTo(I3DExShader* aShader)		// Compare two Fresnel shaders
{
	int32 equality = true;	
	if 	(	FresnelPublicData.Object_Name         != ((Fresnel*)aShader)->FresnelPublicData.Object_Name	  				||
			FresnelPublicData.Mode                != ((Fresnel*)aShader)->FresnelPublicData.Mode             			||
			FresnelPublicData.fMixShade       	  != ((Fresnel*)aShader)->FresnelPublicData.fMixShade        			||
			FresnelPublicData.Preset_n1       	  != ((Fresnel*)aShader)->FresnelPublicData.Preset_n1        			||
			FresnelPublicData.Preset_n1       	  != ((Fresnel*)aShader)->FresnelPublicData.Preset_n1        			||
			FresnelPublicData.Preset_illum        != ((Fresnel*)aShader)->FresnelPublicData.Preset_illum        		||
			fabs(FresnelPublicData.n1r             -  ((Fresnel*)aShader)->FresnelPublicData.n1r              )   >=nsdel|| 
            fabs(FresnelPublicData.n1i             -  ((Fresnel*)aShader)->FresnelPublicData.n1i              )   >=nsdel||
            fabs(FresnelPublicData.n2r             -  ((Fresnel*)aShader)->FresnelPublicData.n2r              )   >=nsdel||
            fabs(FresnelPublicData.n2i             -  ((Fresnel*)aShader)->FresnelPublicData.n2i              )   >=nsdel||
            fabs(FresnelPublicData.mu1r            -  ((Fresnel*)aShader)->FresnelPublicData.mu1r             )   >=nsdel||
            fabs(FresnelPublicData.mu2r            -  ((Fresnel*)aShader)->FresnelPublicData.mu2r             )   >=nsdel||
            fabs(FresnelPublicData.Pper            -  ((Fresnel*)aShader)->FresnelPublicData.Pper             )   >=nsdel||
            FresnelPublicData.TRMode          	  != ((Fresnel*)aShader)->FresnelPublicData.TRMode           			||
            fabs(FresnelPublicData.n2rslider       -  ((Fresnel*)aShader)->FresnelPublicData.n2rslider        )   >=nsdel||
            fabs(FresnelPublicData.n2islider       -  ((Fresnel*)aShader)->FresnelPublicData.n2islider        )   >=nsdel||
            fabs(FresnelPublicData.mu2rslider      -  ((Fresnel*)aShader)->FresnelPublicData.mu2rslider       )   >=nsdel||
            fabs(FresnelPublicData.lambdaslider    -  ((Fresnel*)aShader)->FresnelPublicData.lambdaslider     )   >=lsdel||
            fabs(FresnelPublicData.disp            -  ((Fresnel*)aShader)->FresnelPublicData.disp             )   >=nsdel||
            fabs(FresnelPublicData.n2rd            -  ((Fresnel*)aShader)->FresnelPublicData.n2rd             )   >=nsdel||   
            fabs(FresnelPublicData.n2id            -  ((Fresnel*)aShader)->FresnelPublicData.n2id             )   >=nsdel||
            FresnelPublicData.Color_Object        != ((Fresnel*)aShader)->FresnelPublicData.Color_Object     			||          
     		fabs(FresnelPublicData.illuminant      -  ((Fresnel*)aShader)->FresnelPublicData.illuminant       )   >=de     
            )  
            {   equality = false;};
	return equality;	  
}

MCCOMErr Fresnel::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fNeedsNormal = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput Fresnel::GetImplementedOutput()
{
	return  kUsesGetColor;	
}

#if (VERSIONNUMBER >= 0x040000)
boolean Fresnel::WantsTransform()
{
	return 0;
}
#endif

#if (VERSIONNUMBER == 0x010000)
MCCOMErr Fresnel::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
{
result=TMCColorRGB::kBlack;
hold=TMCColorRGB::kBlack;
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Fresnel::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
{
result=TMCColorRGB::kBlack;
hold=TMCColorRGB::kBlack;
#elif (VERSIONNUMBER == 0x030000)
real Fresnel::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
{
result=TMCColorRGB::kBlack;
hold=TMCColorRGB::kBlack;
#elif (VERSIONNUMBER >= 0x040000)
real Fresnel::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
{
result=TMCColorRGBA::kBlackFullAlpha;
hold=TMCColorRGBA::kBlackFullAlpha;
#endif

	fullArea = false;

	//first figure out the scene
	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShScene> scene;
	TMCCountedPtr<I3DShTreeElement> objecttree;

	//first figure out the scene
	if (MCVerify(shadingIn.fInstance))
		if (shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree) == MC_S_OK)		
			tree->GetScene(&scene);	// Find the scene
	// temp variables
	TTransform3D sped;
	TVector3 Camera_loc;
	TVector3 toCamera_vec;
	// try and get the rendering camera (chopped code from Eric Winemiller)

	TMCString255 temp = "Rendering Camera";
	TMCString255 blank = "";
	if ((FresnelPublicData.Object_Name == temp || FresnelPublicData.Object_Name==blank) & (FresnelPrivateData.first_pass == 1))
	{			
		TMCCountedPtr<I3DRenderingModule> renderingModule;
		TMCString255 cameraName;

		if (MCVerify(scene))
		{
			scene->GetSceneRenderingModule(&renderingModule);
			if (MCVerify(renderingModule))
			{
				renderingModule->GetRenderingCameraByName(cameraName);
				FresnelPublicData.Object_Name = cameraName;
				FresnelPrivateData.first_pass = 0;
			}
		}
	}

	if (MCVerify(scene))
	{
		//now that you have a scene, use gettreeelementbyname to find your object (object name from text box)
		scene->GetTreeElementByName( &objecttree, FresnelPublicData.Object_Name );
		if (MCVerify(objecttree))
		{
			//get the tree's global position
			objecttree->GetGlobalTransform3D(sped);
			
			Camera_loc = sped.fTranslation;
			toCamera_vec = Camera_loc - shadingIn.fPoint;
		}
		else
		{
			Camera_loc.x=30.46f;
			Camera_loc.y=30.46f;
			Camera_loc.z=31.20f;
			toCamera_vec = Camera_loc - shadingIn.fPoint;
		}
	}

	FresnelPrivateData.ref_vec = toCamera_vec;		// swap to the Camera/light vector

	// calculate angle of surface vs angle of ref vector (light incidence)
	real32 dotCamera_SurfaceNormal=FresnelPrivateData.ref_vec*shadingIn.fGNormal;



    // ok, so every once in a (great) while, this eqn was coming up negative and barfing out the sqrt below. So this is a hack.

	double temp1 =fabs( FresnelPrivateData.ref_vec.GetMagnitudeSquared() * shadingIn.fGNormal.GetMagnitudeSquared()- pow(dotCamera_SurfaceNormal,2));

	FresnelPrivateData.angle = fabs(atan2(real32(sqrt(temp1)),dotCamera_SurfaceNormal));
	// Add in mix shader, if requested
	if (FresnelPublicData.Mode == kShader_mode)
		{
			if (!fMixShader)  // if the mix shader is null, just return black.
			{
				FresnelPrivateData.luma = 0;
			}
			else
			{
				if (FresnelPrivateData.Imp_Out & (kUsesGetColor | kUsesDoShade))		// get color implemented
				{
					fMixShader->GetColor(FresnelPrivateData.Color, fullArea, shadingIn);
					FresnelPrivateData.luma = (FresnelPrivateData.Color.R*0.3 + FresnelPrivateData.Color.G*0.59+ FresnelPrivateData.Color.B*0.11);
				}
				else if (FresnelPrivateData.Imp_Out & kUsesGetValue)  //  or get value implemented
					fMixShader->GetValue(FresnelPrivateData.luma, fullArea, shadingIn);
				else 
					FresnelPrivateData.luma = 0;		// no useful data back from mix shader, return black
			}
		FresnelPrivateData.luma = FresnelPrivateData.luma > 1 ? FresnelPrivateData.last_luma : FresnelPrivateData.luma;

		FresnelPrivateData.last_luma = FresnelPrivateData.luma;	// in case it returns over 1.0, a bad value, preserve the last good value

		FresnelPrivateData.angle = ((FresnelPrivateData.angle + (FresnelPrivateData.luma - 0.5)) > 0 ?(FresnelPrivateData.angle + (FresnelPrivateData.luma - 0.5)) : 0); // pin it at zero so the pattern won't repeat.
		}

    
	real32 luma_val =  Fresnel_filter ( FresnelPrivateData.angle );
	if ( luma_val > 1.0) { luma_val=1.0;};
	if ( luma_val < 0.0) { luma_val=0.0;};
	if ( FresnelPublicData.Color_Object == false )
	{
   	                                      //It is possible to pass 1020 gray levels to Carrara.
/*		real32 gray=luma_val*1020.0;       //Scale luma to 0.0 to 1020.0 .
    	int32 r =round(gray/4.0);           //Carrara gets 50% gray value from Red   component.
    	real32 gb=gray-2*r;                //Remainder is Green + Blue.                
    	int32 g =round(gb/2.0);             //Carrara gets 25% gray value from Green component.
    	int32 b =round(gray-2*r-g);         //Carrara gets 25% gray value from Blue  component.
		result.R = r/255.0;               //Scale back down to 0.0-1.0.
		result.G = g/255.0;
		result.B = b/255.0;*/
		if ((shadingIn.fUV.x > x1) && (shadingIn.fUV.x < x2) && (shadingIn.fUV.y > y1) && (shadingIn.fUV.y < y2)) {
			real temp = 0;
		}
		result.R = luma_val;               
		result.G = luma_val;
		result.B = luma_val;
				
	}else
	{  // Color object according to hue of Spectral Light. Luma is the value returned from the Fresnel Filter. 
		//Intensity of Spectral color is returned in the saturation channel.
		TMCColorRGBA S;
		real32 SH,SL,SS; 
		S.R=FresnelPrivateData.SpectralRGB.R;
		S.G=FresnelPrivateData.SpectralRGB.G;
		S.B=FresnelPrivateData.SpectralRGB.B;
		S.GetHLS(SH,SL,SS);
		result.SetHLS(SH,luma_val,FresnelPrivateData.SpectralLab.L/100.0);
	};

#if (VERSIONNUMBER == 0x010000)
return MC_S_OK;
#elif (VERSIONNUMBER == 0x020000)
return MC_S_OK;
#elif (VERSIONNUMBER >= 0x030000)
return 1.0f;
#endif
}

complex operator^ ( complex a, complex b)  //   (a^b),   a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 br = REAL (b), bi = IMAG (b);
  real32 f = pow(ar*ar + ai*ai, real32(br/2.0))*exp(-bi*atan2(ai,ar));
  real32 znr = cos(br*atan2(ai,ar)+0.5*bi*log(ai*ai+ar*ar));
  real32 zni = sin(br*atan2(ai,ar)+0.5*bi*log(ai*ai+ar*ar));
  complex z;
  SET_COMPLEX (&z, znr*f, zni*f);
  return z;
}

complex operator^ ( complex a, int b)  //   (a^b),   a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  complex bc;
  SET_COMPLEX (&bc, b, 0);
  real32 br = REAL (bc), bi = IMAG (bc);
  real32 f = pow(ar*ar + ai*ai,real32(br/2.0))*exp(-bi*atan2(ai,ar));
  real32 znr = cos(br*atan2(ai,ar)+0.5*bi*log(ai*ai+ar*ar));
  real32 zni = sin(br*atan2(ai,ar)+0.5*bi*log(ai*ai+ar*ar));
  complex z;
  SET_COMPLEX (&z, znr*f, zni*f);
  return z;
}
complex operator^ ( complex a, real32 b)  //   (a^b),   a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  complex bc;
  SET_COMPLEX (&bc, b, 0);
  real32 br = REAL (bc), bi = IMAG (bc);
  real32 f = pow(ar*ar + ai*ai,real32(br/2.0))*exp(-bi*atan2(ai,ar));
  real32 znr = cos(br*atan2(ai,ar)+0.5*bi*log(ai*ai+ar*ar));
  real32 zni = sin(br*atan2(ai,ar)+0.5*bi*log(ai*ai+ar*ar));
  complex z;
  SET_COMPLEX (&z, znr*f, zni*f);
  return z;
}
complex operator^ ( real32 a, complex b)  //   (a^b),   a,b complex.
{ real32 br = REAL (b), bi = IMAG (b);
  complex ac;
  SET_COMPLEX (&ac, a, 0);
  real32 ar = REAL (ac), ai = IMAG (ac);
  real32 f = pow(ar*ar + ai*ai,real32(br/2.0))*exp(-bi*atan2(ai,ar));
  real32 znr = cos(br*atan2(ai,ar)+0.5*bi*log(ai*ai+ar*ar));
  real32 zni = sin(br*atan2(ai,ar)+0.5*bi*log(ai*ai+ar*ar));
  complex z;
  SET_COMPLEX (&z, znr*f, zni*f);
  return z;
}
complex Fresnel::cadd ( complex a , complex b ) //(a + b), a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 br = REAL (b), bi = IMAG (b);
  complex z;
  SET_COMPLEX (&z, ar + br, ai + bi);
  return z;
}
complex Fresnel::csub ( complex a , complex b ) //(a - b), a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 br = REAL (b), bi = IMAG (b);
  complex z;
  SET_COMPLEX (&z, ar - br, ai - bi);
  return z;
}
complex Fresnel::cmul ( complex a , complex b ) //(a * b), a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 br = REAL (b), bi = IMAG (b);
  complex z;
  SET_COMPLEX (&z, ar * br - ai * bi, ar * bi + ai * br);
  return z;
}
complex Fresnel::cneg ( complex a )            //    (-a),   a complex.
{ complex z;
  SET_COMPLEX (&z, -REAL (a), -IMAG (a));
  return z;
}
complex Fresnel::sinc ( complex a )            //    sin(a), a complex.
{ real32 R = REAL (a), I = IMAG (a);
  complex z;
  if (I == 0.0) 
    {SET_COMPLEX (&z, sin (R), 0.0);} 
  else 
    {SET_COMPLEX (&z, sin (R) * cosh (I), cos (R) * sinh (I));}
  return z;
}
complex Fresnel::cosc ( complex a )            //    cos(a), a complex.
{ real32 R = REAL (a), I = IMAG (a);
  complex z;
  if (I == 0.0) 
    {SET_COMPLEX (&z, cos (R), 0.0);} 
  else 
    {SET_COMPLEX (&z, cos (R) * cosh (I), sin (R) * sinh (-I));}
  return z;
}
complex Fresnel::arcsinc ( complex a )         // arcsin(a), a complex.
{ real32 R = REAL (a), I = IMAG (a);
  const real32 HALF_PI=PI/2.0;
  complex z;
  if (I == 0)
    {
  if (fabs (R) <= 1.0)
    {
      SET_COMPLEX (&z, asin (R), 0.0);
    }
  else
    {
      if (R < 0.0)
        {
          SET_COMPLEX (&z, -HALF_PI, log(-R + sqrt(-R-1.0)*sqrt(1.0 - R)));
        }
      else
        {
          SET_COMPLEX (&z, HALF_PI, -log( R + sqrt( R-1.0)*sqrt(1.0 + R)));
        }
    }
    }
  else
    {
      real32 x = fabs (R), y = fabs (I);
      real32 r = hypot (x + 1, y), s = hypot (x - 1, y);
      real32 A = 0.5 * (r + s);
      real32 B = x / A;
      real32 y2 = y * y;
      real32 real, imag;
      const real32  A_crossover = 1.5, B_crossover = 0.6417;
      if (B <= B_crossover)
        {
          real = asin (B);
        }
      else
        {
          if (x <= 1)
            {
              real32 D = 0.5 * (A + x) * (y2 / (r + x + 1) + (s + (1 - x)));
              real = atan (x / sqrt (D));
            }
          else
            {
              real32 Apx = A + x;
              real32 D = 0.5 * (Apx / (r + x + 1) + Apx / (s + (x - 1)));
              real = atan (x / (y * sqrt (D)));
            }
        }
      if (A <= A_crossover)
        {
          real32 Am1;
          if (x < 1)
            {
              Am1 = 0.5 * (y2 / (r + (x + 1)) + y2 / (s + (1 - x)));
            }
          else
            {
              Am1 = 0.5 * (y2 / (r + (x + 1)) + (s + (x - 1)));
            }
          imag = log (1.0 + Am1 + sqrt (Am1 * (A + 1)));
        }
      else
        {
          imag = log (A + sqrt (A * A - 1));
        }
      SET_COMPLEX (&z, (R >= 0) ? real : -real, (I >= 0) ? imag : -imag);
    }

  return z;
}
complex Fresnel::csqr ( complex a )            //   (a^2),   a complex.
{ return cmul (a,a);
}
complex Fresnel::cabs ( complex a )            //   (|a|),   a complex.
{ real32 R = REAL (a);
  real32 I = IMAG (a);
  complex z;
  SET_COMPLEX (&z, sqrt(R*R+I*I), 0.0);
  return z;
}
complex Fresnel::cabsq ( complex a )           //   (|a|^2), a complex.
{ real32 R = REAL (a);
  real32 I = IMAG (a);
  complex z;
  SET_COMPLEX (&z, R*R+I*I, 0.0);
  return z;
}
complex Fresnel::cdiv ( complex a, complex b)  //   (a/b),   a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 br = REAL (b), bi = IMAG (b);
  real32 d = 1.0/(br*br+bi*bi);
  real32 znr = (ar * br + ai * bi) * d;
  real32 zni = (ai * br - ar * bi) * d;
  complex z;
  SET_COMPLEX (&z, znr, zni);
  return z;
}
complex Fresnel::cosfc ( complex n, complex d, complex a) //(n/d)*cos(a)), n,d,a complex.
{ return cmul(cdiv(n,d),cosc(a));
}

complex Fresnel::n1 ( complex thetai ) //Incident medium refractive index
{ 
  real32  n1r=FresnelPublicData.n1r;
  real32  n1i=FresnelPublicData.n1i;
  real32  n2r=FresnelPublicData.n2rd;
  real32  n2i=FresnelPublicData.n2id;
  real32 r = ( REAL (thetai));
  real32 i = ( IMAG (thetai));
  const real32 p2 = PI/2.0;
  complex z;
          if ( r <= p2  &&  i <= p2 )
            {
               SET_COMPLEX (&z, n1r, n1i);
            }
          else
            {
           if ( r > p2 &&  i >  p2 )
            {
               SET_COMPLEX (&z, n2r, n2i);
            }
          else
           if ( r > p2 &&  i <= p2 ) 
           {
               SET_COMPLEX (&z, n2r, n1i);
           } 
          else 
           if ( r <= p2 &&  i >  p2 ) 
           {
               SET_COMPLEX (&z, n1r, n2i);
           }
           }
          return z;
}
complex Fresnel::n2 ( complex thetai ) //target medium refractive index
{ 
  real32  n1r=FresnelPublicData.n1r;
  real32  n1i=FresnelPublicData.n1i;
  real32  n2r=FresnelPublicData.n2rd;
  real32  n2i=FresnelPublicData.n2id;
  real32 r = ( REAL (thetai));
  real32 i = ( IMAG (thetai));
  const real32 p2 = PI/2.0;
  complex z;
          if ( r <= p2  &&  i <= p2 )
            {
               SET_COMPLEX (&z, n2r, n2i);
            }
          else
            {
           if ( r > p2 &&  i >  p2 )
            {
               SET_COMPLEX (&z, n1r, n1i);
            }
          else
           if ( r > p2 &&  i <=  p2 ) 
           {
               SET_COMPLEX (&z, n1r, n2i);
           } 
          else 
           if ( r <= p2 &&  i >  p2 ) 
           {
               SET_COMPLEX (&z, n2r, n1i);
           }
           }
          return z;
}
complex Fresnel::mu1 ( complex thetai ) //Incident medium magnetic permeability.
{ 
  real32 mu1r=FresnelPublicData.mu1r;
  real32 mu1i=0.0;
  real32 mu2r=FresnelPublicData.mu2r;
  real32 mu2i=0.0;
  real32 r = ( REAL (thetai));
  real32 i = ( IMAG (thetai));
  const real32 p2 = PI/2.0;
  complex z;
          if ( r <= p2  &&  i <= p2 )
            {
               SET_COMPLEX (&z, mu1r, mu1i);
            }
          else
            {
           if ( r > p2 &&  i >  p2 )
            {
               SET_COMPLEX (&z, mu2r, mu2i);
            }
          else
           if ( r > p2 &&  i <=  p2 ) 
           {
               SET_COMPLEX (&z, mu2r, mu1i);
           } 
          else 
           if ( r <= p2 &&  i >  p2 ) 
           {
               SET_COMPLEX (&z, mu1r, mu2i);
           }
           }
          return z;
}
complex Fresnel::mu2 ( complex thetai ) //Target medium magnetic permeability.
{ 
  real32 mu1r=FresnelPublicData.mu1r;
  real32 mu1i=0.0;
  real32 mu2r=FresnelPublicData.mu2r;
  real32 mu2i=0.0;
  real32 r = ( REAL (thetai));
  real32 i = ( IMAG (thetai));
  const real32 p2 = PI/2.0;
  complex z;
          if ( r <= p2  &&  i <= p2 )
            {
               SET_COMPLEX (&z, mu2r, mu2i);
            }
          else
            {
           if ( r > p2 &&  i >  p2 )
            {
               SET_COMPLEX (&z, mu1r, mu1i);
            }
          else
           if ( r > p2 &&  i <= p2 ) 
           {
               SET_COMPLEX (&z, mu1r, mu2i);
           } 
          else 
           if ( r <= p2 &&  i >  p2 ) 
           {
               SET_COMPLEX (&z, mu2r, mu1i);
           }
           }
          return z;
}
complex Fresnel::theta ( complex thetai ) //Incident angle. 
{ 
  real32 r = ( REAL (thetai));
  real32 i = ( IMAG (thetai));
  const real32 p2 = PI/2.0;
  complex z;
          if ( r <= p2  &&  i <= p2 )
            {
               SET_COMPLEX (&z,      r,      i);
            }
          else
            {
           if ( r > p2 &&  i >  p2 )
            {
               SET_COMPLEX (&z, PI - r, PI - i);
            }
          else
           if ( r > p2 &&  i <= p2 ) 
           {
               SET_COMPLEX (&z, PI - r,      i);
           } 
          else 
           if ( r <= p2 &&  i >  p2 ) 
           {
               SET_COMPLEX (&z,      r, PI - i);
           }
           }
          return z;
}
real32 Fresnel::Re ( complex x ) //Real part of x, x complex.
{ 
          return ( REAL (x));
}
real32 Fresnel::Im ( complex x ) //Imaginary part of x, x complex.
{ 
          return ( IMAG (x));
}
complex Fresnel::thetat ( complex thetai ) //Refracted beam angle, complex.
{ 
          return arcsinc(cmul(cdiv(n1(thetai),n2(thetai)), sinc(theta(thetai))));
}
//Numerator Equation (5) http://scienceworld.wolfram.com/physics/FresnelEquations.html
complex Fresnel::Eperr ( complex thetai ) 
{ 
          return csub(cosfc(n1(thetai),mu1(thetai),theta(thetai)),cosfc(n2(thetai),mu2(thetai),thetat(thetai)));
}
//Denominator Equations (5) and (6) http://scienceworld.wolfram.com/physics/FresnelEquations.html
complex Fresnel::Eperi ( complex thetai ) 
{ 
          return cadd(cosfc(n1(thetai),mu1(thetai),theta(thetai)),cosfc(n2(thetai),mu2(thetai),thetat(thetai)));
}
//Numerator Equation (6) http://scienceworld.wolfram.com/physics/FresnelEquations.html
complex Fresnel::Epert ( complex thetai ) 
{         complex two;
          SET_COMPLEX (&two,      2.0,      0.0);
          return cosfc(cmul(two ,n1(thetai)),mu1(thetai),theta(thetai));
}
//Numerator Equation (14) http://scienceworld.wolfram.com/physics/FresnelEquations.html
complex Fresnel::Eparr ( complex thetai )  
{ 
          return csub(cosfc(n2(thetai),mu2(thetai),theta(thetai)),cosfc(n1(thetai),mu1(thetai),thetat(thetai)));
}
//Denominator Equations (14) and (15) http://scienceworld.wolfram.com/physics/FresnelEquations.html
complex Fresnel::Epari ( complex thetai ) 
{ 
          return cadd(cosfc(n2(thetai),mu2(thetai),theta(thetai)),cosfc(n1(thetai),mu1(thetai),thetat(thetai)));
}

//Numerator Equation (15) http://scienceworld.wolfram.com/physics/FresnelEquations.html
complex Fresnel::Epart ( complex thetai ) 
{         complex two;
          SET_COMPLEX (&two,      2.0,      0.0);
          return cosfc(cmul(two ,n1(thetai)),mu1(thetai),theta(thetai));
}
complex Fresnel::rper ( complex thetai ) //Perpendicular polarised light reflection coefficient.
{ 
          return cdiv(Eperr(thetai),Eperi(thetai));
}
complex Fresnel::rpar ( complex thetai ) //Parallel polarised light reflection coefficient.
{ 
          return cdiv(Eparr(thetai),Epari(thetai));
}
complex Fresnel::tper ( complex thetai ) //Perpendicular polarised light reflection coefficient.
{ 
          return cdiv(Epert(thetai),Eperi(thetai));
}
complex Fresnel::tpar ( complex thetai ) //Parallel polarised light transmission coefficient.
{ 
          return cdiv(Epart(thetai),Epari(thetai));
}
complex Fresnel::TArea ( complex thetai ) //Reciprocal of Area of refracted beam relative to incident beam.
{ 
          return cdiv(cosfc(n2(thetai),n1(thetai),thetat(thetai)),cosc(theta(thetai)));
}
complex Fresnel::Rper ( complex thetai ) //Intensity (power per unit area) of perpendicular polarised beam.
{ 
          return cabsq(rper(thetai));
}
complex Fresnel::Tper ( complex thetai ) //Power of perpendicular polarised refracted beam, Intensity/Area.
{ 
          return cmul(cabsq(tper(thetai)),TArea(thetai));
}
complex Fresnel::Rpar ( complex thetai ) //Intensity (power per unit area) of parallel polarised reflected beam.
{ 
          return cabsq(rpar(thetai)); //Intensity (power per unit area) of parallel polarised reflected beam.
}
complex Fresnel::Tpar ( complex thetai )
{ 
          return cmul(cabsq(tpar(thetai)),TArea(thetai)); //Power of parallel polarised refracted beam, Intensity/Area.
}
inline real32 Fresnel::R ( complex thetai ) //Mix parallel and perpendicular reflected beams in relative proportions.
{  real32 result;
   real32 Ppar=100. - FresnelPublicData.Pper ; 
   real32 Pper=FresnelPublicData.Pper ;
   int32 thetaiindex=floor(Re(thetai)*CoeffScale/PI);
   real32 thetap=Re(thetai)*CoeffScale/PI;
   int32 thetaiindexpls1;
   if ((thetaiindex+1)<CoeffSize)
	{thetaiindexpls1=thetaiindex+1;}
   else
	{thetaiindexpls1=thetaiindex;};
   real32 basepar=FresnelPrivateData.Rparc.dat[thetaiindex];
   real32 baseparp1=FresnelPrivateData.Rparc.dat[thetaiindexpls1];
   real32 baseper=FresnelPrivateData.Rperc.dat[thetaiindex];
   real32 baseperp1=FresnelPrivateData.Rperc.dat[thetaiindexpls1];
   result= (((Ppar/100.0 * (basepar+(baseparp1-basepar)*(thetap-thetaiindex)))))+((Pper/100.0 * (baseper+(baseperp1-baseper)*(thetap-thetaiindex))));
   if(result <= 1.0 && result >= 0.0 ) 
   { 
     return result;
   }
   else
   {
     return ((Ppar/100.0 * FresnelPrivateData.Rparc.dat[round(Re(thetai)*CoeffScale/PI)])+(Pper/100.0 * FresnelPrivateData.Rperc.dat[round(Re(thetai)*CoeffScale/PI)]));
   }
}
inline real32 Fresnel::T ( complex thetai ) //Mix parallel and perpendicular refracted beams in relative proportions.
{  real32 result;
   real32 Ppar=100. - FresnelPublicData.Pper ; 
   real32 Pper=FresnelPublicData.Pper ;
   int32 thetaiindex=floor(Re(thetai)*CoeffScale/PI);
   real32 thetap=Re(thetai)*CoeffScale/PI;
   int32 thetaiindexpls1;
   if ((thetaiindex+1)<CoeffSize){thetaiindexpls1=thetaiindex+1;}else{thetaiindexpls1=thetaiindex;};
   real32 basepar=FresnelPrivateData.Tparc.dat[thetaiindex];
   real32 baseparp1=FresnelPrivateData.Tparc.dat[thetaiindexpls1];
   real32 baseper=FresnelPrivateData.Tperc.dat[thetaiindex];
   real32 baseperp1=FresnelPrivateData.Tperc.dat[thetaiindexpls1];
   result= (((Ppar/100.0 * (basepar+(baseparp1-basepar)*(thetap-thetaiindex)))))+((Pper/100.0 * (baseper+(baseperp1-baseper)*(thetap-thetaiindex))));
   if(result <= 1.0 && result >= 0.0 ) 
   { 
     return result;
   }
   else
   {
     return ((Ppar/100.0 * FresnelPrivateData.Tparc.dat[round(Re(thetai)*CoeffScale/PI)])+(Pper/100.0 * FresnelPrivateData.Tperc.dat[round(Re(thetai)*CoeffScale/PI)]));
   }
}
real32 Fresnel::ndispersion ( real32 lambda , real32 nt, real32 dispnt)
{ real32 lambda2=lambda*lambda/1000000.0;
  real32 nglass=sqrt(1 + 1.01047 * lambda2 / (-103.561 + lambda2) + 
	  0.231792 * lambda2 / (-0.0200179 + lambda2) + 1.03961 * lambda2 / (-0.0060007 + lambda2));
  return -188.246  * dispnt *  nt + 124.113  * dispnt *  nt  * nglass + 188.246 * dispnt - 
	  124.113 * dispnt * nglass + nt;
}

inline real32 Fresnel::Fresnel_filter (real32 angle) //Calculated power of refracted and reflected beams.
{   complex thetai;
    complex nu;  //RFI of vacuum.
    complex zero;
    complex anglei;     
    SET_COMPLEX (&nu,      1.0,      0.0);
    SET_COMPLEX (&zero,      0.0,      0.0); //Normal incidence.
    SET_COMPLEX (&anglei,      angle,      PI/2.0);
    SET_COMPLEX (&thetai,      angle,      0.0);

    if (FresnelPublicData.TRMode == kUseTransparency){return ((T(thetai)));};
	if (FresnelPublicData.TRMode == kUseReflection  ){return ((R(thetai)));};
	return 0;
}

int32 Fresnel::round(real32 input)
{
	real32 temp;
	int32 value = (modff(input, &temp) >= 0.5) ? ceilf(input): floorf(input);
	return value;
}


void Fresnel::Chromatic_Adaption_xyY_XYZ ( struct colourSystem WScs,struct colourSystem WDcs,real32 xS, real32 yS, real32 YS, real32 *XD, real32 *YD,real32 *ZD)
{   real32 xWSw = WScs.xWhite;  real32 yWSw = WScs.yWhite;  //real32 zWSw = 1 - (xWSw + yWSw);
    real32 xWDw = WDcs.xWhite;  real32 yWDw = WDcs.yWhite;  //real32 zWDw = 1 - (xWDw + yWDw);
	*XD=	(YS*yWSw*(xS*(0.823628203395*xWDw*(-1.1812899339025047 + xWSw)*(-0.05738251648434092 + xWSw) - 
          	1.3877787807814457e-17*(-8.699124861174418e15 + xWSw)*(-0.05051755480259601 + xWSw)) - 
       		0.12072460893706197*(-0.05051755480259608 + xWSw)*xWSw + 0.10669937333563656*(-1.4032316397788307e-17 + xWSw)*(0.15387850925575175 + xWSw)*yS + 
       		xWDw*(-0.006098712048 - 0.01641874050741066*yS + xWSw*(0.12072460893706193 + (-0.10669937333563656 + 2.6020852139652113e-17*xWSw)*yS)) - 
       		1.3877787807814457e-17*(-7.688500127920612e15*(2.641377204289564e-17 + xWSw)*(0.1538785092557517 + xWSw) + 
          	1.*xS*(0.15387850925575178 + xWSw)*(7.688500127920612e15 + xWSw) - 
          	2.9775933729623684e16*(-1.4137295769877478 + xWSw)*(-0.12734218568174038 + xWSw)*yS)*yWDw - 
       		0.5086425272810002*(-2.7283970693523342e-17 + 0.1462552981161036*yS + 
          	xWDw*(0.4709754429917048 + 0.20977281216733645*xWSw - 1.627194399094613*yS + 0.8124057032513458*xWSw*yS) + 0.14625529811610358*yWDw + 
          	xS*(0.4709754429917048 + 0.20977281216733648*xWSw + xWDw*(-3.462741284562788 + 1.*xWSw) - 1.6271943990946132*yWDw + 
            0.8124057032513459*xWSw*yWDw) + xWSw*(-0.4386959153730082 + 0.3752189099823166*yS + 0.37521890998231666*yWDw - 1.5486428388808928*yS*yWDw))*
        	yWSw - 1.8731071356267375*(-0.039715648425587655 + xWDw*(-0.10189075201679207 + 0.420534198200273*yS) + 
          	xS*(-0.10189075201679207 + 1.*xWDw + 0.42053419820027305*yWDw) + 2.7783623952663646e-18*yS*yWDw)*pow(yWSw,2)))/
   			(yS*yWDw*(1.0296 - 0.9907*xWSw - 1.0981*yWSw)*(-0.1614 + 1.0565*xWSw + 0.4278*yWSw)*(0.0367 - 0.7869*xWSw + 1.6768*yWSw));
	*YD=	(YS*yWSw*(1.0842021724855043e-19 - 9.215718466126788e-19*xWDw - 0.035392067104733936*pow(xWSw,2) - 0.006098712048*yS + 
       		0.05582995824493805*xWSw*yS - 0.010327305070333706*pow(xWSw,2)*yS + 
       		xWDw*(-1.3877787807814454e-17*(-2.550267203596023e15 + xWSw)*xWSw + 
          	0.8236282033949998*(-1.0918928408815618 + xWSw)*(-0.1342408146610028 + xWSw)*yS) - 0.006098712048*yWDw + 0.05582995824493805*xWSw*yWDw - 
       		0.010327305070333699*pow(xWSw,2)*yWDw - 0.23955813961058928*yS*yWDw + 1.7612974783002728*xWSw*yS*yWDw - 
       		1.5132124843409431*pow(xWSw,2)*yS*yWDw + xS*(-2.0816681711721685e-17*(-1.7001781357306822e15 + xWSw)*(-1.4742634103981395e-17 + xWSw) + 
          	9.302454639925628e-17*xWDw*(-1.9505370115174856e7 + xWSw)*(1.950536907088581e7 + xWSw) + 
          	0.823628203395*(-1.0918928408815618 + xWSw)*(-0.13424081466100282 + xWSw)*yWDw) + 0.006098712048*yWSw - 
       		0.8236282033950001*(-0.0787911953775111*xWSw + 0.019934650658795485*yS + 0.12954798402461343*xWSw*yS + 
          	xWDw*(0.14657658448245758 - 0.012538794844281068*xWSw + 0.1295479840246133*yS - 1.21968863246681*xWSw*yS) + 0.019934650658795475*yWDw + 
          	0.12954798402461343*xWSw*yWDw - 1.2366169128153768*yS*yWDw + 2.775926341863634*xWSw*yS*yWDw + 
          	xS*(0.14657658448245758 - 0.012538794844281085*xWSw + xWDw*(-1.2135948606982838 + 1.*xWSw) + 0.12954798402461334*yWDw - 
            1.2196886324668101*xWSw*yWDw))*yWSw - 1.0045699570599433*
        	(-0.016344048905725866 + xWDw*(-0.1062139800078351 + 1.65775865108652e-16*yS) - 2.7629310851441998e-17*yS + 
          	xS*(-0.10621398000783511 + 1.*xWDw + 8.569403443767557e-17*yWDw) - 2.07219831385815e-17*yWDw + 0.7841222026282407*yS*yWDw)*pow(yWSw,2)))/
   			(yS*yWDw*(1.0296 - 0.9907*xWSw - 1.0981*yWSw)*(-0.1614 + 1.0565*xWSw + 0.4278*yWSw)*(0.0367 - 0.7869*xWSw + 1.6768*yWSw));
	*ZD=	(YS*yWSw*(-0.006098712048 - 0.8537615837948703*(-0.19965275829856827 + xWSw)*xWSw + 0.006098712048*yS + 
       		0.9081978887946406*(-0.1970364325835254 + xWSw)*xWSw*yS + 
       		xWDw*(0.823628203395*(-0.15400566921021636 + xWSw)*(-0.0480806367541091 + xWSw) - 
          	0.823628203395*(-0.15476962551776996 + xWSw)*(0.018776618417401986 + xWSw)*yS) + 0.006098712048*yWDw - 
       		1.0899405670333107*(-0.8332545060384786*(-0.19703643258352543 + xWSw)*xWSw + 1.*(-0.16416512972392125 + xWSw)*(-0.14355214795851942 + xWSw)*yS)*
        	yWDw + 1.598583094314311*(-0.16394242695179603 + xWDw*
           	(0.15863008692756486 + xWSw*(-0.8863101462256949 + 1.0*yS) - 0.19250598183641632*yS) + 0.17619521977195962*yS + 
          	0.17619521977195962*yWDw - 0.14438220921196554*yS*yWDw + 
          	xWSw*(0.9216057856141071 - 0.9855949767252387*yS - 0.9855949767252388*yWDw + 0.9374712041007879*yS*yWDw))*yWSw + 
       		0.7877056074240001*(0.9354381378256774 + xWDw*(-0.902336801176707 + 1.*yS) - 1.*yWDw + yS*(-1.0 + 1.0*yWDw))*
        	pow(yWSw,2) + xS*(-0.823628203395*xWDw*(-0.1636698131413516 + xWSw)*(-0.07500263724549415 + xWSw) + 
          	0.8236282033950002*(-0.15400566921021633 + xWSw)*(-0.048080636754109084 + xWSw) - 
          	0.8236282033950002*(-0.1547696255177699 + xWSw)*(0.018776618417401993 + xWSw)*yWDw + 
          	1.3322707306760004*(0.1903392226318335 + xWDw*(-0.18244489357083307 + 1.*xWSw) - 0.23098669139261652*yWDw + 
            xWSw*(-1.0634778528510713 + 1.1998935783143583*yWDw))*yWSw + 
          	0.6877481313916897*(-1.0334826452143753 + 1.*xWDw + 1.1453402364467964*yWDw)*pow(yWSw,2))))/
   			(yS*yWDw*(1.0296 - 0.9907*xWSw - 1.0981*yWSw)*(-0.1614 + 1.0565*xWSw + 0.4278*yWSw)*(0.0367 - 0.7869*xWSw + 1.6768*yWSw));
}
void Fresnel::xyY_to_RGB ( struct colourSystem WScs,struct colourSystem WDcs,real32 x, real32 y, real32 Y, real32 *R, real32 *G,real32 *B)
{   real32 xWSr = WScs.xRed;      real32 yWSr = WScs.yRed;    
    real32 xWSg = WScs.xGreen;    real32 yWSg = WScs.yGreen; 
    real32 xWSb = WScs.xBlue;     real32 yWSb = WScs.yBlue;  
    real32 xWDw = WDcs.xWhite;  	 real32 yWDw = WDcs.yWhite; 	real32 YWDw= WDcs.YWhite;
    real32 xWSw = WScs.xWhite;    real32 yWSw = WScs.yWhite; 	real32 YWSw= WScs.YWhite;

	real32 XD;real32 YD; real32 ZD;
	
	if((xWSw!=xWDw) || (yWSw!=yWDw || (YWSw!=YWDw))){Chromatic_Adaption_xyY_XYZ (WScs,WDcs,x,y,Y,&XD,&YD,&ZD);}else{XD=x*Y/y;YD=Y;ZD=(1-x-y)*Y/y;};	
		
	real32 XWSr=xWSr/yWSr; 
	real32 YWSr=1.0;
	real32 ZWSr=(1.0 - xWSr - yWSr)/yWSr;
	
	real32 XWSg=xWSg/yWSg; 
	real32 YWSg=1.0;
	real32 ZWSg=(1.0 - xWSg - yWSg)/yWSg;

	real32 XWSb=xWSb/yWSb; 
	real32 YWSb=1.0;
	real32 ZWSb=(1.0 - xWSb - yWSb)/yWSb;


	real32 r=(yWDw*(XD*(yWSb - yWSg) - xWSg*(YD*(-1 + yWSb) + yWSb*(XD + ZD)) + xWSb*(YD*(-1 + yWSg) + yWSg*(XD + ZD))))/
   			(xWSg*(yWDw - yWSb) + xWDw*(yWSb - yWSg) + xWSb*(-yWDw + yWSg));
	real32 g=(yWDw*(XD*(yWSb - yWSr) - xWSr*(YD*(-1 + yWSb) + yWSb*(XD + ZD)) + xWSb*(YD*(-1 + yWSr) + yWSr*(XD + ZD))))/
   			(xWSr*(yWDw - yWSb) + xWDw*(yWSb - yWSr) + xWSb*(-yWDw + yWSr));
	real32 b=(yWDw*(XD*(yWSg - yWSr) - xWSr*(YD*(-1 + yWSg) + yWSg*(XD + ZD)) + xWSg*(YD*(-1 + yWSr) + yWSr*(XD + ZD))))/
   			(xWSr*(yWDw - yWSg) + xWDw*(yWSg - yWSr) + xWSg*(-yWDw + yWSr));
	*R=r;
	*G=g;
	*B=b;
}
XYZColor Fresnel::RGB_to_XYZ ( struct colourSystem WScs,struct colourSystem WDcs, real32 R, real32 G,real32 B)
{   XYZColor Result;
	real32 xWSr = WScs.xRed;      real32 yWSr = WScs.yRed;    
    real32 xWSg = WScs.xGreen;    real32 yWSg = WScs.yGreen; 
    real32 xWSb = WScs.xBlue;     real32 yWSb = WScs.yBlue;  
    real32 xWDw = WDcs.xWhite;    real32 yWDw = WDcs.yWhite; 	real32 YWDw= WDcs.YWhite;
    real32 xWSw = WScs.xWhite;    real32 yWSw = WScs.yWhite; 	real32 YWSw= WScs.YWhite;

	real32 XD;real32 YD; real32 ZD;
	
		
	real32 XWSr=xWSr/yWSr; 
	real32 YWSr=1.0;
	real32 ZWSr=(1.0 - xWSr - yWSr)/yWSr;
	
	real32 XWSg=xWSg/yWSg; 
	real32 YWSg=1.0;
	real32 ZWSg=(1.0 - xWSg - yWSg)/yWSg;

	real32 XWSb=xWSb/yWSb; 
	real32 YWSb=1.0;
	real32 ZWSb=(1.0 - xWSb - yWSb)/yWSb;
    real32 xWDwyWSb=xWDw*yWSb;    real32 xWDwyWSg=xWDw*yWSg;    real32 xWDwyWSr=xWDw*yWSr;
    real32 xWSbyWDw=xWSb*yWDw;    real32 xWSbyWSg=xWSb*yWSg;    real32 xWSgyWDw=xWSg*yWDw;
    real32 xWSgyWSb=xWSg*yWSb;    real32 xWSryWDw=xWSr*yWDw;    real32 xWSryWSb=xWSr*yWSb;
    real32 xWSryWSg=xWSr*yWSg;
    real32 det=yWDw*(xWSr*(-yWSb + yWSg) + xWSg*(yWSb - yWSr) + xWSb*(-yWSg + yWSr));
	real32 X=	(R*xWSr*(xWSbyWDw - xWSgyWDw - xWDwyWSb + xWSgyWSb + xWDwyWSg - xWSbyWSg) + 
     			B*xWSb*(-(xWSryWDw) - xWDwyWSg + xWSryWSg + xWSg*(yWDw - yWSr) + xWDwyWSr) + 
     			G*xWSg*(-(xWSbyWDw) + xWSryWDw + xWDwyWSb - xWSryWSb - xWDwyWSr + xWSb*yWSr))/
   				(det);
	real32 Y=	(R*(xWSbyWDw - xWSgyWDw - xWDwyWSb + xWSgyWSb + xWDwyWSg - xWSbyWSg)*yWSr + 
     			B*yWSb*(-(xWSryWDw) - xWDwyWSg + xWSryWSg + xWSg*(yWDw - yWSr) + xWDwyWSr) + 
     			G*yWSg*(-(xWSbyWDw) + xWSryWDw + xWDwyWSb - xWSryWSb - xWDwyWSr + xWSb*yWSr))/
   				(det);
	real32 Z=	(R*(xWSgyWDw + xWDwyWSb - xWSgyWSb - xWDwyWSg + xWSb*(-yWDw + yWSg))*(-1 + xWSr + yWSr) + 
     			G*(-1 + xWSg + yWSg)*(-(xWSryWDw) - xWDwyWSb + xWSryWSb + xWSb*(yWDw - yWSr) + xWDwyWSr) + 
     			B*(-1 + xWSb + yWSb)*(xWSryWDw + xWDwyWSg - xWSryWSg - xWDwyWSr + xWSg*(-yWDw + yWSr)))/
   				(det);

	if((xWSw!=xWDw) || (yWSw!=yWDw || (YWSw!=YWDw))){Chromatic_Adaption_xyY_XYZ (WScs,WDcs,X/(X+Y+Z),Y/(X+Y+Z),Y,&XD,&YD,&ZD);}else{XD=X;YD=Y;ZD=Z;};	
	Result.X=XD;
	Result.Y=YD;
	Result.Z=ZD;
	return Result;
}

real32 Fresnel::spectral_spd(real32 lambda,  real32 temperature)
{
//D65 FresnelShadowData.illuminant 380nm to 825nm 1nm step
const real32 D65Array[512]=
	{0.489846,0.493833,0.497821,0.50181,0.505799,0.509787,0.513775,0.517764,0.521753,0.525741,0.52973,0.533718,0.548041,0.572032,
	0.596023,0.620015,0.644006,0.667997,0.691988,0.715979,0.73997,0.763962,0.787952,0.811389,0.818842,0.826294,0.833747,0.8412,
	0.848652,0.856105,0.863558,0.871011,0.878463,0.885916,0.893368,0.897634,0.899295,0.900955,0.902616,0.904277,0.905938,0.907599,
	0.90926,0.910921,0.912581,0.914242,0.915404,0.909643,0.903882,0.89812,0.892359,0.886598,0.880837,0.875076,0.869314,0.863554,
	0.857792,0.852031,0.8587,0.87422,0.88974,0.90526,0.920781,0.936301,0.951821,0.967341,0.98286,0.99838,1.0139,1.0289,1.03926,1.04963,
	1.06,1.07036,1.08072,1.09109,1.10146,1.11182,1.12218,1.13255,1.14292,1.1473,1.14799,1.14868,1.14936,1.15005,1.15073,1.15142,
	1.15211,1.15279,1.15348,1.15417,1.15442,1.1519,1.14938,1.14687,1.14435,1.14183,1.1393,1.13678,1.13427,1.13175,1.12923,1.12671,
	1.12642,1.12733,1.12824,1.12915,1.13005,1.13096,1.13186,1.13277,1.13368,1.13459,1.13549,1.13522,1.12915,1.12308,1.11701,1.11094,
	1.10487,1.0988,1.09273,1.08666,1.08058,1.07451,1.06844,1.06685,1.06732,1.06778,1.06824,1.0687,1.06916,1.06963,1.0701,1.07056,
	1.07102,1.07149,1.07159,1.07027,1.06894,1.06761,1.06629,1.06496,1.06364,1.06232,1.06099,1.05966,1.05834,1.05702,1.0548,1.05223,
	1.04966,1.04708,1.04451,1.04194,1.03937,1.0368,1.03423,1.03165,1.02909,1.0277,1.03018,1.03266,1.03513,1.03761,1.04008,1.04255,
	1.04502,1.0475,1.04997,1.05245,1.05492,1.05343,1.05062,1.04782,1.04502,1.04221,1.03941,1.03661,1.0338,1.031,1.0282,1.02539,
	1.02326,1.02296,1.02265,1.02234,1.02204,1.02173,1.02143,1.02112,1.02082,1.02051,1.0202,1.01989,1.01711,1.01366,1.01021,1.00675,
	1.0033,0.999846,0.996389,0.992939,0.989485,0.986033,0.98258,0.979223,0.976094,0.972964,0.969836,0.966707,0.963578,0.960449,
	0.957319,0.95419,0.951062,0.947933,0.944804,0.943858,0.943392,0.942925,0.942459,0.941993,0.941527,0.941061,0.940595,0.940128,
	0.939661,0.939195,0.936845,0.930783,0.92472,0.918658,0.912595,0.906532,0.90047,0.894408,0.888345,0.882283,0.87622,0.870158,
	0.870233,0.87136,0.872487,0.873615,0.874741,0.875869,0.876997,0.878123,0.879251,0.880378,0.881505,0.882086,0.881739,0.881391,
	0.881044,0.880696,0.880348,0.880001,0.879653,0.879306,0.878958,0.878611,0.878264,0.876785,0.875163,0.873541,0.871919,0.870296,
	0.868675,0.867053,0.86543,0.863808,0.862186,0.860564,0.858076,0.854312,0.850547,0.846783,0.843019,0.839254,0.835489,0.831725,
	0.827961,0.824197,0.820432,0.816668,0.816694,0.817044,0.817395,0.817745,0.818095,0.818446,0.818797,0.819147,0.819498,0.819848,
	0.820198,0.819023,0.815888,0.812753,0.809619,0.806484,0.803349,0.800215,0.79708,0.793946,0.790811,0.787676,0.784542,0.784553,
	0.784713,0.784873,0.785033,0.785194,0.785354,0.785515,0.785675,0.785835,0.785995,0.786156,0.787071,0.788832,0.790593,0.792354,
	0.794115,0.795876,0.797637,0.799398,0.801159,0.802921,0.804682,0.806443,0.803094,0.799685,0.796276,0.792868,0.789459,0.78605,
	0.782641,0.779232,0.775823,0.772415,0.769006,0.763626,0.756317,0.749008,0.741699,0.73439,0.727081,0.719772,0.712463,0.705154,
	0.697845,0.690536,0.683424,0.685035,0.686647,0.688258,0.689869,0.69148,0.693092,0.694704,0.696315,0.697926,0.699537,0.701149,
	0.703152,0.705491,0.70783,0.710169,0.712507,0.714845,0.717184,0.719523,0.721862,0.7242,0.726539,0.728141,0.717262,0.706383,
	0.695504,0.684625,0.673747,0.662868,0.651989,0.64111,0.630231,0.619353,0.608474,0.607873,0.614942,0.62201,0.62908,0.636148,
	0.643217,0.650287,0.657355,0.664424,0.671493,0.678562,0.685396,0.689835,0.694275,0.698715,0.703155,0.707595,0.712034,0.716474,
	0.720914,0.725354,0.729794,0.734233,0.730032,0.720221,0.71041,0.700599,0.690788,0.680976,0.671165,0.661354,0.651543,0.641731,
	0.63192,0.621512,0.606851,0.592192,0.577532,0.562873,0.548213,0.533553,0.518894,0.504234,0.489574,0.474914,0.460255,0.466115,
	0.483517,0.500919,0.518322,0.535724,0.553125,0.570527,0.587929,0.605331,0.622733,0.640135,0.654349,0.651427,0.648506,0.645585,
	0.642663,0.639742,0.63682,0.633899,0.630978,0.628056,0.625135,0.622213,0.62179,0.622576,0.623362,0.624149,0.624935,0.625721,
	0.626507,0.627294,0.62808,0.628867,0.629653,0.6295,0.625358,0.621217,0.617075,0.612934,0.608792,0.60465,0.600508,0.596367,
	0.592225,0.588084,0.583942,0.578206,0.57181,0.565414,0.559018,0.552623,0.546227,0.539832,0.533436,0.52704,0.520644,0.514249,
	0.510337,0.515016,0.519694,0.524373,0.529052,0.533731,0.53841,0.543089,0.547768,0.552447,0.557126,0.561805,0.564833,0.567284,
	0.569736,0.572187,0.574639,0.577089};
if ( FresnelPrivateData.BlackBody==false )
   {int32 index=floor((lambda-CIEmin)*511.0/(CIEmax-CIEmin));
	int32 indexpls1=index+1;
	if ( indexpls1>511){indexpls1=511;};
	real32 D65index=D65Array[index];real32 D65indexpls1=D65Array[indexpls1];
    real32 position=(lambda-CIEmin)*511.0/(CIEmax-CIEmin);
	real32 cie_colour_matchD65=(D65index)+((D65indexpls1-D65index))*(position-index);
    real32 Weight=(0.0188807 + 0.0188807*pow(2.718281828459,(25924.1 / temperature + 2213.52 / lambda)) - 0.0188807*pow(2.718281828459,(2213.52 / lambda)) - 0.0188807*pow(2.718281828459,(25924.1 / temperature))) / (-1 + pow(2.718281828459,(1.43879e+07 / (temperature*lambda))));
	real32 r= Weight*cie_colour_matchD65;
	return r;
	}
	else
	{
    real32 Weight=(0.0188807 + 0.0188807*pow(2.718281828459,(25924.1 / temperature + 2213.52 / lambda)) - 0.0188807*pow(2.718281828459,(2213.52 / lambda)) - 0.0188807*pow(2.718281828459,(25924.1 / temperature))) / (-1 + pow(2.718281828459,(1.43879e+07 / (temperature*lambda))));
	return Weight;
	}
}

void Fresnel::lambda_to_xyzXYZ(real32 *x, real32 *y, real32 *z, real32 *X, real32 *Y,real32 *Z,real32 lambda,real32 temperature)
{
    real32 XBars = 0, YBars = 0, ZBars = 0, XYZBars= 0 ;

const real32 XBar[512] = 
	{0.0026899,0.0030544,0.00343616,0.00385243,0.00432045,0.0048575,0.00548077,0.00620326,0.00702995,0.00796489,0.00901217,0.0101759,
	0.0114604,0.012878,0.0144485,0.0161919,0.0181283,0.0202776,0.0226584,0.0252763,0.0281305,0.0312204,0.0345454,0.0381048,0.0418996,
	0.0459367,0.0502243,0.0547706,0.0595839,0.0646728,0.0700712,0.0758573,0.0821135,0.0889224,0.0963665,0.104525,0.113424,0.12304,
	0.133349,0.144326,0.155949,0.168171,0.1808,0.193579,0.206251,0.218558,0.230243,0.241105,0.251127,0.26033,0.268736,0.276365,
	0.283238,0.28941,0.294982,0.300062,0.304756,0.309171,0.313408,0.317469,0.321275,0.324747,0.327805,0.330367,0.332362,0.333761,
	0.334556,0.334738,0.334296,0.333222,0.331521,0.329232,0.326406,0.32309,0.319334,0.315187,0.310705,0.305955,0.301006,0.295927,
	0.290786,0.285648,0.280539,0.275456,0.270393,0.265346,0.260312,0.255291,0.250329,0.245482,0.24081,0.236369,0.232219,0.228363,
	0.22466,0.220942,0.217045,0.212803,0.208051,0.202728,0.196902,0.190653,0.184058,0.177198,0.170148,0.16296,0.15567,0.148312,
	0.140922,0.133533,0.126179,0.118887,0.11168,0.104582,0.0976169,0.0908085,0.0841818,0.077765,0.0715865,0.065675,0.060059,0.0547663,
	0.0498042,0.0451547,0.0407981,0.0367152,0.0328862,0.0292928,0.0259277,0.0227906,0.0198811,0.0171991,0.0147442,0.0125156,
	0.01051,0.008723,0.00715043,0.00578805,0.00463165,0.00368206,0.00295123,0.00245261,0.00219967,0.00220587,0.00248479,0.00305393,
	0.00393512,0.00515037,0.00672173,0.00867124,0.0110202,0.0137827,0.016969,0.0205891,0.0246531,0.0291712,0.0341495,0.0395777,0.0454413,
	0.0517253,0.0584153,0.0654963,0.0729536,0.0807719,0.0889359,0.0974305,0.10624,0.11535,0.124735,0.134364,0.144205,0.154224,
	0.164389,0.17467,0.18506,0.195564,0.206186,0.216931,0.227803,0.238806,0.249943,0.261215,0.272623,0.284169,0.295855,0.307681,
	0.319646,0.331749,0.343988,0.356363,0.368873,0.381518,0.394301,0.407225,0.420294,0.433509,0.446874,0.460385,0.474036,0.48782,
	0.501731,0.515763,0.529909,0.544156,0.558491,0.5729,0.587371,0.601889,0.616442,0.631022,0.645617,0.660219,0.674817,0.689401,
	0.703961,0.718482,0.732952,0.747357,0.761684,0.775918,0.790041,0.804031,0.817867,0.831528,0.844991,0.858236,0.871236,0.883965,
	0.896399,0.908511,0.920275,0.931669,0.942677,0.953281,0.963466,0.973214,0.982511,0.991347,0.99972,1.00762,1.01506,1.02202,1.0285,
	1.03445,1.0398,1.04449,1.04845,1.05161,1.05395,1.05548,1.05625,1.05631,1.05571,1.05447,1.05262,1.05011,1.0469,1.04293,1.03818,
	1.03258,1.02616,1.01899,1.0111,1.00257,0.993449,0.983787,0.973576,0.962791,0.951403,0.939388,0.926717,0.913374,0.899357,0.884671,
	0.86932,0.853308,0.83664,0.819361,0.801572,0.783374,0.764871,0.746164,0.727354,0.708522,0.689734,0.671059,0.652563,0.634313,
	0.616364,0.598709,0.581323,0.564179,0.547251,0.530514,0.513948,0.497555,0.481338,0.465301,0.449446,0.433777,0.418307,0.403057,
	0.38805,0.373308,0.358855,0.344711,0.330894,0.317417,0.304295,0.29154,0.279167,0.267185,0.255584,0.244349,0.233466,0.222919,
	0.212693,0.202777,0.193164,0.183851,0.174833,0.166104,0.157661,0.149507,0.141652,0.134109,0.126887,0.119999,0.113454,0.107254,
	0.101394,0.0958686,0.0906742,0.0858059,0.0812549,0.076995,0.0729954,0.0692251,0.0656534,0.0622496,0.0589894,0.0558617,0.0528568,
	0.0499652,0.0471771,0.0444833,0.0418825,0.0393815,0.0369874,0.0347072,0.0325481,0.0305161,0.0286103,0.026826,0.0251586,0.0236033,
	0.0221556,0.0208102,0.0195598,0.0183965,0.0173126,0.0163002,0.0153515,0.0144604,0.0136232,0.0128369,0.0120981,0.0114038,
	0.0107507,0.0101361,0.00955781,0.00901347,0.00850083,0.00801765,0.0075617,0.00713127,0.00672483,0.00634086,0.00597785,0.00563429,
	0.00530881,0.00500066,0.00470918,0.00443372,0.00417362,0.00392825,0.00369695,0.00347906,0.00327395,0.00308097,0.00289946,0.00272878,
	0.00256829,0.00241733,0.00227527,0.00214146,0.00201527,0.00189606,0.00178342,0.001677,0.00157644,0.0014814,0.00139152,0.0013065,
	0.00122614,0.0011503,0.0010788,0.00101149,0.000948204,0.000888771,0.000833,0.000780699,0.000731674,0.000685734,0.000642687,0.000602367,
	0.000564627,0.00052932,0.0004963,0.000465418,0.000436533,0.000409531,0.000384308,0.000360759,0.000338782,0.000318271,0.000299126,
	0.000281252,0.000264554,0.00024894,0.000234314,0.000220585,0.000207676,0.000195535,0.000184114,0.000173362,0.000163231,0.000153671,
	0.000144648,0.000136138,0.000128117,0.000120561,0.000113446,0.000106748,0.000100444,9.4515e-05,8.89381e-05,8.36922e-05,7.87562e-05,
	7.41103e-05,6.97385e-05,6.56255e-05,6.17561e-05,5.81149e-05,5.46867e-05,5.14583e-05,4.84192e-05,4.5559e-05,4.28674e-05,
	4.0334e-05,3.79487e-05,3.57028e-05,3.35891e-05,3.16002e-05,2.97286e-05,2.7967e-05,2.63083e-05,2.47468e-05,2.32774e-05,2.18947e-05,
	2.05935e-05,1.93687e-05,1.82154e-05,1.71299e-05,1.61087e-05,1.51481e-05,1.42445e-05,1.33943e-05,1.25945e-05,1.18423e-05,
	1.11351e-05,1.04704e-05,9.84544e-06,9.25769e-06,8.70499e-06,8.18538e-06,7.69695e-06,7.23777e-06,6.8059e-06,6.39953e-06,6.0173e-06,
	5.65798e-06,5.32036e-06,5.00319e-06,4.70526e-06,4.42537e-06,4.16238e-06,3.91514e-06,3.68251e-06,3.46336e-06,3.25656e-06,
	3.06139e-06,2.87753e-06,2.7047e-06,2.5426e-06,2.39093e-06,2.24932e-06,2.11647e-06,1.99064e-06,1.87007e-06,1.75301e-06,1.6377e-06};
const real32 YBar[512] = 
	{0.0002,0.000227489,0.000256211,0.0002874,0.00032229,0.000362114,0.000408103,0.000461239,0.000522028,0.000590925,0.000668383,
	0.000754856,0.000850814,0.000957066,0.00107475,0.001205,0.00134899,0.00150784,0.00168264,0.00187392,0.00208194,0.00230696,0.00254924,
	0.00280905,0.00308672,0.00338296,0.00369854,0.00403422,0.00439077,0.00476899,0.00517135,0.0056033,0.00607054,0.0065788,0.00713381,
	0.00774113,0.00840342,0.00912081,0.00989335,0.0107211,0.0116041,0.012541,0.0135208,0.0145285,0.0155488,0.0165666,0.0175665,0.0185368,
	0.0194762,0.0203857,0.0212661,0.0221186,0.0229441,0.0237473,0.0245386,0.0253288,0.026129,0.02695,0.0278021,0.0286868,0.0295982,
	0.0305305,0.0314774,0.0324332,0.0333917,0.0343463,0.0352899,0.0362158,0.0371168,0.0379863,0.0388201,0.0396233,0.0404022,
	0.0411634,0.0419135,0.0426589,0.0434053,0.0441572,0.044919,0.0456953,0.0464904,0.0473088,0.0481567,0.0490414,0.0499702,0.0509503,
	0.0519892,0.0530953,0.0542844,0.0555751,0.0569855,0.0585343,0.0602397,0.0621155,0.0641622,0.0663784,0.0687627,0.0713136,
	0.0740296,0.076905,0.0799286,0.0830886,0.0863736,0.0897719,0.0932727,0.0968763,0.10059,0.104421,0.108377,0.112466,0.116693,0.121059,
	0.12556,0.130195,0.13496,0.139854,0.144877,0.150042,0.155364,0.160855,0.16653,0.172403,0.178495,0.184836,0.191456,0.198387,
	0.205657,0.213298,0.221326,0.229752,0.238586,0.24784,0.257523,0.267648,0.278233,0.289299,0.300869,0.312963,0.325601,0.338796,
	0.352539,0.366817,0.381619,0.396933,0.412746,0.42903,0.44574,0.462832,0.48026,0.497977,0.515939,0.534097,0.552401,0.570802,
	0.58925,0.607696,0.626082,0.644321,0.662317,0.679974,0.697195,0.713885,0.729979,0.745473,0.76037,0.774674,0.788389,0.801518,
	0.814082,0.826118,0.837662,0.848751,0.859422,0.86971,0.879615,0.889125,0.898225,0.906901,0.91514,0.922932,0.930283,0.937205,
	0.943707,0.949801,0.955497,0.960806,0.965736,0.970296,0.974496,0.978344,0.981849,0.985022,0.98787,0.990406,0.992637,0.994574,
	0.996225,0.99759,0.998665,0.999444,0.999925,1.000000,0.999969,0.999521,0.99875,0.997648,0.996207,0.994421,0.992286,0.989808,
	0.986993,0.983846,0.980373,0.97658,0.972473,0.968055,0.963332,0.958308,0.952988,0.947375,0.941473,0.935284,0.92881,0.922053,
	0.915016,0.907701,0.900111,0.892247,0.884114,0.875714,0.86705,0.85813,0.848971,0.83959,0.830005,0.820232,0.81029,0.800193,
	0.789957,0.779595,0.769122,0.758551,0.747897,0.737167,0.726366,0.7155,0.704574,0.693594,0.682564,0.671489,0.660375,0.649226,
	0.638047,0.626843,0.61562,0.604385,0.593148,0.581915,0.570695,0.559496,0.548329,0.537206,0.526139,0.51514,0.504221,0.493393,
	0.482645,0.471965,0.461336,0.450745,0.440177,0.429619,0.419063,0.4085,0.397923,0.387325,0.376698,0.366052,0.355422,0.344843,
	0.334351,0.323982,0.313769,0.303735,0.293898,0.28427,0.274869,0.26571,0.256803,0.24814,0.239707,0.231486,0.223463,0.215623,
	0.207954,0.200449,0.193106,0.18592,0.178886,0.172001,0.165266,0.158685,0.152265,0.146011,0.13993,0.134026,0.128303,0.12276,
	0.117398,0.112218,0.10722,0.102404,0.097763,0.0932902,0.0889783,0.0848198,0.0808073,0.0769341,0.0731951,0.069585,0.066099,0.0627319,
	0.0594788,0.0563395,0.0533186,0.0504209,0.0476513,0.0450146,0.0425153,0.040153,0.0379251,0.0358288,0.0338613,0.03202,0.0303005,
	0.0286927,0.0271849,0.0257652,0.0244219,0.0231433,0.0219202,0.0207482,0.0196235,0.0185424,0.0175012,0.0164962,0.0155268,
	0.0145955,0.0137046,0.0128567,0.0120544,0.0112998,0.0105924,0.00993034,0.00931188,0.00873516,0.00819836,0.00769946,0.00723577,
	0.00680442,0.00640254,0.00602729,0.00567581,0.00534578,0.00503591,0.00474497,0.00447175,0.00421504,0.00397363,0.00374651,0.00353282,
	0.00333173,0.00314238,0.00296394,0.00279558,0.00263666,0.00248661,0.00234487,0.00221087,0.00208406,0.00196392,0.00185017,0.00174257,
	0.00164088,0.00154486,0.00145427,0.00136887,0.00128842,0.00121269,0.00114142,0.00107439,0.00101135,0.000952073,0.000896311,0.000843828,
	0.000794388,0.000747751,0.00070369,0.000662048,0.000622695,0.000585502,0.000550342,0.000517085,0.000485618,0.000455872,0.000427789,
	0.00040131,0.000376376,0.000352927,0.000330901,0.000310226,0.000290833,0.00027265,0.000255606,0.000239631,0.000224665,0.000210653,0.00019754,
	0.000185274,0.000173799,0.000163065,0.000153028,0.00014365,0.000134892,0.000126717,0.000119085,0.000111959,0.000105305,9.90862e-05,
	9.32695e-05,8.78196e-05,8.27021e-05,7.7889e-05,7.33613e-05,6.91006e-05,6.50886e-05,6.13069e-05,5.77376e-05,5.4368e-05,5.1189e-05,
	4.81917e-05,4.53671e-05,4.27063e-05,4.02004e-05,3.78413e-05,3.56212e-05,3.35322e-05,3.15665e-05,2.97163e-05,2.79742e-05,
	2.63344e-05,2.47912e-05,2.33388e-05,2.19716e-05,2.0684e-05,1.9471e-05,1.83287e-05,1.72532e-05,1.62408e-05,1.52875e-05,1.43895e-05,
	1.35437e-05,1.27474e-05,1.19977e-05,1.12919e-05,1.06274e-05,1.00014e-05,9.41185e-06,8.85681e-06,8.33433e-06,7.84245e-06,
	7.37925e-06,6.94293e-06,6.53209e-06,6.1454e-06,5.78151e-06,5.43907e-06,5.11674e-06,4.81335e-06,4.5279e-06,4.25943e-06,4.00695e-06,
	3.76948e-06,3.54606e-06,3.33588e-06,3.13821e-06,2.95232e-06,2.77748e-06,2.61297e-06,2.45811e-06,2.31237e-06,2.1753e-06,
	2.04645e-06,1.92535e-06,1.81154e-06,1.70457e-06,1.60401e-06,1.50943e-06,1.4204e-06,1.3365e-06,1.25729e-06,1.18251e-06,1.11204e-06,
	1.04576e-06,9.83559e-07,9.25334e-07,8.70934e-07,8.1987e-07,7.71476e-07,7.25085e-07,6.80028e-07,6.3564e-07};
const real32 ZBar [512]= 
	{0.01226,0.0139222,0.0156634,0.0175627,0.0196993,0.0221521,0.0250002,0.0283029,0.0320834,0.0363604,0.0411528,0.0464794,0.0523609,
	0.0588537,0.0660491,0.0740395,0.0829174,0.0927754,0.103699,0.115714,0.128819,0.143013,0.158294,0.174659,0.192116,0.2107,0.230452,
	0.251414,0.273627,0.297135,0.322097,0.348877,0.377856,0.409418,0.443945,0.481806,0.523123,0.567802,0.615739,0.666832,0.720978,
	0.777983,0.836972,0.896779,0.956234,1.01417,1.06942,1.12106,1.169,1.21332,1.25409,1.29138,1.32528,1.35602,1.38411,1.41007,
	1.43443,1.45771,1.48041,1.50254,1.52369,1.54348,1.56149,1.57733,1.59063,1.60127,1.6092,1.61439,1.61682,1.61643,1.61326,1.60751,
	1.59938,1.58911,1.57691,1.56303,1.5477,1.53127,1.51404,1.49634,1.47849,1.4608,1.4434,1.42624,1.40932,1.39259,1.37604,1.35966,
	1.34371,1.32848,1.31429,1.30146,1.2903,1.28084,1.2723,1.26383,1.25452,1.2435,1.22989,1.21331,1.19403,1.17236,1.14859,1.12306,
	1.09604,1.06777,1.03838,1.00804,0.976889,0.945086,0.912785,0.880177,0.847459,0.81483,0.782489,0.750634,0.719423,0.688918,0.659166,
	0.630213,0.602107,0.574893,0.548611,0.523295,0.498976,0.475686,0.453458,0.43232,0.41226,0.393239,0.37522,0.358166,0.342039,
	0.326799,0.312395,0.298774,0.285882,0.273666,0.262073,0.251041,0.240497,0.230362,0.220559,0.211011,0.201642,0.192411,0.183317,
	0.174361,0.165543,0.156863,0.148326,0.139966,0.131835,0.123985,0.116468,0.109335,0.102631,0.0963609,0.090521,0.0851074,0.0801162,
	0.0755435,0.071371,0.067552,0.0640362,0.0607733,0.0577131,0.0548062,0.0520228,0.0493535,0.0467893,0.0443217,0.0419418,0.0396416,
	0.0374192,0.0352759,0.0332128,0.0312311,0.0293321,0.0275167,0.0257844,0.0241343,0.0225658,0.021078,0.0196702,0.0183411,0.0170889,
	0.0159118,0.0148079,0.0137752,0.0128117,0.0119148,0.0110807,0.0103057,0.0095862,0.00891849,0.008299,0.00772506,0.0071944,
	0.00670475,0.00625385,0.00583943,0.00545913,0.00511032,0.00479026,0.00449624,0.00422554,0.00397545,0.00374397,0.00353029,0.0033337,
	0.00315351,0.00298902,0.00283951,0.00270428,0.00258259,0.00247369,0.00237685,0.00229133,0.00221638,0.0021511,0.00209456,0.00204582,
	0.00200393,0.00196796,0.00193681,0.00190888,0.00188246,0.00185585,0.00182736,0.00179529,0.00175864,0.00171745,0.00167185,0.00162196,
	0.00156791,0.00150993,0.00144964,0.00138976,0.00133304,0.00128222,0.00124007,0.00120867,0.00118612,0.00116894,0.00115365,0.00113679,
	0.00111488,0.0010857,0.00105064,0.00101169,0.000970888,0.000930236,0.000891721,0.000855922,0.000821447,0.000786754,0.000750304,
	0.000710556,0.000666131,0.000617824,0.000567974,0.000518957,0.000473147,0.000432917,0.000400214,0.000374578,0.000354711,
	0.000339312,0.000327083,0.000316724,0.00030712,0.000297639,0.000287723,0.000276815,0.000264361,0.000249817,0.000233216,0.000215326,
	0.000196961,0.000178937,0.000162071,0.000147126,0.000134247,0.00012318,0.000113663,0.000105432,9.82251e-05,9.18047e-05,8.60565e-05,
	8.09053e-05,7.6276e-05,7.20936e-05,6.8283e-05,6.47755e-05,6.15175e-05,5.84575e-05,5.55441e-05,5.27258e-05,4.99522e-05,
	4.72026e-05,4.44914e-05,4.18352e-05,3.92504e-05,3.67536e-05,3.43607e-05,3.20835e-05,2.9931e-05,2.79122e-05,2.60361e-05,2.43118e-05,
	2.27443e-05,2.13205e-05,2.00224e-05,1.88318e-05,1.77304e-05,1.67001e-05,1.57269e-05,1.48054e-05,1.39315e-05,1.31009e-05,
	1.23095e-05,1.15533e-05,1.08305e-05,1.01423e-05,9.49e-06,8.87474e-06,8.29777e-06,7.76011e-06,7.26075e-06,6.79761e-06,
	6.3686e-06,5.97162e-06,5.60456e-06,5.26527e-06,4.95125e-06,4.65995e-06,4.38879e-06,4.13523e-06,3.8967e-06,3.67115e-06,3.45753e-06,
	3.25489e-06,3.0623e-06,2.87883e-06,2.70355e-06,2.5361e-06,2.37663e-06,2.22534e-06,2.08242e-06,1.94804e-06,1.82235e-06,1.7051e-06,
	1.59585e-06,1.49414e-06,1.39954e-06,1.31159e-06,1.22987e-06,1.15393e-06,1.08335e-06,1.01772e-06,9.56593e-07,8.99557e-07,
	8.4625e-07,7.96428e-07,7.49861e-07,7.06318e-07,6.65567e-07,6.2738e-07,5.91566e-07,5.57971e-07,5.2644e-07,4.96819e-07,4.68956e-07,
	4.427e-07,4.1794e-07,3.94581e-07,3.72528e-07,3.51686e-07,3.3196e-07,3.13267e-07,2.95561e-07,2.78806e-07,2.62964e-07,2.47999e-07,
	2.33875e-07,2.20552e-07,2.07996e-07,1.96168e-07,1.8503e-07,1.74546e-07,1.64679e-07,1.5539e-07,1.46643e-07,1.38398e-07,1.3062e-07,
	1.23269e-07,1.16311e-07,1.0972e-07,1.03477e-07,9.75628e-08,9.19571e-08,8.66404e-08,8.15957e-08,7.68137e-08,7.22863e-08,
	6.80057e-08,6.39641e-08,6.01534e-08,5.65648e-08,5.31876e-08,5.00114e-08,4.70253e-08,4.42188e-08,4.15813e-08,3.91037e-08,
	3.67777e-08,3.45953e-08,3.25483e-08,3.06285e-08,2.88282e-08,2.71407e-08,2.55599e-08,2.408e-08,2.26948e-08,2.13984e-08,2.01847e-08,
	1.90482e-08,1.79832e-08,1.69841e-08,1.60454e-08,1.51614e-08,1.43277e-08,1.3541e-08,1.27985e-08,1.20971e-08,1.14338e-08,
	1.08056e-08,1.02106e-08,9.64725e-09,9.11431e-09,8.61035e-09,8.13399e-09,7.68386e-09,7.25868e-09,6.85717e-09,6.47808e-09,
	6.12014e-09,5.78209e-09,5.46274e-09,5.16111e-09,4.87624e-09,4.60719e-09,4.35301e-09,4.11275e-09,3.88558e-09,3.67088e-09,
	3.468e-09,3.2763e-09,3.09515e-09,2.92393e-09,2.76209e-09,2.60917e-09,2.46468e-09,2.32816e-09,2.19914e-09,2.07716e-09,1.96184e-09,
	1.85287e-09,1.7499e-09,1.6526e-09,1.56063e-09,1.47368e-09,1.39151e-09,1.31387e-09,1.24054e-09,1.17128e-09,1.10585e-09,
	1.04404e-09,9.8568e-10,9.30581e-10,8.7857e-10,8.2947e-10,7.83106e-10,7.39328e-10,6.98003e-10,6.58995e-10,6.22172e-10,5.87398e-10,
	5.54547e-10,5.23524e-10,4.94242e-10,4.66615e-10,4.40557e-10,4.15982e-10,3.92804e-10,3.70936e-10,3.50294e-10,3.3079e-10,
	3.12341e-10,2.9486e-10,2.78296e-10,2.62625e-10,2.47829e-10,2.33889e-10,2.20783e-10,2.08485e-10,1.96893e-10,1.85866e-10,1.75262e-10,
	1.6494e-10,1.5476e-10};

	int32 index=floor((lambda-CIEmin)*511.0/(CIEmax-CIEmin));real32 spd;
	int32 indexpls1=index+1;
	if ( indexpls1>511){indexpls1=511;};
	real32 XBarCMindex=XBar[index];real32 XBarCMindexpls1=XBar[indexpls1];
	real32 YBarCMindex=YBar[index];real32 YBarCMindexpls1=YBar[indexpls1];
	real32 ZBarCMindex=ZBar[index];real32 ZBarCMindexpls1=ZBar[indexpls1];
    real32 position=(lambda-CIEmin)*511.0/(CIEmax-CIEmin);
	real32 cie_colour_matchXBar=(XBarCMindex)+((XBarCMindexpls1-XBarCMindex))*(position-index);
	real32 cie_colour_matchYBar=(YBarCMindex)+((YBarCMindexpls1-YBarCMindex))*(position-index);
	real32 cie_colour_matchZBar=(ZBarCMindex)+((ZBarCMindexpls1-ZBarCMindex))*(position-index);
	spd=spectral_spd(lambda,temperature);
	
	XBars = cie_colour_matchXBar * spd;
	YBars = cie_colour_matchYBar * spd;
	ZBars = cie_colour_matchZBar * spd;
    *X = XBars;
	*Y = YBars;
	*Z = ZBars;
	XYZBars = (XBars + YBars + ZBars);
    *x = (XBars / XYZBars);
    *y = (YBars / XYZBars);
	*z = (ZBars / XYZBars);
	}
int32 Fresnel::constrain_rgb(real32 *r, real32 *g, real32 *b)
{
    real32 w;
    /* Amount of white needed is w = - min(0, *r, *g, *b) */
    w = (0 < *r) ? 0 : *r;
    w = (w < *g) ? w : *g;
    w = (w < *b) ? w : *b;
    w = -w;
    /* Add just enough white to make r, g, b all positive. */
    if (w > 0 || *r>1||*g>1||*b>1) {
        *r += w;  *g += w; *b += w;
        if (*r>1) {*r=*r/(*r);*g=*g/(*r);*b=*b/(*r);};
        if (*g>1) {*r=*r/(*g);*g=*g/(*g);*b=*b/(*g);};
        if (*b>1) {*r=*r/(*b);*g=*g/(*b);*b=*b/(*b);};
        return 1;                     /* Colour modified to fit RGB gamut */
    }
    return 0;                         /* Colour within RGB gamut */
}
void Fresnel::norm_rgb(real32 *r, real32 *g, real32 *b)
{
#define Max(a, b)   (((a) > (b)) ? (a) : (b))
    real32 greatest = Max(*r, Max(*g, *b));
    
    if (greatest > 0) {
    	*r /= greatest;
	*g /= greatest;
	*b /= greatest;
    }
#undef Max
}

void Fresnel::gamma_correct(struct colourSystem cs, real32 *c)
{
    real32 gamma;
    gamma = cs.gamma;
    if (gamma == GAMMA_REC709) 
    {
	/* Rec. 709 gamma correction. */
	real32 cc = 0.018;	
	if ( *c < cc ) 
		{
	    *c *= ((1.099 * pow(real32(cc), real32(0.45))) - 0.099) / cc;
		} 
	else 
		{
	    *c = (1.099 * pow(real32(*c), real32(0.45))) - 0.099;
		};
	};
    if ( gamma==sRGB ) 
    {	if ( *c <= 0.0031308)
    		{
    			*c=12.92 * real32(*c);
    		}
    		else
    		{
    			*c=( 1.055 * pow( real32(*c), real32(1/2.4) ) )- 0.055;
    		};
    }
    else 
    {
		/* Nonlinear colour = (Linear colour)^(1/gamma) */
		if ( *c<0 ) 
		{
		*c=-pow(real32(-*c), real32(1.0/gamma));
		}
		else
		{
		*c=pow(real32(*c),real32(1.0/gamma));
		};
	};
}

void Fresnel::gamma_correct_rgb(struct colourSystem cs, real32 *r, real32 *g, real32 *b)
{
    gamma_correct(cs, r);
    gamma_correct(cs, g);
    gamma_correct(cs, b);
}
	
struct colourSystem Fresnel::whitepoint (struct colourSystem cs, real32 temperature)
{	colourSystem r=cs;
	real32 x=0.0;real32 y=0.0;
	real32 Xsum=0.0; real32 Ysum=0.0; real32 Zsum=0.0; real32 lambda; 
	real32 z;real32 X=0.0;real32 Y=0.0;real32 Z=0.0;real32 XYZ=0.0;
        for ( lambda=CIEmin;lambda<=CIEmax;lambda=lambda+step)
		{lambda_to_xyzXYZ(&x, &y, &z, &X, &Y ,&Z, lambda,temperature);
		Xsum +=X;
        Ysum +=Y;
        Zsum +=Z;
		};
	XYZ = (Xsum + Ysum + Zsum);
    x = Xsum / XYZ;
    y = Ysum / XYZ;
    z = Zsum / XYZ;
	r.xWhite=x;r.yWhite=y;
	return r;
}

void Fresnel::Calc_Spectral_Color ( struct colourSystem WScs,struct colourSystem WDcs, real32 *R, real32 *G,real32 *B, real32 *L, real32 *a, real32 *b,real32 lambda, real32 temperature,real32 GLf,int32 Subdivides)
{   real32 x;real32 y;real32 z;real32 X;real32 Y;real32 Z; real32 rr,gg,bb;
	LabParameter Projected;
	lambda_to_xyzXYZ(&x, &y, &z, &X, &Y ,&Z, lambda,temperature);
    XYZSpectralColor Spectral; FresnelPrivateData.SolutionFound=false;FresnelPrivateData.partitioncount=0;Projected.Plane=0;
    Chromatic_Adaption_xyY_XYZ (WScs,WDcs,X/(X+Y+Z),Y/(X+Y+Z),Y,&Spectral.SX,&Spectral.SY,&Spectral.SZ);
	Projected=Lab_Gamut_Project ( WDcs,Spectral,GLf,0.,0.,Subdivides);
	if ( FresnelPrivateData.SolutionFound==false ) 
		{ 	lambda=lambda-0.01;
			lambda_to_xyzXYZ(&x, &y, &z, &X, &Y ,&Z, lambda,temperature);
		    XYZSpectralColor Spectral; FresnelPrivateData.SolutionFound=false;FresnelPrivateData.partitioncount=0;Projected.Plane=0;
		    Chromatic_Adaption_xyY_XYZ (WScs,WDcs,X/(X+Y+Z),Y/(X+Y+Z),Y,&Spectral.SX,&Spectral.SY,&Spectral.SZ);
			Projected=Lab_Gamut_Project ( WDcs,Spectral,GLf,0.,0.,Subdivides);
		};
	if ( Projected.Plane==KRB ) {*R=Projected.u;*B=Projected.v;*G=0.0;};
	if ( Projected.Plane==KBG ) {*B=Projected.u;*G=Projected.v;*R=0.0;};
	if ( Projected.Plane==KGR ) {*G=Projected.u;*R=Projected.v;*B=0.0;};
	if ( Projected.Plane==GCY ) {*G=1.0;*B=Projected.u;*R=Projected.v;};
	if ( Projected.Plane==RYM ) {*R=1.0;*G=Projected.u;*B=Projected.v;};
	if ( Projected.Plane==BMC ) {*B=1.0;*R=Projected.u;*G=Projected.v;};
	rr=*R;
	gg=*G;
	bb=*B;
		real32 blend;
		TMCColorRGBA S;
		TMCColorRGBA SV;
		TMCColorRGBA Sp;
		LabColor Lab; 
		XYZColor XYZ;
	    XYZReferenceWhite WhitePnt;
		SV.R=*R;
		SV.G=*G;
		SV.B=*B;
		Lab.L=Projected.L;
		Lab.a=Projected.a;
		Lab.b=Projected.b;
		if( Lab.L > CSt && Lab.L <= CEnd || FresnelPrivateData.SolutionFound==false )
			{
			Lab.a=Lab.a+aCorrection; 
			Lab.b=Lab.b+bCorrection;//Correct for Yellow/Green Bias.
			WhitePnt.Xr=FresnelPublicData.xWhite*1.0/FresnelPublicData.yWhite;
			WhitePnt.Yr=1.0;
			WhitePnt.Zr=(( 1.0 - FresnelPublicData.xWhite - FresnelPublicData.yWhite ) * 1.0)/FresnelPublicData.yWhite;
			XYZ=Lab2XYZ(Lab,WhitePnt);
			xyY_to_RGB (FresnelPrivateData.Monitor_ColorSystem,FresnelPrivateData.Monitor_ColorSystem,XYZ.X/(XYZ.X+XYZ.Y+XYZ.Z),XYZ.Y/(XYZ.X+XYZ.Y+XYZ.Z),XYZ.Y, &*R , &*G , &*B);
			};		
		if ( Lab.L >= BCSt && Lab.L <= CSt && FresnelPrivateData.SolutionFound==true)
			{
			blend=(Lab.L-BCSt)/(CSt-BCSt);
				
			Lab.a=Lab.a+aCorrection;
			Lab.b=Lab.b+bCorrection;
			WhitePnt.Xr=FresnelPublicData.xWhite*1.0/FresnelPublicData.yWhite;
			WhitePnt.Yr=1.0;
			WhitePnt.Zr=(( 1.0 - FresnelPublicData.xWhite - FresnelPublicData.yWhite ) * 1.0)/FresnelPublicData.yWhite;
			XYZ=Lab2XYZ(Lab,WhitePnt);
			xyY_to_RGB (FresnelPrivateData.Monitor_ColorSystem,FresnelPrivateData.Monitor_ColorSystem,XYZ.X/(XYZ.X+XYZ.Y+XYZ.Z),XYZ.Y/(XYZ.X+XYZ.Y+XYZ.Z),XYZ.Y, &*R , &*G , &*B);
			*R=(( blend ) * *R) + (( 1.0 - blend ) * SV.R);					
			*G=(( blend ) * *G) + (( 1.0 - blend ) * SV.G);					
			*B=(( blend ) * *B) + (( 1.0 - blend ) * SV.B);					
				}
		if ( Lab.L >= NCSt && Lab.L <  BCSt && FresnelPrivateData.SolutionFound==true)			    
			{
			*R=SV.R;					
			*G=SV.G;					
			*B=SV.B;					
			};
	FresnelPrivateData.SpectralRGB.R=*R;
	FresnelPrivateData.SpectralRGB.G=*G;
	FresnelPrivateData.SpectralRGB.B=*B;

	FresnelPrivateData.SpectralLab.L=Lab.L;
	FresnelPrivateData.SpectralLab.a=Lab.a;
	FresnelPrivateData.SpectralLab.b=Lab.b;
	*L=Lab.L;
	*a=Lab.a;
	*b=Lab.b;
	*R=rr;					
	*G=gg;					
	*B=bb;					
}


int32 Fresnel::XYZtoCorColorTemp(real32 X,real32 Y, real32 Z, real32 *temp)
{
/* LERP(a,b,c) = linear interpolation macro, is 'a' when c == 0.0 and 'b' when c == 1.0 */
#define LERP(a,b,c)     (((b) - (a)) * (c) + (a))

typedef struct UVT {
        real32  u;
        real32  v;
        real32  t;
} UVT;
//#define FLT_MIN 1.0e-37
real32 rt[31] = {       /* reciprocal temperature (K) */
        FLT_MIN,   10.0e-6,  20.0e-6,  30.0e-6,  40.0e-6,  50.0e-6,
         60.0e-6,  70.0e-6,  80.0e-6,  90.0e-6, 100.0e-6, 125.0e-6,
        150.0e-6, 175.0e-6, 200.0e-6, 225.0e-6, 250.0e-6, 275.0e-6,
        300.0e-6, 325.0e-6, 350.0e-6, 375.0e-6, 400.0e-6, 425.0e-6,
        450.0e-6, 475.0e-6, 500.0e-6, 525.0e-6, 550.0e-6, 575.0e-6,
        600.0e-6
};

UVT uvt[31] = {
        {0.18006, 0.26352, -0.24341},        {0.18066, 0.26589, -0.25479},        {0.18133, 0.26846, -0.26876},
        {0.18208, 0.27119, -0.28539},        {0.18293, 0.27407, -0.30470},        {0.18388, 0.27709, -0.32675},
        {0.18494, 0.28021, -0.35156},        {0.18611, 0.28342, -0.37915},        {0.18740, 0.28668, -0.40955},
        {0.18880, 0.28997, -0.44278},        {0.19032, 0.29326, -0.47888},        {0.19462, 0.30141, -0.58204},
        {0.19962, 0.30921, -0.70471},        {0.20525, 0.31647, -0.84901},        {0.21142, 0.32312, -1.0182},
        {0.21807, 0.32909, -1.2168},         {0.22511, 0.33439, -1.4512},         {0.23247, 0.33904, -1.7298},
        {0.24010, 0.34308, -2.0637},         {0.24702, 0.34655, -2.4681},         {0.25591, 0.34951, -2.9641},        
        {0.26400, 0.35200, -3.5814},         {0.27218, 0.35407, -4.3633},         {0.28039, 0.35577, -5.3762},
        {0.28863, 0.35714, -6.7262},         {0.29685, 0.35823, -8.5955},         {0.30505, 0.35907, -11.324},
        {0.31320, 0.35968, -15.628},         {0.32129, 0.36011, -23.325},         {0.32931, 0.36038, -40.770},
        {0.33724, 0.36051, -116.45}
};

        real32 us, vs, p, di, dm;
        int32 i;
        if ((X < 1.0e-20) && (Y < 1.0e-20) && (Z < 1.0e-20))
                return(-1);     /* protect against possible divide-by-zero failure */
        us = (4.0 * X) / (X + 15.0 * Y + 3.0 * Z);
        vs = (6.0 * Y) / (X + 15.0 * Y + 3.0 * Z);
        dm = 0.0;
        for (i = 0; i < 31; i++) {
                di = (vs - uvt[i].v) - uvt[i].t * (us - uvt[i].u);
                if ((i > 0) && (((di < 0.0) && (dm >= 0.0)) || ((di >= 0.0) && (dm < 0.0))))
                        break;  /* found lines bounding (us, vs) : i-1 and i */
                dm = di;
        }
        if (i == 31)
                return(-1);     /* bad XYZ input, color temp would be less than minimum of 1666.7 degrees, or too far towards blue */
        di = di / sqrt(1.0 + uvt[i    ].t * uvt[i    ].t);
        dm = dm / sqrt(1.0 + uvt[i - 1].t * uvt[i - 1].t);
        p = dm / (dm - di);     /* p = interpolation parameter, 0.0 : i-1, 1.0 : i */
        p = 1.0 / (LERP(rt[i - 1], rt[i], p));
        *temp = p;
        return(0);      /* success */
}



inline complex Fresnel::SetComplex ( real32 a , real32 b)
{
complex R;
SET_COMPLEX (&R,a,b);
return R;
}


inline real32 Fresnel::fxXYZ ( real32 xr )
{	real32 R;
	if (xr > 216/24389.) {R=pow(xr,real32(1/3.));}else{R=(16 + (24389*xr)/27.)/116.;};
	return R;
}
inline real32 Fresnel::fyXYZ ( real32 yr )
{	real32 R;
	if (yr > 216/24389.) {R=pow(yr,real32(1/3.));}else{R=(16 + (24389*yr)/27.)/116.;};
	return R;
}
inline real32 Fresnel::fzXYZ ( real32 zr )
{	real32 R;
	if (zr > 216/24389.) {R=pow(zr,real32(1/3.));}else{R=(16 + (24389*zr)/27.)/116.;};
	return R;
}

LabParameter Fresnel::SolveDumbXYZ ( ParameterVolume V , LabIntersection tuvObjects ,real32 GLf,int32 Subdivides)
{ 	LabResidue Residue; LabParameter R;ParameterVolumePartition Volumes;const real32 Step=1/4.; 
		R.a = 0;R.b=0;R.L=0;R.Plane=0;R.t=0;R.u=0;R.v=0;

	if ( FresnelPrivateData.SolutionFound!=true ) 
		{
		if ( V.Order==0 ) 
			{	if (simple)
				{
				for ( real32 t=V.tmax ; (t>=V.tmin ) ; t=t-(V.tmax-V.tmin)*Step)
					{
					for ( real32 u=V.umin ; (u<=V.umax ); u=u+(V.umax-V.umin)*Step) 
						{
						for (real32 v=V.vmin ; (v<=V.vmax ); v=v+(V.vmax-V.vmin)*Step) 
							{
				real32 c1X=tuvObjects.UVPlane.c1X;				real32 c1Y=tuvObjects.UVPlane.c1Y;				real32 c1Z=tuvObjects.UVPlane.c1Z;
				real32 c2X=tuvObjects.UVPlane.c2X;				real32 c2Y=tuvObjects.UVPlane.c2Y;				real32 c2Z=tuvObjects.UVPlane.c2Z;
				real32 c3X=tuvObjects.UVPlane.c3X;				real32 c3Y=tuvObjects.UVPlane.c3Y;				real32 c3Z=tuvObjects.UVPlane.c3Z;
				real32 SX=tuvObjects.TLine.c1X;					real32 SY=tuvObjects.TLine.c1Y;					real32 SZ=tuvObjects.TLine.c1Z;
				real32 GL=tuvObjects.TLine.c2L;					real32 Ga=tuvObjects.TLine.c2a;					real32 Gb=tuvObjects.TLine.c2b;
				real32 Xr=tuvObjects.RefWhite.Xr;				real32 Yr=tuvObjects.RefWhite.Yr;				real32 Zr=tuvObjects.RefWhite.Zr;
				Residue.L=delta+1.0;				Residue.a=delta+1.0;				Residue.b=delta+1.0;
				Residue.t=t;						Residue.u=u;						Residue.v=v;
				real32 fyXYZval =fyXYZ((c2Y*u + c3Y*v - c1Y*(-1 + u + v))/Yr);
				real32 fyXYZval2=fyXYZ(SY/Yr);
				Residue.L=	4*(4 + (1 + (-1 + GLf)*t)*(-4 + 29*fyXYZval2) - 29*fyXYZval);
         		if ( fabs(Residue.L) <= delta )	
         		{
				Residue.a=	Ga*t - 500*(-1 + t)*fxXYZ(SX/Xr) - 500*fxXYZ((c2X*u + c3X*v - c1X*(-1 + u + v))/Xr) + 500*(-1 + t)*fyXYZval2 + 
   							500*fyXYZval;
                 };    
                     
                if ( (fabs(Residue.L) <= delta) && (fabs(Residue.a) <= delta) )
                { 
				Residue.b=	Gb*t - 200*(-1 + t)*fyXYZval2 - 200*fyXYZval + 200*(-1 + t)*fzXYZ(SZ/Zr) + 
   							200*fzXYZ((c2Z*u + c3Z*v - c1Z*(-1 + u + v))/Zr);
   							Residue.b=Residue.b;
                };
                if( ((fabs(Residue.L) <= delta) && (fabs(Residue.a) <= delta) && (fabs(Residue.b) <= delta)) ||(t==V.tmax && u==V.umax && v==V.vmax))
                 	{
                 	FresnelPrivateData.SolutionFound=true; 
                 	R.Plane=V.Plane;
                 	R.t=t;
                 	R.u=u;
                 	R.v=v;
                 	R.L=4*(-4 + 29*fyXYZ((c2Y*u + c3Y*v - c1Y*(-1 + u + v))/Yr)) ; 
                 	R.a=500*(fxXYZ((c2X*u + c3X*v - c1X*(-1 + u + v))/Xr) - fyXYZ((c2Y*u + c3Y*v - c1Y*(-1 + u + v))/Yr)); 
                 	R.b=200*(fyXYZ((c2Y*u + c3Y*v - c1Y*(-1 + u + v))/Yr) - fzXYZ((c2Z*u + c3Z*v - c1Z*(-1 + u + v))/Zr));
                 	return R;};
							
			                };
                 		};
					}
				}
				else
				{ XYZPoint x1,x2,x3a,x4a,x5a,x3b,x4b,x5b;
				  x1.X =V.tLtmin;				  x1.Y =V.tatmin;				  x1.Z =V.tbtmin;
				  x2.X =V.tLtmax;				  x2.Y =V.tatmax;				  x2.Z =V.tbtmax;
				  x3a.X=V.UVLuminvmin;		      x3a.Y=V.UVauminvmin;			  x3a.Z=V.UVbuminvmin;
				  x4a.X=V.UVLumaxvmin;			  x4a.Y=V.UVaumaxvmin;			  x4a.Z=V.UVbumaxvmin;
				  x5a.X=V.UVLuminvmax;			  x5a.Y=V.UVauminvmax;			  x5a.Z=V.UVbuminvmax;
				  x3b.X=V.UVLumaxvmax;			  x3b.Y=V.UVaumaxvmax;			  x3b.Z=V.UVbumaxvmax;
				  x4b.X=V.UVLuminvmax;			  x4b.Y=V.UVauminvmax;			  x4b.Z=V.UVbuminvmax;
				  x5b.X=V.UVLumaxvmin;			  x5b.Y=V.UVaumaxvmin;			  x5b.Z=V.UVbumaxvmin;
				  PointParam Facet_uminvminumaxvminuminvmax= LinePlaneIntersection (x1,x2,x3a,x4a,x5a);
				     if (( Facet_uminvminumaxvminuminvmax.t>=0.) && (Facet_uminvminumaxvminuminvmax.t<=1. )&&
				         ((Facet_uminvminumaxvminuminvmax.u      +   Facet_uminvminumaxvminuminvmax.v)<=1.)&&
				         ( Facet_uminvminumaxvminuminvmax.v>=0.) && (Facet_uminvminumaxvminuminvmax.u>=0.))
				      {FresnelPrivateData.SolutionFound=true;
				       R.L=Facet_uminvminumaxvminuminvmax.x;
				       R.a=Facet_uminvminumaxvminuminvmax.y;
				       R.b=Facet_uminvminumaxvminuminvmax.z;
				       R.t=V.tmin+Facet_uminvminumaxvminuminvmax.t*(V.tmax-V.tmin);
				       R.u=V.umin+Facet_uminvminumaxvminuminvmax.u*(V.umax-V.umin);
				       R.v=V.vmin+Facet_uminvminumaxvminuminvmax.v*(V.vmax-V.vmin);
				       R.Plane=V.Plane;
				       return R;
				       }
				     else
				     {
				  	 PointParam Facet_umaxvmaxuminvmaxumaxvmin= LinePlaneIntersection (x1,x2,x3b,x4b,x5b);
				     if (( Facet_umaxvmaxuminvmaxumaxvmin.t>=0.) && (Facet_umaxvmaxuminvmaxumaxvmin.t<=1. ) &&
				         ((Facet_umaxvmaxuminvmaxumaxvmin.u      +   Facet_umaxvmaxuminvmaxumaxvmin.v)<=1.) &&
				         ( Facet_umaxvmaxuminvmaxumaxvmin.v>=0.) && (Facet_umaxvmaxuminvmaxumaxvmin.u>=0.))
				      {FresnelPrivateData.SolutionFound=true;
				       R.L=Facet_umaxvmaxuminvmaxumaxvmin.x;
				       R.a=Facet_umaxvmaxuminvmaxumaxvmin.y;
				       R.b=Facet_umaxvmaxuminvmaxumaxvmin.z;				      
				       R.t=V.tmax+Facet_umaxvmaxuminvmaxumaxvmin.t*(V.tmin-V.tmax);
				       R.u=V.umax+Facet_umaxvmaxuminvmaxumaxvmin.u*(V.umin-V.umax);
				       R.v=V.vmax+Facet_umaxvmaxuminvmaxumaxvmin.v*(V.vmin-V.vmax);
				       R.Plane=V.Plane;
				       return R;
				      }
				      else
				  		{if ((( Facet_uminvminumaxvminuminvmax.t>=0.) && (Facet_uminvminumaxvminuminvmax.t<=1.)&&
				  			  ( Facet_uminvminumaxvminuminvmax.u>=0.) && (Facet_uminvminumaxvminuminvmax.u<=1.)&&
				  			  ( Facet_uminvminumaxvminuminvmax.v>=0.) && (Facet_uminvminumaxvminuminvmax.v<=1.))||

							 (( Facet_umaxvmaxuminvmaxumaxvmin.t>=0.) && (Facet_umaxvmaxuminvmaxumaxvmin.t<=1.)&&
				  			  ( Facet_umaxvmaxuminvmaxumaxvmin.u>=0.) && (Facet_umaxvmaxuminvmaxumaxvmin.u<=1.)&&
				  			  ( Facet_umaxvmaxuminvmaxumaxvmin.v>=0.) && (Facet_umaxvmaxuminvmaxumaxvmin.v<=1.))
				  			)
				  			{if ((fabs(Facet_umaxvmaxuminvmaxumaxvmin.u)+fabs(Facet_umaxvmaxuminvmaxumaxvmin.v))<(fabs(Facet_uminvminumaxvminuminvmax.u)+fabs(Facet_uminvminumaxvminuminvmax.v))) 
				  			{R.L=Facet_umaxvmaxuminvmaxumaxvmin.x;
				       		 R.a=Facet_umaxvmaxuminvmaxumaxvmin.y;
				       		 R.b=Facet_umaxvmaxuminvmaxumaxvmin.z;				      
				       		 R.t=V.tmax+Facet_umaxvmaxuminvmaxumaxvmin.t*(V.tmin-V.tmax);
				       		 R.u=V.umax+Facet_umaxvmaxuminvmaxumaxvmin.u*(V.umin-V.umax);
				       		 R.v=V.vmax+Facet_umaxvmaxuminvmaxumaxvmin.v*(V.vmin-V.vmax);
				       		 R.Plane=V.Plane;
				       		 FresnelPrivateData.SolutionFound=true;
						     return R;
							}
				  			else
				  			{R.L=Facet_uminvminumaxvminuminvmax.x;
				       		 R.a=Facet_uminvminumaxvminuminvmax.y;
				       		 R.b=Facet_uminvminumaxvminuminvmax.z;				      
				       		 R.t=V.tmin+Facet_uminvminumaxvminuminvmax.t*(V.tmax-V.tmin);
				       		 R.u=V.umin+Facet_uminvminumaxvminuminvmax.u*(V.umax-V.umin);
				       		 R.v=V.vmin+Facet_uminvminumaxvminuminvmax.v*(V.vmax-V.vmin);
				       		 R.Plane=V.Plane;
				       		 FresnelPrivateData.SolutionFound=true;
						     return R;
							}
				  		}
				  		else
				  			{FresnelPrivateData.SolutionAbort=true;	 R.Plane=V.Plane;
							return R;};
				  		};
				  	};
				};
			}
			else
			{
  				Volumes=PartitionVolume (V,Subdivides);
   				if(FresnelPrivateData.SolutionFound!=true)
   				{Volumes.v100=ContainsIntersection(Volumes.v100,tuvObjects);
   				if(Volumes.v100.Intersects==true)
   					{R=SolveDumbXYZ (Volumes.v100,tuvObjects,GLf,Subdivides);};
   				};
   				if(FresnelPrivateData.SolutionFound!=true)
   				{Volumes.v110=ContainsIntersection(Volumes.v110,tuvObjects);
   				if(Volumes.v110.Intersects==true)
   					{R=SolveDumbXYZ (Volumes.v110,tuvObjects,GLf,Subdivides);};
   				};
   				if(FresnelPrivateData.SolutionFound!=true)
   				{Volumes.v101=ContainsIntersection(Volumes.v101,tuvObjects);
   				if(Volumes.v101.Intersects==true)
   					{R=SolveDumbXYZ (Volumes.v101,tuvObjects,GLf,Subdivides);};
   				};
   				if(FresnelPrivateData.SolutionFound!=true)
   				{Volumes.v111=ContainsIntersection(Volumes.v111,tuvObjects);
   				if(Volumes.v111.Intersects==true)
   					{R=SolveDumbXYZ (Volumes.v111,tuvObjects,GLf,Subdivides);};
   				};
  				if(FresnelPrivateData.SolutionFound!=true)
  				{Volumes.v000=ContainsIntersection(Volumes.v000,tuvObjects);
   				if(Volumes.v000.Intersects==true)
   					{R=SolveDumbXYZ (Volumes.v000,tuvObjects,GLf,Subdivides);};
   				};
   				if(FresnelPrivateData.SolutionFound!=true)
   				{Volumes.v001=ContainsIntersection(Volumes.v001,tuvObjects);
   				if(Volumes.v001.Intersects==true)
   					{R=SolveDumbXYZ (Volumes.v001,tuvObjects,GLf,Subdivides);};
   				};
   				if(FresnelPrivateData.SolutionFound!=true)
   				{Volumes.v010=ContainsIntersection(Volumes.v010,tuvObjects);
   				if(Volumes.v010.Intersects==true)
   					{R=SolveDumbXYZ (Volumes.v010,tuvObjects,GLf,Subdivides);};
   				};
   				if(FresnelPrivateData.SolutionFound!=true)
   				{Volumes.v011=ContainsIntersection(Volumes.v011,tuvObjects);
   				if(Volumes.v011.Intersects==true)
   					{R=SolveDumbXYZ (Volumes.v011,tuvObjects,GLf,Subdivides);};
   				};
  			};
    	};
    	
   	
		return R;
}

CubicRoots Fresnel::SolveCubic ( real32 ar,real32 ai,real32 br,real32 bi,real32 cr,real32 ci,real32 dr,real32 di)
{ 	CubicRoots R;
	R.Root1Valid=false;
	R.Root2Valid=false;
	R.Root3Valid=false;
	complex a=SetComplex(ar,ai);
	complex b=SetComplex(br,bi);
	complex c=SetComplex(cr,ci);
	complex d=SetComplex(dr,di);
	const complex  zero		=SetComplex(0,0);
	
	if ( d!=SetComplex(0,0) ) 
	{complex 		d1			=(-2*(c^3)+9*b*c*d-27*a*(d^2));
	 complex 		d1sq		=(d1^2);
	 const complex 	Re1Implssqroot3=SetComplex(1, pow(real32(3),real32(1/2.)));
	 const complex 	Re1Imminsqroot3=SetComplex(1,-pow(real32(3),real32(1/2.)));
	 complex		d2			=(-(c^2)+3*b*d);
	 complex		d2cubed		=(d2)^3;
	 complex		d3			=(-c/(3*d));
	 const real32    cuberoot2	=pow(2.,1/3.);
	 const real32    cuberoot2sq	=pow(2.,2/3.);
	 const complex  onethird	=SetComplex(1/3.,0);
	 const complex  half		=SetComplex(1/2.,0);
	 const complex  three		=SetComplex(3,0);
	 complex 		det			=(d1+((4*d2cubed+d1sq)^half))^onethird;
	 R.Root1					=d3-cuberoot2*d2/(3*d*det)+det/(3*cuberoot2*d);
	 R.Root1Valid				=true;
	 R.Root2					=d3+Re1Implssqroot3*d2/(3*cuberoot2sq*d*det)-Re1Imminsqroot3*det/(6*cuberoot2*d);
	 R.Root2Valid				=true;
	 R.Root3					=d3+Re1Imminsqroot3*d2/(3*cuberoot2sq*d*det)-Re1Implssqroot3*det/(6*cuberoot2*d);
	 R.Root3Valid				=true;
	}
	else
	{if(d==zero&&c!=zero)
		{const complex  half=SetComplex(1/2.,0);
		 complex det=(-4*a*c+(b^2))^half;
		 R.Root1=-( b+det)/(2*c);
		 R.Root1Valid=true;
		 R.Root2= (-b+det)/(2*c);
		 R.Root2Valid=true;
		}
		else
			{if((d==zero && c==zero && b==zero && a==zero)||(d==zero && c==zero && b!=zero))
			{R.Root1=-a/b;
		 	 R.Root1Valid=true;
			};
		};
	}; 
  return R;
}
ParameterVolume Fresnel::ContainsIntersection (ParameterVolume V, LabIntersection tuvObject)
{   ParameterVolume R=V;R.Intersects=false;
	LabColor LabP1,LabP2; XYZColor XYZP1;XYZReferenceWhite Ref=tuvObject.RefWhite;
	XYZP1.X=tuvObject.TLine.c1X;XYZP1.Y=tuvObject.TLine.c1Y;XYZP1.Z=tuvObject.TLine.c1Z;
	LabP2.L=tuvObject.TLine.c2L;LabP2.a=tuvObject.TLine.c2a;LabP2.b=tuvObject.TLine.c2b;
	LabP1=XYZ2Lab(XYZP1,Ref);
	if (R.tLtmin==UnKnownf||simple){	R.tLtmin=LabP1.L+R.tmin*(LabP2.L-LabP1.L);};
	if (R.tLtmax==UnKnownf||simple){	R.tLtmax=LabP1.L+R.tmax*(LabP2.L-LabP1.L);};
	if (R.tatmin==UnKnownf||simple){	R.tatmin=LabP1.a+R.tmin*(LabP2.a-LabP1.a);};
	if (R.tatmax==UnKnownf||simple){	R.tatmax=LabP1.a+R.tmax*(LabP2.a-LabP1.a);};
	if (R.tbtmin==UnKnownf||simple){	R.tbtmin=LabP1.b+R.tmin*(LabP2.b-LabP1.b);};
	if (R.tbtmax==UnKnownf||simple){	R.tbtmax=LabP1.b+R.tmax*(LabP2.b-LabP1.b);};
	real32 tLmin,tLmax,tamin,tamax,tbmin,tbmax;
	if (R.tLtmin>R.tLtmax){tLmin=R.tLtmax;tLmax=R.tLtmin;}else{tLmin=R.tLtmin;tLmax=R.tLtmax;};
	if (R.tatmin>R.tatmax){tamin=R.tatmax;tamax=R.tatmin;}else{tamin=R.tatmin;tamax=R.tatmax;};
	if (R.tbtmin>R.tbtmax){tbmin=R.tbtmax;tbmax=R.tbtmin;}else{tbmin=R.tbtmin;tbmax=R.tbtmax;};
	real32 c1X=tuvObject.UVPlane.c1X;real32 c1Y=tuvObject.UVPlane.c1Y;real32 c1Z=tuvObject.UVPlane.c1Z;
	real32 c2X=tuvObject.UVPlane.c2X;real32 c2Y=tuvObject.UVPlane.c2Y;real32 c2Z=tuvObject.UVPlane.c2Z;
	real32 c3X=tuvObject.UVPlane.c3X;real32 c3Y=tuvObject.UVPlane.c3Y;real32 c3Z=tuvObject.UVPlane.c3Z;
	real32 Xr=Ref.Xr;	real32 Yr=Ref.Yr;	real32 Zr=Ref.Zr;
	real32 u,v,fyXYZval;
	if(R.UVLuminvmin==UnKnownf||R.UVauminvmin==UnKnownf||R.UVbuminvmin==UnKnownf||simple)
	{u=R.umin;
	 v=R.vmin;
	 fyXYZval=		fyXYZ((c2Y*u + c3Y*v - c1Y*(-1 + u + v))/Yr);
	 R.UVLuminvmin=		4*(-4 + 29*fyXYZval);
	 R.UVauminvmin=   	500*(fxXYZ((c2X*u + c3X*v - c1X*(-1 + u + v))/Xr) - fyXYZval);
	 R.UVbuminvmin=   	200*(fyXYZval - fzXYZ((c2Z*u + c3Z*v - c1Z*(-1 + u + v))/Zr));
	};
	if(R.UVLuminvmax==UnKnownf||R.UVauminvmax==UnKnownf||R.UVbuminvmax==UnKnownf||simple)
	{u=R.umin;
	 v=R.vmax;
	 fyXYZval=			fyXYZ((c2Y*u + c3Y*v - c1Y*(-1 + u + v))/Yr);
	 R.UVLuminvmax=		4*(-4 + 29*fyXYZ((c2Y*u + c3Y*v - c1Y*(-1 + u + v))/Yr));
	 R.UVauminvmax=   	500*(fxXYZ((c2X*u + c3X*v - c1X*(-1 + u + v))/Xr) - fyXYZval);
	 R.UVbuminvmax=   	200*(fyXYZval - fzXYZ((c2Z*u + c3Z*v - c1Z*(-1 + u + v))/Zr));
	};
	if(R.UVLumaxvmin==UnKnownf||R.UVaumaxvmin==UnKnownf||R.UVbumaxvmin==UnKnownf||simple)
	{u=R.umax;
	 v=R.vmin;
	 fyXYZval=			fyXYZ((c2Y*u + c3Y*v - c1Y*(-1 + u + v))/Yr);
	 R.UVLumaxvmin=		4*(-4 + 29*fyXYZval);
	 R.UVaumaxvmin=   	500*(fxXYZ((c2X*u + c3X*v - c1X*(-1 + u + v))/Xr) - fyXYZval);
	 R.UVbumaxvmin=   	200*(fyXYZval - fzXYZ((c2Z*u + c3Z*v - c1Z*(-1 + u + v))/Zr));
	};
	if(R.UVLumaxvmax==UnKnownf||R.UVaumaxvmax==UnKnownf||R.UVbumaxvmax==UnKnownf||simple)
	{u=R.umax;
	 v=R.vmax;
	 fyXYZval=			fyXYZ((c2Y*u + c3Y*v - c1Y*(-1 + u + v))/Yr);
	 R.UVLumaxvmax=		4*(-4 + 29*fyXYZval);
	 R.UVaumaxvmax=   	500*(fxXYZ((c2X*u + c3X*v - c1X*(-1 + u + v))/Xr) - fyXYZval);
	 R.UVbumaxvmax=   	200*(fyXYZval - fzXYZ((c2Z*u + c3Z*v - c1Z*(-1 + u + v))/Zr));
	};
	real32 UVLmax=R.UVLuminvmin;
	if (R.UVLuminvmax>UVLmax){UVLmax=R.UVLuminvmax;};
	if (R.UVLumaxvmin>UVLmax){UVLmax=R.UVLumaxvmin;};
	if (R.UVLumaxvmax>UVLmax){UVLmax=R.UVLumaxvmax;};
	real32 UVamax=R.UVauminvmin;
	if (R.UVauminvmax>UVamax){UVamax=R.UVauminvmax;};
	if (R.UVaumaxvmin>UVamax){UVamax=R.UVaumaxvmin;};
	if (R.UVaumaxvmax>UVamax){UVamax=R.UVaumaxvmax;};
	real32 UVbmax=R.UVbuminvmin;
	if (R.UVbuminvmax>UVbmax){UVbmax=R.UVbuminvmax;};
	if (R.UVbumaxvmin>UVbmax){UVbmax=R.UVbumaxvmin;};
	if (R.UVbumaxvmax>UVbmax){UVbmax=R.UVbumaxvmax;};
	real32 UVLmin=R.UVLuminvmin;
	if (R.UVLuminvmax<UVLmin){UVLmin=R.UVLuminvmax;};
	if (R.UVLumaxvmin<UVLmin){UVLmin=R.UVLumaxvmin;};
	if (R.UVLumaxvmax<UVLmin){UVLmin=R.UVLumaxvmax;};
	real32 UVamin=R.UVauminvmin;
	if (R.UVauminvmax<UVamin){UVamin=R.UVauminvmax;};
	if (R.UVaumaxvmin<UVamin){UVamin=R.UVaumaxvmin;};
	if (R.UVaumaxvmax<UVamin){UVamin=R.UVaumaxvmax;};
	real32 UVbmin=R.UVbuminvmin;
	if (R.UVbuminvmax<UVbmin){UVbmin=R.UVbuminvmax;};
	if (R.UVbumaxvmin<UVbmin){UVbmin=R.UVbumaxvmin;};
	if (R.UVbumaxvmax<UVbmin){UVbmin=R.UVbumaxvmax;};
	if ((tLmin<=UVLmax) && (tLmax>=UVLmin) && (tamin<=UVamax) && (tamax>=UVamin) && (tbmin<=UVbmax) && (tbmax>=UVbmin)) 
	{R.Intersects=true;};
  return R;
}

ParameterVolumePartition Fresnel::PartitionVolume (ParameterVolume V,real32 Subdivides)
{  ParameterVolumePartition R; 	ParameterVolume v000; ParameterVolume v001; ParameterVolume v010; ParameterVolume v011; 
								ParameterVolume v100; ParameterVolume v101; ParameterVolume v110; ParameterVolume v111;
  
   if(V.Order==Subdivides)
  {
    V.tLtmin		=UnKnownf;	V.tLtmax		=UnKnownf;	V.tatmin		=UnKnownf;	V.tatmax		=UnKnownf;	V.tbtmin		=UnKnownf;	V.tbtmax		=UnKnownf;
	V.UVLuminvmin	=UnKnownf;	V.UVauminvmin	=UnKnownf;	V.UVbuminvmin	=UnKnownf;	V.UVLuminvmax	=UnKnownf;	V.UVauminvmax	=UnKnownf;	V.UVbuminvmax	=UnKnownf;
	V.UVLumaxvmin	=UnKnownf;	V.UVaumaxvmin	=UnKnownf;	V.UVbumaxvmin	=UnKnownf;	V.UVLumaxvmax	=UnKnownf;	V.UVaumaxvmax	=UnKnownf;	V.UVbumaxvmax	=UnKnownf;
  };

  
  v000.Index=V000;v001.Index=V001;v010.Index=V010;v011.Index=V011;v100.Index=V100;v101.Index=V101;v110.Index=V110;v111.Index=V111;
  v000.Plane=V.Plane;v001.Plane=V.Plane;v010.Plane=V.Plane;v011.Plane=V.Plane;v100.Plane=V.Plane;v101.Plane=V.Plane;v110.Plane=V.Plane;v111.Plane=V.Plane;
  v000.Order=V.Order-1;v001.Order=V.Order-1;v010.Order=V.Order-1;v011.Order=V.Order-1;v100.Order=V.Order-1;v101.Order=V.Order-1;v110.Order=V.Order-1;v111.Order=V.Order-1;
  
  real32 midVt=(V.tmax + V.tmin)/2.;
  real32 midVu=(V.umax + V.umin)/2.;
  real32 midVv=(V.vmax + V.vmin)/2.;

  v000.tmin=V.tmin;v000.tmax=midVt ;v000.umin=V.umin;v000.umax=midVu ;v000.vmin=V.vmin;v000.vmax=midVv;
  v001.tmin=V.tmin;v001.tmax=midVt ;v001.umin=V.umin;v001.umax=midVu ;v001.vmin=midVv ;v001.vmax=V.vmax;  
  v010.tmin=V.tmin;v010.tmax=midVt ;v010.umin=midVu ;v010.umax=V.umax;v010.vmin=V.vmin;v010.vmax=midVv;  
  v011.tmin=V.tmin;v011.tmax=midVt ;v011.umin=midVu ;v011.umax=V.umax;v011.vmin=midVv ;v011.vmax=V.vmax;  
  v100.tmin=midVt ;v100.tmax=V.tmax;v100.umin=V.umin;v100.umax=midVu ;v100.vmin=V.vmin;v100.vmax=midVv;  
  v101.tmin=midVt ;v101.tmax=V.tmax;v101.umin=V.umin;v101.umax=midVu ;v101.vmin=midVv ;v101.vmax=V.vmax;
  v110.tmin=midVt ;v110.tmax=V.tmax;v110.umin=midVu ;v110.umax=V.umax;v110.vmin=V.vmin;v110.vmax=midVv;  
  v111.tmin=midVt ;v111.tmax=V.tmax;v111.umin=midVu ;v111.umax=V.umax;v111.vmin=midVv ;v111.vmax=V.vmax;
  
  if (!simple)
  {v000.tLtmin=V.tLtmin;  v000.tLtmax=UnKnownf;  v000.tatmin=V.tatmin;  v000.tatmax=UnKnownf;  v000.tbtmin=V.tbtmin;  v000.tbtmax=UnKnownf;
  v000.UVLuminvmin=V.UVLuminvmin;  v000.UVauminvmin=V.UVauminvmin;  v000.UVbuminvmin=V.UVbuminvmin;  v000.UVLuminvmax=UnKnownf;  v000.UVauminvmax=UnKnownf;  v000.UVbuminvmax=UnKnownf;
  v000.UVLumaxvmin=UnKnownf;  v000.UVaumaxvmin=UnKnownf;  v000.UVbumaxvmin=UnKnownf;  v000.UVLumaxvmax=UnKnownf;  v000.UVaumaxvmax=UnKnownf;  v000.UVbumaxvmax=UnKnownf;

  v001.tLtmin=V.tLtmin;  v001.tLtmax=UnKnownf;  v001.tatmin=V.tatmin;  v001.tatmax=UnKnownf;  v001.tbtmin=V.tbtmin;  v001.tbtmax=UnKnownf;
  v001.UVLuminvmin=UnKnownf;  v001.UVauminvmin=UnKnownf;  v001.UVbuminvmin=UnKnownf;  v001.UVLuminvmax=V.UVLuminvmax;  v001.UVauminvmax=V.UVauminvmax;  v001.UVbuminvmax=V.UVbuminvmax;
  v001.UVLumaxvmin=UnKnownf;  v001.UVaumaxvmin=UnKnownf;  v001.UVbumaxvmin=UnKnownf;  v001.UVLumaxvmax=UnKnownf;  v001.UVaumaxvmax=UnKnownf;  v001.UVbumaxvmax=UnKnownf;

  v010.tLtmin=V.tLtmin;  v010.tLtmax=UnKnownf;  v010.tatmin=V.tatmin;  v010.tatmax=UnKnownf;  v010.tbtmin=V.tbtmin;  v010.tbtmax=UnKnownf;
  v010.UVLuminvmin=UnKnownf;  v010.UVauminvmin=UnKnownf;  v010.UVbuminvmin=UnKnownf;  v010.UVLuminvmax=UnKnownf;  v010.UVauminvmax=UnKnownf;  v010.UVbuminvmax=UnKnownf;
  v010.UVLumaxvmin=V.UVLumaxvmin;  v010.UVaumaxvmin=V.UVaumaxvmin;  v010.UVbumaxvmin=V.UVbumaxvmin;  v010.UVLumaxvmax=UnKnownf;  v010.UVaumaxvmax=UnKnownf;  v010.UVbumaxvmax=UnKnownf;
  
  v011.tLtmin=V.tLtmin;  v011.tLtmax=UnKnownf;  v011.tatmin=V.tatmin;  v011.tatmax=UnKnownf;  v011.tbtmin=V.tbtmin;  v011.tbtmax=UnKnownf;
  v011.UVLuminvmin=UnKnownf;  v011.UVauminvmin=UnKnownf;  v011.UVbuminvmin=UnKnownf;  v011.UVLuminvmax=UnKnownf;  v011.UVauminvmax=UnKnownf;  v011.UVbuminvmax=UnKnownf;
  v011.UVLumaxvmin=UnKnownf;  v011.UVaumaxvmin=UnKnownf;  v011.UVbumaxvmin=UnKnownf;  v011.UVLumaxvmax=V.UVLumaxvmax;  v011.UVaumaxvmax=V.UVaumaxvmax;  v011.UVbumaxvmax=V.UVbumaxvmax;
  
  v100.tLtmin=UnKnownf;  v100.tLtmax=V.tLtmax;  v100.tatmin=UnKnownf;  v100.tatmax=V.tatmax;  v100.tbtmin=UnKnownf;  v100.tbtmax=V.tbtmax;
  v100.UVLuminvmin=V.UVLuminvmin;  v100.UVauminvmin=V.UVauminvmin;  v100.UVbuminvmin=V.UVbuminvmin;  v100.UVLuminvmax=UnKnownf;  v100.UVauminvmax=UnKnownf;  v100.UVbuminvmax=UnKnownf;
  v100.UVLumaxvmin=UnKnownf;  v100.UVaumaxvmin=UnKnownf;  v100.UVbumaxvmin=UnKnownf;  v100.UVLumaxvmax=UnKnownf;  v100.UVaumaxvmax=UnKnownf;  v100.UVbumaxvmax=UnKnownf;

  v101.tLtmin=UnKnownf;  v101.tLtmax=V.tLtmax;  v101.tatmin=UnKnownf;  v101.tatmax=V.tatmax;  v101.tbtmin=UnKnownf;  v101.tbtmax=V.tbtmax;
  v101.UVLuminvmin=UnKnownf;  v101.UVauminvmin=UnKnownf;  v101.UVbuminvmin=UnKnownf;  v101.UVLuminvmax=V.UVLuminvmax;  v101.UVauminvmax=V.UVauminvmax;  v101.UVbuminvmax=V.UVbuminvmax;
  v101.UVLumaxvmin=UnKnownf;  v101.UVaumaxvmin=UnKnownf;  v101.UVbumaxvmin=UnKnownf;  v101.UVLumaxvmax=UnKnownf;  v101.UVaumaxvmax=UnKnownf;  v101.UVbumaxvmax=UnKnownf;

  v110.tLtmin=UnKnownf;  v110.tLtmax=V.tLtmax;  v110.tatmin=UnKnownf;  v110.tatmax=V.tatmax;  v110.tbtmin=UnKnownf;  v110.tbtmax=V.tbtmax;
  v110.UVLuminvmin=UnKnownf;  v110.UVauminvmin=UnKnownf;  v110.UVbuminvmin=UnKnownf;  v110.UVLuminvmax=UnKnownf;  v110.UVauminvmax=UnKnownf;  v110.UVbuminvmax=UnKnownf;
  v110.UVLumaxvmin=V.UVLumaxvmin;  v110.UVaumaxvmin=V.UVaumaxvmin;  v110.UVbumaxvmin=V.UVbumaxvmin;  v110.UVLumaxvmax=UnKnownf;  v110.UVaumaxvmax=UnKnownf;  v110.UVbumaxvmax=UnKnownf;
  
  v111.tLtmin=UnKnownf;  v111.tLtmax=V.tLtmax;  v111.tatmin=UnKnownf;  v111.tatmax=V.tatmax;  v111.tbtmin=UnKnownf;  v111.tbtmax=V.tbtmax;
  v111.UVLuminvmin=UnKnownf;  v111.UVauminvmin=UnKnownf;  v111.UVbuminvmin=UnKnownf;  v111.UVLuminvmax=UnKnownf;  v111.UVauminvmax=UnKnownf;  v111.UVbuminvmax=UnKnownf;
  v111.UVLumaxvmin=UnKnownf;  v111.UVaumaxvmin=UnKnownf;  v111.UVbumaxvmin=UnKnownf;  v111.UVLumaxvmax=V.UVLumaxvmax;  v111.UVaumaxvmax=V.UVaumaxvmax;  v111.UVbumaxvmax=V.UVbumaxvmax;
  };
   
  FresnelPrivateData.partitioncount=FresnelPrivateData.partitioncount+8;
  R.v000=v000;R.v001=v001;R.v010=v010;R.v011=v011;R.v100=v100;R.v101=v101;R.v110=v110;R.v111=v111;
  return R;
}

LabColor Fresnel::XYZ2Lab ( XYZColor XYZ , XYZReferenceWhite Ref )
{ 	LabColor R;
	real32 X=XYZ.X;	real32 Y=XYZ.Y;	real32 Z=XYZ.Z;
	real32 Xr=Ref.Xr;	real32 Yr=Ref.Yr;	real32 Zr=Ref.Zr;
	real32 fyXYZYYr=fyXYZ(Y/Yr);
	R.L=	116*fyXYZYYr-16;
	R.a=	500*(fxXYZ(X/Xr)-fyXYZYYr);   	
	R.b=	200*(fyXYZYYr-fzXYZ(Z/Zr));
	return R;
}

XYZColor Fresnel::Lab2XYZ ( LabColor Lab , XYZReferenceWhite Ref )
{	XYZColor R;
	real32 L=Lab.L;	real32 a=Lab.a;	real32 b=Lab.b;	real32 Xr=Ref.Xr;	real32 Yr=Ref.Yr;	real32 Zr=Ref.Zr;real32 yr;
	if ( L>8. ) { yr=pow(16 + L,3)/1.560896e6;}else{yr=(27*L)/24389.;};
	real32 fy;
	if (yr > 216/24389.){fy=(L+16)/116.;}else{fy=((216/24389.)*yr+16)/116.;};
	real32 fx=a/500.+fy;
	real32 fz=fy-b/200.;
	real32 xr,zr;
	if (pow(fx,3) > 216/24389.){xr=pow(fx,3);}else{;xr=(27*(-16 + 116*fx))/24389.;};
	if (pow(fz,3) > 216/24389.){zr=pow(fz,3);}else{;zr=(27*(-16 + 116*fz))/24389.;};
	R.X=Xr*xr;            
    R.Y=Yr*yr;   
	R.Z=Zr*zr;
	return R;
}


LabParameter Fresnel::Lab_Gamut_Project ( struct colourSystem WDcs, XYZSpectralColor Color ,real32 GLf,real32 Ga,real32 Gb,int32 Subdivides)
{   real32 xWDw = WDcs.xWhite;    real32 yWDw = WDcs.yWhite  ;    real32 YWDw = WDcs.YWhite ;
	real32 xWDk = WDcs.xBlack;    real32 yWDk = WDcs.yBlack  ;    real32 YWDk = WDcs.YBlack ;
	real32 xWDr = WDcs.xRed;      real32 yWDr = WDcs.yRed    ;    real32 YWDr = WDcs.YRed   ;
    real32 xWDg = WDcs.xGreen;    real32 yWDg = WDcs.yGreen  ;    real32 YWDg = WDcs.YGreen ;
    real32 xWDb = WDcs.xBlue;     real32 yWDb = WDcs.yBlue   ;    real32 YWDb = WDcs.YBlue  ;
	LabParameter R; ParameterVolume V; LabIntersection Obj; LabColor Lab; XYZColor Col;
	xyYColor xyYBlack		;xyYBlack.x   =xWDk; xyYBlack.y   =yWDk; xyYBlack.Y   =YWDk;
	xyYColor xyYRed			;xyYRed.x     =xWDr; xyYRed.y     =yWDr; xyYRed.Y     =YWDr;
	xyYColor xyYGreen		;xyYGreen.x   =xWDg; xyYGreen.y   =yWDg; xyYGreen.Y   =YWDg;
	xyYColor xyYBlue		;xyYBlue.x    =xWDb; xyYBlue .y   =yWDb; xyYBlue.Y    =YWDb;
	xyYColor xyYWhite		;xyYWhite.x   =xWDw; xyYWhite.y   =yWDw; xyYWhite.Y   =YWDw;
	XYZColor Black  =xyY2XYZ(xyYBlack)  ;
	XYZColor Red    =xyY2XYZ(xyYRed)    ;
	XYZColor Green  =xyY2XYZ(xyYGreen)  ;
	XYZColor Blue   =xyY2XYZ(xyYBlue)   ;
	XYZColor White  =xyY2XYZ(xyYWhite)  ;
	XYZColor Cyan   ;
	XYZColor Magenta;
	XYZColor Yellow ;
	Cyan.X=	 		-Black.X+Green.X+Blue.X ;   Cyan.Y=-Black.Y+Green.Y+ Blue.Y ;Cyan.Z   =-Black.Z+Green.Z + Blue.Z;
	Magenta.X=	 	-Black.X+Red.X+  Blue.X ;Magenta.Y=-Black.Y+  Red.Y+ Blue.Y ;Magenta.Z=-Black.Z+  Red.Z + Blue.Z;
	Yellow.X =	 	-Black.X+Red.X+  Green.X; Yellow.Y=-Black.Y+  Red.Y+ Green.Y;Yellow.Z =-Black.Z+  Red.Z +Green.Z;
	int32 Order=Subdivides;
    XYZReferenceWhite Ref;Ref.Xr=White.X;Ref.Yr=White.Y;Ref.Zr=White.Z;
    V.tmin=0.0;    V.tmax=1.0;    V.umin=0.0;    V.umax=1.0;    V.vmin=0.0;    V.vmax=1.0;
	V.Order=Order;
    V.t=true;    V.u=true;    V.v=true;
    V.Intersects=false;
    Col.X=Color.SX;    Col.Y=Color.SY;    Col.Z=Color.SZ;
    Lab=XYZ2Lab(Col,Ref);
	Obj.TLine.c1X=Color.SX;	Obj.TLine.c1Y=Color.SY;	Obj.TLine.c1Z=Color.SZ;
	Obj.TLine.c2L=Lab.L*GLf;
	Obj.TLine.c2a=Ga;
	Obj.TLine.c2b=Gb;
	Obj.RefWhite=Ref;
		
    for (int32 plane =KRB ; plane<=BMC ; plane++)
	{	if ((plane==KRB) || (plane==KBG) || (plane==KGR) ) {Obj.UVPlane.c1X=Black.X;Obj.UVPlane.c1Y=Black.Y;Obj.UVPlane.c1Z=Black.Z;};
		if ( plane==KRB ) {Obj.UVPlane.c2X=Red.X  ;Obj.UVPlane.c2Y=Red.Y  ;Obj.UVPlane.c2Z=Red.Z  ;};
		if ( plane==KRB ) {Obj.UVPlane.c3X=Blue.X ;Obj.UVPlane.c3Y=Blue.Y ;Obj.UVPlane.c3Z=Blue.Z ;};
		if ( plane==KBG ) {Obj.UVPlane.c2X=Blue.X ;Obj.UVPlane.c2Y=Blue.Y ;Obj.UVPlane.c2Z=Blue.Z ;};
		if ( plane==KBG ) {Obj.UVPlane.c3X=Green.X;Obj.UVPlane.c3Y=Green.Y;Obj.UVPlane.c3Z=Green.Z;};
		if ( plane==KGR ) {Obj.UVPlane.c2X=Green.X;Obj.UVPlane.c2Y=Green.Y;Obj.UVPlane.c2Z=Green.Z;};
		if ( plane==KGR ) {Obj.UVPlane.c3X=Red.X  ;Obj.UVPlane.c3Y=Red.Y  ;Obj.UVPlane.c3Z=Red.Z  ;};
		if ( plane==GCY ) {Obj.UVPlane.c1X=Green.X   ;Obj.UVPlane.c1Y=Green.Y   ;Obj.UVPlane.c1Z=Green.Z    ;};
		if ( plane==GCY ) {Obj.UVPlane.c2X=Cyan.X    ;Obj.UVPlane.c2Y=Cyan.Y    ;Obj.UVPlane.c2Z=Cyan.Z     ;};
		if ( plane==GCY ) {Obj.UVPlane.c3X=Yellow.X  ;Obj.UVPlane.c3Y=Yellow.Y  ;Obj.UVPlane.c3Z=Yellow.Z   ;};
		if ( plane==RYM ) {Obj.UVPlane.c1X=Red.X     ;Obj.UVPlane.c1Y=Red.Y     ;Obj.UVPlane.c1Z=Red.Z      ;};
		if ( plane==RYM ) {Obj.UVPlane.c2X=Yellow.X  ;Obj.UVPlane.c2Y=Yellow.Y  ;Obj.UVPlane.c2Z=Yellow.Z   ;};
		if ( plane==RYM ) {Obj.UVPlane.c3X=Magenta.X ;Obj.UVPlane.c3Y=Magenta.Y ;Obj.UVPlane.c3Z=Magenta.Z  ;};
		if ( plane==BMC ) {Obj.UVPlane.c1X=Blue.X    ;Obj.UVPlane.c1Y=Blue.Y    ;Obj.UVPlane.c1Z=Blue.Z     ;};
		if ( plane==BMC ) {Obj.UVPlane.c2X=Magenta.X ;Obj.UVPlane.c2Y=Magenta.Y ;Obj.UVPlane.c2Z=Magenta.Z  ;};
		if ( plane==BMC ) {Obj.UVPlane.c3X=Cyan.X    ;Obj.UVPlane.c3Y=Cyan.Y    ;Obj.UVPlane.c3Z=Cyan.Z     ;};
		V.Plane=plane;
		if (!FresnelPrivateData.disableKGR || plane!=KGR){R = SolveDumbXYZ ( V , Obj ,GLf,Subdivides);};
		if ( FresnelPrivateData.SolutionFound==true ) { R.Plane=plane ; break ;};
	};
	if ( FresnelPrivateData.SolutionFound==true ) {return R;};
	R.t=1.0;
	R.u=0.0;
	R.v=0.0;
	R.Plane=KRB;
    return R; //Black 
}

XYZColor Fresnel::xyY2XYZ ( xyYColor Color )
{ XYZColor R;
  R.X=Color.x*Color.Y/Color.y;
  R.Y=Color.Y;
  R.Z=(1-Color.x-Color.y)*Color.Y/Color.y;
  if ( Color.y==0 ) { R.X=0.0;R.Y=0.0;R.Z=0.0; };
  return R;
}

PointParam Fresnel::LinePlaneIntersection (XYZPoint xx1,XYZPoint xx2,XYZPoint xx3,XYZPoint xx4,XYZPoint xx5)
{PointParam R;
	real32 x1=xx1.X;real32 x2=xx2.X;real32 x3=xx3.X;real32 x4=xx4.X;real32 x5=xx5.X;
	real32 y1=xx1.Y;real32 y2=xx2.Y;real32 y3=xx3.Y;real32 y4=xx4.Y;real32 y5=xx5.Y;
	real32 z1=xx1.Z;real32 z2=xx2.Z;real32 z3=xx3.Z;real32 z4=xx4.Z;real32 z5=xx5.Z;
                real32 y1z2=y1*z2;                real32 y1z3=y1*z3;                real32 y1z4=y1*z4;                real32 y1z5=y1*z5;
                real32 y2z1=y2*z1;                real32 y2z3=y2*z3;
                real32 y3z1=y3*z1;                real32 y3z2=y3*z2;                real32 y3z4=y3*z4;
                real32 y4z1=y4*z1;                real32 y4z3=y4*z3;
                real32 y5z1=y5*z1;                real32 y5z2=y5*z2;                real32 y5z3=y5*z3;
                real32 x1y2z3=x1*y2z3;                real32 x1y3z2=x1*y3z2;                real32 x1y3z4=x1*y3z4;                real32 x1y3z5=x1*y3*z5;
                real32 x1y4z3=x1*y4z3;                real32 x1y4z5=x1*y4*z5;               real32 x1y5z2=x1*y5z2;                real32 x1y5z3=x1*y5z3;
                real32 x1y5z4=x1*y5*z4;               real32 x2y1z3=x2*y1z3;                real32 x2y3z1=x2*y3z1;                real32 x2y3z4=x2*y3z4;
                real32 x2y4z3=x2*y4z3;                real32 x2y5z1=x2*y5z1;                real32 x2y5z3=x2*y5z3;                real32 x2y5z4=x2*y5*z4;
                real32 x3y1z4=x3*y1z4;                real32 x3y1z5=x3*y1z5;                real32 x3y2z4=x3*y2*z4;               real32 x3y4z1=x3*y4z1;
                real32 x3y4z2=x3*y4*z2;               real32 x3y4z5=x3*y4*z5;               real32 x3y5z1=x3*y5z1;                real32 x3y5z2=x3*y5z2;
                real32 x3y5z4=x3*y5*z4;
                real32 x3x4z1z2=(x3 - x4)*(z1 - z2);
        
real32 det=(x3y4z1 - x3y5z1 - x3y4z2 + x3y5z2 - x1y4z3 + x2y4z3 + x1y5z3 - x2y5z3 + x5*((y3 - y4)*(z1 - z2) - (y1 - y2)*(z3 - z4)) - x3y1z4 + 
   		  x3y2z4 + x1y3z4 - x2y3z4 - x1y5z4 + x2y5z4 + x4*(-((y3 - y5)*(z1 - z2)) + (y1 - y2)*(z3 - z5)) + 
   		  (x3*(y1 - y2) - (x1 - x2)*(y3 - y4))*z5);
R.t=	(x3y4z1 - x3y5z1 - x1y4z3 + x1y5z3 - x3y1z4 + x1y3z4 - x1y5z4 + x3y5z4 + x5*(y3z1 - y4z1 - y1z3 + y4z3 + y1z4 - y3z4) + 
   		x3y1z5 - x1y3z5 + x1y4z5 - x3y4z5 + x4*(y5z1 + y1z3 - y5z3 - y1z5 + y3*(-z1 + z5)))/
   		(det);
R.u=	(-(x2y3z1) + x2y5z1 + x1y3z2 - x1y5z2 + x2y1z3 - x1y2z3 + x1y5z3 - x2y5z3 + x5*(-(y2z1) + y3z1 + y1z2 - y3z2 - y1z3 + y2z3) + 
   		(-(x2*y1) + x1*y2 - x1*y3 + x2*y3)*z5 + x3*(-(y5z1) - y1z2 + y5z2 + y2*(z1 - z5) + y1z5))/
   		(det);
R.v=	(-(((x3 - x4)*(y1 - y2) + (-x1 + x2)*(y3 - y4))*(x3*(-z1 + z2) + x2*(z1 - z3) + x1*(-z2 + z3))) + 
   		(x3*(y1 - y2) + x1*(y2 - y3) + x2*(-y1 + y3))*(-(x3x4z1z2) + (x1 - x2)*(z3 - z4)))/
   		(-(((x3 - x5)*(y1 - y2) + (-x1 + x2)*(y3 - y5))*(x3x4z1z2 + (-x1 + x2)*(z3 - z4))) + 
   		((x3 - x4)*(y1 - y2) + (-x1 + x2)*(y3 - y4))*((x3 - x5)*(z1 - z2) + (-x1 + x2)*(z3 - z5)));
R.x=x1+R.t*(x2-x1);
R.y=y1+R.t*(y2-y1);
R.z=z1+R.t*(z2-z1);
 return R;
}


