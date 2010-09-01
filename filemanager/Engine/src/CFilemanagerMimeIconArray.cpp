/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Maps the mime or file extension to icon
*
*/



// INCLUDE FILES
#include <barsread.h>
#include <AknUtils.h> 
#include <coemain.h> 
#include "CFilemanagerMimeIconArray.h"
#include "CGflmNavigatorModel.h"
#include "CDirectoryLocalizer.h"
#include "CFileManagerCommonDefinitions.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerMimeIconArray::NewL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerMimeIconArray* CFileManagerMimeIconArray::NewL(
        TInt aResId, CGflmNavigatorModel& aNavigator )
    {
    CFileManagerMimeIconArray* self = new( ELeave )
        CFileManagerMimeIconArray( aNavigator );

    CleanupStack::PushL( self );
    self->ConstructFromResourceL( aResId );
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerMimeIconArray::CFileManagerMimeIconArray
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerMimeIconArray::CFileManagerMimeIconArray(
        CGflmNavigatorModel& aNavigator )
        : iNavigator( aNavigator )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerMimeIconArray::ConstructFromResourceL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerMimeIconArray::ConstructFromResourceL( TInt aResId )
    {
    TResourceReader reader;
    CCoeEnv::Static()->CreateResourceReaderLC( reader, aResId );
    
    TInt count( reader.ReadInt16() );

    iArray.ReserveL( count );

    for( TInt i( 0 ); i < count; i++ )
        {
        const TPtrC mime( reader.ReadTPtrC() );
        const TInt iconId( reader.ReadInt16() );
        CMimeIconEntry* entry = CMimeIconEntry::NewLC( mime, iconId );
        iArray.AppendL( entry );
        CleanupStack::Pop( entry );
        }

    CleanupStack::PopAndDestroy(); // reader
    }

// -----------------------------------------------------------------------------
// CFileManagerMimeIconArray::~CFileManagerMimeIconArray
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerMimeIconArray::~CFileManagerMimeIconArray()
    {
    iArray.ResetAndDestroy();
    iArray.Close();
    }

// -----------------------------------------------------------------------------
// CFileManagerMimeIconArray::ResolveIconL
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerMimeIconArray::ResolveIconL( const TDesC& aFullPath )
    {
    TPtrC fileType( iNavigator.ResolveMimeTypeL( aFullPath ) );
    TParsePtrC parse( aFullPath );
    TPtrC fileExt( parse.Ext() );
    TInt count( iArray.Count() );

    for( TInt i( 0 ); i < count; ++i )
        {
        CMimeIconEntry* entry = iArray[ i ];
        TPtrC mime( entry->iMime->Des() );
        if ( fileType.FindF( mime ) != KErrNotFound ||
            fileExt.FindF( mime ) != KErrNotFound )
            {
            return entry->iIconId;
            }
        }
    if ( fileType.Length() )
        {
        // File is recognized, but there is no own icon defined for this type.
        // Use note icon to indicate the user that file is not unsupported.
        return EFileManagerNoteFileIcon;
        }
    return EFileManagerOtherFileIcon;
    }

//-------------------------------------------------------------------------------
// CFileManagerMimeIconArray::CMimeIconEntry::CMimeIconEntry
// 
// -----------------------------------------------------------------------------
// 
CFileManagerMimeIconArray::CMimeIconEntry::CMimeIconEntry(
        const TInt aIconId ) :
    iIconId( aIconId )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerMimeIconArray::CMimeIconEntry::~CMimeIconEntry
// 
// -----------------------------------------------------------------------------
// 
CFileManagerMimeIconArray::CMimeIconEntry::~CMimeIconEntry()
    {
    delete iMime;
    }

// -----------------------------------------------------------------------------
// CFileManagerMimeIconArray::CMimeIconEntry::NewLC
// 
// -----------------------------------------------------------------------------
// 
CFileManagerMimeIconArray::CMimeIconEntry*
    CFileManagerMimeIconArray::CMimeIconEntry::NewLC(
        const TDesC& aMime, const TInt aIconId )
    {
    CMimeIconEntry* self = new( ELeave ) CMimeIconEntry( aIconId );
    CleanupStack::PushL( self );
    self->ConstructL( aMime );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerMimeIconArray::CMimeIconEntry::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerMimeIconArray::CMimeIconEntry::ConstructL(
        const TDesC& aMime )
    {
    iMime = aMime.AllocL();
    }

//  End of File  
