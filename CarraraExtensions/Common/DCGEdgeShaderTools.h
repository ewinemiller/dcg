/*  Carrara plug-in utilities
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
const real32 epsilon = 0.001f;
const real32 HALF_PI = 1.5707963267948966192313216916398f;

boolean SloppyCompare(const TVector3& a,const TVector3& b)
{
	TVector3 diff(fabs(a.x - b.x), fabs(a.y - b.y), fabs(a.z - b.z));

	return (diff.x <= epsilon && diff.y <= epsilon && diff.z <= epsilon);
}

boolean PointInTriangle(const TVector3& p, const TVector3& a,const TVector3& b,const TVector3& c)
{
	if (p == a) return true;
	if (p == b) return true;
	if (p == c) return true;

	double dAngle;

    TVector3 vec0 = ( p - a );
    TVector3 vec1 = ( p - b );
    TVector3 vec2 = ( p - c );
	vec0.Normalize(vec0);
	vec1.Normalize(vec1);
	vec2.Normalize(vec2);


    dAngle =
        acos( vec0 * vec1 ) + 
        acos( vec1 * vec2 ) + 
        acos( vec2 * vec0 ) ;


    if( fabs( dAngle - 2*PI ) < epsilon )
        return true;
	else
		return false;

}

boolean CoplanerLines(const TVector3& x1,const TVector3& x2,const TVector3& x3, const TVector3& x4)
{
	real32 skew = (x3 - x1) * ((x2 - x1) ^ (x4 - x3));

	return (skew == 0);
}

boolean IsLineOnPlane(const TVector3& p1,const TVector3& p2, const TVector3& a,const TVector3& b,const TVector3& c)
{
	return  (CoplanerLines(p1, p2, a, b) && CoplanerLines(p1, p2, b, c) && CoplanerLines(p1, p2, c, a));
}

boolean IsLineFacetEdge(const TVector3& infacet,const TVector3& outfacet, const TVector3& a,const TVector3& b,const TVector3& c, real32& sharedLength)
{
	TVector3 lineslope = outfacet - infacet;
	lineslope.Normalize(lineslope);

	TVector3 edgeslope[3] = {a - b, b - c, c - a};
	const TVector3* vertices[4] = {&a, &b, &c, &a};

	//see if slopes match on any segments of the triangle
	//if so, find the first one where there is a change
	//calc relative direction and return the distance between
	//the inner point and the end point of the line
	for (uint32 edge = 0; edge < 3; edge++)
	{
		edgeslope[edge].Normalize(edgeslope[edge]);
		if (SloppyCompare(lineslope, edgeslope[edge]) || SloppyCompare(lineslope, -edgeslope[edge]))
		{

			for (uint32 direction = 0; direction < 3; direction++)
			{
				if (lineslope[direction] != 0)
				{
					real32 fFarAlong = (outfacet[direction] - (*vertices[edge])[direction]) / lineslope[direction];

					TVector3 outfacetcheck = *vertices[edge] + lineslope * fFarAlong;

					if (SloppyCompare(outfacetcheck, outfacet))
					{
						if (lineslope[direction] * edgeslope[edge][direction] > 0)
						{
							sharedLength = sqr(infacet, *vertices[edge]);
						}
						else
						{
							sharedLength = sqr(infacet, *vertices[edge + 1]);
						}
						return true;
					}
				}
			}
		}
	}

	return false;
}

boolean IsLineFacetEdgeNoInterior(const TVector3& infacet,const TVector3& outfacet, const TVector3& a,const TVector3& b,const TVector3& c, real32& sharedLength)
{
	TVector3 lineslope = outfacet - infacet;
	lineslope.Normalize(lineslope);

	TVector3 edgeslope[3] = {a - b, b - c, c - a};
	const TVector3* vertices[4] = {&a, &b, &c, &a};

	//see if slopes match on any segments of the triangle
	//if so, find the first one where there is a change
	//calc relative direction and return the distance between
	//the inner point and the end point of the line
	for (uint32 edge = 0; edge < 3; edge++)
	{
		edgeslope[edge].Normalize(edgeslope[edge]);
		if (SloppyCompare(lineslope, edgeslope[edge]) || SloppyCompare(lineslope, -edgeslope[edge]))
		{

			for (uint32 direction = 0; direction < 3; direction++)
			{
				if (lineslope[direction] != 0)
				{
					real32 fFarAlong = (outfacet[direction] - (*vertices[edge])[direction]) / lineslope[direction];

					TVector3 outfacetcheck = *vertices[edge] + lineslope * fFarAlong;

					if (SloppyCompare(outfacetcheck, outfacet))
					{
						sharedLength = sqr(*vertices[edge + 1], *vertices[edge]);
						return true;
					}
				}
			}
		}
	}

	return false;
}


void GetLineSegmentLengthInFacet(const TVector3& infacet,const TVector3& outfacet, const TVector3& a,const TVector3& b,const TVector3& c, real32& sharedLength)
{

	const TVector3* vertices[4] = {&a, &b, &c, &a};
	for (uint32 edge = 0;edge < 3; edge++)
	{
		const TVector3 A(outfacet - infacet);
		const TVector3 B(*vertices[edge + 1] - *vertices[edge]);
		const TVector3 C(*vertices[edge] - infacet);
		const TVector3 AB(A ^ B);
		const real32 denom = sqr(AB.GetMagnitude());
		const real32 s = ((C ^ B) * (A ^ B)) / denom;
		if (s >= 0 && s <= 1.0f)
		{
			const TVector3 intersection(infacet + (outfacet - infacet) * s);
			sharedLength = sqr(intersection, infacet);
			return;
		}

	}
	sharedLength = 0;

}

void GetLineSegmentLengthInFacetNoInterior(const TVector3& infacet,const TVector3& outfacet, const TVector3& a,const TVector3& b,const TVector3& c, real32& sharedLength)
{
	TVector3 intersection[2];
	uint32 currentIntersection = 0;

	const TVector3* vertices[4] = {&a, &b, &c, &a};
	for (uint32 edge = 0;edge < 3; edge++)
	{
		const TVector3 A(*vertices[edge + 1] - *vertices[edge]);
		const TVector3 B(outfacet - infacet);
		const TVector3 C(infacet - *vertices[edge]);
		const TVector3 AB(A ^ B);
		const real32 denom = sqr(AB.GetMagnitude());
		const real32 s = ((C ^ B) * (A ^ B)) / denom;
		if (s >= 0 && s <= 1.0f)
		{
			intersection[currentIntersection] = *vertices[edge] + (*vertices[edge + 1] - *vertices[edge]) * s;
			currentIntersection++;
			if (currentIntersection == 2)
				edge = 3;
		}

	}
	if (currentIntersection == 2)
	{
		sharedLength = sqr(intersection[0], intersection[1]);
	}
	else
	{
		sharedLength = 0;
	}
}
