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
private:
    FmOperationService::TOperationType mOperationType;

};

class FmOperationFormat : public FmOperationBase
{
public:
    explicit FmOperationFormat( QObject *parent, QString mDriverName );
    virtual ~FmOperationFormat();
    QString driverName();
private:
    QString mDriverName;
};

class FmOperationDriveDetails : public FmOperationBase
{
public:
    explicit FmOperationDriveDetails( QObject *parent, QString driverName );
    
    virtual ~FmOperationDriveDetails();
    QString driverName();
    QList<FmDriveDetailsSize*> &detailsSizeList();
private:
    QString mDriverName;
    QList<FmDriveDetailsSize*> mDetailsSizeList;
};

class FmOperationFolderDetails : public FmOperationBase
{
public:
    explicit FmOperationFolderDetails( QObject *parent, const QString folderPath );
    
    virtual ~FmOperationFolderDetails();
    QString folderPath();
    int &numofSubFolders();
    int &numofFiles();
    quint64 &sizeofFolder();
    
private:
    QString mFolderPath;
    int mNumofSubFolders;
    int mNumofFiles;
    quint64 mSizeofFolder;
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
