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
#include <hbinputdialog.h>
#include <hbprogressnote.h>



FmFileView::FmFileView() : FmViewBase( EFileView ), mWidget( 0 ),
    mUpButton( 0 ), mStyleAction( 0 ), mSelectableAction( 0 ),
    mFindAction( 0 ), mOperationService( 0 )
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
	FmDriverInfo driverInfo = FmUtils::queryDriverInfo( driveName );
	QString volumeName = driverInfo.volumeName();
	if( volumeName.isEmpty() ){
        quint32 driveState = FmUtils::getDriverState( driveName );
        if( !( driveState & FmDriverInfo::EDriveNotPresent ) ){
            if( driveState & FmDriverInfo::EDriveRemovable ) {
                if( driveState & FmDriverInfo::EDriveMassStorage ) {
                    volumeName.append( tr( "Mass Storage" ) );  
                }
                else{
                    volumeName.append( tr( "Memory Card" ) );
                }
            }
            else{
                volumeName.append( tr( "Phone Memory" ) );
            }
        }
	    }
	
	QString titleText = FmUtils::removePathSplash( driverInfo.name() ) + ' ' + volumeName;
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
	action->setText( tr( "Change orientation" ) );
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
	action->setText( tr( "delete" ) );
	menu()->addAction( action );

    action = new HbAction( this );
    action->setObjectName( "copy" );
    action->setText( tr( "copy to" ) );
    menu()->addAction( action );

    action = new HbAction( this );
    action->setObjectName( "move" );
    action->setText( tr( "move to" ) );
    menu()->addAction( action );
    
    
    HbMenu *subMenu = new HbMenu( "Sort" );
    HbAction *sortNameAction = new HbAction( subMenu );
    sortNameAction->setObjectName( "sortNameAction" );
    sortNameAction->setText( tr( "Sort by name" ) );
    subMenu->addAction( sortNameAction );
    
    HbAction *sortTimeAction = new HbAction( subMenu );
    sortTimeAction->setObjectName( "sortTimeAction" );
    sortTimeAction->setText( tr( "Sort by time" ) );
    subMenu->addAction( sortTimeAction );
    
    HbAction *sortSizeAction = new HbAction( subMenu );
    sortSizeAction->setObjectName( "sortSizeAction" );
    sortSizeAction->setText( tr( "Sort by size" ) );
    subMenu->addAction( sortSizeAction );
    
    HbAction* sortTypeAction = new HbAction( subMenu );
    sortTypeAction->setObjectName( "sortTypeAction" );
    sortTypeAction->setText( tr( "Sort by type" ) );
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
}

void FmFileView::initMainWidget()
{
	QGraphicsLinearLayout *vLayout = new QGraphicsLinearLayout( this );
	vLayout->setOrientation( Qt::Vertical );

	mWidget = new FmFileBrowseWidget( this );
	setStyle( FmFileBrowseWidget::ListStyle );
	setSelectable( false );

	vLayout->addItem( mWidget );
	//set stretch factor to file browser widget, so that it could fully cover the client area.
	vLayout->setStretchFactor( mWidget, 1 );

	setLayout( vLayout );
	
    connect( mWidget, SIGNAL( startSearch( const QString&,  const QString& ) ),
             this, SLOT( startSearch( const QString&, const QString& ) ) );
}

void FmFileView::initToolBar()
{
    mFindAction = new HbAction( this );
    mFindAction->setObjectName( "leftAction" );
    mFindAction->setText( tr("find") );
    toolBar()->addAction( mFindAction );

    mToolBarRightAction = new HbAction( this );
    mToolBarRightAction->setObjectName( "rightAction" );
    mToolBarRightAction->setText( hbTrId( "up" ) );
    toolBar()->addAction( mToolBarRightAction );
    
    toolBar()->setOrientation( Qt::Horizontal );
}

void FmFileView::setStyle( FmFileBrowseWidget::Style style )
{
	if ( style == FmFileBrowseWidget::ListStyle ) {
		mStyleAction->setText( tr("Tree") );
	} else if ( style == FmFileBrowseWidget::TreeStyle ) {
		mStyleAction->setText( tr("List") );
	}
	
	mWidget->setStyle( style );
}

void FmFileView::setSelectable( bool enable )
{
	if (enable) {
		mSelectableAction->setText( tr("No Select") );
	} else {
		mSelectableAction->setText( tr("Select") );
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

void FmFileView::on_copy_triggered()
{
    QList<QFileInfo> files = mWidget->checkedItems();
    QStringList srcFileList;

    if (files.size() == 0) {
        infoNoFileSelected();
    } else {
        QString targetPathName = FmFileDialog::getExistingDirectory( 0, tr( "copy to" ),
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
    toolBar()->removeAction( mFindAction );
    toolBar()->removeAction( mToolBarRightAction );
}

