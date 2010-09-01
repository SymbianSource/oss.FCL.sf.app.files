/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0""
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Application panic codes.
*
*/

#ifndef __DEVENCUI_PAN__
#define __DEVENCUI_PAN__

/** DevEncUi application panic codes */
enum TDevEncUiPanics
    {
    EDevEncUi = 1
    // add further panics here
    };

inline void Panic( TDevEncUiPanics aReason )
    {
    _LIT(applicationName,"DevEncUi");
    User::Panic(applicationName, aReason);
    }

#endif // __DEVENCUI_PAN__
