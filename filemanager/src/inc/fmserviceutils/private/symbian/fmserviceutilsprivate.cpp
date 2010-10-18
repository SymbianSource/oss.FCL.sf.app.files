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
*     The source file of service utils private
*/

#include "fmserviceutilsprivate.h"
#include "fmserviceutilshandler.h"
#include "fmcommon.h"
#include <shareui.h>
#include <afactivitystorage.h>

// CONSTANTS
const int KAppCloseMiniSecTimeout = 1000;

FmServiceUtilsPrivate::FmServiceUtilsPrivate( QObject *parent ) : QObject( parent ),
    mShareUi( 0 ), mServiceUtilsHandler( 0 ), mActivityStorage( 0 ),
    mIsCloseAppsFinished( false ), mIsCloseAppsTimeup( false )
{
    connect(&mCloseAppTimer, SIGNAL(timeout()), this, SLOT(onCloseAppTimeup()));
}

FmServiceUtilsPrivate::~FmServiceUtilsPrivate()
{
    if( mShareUi ) {
        delete mShareUi;
        mShareUi = 0;
    }
    
    if( mServiceUtilsHandler ) {
        delete mServiceUtilsHandler;
        mServiceUtilsHandler = 0;
    }
    
    if( mActivityStorage ) {
        delete mActivityStorage;
        mActivityStorage = 0;
    }
}

/*
   return \a shareUi which is used to send files. 
*/
ShareUi *FmServiceUtilsPrivate::shareUi()
{
    if( !mShareUi ) {
        mShareUi = new ShareUi;
    }
    return mShareUi;
}

/*
   return \a AfActivityStorage which is used to save activity. 
*/
AfActivityStorage *FmServiceUtilsPrivate::activityStorage()
{
    if( !mActivityStorage ) {
        mActivityStorage = new AfActivityStorage;
    }
    return mActivityStorage;
}

/*
   return \a CFmServiceUtilsHandler which is used to closeApps and restartApps
*/
CFmServiceUtilsHandler *FmServiceUtilsPrivate::serviceUtilsHandler()
{
    if( !mServiceUtilsHandler ) {
        TRAPD(err, mServiceUtilsHandler = CFmServiceUtilsHandler::NewL());
        if( err == KErrNone ) {
            mServiceUtilsHandler->setObserver( this );
        } else {
            mServiceUtilsHandler = 0;
        }
    }
    return mServiceUtilsHandler;
}

/*!
    send files that included in \a filePathList via ShareUi
*/
void FmServiceUtilsPrivate::sendFile( const QStringList &filePathList )
{
    // send do not accept const QStringList, so use QStringList()<<filePathList
    shareUi()->send( QStringList()<<filePathList, true );
}

/*!
    call CBaBackupSessionWrapper to close apps, for example, before formant, apps need be closed
    this is synchronous which will take a while, please call this function in thread
*/
void FmServiceUtilsPrivate::closeApps()
{
    CFmServiceUtilsHandler *utilsHandler = serviceUtilsHandler();
    mIsCloseAppsFinished = false;
    mIsCloseAppsTimeup = false;
    if( utilsHandler ) {
        TRAP_IGNORE( utilsHandler->CloseAppsL() );
    }
    
    // Memory card formatting cannot be executed if there are open files on it.
    // It has been detected, that in some cases memory card using applications 
    // have no time to close file handles before formatting is tried to be executed. 
    // To address this issue, we need to add a delay here after client-notification 
    // about pending format and real formatting procedure.
    mCloseAppTimer.start( KAppCloseMiniSecTimeout );
    
    //loop will be closed on both mIsCloseAppsFinished and mIsCloseAppsTimeup turned to true
    mCloseAppLoop.exec();
}

/*
    This is observer function inherit from MServiceUtilsObserver
    So end with L letter no matter if it will leave
    handle close apps complete, exit loop if mIsCloseAppsTimeup is true
*/
void FmServiceUtilsPrivate::handleCloseAppCompleteL( TInt err )
{
    Q_UNUSED( err );
    mIsCloseAppsFinished = true;
    if( mIsCloseAppsTimeup ) {
        mCloseAppLoop.exit();
    }
}

/*
    handle close apps time up. exit loop if mIsCloseAppsFinished is true
*/
void FmServiceUtilsPrivate::onCloseAppTimeup()
{
    mCloseAppTimer.stop();
    mIsCloseAppsTimeup = true;
    if( mIsCloseAppsFinished ) {
        mCloseAppLoop.exit();
    }
}

/*!
    call CBaBackupSessionWrapper to restart apps
    for example, after formant, closed apps need be restarted
    this is synchronous which will return quickly.
*/
void FmServiceUtilsPrivate::restartApps()
{
    CFmServiceUtilsHandler *utilsHandler = serviceUtilsHandler();
    if( utilsHandler ) {
        TRAP_IGNORE( serviceUtilsHandler()->RestartAppsL() );
    }
}

/*!
    call AfActivityStorage to save activity
*/
bool FmServiceUtilsPrivate::saveActivity(const QString &activityId, const QVariant &activityData, const QVariantHash &metadata)
{
    return activityStorage()->saveActivity( activityId, activityData, metadata);
}

/*!
    call AfActivityStorage to remove activity
*/
bool FmServiceUtilsPrivate::removeActivity(const QString &activityId)
{
    return activityStorage()->removeActivity( activityId );
}
