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
* Description:  Empty restore settings to maintain library BC.
*                Remove this file when cleaning up RnD flags
*
*/



// INCLUDE FILES
#include "CFileManagerRestoreSettings.h"
#include "CFilemanagerBackupSettings.h"


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::CFileManagerRestoreSettings
// ----------------------------------------------------------------------------
//
CFileManagerRestoreSettings::CFileManagerRestoreSettings(
        CFileManagerEngine& aEngine ) :
    iEngine( aEngine )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::~CFileManagerRestoreSettings
// ----------------------------------------------------------------------------
//
CFileManagerRestoreSettings::~CFileManagerRestoreSettings()
    {
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
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::SetSelection
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerRestoreSettings::SetSelection(
        const TUint64& /*aSelection*/ )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::RefreshL
// ----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerRestoreSettings::RefreshL()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::CEntry::MdcaCount
// ----------------------------------------------------------------------------
//
TInt CFileManagerRestoreSettings::MdcaCount() const
    {
    return 0;
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::CEntry::MdcaPoint
// ----------------------------------------------------------------------------
//
TPtrC CFileManagerRestoreSettings::MdcaPoint( TInt /*aIndex*/ ) const
    {
    return TPtrC( KNullDesC );
    }

// ----------------------------------------------------------------------------
// CFileManagerRestoreSettings::CEntry::~CEntry
// ----------------------------------------------------------------------------
//
CFileManagerRestoreSettings::CEntry::~CEntry()
    {
    }

// End of File
