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
* Description:  Utility class for accessing sis-registry
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
#include "msengsisxinfo.h"
#include "memscanutils.h"

// constants
_LIT( KDriveC, "C");
_LIT( KDriveZ, "Z");

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CMsengSisxInfo::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CMsengSisxInfo::ConstructL( Swi::RSisRegistryEntry& aEntry, TDriveNumber aDrive )
    {
    TInt err;
    TChar driveLetter;
    RFs::DriveToChar(aDrive, driveLetter);
    driveLetter.UpperCase();
    const TInt KDriveLength = 1;
    TBuf<KDriveLength> driveName;
    driveName.Append(driveLetter);
    
    TRAP(err, aEntry.FilesL(iFiles));
    
    // Get the name of the app
    HBufC* packageName = aEntry.PackageNameL();
    CleanupStack::PushL( packageName );
    TParse parse;
    
    parse.SetNoWild( *packageName, NULL, NULL );    
    iFileName = parse.Name().AllocL();
    CleanupStack::PopAndDestroy(packageName);
    TPtr tmpPtr2 = iFileName->Des();
    TRACES( RDebug::Print( _L("CMsengSisxInfo::ConstructL - %S"), &tmpPtr2 ); );

    // Get Location
    TChar selectedDrive( aEntry.SelectedDriveL() );
    selectedDrive.UpperCase();
    iRequestedLocation = EFalse;
    
    if( selectedDrive == driveLetter )
        {
        // Phone memory
        iRequestedLocation = ETrue;
        }
    else 
		{
		// Check the disk from files
		// if all have C, the software is installed to phone
		// else it is installed to memory card
		TInt count = iFiles.Count();

        TInt allFilesInCZ = 0;
		for ( TInt index = 0; index < count; index++ )
		    {
		    TPtrC firstChar = iFiles[index]->Des().Left(1);
		    TRACES( RDebug::Print( _L("Checking location of %S"), iFiles[index]); );

		    if ( ( firstChar.CompareF( KDriveC ) == KErrNone ) || 
                 ( firstChar.CompareF( KDriveZ ) == KErrNone ) )
                {
		        allFilesInCZ++;
    		    }    		    
		    }

		if ( allFilesInCZ == count && (driveName.CompareF( KDriveC ) == KErrNone ) )
		    {
		    // Phone memory
		    iRequestedLocation = ETrue;
		    }
        }

    TRACES( RDebug::Print( _L("Requested location %d"), iRequestedLocation); );
    }


// -----------------------------------------------------------------------------
// CMsengSisxInfo::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CMsengSisxInfo* CMsengSisxInfo::NewL( Swi::RSisRegistryEntry& aEntry,
                                      TDriveNumber aDrive ) 
    {
    CMsengSisxInfo* self = new ( ELeave ) CMsengSisxInfo( );
    CleanupStack::PushL( self );
    self->ConstructL( aEntry, aDrive );
    CleanupStack::Pop( self );
    return self; 
    }
    

// -----------------------------------------------------------------------------
// CMsengSisxInfo::~CMsengSisxInfo
// Destructor.
// -----------------------------------------------------------------------------
CMsengSisxInfo::~CMsengSisxInfo()
    {   
    delete iFileName;   
    iFiles.ResetAndDestroy();
    iFiles.Close();
    }


// -----------------------------------------------------------------------------
// CAppMngrSisxInfo::RequestedLocation
// Get location of the application.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TBool CMsengSisxInfo::RequestedLocation() const
    {
    return iRequestedLocation;
    }


//  End of File  
