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
* 
* Description:
*     The header file of service utils private
*/

#ifndef FMSERVICEUTILSPRIVATE_H
#define FMSERVICEUTILSPRIVATE_H

#include <QTimer>
#include <QObject>
#include <QVariant>
#include <QEventLoop>
#include <QStringList>
#include <QVariantHash>

#include "fmserviceutilshandler.h"


class ShareUi;
class AfActivityStorage;



/*!
    \class FmServiceUtilsPrivate
    \brief The class FmServiceUtilsPrivate provide service utils APIs private wrapper
*/
class FmServiceUtilsPrivate : public QObject, public MServiceUtilsObserver
{
Q_OBJECT
public:
    explicit FmServiceUtilsPrivate( QObject *parent=0 );
    virtual ~FmServiceUtilsPrivate();

    void sendFile( const QStringList &filePath );
    void closeApps();
    void restartApps();

    // activity implement
    bool saveActivity(const QString &activityId, const QVariant &activityData, const QVariantHash &metadata);
    bool removeActivity(const QString &activityId);
private slots:
    void onCloseAppTimeup();
    
private:
    ShareUi *shareUi();
    AfActivityStorage *activityStorage();
    CFmServiceUtilsHandler *serviceUtilsHandler();
    
    virtual void handleCloseAppCompleteL( TInt err );
    
    
private:
    // used to send files
    ShareUi *mShareUi;
    
    // used for some service utils
    CFmServiceUtilsHandler *mServiceUtilsHandler;

    //own used to save activity.
    AfActivityStorage   *mActivityStorage;
    QEventLoop          mCloseAppLoop;
    QTimer              mCloseAppTimer;
    
    bool                mIsCloseAppsFinished;
    bool                mIsCloseAppsTimeup;

};

#endif
