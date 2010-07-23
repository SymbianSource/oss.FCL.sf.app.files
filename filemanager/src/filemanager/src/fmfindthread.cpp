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
 *     The find thread header file of file manager
 */

#include "fmfindthread.h"

#include <QDir>

// current path, it may come from findDirs.first().entryInfoList()
#define CurrentDir QString( "." )

// parent path, it may come from findDirs.first().entryInfoList()
#define ParentDir QString( ".." )

// if got 5 result and have not send notify event, then send notify event
#define notifyPerCount 5

// if got notifyPerElapsedTime milliseconds and have not send notify event, then send notify event.
#define notifyPerElapsedTime 500

/*!
    \fn void found( const QStringList &dataList )
    This signal is emitted when some data has been found and \a dataList is provided as data list.
    Please connect this signal by Qt::BlockingQueuedConnection as dataList will be cleared immediately
*/

FmFindThread::FmFindThread( QObject *parent )
    : QThread( parent )
{
    setPriority( LowPriority );
}

FmFindThread::~FmFindThread()
{
}

QString FmFindThread::findPath() const
{
    return mFindPath;
}

void FmFindThread::setFindPath( const QString &path )
{
    mFindPath = path;
}

QRegExp FmFindThread::pattern() const
{
    return findPattern;
}

void FmFindThread::setPattern( const QRegExp &regExp )
{
    findPattern = regExp;
}

void FmFindThread::stop()
{
    mStop = true;
}

void FmFindThread::run()
{
    mStop = false;
    tempResultList.clear();
    if (findPattern.isEmpty() || !findPattern.isValid())
        return;

    QDir dir( mFindPath );
    if (!dir.exists())
        return;
    
    if( mFindPath.isEmpty() ){
        findInResult();
        return;
    }

    QList<QDir> findDirs;
    findDirs.append( dir );
    time.restart();
    mStop = false;
    while (!findDirs.isEmpty()) {
        QFileInfoList infoList = findDirs.first().entryInfoList();
        for (QFileInfoList::Iterator it = infoList.begin(); it != infoList.end(); ++it) {
			QString name = it->fileName();
			QString absolutPath = it->absoluteFilePath();
            if (findPattern.exactMatch( it->fileName() )) {
                tempResultList.append( it->absoluteFilePath() );
                if (tempResultList.count() > notifyPerCount) {
                    emitFound();
                } else if (time.elapsed() > notifyPerElapsedTime && tempResultList.count() > 0) {
                    emitFound();
                }
            }

            //We are stopped;
            if (mStop) {
                if( tempResultList.count() > 0 ) {
                    emitFound();
                }
                return;
            }

            // exclude directory named ".." and "."
            if (it->isDir() && it->fileName() != ParentDir && it->fileName() != CurrentDir ) {
                findDirs.append( QDir( it->absoluteFilePath() ) );
            }
        }

        findDirs.removeFirst();
    }
    
    emitFound();
}

/*
    Emit signal "found" to send out found data
*/
void FmFindThread::emitFound()
{
    if( tempResultList.count() > 0 ) {
        emit found( tempResultList );
        tempResultList.clear();
        time.restart();
    }
}

void FmFindThread::setLastResult( QStringList r )
{
    mLastResult = r;
}

/*
    Find keyword in last result
    \sa setLastResult, this function must be called to set last result for findInResult
*/
void FmFindThread::findInResult()
{
    if( mFindPath.isEmpty() ){
        for (QStringList::Iterator it = mLastResult.begin(); it != mLastResult.end(); ++it) { 
            if (mStop){
                return;
            }
            QString absolutPath = (*it);
            QFileInfo fileInfo( absolutPath );
            QString fileName = fileInfo.fileName();
            
            if (findPattern.exactMatch( fileName ) ) {
                tempResultList.append( absolutPath );
                if ( tempResultList.count() > notifyPerCount ) {
                    emitFound();
                } else if (time.elapsed() > notifyPerElapsedTime && tempResultList.count() > 0) {
                    emitFound();
                }
            }
        }    
    }
    emitFound();
}
