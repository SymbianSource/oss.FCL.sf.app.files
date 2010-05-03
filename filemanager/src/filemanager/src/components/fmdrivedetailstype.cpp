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
 *      The source file of the drive details type
 */

#include "fmdrivedetailstype.h"
#include "fmutils.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

QList<FmDriveDetailsDataGroup*> FmDriveDetailsContent::queryDetailsContent()
{    
    QStringList typeFilters;
    QList< FmDriveDetailsDataGroup* > dataGroupList;
    typeFilters.append( QString( "*.bmp" ) );
    typeFilters.append( QString( "*.gif" ) );
    typeFilters.append( QString( "*.jpe" ) );
    typeFilters.append( QString( "*.jpeg" ) );
    typeFilters.append( QString( "*.jpg" ) );
    typeFilters.append( QString( "*.ota" ) );
    typeFilters.append( QString( "*.png" ) );
    typeFilters.append( QString( "*.tif" ) );
    typeFilters.append( QString( "*.tiff" ) );
    typeFilters.append( QString( "*.wbmp" ) );
    typeFilters.append( QString( "*.wmf" ) );
    typeFilters.append( QString( "*.jp2" ) );
    typeFilters.append( QString( "*.jpg2" ) );
    typeFilters.append( QString( "*.jp3" ) );
    typeFilters.append( QString( "*.ico" ) );
    typeFilters.append( QString( "*.vcf" ) );

    dataGroupList.append( new FmDriveDetailsDataGroup( FmDriveDetailsDataGroup::EGroupImages, typeFilters ));
    
    typeFilters.clear();
    typeFilters.append( QString( "*.aac" ) );
    typeFilters.append( QString( "*.amr" ) );
    typeFilters.append( QString( "*.au" ) );
    typeFilters.append( QString( "*.awb" ) );
    typeFilters.append( QString( "*.mid" ) );
    typeFilters.append( QString( "*.mp3" ) );
    typeFilters.append( QString( "*.ra" ) );
    typeFilters.append( QString( "*.rmf" ) );
    typeFilters.append( QString( "*.rng" ) );
    typeFilters.append( QString( "*.snd" ) );
    typeFilters.append( QString( "*.wav" ) );
    typeFilters.append( QString( "*.wve" ) );
    typeFilters.append( QString( "*.wma" ) );
    typeFilters.append( QString( "*.m4a" ) );
    typeFilters.append( QString( "*.ott" ) );
    typeFilters.append( QString( "*.mxmf" ) );

    dataGroupList.append( new FmDriveDetailsDataGroup( FmDriveDetailsDataGroup::EGroupSoundFiles, typeFilters ));
    
    typeFilters.clear();
    typeFilters.append( QString( "*.jad" ) );
    typeFilters.append( QString( "*.jar" ) );
 
    dataGroupList.append( new FmDriveDetailsDataGroup( FmDriveDetailsDataGroup::EGroupMidpJava, typeFilters ));

    typeFilters.clear();
    typeFilters.append( QString( "*.sis" ) );
    typeFilters.append( QString( "*.sisx" ) );

    dataGroupList.append( new FmDriveDetailsDataGroup( FmDriveDetailsDataGroup::EGroupNativeApps, typeFilters ));

    typeFilters.clear();
    typeFilters.append( QString( "*.3gp" ) );
    typeFilters.append( QString( "*.mp4" ) );
    typeFilters.append( QString( "*.nim" ) );
    typeFilters.append( QString( "*.rm" ) );
    typeFilters.append( QString( "*.rv" ) );
    typeFilters.append( QString( "*.wmv" ) );
    typeFilters.append( QString( "*.3g2" ) );
 
    dataGroupList.append( new FmDriveDetailsDataGroup( FmDriveDetailsDataGroup::EGroupVideos, typeFilters ));
 
    typeFilters.clear();
    typeFilters.append( QString( "*.doc" ) );
    typeFilters.append( QString( "*.pdf" ) );
    typeFilters.append( QString( "*.pps" ) );
    typeFilters.append( QString( "*.ppt" ) );
    typeFilters.append( QString( "*.txt" ) );
    typeFilters.append( QString( "*.xls" ) );

    dataGroupList.append( new FmDriveDetailsDataGroup( FmDriveDetailsDataGroup::EGroupDocuments, typeFilters ));

    typeFilters.clear();
    typeFilters.append( QString( "*.vcs" ) );

    dataGroupList.append( new FmDriveDetailsDataGroup( FmDriveDetailsDataGroup::EGroupCalendar, typeFilters ));
    
    return dataGroupList;
    
}

int FmDriveDetailsContent::querySizeofContent( const QString &driveName,
        QList<FmDriveDetailsSize*> &detailsSizeList, volatile bool *isStopped )
{   
    quint64 totalSize = 0;
    
    QString path;
    path.clear();
    
    if( FmUtils::isDriveC( driveName )){
        path = QString( "c:\\Data\\" );
    }
    else{
        path = driveName;
    }
    
    QList< FmDriveDetailsDataGroup* > dataGroupList = queryDetailsContent();
    
    QStringList typeFilter;
    detailsSizeList.clear();
    QDir dir( path );

    for( QList< FmDriveDetailsDataGroup* >::iterator it = dataGroupList.begin(); 
            it!= dataGroupList.end(); ++it ){  
        if ( *isStopped ){
            return FmErrCancel;
        }
        totalSize = 0;
        typeFilter = ( *it )->typeFilters();
        
        QList<QDir> dirs;
        dirs.append( QDir( path ) );
                    
        while (!dirs.isEmpty()) {
            QFileInfoList infoList = dirs.first().entryInfoList( QDir::NoDotAndDotDot | QDir::AllEntries );
            QFileInfoList filterInforList = dirs.first().entryInfoList( typeFilter, QDir::NoDotAndDotDot | QDir::Files );
            
            for ( QFileInfoList::Iterator it = filterInforList.begin(); it != filterInforList.end(); ++it ) {
                if ( *isStopped ){
                    return FmErrCancel;
                }
                
                if ( it->isFile() ) {
                    totalSize += it->size();
                }
            }
            
            for ( QFileInfoList::Iterator it = infoList.begin(); it != infoList.end(); ++it ) {
                if ( *isStopped ){
                    return FmErrCancel;
                }
                
                if ( it->isDir() ) {
                    dirs.append( QDir( it->absoluteFilePath() ) );
                } 
            }
            
            dirs.removeFirst();
        }
    
        detailsSizeList.append( new FmDriveDetailsSize( ( *it )->dataGroups(), totalSize ) );
    }
    return FmErrNone;
}

int FmFolderDetails::queryDetailOfContentList( const QStringList folderPathList,int &numofFolders, 
                                    int &numofFiles, quint64 &totalSize,
                                    volatile bool *isStopped, bool isSysHiddenIncluded )
{
    numofFolders = 0;
    numofFiles = 0;
    totalSize = 0;

    int tempNumofFolders = 0;
    int tempNumofFiles = 0;
    quint64 tempSize = 0;

    int ret = FmErrNone;

    foreach( const QString& folderPath, folderPathList ) {
        QFileInfo fileInfo( folderPath );
        if( fileInfo.isDir() ){
            int tempNumofSubFolders = 0;
            ret = getNumofSubfolders( folderPath, tempNumofSubFolders,
                tempNumofFiles, tempSize, isStopped, isSysHiddenIncluded );
            tempNumofFolders = tempNumofSubFolders + 1; // add itself to folder count;
        } else if( fileInfo.isFile() ) {
            tempNumofFiles = 1;
            tempSize = fileInfo.size();
            ret = FmErrNone;
        } else {
            ret = FmErrIsNotFileOrFolder;
        }
        if( ret != FmErrNone ) {
            return ret;
        }
        numofFolders+= tempNumofFolders;
        numofFiles  += tempNumofFiles;
        totalSize   += tempSize;

        tempNumofFolders = 0;
        tempNumofFiles      = 0;
        tempSize    = 0;
    }

    return ret;
}

int FmFolderDetails::getNumofSubfolders( const QString &folderPath, int &numofSubFolders, 
                                          int &numofFiles, quint64 &sizeofFolder,
                                          volatile bool *isStopped, bool isSysHiddenIncluded )
{
    numofSubFolders = 0;
    numofFiles = 0;
    sizeofFolder = 0;

    QList<QDir> dirs;
    dirs.append( QDir( folderPath ) );
    
    while (!dirs.isEmpty()) {
        QDir::Filters filter = QDir::NoDotAndDotDot | QDir::AllEntries;
        if( isSysHiddenIncluded ) {
            filter = filter | QDir::Hidden | QDir::System;
        }

        QFileInfoList infoList = dirs.first().entryInfoList( filter );
        for ( QFileInfoList::Iterator it = infoList.begin(); it != infoList.end(); ++it ) {
            if ( *isStopped ){
                return FmErrCancel;
            }
            
            if ( it->isFile() ) {
                ++numofFiles;
                sizeofFolder += it->size();
            }
            else if ( it->isDir() ) {
                ++numofSubFolders;
                dirs.append( QDir( it->absoluteFilePath() ) );
            } 
        }
        
        dirs.removeFirst();
    }
    
    return FmErrNone;
}
