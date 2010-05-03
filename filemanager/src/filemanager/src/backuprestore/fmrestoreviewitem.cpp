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
 *     The source file of the restore view list item of file manager
 */
#include "fmrestoreviewitem.h"

#include <QGraphicsLinearLayout>

#include <hblabel.h>
#include <hbcheckbox.h>
#include <hbwidget.h>
#include <hblineedit.h>

//FmRestoreViewItem
FmRestoreViewItem::FmRestoreViewItem( QGraphicsItem *parent )
  : HbListViewItem( parent ),
    mRestoreContentLabel( 0 ),
    mDateTimeLabel( 0 ),
    mCheckBox( 0 )

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

	 mRestoreContentLabel->setPlainText( stringList.first() );

	 if( stringList.first() != stringList.last() ){
	     mDateTimeLabel->setPlainText( stringList.last() );
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

bool FmRestoreViewItem::getCheckBoxState()
{
    if( mCheckBox->checkState() == Qt::Unchecked ) {
        return false;
    } else {
        return true;
    }
}