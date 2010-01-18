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
* Description:  Holds one default folder entry
*
*/


#ifndef CFILEMANAGERFOLDERENTRY_H
#define CFILEMANAGERFOLDERENTRY_H

// INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS
class TResourceReader;

/**
*  One default folder entry.
*  Contains fullpath of the default folder and information
*  is it media folder or nor.
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerFolderEntry) : public CBase
    {
    public:
        /**
        * Two-phased constructors.
        */
        static CFileManagerFolderEntry* NewLC( TResourceReader& aReader );

        static CFileManagerFolderEntry* NewL( TResourceReader& aReader );

        static CFileManagerFolderEntry* NewLC(
            const TInt aDrive, const TDesC& aPath );

        /**
        * Destructor.
        */
        ~CFileManagerFolderEntry();
    public: // New functions

        /**
        * Is the default folder media folder or not
        * @since 2.0
        * @return ETrue if default folder is media folder, EFalse if not
        */
        TBool IsMediaFolder() const;

        /**
        * Returns reference to full path information of default folder
        * @since 2.0
        * @return Reference to default folder full path descriptor.
        */
        TPtrC Name() const;

        /**
        * Used for sorting folder entries
        * @since 3.2
        * @param aFirst Reference to the first folder entry
        * @param aSecond Reference to the second folder entry
        * @return Comparison result.
        */
        static TInt CompareSort(
            const CFileManagerFolderEntry& aFirst,
            const CFileManagerFolderEntry& aSecond );

        /**
        * Used for finding folder from sorted entries
        * @since 3.2
        * @param aPath Path to find
        * @param aItem Reference to the folder entry
        * @return Comparison result.
        */
        static TInt CompareFind(
            const TDesC* aPath,
            const CFileManagerFolderEntry& aItem );

    private:
        /**
        * C++ default constructor.
        */
        CFileManagerFolderEntry();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructFromResourceL( TResourceReader& aReader);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const TInt aDrive, const TDesC& aPath );

    private: // Date
        // Own: Full path information of default folder
        HBufC* iName;

        // Is the default folder media folder
        TBool iIsMediaFolder;

    };


#endif      // CFILEMANAGERFOLDERENTRY_H
            
// End of File
