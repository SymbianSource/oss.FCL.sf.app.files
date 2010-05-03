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
 *     Zhiqiang Yang <zhiqiang.yang@nokia.com>
 * 
 * Description:
 *     The source file of the delete backup view of file manager
 */

#include "fmdeletebackupview.h"
#include "fmdeletebackupwidget.h"
#include "fmviewmanager.h"
#include "fmoperationbase.h"

#include <QApplication>

#include <hbaction.h>
#include <hbtoolbar.h>
#include <hbmenu.h>
#include <hbmainwindow.h>
#include <hbmessagebox.h>



FmDeleteBackupView::FmDeleteBackupView() : FmViewBase( EDeleteBackupView )
{
	setTitle( hbTrId( "Delete Backup" ) );

	initMainWidget();
	initToolBar();
	initMenu();
    adjustActions();
	mOperationService = FmViewManager::viewManager()->operationService();

	QMetaObject::connectSlotsByName( this );
}

FmDeleteBackupView::~FmDeleteBackupView()
{
    removeToolBarAction();
}

void FmDeleteBackupView::initMenu()
{
#ifdef FM_CHANGE_ORIENT_ENABLE
    HbAction *action = 0;
	action = new HbAction( this );
    action->setObjectName( "rotateAction" );
    action->setText( hbTrId( "Change orientation" ) );
    menu()->addAction( action );
#endif

	mDeleteAction = new HbAction( this );
    mDeleteAction->setObjectName( "deleteAction" );
    mDeleteAction->setText( hbTrId( "Delete backups" ) );
    menu()->addAction( mDeleteAction );
}

void FmDeleteBackupView::initMainWidget()
{
	mDeleteBackupWidget = new FmDeleteBackupWidget( this );
    mDeleteBackupWidget->setObjectName( "deleteBackupWidget" );
    setWidget( mDeleteBackupWidget );

}

void FmDeleteBackupView::initToolBar()
{
	mLeftAction = new HbAction( this );
    mLeftAction->setObjectName( "leftAction" );
    mLeftAction->setText( hbTrId( "delete" ) );
    toolBar()->addAction( mLeftAction );
    
    toolBar()->setOrientation( Qt::Horizontal );

    //action = new HbAction( this );
    //action->setObjectName( "rightAction" );
    //action->setText( tr( "" ) );
    //toolBar()->addAction( action );
}

void FmDeleteBackupView::on_leftAction_triggered()
{
    on_deleteAction_triggered();
}

#ifdef FM_CHANGE_ORIENT_ENABLE
void FmDeleteBackupView::on_rotateAction_triggered()
{
	if ( mainWindow()->orientation() == Qt::Vertical ) {
		mainWindow()->setOrientation( Qt::Horizontal );
	}
	else {
		mainWindow()->setOrientation( Qt::Vertical );
	}
}
#endif

void FmDeleteBackupView::on_deleteAction_triggered()
{
    QList<int > items = mDeleteBackupWidget->selectionIndexes();
    if( items.count() <= 0 || !HbMessageBox::question( tr("Confirm Deletion?" ) ) ){
        return;
    }

    quint64 selection( 0 );
    for ( int i( 0 ); i < items.count(); ++i )
       {
       selection |= ( ( quint64 ) 1 ) << ( items[ i ] );
       }
                   
    mOperationService->syncDeleteBackup( selection );
    mDeleteBackupWidget->refresh();
    adjustActions();
}

void FmDeleteBackupView::adjustActions()
{
    if( mDeleteBackupWidget->backupDataCount() > 0 ) {
        mDeleteAction->setDisabled( false );
        mLeftAction->setDisabled( false );
    } else {
        mDeleteAction->setDisabled( true );
        mLeftAction->setDisabled( true );
    }
}

void FmDeleteBackupView::removeToolBarAction()
{
    toolBar()->removeAction( mLeftAction );
}

