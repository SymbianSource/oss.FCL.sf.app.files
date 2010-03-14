/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Wraps removable drive functionality
*
*/



// INCLUDE FILES
#include <FileManagerEngine.rsg>
#include "CFileManagerRemovableDriveHandler.h"
#include "MFileManagerProcessObserver.h"
#include "CFileManagerUtils.h"
#include "CFileManagerEngine.h"
#include <coreapplicationuisdomainpskeys.h>
#include <coemain.h>
#include <apgwgnam.h>
#include <apgtask.h>
#include <bautils.h>
#include <tz.h>
#include <babackup.h>
#include <pathinfo.h>
#include <sysutil.h>
#ifdef RD_MULTIPLE_DRIVE
#include <driveinfo.h>
#endif // RD_MULTIPLE_DRIVE
#include <e32property.h>
#include <centralrepository.h>
#include "CMMCScBkupEngine.h"
#include "MMCScBkupOperations.h"
#include "CMMCScBkupOperationParameters.h"
#include "CFileManagerItemProperties.h"
#include "CFileManagerBackupSettings.h"
#include "FileManagerPrivateCRKeys.h"
#include "BkupEngine.hrh"
#include "FileManagerDebug.h"
#include "CFileManagerCommonDefinitions.h"
#include "FileManagerUID.h"
#include "filemanagerprivatepskeys.h"


// CONSTANTS
const TInt KEjectScanInterval = 1000000; // 1 second
const TInt KEjectScanRoundsMax = 7;
NONSHARABLE_CLASS(TMaskLookup)
    {
    public:
        TUint32 iBkupMask;
        TUint32 iFmgrMask;
    };

const TMaskLookup KMaskLookup[] = {
    { EBUCatSettings,  EFileManagerBackupContentSettings  },
    { EBUCatMessages,  EFileManagerBackupContentMessages  },
    { EBUCatContacts,  EFileManagerBackupContentContacts  },
    { EBUCatCalendar,  EFileManagerBackupContentCalendar  },
    { EBUCatBookmarks, EFileManagerBackupContentBookmarks },
    { EBUCatUserFiles, EFileManagerBackupContentUserFiles }
};
const TUint32 KMaskLookupLen =
    sizeof( KMaskLookup ) / sizeof( KMaskLookup[ 0 ] );

const TInt KForcedFormatTimeout = 1000000;
const TInt KAppCloseTimeout = 1000000;
const TInt KFileManagerAppUid = 0x101F84EB;

NONSHARABLE_CLASS(TFileManagerVolumeNameStore)
    {
public:
    inline TFileManagerVolumeNameStore() : iDrive( KErrNotFound ), iName( KNullDesC ) {};

    TInt iDrive; // The drive using the store
    TFileName iName; // Use the same length than TVolumeInfo
    };

typedef TPckg< TFileManagerVolumeNameStore > TFileManagerVolumeNameStorePckg;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::CFileManagerRemovableDriveHandler
// ---------------------------------------------------------------------------
//
CFileManagerRemovableDriveHandler::CFileManagerRemovableDriveHandler(
        RFs& aFs,
        CFileManagerUtils& aUtils,
        CFileManagerEngine& aEngine ) :
    CActive( CActive::EPriorityStandard ),
    iFs( aFs ),
    iUtils( aUtils ),
    iEngine( aEngine )
    {
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::ConstructL
// ---------------------------------------------------------------------------
//
void CFileManagerRemovableDriveHandler::ConstructL()
    {
    CActiveScheduler::Add( this );
    iBkupEngine = CMMCScBkupEngine::NewL( iFs );
    PublishBurStatus( EFileManagerBkupStatusUnset );
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::NewL
// ---------------------------------------------------------------------------
//
CFileManagerRemovableDriveHandler* CFileManagerRemovableDriveHandler::NewL(
        RFs& aFs,
        CFileManagerUtils& aUtils,
        CFileManagerEngine& aEngine )
    {
    CFileManagerRemovableDriveHandler* self =
        new( ELeave ) CFileManagerRemovableDriveHandler(
            aFs, aUtils, aEngine );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::~CFileManagerRemovableDriveHandler
// ---------------------------------------------------------------------------
// 
CFileManagerRemovableDriveHandler::~CFileManagerRemovableDriveHandler()
    {
    Cancel();
    delete iBSWrapper;
    delete iEjectScanPeriodic;
    delete iBkupEngine;
    iFormatter.Close();
    PublishBurStatus( EFileManagerBkupStatusUnset );
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::EjectScanAndShutdownApps
// ---------------------------------------------------------------------------
//
TInt CFileManagerRemovableDriveHandler::EjectScanAndShutdownApps( TAny* ptr )
    {
    CFileManagerRemovableDriveHandler* self = 
        static_cast< CFileManagerRemovableDriveHandler* >( ptr );
    TRAPD( err, self->DoEjectScanAndShutdownL() );
    if ( err != KErrNone )
        {
        self->EjectComplete( err );
        }
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::DoEjectScanAndShutdownL
// ---------------------------------------------------------------------------
//
void CFileManagerRemovableDriveHandler::DoEjectScanAndShutdownL()
    {
    RWsSession wsSession;
    User::LeaveIfError( wsSession.Connect() );
    CleanupClosePushL( wsSession );

    const TInt KDefaultWgIdArraySize = 4;
    CArrayFixFlat<TInt>* wgIds = new( ELeave ) CArrayFixFlat< TInt >(
        KDefaultWgIdArraySize );
    CleanupStack::PushL( wgIds );
    User::LeaveIfError( wsSession.WindowGroupList( 0, wgIds ) );
    TInt last( wgIds->Count() - 1 );
    TInt appsToShutDown( 0 );

    for ( TInt i( last ); i >= 0; i-- )
        {
        CApaWindowGroupName* doomedApp =
            CApaWindowGroupName::NewLC( wsSession, wgIds->At( i ) );
        TBool systemApp( doomedApp->IsSystem() );
        TBool hiddenApp( doomedApp->Hidden() );

        if ( !systemApp && !hiddenApp && doomedApp->AppUid().iUid != KFileManagerAppUid  )
            {
            appsToShutDown++;
            TApaTask* task = new (ELeave) TApaTask( wsSession );
            CleanupDeletePushL( task );
            task->SetWgId( wgIds->At( i ) );

            if ( !iEjectScanRounds )
                {
                 // applications are kindly requested to close themselves
                 // on the first round
                task->EndTask();
                }
            else if ( iEjectScanRounds >= KEjectScanRoundsMax )
                {
                task->KillTask();
                }
            CleanupStack::PopAndDestroy( task );
            }
        CleanupStack::PopAndDestroy( doomedApp );
        }
    CleanupStack::PopAndDestroy( wgIds );

    if ( !appsToShutDown || iEjectScanRounds >= KEjectScanRoundsMax )
        {
        EjectComplete( KErrNone );
        }
    iEjectScanRounds++;
    
    CleanupStack::PopAndDestroy( &wsSession );
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::EjectComplete
// ---------------------------------------------------------------------------
//
void CFileManagerRemovableDriveHandler::EjectComplete( TInt aErr )
    {
    delete iEjectScanPeriodic;
    iEjectScanPeriodic = NULL;
    iLastError = aErr;

    TRAP_IGNORE( InformFinishL() );

    RProperty::Set(
        KPSUidCoreApplicationUIs,
        KCoreAppUIsMmcRemovedWithoutEject,
        ECoreAppUIsEjectCommandNotUsed );
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::InformStartL
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::InformStartL( TInt aTotalCount )
    {
    iLastError = KErrNone;
    if ( iObserver )
        {
        iObserver->ProcessStartedL( iProcess,  aTotalCount );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::InformUpdateL
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::InformUpdateL( TInt aCount )
    {
    if ( iObserver )
        {
        iObserver->ProcessAdvanceL( aCount );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::InformFinishL
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::InformFinishL()
    {
    iEngine.ClearDriveInfo();
    PublishBurStatus( EFileManagerBkupStatusUnset );
    iProcess = MFileManagerProcessObserver::ENoProcess;
    if ( iObserver )
        {
        iObserver->ProcessFinishedL( iLastError, KNullDesC );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::InformError
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::InformError( TInt aErr )
    {
    iLastError = aErr;
    if ( iObserver )
        {
        iObserver->Error( aErr );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::HandleBkupEngineEventL
// ---------------------------------------------------------------------------
// 
TInt CFileManagerRemovableDriveHandler::HandleBkupEngineEventL(
        MMMCScBkupEngineObserver::TEvent aEvent, TInt aAssociatedData )
    {
    TInt ret( KErrNone );
    switch( aEvent )
        {
        case MMMCScBkupEngineObserver::ECommonOperationStarting:
            {
            iFinalValue = KMaxTInt;
            InformStartL( KMaxTInt );
            break;
            }
        case MMMCScBkupEngineObserver::ECommonSizeOfTaskUnderstood:
            {
            iFinalValue = aAssociatedData;
            InformStartL( aAssociatedData );
            break;
            }
        case MMMCScBkupEngineObserver::ECommonOperationPrepareEnded:
            {
            PublishBurStatus( EFileManagerBkupStatusUnset );
            // In order to show finished dialog prior SysAp's note,
            // inform observer already at prepare-ended state.
            if( iProcess == MFileManagerProcessObserver::ERestoreProcess )
                {
                if ( iFinalValue )
                    {
                    InformUpdateL( iFinalValue );
                    }
                InformFinishL();
                }
            break;
            }
        case MMMCScBkupEngineObserver::ECommonOperationEnded:
            {
            PublishBurStatus( EFileManagerBkupStatusUnset );
            if( iProcess != MFileManagerProcessObserver::ERestoreProcess )
                {
                if ( iFinalValue )
                    {
                    InformUpdateL( iFinalValue );
                    }
                InformFinishL();
                }
            break;
            }
        case MMMCScBkupEngineObserver::ECommonOperationError:
            {
            iLastError = aAssociatedData;
            break;
            }
        case MMMCScBkupEngineObserver::ECommonProgress:
            {
            InformUpdateL( aAssociatedData );
            break;
            }
        case MMMCScBkupEngineObserver::EBackupAnalysingData:
            {
            if ( iObserver &&
                iProcess == MFileManagerProcessObserver::EBackupProcess )
                {
                ret = iObserver->NotifyL(
                    MFileManagerProcessObserver::ENotifyBackupMemoryLow,
                    aAssociatedData );
                }
            break;
            }
        default:
            {
            break;
            }
        }
    return ret;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::SetObserver
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::SetObserver(
        MFileManagerProcessObserver* aObserver )
    {
    iObserver = aObserver;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::BackupFileNameLC
// ---------------------------------------------------------------------------
// 
HBufC* CFileManagerRemovableDriveHandler::BackupFileNameLC(
        TBool aFullPath ) const
    {
    HBufC* file = CCoeEnv::Static()->AllocReadResourceLC(
        R_TEXT_MMC_BACKUP_FILE );
    if ( !aFullPath )
        {
        return file; // Get just name
        }
    // Get full path
#ifdef RD_MULTIPLE_DRIVE
    TInt drive( 0 );
    User::LeaveIfError( DriveInfo::GetDefaultDrive(
        DriveInfo::EDefaultRemovableMassStorage, drive ) );
    TDriveUnit driveUnit( drive );
#else // RD_MULTIPLE_DRIVE
    TDriveUnit driveUnit( PathInfo::MemoryCardRootPath() );
#endif // RD_MULTIPLE_DRIVE
    HBufC* fileAndPath = HBufC::NewLC( KMaxFileName );
    TPtr ptr( fileAndPath->Des() );
    TPtr filePtr( file->Des() );
    ptr.Copy( driveUnit.Name() );
    ptr.Append( filePtr );
    CleanupStack::Pop( fileAndPath );
    CleanupStack::PopAndDestroy( file );
    CleanupStack::PushL( fileAndPath );
    return fileAndPath;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::StartFormatL()
// ---------------------------------------------------------------------------
//  
void CFileManagerRemovableDriveHandler::StartFormatL(
        const TInt aDrive )
    {
    TInt err( KErrNone );

    iDrive = aDrive;
    iProcess = MFileManagerProcessObserver::EFormatProcess;
    iFinalValue = 0;

    TRAP( err, InformStartL( 0 ) );
    if ( err == KErrNone )
        {
        // Close apps and then start format
        TRAP( err, CloseAppsL() );
        }
    if ( err != KErrNone )
        {
        EndFormatProcessL( err );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::StartEjectL()
// ---------------------------------------------------------------------------
//  
void CFileManagerRemovableDriveHandler::StartEjectL()
    {
    TRAPD( err, StartEjectScanL() );
    if ( err != KErrNone )
        {
        iProcess = MFileManagerProcessObserver::ENoProcess;
        User::Leave( err );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::StartEjectScanL()
// ---------------------------------------------------------------------------
//  
void CFileManagerRemovableDriveHandler::StartEjectScanL()
    {
    iProcess = MFileManagerProcessObserver::EEjectProcess;
    InformStartL( 0 );

    RProperty::Set(
        KPSUidCoreApplicationUIs,
        KCoreAppUIsMmcRemovedWithoutEject,
        ECoreAppUIsEjectCommandUsed );

    iEjectScanRounds = 0;
    iEjectScanPeriodic = CPeriodic::NewL( CActive::EPriorityLow );
    iEjectScanPeriodic->Start(
        KEjectScanInterval,
        KEjectScanInterval,
        TCallBack( EjectScanAndShutdownApps, this ) );
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::CancelProcess()
// ---------------------------------------------------------------------------
//    
void CFileManagerRemovableDriveHandler::CancelProcess()
    {
    switch( iProcess )
        {
        case MFileManagerProcessObserver::EFormatProcess:
            {
            Cancel();
            break;
            }
        case MFileManagerProcessObserver::EBackupProcess: // FALLTHROUGH
        case MFileManagerProcessObserver::ERestoreProcess:
        case MFileManagerProcessObserver::ESchBackupProcess:
            {
            PublishBurStatus( EFileManagerBkupStatusUnset );
            iBkupEngine->CancelOperation();
            break;
            }
        default:
            {
            break;
            }
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::BackupFileExistsL()
// ---------------------------------------------------------------------------
//   
TBool CFileManagerRemovableDriveHandler::BackupFileExistsL(
        const TInt /*aDrive*/ )
    {
    HBufC* backupFile = BackupFileNameLC( ETrue );
    TBool ret( iBkupEngine->ValidArchiveForRestore( *backupFile ) );
    CleanupStack::PopAndDestroy( backupFile );
    return ret;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::StartBackupL()
// ---------------------------------------------------------------------------
//  
void CFileManagerRemovableDriveHandler::StartBackupL(
        MFileManagerProcessObserver::TFileManagerProcess aProcess )
    {
    if ( aProcess != MFileManagerProcessObserver::EBackupProcess &&
        aProcess != MFileManagerProcessObserver::ESchBackupProcess )
        {
        User::Leave( KErrNotSupported );
        }

    CCoeEnv* coeEnv = CCoeEnv::Static();

    // Create backup params - ownership is transferred to
    // secure backup engine
    TResourceReader driveReader;
    coeEnv->CreateResourceReaderLC(
        driveReader,
        R_FILEMANAGER_BACKUP_RESTORE_DRIVES_AND_OPERATIONS );
    TResourceReader categoryReader;
    coeEnv->CreateResourceReaderLC(
        categoryReader,
        R_FILEMANAGER_BACKUP_CATEGORIES );

#ifdef RD_FILE_MANAGER_BACKUP

    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
    TUint32 bkupContent( FmgrToBkupMask( settings.Content() ) );
    TInt drive( settings.TargetDrive() );

    CMMCScBkupOpParamsBackupFull* params =
        CMMCScBkupOpParamsBackupFull::NewL(
            driveReader,
            categoryReader,
            TDriveNumber( drive ),
            bkupContent );

#else // RD_FILE_MANAGER_BACKUP

    HBufC* backupFile = BackupFileNameLC( ETrue );
    TInt drive = TDriveUnit( *backupFile );
    CleanupStack::PopAndDestroy( backupFile );

    CMMCScBkupOpParamsBackupFull* params =
        CMMCScBkupOpParamsBackupFull::NewL( driveReader,
            categoryReader,
            TDriveNumber( drive ),
            EBUCatAllInOne );

#endif // RD_FILE_MANAGER_BACKUP

    CleanupStack::PopAndDestroy(); // categoryReader
    CleanupStack::PopAndDestroy(); // driveReader

    CleanupStack::PushL( params );
    TBool diskFull( SysUtil::DiskSpaceBelowCriticalLevelL( &iFs, 0, drive ) );
    CleanupStack::Pop( params );

    if ( diskFull )
        {
        iProcess = aProcess;
        TRAP_IGNORE( InformStartL( KMaxTInt ) );
        iLastError = KErrDiskFull;
        TRAP_IGNORE( InformFinishL() );
        iProcess = MFileManagerProcessObserver::ENoProcess;
        delete params;
        }
    else
        {
        // Start the process - engine owns the parameters immediately
        iProcess = aProcess;
        
        PublishBurStatus( EFileManagerBkupStatusBackup );
        TRAPD( err, iBkupEngine->StartOperationL(
            EMMCScBkupOperationTypeFullBackup, *this, params ) );
        if ( err != KErrNone )
            {
            PublishBurStatus( EFileManagerBkupStatusUnset );
            iProcess = MFileManagerProcessObserver::ENoProcess;
            User::Leave( err );
            }
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::StartRestoreL()
// ---------------------------------------------------------------------------
//  
void CFileManagerRemovableDriveHandler::StartRestoreL()
    {
    TBool diskFull( SysUtil::DiskSpaceBelowCriticalLevelL(
        &iFs, 0, KFmgrSystemDrive ) );
    if ( diskFull )
        {
        iProcess = MFileManagerProcessObserver::ERestoreProcess;
        TRAP_IGNORE( InformStartL( KMaxTInt ) );
        iLastError = KErrDiskFull;
        TRAP_IGNORE( InformFinishL() );
        iProcess = MFileManagerProcessObserver::ENoProcess;
        return;
        }

    CFileManagerBackupSettings& bkupSettings( iEngine.BackupSettingsL() );

    // Create restore params - ownership is transferred to
    // secure backup engine
    TResourceReader driveReader;
    CCoeEnv::Static()->CreateResourceReaderLC(
        driveReader,
        R_FILEMANAGER_BACKUP_RESTORE_DRIVES_AND_OPERATIONS );

#ifdef RD_FILE_MANAGER_BACKUP

    CMMCScBkupOpParamsRestoreFull* params =
        CMMCScBkupOpParamsRestoreFull::NewL(
            driveReader,
            EBUCatAllSeparately );
    CleanupStack::PopAndDestroy(); // driveReader
    CleanupStack::PushL( params );

    // Get list of all archives
    RPointerArray< CMMCScBkupArchiveInfo > archives;
    TCleanupItem cleanupItem( ResetAndDestroyArchives, &archives );
    CleanupStack::PushL( cleanupItem );
    iBkupEngine->ListArchivesL(
        archives,
        params,
        bkupSettings.AllowedDriveAttMatchMask() );

    // Get user set restore selection
    RArray< CFileManagerRestoreSettings::TInfo > selection;
    CleanupClosePushL( selection );
    CFileManagerRestoreSettings& rstSettings( iEngine.RestoreSettingsL() );
    rstSettings.GetSelectionL( selection );

    // Remove non user selected archives
    TInt i( 0 );
    while ( i < archives.Count() )
        {
        TBool remove( ETrue );

        // Compare archives category and drive
        CMMCScBkupArchiveInfo* archiveInfo = archives[ i ];
        TUint32 fmgrContent(
            BkupToFmgrMask( archiveInfo->Category().iFlags ) );
        TInt drive( archiveInfo->Drive() );

        TInt count( selection.Count() );
        for( TInt j( 0 ); j < count; ++j )
            {
            const CFileManagerRestoreSettings::TInfo& info( selection[ j ] );
            if ( ( drive == info.iDrive ) && ( fmgrContent & info.iContent ) )
                {
                // Found user selected archive
                // Do not check this archive again
                selection.Remove( j );
                remove = EFalse;
                break;
                }
            }
        if ( remove )
            {
            // Remove non selected archive
            archives.Remove( i );
            delete archiveInfo;
            }
        else
            {
            // Move to next archive
            ++i;
            }
        }

    CleanupStack::PopAndDestroy( &selection );
    params->SetArchiveInfosL( archives );
    CleanupStack::Pop( &archives );
    archives.Close();
    CleanupStack::Pop( params );

#else // RD_FILE_MANAGER_BACKUP

    CMMCScBkupOpParamsRestoreFull* params =
        CMMCScBkupOpParamsRestoreFull::NewL( driveReader, EBUCatAllInOne );
    CleanupStack::PopAndDestroy(); // driveReader

    // Get list of all archives
    RPointerArray< CMMCScBkupArchiveInfo > archives;
    TCleanupItem cleanupItem( ResetAndDestroyArchives, &archives );
    CleanupStack::PushL( cleanupItem );
    iBkupEngine->ListArchivesL(
        archives,
        params,
        bkupSettings.AllowedDriveAttMatchMask() );
    params->SetArchiveInfosL( archives );
    CleanupStack::Pop( &archives );

#endif // RD_FILE_MANAGER_BACKUP

    // Start the process - engine owns the parameters immediately
    iProcess = MFileManagerProcessObserver::ERestoreProcess;
    PublishBurStatus( EFileManagerBkupStatusRestore );
    TRAPD( err, iBkupEngine->StartOperationL(
        EMMCScBkupOperationTypeFullRestore, *this, params ) );
    if ( err != KErrNone )
        {
        PublishBurStatus( EFileManagerBkupStatusUnset );
        iProcess = MFileManagerProcessObserver::ENoProcess;
        User::Leave( err );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::GetRestoreInfoArrayL
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::GetRestoreInfoArrayL(
        RArray< CFileManagerRestoreSettings::TInfo >& aArray,
        const TInt aDrive )
    {
    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );

    aArray.Reset();

    TResourceReader driveReader;
    CCoeEnv::Static()->CreateResourceReaderLC(
        driveReader,
        R_FILEMANAGER_BACKUP_RESTORE_DRIVES_AND_OPERATIONS );

    CMMCScBkupOpParamsRestoreFull* params =
        CMMCScBkupOpParamsRestoreFull::NewL(
            driveReader, EBUCatAllSeparately );
    CleanupStack::PopAndDestroy(); // driveReader
    CleanupStack::PushL( params );

    // Get list of all archives
    RPointerArray< CMMCScBkupArchiveInfo > archives;
    TCleanupItem cleanupItem( ResetAndDestroyArchives, &archives );
    CleanupStack::PushL( cleanupItem );
    iBkupEngine->ListArchivesL(
        archives,
        params,
        settings.AllowedDriveAttMatchMask(),
        aDrive );

    // Prepare time zone conversion
    RTz tz;
    User::LeaveIfError( tz.Connect() );
    CleanupClosePushL( tz );

    // Fill restore info
    CFileManagerRestoreSettings::TInfo info;
    TInt count( archives.Count() );
    aArray.ReserveL( count );

    for( TInt i( 0 ); i < count; ++i )
        {
        // Content
        CMMCScBkupArchiveInfo& archiveInfo( *archives[ i ] );
        info.iContent = BkupToFmgrMask( archiveInfo.Category().iFlags );

        // Local time
        info.iTime = archiveInfo.DateTime();
        User::LeaveIfError( tz.ConvertToLocalTime( info.iTime ) );

        // Drive
        info.iDrive = archiveInfo.Drive();

        aArray.AppendL( info );
        }

    CleanupStack::PopAndDestroy( &tz );
    CleanupStack::PopAndDestroy( &archives );
    CleanupStack::PopAndDestroy( params );
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::DoCancel
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::DoCancel()
    {
    switch( iProcess )
        {
        case MFileManagerProcessObserver::EFormatProcess:
            {
            TRAP_IGNORE( EndFormatProcessL( KErrCancel ) );
            break;
            }
        default:
            {
            break;
            }
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::RunL
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::RunL()
    {
    TInt err( iStatus.Int() );
    switch( iProcess )
        {
        case MFileManagerProcessObserver::EFormatProcess:
            {
            if( err != KErrNone )
                {
                if ( !iFinalValue && err == KErrInUse )
                    {
                    // Some app remained open, try still to start format
                    StartFormatProcessL();
                    }
                else
                    {
                    // Format failed
                    EndFormatProcessL( err );
                    }
                }
            else if( !iFinalValue )
                {
                // Apps have been closed. Start format.
                StartFormatProcessL();
                }
            else if( iFormatCountBuf() > 0 )
                {
                // Update observer and format next track
                InformUpdateL( iFinalValue - iFormatCountBuf() );
                iFormatter.Next( iFormatCountBuf, iStatus );
                SetActive( );
                }
            else
                {
                // Format complete
                EndFormatProcessL( KErrNone );
                }
            break;
            }
        default:
            {
            break;
            }
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::RunError
// ---------------------------------------------------------------------------
// 
TInt CFileManagerRemovableDriveHandler::RunError( TInt aError )
    {
    switch( iProcess )
        {
        case MFileManagerProcessObserver::EFormatProcess:
            {
            TRAP_IGNORE( EndFormatProcessL( aError ) );
            break;
            }
        default:
            {
            break;
            }
        }
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::EndFormatProcessL
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::EndFormatProcessL( TInt aErr )
    {
    iFormatter.Close();
    iLastError = aErr;
    if( aErr == KErrNone )
        {
        // Restore the volume name stored before format operation.
        TRAPD( err, RestoreVolumeNameL( iDrive ) );
        LOG_IF_ERROR1( err,
            "FileManagerRemovableDriveHandler::EndFormatProcessL-RestoreVolumeName %d",
            err );
        iUtils.CreateDefaultFolders( iDrive );
        }
    else if ( aErr != KErrCancel )
        {
        InformError( aErr );
        }
    TRAPD( err,  RestartAppsL() );
      LOG_IF_ERROR1( err,
         "FileManagerRemovableDriveHandler::EndFormatProcessL-Restart apps %d",
         err );
    InformFinishL();

    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::StartFormatProcessL
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::StartFormatProcessL()
    {
    // Store the current volume name over format operation.
    TRAPD( err, StoreVolumeNameL( iDrive ) );
    LOG_IF_ERROR1(
        err,
        "FileManagerRemovableDriveHandler::StartFormatProcessL-StoreVolumeName %d",
        err );
    TDriveName driveName( TDriveUnit( iDrive ).Name() );
    // Resolve drive character and open formatter
    iFormatter.Close();
    err = iFormatter.Open(
        iFs, driveName, EFullFormat, iFinalValue );
    // Forced format for locked card
    if ( err == KErrLocked )
        {
        // Erase password and try again
        err = iFs.ErasePassword( iDrive );
        if (err == KErrNone)
        	{
        	err = iFormatter.Open(iFs, driveName, EFullFormat , iFinalValue );        
        	}
        }
    if (err == KErrInUse)
    	{    
    	TBool reallyFormat = ETrue;
    	if (reallyFormat)
    		{    	
    		err = iFormatter.Open(
    				iFs, driveName, EFullFormat | EForceFormat, iFinalValue );    		
    		}
    	}
   TFullName fsName;
   if (err == KErrNone)
	   {
	   err = iFs.FileSystemName( fsName, iDrive );			  

	   if ( err == KErrNone && fsName.Length() > 0 )
		   {
		   // Prevent SysAp shutting down applications
		   RProperty::Set(
				   KPSUidCoreApplicationUIs,
				   KCoreAppUIsMmcRemovedWithoutEject,
				   ECoreAppUIsEjectCommandUsed );
		   }
	   else
		   {
		   // Don't continue with format if there is no file system name
		   // or file system name could not be obtained.
		   err = KErrCancel;
		   }
	   }

    // On successful open of RFormat::Open(), iFinalValue contains the number of
    // tracks to be formatted

    if ( iFinalValue && err == KErrNone )
        {
        TRAP( err, InformStartL( iFinalValue ) );
        if ( err == KErrNone )
            {           
            iFormatCountBuf = iFinalValue;
            iFormatter.Next( iFormatCountBuf, iStatus );
            SetActive();
            }
        }
    if ( !iFinalValue || err != KErrNone )
        {   
        EndFormatProcessL( err );     
        }
    }
// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::CloseAppsL
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::CloseAppsL()
    {
    delete iBSWrapper;
    iBSWrapper = NULL;

    iBSWrapper = CBaBackupSessionWrapper::NewL();

    TBackupOperationAttributes atts(
        MBackupObserver::EReleaseLockNoAccess,
        MBackupOperationObserver::EStart );
    iBSWrapper->NotifyBackupOperationL( atts );
    iBSWrapper->CloseAll( MBackupObserver::EReleaseLockNoAccess, iStatus );
    SetActive();

    // Memory card formatting cannot be executed if there are open files on it.
    // It has been detected, that in some cases memory card using applications 
    // have no time to close file handles before formatting is tried to be executed. 
    // To address this issue, we need to add a delay here after client-notification 
    // about pending format and real formatting procedure.
    User::After( KAppCloseTimeout );
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::RestartAppsL
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::RestartAppsL()
    {
    if ( !iBSWrapper )
        {
        return;
        }

    TBackupOperationAttributes atts(
        MBackupObserver::ETakeLock, MBackupOperationObserver::EEnd );
    iBSWrapper->NotifyBackupOperationL( atts );
    iBSWrapper->RestartAll();

    // Get rid of the wrapper instance
    delete iBSWrapper;
    iBSWrapper = NULL;
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::ResetAndDestroyArchives
// ---------------------------------------------------------------------------
// 
void CFileManagerRemovableDriveHandler::ResetAndDestroyArchives( TAny* aPtr )
    {
    RPointerArray< CMMCScBkupArchiveInfo >* archive = 
        static_cast< RPointerArray< CMMCScBkupArchiveInfo >* >( aPtr );
    archive->ResetAndDestroy();
    archive->Close();
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::BkupToFmgrMask
// ---------------------------------------------------------------------------
// 
TUint32 CFileManagerRemovableDriveHandler::BkupToFmgrMask(
        const TUint32 aBkupMask )
    {
    TUint32 ret( 0 );

    for( TInt i( 0 ); i < KMaskLookupLen; ++i )
        {
        if ( aBkupMask & KMaskLookup[ i ].iBkupMask )
            {
            ret |= KMaskLookup[ i ].iFmgrMask;
            }
        }
    return ret;
    }

#ifdef RD_FILE_MANAGER_BACKUP
// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::FmgrToBkupMask
// ---------------------------------------------------------------------------
// 
TUint32 CFileManagerRemovableDriveHandler::FmgrToBkupMask(
        const TUint32 aFmrgMask )
    {
    TUint32 ret( 0 );

    for( TInt i( 0 ); i < KMaskLookupLen; ++i )
        {
        if ( ( aFmrgMask & EFileManagerBackupContentAll ) ||
            ( aFmrgMask & KMaskLookup[ i ].iFmgrMask ) )
            {
            ret |= KMaskLookup[ i ].iBkupMask;
            }
        }
    return ret;
    }
#endif // RD_FILE_MANAGER_BACKUP

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::IsProcessOngoing
// ---------------------------------------------------------------------------
// 
TBool CFileManagerRemovableDriveHandler::IsProcessOngoing() const
    {
    return iProcess != MFileManagerProcessObserver::ENoProcess;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::PublishBurStatus()
//
// -----------------------------------------------------------------------------
//  
void CFileManagerRemovableDriveHandler::PublishBurStatus( TInt aType )
	{
   _LIT_SECURITY_POLICY_S0( KFileManagerBkupWritePolicy, KFileManagerUID3 );
   _LIT_SECURITY_POLICY_PASS( KFileManagerBkupReadPolicy );

	TInt err( RProperty::Set(
	    KPSUidFileManagerStatus, KFileManagerBkupStatus, aType ) );
	if ( err != KErrNone )
		{
		err = RProperty::Define(
		    KPSUidFileManagerStatus, KFileManagerBkupStatus,
		    RProperty::EInt, KFileManagerBkupReadPolicy,
		    KFileManagerBkupWritePolicy );
		if ( err == KErrNone || err == KErrAlreadyExists )
		    {
		    err = RProperty::Set(
		        KPSUidFileManagerStatus, KFileManagerBkupStatus, aType );
		    }
		}
    LOG_IF_ERROR1(
        err, "FileManagerRemovableDriveHandler::PublishBurStatus-err=%d", err )
	}

// -----------------------------------------------------------------------------
// CFileManagerEngine::IsMassStorageDrive()
//
// -----------------------------------------------------------------------------
//  
#ifdef RD_MULTIPLE_DRIVE

TBool CFileManagerRemovableDriveHandler::IsInternalMassStorage( TInt aDrive )
	{
    FUNC_LOG;

    TBool ret( EFalse );
    TUint driveStatus( 0 );
    DriveInfo::GetDriveStatus( iFs, aDrive, driveStatus );
    if ( ( driveStatus & DriveInfo::EDriveInternal ) &&
         ( driveStatus & DriveInfo::EDriveExternallyMountable ) )
        {
        ret = ETrue;
        }
    INFO_LOG2(
        "FileManagerRemovableDriveHandler::IsInternalMassStorage-drive=%d,ret=%d",
        aDrive, ret );
    return ret;
    }

#else // RD_MULTIPLE_DRIVE

TBool CFileManagerRemovableDriveHandler::IsInternalMassStorage( TInt /*aDrive*/ )
	{
    FUNC_LOG;

    TInt ret( EFalse );
    return ret;
    }

#endif // RD_MULTIPLE_DRIVE

// -----------------------------------------------------------------------------
// CFileManagerEngine::StoreVolumeNameL()
//
// -----------------------------------------------------------------------------
//  
void CFileManagerRemovableDriveHandler::StoreVolumeNameL( TInt aDrive )
	{
    FUNC_LOG;

    if ( IsInternalMassStorage( aDrive ) )
        {
        TVolumeInfo volumeInfo;
        User::LeaveIfError( iFs.Volume( volumeInfo, aDrive ) );
        CRepository* cenRep = CRepository::NewLC( KCRUidFileManagerSettings );
        TFileManagerVolumeNameStore volumeStore;
        TFileManagerVolumeNameStorePckg volumeStorePckg( volumeStore );
        volumeStore.iDrive = aDrive;
        volumeStore.iName.Copy( volumeInfo.iName );
        User::LeaveIfError( cenRep->Set(
            KFileManagerStoredInternalMassStorageVolumeName, volumeStorePckg ) );
        CleanupStack::PopAndDestroy( cenRep );
        INFO_LOG2(
            "FileManagerRemovableDriveHandler::StoreVolumeNameL-drive=%d,name=%S",
            aDrive, &volumeStore.iName );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::RestoreVolumeNameL()
//
// -----------------------------------------------------------------------------
//  
void CFileManagerRemovableDriveHandler::RestoreVolumeNameL( TInt aDrive )
	{
    FUNC_LOG;

    if ( IsInternalMassStorage( aDrive ) )
        {
        TFileName volumeName;
        CRepository* cenRep = CRepository::NewLC( KCRUidFileManagerSettings );
        TFileManagerVolumeNameStore volumeStore;
        TFileManagerVolumeNameStorePckg volumeStorePckg( volumeStore );
        TInt err( cenRep->Get(
            KFileManagerStoredInternalMassStorageVolumeName, volumeStorePckg ) );
        if ( err == KErrNone && volumeStore.iDrive == aDrive )
            {
            volumeName.Copy( volumeStore.iName );
            }
        if ( !volumeName.Length() )
            {
            User::LeaveIfError( cenRep->Get(
                KFileManagerDefaultInternalMassStorageVolumeName, volumeName ) );
            }
        if ( volumeName.Length() > 0 )
            {
            User::LeaveIfError( iFs.SetVolumeLabel( volumeName, aDrive ) );
            }
        CleanupStack::PopAndDestroy( cenRep );
        INFO_LOG2(
            "FileManagerRemovableDriveHandler::RestoreVolumeNameL-drive=%d,name=%S",
            aDrive, &volumeName );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::ListArchivesL
// ---------------------------------------------------------------------------
//  
void CFileManagerRemovableDriveHandler::ListArchivesL(
        RPointerArray< CMMCScBkupArchiveInfo >& aArchives,
        const CFileManagerBackupSettings& aBackupSettings )
    {
    aArchives.ResetAndDestroy();
    TResourceReader driveReader;
    CCoeEnv::Static()->CreateResourceReaderLC(
        driveReader,
        R_FILEMANAGER_BACKUP_RESTORE_DRIVES_AND_OPERATIONS );
    CMMCScBkupOpParamsRestoreFull* params =
        CMMCScBkupOpParamsRestoreFull::NewL(
            driveReader, EBUCatAllSeparately );
    CleanupStack::PopAndDestroy(); // driveReader
    CleanupStack::PushL( params );
    iBkupEngine->ListArchivesL(
        aArchives, params, aBackupSettings.AllowedDriveAttMatchMask() );
    CleanupStack::PopAndDestroy( params );
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::LatestBackupTimeL
// ---------------------------------------------------------------------------
//  
void CFileManagerRemovableDriveHandler::LatestBackupTimeL(
        TTime& aBackupTime )
    {
    aBackupTime = 0;
    CFileManagerBackupSettings& bkupSettings( iEngine.BackupSettingsL() );
    RPointerArray< CMMCScBkupArchiveInfo > archives;
    TCleanupItem cleanupItem( ResetAndDestroyArchives, &archives );
    CleanupStack::PushL( cleanupItem );
    ListArchivesL( archives, bkupSettings );

    // Find the latest archive
    TBool found( EFalse );
    TInt count( archives.Count() );
    for( TInt i( 0 ); i < count; ++i )
        {
        TTime time( archives[ i ]->DateTime() );
        if ( time > aBackupTime )
            {
            aBackupTime = time;
            found = ETrue;
            }
        }
    CleanupStack::PopAndDestroy( &archives );
    if ( !found )
        {
        User::Leave( KErrNotFound );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerRemovableDriveHandler::DeleteBackupsL
// ---------------------------------------------------------------------------
//  
void CFileManagerRemovableDriveHandler::DeleteBackupsL()
    {
    CFileManagerBackupSettings& bkupSettings( iEngine.BackupSettingsL() );
    RPointerArray< CMMCScBkupArchiveInfo > archives;
    TCleanupItem cleanupItem( ResetAndDestroyArchives, &archives );
    CleanupStack::PushL( cleanupItem );
    ListArchivesL( archives, bkupSettings );

    // Get the user set selection
    RArray< CFileManagerRestoreSettings::TInfo > selection;
    CleanupClosePushL( selection );
    CFileManagerRestoreSettings& rstSettings( iEngine.RestoreSettingsL() );
    rstSettings.GetSelectionL( selection );

    // Delete selected archives
    TInt archivesCount( archives.Count() );
    for ( TInt i( 0 ); i < archivesCount; ++i )
        {
        CMMCScBkupArchiveInfo* archiveInfo = archives[ i ];
        TUint32 fmgrContent(
            BkupToFmgrMask( archiveInfo->Category().iFlags ) );
        TInt drive( archiveInfo->Drive() );
        TInt selectionCount( selection.Count() );
        for( TInt j( 0 ); j < selectionCount; ++j )
            {
            const CFileManagerRestoreSettings::TInfo& info( selection[ j ] );
            if ( ( drive == info.iDrive ) && ( fmgrContent & info.iContent ) )
                {
                TPtrC fullPath( archiveInfo->FileName() );
                User::LeaveIfError(
                    CFileManagerUtils::RemoveReadOnlyAttribute(
                        iFs, fullPath ) );
                User::LeaveIfError( iFs.Delete( fullPath ) );
                selection.Remove( j ); // Do not check again
                break;
                }
            }
        }
    CleanupStack::PopAndDestroy( &selection );
    CleanupStack::PopAndDestroy( &archives );
    }


//  End of File  
