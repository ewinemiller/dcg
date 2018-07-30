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
#ifndef __DCGStreamHelper__
#define __DCGStreamHelper__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "IShTokenStream.h"
#include "copyright.h"

class DCGStreamHelper
{

public:
	static int32 GetInt32(IShTokenStream* stream) 
	{
#if VERSIONNUMBER >= 0x060000 
		return stream->GetInt32Token();
#else
		int32 temp;
		stream->GetLong(&temp);
		return temp;
#endif
	};

	static void PutInt32(IShTokenStream* stream, int32 keyword, int32 value) 
	{
#if VERSIONNUMBER >= 0x060000 
		stream->PutInt32Attribute(keyword, value);
#else
	stream->PutKeyword(keyword);
	stream->PutLong(value);
#endif
	};

	static real32 GetReal32(IShTokenStream* stream) 
	{
#if VERSIONNUMBER >= 0x060000 
		return stream->GetReal32Token();
#elif VERSIONNUMBER >= 0x050000 
		return stream->GetReal32();
#else
		real32 temp;
		stream->GetQuickFix(&temp);
		return temp;
#endif
	};

	static void PutReal32(IShTokenStream* stream, int32 keyword, real32 value) 
	{
#if VERSIONNUMBER >= 0x050000 
		stream->PutReal32Attribute(keyword, value);
#else
	stream->PutKeyword(keyword);
	stream->PutQuickFix(value);
#endif
	};


	static boolean GetBoolean(IShTokenStream* stream) 
	{
#if VERSIONNUMBER >= 0x060000 
		return stream->GetBooleanToken();
#else
		int32 temp;
		stream->GetLong(&temp);
		return temp;
#endif
	};

	static void PutBoolean(IShTokenStream* stream, int32 keyword, boolean value) 
	{
#if VERSIONNUMBER >= 0x060000 
		stream->PutBooleanAttribute(keyword, value);
#else
	stream->PutKeyword(keyword);
	stream->PutLong(value);
#endif
	};

	static TMCString255 GetString(IShTokenStream* stream) 
	{
#if VERSIONNUMBER >= 0x060000 
		return stream->GetStringToken();
#else
		char temp[256];
		stream->GetString(temp);
		TMCString255 retValue;
		retValue.FromCPtr(temp);
		return retValue;
#endif
	};

	static void PutString(IShTokenStream* stream, int32 keyword, TMCString255 value) 
	{
#if VERSIONNUMBER >= 0x060000 
		stream->PutStringAttribute(keyword, value);
#else
	stream->PutKeyword(keyword);
	stream->PutString(value.StrGet());
#endif
	};


	static void GetRGBColor(IShTokenStream* stream, TMCColorRGB& value) 
	{
#if VERSIONNUMBER >= 0x060000 
		stream->GetRGBColorToken(value);
#elif VERSIONNUMBER >= 0x050000 
		stream->GetRGBColor(value);
#else
		stream->GetPoint3D(&value.R, &value.G, &value.B);
#endif
	};

	static void PutRGBColor(IShTokenStream* stream, int32 keyword, TMCColorRGB value) 
	{
#if VERSIONNUMBER >= 0x050000 
		stream->PutRGBColorAttribute(keyword, value);
#else
		stream->PutKeyword(keyword);
		stream->PutPoint3D(value.R, value.G, value.B);
#endif
	}


};

#endif