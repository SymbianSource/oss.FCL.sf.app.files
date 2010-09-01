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
* Description:  File system item definitions
*
*/



#ifndef CGFLMFILESYSTEMITEM_H
#define CGFLMFILESYSTEMITEM_H


//  INCLUDES
#include <e32base.h>
#include "CGflmGroupItem.h"


//  FORWARD DECLARATIONS
class TEntry;


// CLASS DECLARATION

/**
*  A class representing a file system item.
*  A file system item is either a file or a directory.
*
*  @lib GFLM.lib
*  @since 2.0
*/
class CGflmFileSystemItem : public CGflmGroupItem
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CGflmFileSystemItem* NewLC(
            const TEntry& aFSEntry,
            const TDesC& aBasePath );

        /**
        * Destructor.
        */
        virtual ~CGflmFileSystemItem( );

    public: // New functions

        /**
        * Get the full path of the item.
        * @since 2.0
        * @param aPath Modifiable descriptor with atleast the size of
        *        KMaxFileName. The full path of the item will be
        *        placed in this descriptor.
        */
        IMPORT_C void GetFullPath( TDes& aPath ) const;

        /**
        * Sets the localized name of the item.
        * @since 2.0
        * @param aName A descriptor containing the localized name for the
                 entry. A copy of the name is created.
        */
        IMPORT_C void SetLocalizedNameL( const TDesC& aName );

        /**
        * Get reference to TEntry object of the item.
        * @since 3.1
        * @return  A reference to TEntry object.
        */
        IMPORT_C const TEntry& Entry() const;

        /**
        * Get the full path of the item.
        * @since 3.2
        * @return Item full path. The pointer must be freed.
        */
        IMPORT_C HBufC* FullPathLC() const;

        /**
        * Get the full path of the item.
        * @since 3.2
        * @return Item full path. The pointer must be freed.
        */
        IMPORT_C HBufC* FullPathL() const;

    public: // From CGflmGroupItem

        TGflmItemType Type() const;

        TPtrC Name() const;

        TTime Date() const;

        TInt64 Size() const;

        TPtrC Ext() const;

    private:

        void SetExt();

    protected:

        /**
        * C++ default constructor.
        */
        CGflmFileSystemItem(
            const TEntry& aFSEntry,
            const TDesC& aBasePath );

        inline CGflmFileSystemItem()
            {
            }

        inline void SetBasePath( const TDesC& aBasePath )
            {
            iBasePath.Set( aBasePath );
            }
    
        void SetEntry( const TEntry& aFSEntry );

    private:    // Data
        // A reference to a TEntry object that is the actual filesystem entry
        const TEntry* iFSEntry;

        // A pointer descriptor that contains the base path of the
        // filesystem entry.
        TPtrC iBasePath;

        // The localized name of the entry. NULL if it doesn't have one.
        HBufC* iLocalizedName;

        // A pointer descriptor that contains the file extension
        TPtrC iExt;

    };

#endif      // CGFLMFILESYSTEMITEM_H

// End of File
