/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  File Manager application class.
*
*/



// INCLUDE FILES
#include <eikstart.h>
#include "CFileManagerApplication.h"
#include "CFileManagerDocument.h"
#include "FileManagerUID.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerApplication::CFileManagerApplication
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerApplication::CFileManagerApplication()
    {
    }
    
// -----------------------------------------------------------------------------
// CFileManagerApplication::~CFileManagerApplication
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerApplication::~CFileManagerApplication()
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerApplication::CreateDocumentL
// 
// -----------------------------------------------------------------------------
// 
CApaDocument* CFileManagerApplication::CreateDocumentL()
	{
	return CFileManagerDocument::NewL( *this );
	}

// -----------------------------------------------------------------------------
// CFileManagerApplication::AppDllUid
// 
// -----------------------------------------------------------------------------
// 
TUid CFileManagerApplication::AppDllUid() const
	{
	return TUid::Uid( KFileManagerUID3 );
	}

// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// NewApplication
// Creates a new FileManager application object (CFileManagerApplication). 
// Called by the application framework.
// Returns the new application object (CFileManagerApplication). 
// NULL if out of memory.
// -----------------------------------------------------------------------------
// 
EXPORT_C CApaApplication* NewApplication()
    {
	return new CFileManagerApplication;
    }

// -----------------------------------------------------------------------------
// E32Main
// Standard entry point for the application exe (FileManager.exe).
// Returns standard Symbian OS error code, KErrNone if succesful.
// -----------------------------------------------------------------------------
//
GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication( NewApplication );
    }

//  End of File  
