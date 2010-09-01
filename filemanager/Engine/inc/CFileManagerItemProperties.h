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
* Description:  Holds item information
*
*/



#ifndef CFILEMANAGERITEMPROPERTIES_H_
#define CFILEMANAGERITEMPROPERTIES_H_

// INCLUDES
#include <e32base.h>
#include <f32file.h>

// FORWARD DECLARATIONS
class CFileManagerActiveSize;
class CFileManagerUtils;
class CFileManagerActiveCount;
class CGflmGroupItem;
class CFileManagerEngine;

/**
*  Engine gathers the item information to this object.
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerItemProperties) : public CBase
    {
    public:
        /* Bitmask bits of the item types */
        enum TFileManagerFileType 
            {
            ENotDefined         = 0x00,
            EFolder             = 0x01,
            EDefault            = 0x02,
            EMedia              = 0x04,
            EFile               = 0x08,
            EForwardLocked      = 0x10,
            EPlaylist           = 0x20,
            ELink               = 0x40,
            EOpen               = 0x80,
            EReadOnly           = 0x100,
            EDrmProtected       = 0x200,
            EDrmLocalDataFile   = 0x400
            };
    public:
        /**
        * Two-phased constructor.       
        */
        static CFileManagerItemProperties* NewL( 
            const TDesC& aFullPath,
            CFileManagerUtils& aUtils,
            CFileManagerEngine& aEngine );

        /**
        * Two-phased constructor.
        */
        static CFileManagerItemProperties* NewL(
            const CGflmGroupItem& aItem,
            CFileManagerUtils& aUtils,
            CFileManagerEngine& aEngine );

        /**
        * Destructor.
        */
        IMPORT_C ~CFileManagerItemProperties();

    public:

        /**
        * Gets the name of the item
        * @since 2.7
        * @return Name of the item, file name without extension 
        * or directory name
        */
        IMPORT_C TPtrC Name() const;

        /**
        * Gets the item modification time (universal time)
        * @since 2.7
        * @return TTime when item was last modified
        */
        IMPORT_C TTime ModifiedDate() const;

        /**
        * Size of the item, size of the file or size of the dir
        * @since 2.7
        * @return Item size in bytes
        */
        IMPORT_C TInt64 SizeL();

        /**
        * Count of files in directory, subdirectories not included
        * @since 2.7
        * @return File count in dir, -1 if item is file
        */
        IMPORT_C TInt FilesContainedL();

        /**
        * Count of folders in directory, 
        * directories in subdirectories are not included
        * @since 2.7
        * @return Folder count in dir, -1 if item is file
        */
        IMPORT_C TInt FoldersContainedL();

        /**
        * Count of open files in directory
        * @since 2.7
        * @return count of open files in directory, -1 if item is file
        */
        IMPORT_C TInt OpenFilesL();

        /**
        * Returns item type information
        * @since 2.7
        * @return TFileManagerFileType information of item
        */
        IMPORT_C TUint32 TypeL();

        /**
        * Item extension
        * @since 2.7
        * @return File extension, empty if item is dir
        */
        IMPORT_C TPtrC Ext() const;
        
        /**
        * Fullpath of item, includes drive, path and name with extension
        * @since 2.7
        * @return fullpath 
        */
        IMPORT_C TPtrC FullPath() const;

        /**
        * Gets the name of the item with extension
        * @since 2.7
        * @return Name of the item
        */
        IMPORT_C TPtrC NameAndExt() const;

        /**
        * Localized name of item if found, if not Name() is returned
        * @since 2.7
        * @return Localized name
        */
        IMPORT_C TPtrC LocalizedName() const;

        /**
        * Gets the item modification time (local time)
        * @since 3.1
        * @param aTime For local time
        * @return Error code
        */
        IMPORT_C TInt ModifiedLocalDate( TTime& aTime ) const;

        /**
        * Checks if item contains any files or folders
        * @since 3.1
        * @return ETrue if contains any items, EFalse if not
        */
        IMPORT_C TBool ContainsAnyFilesOrFolders();

        /**
        * Gets MIME type of file
        * @since 3.1
        * @return MIME type
        */
        IMPORT_C TPtrC MimeTypeL();

        /**
        * Checks if item is a drive item
        * @since 3.1
        * @return ETrue if drive item, otherwise EFalse
        */
        IMPORT_C TBool IsDrive() const;

        /**
        * Gets drive name
        * @since 3.1
        * @return drive name
        */
        IMPORT_C TPtrC DriveName() const;

        /**
        * Checks if item is on remote drive
        * @since 3.2
        * @return ETrue if item on remote drive, otherwise EFalse
        */
        IMPORT_C TBool IsRemoteDrive() const;

        /**
        * Gets drive id
        * @since 5.0
        * @return Drive id
        */
        IMPORT_C TInt DriveId() const;

    private:
        /**
        * C++ default constructor.
        */
        CFileManagerItemProperties( 
            CFileManagerUtils& aUtils,
            CFileManagerEngine& aEngine );
            
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const TDesC& aFullPath );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const TDesC& aFullPath, const TEntry& aEntry );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const CGflmGroupItem& aItem );

        // Counts file sizes in given folder
        TInt64 FolderSizeL( const TDesC& aFullPath );

        // Counts items in directory, given parameter defines the type of count
        TInt CountItemsL(
            const TFileManagerFileType& aType );

        void EnsureEntryDataFetched() const;

        void SetEntryData( const TEntry& aEntry ) const;

    private:

        // Own: Location in file system
        HBufC* iFullPath;

        // Size of the file in bytes. In case of folder, this is a sum of all items
        // under the folder including the subfolders.
        mutable TInt64 iSize;

        // How many files are inside the folder, subfolders not counted
        // In case of file this is undefined
        TInt iFilesContained;

        // How many folders are inside the folder, subfolders are not counted
        // In case of file this is undefined
        TInt iFoldersContained;

        // How many open files are inside the folder
        // In case of file this is undefined
        TInt iOpenFiles;

        // type of item, TFileManagerFileType bitmask
        TUint32 iType;

        // Own: Folder size calculation
        CFileManagerActiveSize* iActiveSize;

        // Own: Folder properties 
        CFileManagerActiveCount* iActiveCount;

        // Item modification day, copied from items TEntry
        mutable TTime iModified;

        // Ref: Contains some useful common services, localizer, RFs...
        CFileManagerUtils& iUtils;

        // Ref: Contains some useful common services
        CFileManagerEngine& iEngine;

        enum TItemState // Internal state bit definitions
            {
            EItemDirectory = 0x1,
            EItemDrive = 0x2,
            EItemAction = 0x4,
            EItemFile = 0x8,
            EItemHasFilesOrFolders = 0x10,
            EItemHasNoFilesOrFolders = 0x20,
            EItemRemoteDrive = 0x40,
            EItemEntryDataFetched = 0x80,
            EItemNotFileOrDir = EItemDrive | EItemAction
            };
        // Internal item state bits, see above
        mutable TUint32 iState;

        // Own: Item name
        HBufC* iName;

    };



#endif // CFILEMANAGERITEMPROPERTIES_H_