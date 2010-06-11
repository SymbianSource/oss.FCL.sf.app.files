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
#include "fmdlgutils.h"

#include <QApplication>

#include <hbaction.h>
#include <hbtoolbar.h>
#include <hbmenu.h>
#include <hbmainwindow.h>

FmRestoreView::FmRestoreView(): FmViewBase( ERestoreView )
{
	setTitle( hbTrId( "Restore" ) );

	initMainWidget();
	initToolBar();
	initMenu();  
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
    action->setText( hbTrId( "Change orientation" ) );
    menu()->addAction( action );
#endif

	mRestoreAction = new HbAction( this );
    mRestoreAction->setObjectName( "restoreAction" );
    mRestoreAction->setText( hbTrId( "Restore data" ) );
    menu()->addAction( mRestoreAction );
    mRestoreAction->setEnabled(false);
}

void FmRestoreView::initMainWidget()
{
	mRestoreWigdet = new FmRestoreWigdet( this );

    setWidget( mRestoreWigdet );
    connect(mRestoreWigdet, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxStateChange()));

}

void FmRestoreView::initToolBar()
{
	mLeftAction = new HbAction( this );
    mLeftAction->setObjectName( "leftAction" );
    mLeftAction->setText( hbTrId( "Restore" ) );
    toolBar()->addAction( mLeftAction );
    mLeftAction->setEnabled(false);
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
                   
    int ret = mOperationService->asyncRestore( selection );
    switch( ret )
    {
    case FmErrNone:
        break;
    case FmErrWrongParam:
        FmDlgUtils::information( QString( hbTrId("Operation canceled with wrong param!") ) );
        break;
    case FmErrAlreadyStarted:
        FmDlgUtils::information( QString( hbTrId("Operation canceled because already started!") ) );
        break;
    default:
        FmDlgUtils::information(tr("restore failed"));
        break;
    }
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

void FmRestoreView::removeToolBarAction()
{
    toolBar()->removeAction( mLeftAction );
}

void FmRestoreView::onCheckBoxStateChange()
{
    QList<int> items = mRestoreWigdet->selectionIndexes();
    if (items.count() > 0) {
        mLeftAction->setEnabled(true);
        mRestoreAction->setEnabled(true);
    } else {
        mLeftAction->setEnabled(false);
        mRestoreAction->setEnabled(false);
    }
}

void FmRestoreView::refreshRestoreView()
{
    mRestoreWigdet->refresh();
}
