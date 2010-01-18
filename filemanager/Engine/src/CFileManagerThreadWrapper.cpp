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
* Description:  Background thread functionality wrapper
*
*/



// INCLUDE FILES
#include "CFileManagerThreadWrapper.h"
#include "FileManagerDebug.h"


// ============================= MEMBER FUNCTIONS =============================

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::CFileManagerThreadWrapper()
//
// ----------------------------------------------------------------------------
CFileManagerThreadWrapper::CFileManagerThreadWrapper() :
    CActive( CActive::EPriorityStandard )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::~CFileManagerThreadWrapper()
//
// ----------------------------------------------------------------------------
CFileManagerThreadWrapper::~CFileManagerThreadWrapper()
    {
    Cancel();
    delete iNotifyObserver;
    iSemaphore.Close();
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::NewL()
//
// ----------------------------------------------------------------------------
CFileManagerThreadWrapper* CFileManagerThreadWrapper::NewL()
    {
    CFileManagerThreadWrapper* self =
        new (ELeave) CFileManagerThreadWrapper();

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::ConstructL()
//
// ----------------------------------------------------------------------------
void CFileManagerThreadWrapper::ConstructL()
    {
    CActiveScheduler::Add( this );
    User::LeaveIfError( iSemaphore.CreateLocal( 0 ) );
    iNotifyObserver = CNotifyObserver::NewL( *this );
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::RunL()
//
// ----------------------------------------------------------------------------
void CFileManagerThreadWrapper::RunL()
    {
    iNotifyObserver->Cancel();

    TInt err( iStatus.Int() );
    LOG_IF_ERROR1( err, "CFileManagerThreadWrapper::RunL()-err=%d", err )

    if ( iNotify & MFileManagerThreadFunction::ENotifyFinished )
        {
        TRAP_IGNORE( iFunction->NotifyThreadClientL(
            MFileManagerThreadFunction::ENotifyFinished, err ) );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::DoNotifyL()
//
// ----------------------------------------------------------------------------

void CFileManagerThreadWrapper::DoNotifyL( TInt aErr )
    {
    LOG_IF_ERROR1( aErr, "CFileManagerThreadWrapper::DoNotifyL()-aErr=%d", aErr )

    iResumePending = ETrue;

    if ( aErr != KErrNone &&
        ( iNotify & MFileManagerThreadFunction::ENotifyError ) )
        {
        iFunction->NotifyThreadClientL(
            MFileManagerThreadFunction::ENotifyError, aErr );
        }
    else if ( iNotify & MFileManagerThreadFunction::ENotifyStepFinished )
        {
        iFunction->NotifyThreadClientL(
            MFileManagerThreadFunction::ENotifyStepFinished, aErr );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::RunError()
//
// ----------------------------------------------------------------------------
TInt CFileManagerThreadWrapper::RunError( TInt aErr )
    {
    if ( aErr != KErrNone )
        {
        ERROR_LOG1( "CFileManagerThreadWrapper::RunError()-err=%d", aErr )
        iCancel = ETrue;
        ResumeThread();
        }
    return aErr;
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::DoCancel()
//
// ----------------------------------------------------------------------------
void CFileManagerThreadWrapper::DoCancel()
    {
    FUNC_LOG

    CancelThread();
    iSemaphore.Signal(); // To avoid deadlock
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::StartThread()
//
// ----------------------------------------------------------------------------
TInt CFileManagerThreadWrapper::StartThread(
        MFileManagerThreadFunction& aFunction,
        TUint aNotify,
        TThreadPriority aPriority )
    {
    FUNC_LOG

    TInt err( KErrAlreadyExists );

    if ( !IsActive() )
        {
        RThread thread;
        err = thread.Create(
            KNullDesC, ThreadFunction, KDefaultStackSize, NULL, this );
        if ( err == KErrNone )
            {
            thread.SetPriority( aPriority );
            thread.Logon( iStatus );

            iClientId = RThread().Id();
            iFunction = &aFunction;
            iNotify = aNotify;
            iCancel = EFalse;

            iNotifyObserver->Activate();
            SetActive();

            thread.Resume();
            thread.Close();
            }
        }

    LOG_IF_ERROR1( err, "CFileManagerThreadWrapper::StartThread()-err=%d",
        err )

    return err;
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::ThreadFunction()
//
// ----------------------------------------------------------------------------
TInt CFileManagerThreadWrapper::ThreadFunction( TAny* ptr )
    {
    FUNC_LOG

    CFileManagerThreadWrapper* self =
        static_cast< CFileManagerThreadWrapper* >( ptr );

    CTrapCleanup* cleanupStack = CTrapCleanup::New();
    if ( !cleanupStack )
        {
        return KErrNoMemory;
        }

    TRAPD( err, self->ThreadFunctionL() );

    self->iFunction->ReleaseThread();

    delete cleanupStack;

    return err;
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::ThreadFunctionL()
//
// ----------------------------------------------------------------------------
void CFileManagerThreadWrapper::ThreadFunctionL()
    {
    FUNC_LOG

    iFunction->InitThreadL();

    while ( !iCancel )
        {
        TRAPD( err, iFunction->ThreadStepL() );

        if ( !iCancel )
            {
            if ( err != KErrNone &&
                ( iNotify & MFileManagerThreadFunction::ENotifyError ) )
                {
                User::LeaveIfError( NotifyClientAndWaitConfirm( err ) );
                }
            else if ( iNotify &
                MFileManagerThreadFunction::ENotifyStepFinished )
                {
                User::LeaveIfError( NotifyClientAndWaitConfirm( err ) );
                }
            else
                {
                User::LeaveIfError( err );
                }
            }
        if ( iFunction->IsThreadDone() )
            {
            break;
            }
        }

    if ( iCancel )
        {
        User::LeaveIfError( KErrCancel );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::CancelThread()
//
// ----------------------------------------------------------------------------
void CFileManagerThreadWrapper::CancelThread()
    {
    FUNC_LOG

    iCancel = ETrue;
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::IsThreadCanceled()
//
// ----------------------------------------------------------------------------
TBool CFileManagerThreadWrapper::IsThreadCanceled() const
    {
    return iCancel;
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::NotifyClientAndWaitConfirm()
//
// ----------------------------------------------------------------------------
TInt CFileManagerThreadWrapper::NotifyClientAndWaitConfirm( TInt aErr )
    {
    FUNC_LOG

    RThread client;
    TInt err( client.Open( iClientId ) );

    if ( err == KErrNone )
        {
        iNotifyObserver->Complete( client, aErr );
        client.Close();
        iSemaphore.Wait(); // Wait resume from client
        }
    return err;
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::ResumeThread()
//
// ----------------------------------------------------------------------------
void CFileManagerThreadWrapper::ResumeThread()
    {
    FUNC_LOG

    if ( !iNotifyObserver->IsActive() && iResumePending )
        {
        iResumePending = EFalse;
        iNotifyObserver->Activate();
        iSemaphore.Signal(); // Resume thread stepping
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::IsThreadStarted()
//
// ----------------------------------------------------------------------------
TBool CFileManagerThreadWrapper::IsThreadStarted() const
    {
    return IsActive();
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::CNotifyObserver::CNotifyObserver()
//
// ----------------------------------------------------------------------------
CFileManagerThreadWrapper::CNotifyObserver::CNotifyObserver(
    CFileManagerThreadWrapper& aWrapper ) :
        CActive( CActive::EPriorityStandard ),
        iWrapper( aWrapper )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::CNotifyObserver::NewL()
//
// ----------------------------------------------------------------------------
CFileManagerThreadWrapper::CNotifyObserver*
    CFileManagerThreadWrapper::CNotifyObserver::NewL(
        CFileManagerThreadWrapper& aWrapper )
    {
    CNotifyObserver* self = new (ELeave) CNotifyObserver( aWrapper );
    CActiveScheduler::Add( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::CNotifyObserver::~CNotifyObserver()
//
// ----------------------------------------------------------------------------
CFileManagerThreadWrapper::CNotifyObserver::~CNotifyObserver()
    {
    Cancel();
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::CNotifyObserver::RunL()
//
// ----------------------------------------------------------------------------
void CFileManagerThreadWrapper::CNotifyObserver::RunL()
    {
    iWrapper.DoNotifyL( iStatus.Int() );
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::CNotifyObserver::RunError()
//
// ----------------------------------------------------------------------------
TInt CFileManagerThreadWrapper::CNotifyObserver::RunError( TInt aErr )
    {
    return iWrapper.RunError( aErr );
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::CNotifyObserver::DoCancel()
//
// ----------------------------------------------------------------------------
void CFileManagerThreadWrapper::CNotifyObserver::DoCancel()
    {
    // Just complete status immediately since
    // background thread does not complete status anymore
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrCancel );
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::CNotifyObserver::Activate()
//
// ----------------------------------------------------------------------------
void CFileManagerThreadWrapper::CNotifyObserver::Activate()
    {
    if ( !IsActive() )
        {
        iStatus = KRequestPending;
        SetActive();
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerThreadWrapper::CNotifyObserver::Complete()
//
// ----------------------------------------------------------------------------
void CFileManagerThreadWrapper::CNotifyObserver::Complete(
    RThread& aThread, TInt aResult )
    {
    if ( IsActive() )
        {
        TRequestStatus* status = &iStatus;
        aThread.RequestComplete( status, aResult );
        }
    }

// End of File  
