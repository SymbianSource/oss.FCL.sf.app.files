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

FmDriverInfo FmUtils::queryDriverInfo( const QString &driverName )
{
	CCoeEnv *env = CCoeEnv::Static();
	RFs& fs = env->FsSession();

	TVolumeInfo volumeInfo;
	TInt drive = 0;
	drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;

	fs.Volume( volumeInfo, drive );
    QString volumeName( (QChar*)( volumeInfo.iName.Des().Ptr() ), volumeInfo.iName.Length() );
	
	return FmDriverInfo( volumeInfo.iSize, volumeInfo.iFree, driverName, volumeName );
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
quint32 FmUtils::getDriverState( const QString &driverName )
{    
    quint32 state( 0 );
    CCoeEnv *env = CCoeEnv::Static();
	RFs& fs = env->FsSession();

	TVolumeInfo volumeInfo;
	TInt drive = 0;
	drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;

	int err = fs.Volume( volumeInfo, drive );

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

    return err;
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

    return err;
}

int FmUtils::setDrivePwd( const QString &driverName, const QString &oldPwd, const QString &newPwd)
{
    QString logString = "Drive name:" + driverName;
    FmLogger::log( logString );
    logString = "Old password:" + oldPwd;
    FmLogger::log( logString );
    logString = "New password:" + newPwd;
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

    //oldPassword.FillZ(oldPassword.MaxLength());
    //oldPassword.Zero();
    //newPassword.Copy(_L8("A"));

    int err( fs.LockDrive( drive, oldPassword, newPassword, ETrue ) );

    logString = "Drive:" + QString::number( drive );
    FmLogger::log( logString );
    logString = "Password set error:" + QString::number( err );
    FmLogger::log( logString );

    return err;
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
    CCoeEnv *env = CCoeEnv::Static();
	RFs& fs = env->FsSession();

    TInt drive = 0;
	drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;

    TPtr newName ( ( XQConversions::qStringToS60Desc( newVolumeName ) )->Des() );

    int err( fs.SetVolumeLabel( newName, drive ));
    
    QString logString = "Rename error:" + QString::number( err );
    FmLogger::log( logString );

    return err;
}

int FmUtils::ejectDrive( const QString &driverName )
{
    CCoeEnv *env = CCoeEnv::Static();
    RFs& fs = env->FsSession();

    TInt drive = 0;
	drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;

    const int KDriveShift = 16;
    
    TFileName fileSystemName;
    TInt err = fs.FileSystemName( fileSystemName, drive );

    RProperty::Set(
        KPSUidCoreApplicationUIs,
        KCoreAppUIsMmcRemovedWithoutEject,
        ECoreAppUIsEjectCommandUsedToDrive | ( drive << KDriveShift )
        );
    
    err = fs.DismountFileSystem( fileSystemName, drive );
    
    if (err == KErrNone){
        // remount the file system
        err = fs.MountFileSystem( fileSystemName, drive );
        
        if (err == KErrInUse){
            // try to remount after a while if locked
            User::After(1000000);
            err = fs.MountFileSystem( fileSystemName, drive );
        }                
    }
    
    return err;
}

int FmUtils::formatDrive( const QString &driverName )
{
    RFormat format;

//    CCoeEnv *env = CCoeEnv::Static();
//	RFs& fs = env->FsSession();
    
    RFs fs;
    int err = fs.Connect();
    
    if( err != KErrNone ){
        return FmErrTypeFormatFailed;
    }

    TInt drive = 0;
	drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;

    TDriveName formatDriveName( TDriveUnit( drive ).Name() );

    int finalValue = 0;
    
    format.Close();

    err = format.Open(
        fs, formatDriveName, EFullFormat | ESpecialFormat, finalValue );
    
    QString logString = "Format open error:" + QString::number( err );
    FmLogger::log( logString );
    
    if( err == KErrLocked ){
        err = fs.ErasePassword( drive );
        
        logString = "ErasePassword error:" + QString::number( err );
        FmLogger::log( logString );
        
        if( err == KErrNone ){
            err = format.Open(
               fs, formatDriveName, EFullFormat, finalValue ); 
            
            logString = "Second format open error:" + QString::number( err );
            FmLogger::log( logString );
        }
    }
    
    if (err == KErrInUse){
        TBool reallyFormat = ETrue;
        if (reallyFormat)
            {
            err = format.Open(
                    fs, formatDriveName, EFullFormat | EForceFormat, finalValue );
            
            logString = "Reallyformat open error:" + QString::number( err );
            FmLogger::log( logString );
            }
    }
    
    TFullName fsName;
    if ( err == KErrNone )
        {
        err = fs.FileSystemName( fsName, drive  );
        
        logString = "FileSystemName error:" + QString::number( err );
        FmLogger::log( logString );

        if ( err == KErrNone && fsName.Length() > 0 )
            {
            // Prevent SysAp shutting down applications
            RProperty::Set(
                    KPSUidCoreApplicationUIs,
                    KCoreAppUIsMmcRemovedWithoutEject,
                    ECoreAppUIsEjectCommandUsed );
            
            logString = "Prevent SysAp shutting down applications" ;
            FmLogger::log( logString );
            }
        else
            {
            // Don't continue with format if there is no file system name
            // or file system name could not be obtained.
            err = KErrCancel;
            
            logString = "Format cancel:" + QString::number( err );
            FmLogger::log( logString );
            }
        }
    
    if( err == KErrNone &&  finalValue ){
    
        while ( finalValue ){        
            logString = "Format tracks:" + QString::number( finalValue );
            FmLogger::log( logString );
            
            err = format.Next( finalValue );
            
            if( err != KErrNone ){ 
                logString = "Format error:" + QString::number( err );
                FmLogger::log( logString );
                break; 
            }
        }
    }
    
    if( !finalValue || err != KErrNone ){
        format.Close();        
        fs.Close();
        
        createDefaultFolders( driverName );
    }

    return FmErrNone;
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
//    CCoeEnv *env = CCoeEnv::Static();
//    RFs& fs = env->FsSession();
    
    RFs fs;
    err = fs.Connect();
    
    QString string( folderPath );
    
    QRegExp regExp( "/" );
    string.replace( regExp, "\\" );
    
    if( string.right( 1 )!= "\\"){
        string.append( "\\" );
    }

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

bool FmUtils::checkDriveFilter( const QString &driveName )
{
    if( driveName.contains( "D:" ) || driveName.contains( "Z:" ) ) {
        return false;
    }
    return true;
}

QString FmUtils::checkDriveToFolderFilter( const QString &path )
{
    QFileInfo fileInfo( path );
    if( !fileInfo.exists() ) {
            return QString();
        }

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
    QFileInfo fileInfo( path );
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
    if( !fileInfo.exists() ) {
        return false;
    }
    return true;
}

bool FmUtils::isDriveAvailable( const QString &path )
{
    QFileInfo fileInfo( path );
    if( !fileInfo.exists() ) {
        return false;
    }
    return true;
}

void FmUtils::getDriveList( QStringList &driveList, bool isHideUnAvailableDrive )
{
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
        quint32 driveState = FmUtils::getDriverState( tempDriveName );
        if( !( driveState & FmDriverInfo::EDriveNotPresent ) ){
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

void FmUtils::sendFiles( QList<QVariant> filePathList )
{
    ShareUi shareui;
    shareui.init( filePathList, false );
}

QString FmUtils::getBurConfigPath( QString appPath )
{
    Q_UNUSED( appPath );
    QString path( BURCONFIGFILE );
    return path;
}
