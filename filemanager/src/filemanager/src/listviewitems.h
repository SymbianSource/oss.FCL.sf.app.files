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
*     The header file of the disk list view item
*
*/

#ifndef LISTVIEWITEMS_H
#define LISTVIEWITEMS_H

#include "fmcommon.h"

#include <hblistviewitem.h>

class HbCheckBox;
class HbLabel;

class DiskListViewItem : public HbListViewItem
{
    Q_OBJECT

public:
    DiskListViewItem( QGraphicsItem *parent = 0 );
    ~DiskListViewItem();

    virtual bool canSetModelIndex( const QModelIndex &index ) const;
    virtual HbAbstractViewItem *createItem();
    virtual void updateChildItems();

private slots:
	void setCheckedState( int state );

private:
    virtual void polish(HbStyleParameters& params);

private:
    void init();

    HbLabel *mIconLabel;
    HbLabel *mDiskNameLabel;
    HbLabel *mSizeLabel;
    HbLabel *mFreeLabel;
    HbCheckBox *mCheckBox;
};

/* not used
class FileListViewItem : public HbListViewItem
{
	Q_OBJECT

public:
    FileListViewItem( QGraphicsItem *parent = 0 );
    ~FileListViewItem();

    virtual bool canSetModelIndex( const QModelIndex &index ) const;
    virtual HbAbstractViewItem *createItem();
    virtual void updateChildItems();

    virtual void polish(HbStyleParameters& params);
private slots:
	void setCheckedState( int state );

private:
    void init();

    HbLabel *mIconLabel;
    HbLabel *mNameLabel;
    HbCheckBox *mCheckBox;
};
*/

#endif //LISTVIEWITEMS_H
