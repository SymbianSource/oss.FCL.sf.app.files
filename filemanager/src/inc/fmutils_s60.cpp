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
*     The source file of the file manager utilities
*/


#include "fmutils.h"
#include "fms60utils.h"
#include "fmcommon.h"

#include <QRegExp>

#include <coemain.h>
#include <driveinfo.h>
#include <e32property.h>
#include <coreapplicationuisdomainpskeys.h>
#include <f32file.h>
#include <apgcli.h>
#include <pathinfo.h>
#include <CDirectoryLocalizer.h>
#include <XQConversions>
#include <QStringList>
#include <QFileInfoList>
#include <QDir>
#include <QFile>

#include <xqaiwrequest.h>
#include <xqappmgr.h>

#include <shareuidialog.h>

#define BURCONFIGFILE  "z:/private/2002BCC0/burconfig.xml"

QString FmUtils::getDriveNameFromPath( const QString &path )
{
    if( path.length() <3 ) {
        return QString();
    }
    return path.left( 3 );
}

QString FmUtils::getDriveLetterFromPath( const QString &path )
{
	if( path.length() <2 ) {
        return QString();
    }
    return path.left( 1 );
}

FmDriverInfo FmUtils::queryDriverInfo( const QString &driverName )
{
    CCoeEnv *env = CCoeEnv::Static();
    RFs& fs = env->FsSession();

    TVolumeInfo volumeInfo;
    TInt drive = 0;
    drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;

    quint32 state( 0 );
    int err = fs.Volume( volumeInfo, drive );
    QString volumeName( (QChar*)( volumeInfo.iName.Des().Ptr() ), volumeInfo.iName.Length() );

    if( err == KErrNone ) {
        TDriveInfo driveInfo = volumeInfo.iDrive;
    
        quint32 drvStatus( 0 );
        err = DriveInfo::GetDriveStatus( fs, drive, drvStatus );
        if( err == KErrNone ) {
            QString logString = driverName +':'+ QString::number( drvStatus);
            FmLogger::log(logString);
            
            if ( ( drvStatus & DriveInfo::EDriveInternal ) &&
                 ( drvStatus & DriveInfo::EDriveExternallyMountable ) ){
                // Handle mass storage bits here
        
                state |= FmDriverInfo::EDriveMassStorage | FmDriverInfo::EDriveRemovable;
            }
        
            
            if ( drvStatus & DriveInfo::EDriveRom ){
                state |= FmDriverInfo::EDriveRom;  
            }
            
            if ( drvStatus & DriveInfo::EDriveRam ){
                state |= FmDriverInfo::EDriveRam;  
            }
        
            if ( driveInfo.iMediaAtt & KMediaAttFormattable ){
                state |= FmDriverInfo::EDriveFormattable;
            }
            if ( driveInfo.iMediaAtt & KMediaAttWriteProtected ){
                state |= FmDriverInfo::EDriveWriteProtected;
            }
            if ( driveInfo.iMediaAtt & KMediaAttHasPassword ){
                state |= FmDriverInfo::EDrivePasswordProtected;
            }    
            if ( driveInfo.iMediaAtt & KMediaAttLocked ){
                state |= FmDriverInfo::EDriveLocked;
            }
        
            if ( driveInfo.iDriveAtt & KDriveAttRemovable ){
                state |= FmDriverInfo::EDriveRemovable;
        
                if ( drvStatus & DriveInfo::EDriveSwEjectable ){
                    state |= FmDriverInfo::EDriveEjectable;
                }
            }
            
            if( driveInfo.iType == EMediaNotPresent ){
                state |= FmDriverInfo::EDriveNotPresent;    
            }
        }
    }
    //handle error code
    switch( err )
    {
    case KErrNone:
        state |= FmDriverInfo::EDriveAvailable;
        break;
    case KErrLocked:
        state |= FmDriverInfo::EDriveLocked;
        break;
    case KErrCorrupt:
        state |= FmDriverInfo::EDriveCorrupted;
        break;
    default: // other errors
        state |= FmDriverInfo::EDriveNotPresent;
        break;
    }
    return FmDriverInfo( volumeInfo.iSize, volumeInfo.iFree, driverName, volumeName, state );
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
    CCoeEnv *env = CCoeEnv::Static();
    RFs& fs = env->FsSession();

    TVolumeInfo volumeInfo;
    TInt drive = 0;
    drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;

    quint32 state( 0 );
    int err = fs.Volume( volumeInfo, drive );
    QString volumeName( (QChar*)( volumeInfo.iName.Des().Ptr() ), volumeInfo.iName.Length() );

    //handle error code
	if( err != KErrNone ) {
            state |= FmDriverInfo::EDriveNotPresent;
	}
	if ( err == KErrLocked ) {
            state |= FmDriverInfo::EDriveLocked;
	}
	
    TDriveInfo driveInfo = volumeInfo.iDrive;

    quint32 drvStatus( 0 );
    DriveInfo::GetDriveStatus( fs, drive, drvStatus );

    QString logString = driverName +':'+ QString::number( drvStatus);
    FmLogger::log(logString);
    
    if ( ( drvStatus & DriveInfo::EDriveInternal ) &&
         ( drvStatus & DriveInfo::EDriveExternallyMountable ) ){
        // Handle mass storage bits here

        state |= FmDriverInfo::EDriveMassStorage | FmDriverInfo::EDriveRemovable;
    }

    
    if ( drvStatus & DriveInfo::EDriveRom ){
        state |= FmDriverInfo::EDriveRom;  
    }
    
    if ( drvStatus & DriveInfo::EDriveRam ){
        state |= FmDriverInfo::EDriveRam;  
    }

    if ( driveInfo.iMediaAtt & KMediaAttFormattable ){
        state |= FmDriverInfo::EDriveFormattable;
    }
    if ( driveInfo.iMediaAtt & KMediaAttWriteProtected ){
        state |= FmDriverInfo::EDriveWriteProtected;
    }
    if ( driveInfo.iMediaAtt & KMediaAttHasPassword ){
        state |= FmDriverInfo::EDrivePasswordProtected;
    }    
    if ( driveInfo.iMediaAtt & KMediaAttLocked ){
        state |= FmDriverInfo::EDriveLocked;
    }

    if ( driveInfo.iDriveAtt & KDriveAttRemovable ){
        state |= FmDriverInfo::EDriveRemovable;

        if ( drvStatus & DriveInfo::EDriveSwEjectable ){
            state |= FmDriverInfo::EDriveEjectable;
        }
    }
    
    if( driveInfo.iType == EMediaNotPresent ){
        state |= FmDriverInfo::EDriveNotPresent;    
    }
    
    return state;

}
*/

int FmUtils::removeDrivePwd( const QString &driverName,  const QString &Pwd )
{
    QString logString = "Drive name:" + driverName;
    FmLogger::log( logString );
    logString = "Password:" + Pwd;
    FmLogger::log( logString );

    CCoeEnv *env = CCoeEnv::Static();
	RFs& fs = env->FsSession();

    TInt drive = 0;

	drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;

    HBufC* password16 = XQConversions::qStringToS60Desc( Pwd );
    TMediaPassword password;   
    TPtr ptrPassword16( password16->Des() );  
    FmS60Utils::ConvertCharsToPwd( ptrPassword16, password );

    int err( fs.ClearPassword( drive, password ) );

    logString = "Drive:" + QString::number( drive );
    FmLogger::log( logString );

    logString = "Clear password error:" + QString::number( err );
    FmLogger::log( logString );

    if( err == KErrNone ){
        return FmErrNone;   
    }
    else if( err == KErrAccessDenied ){
        return FmErrAccessDenied;
    }
    else{
        return FmErrUnKnown;
    }
}

int FmUtils::unlockDrive( const QString &driverName,  const QString &Pwd )
{
    QString logString = "Drive name:" + driverName;
    FmLogger::log( logString );
    logString = "Password:" + Pwd;
    FmLogger::log( logString );

    CCoeEnv *env = CCoeEnv::Static();
	RFs& fs = env->FsSession();

    TInt drive = 0;
	drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;
    
    HBufC* password16 = XQConversions::qStringToS60Desc( Pwd );
    TMediaPassword password;   
    TPtr ptrPassword16( password16->Des() );  
    FmS60Utils::ConvertCharsToPwd( ptrPassword16, password );

    int err( fs.UnlockDrive( drive, password, ETrue) );

    logString = "Drive:" + QString::number( drive );
    FmLogger::log( logString );
    logString = "Unlock drive error:" + QString::number( err );
    FmLogger::log( logString );

    if( err == KErrNone ){
        return FmErrNone;   
    }
    else if( err == KErrAccessDenied ){
        return FmErrAccessDenied;
    }
    else if( err == KErrAlreadyExists ){
        return FmErrAlreadyExists;
    }
    else if( err == KErrNotSupported ){
        return FmErrNotSupported;
    }
    else{
        return FmErrUnKnown;
    }
}

int FmUtils::checkDrivePwd( const QString &driverName, const QString &pwd )
{
    QString logString = "checkDrivePwd Drive name:" + driverName;
    logString += " password:" + pwd;
    FmLogger::log( logString );

    return setDrivePwd( driverName, pwd, pwd );
}

int FmUtils::setDrivePwd( const QString &driverName, const QString &oldPwd, const QString &newPwd)
{
    QString logString = "setDrivePwd Drive name:" + driverName ;
    logString += " Old password:" + oldPwd;
    logString += " New password:" + newPwd;
    FmLogger::log( logString );

    CCoeEnv *env = CCoeEnv::Static();
	RFs& fs = env->FsSession();

    TInt drive = 0;
	drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;
	
    HBufC* newPassword16 = XQConversions::qStringToS60Desc( newPwd);
    HBufC* oldPassword16 = XQConversions::qStringToS60Desc( oldPwd );

    TMediaPassword oldPassword;
    TMediaPassword newPassword;
    
    TPtr ptrNewPassword16( newPassword16->Des() );
    TPtr ptrOldPassword16( oldPassword16->Des() );
    
    FmS60Utils::ConvertCharsToPwd( ptrNewPassword16, newPassword );
    FmS60Utils::ConvertCharsToPwd( ptrOldPassword16, oldPassword );

    int err( fs.LockDrive( drive, oldPassword, newPassword, ETrue ) );

    logString = "Drive:" + QString::number( drive );
    FmLogger::log( logString );
    logString = "Password set error:" + QString::number( err );
    FmLogger::log( logString );

    if( err == KErrNone ){
        return FmErrNone;   
    }
    else if( err == KErrNotSupported ){
        return FmErrNotSupported;
    }
    else{
        return FmErrUnKnown;
    }
}

void FmUtils::emptyPwd( QString &pwd )
{
    TPtr des ( ( XQConversions::qStringToS60Desc( pwd ) )->Des() );
    des.FillZ( des.MaxLength() );
    des.Zero();
    pwd = XQConversions::s60DescToQString( des );
}

int FmUtils::renameDrive( const QString &driverName, const QString &newVolumeName)
{
    foreach( QChar ch, newVolumeName )
    {
        bool a = ch.isSpace();
        bool b = ch.isLetterOrNumber();
        // If not alphadigit or space, return error
        if( !ch.isLetterOrNumber() && !ch.isSpace() )
        {
            return FmErrBadName;
        }   
    }
        
    CCoeEnv *env = CCoeEnv::Static();
	RFs& fs = env->FsSession();

    TInt drive = 0;
	drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;

    TPtr newName ( ( XQConversions::qStringToS60Desc( newVolumeName ) )->Des() );

    int err( fs.SetVolumeLabel( newName, drive ));
    
    QString logString = "Rename error:" + QString::number( err );
    FmLogger::log( logString );

    if( err == KErrNone ){
        return FmErrNone;   
    }
    else if( err == KErrNotReady ){
        return FmErrNotReady;
    }
    else{
        return FmErrUnKnown;
    }
}

void FmUtils::ejectDrive( const QString &driverName )
{
    QString logString = "FmUtils::ejectDrive start";
    FmLogger::log( logString );

    TInt drive = 0;
	drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;

    const int KDriveShift = 16;

    // Let SysAp handle eject
    RProperty::Set(
        KPSUidCoreApplicationUIs,
        KCoreAppUIsMmcRemovedWithoutEject,
        ECoreAppUIsEjectCommandUsedToDrive | ( drive << KDriveShift )
        );
}

QString FmUtils::getFileType( const QString &filePath  )
{
    RApaLsSession apaSession;
    TDataType dataType;
    TUid appUid;
    
    TBuf<128> mimeTypeBuf;
        
    int err = apaSession.Connect();
    
    if ( err == KErrNone ){   
        err = apaSession.AppForDocument( XQConversions::qStringToS60Desc( filePath )->Des(), 
                                         appUid, dataType );
        
        if( err == KErrNone ){
            mimeTypeBuf.Copy(dataType.Des8());
        }  
    }
    
    apaSession.Close();
    return XQConversions::s60DescToQString( mimeTypeBuf );
}

quint64 FmUtils::getDriveDetailsResult( const QString &folderPath, const QString &extension )
{
    int err;
    
    RFs fs;
    err = fs.Connect();
    
    QString string( formatPath( folderPath ) );

    TPtrC desFolderPath( XQConversions::qStringToS60Desc( string )->Des() );
    TPtrC ptrExtension( XQConversions::qStringToS60Desc( extension )->Des() );
    
    CDir* results = 0;
    TParse parse;
    
    quint64 size = 0;
    
    const TInt pathlength = ptrExtension.Length() + desFolderPath.Length();
    
    if ( pathlength > KMaxFileName ){
        err = KErrNotFound;   
    }
    else{
        err = fs.Parse( ptrExtension, desFolderPath, parse );
        err = fs.GetDir( parse.FullName(), KEntryAttMaskSupported|KEntryAttAllowUid, 
            ESortNone, results );
        
        TDesC des = parse.FullName();
        
        if (err == KErrNotFound)
            {
            return 0;
            }
    }
    
    if ( results ){
        CleanupStack::PushL(results);

        // Go through all files in the list and tell subclass
        TFileName file;
        const TInt count = results->Count();
        for( TInt i=0; i<count; ++i ){
            const TEntry& entry = (*results)[i];
            file = desFolderPath;
            file += entry.iName;
            size += entry.iSize;          
        }
        CleanupStack::PopAndDestroy(results);
    }
    
    fs.Close();
    
    return size;  
}

bool FmUtils::isDriveC( const QString &driverName )
{
    TInt drive = 0;
    drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;
    if( drive == EDriveC ){
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
    int err;
    
    TInt drive = 0;
    drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;
    
    RFs fs;
    err = fs.Connect();
    
    if( err != KErrNone ){
        return;
    }
    
    quint32 drvStatus( 0 );
    err = DriveInfo::GetDriveStatus( fs, drive, drvStatus );
    if ( !( drvStatus & DriveInfo::EDriveUserVisible ) ||
        ( drvStatus & ( DriveInfo::EDriveRemote |
                        DriveInfo::EDriveReadOnly |
                        DriveInfo::EDriveUsbMemory ) ) ){
            return; 
        }
    
    TEntry entry;
    CDesCArray* array = PathInfo::GetListOfPathsLC( drive );
    
    TInt count( array->MdcaCount() );
    for ( TInt i( 0 ); i < count; ++i )
        {
        TPtrC fullPath( array->MdcaPoint( i ) );
        TBool allow( ETrue );

        if ( drvStatus & DriveInfo::EDriveRemovable )
            {
            // Filter few folder types from physically removable memory cards
            TInt pathType( PathInfo::PathType( fullPath ) );
            switch( pathType )
                {
                case PathInfo::EGamesPath: // FALL THROUGH
                case PathInfo::EInstallsPath: // FALL THROUGH
                case PathInfo::EGsmPicturesPath: // FALL THROUGH
                case PathInfo::EMmsBackgroundImagesPath: // FALL THROUGH
                case PathInfo::EPresenceLogosPath:
                    {
                    allow = EFalse;
                    }
                default:
                    {
                    break;
                    }
                }
            }

        if ( allow ){
            fs.MkDirAll( fullPath ); // Ignore error

            if ( fs.Entry( fullPath, entry ) == KErrNone ){
                if( entry.IsHidden() ){
                // If setting fails, File Manager can still go on
                    fs.SetEntry(
                         fullPath, entry.iModified, 0, KEntryAttHidden );
                }
            }
        }
    }
    CleanupStack::PopAndDestroy( array );
}

QString FmUtils::fillPathWithSplash( const QString &filePath )
{
    QString newFilePath( filePath );
    if( filePath.isEmpty() ) {
        return newFilePath;
    }

    if( filePath.at( filePath.length()-1 ) != QChar( '/' ) ){
        newFilePath.append( QChar( '/' ) );
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

// used to filter drive which need be hide.
bool FmUtils::checkDriveFilter( const QString &driveName )
{
    if( driveName.contains( "D:" ) || driveName.contains( "Z:" ) ) {
        return false;
    }
    return true;
}

QString FmUtils::checkDriveToFolderFilter( const QString &path )
{
    /*
    QFileInfo fileInfo( path );
    if( !fileInfo.exists() ) {
            return QString();
        }
    */
    QString checkedPath = fillPathWithSplash( path );
    if( checkedPath.compare( QString( "C:/"), Qt::CaseInsensitive ) == 0 ) {
        checkedPath += QString( "data/" );
        return checkedPath;
    }
    return path;

}

QString FmUtils::checkFolderToDriveFilter( const QString &path )
{
    QString logString;
    logString = QString( "checkFolderToDriveFilter: " ) + path;
    FmLogger::log( logString );
    QString checkedPath = fillPathWithSplash( path );

    logString = QString( "checkFolderToDriveFilter_fillPathWithSplash: " ) + checkedPath;
    FmLogger::log( logString );
    
    if( checkedPath.compare( QString( "C:/data/"), Qt::CaseInsensitive ) == 0 ) {
        FmLogger::log( QString( " change from c:/data/ to C:/" ) );
        return QString( "C:/" );
    }
    return path;

}

bool FmUtils::isPathAccessabel( const QString &path )
{
    FmLogger::log( QString( "isPathAccessabel:" ) + path );
    if( path.length() <= 3 && !isDriveAvailable( path ) ) { //used to filter locked drive
        FmLogger::log( QString( "isPathAccessabel false: path is drive and not available" ) );
        return false;
    }
    QFileInfo fileInfo( path );
    if( fileInfo.absoluteFilePath().contains( QString( Drive_C ), Qt::CaseInsensitive ) &&
        !fileInfo.absoluteFilePath().contains( QString( Folder_C_Data ), Qt::CaseInsensitive ) ) {
        FmLogger::log( QString( "isPathAccessabel false: path contain C and not in data folder" ) );
        return false;
    }
    if( fileInfo.absoluteFilePath().contains( QString( Drive_D ), Qt::CaseInsensitive ) ) {
        FmLogger::log( QString( "isPathAccessabel false: path contain D" ) );
        return false;
    }
    if( fileInfo.absoluteFilePath().contains( QString( Drive_Z ), Qt::CaseInsensitive ) ) {
        FmLogger::log( QString( "isPathAccessabel false: path contain Z" ) );
        return false;
    }
    if( !fileInfo.exists() ) {
        FmLogger::log( QString( "isPathAccessabel false: path not exist" ) );
        return false;
    }
    FmLogger::log( QString( "isPathAccessabel true" ) );
    return true;
}

// only used to check drive, when MMC is not inserted, also return false
bool FmUtils::isDriveAvailable( const QString &path )
{
    FmLogger::log( QString( "isDriveAvailable:" ) + path );
    FmDriverInfo::DriveState driveState = queryDriverInfo( path ).driveState();
    if( ( driveState & FmDriverInfo::EDriveAvailable ) ) {
        FmLogger::log( QString( "isDriveAvailable true" ) );
        return true;
    }
    FmLogger::log( QString( "isDriveAvailable false" ) );
    return false;
}

void FmUtils::getDriveList( QStringList &driveList, bool isHideUnAvailableDrive )
{
    if( isHideUnAvailableDrive ) {
        FmLogger::log( QString( "getDriveList HideUnAvailableDrive_true" ) );
    } else {
        FmLogger::log( QString( "getDriveList HideUnAvailableDrive_false" ) );
    }
    QFileInfoList infoList = QDir::drives();

    foreach( QFileInfo fileInfo, infoList ) {
        QString driveName = fileInfo.absolutePath();
        if( checkDriveFilter( driveName ) ) {
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

    ret = removePathSplash( driveName );
    
    FmDriverInfo driverInfo = FmUtils::queryDriverInfo( tempDriveName );
    QString volumeName = driverInfo.volumeName();

    if( volumeName.isEmpty() && isFillWithDefaultVolume ){
    FmDriverInfo::DriveState driveState = queryDriverInfo( tempDriveName ).driveState();
        if( driveState & FmDriverInfo::EDriveAvailable ){
            if( driveState & FmDriverInfo::EDriveRemovable ) {
                if( driveState & FmDriverInfo::EDriveMassStorage ) {
                    volumeName.append( QObject::tr( "Mass Storage" ) );  
                }
                else{
                    volumeName.append( QObject::tr( "Memory Card" ) );
                }
            }
            else{
                volumeName.append( QObject::tr( "Phone Memory" ) );
            }
        }
    }

    ret += QString( " " ) + volumeName;
    return ret;
}

int FmUtils::launchFile( const QString &filePath )
{
    QFile file( filePath );
    XQApplicationManager mAiwMgr;
    XQAiwRequest *request = mAiwMgr.create(file);
    if ( request == 0 ) {
        // No handlers for the URI
        return FmErrUnKnown;
    }
    
    // Set function parameters
    QList<QVariant> args;
    args << file.fileName();
    request->setArguments(args);
    
    // Send the request
    bool res = request->send();
    if  (!res) 
    {
       // Request failed. 
      int error = request->lastError();
      
      delete request;
      return FmErrUnKnown;
    }
    
    delete request;
    return FmErrNone;
}

void FmUtils::sendFiles( QStringList &filePathList )
{
    ShareUi shareui;
    shareui.send( filePathList, false );
}

QString FmUtils::getBurConfigPath( QString appPath )
{
    Q_UNUSED( appPath );
    QString path( BURCONFIGFILE );
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
    TPtrC desFolderPath( XQConversions::qStringToS60Desc( folderPath )->Des() );
    
    TInt pathType( PathInfo::PathType( desFolderPath ) );
    switch( pathType ){
       case PathInfo::ENotSystemPath:{
           QString locString( Localize( folderPath ) );
            if ( locString.isEmpty() ){
                return false;
            }
            return true;
            }
        case PathInfo::EPhoneMemoryRootPath: // FALL THROUGH
        case PathInfo::EMemoryCardRootPath: // FALL THROUGH
        case PathInfo::ERomRootPath:{
            return false;
        }
        // Accept other folders
        default:{
            return true;
        }
    }
}

QString FmUtils::Localize( const QString &path )
{
    QString locPath = formatPath( path );

    TPtrC desPath( XQConversions::qStringToS60Desc( locPath )->Des() );
    CDirectoryLocalizer *localizer = CDirectoryLocalizer::NewL();

    localizer->SetFullPath( desPath );
    if( localizer->IsLocalized() ){   
        return XQConversions::s60DescToQString( localizer->LocalizedName() );
    }
    
    return QString();
}

QString FmUtils::formatPath( const QString &path  )
{
    QString formatPath = path;
    QRegExp regExp( "/" );
    formatPath.replace( regExp, "\\" );
    
    if( path.right( 1 )!= "\\"){
        formatPath.append( "\\" );
    }
    return formatPath;
}
