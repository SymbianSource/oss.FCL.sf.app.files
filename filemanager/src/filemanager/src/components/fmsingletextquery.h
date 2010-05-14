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
 *     The header file of the drive rename query of file manager
 */

#ifndef FMSINGLETEXTQUERY_H
#define FMSINGLETEXTQUERY_H

#include "fmdialog.h"
#include <hblineedit.h>

class HbWidget;

class FmSingleTextQuery : public FmDialog
{
Q_OBJECT
public:
    enum Option
        {
            DimPrimereActionWhenEmpty    = 0x00000001
        };
    Q_DECLARE_FLAGS(Options, Option)
    
    FmSingleTextQuery( Options options = 0,
            HbLineEdit::EchoMode echoMode = HbLineEdit::Normal, QGraphicsItem *parent = 0 );
    ~FmSingleTextQuery();

public:
    void setLineEditText( const QString &text );
    QString getLineEditText();
    void setLineEditMaxLength( int length );

public slots:
        void checkActions();

private:
    void init();

private:
    HbWidget *mContentWidget;
    HbLineEdit  *mTextEdit;
    Options mOptions;
    HbLineEdit::EchoMode mEchoMode;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(FmSingleTextQuery::Options)
#endif
