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

#ifndef FMOPERATIONCOPY_H
#define FMOPERATIONCOPY_H
#include <QObject>
#include <QString>

#include "fmoperationbase.h"

class FmOperationCopy : public FmOperationBase
{
Q_OBJECT
public:
    explicit FmOperationCopy( QObject *parent, QStringList sourceList, QString targetPath );
    virtual ~FmOperationCopy();
    int start( volatile bool *isStopped, QString *errString );

    QStringList sourceList();
    QString     targetPath();

signals:
    void askForRename( const QString &srcFile, QString *destFile );
    void notifyPreparing( bool cancelable );
    void notifyStart( bool cancelable, int maxSteps );
    void notifyProgress( int currentStep );

private:
    int copy( const QString &source, const QString &targetPath, const QString &newTargetName = QString() );
    int copyDirInsideContent( const QString &srcPath, const QString &destPath );
    void IncreaseProgress( quint64 size );

private:
    QStringList mSourceList;
    QString     mTargetPath;
    
private:
    volatile bool *mStop;
    quint64 mTotalSize;
    
    QString       *mErrString;

    quint64 mCopiedSize;
    int     mTotalSteps;
    int     mCurrentStep;
};
#endif
