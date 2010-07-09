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
#include "fmfileiconprovider.h"

#include <QFile>
#include <QFileSystemModel>
#include <QSizePolicy>
#include <QGraphicsLinearLayout>

#include <hbmenu.h>
#include <hblistview.h>
#include <hbtreeview.h>
#include <hbabstractviewitem.h>
#include <hbaction.h>
#include <hbsearchpanel.h>
#include <hblabel.h>

// These define comes from implementation of QFileSystemModel
#define QFileSystemSortName 0
#define QFileSystemSortSize 1 
#define QFileSystemSortType 2
#define QFileSystemSortTime 3

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

    mTreeView->setModel( 0 );
    mListView->setModel( 0 );
    delete mModel;
    
    delete mFileIconProvider;
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

    int err = checkPathAndSetStyle( pathName );
    switch( err )
        {
        case FmErrNone:
            {
            mListView->setRootIndex( mModel->setRootPath( pathName ) );
            emit currentPathChanged( pathName );
            break;
            }
        case FmErrPathNotExist:
            {
            FmDlgUtils::information( hbTrId( "Path is not exist" ) );
            break;
            }
        case FmErrDriveNotAvailable:
            {
            // do not take any action as widget set note in label already. 
            break;
            }
        case FmErrDriveDenied:
        case FmErrPathDenied:
            {
            FmDlgUtils::information( hbTrId( "Can not access" ) );
            break;
            }
        default:
            Q_ASSERT_X( false, "setRootPath", "please handle all error from isPathAccessabel" );
            break;
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
}




bool FmFileBrowseWidget::cdUp()
{
	if (mStyle == ListStyle) {
        QModelIndex parentIndex = mListView->rootIndex().parent();
		// QFileSystemModel will auto refresh for file/folder change
        if (parentIndex.isValid()) {
			changeRootIndex( parentIndex );
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
        this, SLOT( on_viewAction_triggered() ), Qt::QueuedConnection );

    //copy
    HbAction *copyAction = new HbAction();
    copyAction->setObjectName( "copyAction" );
    copyAction->setText( hbTrId( "txt_fmgr_menu_copy" ) );
    contextMenu->addAction( copyAction );

    connect( copyAction, SIGNAL( triggered() ),
    this, SLOT( on_copyAction_triggered() ), Qt::QueuedConnection );

    
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
        this, SLOT( on_moveAction_triggered() ), Qt::QueuedConnection );
    
        //Delete
        HbAction *deleteAction = new HbAction();
        deleteAction->setObjectName( "deleteAction" );
        deleteAction->setText( hbTrId( "txt_fmgr_menu_delete" ) );
        contextMenu->addAction( deleteAction );
    
        connect( deleteAction, SIGNAL( triggered() ),
        this, SLOT( on_deleteAction_triggered() ), Qt::QueuedConnection );
    
        //rename
        HbAction *renameAction = new HbAction();
        renameAction->setObjectName( "renameAction" );
        renameAction->setText( hbTrId( "txt_fmgr_menu_rename" ) );
        contextMenu->addAction( renameAction );
    
        connect( renameAction, SIGNAL( triggered() ),
        this, SLOT( on_renameAction_triggered() ), Qt::QueuedConnection );
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
    connect( mListView, SIGNAL( pressed( const QModelIndex & ) ),
        this, SLOT( on_list_pressed( const QModelIndex & ) ) );
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
    mModel = new QFileSystemModel( this );
    mModel->setReadOnly( false );
    
    mFileIconProvider = new FmFileIconProvider();
    mModel->setIconProvider( mFileIconProvider );
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
        this, SLOT( on_searchPanel_searchOptionsClicked() ), Qt::QueuedConnection );
    
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
	QString filePath = mModel->fileInfo( index ).absoluteFilePath();
	setRootPath( filePath );
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
    // This slot will be triggered when drive inserted/ejected
	// Because filemanger do not notify dir/files changed yet( QFileSystem will auto refresh.)
    QString currPath( currentPath().absoluteFilePath() );
    
    if( currPath.isEmpty() ) {
        // label style and no data shown( dirve is not present or locked, or corrupt )
    
        //set path as drive root, cause refresh, so that data can be shown when insert MMC in device.
        setRootPath( mCurrentDrive );
        // update title
    } else {
        // display drive data normally
        // ignore path refresh event as QFileSystemModel will auto refresh.
    
        // Handle drive refresh event as drive may be ejected.
        if( path.isEmpty() ) { // path is empty means drive is changed.
            checkPathAndSetStyle( currPath );
        }
    }
    emit setTitle( FmUtils::fillDriveVolume( mCurrentDrive, true ) );
    FmLogger::log( "FmFileBrowseWidget::refreshModel end" );
}

int FmFileBrowseWidget::checkPathAndSetStyle( const QString& path )
{
    int err = FmUtils::isPathAccessabel( path );
    switch( err )
        {
        case FmErrNone:
            {
            setStyle( mFileBrowseStyle );
            emit setEmptyMenu( false );
            break;
            }
        case FmErrDriveNotAvailable:
            {
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
			//hide search panel when the drive is removed 
            if ( mSearchPanel->isVisible() ){
                mSearchPanel->hide();
                mLayout->removeItem( mSearchPanel );
            }    
            emit setEmptyMenu( true );
            break;
            }
        case FmErrPathNotExist:
        case FmErrDriveDenied:
        case FmErrPathDenied:
            {
            // do not tack any action, error note shoule be shown by invoker.
            // checkPathAndSetStyle just check path and set style.
            break;
            }
        default:
            Q_ASSERT_X( false, "checkPathAndSetStyle", "please handle all error from isPathAccessabel" );
            break;
        }
    return err;
}

void FmFileBrowseWidget::sortFiles( TSortType sortType )
{
    switch( sortType ){
        case ESortByName:{
			mModel->sort( QFileSystemSortName );
        }
            break;
        case ESortByTime:{
            mModel->sort( QFileSystemSortTime );
        }
            break;
        case ESortBySize:{
            mModel->sort( QFileSystemSortSize );
        }
            break;
        case ESortByType:{
            mModel->sort( QFileSystemSortType );
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
    mFindTargetPath = FmUtils::fillPathWithSplash( FmFileDialog::getExistingDirectory( 0, hbTrId( "Look in:" ), QString(""),
        QStringList() ) );
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
                FmDlgUtils::information( hbTrId( "Operatin already started!" ) );
                break;
            case FmErrWrongParam:
                FmDlgUtils::information( hbTrId( "Wrong parameters!" ) );
                break;
            default:
                FmDlgUtils::information( hbTrId( "Operation fail to start!" ) );
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
                FmDlgUtils::information( hbTrId( "Operatin already started!" ) );
                break;
            case FmErrWrongParam:
                FmDlgUtils::information( hbTrId( "Wrong parameters!" ) );
                break;
            default:
                FmDlgUtils::information( hbTrId( "Operation fail to start!" ) );
        }
    }

}
 
void FmFileBrowseWidget::on_moveAction_triggered()
{
    QStringList fileList;
    fileList.push_back( mModel->filePath( mCurrentItem->modelIndex() ) );

    QString targetPathName = FmFileDialog::getExistingDirectory( 0, hbTrId( "move to" ),
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
                FmDlgUtils::information( hbTrId( "Operatin already started!" ) );
                break;
            case FmErrWrongParam:
                FmDlgUtils::information( hbTrId( "Wrong parameters!" ) );
                break;
            default:
                FmDlgUtils::information( hbTrId( "Operation fail to start!" ) );
        }
    }
}


void FmFileBrowseWidget::on_renameAction_triggered()
{
    QString filePath = mModel->filePath( mCurrentItem->modelIndex() );
    QFileInfo fileInfo = mModel->fileInfo( mCurrentItem->modelIndex() );
    int maxFileNameLength = FmUtils::getMaxFileNameLength();
    // regExpList used to disable primary action of text query dialog if input text is not match
    QStringList regExpList = (QStringList() << Regex_ValidFileFolderName << Regex_ValidNotEndWithDot );

    QString oldSuffix( fileInfo.suffix() );
    QString newName( fileInfo.fileName() );
    while( FmDlgUtils::showTextQuery( hbTrId( "Enter new name for %1" ).arg( newName ), newName, regExpList,
            maxFileNameLength, QString() , true ) ){
        // remove whitespace from the start and the end.
        newName = newName.trimmed();
        QString newTargetPath = FmUtils::fillPathWithSplash(
            fileInfo.absolutePath() ) + newName;
        QFileInfo newFileInfo( newTargetPath );
        QString errString;
        // check if name/path is available for use
        // add new Name to check, in order to avoid problem of newName is empty
        if( !FmUtils::checkNewFolderOrFile( newName, newTargetPath, errString ) ) {
            FmDlgUtils::information( errString );
            continue;
        }
        if( !rename( fileInfo.absoluteFilePath(), newTargetPath ) ) {
            FmDlgUtils::information( hbTrId("Rename failed!") );
        }
        else {
            if ( oldSuffix != newFileInfo.suffix() ) {
                FmDlgUtils::information( hbTrId( "File may become unusable when file name extension is changed" ) );        
            }   
        }
        break;
    }
}

