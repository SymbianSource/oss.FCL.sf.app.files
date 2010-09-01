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
* Description:  Memory scan engine registry scanning
*
*/


// SYSTEM INCLUDES
#include <swi/sisregistryentry.h>
#include <swi/sisregistrysession.h>
#include <AknUtils.h>

#include <SWInstApi.h>
#include <swi/sisregistrypackage.h>
#include <CUIDetailsDialog.h>
#include <SWInstDefs.h>
#include <StringLoader.h>
#include <mseng.rsg>

// USER INCLUDES
#include "msengregistryscanner.h"
#include "msengsisxinfo.h"
#include "memscanutils.h"

// ============================ MEMBER FUNCTIONS ===============================

// ---------------------------------------------------------------------------
// CMsengRegistryScanner::CMsengRegistryScanner()
//
// Default C++ constructor
// ---------------------------------------------------------------------------
CMsengRegistryScanner::CMsengRegistryScanner( 
                                            TDriveNumber aDrive, 
                                            MMsengScannerObserver& aObserver, 
                                            CMsengInfoArray& aScanArray, 
                                            RFs& aFsSession)
    : CMsengScannerBase(aObserver, aScanArray, aFsSession),
    iType( ERegistrySisx ),
    iDrive( aDrive )
    {
    }

// ---------------------------------------------------------------------------
// CMsengRegistryScanner::NewL()
//
// Two-phased constructor
// ---------------------------------------------------------------------------
CMsengRegistryScanner* CMsengRegistryScanner::NewL( 
                                                TDriveNumber aDrive, 
                                                MMsengScannerObserver& aObserver, 
                                                CMsengInfoArray& aScanArray,
                                                RFs& aFsSession)
    {
    CMsengRegistryScanner* self = new (ELeave) CMsengRegistryScanner(aDrive,
        aObserver, aScanArray, aFsSession);
        
    return self;
    }    


// ---------------------------------------------------------------------------
// CMsengRegistryScanner::~CMsengRegistryScanner()
//
// Destructor
// ---------------------------------------------------------------------------
CMsengRegistryScanner::~CMsengRegistryScanner()
    {
    }

// ---------------------------------------------------------------------------
// CMsengRegistryScanner::Scan()
// 
//
// ---------------------------------------------------------------------------
void CMsengRegistryScanner::Scan()
    {
    // Start the scan going
    CompleteRequest();
    }


// ---------------------------------------------------------------------------
// CMsengRegistryScanner::PerformStepL()
// 
//
// ---------------------------------------------------------------------------
CMsengScannerBase::TStepResult CMsengRegistryScanner::PerformStepL()
    {
    TStepResult result = ECompleteRequest;

    // Scan requested registry
    switch( iType )
        {
        case ERegistrySisx:
            {
            ScanSisRegistryL( );
            iType = ERegistryLast;
            break;
            }
        case ERegistryLast:
        default:
            {
            // That's all we had to do
            result = EScanComplete;
            break;
            }
        }

    // Return the response back to the base scanner
    return result;
    }
     

// ---------------------------------------------------------------------------
// CMsengRegistryScanner::ScanSisRegistryL()
// 
//
// ---------------------------------------------------------------------------
void CMsengRegistryScanner::ScanSisRegistryL()
    {
    Swi::RSisRegistrySession regSession;
    CleanupClosePushL(regSession);
    User::LeaveIfError( regSession.Connect() );

    RArray<TUid> uids;
    CleanupClosePushL(uids);
    regSession.InstalledUidsL( uids );    

    RPointerArray<Swi::CSisRegistryPackage> augmentations;
    
    TInt64 totalSize(0);
    
    for ( TInt index(0); index < uids.Count(); index++ )
    	{
        Swi::RSisRegistryEntry entry;
        CleanupClosePushL(entry);
        User::LeaveIfError( entry.Open( regSession, uids[index] ) );

	    //All packages are from installed packages        
	    CMsengSisxInfo* appObj = CMsengSisxInfo::NewL( entry, iDrive );
	    CleanupStack::PushL( appObj ); 
	    TInt64 baseSize( 0 );
	    baseSize = entry.SizeL();
	    
        TRACES
        (
        RDebug::Print(_L("ScanSisRegistryL, IsPresent, %d"), TInt( entry.IsPresentL()));    
        RDebug::Print(_L("ScanSisRegistryL, IsInRom, %d"), TInt( entry.IsInRomL()));    
        );
	    
	    // Only show if not in rom
       	if ( !entry.IsInRomL() && entry.IsPresentL() && appObj->RequestedLocation() )
       		{    
    	    // Get possible augmentations
    	    entry.AugmentationsL( augmentations );
    	    for ( TInt i( 0 ); i < augmentations.Count(); i++ )
    	    	{
    	        Swi::RSisRegistryEntry augmentation;
    	        CleanupClosePushL( augmentation );
    	        augmentation.OpenL( regSession, *augmentations[i] );
    	        // Only show if not in rom
    	        if ( !augmentation.IsInRomL() && augmentation.IsPresentL() )
    	        	{
    	            // Append the size to the total size
    	            baseSize += augmentation.SizeL();
    	            }
    	        CleanupStack::PopAndDestroy( &augmentation );
    	        }
    	    totalSize += baseSize;     
       		}
	        
	   	augmentations.ResetAndDestroy();    
        
        CleanupStack::PopAndDestroy( appObj );
        CleanupStack::PopAndDestroy( &entry );
        }
        
    InfoArray().AddSizeByGroupL(EGroupNativeApps, totalSize);

    CleanupStack::PopAndDestroy(&uids);
    CleanupStack::PopAndDestroy(&regSession);
    }

//  End of File  

