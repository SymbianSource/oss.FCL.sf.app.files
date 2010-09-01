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
* Description:  Resolves different media types' drive letters
*
*/


#ifndef CGFLMDRIVERESOLVER_H
#define CGFLMDRIVERESOLVER_H


//  INCLUDES
#include <f32file.h>
#include "GFLM.hrh"

// CONSTANTS
const TInt KGflmReadBufferLen = 256;

// FORWARD DECLARATIONS
class CGflmDriveItem;
class MGflmItemFilter;


// CLASS DECLARATION
/**
*  Resolves different media types' drive letters
*
*  @lib GFLM.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CGflmDriveResolver) : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CGflmDriveResolver* NewL( RFs& aFs );

        /**
        * Destructor.
        */
        virtual ~CGflmDriveResolver();

    public: // New functions
        /**
        * Refreshes drive information
        * @param aFs Reference to file system handle
        * @param aFilter Pointer to filter
        * @return System wide error code.
        * @since 3.2
        */
        TInt RefreshDrives( MGflmItemFilter* aFilter );

        /**
        * Gets number of drives
        * @since 3.1
        * @return A number of drives
        */
        TInt DriveCount() const;

        /**
        * Gets drive item at position
        * @since 3.1
        * @param aIndex Given position
        * @return A pointer to drive item (not owned).
        */
        CGflmDriveItem* DriveAt( const TInt aIndex ) const;

        /**
        * Gets drive item from given path
        * @since 3.1
        * @param aPath Given path
        * @return A pointer to drive item (not owned)
        *         or NULL if drive not found.
        */
        CGflmDriveItem* DriveFromPath( const TDesC& aPath ) const;

        /**
        * Clears drive information
        * @since 3.1
        */
        void ClearDrives();

		/**
        * Checks if path is drive root path
        * @since 3.1
        */
        TBool IsRootPath( const TDesC& aPath ) const;

		/**
        * Checks if path is on remote drive
        * @since 3.1
        */
        TBool IsRemoteDrive( const TDesC& aPath ) const;

        /**
        * Gets drive item from given id
        * @since 3.2
        * @param aDrive Given drive id
        * @return A pointer to drive item (not owned)
        *         or NULL if drive not found.
        */
        CGflmDriveItem* DriveFromId( const TInt aDrive ) const;

     private:

        /**
        * C++ default constructor.
        */
        CGflmDriveResolver( RFs& aFs );

        void ConstructL();

        void AppendDriveL(
            const TInt aDrive, MGflmItemFilter* aFilter );

        void RefreshDrivesL( MGflmItemFilter* aFilter );

    private:    // Data
        // Shareable file server session.
        RFs& iFs;

        // List of drives. Owned.
        RPointerArray< CGflmDriveItem > iDrives;

        TBool iRefreshed;

        RCriticalSection iCs;

        TBuf< KGflmReadBufferLen > iReadBuffer;
    };

#endif      // CGFLMDRIVERESOLVER_H

// End of File
