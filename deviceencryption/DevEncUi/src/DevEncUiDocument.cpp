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
* Description:  Implementation of CDevEncUiDocument.
*
*/

// INCLUDE FILES
#include "DevEncLog.h"
#include "DevEncUiAppui.h"
#include "DevEncUiDocument.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CDevEncUiDocument::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CDevEncUiDocument* CDevEncUiDocument::NewL( CEikApplication&
                                                          aApp )
    {
    CDevEncUiDocument* self = NewLC( aApp );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CDevEncUiDocument::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CDevEncUiDocument* CDevEncUiDocument::NewLC( CEikApplication&
                                                           aApp )
    {
    CDevEncUiDocument* self =
        new ( ELeave ) CDevEncUiDocument( aApp );

    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CDevEncUiDocument::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CDevEncUiDocument::ConstructL()
    {
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CDevEncUiDocument::CDevEncUiDocument()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CDevEncUiDocument::CDevEncUiDocument( CEikApplication& aApp )
    : CAknDocument( aApp )
    {
    // No implementation required
    }

// ---------------------------------------------------------------------------
// CDevEncUiDocument::~CDevEncUiDocument()
// Destructor.
// ---------------------------------------------------------------------------
//
CDevEncUiDocument::~CDevEncUiDocument()
    {
    // No implementation required
    }

// ---------------------------------------------------------------------------
// CDevEncUiDocument::CreateAppUiL()
// Constructs CreateAppUi.
// ---------------------------------------------------------------------------
//
CEikAppUi* CDevEncUiDocument::CreateAppUiL()
    {
    DFLOG( "CDevEncUiDocument::CreateAppUiL" );
    // Create the application user interface, and return a pointer to it
    // the framework takes ownership of this object
    return ( static_cast <CEikAppUi*> ( new ( ELeave )
                                        CDevEncUiAppUi ) );
    }

// -----------------------------------------------------------------------------
// CDevEncUiDocument::OpenFileL
// -----------------------------------------------------------------------------
//
CFileStore* CDevEncUiDocument::OpenFileL(
    TBool /*aDoOpen*/, 
    const TDesC& aFilename,
    RFs& /*aFs*/)
    {
    iAppUi->OpenFileL(aFilename);
    return NULL;
    }

// -----------------------------------------------------------------------------
// void CDevEncUiDocument::OpenFileL()
// -----------------------------------------------------------------------------
//   
void CDevEncUiDocument::OpenFileL(
	CFileStore*& /*aFileStore*/,
	RFile& aFile)
	{
	((CDevEncUiAppUi*) iAppUi)->OpenFileL(aFile);
	}

// End of File
