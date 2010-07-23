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
* Description:
*     The source file of the operation thread of file manager
*
*/

#include "fmoperationthread.h"
#include "fmoperationbase.h"
#include "fmdrivedetailstype.h"
#include "fmcommon.h"
#include "fmoperationcopy.h"
#include "fmoperationmove.h"
#include "fmoperationremove.h"
#include "fmoperationformat.h"

#include <QDir>
#include <QStack>

FmOperationThread::FmOperationThread( QObject *parent ) : QThread( parent ), mOperationBase( 0 )
{

    
}

FmOperationThread::~FmOperationThread()
{
}

int FmOperationThread::asyncCopy( FmOperationBase* operationBase )
{
    if( isRunning() ){
        return FmErrAlreadyStarted;
    }

    mOperationBase = operationBase;
    mOperationBase->setObjectName( "operationElement" );
    QMetaObject::connectSlotsByName( this );
    connect( mOperationBase, SIGNAL( askForRename( QString, QString* ) ),
        this, SLOT( onAskForRename( QString, QString* )), Qt::BlockingQueuedConnection );
    connect( mOperationBase, SIGNAL( askForReplace( QString, QString, bool* ) ),
        this, SLOT( onAskForReplace( QString, QString, bool* )), Qt::BlockingQueuedConnection );
    connect( mOperationBase, SIGNAL( showNote( QString ) ),
        this, SLOT( onShowNote( QString )), Qt::BlockingQueuedConnection );

    start();
    return FmErrNone;
}

int FmOperationThread::asyncMove( FmOperationBase* operationBase )
{
    if( isRunning() ){
        return FmErrAlreadyStarted;
    }

    mOperationBase = operationBase;
    mOperationBase->setObjectName( "operationElement" );
    QMetaObject::connectSlotsByName( this );
    connect( mOperationBase, SIGNAL( askForRename( QString, QString* ) ),
        this, SLOT( onAskForRename( QString, QString* )), Qt::BlockingQueuedConnection );
    connect( mOperationBase, SIGNAL( askForReplace( QString, QString, bool* ) ),
        this, SLOT( onAskForReplace( QString, QString, bool* )), Qt::BlockingQueuedConnection );
    connect( mOperationBase, SIGNAL( showNote( QString ) ),
        this, SLOT( onShowNote( QString )), Qt::BlockingQueuedConnection );


    start();
    return FmErrNone;
}

int FmOperationThread::asyncRemove( FmOperationBase* operationBase )
{
    if( isRunning() ){
        return FmErrAlreadyStarted;
    }

    mOperationBase = operationBase;
    mOperationBase->setObjectName( "operationElement" );
    QMetaObject::connectSlotsByName( this );

    start();
    return FmErrNone;
}

int FmOperationThread::asyncFormat( FmOperationBase* operationBase )
{
    if( isRunning() ){
        return FmErrAlreadyStarted;
    }

    mOperationBase = operationBase;
    mOperationBase->setObjectName( "operationElement" );
    QMetaObject::connectSlotsByName( this );

    start();
    return FmErrNone;
}
int FmOperationThread::asyncViewDriveDetails( FmOperationBase* operationBase )
{
     if( isRunning() ){
        return FmErrAlreadyStarted;
    }

    mOperationBase = operationBase;
    mOperationBase->setObjectName( "operationElement" );
    QMetaObject::connectSlotsByName( this );

    start();
    return FmErrNone;
}

int FmOperationThread::asyncViewFolderDetails( FmOperationBase* operationBase )
{
    if( isRunning() ){
        return FmErrAlreadyStarted;
    }

    mOperationBase = operationBase;
    mOperationBase->setObjectName( "operationElement" );
    QMetaObject::connectSlotsByName( this );

    start();
    return FmErrNone;
}


void FmOperationThread::stop()
{
    mStop = true;
}

void FmOperationThread::onAskForRename( const QString &srcFile, QString *destFile )
{
    emit askForRename( srcFile, destFile );
}
void FmOperationThread::onAskForReplace( const QString &srcFile, const QString &destFile, bool *isAccepted )
{
    emit askForReplace( srcFile, destFile, isAccepted );
}

void FmOperationThread::onShowNote( const char *noteString )
{
    emit showNote( noteString );
}

void FmOperationThread::on_operationElement_notifyPreparing( bool cancelable )
{
    emit notifyPreparing( cancelable );
}
void FmOperationThread::on_operationElement_notifyStart( bool cancelable, int maxSteps )
{
    emit notifyStart( cancelable, maxSteps );
}
void FmOperationThread::on_operationElement_notifyProgress( int currentStep )
{
    emit notifyProgress( currentStep );
}

void FmOperationThread::run()
{
    mStop = false;
    this->setPriority( LowestPriority );
    switch( mOperationBase->operationType() )
    {
    case FmOperationService::EOperationTypeCopy:
        {
        mErrString.clear();
        FmOperationCopy *operationCopy = static_cast<FmOperationCopy*>(mOperationBase);
        
        int ret = operationCopy->start( &mStop, &mErrString );
        switch( ret )
        {
        case FmErrCancel:
            emit notifyCanceled();
            break;
        case FmErrNone:
            emit notifyFinish();
            break;
        default:
            emit notifyError( ret, mErrString );
            break;
        }
        // refresh drive space no care if cancel, error or finished.
        // as filemanger cannot notify drive space changed
        // do not refresh path as QFileSystemModel will do auto-refresh
        emit driveSpaceChanged();
        break;
        }
    case FmOperationService::EOperationTypeMove:
        {
        mErrString.clear();
        FmOperationMove *operationMove = static_cast<FmOperationMove*>(mOperationBase);
        
        int ret = operationMove->start( &mStop, &mErrString );
        switch( ret )
        {
        case FmErrCancel:
            emit notifyCanceled();
            break;
        case FmErrNone:
            emit notifyFinish();
            break;
        default:
            emit notifyError( ret, mErrString );
        }
        // refresh drive space no care if cancel, error or finished.
        // as filemanger cannot notify drive space changed
        // do not refresh path as QFileSystemModel will do auto-refresh
        emit driveSpaceChanged();
        break;
        }
    case FmOperationService::EOperationTypeRemove:
        {
        mErrString.clear();
        FmOperationRemove *operationRemove = static_cast<FmOperationRemove*>(mOperationBase);
        
        int ret = operationRemove->start( &mStop, &mErrString );
        switch( ret )
        {
        case FmErrCancel:
            emit notifyCanceled();
            break;
        case FmErrNone:
            emit notifyFinish();
            break;
        default:
            emit notifyError( ret, mErrString );
        }
        // refresh drive space no care if cancel, error or finished.
        // as filemanger cannot notify drive space changed
        // do not refresh path as QFileSystemModel will do auto-refresh
        emit driveSpaceChanged();
        break;
        }
    case FmOperationService::EOperationTypeFormat:
        {
//        emit notifyWaiting( false );
        FM_LOG(QString("start format"));
        FmOperationFormat *operationFormat = static_cast<FmOperationFormat*>( mOperationBase );
        FM_LOG(QString("get param and start format"));

        if ( FmErrNone != operationFormat->start() ) {
            emit notifyError(  FmErrTypeFormatFailed, operationFormat->driverName() );
            return;
        }
        FM_LOG(QString("format done"));
        emit notifyFinish();
        // refresh drive space no care if cancel, error or finished.
        // as filemanger cannot notify drive space changed
        // do not refresh path as QFileSystemModel will do auto-refresh
        emit driveSpaceChanged();
        FM_LOG(QString("format done and emit finish"));
        break;
        }
    case FmOperationService::EOperationTypeDriveDetails:
        {
            emit notifyWaiting( true );

            FmOperationDriveDetails *operationDriverDetails = static_cast<FmOperationDriveDetails*>( mOperationBase );
            int ret = FmDriveDetailsContent::querySizeofContent(
                operationDriverDetails->driverName(), operationDriverDetails->detailsSizeList(), &mStop );
            if( ret == FmErrNone ) {
                emit notifyFinish();
            } else if( ret == FmErrCancel ) {
                emit notifyCanceled();
            }

            break;
        }
    case FmOperationService::EOperationTypeFolderDetails:
        {
            emit notifyWaiting( true );

            FmOperationFolderDetails *operationFolderDetails = static_cast<FmOperationFolderDetails*>( mOperationBase );
            int ret = FmFolderDetails::getNumofSubfolders( operationFolderDetails->folderPath(), operationFolderDetails->numofSubFolders(), 
                                                           operationFolderDetails->numofFiles(), operationFolderDetails->sizeofFolder(), 
                                                           &mStop );
            if( ret == FmErrNone ) {
                emit notifyFinish();
            } else if( ret == FmErrCancel ) {
                emit notifyCanceled();
            }

            break;
        
        }
    default:
        Q_ASSERT( false );
        
    }
}

