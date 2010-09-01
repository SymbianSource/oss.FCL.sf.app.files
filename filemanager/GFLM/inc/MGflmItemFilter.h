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
* Description:  Defines an interface for making a custom filter to GFLM
*
*/



#ifndef MGFLMITEMFILTER_H
#define MGFLMITEMFILTER_H


// INCLUDES
#include <e32def.h>


// FORWARD DECLARATIONS
class CGflmGroupItem;
class CGflmDriveItem;


// CLASS DECLARATION

/**
*  Defines an interface for making a custom filter to GFLM.
*
*  @lib GFLM.lib
*  @since 2.0
*/
class MGflmItemFilter
    {
    public: // New functions

        /**
        * This method is called for every item to be filtered
        * Note that the method is run in work thread space
        * @since 2.0
        * @param aItem Group item to be filtered
		* @param aGroupId Id of the group to which the item belongs
		* @param aDrive The drive where item is located, NULL if data is unavailable
        * @return True if item was accepted, false if not
        */
        virtual TBool FilterItemL(
            CGflmGroupItem* aItem,
            TInt aGroupId,
            CGflmDriveItem* aDrive ) = 0;

    };

#endif      // MGFLMITEMFILTER_H

// End of File
