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
* Description:
*     The source file of filemanager view base
*
*/

#include "fmviewbase.h"
#include "fmviewmanager.h"

#include <hbaction.h>

FmViewBase::FmViewBase( TViewType viewType )
{
    mViewType = viewType;
    init();
    QMetaObject::connectSlotsByName( this );
}

FmViewBase::~FmViewBase(void)
{    
}

void FmViewBase::init()
{
    mBackAction = new HbAction( Hb::BackNaviAction, this );
    mBackAction->setObjectName( "backAction" );
    setNavigationAction( mBackAction );
}

void FmViewBase::on_backAction_triggered()
{
    FmViewManager::viewManager()->popViewAndShow();
}

/*!
    called when about to close. used to close some resource before deleted.
 */
void FmViewBase::aboutToClose()
{

}