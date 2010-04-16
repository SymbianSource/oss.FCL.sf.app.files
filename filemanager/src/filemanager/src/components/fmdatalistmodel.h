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
 *     The header file of the data list model of file manager
 */

#ifndef FMDATALISTMODEL_H
#define FMDATALISTMODEL_H

#include "fmcommon.h"

#include <QStringList>
#include <QAbstractItemView>

class FmDataListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    FmDataListModel(QObject *parent = 0);

    /**
     * Return row count
     * @param parent ModelIndex
     * @return row count
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * Return data of designated role
     * @param index ModelIndex
     * @param role role of data, DisplayRole for show and UserRole for data
     * @return data
     */
    QVariant data(const QModelIndex &index, int role) const;

    /**
     * Set data of designated role
     * @param index ModelIndex
     * @param role role of data, DisplayRole for show and UserRole for data
     * @return if operation succeed
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole);

    /**
     * Get flags
     * @param index ModelIndex
     * @return flags
     */
    Qt::ItemFlags flags(const QModelIndex &index) const;

    /**
     * Insert count rows into the model before the given row.
     * @param row the given row,
     * @param count count of rows
     * @param parent parent ModelIndex
     * @return if operation succeed
     */
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());

    /**
     * Remove count rows from the model at the given row.
     * @param row the given row,
     * @param count count of rows
     * @param parent parent ModelIndex
     * @return if operation succeed
     */
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
    QStringList mDisplayLst;
    QStringList mUserDataLst;
};

#endif // FMDATALISTMODEL_H
