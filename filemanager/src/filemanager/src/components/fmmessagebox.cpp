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
 *      The source file of filemanager messagebox
 */

#include "fmmessagebox.h"
#include <hbmessagebox.h>

bool FmMessageBox::question( const QString &questionText, const QString &primaryButtonText,
        const QString &secondaryButtonText )
{
    HbMessageBox::question ( questionText,
            this, SLOT(dialogClosed(HbAction*)), primaryButtonText, secondaryButtonText );
    mEventLoop.exec();
    return mRet;
}

void FmMessageBox::dialogClosed(HbAction *action)
{
    HbMessageBox *dlg = static_cast<HbMessageBox*>(sender());
    if(action == dlg->actions().at(0)) 
    {
        mRet = true;
    }
    else
    {
        mRet = false;
    }
    mEventLoop.exit();
}
