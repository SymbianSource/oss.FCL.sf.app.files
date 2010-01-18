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
* Description:  Concrete implementation of MGflmItemGroup interface
*
*/



#ifndef CGFLMITEMGROUPIMPL_H
#define CGFLMITEMGROUPIMPL_H


//  INCLUDES
#include "MGflmItemGroup.h"
#include <e32std.h>
#include <badesca.h>
#include <collate.h>


// FORWARD DECLARATIONS
class RFs;
class CGflmDirectoryListingCache;
class CGflmFileRecognizer;
class CGflmItemLocalizer;
class CGflmDriveResolver;
class MGflmItemFilter;


// CLASS DECLARATION

/**
*  Concrete implementation of MGflmItemGroup interface
*
*  @lib GFLM.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CGflmItemGroupImpl) : public CBase, public MGflmItemGroup
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aId The new id
        * @param aFss A reference to an open and valid file server session
        * @param aCache A constructed directory listing cache
        * @param aRecognizer A constructed file recognizer
        * @param aLocalizer A constructed item localizer
        * @param aResolver A constructed drive letter resolver
        */
        static CGflmItemGroupImpl* NewLC(
            const TInt aId,
            RFs& aFss,
            CGflmDirectoryListingCache& aCache,
            CGflmItemLocalizer& aLocalizer,
            CGflmDriveResolver& aResolver );

        /**
        * Destructor.
        */
        virtual ~CGflmItemGroupImpl();

    public: // New functions

        /**
        * Refreshes and reformats the contents of the group
        * @since 2.0
        * @param aFilter MGflmItemFilter object used to to filter the contents
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
        * Constructor.
        * @param aId The new id
        * @param aFss A reference to an open and valid file server session
        * @param aCache A constructed directory listing cache
        * @param aLocalizer A constructed item localizer
        * @param aResolver A constructed drive letter resolver
        */
        CGflmItemGroupImpl(
            const TInt aId,
            RFs& aFss,
            CGflmDirectoryListingCache& aCache,
            CGflmItemLocalizer& aLocalizer,
            CGflmDriveResolver& aResolver );

        void ConstructL();

		void PopulateReferenceListL( MGflmItemFilter* aFilter );

    private:    // Data

		// Id of the group
		TInt iId;

		// Reference to an open fileserver session
		RFs& iFss;

        // Inclusion type
		TUint iInclusion;

		// Contains (and owns) static items (action items and symbolic
		// links) of this group
		RPointerArray< CGflmGroupItem > iStaticItems;

		// Contains (and owns) volatile items (information aquired from
		// the filesystem) of this group
		RPointerArray< CGflmGroupItem > iVolatileItems;

		// Contains the same items as iStaticItems and iVolatileItems,
		// but doesn't own them
		RPointerArray< CGflmGroupItem > iItemReferences;

		// Directory name
		HBufC* iDirectory;

		// Reference to a CGflmDirectoryListingCache object. Not owned.
		CGflmDirectoryListingCache& iListingCache;

		// Reference to an CGflmItemLocalizer object. Not owned.
		CGflmItemLocalizer& iItemLocalizer;

		// Reference to an CGflmDriveResolver object. Not owned.
		CGflmDriveResolver& iDriveResolver;

        // Used collation method
		TCollationMethod iSortCollationMethod;

    };

#endif      // CGFLMITEMGROUPIMPL_H

// End of File
