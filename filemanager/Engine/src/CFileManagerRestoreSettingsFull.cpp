/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Restore settings
*
*/



// INCLUDE FILES
#include <StringLoader.h>
#include <FileManagerEngine.rsg>
#include <AknUtils.h>
#include "FileManagerEngine.hrh"
#include "CFileManagerEngine.h"
#include "CFileManagerRestoreSettings.h"
#include "CFileManagerBackupSettings.h"
#include "TFileManagerDriveInfo.h"
#include "FileManagerPrivateCRKeys.h"
#include "FileManagerDebug.h"


// CONSTANTS
_LIT( KDateFormat1, "%1" );
_LIT( KDateFormat2, "%2" );
_LIT( KDateFormat3, "%3" );
_LIT( KTimeFormatBefore, " %-B %J:%T" );
_LIT( KTimeFormatAfter, " %J:%T %+B" );
_LIT( KEmptyChar, " " );
const TUint KSecondSeparator = 1;
const TUint KThirdSeparator = 2;
const TInt KTimeStrMax = 20;
_LIT( KSeparator, "\t" );
const TInt KSeparatorSpace = 9;
_LIT( KIconFormat, "%d" );
const TInt KIconIdStrMax = 3;
_LIT( KTimeSeparatorPlaceHolder, ":" );
const TInt KHourMinSeparator = 1;


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::CFileManagerRestoreSettings
// ----------------------------------------------------------------------------
//
CFileManagerRestoreSettings::CFileManagerRestoreSettings(
        CFileManagerEngine& aEngine ) :
    iEngine( aEngine )
    {
    FUNC_LOG
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::~CFileManagerRestoreSettings
// ----------------------------------------------------------------------------
//
CFileManagerRestoreSettings::~CFileManagerRestoreSettings()
    {
    FUNC_LOG

    iList.ResetAndDestroy();
    iList.Close();
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::NewL
// ----------------------------------------------------------------------------
//
CFileManagerRestoreSettings* CFileManagerRestoreSettings::NewL(
        CFileManagerEngine& aEngine )
    {
    CFileManagerRestoreSettings* self =
        new ( ELeave ) CFileManagerRestoreSettings(
            aEngine );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::ConstructL
// ----------------------------------------------------------------------------
//
void CFileManagerRestoreSettings::ConstructL()
    {
    FUNC_LOG

    RefreshL();
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::SetSelection
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerRestoreSettings::SetSelection(
        const TUint64& aSelection )
    {
    iSelection = aSelection;
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::RefreshL
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerRestoreSettings::RefreshL()
    {
    FUNC_LOG
    
    iList.ResetAndDestroy();

    RArray< TInfo > infoArray;
    iEngine.GetRestoreInfoArrayL( infoArray );
    CleanupClosePushL( infoArray );

    TInt count( infoArray.Count() );
    TUint32 mask( EFileManagerBackupContentFirst);
    while ( mask <= EFileManagerBackupContentLast )
        {
        for ( TInt i( 0 ); i < count; ++i )
            {
            TInfo& info( infoArray[ i ] );
            if ( info.iContent & mask )
                {
                CEntry* entry = CreateEntryLC( info );
                iList.AppendL( entry );
                CleanupStack::Pop( entry );
                }
            }
        mask <<= 1;
        }
    CleanupStack::PopAndDestroy( &infoArray );

    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::CEntry::MdcaCount
// ----------------------------------------------------------------------------
//
TInt CFileManagerRestoreSettings::MdcaCount() const
    {
    return iList.Count();
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::CEntry::MdcaPoint
// ----------------------------------------------------------------------------
//
TPtrC CFileManagerRestoreSettings::MdcaPoint( TInt aIndex ) const
    {
    return TPtrC( *( iList[ aIndex ]->iText ) );
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::CEntry::CreateEntryLC
// ----------------------------------------------------------------------------
//
CFileManagerRestoreSettings::CEntry*
    CFileManagerRestoreSettings::CreateEntryLC(
        const TInfo& aInfo )
    {
    TBuf< KIconIdStrMax > iconStr;
    CEntry* entry = new ( ELeave ) CEntry;
    CleanupStack::PushL( entry );

    TInt titleId( CFileManagerBackupSettings::ContentToTextId( aInfo.iContent ) );
    HBufC* title = StringLoader::LoadLC( titleId );
    HBufC* timeAndDate = DateTimeStringLC( aInfo.iTime );
    entry->iText = HBufC::NewL(
        title->Length() + timeAndDate->Length() + KSeparatorSpace );

    // "0\tFirstLabel\tSecondLabel\t0"
    TPtr ptr( entry->iText->Des() );
    iconStr.Copy( KIconFormat );
    iconStr.Format( KIconFormat, EIconCheckBoxOff );
    ptr.Append( iconStr );
    ptr.Append( KSeparator );
    ptr.Append( *title );
    ptr.Append( KSeparator );
    ptr.Append( *timeAndDate );
    ptr.Append( KSeparator );

    TUint32 drvState( 0 );
    if ( iEngine.DriveState( drvState, aInfo.iDrive ) == KErrNone )
        {
        TInt icon( KErrNotFound );
        if ( drvState & TFileManagerDriveInfo::EDriveUsbMemory )
            {
            icon = EIconUsbMemory;
            }
        else if ( drvState & TFileManagerDriveInfo::EDriveRemovable )
            {
            icon = EIconMemoryCard;
            }
        if ( icon >= 0 )
            {
            iconStr.Copy( KIconFormat );
            iconStr.Format( KIconFormat, icon );
            ptr.Append( iconStr );
            }
        }
    entry->iInfo = aInfo;

    CleanupStack::PopAndDestroy( timeAndDate );
    CleanupStack::PopAndDestroy( title );
    return entry;
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::CEntry::~CEntry
// ----------------------------------------------------------------------------
//
CFileManagerRestoreSettings::CEntry::~CEntry()
    {
    delete iText;
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::GetSelectionL
// ----------------------------------------------------------------------------
//
void CFileManagerRestoreSettings::GetSelectionL(
        RArray< TInfo >& aInfoArray ) const
    {
    aInfoArray.Reset();

    TInt count( iList.Count() );

    for ( TInt i( 0 ); i < count; ++i )
        {
        if ( ( ( ( TUint64 ) 1 ) << i ) & iSelection )
            {
            aInfoArray.AppendL( iList[ i ]->iInfo );
            }
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::DateTimeStringLC
// ----------------------------------------------------------------------------
//
HBufC* CFileManagerRestoreSettings::DateTimeStringLC( const TTime& aTime )
    {
    TBuf< KTimeStrMax > dateStr;
    TBuf< KTimeStrMax > dateStrFormat;

    // Localized date separator form
    TLocale local;
    dateStrFormat.Append( KDateFormat1 );
    dateStrFormat.Append( local.DateSeparator( KSecondSeparator ) );
    dateStrFormat.Append( KDateFormat2 );
    dateStrFormat.Append( local.DateSeparator( KThirdSeparator ) );
    dateStrFormat.Append( KDateFormat3 );
    aTime.FormatL( dateStr, dateStrFormat );

    TBuf< KTimeStrMax > timeFormatStr;
    if ( local.AmPmSymbolPosition() == ELocaleBefore )
        {
        timeFormatStr.Copy( KTimeFormatBefore );
        }
    else
        {
        timeFormatStr.Copy( KTimeFormatAfter );
        }

    TChar timeSeparator( local.TimeSeparator( KHourMinSeparator ) );
    AknTextUtils::ReplaceCharacters(
        timeFormatStr, KTimeSeparatorPlaceHolder, timeSeparator );
    TBuf< KTimeStrMax > timeStr;
    aTime.FormatL( timeStr, timeFormatStr );

    HBufC* buf = HBufC::NewLC( dateStr.Length() +
                               KEmptyChar().Length() +
                               timeStr.Length() );
    TPtr ptrBuffer( buf->Des() );
    ptrBuffer.Append( timeStr );
    ptrBuffer.Append( KEmptyChar );
    ptrBuffer.Append( dateStr );

    return buf;
    }

// End of File
