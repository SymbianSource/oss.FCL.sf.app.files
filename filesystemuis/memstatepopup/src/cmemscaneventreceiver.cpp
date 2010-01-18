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
* Description: 
*     Memory Scan Event Receiver
*
*
*/



// SYSTEM INCLUDES
#include <msenguihandler.h>


// USER INCLUDES
#include "cmemscaneventreceiver.h"


// ========================= MEMBER FUNCTIONS ================================


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::CMemScanEventReceiver()
//
// C++ default constructor.
// ---------------------------------------------------------------------------
CMemScanEventReceiver::CMemScanEventReceiver( MMsengUIHandler& aUIHandler )
: CActive(EPriorityStandard), iUIHandler(aUIHandler)
    {
    CActiveScheduler::Add(this);
    }


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::NewL()
//
// 
// ---------------------------------------------------------------------------    
CMemScanEventReceiver* CMemScanEventReceiver::NewL( MMsengUIHandler& aUIHandler )
    {
    CMemScanEventReceiver* self = new( ELeave ) CMemScanEventReceiver( aUIHandler );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::ConstructL()
//
// Destructor
// ---------------------------------------------------------------------------     
void CMemScanEventReceiver::ConstructL()
    {
    User::LeaveIfError( iScanClient.Connect() );
    }


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::~CMemScanEventReceiver()
//
//
// ---------------------------------------------------------------------------                           
CMemScanEventReceiver::~CMemScanEventReceiver()
    {
    Cancel();
    iScanClient.Close();
    }


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::RunL()
//
//
// --------------------------------------------------------------------------- 
void CMemScanEventReceiver::RunL()
    {
    User::LeaveIfError( iStatus.Int() );
    
    switch( iStatus.Int() )
        {
        case EMemScanEventScanningStarted:
            {
            iUIHandler.StartL();
            break;
            }

        case EMemScanEventScanningFinished:
            {
            iUIHandler.QuitL(iEventError);
            break;
            }

        case EMemScanEventScanningError:
            {
            iUIHandler.ErrorL(iEventError);
            break;
            }

        default:
            {
            break;
            }
        }
    
    RegisterForScanEvents();
    }


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::DoCancel()
//
//
// --------------------------------------------------------------------------- 
void CMemScanEventReceiver::DoCancel()
    {
    iScanClient.RequestScanEventsCancel();
    }


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::RunError()
//
//
// --------------------------------------------------------------------------- 
TInt CMemScanEventReceiver::RunError( TInt /*aError*/ )
    {
    RegisterForScanEvents();
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::DataGroupsL()
//
//
// --------------------------------------------------------------------------- 
CDesCArray* CMemScanEventReceiver::DataGroupsL() const
    {
    CDesCArray* dataGroupArray = iScanClient.DataGroupsL();
    return dataGroupArray;
    }


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::ScanResultL()
//
//
// ---------------------------------------------------------------------------      
CArrayFix<TInt64>* CMemScanEventReceiver::ScanResultL() const
    {
    CArrayFix<TInt64>* scanResultArray = iScanClient.ScanResultL();
    return scanResultArray;
    }


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::ScanInProgress()
//
//
// --------------------------------------------------------------------------- 
TBool CMemScanEventReceiver::ScanInProgress() const
    {
    return iScanClient.ScanInProgress();
    }


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::ScanL()
//
//
// --------------------------------------------------------------------------- 
TInt CMemScanEventReceiver::ScanL(TDriveNumber aDrive)
    {
    RegisterForScanEvents();

    return iScanClient.Scan(aDrive);
    }


// ---------------------------------------------------------------------------
// CMemScanEventReceiver::RegisterForScanEvents()
//
//
// --------------------------------------------------------------------------- 
void CMemScanEventReceiver::RegisterForScanEvents()
    {
    Cancel();

    // We request events from the server. When the server has an event
    // for us, it will complete our request status (iStatus) with an
    // event type (i.e. one of the TMemScanEvent enumerations) and
    // also ensures that iEventError is updated with any associated error
    // value. This will then cause the RunL method to be called.
    iScanClient.RequestScanEvents( iEventError, iStatus );
    SetActive();
    }
