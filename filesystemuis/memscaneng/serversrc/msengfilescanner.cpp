/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*       Scanner class used to scan file system by file UID.
*
*/


// USER INCLUDES
#include "msengfilescanner.h"



// ================= MEMBER FUNCTIONS ========================================

// ---------------------------------------------------------------------------
// CMsengFileScanner::CMsengFileScanner()
//
// Constructor
// ---------------------------------------------------------------------------
CMsengFileScanner::CMsengFileScanner(MMsengScannerObserver& aObserver,
                                     CMsengInfoArray& aScanArray,
                                     RFs& aFsSession)
: CMsengDirectoryScanner(aObserver, aScanArray, aFsSession)
    {
    }

// ---------------------------------------------------------------------------
// CMsengFileScanner::~CMsengFileScanner()
//
// Destructor
// ---------------------------------------------------------------------------

CMsengFileScanner::~CMsengFileScanner()
    {
    }

// ---------------------------------------------------------------------------
// CMsengFileScanner::ScanDirectoryL()
//
// 
// ---------------------------------------------------------------------------
CMsengDirectoryScanner::TScanDirectoryResult CMsengFileScanner::ScanDirectoryL
                                (const TDesC& aDirectory, RFs& /*aFsSession*/)
    {
    TBool moveToNextDirectory = ETrue;
    CDir* results = FindFilesL(aDirectory, moveToNextDirectory);
    if (results)
        {
        CleanupStack::PushL(results);

        // Go through all files in the list and tell subclass
        TFileName file;
        const TInt count = results->Count();
        for(TInt i=0; i<count; i++)
            {
            const TEntry& entry = (*results)[i];
            file = aDirectory;
            file += entry.iName;
            
            // Tell subclass about identified file
            const TLocationResponse response = HandleLocatedEntryL(file, entry);
            }
        CleanupStack::PopAndDestroy(results);
        }


    // Should we request that we move onto the next directory in the list
    TScanDirectoryResult response = EContinueToNextDirectory;
    if (!moveToNextDirectory)
        {
        response = EContinueProcessingCurrentDirectory;
        }
    return response;
    }

//  End of File  
