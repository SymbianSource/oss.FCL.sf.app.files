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
*       Scan directories
*
*/


// SYSTEM INCLUDES
#include <bautils.h>

// USER INCLUDES
#include "msengdirectoryscanner.h"


// CONSTANTS
_LIT(KMsengPathDelimiter, "\\");


// ================= MEMBER FUNCTIONS ========================================

// ---------------------------------------------------------------------------
// CMsengDirectoryScanner::CMsengDirectoryScanner()
//
// Default C++ constructor
// ---------------------------------------------------------------------------
CMsengDirectoryScanner::CMsengDirectoryScanner(
                                            MMsengScannerObserver& aObserver,
                                            CMsengInfoArray& aScanArray,
                                            RFs& aFsSession )
: CMsengScannerBase(aObserver, aScanArray, aFsSession)
    {
    }


// ---------------------------------------------------------------------------
// CMsengDirectoryScanner::~CMsengDirectoryScanner()
//
// Destructor
// ---------------------------------------------------------------------------
CMsengDirectoryScanner::~CMsengDirectoryScanner()
    {
    delete iDirectoryList;
    delete iScanner;
    }







// ---------------------------------------------------------------------------
// CMsengDirectoryScanner::ScanL()
// 
//
// ---------------------------------------------------------------------------
void CMsengDirectoryScanner::ScanL(const CDesCArray& aRootDirs)
    {
    if (iDirectoryList)
        {
        iDirectoryList->Reset();
        }
    else
        {
        iDirectoryList = new(ELeave) CDesCArraySeg(KDirectoryListGranularity);
        }
    if  (!iScanner)
        {
        iScanner = CDirScan::NewL(FsSession());
        }
    // Copy existing directories over
    const TInt count = iNumberOfRootDirectories = aRootDirs.Count();
    for(TInt i=0; i<count; i++)
        {
        // Make sure the copied root dirs exist. Otherwise the RunL()
        // would leave with KErrPathNotFound. After having the list
        // of root dirs, all scanned dirs are taken from the list obtained
        // by a CDirScan instance and are thus known to exist because 
        if( InfoArray().FolderExists(FsSession(),aRootDirs[i]) ) 
            {            
            iDirectoryList->AppendL(aRootDirs[i]);
#ifdef __SHOW_RDEBUG_PRINT_
            TPtrC dir = aRootDirs[i];
            RDebug::Print(_L("Root directory: %S"), &dir);
#endif // __SHOW_RDEBUG_PRINT_
            }
        else 
            {
            iNumberOfRootDirectories--;
            }
        }

    // Start the scan going
    CompleteRequest();
    }


// ---------------------------------------------------------------------------
// CMsengDirectoryScanner::PerformStepL()
// 
//
// ---------------------------------------------------------------------------
CMsengScannerBase::TStepResult CMsengDirectoryScanner::PerformStepL()
    {
    TStepResult result = ECompleteRequest;
    RFs& fsSession = FsSession();

    // Expand the root directories to a full list of 
    // subdirectories
    if (iState == EExpandingRootDirs)
        {
        if (iCurrentIndex >= iNumberOfRootDirectories)
            {
            // Finished scanning for the subdirectories. 
            // Reset the current index so that we pass all the directories
            // (including the root paths) to the subclasses.
            iCurrentIndex = 0;
            iState = EScanningSubDirs;
            }
        else
            {
            const TPtrC pFolder(iDirectoryList->MdcaPoint(iCurrentIndex++));

            // Do the recursive scanning: First set scan data.
            iScanner->SetScanDataL(
                pFolder,
                KEntryAttDir|KEntryAttMatchExclusive,
                ESortNone,
                CDirScan::EScanDownTree
                );

            // Add all the located subdirectories to the array
            iState = EParsingExpandedList;
            }
        }
    else if (iState == EParsingExpandedList)
        {
        CDir* list = NULL;
        iScanner->NextL(list);
        
        if (list)
            {
            CleanupStack::PushL(list);
        
            const TPtrC pFullPath(iScanner->FullPath());
            TFileName file;
        
            const TInt count = list->Count();
            for(TInt i=0; i<count; i++)
                {
                const TPtrC pEntry((*list)[i].iName);
                file = pFullPath;
                file += pEntry;
                file += KMsengPathDelimiter;
#ifdef __SHOW_RDEBUG_PRINT_
                // Don't add text, all print space (256 chars) may be required
                RDebug::Print(_L("%S"), &file); 
#endif // __SHOW_RDEBUG_PRINT_
                AppendDirectoryL(file);
                }
            //
            CleanupStack::PopAndDestroy(list);
            }
        else
            {
            // No more processing to do in this state
            iState = EExpandingRootDirs;
            }
        }
    else if (iState == EScanningSubDirs)
        {
        // Scan this folder
        if (iCurrentIndex < iDirectoryList->Count())
            {
            const TPtrC pFolder(iDirectoryList->MdcaPoint(iCurrentIndex));
            const TScanDirectoryResult scanDirectoryResult = 
                ScanDirectoryL(pFolder, fsSession);
            
            // Check whether we continue with this directory next time
            if (scanDirectoryResult == EContinueToNextDirectory)
                {
                iCurrentIndex++;
                }
            }
        else
            {
            // All directories scanned now
            result = EScanComplete;
            }
        }

    // Return the response back to the base scanner
    return result;
    }


// ---------------------------------------------------------------------------
// CMsengDirectoryScanner::AppendDirectoryL()
//
//
// ---------------------------------------------------------------------------
void CMsengDirectoryScanner::AppendDirectoryL(const TDesC& aDirectory)
    {
    // Append the directory to the directory list,
    // unless it is listed as excluded directory
    if( IsSpecialDir(aDirectory) || !IsExcludedDir(aDirectory) )
        {
        iDirectoryList->AppendL(aDirectory);
        }
    else
        {
#ifdef __SHOW_RDEBUG_PRINT_
        RDebug::Print(_L("Dir is excluded! %S"), &aDirectory);
#endif // __SHOW_RDEBUG_PRINT_
        }
    }

//  End of File  
