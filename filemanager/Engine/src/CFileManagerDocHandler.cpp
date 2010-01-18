/*
* Copyright (c) 2004-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Wraps document handler functions
*
*/



// INCLUDES
#include <DocumentHandler.h>
#include <apmstd.h>
#include <bautils.h>
#include "CFileManagerDocHandler.h"
#include "CFileManagerEngine.h"
#include "CFileManagerUtils.h"
#include "CFileManagerThreadWrapper.h"
#include "MFileManagerProcessObserver.h"
#include "FileManagerDebug.h"


// -----------------------------------------------------------------------------
// CFileManagerDocHandler::CFileManagerDocHandler()
//
// -----------------------------------------------------------------------------
//
CFileManagerDocHandler::CFileManagerDocHandler(
        CFileManagerEngine& aEngine,
        CFileManagerUtils& aUtils ) :
    iEngine( aEngine ),
    iUtils( aUtils ),
    iFs( aEngine.Fs() )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerDocHandler::NewL()
//
// ------------------------------------------------------------------------------
//
CFileManagerDocHandler* CFileManagerDocHandler::NewL(
        CFileManagerEngine& aEngine,
        CFileManagerUtils& aUtils )
    {
    CFileManagerDocHandler* self = new (ELeave) CFileManagerDocHandler(
        aEngine, aUtils );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerDocHandler::~CFileManagerDocHandler()
//
// ------------------------------------------------------------------------------
//
CFileManagerDocHandler::~CFileManagerDocHandler()
    {
    iApaSession.Close();
    delete iThreadWrapper;
    delete iFileMime;
    delete iFileFullPath;
    delete iDocHandler;
    iFile.Close();
    }

// -----------------------------------------------------------------------------
// CFileManagerDocHandler::HandleServerAppExit()
// 
// -----------------------------------------------------------------------------
void CFileManagerDocHandler::HandleServerAppExit( TInt aReason )
    {
    if ( iEmbeddedAppOpen )
        {
        iEngine.NoticeServerAppExit( aReason );
        iEngine.EmbeddedAppStatus( EFalse );
        iEmbeddedAppOpen = EFalse;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerDocHandler::ConstructL()
//
// -----------------------------------------------------------------------------
//
void CFileManagerDocHandler::ConstructL()
    {
    iDocHandler = CDocumentHandler::NewL();
    iDocHandler->SetExitObserver( this );
    User::LeaveIfError( iApaSession.Connect() );
    }

// -----------------------------------------------------------------------------
// CFileManagerDocHandler::OpenFileL()
//
// -----------------------------------------------------------------------------
//
void CFileManagerDocHandler::OpenFileL(
        const TDesC& aFullPath, MFileManagerProcessObserver* aObserver )
    {
    if ( iEmbeddedAppOpen )
        {
        ERROR_LOG( "CFileManagerDocHandler::OpenFileL-OpenAlreadyOngoing" )
        return; // Ignore too fast open events to prevent mess up
        }

    delete iThreadWrapper;
    iThreadWrapper = NULL;

    delete iFileFullPath;
    iFileFullPath = NULL;

    delete iFileMime;
    iFileMime = NULL;

    iFileReady = EFalse;
    iFile.Close();

    iObserver = aObserver;

    //Judge the file extention, if a sis file, notify Engine to flush the cache
    if ( EFileManagerAppFileIcon == iUtils.ResolveIconL( aFullPath ) )
        {
        iEngine.SetSisFile( ETrue );
        }

    // Get fullpath and MIME type
    TPtrC mime16( iUtils.MimeTypeL( aFullPath ) );
    iFileMime = HBufC8::NewL( mime16.Length() );
    iFileMime->Des().Copy( mime16 );
    iFileFullPath = aFullPath.AllocL();

    if ( iUtils.IsRemoteDrive( *iFileFullPath ) )
        {
        iThreadWrapper = CFileManagerThreadWrapper::NewL();
        User::LeaveIfError( iThreadWrapper->StartThread(
            *this,
            MFileManagerThreadFunction::ENotifyFinished, EPriorityNormal ) );

        // Async file open
        if ( iObserver )
            {
            TRAP_IGNORE( iObserver->ProcessStartedL(
                MFileManagerProcessObserver::EFileOpenProcess, KErrNone ) );
            }
        }
    else
        {
        // Sync file open
        if ( iObserver )
            {
            TRAP_IGNORE( iObserver->ProcessStartedL(
                MFileManagerProcessObserver::EFileOpenProcess, KErrNone ) );
            }

        RFile64 sharableFile;
        TRAPD( err, iDocHandler->OpenTempFileL( *iFileFullPath, sharableFile ) );
        if ( err == KErrNone )
            {
            TRAP( err, OpenShareableFileL( sharableFile, *iFileMime ) );
            }
        sharableFile.Close();

        if ( iObserver )
            {
            TRAP_IGNORE( iObserver->ProcessFinishedL( err ) );
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerDocHandler::OpenShareableFileL()
//
// -----------------------------------------------------------------------------
//
void CFileManagerDocHandler::OpenShareableFileL(
        RFile64& aShareableFile, const TDesC8& aMime )
    {
    // Check if file open is denied
    if ( iObserver && iFileFullPath )
        {
        if ( iObserver->NotifyL(
                MFileManagerProcessObserver::ENotifyFileOpenDenied,
                0,
                *iFileFullPath ) )
            {
            return;
            }
        }

    TDataType dataType( aMime );
    CAiwGenericParamList& paramList( iDocHandler->InParamListL() );

    // If remote drive, make sure that file can be opened remotely
    if ( iUtils.IsRemoteDrive( *iFileFullPath ) )
        {
        TUid appUid;
        TDataType fileType;
        User::LeaveIfError( iApaSession.AppForDocument(
            aShareableFile, appUid, fileType ) );
        if ( !iEngine.HasAppRemoteDriveSupport( appUid ) )
            {
            User::Leave( KErrFmgrNotSupportedRemotely );
            }
        dataType = fileType;
        }
    // Otherwise just make sure that file exists before opening it
    else if ( !BaflUtils::FileExists( iEngine.Fs(), *iFileFullPath ) )
        {
        User::Leave( KErrNotFound );
        }

    TInt err( KErrNone );
    TInt err2( KErrNone );

    TRAP( err, err2 = iDocHandler->OpenFileEmbeddedL(
        aShareableFile, dataType, paramList ) );

    LOG_IF_ERROR1( err,
        "CFileManagerDocHandler::OpenShareableFileL-err=%d", err )
    LOG_IF_ERROR1( err2,
        "CFileManagerDocHandler::OpenShareableFileL-err2=%d", err2 )

    // Make sure that not supported functionality is handled properly
    if ( err2 == KErrNotSupported || err2 == KMimeNotSupported ||
         err == KErrNotSupported || err == KMimeNotSupported )
        {
        User::Leave( KErrNotSupported );
        }
    else if ( err == KErrNotFound || err2 == KErrNotFound )
        {
        User::Leave( KErrGeneral );
        }
    User::LeaveIfError( err );
    User::LeaveIfError( err2 );

    iEngine.EmbeddedAppStatus( ETrue );
    iEmbeddedAppOpen = ETrue;
    }

// -----------------------------------------------------------------------------
// CFileManagerDocHandler::ThreadStepL()
//
// -----------------------------------------------------------------------------
//
void CFileManagerDocHandler::ThreadStepL()
    {
    // Open shareable file and make sure that it is loaded to file cache
    // before opening the file with app
    TInt err( iFile.Open( iFs, *iFileFullPath, EFileShareReadersOnly ) );
    if ( err == KErrInUse )
        {
        User::LeaveIfError( iFile.Open(
            iFs, *iFileFullPath, EFileShareReadersOrWriters ) );
        }
    else
        {
        User::LeaveIfError( err );
        }

    TInt64 pos( 0 );
    TInt64 size( 0 );
    TInt readSize( 0 );
    const TInt KReadBufferSize = 16384; // 16KB
    HBufC8* buffer = HBufC8::NewLC( KReadBufferSize );
    TPtr8 ptr( buffer->Des() );

    User::LeaveIfError( iFile.Size( size ) );
    while ( size > 0 )
        {
        if ( iThreadWrapper->IsThreadCanceled() )
            {
            User::Leave( KErrCancel );
            }

        readSize = Min( size, ptr.MaxSize() );
        User::LeaveIfError( iFile.Read( pos, ptr, readSize ) );
        size -= readSize;
        }

    CleanupStack::PopAndDestroy( buffer );
    iFileReady = ETrue;
    }

// -----------------------------------------------------------------------------
// CFileManagerDocHandler::IsThreadDone()
//
// -----------------------------------------------------------------------------
//
TBool CFileManagerDocHandler::IsThreadDone()
    {
    return iFileReady;
    }

// -----------------------------------------------------------------------------
// CFileManagerDocHandler::NotifyThreadClientL()
//
// -----------------------------------------------------------------------------
//
void CFileManagerDocHandler::NotifyThreadClientL(
        TNotifyType aType, TInt aValue )
    {
    switch ( aType )
        {
        case ENotifyFinished:
            {
            // Open shareable file with app and inform observer
            if ( aValue == KErrNone && iFileReady )
                {
                TRAP( aValue, OpenShareableFileL( iFile, *iFileMime ) );
                }
            iFile.Close();

            if ( iObserver )
                {
                TRAP_IGNORE( iObserver->ProcessFinishedL( aValue ) );
                }
            if ( aValue != KErrNone )
                {
                // Ensure that the status gets updated on error
                HandleServerAppExit( 0 );
                }
            break;
            }
        default:
            {
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerDocHandler::CancelFileOpen()
//
// -----------------------------------------------------------------------------
//
void CFileManagerDocHandler::CancelFileOpen()
    {
    if ( iFileFullPath )
        {
        iEngine.CancelTransfer( *iFileFullPath );
        }
    if ( iThreadWrapper )
        {
        iThreadWrapper->CancelThread();
        }
    }

// End of File  
