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
* Description:  View for restore settings
*
*/



// INCLUDE FILES
#include <StringLoader.h>
#include <aknlists.h>
#include <akntitle.h>
#include <CFileManagerEngine.h>
#include <filemanager.rsg>
#include <FileManagerDlgUtils.h>
#include <CFileManagerRestoreSettings.h>
#include <FileManagerDebug.h>
#include "CFileManagerRestoreView.h"
#include "CFileManagerSettingListContainer.h"
#include "CFileManagerDocument.h"
#include "CFileManagerAppUi.h"
#include "FileManager.hrh"

// CONSTANTS
const TUid CFileManagerRestoreView::KDeleteBackups = { EDeleteBackups };
const TInt KMaxSelection = 64;

// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::CFileManagerRestoreView
// ----------------------------------------------------------------------------
//
CFileManagerRestoreView::CFileManagerRestoreView()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::NewLC
// ----------------------------------------------------------------------------
//
CFileManagerRestoreView* CFileManagerRestoreView::NewLC()
    {
    CFileManagerRestoreView* self = new( ELeave ) CFileManagerRestoreView();
    CleanupStack::PushL( self );
    self->ConstructL( R_FILEMANAGER_RESTORE_VIEW );
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::~CFileManagerRestoreView
// ----------------------------------------------------------------------------
// 
CFileManagerRestoreView::~CFileManagerRestoreView()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::DoActivateL
// ----------------------------------------------------------------------------
// 
void CFileManagerRestoreView::DoActivateL(
        const TVwsViewId& aPrevViewId,
        TUid aCustomMessageId,
        const TDesC8& aCustomMessage )
    {
    FUNC_LOG

    iSelection = 0; // Reset stored list selection
    iDeleteBackups = ( aCustomMessageId == KDeleteBackups );

    CFileManagerViewBase::DoActivateL(
        aPrevViewId, aCustomMessageId, aCustomMessage );

    static_cast< CFileManagerAppUi* >( AppUi() )->SetTitleL(
        iDeleteBackups ?
        R_QTN_FMGR_DELETE_BACKUP_TITLE : R_QTN_FMGR_RESTORE_TITLE );

    CFileManagerRestoreSettings& settings( iEngine.RestoreSettingsL() );
    settings.RefreshL();

    iContainer->RefreshListL( iIndex );

    CEikButtonGroupContainer* cba = Cba();
    cba->SetCommandSetL(R_AVKON_SOFTKEYS_OK_CANCEL__MARK );
    cba->MakeCommandVisible( EAknSoftkeyOk, EFalse );
    cba->MakeCommandVisible( EAknSoftkeyMark, ETrue );

    if ( !iContainer->ListBoxNumberOfItems() )
        {
        cba->MakeCommandVisible( EAknSoftkeyMark, EFalse );
        }
    cba->DrawDeferred();
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::DoDeactivate
// ----------------------------------------------------------------------------
// 
void CFileManagerRestoreView::DoDeactivate()
    {
    FUNC_LOG

    CFileManagerViewBase::DoDeactivate();
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::DirectoryChangedL
// ----------------------------------------------------------------------------
// 
void CFileManagerRestoreView::DirectoryChangedL()
    {
    FUNC_LOG

    if ( iContainer )
        {
        CFileManagerRestoreSettings& settings( iEngine.RestoreSettingsL() );
        settings.RefreshL();
        iContainer->RefreshListL( iIndex );
        RestoreSelectionL();
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::Id
// ----------------------------------------------------------------------------
// 
TUid CFileManagerRestoreView::Id() const
    {
    return CFileManagerAppUi::KFileManagerRestoreViewId;
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::CreateContainerL
// ----------------------------------------------------------------------------
// 
CFileManagerContainerBase* CFileManagerRestoreView::CreateContainerL()
    {
    CFileManagerRestoreSettings& settings( iEngine.RestoreSettingsL() );
    CFileManagerContainerBase* container =
        CFileManagerSettingListContainer::NewL(
            ClientRect(),
            iIndex,
            CFileManagerSettingListContainer::EListRestore,
            &settings );
    container->SetEmptyTextL( R_QTN_FMGR_RESTORE_NO_BACKUPS );
    return container;
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::HandleCommandL
// ----------------------------------------------------------------------------
// 
void CFileManagerRestoreView::HandleCommandL( TInt aCommand )
    {
    switch( aCommand )
        {
        case EAknSoftkeySelect: // FALLTHROUGH
        case EAknSoftkeyMark: // FALLTHROUGH
        case EAknSoftkeyUnmark: // FALLTHROUGH
        case EFileManagerSelectionKey: // FALLTHROUGH
        case EFileManagerToggleMark: // FALLTHROUGH
        case EFileManagerOpen:
            {
            CmdSelectL();
            break;
            }
        case EAknSoftkeyOk:
            {
            CmdOkL();
            break;
            }
        case EAknSoftkeyCancel: // FALLTHROUGH
        case EAknSoftkeyBack:
            {
            CmdCancelL();
            break;
            }
        case EFileManagerCheckMark:
            {
            CmdCheckMarkL();
            break;
            }
        default:
            {
            break;
            }
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::CmdOkL
// ----------------------------------------------------------------------------
// 
void CFileManagerRestoreView::CmdOkL()
    {
    if ( !iContainer )
        {
        return;
        }

    TBool isSystemStateNormal = static_cast< CFileManagerAppUi* >( AppUi() )->IsSystemStateNormal();
    if ( !isSystemStateNormal )
        {
        FileManagerDlgUtils::ShowErrorNoteL( R_QTN_FMGR_SYSTEM_NOT_READY );
        return;
        }

    const CArrayFix< TInt >* items = iContainer->ListBoxSelectionIndexes();

    TInt count( items->Count() );

    if ( count )
        {
        if( FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
                iDeleteBackups ?
                R_QTN_FMGR_DELETE_BACKUP_QUERY : R_QTN_FMGR_RESTORE_QUERY ) )
            {
            if ( !CheckPhoneState() )
                {
                FileManagerDlgUtils::ShowErrorNoteL( R_QTN_CRITICAL_ERROR );
                }
            else
                {
                TUint64 selection( 0 );
                for ( TInt i( 0 ); i < count; ++i )
                    {
                    selection |= ( ( TUint64 ) 1 ) << ( ( *items )[ i ] );
                    }
    
                CFileManagerRestoreSettings& settings(
                    iEngine.RestoreSettingsL() );
                settings.SetSelection( selection );
    
                if ( iDeleteBackups )
                    {
                    // Delete selected backups and close this view
                    iEngine.DeleteBackupsL();
                    iIndex = 0;
                    static_cast< CFileManagerAppUi* >(
                        AppUi() )->CloseRestoreViewL();
                    }
                else
                    {
                    // Start restoring selected backups
                    TRAPD( err, StartProcessL( ERestoreProcess ) );
                    if ( err == KErrCorrupt )
                        {
                        FileManagerDlgUtils::ShowErrorNoteL(
                            R_QTN_FMGR_ERROR_CORRUPTED_BACKUP_FILE );
                        }
                    else
                        {
                        User::LeaveIfError( err );
                        }
                    }
                }
            }
        }
    else
        {
        CmdCancelL();
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::CmdCancelL
// ----------------------------------------------------------------------------
// 
void CFileManagerRestoreView::CmdCancelL()
    {
    iIndex = 0;
    static_cast< CFileManagerAppUi* >( AppUi() )->CloseRestoreViewL();
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::CmdSelectL
// ----------------------------------------------------------------------------
// 
void CFileManagerRestoreView::CmdSelectL()
    {
    if ( !iContainer )
        {
        return;
        }
    if ( !iContainer->ListBoxNumberOfItems() )
        {
        return;
        }
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    iContainer->ListBoxToggleItemL( index );
    iSelection ^= ( ( TUint64 ) 1 ) << index; // Toggle stored list selection
    UpdateCbaFromSelectionL();
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::CmdCheckMarkL
// ----------------------------------------------------------------------------
// 
void CFileManagerRestoreView::CmdCheckMarkL()
    {
    if ( !iContainer )
        {
        return;
        }
    TInt mskTextId( R_QTN_MSK_MARK );
    if ( iContainer->ListBoxIsItemSelected(
            iContainer->ListBoxCurrentItemIndex() ) )
        {
        mskTextId = R_QTN_MSK_UNMARK;
        }
    
    SetCbaMskTextL( mskTextId );
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::ProcessFinishedL
// ----------------------------------------------------------------------------
// 
void CFileManagerRestoreView::ProcessFinishedL(
        TInt aError, const TDesC& aName )
    {
    TBool isRestore( iActiveProcess == ERestoreProcess );

    CFileManagerViewBase::ProcessFinishedL( aError, aName );

    // Exit when restore finished
    if ( isRestore )
        {
        CmdCancelL();
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::NotifyForegroundStatusChange
// ----------------------------------------------------------------------------
//
void CFileManagerRestoreView::NotifyForegroundStatusChange(
        TBool aForeground )
    {
    FUNC_LOG

    if ( aForeground )
        {
        TRAP_IGNORE( RefreshSettingsL() );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::RefreshSettingsL
// ----------------------------------------------------------------------------
//
void CFileManagerRestoreView::RefreshSettingsL()
    {
    FUNC_LOG

    DirectoryChangedL();
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::RestoreSelectionL
// ----------------------------------------------------------------------------
//
void CFileManagerRestoreView::RestoreSelectionL()
    {
    if ( !iContainer )
        {
        return;
        }
    const CArrayFix< TInt >* items = iContainer->ListBoxSelectionIndexes();
    for ( TInt i( 0 ); i < KMaxSelection; ++i )
        {
        if ( iSelection & ( ( ( TUint64 ) 1 ) << i ) )
            {
            iContainer->ListBoxSelectItemL( i );
            }
        else
            {
            iContainer->ListBoxDeselectItem( i );
            }
        }
    UpdateCbaFromSelectionL();
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::UpdateCbaFromSelectionL
// ----------------------------------------------------------------------------
//
void CFileManagerRestoreView::UpdateCbaFromSelectionL()
    {
    if ( !iContainer )
        {
        return;
        }
    CmdCheckMarkL();
    CEikButtonGroupContainer* cba = Cba();
    TBool hasItem(iContainer->ListBoxNumberOfItems());
    const CArrayFix< TInt >* items = iContainer->ListBoxSelectionIndexes();
    if ( hasItem && items->Count() )
        {
        cba->MakeCommandVisible( EAknSoftkeyOk, ETrue );
        }
    else
        {
        cba->MakeCommandVisible( EAknSoftkeyOk, EFalse );
        }
    cba->DrawDeferred();
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreView::UpdateCbaL
// ----------------------------------------------------------------------------
//
void CFileManagerRestoreView::UpdateCbaL()
    {
    CEikButtonGroupContainer* cba = Cba();
    TBool hasItem(iContainer->ListBoxNumberOfItems());
    cba->MakeCommandVisible( EAknSoftkeyMark, hasItem );
    TBool hasSelecttionItem( hasItem && ( iContainer->ListBoxSelectionIndexesCount() ) );
    cba->MakeCommandVisible( EAknSoftkeyOk, hasSelecttionItem );
    cba->DrawDeferred();
    }
    
//  End of File  
