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
* Description:  Handles file manager scheduled backup task start
*
*/



// INCLUDE FILES
#include <e32base.h>
#include <schtask.h>
#include <w32std.h>
#include <apgtask.h>
#include <apgcli.h>
#include <apgwgnam.h>
#include <apacmdln.h>
#include <centralrepository.h>
#include <ctsydomainpskeys.h>
#include <UsbWatcherInternalPSKeys.h>
#include <usbpersonalityids.h>
#include <DataSyncInternalPSKeys.h>
#include <connect/sbdefs.h>
#include "filemanagerschbackuptask.h"
#include "filemanagerschsubscriber.h"
#include "FileManagerUID.h"
#include "FileManagerDebug.h"
#include "FileManagerSchDefinitions.h"
#include "FileManagerPrivateCRKeys.h"

using namespace conn;

// CONSTANTS
const TUid KUidFileManager = { KFileManagerUID3 };
_LIT_SECURE_ID( KFileManagerSID, KFileManagerUID3 );
const TInt KStartStateChangeTimeoutSecs = 30;
const TInt KPhoneStateChangeTimeoutSecs = 120;
const TInt KNumAttempts = -1; // Try forever


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// FindStandaloneAppL
// ---------------------------------------------------------------------------
//
static TBool FindStandaloneAppL(
        RWsSession& aWs, const TUid& aUid, TInt& aWgId )
    {
    FUNC_LOG

    aWgId = 0; // Used window group id is always greater than zero
    RArray< RWsSession::TWindowGroupChainInfo > windowChain;
    User::LeaveIfError( aWs.WindowGroupList( &windowChain ) );
    CleanupClosePushL( windowChain );
    TInt count( windowChain.Count() );
    for( TInt i( 0 ); i < count; ++i )
        {
        const RWsSession::TWindowGroupChainInfo& entry( windowChain[ i ] );
        CApaWindowGroupName* app = CApaWindowGroupName::NewLC(
            aWs, entry.iId );
        TUid appUid( app->AppUid() );
        CleanupStack::PopAndDestroy( app );
        // Match the app's UID and the embedded status.
        // The app is standalone when there is no parent window group.
        if ( appUid == aUid && entry.iParentId <= 0 )
            {
            // Standalone application found
            aWgId = entry.iId;
            break;
            }
        }
    CleanupStack::PopAndDestroy( &windowChain );
    return aWgId > 0;
    }


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::CFileManagerSchBackupTask
// ---------------------------------------------------------------------------
//
CFileManagerSchBackupTask::CFileManagerSchBackupTask() :
        CActive( EPriorityStandard ),
        iDay( KErrNotFound ),
        iAttemptsLeft( KNumAttempts )
    {
    FUNC_LOG
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::NewL
// ---------------------------------------------------------------------------
//
CFileManagerSchBackupTask* CFileManagerSchBackupTask::NewL(
        const CScheduledTask& aTask )
    {
    FUNC_LOG

    CFileManagerSchBackupTask* self =
        new ( ELeave ) CFileManagerSchBackupTask();
    CleanupStack::PushL( self );
    self->ConstructL( aTask );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::~CFileManagerSchBackupTask
// ---------------------------------------------------------------------------
//
CFileManagerSchBackupTask::~CFileManagerSchBackupTask()
    {
    FUNC_LOG

    delete iSubscriber;
    delete iCenRep;
    delete iSystemStateMonitor;
    Cancel();
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::RunL
// ---------------------------------------------------------------------------
//
void CFileManagerSchBackupTask::RunL()
    {
    StartL();
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::DoCancel
// ---------------------------------------------------------------------------
//
void CFileManagerSchBackupTask::DoCancel()
    {
    delete iSubscriber;
    iSubscriber = NULL;
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::RunError()
// ---------------------------------------------------------------------------
//
TInt CFileManagerSchBackupTask::RunError(
#ifdef FILE_MANAGER_ERROR_LOG_ENABLED
    TInt aError
#else // FILE_MANAGER_ERROR_LOG_ENABLED
    TInt /*aError*/
#endif // FILE_MANAGER_ERROR_LOG_ENABLED
     )
    {    
    ERROR_LOG1( "CFileManagerSchBackupTask::RunError()-Error=%d", aError )

    // Try again
    Retry();

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::ConstructL
// ---------------------------------------------------------------------------
//
void CFileManagerSchBackupTask::ConstructL( const CScheduledTask& aTask )
    {
    FUNC_LOG

    INFO_LOG1( "CFileManagerTaskScheduler::ConstructL()-Repeat=%d",
        aTask.Info().iRepeat )

    CActiveScheduler::Add( this );

    // Check task creator
    if( aTask.SecurityInfo().iSecureId.iId != KFileManagerSID )
        {
        User::Leave( KErrPermissionDenied );
        }

    // Get pointer to file manager settings
    iCenRep = CRepository::NewL( KCRUidFileManagerSettings );

    // Get backup weekday if given from originator
    const HBufC& data( aTask.Data() );
    if ( data.Length() )
        {
        TLex parse;
        parse.Assign( data );
        User::LeaveIfError( parse.Val( iDay ) );
        }
    
    // Get pointer to system state monitor
    iSystemStateMonitor = CFmSystemStateMonitor::NewL( *this );

    Retry();
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::CheckBackupRequired
// ---------------------------------------------------------------------------
//
TBool CFileManagerSchBackupTask::CheckBackupRequired()
    {
    TBool ret( ETrue );

    // Check schedule type
    TInt scheduling( 0 );
    iCenRep->Get( KFileManagerBackupScheduling, scheduling );
    if ( scheduling == EFileManagerBackupScheduleNone )
        {
        // Backup is not required
        ret = EFalse;
        }

    // Check weekday
    if ( ret && iDay != KErrNotFound )
        {
        TTime time;
        time.HomeTime();
        if ( time.DayNoInWeek() != iDay )
            {
            // Backup is not required today
            ret = EFalse;
            }
        }

    if ( !ret )
        {
        INFO_LOG( "CFileManagerSchBackupTask::CheckBackupRequired()-No" )
        Exit();
        }

    return ret;
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::CheckPhoneStateL
// ---------------------------------------------------------------------------
//
TBool CFileManagerSchBackupTask::CheckPhoneStateL()
    {
    
    // Check phone is normal state or not
    if ( !iSystemStateMonitor->IsSystemStateNormal() )
       {
        INFO_LOG( "CFileManagerSchBackupTask::CheckPhoneStateL()-System State is not normal" )
        iSystemStateMonitor->StartMonitor( ESsmNormal );
        return EFalse;
       }
    
    // Check call state
    TInt callState( 0 );
    RProperty::Get(
        KPSUidCtsyCallInformation, KCTsyCallState, callState ); 
    if ( callState != EPSCTsyCallStateNone )
        {
        INFO_LOG1( "CFileManagerSchBackupTask::CheckPhoneStateL()-callState=%d",
            callState )

        // Set subscriber to wait call to finish
        iSubscriber = CFileManagerSchSubscriber::NewL(
            *this,
            KPSUidCtsyCallInformation,
            KCTsyCallState,
            CFileManagerSchSubscriber::ESubscribePS,
            KPhoneStateChangeTimeoutSecs );
        return EFalse;
        }

    // Check usb transfer state
    TInt usbState( 0 );
    RProperty::Get(
        KPSUidUsbWatcher, KUsbWatcherSelectedPersonality, usbState );
    if ( usbState == KUsbPersonalityIdMS )
        {
        INFO_LOG( "CFileManagerSchBackupTask::CheckPhoneStateL()-Usb transfer on" )

        // Set subscriber to wait usb transfer to finish
        iSubscriber = CFileManagerSchSubscriber::NewL(
            *this,
            KPSUidUsbWatcher,
            KUsbWatcherSelectedPersonality,
            CFileManagerSchSubscriber::ESubscribePS,
            KPhoneStateChangeTimeoutSecs );
        return EFalse;
        }

    // Check synchronization state
    TInt syncState( 0 );
    RProperty::Get(
        KPSUidDataSynchronizationInternalKeys, KDataSyncStatus, syncState );
    if ( syncState > 0 )
        {
        INFO_LOG( "CFileManagerSchBackupTask::CheckPhoneStateL()-Synchronization on" )

        // Set subscriber to wait usb transfer to finish
        iSubscriber = CFileManagerSchSubscriber::NewL(
            *this,
            KPSUidDataSynchronizationInternalKeys,
            KDataSyncStatus,
            CFileManagerSchSubscriber::ESubscribePS,
            KPhoneStateChangeTimeoutSecs );
        return EFalse;
        }

    // Check backup/restore (e.g. PC Suite initiated) state
    TInt burState( 0 );
    RProperty::Get(
        KUidSystemCategory, KUidBackupRestoreKey, burState );

    const TBURPartType partType = static_cast< TBURPartType >
        ( burState & KBURPartTypeMask );
        
    if ( partType != EBURUnset && partType != EBURNormal )
        {
        INFO_LOG( "CFileManagerSchBackupTask::CheckPhoneStateL()-Backup/restore on" )

        // Set subscriber to wait backup or restore to finish
        iSubscriber = CFileManagerSchSubscriber::NewL(
            *this,
            KUidSystemCategory,
            KUidBackupRestoreKey,
            CFileManagerSchSubscriber::ESubscribePS,
            KPhoneStateChangeTimeoutSecs );
        return EFalse;
        }

    return ETrue;
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::StartL
// ---------------------------------------------------------------------------
//
void CFileManagerSchBackupTask::StartL()
    {
    FUNC_LOG

    delete iSubscriber;
    iSubscriber = NULL;

    if ( !CheckBackupRequired() )
        {
        return;
        }
    if ( !CheckPhoneStateL() )
        {
        return;
        }

    // Set backup starting state
    iCenRep->Set(
        KFileManagerLastSchBackupStatus,
        EFileManagerSchBackupStatusStarting );

    // Set subscriber to watch backup start success
    iSubscriber = CFileManagerSchSubscriber::NewL(
        *this,
        KCRUidFileManagerSettings,
        KFileManagerLastSchBackupStatus,
        CFileManagerSchSubscriber::ESubscribeCR,
        KStartStateChangeTimeoutSecs );

    // Start file manager for backup and ensure that it starts properly
    StartFileManagerL();
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::Exit
// ---------------------------------------------------------------------------
//
void CFileManagerSchBackupTask::Exit()
    {
    FUNC_LOG

    Cancel();
    CActiveScheduler::Stop();
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::Retry
// ---------------------------------------------------------------------------
//
void CFileManagerSchBackupTask::Retry()
    {
    if ( iAttemptsLeft != 0 )
        {
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();

        if ( iAttemptsLeft > 0 )
            {
            --iAttemptsLeft;
            }
        }
    else
        {
        Exit();
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::StartFileManagerL
// ---------------------------------------------------------------------------
//
void CFileManagerSchBackupTask::StartFileManagerL()
    {
    FUNC_LOG

    RWsSession wsSession;
    User::LeaveIfError( wsSession.Connect() );
    CleanupClosePushL( wsSession );
    // Backup cannot be done from embedded file manager.
    // Backup operation closes the app where file manager is started and 
    // the backup operation stops because file manager gets closed as well.
    TInt wgId( 0 );
    if ( FindStandaloneAppL( wsSession, KUidFileManager, wgId ) )
        {
        // Notify running standalone file manager to start backup
        HBufC8* param8 = HBufC8::NewLC( KSchBackupTaskName().Length() );
        TPtr8 paramPtr( param8->Des() );
        paramPtr.Copy( KSchBackupTaskName );
        wsSession.SendMessageToWindowGroup(
            wgId,
            TUid::Uid( KUidApaMessageSwitchOpenFileValue ),
            *param8 );
        CleanupStack::PopAndDestroy( param8 );
        }
    else
        {
        // Start standalone file manager for backup
        TApaAppInfo appInfo;
        RApaLsSession apaLsSession;
        User::LeaveIfError( apaLsSession.Connect() );
        CleanupClosePushL( apaLsSession );
        User::LeaveIfError( apaLsSession.GetAppInfo(
            appInfo, KUidFileManager ) );
        CApaCommandLine* apaCmdLine = CApaCommandLine::NewLC();
        apaCmdLine->SetExecutableNameL( appInfo.iFullName );
        apaCmdLine->SetCommandL( EApaCommandBackground );
        TThreadId fmThreadId;
        User::LeaveIfError( apaLsSession.StartApp(
            *apaCmdLine, fmThreadId ) );
        CleanupStack::PopAndDestroy( apaCmdLine );
        CleanupStack::PopAndDestroy( &apaLsSession );
        }
    CleanupStack::PopAndDestroy( &wsSession );
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::NotifyKeyChangeOrTimeoutL
// ---------------------------------------------------------------------------
//
void CFileManagerSchBackupTask::NotifyKeyChangeOrTimeoutL(
        const TUid& aCategory,
        const TUint aKey,
        const TBool aTimeout )
    {
    FUNC_LOG

    if ( aCategory == KCRUidFileManagerSettings &&
        aKey == KFileManagerLastSchBackupStatus &&
        !aTimeout )
        {
        // Backup was started, the task starter can be finished
        Exit();
        }
    else
        {
        // Backup was not started, try starting it again
        Retry();
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerSchBackupTask::SystemStateChangedEvent
// ---------------------------------------------------------------------------
//
void CFileManagerSchBackupTask::SystemStateChangedEvent()
    {
    FUNC_LOG
    
    // Backup was not started, try starting it again
    Retry();
    }

// End of file
