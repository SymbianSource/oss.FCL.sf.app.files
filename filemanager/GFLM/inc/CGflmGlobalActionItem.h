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
* Description:  Global action item definitions
*
*/



#ifndef CGFLMGLOBALACTIONITEM_H
#define CGFLMGLOBALACTIONITEM_H


//  INCLUDES
#include <e32base.h>
#include "CGflmGroupItem.h"


// CLASS DECLARATION

/**
*  A class representing a global action item.
*  A global item item is visible all the time in a group.
*
*  @lib GFLM.lib
*  @since 2.0
*/
class CGflmGlobalActionItem : public CGflmGroupItem
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CGflmGlobalActionItem* NewLC(
            TInt aId, const TDesC& aCaption );

        /**
        * Destructor.
        */
        virtual ~CGflmGlobalActionItem();

    public: // New functions
        /**
        * Returns the id of the action item
        * @since 2.0
        * @return Id of the item
        */
        IMPORT_C TInt Id() const;

    public: // From CGflmGroupItem

        TGflmItemType Type() const;

        TPtrC Name() const;

        TTime Date() const;

        TInt64 Size() const;

        TPtrC Ext() const;

    protected:  // New functions

        /**
        * C++ default constructor.
        */
        CGflmGlobalActionItem();

    private:

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const TDesC& aCaption );

    private:    // Data

        // Id of the action item
        TInt iId;

        // Caption of the action item
        HBufC* iCaption;

    };

#endif      // CGFLMGLOBALACTIONITEM_H

// End of File
