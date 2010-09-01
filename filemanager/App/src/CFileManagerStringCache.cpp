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
* Description:  Cache for strings
*
*/


// INCLUDES
#include <AknUtils.h>
#include <StringLoader.h>
#include <filemanager.rsg>
#include <CFileManagerEngine.h>
#include <CFileManagerItemProperties.h>
#include <CFileManagerCommonDefinitions.h>
#include <CFileManagerFeatureManager.h>
#include "CFileManagerStringCache.h"
#include "CFileManagerIconArray.h"

// CONSTANTS
const TInt KFmgrMaximumSizeOfIconIdString = 3;
_LIT( KIconStr, "%d" );
const TInt KMaxSizeString = 16;
const TInt64 KKileByte = 1024;
const TInt64 KMegaByte = 1048576;
const TInt64 KGigaByte = 1073741824;
const TInt64 KKileByteLowLimit = 100; // 100 bytes
const TInt64 KMegaByteLowLimit = 1048576; // 1MB
const TInt64 KGigaByteLowLimit = 1073741824; // 1GB
const TInt KSizeDecimals = 1;
const TInt KSizeTextArrayLen = 2;

_LIT( KDateFormat1, "%1" );
_LIT( KDateFormat2, "%2" );
_LIT( KDateFormat3, "%3" );
const TUint KSecondSeparator = 1;
const TUint KThirdSeparator = 2;
const TInt KDateStrMax = 20;


// ============================ LOCAL FUNCTIONS ================================

// -----------------------------------------------------------------------------
// GetUnitAndFormatSize
// -----------------------------------------------------------------------------
//
static TInt GetUnitAndFormatSize( TDes& aDes, const TInt64& aSize )
    {
    TRealFormat sizeFormat( aDes.MaxLength(), KSizeDecimals );
    sizeFormat.iType |= KDoNotUseTriads;
    TInt ret( R_QTN_FMGR_UNITS_BYTE );
    if ( aSize >= KGigaByteLowLimit )
        {
        // Format in GBs with decimals
        TReal size( I64REAL( aSize ) / I64REAL( KGigaByte ) );
        aDes.AppendNum( size, sizeFormat );
        ret = R_QTN_FMGR_UNITS_GIGABYTE;
        }
    else if ( aSize >= KMegaByteLowLimit )
        {
        // Format in MBs with decimals
        TReal size( I64REAL( aSize ) / I64REAL( KMegaByte ) );
        aDes.AppendNum( size, sizeFormat );
        ret = R_QTN_FMGR_UNITS_MEGABYTE;
        }
    else if ( aSize >= KKileByteLowLimit )
        {
        // Format in kBs with decimals
        TReal size( I64REAL( aSize ) / I64REAL( KKileByte ) );
        aDes.AppendNum( size, sizeFormat );
        ret = R_QTN_FMGR_UNITS_KILOBYTE;
        }
    else
        {
        // Format in bytes
        aDes.AppendNum( aSize );
        }
    AknTextUtils::LanguageSpecificNumberConversion( aDes );
    return ret;
    }

// -----------------------------------------------------------------------------
// GetFreeSpaceStringL
// -----------------------------------------------------------------------------
//
static HBufC* GetFreeSpaceStringL( const TInt64& aSize )
    {
    CDesCArray* sizeTexts = new ( ELeave ) CDesCArrayFlat( KSizeTextArrayLen );
    CleanupStack::PushL( sizeTexts );
    TBuf< KMaxSizeString > size;
    HBufC* unit = StringLoader::LoadLC( GetUnitAndFormatSize( size, aSize ) );
    sizeTexts->AppendL( size );
    sizeTexts->AppendL( *unit );
    CleanupStack::PopAndDestroy( unit );
    HBufC* ret = StringLoader::LoadL( R_QTN_FMGR_FREE_MEMORY_VAR_UNITS,
        *sizeTexts );
    CleanupStack::PopAndDestroy( sizeTexts );
    return ret;
    }

// -----------------------------------------------------------------------------
// DateStringL
// -----------------------------------------------------------------------------
//
static HBufC* DateStringL( const TTime& aTime )
    {
    TBuf< KDateStrMax > dateStr;
    TBuf< KDateStrMax > dateStrFormat;
    TLocale local;
    dateStrFormat.Append( KDateFormat1 );
    dateStrFormat.Append( local.DateSeparator( KSecondSeparator ) );
    dateStrFormat.Append( KDateFormat2 );
    dateStrFormat.Append( local.DateSeparator( KThirdSeparator ) );
    dateStrFormat.Append( KDateFormat3 );
    aTime.FormatL( dateStr, dateStrFormat );
    return dateStr.AllocL();
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerStringCache::CFileManagerStringCache
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerStringCache::CFileManagerStringCache(
        CFileManagerEngine& aEngine,
        CFileManagerIconArray& aIconArray ) :
    iEngine( aEngine ),
    iIconArray( aIconArray )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerStringCache::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerStringCache* CFileManagerStringCache::NewL(
        CFileManagerEngine& aEngine,
        CFileManagerIconArray& aIconArray )
    {
    CFileManagerStringCache* self = new( ELeave )
        CFileManagerStringCache( aEngine, aIconArray );
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerStringCache::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerStringCache::ConstructL()
    {
    Clear();
    }

// -----------------------------------------------------------------------------
// CFileManagerStringCache::~CFileManagerStringCache
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerStringCache::~CFileManagerStringCache()
    {
    Clear();
    }

// -----------------------------------------------------------------------------
// CFileManagerStringCache::MdcaCount
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerStringCache::MdcaCount() const
    {
    return iEngine.FileList()->MdcaCount();
    }
        
// -----------------------------------------------------------------------------
// CFileManagerStringCache::MdcaPoint
// 
// -----------------------------------------------------------------------------
// 
TPtrC CFileManagerStringCache::MdcaPoint( TInt aIndex ) const
    {
    TInt err( KErrNone );
    TInt index( FormattedStringFound( aIndex ) );

    if ( index >= 0 )
        {
        return FormattedString( index );
        }

    iFormatString.Zero();

    // Get icon
    TInt iconId( EFileManagerOtherFileIcon );
    TRAP( err, iconId = iEngine.IconIdL( aIndex ) );
    if ( err != KErrNone )
        {
        iconId = EFileManagerOtherFileIcon;
        SetError( err );
        }
    // Get name
    CFileManagerItemProperties* prop = NULL;
    TRAP( err, prop = iEngine.GetItemInfoL( aIndex ) );
    if ( err == KErrNone )
        {
        TPtrC ptr( prop->LocalizedName() );
        if ( iconId != EFileManagerFolderIcon &&
             iconId != EFileManagerFolderSubIcon &&
             iconId != EFileManagerFolderEmptyIcon )
            {
            // Append file name with conversion
            HBufC* buffer = NULL;
            TRAP( err,  buffer = AknTextUtils::ConvertFileNameL( ptr ) );
            if ( err == KErrNone )
                {
                iFormatString.Append( *buffer );
                }
            else
                {
                // Put file name without any convertion, its better than nothing
                iFormatString.Append( ptr );
                SetError( err );
                }
            delete buffer;
            }
        else
            {
            // Append folder name
            iFormatString.Append( ptr );
            }
        }
    else
        {
        SetError( err );
        }
    // Remove all other possible tabs in string than the icon separator,
    // so that listbox won't get broken
    AknTextUtils::StripCharacters( iFormatString, KFmgrTab );

    AknTextUtils::ReplaceCharacters( iFormatString, KFmgrLineFeed, KFmgrSpace()[0] );
    AknTextUtils::ReplaceCharacters( iFormatString, KFmgrParagraphSeparator, KFmgrSpace()[0] );

    TInt iconIndex( iIconArray.FindIcon( iconId ) );
    if ( iconIndex >= 0 )
        {
        iFormatString.Insert( 0, KFmgrTab );
        TBuf<KFmgrMaximumSizeOfIconIdString> numBuf;
        numBuf.Copy( KIconStr );
        numBuf.Format( KIconStr, iconIndex );
        iFormatString.Insert( 0, numBuf );
        }
    else
        {
        SetError( iconIndex );
        }

    if ( err == KErrNone && prop && prop->IsDrive() )
        {
        TFileManagerDriveInfo drvInfo;
        TRAP( err, iEngine.GetDriveInfoL( prop->DriveId(), drvInfo ) );
        if ( err == KErrNone )
            {
            // Show free space for accessible local drives
            if ( ( drvInfo.iState & TFileManagerDriveInfo::EDrivePresent ) &&
                 !( drvInfo.iState & ( TFileManagerDriveInfo::EDriveRemote |
                                       TFileManagerDriveInfo::EDriveCorrupted |
                                       TFileManagerDriveInfo::EDriveLocked |
                                       TFileManagerDriveInfo::EDriveInUse ) ) )
                {
                HBufC* freeSpace = NULL;
                TRAP( err, freeSpace = GetFreeSpaceStringL( drvInfo.iSpaceFree ) );
                if ( err == KErrNone )
                    {
                    iFormatString.Append( KFmgrTab );
                    iFormatString.Append( *freeSpace );
                    }
                else
                    {
                    SetError( err );
                    }
                delete freeSpace;
                }
            // Show connection icon for connected remote drives
            else if ( ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote ) &&
                      ( drvInfo.iState & TFileManagerDriveInfo::EDriveConnected ) )
                {
                iconIndex = iIconArray.FindIcon(
                    EFileManagerRemoteDriveConnectedIcon );
                if ( iconIndex >= 0 )
                    {
                    iFormatString.Append( KFmgrTab );
                    iFormatString.Append( KFmgrTab );
                    iFormatString.AppendNum( iconIndex );
                    }
                else if ( iconIndex != KErrNotFound )
                    {
                    SetError( iconIndex );
                    }
                }
            }
        }
    else if ( err == KErrNone && prop && !prop->FullPath().Length() )
        {
        // Show the latest backup date
        TTime time( 0 );
        if( prop->ModifiedLocalDate( time ) == KErrNone )
            {
            HBufC* date = NULL;
            if ( !(time.DateTime().Year()) )
            	{
            	iFormatString.Append( KFmgrTab );
            	iFormatString.Append( KNullDesC );
            	}
            else
            	{
	            
	            TRAPD( err2, date = DateStringL( time ) );
	            if ( err2 == KErrNone )
	                {
	                iFormatString.Append( KFmgrTab );
	                iFormatString.Append( *date );
	                delete date;
	                }
            	}
            }
        }

    delete prop;

    TRAP( err, StoreFormattedStringL( aIndex ) );
    if( err != KErrNone )
        {
        Clear();
        SetError( err );
        }

    return iFormatString;
    }

// -----------------------------------------------------------------------------
// CFileManagerStringCache::Clear
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerStringCache::Clear() const
    {
    for ( TInt i( 0 ); i < KNumStringCacheItems; i++ )
        {
        Clear( i );
        }
    iError = KErrNone;
    }

// -----------------------------------------------------------------------------
// CFileManagerStringCache::FormattedStringFound
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerStringCache::FormattedStringFound( TInt aIndex ) const
    {
    for ( TInt i( 0 ); i < KNumStringCacheItems; i++ )
        {
        if ( iFormattedItems[ i ].iIndex == aIndex )
            {
            return i;
            }
        }
    return KErrNotFound;
    }

// -----------------------------------------------------------------------------
// CFileManagerStringCache::FormattedString
// 
// -----------------------------------------------------------------------------
// 
TPtrC CFileManagerStringCache::FormattedString( TInt aIndex ) const 
    {
    TCacheItem& item = iFormattedItems[ aIndex ];
    if ( item.iString )
        {
        return item.iString->Des();
        }
    return TPtrC( KNullDesC );
    }
 
// -----------------------------------------------------------------------------
// CFileManagerStringCache::StoreFormattedString
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerStringCache::StoreFormattedStringL( TInt aIndex ) const
    {
    TInt count( MdcaCount() );
    TInt replace( 0 );
    TInt maxDist( 0 );

    // Find index to be replaced for cyclic list scroll
    for ( TInt i( 0 ); i < KNumStringCacheItems; i++ )
        {
        TCacheItem& item = iFormattedItems[ i ];
        if ( item.iIndex != KErrNotFound )
            {
            TInt dist( Min( Abs( item.iIndex - aIndex ),
                ( count - aIndex ) + item.iIndex ) );
            if ( dist > maxDist )
                {
                maxDist = dist;
                replace = i;
                }
            }
        else
            {
            replace = i;
            break;
            }
        }
    // Setup new cached item
    Clear( replace );
    TCacheItem& item = iFormattedItems[ replace ];
    item.iString = iFormatString.AllocL();
    item.iIndex = aIndex;
    }

// -----------------------------------------------------------------------------
// CFileManagerStringCache::Clear
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerStringCache::Clear( TInt aIndex ) const
    {
    TCacheItem& item = iFormattedItems[ aIndex ];
    delete item.iString;
    item.iString = NULL;
    item.iIndex = KErrNotFound;
    }

// -----------------------------------------------------------------------------
// CFileManagerStringCache::LastError
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerStringCache::LastError() const
    {
    return iError;
    }

// -----------------------------------------------------------------------------
// CFileManagerStringCache::SetError
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerStringCache::SetError( TInt aError ) const
    {
    if ( aError == KErrNoMemory ||
        ( aError != KErrNone && iError == KErrNone ) )
        {
        iError = aError;
        }
    }

// End of File
