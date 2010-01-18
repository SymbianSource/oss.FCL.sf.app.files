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
* Description:  This class is used to provide localized names
*
*/



#ifndef CGFLMITEMLOCALIZER_H
#define CGFLMITEMLOCALIZER_H


// INCLUDES
#include <e32base.h>


// FORWARD DECLARATIONS
class CDirectoryLocalizer;


// CLASS DECLARATION

/**
*  This class is used to provide localized names to files and directories
*
*  @lib GFLM.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CGflmItemLocalizer) : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CGflmItemLocalizer* NewL();

        /**
        * Destructor.
        */
        virtual ~CGflmItemLocalizer();

    public: // New functions

        /**
        * Get the localized name of a file or a directory by it's path.
        * Works only in main thread context.
        * @since 2.0
        * @param aPath A descriptor containing the full path to be localized
        * @return A TPtrC object containing the localized name. KNullDesC
        *         if no localized name is defined.
        */
        TPtrC Localize( const TDesC& aPath ) const;

        /**
        * Get the localized name of a file or a directory by it's path
        * Works only in work thread context.
        * @since 3.2
        * @param aPath A descriptor containing the full path to be localized
        * @return A TPtrC object containing the localized name. KNullDesC
        *         if no localized name is defined.
        */
        TPtrC LocalizeFromWorkThread( const TDesC& aPath ) const;

        /**
        * Setups localizer for work thread
        * Works only in work thread context.
        * @since 3.2
        */
        void SetupWorkThreadLocalizerL();

        /**
        * Releases localizer used by work thread
        * Works only in work thread context.
        * @since 3.2
        */
        void ReleaseWorkThreadLocalizer();

    private:

        /**
        * C++ default constructor.
        */
        CGflmItemLocalizer();

        void ConstructL();

        TPtrC Localize(
            CDirectoryLocalizer& aLocalizer, const TDesC& aPath ) const;

    private:    // Data
        // Own: For localising in main thread context
        CDirectoryLocalizer* iLocalizer;

        // Own: For localising in work thread context
        CDirectoryLocalizer* iWorkThreadLocalizer;

    };

#endif      // CGFLMITEMLOCALIZER_H

// End of File
