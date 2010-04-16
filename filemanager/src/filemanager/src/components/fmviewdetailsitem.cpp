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
 *     The source file of the view details list item of file manager
 */

#include "fmviewdetailsitem.h"
#include "fmviewmanager.h"

#include <QGraphicsLinearLayout>

#include <hblabel.h>

FmViewDetailsItem::FmViewDetailsItem( QGraphicsItem *parent )
    : HbListViewItem( parent ),
      mDetailsContentLabel( 0 ),
      mDetailsLabel( 0 )

{
    init();
}

FmViewDetailsItem::~FmViewDetailsItem()
{
}

HbAbstractViewItem *FmViewDetailsItem::createItem()
{
    return new FmViewDetailsItem( parentItem() );
}

bool FmViewDetailsItem::canSetModelIndex( const QModelIndex &index ) const
{
    Q_UNUSED( index );
    return true;
}

void FmViewDetailsItem::polish(HbStyleParameters& params)
{
    Q_UNUSED(params);
}

void FmViewDetailsItem::updateChildItems()
{
    QString string = modelIndex().data( Qt::DisplayRole ).toString();   
    
    mDetailsContentLabel->setPlainText( string );
    
    string = modelIndex().data( Qt::UserRole ).toString();  
    
    mDetailsLabel->setPlainText( string );

}

void FmViewDetailsItem::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout();
    
    if( FmViewManager::viewManager()->orientation() == Qt::Vertical ){
        layout->setOrientation( Qt::Vertical );
    }
    else{
        layout->setOrientation( Qt::Horizontal );   
    }


    mDetailsContentLabel = new HbLabel("");
    mDetailsContentLabel->setFontSpec( HbFontSpec( HbFontSpec::Primary ) );
    layout->addItem( mDetailsContentLabel );
    layout->setAlignment( mDetailsContentLabel, Qt::AlignLeft );

    mDetailsLabel = new HbLabel("");
    mDetailsLabel->setFontSpec( HbFontSpec( HbFontSpec::Secondary ) );
    layout->addItem( mDetailsLabel );
    layout->setAlignment( mDetailsLabel, Qt::AlignLeft );

    setLayout( layout );
}
