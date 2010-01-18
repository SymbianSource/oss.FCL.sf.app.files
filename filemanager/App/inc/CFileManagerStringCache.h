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
* Description:  This class caches strings
*
*/



#ifndef CFILEMANAGERSTRINGCACHE_H
#define CFILEMANAGERSTRINGCACHE_H

// INCLUDES
#include <e32base.h>
#include <f32file.h>
#include <badesca.h>

// CONSTANTS
const TInt KNumStringCacheItems = 16;
const TInt KFormatStringLen = KMaxFileName * 2;

// FORWARD DECLARATIONS
class CFileManagerEngine;
class CFileManagerIconArray;


// CLASS DECLARATION
/**
*  This class caches strings  
*
*  @since 2.7
*/
class CFileManagerStringCache : public CBase, public MDesCArray
    {
    public:

        /**
        * Two-phased constructor.
        */
        static CFileManagerStringCache* NewL(
            CFileManagerEngine& aEngine,
            CFileManagerIconArray& aIconArray );

        void Clear() const;
        
        /**
        * Destructor.
        */
        ~CFileManagerStringCache();

        /**
         * Gets last error
         */
        TInt LastError() const;

    public: // From MDesCArray

        TInt MdcaCount() const;
        TPtrC MdcaPoint(TInt aIndex) const;
    
    private:
        /**
        * C++ default constructor.
        */
        CFileManagerStringCache(
            CFileManagerEngine& aEngine,
            CFileManagerIconArray& aIconArray );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
         * Sets last error
         */
        void SetError( TInt aError ) const;

    private:

        /**
        * Searches index from cache and return cache index
        * so it can be retrieve with FormattedString
        */
        TInt FormattedStringFound( TInt aIndex ) const;

        /**
        * Retrieve string from cache with FormattedStringFound()
        * given index
        */
        TPtrC FormattedString( TInt aIndex ) const;

        /** 
        * Stores string in iFormatted string with given 
        * listbox index. Checks the fartest position 
        * and stores there.
        */
        void StoreFormattedStringL( TInt aIndex ) const;

        /**
        * Clears cache item
        * given index
        */
        void Clear( TInt aIndex ) const;

    private: // Data
        class TCacheItem
            {
            public:
                HBufC* iString;
                TInt   iIndex;
            };

        // Ref: To get item details
        CFileManagerEngine&  iEngine;

        // Ref: To resolve icons
        CFileManagerIconArray& iIconArray;

        // Own: array of formatted listbox items
        TFixedArray< TCacheItem, KNumStringCacheItems > mutable
            iFormattedItems;

        // Own: String buffer for formatting string
        TBuf< KFormatStringLen > mutable iFormatString;

        // Own: Last error
        TInt mutable iError;

    };

#endif      // CFILEMANAGERSTRINGCACHE_H
            
// End of File

