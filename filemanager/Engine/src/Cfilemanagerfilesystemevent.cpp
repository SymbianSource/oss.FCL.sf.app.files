/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This AO is signaled when directory entry is added or removed 
*                from file system
*
*/



// INCLUDE FILES
#include <e32std.h>
#include "Cfilemanagerfilesystemevent.h"
#include "CFileManagerEngine.h"
#include "FileManagerDebug.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerFileSystemEvent::CFileManagerFileSystemEvent
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//

CFileManagerFileSystemEvent::CFileManagerFileSystemEvent(
        RFs& aFs,
        CFileManagerEngine& aEngine,
        TNotifyType aNotifyType ) :
    CActive( CActive::EPriorityLow ),
    iFs( aFs ),
    iEngine( aEngine),
    iNotifyType( aNotifyType )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerFileSystemEvent::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFileManagerFileSystemEvent::ConstructL( const TDesC& aFullPath )
    {
    CActiveScheduler::Add( this );

    if ( aFullPath.Length() )
        {
        iFullPath = aFullPath.AllocL();
        }

    Setup();
    }

// -----------------------------------------------------------------------------
// CFileManagerFileSystemEvent::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerFileSystemEvent* CFileManagerFileSystemEvent::NewL(
        RFs& aFs,
        CFileManagerEngine& aEngine,
        TNotifyType aNotifyType,
        const TDesC& aFullPath )
    {
    CFileManagerFileSystemEvent* self =
        new( ELeave ) CFileManagerFileSystemEvent( 
            aFs,
            aEngine,
            aNotifyType );

    CleanupStack::PushL( self );
    self->ConstructL( aFullPath );
    CleanupStack::Pop( self );

    return self;
    }

// ------------------------------------------------------------------------------
// CFileManagerFileSystemEvent::~CFileManagerFileSystemEvent
//
// ------------------------------------------------------------------------------
CFileManagerFileSystemEvent::~CFileManagerFileSystemEvent()
    {
    Cancel();
    delete iFullPath;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileSystemEvent::RunL
// From CActive. Called when asynchronous request is completed. 
// Notifies the observer
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CFileManagerFileSystemEvent::RunL()
    {
    TInt err( iStatus.Int() );

    LOG_IF_ERROR1( err, "CFileManagerFileSystemEvent::RunL-err=%d", err )

    // Mark as received if there is no error
    if ( err == KErrNone )
        {
        iIsReceived = ETrue;
        }

    // If not ENotifyDisk, do refresh when event is checked
    // using CheckFileSystemEvent
    if ( iNotifyType == ENotifyDisk )
        {
        iEngine.DriveAddedOrChangedL();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFileSystemEvent::RunError
// From CActive. Called when error occurred in asynchronous request
// Notifies the observer
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CFileManagerFileSystemEvent::RunError( TInt /*aError*/ )
    {
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileSystemEvent::DoCancel
// From CActive. Called when asynchronous request is canceled
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CFileManagerFileSystemEvent::DoCancel()
    {
    iFs.NotifyChangeCancel( iStatus );
    }

// ------------------------------------------------------------------------------
// CFileManagerFileSystemEvent::Setup
//
// ------------------------------------------------------------------------------
//
void CFileManagerFileSystemEvent::Setup()
    {
    if( IsActive() )
        {
        return;
        }

    iIsReceived = EFalse;

    if( iFullPath )
        {
        iFs.NotifyChange( iNotifyType, iStatus, *iFullPath );
        }
    else
        {
        iFs.NotifyChange( iNotifyType, iStatus );
        }

    SetActive();
    }

// ------------------------------------------------------------------------------
// CFileManagerFileSystemEvent::CheckFileSystemEvent
//
// ------------------------------------------------------------------------------
//
void CFileManagerFileSystemEvent::CheckFileSystemEvent()
    {
    if( iIsReceived )
        {
        TRAP_IGNORE( iEngine.FolderContentChangedL() );
        }
    }

//  End of File
