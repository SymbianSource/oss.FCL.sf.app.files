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
* Description:  Group item definitions
*
*/



// INCLUDE FILES
#include "CGflmGroupItem.h"
#include "MGflmItemGroup.h"
#include <e32std.h>


// CONSTANTS
const TInt KIconIdUndefined = -1;
const TInt KSortCollationLevel = 3;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmGroupItem::CGflmGroupItem
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGflmGroupItem::CGflmGroupItem()
    : iIconId( KIconIdUndefined )
    {
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::~CGflmGroupItem()
// -----------------------------------------------------------------------------
//
CGflmGroupItem::~CGflmGroupItem()
    {
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::GetIconId()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CGflmGroupItem::GetIconId( TInt& aIconId )
    {
    if ( iIconId == KIconIdUndefined )
        {
        aIconId = 0;
        return KErrNotFound;
        }
    aIconId = iIconId;
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::SetIconId()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmGroupItem::SetIconId( const TInt aIconId )
    {
    iIconId = aIconId;
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::CanUseCompareF()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TBool CGflmGroupItem::CanUseCompareF() const    
    {
    if ( iStatus & ECanUseCompareF )
        {
        return ETrue;
        }
    if ( iStatus & ECannotUseCompareF )
        {
        return EFalse;
        }

    TPtrC name( Name() );
    TInt len( name.Length() );
    const TText* ptr = name.Ptr();
    while ( len-- > 0 )
        {
        TText ch = *ptr++;
        // Do not allow CompareF for non Ascii or special chars
        if ( !( ( ch >= 0x000A && ch <= 0x0039 ) ||  // Ascii basic symbols
                ( ch >= 0x0041 && ch <= 0x005A ) ||  // Ascii capital alpha
                ( ch >= 0x0061 && ch <= 0x007A ) ) ) // Ascii small alpha
            {
            iStatus |= ECannotUseCompareF;
            return EFalse;
            }
        }
    iStatus |= ECanUseCompareF;
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::SetGroup()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CGflmGroupItem::SetGroup( MGflmItemGroup* aGroup )
    {
    iGroup = aGroup;
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::GetSortL()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//

TLinearOrder< CGflmGroupItem > CGflmGroupItem::GetSortL(
        TGflmSortMethod aSortMethod )
    {
    switch( aSortMethod )
        {
        case EByName:
            {
            return CompareByName;
            }
        case EByType:
            {
            return CompareByType;
            }
        case EMostRecentFirst:
            {
            return CompareMostRecentFirst;
            }
        case ELargestFirst:
            {
            return CompareLargestFirst;
            }
        default:
            {
            User::Leave( KErrNotSupported );
            }
        }
    return CompareByName;
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::CompareByItemType
// Compares two CGflmGroupItem objects by name (ascending)
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CGflmGroupItem::CompareByItemType(
        const CGflmGroupItem& aFirst, const CGflmGroupItem& aSecond )
    {
    TGflmItemType first( aFirst.Type() );
    TGflmItemType second( aSecond.Type() );
    if ( first < second )
        {
        return -1;
        }
    else if ( first > second )
        {
        return 1;
        }
    return 0;
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::CompareByNameWithoutItemType
// Compares two CGflmGroupItem objects by name (ascending)
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CGflmGroupItem::CompareByNameWithoutItemType(
        const CGflmGroupItem& aFirst, const CGflmGroupItem& aSecond )
    {
    // Do not use CompareC by default with collation
    // because it is very bad for performance when sorting
    // large folders i.e. several 1000s of files with long item names
    // e.g. names containing over 40 chars

    // However, some languages are not sorted correctly with CompareF,
    // so do cached name check to decide what comparison to use.

    if ( !aFirst.CanUseCompareF() || !aSecond.CanUseCompareF() )
        {
        if ( aFirst.iGroup )
            {
            const TCollationMethod* collate =
                aFirst.iGroup->CollationMethod();
            if ( collate )
                {
                return aFirst.Name().CompareC(
                    aSecond.Name(),
                    KSortCollationLevel,
                    collate );
                }
            }
        }

    return aFirst.Name().CompareF( aSecond.Name() );
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::CompareByName
// Compares two CGflmGroupItem objects by name (ascending)
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CGflmGroupItem::CompareByName(
        const CGflmGroupItem& aFirst, const CGflmGroupItem& aSecond )
    {
    TInt ret( CompareByItemType( aFirst, aSecond ) );
    if ( !ret )
        {
        return CompareByNameWithoutItemType( aFirst, aSecond );
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::CompareByType
// Compares two CGflmGroupItem objects by file extension (ascending)
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CGflmGroupItem::CompareByType(
        const CGflmGroupItem& aFirst, const CGflmGroupItem& aSecond )
    {
    TInt ret( CompareByItemType( aFirst, aSecond ) );
    if ( ret )
        {
        return ret;
        }

    // Do not use CompareC by default with collation
    // because it is very bad for performance when sorting
    // large folders i.e. several 1000s of files with long item names
    // e.g. names containing over 40 chars

    // However, some languages are not sorted correctly with CompareF,
    // so do cached name check to decide what comparison to use.

    if ( !aFirst.CanUseCompareF() || !aSecond.CanUseCompareF() )
        {
        if ( aFirst.iGroup )
            {
            const TCollationMethod* collate =
                aFirst.iGroup->CollationMethod();
            if ( collate )
                {
                ret = aFirst.Ext().CompareC(
                    aSecond.Ext(),
                    KSortCollationLevel,
                    collate );
                if  ( !ret )
                    {
                    ret = CompareByName( aFirst, aSecond );
                    }
                return ret;
                }
            }
        }

    ret = aFirst.Ext().CompareF( aSecond.Ext() );
    if  ( !ret )
        {
        ret = CompareByNameWithoutItemType( aFirst, aSecond );
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::CompareMostRecentFirst
// Compares two CGflmGroupItem objects by date (ascending)
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CGflmGroupItem::CompareMostRecentFirst(
        const CGflmGroupItem& aFirst, const CGflmGroupItem& aSecond )
    {
    TInt ret( -CompareByItemType( aFirst, aSecond ) );
    if ( ret )
        {
        return ret;
        }

    TTime first( aFirst.Date() );
    TTime second( aSecond.Date() );

    if ( first > second )
        {
        return -1;
        }
    else if ( first < second )
        {
        return 1;
        }
    return CompareByNameWithoutItemType( aFirst, aSecond );
    }

// -----------------------------------------------------------------------------
// CGflmGroupItem::CompareLargestFirst
// Compares two CGflmGroupItem objects by size (ascending)
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CGflmGroupItem::CompareLargestFirst(
        const CGflmGroupItem& aFirst, const CGflmGroupItem& aSecond )
    {
    TInt ret( -CompareByItemType( aFirst, aSecond ) );
    if ( ret )
        {
        return ret;
        }

    TInt64 first( aFirst.Size() );
    TInt64 second( aSecond.Size() );

    if ( first > second )
        {
        return -1;
        }
    else if ( first < second )
        {
        return 1;
        }
    return CompareByNameWithoutItemType( aFirst, aSecond );
    }

//  End of File
