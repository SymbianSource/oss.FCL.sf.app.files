/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of CDevEncUiMemoryEntity.
*
*/

#include "DevEncLog.h"
#include "DevEncUiMemoryEntity.h"
#include "DevEncUiMemInfoObserver.h"

#include "DevEncDiskUtils.h"
#include "DevEncSession.h"
#include <f32file.h> // for TDriveNumer
#include <DevEncEngineConstants.h>
#include <TerminalControl3rdPartyAPI.h>

// Local definitions
const TInt KPercent100( 100 );
const TTimeIntervalMicroSeconds32 KProgressInterval( 500000 ); // ms, 0.5 seconds
const TTimeIntervalMicroSeconds32 KPollInterval( 2000000 ); // ms, 2 seconds

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::CDevEncUiMemoryEntity()
// 
// --------------------------------------------------------------------------
CDevEncUiMemoryEntity::CDevEncUiMemoryEntity(
                        CEikonEnv* aEikonEnv,
                        TDevEncUiMemoryType aType )
    : iEikEnv( aEikonEnv ),
      iType( aType )
    {
    // TBI: Get the real states from the underlying SW levels
	iState = EDecrypted;
    }


// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::~CDevEncUiMemoryEntity()
// 
// --------------------------------------------------------------------------
CDevEncUiMemoryEntity::~CDevEncUiMemoryEntity()
    {
    if ( iPeriodic )
        {
        iPeriodic->Cancel();
        delete iPeriodic;
        }
    iObservers.Close();
    if ( iSession )
        {
        iSession->Close();
        delete iSession;
        }
    delete iDiskStatusObserver;
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::NewL()
// 
// --------------------------------------------------------------------------
CDevEncUiMemoryEntity* CDevEncUiMemoryEntity::NewL(
                                CEikonEnv* aEikonEnv,
                                TDevEncUiMemoryType aType )
	{
	CDevEncUiMemoryEntity* self =
	    CDevEncUiMemoryEntity::NewLC( aEikonEnv, aType );
	CleanupStack::Pop( self );
	return self;
	}

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::NewLC()
// 
// --------------------------------------------------------------------------
CDevEncUiMemoryEntity* CDevEncUiMemoryEntity::NewLC(
                                CEikonEnv* aEikonEnv,
                                TDevEncUiMemoryType aType )
	{
	CDevEncUiMemoryEntity* self =
	    new ( ELeave ) CDevEncUiMemoryEntity( aEikonEnv, aType );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::ConstructL()
// 
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::ConstructL()
	{
    DFLOG2( ">>CDevEncUiMemoryEntity::ConstructL, drive = %d", iType );

    switch( iType )
        {
    	case EPhoneMemory: iSession = new ( ELeave ) CDevEncSession( /*EDriveC*/EDriveE );
                           iDiskStatusObserver = CDiskStatusObserver::NewL( this, /*EDriveC*/EDriveE ); 
    		               break;
    		               
    	case EPrimaryPhoneMemory: iSession = new ( ELeave ) CDevEncSession( EDriveC );
                          iDiskStatusObserver = CDiskStatusObserver::NewL( this, EDriveC );
    	    		      break;
    	    		      
    	case EMemoryCard: iSession = new ( ELeave ) CDevEncSession( /*EDriveE*/EDriveF );
                          iDiskStatusObserver = CDiskStatusObserver::NewL( this, /*EDriveE*/EDriveF );
    	    		      break;    	    		               
        }

    // Get initial memory state
    TInt nfeDiskStatus( EUnmounted );
#ifndef __WINS__
    User::LeaveIfError( iSession->Connect() );
    TInt err = iSession->DiskStatus( nfeDiskStatus );
    if ( err )
        {
        DFLOG2( "Could not get disk status, error %d", err );
        nfeDiskStatus = EDecrypted;
        //User::Leave( err );
        }
#else
    nfeDiskStatus = EDecrypted;
#endif
    SetState( nfeDiskStatus );

    if ( ( iState == EEncrypting ) ||
         ( iState == EDecrypting ) )
        {
        // If the app was started in the middle of an ongoing operation,
        // start polling the progress
        StartPolling( KProgressInterval );
        }
    else
        {
        // Otherwise poll every once in a while to see if the status changes
        //StartPolling( KPollInterval );
        }

    DFLOG( "<<CDevEncUiMemoryEntity::ConstructL" );
	}

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::AddObserverL()
// 
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::AddObserverL( MDevEncUiMemInfoObserver* aInfoObserver )
    {
    DFLOG( "CDevEncUiMemoryEntity::AddObserverL" );
    if ( !aInfoObserver )
        {
        User::Leave( KErrArgument );
        }
    iObservers.AppendL( aInfoObserver );

    // The new observer probably wants to know the current states
    UpdateMemoryInfo();
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::RemoveObserver()
// 
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::RemoveObserver( MDevEncUiMemInfoObserver* aInfoObserver )
    {
    if ( !aInfoObserver )
        {
        return;
        }
    TInt index( iObservers.Find( aInfoObserver ) );
    if ( index != KErrNotFound )
        {
        iObservers.Remove( index );
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::StartEncryptionL()
// 
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::StartEncryptionL()
    {
    DFLOG( ">>CDevEncUiMemoryEntity::StartEncryptionL" );

#ifndef __WINS__

    if ( ! iSession->Connected() )
        {
        User::LeaveIfError( iSession->Connect() );
        DFLOG( "Connected" );
        }

    // Make sure the memory is decrypted
    TInt nfeDiskStatus( EUnmounted );
    TInt err = iSession->DiskStatus( nfeDiskStatus );
    if ( err )
        {
        DFLOG2( "Could not get disk status, error %d", err );
        User::Leave( err );
        }
    if ( nfeDiskStatus != EDecrypted )
        {
        DFLOG2( "Memory is in wrong state (%d), leaving", nfeDiskStatus );
        User::Leave( KErrNotReady );
        }

    // Launch the encryption process
    err = iSession->StartDiskEncrypt();
    if ( err )
        {
        DFLOG2( "Could not start encryption, error %d", err );
        User::Leave( err );
        }

    // Set our internal state and update UI
    iPercentDone = 0;
    SetState( EEncrypting );

    // Start a timer to periodically update the memory state in the UI
    StartPolling( KProgressInterval );

#endif // __WINS__

    DFLOG( "<<CDevEncUiMemoryEntity::StartEncryptionL" );
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::StartPolling()
// Starts a timer to periodically update the memory state in the UI
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::StartPolling(
                                      TTimeIntervalMicroSeconds32 aInterval )
    {
    TInt error( KErrNone );
    TRAP( error, DoStartPollingL( aInterval ) );
    DFLOG2( "CDevEncUiMemoryEntity::StartPolling result %d", error );
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::DoStartPollingL()
// Starts a timer to periodically update the memory state in the UI
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::DoStartPollingL(
                                      TTimeIntervalMicroSeconds32 aInterval )
    {
    TCallBack pollCallBack( PollTick, static_cast<TAny*>( this ) );
    TCallBack progressCallBack( ProgressTick, static_cast<TAny*>( this ) );

    if ( !iPeriodic )
        {
        iPeriodic = CPeriodic::NewL( EPriorityNormal );
        }
    iPeriodic->Cancel();

    if ( aInterval == KProgressInterval )
        {
        iPeriodic->Start( 0, aInterval, progressCallBack );
        }
    else
        {
        iPeriodic->Start( 0, aInterval, pollCallBack );
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::StartDecryptionL()
// 
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::StartDecryptionL()
    {
    DFLOG( ">>CDevEncUiMemoryEntity::StartDecryptionL" );

#ifndef __WINS__

    if ( ! iSession->Connected() )
        {
        User::LeaveIfError( iSession->Connect() );
        DFLOG( "Connected " );
        }

    // Make sure the memory is encrypted
    TInt nfeDiskStatus( EUnmounted );
    TInt err = iSession->DiskStatus( nfeDiskStatus );
    if ( err )
        {
        DFLOG2( "Could not get disk status, error %d", err );
        //User::Leave( err );
        }
    if ( nfeDiskStatus != EEncrypted )
        {
        DFLOG2( "Memory is in wrong state (%d), leaving", nfeDiskStatus );
        User::Leave( KErrNotReady );
        }

    // Launch the decryption process
    err = iSession->StartDiskDecrypt();
    if ( err )
        {
        DFLOG2( "Could not start decryption, error %d", err );
        User::Leave( err );
        }

    // Set our internal state and update UI
    iPercentDone = 0;
    SetState( EDecrypting );

    // Start a timer to periodically update the memory state in the UI
    StartPolling( KProgressInterval );

#endif // __WINS__
    
    DFLOG( "<<CDevEncUiMemoryEntity::StartDecryptionL" );
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::ProgressTick()
// 
// --------------------------------------------------------------------------
TInt CDevEncUiMemoryEntity::ProgressTick( TAny* aPtr )
    {
    CDevEncUiMemoryEntity* self = static_cast<CDevEncUiMemoryEntity*>( aPtr );
    self->DoProgressTick();
    return 0;
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::DoProgressTick()
// 
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::DoProgressTick()
    {
    CheckProgress();
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::PollTick()
// 
// --------------------------------------------------------------------------
TInt CDevEncUiMemoryEntity::PollTick( TAny* aPtr )
    {
    CDevEncUiMemoryEntity* self = static_cast<CDevEncUiMemoryEntity*>( aPtr );
    TInt error( KErrNone );
    TRAP( error, self->DoPollTickL() );
    DFLOG2( "CDevEncUiMemoryEntity::PollTick result %d", error );
    return 0;
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::DoPollTickL()
// 
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::DoPollTickL()
    {
    DFLOG( "CDevEncUiMemoryEntity::DoPollTickL" );
    TInt nfeDiskStatus( EUnmounted );
    TInt err = iSession->DiskStatus( nfeDiskStatus );
    if ( err )
        {
        DFLOG2( "Could not get disk status, error %d", err );
        // Ignore error
        return;
        }
    if ( ( nfeDiskStatus == EEncrypting ) ||
         ( nfeDiskStatus == EDecrypting ) )
        {
        // Some other component has started an encryption operation.
        // Indicate this to the UI and start polling the progress.
        DFLOG2( "CDevEncUiMemoryEntity::DoPollTick: New operation %d",
                nfeDiskStatus );
        SetState( nfeDiskStatus );
        StartPolling( KProgressInterval );
        }
    else
        {
        if ( iState != nfeDiskStatus )
            {
            DFLOG( "CDevEncUiMemoryEntity::DoPollTickL => SetState" );
            // The Mmc status has changed, but we are not in the middle of
            // any operation. Just set the new state.
            SetState( nfeDiskStatus );
            }
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::SetState()
// 
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::SetState( TUint aState )
    {
    DFLOG3( "CDevEncUiMemoryEntity::SetState, prev %d, new %d",
            iState, aState );
    iState = aState;
    UpdateMemoryInfo();
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::CheckProgress()
// 
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::CheckProgress()
    {
    ASSERT( iSession );
    ASSERT( iSession->Connected() );

    TInt err = iSession->Progress( iPercentDone );

    if ( err )
        {
        DFLOG( "Could not get progress" );
        iPeriodic->Cancel();
        }
    else if ( iPercentDone >= KPercent100 )
        {
        DFLOG( "Operation complete" );
        iPeriodic->Cancel();
        iPercentDone = 0;

        // The disk needs to be finalized, but the finalization is done in
        // the starter component.
        
        DFLOG2( "CDevEncUiMemoryEntity::CheckProgress => iSession->DriveNumber() = %d", iSession->DriveNumber()  );

        if ( iState == EDecrypted && iSession->DriveNumber() == EDriveC )
            {
            DFLOG( "CDevEncUiMemoryEntity::CheckProgress => RestoreAutolockSettings" );
            RestoreAutolockSettings();
            }
        // Restart the polling at a slower pace
        // StartPolling( KPollInterval );
        }
    else // Not yet finished...
        {
        UpdateMemoryInfo();
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::RestoreAutolockSettings()
// 
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::RestoreAutolockSettings()
    {
#ifndef __WINS__
    //the user chose to decrypt the phone memory. restore the previuos autolock settings
    RTerminalControl terminalCtrl;
    TInt ctrlConnect = terminalCtrl.Connect();
    DFLOG2( "DEVENC: terminal control connected %d", ctrlConnect );
    
    RTerminalControl3rdPartySession session;
    TInt retValue = session.Open( terminalCtrl );
    DFLOG2( "DEVENC: terminal control session open %d", retValue );
    
    //read the current autolock period
    TBuf8<21> autoLockPeriodBuf;
    TInt err2 = session.GetDeviceLockParameter( RTerminalControl3rdPartySession::ETimeout, autoLockPeriodBuf );
    DFLOG2( "DEVENC: max value get returned value %d", err2 );
    
    //this is set to 0 because the Tarm control prevent the autolock from being disabled otherwise.
    TBuf8<21> oldMaxPeriodBuf;
    oldMaxPeriodBuf.AppendNum( 0 );
    TInt err = session.SetDeviceLockParameter( RTerminalControl3rdPartySession::EMaxTimeout, oldMaxPeriodBuf );
    DFLOG2( "DEVENC: max value set returned value %d", err );
    
    //set the autolock period as it was previously
    err = session.SetDeviceLockParameter( RTerminalControl3rdPartySession::ETimeout, autoLockPeriodBuf );
    DFLOG2( "DEVENC: max value set returned value %d", err );
    
    session.Close();
    terminalCtrl.Close();
#endif
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::UpdateMemoryInfo()
// 
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::UpdateMemoryInfo()
    {
    DFLOG( "CDevEncUiMemoryEntity::UpdateMemoryInfo" );
    for ( TInt i = 0; i < iObservers.Count(); i++ )
        {
        iObservers[i]->UpdateInfo( iType, iState, iPercentDone );
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::State()
// 
// --------------------------------------------------------------------------
TUint CDevEncUiMemoryEntity::State() const
    {
    return iState;
    }

// --------------------------------------------------------------------------
// CDevEncUiMemoryEntity::DiskStatusChangedL()
// From MDiskStatusObserver
// --------------------------------------------------------------------------
void CDevEncUiMemoryEntity::DiskStatusChangedL( TInt aNfeStatus )
    {
    if ( ( aNfeStatus == EEncrypting ) ||
         ( aNfeStatus == EDecrypting ) )
        {
        // Some other component has started an encryption operation.
        // Indicate this to the UI and start polling the progress.
        DFLOG2( "CDevEncStarterMemoryEntity::DoPollTick: New operation %d",
        		aNfeStatus );
        SetState( aNfeStatus );
        StartPolling( KProgressInterval );
        }
    else
        {
        if ( iState != aNfeStatus )
            {
            // The Mmc status has changed, but we are not in the middle of
            // any operation. Just set the new state.
            SetState( aNfeStatus );
            }
        }
    }

// End of File



