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

FmFindThread::FmFindThread( QStringList *r, QObject *parent )
    : QThread( parent )
{
    mResult = r;
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
    setPriority( LowPriority );
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
    count = 0;
    time.restart();
    mStop = false;
    while (!findDirs.isEmpty()) {
        QFileInfoList infoList = findDirs.first().entryInfoList();
        for (QFileInfoList::Iterator it = infoList.begin(); it != infoList.end(); ++it) {
			QString name = it->fileName();
			QString absolutPath = it->absoluteFilePath();
            if (findPattern.exactMatch( it->fileName() )) {
                mResult->append( it->absoluteFilePath() );
                ++count;
                if (count > 5)
                    emitFound();
                if (time.elapsed() > 500 && count > 0)
                    emitFound();
            }

            //We are stopped;
            if (mStop) {
                if( count > 0 ) {
                    emitFound();
                }
                return;
            }
            
            if (it->isDir() && it->fileName() != ".." && it->fileName() != "." )
                findDirs.append( QDir( it->absoluteFilePath() ) );
        }

        findDirs.removeFirst();
    }
    
    if( count > 0 ) {
        emitFound();
    }
}

void FmFindThread::emitFound()
{
    emit found( count );
    count = 0;
    time.restart();
}

void FmFindThread::setLastResult( QStringList r )
{
    mLastResult = r;
}

void FmFindThread::findInResult()
{
    if( mFindPath.isEmpty() ){
        int count = mLastResult.count();
        for (QStringList::Iterator it = mLastResult.begin(); it != mLastResult.end(); ++it) { 
            if (mStop){
                return;
            }
            QString absolutPath = (*it);
            QFileInfo fileInfo( absolutPath );
            QString fileName = fileInfo.fileName();
            
            if (findPattern.exactMatch( fileName ) ) {
                mResult->append( absolutPath );
                ++count;
                if (count > 5)
                    emitFound();
                if (time.elapsed() > 500 && count > 0)
                   emitFound();
            }
        }    
    }
}
