/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Asynchronous refresher for refreshing the CGflmNavigatorModel
*
*/



// INCLUDE FILES
#include "CFileManagerRefresher.h"
#include "MFileManagerProcessObserver.h"
#include "CGflmNavigatorModel.h"
#include "FileManagerDebug.h"
#include <e32std.h>


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerRefresher::CFileManagerRefresher
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerRefresher::CFileManagerRefresher( CGflmNavigatorModel& aModel ) :
        CActive( CActive::EPriorityStandard ),
        iModel( aModel )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerRefresher::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFileManagerRefresher::ConstructL()
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CFileManagerRefresher::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerRefresher* CFileManagerRefresher::NewL( CGflmNavigatorModel& aModel )
    {
    CFileManagerRefresher* self =
        new( ELeave ) CFileManagerRefresher( aModel );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerRefresher::~CFileManagerRefresher()
//
// -----------------------------------------------------------------------------
//
CFileManagerRefresher::~CFileManagerRefresher()
    {
    Cancel();
    }

// -----------------------------------------------------------------------------
// CFileManagerRefresher::RunL
// -----------------------------------------------------------------------------
//
void CFileManagerRefresher::RunL()
    {
    FUNC_LOG

    TInt err( iStatus.Int() );

    LOG_IF_ERROR1( err, "CFileManagerRefresher::RunL()-err=%d", err )

    if( iObserver )
        {
        iObserver->RefreshStoppedL();
        if ( err == KErrCancel )
            {
            // For going back to parent folder
            iObserver->Error( KErrPathNotFound );
            }
        }

    // Forward OOM, suppress other errors
    if ( err == KErrNoMemory )
        {
        User::Leave( err );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerRefresher::RunError
// -----------------------------------------------------------------------------
//
TInt CFileManagerRefresher::RunError( TInt aError )
    {
    ERROR_LOG1( "CFileManagerRefresher::RunError()-err=%d", aError )
    return aError;
    }

// -----------------------------------------------------------------------------
// CFileManagerRefresher::DoCancel
// -----------------------------------------------------------------------------
//
void CFileManagerRefresher::DoCancel()
    {
    FUNC_LOG

    iModel.CancelRefresh();
    }

// -----------------------------------------------------------------------------
// CFileManagerRefresher::Refresh
// Initiates the model refreshing
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CFileManagerRefresher::Refresh( TGflmRefreshMode aRefreshMode )
    {
    FUNC_LOG

    //  If already active, return
    if( IsActive() )
        {
        ERROR_LOG( "CFileManagerRefresher::Refresh()-Already active" )
        return;
        }

    TInt err( KErrNone );
    if ( iObserver )
        {
        TRAP( err, iObserver->RefreshStartedL() );
        }

    if ( err )
        {
        ERROR_LOG1( "CFileManagerRefresher::Refresh()-Before err=%d", err )
        return;
        }

    // Try to start the refresh operation
    TRAP( err, iModel.RefreshListL( iStatus, aRefreshMode ) );

    // If refreshing couldn't be started, notify observer and return
    if ( err )
        {
        ERROR_LOG1( "CFileManagerRefresher::Refresh()-After err=%d", err )
        if( iObserver )
            {
            TRAP_IGNORE( iObserver->RefreshStoppedL() );
            }
        return;
        }

    // Refreshing was started, set active and wait for asynchronous
    // request to complete
    SetActive();
    }

// ------------------------------------------------------------------------------
// CFileManagerRefresher::SetObserver
// ------------------------------------------------------------------------------
//
void CFileManagerRefresher::SetObserver( MFileManagerProcessObserver* aObserver )
    {
    // Set the observer if not active
    if( !IsActive() )
        {
        iObserver = aObserver;
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerRefresher::CancelRefresh
// ------------------------------------------------------------------------------
//
TBool CFileManagerRefresher::CancelRefresh()
    {
    FUNC_LOG

    TBool ret( IsActive() );
    Cancel();
    return ret;
    }

//  End of File
