/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  system state monitor
*
*/

#include <ssm/ssmstate.h>
#include <ssm/ssmdomaindefs.h>
#include "fmsystemstatemonitor.h"

// ========================= MEMBER FUNCTIONS ==================================

// -----------------------------------------------------------------------------
// CFmSystemStateMonitor::NewL()
// Two-phased constructor
// -----------------------------------------------------------------------------
CFmSystemStateMonitor* CFmSystemStateMonitor::NewL(
    MFmSystemStateMonitorObserver& aObserver )
    {
    CFmSystemStateMonitor* self = new ( ELeave ) CFmSystemStateMonitor( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFmSystemStateMonitor::~CFmSystemStateMonitor()
// Destructor.
// -----------------------------------------------------------------------------
//
CFmSystemStateMonitor::~CFmSystemStateMonitor()
    {
    Cancel();
    iSAS.Close();
    }

// -----------------------------------------------------------------------------
// CFmSystemStateMonitor::StartMonitor()
// Start monitor
// -----------------------------------------------------------------------------
//
void CFmSystemStateMonitor::StartMonitor( TUint aStateToMonitor )
    {
	if( !IsActive() )
	    {
	    iStateToMonitor = aStateToMonitor;
	    iSAS.RequestStateNotification( iStatus );
        SetActive();
	    }
    }   

// -----------------------------------------------------------------------------
// CFmSystemStateMonitor::IsSystemStateNormal()
// Indicate the system state normal or not
// -----------------------------------------------------------------------------
//
TBool CFmSystemStateMonitor::IsSystemStateNormal()
    {
    TSsmState state = iSAS.State();
    return ( state.MainState() == ESsmNormal );
    }

// -----------------------------------------------------------------------------
// CFmSystemStateMonitor::CFmSystemStateMonitor()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
CFmSystemStateMonitor::CFmSystemStateMonitor( MFmSystemStateMonitorObserver& aObserver ) :
        CActive( EPriorityStandard ),iObserver( aObserver )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CFmSystemStateMonitor::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFmSystemStateMonitor::ConstructL()
    {
    User::LeaveIfError( iSAS.Connect( KSM2GenMiddlewareDomain3 ) );
    }

// -----------------------------------------------------------------------------
// CFmSystemStateMonitor::RunL
// -----------------------------------------------------------------------------
//
void CFmSystemStateMonitor::RunL()
    {
    User::LeaveIfError( iStatus.Int() );
    	    
    TSsmState state = iSAS.State();
    if ( state.MainState() == iStateToMonitor )
        {
        iObserver.SystemStateChangedEvent();
        iSAS.AcknowledgeStateNotification( KErrNone );
        }
    else
        {
        iSAS.AcknowledgeAndRequestStateNotification( KErrNone, iStatus );
        SetActive();
        }        
    }

// -----------------------------------------------------------------------------
// CFmSystemStateMonitor::DoCancel()
// From CActive.
// -----------------------------------------------------------------------------
//
void CFmSystemStateMonitor::DoCancel()
    {
    iSAS.RequestStateNotificationCancel();
    }      

// -----------------------------------------------------------------------------
// CFmSystemStateMonitor::RunError()
// From CActive.
// -----------------------------------------------------------------------------
//
TInt CFmSystemStateMonitor::RunError( TInt aError )
    {
    return aError;
    }

// End of File

