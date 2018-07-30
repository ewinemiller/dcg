/*  Wirerender utilities for Carrara
    Copyright (C) 2004 Eric Winemiller

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
#ifndef __WireRendererData__
#define __WireRendererData__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif
#include "MCColorRGBA.h"
#include "copyright.h"

struct RealisticRendererPublicData
{
#if VERSIONNUMBER >= 0x070000
	boolean Shdw;
	boolean Flec;
	boolean Tran;
	boolean LTTO;
	boolean BUMP;
	boolean FRAC;
	boolean Radi;
	boolean Caus;
	boolean PHmp;
	boolean uGam;
	boolean SkyL;
	boolean IEAc;
	boolean irrC;
	boolean sepS;
	real32 Gamm;
	int32 MaxD;
	int32 GItp;
	int32 Qual;
	real32 ILin;
	real32 SkyI;
	real32 CAin;
	real32 Intr;
	real32 AmbR;
	boolean atmD;
	boolean vlpD;
	boolean vlpE;
	int32 OVER;
	int32 Oacc;
	int32 Lacc;
	int32 Iacc;
	int32 Dpht;
	int32 Cpht;
	real32 cfil;
	real32 FltB;
	boolean rayT;
	boolean dofd;
	boolean csfm;
	real32 phtA;
	int32 IrrM;
	boolean SavM;
	boolean GIlt;
	TMCDynamicString iMap;
	TMCDynamicString sMap;
#elif VERSIONNUMBER >= 0x050000
	boolean Shdw;
	boolean Flec;
	boolean Tran;
	boolean LTTO;
	boolean BUMP;
	boolean FRAC;
	boolean Radi;
	boolean Caus;
	boolean PHmp;
	boolean uGam;
	boolean SkyL;
	boolean IEAc;
	boolean irrC;
	real32 Gamm;
	int32 MaxD;
	int32 GItp;
	int32 Qual;
	real32 ILin;
	real32 SkyI;
	real32 CAin;
	real32 Intr;
	real32 AmbR;
	int32 OVER;
	int32 Oacc;
	int32 Lacc;
	int32 Iacc;
	int32 Dpht;
	int32 Cpht;
	real32 cfil;
	real32 FltB;
	boolean rayT;
	boolean dofd;
	real32 phtA;
	int32 IrrM;
	boolean SavM;
	boolean GIlt;
	TMCDynamicString iMap;
	TMCDynamicString sMap;
#else
	boolean Shdw;
	boolean Flec;
	boolean Tran;
	boolean LTTO;
	boolean BUMP;
	boolean FRAC;
	boolean Radi;
	boolean Caus;
	boolean PHmp;
	boolean uGam;
	boolean SkyL;
	boolean IEAc;
	boolean irrC;
	real32 Gamm;
	int32 MaxD;
	int32 Qual;
	real32 ILin;
	real32 SkyI;
	real32 CAin;
	real32 Intr;
	int32 OVER;
	int32 Oacc;
	int32 Lacc;
	int32 Iacc;
	int32 Dpht;
	int32 Cpht;
	real32 cfil;
	real32 FltB;
	boolean rayT;
	boolean dofd;
	real32 phtA;
#endif
};

struct WireRendererPublicData
{
	RealisticRendererPublicData basedata;
	real32 fLineWidth;
	ActionNumber lRenderStyle;
	real32 fVectorAngle;
	real32 fOverdraw;
	boolean bDepthCueing;
	real32 fCueDepth;
	boolean bBackfaceOptimization;
	int32 iBucketOptimization;
	TMCColorRGBA lineColor;
	real32 fLineEffect;
	boolean bRemoveHiddenLines;
	boolean bDrawBackfaces;
	boolean bSmartQuads;
	boolean bFilterEdges;
	ActionNumber iOversampling;
	boolean bVertexEdges;
	boolean bSilhouetteEdges;
	boolean bUseObjectColor;
	boolean bBaseRender;
	boolean doDomainBoundaryEdges;
	ActionNumber alphaContent;

};

enum RenderStyle {rsWireFrame = 1, rsToon = 2};
enum AlphaContent{acBaseRender = 1, acToonLines = 2, acBoth = 3};

struct RealisticStatistics
{
	int32 RayN;
	int32 FacN;
};

struct WireRendererStatistics
{
	RealisticStatistics basedata;
};

#endif