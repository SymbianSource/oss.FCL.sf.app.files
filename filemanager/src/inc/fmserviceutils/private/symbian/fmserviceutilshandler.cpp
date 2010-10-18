/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
* 
* Contributors:
*
* 
* Description:
*     The source file of service utils handler
*/



// INCLUDE FILES
#include "fmserviceutilshandler.h"
#include <coreapplicationuisdomainpskeys.h>
#include <coemain.h>
#include <apgwgnam.h>
#include <apgtask.h>
#include <bautils.h>
#include <tz.h>
#include <babackup.h>
#include <pathinfo.h>
#include <sysutil.h>
#include <driveinfo.h>
#include <e32property.h>
#include <centralrepository.h>

#include "fmutils.h"
#include <QSettings>
#include <XQConversions>

/*!
    constructor.
*/
CFmServiceUtilsHandler::CFmServiceUtilsHandler() :
    CActive( CActive::EPriorityStandard ), iObserver( 0 )
{
}

/*!
    Two-phased constructor.
*/
void CFmServiceUtilsHandler::ConstructL()
{
    CActiveScheduler::Add( this );
}

/*!
    Two-phased constructor.
*/
CFmServiceUtilsHandler* CFmServiceUtilsHandler::NewL()
    {
    RFs& fs( CCoeEnv::Static()->FsSession() );
    CFmServiceUtilsHandler* self =
        new( ELeave ) CFmServiceUtilsHandler();

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

/*!
    destructor.
*/
CFmServiceUtilsHandler::~CFmServiceUtilsHandler()
{
    Cancel();
    if( iBSWrapper ) {
        delete iBSWrapper;
        iBSWrapper = 0;
    }
}

/*
    \sa CActive
*/
void CFmServiceUtilsHandler::DoCancel()
{
}

/*
    \sa CActive
*/ 
void CFmServiceUtilsHandler::RunL()
{
    TInt err( iStatus.Int() );
    if( iObserver ) {
        TRAP_IGNORE( iObserver->handleCloseAppCompleteL( err ) );
    }
}

/*
    \sa CActive
*/
TInt CFmServiceUtilsHandler::RunError( TInt aError )
{
    if( iObserver ) {
        TRAP_IGNORE( iObserver->handleCloseAppCompleteL( aError ) );
        }
    return KErrNone;
}

/*!
    Call CBaBackupSessionWrapper to close apps
    this is synchronous which will take a while, please call this function in thread
*/
void CFmServiceUtilsHandler::CloseAppsL()
{
    if( iBSWrapper ) {
        delete iBSWrapper;
        iBSWrapper = 0;
    }

    iBSWrapper = CBaBackupSessionWrapper::NewL();

    TBackupOperationAttributes atts(
        MBackupObserver::EReleaseLockNoAccess,
        MBackupOperationObserver::EStart );
    iBSWrapper->NotifyBackupOperationL( atts );
    iBSWrapper->CloseAll( MBackupObserver::EReleaseLockNoAccess, iStatus );
    SetActive();
}

/*!
    Call CBaBackupSessionWrapper to restart closed apps
    this is synchronous which will return quickly.
*/
void CFmServiceUtilsHandler::RestartAppsL()
{
    if ( !iBSWrapper )
        {
        return;
        }
    
    TBackupOperationAttributes atts(
        MBackupObserver::ETakeLock, MBackupOperationObserver::EEnd );
    iBSWrapper->NotifyBackupOperationL( atts );
    iBSWrapper->RestartAll();
    
    // Get rid of the wrapper instance
    delete iBSWrapper;
    iBSWrapper = 0;
}

/*!
    set observer so that observer can be notified when operation is finished
*/
void CFmServiceUtilsHandler::setObserver( MServiceUtilsObserver *observer )
{
    iObserver = observer;
}

//  End of File  
