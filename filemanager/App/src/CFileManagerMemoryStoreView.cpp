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
* Description:  Memory store view
*
*/



// INCLUDE FILES
#include <StringLoader.h>
#include <aknlists.h>
#include <akntitle.h>
#include <CFileManagerEngine.h>
#include <FileManager.rsg>
#include <csxhelp/fmgr.hlp.hrh>
#include <FileManagerDebug.h>
#include "CFileManagerMemoryStoreView.h"
#include "CFileManagerFileListContainer.h"
#include "CFileManagerAppUi.h"
#include "CFileManagerDocument.h"
#include "FileManager.hrh"


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::CFileManagerMemoryStoreView
// ----------------------------------------------------------------------------
//
CFileManagerMemoryStoreView::CFileManagerMemoryStoreView()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::NewLC
// ----------------------------------------------------------------------------
//
CFileManagerMemoryStoreView* CFileManagerMemoryStoreView::NewLC()
    {
    CFileManagerMemoryStoreView* self =
        new( ELeave ) CFileManagerMemoryStoreView();

    CleanupStack::PushL( self );
    self->ConstructL( R_FILEMANAGER_FOLDERS_VIEW );

    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::~CFileManagerMemoryStoreView
// ----------------------------------------------------------------------------
// 
CFileManagerMemoryStoreView::~CFileManagerMemoryStoreView()
    {  
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::DoActivateL
// ----------------------------------------------------------------------------
// 
void CFileManagerMemoryStoreView::DoActivateL(
        const TVwsViewId& aPrevViewId,
        TUid aCustomMessageId,
        const TDesC8& aCustomMessage )
    {
    FUNC_LOG

    // Check embedded app exit first
    CFileManagerAppUi* appUi = 
        static_cast< CFileManagerAppUi* >( AppUi() );
    appUi->ExitEmbeddedAppIfNeededL();

    CFileManagerViewBase::DoActivateL(
        aPrevViewId, aCustomMessageId, aCustomMessage );

    TBool showRemoteNotConnected( EFalse );
    // Ensure that root directory is defined
    TInt count( iEngine.NavigationLevel() );
    if ( count > 0 )
        {
        iIndex = 0;
        for ( TInt i( 0 ); i < count; i++ )
            {
            TRAP_IGNORE( iEngine.BackstepL() );
            }
        showRemoteNotConnected = ETrue;
        }

    TRAP_IGNORE ( RefreshDriveInfoL() );
    RefreshTitleL();
    iEngine.SetState( CFileManagerEngine::ENavigation );
    iEngine.SetObserver( this );
    if ( DriveAvailableL( showRemoteNotConnected ) )
        {
        iEngine.RefreshDirectory();
        }
    else
        {
        UpdateCbaL();
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::DoDeactivate
// ----------------------------------------------------------------------------
// 
void CFileManagerMemoryStoreView::DoDeactivate()
    {
    FUNC_LOG

    CFileManagerViewBase::DoDeactivate();
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::DirectoryChangedL
// ----------------------------------------------------------------------------
// 
void CFileManagerMemoryStoreView::DirectoryChangedL()
    {
    FUNC_LOG

    TBool drvAvailable( DriveAvailableL( ETrue ) );

    if ( iContainer )
        {
        if ( drvAvailable )
            {
            TInt index = iEngine.CurrentIndex();
            if (  index != KErrNotFound )
                {
                iIndex = index;
                }
            iContainer->RefreshListL( iIndex );
            }
        else
            {
            iIndex = 0;
            }
        RefreshTitleL();
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::Id
// ----------------------------------------------------------------------------
// 
TUid CFileManagerMemoryStoreView::Id() const
    {
    return CFileManagerAppUi::KFileManagerMemoryStoreViewId;
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::CreateContainerL
// ----------------------------------------------------------------------------
// 
CFileManagerContainerBase* CFileManagerMemoryStoreView::CreateContainerL()
    {
    return CFileManagerFileListContainer::NewL(
        ClientRect(),
        iIndex,
        CFileManagerFileListContainer::EListMemoryStore,
        R_QTN_SELEC_EMPTY_LIST,
        KFMGR_HLP_MEM_STORE_VIEW );
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::HandleCommandL
// ----------------------------------------------------------------------------
// 
void CFileManagerMemoryStoreView::HandleCommandL( TInt aCommand )
    {
    switch( aCommand )
        {
        case EFileManagerOpen:
            {
            CmdOpenL();
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
// CFileManagerMemoryStoreView::DriveAvailableL
// ----------------------------------------------------------------------------
//
TBool CFileManagerMemoryStoreView::DriveAvailableL(
        TBool aShowRemoteNotConnected )
    {
    HBufC* error = NULL;
    TFileManagerDriveInfo& drvInfo( DriveInfo() );
    TInt usb_err( KErrNone );    
    if ( drvInfo.iState & (
            TFileManagerDriveInfo::EDriveRemovable |
            TFileManagerDriveInfo::EDriveRemote ) )
        {
        TRAP( usb_err , RefreshDriveInfoL() );
        }    
	if( usb_err != KErrNone )
		{
		error = StringLoader::LoadLC( R_QTN_MEMC_MAINPAGE_ERROR1 );
        }

    INFO_LOG1( "CFileManagerMemoryStoreView::DriveAvailableL-drvState=%d",
        drvInfo.iState )

    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote )
        {
        iEngine.SetMemoryL( EFmRemoteDrive );
        }
#ifdef RD_MULTIPLE_DRIVE
    else if ( drvInfo.iState & TFileManagerDriveInfo::EDriveMassStorage )
        {
        iEngine.SetMemoryL( EFmMassStorage );
        }
#endif // RD_MULTIPLE_DRIVE
    else if ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemovable )
        {
        iEngine.SetMemoryL( EFmMemoryCard );
        }
    else if ( drvInfo.iState & TFileManagerDriveInfo::EDriveUsbMemory )
        {
        iEngine.SetMemoryL( EFmUsbMemory );
        }
    else
        {
        iEngine.SetMemoryL( EFmPhoneMemory );
        }

#ifdef RD_MULTIPLE_DRIVE
    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveMassStorage )
        {
        // Handle mass storage states
        if ( !( drvInfo.iState & TFileManagerDriveInfo::EDrivePresent ) ||
             ( drvInfo.iState & TFileManagerDriveInfo::EDriveInUse ) )
            {
            error = StringLoader::LoadLC( R_QTN_MEMC_MASS_STORAGE_IN_USE );
            }
        else if ( drvInfo.iState & TFileManagerDriveInfo::EDriveCorrupted )
            {
            error = StringLoader::LoadLC( R_QTN_MEMC_MASS_STORAGE_CORRUPTED );
            }
        }
    else
        {
        // Handle other drives
#endif // RD_MULTIPLE_DRIVE
        if ( drvInfo.iState & TFileManagerDriveInfo::EDriveInUse )
            {
            error = StringLoader::LoadLC( R_QTN_MEMC_MAINPAGE_ERROR4 );
            }
        else if ( drvInfo.iState & TFileManagerDriveInfo::EDriveLocked )
            {
            error = StringLoader::LoadLC( R_QTN_MEMC_MAINPAGE_ERROR2 );
            }
        else if ( drvInfo.iState & TFileManagerDriveInfo::EDriveCorrupted )
            {
            error = StringLoader::LoadLC( R_QTN_MEMC_MAINPAGE_ERROR3 );
            }
        else if ( !( drvInfo.iState & TFileManagerDriveInfo::EDrivePresent ) )
            {
            error = StringLoader::LoadLC( R_QTN_MEMC_NOT_AVAILABLE );
            }
        else if ( aShowRemoteNotConnected &&
                 ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote ) &&
                 ! ( drvInfo.iState & TFileManagerDriveInfo::EDriveConnected ) )
            {
            error = StringLoader::LoadLC( R_QTN_FMGR_EMPTY_REMOTE_DRIVE );
            }
#ifdef RD_MULTIPLE_DRIVE
        }
#endif // RD_MULTIPLE_DRIVE
    if ( error )
        {
        if ( iContainer )
            {
            iContainer->ListBoxSetTextL( *error );
            }
        CleanupStack::PopAndDestroy( error );
        return EFalse;
        }
    return ETrue;
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::RefreshTitleL
// ----------------------------------------------------------------------------
//
void CFileManagerMemoryStoreView::RefreshTitleL()
    {
    static_cast< CFileManagerAppUi* >( AppUi() )->SetTitleL(
        iEngine.CurrentDriveName() );
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::CmdOpenL
// ----------------------------------------------------------------------------
//
void CFileManagerMemoryStoreView::CmdOpenL()
    {
    StoreIndex();
    CFileManagerViewBase::CmdOpenL();
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::CmdBackL
// ----------------------------------------------------------------------------
//
void CFileManagerMemoryStoreView::CmdBackL()
    {
    if ( iActiveProcess != ENoProcess )
        {
        return; // Ignore to avoid container mess up
        }
    CFileManagerAppUi* appUi =
        static_cast< CFileManagerAppUi* >( AppUi() );
    iEngine.BackstepL();
    iIndex = 0;
    appUi->CloseMemoryStoreViewL();
    }

// ----------------------------------------------------------------------------
// CFileManagerMemoryStoreView::UpdateCbaL
// ----------------------------------------------------------------------------
//
void CFileManagerMemoryStoreView::UpdateCbaL()
    {
    UpdateCommonCbaL();
    }

//  End of File  
