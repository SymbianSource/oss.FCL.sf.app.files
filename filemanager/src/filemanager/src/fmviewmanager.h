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
*     The header file of filemanager view manager
*
*/

#ifndef FMVIEWMANAGER_H
#define FMVIEWMANAGER_H

#include <QObject>

class FmMainWindow;
class HbView;
class FmViewManager;
class FmOperationService;
class FmOperationBase;
class QFileSystemWatcher;
class FmDriveWatcher;

class FmViewManager : public QObject
{
    Q_OBJECT

public:
    int viewCount();
    static FmViewManager *CreateViewManager( FmMainWindow* mainWindow );
    static void RemoveViewManager();
    static FmViewManager *viewManager();
    FmOperationService *operationService();
    
    Qt::Orientation orientation(); 

    void createFileView( const QString &path,
        bool enableBackPathCheck = true, bool disableFind = false );
    void createFindView( const QString &keyword, const QString &path );
    void createDriverView();
    void createSplitView();
	void createBackupView();
	void createRestoreView();
    void createDeleteBackupView();

    void addWatchPath( const QString &path );
    void removeWatchPath( const QString &path );

protected:
    explicit FmViewManager( FmMainWindow* mainWindow );
    ~FmViewManager();
    
public slots:
   /// popViewAndShow will delete current view and pop view from stack, then show it.
    void popViewAndShow();
    void on_operationService_refreshModel( FmOperationBase* operationBase, const QString &path );
    void on_fsWatcher_fileChanged(const QString &path);
    void on_fsWatcher_directoryChanged(const QString &path);
    void on_driveWatcher_driveAddedOrChanged();
signals:
    void refreshModel( const QString &path  );

private:
       /// create view will push current view to stack
  //  HbView *createView( FmViewType viewType );

  //  void adjustSecondarySoftKey();
    
private:
     static FmViewManager *mViewManager;

private:
    FmMainWindow *mMainWindow;
    FmOperationService  *mOperationService;
    QFileSystemWatcher  *mFsWatcher;
    FmDriveWatcher  *mDriveWatcher;
};





#endif


