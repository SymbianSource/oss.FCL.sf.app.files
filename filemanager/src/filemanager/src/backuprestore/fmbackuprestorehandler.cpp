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
 *     The source file of backup and restore handler of file manager
 */

#include "fmbackuprestorehandler.h"
#include "fmbackupsettings.h"
#include "fmrestoresettings.h"
#include "fmoperationservice.h"
#include "fmbackupconfigloader.h"
#include "fmbkupengine.h"
#include "fmoperationbase.h"
#include "fmcommon.h"

#include <hbmessagebox.h>

FmBackupRestoreHandler::FmBackupRestoreHandler( QObject *parent ) : QObject( parent ), mBackupConfigLoader( 0 )
{
    mBkupEngine = new FmBkupEngine( this );

    mCurrentProcess = ProcessNone;

    connect( mBkupEngine, SIGNAL( notifyPreparing( bool ) ), this, SLOT( onNotifyPreparing( bool ) ), Qt::QueuedConnection );
    connect( mBkupEngine, SIGNAL( notifyStart( bool, int) ), this, SLOT( onNotifyStart( bool, int ) ), Qt::QueuedConnection );
    connect( mBkupEngine, SIGNAL( notifyUpdate(int) ), this, SLOT( onNotifyUpdate(int) ), Qt::QueuedConnection );
    connect( mBkupEngine, SIGNAL( notifyFinish(int) ), 
        this, SLOT( onNotifyFinish(int) ), Qt::QueuedConnection );
    connect( mBkupEngine, SIGNAL( notifyMemoryLow(int, int& ) ), 
        this, SLOT( onNotifyMemoryLow(int, int&) ) );
}
FmBackupRestoreHandler::~FmBackupRestoreHandler()
{
    delete mBkupEngine;
    delete mBackupConfigLoader;
}

FmBkupEngine *FmBackupRestoreHandler::bkupEngine()
{
    return mBkupEngine;
}

FmBackupConfigLoader *FmBackupRestoreHandler::backupConfigLoader()
{
    if( !mBackupConfigLoader ) {
        mBackupConfigLoader = new FmBackupConfigLoader();
    }
    return mBackupConfigLoader;
}

int FmBackupRestoreHandler::error()
{
    return mBkupEngine->error();
}

bool FmBackupRestoreHandler::startBackup( FmOperationBackup *operationBackup )
{
    Q_UNUSED( operationBackup );
    mCurrentProcess = ProcessBackup;
    bool ret = mBkupEngine->startBackup( backupConfigLoader()->driversAndOperationList(),
        backupConfigLoader()->backupCategoryList(), 
        mBkupEngine->BackupSettingsL()->targetDrive(),
        mBkupEngine->BackupSettingsL()->content() );

    if( !ret )
        {
        mCurrentProcess = ProcessNone;
        HbMessageBox::information(tr("backup failed"));
        }
    return ret;
}
void FmBackupRestoreHandler::cancelBackup()
{
    mBkupEngine->cancelBackup();
}
bool FmBackupRestoreHandler::startRestore( FmOperationRestore *operationRestore )
{
    mCurrentProcess = ProcessRestore;
    mBkupEngine->RestoreSettingsL()->SetSelection( operationRestore->selection() );
    bool ret = mBkupEngine->startRestore( backupConfigLoader()->driversAndOperationList() );
    if( !ret )
        {
        mCurrentProcess = ProcessNone;
        HbMessageBox::information(tr("restore failed"));
        }
    return ret;
}

int FmBackupRestoreHandler::deleteBackup( quint64 selection )
{
    mBkupEngine->RestoreSettingsL()->SetSelection( selection );
    mBkupEngine->deleteBackup( backupConfigLoader()->driversAndOperationList() );
    return FmErrNone;
}

void FmBackupRestoreHandler::onNotifyMemoryLow( int memoryValue, int &userError )
{
    userError = FmErrNone;
    if( memoryValue < FmEstimateLowerLimit ) {
        userError = FmErrDiskFull;
    } else if( memoryValue < FmEstimateUpperLimit ) {
        if ( !HbMessageBox::question( "memory low, continue?" ) ){
            userError = FmErrCancel;
        }
    }
}


void FmBackupRestoreHandler::onNotifyPreparing( bool cancelable )
{
    emit notifyPreparing(  cancelable );
}
void FmBackupRestoreHandler::onNotifyStart( bool cancelable, int maxSteps )
{
    emit notifyStart( cancelable, maxSteps );
}
void FmBackupRestoreHandler::onNotifyUpdate( int currentStep )
{
    emit notifyProgress( currentStep );
}
void FmBackupRestoreHandler::onNotifyFinish( int err )
{
    if( err == FmErrNone ) {
        emit notifyFinish();
    } else if( err == FmErrCancel ){
        emit notifyCanceled();
    } else {
        emit notifyError( err, QString("") );
    }
}
