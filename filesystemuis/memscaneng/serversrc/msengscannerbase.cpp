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
*     Virtual base class for the scanner classes.
*
*/



// USER INCLUDES
#include    "msengscannerbase.h"


// ========================= MEMBER FUNCTIONS ================================
    
// ---------------------------------------------------------------------------
// CMsengScannerBase::CMsengScannerBase()
//
// C++ default constructor. Can NOT contain any code, that might leave.
// ---------------------------------------------------------------------------
CMsengScannerBase::CMsengScannerBase(MMsengScannerObserver& aObserver, 
                                     CMsengInfoArray& aInfoArray,
                                     RFs& aFsSession
                                     )
: CActive(CActive::EPriorityStandard), iObserver(aObserver),
  iInfoArray(aInfoArray), iFsSession(aFsSession)
    {
    CActiveScheduler::Add(this);
    }    

// ---------------------------------------------------------------------------
// CMsengScannerBase::~CMsengScannerBase()
//
// Destructor
// ---------------------------------------------------------------------------
CMsengScannerBase::~CMsengScannerBase()
    {
    Cancel();
    }




// ---------------------------------------------------------------------------
// CMsengScannerBase::CompleteRequest()
//
//
// ---------------------------------------------------------------------------
void CMsengScannerBase::CompleteRequest(TInt aCode)
    {
    if (!IsActive())
        {
        TRequestStatus* status = &iStatus;
        User::RequestComplete(status, aCode);
        SetActive();
        }
    }

// ---------------------------------------------------------------------------
// CMsengScannerBase::RunL()
//
//
// ---------------------------------------------------------------------------
void CMsengScannerBase::RunL()
    {
    // Do one scanning step
    const TStepResult result = PerformStepL();

    switch(result)
        {
        case ECompleteRequest:
            {
            CompleteRequest();
            break;
            }

        case EScanComplete:
            {
            iObserver.HandleScannerEventL(
                MMsengScannerObserver::EScannerEventScanComplete, *this);
            break;
            }
        
        case ERequestIssuedInternally:
        default:
            {
            // Not used, but maybe needed for messaging if some asynchronous
            // service is used which completes iStatus by itself.
            break;
            }
        }
    }

// ---------------------------------------------------------------------------
// CMsengScannerBase::DoCancel()
//
//
// ---------------------------------------------------------------------------
void CMsengScannerBase::DoCancel()
    {
    // Nothing to do here
    }


// ---------------------------------------------------------------------------
// CMsengScannerBase::RunError()
//
//
// ---------------------------------------------------------------------------
TInt CMsengScannerBase::RunError(TInt aError)
    {
    // Implementation for EScannerEventScanError cannot leave at the moment, 
    // but lets trap if implementation changes
    TRAP_IGNORE(iObserver.HandleScannerEventL(
        MMsengScannerObserver::EScannerEventScanError, *this, aError));
    return KErrNone;
    }


//  End of File  
