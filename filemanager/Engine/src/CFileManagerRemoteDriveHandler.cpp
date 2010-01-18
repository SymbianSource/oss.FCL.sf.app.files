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
* Description:  Wraps remote drive functionality
*
*/



// INCLUDES
#include <rsfwmountman.h>
#include "CFileManagerRemoteDriveHandler.h"
#include "CGflmNavigatorModel.h"
#include "CFileManagerEngine.h"
#include "CFileManagerPropertySubscriber.h"
#include "CFileManagerUtils.h"
#include "FileManagerDebug.h"
#include "CFileManagerFeatureManager.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CFileManagerRemoteDriveHandler::CFileManagerRemoteDriveHandler()
// ---------------------------------------------------------------------------
//
CFileManagerRemoteDriveHandler::CFileManagerRemoteDriveHandler(
        CFileManagerEngine& aEngine,
        CFileManagerUtils& aUtils ) :
    iEngine( aEngine ),
    iUtils( aUtils )
    {
    FUNC_LOG
    }

// ---------------------------------------------------------------------------
// CFileManagerRemoteDriveHandler::NewL( const CFileManagerEngine& aEngine )
// ---------------------------------------------------------------------------
//
CFileManagerRemoteDriveHandler* CFileManagerRemoteDriveHandler::NewL(
        CFileManagerEngine& aEngine,
        CFileManagerUtils& aUtils )
    {
    FUNC_LOG

    CFileManagerRemoteDriveHandler* self =
        new (ELeave) CFileManagerRemoteDriveHandler(
            aEngine, aUtils );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemoteDriveHandler::~CFileManagerRemoteDriveHandler()
// ---------------------------------------------------------------------------
//
CFileManagerRemoteDriveHandler::~CFileManagerRemoteDriveHandler()
    {
    FUNC_LOG

    delete iMountMan;
    delete iSubscriber;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemoteDriveHandler::ConstructL()
// ---------------------------------------------------------------------------
//
void CFileManagerRemoteDriveHandler::ConstructL()
    {
    FUNC_LOG

    // Check and set value to indicate if the feature is supported
    iRemoteStorageFwSupported =
        iEngine.FeatureManager().IsRemoteStorageFwSupported();
    if ( !iRemoteStorageFwSupported )
        {
        return;
        }
    iMountMan = CRsfwMountMan::NewL( 0, NULL );
#ifdef FILE_MANAGER_MOUNT_REMOTE_DRIVES_ON_STARTUP
    // Make sure that remote drives are mounted
    _LIT( KMounterExe, "rsfwbootmounter.exe" );
    RProcess mounter;
    if ( mounter.Create( KMounterExe, KNullDesC ) == KErrNone )
        {
    	mounter.Resume();
    	mounter.Close();
        }
#endif // FILE_MANAGER_MOUNT_REMOTE_DRIVES_ON_STARTUP
    iSubscriber = CFileManagerPropertySubscriber::NewL(
        *this, KRfeServerSecureUid, ERsfwPSKeyConnect );
    }

// ---------------------------------------------------------------------------
// CFileManagerRemoteDriveHandler::IsConnected()
// ---------------------------------------------------------------------------
//
TBool CFileManagerRemoteDriveHandler::IsConnected( const TInt aDrive )
    {
    FUNC_LOG

    if ( !iRemoteStorageFwSupported )
        {
        return EFalse;
        }
    TBool ret( EFalse );
    TChar drv( 0 );
    if ( RFs::DriveToChar( aDrive, drv ) == KErrNone )
        {
        TRsfwMountInfo info;
        if ( iMountMan->GetMountInfo( drv, info ) == KErrNone )
            {
            ret = ( info.iMountStatus.iConnectionState ==
                    KMountStronglyConnected );
            }
        }
    return ret;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemoteDriveHandler::SetConnection()
// ---------------------------------------------------------------------------
//
TInt CFileManagerRemoteDriveHandler::SetConnection(
        TInt aDrive, TBool aConnect )
    {
    FUNC_LOG

    if ( !iRemoteStorageFwSupported )
        {
        return KErrNone;
        }
    TChar drv( 0 );
    TInt err( RFs::DriveToChar( aDrive, drv ) );
    if ( err != KErrNone )
        {
        return err;
        }
    err = iMountMan->SetMountConnectionStateBlind(
        drv,
        aConnect ? KMountStronglyConnected : KMountNotConnected );
    if ( err == KErrCancel )
        {
        err = KErrPathNotFound;
        }
    return err;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemoteDriveHandler::DeleteSettings()
// ---------------------------------------------------------------------------
//
TInt CFileManagerRemoteDriveHandler::DeleteSettings(
        const TInt aDrive )
    {
    FUNC_LOG

    if ( !iRemoteStorageFwSupported )
        {
        return KErrNone;
        }
    TChar drv( 0 );
    TInt err( RFs::DriveToChar( aDrive, drv ) );
    if ( err != KErrNone )
        {
        return err;
        }
    TRAP( err, iMountMan->DeleteMountEntryL( drv ) );
    return err;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemoteDriveHandler::PropertyChangedL()
// ---------------------------------------------------------------------------
//
void CFileManagerRemoteDriveHandler::PropertyChangedL(
    const TUid& /*aCategory*/, const TUint /*aKey*/ )
    {
    FUNC_LOG

    iEngine.DriveAddedOrChangedL();
    }

// ---------------------------------------------------------------------------
// CFileManagerRemoteDriveHandler::RefreshDirectory()
// ---------------------------------------------------------------------------
//
TInt CFileManagerRemoteDriveHandler::RefreshDirectory(
        const TDesC& aFullPath )
    {
    FUNC_LOG

    if ( !iRemoteStorageFwSupported )
        {
        return KErrNone;
        }
    return iMountMan->RefreshDirectory( aFullPath );
    }

// ---------------------------------------------------------------------------
// CFileManagerRemoteDriveHandler::HasAppRemoteDriveSupport()
// ---------------------------------------------------------------------------
//
TBool CFileManagerRemoteDriveHandler::HasAppRemoteDriveSupport(
        TUid aUid )
    {
    FUNC_LOG

    if ( !iRemoteStorageFwSupported )
        {
        return EFalse;
        }
    if ( aUid == KNullUid )
        {
        return ETrue;
        }
    return !iMountMan->IsAppOnBlackList( aUid );
    }

// ---------------------------------------------------------------------------
// CFileManagerRemoteDriveHandler::CancelTransfer()
// ---------------------------------------------------------------------------
//
void CFileManagerRemoteDriveHandler::CancelTransfer(
        const TDesC& aFullPath )
    {
    FUNC_LOG

    if ( !iRemoteStorageFwSupported )
        {
        return;
        }
    INFO_LOG1(
        "CFileManagerRemoteDriveHandler::CancelTransfer=%S",
        &aFullPath )
    iMountMan->CancelRemoteTransfer( aFullPath );
    }

//  End of File  
