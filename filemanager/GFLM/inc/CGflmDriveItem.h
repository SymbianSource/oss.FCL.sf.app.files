/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Drive item definitions
*
*/



#ifndef CGFLMDRIVEITEM_H
#define CGFLMDRIVEITEM_H


//  INCLUDES
#include <e32base.h>
#include <f32file.h>
#include "CGflmGroupItem.h"


// CLASS DECLARATION
/**
*  A class representing drive items.
*
*  @lib GFLM.lib
*  @since 3.1
*/
class CGflmDriveItem : public CGflmGroupItem
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CGflmDriveItem* NewLC(
            const TInt aDrive,
            const TVolumeInfo& aFSVolume,
            const TInt aVolumeStatus,
            const TUint aDriveStatus );

        /**
        * Destructor.
        */
        virtual ~CGflmDriveItem( );

    public: // New functions

        /**
        * Sets the localized name of the item.
        * @since 3.1
        * @param aName A descriptor containing the localized name for the
                 entry. A copy of the name is created.
        */
        IMPORT_C void SetLocalizedNameL( const TDesC& aName );

        /**
        * Sets the root directory of the item.
        * @since 3.1
        * @param aName A descriptor containing the path for the
                 entry. A copy of the path is created.
        */
        IMPORT_C void SetRootDirectoryL( const TDesC& aPath );

        /**
        * Gets the root directory of the item.
        * @since 3.1
        * @return A pointer descriptor to root directory
        */
        IMPORT_C TPtrC RootDirectory() const;

        /**
        * Gets the volume info of the item.
        * @since 3.1
        * @return A reference to volume info
        */
        IMPORT_C const TVolumeInfo& VolumeInfo() const;

        /**
        * Get the drive id
        * @since 3.1
        * @return Drive id.
        */
        IMPORT_C TInt Drive() const;

        /**
        * Get the volume status
        * @since 3.1
        * @return volume status
        */
        IMPORT_C TInt VolumeStatus() const;

        /**
        * Get the drive status
        * @since 3.2
        * @return drive status
        */
        IMPORT_C TUint DriveStatus() const;

    public: // From CGflmGroupItem

        TGflmItemType Type() const;

        TPtrC Name() const;

        TTime Date() const;

        TInt64 Size() const;

        TPtrC Ext() const;

    private:

        /**
        * C++ default constructor.
        */
        CGflmDriveItem(
            const TInt aDrive,
            const TVolumeInfo& aFSVolume,
            const TInt aVolumeStatus,
            const TUint aDriveStatus );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
        // A TVolumeInfo object that is the actual entry
        TVolumeInfo iVolume;

        // The drive numeric id.
        TInt iDrive;

        // The root directory of the entry.
        HBufC* iRootDir;

        // The localised name of the entry.
        HBufC* iLocalizedName;

        // Holds volume status i.e return value of drive or volume info
        TInt iVolumeStatus;

        // Holds drive status i.e the value from DriveInfo::GetDriveStatus
        TUint iDriveStatus;

    };

#endif // CGFLMDRIVEITEM_H

// End of File
