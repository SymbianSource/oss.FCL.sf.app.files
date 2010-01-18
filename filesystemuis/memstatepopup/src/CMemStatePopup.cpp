/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0""
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: Definition for CMemStatePopup
*
*
*/


// SYSTEM INCLUDES
#include <e32std.h>
#include <AknProgressDialog.h>
#include <aknlists.h>
#include <ConeResLoader.h>
#include <bautils.h>
#include <StringLoader.h>
#include <aknPopup.h>
#include <MemStatePopup.rsg>
#include <f32file.h>

// USER INCLUDES
#include "cmemstatepopup.h"
#include "cmemstatepopupimpl.h"
#include "cmemscaneventreceiver.h"

// ---------------------------------------------------------------------------

CMemStatePopup::CMemStatePopup( )
    {
    }

// ---------------------------------------------------------------------------

CMemStatePopup::~CMemStatePopup( )
    {
    }


    
// ---------------------------------------------------------------------------

EXPORT_C void CMemStatePopup::RunLD( TDriveNumber aDrive, TDesC& aTitle )
    {
    CMemStatePopupImpl::RunLD( aDrive, aTitle );
    }

// ---------------------------------------------------------------------------

EXPORT_C void CMemStatePopup::GetUtilL( CMSPUtil*& aUtil )
    {
    CMemStatePopupImpl::GetUtilL( aUtil );
    }

// End of File
