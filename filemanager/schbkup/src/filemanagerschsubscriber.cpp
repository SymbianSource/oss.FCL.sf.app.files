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
* Description:  Handles P&S and CenRep notifications
*
*/



// INCLUDES
#include <centralrepository.h>
#include "filemanagerschsubscriber.h"
#include "filemanagerschobserver.h"
#include "FileManagerDebug.h"


// CONSTANTS
const TUint32 KMicroSecsPerSec = 1000000;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CFileManagerSchSubscriber::NewL()
// ---------------------------------------------------------------------------
//
CFileManagerSchSubscriber* CFileManagerSchSubscriber::NewL(
        MFileManagerSchObserver& aObserver,
        const TUid& aCategory,
        const TUint aKey,
        const TType aType,
        const TInt aTimeoutSecs )
    {
    CFileManagerSchSubscriber* self =
        new (ELeave)CFileManagerSchSubscriber(
            aObserver, aCategory, aKey, aType, aTimeoutSecs );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// ---------------------------------------------------------------------------
// CFileManagerSchSubscriber::ConstructL()
// ---------------------------------------------------------------------------
//
void CFileManagerSchSubscriber::ConstructL()
    {
    CActiveScheduler::Add( this );

    if ( iType == ESubscribePS )
        {
        User::LeaveIfError( iProperty.Attach( iCategory, iKey ) );
        }
    else
        {
        iCenRep = CRepository::NewL( iCategory );
        }

    SubscribeL();
    }

// ---------------------------------------------------------------------------
// CFileManagerSchSubscriber::SubscribeL()
// ---------------------------------------------------------------------------
//
void CFileManagerSchSubscriber::SubscribeL()
    {
    iTimeout = EFalse;

    if ( iType == ESubscribePS )
        {
        iProperty.Subscribe( iStatus );
        }
    else
        {
        User::LeaveIfError( iCenRep->NotifyRequest( iKey, iStatus ) );
        }

    if ( iTimeoutSecs > 0 )
        {
        delete iTimer;
        iTimer = NULL;

        iTimer = CPeriodic::NewL( EPriorityStandard );
        TUint32 us( iTimeoutSecs * KMicroSecsPerSec );
        iTimer->Start( us, us, TCallBack( TimeoutCB, this ) );
        }

    SetActive();
    }

// ---------------------------------------------------------------------------
// CFileManagerSchSubscriber::TimeoutCB()
// ---------------------------------------------------------------------------
//
TInt CFileManagerSchSubscriber::TimeoutCB( void* aPtr )
    {
    CFileManagerSchSubscriber* self =
        static_cast< CFileManagerSchSubscriber* >( aPtr );

    self->Timeout();

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CFileManagerSchSubscriber::Timeout()
// ---------------------------------------------------------------------------
//
void CFileManagerSchSubscriber::Timeout()
    {
    delete iTimer;
    iTimer = NULL;

    Cancel();

    // Set timeout status
    iTimeout = ETrue;

    // Complete
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// ---------------------------------------------------------------------------
// CFileManagerSchSubscriber::CFileManagerSchSubscriber()
// ---------------------------------------------------------------------------
//
CFileManagerSchSubscriber::CFileManagerSchSubscriber(
        MFileManagerSchObserver& aObserver,
        const TUid& aCategory,
        const TUint aKey,
        const TType aType,
        const TInt aTimeoutSecs ) :
    CActive( EPriorityStandard ),
    iObserver( aObserver ),
    iCategory( aCategory),
    iKey( aKey ),
    iType( aType ),
    iTimeoutSecs ( aTimeoutSecs )
    {
    }

// ---------------------------------------------------------------------------
// CFileManagerSchSubscriber::RunL()
// ---------------------------------------------------------------------------
//
void CFileManagerSchSubscriber::RunL()
    {
    // If timeout, no need to resubscribe
    if ( !iTimeout )
        {
        SubscribeL();
        }

    iObserver.NotifyKeyChangeOrTimeoutL( iCategory, iKey, iTimeout );
    }

// ---------------------------------------------------------------------------
// CFileManagerSchSubscriber::DoCancel()
// ---------------------------------------------------------------------------
//
void CFileManagerSchSubscriber::DoCancel()
    {
    delete iTimer;
    iTimer = NULL;

    if ( iType == ESubscribePS )
        {
        iProperty.Cancel();
        }
    else
        {
        iCenRep->NotifyCancel( iKey );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerSchSubscriber::RunError()
// ---------------------------------------------------------------------------
//
TInt CFileManagerSchSubscriber::RunError( TInt aError )
    {    
    ERROR_LOG1( "CFileManagerSchSubscriber::RunError()-Error=%d", aError )
    return aError;
    }

// ---------------------------------------------------------------------------
// CFileManagerSchSubscriber::~CFileManagerSchSubscriber()
// ---------------------------------------------------------------------------
//
CFileManagerSchSubscriber::~CFileManagerSchSubscriber()
    {
    Cancel();
    iProperty.Close();
    delete iCenRep;
    delete iTimer;
    }

// End of File


