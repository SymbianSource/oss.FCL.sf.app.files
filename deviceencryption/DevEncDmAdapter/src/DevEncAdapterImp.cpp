/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Printing Framework DM Adapter.
*
*/

// INCLUDE
#include <implementationproxy.h>
#include "DevEncAdapter.h"
#include "DevEncUids.hrh"

// ----------------------------------------------------------------------------
// ImplementationTable
// Ecom plug-in implementation table
// Status : Draft
// ----------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] = 
    {
#ifdef __EABI__ 
	IMPLEMENTATION_PROXY_ENTRY( KDevEncOmaDmPluginImplUid, CDevEncAdapter::NewL )
#else // !__EABI__
     { { KDevEncOmaDmPluginImplUid }, CDevEncAdapter::NewL }
#endif // __EABI__
    };

// ----------------------------------------------------------------------------
// ImplementationGroupProxy
// Instance of implementation proxy
// (exported)
// Status : Draft
// ----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );

    return ImplementationTable;
    }

// End Of File


