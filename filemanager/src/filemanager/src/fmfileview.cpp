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
*     Steven Yao <steven.yao@nokia.com>
* 
* Description:
*     The source file of the file browse view of file manager
*
*/

#include "fmfileview.h"
#include "fmutils.h"
#include "fmfiledialog.h"
#include "fmfilebrowsewidget.h"
#include "fmviewmanager.h"
#include "fmoperationservice.h"
#include "fmdlgutils.h"

#include <QApplication>
#include <QGraphicsLinearLayout>

#include <hbinstance.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbtoolbar.h>
#include <hblineedit.h>
#include <hbmessagebox.h>
#include <hbpushbutton.h>
#include <hbmainwindow.h>

FmFileView::FmFileView() : FmViewBase( EFileView ), mWidget( 0 ),
    mUpButton( 0 ), mStyleAction( 0 ), mSelectableAction( 0 ),
    mFindAction( 0 ), mOperationService( 0 ), mMenu( 0 ) 
{
    mOperationService = FmViewManager::viewManager()->operationService();
	initMenu();
	initMainWidget();
    initToolBar();

	QMetaObject::connectSlotsByName( this );
}

FmFileView::~FmFileView()
{
    removeToolBarAction();
}

void FmFileView::setRootPath( const QString &pathName )
{
    if( pathName.isEmpty() ) {
        return;
    }
    mWidget->setRootPath( pathName );
    QString driveName = FmUtils::getDriveNameFromPath( pathName );
    /*
	FmDriverInfo driverInfo = FmUtils::queryDriverInfo( driveName );
    QString volumeName = driverInfo.volumeName();
	if( volumeName.isEmpty() ){
        FmDriverInfo::DriveState driveState = FmUtils::queryDriverInfo( driveName ).driveState();
        if( !( driveState & FmDriverInfo::EDriveNotPresent ) ){
            if( driveState & FmDriverInfo::EDriveRemovable ) {
                if( driveState & FmDriverInfo::EDriveMassStorage ) {
                    volumeName.append( hbTrId( "Mass Storage" ) );  
                }
                else{
                    volumeName.append( hbTrId( "Memory Card" ) );
                }
            }
            else{
                volumeName.append( hbTrId( "Phone Memory" ) );
            }
        }
	    }
	*/
	//QString titleText = FmUtils::removePathSplash( driverInfo.name() ) + ' ' + volumeName;
    QString titleText( FmUtils::fillDriveVolume( driveName, true ) );
	setTitle( titleText );
}

void FmFileView::setRootLevelPath( const QString &pathName )
{
    rootLevelPath = FmUtils::fillPathWithSplash( pathName );
}

void FmFileView::setFindDisabled( bool disable )
{
    if( mFindAction ) {
        mFindAction->setDisabled( disable );
    }
}

void FmFileView::initMenu()
{
    HbAction *action = 0;
#ifdef FM_CHANGE_ORIENT_ENABLE
	action = new HbAction( this );
	action->setObjectName( "rotateAction" );
	action->setText( hbTrId( "Change orientation" ) );
	menu()->addAction( action );
#endif

	mStyleAction = new HbAction( this );
	mStyleAction->setObjectName( "switchStyle" );
//	menu()->addAction( mStyleAction );

	mSelectableAction = new HbAction( this );
	mSelectableAction->setObjectName( "setSelectable" );
	menu()->addAction( mSelectableAction );

	action = new HbAction( this );
	action->setObjectName( "delete" );
	action->setText( hbTrId( "txt_fmgr_menu_delete" ) );
	menu()->addAction( action );

    action = new HbAction( this );
    action->setObjectName( "copy" );
    action->setText( hbTrId( "txt_fmgr_menu_copy" ) );
    menu()->addAction( action );

    action = new HbAction( this );
    action->setObjectName( "move" );
    action->setText( hbTrId( "txt_fmgr_menu_move" ) );
    menu()->addAction( action );
    
    action = new HbAction( this );
    action->setObjectName( "newFolder" );
    action->setText( hbTrId( "New Folder" ) );
    menu()->addAction( action );
    
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
    
    HbAction* sortTypeAction = new HbAction( subMenu );
    sortTypeAction->setObjectName( "sortTypeAction" );
    sortTypeAction->setText( hbTrId( "Sort by type" ) );
    subMenu->addAction( sortTypeAction );
    
    menu()->addMenu( subMenu );
	
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

void FmFileView::initMainWidget()
{
	QGraphicsLinearLayout *vLayout = new QGraphicsLinearLayout( this );
	vLayout->setOrientation( Qt::Vertical );

	mWidget = new FmFileBrowseWidget( this );
	mWidget->setObjectName( "mainWidget" );
	setStyle( FmFileBrowseWidget::ListStyle );
	setSelectable( false );

	vLayout->addItem( mWidget );
	//set stretch factor to file browser widget, so that it could fully cover the client area.
	vLayout->setStretchFactor( mWidget, 1 );

	setLayout( vLayout );
	
    connect( mWidget, SIGNAL( startSearch( const QString&,  const QString& ) ),
             this, SLOT( startSearch( const QString&, const QString& ) ) );
    connect( mWidget, SIGNAL( setEmptyMenu( bool ) ),
             this, SLOT( on_mainWidget_setEmptyMenu( bool ) ) );
    connect( mWidget, SIGNAL( setTitle( const QString & ) ),
             this, SLOT( on_mainWidget_setTitle( const QString & ) ) );
}

void FmFileView::initToolBar()
{
    toolBar()->clearActions();
    mFindAction = new HbAction( this );
    mFindAction->setObjectName( "leftAction" );
    mFindAction->setText( hbTrId("txt_fmgr_opt_find") );
    toolBar()->addAction( mFindAction );

    mToolBarRightAction = new HbAction( this );
    mToolBarRightAction->setObjectName( "rightAction" );
    mToolBarRightAction->setText( hbTrId( "up" ) );
    toolBar()->addAction( mToolBarRightAction );
    
    toolBar()->setOrientation( Qt::Horizontal );
    
    //mToolBar = takeToolBar();
    connect( mFindAction, SIGNAL( triggered() ),
                 this, SLOT( on_leftAction_triggered() ) );
    connect( mToolBarRightAction, SIGNAL( triggered() ),
             this, SLOT( on_rightAction_triggered() ) );
    
}

void FmFileView::setStyle( FmFileBrowseWidget::Style style )
{
	if ( style == FmFileBrowseWidget::ListStyle ) {
		mStyleAction->setText( hbTrId("Tree") );
	} else if ( style == FmFileBrowseWidget::TreeStyle ) {
		mStyleAction->setText( hbTrId("List") );
	}
	
	mWidget->setStyle( style );
}

void FmFileView::setSelectable( bool enable )
{
	if (enable) {
		mSelectableAction->setText( hbTrId("No Select") );
	} else {
		mSelectableAction->setText( hbTrId("Select") );
	}

	mWidget->setSelectable( enable );
}

void FmFileView::infoNoFileSelected()
{
	HbMessageBox::information( tr("No File/Folder selected" ) );
}

#ifdef FM_CHANGE_ORIENT_ENABLE
void FmFileView::on_rotateAction_triggered()
{
	if ( mainWindow()->orientation() == Qt::Vertical ) {
		mainWindow()->setOrientation( Qt::Horizontal );
	}
	else {
		mainWindow()->setOrientation( Qt::Vertical );
	}
}
#endif

void FmFileView::on_switchStyle_triggered()
{
	if (mWidget->style() == FmFileBrowseWidget::ListStyle) {
		setStyle( FmFileBrowseWidget::TreeStyle );
	} else if (mWidget->style() == FmFileBrowseWidget::TreeStyle) {
		setStyle( FmFileBrowseWidget::ListStyle );
	}
}

void FmFileView::on_setSelectable_triggered()
{
	setSelectable( !mWidget->selectable() );
}

void FmFileView::on_delete_triggered()
{
	QList<QFileInfo> files = mWidget->checkedItems();
    if (files.size() == 0) {
		infoNoFileSelected();
    } else {
        if (HbMessageBox::question( tr("Confirm Deletion?" ) )) {
            QStringList fileList;
            for (int i = 0; i < files.size(); ++i) {
                fileList.push_back( files[i].absoluteFilePath() );
                }
            int ret = mOperationService->asyncRemove( fileList );
            switch( ret ) {
                case FmErrNone:
                    // no error, do not show note to user
                    break;
                case FmErrAlreadyStarted:
                    // last operation have not finished
                    HbMessageBox::information( hbTrId( "Operatin already started!" ) );
                    break;
                case FmErrWrongParam:
                    HbMessageBox::information( hbTrId( "Wrong parameters!" ) );
                    break;
                default:
                    HbMessageBox::information( hbTrId( "Operation fail to start!" ) );
            }
            setSelectable( false );
        }
    }
}

void FmFileView::on_copy_triggered()
{
    QList<QFileInfo> files = mWidget->checkedItems();
    QStringList srcFileList;

    if (files.size() == 0) {
        infoNoFileSelected();
    } else {
        QString targetPathName = FmFileDialog::getExistingDirectory( 0, hbTrId( "copy to" ),
            QString(""), QStringList() );
        if( !targetPathName.isEmpty() && files.size() > 0 ) {
            targetPathName = FmUtils::fillPathWithSplash( targetPathName );

            foreach( QFileInfo fileInfo, files ){
                srcFileList.push_back( fileInfo.absoluteFilePath() );
            }

            int ret = mOperationService->asyncCopy(
                srcFileList, targetPathName );
            switch( ret ) {
                case FmErrNone:
                    // no error, do not show note to user
                    break;
                case FmErrAlreadyStarted:
                    // last operation have not finished
                    HbMessageBox::information( hbTrId( "Operatin already started!" ) );
                    break;
                case FmErrWrongParam:
                    HbMessageBox::information( hbTrId( "Wrong parameters!" ) );
                    break;
                default:
                    HbMessageBox::information( hbTrId( "Operation fail to start!" ) );
            }
            setSelectable( false );
        }
    }

}
 
void FmFileView::on_move_triggered()
{
    QList<QFileInfo> files = mWidget->checkedItems();

    if (files.size() == 0) {
        infoNoFileSelected();
    } else {
        QString targetPathName = FmFileDialog::getExistingDirectory( 0, tr( "move to" ),
            QString(""), QStringList() );

        if( !targetPathName.isEmpty() && files.size() > 0 ) {
            targetPathName = FmUtils::fillPathWithSplash( targetPathName );

            QStringList fileList;
            for (int i = 0; i < files.size(); ++i) {
                fileList.push_back( files[i].absoluteFilePath() );
            }
            int ret = mOperationService->asyncMove( fileList, targetPathName );
            switch( ret ) {
                case FmErrNone:
                    // no error, do not show note to user
                    break;
                case FmErrAlreadyStarted:
                    // last operation have not finished
                    HbMessageBox::information( tr( "Operatin already started!" ) );
                    break;
                case FmErrWrongParam:
                    HbMessageBox::information( tr( "Wrong parameters!" ) );
                    break;
                default:
                    HbMessageBox::information( tr( "Operation fail to start!" ) );
            }
            setSelectable( false );
        }
    }
}

void FmFileView::on_newFolder_triggered()
{
    QString dirName;
    QString path = FmUtils::fillPathWithSplash( mWidget->currentPath().absoluteFilePath() );
    QDir dir( path );
    if( dir.exists() ) {
        while( FmDlgUtils::showTextQuery( hbTrId( "Enter name for " ), dirName, true ) ){
                QString newTargetPath = FmUtils::fillPathWithSplash(
                    dir.absolutePath() ) + dirName;
                QFileInfo newFileInfo( newTargetPath );
                if( newFileInfo.exists() ) {
                    HbMessageBox::information( hbTrId( "%1 already exist!" ).arg( dirName ) );
                    continue;
                }
    
                if( !dir.mkdir( dirName ) ) {
                    HbMessageBox::information( hbTrId("Operation failed!") );
                }
                refreshModel( path );
                break;
            }
        
    }
}
void FmFileView::on_upAction_triggered()
{
	mWidget->cdUp();
}

void FmFileView::on_leftAction_triggered()
{
//    FmFindDialog findDialog;
//    QString keyword;
//    QString findFolder;
//    bool ret = findDialog.exec( keyword, findFolder );
//
//    if ( ret && !keyword.isEmpty() && !findFolder.isEmpty() )
//        FmViewManager::viewManager()->createFindView( keyword, findFolder );
    mWidget->activeSearchPanel();
}

void FmFileView::on_rightAction_triggered()
{
    QString currentPath( 
        FmUtils::fillPathWithSplash( mWidget->currentPath().filePath() ) );
    if( rootLevelPath.length() != 0 &&
        rootLevelPath.compare( currentPath, Qt::CaseInsensitive ) == 0  ) {
        emit popViewAndShow();
    } else if ( !mWidget->cdUp() ) {
		//hbInstance->allMainWindows()[0]->softKeyAction(Hb::SecondarySoftKey)->trigger();
        emit popViewAndShow();
	}
}

void FmFileView::refreshModel( const QString &path )
{
    mWidget->refreshModel( path );  
}

void FmFileView::on_sortNameAction_triggered()
{
    mWidget->sortFiles( FmFileBrowseWidget::ESortByName );
}

void FmFileView::on_sortTimeAction_triggered()
{
    mWidget->sortFiles( FmFileBrowseWidget::ESortByTime );
}

void FmFileView::on_sortSizeAction_triggered()
{
    mWidget->sortFiles( FmFileBrowseWidget::ESortBySize );
}

void FmFileView::on_sortTypeAction_triggered()
{
    mWidget->sortFiles( FmFileBrowseWidget::ESortByType );
}

void FmFileView::startSearch( const QString &targetPath, const QString &criteria )
{    
    if ( !criteria.isEmpty() && !targetPath.isEmpty() ) {
        FmViewManager::viewManager()->createFindView( criteria, targetPath );
    }
}

void FmFileView::removeToolBarAction()
{
    toolBar()->clearActions();
}

void FmFileView::on_mainWidget_setEmptyMenu( bool isMenuEmpty )
{
    if( isMenuEmpty ){
        FmLogger::log( "setEmptyMenu true" );
        if( !mMenu ) {
            mMenu = takeMenu();
        }
        toolBar()->clearActions();
    }
    else {
        FmLogger::log( "setEmptyMenu false" );
        if( mMenu ) {
            setMenu( mMenu );
            mMenu = 0;
        }
        initToolBar();
    }
}

void FmFileView::on_mainWidget_setTitle( const QString &title )
{
    this->setTitle( title );
}
