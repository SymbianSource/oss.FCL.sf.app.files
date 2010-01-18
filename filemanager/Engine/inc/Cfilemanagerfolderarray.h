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
* Description:  Holds the array of CFileManagerFolderEntry
*
*/



#ifndef CFILEMANAGERFOLDERARRAY_H
#define CFILEMANAGERFOLDERARRAY_H


// INCLUDES
#include <e32base.h>


// FORWARD DECLARATIONS
#ifndef RD_MULTIPLE_DRIVE
class CFileManagerFolderEntry;
#endif // RD_MULTIPLE_DRIVE
class CFileManagerUtils;


// CLASS DECLARATION


/**
*  Array of CFileManagerFolderEntry's
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerFolderArray) : public CBase
    {
    public:
        /**
        * Two-phased constructors.
        */        
#ifdef RD_MULTIPLE_DRIVE
        static CFileManagerFolderArray* NewL( CFileManagerUtils& aUtils );
#else // RD_MULTIPLE_DRIVE
        static CFileManagerFolderArray* NewL(
            TInt aResId, CFileManagerUtils& aUtils );
#endif // RD_MULTIPLE_DRIVE

        /**
        * Is the given folder in the default folder list
        * @since 2.0
        * @param aFolderName folder which is compared to default folder list
        * @return ETrue if the given folder is in list, EFalse if not
        */
        TBool Contains( const TDesC& aFolderName );

        /**
        * Create folders from array
        * @since 3.2
        * @param aDrive Drive which default folders are created
        * @param aForced Selects forced or normal creation
        */
        void CreateFolders(
            const TInt aDrive = KErrNotFound,
            const TBool aForced = ETrue );

        /**
        * Destructor.
        */
        ~CFileManagerFolderArray();
    private:
        /**
        * C++ default constructor.
        */
        CFileManagerFolderArray( CFileManagerUtils& aUtils );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( TInt aResId );

#ifndef RD_MULTIPLE_DRIVE
        /**
        * Goes through the array of CFileManagerEntry's and tries to 
        * lookup if the given folder is in the array.
        * @since 2.0
        * @return CFileManagerFolderEntry if found, otherwise NULL.
        */
        CFileManagerFolderEntry* FindFolder( const TDesC& aFolderName );
#endif // RD_MULTIPLE_DRIVE

        void DoCreateFoldersL( const TInt aDrive );

    private: // Data
        // Ref: Reference to utils
        CFileManagerUtils& iUtils;

        // Own: For checking if folder creation has been done or not
        TUint iCreateCheckMask;

#ifndef RD_MULTIPLE_DRIVE
        // Own: Array of CFileManagerFolderEntry's
        RPointerArray< CFileManagerFolderEntry > iFolderArray;

        // Own: Indicates if folder array has been sorted
        TBool iSorted;
#endif // RD_MULTIPLE_DRIVE

    };

#endif      // CFILEMANAGERFOLDERARRAY_H
            
// End of File
