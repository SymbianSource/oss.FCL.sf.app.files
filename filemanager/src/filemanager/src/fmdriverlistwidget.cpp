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

#include <QDir>
#include <QGraphicsLinearLayout>
#include <QFileSystemWatcher>

#include <hblistview.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbsearchpanel.h>

FmDriverListWidget::FmDriverListWidget( QGraphicsItem *parent )
: HbWidget( parent ), mListView(0), mModel(0),
  mCurrentItem(0), mSearchPanel(0),  mOperationService(0),
  mFileSystemWatcher(0), mLayout(0), mContextMenu(0), mListLongPressed( false )
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
	if (mContextMenu) {
        mContextMenu->deleteLater();
	}
}

void FmDriverListWidget::on_list_activated( const QModelIndex &index )
{
    FM_LOG("FmDriverListWidget::on_list_activated");
    if( mListLongPressed ) {
        FM_LOG("FmDriverListWidget::on_list_activated return because long pressed");
        return;
    }
    FM_LOG("FmDriverListWidget::on_list_activated emit activate to open drive");
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
    
	mListView->setItemPrototype( new DiskListViewItem(this) );
	connect( mListView, SIGNAL( activated( const QModelIndex & ) ),
		     this, SLOT( on_list_activated( const QModelIndex & ) ) );
	connect( mListView, SIGNAL( pressed( const QModelIndex & ) ),
	             this, SLOT( on_list_pressed( const QModelIndex & ) ) );

    connect( mListView, SIGNAL( longPressed( HbAbstractViewItem *, const QPointF & ) ),
        this, SLOT( on_list_longPressed( HbAbstractViewItem *, const QPointF & ) ) );
    
    connect( mSearchPanel, SIGNAL( searchOptionsClicked() ),
        this, SLOT( on_searchPanel_searchOptionsClicked() ), Qt::QueuedConnection );
    
    connect( mSearchPanel, SIGNAL( criteriaChanged( const QString & ) ),
        this, SLOT( on_searchPanel_criteriaChanged( const QString & ) ) );
       
    connect( mSearchPanel, SIGNAL( exitClicked() ),
        this, SLOT( on_searchPanel_exitClicked() ) );
    
    setLayout( mLayout );
}

void FmDriverListWidget::refreshDrive()
{
    mModel->refresh();
}

void FmDriverListWidget::on_list_longPressed( HbAbstractViewItem *item, const QPointF &coords )
{   
    FM_LOG("FmDriverListWidget::on_list_longPressed");
    mListLongPressed = true;
    mCurrentItem = item;
    QString diskName = mModel->driveName( mCurrentItem->modelIndex() );

	if( !mContextMenu ) {
		mContextMenu = new HbMenu();
	} else {
		mContextMenu->clearActions();
	}
	FmDriverInfo driverInfo = FmUtils::queryDriverInfo( diskName );
    FmDriverInfo::DriveState state = driverInfo.driveState();
    if( !( state & FmDriverInfo::EDriveNotPresent ) ) {
        if( state & FmDriverInfo::EDriveAvailable ) {
            HbAction *viewAction = new HbAction();
            viewAction->setObjectName( "viewAction" );
            viewAction->setText( hbTrId( "txt_fmgr_menu_view_details_memory" ) );
            mContextMenu->addAction( viewAction );
    
            //state = 0x210;
        
            connect( viewAction, SIGNAL( triggered() ),
                this, SLOT( on_viewAction_triggered() ), Qt::QueuedConnection );
        }
    
        if( ( state & FmDriverInfo::EDriveAvailable ) && ( state & FmDriverInfo::EDriveRemovable ) && !( state & FmDriverInfo::EDriveMassStorage ) ){
            // MMC or Usb memory
            if ( driverInfo.volumeName().length() ){
                HbAction *renameAction = new HbAction();
                renameAction->setObjectName( "renameAction" );
                renameAction->setText( hbTrId( "txt_fmgr_menu_rename" ) );
                mContextMenu->addAction( renameAction );
    
                connect( renameAction, SIGNAL( triggered() ),
                 this, SLOT( on_renameAction_triggered() ), Qt::QueuedConnection );
            }
            else{
                HbAction *nameAction = new HbAction();
                nameAction->setObjectName( "nameAction" );
                nameAction->setText( hbTrId( "txt_fmgr_menu_name" ) );
                mContextMenu->addAction( nameAction );
    
                connect( nameAction, SIGNAL( triggered() ),
                 this, SLOT( on_nameAction_triggered() ), Qt::QueuedConnection );
            }
            if( !( state & FmDriverInfo::EDriveUsbMemory ) ) { // MMC
                if( state & FmDriverInfo::EDrivePasswordProtected ){
                    HbAction *changePwdAction = new HbAction();
                    changePwdAction->setObjectName( "changePwdAction" );
                    changePwdAction->setText( hbTrId( "txt_fmgr_menu_change_password" ) );
                    mContextMenu->addAction( changePwdAction );
        
                    HbAction *removePwdAction = new HbAction();
                    removePwdAction->setObjectName( "removePwdAction" );
                    removePwdAction->setText( hbTrId( "txt_fmgr_menu_remove_password" ) );
                    mContextMenu->addAction( removePwdAction );
        
                    connect( changePwdAction, SIGNAL( triggered() ),
                     this, SLOT( on_changePwdAction_triggered() ), Qt::QueuedConnection );
                    connect( removePwdAction, SIGNAL( triggered() ),
                     this, SLOT( on_removePwdAction_triggered() ), Qt::QueuedConnection );
                }
                else{
                    HbAction *setPwdAction = new HbAction();
                    setPwdAction->setObjectName( "setPwdAction" );
                    setPwdAction->setText( hbTrId( "txt_fmgr_menu_set_password" ) );
                    mContextMenu->addAction( setPwdAction );
        
                    connect( setPwdAction, SIGNAL( triggered() ),
                     this, SLOT( on_setPwdAction_triggered() ), Qt::QueuedConnection );
                }
            }
        }
        
        if( state & FmDriverInfo::EDriveEjectable ){
            HbAction *ejectAction = new HbAction();
            ejectAction->setObjectName( "ejectAction" );
            ejectAction->setText( hbTrId( "txt_fmgr_menu_eject" ) );
            mContextMenu->addAction( ejectAction );
            
            connect( ejectAction, SIGNAL( triggered() ),
            this, SLOT( on_ejectAction_triggered() ), Qt::QueuedConnection );
        }  
        
    #ifndef _DEBUG_ENABLE_FORMATMENU_
    if ( ( state & FmDriverInfo::EDriveRemovable ) || ( state & FmDriverInfo::EDriveCorrupted )
             || ( state & FmDriverInfo::EDriveLocked ) ){
    #endif
                HbAction *formatAction = new HbAction();
                formatAction->setObjectName( "formatAction" );
                formatAction->setText( hbTrId( "txt_fmgr_menu_format" ) );
                mContextMenu->addAction( formatAction );
    
                connect( formatAction, SIGNAL( triggered() ),
                    this, SLOT( on_formatAction_triggered() ), Qt::QueuedConnection );
    #ifndef _DEBUG_ENABLE_FORMATMENU_
        }
    #endif
        
        if( state & FmDriverInfo::EDriveLocked ){
            HbAction *unLockedAction = new HbAction();
            unLockedAction->setObjectName( "unLockedAction" );
            unLockedAction->setText( hbTrId( "Unlock" ) );
            mContextMenu->addAction( unLockedAction );
    
            connect( unLockedAction, SIGNAL( triggered() ),
             this, SLOT( on_unLockedAction_triggered() ), Qt::QueuedConnection );
        }
    }
	if( mContextMenu->actions().count() > 0 ) {
		mContextMenu->setPreferredPos( coords );
		mContextMenu->open();
	} else {
		emit activated( diskName );
	}
}

void FmDriverListWidget::on_list_pressed( const QModelIndex &  index )
{
    mListLongPressed = false;
}

void FmDriverListWidget::on_viewAction_triggered()
{
    QString diskName = mModel->driveName( mCurrentItem->modelIndex() );
    mOperationService->asyncViewDriveDetails( diskName );
}

void FmDriverListWidget::on_renameAction_triggered()
{    
    QString diskName = mModel->driveName( mCurrentItem->modelIndex() );
    
    FmDriverInfo::DriveState state = FmUtils::queryDriverInfo( diskName ).driveState();
    FmDriverInfo driverInfo = FmUtils::queryDriverInfo( diskName );

    if ( state & FmDriverInfo::EDriveWriteProtected ){
        FmDlgUtils::information( hbTrId( "Unable to perform operation. Memory card is read only." ) );
        return;
        }

    QString title( hbTrId( "Drive name ") );
    //save the volume status, empty or set
    bool needToSetVolume = false;
    QString volumeName = FmUtils::getVolumeNameWithDefaultNameIfNull( diskName, needToSetVolume );    
    QString oldVolumeName( volumeName );
    QString associatedDrives( FmUtils::getDriveLetterFromPath( diskName ) );
    //use isReturnFalseWhenNoTextChanged = false in order that FmUtils::renameDrive( driveName, volumeName ) will
    //be excuted at lease once to set the volume name.
    while( FmDlgUtils::showTextQuery( title, volumeName, QStringList(), FmMaxLengthofDriveName, associatedDrives, false ) ){
        //if volume is not set or oldVolumeName != volumeName , FmUtils::renameDrive will be called
        if ( oldVolumeName == volumeName && !needToSetVolume ) {
            break;
        }
        int err = FmUtils::renameDrive( diskName, volumeName );
        if ( err == FmErrNone ) {
            FmDlgUtils::information( hbTrId( "The name has been changed!" ) );
            mModel->refresh();
            break;
        } else if( err == FmErrBadName ) {
            FmDlgUtils::information( hbTrId( "Illegal characters! Use only letters and numbers." ) );
        } else{
            FmDlgUtils::information( hbTrId( "Error occurred, operation cancelled!" ) );
            break;
        }                
    }
}

void FmDriverListWidget::on_nameAction_triggered()
{
    on_renameAction_triggered();
}

void FmDriverListWidget::on_setPwdAction_triggered()
{
    QString firstLabel( hbTrId( "New Password: ") );
    QString secondLabel( hbTrId( "Confirm new Password: ") );

    QString oldPwd;
    QString newPwd;

    FmUtils::emptyPwd( oldPwd );

    QString diskName = mModel->driveName( mCurrentItem->modelIndex() );

    QString associatedDrives( FmUtils::getDriveLetterFromPath( diskName ) );
    if( FmDlgUtils::showMultiPasswordQuery( firstLabel, secondLabel, newPwd, FmMaxLengthofDrivePassword, associatedDrives ) ) {
       if ( FmUtils::setDrivePwd( diskName, oldPwd, newPwd ) == 0 ){
            FmDlgUtils::information( hbTrId( "The password has been set!" ) );
        }
        else{
            FmDlgUtils::information( hbTrId( "Error occurred, operation cancelled!" ) );
        }
    }
}

void FmDriverListWidget::on_changePwdAction_triggered()
{
    QString title( hbTrId( "txt_common_dialog_password")  );
    QString firstLabel( hbTrId( "txt_common_dialog_new_password") );
    QString secondLabel( hbTrId( "Confirm new Password: ") );

    QString oldPwd;
    QString newPwd;

    QString diskName = mModel->driveName( mCurrentItem->modelIndex() );
    
    QString associatedDrives( FmUtils::getDriveLetterFromPath( diskName ) );
    while( FmDlgUtils::showSinglePasswordQuery( title, oldPwd, FmMaxLengthofDrivePassword, associatedDrives ) ) {
       if ( FmUtils::checkDrivePwd( diskName, oldPwd ) == 0 ){
            if( FmDlgUtils::showMultiPasswordQuery( firstLabel, secondLabel, newPwd ) ){
                if ( FmUtils::setDrivePwd( diskName, oldPwd, newPwd ) == 0 ){
                    FmDlgUtils::information( hbTrId( "The password has been changed!" ) );
                } else {
                    FmDlgUtils::information( hbTrId( "Error occurred, operation cancelled!" ) );
                }
                break;
            } else {  
                //cancel muti password query
                break;
            }
       } else {
            FmDlgUtils::information( hbTrId( "The password is incorrect, try again!" ) );
       }
         
    }
}

void FmDriverListWidget::on_removePwdAction_triggered()
{
    QString title( hbTrId( "Password: ")  );

    QString oldPwd;

    QString diskName = mModel->driveName( mCurrentItem->modelIndex() );
    if( FmDlgUtils::question( hbTrId( "Do you want to remove the password? Memory card becomes unlocked." ) ) ){
        QString associatedDrives( FmUtils::getDriveLetterFromPath( diskName ) );
        while( FmDlgUtils::showSinglePasswordQuery( title, oldPwd, FmMaxLengthofDrivePassword, associatedDrives ) ) {
            if ( FmUtils::checkDrivePwd( diskName, oldPwd ) == 0 ) {
                if ( FmUtils::removeDrivePwd( diskName, oldPwd ) == 0 ){
                    FmDlgUtils::information( hbTrId( "The password has been removed!" ) );
                }
                else{
                    FmDlgUtils::information( hbTrId( "Error occurred, operation cancelled!" ) );
                }
                break;
            }
            else {
                FmDlgUtils::information( hbTrId( "The password is incorrect, try again!" ) );
            }
            
        }
    }
}

void FmDriverListWidget::on_unLockedAction_triggered()
{
    QString title( hbTrId( "Password: ")  );

    QString oldPwd;

    QString diskName = mModel->driveName( mCurrentItem->modelIndex() );
    
    //Do not add associatedDrives as Locked MMC is not available Drive but only present Drive
    while( FmDlgUtils::showSinglePasswordQuery( title, oldPwd ) ) {
        int err = FmUtils::unlockDrive( diskName, oldPwd );
        if( err == FmErrNone ) {
            FmDlgUtils::information( hbTrId( "The memory is unlocked!" ) );
            break;
        } else if ( err == FmErrAccessDenied ) {
            FmDlgUtils::information( hbTrId( "The password is incorrect, try again!" ) );
        } else if (err == FmErrAlreadyExists ) {
            FmDlgUtils::information( hbTrId( "The disk has already been unlocked!" ) );
            break;
        } else if( err == FmErrNotSupported ) {
            FmDlgUtils::information( hbTrId( "The media does not support password locking!" ) );
            break;
        } else {
            FmDlgUtils::information( hbTrId( "Error occurred, operation cancelled!" ) );
            break;
        }
    }
}

void FmDriverListWidget::on_formatAction_triggered()
{
    QString diskName = mModel->driveName( mCurrentItem->modelIndex() );
    
    if( FmDlgUtils::question( hbTrId( "Format? Data will be deleted during formatting." ) ) ){
        if( FmErrNone != mOperationService->asyncFormat( diskName ) )
            FmDlgUtils::information( hbTrId( "Formatting failed." ) );
        }
}

void FmDriverListWidget::on_ejectAction_triggered()
{
    QString diskName = mModel->driveName( mCurrentItem->modelIndex() );
    
    if( FmDlgUtils::question( hbTrId( "Eject memory card? Some applications will be closed." ) ) ){
        FmUtils::ejectDrive( diskName );
    }
}

void FmDriverListWidget::on_directoryChanged( const QString &path )
{
    Q_UNUSED( path );
    mModel->refresh();
}

void FmDriverListWidget::activeSearchPanel()
{
    QStringList driveList;
    FmUtils::getDriveList( driveList, true );
    if(driveList.count() > 0 ) {
        mFindTargetPath =  driveList.first();
        if( FmUtils::isDriveC( mFindTargetPath ) ) {
            mFindTargetPath =  QString( Folder_C_Data );
        }
    } else {
        mFindTargetPath.clear();
    }
    mLayout->addItem( mSearchPanel );
    mSearchPanel->show();
}

void FmDriverListWidget::on_searchPanel_searchOptionsClicked()
{
    mFindTargetPath = FmUtils::fillPathWithSplash( FmFileDialog::getExistingDirectory( 0, hbTrId( "Look in:" ),
        QString(""), QStringList() ) );
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
