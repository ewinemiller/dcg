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
#ifndef __DCGDEBUG__
#define __DCGDEBUG__


#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"
#include "comutilities.h"
#include "IShUtilities.h"
#if VERSIONNUMBER >= 0x050000
#include "COMSafeUtilities.h"
#include "IShPartUtilities.h"
#endif
inline void Alert(const TMCString& mainMessage)
{
	gShellUtilities->DoAlert(kShNotificationAlert,			// Alert severity icon. See kShNotificationAlert, kShCautionAlert and kShStopAlert constants
		mainMessage,	// Main message to display
		kNullString, 
		kNullString,
		kNullString,			// Substitution strings : any "^0", "^1" and "^2" token in 'mainMessage' will be replaced by 'arg0', 'arg1', and 'arg2'
		1,			// Number of buttons (1, 2 or 3)
		kNullString,		// Text of the default button.
		kNullString,		// Text of the second button.
		kNullString		// Text of the third button.

		// If you give empty strings for buttons, you get the following default buttons:
		//  buttonsCount == 1   => OK
		//  buttonsCount == 2   => OK Cancel
		//  buttonsCount == 3   => Yes No Cancel

	);	// The returned value is the number of the hit button (0, 1 or 2)
}
inline void Alert(const char* mainMessage)
{
	TMCString1023 tempmsg = mainMessage;
	gShellUtilities->DoAlert(kShNotificationAlert,			// Alert severity icon. See kShNotificationAlert, kShCautionAlert and kShStopAlert constants
		tempmsg,	// Main message to display
		kNullString, 
		kNullString,
		kNullString,			// Substitution strings : any "^0", "^1" and "^2" token in 'mainMessage' will be replaced by 'arg0', 'arg1', and 'arg2'
		1,			// Number of buttons (1, 2 or 3)
		kNullString,		// Text of the default button.
		kNullString,		// Text of the second button.
		kNullString		// Text of the third button.

		// If you give empty strings for buttons, you get the following default buttons:
		//  buttonsCount == 1   => OK
		//  buttonsCount == 2   => OK Cancel
		//  buttonsCount == 3   => Yes No Cancel

	);	// The returned value is the number of the hit button (0, 1 or 2)

}

inline void alertAndThrow(TMCString* mainMessage)
{
	Alert(*mainMessage);
	ThrowException(MC_E_UNEXPECTED, mainMessage);

}

#endif