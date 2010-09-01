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

// INCLUDE FILES

#include <eikenv.h>
#include <implementationproxy.h>

#include "DevEncUids.hrh"
#include "DevEncPasswdDlg.h"
#include "DevEncLog.h"

// CONSTANTS



// ================= EXPORTED FUNCTIONS =======================================

// ----------------------------------------------------------------------------
//
// Instantiate notifiers
//
// ----------------------------------------------------------------------------

LOCAL_C void CreateNotifiersL( CArrayPtrFlat<MEikSrvNotifierBase2>* aNotifiers )
    {
    CDevEncPasswdDlg* passWdDlg = CDevEncPasswdDlg::NewLC();
    aNotifiers->AppendL( passWdDlg );
    CleanupStack::Pop( passWdDlg );
    }

// ----------------------------------------------------------------------------
//
// Lib main entry point: Creates a notifiers array.
//
// ----------------------------------------------------------------------------

EXPORT_C CArrayPtr<MEikSrvNotifierBase2>* NotifierArray()
    {
    CArrayPtrFlat<MEikSrvNotifierBase2>* notifiers = NULL;
    
    TRAPD( err, notifiers = new (ELeave)CArrayPtrFlat<MEikSrvNotifierBase2>( 1 ));
            
    if ( err == KErrNone )
        {
        if( notifiers )
            {
            TRAPD( err, CreateNotifiersL( notifiers ));
            if( err )
                {
                TInt count = notifiers->Count();
                while(count--)
                    {
                    (*notifiers)[count]->Release();
                    }
                delete notifiers;
                notifiers = NULL;
                }
            }
        }
    else
        {
        DFLOG( "NotifierArray: CArrayPtrFlat: error" );
        }  
        
    return notifiers;
    }

// ----------------------------------------------------------------------------
//
// DLL entry point
//
// ----------------------------------------------------------------------------
#ifndef EKA2
GLDEF_C TInt E32Dll( TDllReason /* aReason */)
    {
    return( KErrNone );
    }
#endif

// ----------------------------------------------------------------------------
//
// ECOM 
//
// ----------------------------------------------------------------------------

const TImplementationProxy ImplementationTable[] =
    {
#ifdef __EABI__
    {{KDevEncNotifPluginImplUid},(TFuncPtr)NotifierArray},
#else
    IMPLEMENTATION_PROXY_ENTRY( KDevEncNotifPluginImplUid, NotifierArray ),
#endif
    };

EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
    return ImplementationTable;
    }

// End of file

