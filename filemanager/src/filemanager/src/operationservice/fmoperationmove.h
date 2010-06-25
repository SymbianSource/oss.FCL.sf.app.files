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

#ifndef FMOPERATIONMOVE_H
#define FMOPERATIONMOVE_H
#include <QObject>
#include <QString>

#include "fmoperationbase.h"

class FmOperationMove : public FmOperationBase
{
Q_OBJECT
public:
    explicit FmOperationMove( QObject *parent, QStringList sourceList, QString targetPath );
    virtual ~FmOperationMove();
    QStringList sourceList();
    QString     targetPath();
    int start( volatile bool *isStopped, QString *errString );

signals:
    void askForRename( const QString &srcFile, QString *destFile );
    void askForReplace( const QString &srcFile, const QString &destFile, bool *isAccepted );
    void showNote( const QString &noteString );
    void notifyPreparing( bool cancelable );
    void notifyStart( bool cancelable, int maxSteps );
    void notifyProgress( int currentStep );

private:
    int move( const QString &source, const QString &targetPath, const QString &newTargetName = QString() );
    int moveDirInsideContent( const QString &srcPath, const QString &destPath );
    void increaseProgress( quint64 size );
    void queryForRename( const QString &srcFile, QString *destFile );
     
private:
    QStringList mSourceList;
    QString     mTargetPath;
    
private:
    volatile bool  *mStop;
    quint64   mTotalSize;
    QString   *mErrString;

    quint64 mMovedSize;
    int     mTotalSteps;
    int     mCurrentStep;
};

#endif
