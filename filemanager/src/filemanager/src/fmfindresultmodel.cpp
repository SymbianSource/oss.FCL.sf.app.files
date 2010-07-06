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
 *     The find result model source file of file manager
 */

#include "fmfindresultmodel.h"
#include "fmfindthread.h"
#include "fmfileiconprovider.h"

#include <QDateTime>

#include <hbglobal.h>

FmFindResultModel::FmFindResultModel( QObject *parent )
    : QAbstractListModel( parent )
{
    init();
    connect( mFindThread, SIGNAL(finished()), this, SIGNAL(finished()) );
	connect( mFindThread, SIGNAL(found(int)), this, SLOT(on_findThread_found( int) ), Qt::BlockingQueuedConnection ); 
}

FmFindResultModel::~FmFindResultModel()
{
	delete mIconProvider;
}

int FmFindResultModel::rowCount( const QModelIndex &parent ) const
{
    if (!parent.isValid())
        return mFindResult.size();

    return 0;
}

int FmFindResultModel::columnCount( const QModelIndex &parent ) const
{
    if (!parent.isValid())
        return 4;
    
    return 0;
}

QVariant FmFindResultModel::data( const QModelIndex &index, int role ) const
{
    if (!indexValid( index ))
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return fileInfo( index ).fileName();
            case 1: return fileInfo( index ).size();
            case 2: return mIconProvider->type( fileInfo( index ) );
            case 3: return fileInfo( index ).lastModified().toString( Qt::LocalDate );
            default:
                qWarning( "FmFindResultData: invalid display value column %d", index.column() );
                return QVariant();
        }
    }

    if (index.column() == 0) {
        if (role == FileIconRole) {
            return mIconProvider->icon( fileInfo( index ) );
        }
    }

    if (index.column() == 1 && role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }

    return QVariant();
}

QString FmFindResultModel::filePath ( const QModelIndex & index ) const
{
   return fileInfo( index ).filePath();
}

QVariant FmFindResultModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if (orientation == Qt::Horizontal) {
        if (role != Qt::DisplayRole)
            return QVariant();

        switch (section) {
            case 0: return hbTrId("Name");
            case 1: return hbTrId("Size");
            case 2: return hbTrId("Type");
            case 3: return hbTrId("Date Modified");
            default: return QVariant();
        }
    }

    return QAbstractItemModel::headerData( section, orientation, role );
}

bool FmFindResultModel::insertRows( int row, int count, const QModelIndex &parent )
{
    Q_UNUSED( parent );
	if (row < 0 || count < 1)
		return false;

	beginInsertRows( QModelIndex(), row, row + count - 1 );

	endInsertRows();

	return true;
}

bool FmFindResultModel::removeRows( int row, int count, const QModelIndex &parent )
{
    Q_UNUSED( parent );
    
	if (row < 0 || count < 1 || row + count > mFindResult.size())
		return false;

	beginRemoveRows( QModelIndex(), row, row + count - 1 );

	for (int i = 0; i < count; ++i)
		mFindResult.removeAt(row);

	endRemoveRows();
	
	return true;
}

QFileInfo FmFindResultModel::fileInfo( const QModelIndex &index ) const
{
    if (index.row() >= 0 && index.row() < mFindResult.size())
        return QFileInfo( mFindResult[index.row()] );
    else
        return QFileInfo();
}

QString FmFindResultModel::findPath() const
{
    return mFindThread->findPath();
}

void FmFindResultModel::setFindPath( const QString &path )
{
    mFindThread->setFindPath( path );
}

QRegExp FmFindResultModel::pattern() const
{
    return mFindThread->pattern();
}

void FmFindResultModel::setPattern( const QRegExp &regExp )
{
    mFindThread->setPattern( regExp );
}

void FmFindResultModel::find()
{
	if (mFindThread->isRunning())
		return;

    if( findPath().isEmpty() ){
        mFindThread->setLastResult( mFindResult );
    }
	removeRows( 0, mFindResult.size() );
    mFindThread->start();
}

void FmFindResultModel::stop()
{
    mFindThread->stop();
    mFindThread->wait();
}

bool FmFindResultModel::isFinding() const
{
    return mFindThread->isRunning();
}

void FmFindResultModel::on_findThread_found( int count )
{
    if( count > 0 ) {
        int size = mFindResult.size();
        insertRows( mFindResult.size() - count, count );
    }
    emit modelCountChanged( mFindResult.size() );
}

bool FmFindResultModel::indexValid( const QModelIndex &index ) const
{
    Q_UNUSED( index );
    return true;
}

void FmFindResultModel::init()
{
    mFindThread = new FmFindThread( &mFindResult, this );
    mFindThread->setObjectName( "findThread" );
    mIconProvider = new FmFileIconProvider();
}

bool FmFindResultModel::caseNameLessThan(const QString &s1, const QString &s2)
{
    QFileInfo info1( s1 );
    QFileInfo info2( s2 );
    
    return info1.fileName() < info2.fileName();
}

bool FmFindResultModel::caseTimeLessThan(const QString &s1, const QString &s2)
{
    QFileInfo info1( s1 );
    QFileInfo info2( s2 );
    
    return info1.lastModified() < info2.lastModified();
}

bool FmFindResultModel::caseSizeLessThan(const QString &s1, const QString &s2)
{
    QFileInfo info1( s1 );
    QFileInfo info2( s2 );
    
    return info1.size() < info2.size();
}

bool FmFindResultModel::caseTypeLessThan(const QString &s1, const QString &s2)
{
    QFileInfo info1( s1 );
    QFileInfo info2( s2 );
    
    if( info1.isDir() != info2.isDir() ){
        return info1.isDir();
    }
    else{
        return info1.suffix().toLower() < info2.suffix().toLower();   
    }
}


void FmFindResultModel::sort ( int column, Qt::SortOrder order )
{  
    Q_UNUSED( order );
           
//    emit  layoutAboutToBeChanged();
    
    QStringList lst( mFindResult );
    removeRows( 0, mFindResult.size() );
    
    switch( ( SortFlag )column )
    {
    case Name:
        qSort( lst.begin(), lst.end(), caseNameLessThan );
        break;
    case Time:
        qSort( lst.begin(), lst.end(), caseTimeLessThan );
        break;
    case Size:
        qSort( lst.begin(), lst.end(), caseSizeLessThan );
        break;
    case Type:
        qSort( lst.begin(), lst.end(), caseTypeLessThan );
        break;
    }    
    
    mFindResult = lst;
    insertRows( 0, mFindResult.size() );
    emit modelCountChanged( mFindResult.size() );
}
