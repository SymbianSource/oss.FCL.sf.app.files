/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Searches items matching with given string in given file system
*
*/



// INCLUDE FILES
#include <e32std.h>
#include <f32file.h>
#include <bautils.h>
#include "CGflmFileFinder.h"
#include "CGflmFileFinderItem.h"
#include "CGflmDriveItem.h"
#include "CGflmDriveResolver.h"
#include "CGflmItemLocalizer.h"
#include "MGflmItemFilter.h"
#include "GflmUtils.h"

// CONSTANTS
const TInt KWildCardSpace = 2;
_LIT( KWildCard, "*" );
const TInt KSearchResultsGranularity = 32;

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// MakeWildStringL
// -----------------------------------------------------------------------------
//
static HBufC* MakeWildStringL( const TDesC& aString )
    {
    if ( GflmUtils::HasWildCard( aString ) )
        {
        return aString.AllocL();
        }
    HBufC* ret = HBufC::NewL( aString.Length() + KWildCardSpace );
    TPtr ptr( ret->Des() );
    ptr.Copy( KWildCard );
    ptr.Append( aString );
    ptr.Append( KWildCard );
    return ret;
    }

// -----------------------------------------------------------------------------
// MakeWildPathL
// -----------------------------------------------------------------------------
//
#if 0
static HBufC* MakeWildPathL( const TDesC& aFolder, const TDesC& aString )
    {
    const TInt KBackslashSpace = 1;
    if ( !GflmUtils::HasWildCard( aString ) )
        {
        return aFolder.AllocL();
        }
    HBufC* ret = HBufC::NewL(
        aFolder.Length() + aString.Length() + KBackslashSpace );
    TPtr ptr( ret->Des() );
    ptr.Copy( aFolder );
    GflmUtils::EnsureFinalBackslash( ptr );
    ptr.Append( aString );
    return ret;
    }
#endif // 0

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmFileFinder::CGflmFileFinder
// -----------------------------------------------------------------------------
//
CGflmFileFinder::CGflmFileFinder(
            RFs& aFss,
            CGflmItemLocalizer& aLocalizer,
            CGflmDriveResolver& aResolver,
            const TBool& aCancelIndicator ) :
        iFss( aFss ),
        iItemLocalizer( aLocalizer ),
        iDriveResolver( aResolver ),
        iCancelIndicator( aCancelIndicator ),
        iSearchResults( KSearchResultsGranularity )
    {
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::NewL
// -----------------------------------------------------------------------------
//
CGflmFileFinder* CGflmFileFinder::NewL(
        RFs& aFss,
        CGflmItemLocalizer& aLocalizer,
        CGflmDriveResolver& aResolver,
        const TBool& aCancelIndicator )
    {
    CGflmFileFinder* self = new( ELeave ) CGflmFileFinder( 
        aFss, aLocalizer, aResolver, aCancelIndicator );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::ConstructL
// -----------------------------------------------------------------------------
//
void CGflmFileFinder::ConstructL()
    {
    iSortCollationMethod = *Mem::CollationMethodByIndex( 0 );
    iSortCollationMethod.iFlags |=
        TCollationMethod::EIgnoreNone | TCollationMethod::EFoldCase;
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::~CGflmFileFinder
// -----------------------------------------------------------------------------
// 
CGflmFileFinder::~CGflmFileFinder()
    {
    delete iSearchFolder;
    delete iSearchFolderWild;
    delete iSearchString;
    delete iSearchStringWild;
    iSearchResults.ResetAndDestroy();
    iSearchResults.Close();
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::SetSearchFolderL
// -----------------------------------------------------------------------------
// 
void CGflmFileFinder::SetSearchFolderL( const TDesC& aSearchFolder )
    {
    delete iSearchFolderWild;
    iSearchFolderWild = NULL;

    delete iSearchFolder;
    iSearchFolder = NULL;
    iSearchFolder = aSearchFolder.AllocL();
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::SetSearchStringL
// -----------------------------------------------------------------------------
// 
void CGflmFileFinder::SetSearchStringL( const TDesC& aSearchString )
    {
    delete iSearchFolderWild;
    iSearchFolderWild = NULL;

    delete iSearchString;
    iSearchString = NULL;
    iSearchString = aSearchString.AllocL();

    delete iSearchStringWild;
    iSearchStringWild = NULL;
    iSearchStringWild = MakeWildStringL( aSearchString );
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::RefreshL
// -----------------------------------------------------------------------------
// 
void CGflmFileFinder::RefreshL(
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
        TRAPD( err, DoSearchL( aFilter ) );

        // If error just log it and show already found items
        if ( err != KErrNone )
            {
            ERROR_LOG1( "CGflmFileFinder::RefreshL-err=%d", err )
            }
        }

    if ( aSortMethod == EByMatch )
        {
        iSearchResults.Sort( CGflmFileFinderItem::CompareByMatch );
        }
    else
        {
        iSearchResults.Sort( CGflmGroupItem::GetSortL( aSortMethod ) );
        }
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::DoSearchL
// -----------------------------------------------------------------------------
// 
void CGflmFileFinder::DoSearchL( MGflmItemFilter* aFilter )
    {
    FUNC_LOG

    iSearchResults.ResetAndDestroy();

    // Ensure that search data is defined
    if ( !iSearchFolder || !iSearchString || !iSearchStringWild )
        {
        return;
        }
    // Ensure that search drive exists
    CGflmDriveItem* drvItem = iDriveResolver.DriveFromPath( *iSearchFolder );
    if ( !drvItem )
        {
        return;
        }
    if ( !iSearchFolderWild )
        {
#if 0
        if ( iDriveResolver.IsRemoteDrive( *iSearchFolder ) )
            {
            // Some remote drives are case sensitive with wild cards. 
            // Therefore, wild cards cannot be used for remote drives, 
            // because then it is not possible to do case 
            // insensitive comparison like it is done for local drives. 
            iSearchFolderWild = iSearchFolder->AllocL();
            }
        else
            {
            // Do search using the folder with wild string for better performance.
            // It is faster to let file server do the wild matching instead of 
            // reading all subitems under search folder to be matched here.

            // Prepare search folder with wild search string
            iSearchFolderWild = MakeWildPathL(
                *iSearchFolder, *iSearchStringWild );
            }
#else
        // Because of problems with localized folder names, 
        // wild card folder string cannot be given to file server since 
        // file server is not aware of localized names. 
        // This search operation is slower because all items are transferred 
        // here for comparison. 
        iSearchFolderWild = iSearchFolder->AllocL();
#endif
        }

    INFO_LOG1( "CGflmFileFinder::DoSearchL-CDirScan::NewLC-%S",
        iSearchFolderWild )
    CDirScan* dirScan = CDirScan::NewLC( iFss );

    INFO_LOG( "CGflmFileFinder::DoSearchL-CDirScan::SetScanDataL" )
    dirScan->SetScanDataL( *iSearchFolderWild, KEntryAttDir, ESortNone );

    INFO_LOG( "CGflmFileFinder::DoSearchL-CDirScan::NextL" )
    CDir* dir = NULL;
    dirScan->NextL( dir );

    while( dir )
        {
        CleanupStack::PushL( dir );
        if ( iCancelIndicator )
            {
            User::Leave( KErrCancel );
            }
        TPtrC basePath( dirScan->FullPath() );
        INFO_LOG1( "CGflmFileFinder::DoSearchL-Search-%S", &basePath )

        TInt count( dir->Count() );
        for ( TInt i( 0 ); i < count; ++i )
            {
            if ( iCancelIndicator )
                {
                User::Leave( KErrCancel );
                }
            const TEntry& entry( ( *dir )[ i ] );
            TPtrC name( entry.iName );
            TBool isLocalized( EFalse );
            TBool isDir( entry.IsDir() );
            if( isDir )
                {
                // Handle localized name
                GflmUtils::GetFullPath( basePath, entry, iFullPath );
                TPtrC localizedName(
                    iItemLocalizer.LocalizeFromWorkThread( iFullPath ) );
                if ( localizedName.Length() )
                    {
                    name.Set( localizedName );
                    isLocalized = ETrue;
                    }
                }
            // Append matching items
            if( name.MatchC( *iSearchStringWild ) != KErrNotFound )
                {
                CGflmFileFinderItem* finderItem = CGflmFileFinderItem::NewLC(
                    entry, basePath );
                if ( isLocalized )
                    {
                    finderItem->SetLocalizedNameL( name );
                    }
                // Filter out unwanted items
                if ( !aFilter ||
                     ( aFilter && aFilter->FilterItemL(
                        finderItem,
                        isDir ? CGflmGroupItem::EDirectory : CGflmGroupItem::EFile,
                        drvItem ) ) )
                    {
                    finderItem->PrepareSort( *iSearchString );
                    finderItem->SetGroup( this );
                    iSearchResults.AppendL( finderItem );
                    CleanupStack::Pop( finderItem );
                    }
                else
                    {
                    CleanupStack::PopAndDestroy( finderItem );
                    }
                }
            }
        CleanupStack::PopAndDestroy( dir );

        INFO_LOG( "CGflmFileFinder::DoSearchL-CDirScan::NextL" )
        dirScan->NextL( dir );
        }
    CleanupStack::PopAndDestroy( dirScan );
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::ItemCount
// -----------------------------------------------------------------------------
// 
TInt CGflmFileFinder::ItemCount() const
    {
    return iSearchResults.Count();
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::Item
// -----------------------------------------------------------------------------
// 
CGflmGroupItem* CGflmFileFinder::Item( TInt aIndex )
    {
    return iSearchResults[ aIndex ];
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::Id
// -----------------------------------------------------------------------------
// 
TInt CGflmFileFinder::Id() const
    {
    return KErrNotFound;
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::AddSourceL
// -----------------------------------------------------------------------------
// 
void CGflmFileFinder::AddSourceL( const TDesC& /*aDirectory*/ )
    {
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::ResetSources
// -----------------------------------------------------------------------------
// 
void CGflmFileFinder::ResetSources()
    {
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::AddActionItemL
// -----------------------------------------------------------------------------
// 
void CGflmFileFinder::AddActionItemL( TInt /*aId*/, const TDesC& /*aCaption*/ )
    {
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::SetInclusion
// -----------------------------------------------------------------------------
// 
void CGflmFileFinder::SetInclusion( TUint /*aInclusion*/ )
    {
    }

// -----------------------------------------------------------------------------
// CGflmFileFinder::CollationMethod
// -----------------------------------------------------------------------------
// 
const TCollationMethod* CGflmFileFinder::CollationMethod()
    {
    return &iSortCollationMethod;
    }

// End of File
