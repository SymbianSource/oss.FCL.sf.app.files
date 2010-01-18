/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declaration of private ps-keys
*
*/


#ifndef FILEMANAGEPRIVATEPSKEYS_H
#define FILEMANAGEPRIVATEPSKEYS_H

// INCLUDES
#include <e32std.h>

// =============================================================================
// FileManager Status PubSub API
// =============================================================================
const TUid KPSUidFileManagerStatus = { 0x101F84EB }; // File Manager SID

/**
* Backup status.
* The value is controlled by FileManager and status observed by backup checker
*/
const TUint32 KFileManagerBkupStatus = 0x00000001;

enum TFileManagerBkupStatusType
    {
    EFileManagerBkupStatusUnset   = 0x00000000,
    EFileManagerBkupStatusBackup  = 0x00000001,
    EFileManagerBkupStatusRestore = 0x00000002
    };

#endif // FILEMANAGERPRIVATEPSKEYS_H

// End of File

