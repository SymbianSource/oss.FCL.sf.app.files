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
*     The source file of the file widget
*
*/
#include "fmfilewidget.h"
#include "fmutils.h"
#include "fmdrivemodel.h"
#include "fmdrivewatcher.h"
#include "fmcommon.h"

#include "hbstyle.h"
#include "hbabstractitemview.h"
#include <hblistview.h>

#include <QModelIndex>
#include <QGraphicsLinearLayout>
#include <QDirModel>
#include <QTime>
#include <QFileInfo>




FmFileWidget::FmFileWidget( HbWidget *parent ) :
    HbWidget( parent ), mCurrentModel( 0 )
{
    init();
}
FmFileWidget::~FmFileWidget()
{
    setModel( 0 );
    delete mDirModel;
    delete mDriveModel;
    
    mDriveWatcher->cancelWatch();
    delete mDriveWatcher;
    mDriveWatcher = 0;
}

QFileInfo FmFileWidget::currentPath() const
{
    QModelIndex index( mListView->rootIndex() );
    if( !index.isValid() ) {
        return QFileInfo();
    }
    if( mCurrentModel == mDirModel ) {
        return mDirModel->fileInfo( index );
    } else {
        return QFileInfo();
    }
}

void FmFileWidget::setRootPath( const QString &pathName )
{
    FmLogger::log( "FmFileWidget::setRootPath start" );
    if( pathName.isEmpty() || !FmUtils::isPathAccessabel( pathName ) ) {
        FmLogger::log( "FmFileWidget::setRootPath set drive model" );
        setModel( mDriveModel );
        FmLogger::log( "FmFileWidget::setRootPath set drive model end" );
        emit pathChanged( QString() );
    } else {
        FmLogger::log( "FmFileWidget::setRootPath set dir model end" );
        setModel( mDirModel );
        FmLogger::log( "FmFileWidget::setRootPath set dir model end" );
        mListView->setRootIndex( mDirModel->index( pathName ) );
        FmLogger::log( "FmFileWidget::setRootPath set rootIndex" );
        emit pathChanged( pathName );
    }    
    FmLogger::log( "FmFileWidget::setRootPath end" );
}
void FmFileWidget::on_list_activated( const QModelIndex &index )
    {
    mActivatedModelIndex = index;
    emit listActivated();
    }

void FmFileWidget::on_listActivated()
{
    FmLogger::log("FmFileWidget::on_list_activated start" );
    if( mCurrentModel == mDriveModel ) {
        QString driveName = mDriveModel->driveName( mActivatedModelIndex );
        QString checkedPath = FmUtils::checkDriveToFolderFilter( driveName );
        if( checkedPath.isEmpty() ) {
            FmLogger::log("FmFileWidget::on_list_activated end becaise checkedpath empty" );
            return;
        }

        FmLogger::log("FmFileWidget::on_list_activated setModel dir start" );
        setModel( mDirModel );
        FmLogger::log("FmFileWidget::on_list_activated setModel dir end" );
        mListView->setRootIndex( mDirModel->index( checkedPath ) );
        FmLogger::log("FmFileWidget::on_list_activated setRootIndex" );
        emit pathChanged( checkedPath );
        FmLogger::log("FmFileWidget::on_list_activated finish emit pathChanged" );
    }
    else if( mCurrentModel == mDirModel ) {
        if ( mDirModel->isDir( mActivatedModelIndex ) ) {
            FmLogger::log("FmFileWidget::on_list_activated start changeRootIndex" );
            changeRootIndex( mActivatedModelIndex );
            FmLogger::log("FmFileWidget::on_list_activated finish changeRootIndex" );
            FmLogger::log("FmFileWidget::on_list_activated finish emit fileActivated" );
        } else {
            QFileInfo fileInfo( mDirModel->filePath( mActivatedModelIndex ) );
            if( fileInfo.isFile() ) {
                emit fileActivated( fileInfo.fileName() );
                FmLogger::log("FmFileWidget::on_list_activated finish emit fileActivated" );
            }
        }
    } else {
        Q_ASSERT( false );
    }
    FmLogger::log("FmFileWidget::on_list_activated end" );
}

void FmFileWidget::setModelFilter( QDir::Filters filters )
{
    mDirModel->setFilter( filters );
}

void FmFileWidget::setNameFilters( const QStringList &nameFilters )
{
    mDirModel->setNameFilters( nameFilters );
}

void FmFileWidget::changeRootIndex( const QModelIndex &index )
{
    FmLogger::log("FmFileWidget::changeRootIndex start" );
    if( mCurrentModel != mDirModel ) {
        FmLogger::log("FmFileWidget::changeRootIndex end because model not equal dirmodel" );
        return;
    }

    mDirModel->fetchMore(index);
    mListView->setRootIndex( index );
    QFileInfo fileInfo = mDirModel->fileInfo( mListView->rootIndex() );
    QString string = fileInfo.absoluteFilePath();
    emit pathChanged( string );
    FmLogger::log("FmFileWidget::changeRootIndex end" );
}

void FmFileWidget::init()
{
    mLayout = new QGraphicsLinearLayout( this );
    mLayout->setOrientation( Qt::Vertical );
    setLayout( mLayout );

    mListView = new HbListView( this );
    mListView->setObjectName( "list" );
    mLayout->addItem( mListView );

    mDriveModel = new FmDriveModel( this, 
        FmDriveModel::FillWithVolume | FmDriveModel::FillWithDefaultVolume | FmDriveModel::HideUnAvailableDrive );
    qDebug("constructed dirveModel");
    qDebug( QTime::currentTime().toString().toUtf8().data() );
    mDirModel = new QDirModel( this );
    qDebug("constructed dirModel");
    qDebug( QTime::currentTime().toString().toUtf8().data() );
    setModel( mDriveModel );
    qDebug("setmodel");
    qDebug( QTime::currentTime().toString().toUtf8().data() );

    mDriveWatcher = new FmDriveWatcher( this );
    mDriveWatcher->setObjectName( "driveWatcher" );
    mDriveWatcher->startWatch();
    
//    QMetaObject::connectSlotsByName( this );
    connect( mListView, SIGNAL( activated( QModelIndex ) ),
        this, SLOT( on_list_activated( QModelIndex ) ) );
    connect( this, SIGNAL( listActivated() ),
        this, SLOT( on_listActivated() ), Qt::QueuedConnection );
        
    connect( mDriveWatcher, SIGNAL( driveAddedOrChanged() ),
            this, SLOT( on_driveWatcher_driveAddedOrChanged() ) );
    

}

void FmFileWidget::setModel( QAbstractItemModel *model )
{
    mListView->setModel( model );
    mCurrentModel = model;
}

FmFileWidget::ViewType FmFileWidget::currentViewType()
{
    ViewType viewType = DriveView;
    if( mCurrentModel == mDriveModel ) {
        viewType = DriveView;
    } else if( mCurrentModel == mDirModel ) {
        viewType = DirView;
    } else {
        Q_ASSERT( false );
    }
    return viewType;
}


bool FmFileWidget::cdUp()
{
    QString path( FmUtils::checkFolderToDriveFilter( currentPath().absoluteFilePath() ) );
    QFileInfo fileInfo( path );
    QString cdUpPath;
    if( path.length() > 3 ) {
        cdUpPath = fileInfo.dir().absolutePath();
    }
    setRootPath( FmUtils::fillPathWithSplash( cdUpPath ) );
    return true;
}

void FmFileWidget::on_driveWatcher_driveAddedOrChanged()
{
    FmLogger::log( QString( "FmFileDialog_FmFileWidget::on_driveWatcher_driveAddedOrChanged start" ) );
    mDriveModel->refresh();
    if( currentViewType() == DriveView ) {
        setModel( 0 );
        setModel( mDriveModel );
        emit pathChanged( QString() );
    } else if( currentViewType() == DirView ) {
        if( !FmUtils::isPathAccessabel( currentPath().absoluteFilePath() ) ) {
            // path not available, set model to drive
            FmLogger::log( QString( "FmFileDialog_FmFileWidget::on_driveWatcher_driveAddedOrChanged path not availeable, set drivemodel:"
                    + currentPath().absoluteFilePath() ) );
            setModel( mDriveModel );
            emit pathChanged( QString() );
        }
    }
    FmLogger::log( QString( "FmFileDialog_FmFileWidget::on_driveWatcher_driveAddedOrChanged end" ) );
}
