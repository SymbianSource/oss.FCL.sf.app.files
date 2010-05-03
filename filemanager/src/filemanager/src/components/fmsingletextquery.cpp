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
 *     The source file of the drive rename query of file manager
 */

#include "fmsingletextquery.h"

#include <QGraphicsLinearLayout>

#include <hbwidget.h>
#include <hblineedit.h>
#include <hbaction.h>

FmSingleTextQuery::FmSingleTextQuery( Options options,
    HbLineEdit::EchoMode echoMode, QGraphicsItem *parent  ) :
    HbDialog( parent ), mOptions( options ), mEchoMode( echoMode )
{
    init();
}

FmSingleTextQuery::~FmSingleTextQuery()
{
}

void FmSingleTextQuery::init()
{
    setDismissPolicy( HbPopup::NoDismiss );
    mContentWidget = new HbWidget();
	setContentWidget( mContentWidget );

    QGraphicsLinearLayout *vLayout = new QGraphicsLinearLayout();
    vLayout->setOrientation( Qt::Vertical );

    mTextEdit = new HbLineEdit();
    mTextEdit->setEchoMode( mEchoMode );
    mTextEdit->setFontSpec( HbFontSpec( HbFontSpec::Primary ) );
	vLayout->addItem( mTextEdit );

    mContentWidget->setLayout( vLayout );

    setTimeout( NoTimeout );

    connect( mTextEdit, SIGNAL(contentsChanged()), this, SLOT(checkActions()) );
}

void FmSingleTextQuery::setLineEditText( const QString &text )
{
    mTextEdit->setText( text );
    mTextEdit->setSelection( 0, text.length() );
}

QString FmSingleTextQuery::getLineEditText()
{
    QString text = mTextEdit->text();

    return text;
}

void FmSingleTextQuery::setLineEditMaxLength( int length )
{
    mTextEdit->setMaxLength( length );
}

void FmSingleTextQuery::checkActions()
{
    if( mOptions & DimPrimereActionWhenEmpty ) {
        if( !mTextEdit->text().isEmpty() ){
            this->primaryAction()->setEnabled( true );
        } else {
            this->primaryAction()->setEnabled( false );
        }
    }
}
