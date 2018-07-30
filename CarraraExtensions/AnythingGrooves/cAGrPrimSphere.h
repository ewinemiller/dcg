/*  Anything Grooves - plug-in for Carrara
    Copyright (C) 2000 Eric Winemiller

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
#include "IAGrPrim.h"
#include "AGrPrimData.h"


class cAGrPrimSphere: public IAGrPrim {
	private:
		~cAGrPrimSphere();
	
		//precalculated stuff
		real32* VLines;
		real32* sinVertLines;
		real32* cosVertLines;

		//the middle point stuff if we are using smart split
		real32* midULines;
		real32* midVLines;
		real32* midsinHorzLines;
		real32* midcosHorzLines;
		real32* midsinVertLines;
		real32* midcosVertLines;

		real32* displacement;

		TVertex3DLite* points;

		DCGFacetMeshAccumulator* accu;
		TMCCountedPtr<I3DShShader> shader;
		uint32 fUVSpaceID;
		TVector3 scale;
		real32 oneoverU;
		real32 oneoverV;

		AGrPrimData fData;
		
		void AddQuad(const int32& u, const int32& v);
		void AddFacet(const TVertex3DLite& pt1,const TVertex3DLite& pt2,const TVertex3DLite& pt3);
		void CleanUp();
	public:
		cAGrPrimSphere(const AGrPrimData& pfData);
		virtual void DoBasicMesh(FacetMesh** outMesh);
		virtual void DoMesh(DCGFacetMeshAccumulator* pAccu, TMCCountedPtr<I3DShShader> pShader);
		virtual void GetBoundingBox(TBBox3D& bbox);
		virtual uint32 GetUVSpaceCount();
		virtual MCCOMErr GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo);

	};