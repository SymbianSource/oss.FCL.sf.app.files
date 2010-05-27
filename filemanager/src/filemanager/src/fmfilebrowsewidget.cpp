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
*     Yong Zhang <yong.5.zhang@nokia.com>
* 
* Description:
*     The source file of the widget to browse files
*
*/

#include "fmfilebrowsewidget.h"
#include "fmviewdetailsdialog.h"
#include "fmviewmanager.h"
#include "fmfiledialog.h"
#include "fmdlgutils.h"

#include <QFile>
#include <QDirModel>
#include <QSizePolicy>
#include <QGraphicsLinearLayout>

#include <hbmenu.h>
#include <hblistview.h>
#include <hbtreeview.h>
#include <hbabstractviewitem.h>
#include <hbaction.h>
#include <hbsearchpanel.h>
#include <hbmessagebox.h>
#include <hblabel.h>

FmFileBrowseWidget::FmFileBrowseWidget( HbWidget *parent, FmFileBrowseWidget::Style style )
    : HbWidget( parent ),
      mTreeView( 0 ),
      mListView( 0 ),
      mLayout( 0 ),
      mModel( 0 ),
      mSelectable( false ), 
      mStyle( NoStyle ), 
      mFileBrowseStyle( style ),
      mCurrentItem( 0 ),
      mOperationService( 0 ),
      mSearchPanel( 0 ),
      mListLongPressed( false )
{
    initFileModel();
    initListView();
    initTreeView();
    initSearchPanel();
    initEmptyTipsLabel();
    initLayout();
    
    mOperationService = FmViewManager::viewManager()->operationService();
    setStyle( style );
}

FmFileBrowseWidget::~FmFileBrowseWidget()
{
    //take the model out from view and delete it.
    //so that the model could be destroy earlier
    //since there is a thread running in background
    //if the model destroy later, the thread might not quit properly.

    if( mStyle == ListStyle || mStyle == TreeStyle ) {
        QFileInfo oldFileInfo = mModel->fileInfo( mListView->rootIndex() );
        if( oldFileInfo.exists() ) {
            FmViewManager *viewManager = FmViewManager::viewManager();
            if( viewManager ) {
                viewManager->removeWatchPath( oldFileInfo.absoluteFilePath() );
            }   
        }
    }

    mTreeView->setModel( 0 );
    mListView->setModel( 0 );
    delete mModel;
}

QFileInfo FmFileBrowseWidget::currentPath() const
{
    QModelIndex index = mListView->rootIndex();
    if( !index.isValid() ) {
        return QFileInfo();
    }

    if (mStyle == ListStyle) {
        return mModel->fileInfo( mListView->rootIndex() );
    } else if ( mStyle == TreeStyle ) {
        return QFileInfo();
    } else {
        return QFileInfo();
    }
}

QList<QFileInfo> FmFileBrowseWidget::checkedItems() const
{
    QList<QFileInfo> infos;
    
    QItemSelection selection;
    if (mStyle == ListStyle) {
        selection = mListView->selectionModel()->selection();
    }

    QModelIndexList indexes = selection.indexes();
    for (int i = 0; i < indexes.size(); ++i) {
        infos.append( mModel->fileInfo( indexes.at(i) ) );
    }
 
    return infos;
}

void FmFileBrowseWidget::setRootPath( const QString &pathName )
{
    QString logString = "FmFileBrowseWidget::setRootPath(" + pathName + ')';
    FmLogger::log( logString );

    if( checkPathAndSetStyle( pathName ) ) {
        mListView->setModel(0);
        mTreeView->setModel(0);
        delete mModel;
        mModel = new QDirModel(this);
        mListView->setModel(mModel);
        mTreeView->setModel(mModel);
        
        mListView->setRootIndex( mModel->index( pathName ) );
        mTreeView->setRootIndex( mModel->index( pathName ) );
        FmViewManager::viewManager()->addWatchPath( pathName );
    }
    mCurrentDrive = pathName.left( 3 );
}

void FmFileBrowseWidget::setStyle( FmFileBrowseWidget::Style style )
{
    if ( mStyle == style ) {
        return;
    }

    if ( mStyle == ListStyle ) {
        mLayout->removeItem( mListView );
        mListView->hide();
    } else if ( mStyle == TreeStyle ) {
        mLayout->removeItem( mTreeView );
        mTreeView->hide();
    } else if( mStyle == LabelStyle ){
        mLayout->removeItem( mEmptyTipLabel );
        mEmptyTipLabel->hide();
    }

    if ( style == ListStyle ) {
        mLayout->addItem( mListView );
        mListView->show();
        mFileBrowseStyle = ListStyle;
    } else if ( style == TreeStyle ) {
        mLayout->addItem( mTreeView );
        mTreeView->show();
        mFileBrowseStyle = TreeStyle;
    } else if ( style == LabelStyle ){
        mLayout->addItem( mEmptyTipLabel );
        mEmptyTipLabel->show();
    }
    
    mStyle = style;
}

bool FmFileBrowseWidget::selectable() const
{
    return mSelectable;
}

void FmFileBrowseWidget::setSelectable( bool enable )
{
    if (mSelectable != enable) {
        mSelectable = enable;
        if (mStyle == ListStyle) {
            mListView->setSelectionMode( mSelectable ? HbAbstractItemView::MultiSelection 
                                                    : HbAbstractItemView::NoSelection );
        }
    }
}

void FmFileBrowseWidget::clearSelection()
{
    QItemSelectionModel *selectionModel = 0;

    if (mStyle == ListStyle) {
        selectionModel = mListView->selectionModel();
    } else if (mStyle == TreeStyle) {
        selectionModel = mTreeView->selectionModel();
    }

    if( selectionModel ){
        selectionModel->clear();
    }

}


bool FmFileBrowseWidget::rename( const QString &oldName, const QString &newName )
{
    return QFile::rename( oldName, newName );
    /*
    if (QFile::rename( oldName, newName )) {
    QModelIndex index = mModel->index( newName );
    mModel->refresh( index );
    index = mModel->index( oldName );
    mModel->refresh( index );
    }
    */
}




bool FmFileBrowseWidget::cdUp()
{
    if (mStyle == ListStyle) {
        QModelIndex index = mListView->rootIndex().parent();
        mModel->refresh(index);
        if (index.isValid()) {
            changeRootIndex( index );
            return true;
        }
    }

    return false;
}

void FmFileBrowseWidget::on_list_activated( const QModelIndex &index )
{
    mActivatedModelIndex = index;
    emit listActivated();
}

void FmFileBrowseWidget::on_listActivated()
{
    FmLogger::log("FmFileBrowseWidget::on_listActivated start");
    if( mListLongPressed ) {
        FmLogger::log("FmFileBrowseWidget::on_list_activated end because longPressed");
        return;
    }
    if (!mSelectable) {
        if (mModel->isDir(mActivatedModelIndex) ) {
            FmLogger::log("FmFileBrowseWidget::on_list_activated changeRootIndex>>");
            changeRootIndex( mActivatedModelIndex );
            FmLogger::log("FmFileBrowseWidget::on_list_activated changeRootIndex<<");
        } else {
            QString filePath( mModel->filePath( mActivatedModelIndex ) );
            QFileInfo fileInfo( filePath );
            if ( fileInfo.isFile() ) {
                mOperationService->syncLaunchFileOpen( filePath );
            }
        }
    }
    FmLogger::log("FmFileBrowseWidget::on_listActivated end");
}

void FmFileBrowseWidget::on_tree_activated( const QModelIndex &index )
{
    if (!mSelectable) {
        mTreeView->setExpanded( index, !mTreeView->isExpanded( index ) );
    }
}

void FmFileBrowseWidget::on_list_longPressed( HbAbstractViewItem *item, const QPointF &coords )
{
    mListLongPressed = true;
    HbMenu *contextMenu = new HbMenu();
    mCurrentItem = item;
    
    HbAction *viewAction = new HbAction();
    viewAction->setObjectName( "viewAction" );
    viewAction->setText( hbTrId( "txt_fmgr_menu_view_details_file" ) );
    contextMenu->addAction( viewAction );

    connect( viewAction, SIGNAL( triggered() ),
    this, SLOT( on_viewAction_triggered() ) );

    //copy
    HbAction *copyAction = new HbAction();
    copyAction->setObjectName( "copyAction" );
    copyAction->setText( hbTrId( "txt_fmgr_menu_copy" ) );
    contextMenu->addAction( copyAction );

    connect( copyAction, SIGNAL( triggered() ),
    this, SLOT( on_copyAction_triggered() ) );

    
    QString filePath( mModel->filePath( item->modelIndex() ) );
    QString formatFilePath( FmUtils::formatPath( filePath ) );
    QFileInfo fileInfo( filePath );
    
    if( ( fileInfo.isFile() ) || ( fileInfo.isDir() && !( FmUtils::isDefaultFolder( formatFilePath ) ) ) ){
        //Move
        HbAction *moveAction = new HbAction();
        moveAction->setObjectName( "moveAction" );
        moveAction->setText( hbTrId( "txt_fmgr_menu_move" ) );
        contextMenu->addAction( moveAction );
    
        connect( moveAction, SIGNAL( triggered() ),
        this, SLOT( on_moveAction_triggered() ) );
    
        //Delete
        HbAction *deleteAction = new HbAction();
        deleteAction->setObjectName( "deleteAction" );
        deleteAction->setText( hbTrId( "txt_fmgr_menu_delete" ) );
        contextMenu->addAction( deleteAction );
    
        connect( deleteAction, SIGNAL( triggered() ),
        this, SLOT( on_deleteAction_triggered() ) );
    
        //rename
        HbAction *renameAction = new HbAction();
        renameAction->setObjectName( "renameAction" );
        renameAction->setText( hbTrId( "txt_fmgr_menu_rename" ) );
        contextMenu->addAction( renameAction );
    
        connect( renameAction, SIGNAL( triggered() ),
        this, SLOT( on_renameAction_triggered() ) );
    }
    
//    if( fileInfo.isFile() ){
//        HbAction *sendAction = new HbAction();
//        sendAction->setObjectName( "sendAction" );
//        sendAction->setText( hbTrId( "txt_fmgr_menu_send" ) );
//        contextMenu->addAction( sendAction );
//        
//        connect( sendAction, SIGNAL( triggered() ),
//        this, SLOT( on_sendAction_triggered() ) );
//    }
    
    contextMenu->setPreferredPos( coords );
    contextMenu->open();
}

void FmFileBrowseWidget::on_list_pressed( const QModelIndex &  index )
{
    mListLongPressed = false;
}

void FmFileBrowseWidget::on_tree_longPressed( HbAbstractViewItem *item, const QPointF &coords )
{
    Q_UNUSED( item );
    Q_UNUSED( coords );
    /*
    HbMenu *contextMenu = new HbMenu( this );

    if ( isFolder( item->modelIndex() ) ) {
        contextMenu->addAction("View details");
        contextMenu->addAction("Copy");
        contextMenu->addAction("Rename");
        contextMenu->addAction("Delete");
    } else {
        contextMenu->addAction("View details");
        contextMenu->addAction("Open");
        contextMenu->addAction("Copy");
        contextMenu->addAction("Rename");
        contextMenu->addAction("Delete");
    }

    contextMenu->exec( coords );
    */
}

void FmFileBrowseWidget::initListView()
{
    mListView = new HbListView();
    mListView->setObjectName( "list" );
    mListView->setModel( mModel );

    connect( mListView, SIGNAL( activated( const QModelIndex& ) ),
        this, SLOT( on_list_activated( const QModelIndex& ) ) );
    connect( this, SIGNAL( listActivated() ),
        this, SLOT( on_listActivated() ), Qt::QueuedConnection );
    connect( mListView, SIGNAL( longPressed( HbAbstractViewItem *, const QPointF & ) ),
        this, SLOT( on_list_longPressed( HbAbstractViewItem *, const QPointF & ) ) );
}

void FmFileBrowseWidget::initTreeView()
{
    mTreeView = new HbTreeView();
    mTreeView->setObjectName( "tree" );
    mTreeView->setModel( mModel );

    connect( mTreeView, SIGNAL( activated( const QModelIndex& ) ),
        this, SLOT( on_tree_activated( const QModelIndex& ) ) );
    connect( mTreeView, SIGNAL( longPressed( HbAbstractViewItem *, const QPointF & ) ),
        this, SLOT( on_tree_longPressed( HbAbstractViewItem *, const QPointF & ) ) );
}

void FmFileBrowseWidget::initFileModel()
{
    mModel = new QDirModel( this );
    mModel->setReadOnly( false );
    disconnect( mModel, SIGNAL( rowsInserted( const QModelIndex &, int, int ) ), 0 ,0 );
}

void FmFileBrowseWidget::initLayout()
{
    mLayout = new QGraphicsLinearLayout( this );
    mLayout->setOrientation( Qt::Vertical );
    
    setLayout( mLayout );
}

void FmFileBrowseWidget::initSearchPanel()
{
    mSearchPanel = new HbSearchPanel( this );
    mSearchPanel->setObjectName( "searchPanel" );
    mSearchPanel->setSearchOptionsEnabled( true );
    mSearchPanel->setProgressive( false );
    mSearchPanel->hide();
    
    connect( mSearchPanel, SIGNAL( searchOptionsClicked() ),
        this, SLOT( on_searchPanel_searchOptionsClicked() ) );
    
    connect( mSearchPanel, SIGNAL( criteriaChanged( const QString & ) ),
        this, SLOT( on_searchPanel_criteriaChanged( const QString & ) ) );
    
    connect( mSearchPanel, SIGNAL( exitClicked() ),
        this, SLOT( on_searchPanel_exitClicked() ) );
}

void FmFileBrowseWidget::initEmptyTipsLabel()
{
    mEmptyTipLabel = new HbLabel( this );
    mEmptyTipLabel->setObjectName( "searchPanel" );
    mEmptyTipLabel->hide();
}


void FmFileBrowseWidget::changeRootIndex( const QModelIndex &index )
{
    QFileInfo oldFileInfo = mModel->fileInfo( mListView->rootIndex() );
    FmViewManager::viewManager()->removeWatchPath( oldFileInfo.absoluteFilePath() );

    mModel->refresh(index);
    if ( mStyle == ListStyle ) {
        mListView->setRootIndex( index );
    } else if ( mStyle == TreeStyle ) {
        mTreeView->setRootIndex( index );
    }
    QFileInfo fileInfo = mModel->fileInfo( mListView->rootIndex() );
    QString string = fileInfo.absoluteFilePath();
    emit currentPathChanged( string );
    FmViewManager::viewManager()->addWatchPath( string );
}

bool FmFileBrowseWidget::isDriver(const QModelIndex &index) const
{
    QString path = mModel->filePath(index);
    return path.right(1) == ":";
}

bool FmFileBrowseWidget::isFolder(const QModelIndex &index) const
{
    if( isDriver( index ) )    {
        return false;
    }

    return mModel->isDir(index);
}

void FmFileBrowseWidget::setModelFilter( QDir::Filters filters ) 
{
    mModel->setFilter( filters );
}

void FmFileBrowseWidget::refreshModel( const QString& path )
{
    FmLogger::log( "FmFileBrowseWidget::refreshModel start" );
    QString currPath( currentPath().absoluteFilePath() );
    QString refreshPath( path );
    
    if( !currPath.isEmpty() ) {
        if( refreshPath.isEmpty() ) {
            refreshPath = currPath;
        }
        if(  !FmUtils::isPathEqual( refreshPath, currPath ) ) {
            // no need refresh other path
            return;
        }
        if( checkPathAndSetStyle( refreshPath ) ) {               
            mModel->refresh( mModel->index( refreshPath ) );
        } else {
            FmViewManager *viewManager = FmViewManager::viewManager();
            if( viewManager ) {
                viewManager->removeWatchPath( currentPath().absoluteFilePath() );
            } 
        }
    } else {
        // current path is empty, so change root path to Drive root.
        refreshPath = mCurrentDrive;
        setRootPath( refreshPath );
        emit setTitle( FmUtils::fillDriveVolume( mCurrentDrive, true ) );
    }
    FmLogger::log( "FmFileBrowseWidget::refreshModel end" );
}

bool FmFileBrowseWidget::checkPathAndSetStyle( const QString& path )
{
    if( !FmUtils::isPathAccessabel( path ) ){
        QString driveName = FmUtils::getDriveNameFromPath( path );
        FmDriverInfo::DriveState state = FmUtils::queryDriverInfo( driveName ).driveState();
        
        if( state & FmDriverInfo::EDriveLocked ) {
            mEmptyTipLabel->setPlainText( hbTrId( "Drive is locked" ) );       
        } else if( state & FmDriverInfo::EDriveNotPresent ) {
            mEmptyTipLabel->setPlainText( hbTrId( "Drive is not present" ) );
        } else if( state & FmDriverInfo::EDriveCorrupted ) {
            mEmptyTipLabel->setPlainText( hbTrId( "Drive is Corrupted" ) );
        } else {
            mEmptyTipLabel->setPlainText( hbTrId( "Drive can not be opened " ) );
        }
        setStyle( LabelStyle );
        emit setEmptyMenu( true );
        return false;
    } else {
        setStyle( mFileBrowseStyle );
        emit setEmptyMenu( false );
        return true;
    }
    
}

void FmFileBrowseWidget::sortFiles( TSortType sortType )
{
    switch( sortType ){
        case ESortByName:{
            mModel->setSorting( QDir::Name );
        }
            break;
        case ESortByTime:{
            mModel->setSorting( QDir::Time );
        }
            break;
        case ESortBySize:{
            mModel->setSorting( QDir::Size );
        }
            break;
        case ESortByType:{
            mModel->setSorting( QDir::Type | QDir::DirsFirst );
        }
            break;
        default:
            break;
    }
}

void FmFileBrowseWidget::activeSearchPanel()
{
    mFindTargetPath.clear();
    mLayout->addItem( mSearchPanel );
    mSearchPanel->show();
}

void FmFileBrowseWidget::on_searchPanel_searchOptionsClicked()
{
    mFindTargetPath = FmFileDialog::getExistingDirectory( 0, hbTrId( "Look in:" ), QString(""),
        QStringList() );
}

void FmFileBrowseWidget::on_searchPanel_criteriaChanged( const QString &criteria )
{
    if( mFindTargetPath.isEmpty() ){
        mFindTargetPath = currentPath().filePath();
    }
    emit startSearch( mFindTargetPath, criteria );
    
    mSearchPanel->hide();
    mLayout->removeItem( mSearchPanel );
}

void FmFileBrowseWidget::on_searchPanel_exitClicked()
{
    mSearchPanel->hide();
    mLayout->removeItem( mSearchPanel );
}

void FmFileBrowseWidget::on_sendAction_triggered()
{
    QString filePath = mModel->filePath( mCurrentItem->modelIndex() );
    QStringList list;
    list.append( filePath );
    FmUtils::sendFiles( list );
}

void FmFileBrowseWidget::on_viewAction_triggered()
{
    QString filePath = mModel->filePath( mCurrentItem->modelIndex() );
    QFileInfo fileInfo = mModel->fileInfo( mCurrentItem->modelIndex() );
    
    if( fileInfo.isDir() ){
        mOperationService->asyncViewFolderDetails( filePath );  
    }
    else if( fileInfo.isFile() ){
		FmViewDetailsDialog::showFileViewDetailsDialog( filePath, FmUtils::getDriveLetterFromPath( filePath ) );
    } 
}

void FmFileBrowseWidget::on_deleteAction_triggered()
{
    QStringList fileList;
    fileList.push_back( mModel->filePath( mCurrentItem->modelIndex() ) );
    if ( FmDlgUtils::question( hbTrId("Confirm Deletion?" ) )) {
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
    }
}

void FmFileBrowseWidget::on_copyAction_triggered()
{
    QStringList srcFileList;
    srcFileList.push_back( mModel->filePath( mCurrentItem->modelIndex() ) );

    QString targetPathName = FmFileDialog::getExistingDirectory( 0, hbTrId( "copy to" ),
    QString(""), QStringList() );
    if( !targetPathName.isEmpty() ) {
        targetPathName = FmUtils::fillPathWithSplash( targetPathName );

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
    }

}
 
void FmFileBrowseWidget::on_moveAction_triggered()
{
    QStringList fileList;
    fileList.push_back( mModel->filePath( mCurrentItem->modelIndex() ) );

    QString targetPathName = FmFileDialog::getExistingDirectory( 0, tr( "move to" ),
            QString(""), QStringList() );

    if( !targetPathName.isEmpty() ) {
        targetPathName = FmUtils::fillPathWithSplash( targetPathName );

        int ret = mOperationService->asyncMove( fileList, targetPathName );
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
    }
}


void FmFileBrowseWidget::on_renameAction_triggered()
{
    QString filePath = mModel->filePath( mCurrentItem->modelIndex() );
    QFileInfo fileInfo = mModel->fileInfo( mCurrentItem->modelIndex() );
    int maxFileNameLength = FmUtils::getMaxFileNameLength();
    
    QString newName( fileInfo.fileName() );
    while( FmDlgUtils::showTextQuery( hbTrId( "Enter new name for %1" ).arg( newName ), newName, true,
            maxFileNameLength, QString() , false ) ){
        QString newTargetPath = FmUtils::fillPathWithSplash(
            fileInfo.absolutePath() ) + newName;
        QFileInfo newFileInfo( newTargetPath );
        if( newFileInfo.exists() ) {
            HbMessageBox::information( hbTrId( "%1 already exist!" ).arg( newName ) );
            continue;
        }
        if( !FmUtils::checkFolderFileName( newName ) ) {
            HbMessageBox::information( hbTrId( "Invalid file or folder name, try again!" ) );
            continue;
        }
        if( !FmUtils::checkMaxPathLength( newTargetPath ) ) {
            HbMessageBox::information( hbTrId( "the path you specified is too long, try again!" ) );
            continue;
        }
        if( !rename( fileInfo.absoluteFilePath(), newTargetPath ) ) {
            HbMessageBox::information( hbTrId("Rename failed!") );
        }
        break;
    }   
}
