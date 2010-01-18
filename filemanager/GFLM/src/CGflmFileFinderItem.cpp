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
* Description:  File finder item definitions
*
*/



// INCLUDE FILES
#include <e32std.h>
#include <f32file.h>
#include "CGflmFileFinderItem.h"
#include "GflmUtils.h"

// CONSTANTS
const TInt16 KExactMatch = 0;
const TInt16 KBeginMatch = 1;
const TInt16 KNoMatch = KMaxTInt16;


// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// GetSortValue
// -----------------------------------------------------------------------------
//
TInt16 GetSortValue( const TDesC& aName, const TDesC& aSearchString )
    {
    TInt16 ret( 0 );
    TInt pos( 0 );
    if ( GflmUtils::HasWildCard( aSearchString ) )
        {
        pos = aName.MatchC( aSearchString );
        if ( pos != KErrNotFound )
            {
            ret = KBeginMatch + pos;
            }
        else
            {
            ret = KNoMatch;
            }
        }
    else
        {
        pos = aName.FindC( aSearchString );
        if ( !pos && aName.Length() == aSearchString.Length() )
            {
            ret = KExactMatch;
            }
        else if ( pos != KErrNotFound )
            {
            ret = KBeginMatch + pos;
            }
        else
            {
            ret = KNoMatch;
            }
        }
    return ret;
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmFileFinderItem::CGflmFileFinderItem
// -----------------------------------------------------------------------------
//
CGflmFileFinderItem::CGflmFileFinderItem( const TEntry& aEntry ) :
        iEntry( aEntry )
    {
    }

// -----------------------------------------------------------------------------
// CGflmFileFinderItem::NewLC
// -----------------------------------------------------------------------------
//
CGflmFileFinderItem* CGflmFileFinderItem::NewLC(
        const TEntry& aFSEntry,
        const TDesC& aBasePath )
    {
    CGflmFileFinderItem* self = new( ELeave ) CGflmFileFinderItem( aFSEntry );

    CleanupStack::PushL( self );
    self->ConstructL( aBasePath );

    return self;
    }

// -----------------------------------------------------------------------------
// CGflmFileFinderItem::ConstructL
// -----------------------------------------------------------------------------
//
void CGflmFileFinderItem::ConstructL( const TDesC& aBasePath )
    {
    iBasePath = aBasePath.AllocL();
    SetBasePath( *iBasePath );
    SetEntry( iEntry );
    }

// -----------------------------------------------------------------------------
// CGflmFileFinderItem::~CGflmFileFinderItem
// -----------------------------------------------------------------------------
//
CGflmFileFinderItem::~CGflmFileFinderItem()
    {
    delete iBasePath;
    }

// -----------------------------------------------------------------------------
// CGflmFileFinderItem::PrepareSort
// -----------------------------------------------------------------------------
//
void CGflmFileFinderItem::PrepareSort(
        const TDesC& aSearchString )
    {
    iSortValue = GetSortValue( Name(), aSearchString );
    }

// -----------------------------------------------------------------------------
// CGflmFileFinderItem::CompareByMatch
// -----------------------------------------------------------------------------
//
TInt CGflmFileFinderItem::CompareByMatch(
        const CGflmGroupItem& aFirst,
        const CGflmGroupItem& aSecond )
    {
    const CGflmFileFinderItem& first(
        static_cast< const CGflmFileFinderItem& >( aFirst ) );
    const CGflmFileFinderItem& second(
        static_cast< const CGflmFileFinderItem& >( aSecond ) );

    if ( first.iSortValue == second.iSortValue )
        {
        // If both sort value is the same then the directory should be first
        TBool isFirstDir( first.iEntry.IsDir() );
        TBool isSecondDir( second.iEntry.IsDir() );
        if ( isFirstDir && !isSecondDir )
            {
            return -1;
            }
        if ( !isFirstDir && isSecondDir )
            {
            return 1;
            }
        // If even then alphabetical order is used
        return CompareByNameWithoutItemType( aFirst, aSecond );
        }
    if ( first.iSortValue < second.iSortValue )
        {
        return -1;
        }
    return 1;
    }

//  End of File
