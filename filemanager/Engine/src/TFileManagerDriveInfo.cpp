/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Drive info storage
*
*/


// INCLUDE FILES
#include <coreapplicationuisdomainpskeys.h>
#include "TFileManagerDriveInfo.h"
#include "CFileManagerEngine.h"
#include "FileManagerDebug.h"
#include "CFileManagerCommonDefinitions.h"
#include "CFileManagerFeatureManager.h"
#ifdef RD_MULTIPLE_DRIVE
#include <driveinfo.h>
#endif // RD_MULTIPLE_DRIVE

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TFileManagerDriveInfo::TFileManagerDriveInfo
// -----------------------------------------------------------------------------
//
EXPORT_C TFileManagerDriveInfo::TFileManagerDriveInfo()
    {
    Reset();
    }

// -----------------------------------------------------------------------------
// TFileManagerDriveInfo::TFileManagerDriveInfo
// -----------------------------------------------------------------------------
//
EXPORT_C void TFileManagerDriveInfo::Reset()
    {
    iName.Zero();
    iCapacity = 0;
    iSpaceFree = 0;
    iUid = 0;
    iDrive = 0;
    iState = 0;
    }

// -----------------------------------------------------------------------------
// TFileManagerDriveInfo::GetInfoL
// -----------------------------------------------------------------------------
//
void TFileManagerDriveInfo::GetInfoL(
        const CFileManagerEngine& aEngine, const TInt aDrive )
    {
    // Default behavior is to check whether the remote drive is connected 
    GetInfoL( aEngine, aDrive, ETrue );
    }


// -----------------------------------------------------------------------------
// TFileManagerDriveInfo::GetInfoL
// -----------------------------------------------------------------------------
//
void TFileManagerDriveInfo::GetInfoL(
        const CFileManagerEngine& aEngine, const TInt aDrive,TBool aCheckRemoteDriveConnected )
    {
    FUNC_LOG

    INFO_LOG1( "TFileManagerDriveInfo::GetInfoL - aDrive %d", aDrive )

    Reset();
    iDrive = aDrive;

    RFs& fs( aEngine.Fs() );
    CheckMountL( fs, aDrive );
    TInt err( FillStateFromVolumeInfo( fs, aDrive ) );
    if ( err == KErrInUse )
        {
        iState |= EDriveInUse;
        }
    else if ( err == KErrCorrupt )
        {
        iState |= EDrivePresent | EDriveCorrupted;
        }
    else if ( err == KErrLocked )
        {
        iState |= EDrivePresent | EDriveLocked | EDriveFormatted |
            EDrivePasswordProtected | EDriveLockable;
        }
    else if ( err == KErrNone )
        {
        // If the drive is remote drive, and need to check whether remote drive is connected
        if ( iState & EDriveRemote && aCheckRemoteDriveConnected )
            {
            if ( aEngine.IsRemoteDriveConnected( aDrive ) )
                {
                iState |= EDriveConnected;
                }
            }

#ifndef RD_FILE_MANAGER_BACKUP
        if ( iState & EDriveRemovable )
            {
            if ( aEngine.BackupFileExistsL( aDrive ) )
                {
                iState |= EDriveBackupped;
                }
            }
#endif // RD_FILE_MANAGER_BACKUP

        }

    INFO_LOG3( "TFileManagerDriveInfo::GetInfoL - aDrive %d, err %d, iState 0x%x",
        aDrive, err, iState )

    if ( aEngine.FeatureManager().IsEmbedded() )
        {
        // Disable format in embedded mode, because it messes up 
        // operations since embedded apps are closed.
        iState &= ~EDriveFormattable;
        }
    }

// -----------------------------------------------------------------------------
// TFileManagerDriveInfo::CheckMountL()
// -----------------------------------------------------------------------------
// 
void TFileManagerDriveInfo::CheckMountL(
        RFs& aFs, const TInt aDrive ) const
    {
    FUNC_LOG

    HBufC* fullname = HBufC::NewLC( KMaxFullName );
    TPtr name( fullname->Des() );
    TInt err( aFs.FileSystemName( name, aDrive ) );
    if ( err == KErrNone && !name.Length() )
        {
        // Drive isn't mounted at present, so try it now....
        // Returns KErrLocked, if locked and the password
        // is not in store
        err = aFs.MountFileSystem( KFmgrFatFSName, aDrive );
        }
    CleanupStack::PopAndDestroy( fullname );

    LOG_IF_ERROR1( err, "TFileManagerDriveInfo::CheckMountL-err=%d", err )
    }

// -----------------------------------------------------------------------------
// TFileManagerDriveInfo::FillStateFromDriveInfo()
// -----------------------------------------------------------------------------
// 
TInt TFileManagerDriveInfo::FillStateFromDriveInfo(
#ifdef RD_MULTIPLE_DRIVE
        RFs& aFs,
#else // RD_MULTIPLE_DRIVE
        RFs& /*aFs*/,
#endif // RD_MULTIPLE_DRIVE
        const TInt aDrive,
        const TDriveInfo& aDrvInfo )
    {
    FUNC_LOG

    TInt ret( KErrNone );

#ifdef RD_MULTIPLE_DRIVE
    TUint drvStatus( 0 );
    TInt err( DriveInfo::GetDriveStatus( aFs, aDrive, drvStatus ) );
    if ( err != KErrNone )
        {
        ERROR_LOG2( "TFileManagerDriveInfo::FillStateFromDriveInfo - aDrive %d, Status err %d",
            aDrive, err )
        }
    INFO_LOG2( "TFileManagerDriveInfo::FillStateFromDriveInfo - Media type %d, drvStatus 0x%x",
        aDrvInfo.iType, drvStatus )
    if ( aDrvInfo.iType != EMediaNotPresent && ( drvStatus & DriveInfo::EDriveInUse ) )
        {
        ret = KErrInUse; // Drive is reserved for exclusive usage like file transfer
        }
    if ( ( drvStatus & DriveInfo::EDriveInternal ) &&
        ( drvStatus & DriveInfo::EDriveExternallyMountable ) )
        {
        // Handle mass storage bits here
        iState |= EDriveMassStorage | EDriveRemovable;
        if ( aDrvInfo.iMediaAtt & KMediaAttFormattable )
            {
            iState |= EDriveFormattable;
            }
        if ( aDrvInfo.iMediaAtt & KMediaAttLocked )
            {
            // Locking internal drives is not supported
            iState |= EDriveCorrupted;
            ret = KErrCorrupt;
            }
        return ret;
        }
    if ( drvStatus & DriveInfo::EDriveUsbMemory )
        {
        iState |= EDriveUsbMemory;
        }
#endif // RD_MULTIPLE_DRIVE

    // Setup flags from drive info
    if ( aDrvInfo.iMediaAtt & KMediaAttWriteProtected )
        {
        iState |= EDriveWriteProtected;
        }
    if ( aDrvInfo.iMediaAtt & KMediaAttLocked )
        {
        iState |= EDriveLocked;
        ret = KErrLocked;
        }
    if ( aDrvInfo.iMediaAtt & KMediaAttFormattable )
        {
        iState |= EDriveFormattable;
        }
    if ( aDrvInfo.iMediaAtt & KMediaAttLockable )
        {
        iState |= EDriveLockable;
        }
    if ( aDrvInfo.iMediaAtt & KMediaAttHasPassword )
        {
        iState |= EDrivePasswordProtected;
        }
    if ( ( aDrvInfo.iDriveAtt & KDriveAttRemovable ) ||
        aDrive == KFmgrMemoryCardDrive )
        {
        iState |= EDriveRemovable;
#ifdef RD_MULTIPLE_DRIVE
        if ( drvStatus & DriveInfo::EDriveSwEjectable )
            {
            iState |= EDriveEjectable;
            }
#else // RD_MULTIPLE_DRIVE
        // Eject is possible for external memory cards
        if ( !( aDrvInfo.iDriveAtt & KDriveAttInternal ) ||
            aDrive == KFmgrMemoryCardDrive )
            {
            iState |= EDriveEjectable;
            }
#endif // RD_MULTIPLE_DRIVE
        }
    if ( aDrvInfo.iDriveAtt & KDriveAttRemote )
        {
        iState |= EDriveRemote;
        }

    return ret;
    }

// -----------------------------------------------------------------------------
// TFileManagerDriveInfo::FillStateFromVolumeInfo()
// -----------------------------------------------------------------------------
// 
TInt TFileManagerDriveInfo::FillStateFromVolumeInfo(
        RFs& aFs, const TInt aDrive )
    {
    FUNC_LOG

    TVolumeInfo volInfo;
    TInt ret( aFs.Volume( volInfo, aDrive ) );
    LOG_IF_ERROR1( ret,
        "TFileManagerDriveInfo::FillStateFromVolumeInfo - Volume err %d", ret );
    if ( ret == KErrNone )
        {
        // Get all information from volume info
        TInt maxLen( Min( volInfo.iName.Length(), KMaxVolumeName ) );

        iName = volInfo.iName.Left( maxLen );
        iUid = volInfo.iUniqueID;
        iCapacity = volInfo.iSize;
        iSpaceFree = volInfo.iFree;

        iState |= EDrivePresent;

        if ( volInfo.iSize )
            {
            iState |= EDriveFormatted;
            }

        ret = FillStateFromDriveInfo( aFs, aDrive, volInfo.iDrive );
        }
    else
        {
        // Get all information from drive info
        TDriveInfo& drvInfo( volInfo.iDrive );
        TInt err( aFs.Drive( drvInfo, aDrive ) );
        LOG_IF_ERROR1( err,
            "TFileManagerDriveInfo::FillStateFromVolumeInfo - Drive err %d", err );
        if ( err == KErrNone )
            {
            err = FillStateFromDriveInfo( aFs, aDrive, drvInfo );

            // If memory card is not ready but type is present,
            // then check if it is reserved.
            if( err == KErrNone &&
               ret == KErrNotReady &&
               drvInfo.iType != EMediaNotPresent )
                {
                // Check USB file transfer state
                TInt prop( ECoreAppUIsUSBFileTransferUninitialized );
                RProperty::Get(
                    KPSUidCoreApplicationUIs,
                    KCoreAppUIsUSBFileTransfer, prop );
                if ( prop == ECoreAppUIsUSBFileTransferActive )
                    {
                    ret = KErrInUse; // Reserved for file transfer
                    }
                INFO_LOG2( "TFileManagerDriveInfo::FillStateFromVolumeInfo-prop=%d,ret=%d",
                    prop, ret )
                }
            }
        if ( err != KErrNone )
            {
            ret = err;
            }
        }

    LOG_IF_ERROR2( ret,
        "TFileManagerDriveInfo::FillStateFromVolumeInfo - aDrive %d, Ret %d",
        aDrive, ret )

    return ret;
    }

//  End of File  
