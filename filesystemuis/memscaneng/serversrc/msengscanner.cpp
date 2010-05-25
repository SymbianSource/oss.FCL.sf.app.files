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
*     This is the class that is in control most of the time.
*     It instantiates CMsengScannerBase derived scanner classes
*     to do the job.
*
*/


// SYSTEM INCLUDES
#include <bautils.h>
#include <mseng.rsg>

// USER INCLUDES
#include "msengscanner.h"
#include "msengfileextscanner.h"
#include "msengfindallscanner.h"
#include "msengregistryscanner.h"
#include "msenguihandler.h"
#include "mseng.h"


// LOCAL CONSTANTS AND MACROS
#ifdef _DEBUG
_LIT(KClassName, "CMsengScanner");
#endif

// ================= MEMBER FUNCTIONS ========================================

// ---------------------------------------------------------------------------
// CMsengScanner::CMsengScanner()
//
// C++ default constructor. Can NOT contain any code, that might leave.
// ---------------------------------------------------------------------------
CMsengScanner::CMsengScanner(MMsengUIHandler& aUIHandler, CResourceFile& aResFile) :
iUIHandler(aUIHandler), iResFile(aResFile)
    {
    }

// ---------------------------------------------------------------------------
// CMsengScanner::~CMsengScanner()
//
// Destructor
// ---------------------------------------------------------------------------
CMsengScanner::~CMsengScanner()
    {
    delete iScanArray;
    delete iFileExtScanner;
    delete iFindAllScanner;
    delete iRegistryScanner;
    }



// ---------------------------------------------------------------------------
// CMsengScanner::Cancel()
// 
// ---------------------------------------------------------------------------
//
void CMsengScanner::Cancel()
    {
    iFileExtScanner->Cancel();
    iFindAllScanner->Cancel();
    iRegistryScanner->Cancel();
    TRAP_IGNORE( iUIHandler.QuitL(KErrCancel) );
    }

// ---------------------------------------------------------------------------
// CMsengScanner::HaveActiveScanners()
// 
// ---------------------------------------------------------------------------
//
TBool CMsengScanner::HaveActiveScanners() const
    {
    const TBool isActive = 
            (iFileExtScanner && iFileExtScanner->IsActive())
            || (iFindAllScanner && iFindAllScanner->IsActive())
            || (iRegistryScanner && iRegistryScanner->IsActive())
		 ;
	return isActive;
	}

// ---------------------------------------------------------------------------
// CMsengScanner::ScanL()
// 
// ---------------------------------------------------------------------------
//
TInt CMsengScanner::ScanL(TDriveNumber aDrive, TInt aNumberOfDataGroups, RFs& aFsSession)
    {
    if(HaveActiveScanners())
        {
        return KErrInUse;
        }

    // Create data structures; delete the old ones
    CMsengInfoArray* infoArray = CMsengInfoArray::NewL( aDrive,
                                                        aNumberOfDataGroups,
                                                        aFsSession, iResFile );
    
    delete iScanArray;
	iScanArray = infoArray;

    // Do some scanning tasks not include in ScanL
    PreScanL(aDrive, aFsSession);

    // Start the scan by filename extension
    iFileExtScanner = CMsengFileExtScanner::NewL(*this, *iScanArray, aFsSession);
    iFileExtScanner->ScanL(iScanArray->Dirs());

    // Start scanning data dirs (including messages)
    iFindAllScanner = CMsengFindAllScanner::NewL(*this, *iScanArray, aFsSession);
    iFindAllScanner->ScanL(iScanArray->DataDirs());

    // Start the scan from registries
    iRegistryScanner = CMsengRegistryScanner::NewL( aDrive, *this, *iScanArray, aFsSession );
    iRegistryScanner->Scan();

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// MsengScanner::HandleScannerEventL()
// 
// Handle the events from scanning
// ---------------------------------------------------------------------------
//
void CMsengScanner::HandleScannerEventL(TScannerEvent aEvent,
                                        const CMsengScannerBase& /*aScanner*/,
                                        TInt aError)
    {
#ifdef __SHOW_RDEBUG_PRINT_
    RDebug::Print(_L("CMsengScanner::HandleScannerEventL() called with event: %d"), aEvent);
#endif // __SHOW_RDEBUG_PRINT_

    switch(aEvent)
        {
        case MMsengScannerObserver::EScannerEventScanComplete:
            // This scanner object has finished all of its scanning.
            // If all the others are finished too,
            // then we tell the UI that scanning is complete.
            if (!HaveActiveScanners())
                {
                iUIHandler.QuitL(KErrNone);
                }
            break;

        case MMsengScannerObserver::EScannerEventScanError:
            // An error has happened
            iUIHandler.ErrorL(aError);
            break;

        default:
            // should never happen
            __ASSERT_DEBUG(EFalse,User::Panic(KClassName, KErrGeneral));
            break;
        }
    }

// ---------------------------------------------------------------------------
// CMsengScanner::PreScanL()
// 
// ---------------------------------------------------------------------------
//
void CMsengScanner::PreScanL(TDriveNumber aDrive, RFs& aFsSession)
    {
    // Before actual scanning, check the sizes of the few extra
    // data files that are not scanned in normal way.

    RResourceReader theReader;  
    
    TBool somethingToDo = EFalse;
    if( CMseng::IsInternalDrive( aFsSession, aDrive )
        && !CMseng::IsMassStorageDrive( aFsSession, aDrive ) )
        {
        theReader.OpenLC( &iResFile, C_EXTRADATAFILES );
        somethingToDo = ETrue;
        }
    else
        {
        theReader.OpenLC( &iResFile, E_EXTRADATAFILES );
        somethingToDo = ETrue;
        }
    
    if(somethingToDo)
        {
        //the first WORD contains the number of elements in the resource
        const TInt length = theReader.ReadInt16L();

        // Read the array resource, 
        TInt groupindex; // value from enum TDataGroups
        for(TInt i=0; i<length; i++)
            {
            groupindex = theReader.ReadInt8L();
            HBufC* name = theReader.ReadHBufCL();
            TChar ch;
            if ( RFs::DriveToChar( aDrive, ch ) == KErrNone )
                {
                CleanupStack::PushL(name);
                TPtr ptrName = name->Des();
                TBuf<1> drive;
                drive.Append(ch);
                ptrName.Replace(0, drive.Length(), drive);

                // check the size of the file whose path was in the resource.
                TEntry file;
                TInt err = aFsSession.Entry(ptrName, file);
                if (err == KErrNone)
                    {
                    TInt size = file.iSize;
                    iScanArray->AddSizeByGroupL(groupindex, size);
                    }
                    
                CleanupStack::PopAndDestroy(name);
                }
            }
        }
    CleanupStack::PopAndDestroy(&theReader);
    }

//  End of File  
