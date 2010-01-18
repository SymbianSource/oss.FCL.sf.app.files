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
* Description:  Concrete implementation of MGflmItemGroup interface
*
*/



// INCLUDE FILES
#include "CGflmItemGroupImpl.h"
#include "MGflmItemFilter.h"
#include "CGflmItemLocalizer.h"
#include "CGflmDriveResolver.h"
#include "CGflmFileRecognizer.h"
#include "CGflmDirectoryListingCache.h"
#include "CGflmGlobalActionItem.h"
#include "CGflmFileSystemItem.h"
#include "CGflmDriveItem.h"
#include "GFLMConsts.h"
#include "GFLM.hrh"
#include "GflmUtils.h"
#include <f32file.h>


// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::CGflmItemGroupImpl
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGflmItemGroupImpl::CGflmItemGroupImpl(
        const TInt aId,
        RFs& aFss,
        CGflmDirectoryListingCache& aCache,
        CGflmItemLocalizer& aLocalizer,
        CGflmDriveResolver& aResolver ) :
    iId( aId ),
    iFss( aFss ),
    iListingCache( aCache ),
    iItemLocalizer( aLocalizer ),
    iDriveResolver( aResolver )
    {
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::~CGflmItemGroupImpl
//
// -----------------------------------------------------------------------------
//
CGflmItemGroupImpl::~CGflmItemGroupImpl()
    {
    iVolatileItems.ResetAndDestroy();
    iVolatileItems.Close();

    iStaticItems.ResetAndDestroy();
    iStaticItems.Close();

    iItemReferences.Close();

    delete iDirectory;
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CGflmItemGroupImpl* CGflmItemGroupImpl::NewLC(
        const TInt aId,
        RFs& aFss,
        CGflmDirectoryListingCache& aCache,
        CGflmItemLocalizer& aLocalizer,
        CGflmDriveResolver& aResolver )
    {
    CGflmItemGroupImpl* self = new( ELeave ) CGflmItemGroupImpl(
        aId, aFss, aCache, aLocalizer, aResolver );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::ContructL
// -----------------------------------------------------------------------------
//
void CGflmItemGroupImpl::ConstructL()
    {
    iSortCollationMethod = *Mem::CollationMethodByIndex( 0 );
    iSortCollationMethod.iFlags |=
        TCollationMethod::EIgnoreNone | TCollationMethod::EFoldCase;
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::RefreshL()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CGflmItemGroupImpl::RefreshL(
        MGflmItemFilter* aFilter,
        TGflmSortMethod aSortMethod,
        TGflmRefreshMode aRefreshMode )
    {
    FUNC_LOG

    INFO_LOG2(
        "CGflmItemGroupImpl::RefreshL-aRefreshMode=%d, aSortMethod=%d",
        aRefreshMode, aSortMethod )

    if ( aRefreshMode == ERefreshItems )
        {
        // Reset reference list
        iItemReferences.Reset();

        // Destroy old filesystem items
        iVolatileItems.ResetAndDestroy();

        // Re-read contents of the assigned directories, filter out unwanted
        // items and add rest to the item list
        TIMESTAMP( "GFLM reading and filtering directories started: " )
        PopulateReferenceListL( aFilter );
        TIMESTAMP( "GFLM reading and filtering directories ended: " )
        }

    if ( aRefreshMode == ERefreshItems || aRefreshMode == ERefreshSort )
        {
        if ( !( iInclusion & EDrives ) )
            {
            TIMESTAMP( "GFLM group sorting started: " )
            iItemReferences.Sort( CGflmGroupItem::GetSortL( aSortMethod ) );
            TIMESTAMP( "GFLM group sorting ended: " )
            }
        }
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::PopulateReferenceListL()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CGflmItemGroupImpl::PopulateReferenceListL( MGflmItemFilter* aFilter )
    {
    CGflmGroupItem* item = NULL;
    TInt i( 0 );

    // Add drive items to the reference list
    if ( iInclusion & EDrives )
        {
        if ( iDirectory )
            {
            // Hide drive selection when location inside folder.
            return;
            }

        TInt count( iDriveResolver.DriveCount() );
        for ( i = 0; i < count; i++ )
            {
            // Add directly, items are filtered from drive resolver
            CGflmDriveItem* drvItem = iDriveResolver.DriveAt( i );
            iItemReferences.AppendL( drvItem );
            drvItem->SetGroup( this );
            }
        }

    // Add pointers to the static items (action items) to the
    // reference list
    TInt staticCount( iStaticItems.Count() );

    for ( i = 0; i < staticCount; i++ )
        {
        item = iStaticItems[ i ];
        if ( !aFilter ||
            ( aFilter && aFilter->FilterItemL( item, iId, NULL ) ) )
            {
            iItemReferences.AppendL( item );
            item->SetGroup( this );
            }
        }

    if ( iInclusion & ( EFiles | EDirectories ) )
        {
        if ( iDirectory )
            {
            // Check directory name length validity
            TPtrC directory( iDirectory->Des() );
            TInt dirLen( directory.Length() );
            if ( !dirLen || dirLen > KMaxFileName )
                {
                return;
                }

            // Get the drive where items are located
            CGflmDriveItem* drvItem = iDriveResolver.DriveFromPath(
                directory );

            const CGflmDirectoryListingCache::CEntryArray* dir =
                iListingCache.ListingL( directory );
            TInt entryCount( dir->Count() );

            // Reserve room for all entries at once to avoid overhead
            iVolatileItems.ReserveL( entryCount );
            iItemReferences.ReserveL( iItemReferences.Count() + entryCount );

            // Create buffer for filenames
            HBufC* buffer = HBufC::NewLC( KMaxFileName );
            TPtr filename( buffer->Des() );

            TIMESTAMP( "GFLM files and folders list genaration started: " )

            TInt bsLen( KGFLMBackslash().Length() );

            // For all items in a directory listing
            for( i = 0; i < entryCount; i++ )
                {
                const TEntry& entry( ( *dir )[ i ] );
                TBool isDir( entry.IsDir() );
                TInt extLen( isDir ? bsLen : 0 ); // Handle final backslash
                if ( dirLen + entry.iName.Length() + extLen > KMaxFileName )
                    {
                    continue; // Too long name
                    }
                if ( isDir && !( iInclusion & EDirectories ) )
                    {
                    continue; // No directories allowed
                    }
                if ( !isDir && !( iInclusion & EFiles ) )
                    {
                    continue; // No files allowed
                    }

                // Create a new filesystem item
                CGflmFileSystemItem* fsItem = CGflmFileSystemItem::NewLC(
                    entry, directory );

                // Filter out unwanted items
                if ( !aFilter ||
                    ( aFilter && aFilter->FilterItemL( fsItem, iId, drvItem ) ) )
                    {
                    // Only directory items can be localized
                    if ( isDir )
                        {
                        // Get localized name for the item, if it has one
                        GflmUtils::GetFullPath( directory, entry, filename );
                        TPtrC localizedName(
                            iItemLocalizer.LocalizeFromWorkThread( filename ) );
                        if ( localizedName.Length() )
                            {
                            fsItem->SetLocalizedNameL( localizedName );
                            }
                        }
                    iVolatileItems.AppendL( fsItem );
                    CleanupStack::Pop( fsItem );
                    iItemReferences.AppendL( fsItem );
                    fsItem->SetGroup( this );
                    }
                else
                    {
                    CleanupStack::PopAndDestroy( fsItem );
                    }
                }

            CleanupStack::PopAndDestroy( buffer );

            TIMESTAMP( "GFLM files and folders list genaration ended: " )

            }
        }

    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::ItemCount()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CGflmItemGroupImpl::ItemCount() const
    {
    return iItemReferences.Count();
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::Item()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
CGflmGroupItem* CGflmItemGroupImpl::Item( TInt aIndex )
    {
    return iItemReferences[ aIndex ];
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::Id()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CGflmItemGroupImpl::Id() const
    {
    return iId;
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::SetInclusion()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CGflmItemGroupImpl::SetInclusion( TUint aInclusion )
    {
    iInclusion = aInclusion;
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::AddSourceL()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CGflmItemGroupImpl::AddSourceL( const TDesC& aDirectory )
    {
    HBufC* temp = aDirectory.AllocL();
    delete iDirectory;
    iDirectory = temp;
    // Reset reference list to cleanup all references to deleted directory
    iItemReferences.Reset();
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::ResetSources()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CGflmItemGroupImpl::ResetSources()
    {
    delete iDirectory;
    iDirectory = NULL;
    // Reset reference list to cleanup all references to deleted directory
    iItemReferences.Reset();
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::AddActionItemL
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CGflmItemGroupImpl::AddActionItemL( TInt aId, const TDesC& aCaption )
    {
    CGflmGlobalActionItem* actionItem =
        CGflmGlobalActionItem::NewLC( aId, aCaption );
    iStaticItems.AppendL( actionItem );
    CleanupStack::Pop( actionItem );
    }

// -----------------------------------------------------------------------------
// CGflmItemGroupImpl::CollationMethod
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
const TCollationMethod* CGflmItemGroupImpl::CollationMethod()
    {
    return &iSortCollationMethod;
    }

//  End of File
