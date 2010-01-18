/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Drive item definitions
*
*/



// INCLUDE FILES
#include "CGflmDriveItem.h"
#include "GflmUtils.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmDriveItem::CGflmDriveItem
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGflmDriveItem::CGflmDriveItem(
        const TInt aDrive,
        const TVolumeInfo& aVolume,
        const TInt aVolumeStatus,
        const TUint aDriveStatus )
        : iVolume( aVolume ),
          iDrive( aDrive ),
          iVolumeStatus( aVolumeStatus ),
          iDriveStatus( aDriveStatus )
    {
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CGflmDriveItem* CGflmDriveItem::NewLC(
        const TInt aDrive,
        const TVolumeInfo& aVolume,
        const TInt aVolumeStatus,
        const TUint aDriveStatus )
    {
    CGflmDriveItem* self = new( ELeave ) CGflmDriveItem(
        aDrive,
        aVolume,
        aVolumeStatus,
        aDriveStatus );

    CleanupStack::PushL( self );
    self->ConstructL();

    return self;
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::ConstructL
//
// -----------------------------------------------------------------------------
//
void CGflmDriveItem::ConstructL()
    {
    SetRootDirectoryL( TDriveUnit( iDrive ).Name() );
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::~CGflmDriveItem
//
// -----------------------------------------------------------------------------
//
CGflmDriveItem::~CGflmDriveItem()
    {
    delete iLocalizedName;
    delete iRootDir;
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::Type
//
// -----------------------------------------------------------------------------
//
CGflmGroupItem::TGflmItemType CGflmDriveItem::Type() const
    {
    return EDrive;
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::Name
//
// -----------------------------------------------------------------------------
//
TPtrC CGflmDriveItem::Name() const
    {
    if ( iLocalizedName )
        {
        return iLocalizedName->Des();
        }
    return TPtrC( iVolume.iName );
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::Date
//
// -----------------------------------------------------------------------------
//
TTime CGflmDriveItem::Date() const
    {
    return TTime( 0 );
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::Size
//
// -----------------------------------------------------------------------------
//
TInt64 CGflmDriveItem::Size() const
    {
    return 0;
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::Ext
//
// -----------------------------------------------------------------------------
//
TPtrC CGflmDriveItem::Ext() const
    {
    return TPtrC( KNullDesC );
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::SetLocalizedNameL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmDriveItem::SetLocalizedNameL( const TDesC& aName )
    {
    HBufC* temp = aName.AllocL();
    delete iLocalizedName;
    iLocalizedName = temp;
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::SetRootDirectoryL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmDriveItem::SetRootDirectoryL( const TDesC& aPath )
    {
    const TInt KFmgrBackslashSpace = 1;
    TInt len( aPath.Length() );
    HBufC* temp = HBufC::NewL( len + KFmgrBackslashSpace );
    TPtr ptr( temp->Des() );
    ptr.Copy( aPath );
    GflmUtils::EnsureFinalBackslash( ptr );
    delete iRootDir;
    iRootDir = temp;
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::RootDirectory
//
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CGflmDriveItem::RootDirectory() const
    {
    return iRootDir->Des();
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::GetVolumeInfo
//
// -----------------------------------------------------------------------------
//
EXPORT_C const TVolumeInfo& CGflmDriveItem::VolumeInfo() const
    {
    return iVolume;
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::Drive
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CGflmDriveItem::Drive() const
    {
    return iDrive;
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::VolumeStatus
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CGflmDriveItem::VolumeStatus() const
    {
    return iVolumeStatus;
    }

// -----------------------------------------------------------------------------
// CGflmDriveItem::DriveStatus
//
// -----------------------------------------------------------------------------
//
EXPORT_C TUint CGflmDriveItem::DriveStatus() const
    {
    return iDriveStatus;
    }

//  End of File
