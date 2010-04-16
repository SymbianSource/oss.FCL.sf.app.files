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
*     The source file of the operation param of file manager
*
*/

#include "fmoperationbase.h"

FmOperationBase::FmOperationBase( QObject *parent, FmOperationService::TOperationType operationType ) : 
    QObject( parent ), mOperationType( operationType )
{
}

FmOperationBase::~FmOperationBase()
{

}

FmOperationService::TOperationType FmOperationBase::operationType()
{
    return mOperationType;
}


//FmOperationFormat
FmOperationFormat::FmOperationFormat( QObject *parent, QString mDriverName ) : FmOperationBase( parent, FmOperationService::EOperationTypeFormat ),
    mDriverName( mDriverName )
{
}
FmOperationFormat::~FmOperationFormat()
{
}

QString FmOperationFormat::driverName()
{
    return mDriverName;
}

//FmOperationFormat
FmOperationDriveDetails::FmOperationDriveDetails( QObject *parent, QString driverName ) :
        FmOperationBase( parent, FmOperationService::EOperationTypeDriveDetails ),
        mDriverName( driverName )
{
}
FmOperationDriveDetails::~FmOperationDriveDetails()
{
}

QString FmOperationDriveDetails::driverName()
{
    return mDriverName;
}

QList<FmDriveDetailsSize*> &FmOperationDriveDetails::detailsSizeList()
{
    return mDetailsSizeList;
}

//FmOperationFolderDetails
FmOperationFolderDetails::FmOperationFolderDetails( QObject *parent, const QString folderPath ) :
        FmOperationBase( parent, FmOperationService::EOperationTypeFolderDetails ),
        mFolderPath( folderPath ),
        mNumofSubFolders( 0 ),
        mNumofFiles( 0 ),
        mSizeofFolder( 0 )

{
}
FmOperationFolderDetails::~FmOperationFolderDetails()
{

}

QString FmOperationFolderDetails::folderPath()
{
    return mFolderPath;
}

int &FmOperationFolderDetails::numofSubFolders()
{
    return mNumofSubFolders;
}

int &FmOperationFolderDetails::numofFiles()
{
    return mNumofFiles;
}

quint64 &FmOperationFolderDetails::sizeofFolder()
{
    return mSizeofFolder;
}



FmOperationBackup::FmOperationBackup( QObject *parent )
    : FmOperationBase( parent, FmOperationService::EOperationTypeBackup )
{
}

FmOperationBackup::~FmOperationBackup()
{
}


FmOperationRestore::FmOperationRestore( QObject *parent, quint64 selection )
    : FmOperationBase( parent, FmOperationService::EOperationTypeRestore ), mSelection( selection )
{
}

FmOperationRestore::~FmOperationRestore()
{
}

quint64 FmOperationRestore::selection()
{
    return mSelection;
}
