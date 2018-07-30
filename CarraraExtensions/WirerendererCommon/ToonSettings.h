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
#ifndef __TOONSETTINGS__
#define __TOONSETTINGS__  

#if CP_PRAGMA_ONCE
#pragma once
#endif
#define UNSPECIFIED_SORT_ORDER 1000000

class ShadingDomainLineLevels
{
public:
	ShadingDomainLineLevels()
	{
		overrideSettings = false;
		lineEffect = 1.0f;
		color = TMCColorRGBA::kBlack;
		useObjectColor = false;
		lineWidth = 2.0f;
		overDraw = 0;
		depthCue = 100;
		doDepthCueing = false;
		vectorAngle = 30;
		removeHiddenLines = true;

	}

	boolean overrideSettings;
	TMCString255 name;
	real32 lineEffect;
	TMCColorRGBA color;
	boolean useObjectColor;
	real32 lineWidth;
	real32 overDraw;
	boolean doDepthCueing;
	real32 depthCue;
	real32 vectorAngle;
	boolean doDomainBoundaryEdges;
	boolean removeHiddenLines;
};

class ToonSettings 
{
private:
	TMCClassArray<ShadingDomainLineLevels> levels;
	TMCArray<uint32> sortOrder;
	void swap(const uint32 domainRank1, const uint32 domainRank2)
	{
		uint32 temp = getRankedDomainIndex(domainRank1);
		setRankedDomainIndex(domainRank1, getRankedDomainIndex(domainRank2));
		setRankedDomainIndex(domainRank2, temp);
	}

public:


	uint32 getHighestSortedDomainIndex(const int32 domain1, const int32 domain2)
	{
		uint32 sortOrderCount = sortOrder.GetElemCount();
		for (uint32 sortOrderIndex = 0; sortOrderIndex < sortOrderCount; sortOrderIndex++)
		{
			if (sortOrder[sortOrderIndex] == domain1)
			{
				return domain1;
			}
			else if (sortOrder[sortOrderIndex] == domain2)
			{
				return domain2;
			}

		}
		return domain1;
	}
	ShadingDomainLineLevels& getDomainByIndex(const uint32 domainIndex)
	{
		//particle emitters play games with the shading domain
		//if it's out of bounds, just return 0
		if (domainIndex > levels.GetElemCount())
		{
			return levels[0];
		}
		else
		{
			return levels[domainIndex];
		}
	}
	ShadingDomainLineLevels& getDomainByRank(const uint32 domainRank)
	{
		return levels[sortOrder[domainRank]];
	}
	uint32 getRankByIndex(const uint32 domainIndex)
	{
		uint32 sortOrderCount = sortOrder.GetElemCount();
		for (uint32 sortOrderIndex = 0; sortOrderIndex < sortOrderCount; sortOrderIndex++)
		{
			if (sortOrder[sortOrderIndex] == domainIndex)
			{
				return sortOrderIndex;
			}
		}
		return UNSPECIFIED_SORT_ORDER;
	}
	uint32 getRankedDomainIndex(const uint32 domainRank)
	{
		return sortOrder[domainRank];
	}
	void setRankedDomainIndex(const uint32 domainRank, const uint32 domainIndex)
	{
		sortOrder[domainRank] = domainIndex;
	}
	void SetElemCount(const uint32 count)
	{
		levels.SetElemCount(count);
		sortOrder.SetElemCount(count);
		sortOrder.FillWithValue(UNSPECIFIED_SORT_ORDER);
	}
	uint32 GetElemCount()
	{
		return levels.GetElemCount();
	}
	void SetElem(const uint32 index, const ShadingDomainLineLevels newElem)
	{
		return levels.SetElem(index, newElem);
	}
	int32 reRank(const uint32 domainRank, const int32 step)
	{
		int32 newRank = domainRank + step;

		//just return if this drops us out of bounds
		if (newRank < 0 || newRank >= levels.GetElemCount())
		{
			return domainRank;
		}
		swap(domainRank, newRank);

		return newRank;
	}

};

#endif
