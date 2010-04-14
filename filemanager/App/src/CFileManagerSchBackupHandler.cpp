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
* Description:  Handler for scheduled backup
*
*/


 #define FILE_MANAGER_POWER_SAVING_MODE

// INCLUDE FILES
#include <StringLoader.h>
#include <centralrepository.h>
#include <avkon.hrh>
#include <avkon.rsg>
#include <FileManager.rsg>
#include <FileManagerDebug.h>
#include <FileManagerPrivateCRKeys.h>
#include <CFileManagerEngine.h>
#include <CFileManagerBackupSettings.h>
#include <MFileManagerProcessObserver.h>
#include <CFileManagerGlobalDlg.h>
#ifdef FILE_MANAGER_POWER_SAVING_MODE
 #include <psmsrvdomaincrkeys.h>
#endif // FILE_MANAGER_POWER_SAVING_MODE
#include "CFileManagerSchBackupHandler.h"
#include "MFileManagerSchBackupObserver.h"


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::CFileManagerSchBackupHandler
// ----------------------------------------------------------------------------
//
CFileManagerSchBackupHandler::CFileManagerSchBackupHandler(
        CFileManagerEngine& aEngine ) :
    iEngine( aEngine )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::NewL
// ----------------------------------------------------------------------------
//
CFileManagerSchBackupHandler* CFileManagerSchBackupHandler::NewL(
        CFileManagerEngine& aEngine )
    {
    CFileManagerSchBackupHandler* self =
        new( ELeave ) CFileManagerSchBackupHandler( aEngine );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::ConstructL
// ----------------------------------------------------------------------------
// 
void CFileManagerSchBackupHandler::ConstructL()
    {
    FUNC_LOG

    iGlobalDlg = CFileManagerGlobalDlg::NewL();
    iGlobalDlg->SetObserver( this );

   	iCenRep = CRepository::NewL( KCRUidFileManagerSettings );
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::~CFileManagerSchBackupHandler
// ----------------------------------------------------------------------------
// 
CFileManagerSchBackupHandler::~CFileManagerSchBackupHandler()
    {
    FUNC_LOG

    delete iGlobalDlg;
    delete iCenRep;
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::HandleGlobalDlgResult
// ----------------------------------------------------------------------------
// 
void CFileManagerSchBackupHandler::HandleGlobalDlgResult(
        TInt aDlgType,
        TInt aDlgResult )
    {
    INFO_LOG2(
        "CFileManagerSchBackupHandler::HandleGlobalDlgResult-aDlgType=%d,aDlgResult=%d",
        aDlgType, aDlgResult )
    TInt err( KErrNone );
    switch ( aDlgType  )
        {
        case CFileManagerGlobalDlg::ECountdownQuery:
            {
            if ( aDlgResult == EAknSoftkeyYes )
                {
                // Backup was accepted
                StartBackup();

                err = iCenRep->Set(
                    KFileManagerLastSchBackupStatus,
                    EFileManagerSchBackupStatusInProgress );
                }
            else
                {
                // Backup was rejected
                InformObserver( KErrCancel );
                CancelBackupStarter();
                }
            break;
            }
        case CFileManagerGlobalDlg::EProgressDialog:
            {
            // Backup was canceled
            iEngine.CancelProcess(
                MFileManagerProcessObserver::ESchBackupProcess );
            InformObserver( KErrCancel );
            CancelBackupStarter();
            break;
            }
        default:
            {
            InformObserver( aDlgResult );
            break;
            }
        }
        
    LOG_IF_ERROR1( err,
        "CFileManagerSchBackupHandler::HandleGlobalDlgResult-err=%d", err )

    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::StartBackupWithConfirmL
// ----------------------------------------------------------------------------
//
void CFileManagerSchBackupHandler::StartBackupWithConfirmL()
    {
    TBool psmOn( EFalse );
    TRAPD( err, psmOn = IsPowerSavingModeOnL() );
    if ( psmOn && err == KErrNone )
        {
        iGlobalDlg->ShowDialogL(
            CFileManagerGlobalDlg::EQueryWithInfoIcon,
            R_QTN_FMGR_SCHEDULED_BACKUP_CANCELLED_PS,
            R_AVKON_SOFTKEYS_OK_EMPTY );
        CancelBackupStarter();
        return;
        }

    // Show cancelable global confirm countdown before starting backup.
    // Backup is started only if accepted or countdown has expired.
    iGlobalDlg->ShowDialogL(
        CFileManagerGlobalDlg::ECountdownQuery,
        R_QTN_FMGR_SCHEDULED_BACKUP_QUERY );
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::StartBackupL
// ----------------------------------------------------------------------------
//
void CFileManagerSchBackupHandler::StartBackupL()
    {
    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
    TFileManagerDriveInfo drvInfo;
    iEngine.GetDriveInfoL( settings.TargetDrive(), drvInfo );
    TBool drvAvailable( EFalse );

    if( !IsTargetDriveAvailableL(drvInfo ) )
        {
        for( TInt i ( EDriveA ); i <= EDriveZ; i++ )
            {
            iEngine.GetDriveInfoL( i , drvInfo );
            if( ( IsTargetDriveAvailableL( drvInfo ) ) && ( drvInfo.iState & TFileManagerDriveInfo::EDriveEjectable ) )
                {
                 settings.SetTargetDrive( i );
                 settings.SaveL();
                 drvAvailable = ETrue;
                 break;
                }
            }
        }
    else
        {
        drvAvailable = ETrue;
        }
    if ( !drvAvailable )
        {
        iGlobalDlg->ShowDialogL(
            CFileManagerGlobalDlg::EQueryWithWarningIcon,
            R_QTN_FMGR_SCHEDULED_BACKUP_ERROR,
            R_AVKON_SOFTKEYS_OK_EMPTY );
        CancelBackupStarter();
        return;
        }

    HBufC* text = StringLoader::LoadLC(
        R_QTN_FMGR_SCHEDULED_BACKUP_PROGRESS,
        iEngine.DriveName( settings.TargetDrive() ) );

    iGlobalDlg->ShowDialogL(
        CFileManagerGlobalDlg::EProgressDialog,
        *text );

    CleanupStack::PopAndDestroy( text );

    iEngine.StartBackupProcessL(
        MFileManagerProcessObserver::ESchBackupProcess );
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::SetObserver
// ----------------------------------------------------------------------------
//
void CFileManagerSchBackupHandler::SetObserver(
        MFileManagerSchBackupObserver* aObserver )
    {
    iObserver = aObserver;
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::StartBackupWithConfirm
// ----------------------------------------------------------------------------
//
void CFileManagerSchBackupHandler::StartBackupWithConfirm()
    {
    FUNC_LOG

    if ( iBackupOngoing )
        {
        return;
        }

    iBackupOngoing = ETrue;

    TRAPD( err, StartBackupWithConfirmL() );
    if ( err != KErrNone )
        {
        InformObserver( err );
        }
    else
        {
        // Confirm scheduled backup start state
        TInt err( iCenRep->Set(
            KFileManagerLastSchBackupStatus,
            EFileManagerSchBackupStatusInProgress ) );

        LOG_IF_ERROR1( err,
            "CFileManagerSchBackupHandler::StartBackupWithConfirm-err=%d", err )

        }
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::StartBackup
// ----------------------------------------------------------------------------
//
void CFileManagerSchBackupHandler::StartBackup()
    {
    FUNC_LOG

    TRAPD( err, StartBackupL() );
    if ( err != KErrNone )
        {
        InformObserver( err );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::InformObserver
// ----------------------------------------------------------------------------
//
void CFileManagerSchBackupHandler::InformObserver( const TInt aValue )
    {
    INFO_LOG1( "CFileManagerSchBackupHandler::InformObserver-aValue=%d", aValue )

    iBackupOngoing = EFalse;
    if ( iObserver )
        {
        TRAP_IGNORE( iObserver->SchBackupFinishedL( aValue ) );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::ProcessFinishedL
// ----------------------------------------------------------------------------
//
void CFileManagerSchBackupHandler::ProcessFinishedL(
        TInt aError, const TDesC& /*aName*/ )
    {
    INFO_LOG1( "CFileManagerSchBackupHandler::ProcessFinishedL-aError=%d",
        aError )

    iGlobalDlg->CancelDialog();
    TInt err( KErrNone );
    if ( aError == KErrCancel )
        {
        CancelBackupStarter();
        }
    else if ( aError != KErrNone  )
        {
        iGlobalDlg->ShowDialogL(
            CFileManagerGlobalDlg::EErrorNote,
            R_QTN_CRITICAL_ERROR );
            
        err = iCenRep->Set(
            KFileManagerLastSchBackupStatus,
            EFileManagerSchBackupStatusFailed );
        }
    else
        {
        iGlobalDlg->ShowDialogL(
            CFileManagerGlobalDlg::EInfoNote,
            R_QTN_BACKUP_COMPLETED );

        err = iCenRep->Set(
            KFileManagerLastSchBackupStatus,
            EFileManagerSchBackupStatusOk );
        }

    LOG_IF_ERROR1( err,
        "CFileManagerSchBackupHandler::ProcessFinishedL-err=%d", err )

    InformObserver( aError );

    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::ProcessAdvanceL
// ----------------------------------------------------------------------------
//
void CFileManagerSchBackupHandler::ProcessAdvanceL( TInt aValue )
    {
    iGlobalDlg->UpdateProgressDialog( aValue );
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::ProcessStartedL
// ----------------------------------------------------------------------------
//
void CFileManagerSchBackupHandler::ProcessStartedL( TInt aFinalValue )
    {
    FUNC_LOG
    
    iGlobalDlg->UpdateProgressDialog( 0, aFinalValue );
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::CancelBackupStarter
// ----------------------------------------------------------------------------
//
void CFileManagerSchBackupHandler::CancelBackupStarter()
    {
    TInt err( iCenRep->Set(
        KFileManagerLastSchBackupStatus,
        EFileManagerSchBackupStatusCanceled ) );
    LOG_IF_ERROR1( err,
        "CFileManagerSchBackupHandler::CancelBackupStarter-err=%d", err )
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::IsPowerSavingModeOnL
// ----------------------------------------------------------------------------
//
TBool CFileManagerSchBackupHandler::IsPowerSavingModeOnL()
    {
    TBool ret( EFalse );
#ifdef FILE_MANAGER_POWER_SAVING_MODE
    // Check power saving mode before start
    CRepository* psmCenRep = CRepository::NewLC( KCRUidPowerSaveMode );
    TInt psmMode( 0 );
    User::LeaveIfError( psmCenRep->Get( KPsmCurrentMode, psmMode ) );
    if ( psmMode )
        {
        ERROR_LOG1(
            "CFileManagerSchBackupHandler::IsPowerSavingModeOnL-PsmMode=%d",
            psmMode )
        ret = ETrue;
        }
    CleanupStack::PopAndDestroy( psmCenRep );
#endif // FILE_MANAGER_POWER_SAVING_MODE
    return ret;
    }

// ----------------------------------------------------------------------------
// CFileManagerSchBackupHandler::IsTargetDriveAvailableL
// ----------------------------------------------------------------------------
//
TBool CFileManagerSchBackupHandler::IsTargetDriveAvailableL( const TFileManagerDriveInfo& aDrvInfo )
    {
    // Start backup with cancelable global progress dialog

    // Check is target drive available
    TBool ret( ETrue );
    if ( !( aDrvInfo.iState & TFileManagerDriveInfo::EDrivePresent ) ||
         ( aDrvInfo.iState &
            ( TFileManagerDriveInfo::EDriveLocked |
              TFileManagerDriveInfo::EDriveCorrupted |
              TFileManagerDriveInfo::EDriveWriteProtected ) ) )
        {
        ERROR_LOG1(
            "CFileManagerSchBackupHandler::IsTargetDriveAvailableL-DrvState=%d",
            aDrvInfo.iState )
        ret = EFalse;
        }
    return ret;
    }

//  End of File  
