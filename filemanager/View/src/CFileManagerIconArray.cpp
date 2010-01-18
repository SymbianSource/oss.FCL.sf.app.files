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
* Description:  All the files and folders icons are stored here
*
*/


// INCLUDE FILES
#include <FileManagerEngine.hrh>
#include <barsread.h>
#include <coemain.h>
#include <gulicon.h>
#include <AknsItemID.h>
#include <AknsUtils.h>
#include <FileManagerView.rsg>
#include "CFileManagerIconArray.h"
#include "FileManagerView.hrh"



// CONSTANTS
const TInt KGranularity = 1;

// needed because _LIT macro does not expand parameter, which is also macro
#define _CREATE_LIT( a, b ) _LIT( a, b )

_CREATE_LIT( KFileManagerMifFile, filemanager_mbm_file_location );


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerIconArray::CFileManagerIconArray
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerIconArray::CFileManagerIconArray() :
        CArrayPtrFlat( KGranularity )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerIconArray::NewL
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C CFileManagerIconArray* CFileManagerIconArray::NewL()
    {
    CFileManagerIconArray* self = new (ELeave) CFileManagerIconArray();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerIconArray::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerIconArray::ConstructL()
    {
    TResourceReader reader;
    CCoeEnv::Static()->CreateResourceReaderLC(
        reader, R_FILEMANAGER_ICON_ARRAY );

    TInt count( reader.ReadInt16() );
    iIconInfo.ReserveL( count );
    SetReserveL( count );

    // Get File Manager mif drive from exe location
    TFileName exeFileName( RProcess().FileName() );

    // Load icon info now and actual icon data later when needed to decrease
    // construction time
    for( TInt i( 0 ); i < count; i++ )
        {
        TIconInfo info;

        info.iId = reader.ReadInt8();
        info.iFile = reader.ReadHBufCL();
        info.iIconId = reader.ReadInt16();

        TPtr ptr( info.iFile->Des() );
        if ( !ptr.CompareF( KFileManagerMifFile ) )
            {
            // Set correct File Manager mif drive
            ptr[ 0 ] = exeFileName[ 0 ];
            }

        info.iMaskId =  reader.ReadInt16();
        info.iMajorSkinId = reader.ReadInt32();
        info.iMinorSkinId = reader.ReadInt32();
        info.iIconType = reader.ReadInt8();
        info.iIndex = KErrNotFound;

        CleanupStack::PushL( info.iFile );
        iIconInfo.AppendL( info );
        CleanupStack::Pop( info.iFile );
        }

    CleanupStack::PopAndDestroy(); // reader

    LoadMandatoryIconsL();
    }

// -----------------------------------------------------------------------------
// CFileManagerIconArray::~CFileManagerIconArray
// Destructor
// -----------------------------------------------------------------------------
// 
EXPORT_C CFileManagerIconArray::~CFileManagerIconArray()
    {
    TInt count( iIconInfo.Count() );
    for( TInt i( 0 ); i < count; i++ )
        {
        TIconInfo& info = iIconInfo[ i ];
        delete info.iFile;
        info.iFile = NULL;
        }
    iIconInfo.Close();
    }

// -----------------------------------------------------------------------------
// CFileManagerIconArray::UpdateIconsL
//
// -----------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerIconArray::UpdateIconsL()
    {
    // Remove icon data and set info to load icon data again when needed
    ResetAndDestroy();

    TInt count( iIconInfo.Count() );
    SetReserveL( count );

    for( TInt i( 0 ); i < count; i++ )
        {
        TIconInfo& info = iIconInfo[ i ];
        info.iIndex = KErrNotFound;
        }

    LoadMandatoryIconsL();
    }

// -----------------------------------------------------------------------------
// CFileManagerIconArray::FindIcon
//
// -----------------------------------------------------------------------------
// 
EXPORT_C TInt CFileManagerIconArray::FindIcon( TInt aIconId )
    {
    TInt count( iIconInfo.Count() );
    for( TInt i( 0 ); i < count; i++ )
        {
        TIconInfo& info = iIconInfo[ i ];
        if ( info.iId == aIconId )
            {
            if ( info.iIndex == KErrNotFound )
                {
                TRAPD( err, LoadIconL( info ) );
                if ( err != KErrNone )
                    {
                    return err;
                    }
                }
            return info.iIndex;
            }
        }
    return KErrNotFound;
    }

// -----------------------------------------------------------------------------
// CFileManagerIconArray::LoadIconL
//
// -----------------------------------------------------------------------------
// 
void CFileManagerIconArray::LoadIconL( TIconInfo& aInfo )
    {
    CGulIcon* icon = LoadIconL(
        *aInfo.iFile,
        aInfo.iIconId,
        aInfo.iMaskId,
        aInfo.iMajorSkinId,
        aInfo.iMinorSkinId,
        aInfo.iIconType == EFileManagerIconColorIcon );
    CleanupStack::PushL( icon );
    TInt index( Count() );
    AppendL( icon );
    CleanupStack::Pop( icon );
    aInfo.iIndex = index;
    }

// -----------------------------------------------------------------------------
// CFileManagerIconArray::LoadMandatoryIconsL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerIconArray::LoadMandatoryIconsL()
    {
    // At least these icons must be loaded on startup.
    // Other icons can be loaded first time when needed.

    // Make sure that selection icon is always loaded to first index.
    // Required by list control.
    User::LeaveIfError( FindIcon( EFileManagerMarkIcon ) );

    }

// -----------------------------------------------------------------------------
// CFileManagerIconArray::LoadIconL
//
// -----------------------------------------------------------------------------
// 
EXPORT_C CGulIcon* CFileManagerIconArray::LoadIconL(
        const TDesC& aIconFile,
        TInt aIconId,
        TInt aMaskId,
        TInt aMajorSkin,
        TInt aMinorSkin,
        TBool aIsColorIcon )
    {
    TAknsItemID aknsItemId;
    aknsItemId.Set( aMajorSkin, aMinorSkin );
    MAknsSkinInstance* aknsSkin = AknsUtils::SkinInstance();
    CGulIcon* ret = NULL;
    if ( aIsColorIcon )
        {
        TRgb defaultColour( KRgbBlack );
        CFbsBitmap* bmap = NULL;
        CFbsBitmap* mask = NULL;
        AknsUtils::GetCachedColor(
            aknsSkin,
            defaultColour,
            KAknsIIDQsnIconColors,
            EAknsCIQsnIconColorsCG13 );
        AknsUtils::CreateColorIconLC(
            aknsSkin,
            aknsItemId,
            KAknsIIDQsnIconColors,
            EAknsCIQsnIconColorsCG13,
            bmap,
            mask,
            aIconFile,
            aIconId,
            aMaskId,
            defaultColour );
        ret = CGulIcon::NewL( bmap, mask );
        ret->SetBitmapsOwnedExternally( EFalse );
        CleanupStack::Pop( 2 ); // icon owns the bitmaps now
        }
    else
        {
        ret = AknsUtils::CreateGulIconL(
            aknsSkin, aknsItemId, aIconFile, aIconId, aMaskId );
        ret->SetBitmapsOwnedExternally( EFalse );
        }
    return ret;
    }

//  End of File  
