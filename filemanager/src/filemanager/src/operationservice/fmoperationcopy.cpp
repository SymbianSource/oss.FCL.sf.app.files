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
* Description: 
*
*/

#include "fmoperationcopy.h"
#include "fmcommon.h"
#include "fmoperationbase.h"
#include "fmdrivedetailstype.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QStack>


FmOperationCopy::FmOperationCopy( QObject *parent, QStringList sourceList, QString targetPath ) :
        FmOperationBase( parent, FmOperationService::EOperationTypeCopy ), 
        mSourceList( sourceList ), mTargetPath( targetPath ),
        mStop( 0 ), mTotalSize( 0 ), mErrString( 0 ), mCopiedSize( 0 ), mTotalSteps( 100 ), mCurrentStep( 0 )
{
}

FmOperationCopy::~FmOperationCopy()
{
}

QStringList FmOperationCopy::sourceList()
{
    return mSourceList;
}

QString FmOperationCopy::targetPath()
{
    return mTargetPath;
}

int FmOperationCopy::start( volatile bool *isStopped, QString *errString )
{
    mStop = isStopped;
    mErrString = errString;
    mTotalSize   = 0;
    mCopiedSize  = 0;
    mCurrentStep = 0;

    if( mSourceList.empty() ) {
        return FmErrWrongParam;
    }

    emit notifyPreparing( true );

    int numofFolders = 0;
    int numofFiles      = 0;

    int ret = FmFolderDetails::queryDetailOfContentList( mSourceList, numofFolders, 
        numofFiles, mTotalSize, mStop, true );
    if( ret != FmErrNone ) {
        return ret;
    }

    emit notifyStart( true, mTotalSteps );

    foreach( const QString& source, mSourceList ) {
        QFileInfo fi( source );
        if( !fi.exists() ) {
            *mErrString = source;
            ret = FmErrSrcPathDoNotExist;
            return ret;
        }
        QString newName;
        bool    isAcceptReplace = false;
        QFileInfo destFi( mTargetPath + fi.fileName() );

        // while for duplicated file/dir
        while( destFi.exists() ) {
            if( destFi.isFile() && destFi.absoluteFilePath().compare( fi.absoluteFilePath(), Qt::CaseInsensitive ) != 0 ) {
                emit askForReplace( destFi.absoluteFilePath(), fi.absoluteFilePath(), &isAcceptReplace );
                if( isAcceptReplace ) {
                    //delete src file
                    if( !QFile::remove( destFi.absoluteFilePath() ) ) {
                        *mErrString = destFi.absoluteFilePath();
                        ret = FmErrCannotRemove;
                        break;
                    }
                    destFi.setFile( destFi.absoluteFilePath() );
                } else {
                    emit askForRename( destFi.absoluteFilePath(), &newName );
                    if( newName.isEmpty() ) {
                        ret = FmErrCancel;
                        break;
                    }
                    QString targetName = mTargetPath + newName;
                    destFi.setFile( targetName );
                }
            } else{
                // destination is dir
                emit askForRename( destFi.absoluteFilePath(), &newName );
                if( newName.isEmpty() ) {
                    ret = FmErrCancel;
                    break;
                }
                QString targetName = mTargetPath + newName;
                destFi.setFile( targetName );
            }
        }
        if( ret != FmErrNone ) {
            return ret;
        }
        ret = copy( source, mTargetPath, newName );
        if( ret != FmErrNone ) {
            return ret;
        }
    }
    return FmErrNone;
}



int FmOperationCopy::copy( const QString &source, const QString &targetPath,
                          const QString &newTargetName )
{        
    if( *mStop ) {
        return FmErrCancel;
    }

    QFileInfo fi( source );
    if( !fi.exists() ) {
        *mErrString = source;
        return FmErrSrcPathDoNotExist;
    }
    QString newName;
    if( !newTargetName.isEmpty() ) {
        newName = targetPath + newTargetName;
    } else {
        newName = targetPath + fi.fileName();
    }

    int ret = FmErrNone;
    
    if (fi.isFile()) {
        quint64 fileSize = fi.size();
        if ( !QFile::copy( source, newName )) {
            *mErrString = source;
            ret = FmErrCannotCopy;
        } else {
        IncreaseProgress( fileSize );
        }
    } else if (fi.isDir()) {
        ret = copyDirInsideContent( source, newName );
    } else {
        qWarning( "Things other than file and directory are not copied" );
        ret = FmErrIsNotFileOrFolder;
    }

    return ret;
}



int FmOperationCopy::copyDirInsideContent( const QString &srcPath, const QString &destPath )
{
    if( destPath.contains( srcPath, Qt::CaseInsensitive ) ) {
        *mErrString = destPath;
        return FmErrCopyDestToSubFolderInSrc;
    }

    QFileInfo srcInfo( srcPath );
    if( !srcInfo.isDir() || !srcInfo.exists() ) {
        *mErrString = srcPath;
        return FmErrSrcPathDoNotExist;
    }

    QFileInfo destInfo( destPath );
    if( !destInfo.exists() ) {
        if( !destInfo.dir().mkdir( destInfo.absoluteFilePath() ) ) {
            *mErrString = destPath;
            return FmErrCannotMakeDir;
        }
    }    

    if( !srcInfo.isDir() ) {
        *mErrString = destPath;
        return FmErrCannotMakeDir;
    }

    //start to copy
    QFileInfoList infoList = QDir( srcPath ).entryInfoList( QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden | QDir::System );
    while( !infoList.isEmpty() ) {
        if( *mStop ) {
            return FmErrCancel;
        }

        QFileInfo fileInfo = infoList.takeFirst();
        if( fileInfo.isFile() ){
            //copy file
            QString newFilePath = destPath + fileInfo.absoluteFilePath().mid( srcPath.length() );
            if (!QFile::copy( fileInfo.absoluteFilePath(), newFilePath ) ) {
                *mErrString = fileInfo.absoluteFilePath();
                return FmErrCannotCopy;
            }
            IncreaseProgress( fileInfo.size() );
        } else if( fileInfo.isDir() ) {
            //makedir
            QString newDirPath = destPath + fileInfo.absoluteFilePath().mid( srcPath.length() );
            if( !QDir( newDirPath ).exists() && !QDir( destPath ).mkdir( newDirPath ) ) {
                *mErrString = newDirPath;
                return FmErrCannotMakeDir;
            }
            // add dir content to list.
            QFileInfoList infoListDir = QDir( fileInfo.absoluteFilePath() ).entryInfoList(
                QDir::NoDotAndDotDot | QDir::AllEntries );
            while( !infoListDir.isEmpty() ) {
                infoList.push_front( infoListDir.takeLast() );
            }

        } else {
            *mErrString = fileInfo.absoluteFilePath();
            return FmErrIsNotFileOrFolder;
        }

    }

    return FmErrNone;
}

void FmOperationCopy::IncreaseProgress( quint64 size )
{
    if( mTotalSize <=0 ) {
        return;
    }
    mCopiedSize += size;
    int step = ( mCopiedSize * 100 ) / mTotalSize;
    if( step > mCurrentStep ) {
        mCurrentStep = step;
        emit notifyProgress( mCurrentStep );
    }
}
