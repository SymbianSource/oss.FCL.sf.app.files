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
* Description:   Holds one default folder entry
*
*/



// INCLUDE FILES
#include "CFileManagerFolderEntry.h"
#include <barsread.h>               // TResourceReader
#include <bautils.h>                // BaflUtils::EnsurePathExists
#include <coemain.h>                // CCoeEnv
#include "CFileManagerUtils.h"
#include "CFileManagerCommonDefinitions.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerFolderEntry::NewLC
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFolderEntry* CFileManagerFolderEntry::NewLC(
        TResourceReader& aReader )
    {
    CFileManagerFolderEntry* self = new( ELeave ) CFileManagerFolderEntry;
    CleanupStack::PushL( self );
    self->ConstructFromResourceL( aReader );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderEntry::NewL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFolderEntry* CFileManagerFolderEntry::NewL(
        TResourceReader& aReader )
    {
    CFileManagerFolderEntry* self = CFileManagerFolderEntry::NewLC( aReader );
    CleanupStack::Pop( self );
    
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderEntry::NewLC
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFolderEntry* CFileManagerFolderEntry::NewLC(
        const TInt aDrive, const TDesC& aPath )
    {
    CFileManagerFolderEntry* self = new( ELeave ) CFileManagerFolderEntry;
    CleanupStack::PushL( self );
    self->ConstructL( aDrive, aPath );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderEntry::CFileManagerFolderEntry
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFolderEntry::CFileManagerFolderEntry()
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderEntry::~CFileManagerFolderEntry
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerFolderEntry::~CFileManagerFolderEntry()
    {
    delete iName;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderEntry::ConstructFromResourceL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderEntry::ConstructFromResourceL( TResourceReader& aReader )
    {
    const TPtrC root( aReader.ReadTPtrC() );
    const TPtrC path( aReader.ReadTPtrC() );

    iIsMediaFolder = aReader.ReadInt8();
    iName = HBufC::NewL(
        root.Length() +
        path.Length() +
        KFmgrBackslashSpace +
        KFmgrBackslashSpace );

    TPtr ptr( iName->Des() );
    ptr.Copy( root );
    CFileManagerUtils::EnsureFinalBackslash( ptr );
    ptr.Append( path );
    CFileManagerUtils::EnsureFinalBackslash( ptr );
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderEntry::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderEntry::ConstructL(
        const TInt aDrive, const TDesC& aPath )
    {
    TChar driveLetter;

    User::LeaveIfError( RFs::DriveToChar( aDrive, driveLetter ) );
    iName = aPath.AllocL();
    iName->Des()[ 0 ] = driveLetter;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderEntry::IsMediaFolder
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerFolderEntry::IsMediaFolder() const
    {
    return iIsMediaFolder;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderEntry::Name
// 
// -----------------------------------------------------------------------------
// 
TPtrC CFileManagerFolderEntry::Name() const
    {
    return iName->Des();
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderEntry::CompareSort
// -----------------------------------------------------------------------------
//
TInt CFileManagerFolderEntry::CompareSort(
        const CFileManagerFolderEntry& aFirst,
        const CFileManagerFolderEntry& aSecond )
    {
    return aFirst.iName->Des().CompareF( *( aSecond.iName ) );
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderEntry::CompareFind
// -----------------------------------------------------------------------------
//
TInt CFileManagerFolderEntry::CompareFind(
        const TDesC* aPath,
        const CFileManagerFolderEntry& aItem )
    {
    return aPath->CompareF( *( aItem.iName ) );
    }

//  End of File  
