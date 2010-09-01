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
* Description:  Empty backup settings to maintain library BC.
*                Remove this file when cleaning up RnD flags
*
*/



// INCLUDE FILES
#include "cfilemanagerbackupsettings.h"


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::CFileManagerBackupSettings
// ----------------------------------------------------------------------------
//
CFileManagerBackupSettings::CFileManagerBackupSettings(
        CFileManagerEngine& aEngine ) :
    iEngine( aEngine )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::~CFileManagerBackupSettings
// ----------------------------------------------------------------------------
//
CFileManagerBackupSettings::~CFileManagerBackupSettings()
    {
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
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SetContent
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SetContent(
        const TUint32 /*aContent*/ )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SetScheduling
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SetScheduling(
        const TInt /*aScheduling*/ )
    {
    }        

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SetDay
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SetDay( const TInt /*aDay*/ )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SetTime
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SetTime( const TTime& /*aTime*/ )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SetTargetDrive
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SetTargetDrive(
        const TInt /*aDrive*/ )
    {
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
    return 0;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::Day
// ----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerBackupSettings::Day() const
    {
    return 0;
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
    return 0;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::SaveL
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::SaveL()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::MdcaCount
// ----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerBackupSettings::SettingAt( const TInt /*aIndex*/ )
    {
    return 0;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::MdcaCount
// ----------------------------------------------------------------------------
//
TInt CFileManagerBackupSettings::MdcaCount() const
    {
    return 0;
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::MdcaPoint
// ----------------------------------------------------------------------------
//
TPtrC CFileManagerBackupSettings::MdcaPoint( TInt /*aIndex*/ ) const
    {
    return TPtrC( KNullDesC );
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::CEntry::~CEntry
// ----------------------------------------------------------------------------
//
CFileManagerBackupSettings::CEntry::~CEntry()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::RefreshL
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerBackupSettings::RefreshL()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerBackupSettings::AllowedDriveAttMatchMask
// ----------------------------------------------------------------------------
// 
EXPORT_C TUint32 CFileManagerBackupSettings::AllowedDriveAttMatchMask() const
    {
    return KDriveAttRemovable;
    }

// End of File
