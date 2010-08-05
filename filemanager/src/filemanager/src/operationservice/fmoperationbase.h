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
*     The header file of the operation param of file manager
*
*/

#ifndef FMOPERATIONBASE_H
#define FMOPERATIONBASE_H

#include "fmoperationservice.h"

#include <QObject>
#include <QString>

class FmOperationBase : public QObject
{
Q_OBJECT
public:
    explicit FmOperationBase( QObject* parent, FmOperationService::TOperationType operationType );
    virtual ~FmOperationBase();

    FmOperationService::TOperationType operationType();
    virtual int prepare();
    virtual void start( volatile bool *isStopped );
    
signals:    
    void showNote( const QString &note );
    void notifyWaiting( bool cancelable );
    void notifyPreparing( bool cancelable );
    void notifyStart( bool cancelable, int maxSteps );
    void notifyProgress( int currentStep );    
    void notifyFinish(); 
    void notifyError( int error, const QString &errString );
     
private:
    FmOperationService::TOperationType mOperationType;

};


class FmOperationBackup : public FmOperationBase
{
public:
    explicit FmOperationBackup( QObject *parent );
    virtual ~FmOperationBackup();
};

class FmOperationRestore : public FmOperationBase
{
public:
    explicit FmOperationRestore( QObject *parent, quint64 selection );
    virtual ~FmOperationRestore();

    quint64 selection();
private:
    quint64                 mSelection;
};

#endif
