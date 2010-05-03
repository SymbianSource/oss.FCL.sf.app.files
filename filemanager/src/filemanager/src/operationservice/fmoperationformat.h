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

#ifndef FMOPERATIONFORMAT_H
#define FMOPERATIONFORMAT_H

#include "fmoperationbase.h"

#include <QObject>
#include <QString>

class FmOperationFormat : public FmOperationBase
{
    Q_OBJECT
public:
    explicit FmOperationFormat( QObject *parent, QString mDriverName );
    virtual ~FmOperationFormat();
    QString driverName();
    
    int start();

signals:
    void notifyStart( bool cancelable, int maxSteps );
    void notifyProgress( int currentStep );
    void notifyPreparing( bool cancelable );
    
private:
    QString mDriverName;
    
    int mTotalSteps;
};

#endif
