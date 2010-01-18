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
* Description:  Subscriber (Publish & Subscribe)
*
*/


// INCLUDES
#include "CFileManagerPropertySubscriber.h"
#include "FileManagerDebug.h"

// ============================= MEMBER FUNCTIONS =============================

// ----------------------------------------------------------------------------
// CFileManagerPropertySubscriber::NewL()
//
// ----------------------------------------------------------------------------
CFileManagerPropertySubscriber* CFileManagerPropertySubscriber::NewL(
        MFileManagerPropertyObserver& aObserver,
        const TUid& aCategory,
        const TUint aKey )
    {
    CFileManagerPropertySubscriber* self = new (ELeave)
        CFileManagerPropertySubscriber(
            aObserver, aCategory, aKey );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerPropertySubscriber::ConstructL()
//
// ----------------------------------------------------------------------------
void CFileManagerPropertySubscriber::ConstructL()
    {
    CActiveScheduler::Add( this );
    User::LeaveIfError( iProperty.Attach( iCategory, iKey ) );
    Subscribe();
    }

// ----------------------------------------------------------------------------
// CFileManagerPropertySubscriber::Subscribe()
//
// ----------------------------------------------------------------------------
void CFileManagerPropertySubscriber::Subscribe()
    {
    iProperty.Subscribe( iStatus );
    SetActive();
    }

// ----------------------------------------------------------------------------
// CFileManagerPropertySubscriber::CFileManagerPropertySubscriber()
//
// ----------------------------------------------------------------------------
CFileManagerPropertySubscriber::CFileManagerPropertySubscriber(
        MFileManagerPropertyObserver& aObserver,
        const TUid& aCategory,
        const TUint aKey ) :
    CActive( EPriorityStandard ),
    iObserver( aObserver ),
    iCategory( aCategory),
    iKey( aKey )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerPropertySubscriber::RunL()
//
// ----------------------------------------------------------------------------
void CFileManagerPropertySubscriber::RunL()
    {
    Subscribe();
    iObserver.PropertyChangedL( iCategory, iKey );
    }

// ----------------------------------------------------------------------------
// CFileManagerPropertySubscriber::DoCancel()
//
// ----------------------------------------------------------------------------
void CFileManagerPropertySubscriber::DoCancel()
    {
    iProperty.Cancel();
    }

// ----------------------------------------------------------------------------
// CFileManagerPropertySubscriber::RunError()
//
// ----------------------------------------------------------------------------
TInt CFileManagerPropertySubscriber::RunError( TInt aError )
    {    
    ERROR_LOG1( "CFileManagerPropertySubscriber::RunError()-error=%d", aError )
    return aError;
    }

// ----------------------------------------------------------------------------
// CFileManagerPropertySubscriber::~CFileManagerPropertySubscriber()
//
// ----------------------------------------------------------------------------
CFileManagerPropertySubscriber::~CFileManagerPropertySubscriber()
    {
    Cancel();
    iProperty.Close();
    }

// End of File


