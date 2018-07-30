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


class cAGrPrimCylinder: public IAGrPrim {
	private:
		~cAGrPrimCylinder();
		real32* XLines;
		real32* YLines;
		real32* ULines;
		real32* VLines;
		boolean* bFilled;
		real32* Z;
		
		real32* midULines;
		real32* midVLines;
		real32* midsinHorzLines;
		real32* midcosHorzLines;
		real32* midVertLines;

		TVector3 scale;
		int32 topedge, bottomedge;

		TVertex3DLite* sidepoints;
		real32* sidedisplacement;
		real32* VertLines;

		TVertex3DLite* toppoints;
		real32* topdisplacement;
		
		TVertex3DLite* bottompoints;
		real32* bottomdisplacement;

		DCGFacetMeshAccumulator* accu;
		TMCCountedPtr<I3DShShader> shader;
		uint32	fUVSpaceID;

		AGrPrimData fData;
		
		void GetBlockSize(const real32* displacement, const TVertex3DLite* points
			, const uint32& u, const uint32& v, const uint32& maxv, uint32& vsize);
		void AddQuad(const real32* displacement, const TVertex3DLite* points
			, const int32& u, const int32& v, const int32& vsize);
		void AddFacet(const TVertex3DLite& pt1,const TVertex3DLite& pt2,const TVertex3DLite& pt3);
		void CleanUp();

		uint32 GetIndex(const uint32& u, const uint32& v) {
			return v * (fData.lU + 1) + u;
			};

	public:
		cAGrPrimCylinder(const AGrPrimData& pfData);
		virtual void DoBasicMesh(FacetMesh** outMesh);
		virtual void DoMesh(DCGFacetMeshAccumulator* pAccu, TMCCountedPtr<I3DShShader> pShader);
		virtual void GetBoundingBox(TBBox3D& bbox);
		virtual uint32 GetUVSpaceCount();
		virtual MCCOMErr GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo);

	};