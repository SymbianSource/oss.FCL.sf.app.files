/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Provides directory listing cache
*
*/



#ifndef CGFLMDIRECTORYLISTINGCACHE_H
#define CGFLMDIRECTORYLISTINGCACHE_H


//  INCLUDES
#include <f32file.h>
#include "GFLM.hrh"


// CLASS DECLARATION

/**
*  A class that encapsulates reading and storing the contents of a
*  directory. When a directory listing is read, the results are
*  cached so that the same listing can be used in other groups
*  without having to re-read the same directory.
*
*  @lib GFLM.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CGflmDirectoryListingCache) : public CBase
    {
    public:
        typedef CArrayPakFlat< TEntry > CEntryArray;

    private:
        /**
        * An internal data structure for storing directory name and listing
        * pairs.
        */
        NONSHARABLE_CLASS(CListingNamePair) : public CBase
            {
            public:
                ~CListingNamePair();
                static CListingNamePair* NewLC( const TDesC& aDirectory );
            private:
                void ConstructL( const TDesC& aDirectory );
            public:
                HBufC* iDirectory;
                CEntryArray* iListing;
            };

    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @since 2.0
        * @param aFss A reference to a valid and open file server session
        * @param aCancelIndicator A reference to cancel indicator
        */
        static CGflmDirectoryListingCache* NewL(
            RFs& aFss,
            const TBool& aCancelIndicator );

        /**
        * Destructor.
        */
        virtual ~CGflmDirectoryListingCache();

    public: // New functions

        /**
        * Clears the contents of the cache
        * @since 2.0
        */
        void ClearCache();

        /**
        * Retrieves a directory listing
        * @since 2.0
        * @param aDirectory Path of the directory
        * @return A pointer to a CEntryArray object that contains the requested items
        */
        const CEntryArray* ListingL( const TDesC& aDirectory );

    private:

        /**
        * C++ default constructor.
        */
        CGflmDirectoryListingCache(
            RFs& aFss,
            const TBool& aCancelIndicator );

        void GetDirL( CListingNamePair& aPair );

    private:    // Data
        // For caching directory contents. Owned.
        CListingNamePair* iCache;

        // An open fileserver session. Not owned.
        RFs& iFss;

        // Read only cancel indicator. Not owned
        const TBool& iCancelIndicator;

        // A buffer for reading directory contents
        TEntryArray iEntryBuffer;

        // An indicator if cache must be cleared
        TBool iClearCache;

    };

#endif      // CGFLMDIRECTORYLISTINGCACHE_H

// End of File
