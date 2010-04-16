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

#include "fmoperationremove.h"
#include "fmcommon.h"
#include "fmoperationbase.h"
#include "fmdrivedetailstype.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QStack>

FmOperationRemove::FmOperationRemove( QObject *parent, QStringList pathList ) :
        FmOperationBase( parent, FmOperationService::EOperationTypeRemove ),
        mPathList( pathList ), mStop( 0 ), mTotalCount( 0 ),
        mErrString( 0 ), mRemovedCount( 0 ), mTotalSteps( 100 ), mCurrentStep( 0 )
{
}

FmOperationRemove::~FmOperationRemove()
{
}

QStringList FmOperationRemove::pathList()
{
    return mPathList;
}

int FmOperationRemove::start( volatile bool *isStopped, QString *errString )
{
    mStop = isStopped;
    mErrString = errString;

    mTotalCount   = 0;
    mRemovedCount  = 0;
    mCurrentStep = 0;

    if( mPathList.empty() ) {
        return FmErrWrongParam;
    }

    emit notifyPreparing( true );

    quint64 totalSize= 0; 
    int numofFolders = 0;
    int numofFiles      = 0;

    int ret = FmFolderDetails::queryDetailOfContentList( mPathList, numofFolders, 
        numofFiles, totalSize, mStop, true );
    if( ret != FmErrNone ) {
        return ret;
    }
    mTotalCount = numofFolders + numofFiles;

    emit notifyStart( true, mTotalSteps );

    foreach( const QString& srcPath, mPathList ) {
        int ret = remove( srcPath );
        if( ret != FmErrNone ) {
            return ret;
        }
    }
    return FmErrNone;
}

int FmOperationRemove::remove( const QString &fileName )
{
    if( *mStop ) {
        return FmErrCancel;
    }

    int ret = FmErrNone;
    QFileInfo fi( fileName );
    if (fi.isFile()) {
        if( !QFile::remove( fileName ) ) {
            *mErrString = fileName;
            ret = FmErrCannotRemove;
        }
        IncreaseProgressOnce();
    } else if (fi.isDir()) {
       ret = recursiveRemoveDir( fileName );
    } else {
        qWarning( "Things other than file and directory are not copied" );
        ret = FmErrIsNotFileOrFolder;
    }
    return ret;
}

int FmOperationRemove::recursiveRemoveDir( const QString &pathName )
{
    QFileInfo fi( pathName );
    if (!fi.exists() || !fi.isDir())
        return FmErrSrcPathDoNotExist;

    QStack<QDir> dirs;
    dirs.push( QDir( pathName ) );

    while (!dirs.isEmpty()) {
        QFileInfoList infoList = dirs.top().entryInfoList( QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden | QDir::System );
        if (infoList.size() == 0) {
            QDir dirToRemove( dirs.pop() );
            if ( !dirToRemove.rmdir( dirToRemove.absolutePath() ) ) {
                *mErrString = dirToRemove.absolutePath();
                return FmErrCannotRemove;
            }
            IncreaseProgressOnce();

        } else {
            QList<QDir> dirList;
            for (QFileInfoList::Iterator it = infoList.begin(); it != infoList.end(); ++it) {
                if( *mStop ) {
                    return FmErrCancel;
                }

                if (it->isDir()) {
                    dirList.push_front( QDir( it->absoluteFilePath() ) );
                } else {
                    if ( !QFile::remove( it->absoluteFilePath() ) ) {
                        *mErrString = it->absoluteFilePath();
                        return FmErrCannotRemove;
                    }
                    IncreaseProgressOnce();
                }
            }
            foreach( const QDir& dir, dirList ) {
                dirs.push( dir );
            }
        }
    }
    return FmErrNone;
}

void FmOperationRemove::IncreaseProgressOnce()
{
    mRemovedCount++;
    int step = ( mRemovedCount * 100 ) / mTotalCount;
    if( step > mCurrentStep ) {
        mCurrentStep = step;
        emit notifyProgress( mCurrentStep );
    }
}
