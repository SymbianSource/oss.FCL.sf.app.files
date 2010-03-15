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
* Description:  View for folder
*
*/



// INCLUDE FILES
#include <akntitle.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <barsread.h>
#include <aknlists.h>
#include <AknsConstants.h>
#include <AknInfoPopupNoteController.h>
#include <csxhelp/fmgr.hlp.hrh>
#include <CFileManagerEngine.h>
#include <FileManager.rsg>
#include <FileManagerView.rsg>
#include <TFileManagerDriveInfo.h>
#include <CFileManagerCommonDefinitions.h>
#include <FileManagerDebug.h>
#include "CFileManagerFoldersView.h"
#include "CFileManagerFileListContainer.h"
#include "CFileManagerAppUi.h"
#include "CFileManagerDocument.h"
#include "Cfilemanagerfoldernavigationpane.h"
#include "FileManager.hrh"
#include "FileManagerUID.h"

// CONSTANTS
const TUid CFileManagerFoldersView::KOpenFromSearchResultsView =
    { EOpenFromSearchResultsView };


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerFoldersView::CFileManagerFoldersView
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerFoldersView::CFileManagerFoldersView()
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerFoldersView::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFileManagerFoldersView::ConstructL()
    {
    CFileManagerViewBase::ConstructL( R_FILEMANAGER_FOLDERS_VIEW );

    CEikStatusPane* sp = StatusPane();

    iNaviPane = static_cast< CAknNavigationControlContainer* >
        ( sp->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) ) );

    iPopupController = CAknInfoPopupNoteController::NewL();
    iPopupController->SetTimeDelayBeforeShow( 0 );
    }

// -----------------------------------------------------------------------------
// CFileManagerFoldersView::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerFoldersView* CFileManagerFoldersView::NewLC()
    {
    CFileManagerFoldersView* self = new( ELeave ) CFileManagerFoldersView;
    
    CleanupStack::PushL( self );
    self->ConstructL();

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerFoldersView::~CFileManagerFoldersView
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerFoldersView::~CFileManagerFoldersView()
    {
    delete iPopupController;
    delete iNaviDecorator;
    }

// -----------------------------------------------------------------------------
// CFileManagerFoldersView::Id
// 
// -----------------------------------------------------------------------------
// 
TUid CFileManagerFoldersView::Id() const
    {
    return CFileManagerAppUi::KFileManagerFoldersViewId;
    }

// -----------------------------------------------------------------------------
// CFileManagerFoldersView::CreateContainerL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerContainerBase* CFileManagerFoldersView::CreateContainerL() 
    {
    return CFileManagerFileListContainer::NewL(
        ClientRect(),
        iIndex,
        CFileManagerFileListContainer::EListFolder,
        R_QTN_SELEC_EMPTY_LIST,
        KFMGR_HLP_MEM_STORE_VIEW );
    }

// -----------------------------------------------------------------------------
// CFileManagerFoldersView::DoActivateL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFoldersView::DoActivateL( const TVwsViewId& aPrevViewId, 
                                           TUid aCustomMessageId, 
                                           const TDesC8& aCustomMessage )
    {
    FUNC_LOG

    if ( aCustomMessageId == KOpenFromSearchResultsView )
        {
        iInitialFolderDepth = iEngine.FolderLevel();
        }
    else
        {
        // Check embedded app exit
        CFileManagerAppUi* appUi = 
            static_cast< CFileManagerAppUi* >( AppUi() );
        appUi->ExitEmbeddedAppIfNeededL();

        iInitialFolderDepth = 1; // First folder view level
        }

    CFileManagerViewBase::DoActivateL( aPrevViewId, aCustomMessageId, aCustomMessage );

    TResourceReader reader;
    iCoeEnv->CreateResourceReaderLC( reader, R_FILEMANAGER_FOLDER_NAVIGATION_PANE );
    iNaviControl = CFileManagerFolderNavigationPane::NewL( iEngine.Memory(), 0, reader );
    CleanupStack::PopAndDestroy(); // reader

    iNaviControl->SetObserver( this );

    iNaviDecorator = CAknNavigationDecorator::NewL( iNaviPane, iNaviControl );
    iNaviDecorator->SetContainerWindowL( *iNaviPane );
    iNaviPane->PushL( *iNaviDecorator );

    RefreshTitleL();
    iEngine.SetState( CFileManagerEngine::ENavigation );
    iEngine.SetObserver( this );
    iEngine.RefreshDirectory();
    }
// -----------------------------------------------------------------------------
// CFileManagerFoldersView::DoDeactivate
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFoldersView::DoDeactivate()
    {
    FUNC_LOG

    CFileManagerViewBase::DoDeactivate();

    iNaviPane->Pop( iNaviDecorator );
    delete iNaviDecorator;
    iNaviDecorator = NULL;
    iNaviControl = NULL; // Not owned and deleted by decorator

    iPopupController->HideInfoPopupNote();
    }

// -----------------------------------------------------------------------------
// CFileManagerFoldersView::RefreshTitleL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFoldersView::RefreshTitleL()
    {
    static_cast< CFileManagerAppUi* >( AppUi() )->SetTitleL(
        iEngine.LocalizedNameOfCurrentDirectory() );

    if ( iNaviControl ) // May be deleted by view deactivation
        {
        iNaviControl->ChangeRootL( iEngine.Memory() );
        iNaviControl->SetFolderDepth( iEngine.FolderLevel() );
        }

    iNaviPane->DrawDeferred();
    }

// -----------------------------------------------------------------------------
// CFileManagerFoldersView::HandleCommandL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFoldersView::HandleCommandL( TInt aCommand )
    {
    switch( aCommand )
        {
        case EAknSoftkeyBack:
            {
            BackstepL();
            break;
            }
        case EFileManagerOpen:
            {
            iPopupController->HideInfoPopupNote();
            CFileManagerViewBase::CmdOpenL();
            break;
            }
        default:
            {
            CFileManagerViewBase::HandleCommandL( aCommand );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFoldersView::DirectoryChangedL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFoldersView::DirectoryChangedL()
    {
    FUNC_LOG
    
    CFileManagerAppUi* appUi = static_cast< CFileManagerAppUi* >( AppUi() );

    if ( iEngine.FolderLevel() < iInitialFolderDepth || !iContainer )
        {
        appUi->CloseFoldersViewL();
        }
    else
        {
        if ( !appUi->IsSearchViewOpen() )
            {
            appUi->ExitEmbeddedAppIfNeededL();
            }
            TRAP_IGNORE( RefreshDriveInfoL() );
            TFileManagerDriveInfo& drvInfo( DriveInfo() );
            if ( !( drvInfo.iState & TFileManagerDriveInfo::EDrivePresent ) )
                {
                User::Leave(KErrPathNotFound);
                }
            else
                {
                TInt index( iEngine.CurrentIndex() );
                if ( index != KErrNotFound )
                    {
                    iContainer->RefreshListL( index );
                    }
                else
                    {
                    iContainer->RefreshListL( iIndex );
                    }
                RefreshTitleL();
                }
        iFolderDepth = iEngine.FolderLevel();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFoldersView::HandleResourceChangeL
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CFileManagerFoldersView::HandleResourceChangeL( TInt aType )
    {
    if ( iNaviControl ) // May be deleted by view deactivation
        {
        iNaviControl->HandleResourceChangeL( aType );
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerFoldersView::UpdateCbaL
//
// ------------------------------------------------------------------------------
//
void CFileManagerFoldersView::UpdateCbaL()
    {
    UpdateCommonCbaL();
    }

// ------------------------------------------------------------------------------
// CFileManagerFoldersView::BackstepL
//
// ------------------------------------------------------------------------------
//
void CFileManagerFoldersView::BackstepL(
       TInt aBacksteps )
    {
    if (iActiveProcess != ENoProcess)
        {
        return; // Ignore to avoid container mess up
        }
    iPopupController->HideInfoPopupNote();
    CFileManagerAppUi* appUi = static_cast<CFileManagerAppUi*> (AppUi());
    TInt level(iEngine.FolderLevel());
    while ( aBacksteps > 0 )
        {
        if ( level < iInitialFolderDepth )
            {
            break;
            }
        TRAP_IGNORE( iEngine.BackstepL() );
        --level;
        --aBacksteps;
        }

    if ( !appUi->IsSearchViewOpen() )
        {
        appUi->ExitEmbeddedAppIfNeededL();
        }

    if (level < iInitialFolderDepth)
        {
        iFolderDepth = 0;
        iIndex = 0;
        appUi->CloseFoldersViewL();
        }
    else
        {
        iEngine.SetObserver(this);
        iEngine.RefreshDirectory();
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerFoldersView::FolderName
//
// ------------------------------------------------------------------------------
//
TPtrC CFileManagerFoldersView::FolderName( const TInt aLevel )
    {
    if ( !aLevel )
        {
        // Get memory store name
        return iEngine.CurrentDriveName();
        }

    // Get folder name
    TPtrC dir( iEngine.CurrentDirectory() );
    TInt drive = TDriveUnit( dir );
    TPtrC root( iEngine.DriveRootDirectory( drive ) );
    TInt count( dir.Length() );
    TInt bsCount( 0 );

    for ( TInt i( root.Length() ); i < count; )
        {
        TPtrC ptr( dir.Mid( i ) );
        TInt j( ptr.Locate( KFmgrBackslash()[ 0 ] ) );
        if ( j != KErrNotFound )
            {
            ++bsCount;
            if ( bsCount == aLevel )
                {
                // Use localised folder name if it exists
                TPtrC locName( iEngine.LocalizedName( dir.Left(
                    i + j + 1 ) ) );
                if ( locName.Length() )
                    {
                    return locName; 
                    }
                return TPtrC( ptr.Left( j ) );
                }
            }
        else
            {
            break;
            }
        i += j + 1;
        }
    return TPtrC( KNullDesC );
    }

// ------------------------------------------------------------------------------
// CFileManagerFoldersView::HandleFolderNaviEventL
//
// ------------------------------------------------------------------------------
//
void CFileManagerFoldersView::HandleFolderNaviEventL(
        TNaviEvent aEvent, TInt aValue )
    {
    switch ( aEvent )
        {
        case ENaviTapDown:
            {
            iPopupController->HideInfoPopupNote();
            break;
            }
        case ENaviTapUp:
            {
            if ( aValue != KErrNotFound )
                {
                TInt level( iEngine.FolderLevel() );
                if ( level > aValue )
                    {
                    BackstepL( level - aValue );
                    }
                }
            break;
            }
        case ENaviLongTap:
            {
            if ( aValue != KErrNotFound )
                {
                TPtrC folder( FolderName( aValue ) );
                if ( folder.Length() )
                    {
                    iPopupController->SetTextL( folder );
                    iPopupController->ShowInfoPopupNote();
                    }
                }
            break;
            }
        default:
            {
            break;
            }
        }
    }

//  End of File  
