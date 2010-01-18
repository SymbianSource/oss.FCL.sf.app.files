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
* Description:  File finder item definitions
*
*/



#ifndef CGFLMFILEFINDERITEM_H
#define CGFLMFILEFINDERITEM_H


//  INCLUDES
#include <e32base.h>
#include "CGflmFileSystemItem.h"


//  FORWARD DECLARATIONS


// CLASS DECLARATION

/**
*  A class representing a file system item.
*  A file system item is either a file or a directory.
*
*  @lib GFLM.lib
*  @since 3.2
*/
class CGflmFileFinderItem : public CGflmFileSystemItem
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CGflmFileFinderItem* NewLC(
            const TEntry& aEntry,
            const TDesC& aBasePath );

        /**
        * Destructor.
        */
        virtual ~CGflmFileFinderItem();

        /**
        * Compares items by match
        * @since 3.2
        * @param aFirst Given first item for comparison
        * @param aSecond Given second item for comparison
        * @return Comparison result
        */
        static TInt CompareByMatch(
            const CGflmGroupItem& aFirst,
            const CGflmGroupItem& aSecond );

        /**
        * Prepares item for by match sorting
        * @since 3.2
        * @param aSearchString Given search string
        */
        void PrepareSort( const TDesC& aSearchString );

    private:

        /**
        * C++ default constructor.
        */
        CGflmFileFinderItem( const TEntry& aFSEntry );

        void ConstructL( const TDesC& aBasePath );

    private:    // Data
        TEntry iEntry;
        HBufC* iBasePath;
        TInt16 iSortValue;
    };

#endif      // CGFLMFILEFINDERITEM_H

// End of File
