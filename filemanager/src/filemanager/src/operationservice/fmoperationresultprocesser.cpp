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
*
*/
#include "fmoperationresultprocesser.h"
#include "fmoperationbase.h"
#include "fmoperationservice.h"
#include "fmoperationformat.h"
#include "fmviewdetailsdialog.h"
#include "fmdlgutils.h"
#include "fmutils.h"
#include <hbaction.h>
#include <hbprogressdialog.h>
#include <hbaction.h>
#include <hbglobal.h>
#include <QFileInfo>

FmOperationResultProcesser::FmOperationResultProcesser( FmOperationService *operationService )
    : mOperationService( operationService ), mNote( 0 )
{
}

FmOperationResultProcesser::~FmOperationResultProcesser(void)
{
}

void FmOperationResultProcesser::onAskForRename(
    FmOperationBase* operationBase, const QString &srcFile, QString *destFile )
{
    Q_UNUSED( operationBase );
    int maxFileNameLength = FmUtils::getMaxFileNameLength();
    
    QString questionText = QString( "file " ) +
        srcFile + QString( " already exist, please rename:" );
    QString value;   
    QFileInfo fileInfo(srcFile);
    bool ret = FmDlgUtils::showTextQuery( questionText, value, true, maxFileNameLength, QString(), false );
    while ( ret ) {
        bool checkResult = true;
        QString newTargetPath = FmUtils::fillPathWithSplash(
                                fileInfo.absolutePath() ) + value;
        QFileInfo newFileInfo( newTargetPath );
        if (!FmUtils::checkFolderFileName(value)) {
            FmDlgUtils::information( hbTrId( "Invalid file or folder name, try again!" ) );
            checkResult = false;
        }
        if( !FmUtils::checkMaxPathLength( newTargetPath ) ) {
            FmDlgUtils::information( hbTrId( "the path you specified is too long, try again!" ) );
            checkResult = false;
        }
        if (newFileInfo.exists()) {
            FmDlgUtils::information( hbTrId( "%1 already exist!" ).arg( value ) );
            checkResult = false;
        }
        if( !checkResult ) {
            ret = FmDlgUtils::showTextQuery( questionText, value, true, maxFileNameLength, QString(), false );
            continue;
        } else {
            break;
        }
    }   
	if( ret ) {
		*destFile = value;
	}
}

void FmOperationResultProcesser::onAskForReplace(
    FmOperationBase* operationBase, const QString &srcFile, const QString &destFile, bool *isAccepted )
{
    Q_UNUSED( operationBase );
    Q_UNUSED( destFile );
    
    QString questionText = QString( "file " ) +
        srcFile + QString( " already exist, replace it?" );
    if( FmDlgUtils::question( questionText ) ) {
        *isAccepted = true;
    } else {
        *isAccepted = false;
    }
}

void FmOperationResultProcesser::onShowNote( FmOperationBase* operationBase, const char *noteString )
{
    FmDlgUtils::information(hbTrId(noteString));
}

void FmOperationResultProcesser::onNotifyWaiting( FmOperationBase* operationBase, bool cancelable )
{
    QString title = tr("Operation");
    switch( operationBase->operationType() )
    {
    case FmOperationService::EOperationTypeBackup:
        title = hbTrId("backuping");
        break;
    case FmOperationService::EOperationTypeRestore:
        title = hbTrId("restoring");
        break;
      case FmOperationService::EOperationTypeDriveDetails:
          title = hbTrId( "Scaning memory..." );
        break;
        
    case FmOperationService::EOperationTypeFolderDetails:
        title = hbTrId( "Scaning folder..." );
        break;
    default:
        break;
    }

    showWaiting( title, cancelable );
}

void FmOperationResultProcesser::onNotifyPreparing( FmOperationBase* operationBase, bool cancelable )
{
    QString title = hbTrId("Operation");
    switch( operationBase->operationType() )
    {
    case FmOperationService::EOperationTypeBackup:
        title = hbTrId("backup preparing");
        break;
    case FmOperationService::EOperationTypeRestore:
        title = hbTrId("restore preparing");
        break;
    case FmOperationService::EOperationTypeCopy:
        title = hbTrId("copy preparing");
        break;
    case FmOperationService::EOperationTypeMove:
        title = hbTrId("move preparing");
        break;
    case FmOperationService::EOperationTypeRemove:
        title = hbTrId("delete preparing");
        break;
    case FmOperationService::EOperationTypeFormat:
        title = hbTrId("format preparing");
        break;
    default:
        break;
    }

    showPreparing( title, cancelable );
}

void FmOperationResultProcesser::onNotifyStart( FmOperationBase* operationBase, bool cancelable, int maxSteps )
{
    QString title = tr("Operation");
    switch( operationBase->operationType() )
    {
    case FmOperationService::EOperationTypeBackup:
        title = hbTrId("backup...");
        break;
    case FmOperationService::EOperationTypeRestore:
        title = hbTrId("restore...");
        break;
    case FmOperationService::EOperationTypeCopy:
        title = hbTrId("copy...");
        break;
    case FmOperationService::EOperationTypeMove:
        title = hbTrId("move...");
        break;
    case FmOperationService::EOperationTypeRemove:
        title = hbTrId("delete...");
        break;
    case FmOperationService::EOperationTypeFormat:
        title = hbTrId("formating...");
        break;
    default:
        break;
    }

    showProgress( title, cancelable, maxSteps );   
}

void FmOperationResultProcesser::onNotifyProgress( FmOperationBase* operationBase, int currentStep )
{
    Q_UNUSED( operationBase );
    setProgress( currentStep );
}

void FmOperationResultProcesser::onNotifyFinish( FmOperationBase* operationBase )
{
    
    finishProgress();
    switch( operationBase->operationType() )
    {
    case FmOperationService::EOperationTypeDriveDetails:
        {
            FmOperationDriveDetails *paramDriveDetails = static_cast<FmOperationDriveDetails*>( operationBase );
            QString diskName = paramDriveDetails->driverName();
            FmViewDetailsDialog::showDriveViewDetailsDialog( diskName, paramDriveDetails->detailsSizeList(),
				FmUtils::getDriveLetterFromPath( diskName ) );
            break;
        }
    case FmOperationService::EOperationTypeFolderDetails:
        {
            FmOperationFolderDetails *paramFolderDetails = static_cast<FmOperationFolderDetails*>( operationBase );
            FmViewDetailsDialog::showFolderViewDetailsDialog( paramFolderDetails->folderPath(),
                paramFolderDetails->numofSubFolders(),
                paramFolderDetails->numofFiles(),
                paramFolderDetails->sizeofFolder(), 
				FmUtils::getDriveLetterFromPath( paramFolderDetails->folderPath() ) );
            break;
        }
    case FmOperationService::EOperationTypeFormat:
        {
            FmDlgUtils::information( QString( hbTrId("Format succeed!")) );
            FmOperationFormat *paramFormat = static_cast<FmOperationFormat*>( operationBase );
            QString title( tr( "Drive name ") );  
            QString driveName( paramFormat->driverName() );
            FmDriverInfo driverInfo = FmUtils::queryDriverInfo( driveName );
            FmDriverInfo::DriveState state = driverInfo.driveState();
            if( ( state & FmDriverInfo::EDriveAvailable ) &&
                ( state & FmDriverInfo::EDriveRemovable ) &&
                !( state & FmDriverInfo::EDriveMassStorage ) ) { 
                QString volumeName;
                while( FmDlgUtils::showTextQuery( title, volumeName, false, FmMaxLengthofDriveName ) ){
                    int err = FmUtils::renameDrive( driveName, volumeName );
                    if ( err == FmErrNone ){
                        FmDlgUtils::information( hbTrId( "The name has been changed!" ) );
                        mOperationService->on_operationThread_refreshModel( driveName );
                        break;
                    } else if( err == FmErrBadName ) {
                        FmDlgUtils::information( hbTrId( "Illegal characters! Use only letters and numbers." ) );
                    } else{
                        FmDlgUtils::information( hbTrId( "Error occurred, operation cancelled!" ) );
                        break;
                    }                
                }
            }
            break;
        }
    case FmOperationService::EOperationTypeBackup:
        {
            FmDlgUtils::information( QString( hbTrId("Backup succeed!")) );
            break;
        }
    case FmOperationService::EOperationTypeRestore:
        {
            FmDlgUtils::information( QString( hbTrId("Restore succeed!")) );
            break;
        }
    default:
        FmDlgUtils::information( QString( hbTrId("Operation finished")) );

    }
}
void FmOperationResultProcesser::onNotifyError( FmOperationBase* operationBase, int error, QString errString )
{
    Q_UNUSED( errString );
    failAndCloseProgress();
    switch( error )
    {
        case FmErrAlreadyStarted:
            FmDlgUtils::information( QString( hbTrId("Operation already started!")) );
            return;
        case FmErrLocked:
            FmDlgUtils::information( QString( hbTrId("Operation failed because drive is locked!")) );
            return;
        case FmErrPathNotFound:
            FmDlgUtils::information( QString( hbTrId("Operation failed because can not find target path or drive is not available!") ) );
            return;
        case FmErrCorrupt:
            FmDlgUtils::information( QString( hbTrId("Operation failed because target media is corrupted!") ) );
            return;
        case FmErrNotReady: // Caused when MMC & OTG is not inserted when start backup
            FmDlgUtils::information( QString( hbTrId("Operation failed because device is not ready!") ) );
            return;
        case FmErrDisMounted: // Caused by eject MMC when preparing backup
            FmDlgUtils::information( QString( hbTrId("Operation failed because device has been removed!") ) );
            return;
        case FmErrDiskFull:
            FmDlgUtils::information( QString( hbTrId("Not enough space. Operation cancelled!")) );
            return;
        case FmErrCopyDestToSubFolderInSrc:
            FmDlgUtils::information( QString( hbTrId("Can not copy to sub folder!")) );
            return;
        case FmErrMoveDestToSubFolderInSrc:
            FmDlgUtils::information( QString( hbTrId("Can not move to sub folder!")) );
            return;
        case FmErrCannotRemove:{
            if( operationBase->operationType() == FmOperationService::EOperationTypeCopy ) {
                // when copy a file/dir to same name destination, and delete dest fail, this error will occur
                FmDlgUtils::information( QString( hbTrId( "Can not copy because %1 can not be deleted!" ).arg( errString ) ) );
                return;
            }
            else if( operationBase->operationType() == FmOperationService::EOperationTypeMove ) {
                // when move a file/dir to same name destination, and delete dest fail, this error will occur
                FmDlgUtils::information( QString( hbTrId( "Can not move because %1 can not be deleted!" ).arg( errString ) ) );
                return;
            }
            // when delete file/dir fail, this error will occur
            FmDlgUtils::information( QString( hbTrId( "Can not delete %1!" ).arg( errString ) ) );
            return; 
        }      
        case FmErrRemoveDefaultFolder:{
            if( operationBase->operationType() == FmOperationService::EOperationTypeMove ) {
                // when move a default folder
                FmDlgUtils::information( QString( hbTrId( "Could not move because the default folder %1 can not be deleted!" ).arg( errString ) ) );
                return;
            }
            else {
               // when delete the default folder
               FmDlgUtils::information( QString( hbTrId( "Could not remove the default folder %1 " ).arg( errString ) ) );
               return;
            }
        }
    }

    switch( operationBase->operationType() )
    {
    case FmOperationService::EOperationTypeFormat:
        FmDlgUtils::information( QString( hbTrId("Format failed!")) );
        break;
    default:
        FmDlgUtils::information( QString( hbTrId("Operation failed")) );
    }

}

void FmOperationResultProcesser::onNotifyCanceled( FmOperationBase* operationBase )
{
    Q_UNUSED( operationBase );
    cancelProgress();
    FmDlgUtils::information( QString( hbTrId("Operation Canceled!") ) );
}


void FmOperationResultProcesser::onProgressCancelled()
{
    mOperationService->cancelOperation();
}


//
void FmOperationResultProcesser::showWaiting( QString title, bool cancelable )
{
    qDebug("show warning");
    if( mNote ){
        mNote->close();
        delete mNote;        
    }
    
    mNote = new HbProgressDialog( HbProgressDialog::WaitDialog );
    connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    if( !mNote ) {
//        mNote = new HbProgressDialog( HbProgressDialog::WaitNote );
//        connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    } else {
//        mNote->setProgressDialogType( HbProgressDialog::WaitNote );
//    }
    mNote->setText( title );
    QList<QAction *> actionList = mNote->actions();
    if (actionList.size() > 0) {        
        QAction *cancelAction = actionList.at(0);
        if (!cancelable) {
            cancelAction->setDisabled( true );
        } else {
            cancelAction->setDisabled( false );
        }        
    } 
    mNote->open();

}

void FmOperationResultProcesser::showPreparing( QString title, bool cancelable )
{
    qDebug("show preparing");
    
    if( mNote ){
        mNote->close();
        delete mNote;        
    }
    
    mNote = new HbProgressDialog( HbProgressDialog::ProgressDialog );
    connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    if( !mNote ) {
//        mNote = new HbProgressDialog( HbProgressDialog::ProgressDialog );
//        connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    } else {
//        mNote->setProgressDialogType( HbProgressDialog::ProgressDialog );
//    }
    mNote->setMinimum(0);
    mNote->setMaximum( 65535 );
    mNote->setProgressValue( 0 );
    mNote->setText( title );
    QList<QAction *> actionList = mNote->actions();
    if (actionList.size() > 0) {        
        QAction *cancelAction = actionList.at(0);
        if (!cancelable) {
            cancelAction->setDisabled( true );
        } else {
            cancelAction->setDisabled( false );
        }        
    } 
    mNote->open();
}

void FmOperationResultProcesser::showProgress( QString title, bool cancelable, int maxValue )
{
    qDebug("show progress");
    
    if( mNote ){
        mNote->close();
        delete mNote;        
    }
    
    mNote = new HbProgressDialog( HbProgressDialog::ProgressDialog );
    connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    if( !mNote ) {
//        mNote = new HbProgressDialog( HbProgressDialog::ProgressDialog );
//        connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    } else {
//        mNote->setProgressDialogType( HbProgressDialog::ProgressDialog );
//    }
    mNote->setText( title );

    mNote->setMinimum(0);
    mNote->setMaximum( maxValue );

    mNote->setProgressValue( 0 );
    QList<QAction *> actionList = mNote->actions();
    if (actionList.size() > 0) {        
        QAction *cancelAction = actionList.at(0);
        if(!cancelable) {
            cancelAction->setDisabled( true );
        } else {
            cancelAction->setDisabled( false );
        }        
    } 
    mNote->open();
}

void FmOperationResultProcesser::setProgress( int value )
{
    qDebug("set progress");
    if( mNote )
        mNote->setProgressValue( value );
}
void FmOperationResultProcesser::finishProgress()
{
    qDebug("finish progress");
    if( mNote ) {
        mNote->close();
    }
}

void FmOperationResultProcesser::cancelProgress()
{
    qDebug("cancel progress");
    if( mNote ) {
        mNote->close();
    }
}

void FmOperationResultProcesser::failAndCloseProgress()
{
    qDebug("fail progress");
    if( mNote ) {
        mNote->close();
    }
}
