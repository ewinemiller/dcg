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

class cAGrPrimCube: public IAGrPrim {
	private:
		~cAGrPrimCube();
		uint32 chunku, chunkv;
		TVertex3DLite* toppoints;
		TVertex3DLite* leftpoints;
		TVertex3DLite* rightpoints;
		TVertex3DLite* bottompoints;
		TVertex3DLite* frontpoints;
		TVertex3DLite* backpoints;

		TVector3* midtoppoints;
		TVector3* midleftpoints;
		TVector3* midrightpoints;
		TVector3* midbottompoints;
		TVector3* midfrontpoints;
		TVector3* midbackpoints;

		real32* topdisplacement;
		real32* bottomdisplacement;
		real32* leftdisplacement;
		real32* rightdisplacement;
		real32* frontdisplacement;
		real32* backdisplacement;

		boolean* topfilled;
		boolean* bottomfilled;
		boolean* leftfilled;
		boolean* rightfilled;
		boolean* frontfilled;
		boolean* backfilled;

		real32* ULines;
		real32* VLines;

		real32* midULines;
		real32* midVLines;
		
		DCGFacetMeshAccumulator* accu;
		TMCCountedPtr<I3DShShader> shader;
		uint32	fUVSpaceID;

		AGrPrimData fData;
		
		void GetBlockSize(boolean* bFilled, const real32* displacement
			, const uint32& u, const uint32& v, uint32& usize, uint32& vsize);
		void AddQuad(const real32* displacement, const TVertex3DLite* points
			, const int32& u, const int32& v
			, const int32& usize, const int32& vsize, const TVector3* midpoints);
		void AddFacet(const TVertex3DLite& pt1,const TVertex3DLite& pt2,const TVertex3DLite& pt3);
		void CleanUp();

	public:
		cAGrPrimCube(const AGrPrimData& pfData);
		virtual void DoBasicMesh(FacetMesh** outMesh);
		virtual void DoMesh(DCGFacetMeshAccumulator* pAccu, TMCCountedPtr<I3DShShader> pShader);
		virtual void GetBoundingBox(TBBox3D& bbox);
		virtual uint32 GetUVSpaceCount();
		virtual MCCOMErr GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo);

	};