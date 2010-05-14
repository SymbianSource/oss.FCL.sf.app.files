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
 *     The source file of view details dialog of file manager
 */

#include "fmviewdetailsdialog.h"
#include "fmutils.h"
#include "fmviewdetailsitem.h"
#include "fmdatalistmodel.h"
#include "fmdrivedetailstype.h"
#include "fmviewmanager.h"

#include <QDirModel>
#include <QFileInfo>
#include <QDateTime>

#include <hblabel.h>
#include <hblistview.h>
#include <hbaction.h>

FmViewDetailsDialog::FmViewDetailsDialog( QGraphicsItem *parent ) 
    : FmDialog( parent ),
      mNumofSubFolers( 0 ),
      mNumofFiles( 0 ),
      mSizeofFolder( 0 ),
      mSizeofImage( 0 ),
      mSizeofSounds( 0 ),
      mSizeofVideo( 0 ),
      mSizeofJava( 0 ),
      mSizeofNativeFiles( 0 ),
      mSizeofDocument( 0 ),
      mSizeofCalendar( 0 )      
{
    init();
    
    setTimeout( NoTimeout );
}

FmViewDetailsDialog::~FmViewDetailsDialog()
{
    if( mDataListModel ){
        mListView->setModel( 0 );
        delete mListView;
    }
    
    if( mDirModel ){
        delete mDirModel;
    }

}

void FmViewDetailsDialog::init()
{
    mHeaderLabel = new HbLabel( this );
    setHeadingWidget( mHeaderLabel );

    mListView = new HbListView( this );
    setContentWidget( mListView );
    
    mDataListModel = new FmDataListModel();
    mListView->setModel( mDataListModel ); 
    mListView->setItemPrototype( new FmViewDetailsItem( this ) );

    mOkAction = new HbAction( this );
    mOkAction->setText( tr("ok") );
    setPrimaryAction( mOkAction );
    
    mDirModel = new QDirModel();
}

void FmViewDetailsDialog::setDriveDetails( const QString &driverName )
{
    const QString dataDir = "C:\\data";
    mHeaderLabel->setPlainText( hbTrId( "txt_fmgr_title_memory_details" ) );
    
    quint64 sizeOfOthers = 0;
    
    FmDriverInfo driverInfo = FmUtils::queryDriverInfo( driverName );
    QString detailString( "" );
    
    mDataListModel->removeRows( 0, mDataListModel->rowCount() );
    mDataListModel->insertRows( 0, EDriveDetailEntryEnd );
    
    detailString.append( hbTrId ( "txt_fmgr_dblist_memory_name" ) );
    QVariant variant( detailString ); 
    mDataListModel->setData(  mDataListModel->index( EMemoryName ), variant, Qt::DisplayRole );

    detailString.clear();
    detailString.append( FmUtils::fillDriveVolume( driverInfo.name(), true ) ); 
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EMemoryName ), variant, Qt::UserRole );
    
    detailString.clear();
    detailString.append( hbTrId ( "txt_fmgr_dblist_size_memory" ) ); 
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EMemorySize ), variant, Qt::DisplayRole );

    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( driverInfo.size() ) );
    variant = QVariant( detailString );; 
    mDataListModel->setData(  mDataListModel->index( EMemorySize ), variant, Qt::UserRole );

    detailString.clear();
    detailString.append( hbTrId ( "txt_fmgr_dblist_free_memory" ) ); 
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EMemoryFree ), variant, Qt::DisplayRole );

    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( driverInfo.freeSize() ) );
    variant = QVariant( detailString );; 
    mDataListModel->setData(  mDataListModel->index( EMemoryFree ), variant, Qt::UserRole );  
    
    sizeOfOthers = driverInfo.size() - driverInfo.freeSize();
    
//    getContentDetails( dataDir );
    
    detailString.clear();
    detailString.append( hbTrId ( "Images:" ) ); 
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EMemoryImages ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( mSizeofImage ) );
    variant = QVariant( detailString );; 
    mDataListModel->setData(  mDataListModel->index( EMemoryImages ), variant, Qt::UserRole );  
    
    sizeOfOthers -= mSizeofImage;
    
    detailString.clear();
    detailString.append( hbTrId ( "Sounds:" ) ); 
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EMemorySounds ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( mSizeofSounds ) );
    variant = QVariant( detailString );; 
    mDataListModel->setData(  mDataListModel->index( EMemorySounds ), variant, Qt::UserRole ); 
    
    sizeOfOthers -= mSizeofSounds;
    
    detailString.clear();
    detailString.append( hbTrId ( "Video:" ) ); 
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EMemoryVideos ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( mSizeofVideo ) );
    variant = QVariant( detailString );; 
    mDataListModel->setData(  mDataListModel->index( EMemoryVideos ), variant, Qt::UserRole ); 
    
    sizeOfOthers -= mSizeofVideo;
    
    detailString.clear();
    detailString.append( hbTrId ( "Java Files:" ) ); 
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EMemoryMidpJava ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( mSizeofJava ) );
    variant = QVariant( detailString );; 
    mDataListModel->setData(  mDataListModel->index( EMemoryMidpJava ), variant, Qt::UserRole );
    
    sizeOfOthers -= mSizeofJava;
    
    detailString.clear();
    detailString.append( hbTrId ( "Sis files:" ) ); 
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EMemoryNativeApps ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( mSizeofNativeFiles ) );
    variant = QVariant( detailString );; 
    mDataListModel->setData(  mDataListModel->index( EMemoryNativeApps ), variant, Qt::UserRole ); 
    
    sizeOfOthers -= mSizeofNativeFiles;
    
    detailString.clear();
    detailString.append( hbTrId ( "Documents:" ) ); 
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EMemoryDocuments ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( mSizeofDocument ) );
    variant = QVariant( detailString );; 
    mDataListModel->setData(  mDataListModel->index( EMemoryDocuments ), variant, Qt::UserRole ); 
    
    sizeOfOthers -= mSizeofDocument;
    
    detailString.clear();
    detailString.append( hbTrId ( "Calendars:" ) ); 
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EMemoryCalendar ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( mSizeofCalendar ) );
    variant = QVariant( detailString );; 
    mDataListModel->setData(  mDataListModel->index( EMemoryCalendar ), variant, Qt::UserRole );
    
    sizeOfOthers -= mSizeofCalendar;
    
    detailString.clear();
    detailString.append( hbTrId ( "Others:" ) ); 
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EMemoryOthers ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( sizeOfOthers ) );
    variant = QVariant( detailString );; 
    mDataListModel->setData(  mDataListModel->index( EMemoryOthers ), variant, Qt::UserRole );   
}

void FmViewDetailsDialog::setFolderDetails( const QString &folderPath )
{
    mHeaderLabel->setPlainText( hbTrId( "txt_fmgr_title_folder_details" ) );
    
    QString detailString( "" );
    
    QModelIndex modelIndex = mDirModel->index( folderPath );
    
    mDataListModel->removeRows( 0, mDataListModel->rowCount() );
    mDataListModel->insertRows( 0, EFolderDetailEntryEnd );
    
    detailString.append( hbTrId ( "txt_fmgr_dblist_name_folder" ) );
    QVariant variant( detailString ); 
    mDataListModel->setData(  mDataListModel->index( EFolderName ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( mDirModel->fileName( modelIndex ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EFolderName ), variant, Qt::UserRole );
    
    QFileInfo fileInfo = mDirModel->fileInfo( mDirModel->index( folderPath ) );
    QDateTime modifiedDateTime = fileInfo.lastModified ();
    QDate modifiedDate = modifiedDateTime.date();
    QTime modifiedTime = modifiedDateTime.time();
    
    detailString.clear();
    detailString.append( hbTrId ( "txt_fmgr_dblist_date_folder" ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EModifiedDate ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( modifiedDate.toString() );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EModifiedDate ), variant, Qt::UserRole );

    detailString.clear();
    detailString.append( hbTrId ( "txt_fmgr_dblist_time_folder" ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EModifiedTime ), variant, Qt::DisplayRole );
   
    detailString.clear();
    detailString.append( modifiedTime.toString() );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EModifiedTime ), variant, Qt::UserRole );
    
    detailString.clear();
    detailString.append( hbTrId ( "txt_fmgr_dblist_size_folder" ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EFolderSize ), variant, Qt::DisplayRole );
   
    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( mSizeofFolder ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EFolderSize ), variant, Qt::UserRole );

    detailString.clear();
    detailString.append( hbTrId ( "txt_fmgr_dblist_subfolders" ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( ENumofSubFolders ), variant, Qt::DisplayRole );

    detailString.clear();
    detailString = QString::number( mNumofSubFolers );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( ENumofSubFolders ), variant, Qt::UserRole );

    detailString.clear();
    detailString.append( hbTrId ( "txt_fmgr_dblist_files" ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( ENumofFiles ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString = QString::number( mNumofFiles );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( ENumofFiles ), variant, Qt::UserRole );
}

void FmViewDetailsDialog::setFileDetails( const QString &filePath )
{
    mHeaderLabel->setPlainText( hbTrId( "txt_fmgr_title_file_details") );
    
    QString detailString( "" );
    
    QModelIndex modelIndex = mDirModel->index( filePath );
    
    mDataListModel->removeRows( 0, mDataListModel->rowCount() );
    mDataListModel->insertRows( 0, EFileDetailEntryEnd );
    
    detailString.append( hbTrId ( "txt_fmgr_dblist_name_file" ) );
    QVariant variant( detailString ); 
    mDataListModel->setData(  mDataListModel->index( EFileName ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( mDirModel->fileName( modelIndex ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EFileName ), variant, Qt::UserRole );
    
    detailString.clear();
    detailString.append( hbTrId ( "txt_fmgr_dblist_type_file" ) );
    variant = QVariant( detailString ); 
    mDataListModel->setData(  mDataListModel->index( EFileType ), variant, Qt::DisplayRole );
    
    QFileInfo fileInfo = mDirModel->fileInfo( mDirModel->index( filePath ) );
    
    QString fileType = FmUtils::getFileType( filePath );
    if( fileType.isEmpty() ){
        fileType.append( fileInfo.suffix() );
    }
    detailString.clear();
    detailString.append( fileType );
    variant = QVariant( detailString ); 
    mDataListModel->setData(  mDataListModel->index( EFileType ), variant, Qt::UserRole );

    QDateTime modifiedDateTime = fileInfo.lastModified ();
    QDate modifiedDate = modifiedDateTime.date();
    QTime modifiedTime = modifiedDateTime.time();
    
    detailString.clear();
    detailString.append( hbTrId ( "txt_fmgr_dblist_date_file" ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EFileModifiedDate ), variant, Qt::DisplayRole );
    
    detailString.clear();
    detailString.append( modifiedDate.toString() );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EFileModifiedDate ), variant, Qt::UserRole );

    detailString.clear();
    detailString.append( hbTrId ( "txt_fmgr_dblist_time_file" ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EFileModifiedTime ), variant, Qt::DisplayRole );
   
    detailString.clear();
    detailString.append( modifiedTime.toString() );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EFileModifiedTime ), variant, Qt::UserRole );

    detailString.clear();
    detailString.append( hbTrId ( "txt_fmgr_dblist_size_file" ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EFileSize ), variant, Qt::DisplayRole );
   
    detailString.clear();
    detailString.append( FmUtils::formatStorageSize( fileInfo.size() ) );
    variant = QVariant( detailString );
    mDataListModel->setData(  mDataListModel->index( EFileSize ), variant, Qt::UserRole );
}

void FmViewDetailsDialog::setNumofSubfolders(  int numofSubFolders, int numofFiles, quint64 sizeofFolder )
{
    mNumofSubFolers = 0;
    mNumofFiles = 0;
    mSizeofFolder = 0;
    
    mNumofSubFolers = numofSubFolders;
    mNumofFiles = numofFiles;
    mSizeofFolder = sizeofFolder;    
}

void FmViewDetailsDialog::setSizeofContent( QList<FmDriveDetailsSize*> detailsSizeList )
{
    mSizeofImage = 0;
    mSizeofSounds = 0;
    mSizeofVideo = 0;
    mSizeofJava = 0;
    mSizeofNativeFiles = 0;
    mSizeofDocument = 0;
    mSizeofCalendar = 0; 
    
    for( QList< FmDriveDetailsSize* >::iterator it = detailsSizeList.begin(); 
            it!= detailsSizeList.end(); ++it ){
    
        switch( ( *it )->dataGroups() ){
            case FmDriveDetailsDataGroup::EGroupImages:
            {
                mSizeofImage = ( *it )->size();             
            }
                break;
            case FmDriveDetailsDataGroup::EGroupSoundFiles:
            {
                mSizeofSounds = ( *it )->size();             
            }
                break;
            case FmDriveDetailsDataGroup::EGroupMidpJava:
            {
                mSizeofJava = ( *it )->size();             
            }
                break;
            case FmDriveDetailsDataGroup::EGroupNativeApps:
            {
                mSizeofNativeFiles = ( *it )->size();             
            }
                break;
            case FmDriveDetailsDataGroup::EGroupVideos:
            {
                mSizeofVideo = ( *it )->size();             
            }
                break;
            case FmDriveDetailsDataGroup::EGroupDocuments:
            {
                mSizeofDocument = ( *it )->size();             
            }
                break;
            case FmDriveDetailsDataGroup::EGroupCalendar:
            {
                mSizeofCalendar = ( *it )->size();             
            }
                break;
            default:
                break;
            
        }

    }
}

HbAction *FmViewDetailsDialog::executeDialog( FmDialog *dialog, const QString &associatedDrives )
{
    for( int i = 0; i < associatedDrives.length(); i++ ) {
        QString drive( associatedDrives[i] + QString( ":/" ) );
        if( !FmUtils::isDriveAvailable( drive ) ) {
            FmLogger::log( "executeDialog return 0_ " + associatedDrives );
            return 0;
        }
    }
    
	FmDlgCloseUnit dlgCloseUnit( dialog );
	dlgCloseUnit.addAssociatedDrives( associatedDrives );

	FmViewManager::viewManager()->addDlgCloseUnit( &dlgCloseUnit );
	FmLogger::log( " Exec Dialog start " );
    HbAction* action = dialog->exec();
	FmLogger::log( " Exec Dialog end " );
	FmViewManager::viewManager()->removeDlgCloseUnit( &dlgCloseUnit );
	return action;
}

void FmViewDetailsDialog::showDriveViewDetailsDialog( const QString &driverName, 
            QList<FmDriveDetailsSize*> detailsSizeList, const QString& associatedDrives )
{
	FmLogger::log( "showDriveViewDetailsDialog_" + associatedDrives );
    FmViewDetailsDialog viewDetailsDialog;
    
    viewDetailsDialog.setSizeofContent( detailsSizeList );
    viewDetailsDialog.setDriveDetails( driverName );
       
    //viewDetailsDialog.exec();
	executeDialog( &viewDetailsDialog , associatedDrives );
}

void FmViewDetailsDialog::showFolderViewDetailsDialog( const QString &folderPath,
           int numofSubFolders, int numofFiles, quint64 sizeofFolder, const QString& associatedDrives )
{
    FmViewDetailsDialog viewDetailsDialog;
    
    viewDetailsDialog.setNumofSubfolders( numofSubFolders, numofFiles, sizeofFolder );
    viewDetailsDialog.setFolderDetails( folderPath );
    
    //viewDetailsDialog.exec();
	executeDialog( &viewDetailsDialog , associatedDrives );
}

void FmViewDetailsDialog::showFileViewDetailsDialog( const QString &filePath, const QString& associatedDrives )
{
    FmViewDetailsDialog viewDetailsDialog;
    viewDetailsDialog.setFileDetails( filePath );
    
    //viewDetailsDialog.exec();
	executeDialog( &viewDetailsDialog , associatedDrives );
}
