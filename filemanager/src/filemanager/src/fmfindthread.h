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

#ifndef FMFINDTHREAD_H
#define FMFINDTHREAD_H

#include "fmcommon.h"

#include <QTime>
#include <QThread>
#include <QRegExp>
#include <QStringList>

class FmFindThread : public QThread
{
    Q_OBJECT
public:
    explicit FmFindThread( QObject *parent = 0 );
    ~FmFindThread();

    QString findPath() const;
    void setFindPath( const QString &path );

    QRegExp pattern() const;
    void setPattern( const QRegExp &regExp );
    
    void setLastResult( QStringList r );

    void stop();

signals:
    void found( const QStringList &dataList );

protected:
    void run();

private:
    void emitFound();
    void findInResult();

    volatile bool mStop;
    int count;
    QString mFindPath;
    QTime time;
    QRegExp findPattern;
    QStringList mLastResult;
    // Used to store temp search result. if emit found, the result will be cleared.
    QStringList tempResultList;
};

#endif
