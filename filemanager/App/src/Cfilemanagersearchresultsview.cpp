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
* Description:  View for search results
*
*/



// INCLUDE FILES
#include <CFileManagerEngine.h>
#include <filemanager.rsg>
#include <aknlists.h>
#include <aknnavi.h>
#include <akntitle.h> 
#include <csxhelp/fmgr.hlp.hrh>
#include <FileManagerDebug.h>
#include "Cfilemanagersearchresultsview.h"
#include "CFileManagerAppUi.h"
#include "CFileManagerFileListContainer.h"
#include "CFileManagerAppUi.h"
#include "CFileManagerDocument.h"
#include "FileManager.hrh"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerSearchResultsView::CFileManagerSearchResultsView
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerSearchResultsView::CFileManagerSearchResultsView()
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerSearchResultsView::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFileManagerSearchResultsView::ConstructL()
    {
	CFileManagerViewBase::ConstructL( R_FILEMANAGER_FOLDERS_VIEW );

	CEikStatusPane* sp = StatusPane();
	iNaviPane = static_cast< CAknNavigationControlContainer* >
		( sp->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) ) );
    }

// -----------------------------------------------------------------------------
// CFileManagerSearchResultsView::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerSearchResultsView* CFileManagerSearchResultsView::NewLC()
    {
    CFileManagerSearchResultsView* self =
        new( ELeave ) CFileManagerSearchResultsView;
    
    CleanupStack::PushL( self );
    self->ConstructL();

    return self;
    }
 
// -----------------------------------------------------------------------------
// CFileManagerSearchResultsView::~CFileManagerSearchResultsView
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerSearchResultsView::~CFileManagerSearchResultsView()
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerSearchResultsView::DirectoryChangedL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerSearchResultsView::DirectoryChangedL()
	{
    FUNC_LOG

    RefreshTitleL();

    if ( iContainer )
        {
        iContainer->RefreshListL( iIndex );
        UpdateCbaL();
        }
    }
// -----------------------------------------------------------------------------
// CFileManagerSearchResultsView::Id
// 
// -----------------------------------------------------------------------------
// 
TUid CFileManagerSearchResultsView::Id() const
	{
	return CFileManagerAppUi::KFileManagerSearchResultsViewId;
	}

// -----------------------------------------------------------------------------
// CFileManagerSearchResultsView::CreateContainerL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerContainerBase* CFileManagerSearchResultsView::CreateContainerL() 
	{
    return CFileManagerFileListContainer::NewL(
        ClientRect(),
        iIndex,
        CFileManagerFileListContainer::EListFolder,
        R_QTN_FMGR_SEARCH_NOT_FOUND,
        KFMGR_HLP_SEARCH_RESULTS );
	}

// -----------------------------------------------------------------------------
// CFileManagerSearchResultsView::DoActivateL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerSearchResultsView::DoActivateL( const TVwsViewId& aPrevViewId, 
                                                 TUid aCustomMessageId, 
                                                 const TDesC8& aCustomMessage )
	{
    FUNC_LOG

	CFileManagerViewBase::DoActivateL( aPrevViewId, aCustomMessageId, aCustomMessage );

    iEngine.SetState( CFileManagerEngine::ESearch );
	iNaviPane->PushDefaultL( ETrue );
    iNaviDecorator = iNaviPane->Top();
    iEngine.SetObserver( this );

    if ( !aCustomMessage.Compare( KFileManagerSearchViewRefreshMsg ) )
        {
        // Start new search only if explicitly requested
        iIndex = 0;
        RefreshTitleL();
        iEngine.RefreshDirectory();
        }
    else
        {
        // Update list without new search
        DirectoryChangedL();
        }
	}

// -----------------------------------------------------------------------------
// CFileManagerSearchResultsView::DoDeactivate
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerSearchResultsView::DoDeactivate()
	{
    FUNC_LOG

	CFileManagerViewBase::DoDeactivate();

	iNaviPane->Pop( iNaviDecorator );
	}

// -----------------------------------------------------------------------------
// CFileManagerSearchResultsView::HandleCommandL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerSearchResultsView::HandleCommandL( TInt aCommand )
	{
	switch( aCommand )
		{
        case EFileManagerOpen:
            {
            StoreIndex();
            CFileManagerViewBase::CmdOpenL();
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

// ------------------------------------------------------------------------------
// CFileManagerSearchResultsView::UpdateCbaL
//
// ------------------------------------------------------------------------------
//
void CFileManagerSearchResultsView::UpdateCbaL()
    {
    UpdateCommonCbaL();
    }

// ----------------------------------------------------------------------------
// CFileManagerSearchResultsView::CmdBackL
// ----------------------------------------------------------------------------
//
void CFileManagerSearchResultsView::CmdBackL()
    {
    if ( iActiveProcess != ENoProcess )
        {
        return; // Ignore to avoid container mess up
        }
    iIndex = 0;
    static_cast< CFileManagerAppUi* >( AppUi() )->CloseSearchResultsViewL();
    }

// -----------------------------------------------------------------------------
// CFileManagerSearchResultsView::RefreshTitleL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerSearchResultsView::RefreshTitleL()
    {
    static_cast< CFileManagerAppUi* >( AppUi() )->SetTitleL(
        iEngine.SearchString() );
    iNaviPane->DrawDeferred();
    }

//  End of File  
