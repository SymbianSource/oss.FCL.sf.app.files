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
* Description:
*     The source file of filemanager view manager
*
*/

#include "fmviewmanager.h"
#include "fmmainwindow.h"
#include "fmdriverview.h"
#include "fmfileview.h"
#include "fmfindview.h"
#include "fmbackupview.h"
#include "fmrestoreview.h"
#include "fmdeletebackupview.h"
#include "fmsplitview.h"
#include "fmoperationservice.h"
#include "fmoperationbase.h"
#include "fmdrivewatcher.h"

#include <hbview.h>
#include <hbmessagebox.h>
#include <QFileSystemWatcher>
#include <QFileInfo>

FmViewManager *FmViewManager::mViewManager = 0;

FmViewManager::FmViewManager( FmMainWindow* mainWindow )
{
    mMainWindow = mainWindow;
    mOperationService = new FmOperationService( this );
    mOperationService->setObjectName( "operationService" );

    mFsWatcher = new QFileSystemWatcher( this );
    mFsWatcher->setObjectName( "fsWatcher" );

    mDriveWatcher = new FmDriveWatcher( this );
    mDriveWatcher->setObjectName( "driveWatcher" );
    mDriveWatcher->startWatch();

    QMetaObject::connectSlotsByName( this );
}

FmViewManager::~FmViewManager(void)
{
    
    FmViewBase *view = static_cast<FmViewBase *>( mMainWindow->currentView() );
    while( view ) {
        mMainWindow->removeView( view );
        delete view;
        view = static_cast<FmViewBase *>( mMainWindow->currentView() );
    }

    delete mOperationService;
    mOperationService = 0;
    delete mFsWatcher;
    mFsWatcher = 0;

    mDriveWatcher->cancelWatch();
    delete mDriveWatcher;
    mDriveWatcher = 0;
}

FmViewManager *FmViewManager::CreateViewManager( FmMainWindow* mainWindow )
{
    if( !mViewManager ){
        mViewManager = new FmViewManager( mainWindow );
    }

    return mViewManager;
}
void FmViewManager::RemoveViewManager()
{
    delete mViewManager;
    mViewManager = 0;
}

FmViewManager *FmViewManager::viewManager()
{
    return mViewManager;
}

FmOperationService *FmViewManager::operationService()
{
    return mOperationService;
}

void FmViewManager::popViewAndShow()
{
    FmViewBase *view = static_cast<FmViewBase *>( mMainWindow->currentView() );
    view->setNavigationAction( 0 );

    mMainWindow->removeView( view );   
    delete view;

    if( viewCount() < 1 )
    {
        mMainWindow->close();
    }
}

void FmViewManager::on_operationService_refreshModel( FmOperationBase *operationBase, const QString &path )
{
    Q_UNUSED( operationBase );
    emit refreshModel( path );
}


int FmViewManager::viewCount()
{
    return mMainWindow->viewCount();
}


void FmViewManager::createDriverView()
{
    FmDriverView *driverView= new FmDriverView();

    mMainWindow->addView( driverView );
    mMainWindow->setCurrentView( driverView );

    connect( this, SIGNAL( refreshModel( QString ) ), 
        driverView, SLOT( refreshModel( QString ) ) );
}

void FmViewManager::createFileView( const QString &path,
                                    bool enableBackPathCheck, bool disableFind )
{
    QFileInfo fileInfo( path );
    QString absolutePath = fileInfo.absoluteFilePath();

    QString checkedPath = FmUtils::checkDriveToFolderFilter( absolutePath );
    
    if( !FmUtils::isPathAccessabel( checkedPath ) ) {
        checkedPath.clear();
    }
    if( checkedPath.isEmpty() ) {
        HbMessageBox::information( QString( tr("Path: %1 is unavailable!").arg( path )) );
        return;
    }

    FmFileView *fileView= new FmFileView();
    fileView->setRootPath( checkedPath );
    if( enableBackPathCheck ) {
        fileView->setRootLevelPath( checkedPath );
    }

    fileView->setFindDisabled( disableFind );

    mMainWindow->addView( fileView );
    mMainWindow->setCurrentView( fileView );
    
    connect( this, SIGNAL( refreshModel( const QString& ) ), //emit when need refresh models
        fileView, SLOT( refreshModel( const QString& ) ) );

    connect( fileView, SIGNAL( popViewAndShow() ),                  //emit when fileView need delete itself and pop&show view from stack.
        this, SLOT( popViewAndShow() ), Qt::QueuedConnection );
}

void FmViewManager::createFindView( const QString &keyword, const QString &path )
{
    FmFindView *findView= new FmFindView();
    
    mMainWindow->addView( findView );
    mMainWindow->setCurrentView( findView );

    findView->find( keyword, path );
 }

void FmViewManager::createSplitView()
{
    FmSplitView *splitView = new FmSplitView();
    mMainWindow->addView( splitView );
    mMainWindow->setCurrentView( splitView );

}

void FmViewManager::createBackupView()
{
    FmBackupView *backupView= new FmBackupView();

    mMainWindow->addView( backupView );
    mMainWindow->setCurrentView( backupView );

}

void FmViewManager::createRestoreView()
{
    FmRestoreView *restoreView= new FmRestoreView();

    mMainWindow->addView( restoreView );
    mMainWindow->setCurrentView( restoreView );

}

void FmViewManager::createDeleteBackupView()
{
   FmDeleteBackupView *deleteBackupView= new FmDeleteBackupView();

    mMainWindow->addView( deleteBackupView );
    mMainWindow->setCurrentView( deleteBackupView );

}

Qt::Orientation FmViewManager::orientation(){
    return mMainWindow->orientation();
}

void FmViewManager::on_fsWatcher_fileChanged(const QString &path)
{
    emit refreshModel( path );
}
void FmViewManager::on_fsWatcher_directoryChanged(const QString &path)
{
    emit refreshModel( path );
}
void FmViewManager::addWatchPath( const QString &path )
{
    mFsWatcher->addPath( path );
}
void FmViewManager::removeWatchPath( const QString &path )
{
    if( !mViewManager || !mFsWatcher ) {
        return;
    }
    mFsWatcher->removePath( path );
}


void FmViewManager::on_driveWatcher_driveAddedOrChanged()
{
    FmLogger::log( QString( "FmViewManager::on_driveWatcher_driveAddedOrChanged start" ) );
    emit refreshModel( QString("") );
    FmLogger::log( QString( "FmViewManager::on_driveWatcher_driveAddedOrChanged end" ) );

}

