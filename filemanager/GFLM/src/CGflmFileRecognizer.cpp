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
* Description:  Recogniser wrapper
*
*/



// INCLUDE FILES
#include "CGflmFileRecognizer.h"
#include "CGflmDriveResolver.h"
#include "CGflmDriveItem.h"
#include "GflmUtils.h"
#include "GFLMConsts.h"
#include <e32svr.h>
#include <barsread.h>
#include <GflmFileExtensionMimeTypes.rsg>
#include <data_caging_path_literals.hrh> 


// CONSTANTS
_LIT( KFileExtensionResourceFile, "gflmfileextensionmimetypes.rsc" );
const TInt KMimeTypeGranularity = 10;
const TUint KPerCentToDrop = 30;
const TUint KHundredPerCent = 100;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::CGflmFileRecognizer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGflmFileRecognizer::CGflmFileRecognizer(
        CGflmDriveResolver* aDriveResolver,
        RFs& aFs ) :
    iCache( _FOFF( CPathTypePair, iLink ) ),
    iDriveResolver( aDriveResolver ),
    iFs( aFs )
    {
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CGflmFileRecognizer::ConstructL( TInt aMemoryConsumption )
    {
    User::LeaveIfError( iApaSession.Connect() );
    iMimeTypes = new( ELeave ) CDesCArraySeg( KMimeTypeGranularity );
    iCacheMaxMemoryUsage = aMemoryConsumption;

    // Get resource drive from dll location
    TFileName dllFileName;
    Dll::FileName( dllFileName );
    TParsePtrC dllParse( dllFileName );

    TFileName resFileName;
    resFileName.Copy( dllParse.Drive() );
    resFileName.Append( KDC_RESOURCE_FILES_DIR );
    GflmUtils::EnsureFinalBackslash( resFileName );
    resFileName.Append( KFileExtensionResourceFile );

    RResourceFile resFile;
    resFile.OpenL( iFs, resFileName );
    CleanupClosePushL( resFile );
    resFile.ConfirmSignatureL();

    ConstructExtMimePairsL( resFile, R_EXT_MIME_PAIRS, iExtMimePairs );
    ConstructExtMimePairsL(
        resFile, R_REMOTE_EXT_MIME_PAIRS, iRemoteExtMimePairs );

    CleanupStack::PopAndDestroy( &resFile );
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::ConstructExtMimePairsL
//
// -----------------------------------------------------------------------------
//
void CGflmFileRecognizer::ConstructExtMimePairsL(
        RResourceFile& aResFile,
        TInt aResId,
        RPointerArray< CExtMimePair >& aPairs )
    {   
    TCleanupItem cleanupItem( ResetAndDestroyExtMimePairs, &aPairs );
    CleanupStack::PushL( cleanupItem );
    // read the pairs
    HBufC8* resData = aResFile.AllocReadLC( aResId );
    TResourceReader reader;
    reader.SetBuffer( resData );
    TInt count( reader.ReadInt16() );
    aPairs.ReserveL( count );
    for ( TInt i( 0 ); i < count; i++ )
        {
        TPtrC ext( reader.ReadTPtrC() );
        TPtrC mime( reader.ReadTPtrC() );
        CExtMimePair* pair = new (ELeave) CExtMimePair();
        CleanupStack::PushL( pair );
        pair->iExt = ext.AllocL();
        pair->iMime = mime.AllocL();
        aPairs.AppendL( pair );
        CleanupStack::Pop( pair );
        }
    CleanupStack::PopAndDestroy( resData );
    CleanupStack::Pop( &aPairs );
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::FindMimeFromExt
//
// -----------------------------------------------------------------------------
//
TPtrC CGflmFileRecognizer::FindMimeFromExt(
        const TDesC& aExt,
        RPointerArray< CExtMimePair >& aPairs )
    {
    TInt count( aPairs.Count() );

    for( TInt i( 0 ); i < count; i++)
        {
        CExtMimePair* pair = aPairs[ i ];
        if( !aExt.CompareF( *pair->iExt ) )
            {
            return TPtrC( *pair->iMime );
            }
        }
    return TPtrC( KNullDesC );
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CGflmFileRecognizer* CGflmFileRecognizer::NewL(
        RFs& aFs,
        TInt aMemoryConsumption,
        CGflmDriveResolver* aDriveResolver )
    {
    CGflmFileRecognizer* self =
        new( ELeave ) CGflmFileRecognizer( aDriveResolver, aFs );

    CleanupStack::PushL( self );
    self->ConstructL( aMemoryConsumption );
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::~CGflmFileRecognizer
//
// -----------------------------------------------------------------------------
//
CGflmFileRecognizer::~CGflmFileRecognizer()
    {
    iApaSession.Close();

    FlushCache();

    delete iMimeTypes;

    iExtMimePairs.ResetAndDestroy();
    iExtMimePairs.Close();

    iRemoteExtMimePairs.ResetAndDestroy();
    iRemoteExtMimePairs.Close();
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::RecognizeL( const TDesC& aFilename )
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TPtrC CGflmFileRecognizer::RecognizeL( const TDesC& aFilename )
    {
    TPtrC mimeType;
    TRAPD( err, mimeType.Set( DoRecognizeL( aFilename ) ) );
    if ( err == KErrNoMemory )
        {
        FlushCache();
        return DoRecognizeL( aFilename );
        }
    User::LeaveIfError( err );
    return mimeType;
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::DoRecognizeL( const TDesC& aFilename )
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TPtrC CGflmFileRecognizer::DoRecognizeL( const TDesC& aFilename )
    {
    // Search the cache for a hit
    TInt cropPoint( aFilename.LocateReverse( KGFLMDot()[ 0 ] ) );
    TBool isRemoteDrive( EFalse );

    if ( iDriveResolver && iDriveResolver->IsRemoteDrive( aFilename ) )
        {
        isRemoteDrive = ETrue;
        }

    if( cropPoint != KErrNotFound )
        {        
        TPtrC ext( aFilename.Mid( cropPoint ) );
        TPtrC mime( FindMimeFromExt( ext, iExtMimePairs ) );
        if ( mime.Length() )
            {
            return mime;
            }

        // Use remote drive specific extension mime pairs and skip
        // content recognition because it is so slow for remote drives.
        // Mime types are resolved from extension by remote storage framework
        // anyway.
        if ( isRemoteDrive)
            {
            return FindMimeFromExt( ext, iRemoteExtMimePairs );
            }
        }
    else if ( isRemoteDrive)
        {
        // Skip content recognition for remote drives because it is so slow
        return TPtrC( KNullDesC );
        }

    TDblQueIter< CPathTypePair > pairIt( iCache );
    pairIt.SetToFirst();
    CPathTypePair* pair = NULL;
    while ( ( pair = pairIt++ ) != NULL )
        {
        TPtrC ptr( *( pair->iFilename ) );
        if( ptr.Length() == aFilename.Length() &&
           !aFilename.CompareF( ptr ) )
            {
            // Cache hit
            // Move item to the first position if not already.
            // When clearing cache, oldest items can be dropped easily.
            pair->iLink.Deque();
            iCache.AddFirst( *pair );

            if ( pair->iTypeIndex != KErrNotFound )
                {
                return iMimeTypes->MdcaPoint( pair->iTypeIndex );
                }
            return TPtrC( KNullDesC );
            }
        }

        // Cache miss
        TUid uid( KNullUid );
        TDataType datatype;

        TIMESTAMP( "GFLM AppForDocument started: " )

        // Acquire the mime-type of the file
        TInt err( iApaSession.AppForDocument( aFilename, uid, datatype ) );

        TIMESTAMP( "GFLM AppForDocument ended: " )

        if ( err != KErrNone )
            {
            // If recognition returns error, handle as unrecognized file
            ERROR_LOG1(
                "CGflmFileRecognizer::DoRecognizeL-AppForDocument=%d",
                err )
            return TPtrC( KNullDesC );
            }

        CPathTypePair* newPair = CPathTypePair::NewLC( aFilename );
        TPtrC mimeType( datatype.Des() );

        INFO_LOG2( "GFLM AppForDocument:file=%S,mime=%S", &aFilename, &mimeType )

        // Run through the mime-type list to find the correct index
        if ( mimeType.Length() )
            {
            TBool isMimeFound( EFalse );
            TInt mimeCount( iMimeTypes->MdcaCount() );
            for ( TInt j( 0 ); j < mimeCount; j++ )
                {
                if( !mimeType.CompareF( iMimeTypes->MdcaPoint( j ) ) )
                    {
                    newPair->iTypeIndex = j;
                    isMimeFound = ETrue;
                    break;
                    }
                }
            // Add mime type to list if not already added
            if ( !isMimeFound )
                {
                iMimeTypes->AppendL( mimeType );
                newPair->iTypeIndex = mimeCount;
                }
            }

        // Cleanup cache and add new pair
        CleanupStack::Pop( newPair );
        CleanupCache();
        iCacheMemoryUsage += newPair->Size();
        iCache.AddFirst( *newPair );

        if ( newPair->iTypeIndex != KErrNotFound )
            {
            return iMimeTypes->MdcaPoint( newPair->iTypeIndex );
            }
        return TPtrC( KNullDesC );
    }


// -----------------------------------------------------------------------------
//  CGflmFileRecognizer::ResetAndDestroyExtMimePairs( )
// 
// -----------------------------------------------------------------------------
//
void CGflmFileRecognizer::ResetAndDestroyExtMimePairs( TAny* aPtr )
    {
    RPointerArray< CExtMimePair >* extMimePairs = 
        static_cast< RPointerArray< CExtMimePair >* >( aPtr );
    extMimePairs->ResetAndDestroy();
    extMimePairs->Close();
    }
 
 
// -----------------------------------------------------------------------------
// CGflmFileRecognizer::FlushCache( )
//
// -----------------------------------------------------------------------------
//
void CGflmFileRecognizer::FlushCache()
    {
    TDblQueIter< CPathTypePair > pairIt( iCache );
    pairIt.SetToFirst();
    CPathTypePair* pair = NULL;
    while ( ( pair = pairIt++ ) != NULL )
        {
        pair->iLink.Deque();
        delete pair;
        }
    iCacheMemoryUsage = 0;
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::CleanupCache( )
//
// -----------------------------------------------------------------------------
//
void CGflmFileRecognizer::CleanupCache( )
    {
    if( iCacheMemoryUsage > iCacheMaxMemoryUsage )
        {
        TInt dropThreshold = ( TInt ) ( ( ( ( TUint ) iCacheMaxMemoryUsage ) *
            KPerCentToDrop ) / KHundredPerCent );
        TInt droppedSize( 0 );

        TDblQueIter< CPathTypePair > pairIt( iCache );
        pairIt.SetToLast();
        CPathTypePair* pair = NULL;
        while ( ( pair = pairIt-- ) != NULL )
            {
            droppedSize += pair->Size();
            pair->iLink.Deque();
            delete pair;
            if ( droppedSize >= dropThreshold )
                {
                break;
                }
            }
        iCacheMemoryUsage -= droppedSize;
        }
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::CPathTypePair::NewLC()
// 
// -----------------------------------------------------------------------------
//
CGflmFileRecognizer::CPathTypePair*
        CGflmFileRecognizer::CPathTypePair::NewLC( const TDesC& aFilename )
    {
    CPathTypePair* self = new ( ELeave ) CPathTypePair();
    CleanupStack::PushL( self );
    self->ConstructL( aFilename );
    return self;
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::CPathTypePair::ConstructL()
// 
// -----------------------------------------------------------------------------
//
void CGflmFileRecognizer::CPathTypePair::ConstructL( const TDesC& aFilename )
    {
    iFilename = aFilename.AllocL();
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::CPathTypePair::CPathTypePair()
// 
// -----------------------------------------------------------------------------
//
CGflmFileRecognizer::CPathTypePair::CPathTypePair() :
        iTypeIndex( KErrNotFound )
    {
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::CPathTypePair::~CPathTypePair()
// 
// -----------------------------------------------------------------------------
//
CGflmFileRecognizer::CPathTypePair::~CPathTypePair()
    {
    delete iFilename;
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::CPathTypePair::Size()
// 
// -----------------------------------------------------------------------------
//
TInt CGflmFileRecognizer::CPathTypePair::Size() const
    {
    return ( sizeof( TInt ) + iFilename->Size() + sizeof( TDblQueLink ) );
    }

// -----------------------------------------------------------------------------
// CGflmFileRecognizer::CExtMimePair::~CExtMimePair()
// 
// -----------------------------------------------------------------------------
//
CGflmFileRecognizer::CExtMimePair::~CExtMimePair()
    {
    delete iExt;
    delete iMime;
    }

//  End of File
