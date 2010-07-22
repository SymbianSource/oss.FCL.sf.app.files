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
 *      The header file of the drive details type
 */

#ifndef FMDRIVEDETAILSTYPE_H
#define FMDRIVEDETAILSTYPE_H

#include <QString>
#include <QList>
#include <QStringList>

class FmDriveDetailsDataGroup
{
public:
    // used to identify recognize method
    enum TDataRecognizeType{
        EDataRecognizeExtName = 0,  // recognize data by extension of filename
        EDataRecognizeAbsolutePath  // recognize data by ablsolute file path
    };
    
    enum TDataGroups{
        EGroupImages = 0,
        EGroupSoundFiles,
        EGroupMidpJava,
        EGroupNativeApps,
        EGroupVideos,
        EGroupDocuments,
        EGroupCalendar,
        EGroupContacts,
        EGroupMessages,
        EGroupOthers
    };
    
public:
       FmDriveDetailsDataGroup( TDataGroups dataGroups,
                    QStringList typeFilters,
                    TDataRecognizeType recognizeType ) 
                   : mDataGroups( dataGroups ), mTypeFilters( typeFilters ),
                     mDataRecognizeType( recognizeType )
        {
        }
        FmDriveDetailsDataGroup( const FmDriveDetailsDataGroup &other )
        {
            *this = other;
        }
        
        FmDriveDetailsDataGroup &operator= ( const FmDriveDetailsDataGroup &rhs )
        {
            this->mDataGroups = rhs.mDataGroups;
            this->mTypeFilters = rhs.mTypeFilters;
            return *this;
        }
        
        TDataGroups dataGroups() const { return mDataGroups; }
        QStringList typeFilters() const { return mTypeFilters; }
        TDataRecognizeType dataRecognizeType() const { return mDataRecognizeType; }
    
private:
      TDataGroups mDataGroups;
      QStringList mTypeFilters;
      TDataRecognizeType mDataRecognizeType;
};

class FmDriveDetailsSize
{
public:
    FmDriveDetailsSize( FmDriveDetailsDataGroup::TDataGroups dataGroups, quint64 size ) 
        : mDataGroups( dataGroups ), mSize( size ) {}
    FmDriveDetailsSize( const FmDriveDetailsSize &other )
    {
        *this = other;
    }
    
    FmDriveDetailsSize &operator= ( const FmDriveDetailsSize &rhs )
    {
        this->mDataGroups = rhs.mDataGroups;
        this->mSize = rhs.mSize;
        return *this;
    }
    
    FmDriveDetailsDataGroup::TDataGroups dataGroups() const { return mDataGroups; }
    quint64 size() const { return mSize; }
    
private:
    FmDriveDetailsDataGroup::TDataGroups mDataGroups;
    quint64 mSize;
};

class FmDriveDetailsContent
{
public:
    static QList<FmDriveDetailsDataGroup*> queryDetailsContent(); 
    static int querySizeofContent(
            const QString &driveName, QList<FmDriveDetailsSize*> &detailsSizeList, volatile bool *isStopped );
    /**
     * Gets data size for single FmDriveDetailsDataGroup, the method is scan files for extension.
     * @param driveName which drive is searching
     * @param dataGroup which dataGroup is searching, for example, EGroupImages, EGroupSoundFiles...
     * @param detailsSizeList if got result, new FmDriveDetailsSize will be appended to detailsSizeList
     * @param isStopped isStopped will be set as true if user cancel this operation
     * @return Filemanage wide error. Please refer to fmdefine.h
     */
    static int getDataSizeByExtName( const QString &driveName, const FmDriveDetailsDataGroup* const dataGroup,
               QList<FmDriveDetailsSize*> &detailsSizeList, volatile bool *isStopped );
    
    /**
     * Gets data size for single FmDriveDetailsDataGroup, the method is find file of absolute path
     * @param driveName which drive is searching
     * @param dataGroup which dataGroup is searching, for example, EGroupContacts...
     * @param detailsSizeList if got result, new FmDriveDetailsSize will be appended to detailsSizeList
     * @param isStopped isStopped will be set as true if user cancel this operation
     * @return Filemanage wide error. Please refer to fmdefine.h
     */
    static int getDataSizeByAbsolutePath( const QString &driveName, const FmDriveDetailsDataGroup* const dataGroup, 
               QList<FmDriveDetailsSize*> &detailsSizeList, volatile bool *isStopped );
};

class FmFolderDetails
{
public:

    static int queryDetailOfContentList( const QStringList folderPathList,int &numofFolders, 
                                    int &numofFiles, quint64 &totalSize, volatile bool *isStopped, bool isSysHiddenIncluded = false );

    static int getNumofSubfolders( const QString &folderPath, int &numofSubFolders, 
                                    int &numofFiles, quint64 &sizeofFolder,
                                    volatile bool *isStopped, bool  isSysHiddenIncluded = false );


};

#endif /* FMDRIVEDETAILSTYPE_H */

