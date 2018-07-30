/*  Shader Ops - plug-in for Carrara
    Copyright (C) 2003 Eric Winemiller

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
#include "copyright.h"
#include "RealUV.h"
#include "Clip.h"
#include "Compress.h"
#include "Stretch.h"
#include "UVTile.h"
#include "Threshold.h"
#include "Invert.h"
#include "Brightness.h"
#include "Max.h"
#include "Min.h"
#include "MaxThreshold.h"
#include "MinThreshold.h"
#include "Note.h"
#include "Scale.h"
#include "Translate.h"
#include "Fresnel.h"
#include "UVShifter.h"
#include "RadialGradient.h"
#include "ProgScale.h"
#include "GroupShader.h"


void Extension3DInit(IMCUnknown* utilities)
{
}

void Extension3DCleanup()
{
	// Perform any nec clean-up here
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)	
{														
	TBasicUnknown* res = NULL;

	if (classId == CLSID_GroupShader)
	{
		res = new GroupShader;
		return res;
	}

	if (classId == CLSID_RealUV) res = new RealUV;
	if (classId == CLSID_Clip) res = new Clip;
	if (classId == CLSID_Compress) res = new Compress;
	if (classId == CLSID_Stretch) res = new Stretch;
	if (classId == CLSID_UVTile) res = new UVTile;
	if (classId == CLSID_UVShifter) res = new UVShifter;
	if (classId == CLSID_Threshold) res = new Threshold;
	if (classId == CLSID_Invert) res = new Invert;
	if (classId == CLSID_Brightness) res = new Brightness;
	if (classId == CLSID_Max) res = new Max;
	if (classId == CLSID_Min) res = new Min;
	if (classId == CLSID_MaxThreshold) res = new MaxThreshold;
	if (classId == CLSID_MinThreshold) res = new MinThreshold;
	if (classId == CLSID_Note) res = new Note;
	if (classId == CLSID_Scale) res = new Scale;
	if (classId == CLSID_Translate) res = new Translate;
	if (classId == CLSID_Fresnel) res = new Fresnel;
	if (classId == CLSID_RadialGradient) res = new RadialGradient;
	if (classId == CLSID_ProgScale) res = new ProgScale;

	return res;
}
