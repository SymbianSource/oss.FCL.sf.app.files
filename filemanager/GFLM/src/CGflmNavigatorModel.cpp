/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Navigable file list model
*
*/



// INCLUDE FILES
#include <bautils.h>
#include "CGflmNavigatorModel.h"
#include "CGflmItemGroupImpl.h"
#include "CGflmItemLocalizer.h"
#include "CGflmDriveResolver.h"
#include "GFLM.hrh"
#include "GflmUtils.h"
#include "GFLMConsts.h"


// CONSTANTS

// Granularity of iBackstepStack array
const TInt KBackstepStackGranularity = 3;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::CGflmNavigatorModel
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGflmNavigatorModel::CGflmNavigatorModel( RFs& aFs ) :
        CGflmFileListModel( aFs )
    {
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CGflmNavigatorModel::ConstructL()
    {
    CGflmFileListModel::ConstructL();
    iBackstepStack = new( ELeave ) CDesCArraySeg( KBackstepStackGranularity );
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CGflmNavigatorModel* CGflmNavigatorModel::NewL( RFs& aFs )
    {
    CGflmNavigatorModel* self = new( ELeave ) CGflmNavigatorModel( aFs );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::~CGflmNavigatorModel()
// -----------------------------------------------------------------------------
//
CGflmNavigatorModel::~CGflmNavigatorModel()
    {
    delete iBackstepStack;
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::GoToDirectoryL()
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmNavigatorModel::GoToDirectoryL(
        const TDesC& aPath, TBool aBackstepping )
    {
    if ( !aPath.CompareF( CurrentDirectory() ) )
        {
        return; // Already in the directory
        }
    SetBaseDirectoryL( aPath );

    // Update the backstepping stack after calling SetBaseDirectoryL()
    // because it might leave
    if ( !aBackstepping )
        {
        iBackstepStack->Reset();
        }
    iBackstepStack->AppendL( aPath );

    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::BackstepL()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmNavigatorModel::BackstepL()
    {
    if ( iBackstepStack->MdcaCount() )
        {
        iBackstepStack->Delete( iBackstepStack->MdcaCount() - 1 );
        if ( iBackstepStack->MdcaCount() )
            {
            TPtrC dir( iBackstepStack->MdcaPoint(
                    iBackstepStack->MdcaCount() - 1 ) );
            SetSourceL( dir );
            if ( !IsValidSource( dir ) )
                {
                User::Leave( KErrPathNotFound );
                }
            }
        else
            {
            SetSourceL( KNullDesC );
            }
        }
    else
        {
        User::Leave( KErrUnderflow );
        }
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::GoToDirectoryL()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmNavigatorModel::GoToDirectoryL( const TDesC& aBasePath,
                                                   const TDesC& aTopPath )
    {
    // Use copies because descriptors may come from backstep stack itself
    HBufC* basePathBuffer = aBasePath.AllocLC();
    TPtrC basePath( basePathBuffer->Des() );
    HBufC* topPathBuffer = aTopPath.AllocLC();
    TPtrC topPath( topPathBuffer->Des() );
    const TInt KNthMatch = 2;

    // Clear the backstepping stack
    iBackstepStack->Reset();

    // Construct the backstepping stack

    while ( basePath.CompareF( topPath ) )
        {
        iBackstepStack->InsertL( 0, topPath );

        // Find the second backslash starting from the end
        TInt cutPoint( GflmUtils::LocateReverseNth(
            topPath, KGFLMBackslash()[ 0 ], KNthMatch ) );
        User::LeaveIfError( cutPoint );
        topPath.Set( topPath.Left( cutPoint + 1 ) );
        }

    // Add basepath too
    iBackstepStack->InsertL( 0, topPath );

    SetBaseDirectoryL( *topPathBuffer );

    CleanupStack::PopAndDestroy( topPathBuffer );
    CleanupStack::PopAndDestroy( basePathBuffer );
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::CurrentDirectory()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CGflmNavigatorModel::CurrentDirectory() const
    {
    TInt stackSize( iBackstepStack->MdcaCount() );
    if ( stackSize <= 0 )
        {
        return KNullDesC();
        }
    return iBackstepStack->MdcaPoint( stackSize - 1 );
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::LocalizedNameOfCurrentDirectory()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CGflmNavigatorModel::LocalizedNameOfCurrentDirectory() const
    {
    TInt stackSize( iBackstepStack->MdcaCount() );
    if ( stackSize <= 0 )
        {
        return TPtrC( KNullDesC );
        }
    TPtrC directory( iBackstepStack->MdcaPoint( stackSize - 1 ) );
    TPtrC localizedName( iItemLocalizer->Localize( directory ) );
    if ( localizedName.Length() )
        {
        // Return the localized name
        return localizedName;
        }
    // Directory has no localized name => return the name of the directory
    TPtrC ptr( GflmUtils::StripFinalBackslash( directory ) );
    TInt lastBs( ptr.LocateReverse( KGFLMBackslash()[ 0 ] ) );
    if ( lastBs != KErrNotFound )
        {
        return ptr.Mid( lastBs + 1 );
        }
    return TPtrC( KNullDesC );
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::SetBaseDirectoryL()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CGflmNavigatorModel::SetBaseDirectoryL( const TDesC& aDirectory )
    {
    if ( IsValidSource( aDirectory ) )
        {
        SetSourceL( aDirectory );
        }
    else
        {
        User::Leave( KErrPathNotFound );
        }
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::NavigationLevel()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CGflmNavigatorModel::NavigationLevel() const
    {
    return iBackstepStack->Count() - 1;
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::CurrentDrive()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C CGflmDriveItem* CGflmNavigatorModel::CurrentDrive() const
    {
    return DriveFromPath( CurrentDirectory() );
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::SetSourceL()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CGflmNavigatorModel::SetSourceL( const TDesC& aSource )
    {
    TBool empty( !aSource.Length() );
    TInt groupCount( iGroups->Count() );
    for ( TInt i( 0 ); i < groupCount; i++ )
        {
        MGflmItemGroup* group = iGroups->At( i );
        group->ResetSources();
        if ( !empty )
            {
            group->AddSourceL( aSource );
            }
        }
    }

// -----------------------------------------------------------------------------
// CGflmNavigatorModel::IsValidSource()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TBool CGflmNavigatorModel::IsValidSource( const TDesC& aSource ) const
    {
    if ( aSource.Length() &&
        !iDriveResolver->IsRemoteDrive( aSource ) &&
        !BaflUtils::PathExists( iFs, aSource ) &&
        !iDriveResolver->IsRootPath( aSource ) )
        {
        return EFalse;
        }
    return ETrue;
    }

//  End of File
