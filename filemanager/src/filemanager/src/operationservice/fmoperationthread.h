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
*     The header file of the operation thread of file manager
*
*/

#ifndef FMOPERATIONTHREAD_H
#define FMOPERATIONTHREAD_H

#include "fmoperationservice.h"
#include "fmcommon.h"

#include <QString>
#include <QStringList>

#include <QThread>

class FmDriveDetailsSize;
class FmOperationBase;

class FmOperationCopy;
class FmOperationMove;
class FmOperationRemove;

class FmOperationThread : public QThread
{
    Q_OBJECT
public:
    FmOperationThread( QObject *parent );
    ~FmOperationThread();

    int asyncCopy( FmOperationBase* operationBase );
    int asyncMove( FmOperationBase *operationBase );
    int asyncRemove( FmOperationBase *operationBase );

    int asyncFormat( FmOperationBase *operationBase );
    int asyncViewDriveDetails( FmOperationBase *operationBase );
    int asyncViewFolderDetails( FmOperationBase *operationBase );
    void stop();

signals:
    void askForRename( const QString &srcFile, QString *destFile );
    void askForReplace( const QString &srcFile, const QString &destFile, bool *isAccepted );
    void refreshModel( const QString &path );

    void showNote(const char*);
    void notifyWaiting( bool cancelable );
    void notifyPreparing( bool cancelable );      // this step could not be used if not needed.
    void notifyStart( bool cancelable, int maxSteps );
    void notifyProgress( int currentStep );

    void notifyFinish(); 
    void notifyError(int error, QString errString ); 
    void notifyCanceled(); 

private slots:
    void onAskForRename( const QString &srcFile, QString *destFile );
    void onAskForReplace( const QString &srcFile, const QString &destFile, bool *isAccepted );
    void onShowNote( const char *noteString );
    void on_operationElement_notifyPreparing( bool cancelable );
    void on_operationElement_notifyStart( bool cancelable, int maxSteps );
    void on_operationElement_notifyProgress( int currentStep );

protected:
    void run();

private:
    volatile bool mStop;
    FmOperationBase* mOperationBase;
    QString mErrString;
};

#endif
