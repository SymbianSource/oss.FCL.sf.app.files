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
 *     The source file of File Manager driver view
 */

#include "fmdriverview.h"
#include "fmdriverlistwidget.h"
#include "fmviewmanager.h"
#include "fmcommon.h"

#include <QApplication>

#include <hbmessagebox.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbtoolbar.h>
#include <hbmainwindow.h>
#include <hbsearchpanel.h>

FmDriverView::FmDriverView() : FmViewBase( EDriverView )
{
    FmLogger::log( "FmDriverView::FmDriverView" );
    initMenu(); 
    initToolBar();
    initDiskListWidget();

    QMetaObject::connectSlotsByName( this );
}

FmDriverView::~FmDriverView()
{
    removeToolBarAction();
}

void FmDriverView::initMenu()
{
    HbAction *action = 0;

#ifdef FM_CHANGE_ORIENT_ENABLE
    action = new HbAction( this );
    action->setObjectName( "rotateAction" );
    action->setText( hbTrId( "Change orientation" ) );
    menu()->addAction( action );
#endif

    action = new HbAction( this );
    action->setObjectName( "findAction" );
    action->setText( hbTrId( "txt_fmgr_opt_find" ) );
    menu()->addAction( action );

    action = new HbAction( this );
    action->setObjectName( "backupAction" );
    action->setText( hbTrId( "txt_fmgr_opt_backup" ) );
    menu()->addAction( action );

    action = new HbAction( this );
    action->setObjectName( "restoreAction" );
    action->setText( hbTrId( "txt_fmgr_opt_restore" )  );
    menu()->addAction( action );
   
    action = new HbAction( this );
    action->setObjectName( "exitAction" );
    action->setText( hbTrId( "Exit" ) );
    menu()->addAction( action );

}

void FmDriverView::initDiskListWidget()
{
    mDriverList = new FmDriverListWidget( this );
    connect( mDriverList, SIGNAL( activated( const QString& ) ),
             this, SLOT( activated( const QString& ) ) );
    
    connect( mDriverList, SIGNAL( startSearch( const QString&,  const QString& ) ),
             this, SLOT( startSearch( const QString&, const QString& ) ) );
    
    setWidget( mDriverList );
}

void FmDriverView::initToolBar()
{ 
    mToolBarLeftAction = new HbAction( this );
    mToolBarLeftAction->setObjectName( "leftAction" );
    mToolBarLeftAction->setText( hbTrId( "txt_fmgr_opt_backup" ) );
    toolBar()->addAction( mToolBarLeftAction );

    mToolBarRightAction = new HbAction( this );
    mToolBarRightAction->setObjectName( "rightAction" );
    mToolBarRightAction->setText( hbTrId( "txt_fmgr_opt_restore" ) );
    toolBar()->addAction( mToolBarRightAction );
	
    toolBar()->setOrientation( Qt::Horizontal );
}

void FmDriverView::activated( const QString& pathName )
{
    FmViewManager::viewManager()->createFileView( pathName );
}

void FmDriverView::refreshModel( const QString &path  )
{
    FmLogger::log( QString( "FmDriverView::refreshModel start" ) );
    QString logstring = QString( "Refresh Path:" );
    logstring.append( path );
    mDriverList->refreshModel( path );
    FmLogger::log( QString( "FmDriverView::refreshModel end" ) );
}

void FmDriverView::on_leftAction_triggered()
{
    FmViewManager::viewManager()->createBackupView();
}

void FmDriverView::on_rightAction_triggered()
{
	FmViewManager::viewManager()->createRestoreView();
}

#ifdef FM_CHANGE_ORIENT_ENABLE
void FmDriverView::on_rotateAction_triggered()
{
	if ( mainWindow()->orientation() == Qt::Vertical ) {
		mainWindow()->setOrientation( Qt::Horizontal );
	}
	else {
		mainWindow()->setOrientation( Qt::Vertical );
	}
}
#endif

void FmDriverView::on_exitAction_triggered()
{
    qApp->quit();
}

void FmDriverView::on_backupAction_triggered()
{
	FmViewManager::viewManager()->createBackupView();
}

void FmDriverView::on_restoreAction_triggered()
{
	FmViewManager::viewManager()->createRestoreView();
}

void FmDriverView::on_findAction_triggered()
{
    mDriverList->activeSearchPanel();  
}

void FmDriverView::startSearch( const QString &targetPath, const QString &criteria )
{    
    if ( !criteria.isEmpty() && !targetPath.isEmpty() ) {
        FmViewManager::viewManager()->createFindView( criteria, targetPath );
    }
}

void FmDriverView::removeToolBarAction()
{
    toolBar()->removeAction( mToolBarLeftAction );
    toolBar()->removeAction( mToolBarRightAction );
}


