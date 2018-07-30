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
#include "DCGBezier.h"
#include "MFPartMessages.h"
#include "commessages.h"
#include "IMFCurvePart.h"
#include "IMFPart.h"
#include "copyright.h"

void DCGBezier::InitCurve()
{
	fBezierCurve.fLocations.ArrayFree();

	TMCLocationRec newLoc;
	newLoc.fSegmentBeginPos=0.0f;
	newLoc.fLocation=TVector3(0.0f, 0.0f, 0.0f);
	newLoc.SetPrevControlLocation(TVector3(-0.25f, -0.25f, 0.0f));
	newLoc.SetNextControlLocation( TVector3(0.25f, 0.25f, 0.0f) );
	fBezierCurve.SetLocation(newLoc);

	newLoc.fSegmentBeginPos=1.0f;
	newLoc.fLocation=TVector3(1.0f, 1.0f, 0.0f);
	newLoc.SetPrevControlLocation( TVector3(0.75f, 0.75f, 0.0f) );
	newLoc.SetNextControlLocation( TVector3(1.25f, 1.25f, 0.0f) );
	fBezierCurve.SetLocation(newLoc);

	fBezierCurve.SetIsLinear(true);

	fBezierCurve.ReCalcAllApprox();
}

MCCOMErr DCGBezier::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
{

         int8 token[256];
         MCCOMErr result;

         result = stream->GetBegin();
         if (result) return result;

         result=stream->GetNextToken(token);
         if (result) return result;

         // remove the default points from the curve
         fBezierCurve.fLocations.ArrayFree();

         while (!stream->IsEndToken(token))
         {
                 int32 keyword;
                 stream->CompactAttribute(token,&keyword);

                 switch (keyword)
                 {
                         case 'line':
                         {
                                 int32 linear=0;
#if (VERSIONNUMBER >= 0x080000)
                                 linear = stream->GetInt32Token();
#else
                                 stream->GetLong(&linear);
#endif
                                 fBezierCurve.SetIsLinear((boolean)linear);
                         } break;
                         case 'locn':
                         {
                                 TMCLocationRec * loc=new TMCLocationRec();
                                 TMCLocationRecReadWrite reader(loc);
                                 reader.Read(stream);
                                 fBezierCurve.fLocations.AddElem(loc);
                         } break;

                 default:
                         stream->SkipTokenData();
                         break;
                 };
                 result=stream->GetNextToken(token);
                 if (result)
                 {
                         // An error occured, restore a default curve
                         	InitCurve();
                         return result;
                 }
         }

         fBezierCurve.ReCalcAllApprox();

         return result;
}

MCCOMErr DCGBezier::Write(IShTokenStream* stream)
{
         MCCOMErr result;

#if (VERSIONNUMBER >= 0x080000)
         stream->PutInt32Attribute('line', (int32)fBezierCurve.IsLinear());
#else
         result=stream->PutKeyword('line');
         if (MCCOMERR_SEVERITY(result))
                 return result;
         result=stream->PutLong((int32)fBezierCurve.IsLinear());
         if (MCCOMERR_SEVERITY(result))
                 return result;
#endif

         TMCPtrArray<TMCLocationRec>::iterator iter(fBezierCurve.fLocations);
         for (TMCLocationRec * loc=iter.First(); iter.More() ; loc=iter.Next())
         {
                 result=stream->PutKeywordAndBegin('locn');
                 if (MCCOMERR_SEVERITY(result))
                         return result;

                 TMCLocationRecReadWrite writer(loc);

                 result=writer.Write(stream);
                 if (MCCOMERR_SEVERITY(result))
                         return result;

                 result=stream->PutEnd();
                 if (MCCOMERR_SEVERITY(result))
                         return result;
         }

         return MC_S_OK;
}

MCCOMErr DCGBezier::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
     if(message == kMsg_CUIP_ComponentAttached)
     {
             TMCCountedPtr<IMFPart> sourcePart;
             source->QueryInterface(IID_IMFPart, (void **)&sourcePart);

             if(MCVerify(sourcePart))
             {
                     TMCCountedPtr<IMFPart> buttonPart;
                     sourcePart->FindChildPartByID(&buttonPart, 'butn');

                     if (MCVerify(buttonPart))
                     {
                             TMCCountedPtr<IMFCurvePart> curvePart;
                             buttonPart->QueryInterface(IID_IMFCurvePart, (void **)&curvePart);
                             if(MCVerify(curvePart))
                             {
                                     // Init when clicking on the button
                                     curvePart->SetBezierCurve(&fBezierCurve);
                             }
                     }
             }
     }
	return MC_S_OK;
}
