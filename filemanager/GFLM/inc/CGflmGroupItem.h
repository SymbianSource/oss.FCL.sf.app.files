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
* Description:  Group item definitions
*
*/



#ifndef CGFLMGROUPITEM_H
#define CGFLMGROUPITEM_H


//  INCLUDES
#include <e32base.h>
#include "GFLM.hrh"

// FORWARD DECLARATIONS
class MGflmItemGroup;

// CLASS DECLARATION

/**
*  A base class for all GFLM item classes
*
*  @lib GFLM.lib
*  @since 2.0
*/
class CGflmGroupItem : public CBase
    {

    public:
        // Data types in default priority order
        enum TGflmItemType
            {
            EGlobalActionItem = 0,
            EDrive,
            EDirectory,
            EFile
            };

    public:  // Constructors and destructor

        /**
        * Destructor.
        */
        virtual ~CGflmGroupItem();

    public: // New functions

        /**
        * Returns the type of the item.
        * @since 2.0
        * @return Type of the item.
        */
        virtual TGflmItemType Type() const = 0;

        /**
        * Returns the name of the item.
        * @since 2.0
        * @return A pointer descriptor containing the name of the item.
        */
        virtual TPtrC Name() const = 0;

        /**
        * Returns the date of the item.
        * @since 2.0
        * @return A TTime object containing the date of the item.
        */
        virtual TTime Date() const = 0;

        /**
        * Returns the size.
        * @since 5.0
        * @return Size in bytes
        */
        virtual TInt64 Size() const = 0;

        /**
        * Returns the extension of the item.
        * @since 5.0
        * @return A pointer descriptor containing the extension of the item
        */
        virtual TPtrC Ext() const = 0;

        /**
        * Returns the icon id of the item.
        * @since 3.1
        * @param aIconId Reference of icon id. Filled if icon is defined.
        * @return KErrNotFound if icon id is undefined.
        *         Otherwise KErrNone.
        */
        IMPORT_C TInt GetIconId( TInt& aIconId );

        /**
        * Stores the icon id of the item.
        * @since 3.1
        * @param aIconId Icon id of the item.
        */
        IMPORT_C void SetIconId( const TInt aIconId );

        /**
        * Checks if simple name comparison can be used that is
        * significantly faster compared to normal comparison.
        * @since 3.2
        * @return ETrue if simple name comparison can be used, otherwise EFalse
        */
        TBool CanUseCompareF() const;

        /**
        * Sets the group owning the item
        * @since 3.2
        * @param Pointer to the group or NULL
        */
        void SetGroup( MGflmItemGroup* aGroup );

        /**
        * Gets sort function from method
        * @since 5.0
        * @param aSortMethod Given sort method
        */
        static TLinearOrder< CGflmGroupItem > GetSortL( TGflmSortMethod aSortMethod );

        /**
        * Compares items by name and ignores item type
        * @since 5.0
        * @param aFirst Given first item for comparison
        * @param aSecond Given second item for comparison
        * @return Comparison result
        */
        static TInt CompareByNameWithoutItemType(
            const CGflmGroupItem& aFirst,
            const CGflmGroupItem& aSecond );

        /**
        * Compares items by name with item type
        * @since 5.0
        * @param aFirst Given first item for comparison
        * @param aSecond Given second item for comparison
        * @return Comparison result
        */
        static TInt CompareByName(
            const CGflmGroupItem& aFirst,
            const CGflmGroupItem& aSecond );

        /**
        * Compares items by type
        * @since 5.0
        * @param aFirst Given first item for comparison
        * @param aSecond Given second item for comparison
        * @return Comparison result
        */
        static TInt CompareByType(
            const CGflmGroupItem& aFirst,
            const CGflmGroupItem& aSecond );

        /**
        * Compares items by modified date and time
        * @since 5.0
        * @param aFirst Given first item for comparison
        * @param aSecond Given second item for comparison
        * @return Comparison result
        */            
        static TInt CompareMostRecentFirst(
            const CGflmGroupItem& aFirst,
            const CGflmGroupItem& aSecond );

        /**
        * Compares items by size
        * @since 5.0
        * @param aFirst Given first item for comparison
        * @param aSecond Given second item for comparison
        * @return Comparison result
        */            
        static TInt CompareLargestFirst(
            const CGflmGroupItem& aFirst,
            const CGflmGroupItem& aSecond );

    protected:  // New functions

        /**
        * C++ default constructor.
        */
        CGflmGroupItem();

    private:
        // Internal status flags
        enum TStatus
            {
            ECanUseCompareF = 0x1,
            ECannotUseCompareF = 0x2
            };

        static TInt CompareByItemType(
            const CGflmGroupItem& aFirst,
            const CGflmGroupItem& aSecond );

    private:    // Data
        // Icon id of the item.
        TInt iIconId;
        // Internal status of the item.
        mutable TUint iStatus;
        // Group owning the item. Not owned.
        MGflmItemGroup* iGroup;
    };

#endif      // CGFLMGROUPITEM_H

// End of File
