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
* Description:  Maps the mime or file extension to icon
*
*/


#ifndef CFILEMANAGERMIMEICONARRAY_H
#define CFILEMANAGERMIMEICONARRAY_H

// INCLUDES
#include <e32base.h>
#include "FileManagerEngine.hrh"

// FORWARD DECLARATIONS
class CGflmNavigatorModel;

// CLASS DECLARATION

/**
*  Contains the array of the icons. Formats the given string with given item.
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerMimeIconArray) : public CBase
    {
    public:

        /**
        * Two-phased constructor.
        * @param aResId resource id of mime icon array
        * @param aNavigator ref to GFLM navigator
        * @return pointer to CFileManagerMimeIconArray object
        */
        static CFileManagerMimeIconArray* NewL(
            TInt aResId, CGflmNavigatorModel& aNavigator );

        /**
        * Gets the icon id of the item
        * @since 2.0
        * @param aFullPath Item with full path information
        * @return TFileManagerIconId, icon id of the item.
        *         If not found other icon id is used.
        */
        TInt ResolveIconL( const TDesC& aFullPath );

        /**
        * Destructor.
        */
        ~CFileManagerMimeIconArray();

    private:

        /**
        * C++ default constructor.
        */
        CFileManagerMimeIconArray( CGflmNavigatorModel& aNavigator );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructFromResourceL( TInt aResId );

        NONSHARABLE_CLASS(CMimeIconEntry) : public CBase
            {
            public: // Constructor and destructor
                static CMimeIconEntry* NewLC(
                    const TDesC& aMime, const TInt aIconId );

                CMimeIconEntry( const TInt aIconId );

                void ConstructL( const TDesC& aMime );

                ~CMimeIconEntry();
            public: // Data
                // Own: MIME type string
                HBufC* iMime;
                // ID of the icon this entry represents.
                TInt iIconId;

            };

    private: // Data
        // Own: Array of icons
        RPointerArray< CMimeIconEntry > iArray;

        // Ref: To get the current directory and MIME type of the item
        CGflmNavigatorModel& iNavigator;

    };

#endif      // CFILEMANAGERMIMEICONARRAY_H
            
// End of File
