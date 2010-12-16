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
* Description:  Drive info storage
*
*/


#ifndef TFILEMANAGERDRIVEINFO_H
#define TFILEMANAGERDRIVEINFO_H


// INCLUDES
#include <f32file.h>


// CONSTANTS
const TInt KMaxVolumeName = 11; // Limited by FAT
// Remove these when there is API available for checking
const TInt KFmgrSystemDrive = EDriveC;
const TInt KFmgrMemoryCardDrive = EDriveE;
const TInt KFmgrRomDrive = EDriveZ;
const TInt KFmgrRamDrive = EDriveD;


// FORWARD DECLARATIONS
class RFs;
class CFileManagerEngine;


// CLASS DECLARATION
/**
*  TFileManagerDriveInfo is used for storing drive info
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
class TFileManagerDriveInfo
    {
    public:
        /**
        * C++ default constructor.
        */
        IMPORT_C TFileManagerDriveInfo();

        /**
        * Resets contents
        */
        IMPORT_C void Reset();

        TBuf< KMaxVolumeName > iName;
        TInt64 iCapacity;
        TInt64 iSpaceFree;
        TUint iUid;
        TInt iDrive; // EDriveA...EDriveZ
        // Drive state bits
        enum TDriveState
            {
            EDrivePresent = 0x1,
            EDriveLocked = 0x2,
            EDriveCorrupted = 0x4,
            EDriveWriteProtected = 0x8,
            EDriveRemovable = 0x10,
            EDriveRemote = 0x20,
            EDriveFormattable = 0x40,
            EDriveFormatted = 0x80,
            EDriveLockable = 0x100,
            EDrivePasswordProtected = 0x200,
            EDriveBackupped = 0x400,
            EDriveConnected = 0x800,
            EDriveEjectable = 0x1000,
            EDriveInUse = 0x2000,
            EDriveMassStorage = 0x4000,
            EDriveUsbMemory = 0x8000
            };
        TUint32 iState;

    public:
        /**
         * Gets drive info
         * @since 3.1
         * @param aEngine Reference to engine
         * @param aDrive Drive that info is required
         */
        void GetInfoL( const CFileManagerEngine& aEngine, const TInt aDrive );
        
        /**
         * Gets drive info
         * @since 5.2
         * @param aEngine Reference to engine
         * @param aDrive Drive that info is required
         * @param aCheckRemoteDriveConnected Indicates whether to check Remote Drive is connected
         */
        void GetInfoL(const CFileManagerEngine& aEngine, const TInt aDrive, TBool aCheckRemoteDriveConnected );

    private:
        void CheckMountL( RFs& aFs, const TInt aDrive ) const;

        TInt FillStateFromDriveInfo(
            RFs& aFs, const TInt aDrive, const TDriveInfo& aDrvInfo );

        TInt FillStateFromVolumeInfo( RFs& aFs, const TInt aDrive );

    };

#endif // TFILEMANAGERDRIVEINFO_H

// End of File
