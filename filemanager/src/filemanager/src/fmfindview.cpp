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
*     The source file of the file find view of file manager
*
*/

#include "fmfindview.h"
#include "fmfindwidget.h"
#include "fmviewmanager.h"

#include <hbprogressdialog.h>
#include <hbaction.h>
#include <hbmenu.h>

FmFindView::FmFindView() : FmViewBase( EFindView ),
                           mWaitNote( 0 ),
                           mMenu( 0 )
{
    initMenu();
    initMainWidget();
    initToolBar();

    QMetaObject::connectSlotsByName( this );
}

FmFindView::~FmFindView()
{
    delete mWaitNote;
    mWaitNote = 0;
}

void FmFindView::activated( const QString& pathName )
{
    FmLogger::log( QString( "activate path from findview:" ) + pathName );
    FmViewManager::viewManager()->createFileView( pathName, true, true );
}

void FmFindView::find( const QString &keyword, const QString &path )
{
    setTitle( keyword );
    mFindWidget->find( keyword, path );
    mWaitNote->open();
}

void FmFindView::stopFind()
{
    mFindWidget->stopFind();
}

void FmFindView::findFinished()
{
    if( mWaitNote ){
        mWaitNote->close();
    }
//    mFindWidget->activeSearchPanel();
}

void FmFindView::initMenu()
{
    HbAction *findAction = new HbAction( this );
    findAction->setObjectName( "findAction" );
    findAction->setText( hbTrId( "Find" ) );
    menu()->addAction( findAction ); 
    
    HbMenu *subMenu = new HbMenu( hbTrId( "Sort" ) );
    
    HbAction *sortNameAction = new HbAction( subMenu );
    sortNameAction->setObjectName( "sortNameAction" );
    sortNameAction->setText( hbTrId( "Sort by name" ) );
    subMenu->addAction( sortNameAction );
    
    HbAction *sortTimeAction = new HbAction( subMenu );
    sortTimeAction->setObjectName( "sortTimeAction" );
    sortTimeAction->setText( hbTrId( "Sort by time" ) );
    subMenu->addAction( sortTimeAction );
    
    HbAction *sortSizeAction = new HbAction( subMenu );
    sortSizeAction->setObjectName( "sortSizeAction" );
    sortSizeAction->setText( hbTrId( "Sort by size" ) );
    subMenu->addAction( sortSizeAction );
    
    HbAction *sortTypeAction = new HbAction( subMenu );
    sortTypeAction->setObjectName( "sortTypeAction" );
    sortTypeAction->setText( hbTrId( "Sort by type" ) );
    subMenu->addAction( sortTypeAction );
    
    menu()->addMenu( subMenu );
    
    connect( findAction, SIGNAL( triggered() ),
             this, SLOT( on_findAction_triggered() ) );
    connect( sortNameAction, SIGNAL( triggered() ),
             this, SLOT( on_sortNameAction_triggered() ) );
    connect( sortTimeAction, SIGNAL( triggered() ),
             this, SLOT( on_sortTimeAction_triggered() ) );
    connect( sortSizeAction, SIGNAL( triggered() ),
             this, SLOT( on_sortSizeAction_triggered() ) );
    connect( sortTypeAction, SIGNAL( triggered() ),
             this, SLOT( on_sortTypeAction_triggered() ) );
    
    mMenu = takeMenu();
}

void FmFindView::initMainWidget()
{
    mFindWidget = new FmFindWidget( this );
    connect( mFindWidget, SIGNAL(finished()), this, SLOT(findFinished()) );
    setWidget( mFindWidget );

    if( !mWaitNote ){
        mWaitNote = new HbProgressDialog( HbProgressDialog::WaitDialog );
        mWaitNote->setText( hbTrId( "Finding..." ) );
    }
    connect( mWaitNote, SIGNAL(cancelled()), this, SLOT(stopFind()) );
    connect( mFindWidget, SIGNAL(activated( const QString&)), this, SLOT( activated(const QString&)) );
    connect( mFindWidget, SIGNAL( startSearch( const QString&,  const QString& ) ),
             this, SLOT( startSearch( const QString&, const QString& ) ) );
    connect( mFindWidget, SIGNAL( setEmptyMenu( bool ) ),
             this, SLOT( on_findWidget_setEmptyMenu( bool ) ) );
}

void FmFindView::initToolBar()
{
}

void FmFindView::on_sortNameAction_triggered()
{
    mFindWidget->sortFiles( FmFindResultModel::Name  );
}

void FmFindView::on_sortTimeAction_triggered()
{
    mFindWidget->sortFiles( FmFindResultModel::Time  );
}

void FmFindView::on_sortSizeAction_triggered()
{
    mFindWidget->sortFiles( FmFindResultModel::Size  );
}

void FmFindView::on_sortTypeAction_triggered()
{
    mFindWidget->sortFiles( FmFindResultModel::Type  );
}

void FmFindView::startSearch( const QString &targetPath, const QString &criteria )
{  
    if ( !criteria.isEmpty( ) ) {
        find( criteria, targetPath );  
    }
}

void FmFindView::on_findAction_triggered()
{
    mFindWidget->activeSearchPanel();
}

void FmFindView::on_findWidget_setEmptyMenu( bool isMenuEmpty )
{
    if( isMenuEmpty ){
        if( !mMenu ) {
            mMenu = takeMenu();
        }
    }
    else{ 
        if( mMenu ) {
            setMenu( mMenu );
            mMenu = 0;
        }
    }
}
