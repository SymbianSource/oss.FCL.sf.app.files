/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
*     Scanner class used to scan file system by filename extensions.
*
*/


// USER INCLUDES
#include    "msengfileextscanner.h"
#include    "msenguihandler.h"


// ================= MEMBER FUNCTIONS ========================================

// ---------------------------------------------------------------------------
// CMsengFileExtScanner::CMsengFileExtScanner()
//
// C++ default constructor. Can NOT contain any code, that might leave.
// ---------------------------------------------------------------------------
CMsengFileExtScanner::CMsengFileExtScanner(MMsengScannerObserver& aObserver, 
                                           CMsengInfoArray& aScanArray,
                                           RFs& aFsSession)
: CMsengFileScanner(aObserver, aScanArray, aFsSession), iMoveToNextDirectory(ETrue)
    {
    }

// ---------------------------------------------------------------------------
// CMsengFileExtScanner::NewL()
//
// Two-phased constructor
// ---------------------------------------------------------------------------
CMsengFileExtScanner* CMsengFileExtScanner::NewL(MMsengScannerObserver& aObserver, 
                                                 CMsengInfoArray& aScanArray,
                                                 RFs& aFsSession)
    {
    CMsengFileExtScanner* self = 
        new (ELeave) CMsengFileExtScanner(aObserver, aScanArray, aFsSession);
    return self;
    }    

// ---------------------------------------------------------------------------    
// CMsengFileExtScanner::~CMsengFileExtScanner()
//
// Destructor
// --------------------------------------------------------------------------- 
CMsengFileExtScanner::~CMsengFileExtScanner()
    {
    }




// ---------------------------------------------------------------------------
// CMsengFileExtScanner::FindFilesL()
// 
// ---------------------------------------------------------------------------
//
CDir* CMsengFileExtScanner::FindFilesL(const TDesC& aDirectory, TBool& aMoveToNextDirectory)
    {

    // iMoveToNextDirectory is true only when starting to handle the
    // current directory. Reset iCurrentExtensionIndex.
    if(iMoveToNextDirectory)
        {
        iCurrentExtensionIndex = -1;
        iMoveToNextDirectory = EFalse;
#ifdef __SHOW_RDEBUG_PRINT_
		RDebug::Print(_L("Entering directory:"));
		RDebug::Print(_L("%S"), &aDirectory);
#endif // __SHOW_RDEBUG_PRINT_
        }

    // This function performs a search for each file in the directory by extension.
    iCurrentExtensionIndex++;
    const TPtrC pCurrentExtension(InfoArray().Exts()[iCurrentExtensionIndex]);

#ifdef __SHOW_RDEBUG_PRINT_
	RDebug::Print(_L("Searching files with extension %d"), iCurrentExtensionIndex);
#endif // __SHOW_RDEBUG_PRINT_

    // Get a list of results for this directory
    CDir* results = NULL;
    TParse parse;
	TInt error;

	const TInt pathlength = pCurrentExtension.Length() + aDirectory.Length();
	if ( pathlength > KMaxFileName )
	{
	error = KErrNotFound;	
#ifdef __SHOW_RDEBUG_PRINT_
	RDebug::Print(_L("Path too long, files with extension %d do not fit to directory"), 
	    iCurrentExtensionIndex);
#endif // __SHOW_RDEBUG_PRINT_
	}
	
	else
	{	
	FsSession().Parse(pCurrentExtension, aDirectory, parse);
	error = FsSession().GetDir(parse.FullName(), KEntryAttMaskSupported|KEntryAttAllowUid, 
	    ESortNone, results);
	}

    if (error == KErrNotFound)
        {
        results = NULL;
        }

    // Should we move onto searching the next directory
    // Yes, if this was the last extension.
    const TInt extensionCount = InfoArray().Exts().Count();
    iMoveToNextDirectory = (iCurrentExtensionIndex >= extensionCount-1);
    aMoveToNextDirectory = iMoveToNextDirectory;
	
    // Return populated (or potentially NULL) list.
    return results;
    }

// ---------------------------------------------------------------------------
// CMsengFileExtScanner::HandleLocatedEntryL()
// 
// 
// ---------------------------------------------------------------------------
//
CMsengFileScanner::TLocationResponse CMsengFileExtScanner::HandleLocatedEntryL(
					const TDesC& aFullFileNameAndPath, const TEntry& aEntry)
    {
    TLocationResponse response = EEntryWasDiscarded;
    const TInt KUidLocation = 2;
    TUid fileUid = aEntry[KUidLocation];
    
    if ( fileUid == KNullUid )
        {
        // Check whether extension found in special data dir
        TBool addSize( ETrue );
        TInt dataDirCount = InfoArray().DataDirs().Count();
        for(TInt i=0; i < dataDirCount; i++)
            {
            TPtrC dataDirPath = InfoArray().DataDirs().MdcaPoint(i);

            if(aFullFileNameAndPath.Length() >= dataDirPath.Length())
                {
                TFileName currentPath;
                currentPath.Copy(aFullFileNameAndPath.Left(dataDirPath.Length()));

                // Compare whether folder matches
                if(!currentPath.CompareF(dataDirPath))
                    {
                    addSize = EFalse;
                    break;
                    }
                }
            }

        if( addSize )
            {
            // Add size of object
            InfoArray().AddSizeByExtL(iCurrentExtensionIndex,  aEntry.FileSize() );

#ifdef __SHOW_RDEBUG_PRINT_
        	const TInt pathlength = aFullFileNameAndPath.Length();
        	
        	if ( pathlength < KMaxFileName )
        		{	
        		RDebug::Print(_L("File: %S, extension number: %d, size: %d"), 
        		    &aFullFileNameAndPath, iCurrentExtensionIndex,  aEntry.FileSize());
        		}
        	else
        		{
        		RDebug::Print(_L("File: see next line, extension number: %d, size: %d"), 
        		    iCurrentExtensionIndex, aEntry.FileSize() );
        		RDebug::Print(_L("Can not print %d characters long file name"), pathlength );
        		}
#endif // __SHOW_RDEBUG_PRINT_

            // We processed this one
            response = EEntryWasProcessed;
            }
        }
    else
        {
#ifdef __SHOW_RDEBUG_PRINT_
        RDebug::Print(_L("Handling file: %S, file Uid: 0x%x"), 
            &aFullFileNameAndPath, fileUid.iUid);
#endif // __SHOW_RDEBUG_PRINT_
    
        // Make sure other than native applications are not calculated
        if( iCurrentExtensionIndex == EExtSis || iCurrentExtensionIndex == EExtSisx )
            {
            InfoArray().AddSizeByExtL(iCurrentExtensionIndex, aEntry.FileSize() );
            }
        }
        
    return response;
    }

//  End of File 
