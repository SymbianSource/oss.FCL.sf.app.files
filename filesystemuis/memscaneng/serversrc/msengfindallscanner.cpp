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
*     Scanner class used to scan file system by filename extension
*
*
*/


// SYSTEM INCLUDES
#include    <f32file.h>

// USER INCLUDES
#include    "msengfindallscanner.h"
#include    "msenguihandler.h"
#include    "memscanutils.h"


// ================= MEMBER FUNCTIONS ========================================

// ---------------------------------------------------------------------------
// CMsengFindAllScanner::CMsengFindAllScanner()
//
// C++ default constructor. Can NOT contain any code, that might leave.
// ---------------------------------------------------------------------------
CMsengFindAllScanner::CMsengFindAllScanner(MMsengScannerObserver& aObserver, 
                                           CMsengInfoArray& aScanArray, 
                                           RFs& aFsSession)
: CMsengFileScanner(aObserver, aScanArray, aFsSession)
    {
    }


// ---------------------------------------------------------------------------
// CMsengFindAllScanner::NewL()
//
// Two-phased constructor
// ---------------------------------------------------------------------------
CMsengFindAllScanner* CMsengFindAllScanner::NewL(
                                        MMsengScannerObserver& aObserver, 
                                        CMsengInfoArray& aScanArray, 
                                        RFs& aFsSession)
    {
    CMsengFindAllScanner* self = new (ELeave) CMsengFindAllScanner(aObserver, 
                                                    aScanArray, aFsSession);
    return self;
    }

// ---------------------------------------------------------------------------
// CMsengFindAllScanner::~CMsengFindAllScanner()
//
// Destructor
// ---------------------------------------------------------------------------
CMsengFindAllScanner::~CMsengFindAllScanner()
    {
    }

// ---------------------------------------------------------------------------
// CMsengFindAllScanner::FindFilesL()
//
//
// ---------------------------------------------------------------------------
CDir* CMsengFindAllScanner::FindFilesL(const TDesC& aDirectory, TBool& aMoveToNextDirectory)
    {
    TRACES( RDebug::Print(_L("CMsengFindAllScanner::FindFilesL(%S)"), &aDirectory) );

    // Get a list of results for this directory
    CDir* results;
    const TInt error = FsSession().GetDir(aDirectory, 
        KEntryAttMaskSupported|KEntryAttAllowUid, ESortNone, results);
    if (error == KErrNotFound)
        {
        results = NULL;
        }

    // Always move onto searching the next directory
    aMoveToNextDirectory = ETrue;
    
    // Return populated (or potentially NULL) list.
    return results;
    }

// ---------------------------------------------------------------------------
// CMsengFindAllScanner::HandleLocatedEntryL()
// 
// 
// ---------------------------------------------------------------------------
CMsengFileScanner::TLocationResponse CMsengFindAllScanner::HandleLocatedEntryL(
    const TDesC& aFullFileNameAndPath, const TEntry& aEntry)
    {
    // Figure out the data group by comparing the start of the path

    TInt dataDirCount = InfoArray().DataDirs().Count();
    for(TInt i=0; i < dataDirCount; i++)
        {
        TPtrC dataDirPath = InfoArray().DataDirs().MdcaPoint(i);
        if(aFullFileNameAndPath.Length() >= dataDirPath.Length())
            {
            TFileName currentPath;
            currentPath.Copy(aFullFileNameAndPath.Left(dataDirPath.Length()));

            // Compare whether folder matches
            if(!currentPath.Compare(dataDirPath))
                {
                TBool isExcluded = EFalse;
                TInt fileLength = aFullFileNameAndPath.Length() - dataDirPath.Length();
                TInt excludedFiles = 0;
                if(InfoArray().DataDirExcludedFiles().Count())
                    {
                    excludedFiles = InfoArray().DataDirExcludedFiles().At(i)->MdcaCount();
                    }

                currentPath.Copy(aFullFileNameAndPath.Right(fileLength));

                TRACES( RDebug::Print(_L("Check file %S"), &currentPath) );
                
                for(TInt j=0; j < excludedFiles; j++)
                    {
                    TRACES
                    ( 
                    TPtrC file = InfoArray().DataDirExcludedFiles().At(i)->MdcaPoint(j);
                    RDebug::Print(_L("Comparing to excluded file %S"), &file);
                    );
                    
                    if(!currentPath.Compare(
                        InfoArray().DataDirExcludedFiles().At(i)->MdcaPoint(j)))
                        {
                        isExcluded = ETrue;
                        break;
                        }
                    }

                if(!isExcluded)
                    {
                    // Add size of object when file is not in list of excluded files
                    if(InfoArray().DataDirGroups().Count())
                        {
                        TInt group = InfoArray().DataDirGroups().At(i);
                        InfoArray().AddSizeByGroupL(group, aEntry.FileSize());

                        TRACES
                        ( 
                        RDebug::Print(_L("File %S belongs to group: %d"), 
                            &aFullFileNameAndPath, group)
                        );
                        }
                    }
                }
            }
        }
        
    // We processed this one
    return EEntryWasProcessed;
    }

//  End of File
