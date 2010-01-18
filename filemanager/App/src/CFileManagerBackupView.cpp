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
* Description:  View for backup settings
*
*/


// INCLUDE FILES
#include <StringLoader.h>
#include <aknlists.h>
#include <akntitle.h>
#include <csxhelp/fmgr.hlp.hrh>
#include <CFileManagerEngine.h>
#include <FileManager.rsg>
#include <CFileManagerBackupSettings.h>
#include <FileManagerPrivateCRKeys.h>
#include <FileManagerDlgUtils.h>
#include <CAknMemorySelectionDialog.h>
#include <CAknMemorySelectionSettingPage.h>
#ifdef RD_MULTIPLE_DRIVE
#include <AknCommonDialogsDynMem.h>
#include <CAknMemorySelectionDialogMultiDrive.h>
#endif // RD_MULTIPLE_DRIVE
#include <FileManagerDebug.h>
#include "CFileManagerBackupView.h"
#include "CFileManagerTaskScheduler.h"
#include "CFileManagerSettingListContainer.h"
#include "CFileManagerAppUi.h"
#include "FileManager.hrh"


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerBackupView::CFileManagerBackupView
// ----------------------------------------------------------------------------
//
CFileManagerBackupView::CFileManagerBackupView()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::NewLC
// ----------------------------------------------------------------------------
//
CFileManagerBackupView* CFileManagerBackupView::NewLC()
    {
    CFileManagerBackupView* self = new( ELeave ) CFileManagerBackupView();
    CleanupStack::PushL( self );
    self->ConstructL( R_FILEMANAGER_BACKUP_VIEW );
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::~CFileManagerBackupView
// ----------------------------------------------------------------------------
// 
CFileManagerBackupView::~CFileManagerBackupView()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::DoActivateL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::DoActivateL(
        const TVwsViewId& aPrevViewId,
        TUid aCustomMessageId,
        const TDesC8& aCustomMessage )
    {
    FUNC_LOG

    CFileManagerViewBase::DoActivateL(
        aPrevViewId, aCustomMessageId, aCustomMessage );

    static_cast< CFileManagerAppUi* >( AppUi() )->SetTitleL(
        R_QTN_FMGR_BACKUP_TITLE );

    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
    settings.RefreshL();

    iContainer->RefreshListL( iIndex );
    }

// -----------------------------------------------------------------------------
// CFileManagerBackupView::DoDeactivate
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::DoDeactivate()
    {
    FUNC_LOG

    CFileManagerViewBase::DoDeactivate();
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::DirectoryChangedL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::DirectoryChangedL()
    {
    FUNC_LOG

    if ( iContainer )
        {
        iContainer->RefreshListL( iIndex );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::Id
// ----------------------------------------------------------------------------
// 
TUid CFileManagerBackupView::Id() const
    {
    return CFileManagerAppUi::KFileManagerBackupViewId;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::CreateContainerL
// ----------------------------------------------------------------------------
// 
CFileManagerContainerBase* CFileManagerBackupView::CreateContainerL()
    {
    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );

    return CFileManagerSettingListContainer::NewL(
        ClientRect(),
        iIndex,
        CFileManagerSettingListContainer::EListBackup,
        &settings,
        KFMGR_HLP_BACKUP_VIEW );
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::HandleCommandL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::HandleCommandL( TInt aCommand )
    {
    switch( aCommand )
        {
        case EFileManagerSelectionKey: // FALLTHROUGH
        case EFileManagerOpen:
            {
            CmdChangeL();
            break;
            }
        case EFileManagerBackup:
            {
            CmdBackupL();
            break;
            }
        case EFileManagerRestore:
            {
            CmdRestoreL();
            break;
            }
        case EFileManagerDeleteBackup:
            {
            CmdDeleteBackupL();
            break;
            }
        case EAknSoftkeyBack:
            {
            CmdBackL();
            break;
            }
        default:
            {
            CFileManagerViewBase::HandleCommandL( aCommand );
            break;
            }
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::CmdChangeL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::CmdChangeL()
    {
    if ( !iContainer )
        {
        return;
        }
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
    
    StoreIndex();

    switch ( settings.SettingAt( index ) )
        {
        case CFileManagerBackupSettings::EContents:
            {
            ChangeContentsL();
            break;
            }
        case CFileManagerBackupSettings::EScheduling:
            {
            ChangeSchedulingL();
            break;
            }
        case CFileManagerBackupSettings::EWeekday:
            {
            ChangeWeekdayL();
            break;
            }
        case CFileManagerBackupSettings::ETime:
            {
            ChangeTimeL();
            break;
            }
        case CFileManagerBackupSettings::ETarget:
            {
            ChangeTargetDriveL();
            break;
            }
        default:
            {
            break;
            }
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::CmdBackupL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::CmdBackupL()
    {
    if ( !iContainer )
        {
        return;
        }
    StoreIndex();
    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
    TInt drive( settings.TargetDrive() );
    TInt MemoryCardDriveF( KFmgrMemoryCardDrive + 1 );
    //KFmgrMemoryCardDrive has been changed as Mass memory,MemoryCard is changed to F
    
    if( drive != MemoryCardDriveF && !IsDriveAvailable( drive ) )
    	{
    	drive = MemoryCardDriveF;
    	settings.SetTargetDrive( drive );
    	settings.SaveL();    	
    	}
    TFileManagerDriveInfo drvInfo;
    iEngine.GetDriveInfoL( drive, drvInfo );
    RArray< CFileManagerRestoreSettings::TInfo > info;
    CleanupClosePushL( info );
    iEngine.GetRestoreInfoArrayL( info, drive );
    TInt count( info.Count() );
    CleanupStack::PopAndDestroy( &info );
    TPtrC driveName( iEngine.DriveName( drive ) );
    TBool isContinue( true );
    
	
    const TInt64 KBackupLimitSize = 10000000;
	
    //define a protected limit size to avoid not enough space during backup
	
    if( count )
        {
        TInt textId( R_QTN_FMGR_BACKUP_QUERY_EXISTS );
       
        if( !FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
               textId, driveName ) )
            {
            isContinue = false;
            }
        }
    if( isContinue ) 
    	{
		if ( !DriveReadOnlyMmcL( drive ) )
            {
            if ( !IsDriveAvailable( drive ) )
                {
                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_FMGR_ERROR_BACKUP_MEMORYCARD_NOT_AVAILABLE );
                }
            else if ( !CheckPhoneState() )
                {
                FileManagerDlgUtils::ShowErrorNoteL( R_QTN_CRITICAL_ERROR );
                }
            else if ( !iEngine.EnoughSpaceL(
                    TDriveUnit( drive ).Name(), KBackupLimitSize, EBackupProcess ) ) 
                {
                FileManagerDlgUtils::ShowConfirmQueryWithOkL(
                    FileManagerDlgUtils::EErrorIcons,
                    R_QTN_FMGR_BACKUP_DESTINATION_FULL,
                    driveName );
                }
            else
                {
                StartProcessL( EBackupProcess );
                }
            }
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::CmdRestoreL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::CmdRestoreL()
    {
    StoreIndex();
    static_cast< CFileManagerAppUi* >( AppUi() )->ActivateRestoreViewL();
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::ChangeContentsL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::ChangeContentsL()
    {
    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
    TUint32 selection( settings.Content() );

    if ( FileManagerDlgUtils::ShowNOfMSettingQueryL(
           R_QTN_FMGR_BACKUP_CONTENTS,
           R_FILEMANAGER_BACKUP_CONTENTS_TEXTS,
           selection,
           0 ) )
        {
        settings.SetContent( selection );
        settings.SaveL();
        iContainer->RefreshListL( iIndex );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::ChangeSchedulingL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::ChangeSchedulingL()
    {
    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
    TInt index( settings.Scheduling() );

    if ( FileManagerDlgUtils::ShowOOfMSettingQueryL(
           R_QTN_FMGR_BACKUP_SCHEDULING,
           R_FILEMANAGER_BACKUP_SCHEDULING_TEXTS,
           index ) )
        {
        settings.SetScheduling( index );
        settings.SaveL();
        iContainer->RefreshListL( iIndex );
        UpdateScheduleL();
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::ChangeWeekdayL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::ChangeWeekdayL()
    {
    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
    TDay day( static_cast< TDay >( settings.Day() ) );
    
    if ( FileManagerDlgUtils::ShowWeekdayQueryL(
            R_QTN_FMGR_BACKUP_WEEKDAY,
            day ) )
        {
        settings.SetDay( day );
        settings.SaveL();
        iContainer->RefreshListL( iIndex );
        UpdateScheduleL();
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::ChangeTargetDriveL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::ChangeTargetDriveL()
    {
#ifdef RD_MULTIPLE_DRIVE
    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );

    HBufC* title = StringLoader::LoadLC(
        R_QTN_FMGR_SELECT_BACKUP_DESTINATION );

    TUint32 drvMask( settings.AllowedDriveAttMatchMask() );
    TInt memType( AknCommonDialogsDynMem::EMemoryTypeMMCExternal );
    if ( drvMask & KDriveAttRemote )
        {
        memType |= AknCommonDialogsDynMem::EMemoryTypeRemote;
        }

    CAknMemorySelectionDialogMultiDrive* dlg =
        CAknMemorySelectionDialogMultiDrive::NewL(
            ECFDDialogTypeNormal,
            R_FILEMANAGER_FIND_MEMORY_SELECTIONDIALOG,
            ETrue,
            memType );
    CleanupStack::PushL( dlg );
    dlg->SetTitleL( *title );
    TDriveNumber driveNumber( static_cast< TDriveNumber >(
        settings.TargetDrive() ) );
    TBool ret( dlg->ExecuteL( driveNumber ) );
    CleanupStack::PopAndDestroy( dlg );
    CleanupStack::PopAndDestroy( title );
    if ( ret )
        {
        TInt drive( static_cast< TInt >( driveNumber ) );
        if ( drive != settings.TargetDrive() )
            {
            settings.SetTargetDrive( drive );
            settings.SaveL();
            iContainer->RefreshListL( iIndex );
            }
        }
#endif // RD_MULTIPLE_DRIVE
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::ChangeTimeL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::ChangeTimeL()
    {
    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
    TTime time( settings.Time() );

    if ( FileManagerDlgUtils::ShowTimeSettingQueryL(
           R_QTN_FMGR_BACKUP_TIME,
           time ) )
        {
        settings.SetTime( time );
        settings.SaveL();
        iContainer->RefreshListL( iIndex );
        UpdateScheduleL();
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::CmdBackL
// ----------------------------------------------------------------------------
//
void CFileManagerBackupView::CmdBackL()
    {
    iIndex = 0;
    static_cast< CFileManagerAppUi* >( AppUi() )->CloseBackupViewL();
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::UpdateScheduleL
// ----------------------------------------------------------------------------
//
void CFileManagerBackupView::UpdateScheduleL()
    {
    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
    CFileManagerAppUi* appUi =
        static_cast< CFileManagerAppUi* >( AppUi() );
    CFileManagerTaskScheduler& scheduler( appUi->TaskSchedulerL() );

    if ( settings.Scheduling() == EFileManagerBackupScheduleNone )
        {
        scheduler.EnableBackupScheduleL( EFalse );
        }
    else
        {
        scheduler.EnableBackupScheduleL( ETrue );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::NotifyForegroundStatusChange
// ----------------------------------------------------------------------------
//
void CFileManagerBackupView::NotifyForegroundStatusChange(
        TBool aForeground )
    {
    FUNC_LOG

    if ( aForeground )
        {
        TRAP_IGNORE( RefreshSettingsL() );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::RefreshSettingsL
// ----------------------------------------------------------------------------
//
void CFileManagerBackupView::RefreshSettingsL()
    {
    FUNC_LOG

    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
    settings.RefreshL();
    if ( iContainer )
        {
        iContainer->RefreshListL( iIndex );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::CmdDeleteBackupL
// ----------------------------------------------------------------------------
//
void CFileManagerBackupView::CmdDeleteBackupL()
    {
    StoreIndex();
    static_cast< CFileManagerAppUi* >( AppUi() )->ActivateRestoreViewL( ETrue );
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::DynInitMenuPaneL
// ----------------------------------------------------------------------------
// 
void CFileManagerBackupView::DynInitMenuPaneL(
        TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    switch( aResourceId )
        {
        case R_FILEMANAGER_BACKUP_VIEW_MENU:
            {
            BackupMenuFilteringL( *aMenuPane );
            break;
            }
        default:
            {
            CFileManagerViewBase::DynInitMenuPaneL( aResourceId, aMenuPane );
            break;
            }
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupView::BackupMenuFilteringL
// ----------------------------------------------------------------------------
//
void CFileManagerBackupView::BackupMenuFilteringL( CEikMenuPane& aMenuPane )
    {
    CFileManagerRestoreSettings& settings( iEngine.RestoreSettingsL() );
    TRAP_IGNORE( settings.RefreshL() );
    if ( !settings.MdcaCount() )
        {
        aMenuPane.SetItemDimmed( EFileManagerDeleteBackup, ETrue );
        }
        aMenuPane.SetItemDimmed( EFileManagerOpen, ETrue );
    }

//  End of File  
