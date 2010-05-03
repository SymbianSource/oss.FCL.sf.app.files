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

#include <hbprogressdialog.h>
#include <hbaction.h>
#include <hbmessagebox.h>

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
    
    QString questionText = QString( "file " ) +
        srcFile + QString( " already exist, please rename:" );
    QString value;
    FmDlgUtils::showTextQuery( questionText, value, true );
    *destFile = value;
}

void FmOperationResultProcesser::onAskForReplace(
    FmOperationBase* operationBase, const QString &srcFile, const QString &destFile, bool *isAccepted )
{
    Q_UNUSED( operationBase );
    Q_UNUSED( destFile );
    
    QString questionText = QString( "file " ) +
        srcFile + QString( " already exist, replace it?" );
    if( HbMessageBox::question( questionText ) ) {
        *isAccepted = true;
    } else {
        *isAccepted = false;
    }
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
            HbMessageBox::information( QString( hbTrId("Format succeed!")) );
            FmOperationFormat *paramFormat = static_cast<FmOperationFormat*>( operationBase );
            QString title( tr( "Drive name ") );  
            QString driveName( paramFormat->driverName() );
            QString volumeName;
            while( FmDlgUtils::showTextQuery( title, volumeName, false, FmMaxLengthofDriveName ) ){
                    int err = FmUtils::renameDrive( driveName, volumeName );
                    if ( err == FmErrNone ){
                        HbMessageBox::information( hbTrId( "The name has been changed!" ) );
                        mOperationService->on_operationThread_refreshModel( driveName );
                        break;
                    } else if( err == FmErrBadName ) {
                        HbMessageBox::information( hbTrId( "Illegal characters! Use only letters and numbers." ) );
                    } else{
                        HbMessageBox::information( hbTrId( "Error occurred, operation cancelled!" ) );
                        break;
                    }                
                }
            break;
        }
    case FmOperationService::EOperationTypeBackup:
        {
            HbMessageBox::information( QString( hbTrId("Backup succeed!")) );
            break;
        }
    case FmOperationService::EOperationTypeRestore:
        {
            HbMessageBox::information( QString( hbTrId("Restore succeed!")) );
            break;
        }
    default:
        HbMessageBox::information( QString( hbTrId("Operation finished")) );

    }
}
void FmOperationResultProcesser::onNotifyError( FmOperationBase* operationBase, int error, QString errString )
{
    Q_UNUSED( errString );
    failAndCloseProgress();
    switch( error )
    {
        case FmErrAlreadyStarted:
            HbMessageBox::information( QString( hbTrId("Operation already started!")) );
            return;
        case FmErrDiskFull:
            HbMessageBox::information( QString( hbTrId("Not enough space. Operation cancelled.!")) );
            return;
        case FmErrCopyDestToSubFolderInSrc:
            HbMessageBox::information( QString( hbTrId("Can not copy to sub folder!")) );
            return;
        case FmErrMoveDestToSubFolderInSrc:
            HbMessageBox::information( QString( hbTrId("Can not move to sub folder!")) );
            return;
        case FmErrCannotRemove:{
            if( operationBase->operationType() == FmOperationService::EOperationTypeCopy ) {
                // when copy a file/dir to same name destination, and delete dest fail, this error will occur
                HbMessageBox::information( QString( hbTrId( "Can not copy because %1 can not be deleted!" ).arg( errString ) ) );
                return;
            }
            else if( operationBase->operationType() == FmOperationService::EOperationTypeMove ) {
                // when move a file/dir to same name destination, and delete dest fail, this error will occur
                HbMessageBox::information( QString( hbTrId( "Can not move because %1 can not be deleted!" ).arg( errString ) ) );
                return;
            }
            // when delete file/dir fail, this error will occur
            HbMessageBox::information( QString( hbTrId( "Can not delete %1!" ).arg( errString ) ) );
            return; 
        }      
        case FmErrRemoveDefaultFolder:{
            if( operationBase->operationType() == FmOperationService::EOperationTypeMove ) {
                // when move a default folder
                HbMessageBox::information( QString( hbTrId( "Could not move because the default folder %1 can not be deleted!" ).arg( errString ) ) );
                return;
            }
            else {
               // when delete the default folder
               HbMessageBox::information( QString( hbTrId( "Could not remove the default folder %1 " ).arg( errString ) ) );
               return;
            }
        }
    }

    switch( operationBase->operationType() )
    {
    case FmOperationService::EOperationTypeFormat:
        HbMessageBox::information( QString( hbTrId("Format failed!")) );
        break;
    default:
        HbMessageBox::information( QString( hbTrId("Operation failed")) );
    }

}

void FmOperationResultProcesser::onNotifyCanceled( FmOperationBase* operationBase )
{
    Q_UNUSED( operationBase );
    cancelProgress();
    HbMessageBox::information( QString( hbTrId("Operation Canceled!") ) );
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
    if( !cancelable )
        mNote->primaryAction()->setDisabled( true );
    else
        mNote->primaryAction()->setDisabled( false );
    mNote->exec();

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
    if( !cancelable ){
        mNote->primaryAction()->setDisabled( true );  
    }
    else{
        mNote->primaryAction()->setDisabled( false );       
    }
    mNote->exec();
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

    if( !cancelable ){
        mNote->primaryAction()->setDisabled( true );
    }
    else{
        mNote->primaryAction()->setDisabled( false );
    }

    mNote->exec();
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
