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

#include <hbstyle.h>
#include <hbabstractitemview.h>
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
    if( pathName.isEmpty() || !FmUtils::isPathAccessabel( pathName ) ) {
        setModel( mDriveModel );
        emit pathChanged( QString() );
    } else {
        setModel( mDirModel );
        mListView->setRootIndex( mDirModel->index( pathName ) );
        emit pathChanged( pathName );
    }    
}


void FmFileWidget::on_list_activated( const QModelIndex &index )
{
    if( mCurrentModel == mDriveModel ) {
        QString driveName = mDriveModel->driveName( index );
        QString checkedPath = FmUtils::checkDriveToFolderFilter( driveName );
        if( checkedPath.isEmpty() ) {
            return;
        }

        setModel( mDirModel );
        mListView->setRootIndex( mDirModel->index( checkedPath ) );
        emit pathChanged( checkedPath );
    }
    else if( mCurrentModel == mDirModel ) {
        if ( mDirModel->isDir(index) ) {
            changeRootIndex( index );
        } else {
            QFileInfo fileInfo( mDirModel->filePath( index ) );
            if( fileInfo.isFile() ) {
                emit fileActivated( fileInfo.fileName() );
            }
        }
    } else {
        Q_ASSERT( false );
    }
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
    if( mCurrentModel != mDirModel ) {
        return;
    }

    mDirModel->fetchMore(index);
    mListView->setRootIndex( index );
    QFileInfo fileInfo = mDirModel->fileInfo( mListView->rootIndex() );
    QString string = fileInfo.absoluteFilePath();
    emit pathChanged( string );
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

//    QMetaObject::connectSlotsByName( this );
    connect( mListView, SIGNAL( activated( QModelIndex ) ),
        this, SLOT( on_list_activated( QModelIndex ) ) );
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
