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
* Description:  Main view
*
*/



// INCLUDE FILES
#include <aknlists.h>
#include <CFileManagerEngine.h>
#include <CFileManagerItemProperties.h>
#include <FileManager.rsg>
#include <csxhelp/fmgr.hlp.hrh>
#include <FileManagerDlgUtils.h>
#include <FileManagerDebug.h>
#include <CFileManagerFeatureManager.h>
#include <coreapplicationuisdomainpskeys.h>
#include <StringLoader.h>
#include <cmemstatepopup.h>
#include "CFileManagerMainView.h"
#include "CFileManagerFileListContainer.h"
#include "CFileManagerAppUi.h"
#include "CFileManagerDocument.h"
#include "FileManager.hrh"


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerMainView::CFileManagerMainView
// ----------------------------------------------------------------------------
//
CFileManagerMainView::CFileManagerMainView()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::NewLC
// ----------------------------------------------------------------------------
//
CFileManagerMainView* CFileManagerMainView::NewLC()
    {
    CFileManagerMainView* self = new( ELeave ) CFileManagerMainView();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::ConstructL
// ----------------------------------------------------------------------------
//
void CFileManagerMainView::ConstructL()
    {
    CFileManagerViewBase::ConstructL(
        FeatureManager().IsEmbedded() ?
            R_FILEMANAGER_MAIN_VIEW_EMBEDDED :
            R_FILEMANAGER_MAIN_VIEW );
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::~CFileManagerMainView
// ----------------------------------------------------------------------------
// 
CFileManagerMainView::~CFileManagerMainView()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::DoActivateL
// ----------------------------------------------------------------------------
// 
void CFileManagerMainView::DoActivateL(
        const TVwsViewId& aPrevViewId,
        TUid aCustomMessageId,
        const TDesC8& aCustomMessage )
    {
    FUNC_LOG

    CFileManagerViewBase::DoActivateL(
        aPrevViewId, aCustomMessageId, aCustomMessage );

    // Ensure that no directory is defined
    if ( iEngine.CurrentDirectory().Length() )
        {
        TInt count( iEngine.NavigationLevel() );
        ++count;
        for ( TInt i( 0 ); i < count; i++ )
            {
            TRAP_IGNORE( iEngine.BackstepL() );
            }
        }

    CFileManagerAppUi* appUi = 
        static_cast< CFileManagerAppUi* >( AppUi() );
    appUi->RestoreDefaultTitleL();
    iEngine.SetState( CFileManagerEngine::ENavigation );
    iEngine.SetObserver( this );
    appUi->ExitEmbeddedAppIfNeededL();
    if ( !appUi->WaitingForInputParams() )
        {
        iEngine.RefreshDirectory();
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::DoDeactivate
// ----------------------------------------------------------------------------
// 
void CFileManagerMainView::DoDeactivate()
    {
    FUNC_LOG

    CFileManagerViewBase::DoDeactivate();
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::DirectoryChangedL
// ----------------------------------------------------------------------------
// 
void CFileManagerMainView::DirectoryChangedL()
    {
    FUNC_LOG

    CFileManagerAppUi* appUi = 
        static_cast< CFileManagerAppUi* >( AppUi() );

    if ( iContainer && !appUi->WaitingForInputParams() )
        {
        iContainer->RefreshListL( iIndex );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::Id
// ----------------------------------------------------------------------------
// 
TUid CFileManagerMainView::Id() const
    {
    return CFileManagerAppUi::KFileManagerMainViewId;
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::CreateContainerL
// ----------------------------------------------------------------------------
// 
CFileManagerContainerBase* CFileManagerMainView::CreateContainerL()
    {
    return CFileManagerFileListContainer::NewL(
        ClientRect(),
        iIndex,
        CFileManagerFileListContainer::EListMain,
        R_QTN_SELEC_EMPTY_LIST,
        KFMGR_HLP_MAIN_VIEW );
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::DynInitMenuPaneL
// ----------------------------------------------------------------------------
// 
void CFileManagerMainView::DynInitMenuPaneL(
        TInt aResourceId, 
        CEikMenuPane* aMenuPane)
    {
    switch( aResourceId )
        {
        // These menus are used only by main view
        case R_FILEMANAGER_MAIN_VIEW_MENU:
            {
            MainMenuFilteringL( *aMenuPane );
            break;
            }
        case R_FILEMANAGER_REMOTE_DRIVES_MENU:
            {
            RemoteDrivesMenuFilteringL( *aMenuPane );
            break;
            }
        case R_FILEMANAGER_MEMORY_STORAGE_MENU:
            {
            MemoryStorageMenuFilteringL( *aMenuPane );
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
// CFileManagerMainView::MainMenuFilteringL
// ----------------------------------------------------------------------------
//
void CFileManagerMainView::MainMenuFilteringL( CEikMenuPane& aMenuPane )
    {
    RemoteDriveCommonFilteringL( aMenuPane );

    if ( !iContainer->ListBoxNumberOfItems() )
        {
        aMenuPane.SetItemDimmed( EFileManagerEject, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerFindFile, ETrue );
#ifndef RD_FILE_MANAGER_BACKUP
        aMenuPane.SetItemDimmed( EFileManagerBackup, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerRestore, ETrue );
#endif // RD_FILE_MANAGER_BACKUP
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorage, ETrue );
        return;
        }

    if ( !FeatureManager().IsRemoteStorageFwSupported() )
        {
        aMenuPane.SetItemDimmed( EFileManagerRemoveDrives, ETrue );
        }

    CFileManagerItemProperties* prop = iEngine.GetItemInfoLC(
        iContainer->ListBoxCurrentItemIndex() );
    TUint32 drvState( 0 );
    if ( prop->IsDrive() )
        {
        iEngine.DriveState( drvState, prop->FullPath() );
        }

    TFileManagerDriveInfo drvInfo;
    if ( DriveInfoAtCurrentPosL( drvInfo ) < 0 )
        {
        // No drive selected
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorage, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerEject, ETrue );
        }
    else
        {
        if ( !( drvInfo.iState & TFileManagerDriveInfo::EDriveEjectable ) ||
             !( drvInfo.iState & TFileManagerDriveInfo::EDrivePresent ) )
            {
            aMenuPane.SetItemDimmed( EFileManagerEject, ETrue );
            }
        if ( ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemovable ) && 
            !( drvInfo.iState & TFileManagerDriveInfo::EDrivePresent ) )
            {
            aMenuPane.SetItemDimmed( EFileManagerMemoryStorage, ETrue );
            }
        if ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote )
            {
            aMenuPane.SetItemDimmed( EFileManagerMemoryStorage, ETrue );
            }
        }

#ifndef RD_FILE_MANAGER_BACKUP
    TFileManagerDriveInfo mmcinfo( iEngine.GetMMCInfoL() );
    if ( mmcinfo.iState & ( TFileManagerDriveInfo::EDriveLocked |
                            TFileManagerDriveInfo::EDriveCorrupted ) )
        {
        aMenuPane.SetItemDimmed( EFileManagerBackup, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerRestore, ETrue );
        }
    else if ( !( mmcinfo.iState & TFileManagerDriveInfo::EDrivePresent ) )
        {
        aMenuPane.SetItemDimmed( EFileManagerBackup, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerRestore, ETrue );
        }
    else if ( !( mmcinfo.iState & TFileManagerDriveInfo::EDriveBackupped ) )
        {
        aMenuPane.SetItemDimmed( EFileManagerRestore, ETrue );
        }
    if ( FeatureManager().IsEmbedded() )
        {
        // Disable backup in embedded mode, because it messes up 
        // backup and restore operations since embedded apps are closed.
        aMenuPane.SetItemDimmed( EFileManagerBackup, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerRestore, ETrue );
        }
#endif // RD_FILE_MANAGER_BACKUP

    CleanupStack::PopAndDestroy( prop );
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::RemoteDrivesMenuFilteringL
// ----------------------------------------------------------------------------
//
void CFileManagerMainView::RemoteDrivesMenuFilteringL(
        CEikMenuPane& aMenuPane )
    {
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    CFileManagerItemProperties* prop = iEngine.GetItemInfoLC( index );
    TUint32 drvState( 0 );
    if ( iEngine.DriveState( drvState, prop->FullPath() ) == KErrNone )
        {
        if ( !( drvState & TFileManagerDriveInfo::EDriveRemote ) ||
             ( drvState & TFileManagerDriveInfo::EDriveConnected ) )
            {
            aMenuPane.SetItemDimmed(
                EFileManagerRemoveDrivesSettings, ETrue );
            aMenuPane.SetItemDimmed(
                EFileManagerRemoveDrivesDelete, ETrue );
            }
        }
    else
        {
        aMenuPane.SetItemDimmed( EFileManagerRemoveDrivesSettings, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerRemoveDrivesDelete, ETrue );
        }
    CleanupStack::PopAndDestroy( prop );
	TBool dimAll( EFalse );
	if ( !FeatureManager().IsRemoteStorageFwSupported() )
		{
		dimAll = ETrue;
		}
	else
		{
		if ( drvState & TFileManagerDriveInfo::EDriveRemote )
		    {
			if ( drvState & TFileManagerDriveInfo::EDriveConnected )
			    {
				aMenuPane.SetItemDimmed( EFileManagerConnectRemoveDrive, ETrue );
				}
			else
				{
				aMenuPane.SetItemDimmed( EFileManagerDisconnectRemoveDrive,ETrue );
				}
			}
		else
			{
			dimAll = ETrue;
			}
		}
		
	if ( dimAll )
        {
	    aMenuPane.SetItemDimmed( EFileManagerConnectRemoveDrive, ETrue );
	    aMenuPane.SetItemDimmed( EFileManagerDisconnectRemoveDrive, ETrue );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::HandleCommandL
// ----------------------------------------------------------------------------
// 
void CFileManagerMainView::HandleCommandL( TInt aCommand )
    {
    switch( aCommand )
        {
        case EFileManagerOpen:
            {
            CmdOpenMemoryStoreL();
            break;
            }
        case EFileManagerEject:
            {
            CmdEjectL();
            break;
            }
#ifndef RD_FILE_MANAGER_BACKUP
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
#endif // RD_FILE_MANAGER_BACKUP
        case EFileManagerRemoveDrivesMapDrive:
            {
            CmdMapRemoteDriveL();
            break;
            }
        case EFileManagerRemoveDrivesSettings:
            {
            CmdRemoteDriveSettingsL();
            break;
            }
        case EFileManagerDelete: // Fall through
        case EFileManagerRemoveDrivesDelete:
            {
            CmdRemoteDriveDeleteL();
            break;
            }
        case EFileManagerSend: // Suppress
            {
            break;
            }
        case EFileManagerMemoryStorageDetails:
            {
            CmdMemoryStorageDetailsL();
            break;
            }
        case EFileManagerMemoryStorageName:
        case EFileManagerMemoryStorageRename: // Fall through
            {
            CmdRenameDriveL();
            break;
            }
        case EFileManagerMemoryStorageFormat:
            {
            CmdFormatDriveL();
            break;
            }
        case EFileManagerMemoryStorageSetPassword:
            {
            CmdSetDrivePasswordL();
            break;
            }
        case EFileManagerMemoryStorageChangePassword:
            {
            CmdChangeDrivePasswordL();
            break;
            }
        case EFileManagerMemoryStorageRemovePassword:
            {
            CmdRemoveDrivePasswordL();
            break;
            }
        case EFileManagerMemoryStorageUnlock:
            {
            CmdUnlockDriveL();
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
// CFileManagerMainView::CmdOpenMemoryStoreL
// ----------------------------------------------------------------------------
// 
void CFileManagerMainView::CmdOpenMemoryStoreL()
    {
    StoreIndex();
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    if ( index >= 0 )
        {
        CFileManagerAppUi* appUi = 
            static_cast< CFileManagerAppUi* >( AppUi() );
        CFileManagerItemProperties* prop = iEngine.GetItemInfoLC( index );

        TRAPD( err, iEngine.OpenL( index ) );
        if ( err == KErrNone && prop->FullPath().Length() )
            {
            appUi->ActivateMemoryStoreViewL();
            }
        CleanupStack::PopAndDestroy( prop );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::CmdEjectL
// ----------------------------------------------------------------------------
//
void CFileManagerMainView::CmdEjectL()
    {
    StoreIndex();

#ifdef RD_MULTIPLE_DRIVE
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    CFileManagerItemProperties* prop = iEngine.GetItemInfoLC( index );
    TInt drive( prop->DriveId() );
    const TInt KDriveShift = 16;

    // Let SysAp handle eject
    RProperty::Set(
        KPSUidCoreApplicationUIs,
        KCoreAppUIsMmcRemovedWithoutEject,
        ECoreAppUIsEjectCommandUsedToDrive | ( drive << KDriveShift )
        );

    CleanupStack::PopAndDestroy( prop );
#else // RD_MULTIPLE_DRIVE
    if ( FileManagerDlgUtils::ShowConfirmQueryWithYesNoL( R_QTN_CONF_EJECT ) )
        {
        StartProcessL( EEjectProcess );
        }
#endif // RD_MULTIPLE_DRIVE
    }

#ifndef RD_FILE_MANAGER_BACKUP
// ----------------------------------------------------------------------------
// CFileManagerMainView::CmdBackupL
//
// ----------------------------------------------------------------------------
//
void CFileManagerMainView::CmdBackupL()
    {
    StoreIndex();
    TInt textId( R_QTN_CONFIRM_BACKUP_TEXT );
    TFileManagerDriveInfo mmcinfo( iEngine.GetMMCInfoL() );

    if( mmcinfo.iState & TFileManagerDriveInfo::EDriveBackupped )
        {
        textId = R_QTN_CONFIRM_BACKUP_TEXT2;
        }
    if( FileManagerDlgUtils::ShowConfirmQueryWithYesNoL( textId ) )
        {
        if ( !DriveReadOnlyMmcL( mmcinfo.iDrive ) )
            {
            if ( !IsDriveAvailable( mmcinfo.iDrive ) || 
                !CheckPhoneState() )
                {
                FileManagerDlgUtils::ShowErrorNoteL( R_QTN_CRITICAL_ERROR );
                }
            else
                {
                StartProcessL( EBackupProcess );
                }
            }
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::CmdRestoreL
// ----------------------------------------------------------------------------
//
void CFileManagerMainView::CmdRestoreL()
    {
    StoreIndex();
    if( FileManagerDlgUtils::ShowConfirmQueryWithYesNoL( R_QTN_CONFIRM_RESTORE_TEXT ) )
        {
        TFileManagerDriveInfo mmcinfo( iEngine.GetMMCInfoL() );
        
        if ( !IsDriveAvailable( mmcinfo.iDrive ) || 
            !CheckPhoneState() )
            {
            FileManagerDlgUtils::ShowErrorNoteL( R_QTN_CRITICAL_ERROR );
            }
        else
            {
            StartProcessL( ERestoreProcess );
            }
        }
    }
#endif // RD_FILE_MANAGER_BACKUP

// ----------------------------------------------------------------------------
// CFileManagerMainView::CmdMapRemoteDriveL
// ----------------------------------------------------------------------------
//
void CFileManagerMainView::CmdMapRemoteDriveL()
    {
    StoreIndex();
    const TInt KMaxRemoteDrives = 9;
    RFs& fs( CCoeEnv::Static()->FsSession() );
    TDriveList driveList;
    User::LeaveIfError( fs.DriveList( driveList, KDriveAttRemote ) );
    TInt numRemote( 0 );
    TInt count( driveList.Length() );
    for( TInt i( 0 ); i < count; ++i )
        {
        if ( driveList[ i ] & KDriveAttRemote )
            {
            ++numRemote;
            }
        }
    if ( numRemote < KMaxRemoteDrives )
        {
        OpenRemoteDriveSettingsL();
        }
    else
        {
        FileManagerDlgUtils::ShowConfirmQueryWithOkL(
            FileManagerDlgUtils::EErrorIcons,
            R_QTN_RD_ERROR_MAX_DRIVES );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerMainView::CmdRemoteDriveSettingsL
// ------------------------------------------------------------------------------
//
void CFileManagerMainView::CmdRemoteDriveSettingsL()
    {
    StoreIndex();
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    CFileManagerItemProperties* prop = iEngine.GetItemInfoLC( index );
    OpenRemoteDriveSettingsL( prop->DriveName() );
    CleanupStack::PopAndDestroy( prop );
    }

// ------------------------------------------------------------------------------
// CFileManagerMainView::CmdRemoteDriveDeleteL
//
// ------------------------------------------------------------------------------
//
void CFileManagerMainView::CmdRemoteDriveDeleteL()
    {
    StoreIndex();
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    CFileManagerItemProperties* prop = iEngine.GetItemInfoLC( index );
    if ( IsDisconnectedRemoteDrive( *prop ) )
        {
        if ( FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
                R_QTN_RD_QUERY_DELETE_DRIVE, prop->Name() ) )
            {
            TInt drv = TDriveUnit( prop->FullPath() );
            User::LeaveIfError( iEngine.DeleteRemoteDrive( drv ) );
            }
        }
    CleanupStack::PopAndDestroy( prop );
    }

// ------------------------------------------------------------------------------
// CFileManagerMainView::NotifyL
//
// ------------------------------------------------------------------------------
//
TInt CFileManagerMainView::NotifyL(
        TFileManagerNotify aType, TInt aData, const TDesC& aName )
    {
    TInt ret( KErrNone );
    switch ( aType )
        {
#ifdef RD_FILE_MANAGER_BACKUP
        case ENotifyActionSelected:
            {
            if ( aData == EFileManagerBackupAction )
                {
                StoreIndex();
                CFileManagerAppUi* appUi = 
                    static_cast< CFileManagerAppUi* >( AppUi() );
                appUi->ActivateBackupViewL();
                }
            break;
            }
#endif // RD_FILE_MANAGER_BACKUP
        default:
            {
            ret = CFileManagerViewBase::NotifyL( aType, aData, aName );
            break;
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerMainView::MemoryStorageMenuFilteringL
// -----------------------------------------------------------------------------
// 
void CFileManagerMainView::MemoryStorageMenuFilteringL( CEikMenuPane& aMenuPane )
    {
    TFileManagerDriveInfo drvInfo;
    if( DriveInfoAtCurrentPosL( drvInfo ) < 0 )
        {
        // No drive selected
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageUnlock, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageDetails, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageFormat, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageName, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageRename, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageSetPassword, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageChangePassword, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageRemovePassword, ETrue );
        return;
        }

    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveCorrupted )
        {
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageDetails, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageName, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageRename, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageSetPassword, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageChangePassword, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageRemovePassword, ETrue );
        }
    else if ( !( drvInfo.iState & TFileManagerDriveInfo::EDriveRemovable ) || 
              ( drvInfo.iState & ( TFileManagerDriveInfo::EDriveLocked |
                                   TFileManagerDriveInfo::EDriveMassStorage ) ) )
        {
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageName, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageRename, ETrue );
        }
    else if ( drvInfo.iName.Length() > 0 )
        {
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageName, ETrue );
        }
    else
        {
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageRename, ETrue );
        }
    if (!( drvInfo.iState & TFileManagerDriveInfo::EDrivePresent) )
    	{
			aMenuPane.SetItemDimmed(EFileManagerMemoryStorageSetPassword, ETrue );
			aMenuPane.SetItemDimmed(EFileManagerMemoryStorageName, ETrue );
			aMenuPane.SetItemDimmed(EFileManagerMemoryStorageRename, ETrue );
			aMenuPane.SetItemDimmed(EFileManagerMemoryStorageDetails, ETrue );
			
    	}

    if ( FeatureManager().IsEmbedded() ||
         !( drvInfo.iState & TFileManagerDriveInfo::EDriveRemovable ) ||
         !( drvInfo.iState & TFileManagerDriveInfo::EDriveFormattable ) )
        {
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageFormat, ETrue );
        }
        
    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveLocked )
        {
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageDetails, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageSetPassword, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageChangePassword, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageRemovePassword, ETrue );
        }
    else
        {
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageUnlock, ETrue );
        }

    if ( !( drvInfo.iState & TFileManagerDriveInfo::EDriveRemovable ) ||
         ( drvInfo.iState & ( TFileManagerDriveInfo::EDriveMassStorage |
                              TFileManagerDriveInfo::EDriveUsbMemory ) ) )
        {
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageSetPassword, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageChangePassword, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageRemovePassword, ETrue );
        }
    else if ( drvInfo.iState & TFileManagerDriveInfo::EDrivePasswordProtected ) 
        {
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageSetPassword, ETrue );
        }
    else
        {
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageChangePassword, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageRemovePassword, ETrue );
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerMainView::CmdMemoryStorageDetailsL
// ------------------------------------------------------------------------------
//
void CFileManagerMainView::CmdMemoryStorageDetailsL()
    {
    StoreIndex();
    TFileManagerDriveInfo drvInfo;
    TInt drive( DriveInfoAtCurrentPosL( drvInfo ) );
    if ( drive < 0 )
        {
        return; // No drive selected
        }
    HBufC* title = StringLoader::LoadLC( R_QTN_FMGR_MSTATE_HEADING );
    CMemStatePopup::RunLD(
        static_cast< TDriveNumber >( drive ), *title );
    CleanupStack::PopAndDestroy( title );
//    TFileManagerDriveInfo drvInfo;
//    iEngine.GetDriveInfoL( iEngine.CurrentDrive(), drvInfo );
//    FileManagerDlgUtils::ShowMemoryStoreInfoPopupL( drvInfo );
    }

// ------------------------------------------------------------------------------
// CFileManagerMainView::CmdRenameDriveL
// ------------------------------------------------------------------------------
//
void CFileManagerMainView::CmdRenameDriveL()
    {
    StoreIndex();
    TFileManagerDriveInfo drvInfo;
    if ( DriveInfoAtCurrentPosL( drvInfo ) < 0 )
        {
        return; // No drive selected
        }
    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveWriteProtected )
        {
        FileManagerDlgUtils::ShowErrorNoteL( R_QTN_MEMORYCARD_READONLY );
        return;
        }
    RenameDriveL( EFalse );
    iEngine.SetObserver( this );
    iEngine.RefreshDirectory();
    }

// ------------------------------------------------------------------------------
// CFileManagerMainView::CmdSetDrivePasswordL
// ------------------------------------------------------------------------------
//
void CFileManagerMainView::CmdSetDrivePasswordL()
    {
    StoreIndex();
    TInt drive( DriveAtCurrentPosL() );
    if ( drive < 0 )
        {
        return; // No drive selected
        }

    TBuf< KFmgrMaxMediaPassword > pwd;
    if( FileManagerDlgUtils::ShowPasswordQueryL( pwd ) )
        {
        TBuf< KFmgrMaxMediaPassword > nullPwd;
        EmptyPwd( nullPwd );
        if( UpdatePassword( drive, nullPwd, pwd ) == KErrNone )
            {
            FileManagerDlgUtils::ShowConfirmNoteL( R_QTN_PASSWORD_SET_TEXT );
            }
        else
            {
            FileManagerDlgUtils::ShowErrorNoteL( R_QTN_CRITICAL_ERROR );
            }
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerMainView::CmdChangeDrivePasswordL
// ------------------------------------------------------------------------------
//
void CFileManagerMainView::CmdChangeDrivePasswordL()
    {
    StoreIndex();
    TInt drive( DriveAtCurrentPosL() );
    if ( drive < 0 )
        {
        return; // No drive selected
        }

    TBuf< KFmgrMaxMediaPassword > pwd;
    TBuf< KFmgrMaxMediaPassword > oldPwd;
    TBool isDone( EFalse );
    TBool isCanceled( EFalse );
    TInt err( KErrNone );

    // Ask for the old password until the correct one is given
    while( !isDone )
        {
        EmptyPwd( oldPwd );
        if( FileManagerDlgUtils::ShowSimplePasswordQueryL(
                R_QTN_PASSWORD_OLD_TEXT, oldPwd ) )
			{
			TInt index(iContainer->ListBoxCurrentItemIndex());
						CFileManagerItemProperties* prop = iEngine.GetItemInfoLC(index);
						TUint32 drvState(0);
						TInt error= iEngine.DriveState(drvState, prop->FullPath());
						if ((error!=KErrNone)||(!(drvState & TFileManagerDriveInfo::EDrivePresent)))
							{
							isDone = ETrue;
							isCanceled = ETrue;
							FileManagerDlgUtils::ShowErrorNoteL(R_QTN_MEMC_NOT_AVAILABLE );
							}
						else
            {
            err = UpdatePassword( drive, oldPwd, oldPwd );
            if( err == KErrNone )
                {
                isDone = ETrue;
                }
            else
                {
                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_PASSWORDS_WRONG_TEXT );
                }
            }
						CleanupStack::PopAndDestroy( prop );
						}
        else
            {
            isDone = ETrue;
            isCanceled = ETrue;
            }
        }

    // Then query for the new password
    if( !isCanceled )
        {
        if( FileManagerDlgUtils::ShowPasswordQueryL( pwd ) )
            {
            err = UpdatePassword( drive, oldPwd, pwd );
            if( err == KErrNone )
                {
                FileManagerDlgUtils::ShowConfirmNoteL(
                    R_QTN_PASSWORD_CHANGED_TEXT );
                }
            else
                {
                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_CRITICAL_ERROR );
                }
            }
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerMainView::CmdRemoveDrivePasswordL
// ------------------------------------------------------------------------------
//
void CFileManagerMainView::CmdRemoveDrivePasswordL()
    {
    StoreIndex();
    TInt drive( DriveAtCurrentPosL() );
    if ( drive < 0 )
        {
        return; // No drive selected
        }

    if( !UnlockRemovePasswordL( drive, ETrue ) )
        {
        FileManagerDlgUtils::ShowConfirmNoteL( R_QTN_PASSWORD_REMOVED_TEXT );
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerMainView::DriveAtCurrentPosL
// ------------------------------------------------------------------------------
//
TInt CFileManagerMainView::DriveAtCurrentPosL()
    {
    TFileManagerDriveInfo dummy;
    return DriveInfoAtCurrentPosL( dummy );
    }

//  End of File  
