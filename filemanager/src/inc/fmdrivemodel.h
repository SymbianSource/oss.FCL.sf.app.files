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

#ifndef FMDRIVEMODEL_H
#define FMDRIVEMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QFileIconProvider>
#include <QModelIndex>

class FmDriveModel : public QAbstractListModel
{
Q_OBJECT
public:
    enum Option
    {
        FillWithVolume        = 0x00000001, // add volume behind disname
        FillWithDefaultVolume = 0x00000002, // add default diskname while enable FillWithVolume and volume is empty
        HideUnAvailableDrive  = 0x00000004  // for example: drive F when no MMC card inserted.
    };
    Q_DECLARE_FLAGS(Options, Option)

    explicit FmDriveModel( QObject *parent = 0, Options options = 0 );
    virtual ~FmDriveModel();

    void refresh();
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    bool indexValid( const QModelIndex &index ) const;
    QString driveName( const QModelIndex &index ) const;
    QString displayString( const QModelIndex &index ) const;
    
private:
    QStringList         mFindResult;
    QFileIconProvider   *mIconProvider;
    QStringList         mDriveList;
    Options             mOptions;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FmDriveModel::Options)

#endif
