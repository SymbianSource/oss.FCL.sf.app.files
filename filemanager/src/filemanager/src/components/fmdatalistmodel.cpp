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
 *     The source file of the data list model of file manager
 */

#include "fmdatalistmodel.h"


FmDataListModel::FmDataListModel( QObject *parent )
    : QAbstractListModel( parent )
{
}

int FmDataListModel::rowCount( const QModelIndex &parent ) const
{
    if (parent.isValid())
        return 0;

    return mDisplayLst.count();
}

QVariant FmDataListModel::data( const QModelIndex &index, int role ) const
{
    if ( index.row() < 0 || index.row() >= mDisplayLst.size() )
        return QVariant();

    if ( role == Qt::DisplayRole || role == Qt::EditRole )
        return mDisplayLst.at( index.row() );

    if ( role == Qt::UserRole )
        return mUserDataLst.at( index.row() );

    return QVariant();
}

Qt::ItemFlags FmDataListModel::flags( const QModelIndex &index ) const
{
    if ( !index.isValid() )
        return QAbstractItemModel::flags( index ) | Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags( index ) | Qt::ItemIsEditable;
}

bool FmDataListModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if ( index.row() >= 0 && index.row() < mDisplayLst.size()
        && ( role == Qt::EditRole || role == Qt::DisplayRole ) ){
        mDisplayLst.replace( index.row(), value.toString() );
        emit dataChanged( index, index );
        return true;
    }

    if ( index.row() >= 0 && index.row() < mUserDataLst.size()
        && ( role == Qt::UserRole ) ) {
        mUserDataLst.replace( index.row(), value.toString() );
        emit dataChanged( index, index );
        return true;
    }
    return false;
}

bool FmDataListModel::insertRows( int row, int count, const QModelIndex &parent )
{
    if ( count < 1 || row < 0 || row > rowCount( parent ) )
        return false;

    beginInsertRows( QModelIndex(), row, row + count - 1 );

    for ( int r = 0; r < count; ++r )
    {
        mDisplayLst.insert( row, QString() );
        mUserDataLst.insert( row, QString() );
    }

    endInsertRows();

    return true;
}

bool FmDataListModel::removeRows( int row, int count, const QModelIndex &parent )
{
    if ( count <= 0 || row < 0 || ( row + count ) > rowCount( parent ) )
        return false;

    beginRemoveRows( QModelIndex(), row, row + count - 1 );

    for ( int r = 0; r < count; ++r )
    {
        mDisplayLst.removeAt( row );
        mUserDataLst.removeAt( row );
    }

    endRemoveRows();
    return true;
}
