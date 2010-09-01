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
* Description:  Resolves different media types' drive letters
*
*/



// INCLUDE FILES
#include "CGflmDriveResolver.h"
#include "CGflmDriveItem.h"
#include "MGflmItemFilter.h"
#include "GflmUtils.h"
#include <f32file.h>
#ifdef RD_MULTIPLE_DRIVE
 #include <driveinfo.h>
#endif // RD_MULTIPLE_DRIVE


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmDriveResolver::CGflmDriveResolver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGflmDriveResolver::CGflmDriveResolver( RFs& aFs ) :
        iFs( aFs )
    {
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CGflmDriveResolver* CGflmDriveResolver::NewL( RFs& aFs )
    {
    CGflmDriveResolver* self = new ( ELeave ) CGflmDriveResolver( aFs );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::ConstructL
// -----------------------------------------------------------------------------
//
void CGflmDriveResolver::ConstructL()
    {
    User::LeaveIfError( iCs.CreateLocal() );
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::~CGflmDriveResolver
//
// -----------------------------------------------------------------------------
//
CGflmDriveResolver::~CGflmDriveResolver()
    {
    iDrives.ResetAndDestroy();
    iDrives.Close();
    iCs.Close();
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::RefreshDrives()
//
// -----------------------------------------------------------------------------
//
TInt CGflmDriveResolver::RefreshDrives( MGflmItemFilter* aFilter )
    {
    iCs.Wait();

    TRAPD( ret, RefreshDrivesL( aFilter ) );

    LOG_IF_ERROR1(
        ret, "CGflmDriveResolver::RefreshDrives-ret=%d", ret )

    iCs.Signal();

    return ret;
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::RefreshDrivesL()
//
// -----------------------------------------------------------------------------
//
void CGflmDriveResolver::RefreshDrivesL( MGflmItemFilter* aFilter )
    {
    TIMESTAMP( "GFLM refresh drives started: " )

    if ( !iRefreshed )
        {
        iDrives.ResetAndDestroy();
        TDriveList drives;
        User::LeaveIfError( iFs.DriveList( drives, KDriveAttAll ) );
        TInt count( drives.Length() );
        for ( TInt i( 0 ); i < count; i++ )
            {
            if ( drives[ i ] )
                {
                AppendDriveL( i, aFilter );
                }
            }
        iRefreshed = ETrue;
        }

    TIMESTAMP( "GFLM refresh drives ended: " )
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::DriveCount()
//
// -----------------------------------------------------------------------------
//
TInt CGflmDriveResolver::DriveCount() const
    {
    return iDrives.Count();
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::DriveAt()
//
// -----------------------------------------------------------------------------
//
CGflmDriveItem* CGflmDriveResolver::DriveAt( const TInt aIndex ) const
    {
    return iDrives[ aIndex ];
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::DriveFromPath()
//
// -----------------------------------------------------------------------------
//
CGflmDriveItem* CGflmDriveResolver::DriveFromPath( const TDesC& aPath ) const
    {
    TInt drv( 0 );
    if ( aPath.Length() &&
         RFs::CharToDrive( aPath[ 0 ], drv ) == KErrNone )
        {
        return DriveFromId( drv );
        }
    return NULL;
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::ClearDrives()
//
// -----------------------------------------------------------------------------
//
void CGflmDriveResolver::ClearDrives()
    {
    iRefreshed = EFalse;
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::IsRootPath()
//
// -----------------------------------------------------------------------------
//
TBool CGflmDriveResolver::IsRootPath( const TDesC& aPath ) const
    {
    TInt count( iDrives.Count() );
    TInt pathLen( aPath.Length() );

    for ( TInt i( 0 ); i < count; i++ )
        {
        CGflmDriveItem* drvItem = iDrives[ i ];
        TPtrC root( drvItem->RootDirectory() );

        if ( pathLen == root.Length() && !root.CompareF( aPath ) )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::AppendDriveL()
//
// -----------------------------------------------------------------------------
//
void CGflmDriveResolver::AppendDriveL(
        const TInt aDrive, MGflmItemFilter* aFilter )
    {
    INFO_LOG1( "CGflmDriveResolver::AppendDriveL()-aDrive=%d", aDrive )

    TVolumeInfo volInfo;
    TInt err( iFs.Volume( volInfo, aDrive ) );

    LOG_IF_ERROR1( err, "CGflmDriveResolver::AppendDriveL()-err=%d", err )

    if ( err != KErrNone )
        {
        TInt err2( iFs.Drive( volInfo.iDrive, aDrive ) );

        LOG_IF_ERROR1( err2, "CGflmDriveResolver::AppendDriveL()-err2=%d", err )

        if ( err2 != KErrNone )
            {
            return;
            }
        }

    TDriveInfo& drvInfo( volInfo.iDrive );
    if ( drvInfo.iMediaAtt & KMediaAttLocked )
        {
        err = KErrLocked;
        }
    if ( drvInfo.iType == EMediaNotPresent )
        {
        err = KErrNotReady;
        }

    TUint drvStatus( 0 );

#ifdef RD_MULTIPLE_DRIVE
    TInt err3( DriveInfo::GetDriveStatus( iFs, aDrive, drvStatus ) );
    LOG_IF_ERROR1( err3, "CGflmDriveResolver::AppendDriveL()-err3=%d", err3 )
    if ( err3 != KErrNone )
        {
        // Supress error
        }
#endif // RD_MULTIPLE_DRIVE

    CGflmDriveItem* drvItem = CGflmDriveItem::NewLC(
        aDrive, volInfo, err, drvStatus );

    if ( drvInfo.iDriveAtt & KDriveAttRemote )
        {
        // Get remote drive name
        if ( iFs.GetDriveName( aDrive, iReadBuffer ) == KErrNone )
            {
            drvItem->SetLocalizedNameL( iReadBuffer );
            }
        }

    TBool allowed( ETrue );
    if ( aFilter )
        {
        allowed = aFilter->FilterItemL( drvItem, 0, NULL );
        }
    if ( allowed )
        {
        iDrives.AppendL( drvItem );
        CleanupStack::Pop( drvItem );
        }
    else
        {
        CleanupStack::PopAndDestroy( drvItem );
        }
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::IsRemoteDrive()
//
// -----------------------------------------------------------------------------
//
TBool CGflmDriveResolver::IsRemoteDrive( const TDesC& aPath ) const
    {
    CGflmDriveItem* drv = DriveFromPath( aPath );
    if ( drv )
        {
        const TVolumeInfo& vol( drv->VolumeInfo() );
        if ( vol.iDrive.iDriveAtt & KDriveAttRemote )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CGflmDriveResolver::DriveFromId()
//
// -----------------------------------------------------------------------------
//
CGflmDriveItem* CGflmDriveResolver::DriveFromId( const TInt aDrive ) const
    {
    TInt count( iDrives.Count() );
    for ( TInt i( 0 ); i < count; i++ )
        {
        CGflmDriveItem* drvItem = iDrives[ i ];
        if ( aDrive == drvItem->Drive() )
            {
            return drvItem;
            }
        }
    return NULL;
    }

//  End of File
