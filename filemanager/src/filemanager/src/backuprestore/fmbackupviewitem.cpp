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
 *     The source file of the back up view list item of file manager
 */

#include "fmbackupviewitem.h"

#include <QGraphicsLinearLayout>

#include <hblabel.h>
#include <hbcheckbox.h>
#include <hbwidget.h>
#include <hblineedit.h>


FmBackupViewItem::FmBackupViewItem( QGraphicsItem *parent )
  : mTitleLabel( 0 ),
    //mTipsLabel( 0 ),
	mTipsLineEdit( 0 ),
    HbListViewItem( parent )
{
	init();
}

FmBackupViewItem::~FmBackupViewItem()
{
}

void FmBackupViewItem::polish(HbStyleParameters& params)
{
    Q_UNUSED(params);
}

HbAbstractViewItem *FmBackupViewItem::createItem()
{
	return new FmBackupViewItem( parentItem() );
}

void FmBackupViewItem::updateChildItems()
{
	QString string = modelIndex().data( Qt::DisplayRole ).toString();	

	QStringList stringList = string.split( '\t' );

	if( stringList.count() == 0 ){
		return;
	}

	 mTitleLabel->setText( stringList.first() );

	 if( stringList.first() != stringList.last() ){
	     //mTipsLabel->setText( stringList.last() );
		 mTipsLineEdit->setText( stringList.last() );
	 }
}


void FmBackupViewItem::init()
{
	QGraphicsLinearLayout *vLayout = new QGraphicsLinearLayout();
	vLayout->setOrientation( Qt::Vertical );

	mTitleLabel = new HbLabel( "" );
	mTitleLabel->setFontSpec( HbFontSpec( HbFontSpec::Primary ) );
	vLayout->addItem( mTitleLabel );
	vLayout->setAlignment( mTitleLabel, Qt::AlignLeft );

	//mTipsLabel = new HbLabel( "" );
	//mTipsLabel->setFontSpec( HbFontSpec( HbFontSpec::Secondary ) );
	//mTipsLabel->setAlignment( Qt::AlignHCenter );
		
	//vLayout->addItem( mTipsLabel );
	//vLayout->setAlignment( mTipsLabel, Qt::AlignLeft );

	mTipsLineEdit = new HbLineEdit( "" );
	mTipsLineEdit->setFontSpec( HbFontSpec( HbFontSpec::Secondary ) );
	//mTipsLineEdit->setAlignment( Qt::AlignHCenter );
	mTipsLineEdit->setReadOnly( true );

	vLayout->addItem( mTipsLineEdit );
	vLayout->setAlignment( mTipsLineEdit, Qt::AlignLeft );

	setLayout( vLayout );
}

//FmRestoreViewItem
FmRestoreViewItem::FmRestoreViewItem( QGraphicsItem *parent )
  : mRestoreContentLabel( 0 ),
    mDateTimeLabel( 0 ),
	mCheckBox( 0 ),
    HbListViewItem( parent )
{
	init();
}

FmRestoreViewItem::~FmRestoreViewItem()
{
}


HbAbstractViewItem *FmRestoreViewItem::createItem()
{
	return new FmRestoreViewItem( parentItem() );
}

void FmRestoreViewItem::polish(HbStyleParameters& params)
{
    Q_UNUSED(params);
}

void FmRestoreViewItem::updateChildItems()
{
	QString string = modelIndex().data( Qt::DisplayRole ).toString();	

	QStringList stringList = string.split( '\t' );

	if( stringList.count() == 0 ){
		return;
	}

	 mRestoreContentLabel->setText( stringList.first() );

	 if( stringList.first() != stringList.last() ){
	     mDateTimeLabel->setText( stringList.last() );
	 }

}


void FmRestoreViewItem::init()
{
	QGraphicsLinearLayout *hLayout = new QGraphicsLinearLayout();
	hLayout->setOrientation( Qt::Horizontal );

	mCheckBox = new HbCheckBox( this );
    hLayout->addItem( mCheckBox );
	hLayout->setAlignment( mCheckBox, Qt::AlignVCenter );

	QGraphicsLinearLayout *vLayout = new QGraphicsLinearLayout();
	vLayout->setOrientation( Qt::Vertical );

	mRestoreContentLabel = new HbLabel("");
	mRestoreContentLabel->setFontSpec( HbFontSpec( HbFontSpec::Primary ) );
	vLayout->addItem( mRestoreContentLabel );
	vLayout->setAlignment( mRestoreContentLabel, Qt::AlignLeft );

	mDateTimeLabel = new HbLabel("");
	mDateTimeLabel->setFontSpec( HbFontSpec( HbFontSpec::Secondary ) );
	vLayout->addItem( mDateTimeLabel );
	vLayout->setAlignment( mDateTimeLabel, Qt::AlignLeft );

	HbWidget *textWidget = new HbWidget();
	textWidget->setLayout(vLayout);

	hLayout->addItem( textWidget );
	hLayout->setAlignment( textWidget, Qt::AlignVCenter );

	setLayout( hLayout );

}

void FmRestoreViewItem::setCheckBoxState()
{
	if ( mCheckBox->checkState() ==  Qt::Unchecked ){
		mCheckBox->setCheckState( Qt::Checked );
		setSelected( true );
	}
	else if( mCheckBox->checkState() ==  Qt::Checked ){
		mCheckBox->setCheckState( Qt::Unchecked );
		setSelected( false );
	}
}
