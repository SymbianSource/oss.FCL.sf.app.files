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

FmFileBrowseWidget::FmFileBrowseWidget( HbWidget *parent, FmFileBrowseWidget::Style style )
    : HbWidget( parent ),
      mTreeView( 0 ),
      mListView( 0 ),
      mLayout( 0 ),
      mModel( 0 ),
      mSelectable( false ), 
      mStyle( NoStyle ), 
      mCurrentItem( 0 ),
      mOperationService( 0 ),
      mSearchPanel( 0 )

{
    initFileModel();
    initListView();
    initTreeView();
    initSearchPanel();
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

    QFileInfo oldFileInfo = mModel->fileInfo( mListView->rootIndex() );
    FmViewManager *viewManager = FmViewManager::viewManager();
    if( viewManager ) {
        viewManager->removeWatchPath( oldFileInfo.absoluteFilePath() );
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

    QModelIndex modelIndex = mModel->index( pathName );
    bool i = QFileInfo(pathName).exists();
    QString string2; 
    if( i )
        {
        string2 = QString( "true" ) ;
        }
    else
        {
        string2 = QString( "false" ) ;
        }

    logString = "FmFileBrowseWidget::setRootPath exists(" + string2 + ')';
    FmLogger::log( logString );

    mModel->refresh(modelIndex);
    QString string = mModel->fileName( modelIndex );
    logString = "FmFileBrowseWidget::setRootPath:filename(" + string + ')';
    FmLogger::log( logString );

    string  = mModel->filePath( modelIndex );
    logString = "FmFileBrowseWidget::setRootPath:filepath(" + string + ')';
    FmLogger::log( logString );


    FmViewManager::viewManager()->addWatchPath( pathName );

    mListView->setRootIndex( mModel->index( pathName ) );
    mTreeView->setRootIndex( mModel->index( pathName ) );
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
    } 

    if ( style == ListStyle ) {
        mLayout->addItem( mListView );
        mListView->show();
    } else if ( style == TreeStyle ) {
        mLayout->addItem( mTreeView );
        mTreeView->show();
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

    selectionModel->clear();
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
    if (!mSelectable) {
        if (mModel->isDir(index) ) {
            changeRootIndex( index );
        } else {
            QString filePath( mModel->filePath( index ) );
            QFileInfo fileInfo( filePath );
            if ( fileInfo.isFile() ) {
                mOperationService->syncLaunchFileOpen( filePath );
            }
        }
    }
}

void FmFileBrowseWidget::on_tree_activated( const QModelIndex &index )
{
    if (!mSelectable) {
        mTreeView->setExpanded( index, !mTreeView->isExpanded( index ) );
    }
}

void FmFileBrowseWidget::on_list_longPressed( HbAbstractViewItem *item, const QPointF &coords )
{
    HbMenu *contextMenu = new HbMenu();
    mCurrentItem = item;
    
    HbAction *viewAction = new HbAction();
    viewAction->setObjectName( "viewAction" );
    viewAction->setText( tr( "View details" ) );
    contextMenu->addAction( viewAction );

    connect( viewAction, SIGNAL( triggered() ),
    this, SLOT( on_viewAction_triggered() ) );

    //copy
    HbAction *copyAction = new HbAction();
    copyAction->setObjectName( "copyAction" );
    copyAction->setText( tr( "Copy" ) );
    contextMenu->addAction( copyAction );

    connect( copyAction, SIGNAL( triggered() ),
    this, SLOT( on_copyAction_triggered() ) );

    //Move
    HbAction *moveAction = new HbAction();
    moveAction->setObjectName( "moveAction" );
    moveAction->setText( tr( "Move" ) );
    contextMenu->addAction( moveAction );

    connect( moveAction, SIGNAL( triggered() ),
    this, SLOT( on_moveAction_triggered() ) );

    //Delete
    HbAction *deleteAction = new HbAction();
    deleteAction->setObjectName( "deleteAction" );
    deleteAction->setText( tr( "Delete" ) );
    contextMenu->addAction( deleteAction );

    connect( deleteAction, SIGNAL( triggered() ),
    this, SLOT( on_deleteAction_triggered() ) );

    //rename
    HbAction *renameAction = new HbAction();
    renameAction->setObjectName( "renameAction" );
    renameAction->setText( tr( "Rename" ) );
    contextMenu->addAction( renameAction );

    connect( renameAction, SIGNAL( triggered() ),
    this, SLOT( on_renameAction_triggered() ) );
    
//remove send action as it can not be used
//    HbAction *sendAction = new HbAction();
//    sendAction->setObjectName( "sendAction" );
//    sendAction->setText( tr( "Send" ) );
//    contextMenu->addAction( sendAction );
//    
//    connect( sendAction, SIGNAL( triggered() ),
//    this, SLOT( on_sendAction_triggered() ) );
    
    contextMenu->exec( coords );     
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
    if( !path.isEmpty() ) {
        mModel->refresh( mModel->index( path ) );
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
            mModel->setSorting( QDir::Type );
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
    mFindTargetPath = FmFileDialog::getExistingDirectory( 0, tr( "Look in:" ), QString(""),
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
    QList<QVariant> list;
    list.append( QVariant(filePath ) );
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
        FmViewDetailsDialog::showFileViewDetailsDialog( filePath );
    } 
}

void FmFileBrowseWidget::on_deleteAction_triggered()
{
    QStringList fileList;
    fileList.push_back( mModel->filePath( mCurrentItem->modelIndex() ) );
    if ( HbMessageBox::question( tr("Confirm Deletion?" ) )) {
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
    }
}

void FmFileBrowseWidget::on_copyAction_triggered()
{
    QStringList srcFileList;
    srcFileList.push_back( mModel->filePath( mCurrentItem->modelIndex() ) );

    QString targetPathName = FmFileDialog::getExistingDirectory( 0, tr( "copy to" ),
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
                HbMessageBox::information( tr( "Operatin already started!" ) );
                break;
            case FmErrWrongParam:
                HbMessageBox::information( tr( "Wrong parameters!" ) );
                break;
            default:
                HbMessageBox::information( tr( "Operation fail to start!" ) );
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
                HbMessageBox::information( tr( "Operatin already started!" ) );
                break;
            case FmErrWrongParam:
                HbMessageBox::information( tr( "Wrong parameters!" ) );
                break;
            default:
                HbMessageBox::information( tr( "Operation fail to start!" ) );
        }
    }
}


void FmFileBrowseWidget::on_renameAction_triggered()
{
    QString filePath = mModel->filePath( mCurrentItem->modelIndex() );
    QFileInfo fileInfo = mModel->fileInfo( mCurrentItem->modelIndex() );

    QString newName( fileInfo.fileName() );
    
    while( FmDlgUtils::showTextQuery( tr( "Enter new name for %1" ).arg( newName ), newName ) ){
        QString newTargetPath = FmUtils::fillPathWithSplash(
            fileInfo.absolutePath() ) + newName;
        QFileInfo newFileInfo( newTargetPath );
        if( newFileInfo.exists() ) {
            HbMessageBox::information( tr( "%1 already exist!" ).arg( newName ) );
            continue;
        }

        if( !rename( fileInfo.absoluteFilePath(), newTargetPath ) ) {
            HbMessageBox::information( tr("Rename failed!") );
        }
        break;
    }
    
   
}
