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
 *     Yong Zhang <yong.5.zhang@nokia.com>
 * 
 * Description:
 *     The driver model file for file manager
 */
#include "fmdriverlistwidget.h"
#include "listviewitems.h"
#include "fmutils.h"
#include "fmdlgutils.h"
#include "fmviewmanager.h"
#include "fmviewdetailsdialog.h"
#include "fmoperationbase.h"
#include "fmdrivemodel.h"
#include "fmfiledialog.h"

#include <QDirModel>
#include <QGraphicsLinearLayout>
#include <QFileSystemWatcher>

#include <hblistview.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbmessagebox.h>
#include <hbsearchpanel.h>

FmDriverListWidget::FmDriverListWidget( QGraphicsItem *parent )
: HbWidget( parent ),
  mCurrentItem( 0 ),
  mOperationService( 0 )
{
	init();
	mOperationService = FmViewManager::viewManager()->operationService();

    mFileSystemWatcher = new QFileSystemWatcher( this );
    connect( mFileSystemWatcher, SIGNAL( directoryChanged ( const QString & ) ), 
            this, SLOT( on_directoryChanged( const QString & ) ) ); 
    
    QMetaObject::connectSlotsByName( this );
}

FmDriverListWidget::~FmDriverListWidget()
{
}

void FmDriverListWidget::on_list_activated( const QModelIndex &index )
{
	emit activated( mModel->driveName( index ) );
}

void FmDriverListWidget::init()
{
	mLayout = new QGraphicsLinearLayout( this );
	mLayout->setOrientation( Qt::Vertical );

	mListView = new HbListView( this );
	mListView->setSelectionMode( HbAbstractItemView::SingleSelection );

	mModel = new FmDriveModel( this,
         FmDriveModel::FillWithVolume | FmDriveModel::FillWithDefaultVolume );
	mListView->setModel( mModel );

	mLayout->addItem( mListView );
	
	mSearchPanel = new HbSearchPanel( this );
	mSearchPanel->setObjectName( "searchPanel" );
	mSearchPanel->setSearchOptionsEnabled( true );
	mSearchPanel->setProgressive( false );
	mSearchPanel->hide();
//	mLayout->addItem( mSearchPanel );
    
	mListView->setItemPrototype( new DiskListViewItem( mListView ) );
	connect( mListView, SIGNAL( activated( const QModelIndex & ) ),
		     this, SLOT( on_list_activated( const QModelIndex & ) ) );

    connect( mListView, SIGNAL( longPressed( HbAbstractViewItem *, const QPointF & ) ),
        this, SLOT( on_list_longPressed( HbAbstractViewItem *, const QPointF & ) ) );
    
    connect( mSearchPanel, SIGNAL( searchOptionsClicked() ),
        this, SLOT( on_searchPanel_searchOptionsClicked() ) );
    
    connect( mSearchPanel, SIGNAL( criteriaChanged( const QString & ) ),
        this, SLOT( on_searchPanel_criteriaChanged( const QString & ) ) );
       
    connect( mSearchPanel, SIGNAL( exitClicked() ),
        this, SLOT( on_searchPanel_exitClicked() ) );
    
    setLayout( mLayout );
}

void FmDriverListWidget::refreshModel( const QString &path )
{
    Q_UNUSED( path );
    mModel->refresh();
}

void FmDriverListWidget::on_list_longPressed( HbAbstractViewItem *item, const QPointF &coords )
{   
    HbMenu *contextMenu = new HbMenu();

    mCurrentItem = item;
    QString diskName = mCurrentItem->modelIndex().data( Qt::DisplayRole ).toString();

    quint32 state = FmUtils::getDriverState( diskName );
    FmDriverInfo driverInfo = FmUtils::queryDriverInfo( diskName );

    if( !( state & FmDriverInfo::EDriveNotPresent ) ) {
        HbAction *viewAction = new HbAction();
        viewAction->setObjectName( "viewAction" );
        viewAction->setText( tr( "View details" ) );
        contextMenu->addAction( viewAction );

        //state = 0x210;
    
        connect( viewAction, SIGNAL( triggered() ),
        this, SLOT( on_viewAction_triggered() ) );
    }

    HbMenu *subMenu = new HbMenu( "Memory name" );
    if( ( state & FmDriverInfo::EDriveRemovable ) && !( state & FmDriverInfo::EDriveMassStorage ) ){
        if ( driverInfo.volumeName().length() ){
            HbAction *renameAction = new HbAction();
            renameAction->setObjectName( "renameAction" );
            renameAction->setText( tr( "Rename" ) );
            subMenu->addAction( renameAction );

            connect( renameAction, SIGNAL( triggered() ),
		     this, SLOT( on_renameAction_triggered() ) );
        }
        else{
            HbAction *nameAction = new HbAction();
            nameAction->setObjectName( "nameAction" );
            nameAction->setText( tr( "Name" ) );
            subMenu->addAction( nameAction );

            connect( nameAction, SIGNAL( triggered() ),
		     this, SLOT( on_nameAction_triggered() ) );
        }
        contextMenu->addMenu( subMenu );

        subMenu = new HbMenu( "Memory password" );
        if( state & FmDriverInfo::EDrivePasswordProtected ){
            HbAction *changePwdAction = new HbAction();
            changePwdAction->setObjectName( "changePwdAction" );
            changePwdAction->setText( tr( "Change Password" ) );
            subMenu->addAction( changePwdAction );

            HbAction *removePwdAction = new HbAction();
            removePwdAction->setObjectName( "removePwdAction" );
            removePwdAction->setText( tr( "Remove Password" ) );
            subMenu->addAction( removePwdAction );

            connect( changePwdAction, SIGNAL( triggered() ),
		     this, SLOT( on_changePwdAction_triggered() ) );
            connect( removePwdAction, SIGNAL( triggered() ),
		     this, SLOT( on_removePwdAction_triggered() ) );
        }
        else{
            HbAction *setPwdAction = new HbAction();
            setPwdAction->setObjectName( "setPwdAction" );
            setPwdAction->setText( tr( "Set Password" ) );
            subMenu->addAction( setPwdAction );

            connect( setPwdAction, SIGNAL( triggered() ),
		     this, SLOT( on_setPwdAction_triggered() ) );
        }
        contextMenu->addMenu( subMenu );
         
        if( state & FmDriverInfo::EDriveEjectable ){
            HbAction *ejectAction = new HbAction();
            ejectAction->setObjectName( "ejectAction" );
            ejectAction->setText( tr( "Eject" ) );
            contextMenu->addAction( ejectAction );
            
            connect( ejectAction, SIGNAL( triggered() ),
            this, SLOT( on_ejectAction_triggered() ) );
        }         
    }
#ifndef _DEBUG_ENABLE_FORMATMENU_
    if ( state & FmDriverInfo::EDriveRemovable ){
        if( state & FmDriverInfo::EDriveFormattable ){
#endif
            HbAction *formatAction = new HbAction();
            formatAction->setObjectName( "formatAction" );
            formatAction->setText( tr( "Format" ) );
            contextMenu->addAction( formatAction );

            connect( formatAction, SIGNAL( triggered() ),
             this, SLOT( on_formatAction_triggered() ) );
#ifndef _DEBUG_ENABLE_FORMATMENU_
        }
    }
#endif
    
    if( state & FmDriverInfo::EDriveLocked ){
        HbAction *unLockedAction = new HbAction();
        unLockedAction->setObjectName( "unLockedAction" );
        unLockedAction->setText( tr( "Unlock" ) );
        contextMenu->addAction( unLockedAction );

        connect( unLockedAction, SIGNAL( triggered() ),
         this, SLOT( on_unLockedAction_triggered() ) );
    } 

    contextMenu->exec( coords );   
}

void FmDriverListWidget::on_viewAction_triggered()
{
    QString diskName = mCurrentItem->modelIndex().data( Qt::DisplayRole ).toString();
    mOperationService->asyncViewDriveDetails( diskName );
}

void FmDriverListWidget::on_renameAction_triggered()
{    
    QString diskName = mCurrentItem->modelIndex().data( Qt::DisplayRole ).toString();

    quint32 state = FmUtils::getDriverState( diskName );
    FmDriverInfo driverInfo = FmUtils::queryDriverInfo( diskName );

    if ( state & FmDriverInfo::EDriveWriteProtected ){
        HbMessageBox::information( tr( "Unable to perform operation. Memory card is read only." ) );
        return;
        }

    QString title( tr( "Drive name ") );  
    QString volumeName = driverInfo.volumeName();

    if( FmDlgUtils::showTextQuery( title, volumeName ) ){

        if ( FmUtils::renameDrive( diskName, volumeName ) == 0 ){
             HbMessageBox::information( tr( "The name has been changed!" ) );
             mModel->refresh();
        }
        else{
             HbMessageBox::information( tr( "Error occurred, operation cancelled!" ) );
        }                
    }
}

void FmDriverListWidget::on_nameAction_triggered()
{
    on_renameAction_triggered();
}

void FmDriverListWidget::on_changePwdAction_triggered()
{
    QString title( tr( "Password: ")  );
    QString firstLabel( tr( "New Password: ") );
    QString secondLabel( tr( "Confirm new Password: ") );

    QString oldPwd;
    QString newPwd;

    QString diskName = mCurrentItem->modelIndex().data( Qt::DisplayRole ).toString();

    if( FmDlgUtils::showConfirmPasswordQuery( title, diskName, oldPwd ) ){
        if( FmDlgUtils::showChangePasswordQuery( firstLabel, secondLabel, newPwd ) ){
            if ( FmUtils::setDrivePwd( diskName, oldPwd, newPwd ) == 0 ){
                HbMessageBox::information( tr( "The password has been changed!" ) );
            }
            else{
                HbMessageBox::information( tr( "Error occurred, operation cancelled!" ) );
            }
        }
    }
}

void FmDriverListWidget::on_removePwdAction_triggered()
{
    QString title( tr( "Password: ")  );

    QString oldPwd;

    QString diskName = mCurrentItem->modelIndex().data( Qt::DisplayRole ).toString();
    //QString password( tr( "a ") );
    if( HbMessageBox::question( tr( "Do you want to remove the password? Memory card becomes unlocked." ) ) ){
        if( FmDlgUtils::showConfirmPasswordQuery( title, diskName, oldPwd ) ){

            if ( FmUtils::removeDrivePwd( diskName, oldPwd ) == 0 ){
                HbMessageBox::information( tr( "The password has been removed!" ) );
            }
            else{
                HbMessageBox::information( tr( "Error occurred, operation cancelled!" ) );
            }
        }
    }
}

void FmDriverListWidget::on_unLockedAction_triggered()
{
    QString title( tr( "Password: ")  );

    QString oldPwd;

    QString diskName = mCurrentItem->modelIndex().data( Qt::DisplayRole ).toString();
    
    if( FmDlgUtils::showConfirmPasswordQuery( title, diskName, oldPwd ) ){

        if ( FmUtils::unlockDrive( diskName, oldPwd ) == 0 ){
            HbMessageBox::information( tr( "The memory is unlocked!" ) );
        }
        else{
            HbMessageBox::information( tr( "Error occurred, operation cancelled!" ) );
        }
    }
}

void FmDriverListWidget::on_setPwdAction_triggered()
{
    QString firstLabel( tr( "New Password: ") );
    QString secondLabel( tr( "Confirm new Password: ") );

    QString oldPwd;
    QString newPwd;

    FmUtils::emptyPwd( oldPwd );

    QString diskName = mCurrentItem->modelIndex().data( Qt::DisplayRole ).toString();

    if( FmDlgUtils::showChangePasswordQuery( firstLabel, secondLabel, newPwd ) ){
        if ( FmUtils::setDrivePwd( diskName, oldPwd, newPwd ) == 0 ){
            HbMessageBox::information( tr( "The password has been set!" ) );
        }
        else{
            HbMessageBox::information( tr( "Error occurred, operation cancelled!" ) );
        }
    }
}

void FmDriverListWidget::on_formatAction_triggered()
{
    QString diskName = mCurrentItem->modelIndex().data( Qt::DisplayRole ).toString();

    if( HbMessageBox::question( tr( "Format? Data will be deleted during formatting." ) ) ){
        if( FmErrNone != mOperationService->asyncFormat( diskName ) )
            HbMessageBox::information( tr( "Formatting failed." ) );
        }

}

void FmDriverListWidget::on_ejectAction_triggered()
{
    QString diskName = mCurrentItem->modelIndex().data( Qt::DisplayRole ).toString();

    if( HbMessageBox::question( tr( "Eject memory card? Some applications will be closed." ) ) ){
        if( !FmUtils::ejectDrive( diskName ) ){  
            HbMessageBox::information( tr( "Error occurred, operation cancelled!") );
        }
    }
}

void FmDriverListWidget::on_directoryChanged( const QString &path )
{
    Q_UNUSED( path );
    mModel->refresh();
}

void FmDriverListWidget::activeSearchPanel()
{
    mFindTargetPath.clear();
    mLayout->addItem( mSearchPanel );
    mSearchPanel->show();
}

void FmDriverListWidget::on_searchPanel_searchOptionsClicked()
{
    mFindTargetPath = FmFileDialog::getExistingDirectory( 0, tr( "Look in:" ),
        QString(""), QStringList() );
}

void FmDriverListWidget::on_searchPanel_criteriaChanged( const QString &criteria )
{
    emit startSearch( mFindTargetPath, criteria );
    mSearchPanel->hide();
    mLayout->removeItem( mSearchPanel );
}

void FmDriverListWidget::on_searchPanel_exitClicked()
{
    mSearchPanel->hide();
    mLayout->removeItem( mSearchPanel );
}
