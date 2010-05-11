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
* Description:  Recogniser wrapper
*
*/



#ifndef CGFLMFILERECOGNIZER_H
#define CGFLMFILERECOGNIZER_H


//  INCLUDES
#include <apgcli.h>
#include <e32base.h>
#include <barsc.h>


// FORWARD DECLARATIONS
class CGflmDriveResolver;


// CLASS DECLARATION

/**
*  A class for wrapping recogniser functionality.
*
*  @lib GFLM.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CGflmFileRecognizer) : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CGflmFileRecognizer* NewL(
            RFs& aFs,
            TInt aMemoryConsumption,
            CGflmDriveResolver* aDriveResolver );

        /**
        * Destructor.
        */
        virtual ~CGflmFileRecognizer();

    public: // New functions

        /**
        * Recognises a MIME type of file.
        * @since S60 3.1
        * @param aFilename A name of the file
        * @return Pointer to MIME type. KNullDesC if not recognised.
        */
        TPtrC RecognizeL( const TDesC& aFilename );

         /**
        * Flushed recogniser cache.
        * @since S60 3.1
        */
        void FlushCache();

    private:
        
        NONSHARABLE_CLASS(CPathTypePair) : public CBase
            {
            public:
                static CPathTypePair* NewLC( const TDesC& aFilename );
                void ConstructL( const TDesC& aFilename );
                CPathTypePair();
                ~CPathTypePair();
                TInt Size() const;

            public:
                TDblQueLink iLink;
                TInt iTypeIndex;
                HBufC* iFilename;
            };

        NONSHARABLE_CLASS(CExtMimePair) : public CBase
            {
            public:
                ~CExtMimePair();

            public:
                HBufC* iExt;
                HBufC* iMime;
            };

        /**
        * C++ default constructor.
        */
        CGflmFileRecognizer(
            CGflmDriveResolver* aDriveResolver,
            RFs& aFs );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( TInt aMemoryConsumption );

        /**
        * Cleans old entries from cache according to specified per cent
        *
        */
        void CleanupCache( );

        /**
        * Constructs file extension mime pairs
        *
        */
        void ConstructExtMimePairsL(
            RResourceFile& aResFile,
            TInt aResId,
            RPointerArray< CExtMimePair >& aPairs );

        /**
        * Finds mime from file extension
        *
        */
        TPtrC FindMimeFromExt(
            const TDesC& aExt, RPointerArray< CExtMimePair >& aPairs );

        /**
        * Does actual recognition
        *
        */
        TPtrC DoRecognizeL( const TDesC& aFilename );
        
        /**
         * Reset and Destroy the RPointArray of CExtMimePair
         * 
         */
        static void ResetAndDestroyExtMimePairs( TAny* aPtr );

    private:    // Data
        // Own: For using the apparc recognisers
        RApaLsSession iApaSession;

        // Array for mime types, owned
        CDesCArraySeg* iMimeTypes;

        // Mime type cache, owned
        TDblQue< CPathTypePair > iCache;

        // Current memory usage for cache
        TInt iCacheMemoryUsage;

        // Maximum memory usage for cache
        TInt iCacheMaxMemoryUsage;

        // Ref: Pointer to drive resolver
        CGflmDriveResolver* iDriveResolver;

        // Own: General file extension mime pairs
        RPointerArray< CExtMimePair > iExtMimePairs;

        // Own: Remote drive specific file extension mime pairs
        RPointerArray< CExtMimePair > iRemoteExtMimePairs;

        // Ref: Open file server session
        RFs& iFs;

    };

#endif      // CGFLMFILERECOGNIZER_H

// End of File
