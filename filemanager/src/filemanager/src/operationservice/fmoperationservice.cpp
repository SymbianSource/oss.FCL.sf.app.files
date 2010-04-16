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
 *     The source file of the operation service of file manager
 */

#include "fmoperationservice.h"
#include "fmoperationthread.h"
#include "fmbackupconfigloader.h"
#include "fmbackuprestorehandler.h"
#include "fmbkupengine.h"
#include "fmviewdetailsdialog.h"
#include "fmoperationresultprocesser.h"
#include "fmoperationcopy.h"
#include "fmoperationmove.h"
#include "fmoperationremove.h"

#include <hbprogressnote.h>
#include <hbaction.h>
#include <hbmessagebox.h>

FmOperationService::FmOperationService( QObject *parent ) : QObject( parent ),
        mCurrentOperation( 0 )
{
    mThread = new FmOperationThread( this );
    mThread->setObjectName( "operationThread" );
    mBackupRestoreHandler = new FmBackupRestoreHandler( this );
    mBackupRestoreHandler->setObjectName( "backupRestore" ) ;

    mOperationResultProcesser = new FmOperationResultProcesser( this );
    
    QMetaObject::connectSlotsByName( this );
}

FmOperationService::~FmOperationService()
{
    delete mThread;
    
    delete mBackupRestoreHandler;
    
}

bool FmOperationService::isRunning()
{
    if( mCurrentOperation ) {
        return true;
    } else {
        return false;
    }
    
}

int FmOperationService::asyncCopy( QStringList sourceList, QString targetPath )
{
    if ( isRunning() )
        return FmErrAlreadyStarted;
    Q_ASSERT( !mCurrentOperation );
     
    if( sourceList.empty() ) {
        return FmErrWrongParam;
    }
    mCurrentOperation = new FmOperationCopy( mThread, sourceList, targetPath );

    int ret = mThread->asyncCopy( mCurrentOperation );
    if( ret != FmErrNone ) {
        resetOperation();
    }
    return ret;
}

int FmOperationService::asyncMove( QStringList sourceList, QString targetPath )
{
    if ( isRunning() )
        return FmErrAlreadyStarted;
    Q_ASSERT( !mCurrentOperation );

    if( sourceList.empty() ) {
        return FmErrWrongParam;
    }
    mCurrentOperation = new FmOperationMove( mThread, sourceList, targetPath );

    int ret = mThread->asyncMove( mCurrentOperation );
    if( ret != FmErrNone ) {
        resetOperation();
    }
    return ret;
}

int FmOperationService::asyncRemove( QStringList pathList )
{
    if ( isRunning() )
        return FmErrAlreadyStarted;
    Q_ASSERT( !mCurrentOperation ); 

    mCurrentOperation = new FmOperationRemove( mThread, pathList );

    int ret = mThread->asyncRemove( mCurrentOperation );
    if( ret != FmErrNone ) {
        resetOperation();
    }
    return ret;
}

int FmOperationService::asyncFormat( QString driverName )
{
    if ( isRunning() )
        return FmErrAlreadyStarted;
    Q_ASSERT( !mCurrentOperation );

    mCurrentOperation = new FmOperationFormat( mThread, driverName );

    int ret = mThread->asyncFormat( mCurrentOperation );
    if( ret != FmErrNone ) {
        resetOperation();
    }
    return ret;
}
int FmOperationService::asyncViewDriveDetails( const QString driverName )
{
    if ( isRunning() )
        return FmErrAlreadyStarted;
    Q_ASSERT( !mCurrentOperation );

    mCurrentOperation = new FmOperationDriveDetails( mThread, driverName );

    int ret = mThread->asyncViewDriveDetails( mCurrentOperation );
    if( ret != FmErrNone ) {
        resetOperation();
    }
    return ret;
}

int FmOperationService::asyncViewFolderDetails( const QString folderPath )
{
    if ( isRunning() )
        return FmErrAlreadyStarted;
    Q_ASSERT( !mCurrentOperation );

    mCurrentOperation = new FmOperationFolderDetails( mThread, folderPath );

    int ret = mThread->asyncViewFolderDetails( mCurrentOperation );
    if( ret != FmErrNone ) {
        resetOperation();
    }
    return ret;
}

int FmOperationService::asyncBackup()
{   
    if ( isRunning() )
        return FmErrAlreadyStarted;
    Q_ASSERT( !mCurrentOperation );

    FmOperationBackup *operationBackup = new FmOperationBackup( mBackupRestoreHandler );
    mCurrentOperation = operationBackup;
    int ret = backupRestoreHandler()->startBackup( operationBackup );
    if( ret ){
        return FmErrNone;
    } else {
        resetOperation();
        return backupRestoreHandler()->error();
    }
}

int FmOperationService::asyncRestore( quint64 selection )
{
    if ( isRunning() )
        return FmErrAlreadyStarted;
    Q_ASSERT( !mCurrentOperation );

    FmOperationRestore* operationRestore = new FmOperationRestore( mBackupRestoreHandler, selection );
    mCurrentOperation = operationRestore;
    int ret = backupRestoreHandler()->startRestore( operationRestore );
    if( ret ){
        return FmErrNone;
    } else {
        resetOperation();
        return backupRestoreHandler()->error();
    }
}

int FmOperationService::syncDeleteBackup( quint64 selection )
{
    return mBackupRestoreHandler->deleteBackup( selection );
}

void FmOperationService::cancelOperation()
{
    switch( mCurrentOperation->operationType() )
    {
    case  EOperationTypeBackup:
        backupRestoreHandler()->cancelBackup();
        break;
    case EOperationTypeDriveDetails:
        mThread->stop();
        break;
    case EOperationTypeFolderDetails:
        mThread->stop();
        break;
    case EOperationTypeFormat:
        //can not cancel format
        break;
    case EOperationTypeCopy:
         mThread->stop();
        break;
    case EOperationTypeMove:
         mThread->stop();
        break;
    case EOperationTypeRemove:
         mThread->stop();
        break;
    default:
        Q_ASSERT( false );
    }    
}


int FmOperationService::syncSetVolume( const QString &driverName, const QString &volume )
{
    Q_UNUSED( driverName );
    Q_UNUSED( volume );
    return FmErrNone;
}

int FmOperationService::syncSetdDriverPassword( const QString &driverName,
                                               const QString &oldPassword, 
                                               const QString &newPassword )
{
    Q_UNUSED( driverName );
    Q_UNUSED( oldPassword );
    Q_UNUSED( newPassword );
    return FmErrNone;
}

int FmOperationService::syncRename( const QString &oldPath, const QString &newName )
{
    Q_UNUSED( oldPath );
    Q_UNUSED( newName );
    return FmErrNone;
}

int FmOperationService::syncLaunchFileOpen( const QString &filePath )
{
    return FmUtils::launchFile( filePath );
}



FmBackupRestoreHandler *FmOperationService::backupRestoreHandler()
{
    if( !mBackupRestoreHandler ) {
        mBackupRestoreHandler = new FmBackupRestoreHandler( this );
        QMetaObject::connectSlotsByName( this );
    }
    return mBackupRestoreHandler;
}

void FmOperationService::resetOperation()
{
   if( mCurrentOperation ) {
        delete mCurrentOperation;
        mCurrentOperation = 0;
    }
}

/////////////////////////////////////////////////////
///Thread
void FmOperationService::on_operationThread_askForRename( const QString &srcFile, QString *destFile )
{
    mOperationResultProcesser->onAskForRename(
        mCurrentOperation, srcFile, destFile );
}
void FmOperationService::on_operationThread_refreshModel( const QString &path )
{
    emit refreshModel( mCurrentOperation, path );
}
void FmOperationService::on_operationThread_notifyWaiting( bool cancelable )
{
    mOperationResultProcesser->onNotifyWaiting(
        mCurrentOperation, cancelable );
    emit notifyWaiting( mCurrentOperation, cancelable );
}
void FmOperationService::on_operationThread_notifyPreparing( bool cancelable )
{
    mOperationResultProcesser->onNotifyPreparing(
        mCurrentOperation, cancelable );
    emit notifyPreparing( mCurrentOperation, cancelable );
}
void FmOperationService::on_operationThread_notifyStart( bool cancelable, int maxSteps )
{
    mOperationResultProcesser->onNotifyStart(
        mCurrentOperation, cancelable, maxSteps );
    emit notifyStart( mCurrentOperation, cancelable, maxSteps );
}
void FmOperationService::on_operationThread_notifyProgress( int currentStep )
{
    mOperationResultProcesser->onNotifyProgress(
        mCurrentOperation, currentStep );
    emit notifyProgress( mCurrentOperation, currentStep );
}
void FmOperationService::on_operationThread_notifyFinish()
{
    mOperationResultProcesser->onNotifyFinish( mCurrentOperation );
    emit notifyFinish( mCurrentOperation );
    resetOperation();
}
void FmOperationService::on_operationThread_notifyError(int error, QString errString )
{
    mOperationResultProcesser->onNotifyError(
        mCurrentOperation, error, errString );
    emit notifyError( mCurrentOperation, error, errString );
    resetOperation();
}
void FmOperationService::on_operationThread_notifyCanceled()
{
    mOperationResultProcesser->onNotifyCanceled(
        mCurrentOperation );
    emit notifyCanceled( mCurrentOperation );
    resetOperation();
}
///
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
///BackupRestore
void FmOperationService::on_backupRestore_notifyPreparing( bool cancelable )
{
    mOperationResultProcesser->onNotifyPreparing(
        mCurrentOperation, cancelable );
     emit notifyPreparing( mCurrentOperation, cancelable );
}
void FmOperationService::on_backupRestore_notifyStart( bool cancelable, int maxSteps )
{
    mOperationResultProcesser->onNotifyStart(
        mCurrentOperation, cancelable, maxSteps );
    emit notifyStart( mCurrentOperation, cancelable, maxSteps );
}
void FmOperationService::on_backupRestore_notifyProgress( int currentStep )
{
    mOperationResultProcesser->onNotifyProgress(
        mCurrentOperation, currentStep );
    emit notifyProgress( mCurrentOperation, currentStep );
}
void FmOperationService::on_backupRestore_notifyFinish()
{
    mOperationResultProcesser->onNotifyFinish( mCurrentOperation );
    emit notifyFinish( mCurrentOperation );
    resetOperation();
}
void FmOperationService::on_backupRestore_notifyError(int error, QString errString )
{
    mOperationResultProcesser->onNotifyError(
        mCurrentOperation, error, errString );
    emit notifyError( mCurrentOperation, error, errString );
    resetOperation();
}
void FmOperationService::on_backupRestore_notifyCanceled()
{
    mOperationResultProcesser->onNotifyCanceled(
        mCurrentOperation );
    emit notifyCanceled( mCurrentOperation );
    resetOperation();
}

///
/////////////////////////////////////////////////////
