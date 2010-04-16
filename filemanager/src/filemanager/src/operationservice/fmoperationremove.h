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

#ifndef FMOPERATIONREMOVE_H
#define FMOPERATIONREMOVE_H
#include <QObject>

#include "fmoperationbase.h"

class FmOperationRemove : public FmOperationBase
{
Q_OBJECT
public:
    explicit FmOperationRemove( QObject *parent, QStringList pathList );
    virtual ~FmOperationRemove();
    
    QStringList pathList();
    int start( volatile bool *isStopped, QString *errString );

signals:
    void askForRename( const QString &srcFile, QString &destFile );

    void notifyPreparing( bool cancelable );
    void notifyStart( bool cancelable, int maxSteps );
    void notifyProgress( int currentStep );

private:
    int remove( const QString &fileName );
	int recursiveRemoveDir( const QString &path );
    void IncreaseProgressOnce();

private:
    QStringList mPathList;
    
private:
    volatile bool           *mStop;
    quint64 mTotalCount;
    
    QString *mErrString;

    quint64 mRemovedCount;
    int     mTotalSteps;
    int     mCurrentStep;
};

#endif
