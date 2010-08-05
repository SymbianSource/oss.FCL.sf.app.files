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
*     The source file of the file manager utilities on windows
*/

#include <windows.h>

#include "fmutils.h"
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QVariant>

#include <hbglobal.h>

#define BURCONFIGFILE  "burconfig.xml"
const int KMaxFileName=0x100;
const int KMaxPath=0x100;

QString FmUtils::getDriveNameFromPath( const QString &path )
{
    if( path.length() <3 ) {
        return QString();
    }
    return path.left( 3 );
}

QString FmUtils::getDriveLetterFromPath( const QString &path )
{
	if( path.length() <3 ) {
        return QString();
    }
    return path.left( 1 );
}

FmDriverInfo FmUtils::queryDriverInfo( const QString &driverName )
{
    quint64 size = 0;
    quint64 freeSize = 0;
    QString driver = driverName;
    
    driver.replace( '/', "\\" );
    if ( driver.right(1) != "\\" ) {
        driver.append( "\\" );
    }
    GetDiskFreeSpaceEx( (LPCWSTR)driver.constData(),
                        (PULARGE_INTEGER)&freeSize,
                        (PULARGE_INTEGER)&size,
                        0 );

    TCHAR volumeName[MAX_PATH + 1] = { 0 };
    GetVolumeInformation( (LPCWSTR)driver.constData(),
                          &volumeName[0],
                          MAX_PATH + 1,
                          0,
                          0,
                          0,
                          0,
                          0 );

    quint32 state( 0 );
    quint32 drvStatus = GetDriveType( (LPCWSTR)driver.constData() );
    if ( drvStatus == DRIVE_REMOVABLE  ) {
        state |= FmDriverInfo::EDriveRemovable;
    }
#ifdef _DEBUG_DISABLE_DRIVE_D_TEST_DRIVEHIDE_
	if ( driverName.contains( Drive_D, Qt::CaseInsensitive )  ) {
		state |= FmDriverInfo::EDriveNotPresent;
    }
#endif

#ifdef _DEBUG_LOCKED_DRIVE_Z
	if ( driverName.contains( Drive_Z, Qt::CaseInsensitive )  ) {
		state |= FmDriverInfo::EDriveLocked;
    }
#endif
	if( !(state&FmDriverInfo::EDriveNotPresent) && !(state&FmDriverInfo::EDriveLocked) &&
		!(state&FmDriverInfo::EDriveCorrupted) ) {
		state |= FmDriverInfo::EDriveAvailable;
	}
    return FmDriverInfo( size, freeSize, driverName, QString::fromWCharArray( &volumeName[0] ), state );
}

QString FmUtils::formatStorageSize( quint64 size )
{
	if ( size < 1000 ) {
		return QString::number( size ) + " B";
	} else if ( size < 1000 * 1000 ) {
		return QString::number( size / 1024.0, 'f', 2 ) + " KB";
	} else if ( size < 1000 * 1000 * 1000 ) {
		return QString::number( size / (1024.0 * 1024.0), 'f', 1 ) + " MB";
	} else {
	    return QString::number( size / ( 1024.0 * 1024.0 * 1024.0 ), 'f', 1 ) + " GB";	    
	}
}

/*
quint32 FmUtils::getDriverState( const QString &driverName )
{
    quint32 state( 0 );

    QString driver = driverName;
    
    driver.replace( '/', "\\" );
    if ( driver.right(1) != "\\" ) {
        driver.append( "\\" );
    }

    quint32 drvStatus = GetDriveType( (LPCWSTR)driver.constData() );

    if ( drvStatus == DRIVE_REMOVABLE  ) {
        state |= FmDriverInfo::EDriveRemovable;
    }

    return state;

}
*/
int FmUtils::removeDrivePwd( const QString &driverName,  const QString &Pwd )
{
    Q_UNUSED( driverName );
    Q_UNUSED( Pwd );
    return 0;
}

int FmUtils::unlockDrive( const QString &driverName,  const QString &Pwd )
{
    Q_UNUSED( driverName );
    Q_UNUSED( Pwd );
    return 0;
}

int FmUtils::checkDrivePwd( const QString &driverName, const QString &pwd)
{
    Q_UNUSED( driverName );
    Q_UNUSED( pwd );
    return 0;
}

int FmUtils::setDrivePwd( const QString &driverName, const QString &oldPwd, const QString &newPwd)
{
    Q_UNUSED( driverName );
    Q_UNUSED( oldPwd );
    Q_UNUSED( newPwd );
    return 0;
}

void FmUtils::emptyPwd( QString &pwd )
{
    Q_UNUSED( pwd );
}

int FmUtils::renameDrive( const QString &driverName, const QString &newVolumeName)
{
    Q_UNUSED( driverName );
    foreach( const QChar &ch, newVolumeName )
    {
        // If not alphadigit or space, return error
        if( !ch.isLetterOrNumber() && !ch.isSpace() )
        {
            return FmErrBadName;
        }   
    }
    return 0;
}

int FmUtils::ejectDrive( const QString &driverName )
{
    Q_UNUSED( driverName );
	return FmErrNone; 
}

QString FmUtils::getFileType( const QString &filePath  )
{
    Q_UNUSED( filePath );
    return QString( "" );
}

quint64 FmUtils::getDriveDetailsResult( const QString &folderPath, const QString &extension )
{
    Q_UNUSED( folderPath );
    Q_UNUSED( extension );
    return 0;
}

bool FmUtils::isDriveC( const QString &driverName )
{
	if( driverName.contains(Drive_C,Qt::CaseInsensitive) ){
        return true;
    }
    else{
        return false;
    }
}

bool FmUtils::isDrive( const QString &path )
{
   bool ret( false );
   if( path.length() <= 3 && path.length() >=2 ) {
       ret = true;
   }
   
   return ret;   
}

void FmUtils::createDefaultFolders( const QString &driverName )
{
    Q_UNUSED( driverName );
}

/*!
    fill splash in the end of \a filePath if the path is not a file
    All "/" and "\" will be changed to QDir::separator
    \sa formatPath only changed "/" and "\" to QDir::separator
*/
QString FmUtils::fillPathWithSplash( const QString &filePath )
{
    QString newFilePath;
    if( filePath.isEmpty() ) {
        return newFilePath;
    }

    newFilePath = formatPath( filePath );
    
    if( newFilePath.right( 1 )!= QDir::separator() ){
        newFilePath.append( QDir::separator() );
    }
    return newFilePath;
}

QString FmUtils::removePathSplash( const QString &filePath )
{
    QString newFilePath( filePath );
    if( filePath.right( 1 ) == QChar( '/' ) || filePath.right(1) == QString( "\\" ) ) {
        newFilePath = filePath.left( filePath.length() - 1 );
    }
    return newFilePath;
}

bool FmUtils::checkDriveAccessFilter( const QString &driveName )
{
#ifdef _DEBUG_HIDE_VIEWFOLDER_WINDOWS_
    if( driveName.contains( Drive_D, Qt::CaseInsensitive ) || driveName.contains( Drive_Z, Qt::CaseInsensitive ) ) {
        return false;
    }
#endif
    return true;
}

QString FmUtils::checkDriveToFolderFilter( const QString &path )
{
    QFileInfo fileInfo( path );
    if( !fileInfo.exists() ) {
            return QString();
        }

#ifdef _DEBUG_HIDE_VIEWFOLDER_WINDOWS_
    QString checkedPath = fillPathWithSplash( path );
    if( checkedPath.compare( QString( "C:/"), Qt::CaseInsensitive ) == 0 ) {
        checkedPath += QString( "data/" );
        QFileInfo fileInfo( checkedPath );
        if( !fileInfo.exists() ) {
            return QString();
        }
        return checkedPath;
    }
#endif
    return path;

}

QString FmUtils::checkFolderToDriveFilter( const QString &path )
{
#ifdef _DEBUG_HIDE_VIEWFOLDER_WINDOWS_
    QString logString;
    logString = QString( "checkFolderToDriveFilter: " ) + path;
    FM_LOG( logString );
    QString checkedPath = fillPathWithSplash( path );

    logString = QString( "checkFolderToDriveFilter_fillPathWithSplash: " ) + checkedPath;
    FM_LOG( logString );
    
    if( checkedPath.compare( QString( "C:/data/"), Qt::CaseInsensitive ) == 0 ) {
        FM_LOG( QString( " change from c:/data/ to C:/" ) );
        return QString( "C:/" );
    }
#endif
    return path;

}

int FmUtils::isPathAccessabel( const QString &path )
{
    if(!isDriveAvailable( path ) ) { //used to filter locked drive
        return FmErrDriveNotAvailable;
    }

    QFileInfo fileInfo( path );

#ifdef _DEBUG_HIDE_VIEWFOLDER_WINDOWS_
    if( fileInfo.absoluteFilePath().contains( QString( Drive_C ), Qt::CaseInsensitive ) &&
        !fileInfo.absoluteFilePath().contains( QString( Folder_C_Data ), Qt::CaseInsensitive ) ) {
        return false;
    }
    if( fileInfo.absoluteFilePath().contains( QString( Drive_D ), Qt::CaseInsensitive ) ) {
        return false;
    }
    if( fileInfo.absoluteFilePath().contains( QString( Drive_Z ), Qt::CaseInsensitive ) ) {
        return false;
    }
#endif
    if( !fileInfo.exists() ) {
        return FmErrPathNotExist;
    }
    return FmErrNone;
}

bool FmUtils::isDriveAvailable( const QString &path )
{
#ifdef _DEBUG_DISABLE_DRIVE_D_TEST_DRIVEHIDE_
	if(path.contains(Drive_D, Qt::CaseInsensitive))
		return false;
#endif
#ifdef _DEBUG_LOCKED_DRIVE_Z
	if(path.contains(Drive_Z, Qt::CaseInsensitive))
		return false;
#endif
	return true;
}

void FmUtils::getDriveList( QStringList &driveList, bool isHideUnAvailableDrive )
{
    QFileInfoList infoList = QDir::drives();

    foreach( QFileInfo fileInfo, infoList ) {
        QString driveName = fileInfo.absolutePath();
        if( checkDriveAccessFilter( driveName ) ) {
            if( !isHideUnAvailableDrive ) {
                driveList.append( driveName );
            }
            else if ( isDriveAvailable( driveName ) ) {
                driveList.append( driveName );
            }
        }
    }
    return;
}

QString FmUtils::fillDriveVolume( QString driveName, bool isFillWithDefaultVolume )
{
    QString ret;
    QString tempDriveName = fillPathWithSplash( driveName );

    QString checkedDriveName( removePathSplash( driveName ) );
    
    FmDriverInfo driverInfo = FmUtils::queryDriverInfo( tempDriveName );
    QString volumeName = driverInfo.volumeName();

    if( volumeName.isEmpty() && isFillWithDefaultVolume ){
    FmDriverInfo::DriveState driveState = queryDriverInfo( tempDriveName ).driveState();
        if( driveState & FmDriverInfo::EDriveAvailable ){
            if( driveState & FmDriverInfo::EDriveRemovable ) {
                if( driveState & FmDriverInfo::EDriveMassStorage ) {
                    // Mass Storage
                    ret = hbTrId( "txt_fmgr_dblist_1_mass_storage" ).arg( checkedDriveName );  
                } else if( driveState & FmDriverInfo::EDriveUsbMemory ) {
                    // USB Memory
                    ret = hbTrId( "txt_fmgr_dblist_1_usb_memory" ).arg( checkedDriveName );
                } else{
                    // Memory Card
                    ret = hbTrId( "txt_fmgr_dblist_1_memory_card" ).arg( checkedDriveName );
                }
            } else{
				// phone memory
				ret = hbTrId( "txt_fmgr_dblist_1_device_memory" ).arg( checkedDriveName );
            }
        }
    } else {
        ret = hbTrId( "txt_fmgr_dblist_1_2" ).arg( checkedDriveName ).arg( volumeName );
    }
    return ret;
}

int FmUtils::launchFile( const QString &filePath )
{
    if( QDesktopServices::openUrl( QUrl::fromLocalFile( filePath ) ) ) {
        return FmErrNone;
    } else {
        return FmErrGeneral;
    }
}

void FmUtils::sendFiles( QStringList &filePathList )
{

}

QString FmUtils::getBurConfigPath( QString appPath )
{
    QFileInfo fileInfo( appPath );
    QString testString = fileInfo.absolutePath();
    QString path = fillPathWithSplash( fileInfo.absolutePath() );
    path = path + QString( "src/filemanager/" );
    path = path + QString( BURCONFIGFILE );
    return path;
}

bool FmUtils::isPathEqual( const QString &pathFst, const QString &pathLast )
{
    QString fst( fillPathWithSplash( pathFst ) );
    QString last( fillPathWithSplash( pathLast ) );
    if( fst.compare( last, Qt::CaseInsensitive ) == 0 ) {
        return true;
    }
    return false;
}

bool FmUtils::isDefaultFolder( const QString &folderPath  )
{
    Q_UNUSED( folderPath );
    return false;
}

/*!
    All "/" and "\" in \a path will be changed to QDir::separator
    \sa fillPathWithSplash, fillPathWithSplash will append QDir::separator in the end if path is no a file
*/
QString FmUtils::formatPath( const QString &path  )
{
    QString formatPath;
    if( path.isEmpty() ) {
        return formatPath;
    }
    
    foreach( QChar ch, path ) {
        if( ch == QChar('\\') || ch == QChar('/') ) {
            formatPath.append( QDir::separator() );
        } else {
            formatPath.append( ch );
        }
    }

    return formatPath;
}

int FmUtils::getMaxFileNameLength()
{
	return KMaxFileName;
}

bool FmUtils::checkMaxPathLength( const QString& path )
{
	if( path.length() > KMaxPath ) {
		return false;
	}
	return true;
}

bool FmUtils::checkFolderFileName( const QString& name )
{
    // trim space firest, because there may be some spaces after "." ,  it is also not valid
    // or there may only have spaces in name
    QString trimmedName( name.trimmed() );
	if( trimmedName.isEmpty() ) {
		return false;
	}
    if( trimmedName.endsWith( QChar('.'),  Qt::CaseInsensitive ) ) {
        return false;
    }
    if( trimmedName.contains( QChar('\\'), Qt::CaseInsensitive ) ||
        trimmedName.contains( QChar('/'),  Qt::CaseInsensitive ) ||
        trimmedName.contains( QChar(':'),  Qt::CaseInsensitive ) ||
        trimmedName.contains( QChar('*'),  Qt::CaseInsensitive ) ||
        trimmedName.contains( QChar('?'),  Qt::CaseInsensitive ) ||
        trimmedName.contains( QChar('\"'), Qt::CaseInsensitive ) ||
        trimmedName.contains( QChar('<'),  Qt::CaseInsensitive ) ||
        trimmedName.contains( QChar('>'),  Qt::CaseInsensitive ) ||
        trimmedName.contains( QChar('|'),  Qt::CaseInsensitive ) ){
        return false;
    }
    // use orignal name to exam max size of file name
    if( name.length() > KMaxFileName ) {
        return false;
    }
    return true;
}

bool FmUtils::checkNewFolderOrFile( const QString &fileName, const QString &path, QString &errString )
{
    // first check if fileName is valid, then check if path length is valid, and check if file/foler is existed at last
    QFileInfo fileInfo( path );
    bool ret( true );   
    if (!FmUtils::checkFolderFileName( fileName ) ) {
        errString = hbTrId( "Invalid file or folder name, try again!" );
        ret = false;
    } else if( !FmUtils::checkMaxPathLength( path ) ) {
        errString = hbTrId( "the path you specified is too long, try again!" );
        ret = false;
    } else if (fileInfo.exists()) {
        errString = hbTrId( "%1 already exist!" ).arg( fileInfo.fileName() );
        ret = false;
    }
    return ret;
}

QString FmUtils::getVolumeNameWithDefaultNameIfNull( const QString &diskName, bool &defaultName )
{
    FmDriverInfo driverInfo = FmUtils::queryDriverInfo( diskName );

    // do not add default volume for win32 version as this is only the dummy implememnt for debug on windows
    return driverInfo.volumeName();
}

/*!
    Check if \a dest is sub level path of \a src
    Used to check True/False when copy a folder to itself or its subfolder
    For example, c:\data\test is sub path of c:\data.
    But c:\data123\test is not sub path of c:\data.
    So after got right part of path, the first char must be \ or /
*/
bool FmUtils::isSubLevelPath( const QString &src, const QString &dest )
{
    FM_LOG("FmUtils::isSubFolder: src=" + src + " dest=" + dest);
    QString checkedSrc( FmUtils::fillPathWithSplash( src ) );
    QString checkedDest( FmUtils::fillPathWithSplash( dest ) );
    
    if( checkedDest.contains( checkedSrc, Qt::CaseInsensitive) &&
            checkedDest.length() > checkedSrc.length() ) {
        // for example c:\data\ vs c:\data\123\ 
        FM_LOG("FmUtils::isSubFolder: true");
        return true;
    }
    // for example c:\data\ vs c:\data\ 
    // for example c:\data\ vs c:\data123\ 

    FM_LOG("FmUtils::isSubFolder: false");
    return false;
}

int FmUtils::setFileAttributes( const QString &srcFile, const QString &desFile )
{
    return FmErrNone;
}
