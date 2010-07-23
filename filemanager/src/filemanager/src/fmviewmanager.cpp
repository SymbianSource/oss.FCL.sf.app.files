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
#include "fmdialog.h"
#include "fmdlgutils.h"

#include <hbview.h>
#include <QFileSystemWatcher>
#include <QFileInfo>


FmViewManager *FmViewManager::mViewManager = 0;


FmDlgCloseUnit::FmDlgCloseUnit( FmDialog *dialog ) : mDialog( dialog )
{
}
FmDlgCloseUnit::~FmDlgCloseUnit()
{
}

void FmDlgCloseUnit::addAssociatedDrives( QString drives )
{
	for( int i = 0; i< drives.length(); ++i ) {
		if( !mAssociatedDrives.contains( drives[i], Qt::CaseInsensitive ) ) {
			mAssociatedDrives += drives[i];
		}
	}
}
void FmDlgCloseUnit::removeAssociatedDrives( QString drives )
{
	for( int i = 0; i < drives.length(); ++i ){
		mAssociatedDrives.remove( drives[i], Qt::CaseInsensitive );
	}
}
QString FmDlgCloseUnit::associatedDrives()
{
	return mAssociatedDrives;
}

FmDialog *FmDlgCloseUnit::dialog()
{
	return mDialog;
}

FmViewManager::FmViewManager( FmMainWindow* mainWindow )
{
    mMainWindow = mainWindow;
    mOperationService = new FmOperationService( this );
    mOperationService->setObjectName( "operationService" );

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

void FmViewManager::on_operationService_driveSpaceChanged( FmOperationBase *operationBase )
{
    Q_UNUSED( operationBase );
    // request drive view to refresh
    emit driveSpaceChanged();
}

void FmViewManager::on_operationService_notifyFinish( FmOperationBase *operationBase )
{
    if( !operationBase ){
        Q_ASSERT_X( false, "FmViewManager",
                "NULL is not accepted in on_operationService_notifyFinish()" );
        return;
    }
    
    if( operationBase->operationType() == FmOperationService::EOperationTypeBackup )
        {
        // after finish backup, we need refresh backup date in backup view.
        emit refreshBackupDate();
        }
}


int FmViewManager::viewCount()
{
    return mMainWindow->views().count();
}


void FmViewManager::createDriverView()
{
    FmDriverView *driverView= new FmDriverView();

    mMainWindow->addView( driverView );
    mMainWindow->setCurrentView( driverView );

    connect( this, SIGNAL( driveSpaceChanged() ), 
        driverView, SLOT( refreshDrive() ) );
    
    connect( this, SIGNAL( driveChanged() ), 
        driverView, SLOT( refreshDrive() ) );
}

void FmViewManager::createFileView( const QString &path,
                                    bool enableBackPathCheck, bool disableFind )
{
    QFileInfo fileInfo( path );
    QString absolutePath = fileInfo.absoluteFilePath();

    QString checkedPath = FmUtils::checkDriveToFolderFilter( absolutePath );
    if( FmUtils::isDriveAvailable( path ) ) {
        if( FmErrNone != FmUtils::isPathAccessabel( checkedPath )  ) {
            checkedPath.clear();
        }
    }
   
    if( checkedPath.isEmpty() ) {
        FmDlgUtils::information( QString( hbTrId("Path: %1 is unavailable!").arg( path )) );
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
    
    connect( this, SIGNAL( driveChanged() ), //emit when drive changed.
        fileView, SLOT( on_driveChanged() ) );

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
    connect( this, SIGNAL( driveChanged() ), //emit when need refresh model
            backupView, SLOT( refreshModel() ) );
    connect( this, SIGNAL( refreshBackupDate() ),  //emit when need refresh backup date
            backupView, SLOT( refreshBackupDate() ) );
}

void FmViewManager::createRestoreView()
{
    FmRestoreView *restoreView= new FmRestoreView();

    mMainWindow->addView( restoreView );
    mMainWindow->setCurrentView( restoreView );
    connect( this, SIGNAL( driveChanged() ), restoreView, SLOT( refreshRestoreView() ) );

}

void FmViewManager::createDeleteBackupView()
{
   FmDeleteBackupView *deleteBackupView= new FmDeleteBackupView();

    mMainWindow->addView( deleteBackupView );
    mMainWindow->setCurrentView( deleteBackupView );
    connect( this, SIGNAL( driveChanged() ), deleteBackupView, SLOT( refreshDeleteBackupView() ) );

}

Qt::Orientation FmViewManager::orientation(){
    return mMainWindow->orientation();
}

void FmViewManager::on_driveWatcher_driveAddedOrChanged()
{
    FM_LOG( QString( "FmViewManager::on_driveWatcher_driveAddedOrChanged start" ) );
    emit driveChanged();
	checkDlgCloseUnit();
    FM_LOG( QString( "FmViewManager::on_driveWatcher_driveAddedOrChanged end" ) );

}


void FmViewManager::checkDlgCloseUnit()
{
	foreach( FmDlgCloseUnit* unit, mDlgCloseUnitList ) {
		QString drives( unit->associatedDrives() );
		for( int i = 0; i < drives.length(); i++ ) {
			QString drive( drives[i] + QString( ":/" ) );
			if( !FmUtils::isDriveAvailable( drive ) ) {
				FM_LOG( " close Dialog start " );
				unit->dialog()->close();
				FM_LOG( " close Dialog end " );
			}
		}
	}
}
