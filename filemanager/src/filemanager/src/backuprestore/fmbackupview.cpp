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
 *     The source file of the back up view of file manager
 */

#include "fmbackupview.h"
#include "fmbackupwidget.h"
#include "fmviewmanager.h"
#include "fmoperationbase.h"


#include <QApplication>

#include <hbaction.h>
#include <hbtoolbar.h>
#include <hbmenu.h>
#include <hbmainwindow.h>
#include <hbmessagebox.h>


FmBackupView::FmBackupView() : FmViewBase( EBackupView )
{
	setTitle( tr( "Backup" ) );

	initToolBar();
	initMainWidget();
	initMenu();
	mOperationService = FmViewManager::viewManager()->operationService();

	QMetaObject::connectSlotsByName( this );
}

FmBackupView::~FmBackupView()
{
    removeToolBarAction();
}

void FmBackupView::initMenu()
{
    HbAction *action = 0;
    
#ifdef FM_CHANGE_ORIENT_ENABLE
	action = new HbAction( this );
    action->setObjectName( "rotateAction" );
    action->setText( tr( "Change orientation" ) );
    menu()->addAction( action );
#endif

	action = new HbAction( this );
    action->setObjectName( "backupAction" );
    action->setText( tr( "Start backup" ) );
    menu()->addAction( action );

	action = new HbAction( this );
    action->setObjectName( "deleteBackupAction" );
    action->setText( tr( "Delete backup" ) );
    menu()->addAction( action );

}

void FmBackupView::initMainWidget()
{
	mMainWidget = new FmBackupWidget( this );
    setWidget( mMainWidget );

}

void FmBackupView::initToolBar()
{
    mToolBarAction = new HbAction( this );
    mToolBarAction->setObjectName( "leftAction" );
    mToolBarAction->setText( hbTrId("Start backup") );
    toolBar()->addAction( mToolBarAction );
    
    toolBar()->setOrientation( Qt::Horizontal );
}

void FmBackupView::on_leftAction_triggered()
{
    int ret = mOperationService->asyncBackup();
    switch( ret )
    {
    case FmErrNone:
        break;
    case FmErrWrongParam:
        HbMessageBox::information( QString( tr("Operation canceled with wrong param!") ) );
        break;
    case FmErrAlreadyStarted:
        HbMessageBox::information( QString( tr("Operation canceled because already started!") ) );
        break;
    default:
        HbMessageBox::information( QString( tr("Operation canceled with error!") ) );
        break;
    }
}
#ifdef FM_CHANGE_ORIENT_ENABLE
void FmBackupView::on_rotateAction_triggered()
{
	if ( mainWindow()->orientation() == Qt::Vertical ) {
		mainWindow()->setOrientation( Qt::Horizontal );
	}
	else {
		mainWindow()->setOrientation( Qt::Vertical );
	}
}
#endif //FM_CHANGE_ORIENT_ENABLE

void FmBackupView::on_backupAction_triggered()
{
    int ret = mOperationService->asyncBackup();
    switch( ret )
    {
    case FmErrNone:
        break;
    case FmErrWrongParam:
        HbMessageBox::information( QString( tr("Operation canceled with wrong param!") ) );
        break;
    case FmErrAlreadyStarted:
        HbMessageBox::information( QString( tr("Operation canceled because already started!") ) );
        break;
    default:
        HbMessageBox::information( QString( tr("Operation canceled with error!") ) );
        break;
    }
}

void FmBackupView::on_deleteBackupAction_triggered()
{
    FmViewManager::viewManager()->createDeleteBackupView();
}

void FmBackupView::removeToolBarAction()
{
    toolBar()->removeAction( mToolBarAction );
}

