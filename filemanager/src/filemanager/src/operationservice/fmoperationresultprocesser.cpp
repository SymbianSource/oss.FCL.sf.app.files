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
#include "fmviewdetailsdialog.h"
#include "fmdlgutils.h"

#include <hbprogressnote.h>
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
    FmDlgUtils::showTextQuery( questionText, value );
    *destFile = value;
}

void FmOperationResultProcesser::onNotifyWaiting( FmOperationBase* operationBase, bool cancelable )
{
    QString title = tr("Operation");
    switch( operationBase->operationType() )
    {
    case FmOperationService::EOperationTypeBackup:
        title = tr("backuping");
        break;
    case FmOperationService::EOperationTypeRestore:
        title = tr("restoring");
        break;
      case FmOperationService::EOperationTypeDriveDetails:
          title = tr( "Scaning memory..." );
        break;
        
    case FmOperationService::EOperationTypeFolderDetails:
        title = tr( "Scaning folder..." );
        break;
    default:
        break;
    }

    showWaiting( title, cancelable );
}

void FmOperationResultProcesser::onNotifyPreparing( FmOperationBase* operationBase, bool cancelable )
{
    QString title = tr("Operation");
    switch( operationBase->operationType() )
    {
    case FmOperationService::EOperationTypeBackup:
        title = tr("backup preparing");
        break;
    case FmOperationService::EOperationTypeRestore:
        title = tr("restore preparing");
        break;
    case FmOperationService::EOperationTypeCopy:
        title = tr("copy preparing");
        break;
    case FmOperationService::EOperationTypeMove:
        title = tr("move preparing");
        break;
    case FmOperationService::EOperationTypeRemove:
        title = tr("delete preparing");
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
        title = tr("backup...");
        break;
    case FmOperationService::EOperationTypeRestore:
        title = tr("restore...");
        break;
    case FmOperationService::EOperationTypeCopy:
        title = tr("copy...");
        break;
    case FmOperationService::EOperationTypeMove:
        title = tr("move...");
        break;
    case FmOperationService::EOperationTypeRemove:
        title = tr("delete...");
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
            FmViewDetailsDialog::showDriveViewDetailsDialog( diskName, paramDriveDetails->detailsSizeList() );
            break;
        }
    case FmOperationService::EOperationTypeFolderDetails:
        {
            FmOperationFolderDetails *paramFolderDetails = static_cast<FmOperationFolderDetails*>( operationBase );
            FmViewDetailsDialog::showFolderViewDetailsDialog( paramFolderDetails->folderPath(),
                paramFolderDetails->numofSubFolders(),
                paramFolderDetails->numofFiles(),
                paramFolderDetails->sizeofFolder() );
            break;
        }
    case FmOperationService::EOperationTypeFormat:
        {
            HbMessageBox::information( QString( tr("Format succeed!")) );
            break;
        }
    case FmOperationService::EOperationTypeBackup:
        {
            HbMessageBox::information( QString( tr("Backup succeed!")) );
            break;
        }
    case FmOperationService::EOperationTypeRestore:
        {
            HbMessageBox::information( QString( tr("Restore succeed!")) );
            break;
        }
    default:
        HbMessageBox::information( QString( tr("Operation finished")) );

    }
}
void FmOperationResultProcesser::onNotifyError( FmOperationBase* operationBase, int error, QString errString )
{
    Q_UNUSED( errString );
    failAndCloseProgress();
    switch( error )
    {
    case FmErrAlreadyStarted:
        HbMessageBox::information( QString( tr("Operation already started!")) );
        return;
    case FmErrDiskFull:
        HbMessageBox::information( QString( tr("Not enough space. Operation cancelled.!")) );
        return;
    case FmErrCopyDestToSubFolderInSrc:
        HbMessageBox::information( QString( tr("Can not copy to sub folder!")) );
        return;
    case FmErrMoveDestToSubFolderInSrc:
        HbMessageBox::information( QString( tr("Can not move to sub folder!")) );
        return;
    }


    switch( operationBase->operationType() )
    {
    case FmOperationService::EOperationTypeFormat:
        HbMessageBox::information( QString( tr("Format failed!")) );
        break;
    default:
        HbMessageBox::information( QString( tr("Operation failed")) );
    }

}

void FmOperationResultProcesser::onNotifyCanceled( FmOperationBase* operationBase )
{
    Q_UNUSED( operationBase );
    cancelProgress();
    HbMessageBox::information( QString( tr("Operation Canceled!") ) );
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
    
    mNote = new HbProgressNote( HbProgressNote::WaitNote );
    connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    if( !mNote ) {
//        mNote = new HbProgressNote( HbProgressNote::WaitNote );
//        connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    } else {
//        mNote->setProgressNoteType( HbProgressNote::WaitNote );
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
    
    mNote = new HbProgressNote( HbProgressNote::ProgressNote );
    connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    if( !mNote ) {
//        mNote = new HbProgressNote( HbProgressNote::ProgressNote );
//        connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    } else {
//        mNote->setProgressNoteType( HbProgressNote::ProgressNote );
//    }
    mNote->setMinimum(0);
    mNote->setMaximum( 65535 );
    mNote->setProgressValue( 0 );
    mNote->setText( title );
    if( !cancelable )
        mNote->primaryAction()->setDisabled( true );
    else
        mNote->primaryAction()->setDisabled( false );
    mNote->exec();
}

void FmOperationResultProcesser::showProgress( QString title, bool cancelable, int maxValue )
{
    qDebug("show progress");
    
    if( mNote ){
        mNote->close();
        delete mNote;        
    }
    
    mNote = new HbProgressNote( HbProgressNote::ProgressNote );
    connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    if( !mNote ) {
//        mNote = new HbProgressNote( HbProgressNote::ProgressNote );
//        connect( mNote, SIGNAL( cancelled() ), this, SLOT(onProgressCancelled() ) );
//    } else {
//        mNote->setProgressNoteType( HbProgressNote::ProgressNote );
//    }
    mNote->setText( title );

    mNote->setMinimum(0);
    mNote->setMaximum( maxValue );

    mNote->setProgressValue( 0 );

    if( !cancelable )
        mNote->primaryAction()->setDisabled( true );
    else
        mNote->primaryAction()->setDisabled( false );

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
