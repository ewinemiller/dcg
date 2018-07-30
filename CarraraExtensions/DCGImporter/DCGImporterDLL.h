/*  DCG Importer - plug-in for Carrara
    Copyright (C) 2012 Eric Winemiller

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
#ifndef __DCGIMPORTERDLL__
#define __DCGIMPORTERDLL__  


#if CP_PRAGMA_ONCE
#pragma once
#endif

struct ObjImporterData {
	boolean Noto;
	real32 Conv;
	boolean OtoP;
	boolean TrCo;
	int32 Obj0;
	int32 Poly;
	int32 Grp0;
	boolean POri;
	int32 Nam0;
	int32 BkFc;
	boolean UV01;
	real32 Smoo;
	boolean MgSn;
	int32 apim;
	boolean dehy;
};

const int32 VERTEX_PRIMITIVE = 'Obj2';
const int32 FACET_MESH = 'Obj1';

#endif