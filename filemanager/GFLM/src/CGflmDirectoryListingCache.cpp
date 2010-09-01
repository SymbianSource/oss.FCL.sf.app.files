/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Provides directory listing cache
*
*/



// INCLUDE FILES
#include "CGflmDirectoryListingCache.h"
#include "GFLM.hrh"
#include "GflmUtils.h"
#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <f32file.h>
#else
#include <f32file.h>
#include <f32file_private.h>
#endif
#include <e32std.h>


// CONSTANTS
const TInt KListingGranularity = 0x2000; // 8KB


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmDirectoryListingCache::CGflmDirectoryListingCache
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGflmDirectoryListingCache::CGflmDirectoryListingCache(
        RFs& aFss,
        const TBool& aCancelIndicator ) :
    iFss( aFss ),
    iCancelIndicator( aCancelIndicator )
    {
    }

// -----------------------------------------------------------------------------
// CGflmDirectoryListingCache::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CGflmDirectoryListingCache* CGflmDirectoryListingCache::NewL(
        RFs& aFss,
        const TBool& aCancelIndicator )
    {
    return new( ELeave ) CGflmDirectoryListingCache(
        aFss, aCancelIndicator );
    }

// -----------------------------------------------------------------------------
// CGflmDirectoryListingCache::~CGflmDirectoryListingCache()
// -----------------------------------------------------------------------------
//
CGflmDirectoryListingCache::~CGflmDirectoryListingCache()
    {
    delete iCache;
    }

// -----------------------------------------------------------------------------
// CGflmDirectoryListingCache::ClearCache()
// -----------------------------------------------------------------------------
//
void CGflmDirectoryListingCache::ClearCache()
    {
    iClearCache = ETrue;
    }

// -----------------------------------------------------------------------------
// CGflmDirectoryListingCache::ListingL()
// Looks if a directory's listing is already loaded and returns it. If the
// the listing is not found, it's read and added to the cache.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
const CGflmDirectoryListingCache::CEntryArray*
    CGflmDirectoryListingCache::ListingL( const TDesC& aDirectory )
    {

    TIMESTAMP( "GFLM listing fetch started: " )

    // Clear directory cache first if needed
    if ( iClearCache )
        {
        delete iCache;
        iCache = NULL;
        iClearCache = EFalse;
        }

    // Check if directory is already cached
    TBool found( EFalse );
    if ( iCache && !aDirectory.Compare( *( iCache->iDirectory ) ) )
        {
        found = ETrue;
        }

    if ( !found )
        {
        // Read directory to cache
        CListingNamePair* newPair = CListingNamePair::NewLC( aDirectory );
        GetDirL( *newPair );
        CleanupStack::Pop( newPair );
        delete iCache;
        iCache = NULL;
        iCache = newPair;
        }

    TIMESTAMP( "GFLM listing fetch ended: " )

    return iCache->iListing;
    }

// -----------------------------------------------------------------------------
// CGflmDirectoryListingCache::GetDirL
// -----------------------------------------------------------------------------
//
void CGflmDirectoryListingCache::GetDirL( CListingNamePair& aPair )
    {
    RDir dir;
    User::LeaveIfError( dir.Open(
        iFss,
        *aPair.iDirectory,
        KEntryAttNormal | KEntryAttDir ) );
    CleanupClosePushL( dir );

    TInt err( KErrNone );

	do
		{
        if ( iCancelIndicator )
            {
            // User has canceled the operation, abort
            User::Leave( KErrCancel );
            }

		err = dir.Read( iEntryBuffer );
		if ( err == KErrNone || err == KErrEof )
			{
			TInt entryCount( iEntryBuffer.Count() );
			if ( !entryCount )
			    {
				break;
				}
            for ( TInt i( 0 ); i < entryCount; i++ )
			    {
                const TEntry& entry( iEntryBuffer[ i ] );
                TInt entrySize( EntrySize( entry, ETrue ) );
                aPair.iListing->AppendL( entry, entrySize );
				}
			}

		}while ( err == KErrNone );

    if ( err != KErrNone && err != KErrEof )
        {
        User::Leave( err );
        }

    CleanupStack::PopAndDestroy( &dir );
    }

// -----------------------------------------------------------------------------
// CGflmDirectoryListingCache::CListingNamePair::~CListingNamePair()
// -----------------------------------------------------------------------------
//
CGflmDirectoryListingCache::CListingNamePair::~CListingNamePair()
    {
    delete iDirectory;

    if ( iListing )
        {
        iListing->Reset();
        delete iListing;
        }
    }

// -----------------------------------------------------------------------------
// CGflmDirectoryListingCache::CListingNamePair::ConstructL()
// -----------------------------------------------------------------------------
//
void CGflmDirectoryListingCache::CListingNamePair::ConstructL(
        const TDesC& aDirectory )
    {
    iDirectory = aDirectory.AllocL();
    iListing = new( ELeave ) CEntryArray( KListingGranularity );
    }

// -----------------------------------------------------------------------------
// CGflmDirectoryListingCache::CListingNamePair::NewLC()
// -----------------------------------------------------------------------------
//
CGflmDirectoryListingCache::CListingNamePair*
    CGflmDirectoryListingCache::CListingNamePair::NewLC(
        const TDesC& aDirectory )
    {
    CListingNamePair* self = new ( ELeave ) CListingNamePair();
    CleanupStack::PushL( self );
    self->ConstructL( aDirectory );
    return self;
    }

//  End of File
