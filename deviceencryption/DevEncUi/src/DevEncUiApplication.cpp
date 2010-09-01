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
* Description:  Implementation of CDevEncUiApplication.
*
*/

// INCLUDE FILES
#include "DevEncLog.h"
#include "DevEncUiDocument.h"
#include "DevEncUiApplication.h"
#include "DevEncUids.hrh"

// ============================ MEMBER FUNCTIONS ===============================

// UID for the application
// this should correspond to the uid defined in the mmp file
const TUid KUidDevEncUIApp = { KDevEncUiUid };

// --------------------------------------------------------------------------
// CDevEncUiApplication::AppDllUid()
// Returns application UID
// --------------------------------------------------------------------------
//
TUid CDevEncUiApplication::AppDllUid() const
    {
    // Return the UID for the DevEncUI application
    return KUidDevEncUIApp;
    }

// --------------------------------------------------------------------------
// CDevEncUiApplication::CreateDocumentL()
// Creates CApaDocument object
// --------------------------------------------------------------------------
//
CApaDocument* CDevEncUiApplication::CreateDocumentL()
    {
    DFLOG( "CDevEncUiApplication::CreateDocumentL" );
    // Create an DevEncUI document, and return a pointer to it
    return ( static_cast<CApaDocument*>
                    ( CDevEncUiDocument::NewL( *this ) ) );
    }

// End of File
