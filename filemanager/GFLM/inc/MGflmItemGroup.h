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
* Description:  Defines an interface for GFLM clients to access item groups
*
*/



#ifndef MGFLMITEMGROUP_H
#define MGFLMITEMGROUP_H


//  INCLUDES
#include "GFLM.hrh"
#include <e32std.h>


// FORWARD DECLARATIONS
class CGflmGroupItem;


// CLASS DECLARATION

/**
*  Defines an interface for GFLM clients to access item groups
*
*  @lib GFLM.lib
*  @since 2.0
*/
class MGflmItemGroup
    {
    public: // New functions

		/**
        * Returns the Id of the group
        * @since 2.0
        * @return Id of the group
        */
		virtual TInt Id() const = 0;

		/**
        * Returns the number of items in the group
        * @since 2.0
        * @return Number of items in the group
        */
        virtual TInt ItemCount() const = 0;

		/**
        * Returns a pointer to a specific item in the group by its index
        * @since 2.0
		* @param aIndex Index of the requested CGflmGroupItem object
        * @return Pointer to a CGflmGroupItem object
        */
		virtual CGflmGroupItem* Item( TInt aIndex ) = 0;

		/**
        * Adds a new filesystem source to the group
        * @since 2.0
		* @param aDirectory Path of the directory to add to the group
        */
		virtual void AddSourceL( const TDesC& aDirectory ) = 0;

		/**
        * Resets the list of filesystem sources
        * @since 2.0
        */
		virtual void ResetSources() = 0;

		/**
        * Adds a new action item to the group
        * @since 2.0
		* @param aId Id of the new action item
		* @param aCaption Caption of the new action item
        */
		virtual void AddActionItemL( TInt aId, const TDesC& aCaption ) = 0;

		/**
        * Sets the inclusion mode (files/directories) of the group
        * @since 2.0
		* @param aInclusion New inclusion mode. See TGflmFSItemInclusion.
        */
		virtual void SetInclusion( TUint aInclusion ) = 0;

		/**
        * Gets the collation method
        * @since 5.0
		* @return Pointer to collation method
        */
        virtual const TCollationMethod* CollationMethod() = 0;

    };

#endif      // MGFLMITEMGROUP_H

// End of File
