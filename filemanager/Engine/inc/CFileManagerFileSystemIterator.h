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
* Description:  Goes through folder tree one item at time
*
*/


#ifndef CFILEMANAGERFILESYSTEMITERATOR_H
#define CFILEMANAGERFILESYSTEMITERATOR_H

// INCLUDES
#include <e32base.h>
#include <f32file.h>
#include "Mfilemanageritemiterator.h"


class CFileManagerEngine;

/**
*  Goes through all the folder items of given folder. Implements the
*  MFileManagerItemIterator.
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerFileSystemIterator) :
        public CBase,
        public MFileManagerItemIterator
    {
    public:

        /**
        * Two-phased constructor.
        */
        static CFileManagerFileSystemIterator* NewL( 
            RFs& aFs, 
            const TDesC& aSrcDir, 
            const TDesC& aDstDir,
            CFileManagerEngine& aEngine );

        /**
        * Destructor.
        */
        ~CFileManagerFileSystemIterator();

    public: // From MFileManagerItemIterator        
        void CurrentL(
            HBufC** aSrc, HBufC** aDst, TFileManagerTypeOfItem& aItemType );

        TBool NextL();

    public:

        /**
        * returns localized path 
        * @since 2.0
        * @return reference to localized path string
        */
        TPtrC   LocalizedAbbreviatedPath();

    private:
        /**
        * C++ default constructor.
        */
        CFileManagerFileSystemIterator(
            const TDesC& aDstDir, CFileManagerEngine& aEngine );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL(  RFs& aFs, const TDesC& aSrcDir );

    private:
        // Own: With this folders are scan through
        CDirScan* iDirScan;

        // Own: CDirScan returns this, contains one folder content at time
        CDir*     iDir;

        // Index to current CDir array item which is processed
        TInt      iCount;

        // Own: Current item source fullpath
        // reserved here so that caller can then use this via reference
        HBufC*    iSrc;

        // Own: Current item destination fullpath
        // reserved here so that caller can then use this via reference
        HBufC*    iDst;

        // Ref: Destination directory
        const TDesC& iDstDir;

        // Set when all folders are gone through
        TBool     iFolderScanDone;

        // Own: Source directory 
        HBufC*    iSrcDir;

        // Ref: File Manager engine
        CFileManagerEngine& iEngine;

        // Own: Localized Abbreviated Path
        TFileName iLocalizedAbbreviatedPath;

    };

#endif // CFILEMANAGERFILESYSTEMITERATOR_H

