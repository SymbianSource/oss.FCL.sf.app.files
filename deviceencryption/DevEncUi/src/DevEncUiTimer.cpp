/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/

#include "DevEncUiTimer.h"
#include "DevEncLog.h"

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CDevEncUiTimer::NewL
// Instancies CDevEncUiTimer object
// ----------------------------------------------------------
//
CDevEncUiTimer* CDevEncUiTimer::NewL( MDevEncUiTimerCallback* aCallback )
    {
    CDevEncUiTimer* self = new ( ELeave ) CDevEncUiTimer( aCallback );
    CleanupStack::PushL ( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------
// CDevEncUiTimer::ConstructL()
// Initializes data objects
// ----------------------------------------------------------
//
void CDevEncUiTimer::ConstructL()
    {
    CTimer::ConstructL();
    }

// ----------------------------------------------------------
// CDevEncUiTimer::CDevEncUiTimer()
// Constructor
// ----------------------------------------------------------
//
CDevEncUiTimer::CDevEncUiTimer( MDevEncUiTimerCallback* aCallback )
    : CTimer( EPriorityStandard ),
      iCallback( aCallback )
    {
    CActiveScheduler::Add( this );
    }

// ----------------------------------------------------------
// CDevEncUiTimer::CDevEncUiTimer()
// Destructor
// ----------------------------------------------------------
//
CDevEncUiTimer::~CDevEncUiTimer()
    {
    Cancel();
    }

// ----------------------------------------------------------
// CDevEncUiTimer::RunError()
// CActive Object method
// ----------------------------------------------------------
//
TInt CDevEncUiTimer::RunError( TInt aError )
    {
    return aError;
    }

// ----------------------------------------------------------
// CDevEncUiTimer::RunL()
// CActive Object method
// ----------------------------------------------------------
//
void CDevEncUiTimer::RunL()
    {
    DFLOG2( "CDevEncUiTimer::RunL, status %d", iStatus.Int()  );
    if ( iStatus == KErrNone )
        {
        iCallback->Timeout();
        }
    }

// ----------------------------------------------------------
// CDevEncUiTimer::DoCancel()
// CActive Object method
// ----------------------------------------------------------
//
void CDevEncUiTimer::DoCancel()
    {
    }

// End of file
