/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Backup settings
*
*/



// INCLUDE FILES
#include <avkon.rsg>
#include <FileManagerEngine.rsg>
#include <StringLoader.h>
#include <centralrepository.h>
#ifdef RD_MULTIPLE_DRIVE
#include <driveinfo.h>
#endif // RD_MULTIPLE_DRIVE
#include <AknUtils.h>
#include "FileManagerDebug.h"
#include "CFileManagerEngine.h"
#include "FileManagerPrivateCRKeys.h"
#include "CFileManagerBackupSettings.h"
#include "CFileManagerCommonDefinitions.h"

// CONSTANTS
_LIT( KSeparator, "\t" );
const TInt KSeparatorSpace = 3;
_LIT( KTimeFormatBefore, " %-B %J:%T" );
_LIT( KTimeFormatAfter, " %J:%T %+B" );
const TInt KTimeStrMax = 20;

const TInt KSchedulingTextLookup[] = {
    R_QTN_FMGR_BACKUP_NO_SCHEDULING, // CFileManagerBackupSettings::ENoScheduling
    R_QTN_FMGR_BACKUP_DAILY, // CFileManagerBackupSettings::EDaily
    R_QTN_FMGR_BACKUP_WEEKLY // CFileManagerBackupSettings::EWeekly
};

const TInt KMinToMicroSecMultiplier = 60000000; 
const TInt KTargetDriveStrMax = 4;
_LIT( KTimeSeparatorPlaceHolder, ":" );
const TInt KHourMinSeparator = 1;


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::CFileManagerBackupSettings
// ----------------------------------------------------------------------------
//
CFileManagerBackupSettings::CFileManagerBackupSettings(
        CFileManagerEngine& aEngine ) :
    iEngine( aEngine )
    {
    FUNC_LOG
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::~CFileManagerBackupSettings
// ----------------------------------------------------------------------------
//
CFileManagerBackupSettings::~CFileManagerBackupSettings()
    {
    FUNC_LOG
    delete iCenRep;
    iList.ResetAndDestroy();
    iList.Close();
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::NewL
// ----------------------------------------------------------------------------
//
CFileManagerBackupSettings* CFileManagerBackupSettings::NewL(
        CFileManagerEngine& aEngine )
    {
    CFileManagerBackupSettings* self =
        new ( ELeave ) CFileManagerBackupSettings(
            aEngine );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::ConstructL
// ----------------------------------------------------------------------------
//
void CFileManagerBackupSettings::ConstructL()
    {
    FUNC_LOG

    iCenRep = CRepository::NewL( KCRUidFileManagerSettings );

    RefreshL();
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SetContent
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SetContent(
        const TUint32 aContent )
    {
    iContent = aContent;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SetScheduling
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SetScheduling(
        const TInt aScheduling )
    {
    iScheduling = aScheduling;
    }        

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SetDay
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SetDay( const TInt aDay )
    {
    iDay = aDay;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SetTime
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SetTime( const TTime& aTime )
    {
    iTime = aTime;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SetTargetDrive
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SetTargetDrive( const TInt aDrive )
    {
    iTargetDrive = aDrive;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::Content
// ----------------------------------------------------------------------------
//
EXPORT_C TUint32 CFileManagerBackupSettings::Content() const
    {
    return iContent;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::Scheduling
// ----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerBackupSettings::Scheduling() const
    {
    return iScheduling;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::Day
// ----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerBackupSettings::Day() const
    {
    return iDay;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::Time
// ----------------------------------------------------------------------------
//
EXPORT_C const TTime& CFileManagerBackupSettings::Time() const
    {
    return iTime;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::TargetDrive
// ----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerBackupSettings::TargetDrive() const
    {
    return iTargetDrive;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::ContentCount
// ----------------------------------------------------------------------------
//
TInt CFileManagerBackupSettings::ContentsSelected() const
    {
    TInt ret( 0 );
    // Count selected only
    TUint32 mask( iContent );
    while ( mask )
        {
        if ( mask & 1 )
            {
            ++ret;
            }
        mask >>= 1;
        }
    return ret;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SaveL
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SaveL()
    {
    FUNC_LOG

    TInt err( KErrNone );

    err = iCenRep->Set(
        KFileManagerBackupContent, static_cast< TInt >( iContent ) );
    LOG_IF_ERROR1( err, "CFileManagerBackupSettings::SaveL-Content-err%d",
        err )

    err = iCenRep->Set(
        KFileManagerBackupScheduling, static_cast< TInt >( iScheduling ) );
    LOG_IF_ERROR1( err, "CFileManagerBackupSettings::SaveL-Scheduling-err%d",
        err )

    err = iCenRep->Set(
        KFileManagerBackupDay, static_cast< TInt >( iDay ) );
    LOG_IF_ERROR1( err, "CFileManagerBackupSettings::SaveL-Day-err%d",
        err )

    // Convert micro secs to minutes
    TInt minutes( iTime.Int64() / KMinToMicroSecMultiplier );
    err = iCenRep->Set( KFileManagerBackupTime, minutes );
    LOG_IF_ERROR1( err, "CFileManagerBackupSettings::SaveL-Time-err%d",
        err )

    TChar ch;
    User::LeaveIfError( RFs::DriveToChar( iTargetDrive, ch ) );
    TBuf< KTargetDriveStrMax > drive;
    drive.Append( ch );
    err = iCenRep->Set( KFileManagerBackupTargetPath, drive );
    LOG_IF_ERROR1( err, "CFileManagerBackupSettings::SaveL-Target-err%d",
        err )

    RefreshListL();
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::LoadL
// ----------------------------------------------------------------------------
//
void CFileManagerBackupSettings::LoadL()
    {
    FUNC_LOG

    TInt buffer( 0 );
    TInt err( KErrNone );
    err = iCenRep->Get( KFileManagerBackupContent, buffer );
    LOG_IF_ERROR1( err, "CFileManagerBackupSettings::LoadL-Content-err%d",
        err )

    iContent = buffer;
    if ( !iContent )
         {
         iContent |= EFileManagerBackupContentAll;
         }

    buffer = 0;
    err = iCenRep->Get( KFileManagerBackupScheduling, buffer );
    LOG_IF_ERROR1( err, "CFileManagerBackupSettings::LoadL-Scheduling-err%d",
        err )

    iScheduling = buffer;

    buffer = 0;
    err = iCenRep->Get( KFileManagerBackupDay, buffer );
    LOG_IF_ERROR1( err, "CFileManagerBackupSettings::LoadL-Day-err%d",
        err )

    iDay = buffer;
    if ( iDay == KErrNotFound )
        {
        TLocale locale;
        iDay = locale.StartOfWeek();
        }
    // Convert minutes to micro secs
    buffer = 0;
    err = iCenRep->Get( KFileManagerBackupTime, buffer );
    LOG_IF_ERROR1( err, "CFileManagerBackupSettings::LoadL-Time-err%d",
        err )

    TInt64 microSecs( buffer );
    microSecs *= KMinToMicroSecMultiplier;
    iTime = microSecs;

    HBufC* path = HBufC::NewLC( KMaxPath );
    TPtr ptr( path->Des() );
    err = iCenRep->Get( KFileManagerBackupTargetPath, ptr );
    LOG_IF_ERROR1( err, "CFileManagerBackupSettings::LoadL-Target-err%d",
        err )

    if ( !ptr.Length() )
        {
        User::Leave( KErrNotFound );
        }

    _LIT( KDefault, "?" );
    if ( ptr[ 0 ] == KDefault()[ 0 ] )
        {
#ifdef RD_MULTIPLE_DRIVE
        err = DriveInfo::GetDefaultDrive(
            DriveInfo::EDefaultRemovableMassStorage, iTargetDrive );
        LOG_IF_ERROR1( err, "CFileManagerBackupSettings::LoadL-GetDefaultDrive-err%d",
            err )
        User::LeaveIfError( err );
#else // RD_MULTIPLE_DRIVE
        iTargetDrive = KFmgrMemoryCardDrive;
#endif // RD_MULTIPLE_DRIVE
        }
    else
        {
        User::LeaveIfError( RFs::CharToDrive( ptr[ 0 ], iTargetDrive ) );
        }
    CleanupStack::PopAndDestroy( path );

    // Get allowed drive mask
    err = iCenRep->Get( KFileManagerFeatures, iFileManagerFeatures );
    if ( err != KErrNone )
        {
        ERROR_LOG1( "CFileManagerBackupSettings::LoadL-Features-err%d",
            err )
        iFileManagerFeatures = 0;
        }
    INFO_LOG1(
        "CFileManagerBackupSettings::LoadL-iFileManagerFeatures=0x%x",
        iFileManagerFeatures )
    if ( iFileManagerFeatures & EFileManagerFeatureBackupAllowAllDrives )
        {
        iAllowedDriveMatchMask = KDriveAttRemovable | KDriveAttRemote;
        }
    else
        {
        iAllowedDriveMatchMask = KDriveAttRemovable;
        }

    RefreshListL();
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SettingAt
// ----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerBackupSettings::SettingAt( const TInt aIndex )
    {
    TInt type( ENone );
    if ( aIndex >= 0 && aIndex < iList.Count() )
        {
        type = iList[ aIndex ]->iType;
        }
    return type;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::MdcaCount
// ----------------------------------------------------------------------------
//
TInt CFileManagerBackupSettings::MdcaCount() const
    {
    return iList.Count();
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::MdcaPoint
// ----------------------------------------------------------------------------
//
TPtrC CFileManagerBackupSettings::MdcaPoint( TInt aIndex ) const
    {
    return TPtrC( *( iList[ aIndex ]->iText ) );
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::CEntry::~CEntry
// ----------------------------------------------------------------------------
//
CFileManagerBackupSettings::CEntry::~CEntry()
    {
    delete iText;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::CreateEntryLC
// ----------------------------------------------------------------------------
// 
CFileManagerBackupSettings::CEntry* CFileManagerBackupSettings::CreateEntryLC(
        const TSettingType aType,
        const TInt aTitleId,
        const TInt aTextId,
        const TInt aValue )
    {
    HBufC* value = NULL;
    if ( aValue )
        {
        value = StringLoader::LoadLC( aTextId, aValue );
        }
    else
        {
        value = StringLoader::LoadLC( aTextId );
        }

    CEntry* entry = CreateEntryLC( aType, aTitleId, *value );
    CleanupStack::Pop( entry );
    CleanupStack::PopAndDestroy( value );
    CleanupStack::PushL( entry );
    return entry;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::CreateEntryLC
// ----------------------------------------------------------------------------
// 
CFileManagerBackupSettings::CEntry* CFileManagerBackupSettings::CreateEntryLC(
        const TSettingType aType,
        const TInt aTitleId,
        const TDesC& aText )
    {
    CEntry* entry = new ( ELeave ) CEntry;
    CleanupStack::PushL( entry );

    HBufC* title = StringLoader::LoadLC( aTitleId );
    entry->iText = HBufC::NewL(
        title->Length() + aText.Length() + KSeparatorSpace );

    // "\tFirstLabel\t\tValueText"
    TPtr ptr( entry->iText->Des() );
    ptr.Append( KSeparator );
    ptr.Append( *title );
    ptr.Append( KSeparator );
    ptr.Append( KSeparator );
    ptr.Append( aText );

    entry->iType = aType;

    CleanupStack::PopAndDestroy( title );
    return entry;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::CreateContentsEntryLC
// ----------------------------------------------------------------------------
// 
CFileManagerBackupSettings::CEntry*
        CFileManagerBackupSettings::CreateContentsEntryLC()
    {
    TInt selected( ContentsSelected() );
    if ( !( iContent & EFileManagerBackupContentAll ) && selected > 1 )
        {
        return CreateEntryLC(
            EContents,
            R_QTN_FMGR_BACKUP_CONTENTS,
            R_QTN_FMGR_BACKUP_CONTENT_SELECTED,
            selected );
        }
    TInt textId( ContentToTextId( iContent ) );
    return CreateEntryLC( EContents, R_QTN_FMGR_BACKUP_CONTENTS, textId );
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::CreateTimeEntryLC
// ----------------------------------------------------------------------------
// 
CFileManagerBackupSettings::CEntry*
        CFileManagerBackupSettings::CreateTimeEntryLC()
    {
    TBuf< KTimeStrMax > timeFormatStr;
    TLocale local;
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
    iTime.FormatL( timeStr, timeFormatStr );
    AknTextUtils::DisplayTextLanguageSpecificNumberConversion( timeStr );
    return CreateEntryLC( ETime, R_QTN_FMGR_BACKUP_TIME, timeStr );
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::RefreshListL 
// ----------------------------------------------------------------------------
//
void CFileManagerBackupSettings::RefreshListL()
    {
    iList.ResetAndDestroy();

    // Contents
    CEntry* entry = CreateContentsEntryLC();
    iList.AppendL( entry );
    CleanupStack::Pop( entry );

    if ( !( iFileManagerFeatures & EFileManagerFeatureScheduledBackupDisabled ) )
        {
        // Scheduling
        entry = CreateEntryLC(
            EScheduling,
            R_QTN_FMGR_BACKUP_SCHEDULING,
            KSchedulingTextLookup[ iScheduling ] );
        iList.AppendL( entry );
        CleanupStack::Pop( entry );
    
        // Weekday
        if ( iScheduling == EFileManagerBackupScheduleWeekly )
            {
            TDayName dayName( static_cast< TDay >( iDay ) );
            entry = CreateEntryLC(
                EWeekday,
                R_QTN_FMGR_BACKUP_WEEKDAY,
                dayName );
            iList.AppendL( entry );
            CleanupStack::Pop( entry );
            }
    
        // Time
        if ( iScheduling == EFileManagerBackupScheduleWeekly ||
            iScheduling == EFileManagerBackupScheduleDaily )
            {
            entry = CreateTimeEntryLC();
            iList.AppendL( entry );
            CleanupStack::Pop( entry );
            }
        }

    // Target drive
    if ( HasMultipleBackupTargets() )
        {
        entry = CreateTargetDriveEntryLC();
        iList.AppendL( entry );
        CleanupStack::Pop( entry );
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::ContentToTextId
// ----------------------------------------------------------------------------
//
TInt CFileManagerBackupSettings::ContentToTextId( const TUint32 aContent )
    {
    TInt ret( R_QTN_FMGR_BACKUP_CONTENT_ALL );

    if ( aContent & EFileManagerBackupContentAll )
        {
        ret = R_QTN_FMGR_BACKUP_CONTENT_ALL;
        }
    else if ( aContent & EFileManagerBackupContentSettings )
        {
        ret = R_QTN_FMGR_BACKUP_CONTENT_SETTINGS;
        }
    else if ( aContent & EFileManagerBackupContentMessages )
        {
        ret = R_QTN_FMGR_BACKUP_CONTENT_MESSAGES;
        }
    else if ( aContent & EFileManagerBackupContentContacts )
        {
        ret = R_QTN_FMGR_BACKUP_CONTENT_CONTACTS;
        }
    else if ( aContent & EFileManagerBackupContentCalendar )
        {
        ret = R_QTN_FMGR_BACKUP_CONTENT_CALENDAR;
        }
    else if ( aContent & EFileManagerBackupContentBookmarks )
        {
        ret = R_QTN_FMGR_BACKUP_CONTENT_BOOKMARKS;
        }
    else if ( aContent & EFileManagerBackupContentUserFiles )
        {
        ret = R_QTN_FMGR_BACKUP_CONTENT_USERFILES;
        }
    return ret;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::RefreshL
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::RefreshL()
    {
    LoadL();
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::CreateTargetDriveEntryLC
// ----------------------------------------------------------------------------
// 
CFileManagerBackupSettings::CEntry*
        CFileManagerBackupSettings::CreateTargetDriveEntryLC()
    {
    TPtrC driveName( iEngine.DriveName( iTargetDrive ) );

    if ( driveName.Length() )
        {
        return CreateEntryLC(
            ETarget, R_QTN_FMGR_BACKUP_DESTINATION, driveName );
        }
        
    TChar ch;
    User::LeaveIfError( RFs::DriveToChar( iTargetDrive, ch ) );
    TBuf< KTargetDriveStrMax > drive;
    drive.Append( ch );
    return CreateEntryLC( ETarget, R_QTN_FMGR_BACKUP_DESTINATION, drive );
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::AllowedDriveAttMatchMask
// ----------------------------------------------------------------------------
// 
EXPORT_C TUint32 CFileManagerBackupSettings::AllowedDriveAttMatchMask() const
    {
    return iAllowedDriveMatchMask;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::HasMultipleBackupTargets
// ----------------------------------------------------------------------------
// 
TBool CFileManagerBackupSettings::HasMultipleBackupTargets()
    {
#ifdef RD_MULTIPLE_DRIVE
    TBool count( 0 );
    TUint32 driveAttMask( AllowedDriveAttMatchMask() );
    RFs& fs( iEngine.Fs() );

    for ( TInt i( 0 ); i < KMaxDrives; ++i )
        {
        TDriveInfo driveInfo;
        if ( fs.Drive( driveInfo, i ) == KErrNone )
            {
            // Do not allow backup for internal drives
            TUint driveStatus( 0 );
            DriveInfo::GetDriveStatus( fs, i, driveStatus );
            if ( driveStatus & DriveInfo::EDriveInternal )
                {
                continue;
                }
            if ( driveInfo.iDriveAtt & driveAttMask )
                {
                ++count;
                }
            }
        if ( count > 1 )
            {
            break;
            }
        }
    return count > 1;
#else // RD_MULTIPLE_DRIVE
    return EFalse;
#endif // RD_MULTIPLE_DRIVE
    }

// End of File
