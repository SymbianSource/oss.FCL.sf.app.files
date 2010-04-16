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
 *     The source file of the restore view of file manager
 */

#include "fmrestoreview.h"
#include "fmrestorewidget.h"
#include "fmviewmanager.h"
#include "fmoperationbase.h"

#include <QApplication>

#include <hbaction.h>
#include <hbtoolbar.h>
#include <hbmenu.h>
#include <hbmainwindow.h>
#include <hbmessagebox.h>



FmRestoreView::FmRestoreView(): FmViewBase( ERestoreView )
{
	setTitle( tr( "Restore" ) );

	initMainWidget();
	initToolBar();
	initMenu();
    adjustActions();
	mOperationService = FmViewManager::viewManager()->operationService();

	QMetaObject::connectSlotsByName( this );
}

FmRestoreView::~FmRestoreView()
{
    removeToolBarAction();
}

void FmRestoreView::initMenu()
{
#ifdef FM_CHANGE_ORIENT_ENABLE
    HbAction *action = 0;
	action = new HbAction( this );
    action->setObjectName( "rotateAction" );
    action->setText( tr( "Change orientation" ) );
    menu()->addAction( action );
#endif

	mRestoreAction = new HbAction( this );
    mRestoreAction->setObjectName( "restoreAction" );
    mRestoreAction->setText( tr( "Restore data" ) );
    menu()->addAction( mRestoreAction );
}

void FmRestoreView::initMainWidget()
{
	mRestoreWigdet = new FmRestoreWigdet( this );

    setWidget( mRestoreWigdet );

}

void FmRestoreView::initToolBar()
{
	mLeftAction = new HbAction( this );
    mLeftAction->setObjectName( "leftAction" );
    mLeftAction->setText( tr( "Restore" ) );
    toolBar()->addAction( mLeftAction );
    
    toolBar()->setOrientation( Qt::Horizontal );
}

void FmRestoreView::on_leftAction_triggered()
{
    QList<int > items = mRestoreWigdet->selectionIndexes();
    quint64 selection( 0 );
    for ( int i( 0 ); i < items.count(); ++i )
       {
       selection |= ( ( quint64 ) 1 ) << ( items[ i ] );
       }
                   
    mOperationService->asyncRestore( selection );
}

#ifdef FM_CHANGE_ORIENT_ENABLE
void FmRestoreView::on_rotateAction_triggered()
{
	if ( mainWindow()->orientation() == Qt::Vertical ) {
		mainWindow()->setOrientation( Qt::Horizontal );
	}
	else {
		mainWindow()->setOrientation( Qt::Vertical );
	}
}
#endif

void FmRestoreView::on_restoreAction_triggered()
{
    on_leftAction_triggered();
}

void FmRestoreView::adjustActions()
{
    if( mRestoreWigdet->backupDataCount() > 0 ) {
        mRestoreAction->setDisabled( false );
        mLeftAction->setDisabled( false );
    } else {
        mRestoreAction->setDisabled( true );
        mLeftAction->setDisabled( true );
    }
}

void FmRestoreView::removeToolBarAction()
{
    toolBar()->removeAction( mLeftAction );
}

