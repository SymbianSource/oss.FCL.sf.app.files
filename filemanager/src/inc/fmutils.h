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
* 
* Description:
*     The header file of the file manager utilities
*/

#ifndef FMUTILS_H
#define FMUTILS_H

#include "fmcommon.h"

#include <QString>

class FmDriverInfo
{
public:
    enum driveState
    {
        EDriveNotPresent = 0x1,
        EDriveLocked = 0x2,
        EDriveCorrupted = 0x4,
        EDriveWriteProtected = 0x8,
        EDriveRemovable = 0x10,
        EDriveRom = 0x20,
        EDriveFormattable = 0x40,
        EDriveFormatted = 0x80,
        EDriveLockable = 0x100,
        EDrivePasswordProtected = 0x200,
        EDriveBackupped = 0x400,
        EDriveConnected = 0x800,
        EDriveEjectable = 0x1000,
        EDriveInUse = 0x2000,
        EDriveMassStorage = 0x4000,
        EDriveRam = 0x8000,
        EDriveAvailable = 0x10000
     };
    Q_DECLARE_FLAGS( DriveState, driveState )
    
    FmDriverInfo( quint64 s, quint64 f, const QString &n, const QString &vN, const quint32 driveState ) :
        mSize( s ), mFreeSize( f ), mName( n ), mVolumeName( vN ), mDriveState( driveState ) {}
    FmDriverInfo( const FmDriverInfo &other )
    {
        *this = other;
    }

    FmDriverInfo &operator= ( const FmDriverInfo &rhs )
    {
        this->mSize = rhs.mSize;
        this->mFreeSize = rhs.mFreeSize;
        this->mName = rhs.mName;
        this->mVolumeName = rhs.mVolumeName;
        this->mDriveState = rhs.mDriveState;
        return *this;
    }

    quint64 size() const { return mSize; }
    quint64 freeSize() const { return mFreeSize; }
    QString name() const { return mName; }
    QString volumeName() const { return mVolumeName; }
    DriveState driveState() const { return mDriveState; }
    
private:
    quint64 mSize;
    quint64 mFreeSize;
    QString mName;
    QString mVolumeName;
    DriveState mDriveState;
};
Q_DECLARE_OPERATORS_FOR_FLAGS( FmDriverInfo::DriveState )

class FmUtils
{
public:
    static QString getDriveNameFromPath( const QString &path );
	static QString getDriveLetterFromPath( const QString &path );
    static FmDriverInfo queryDriverInfo( const QString &driverName );
    static QString formatStorageSize( quint64 size );
    //static quint32 getDriverState( const QString &driverName );
    static int removeDrivePwd( const QString &driverName, const QString &Pwd );
    static int unlockDrive( const QString &driverName, const QString &Pwd );
    static int checkDrivePwd( const QString &driverName, const QString &pwd);
    static int setDrivePwd( const QString &driverName, const QString &oldPwd, const QString &newPwd);
    static void emptyPwd( QString &pwd );
    static int renameDrive( const QString &driverName, const QString &newVolumeName);
    static void ejectDrive( const QString &driverName );
    static QString getFileType( const QString &filePath  );
    static quint64 getDriveDetailsResult( const QString &folderPath, const QString &extension );
    static bool isDriveC( const QString &driverName );
    static bool isDrive( const QString &path );
	static void createDefaultFolders( const QString &driverName );
    static QString fillPathWithSplash( const QString &filePath );
    static QString removePathSplash( const QString &filePath );
    static QString formatPath( const QString &path  );
    static bool checkDriveFilter( const QString &driveName );
    static QString checkDriveToFolderFilter( const QString &path );
    static QString checkFolderToDriveFilter( const QString &path );
    static bool isPathAccessabel( const QString &path );
    static bool isDriveAvailable( const QString &path );
    static bool isPathEqual( const QString &pathFst, const QString &pathLast );

    /// fill driveList of drives can be shown in driveListView
    static void getDriveList( QStringList &driveList, bool isHideUnAvailableDrive );
    static QString fillDriveVolume( QString driveName, bool isFillWithDefaultVolume );

    static int launchFile( const QString &filePath );
    static void sendFiles( QStringList &filePathList );
    static QString getBurConfigPath( QString appPath );
    static bool isDefaultFolder( const QString &folderPath  );
    static QString Localize( const QString &path );

};

#endif

