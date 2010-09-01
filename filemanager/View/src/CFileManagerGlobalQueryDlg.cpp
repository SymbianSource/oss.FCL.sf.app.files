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
* Description:  Global query dialog handling
*
*/



// INCLUDE FILES
#include <StringLoader.h>
#include <AknGlobalConfirmationQuery.h>
#include "CFileManagerGlobalQueryDlg.h"
#include "CFileManagerGlobalDlg.h"
#include "MFileManagerGlobalDlgObserver.h"


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::CFileManagerGlobalQueryDlg
// ----------------------------------------------------------------------------
//
CFileManagerGlobalQueryDlg::CFileManagerGlobalQueryDlg() :
        CActive( EPriorityUserInput )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::NewL
// ----------------------------------------------------------------------------
//
CFileManagerGlobalQueryDlg* CFileManagerGlobalQueryDlg::NewL()
    {
    CFileManagerGlobalQueryDlg* self =
        new( ELeave ) CFileManagerGlobalQueryDlg();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::ConstructL
// ----------------------------------------------------------------------------
// 
void CFileManagerGlobalQueryDlg::ConstructL()
    {
    CActiveScheduler::Add( this );
    }

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::~CFileManagerGlobalQueryDlg
// ----------------------------------------------------------------------------
// 
CFileManagerGlobalQueryDlg::~CFileManagerGlobalQueryDlg()
    {
    Cancel();
    delete iQueryDialog;
    }

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::CancelDialog
// ----------------------------------------------------------------------------
// 
void CFileManagerGlobalQueryDlg::CancelDialog()
    {
    if ( iQueryDialog )
        {
        iQueryDialog->CancelConfirmationQuery();
        delete iQueryDialog;
        iQueryDialog = NULL;
        }
    Cancel();
    }

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::ShowDialogL
// ----------------------------------------------------------------------------
// 
void CFileManagerGlobalQueryDlg::ShowDialogL(
        const TDesC& aText,
        const TInt aSkId )
    {
    CancelDialog();

    iQueryDialog = CAknGlobalConfirmationQuery::NewL();
    iQueryDialog->ShowConfirmationQueryL(
        iStatus, aText, aSkId );
    SetActive();
    }

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::ShowDialogL
// ----------------------------------------------------------------------------
// 
void CFileManagerGlobalQueryDlg::ShowDialogL(
        const TDesC& aText,
        const TInt aSkId,
        const TDesC& aBitmapFile,
        const TInt aImageId,
        const TInt aMaskId )
    {
    CancelDialog();

    iQueryDialog = CAknGlobalConfirmationQuery::NewL();
    iQueryDialog->ShowConfirmationQueryL(
        iStatus,
        aText,
        aSkId,
        0,
        aBitmapFile,
        aImageId,
        aMaskId
         );
    SetActive();
    }

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::ShowDialogL
// ----------------------------------------------------------------------------
// 
void CFileManagerGlobalQueryDlg::ShowDialogL(
        const TInt aTextId,
        const TInt aSkId )
    {
    HBufC* text = StringLoader::LoadLC( aTextId );
    ShowDialogL( *text, aSkId );
    CleanupStack::PopAndDestroy( text );
    }

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::DoCancel
// ----------------------------------------------------------------------------
// 
void CFileManagerGlobalQueryDlg::DoCancel() 
	{
    if ( iQueryDialog )
        {
        iQueryDialog->CancelConfirmationQuery();
        }
	}

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::RunL
// ----------------------------------------------------------------------------
// 
void CFileManagerGlobalQueryDlg::RunL()
    {
    CancelDialog();
    NotifyObserver( iStatus.Int() );
    }

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::SetObserver
// ----------------------------------------------------------------------------
//
void CFileManagerGlobalQueryDlg::SetObserver(
        MFileManagerGlobalDlgObserver* aObserver )
    {
    iObserver = aObserver;
    }

// ----------------------------------------------------------------------------
// CFileManagerGlobalQueryDlg::NotifyObserver
// ----------------------------------------------------------------------------
// 
void CFileManagerGlobalQueryDlg::NotifyObserver( const TInt aValue )
    {
    if ( iObserver )
        {
        iObserver->HandleGlobalDlgResult(
            CFileManagerGlobalDlg::EQuery,
            aValue );
        }
    }

//  End of File  
