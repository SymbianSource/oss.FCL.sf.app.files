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

#include "fmdrivemodel.h"
#include "fmutils.h"

#include <QDir>
#include <QFileInfo>

FmDriveModel::FmDriveModel( QObject *parent, Options options ) :
    QAbstractListModel( parent ), mOptions( options )
{
    mIconProvider = new QFileIconProvider();
    refresh();
}

FmDriveModel::~FmDriveModel(void)
{
    delete mIconProvider;
}

void FmDriveModel::refresh()
{
    QFileInfoList infoList = QDir::drives();

    mDriveList.clear();
    if( mOptions & HideUnAvailableDrive ) {
        FmUtils::getDriveList( mDriveList, true );
    } else {
        FmUtils::getDriveList( mDriveList, false );
    }
    emit layoutChanged();
}


int FmDriveModel::rowCount( const QModelIndex &parent ) const
{
    if (!parent.isValid())
        return mDriveList.size();

    return 0;
}

int FmDriveModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return 1;
}

QVariant FmDriveModel::data( const QModelIndex &index, int role ) const
{
    if (!indexValid( index ))
        return QVariant();

    if (role == Qt::DisplayRole ) {
       return displayString( index );
    } else if (role == Qt::UserRole ) {
       return driveName( index );
    }

    if (index.column() == 0) {
        if (role == Qt::DecorationRole ) {
            QString path = driveName( index );
            return mIconProvider->icon( QFileInfo( path ) );
        }
    }

    if (index.column() == 1 && role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }

    return QVariant();
}


QVariant FmDriveModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if (orientation == Qt::Horizontal) {
        if (role != Qt::DisplayRole)
            return QVariant();

        switch (section) {
            case 0: return tr("Name");
            case 1: return tr("Size");
            case 2: return tr("Type");
            case 3: return tr("Date Modified");
            default: return QVariant();
        }
    }

    return QAbstractItemModel::headerData( section, orientation, role );
}

bool FmDriveModel::indexValid( const QModelIndex &index ) const
{
    if( !(&index) )
        return false;
    return true;
}

QString FmDriveModel::driveName( const QModelIndex &index ) const
{
    QString data;
    if (index.row() >= 0 && index.row() < mDriveList.size()) {
        int row = index.row();
        QString diskName = mDriveList[ row ];
        data = diskName;
    }
    return data;
}

QString FmDriveModel::displayString( const QModelIndex &index ) const
{
    QString data;
    if (index.row() >= 0 && index.row() < mDriveList.size()) {
        int row = index.row();
        QString diskName = mDriveList[ row ];

        if( mOptions & FillWithVolume ) {
            data = FmUtils::fillDriveVolume( diskName, mOptions & FillWithDefaultVolume );
        } else {
            data = FmUtils::removePathSplash( diskName );
        }
    }
    return data;
}
