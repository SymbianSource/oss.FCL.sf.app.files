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
      mSizeLabel( 0 ),
      mFreeLabel( 0 ),
      mCheckBox( 0 )

{
    init();
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
	return new DiskListViewItem( parentItem() );
}

void DiskListViewItem::updateChildItems()
{
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
    mSizeLabel->setPlainText( hbTrId ( "Size: " ) + FmUtils::formatStorageSize( driverInfo.size() ) );
    mFreeLabel->setPlainText( hbTrId ( "Free: " ) + FmUtils::formatStorageSize( driverInfo.freeSize() ) );

//    mCheckBox->setCheckState( checkState() );
}

void DiskListViewItem::setCheckedState( int state )
{
	HbAbstractViewItem::setCheckState( static_cast<Qt::CheckState>(state) );
}

void DiskListViewItem::init()
{
	QGraphicsLinearLayout *hLayout = new QGraphicsLinearLayout();
	hLayout->setOrientation( Qt::Horizontal );

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

	mSizeLabel = new HbLabel();
	mSizeLabel->setFontSpec( HbFontSpec( HbFontSpec::Secondary ) );
	vLayout->addItem( mSizeLabel );
	vLayout->setAlignment( mSizeLabel, Qt::AlignLeft );

	mFreeLabel = new HbLabel();
	mFreeLabel->setFontSpec( HbFontSpec( HbFontSpec::Secondary ) );
	vLayout->addItem( mFreeLabel );
	vLayout->setAlignment( mFreeLabel, Qt::AlignLeft );

	HbWidget *labelsWidget = new HbWidget();
	labelsWidget->setLayout(vLayout);

	hLayout->addItem( labelsWidget );
	hLayout->setStretchFactor( labelsWidget, 5 );

	setLayout( hLayout );
}

//file list item, not used.
/*
FileListViewItem::FileListViewItem( QGraphicsItem *parent )
    : HbListViewItem( parent ),
      mIconLabel( 0 ),
      mNameLabel( 0 ),
      mCheckBox( 0 )
{
    init();
}

FileListViewItem::~FileListViewItem()
{
}

void FileListViewItem::polish(HbStyleParameters& params)
{
    Q_UNUSED(params);
}

bool FileListViewItem::canSetModelIndex( const QModelIndex &index ) const
{
    Q_UNUSED( index );
    return true;

//  do not used
//	const QFileSystemModel *model = dynamic_cast<const QFileSystemModel *>(index.model());
//	QFileInfo info = model->fileInfo( index );
//	QString path = info.path();

//	return (path.right(1) != ":");
   
}


HbAbstractViewItem *FileListViewItem::createItem()
{
	return new FileListViewItem( parentItem() );
}

void FileListViewItem::updateChildItems()
{
	QVariant variant = modelIndex().data( Qt::DecorationRole );
	QIcon icon = qvariant_cast<QIcon>( variant );
	QString diskName = modelIndex().data( Qt::DisplayRole ).toString();

    QString debugString = "updateChindItems: diskName = " + diskName;
    FmLogger::log(debugString);
	mIconLabel->setIcon( HbIcon( icon ) );
	mNameLabel->setPlainText( diskName );
    mCheckBox->setCheckState( checkState() );
}

void FileListViewItem::setCheckedState( int state )
{
	HbAbstractViewItem::setCheckState( static_cast<Qt::CheckState>(state) );
}

void FileListViewItem::init()
{
	QGraphicsLinearLayout *hLayout = new QGraphicsLinearLayout();
	hLayout->setOrientation( Qt::Horizontal );

	mIconLabel = new HbLabel();
	mIconLabel->setMinimumWidth(32);
	hLayout->addItem( mIconLabel );
	hLayout->setAlignment( mIconLabel, Qt::AlignTop );
	hLayout->setStretchFactor( mIconLabel, 1 );

	mNameLabel = new HbLabel();
	mNameLabel->setFontSpec( HbFontSpec( HbFontSpec::Primary ) );
	mNameLabel->setAlignment( Qt::AlignVCenter );
	hLayout->addItem( mNameLabel );
	hLayout->setAlignment( mNameLabel, Qt::AlignLeft );
	hLayout->setStretchFactor( mNameLabel, 20 );

    mCheckBox = new HbCheckBox( this );
	connect( mCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setCheckedState(int)) );
    hLayout->addItem( mCheckBox );
    hLayout->setAlignment( mCheckBox, Qt::AlignLeft );

	setLayout( hLayout );
}
*/
