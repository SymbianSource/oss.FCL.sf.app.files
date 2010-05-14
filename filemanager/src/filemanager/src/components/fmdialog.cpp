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
 *      The source file of filemanager base dialog class
 */

#include "fmdialog.h"
#include <hbdialog.h>

FmDialog::FmDialog( QGraphicsItem *parent) : HbDialog( parent ), mRetAction( 0 )
{

}

HbAction *FmDialog::exec()
{
    HbDialog::open( this, SLOT(dialogClosed(HbAction*)) );
    mEventLoop.exec();
    return mRetAction;
}

void FmDialog::dialogClosed(HbAction *action)
{
    mRetAction = action;
    mEventLoop.exit();
}
