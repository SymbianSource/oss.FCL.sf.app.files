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
 *     Zhiqiang Yang <zhiqiang.yang@nokia.com>
 * 
 * Description:
 *     The find result model header file of file manager
 */

#ifndef FMFINDRESULTMODEL_H
#define FMFINDRESULTMODEL_H

#include "fmcommon.h"

#include <QFileInfo>
#include <QStringList>
#include <QAbstractListModel>

class FmFindThread;
class QFileIconProvider;

class FmFindResultModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        FileIconRole = Qt::DecorationRole
    };

    enum SortFlag {
        Name = 0,
        Time = 1,
        Size = 2,
        Type = 3
    };

    FmFindResultModel( QObject *parent = 0 );
    ~FmFindResultModel();
    
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
	bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() );
	bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );
    QString filePath ( const QModelIndex & index ) const;

    QFileInfo fileInfo( const QModelIndex &index ) const;

    QString findPath() const;
    void setFindPath( const QString &path );

    QRegExp pattern() const;
    void setPattern( const QRegExp &regExp );

    void find();
    void stop();
    bool isFinding() const;

    virtual void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );

    static bool caseNameLessThan( const QString &s1, const QString &s2 );
    static bool caseTimeLessThan( const QString &s1, const QString &s2 );
    static bool caseSizeLessThan( const QString &s1, const QString &s2 );
    static bool caseTypeLessThan( const QString &s1, const QString &s2 );

signals:
    void finished();

    // pass modelCountChanged signal to parent widget
    // so parent widget could change contentWiget between emptyTipsWidget and listWidget
    void modelCountChanged( int count );

private slots:
    void on_findThread_found( int count );

private:
    bool indexValid( const QModelIndex &index ) const;
    void init();

    FmFindThread *mFindThread;
    QStringList mFindResult;
    QFileIconProvider *mIconProvider;
};

#endif
