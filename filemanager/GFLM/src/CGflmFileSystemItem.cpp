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
* Description:  File system item definitions
*
*/



// INCLUDE FILES
#include "CGflmFileSystemItem.h"
#include "GflmUtils.h"
#include "GFLMConsts.h"
#include <f32file.h>


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::CGflmFileSystemItem
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGflmFileSystemItem::CGflmFileSystemItem(
        const TEntry& aFSEntry,
        const TDesC& aBasePath ) :
    iFSEntry( &aFSEntry ),
    iBasePath( aBasePath )
    {
    SetExt();
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CGflmFileSystemItem* CGflmFileSystemItem::NewLC(
        const TEntry& aFSEntry,
        const TDesC& aBasePath )
    {
    CGflmFileSystemItem* self = new( ELeave ) CGflmFileSystemItem(
        aFSEntry, aBasePath );

    CleanupStack::PushL( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::~CGflmFileSystemItem
//
// -----------------------------------------------------------------------------
//
CGflmFileSystemItem::~CGflmFileSystemItem()
    {
    delete iLocalizedName;
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::Type
//
// -----------------------------------------------------------------------------
//
CGflmGroupItem::TGflmItemType CGflmFileSystemItem::Type() const
    {
    if ( iFSEntry->IsDir() )
        {
        return EDirectory;
        }
    return EFile;
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::Name
//
// -----------------------------------------------------------------------------
//
TPtrC CGflmFileSystemItem::Name() const
    {
    if ( iLocalizedName )
        {
        return iLocalizedName->Des();
        }
    // The entry has no localized name
    return TPtrC( iFSEntry->iName );
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::Date
//
// -----------------------------------------------------------------------------
//
TTime CGflmFileSystemItem::Date() const
    {
    return iFSEntry->iModified;
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::Size
//
// -----------------------------------------------------------------------------
//
TInt64 CGflmFileSystemItem::Size() const
    {
    return static_cast<TUint>( iFSEntry->iSize );
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::Ext
//
// -----------------------------------------------------------------------------
//
TPtrC CGflmFileSystemItem::Ext() const
    {
    return iExt;
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::GetFullPath
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileSystemItem::GetFullPath( TDes& aPath ) const
    {
    GflmUtils::GetFullPath( iBasePath, *iFSEntry, aPath );
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::SetLocalizedNameL
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileSystemItem::SetLocalizedNameL( const TDesC& aName )
    {
    HBufC* temp = aName.AllocL();
    delete iLocalizedName;
    iLocalizedName = temp;
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::Entry
//
// -----------------------------------------------------------------------------
//
EXPORT_C const TEntry& CGflmFileSystemItem::Entry() const
    {
    return *iFSEntry;
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::FullPathLC
//
// -----------------------------------------------------------------------------
//
EXPORT_C HBufC* CGflmFileSystemItem::FullPathLC() const
    {
    return GflmUtils::FullPathLC( iBasePath, *iFSEntry );
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::FullPathL
//
// -----------------------------------------------------------------------------
//
EXPORT_C HBufC* CGflmFileSystemItem::FullPathL() const
    {
    return GflmUtils::FullPathL( iBasePath, *iFSEntry );
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::SetEntry
//
// -----------------------------------------------------------------------------
//
void CGflmFileSystemItem::SetEntry( const TEntry& aEntry )
    {
    iFSEntry = &aEntry;
    SetExt();
    }

// -----------------------------------------------------------------------------
// CGflmFileSystemItem::SetExt
//
// -----------------------------------------------------------------------------
//
void CGflmFileSystemItem::SetExt()
    {
    TPtrC name( iFSEntry->iName );
    TInt pos( name.LocateReverse( KGFLMDot()[ 0 ] ) );
    if( pos != KErrNotFound )
        {
        iExt.Set( name.Mid( pos ) );
        }
    else
        {
        iExt.Set( KNullDesC );
        }
    }

//  End of File
