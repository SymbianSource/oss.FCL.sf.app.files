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
 *     The header file of the operation service of file manager
 */

#ifndef FMOPERATIONSERVICE_H
#define FMOPERATIONSERVICE_H
#include <QObject>
#include "fmutils.h"
#include "fmcommon.h"

#include <QString>
#include <QStringList>

class FmOperationThread;
class FmOperationFormat;
class FmOperationBase;

class FmBackupRestoreHandler;
class FmDriveDetailsSize;
class FmOperationResultProcesser;

class FmOperationService : public QObject
{
    Q_OBJECT
public:

    enum TOperationType
    {
        EOperationTypeNone,
        EOperationTypeCopy,
        EOperationTypeRemove,
        EOperationTypeMove,
        EOperationTypeFormat,
        EOperationTypeBackup,
        EOperationTypeRestore,
        EOperationTypeDriveDetails,
        EOperationTypeFolderDetails
    };

public:
    explicit FmOperationService( QObject *parent );
    ~FmOperationService();

    //return error if any path exist.
    //renturn error if list count is not as the same the other.
    int asyncCopy( QStringList sourceList, QString targetPath );

    int asyncMove( QStringList sourceList, QString targetPath );

    int asyncRemove( QStringList pathList );

    //return error if can not be format.
    //notify error while format error.
    int asyncFormat( QString driverName );
    int asyncViewDriveDetails( const QString driverName );
    int asyncViewFolderDetails( const QString folderPath );
    int asyncBackup();
    int asyncRestore( quint64 selection );
    
    void cancelOperation();
    

    int syncSetVolume(  const QString &driverName, const QString &volume );
    int syncSetdDriverPassword( const QString &driverName,
                                const QString &oldPassword, const QString &newPassword );
    int syncRename( const QString &oldPath, const QString &newName );
    int syncLaunchFileOpen( const QString &filePath );
    int syncDeleteBackup( quint64 selection );

    FmBackupRestoreHandler *backupRestoreHandler();
    bool isRunning();
    
public slots:
    void on_backupRestore_notifyPreparing( bool cancelable );
    void on_backupRestore_notifyStart( bool cancelable, int maxSteps );
    void on_backupRestore_notifyProgress( int currentStep );
    void on_backupRestore_notifyFinish();
    void on_backupRestore_notifyError(int error, QString errString );
    void on_backupRestore_notifyCanceled();
    
    void on_operationThread_askForRename( const QString &srcFile, QString *destFile );
    void on_operationThread_askForReplace( const QString &srcFile, const QString &destFile, bool *isAccepted );
    void on_operationThread_refreshModel( const QString &path );
    void on_operationThread_notifyWaiting( bool cancelable );
    void on_operationThread_notifyPreparing( bool cancelable );
    void on_operationThread_notifyStart( bool cancelable, int maxSteps );
    void on_operationThread_notifyProgress( int currentStep );
    void on_operationThread_notifyFinish(); 
    void on_operationThread_notifyError(int error, QString errString ); 
    void on_operationThread_notifyCanceled(); 

signals:
//    void askForRename( FmOperationBase* operationBase, const QString &srcFile, QString &destFile );
    void refreshModel( FmOperationBase* operationBase, const QString& filePath );

    void notifyWaiting( FmOperationBase* operationBase, bool cancelable );
    void notifyPreparing( FmOperationBase* operationBase, bool cancelable );      // this step could not be used if not needed.
    void notifyStart( FmOperationBase* operationBase, bool cancelable, int maxSteps );
    void notifyProgress( FmOperationBase* operationBase, int currentStep );
    
    void notifyFinish( FmOperationBase* operationBase ); 
    void notifyError( FmOperationBase* operationBase, int error, QString errString ); 
    void notifyCanceled( FmOperationBase* operationBase ); 

private:
    void resetOperation();

private:
    FmOperationThread           *mThread;
    FmBackupRestoreHandler      *mBackupRestoreHandler;
    FmOperationBase*       mCurrentOperation;
    FmOperationResultProcesser  *mOperationResultProcesser;
    
};





#endif

