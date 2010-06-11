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
*     Steven Yao <steven.yao@nokia.com>
* 
* Description:
*     The source file of the disk list view item
*
*/
#include "listviewitems.h"
#include "fmutils.h"

#include <QFileIconProvider>
#include <QGraphicsLinearLayout>

#include <hblabel.h>
#include <hbcheckbox.h>
#include <hbpushbutton.h>
#include <hbabstractitemview.h>

DiskListViewItem::DiskListViewItem( QGraphicsItem *parent )
    : HbListViewItem( parent ),
      mIconLabel( 0 ),
      mDiskNameLabel( 0 ),
      mFirstLabel( 0 ),
      mSecondLabel( 0 ),
      mCheckBox( 0 ),
      hLayout( 0 )

{
    //init();
}

DiskListViewItem::~DiskListViewItem()
{
}

void DiskListViewItem::polish(HbStyleParameters& params)
{
    Q_UNUSED(params);
}

bool DiskListViewItem::canSetModelIndex( const QModelIndex &index ) const
{
    Q_UNUSED( index );
	return true;
}


HbAbstractViewItem *DiskListViewItem::createItem()
{
	return new DiskListViewItem( *this );
}

void DiskListViewItem::updateChildItems()
{
    //HbListViewItem::updateChildItems();
    if( !hLayout ) {
        init();
    }
	QVariant variant = modelIndex().data( Qt::DecorationRole );
	QIcon icon = qvariant_cast<QIcon>( variant );
    if( icon.isNull() ) {
        QFileIconProvider fileIconProvider;
        icon = fileIconProvider.icon( QFileIconProvider::Drive );
    }
    QString displayString = modelIndex().data( Qt::DisplayRole ).toString();
	QString diskName = modelIndex().data( Qt::UserRole ).toString();

    diskName = FmUtils::fillPathWithSplash( diskName );

	mIconLabel->setIcon( HbIcon( icon ) );
	
	FmDriverInfo driverInfo = FmUtils::queryDriverInfo( diskName );

    mDiskNameLabel->setPlainText( displayString );
	if( driverInfo.driveState() & FmDriverInfo::EDriveAvailable ) {
		mFirstLabel->setPlainText( hbTrId ( "Size: " ) + FmUtils::formatStorageSize( driverInfo.size() ) );
		mSecondLabel->setPlainText( hbTrId ( "Free: " ) + FmUtils::formatStorageSize( driverInfo.freeSize() ) );
	} else if( driverInfo.driveState() & FmDriverInfo::EDriveLocked ) {
		mFirstLabel->setPlainText( hbTrId ( "Locked" ) );
		mSecondLabel->setPlainText( QString(" ") );
	} else if( driverInfo.driveState() & FmDriverInfo::EDriveCorrupted ) {
		mFirstLabel->setPlainText( hbTrId ( "Corrupted" ) );
		mSecondLabel->setPlainText( QString(" ") );
	} else if( driverInfo.driveState() & FmDriverInfo::EDriveNotPresent ) {
		mFirstLabel->setPlainText( hbTrId ( "Not Ready" ) );
		mSecondLabel->setPlainText( QString(" ") );
	}

//    mCheckBox->setCheckState( checkState() );
}

void DiskListViewItem::setCheckedState( int state )
{
	HbAbstractViewItem::setCheckState( static_cast<Qt::CheckState>(state) );
}

void DiskListViewItem::init()
{
    hLayout = new QGraphicsLinearLayout();

	hLayout->setOrientation( Qt::Horizontal );
	hLayout->addItem(layout());
	
	mIconLabel = new HbLabel();
	mIconLabel->setMinimumWidth(32);
	hLayout->addItem( mIconLabel );
	hLayout->setAlignment( mIconLabel, Qt::AlignTop );
	hLayout->setStretchFactor( mIconLabel, 1 );

	QGraphicsLinearLayout *vLayout = new QGraphicsLinearLayout();
	vLayout->setOrientation( Qt::Vertical );

	mDiskNameLabel = new HbLabel();
	mDiskNameLabel->setFontSpec( HbFontSpec( HbFontSpec::Primary ) );
	vLayout->addItem( mDiskNameLabel );
	vLayout->setAlignment( mDiskNameLabel, Qt::AlignLeft );

	mFirstLabel = new HbLabel();
	mFirstLabel->setFontSpec( HbFontSpec( HbFontSpec::Secondary ) );
	vLayout->addItem( mFirstLabel );
	vLayout->setAlignment( mFirstLabel, Qt::AlignLeft );

	mSecondLabel = new HbLabel();
	mSecondLabel->setFontSpec( HbFontSpec( HbFontSpec::Secondary ) );
	vLayout->addItem( mSecondLabel );
	vLayout->setAlignment( mSecondLabel, Qt::AlignLeft );

	HbWidget *labelsWidget = new HbWidget();
	labelsWidget->setLayout(vLayout);

	hLayout->addItem( labelsWidget );
	hLayout->setStretchFactor( labelsWidget, 5 );

	setLayout( hLayout );
}
