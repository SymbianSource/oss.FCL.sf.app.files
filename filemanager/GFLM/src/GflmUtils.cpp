/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Static utility class for GFLM
*
*/



// INCLUDE FILES
#include "GflmUtils.h"
#include "GFLMConsts.h"
#include <e32def.h>
#include <e32svr.h>
#include <f32file.h>
#include <collate.h>

// CONSTANTS
_LIT( KWildCard, "*" );
_LIT( KWildCard2, "?" );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// GflmUtils::LocateReverseNth()
// -----------------------------------------------------------------------------
//
TInt GflmUtils::LocateReverseNth(
        const TDesC& aDescriptor,
        TChar aChar,
        const TInt aNth )
    {
    TPtrC ptr( aDescriptor );
    TInt ret( KErrArgument );
    for ( TInt i( 0 ); i < aNth; i++ )
        {
        ret = ptr.LocateReverse( aChar );
        if ( ret == KErrNotFound )
            {
            break;
            }
        ptr.Set( ptr.Left( ret ) );
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// GflmUtils::EnsureFinalBackslash()
// -----------------------------------------------------------------------------
//
EXPORT_C void GflmUtils::EnsureFinalBackslash( TDes& aPath )
    {
    if ( !HasFinalBackslash( aPath ) )
        {
        aPath.Append( KGFLMBackslash );
        }
    }

// -----------------------------------------------------------------------------
// GflmUtils::StripFinalBackslash()
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC GflmUtils::StripFinalBackslash( const TDesC& aPath )
    {
    return StripFinal( aPath, KGFLMBackslash );
    }

// -----------------------------------------------------------------------------
// GflmUtils::HasFinalBackslash
// -----------------------------------------------------------------------------
//
EXPORT_C TBool GflmUtils::HasFinalBackslash( const TDesC& aPath )
    {
    TPtrC ptr( aPath.Right( KGFLMBackslash().Length() ) );
    if ( KGFLMBackslash().Compare( ptr ) )
        {
        return EFalse;
        }
    return ETrue;
    }

// -----------------------------------------------------------------------------
// GflmUtils::GetFullPath
// -----------------------------------------------------------------------------
//
EXPORT_C void GflmUtils::GetFullPath(
        const TDesC& aDir, const TEntry& aEntry, TDes& aFullPath )
    {
    aFullPath.Copy( aDir );
    EnsureFinalBackslash( aFullPath );
    aFullPath.Append( aEntry.iName );
    if ( aEntry.IsDir() )
        {
        aFullPath.Append( KGFLMBackslash );
        }
    }

// -----------------------------------------------------------------------------
// GflmUtils::FullPathLC
// -----------------------------------------------------------------------------
//
EXPORT_C HBufC* GflmUtils::FullPathLC(
        const TDesC& aDir, const TEntry& aEntry )
    {
    HBufC* fullPath = FullPathL( aDir, aEntry );
    CleanupStack::PushL( fullPath );
    return fullPath;
    }

// -----------------------------------------------------------------------------
// GflmUtils::FullPathL
// -----------------------------------------------------------------------------
//
EXPORT_C HBufC* GflmUtils::FullPathL(
        const TDesC& aDir, const TEntry& aEntry )
    {
    TInt len( aDir.Length() + aEntry.iName.Length() );
    if ( aEntry.IsDir() )
        {
        len += KGFLMBackslash().Length();
        }
    HBufC* fullPath = HBufC::NewL( len );
    TPtr ptr( fullPath->Des() );
    GetFullPath( aDir, aEntry, ptr );
    return fullPath;
    }

// -----------------------------------------------------------------------------
// GflmUtils::StripFinalDot
// -----------------------------------------------------------------------------
//
TPtrC GflmUtils::StripFinalDot( const TDesC& aPath )
    {
    return StripFinal( aPath, KGFLMDot );
    }

// -----------------------------------------------------------------------------
// GflmUtils::StripFinal
// -----------------------------------------------------------------------------
//
TPtrC GflmUtils::StripFinal(
        const TDesC& aString, const TDesC& aStringToStrip )
    {
    TInt len( aStringToStrip.Length() );
    TPtrC ptr( aString.Right( len ) );

    if( !aStringToStrip.Compare( ptr ) )
        {
        return aString.Left( aString.Length() - len );
        }
    return TPtrC( aString );
    }

// -----------------------------------------------------------------------------
// GflmUtils::HasWildCard
// -----------------------------------------------------------------------------
//
TBool GflmUtils::HasWildCard( const TDesC& aString )
    {
    return ( aString.Find( KWildCard ) != KErrNotFound ||
             aString.Find( KWildCard2 ) != KErrNotFound );
    }

//  End of File  
