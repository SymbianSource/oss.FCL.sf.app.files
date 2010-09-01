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
* Description:  Navigable file list model
*
*/



#ifndef CGFLMNAVIGATORMODEL_H
#define CGFLMNAVIGATORMODEL_H


//  INCLUDES
#include "CGflmFileListModel.h"


// FORWARD DECLARATIONS
class CGflmDriveItem;


// CLASS DECLARATION

/**
*  Decorates the file list model with navigator functionality
*
*  @lib GFLM.lib
*  @since 2.0
*/
class CGflmNavigatorModel : public CGflmFileListModel
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CGflmNavigatorModel* NewL( RFs& aFs );

        /**
        * Destructor.
        */
        virtual ~CGflmNavigatorModel();

    public: // New functions

        /**
        * Sets the navigator's basepath to a given directory
        * @since 2.0
        * @param aPath A descriptor containing the directory to enter
        * @param aBackstepping If false the backstepping stack is reset
        */
        IMPORT_C void GoToDirectoryL( const TDesC& aPath,
                                      TBool aBackstepping = ETrue );

        /**
        * Sets the navigator's basepath to a given directory and
        * constructs a backstepping stack that leads from the base path
        * to the top path one intermediate directory at a time
        * @since 2.0
        * @param aBasePath A descriptor containing the base path
        * @param aTopPath A descriptor containing the directory to enter
        */
        IMPORT_C void GoToDirectoryL( const TDesC& aBasePath,
                                      const TDesC& aTopPath );

        /**
        * Navigates one step backwards.
        * @since 2.0
        */
        IMPORT_C void BackstepL();

        /**
        * Returns the current navigation directory
        * @since 2.0
        * @return A pointer descriptor containing the current navigation
        *         directory.
        */
        IMPORT_C TPtrC CurrentDirectory() const;

        /**
        * Returns the localized name of the current navigation directory
        * @since 2.0
        * @return A pointer descriptor containing the localized name
        *         of the current navigation directory.
        */
        IMPORT_C TPtrC LocalizedNameOfCurrentDirectory() const;

        /**
        * Returns the current navigation level, the depth of backstep stack
        * @since 2.0
        * @return Current navigation level
        */
        IMPORT_C TInt NavigationLevel() const;

        /**
        * Obtains a pointer to current drive (not own).
        * @since 3.1
        * @return Pointer to current drive or NULL.
        */
        IMPORT_C CGflmDriveItem* CurrentDrive() const;

    private:

        /**
        * C++ default constructor.
        */
        CGflmNavigatorModel( RFs& aFs );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Sets the navigator's base directory
        * @since 2.0
        * @param aDirectory The new base directory
        */
        void SetBaseDirectoryL( const TDesC& aDirectory );

        void SetSourceL( const TDesC& aSource );

        TBool IsValidSource( const TDesC& aSource ) const;

    private:    // Data

        // An dynamic array holding backstepping information
        CDesCArraySeg* iBackstepStack;

    };

#endif      // CGFLMNAVIGATORMODEL_H

// End of File
