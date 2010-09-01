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
* Description:  Goes through the list of given files
*
*/


#ifndef CFILEMANAGERINDEXITERATOR_H
#define CFILEMANAGERINDEXITERATOR_H

//  INCLUDES
#include <e32base.h>
#include "Mfilemanageritemiterator.h"

// FORWARD DECLARATIONS
class CFileManagerEngine;

/**
*  Goes through the given file list. Implements the
*  MFileManagerItemIterator.
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerIndexIterator) :
        public CBase,
        public MFileManagerItemIterator
    {
    public:

        /**
        * Two-phased constructor.
        */
        static CFileManagerIndexIterator* NewL(
            CFileManagerEngine& aEngine,
            CArrayFixFlat< TInt >& aIndexList,
            const TDesC& aDstDir );

        /**
        * Destructor.
        */
        ~CFileManagerIndexIterator();

    public: // From MFileManagerItemIterator

        void CurrentL(
            HBufC** aSrc, HBufC** aDst, TFileManagerTypeOfItem& aItemType);

        TBool NextL();

    private:

        /**
        * C++ default constructor.
        */
        CFileManagerIndexIterator(
            CFileManagerEngine& aEngine,
            CArrayFixFlat< TInt >& aIndexList,
            const TDesC& aDstDir );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:
        // Index to currently processed item in given array
        TInt iCurrentIndex;

        // To get the full path information
        CFileManagerEngine& iEngine;

        // User chosen list of files
        CArrayFixFlat< TInt >& iIndexList;

        // Destination directory
        const TDesC& iDstDir;

        // Own: Source of the currently processed item with full path
        HBufC* iSrc;

        // Own: Destination of the currently processes item with full path
        HBufC* iDst;

    };

#endif // CFILEMANAGERINDEXITERATOR_H