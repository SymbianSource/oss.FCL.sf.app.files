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
* Description:  Searches items matching with given string in given file system
*
*/


#ifndef CGFLMITEMFINDER_H
#define CGFLMITEMFINDER_H

// INCLUDES
#include <e32std.h>
#include <f32file.h>
#include <collate.h>
#include "GFLM.hrh"
#include "MGflmItemGroup.h"

// FORWARD DECLARATIONS
class CGflmItemLocalizer;
class CGflmDriveResolver;
class MGflmItemFilter;
class CGflmGroupItem;

// CLASS DECLARATION
/**
*  Finds the items matching with given string in given folder
*
*  @lib GFLM.lib
*  @since 3.2
*/
NONSHARABLE_CLASS(CGflmFileFinder) : public CBase,
                                     public MGflmItemGroup
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CGflmFileFinder* NewL(
            RFs& aFss,
            CGflmItemLocalizer& aLocalizer,
            CGflmDriveResolver& aResolver,
            const TBool& aCancelIndicator );

        /**
        * Destructor.
        */
        ~CGflmFileFinder();

    public:
        /**
        * Sets search folder
        * @since 3.2
        * @param aSearchFolder Given search folder
        */
        void SetSearchFolderL( const TDesC& aSearchFolder );

        /**
        * Sets search string. Wild cards are allowed.
        * @since 3.2
        * @param aSearchString Given search string
        */
        void SetSearchStringL( const TDesC& aSearchString );

        /**
        * Refreshes the model
        * @since 3.2
        * @param aFilter Pointer to filter. NULL if not used.
        * @param aSortMethod Sort method
        * @param aRefreshMode Refresh mode
        */
        void RefreshL(
            MGflmItemFilter* aFilter,
            TGflmSortMethod aSortMethod,
            TGflmRefreshMode aRefreshMode );

    public: // From MGflmItemGroup

		TInt Id() const;

        TInt ItemCount() const;

		CGflmGroupItem* Item( TInt aIndex );

		void AddSourceL( const TDesC& aDirectory );

		void ResetSources();

		void AddActionItemL( TInt aId, const TDesC& aCaption );

		void SetInclusion( TUint aInclusion );

        const TCollationMethod* CollationMethod();

    private:
        /**
        * C++ default constructor.
        */
		CGflmFileFinder(
		    RFs& aFss,
            CGflmItemLocalizer& aLocalizer,
            CGflmDriveResolver& aResolver,
            const TBool& aCancelIndicator );

        void ConstructL();

        void DoSearchL( MGflmItemFilter* aFilter );

	private: // Data
        // Ref: An open shareable file server session
		RFs& iFss;

        // Ref: Item localizer
        CGflmItemLocalizer& iItemLocalizer;

        // Ref: Drive resolver
        CGflmDriveResolver& iDriveResolver;

        // Own: Read only cancel indicator
        const TBool& iCancelIndicator;

        // Own: Contains folder string
		HBufC* iSearchFolder;

        // Own: Contains folder path with search string with wild cards
		HBufC* iSearchFolderWild;

        // Own: Contains search string
		HBufC* iSearchString;

        // Own: Contains search string with wild cards
		HBufC* iSearchStringWild;

        // Own: Contains found items
		RPointerArray< CGflmGroupItem > iSearchResults;

        // Own: Buffer for full path handling
        TFileName iFullPath;

        // Own: Used collation method
		TCollationMethod iSortCollationMethod;
    };

#endif      // CGFLMITEMFINDER_H
            
// End of File

