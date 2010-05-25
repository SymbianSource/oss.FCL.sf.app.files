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
* Description:  Popup to show the item information, view info
*
*/



// INCLUDE FILES
#include <aknlists.h>
#include <StringLoader.h>
#include <cmemstatepopup.h>
#include <filemanagerview.rsg>
#include <CFileManagerItemProperties.h>
#include <CFileManagerFeatureManager.h>
#include <drmuihandling.h>
#include "CFileManagerInfoPopup.h"
#include "CFileManagerFullOmaDrmInfo.h"

// CONSTANTS
_LIT( KDateFormat1, "%1" );
_LIT( KDateFormat2, "%2" );
_LIT( KDateFormat3, "%3" );
_LIT( KTimeFormatBefore, "%-B" );
_LIT( KTimeFormatAfter, "%+B" );
_LIT( KTimeFormatHour, "%J" );
_LIT( KTimeFormatMinutes, "%T" );
_LIT( KTimeFormatSpace, " ");

_LIT( KNumberFormat, "%d" );
_LIT( KSeparator, "\t" );
const TUint KDefaultItemCount = 3;
const TUint KMaxInfoPopupWidth = 16; // From AVKON LAF document
const TUint KSecondSeparator = 1;
const TUint KThirdSeparator = 2;
const TInt KNumBufLen = 24;
_LIT( KNumBufFormatString, "%d" );
_LIT( KNumBufSpaceString, " " );


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::CFileManagerInfoPopup
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerInfoPopup::CFileManagerInfoPopup(
        CFileManagerItemProperties& aProperties,
        const CFileManagerFeatureManager& aFeatureManager ) :
    iProperties( aProperties ),
    iFeatureManager( aFeatureManager ),
    iUiHandling( NULL )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::ConstructDataArrayL
// 
// -----------------------------------------------------------------------------
// 
MDesCArray* CFileManagerInfoPopup::ConstructDataArrayL()
    {
    // Get unit utility instance
    CMemStatePopup::GetUtilL( iUtil );

    CDesCArray* array = new( ELeave ) CDesCArrayFlat( KDefaultItemCount );
    CleanupStack::PushL( array );

    HBufC* title = NULL;
    TUint32 itemType( iProperties.TypeL() );
    if( itemType & CFileManagerItemProperties::EFolder )
        {
        title = StringLoader::LoadLC( R_QTN_FMGR_INFO_HEADING_FLDR );
        }
    else if( itemType & CFileManagerItemProperties::ELink )
        {
        title = StringLoader::LoadLC( R_QTN_FMGR_INFO_HEADING_LINK );
        }
    else
        {
        title = StringLoader::LoadLC( R_QTN_FMGR_INFO_HEADING_FILE );
        }
    SetTitleL( *title );
    CleanupStack::PopAndDestroy( title );

    if ( ( itemType & CFileManagerItemProperties::EDrmProtected ) &&
         iFeatureManager.IsDrmFullSupported() )
        {
        CFileManagerFullOmaDrmInfo* drmInfo = 
            CFileManagerFullOmaDrmInfo::NewLC( *array, iProperties.FullPath(), *iCoeEnv );  
        CleanupStack::PopAndDestroy( drmInfo );
        }

    // Name of the file/folder
    HBufC* nameEntry = NameEntryLC();
    array->AppendL( *nameEntry );
    CleanupStack::PopAndDestroy( nameEntry );

    if( itemType & CFileManagerItemProperties::EFile )
        {
        // Type of the file 
        HBufC* type = TypeEntryLC();
        array->AppendL( *type );
        CleanupStack::PopAndDestroy( type );
        }

    // Modification date
    HBufC* date = DateEntryLC();
    array->AppendL( *date );
    CleanupStack::PopAndDestroy( date ); 

    // Modification time
    HBufC* time = TimeEntryLC();
    array->AppendL( *time );
    CleanupStack::PopAndDestroy( time ); 

    // Size of file/folder
    HBufC* size = SizeEntryLC();
    array->AppendL( *size );
    CleanupStack::PopAndDestroy( size );

    TUint32 defaultFolderMask = CFileManagerItemProperties::EFolder |
                                CFileManagerItemProperties::EDefault;
    if( ( itemType & defaultFolderMask ) == defaultFolderMask )
        {
        HBufC* str = DefaultFolderEntryLC();
        array->AppendL( *str );
        CleanupStack::PopAndDestroy( str );
        }

    if( itemType & CFileManagerItemProperties::EFolder )
        {
        HBufC* folders = ResourceStringValueLC( 
            R_QTN_FMGR_INFO_CONTAIN_FLDR, 
            iProperties.FoldersContainedL() );
        array->AppendL( *folders );
        CleanupStack::PopAndDestroy( folders );

        HBufC* files = ResourceStringValueLC( 
            R_QTN_FMGR_INFO_CONTAIN_FILE, 
            iProperties.FilesContainedL()  );
        array->AppendL( *files );
        CleanupStack::PopAndDestroy( files );
        }

    CleanupStack::Pop( array ); 
    return array;
    }

// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerInfoPopup* CFileManagerInfoPopup::NewL(
        CFileManagerItemProperties& aProperties,
        const CFileManagerFeatureManager& aFeatureManager )
    {
    CFileManagerInfoPopup* self = new( ELeave ) CFileManagerInfoPopup(
        aProperties, aFeatureManager );
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFileManagerInfoPopup::ConstructL()
    {
    iUiHandling = DRM::CDrmUiHandling::NewL();
    
    CFileManagerPopupBase::ConstructL();
    }
    
// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::~CFileManagerInfoPopup
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerInfoPopup::~CFileManagerInfoPopup()
    {
    delete iUtil;
    
    delete iUiHandling;
    }

// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::NameEntryLC
// 
// -----------------------------------------------------------------------------
// 
HBufC* CFileManagerInfoPopup::NameEntryLC()
    {
    HBufC* name = StringLoader::LoadLC( R_QTN_SWINS_LSH2_APPS_NAME );
    HBufC* dataStr = HBufC::NewLC(
        name->Length() + 1 + iProperties.LocalizedName().Length() );
    TPtr dataPtr( dataStr->Des() );
    dataPtr.Append( *name );
    dataPtr.Append( KSeparator );
    HBufC* itemNameBuf = NULL;
    if ( iProperties.TypeL() & CFileManagerItemProperties::EFolder )
        {
        const TPtrC ptr( iProperties.LocalizedName() );
        itemNameBuf = ptr.AllocLC();
        }
    else
        {
        const TPtrC ptr( iProperties.Name() );
        itemNameBuf = ptr.AllocLC();
        }
    // Remove all possible tabs in the name so listbox won't get broken
    TPtr itemNamePtr = itemNameBuf->Des();
    AknTextUtils::StripCharacters( itemNamePtr, KSeparator );
    dataPtr.Append( itemNamePtr );
    CleanupStack::PopAndDestroy( itemNameBuf );
    CleanupStack::Pop( dataStr );
    CleanupStack::PopAndDestroy( name );
    CleanupStack::PushL( dataStr );

    return dataStr;
    }

// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::TypeEntryLC
// 
// -----------------------------------------------------------------------------
// 
HBufC* CFileManagerInfoPopup::TypeEntryLC()
    {
    HBufC* type = StringLoader::LoadLC( R_QTN_SWINS_LSH2_APPS_TYPE );
    HBufC* dataStr = HBufC::NewL( type->Length() + 1 + iProperties.Ext().Length() );
    TPtr dataPtr( dataStr->Des() );
    dataPtr.Append( *type );
    dataPtr.Append( KSeparator );
    const TPtrC ptr( iProperties.Ext() );
    HBufC* extBuf = ptr.AllocLC();
    // Remove all possible tabs in the name so listbox won't get broken
    TPtr extPtr = extBuf->Des();
    AknTextUtils::StripCharacters( extPtr, KSeparator );
    dataPtr.Append( extPtr );
    CleanupStack::PopAndDestroy( extBuf );
    CleanupStack::PopAndDestroy( type );
    CleanupStack::PushL( dataStr );

    return dataStr;
    }

// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::DateEntryLC
// 
// -----------------------------------------------------------------------------
// 
HBufC* CFileManagerInfoPopup::DateEntryLC()
    {
    HBufC* date = StringLoader::LoadLC( R_QTN_ALBUM_DATIM_DATE );
    TBuf<KMaxInfoPopupWidth> dateStr;
    TBuf<KMaxInfoPopupWidth> dateStrFormat;
    // Localized date separator form
    TLocale local;
    TTime localTime;
    User::LeaveIfError( iProperties.ModifiedLocalDate( localTime ) );
    dateStrFormat.Append( KDateFormat1 );
    dateStrFormat.Append( local.DateSeparator( KSecondSeparator ) );
    dateStrFormat.Append( KDateFormat2 );
    dateStrFormat.Append( local.DateSeparator( KThirdSeparator ) );
    dateStrFormat.Append( KDateFormat3 );
    localTime.FormatL( dateStr, dateStrFormat );
    HBufC* dataStr = HBufC::NewL( date->Length() + 1 + dateStr.Length() );
    TPtr dataPtr( dataStr->Des() );
    dataPtr.Append( *date );
    dataPtr.Append( KSeparator );
    dataPtr.Append( dateStr );

    CleanupStack::PopAndDestroy( date );
    CleanupStack::PushL( dataStr );

    AknTextUtils::LanguageSpecificNumberConversion( dataPtr );

    return dataStr;
    }

// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::TimeEntryLC
// 
// -----------------------------------------------------------------------------
// 
HBufC* CFileManagerInfoPopup::TimeEntryLC()
    {
    HBufC* time = StringLoader::LoadLC( R_QTN_ALBUM_DATIM_TIME );
    TBuf<KMaxInfoPopupWidth> timeStr;
    TBuf<KMaxInfoPopupWidth> timeStrFormat;
    TLocale local;
    TTime localTime;
    User::LeaveIfError( iProperties.ModifiedLocalDate( localTime ) );

    if( local.AmPmSymbolPosition() == ELocaleBefore )
        {
        timeStrFormat.Append( KTimeFormatBefore );
        timeStrFormat.Append( KTimeFormatSpace );
        timeStrFormat.Append( KTimeFormatHour );
        timeStrFormat.Append( local.TimeSeparator( KSecondSeparator ) );
        timeStrFormat.Append( KTimeFormatMinutes );

        localTime.FormatL( timeStr, timeStrFormat );
        }
    else
        {
        timeStrFormat.Append( KTimeFormatHour );
        timeStrFormat.Append( local.TimeSeparator( KSecondSeparator ) );
        timeStrFormat.Append( KTimeFormatMinutes );
        timeStrFormat.Append( KTimeFormatSpace );
        timeStrFormat.Append( KTimeFormatAfter );

        localTime.FormatL( timeStr, timeStrFormat );
        }

    HBufC* dataStr = HBufC::NewL( time->Length() + 1 + timeStr.Length() );
    TPtr dataPtr( dataStr->Des() );
    dataPtr.Append( *time );
    dataPtr.Append( KSeparator );
    dataPtr.Append( timeStr );

    CleanupStack::PopAndDestroy( time );
    CleanupStack::PushL( dataStr );

    AknTextUtils::LanguageSpecificNumberConversion( dataPtr );

    return dataStr;
    }

// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::SizeEntryLC
// 
// -----------------------------------------------------------------------------
// 
HBufC* CFileManagerInfoPopup::SizeEntryLC()
    {
    TInt64 sizeInBytes( iProperties.SizeL() );
    TPtrC unit( iUtil->SolveUnitAndSize( sizeInBytes ) );
    TBuf< KNumBufLen > numBuf;
    numBuf.Format( KNumBufFormatString, I64INT( sizeInBytes ) );

    HBufC* header = StringLoader::LoadLC( R_QTN_ALBUM_LITXT_SIZE );
    HBufC* dataStr = HBufC::NewL(
        header->Length() + 1 +
        numBuf.Length() + 
        KNumBufSpaceString().Length() +
        unit.Length() );
    TPtr dataPtr( dataStr->Des() );

    dataPtr.Append( *header );
    dataPtr.Append( KSeparator );
    dataPtr.Append( numBuf );
    dataPtr.Append( KNumBufSpaceString );
    dataPtr.Append( unit );

    CleanupStack::PopAndDestroy( header );
    CleanupStack::PushL( dataStr );

    AknTextUtils::LanguageSpecificNumberConversion( dataPtr );
    return dataStr;
    }

// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::DefaultFolderEntryLC
// 
// -----------------------------------------------------------------------------
// 
HBufC* CFileManagerInfoPopup::DefaultFolderEntryLC()
    {
    HBufC* header = StringLoader::LoadLC( R_QTN_FMGR_INFO_FLDR_DEFAULT );
    HBufC* data = StringLoader::LoadLC( R_QTN_FMGR_INFO_DEF_FLDR_TEXT );
    HBufC* dataStr = HBufC::NewL( header->Length() + 1 + data->Length() );
    TPtr dataPtr( dataStr->Des() );
    dataPtr.Append( *header );
    dataPtr.Append( KSeparator );
    dataPtr.Append( *data );

    CleanupStack::PopAndDestroy( data );
    CleanupStack::PopAndDestroy( header );
    CleanupStack::PushL( dataStr );

    return dataStr;
    }

// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::ResourceStringValueLC
// 
// -----------------------------------------------------------------------------
// 
HBufC* CFileManagerInfoPopup::ResourceStringValueLC( TInt aResId, TInt aValue )
    {
    HBufC* string = StringLoader::LoadLC( aResId );
    TBuf<KMaxInfoPopupWidth> infoStr;
    infoStr.Format( KNumberFormat, aValue );
    HBufC* dataStr = HBufC::NewL( string->Length() + 1 + infoStr.Length() );
    TPtr dataPtr( dataStr->Des() );
    dataPtr.Append( *string );
    dataPtr.Append( KSeparator );
    dataPtr.Append( infoStr );

    CleanupStack::PopAndDestroy( string );
    CleanupStack::PushL( dataStr );

    AknTextUtils::LanguageSpecificNumberConversion( dataPtr );

    return dataStr;
    }

#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
// -----------------------------------------------------------------------------
// CFileManagerInfoPopup::ActivateLinkL
// 
// -----------------------------------------------------------------------------
//
void CFileManagerInfoPopup::ActivateLinkL()
    {
    CFileManagerFullOmaDrmInfo::ViewDetailsL(
        iProperties.FullPath(),
        iUiHandling );
    }
#endif //RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

//  End of File  
