/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  All the files and folders icons are stored here
*
*/



#ifndef CFILEMANAGERICONARRAY_H
#define CFILEMANAGERICONARRAY_H

//  INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS
class CGulIcon;

// CLASS DECLARATION
/**
 * Array of icons.
 * Is an array of icons that is read from resource file.
 *
 *  @lib FileManagerView.lib
 *  @since S60 2.0
 */
class CFileManagerIconArray : public CArrayPtrFlat< CGulIcon >
    {
    public:  // Constructors and destructor
        /**
         * Two-phased constructor.
         * @return a new icon array.
         */
        IMPORT_C static CFileManagerIconArray* NewL();

        /**
         * Destructor.
         */
        IMPORT_C ~CFileManagerIconArray();

        /**
         * Updates icons.
         */
        IMPORT_C void UpdateIconsL();

        /**
         * Finds array index of the icon
         * @param aIconId Icon id
         * @return Array index of the icon or KErrNotFound
         */
        IMPORT_C TInt FindIcon( TInt aIconId );

        /**
         * Loads icon from icon file
         * @param aIconFile Full path of the icon file
         * @param aIconId Icon id
         * @param aMaskId Mask id
         * @param aMajorSkin Major skin id
         * @param aMinorSkin Minor skin id
         * @param aIsColorIcon True if color icon
         * @return Pointer to new icon and the ownership is transferred.
         */
        IMPORT_C static CGulIcon* LoadIconL(
            const TDesC& aIconFile,
            TInt aIconId,
            TInt aMaskId,
            TInt aMajorSkin,
            TInt aMinorSkin,
            TBool aIsColorIcon );

    private:
        NONSHARABLE_CLASS(TIconInfo)
            {
            public:
                HBufC* iFile;
                TInt iId;
                TInt iIconId;
                TInt iMaskId;
                TInt iMajorSkinId;
                TInt iMinorSkinId;
                TInt iIconType;
                TInt iIndex;
            };
        /**
         * C++ default constructor.
         */
        CFileManagerIconArray();

        /**
         * By default Symbian 2nd phase constructor is private.
         */
        void ConstructL();

        void LoadIconL( TIconInfo& aInfo );

        void LoadMandatoryIconsL();

    private:    // Data
        RArray< TIconInfo > iIconInfo;

    };

#endif      // CFILEMANAGERICONARRAY_H   
            
// End of File
