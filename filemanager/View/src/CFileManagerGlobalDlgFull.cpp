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
* Description:  Global dialog handling
*
*/



// INCLUDE FILES
#include <StringLoader.h>
#include <AknGlobalNote.h>
#include <AknGlobalConfirmationQuery.h>
#include <AknGlobalProgressDialog.h>
#include <avkon.rsg>
#include <aknconsts.h>
#include <avkon.mbg>
#include "CFileManagerGlobalDlg.h"
#include "CFileManagerGlobalQueryDlg.h"


// CONSTANTS
const TUint KCountdownTimeout = 1000000; // 1s
const TInt KCountdownSteps = 30;
const TInt KCountdownShowSKStep = 3;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::CFileManagerGlobalDlg
// ---------------------------------------------------------------------------
//
CFileManagerGlobalDlg::CFileManagerGlobalDlg() :
        CActive( EPriorityUserInput )
    {
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::NewL
// ---------------------------------------------------------------------------
//
EXPORT_C CFileManagerGlobalDlg* CFileManagerGlobalDlg::NewL()
    {
    CFileManagerGlobalDlg* self =
        new( ELeave ) CFileManagerGlobalDlg();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::ConstructL
// ---------------------------------------------------------------------------
// 
void CFileManagerGlobalDlg::ConstructL()
    {
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::~CFileManagerGlobalDlg
// ---------------------------------------------------------------------------
// 
EXPORT_C CFileManagerGlobalDlg::~CFileManagerGlobalDlg()
    {
    Cancel();
    delete iQueryDialog;
    delete iProgressDialog;
    delete iCountdown;
    delete iCountdownText;
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::CancelDialog
// ---------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerGlobalDlg::CancelDialog()
    {
    delete iCountdown;
    iCountdown = NULL;

    if ( iQueryDialog )
        {
        iQueryDialog->CancelDialog();
        delete iQueryDialog;
        iQueryDialog = NULL;
        }

    if ( iProgressDialog )
        {
        iProgressDialog->CancelProgressDialog();
        delete iProgressDialog;
        iProgressDialog = NULL;
        }

    Cancel();
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::ShowDialogL
// ---------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerGlobalDlg::ShowDialogL(
        const CFileManagerGlobalDlg::TType aType,
        const TDesC& aText,
        const TInt aSkId )
    {
    CancelDialog();

    iType = aType;

    switch ( aType )
        {
        case ECountdownQuery:
            {
            iCountdownText = aText.AllocL();
            iQueryDialog = CFileManagerGlobalQueryDlg::NewL();
            iQueryDialog->ShowDialogL(
                *iCountdownText,
                R_AVKON_SOFTKEYS_EMPTY );
            iCountdownSteps = 0;
            StartCountdownL( KCountdownTimeout );
            break;
            }
        case EQuery:
            {
            iQueryDialog = CFileManagerGlobalQueryDlg::NewL();
            iQueryDialog->SetObserver( this );
            iQueryDialog->ShowDialogL(
                aText,
                aSkId );
            break;
            }
        case EQueryWithWarningIcon:
            {
            iQueryDialog = CFileManagerGlobalQueryDlg::NewL();
            iQueryDialog->SetObserver( this );
            iQueryDialog->ShowDialogL(
                aText,
                aSkId,
                KAvkonBitmapFile,
                EMbmAvkonQgn_note_warning,
                EMbmAvkonQgn_note_warning_mask );
            break;
            }
        case EQueryWithInfoIcon:
            {
            iQueryDialog = CFileManagerGlobalQueryDlg::NewL();
            iQueryDialog->SetObserver( this );
            iQueryDialog->ShowDialogL(
                aText,
                aSkId,
                KAvkonBitmapFile,
                EMbmAvkonQgn_note_info,
                EMbmAvkonQgn_note_info_mask );
            break;
            }
        case EProgressDialog:
            {
            iProgressDialog = CAknGlobalProgressDialog::NewL();
            if ( aSkId )
                {
                iProgressDialog->ShowProgressDialogL(
                    iStatus, aText, aSkId );
                }
            else
                {
                iProgressDialog->ShowProgressDialogL(
                    iStatus, aText );
                }
            SetActive();
            iFinalValue = 0;
            break;
            }
        case EErrorNote:
            {
            CAknGlobalNote* note = CAknGlobalNote::NewLC();
            note->SetTone( EAvkonSIDNoSound ); // no tone
            note->ShowNoteL( EAknGlobalErrorNote, aText );
            CleanupStack::PopAndDestroy( note );
            break;
            }
        case EInfoNote:
            {
            CAknGlobalNote* note = CAknGlobalNote::NewLC();
            note->SetTone( EAvkonSIDNoSound ); // no tone
            note->ShowNoteL( EAknGlobalInformationNote, aText );
            CleanupStack::PopAndDestroy( note );
            break;
            }
        default:
            {
            break;
            }
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::ShowDialogL
// ---------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerGlobalDlg::ShowDialogL(
        const CFileManagerGlobalDlg::TType aType,
        const TInt aTextId,
        const TInt aSkId )
    {
    HBufC* text = StringLoader::LoadLC( aTextId );
    ShowDialogL( aType, *text, aSkId );
    CleanupStack::PopAndDestroy( text );
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::DoCancel
// ---------------------------------------------------------------------------
// 
void CFileManagerGlobalDlg::DoCancel() 
	{
    if ( iQueryDialog )
        {
        iQueryDialog->CancelDialog();
        }
    if ( iProgressDialog )
        {
        iProgressDialog->CancelProgressDialog();
        }
	}

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::RunL
// ---------------------------------------------------------------------------
// 
void CFileManagerGlobalDlg::RunL()
    {
    CancelDialog();
    NotifyObserver( iStatus.Int() );
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::SetObserver
// ---------------------------------------------------------------------------
//
EXPORT_C void CFileManagerGlobalDlg::SetObserver(
        MFileManagerGlobalDlgObserver* aObserver )
    {
    // This instance is not allowed to set by client
    if ( aObserver != this )
        {
        iObserver = aObserver;
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::StartCountdownL
// ---------------------------------------------------------------------------
//
void CFileManagerGlobalDlg::StartCountdownL( const TUint aTimeout )
    {
    delete iCountdown;
    iCountdown = NULL;
    iCountdown = CPeriodic::NewL( CActive::EPriorityStandard );
    iCountdown->Start( aTimeout, aTimeout, TCallBack( CountdownCB, this ) );
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::CountdownCB
// ---------------------------------------------------------------------------
// 
TInt CFileManagerGlobalDlg::CountdownCB( TAny* aPtr )
    {
    CFileManagerGlobalDlg* self =
        static_cast< CFileManagerGlobalDlg* >( aPtr );

    TRAP_IGNORE( self->CountdownL() );

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::CountdownL
// ---------------------------------------------------------------------------
// 
void CFileManagerGlobalDlg::CountdownL()
    {
    if( iCountdownSteps < KCountdownSteps )
        {
        // Countdown in progress
        ++iCountdownSteps;
        if ( iCountdownSteps == KCountdownShowSKStep )
            {
            iProgressDialog = CAknGlobalProgressDialog::NewL();
            iProgressDialog->SetImageL(
                KAvkonBitmapFile,
                EMbmAvkonQgn_note_query,
                EMbmAvkonQgn_note_query_mask );
            iProgressDialog->ShowProgressDialogL(
                iStatus,
                *iCountdownText,
                R_AVKON_SOFTKEYS_YES_NO );
            SetActive();

            if ( iQueryDialog )
                {
                iQueryDialog->CancelDialog();
                delete iQueryDialog;
                iQueryDialog = NULL;
                }
            }

        if ( iProgressDialog &&
            iCountdownSteps >= KCountdownShowSKStep )
            {
            iProgressDialog->UpdateProgressDialog(
                iCountdownSteps, KCountdownSteps );
            }
        }
    else
        {
        // Countdown expired
        CancelDialog();
        NotifyObserver( EAknSoftkeyYes );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::UpdateProgressDialog
// ---------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerGlobalDlg::UpdateProgressDialog(
        const TInt aValue,
        const TInt aFinalValue )
    {
    if ( iProgressDialog )
        {
        if ( aFinalValue >= 0 )
            {
            iFinalValue = aFinalValue;
            }
        iProgressDialog->UpdateProgressDialog( aValue, iFinalValue );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::ProcessFinished
// ---------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerGlobalDlg::ProcessFinished()
    {
    if ( iProgressDialog )
        {
        iProgressDialog->ProcessFinished();
        delete iProgressDialog;
        iProgressDialog = NULL;
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::NotifyObserver
// ---------------------------------------------------------------------------
// 
void CFileManagerGlobalDlg::NotifyObserver( const TInt aValue )
    {
    if ( iObserver )
        {
        iObserver->HandleGlobalDlgResult( iType, aValue );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerGlobalDlg::HandleGlobalDlgResult
// ---------------------------------------------------------------------------
// 
void CFileManagerGlobalDlg::HandleGlobalDlgResult(
        TInt /*aDlgType*/,
        TInt aDlgResult )
    {
    NotifyObserver( aDlgResult );
    }

//  End of File  
