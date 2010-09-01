/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Main class of the File Manager Engine
*
*/



#ifndef CFILEMANAGERENGINE_H
#define CFILEMANAGERENGINE_H

// INCLUDES
#include <e32base.h>
#include <badesca.h>
#include <f32file.h>
#include <AknServerApp.h>
#include "MFileManagerProcessObserver.h"
#include "FileManagerEngine.hrh"
#include "TFileManagerDriveInfo.h"
#include "CFileManagerRestoreSettings.h"

// CONSTANTS
// File Manager engine spcific error codes
// defined numbers should be far away from system wide error code numbers
const TInt KErrFmgrSeveralFilesInUse    = -4500;
const TInt KErrFmgrDefaultFolder        = -4501;
const TInt KErrFmgrNotSupportedRemotely = -4502;

// FORWARD DECLARATIONS
class CGflmNavigatorModel;
class CFileManagerItemProperties;
class CFileManagerActiveDelete;
class CFileManagerActiveSize;
class CFileManagerActiveExecute;
class CFileManagerRefresher;
class CFileManagerFileSystemEvent;
class CFileManagerUtils;
//class CMGXFileManager;
class CFileManagerItemFilter;
class CFileManagerRemovableDriveHandler;
class CFileManagerRemoteDriveHandler;
class CFileManagerDocHandler;
class CFileManagerBackupSettings;
class CFileManagerRestoreSettings;
class CFileManagerActiveRename;
class CFileManagerFeatureManager;
class MAknServerAppExitObserver;


// CLASS DECLARATION
/**
*  This class implements the main functionalities of File Manager Engine
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerEngine) : public CBase
    {

    public:

        // Engine state
        enum TState
            {
            ENavigation = 0, // For using navigation list model
            ESearch // For using search list model
            };

        // Sort method
        enum TSortMethod
            {
            EByName = 0,
            EByType,
            EMostRecentFirst,
            ELargestFirst,
            EByMatch
            };

    public:
        /**
        * Two-phased constructor.
        * @param aFs Shareable file server session, see RFs::ShareProtected()
        * @return Pointer to a CFileManagerEngine instance
        */
        IMPORT_C static CFileManagerEngine* NewL( RFs& aFs );

        /**
        * Destructor.
        */
        IMPORT_C ~CFileManagerEngine();

    public: // Interface
    
        /**
        * Set currently viewed memory
        * @since 2.0
        * @param aMemory The memory type
        * @return system wide error code
        */
        IMPORT_C TInt SetMemoryL( TFileManagerMemory aMemory );

        /**
        * Returns the currently viewed memory
        * @since 2.0
        * @return TFileManagerMemory, the memory type
        */
        IMPORT_C TFileManagerMemory Memory() const;

        /**
        * Returns selected views item in array, which ListBox can show
        * @since 2.0
        * @return Array of formatted strings of current view
        */
        IMPORT_C MDesCArray* FileList() const;

        /**
        * Engine signals events with this observer
        * @since 2.0
        * @param aObserver The process observer or NULL, ownership is not transferred
        */
        IMPORT_C void SetObserver( MFileManagerProcessObserver* aObserver );

        /**
        * Returns full path of the current directory
        * @since 2.0
        * @return Full path of current directory
        */
        IMPORT_C TPtrC CurrentDirectory() const;

        /**
        * Returns localized name of the current directory
        * @since 2.0
        * @return Full path of current directory
        */
        IMPORT_C TPtrC LocalizedNameOfCurrentDirectory() const;

        /**
        * This backsteps the navigator to previously visible folder
        * @since 2.0
        */
        IMPORT_C void BackstepL();

        /**
        * Returns the type of given item index in CFileManagerItemProperties bitmask
        * @since 2.0
        * @param aIndex The index to current view index.
        * @return CFileManagerItemProperties bitmask
        */
        IMPORT_C TUint32 FileTypeL( const TInt aIndex ) const;

        /**
        * Returns the type of given item index in CFileManagerItemProperties bitmask
        * @since 2.0
        * @param aFullPath  Full path to item which type is needed..
        * @return CFileManagerItemProperties bitmask
        */
        IMPORT_C TUint32 FileTypeL( const TDesC& aFullPath ) const;

        /**
        * Delete file(s) or folder, caller should delete the returned object.
        * @since 2.0
        * @param aIndexList List of indexes from current directory which is going to be deleted
        * @return pointer to newly created CFileManagerActiveDelete object
        */
        IMPORT_C CFileManagerActiveDelete* CreateActiveDeleteL(
            CArrayFixFlat< TInt >& aIndexList );

        /**
        * Creates a new folder
        * @since 2.0
        * @param aFolderName Name of the new folder which is created current directory
        */
        IMPORT_C void NewFolderL( const TDesC& aFolderName );

        /**
        * Set the current item name
        * @since 5.0
        * @param aFileName File name to be set
        */
        IMPORT_C void SetCurrentItemName( const TDesC& aFileName );
        
        /**
        * Rename the file or folder.
        * @since 3.2
        * @param aIndex index of the file or folder from current directory 
        * @param aNewFileName name where that file or folder name is changed
        * @return System wide error code
        */
        IMPORT_C void RenameL( const TInt aIndex, const TDesC& aNewFileName );

        /**
        * Opens the file or folder
        * @since 2.0
        * @param aIndex from the current directory list
        */
        IMPORT_C void OpenL( const TInt aIndex );

        /**
        * Returns the depth of the current folder
        * @since 2.0
        * @return Current depth in folder tree, 0 is root, 1 one folder down
        */
        IMPORT_C TInt FolderLevel();

        /**
        * Checks from current directory, if the name is found
        * @since 2.0
        * @param aString Name which is matched with current directory files and folders
        * @return ETrue if exists
        *         EFalse if not exists in current directory
        */
        IMPORT_C TBool IsNameFoundL(const TDesC& aString);

        /**
        * returns number of the current drive.
        * @since 2.0
        * @returns TDriveNumber type, or KErrNotFound if no current drive set
        */
        IMPORT_C TInt CurrentDrive();

        /**
        * returns info class of the item
        * @since 2.0
        * @param aIndex to current directory item
        * @return class which contains all necessary data for info box, receiver must delete it
        */
        IMPORT_C CFileManagerItemProperties* GetItemInfoL( const TInt aIndex );

        /**
        * returns info about MMC status
        * @since 2.0
        * @return class which contains all necessary data for MMC status
        */
        IMPORT_C TFileManagerDriveInfo GetMMCInfoL() const;

        /**
        * Sets the state of the engine
        * @since 2.0
        * @param aState TState, the state type
        */
        IMPORT_C void SetState( TState aState );

        /**
        * Gets the state of the engine
        * @since 2.0
        * @return TState, the state type
        */
        IMPORT_C CFileManagerEngine::TState State() const;

        /**
        * Updates the current view
        * @since 2.0
        */
        IMPORT_C void RefreshDirectory();

        /**
        * Returns string which was used for finding items
        * @since 2.0
        * @return Ref to string 
        */
        IMPORT_C TPtrC SearchString() const;

        /**
        * Is given file name valid
        * @since 2.0
        * @param aDriveAndPath root path
        * @param aName name of the file
        * @param aIsFolder ETrue folder and EFalse file name
        * @return ETrue if file name is valid
        */
        IMPORT_C TBool IsValidName(
            const TDesC& aDriveAndPath,
            const TDesC& aName,
            TBool aIsFolder ) const;

        /**
        * Check that if given file name contains illegal characters
        * @since 2.0
        * @param aName name of the file
        * @return ETrue if file name is valid
        */
        IMPORT_C TBool IllegalChars( const TDesC& aName ) const;

        /**
        * Check that there is enough memory to do specific operation
        * @since 2.0
        * @param aToFolder Target memory
        * @param aSize Space needed bytes
        * @param aOperation Move or Copy
        * @return ETrue if there is enough space for operation
        */
        IMPORT_C TBool EnoughSpaceL(
            const TDesC& aToFolder,
            TInt64 aSize,
            MFileManagerProcessObserver::TFileManagerProcess aOperation ) const;

        /**
        * Sets the file system event on or off
        * @since 2.0
        * @param aSet   ETrue sets file system event
        *               EFalse turns events off
        */
        IMPORT_C void FileSystemEvent( TBool aEventOn );

        /**
        * Returns the current focus index if rename, copy/move or new folder
        * operation has happened
        * @since 2.0
        * @return   index to current focused item, 
        *           KErrNotFound if not found
        */
        IMPORT_C TInt CurrentIndex();

        /**
        * returns the file name with full path which is behind given index
        * @since 2.0
        * @param aIndex to current directory item
        * @return HBufC buffer which caller must release after usage
        */
        IMPORT_C HBufC* IndexToFullPathL( const TInt aIndex) const; 

        /**
        * returns the file name with full path which is behind given index
        * @since 3.2
        * @param aIndex to current directory item
        * @return HBufC buffer which caller must release after usage
        */
        IMPORT_C HBufC* IndexToFullPathLC( const TInt aIndex ) const;

        /**
        * Checks given item that can it be deleted.
        * @since 2.0
        * @param aFullPath full path to item which is checked
        * @returns ETrue if item can be deleted, EFalse if not
        */
        TBool CanDelete( const TDesC& aFullPath ) const;

        /**
        * returns localized name of the given item
        * @since 2.0
        * @param aFullPath full path to item which is checked
        * @returns TDesC reference to localized name
        */
        IMPORT_C TPtrC LocalizedName( const TDesC& aFullPath ) const;

        /**
        * returns the count of files in folder
        * @since 2.0
        * @return count of files in folder
        */
        IMPORT_C TInt FilesInFolderL();

        /**
        * Cancels the refresh operation
        * @since 2.0
        * @return   ETrue if refresh process is cancelled, 
        *           EFalse if there wasn't any refresh process ongoing
        */
        IMPORT_C TBool CancelRefresh();

        /**
        * Checks given index is it folder, this call is very quick
        * because it uses icon information to retrieve item type
        * @since 2.0
        * @param aIndex index to item 
        * @return   ETrue item behind the index is folder 
        *           EFalse item behind the index is not folder
        */
        IMPORT_C TBool IsFolder( const TInt aIndex ) const;

        /**
        * Asynchronous notifies about drive added or changed
        * @since 5.2
        * @param aPtr Pointer to an instance of this class
        * @return 0
        */
        static TInt DriveAddedOrChangeAsyncL( TAny* aPtr );

        /**
        * Notifies about drive added or changed
        * @since 3.1
        */
        void DriveAddedOrChangedL();

        /**
        * Notifies about folder content changed
        * @since 3.2
        */
        void FolderContentChangedL();

        /**
        * Resolves the icon id of the given item index
        * @since 2.7
        * @param aIndex index to item 
        * @return   Icon id of the item behind the index
        *           
        */
        IMPORT_C TInt IconIdL( const TInt aIndex ) const;

        /**
        * Checks is given folder system folder
        * @since 3.1
        * @param aFull Full path to item
        * @return ETrue if folder is system folder, EFalse if not
        */
        IMPORT_C TBool IsSystemFolder( const TDesC& aFullPath ) const;

        /**
        * Gets current drive name
        * @since 3.2
        * @return Drive name
        */
        IMPORT_C TPtrC CurrentDriveName();

        /**
         * Renames the current drive
         * @since 3.1
         * @param aDrive Drive number EDriveA...EDriveZ
         * @param aName The new name
         * @return System wide error code
         */
        IMPORT_C TInt RenameDrive(
            const TInt aDrive, const TDesC& aName );

        /**
         * Retrieves current drive information
         * @param aInfo A reference to a TFileManagerDriveInfo object
         */
        IMPORT_C void GetDriveInfoL( TFileManagerDriveInfo& aInfo );

        /**
         * Sets password on current drive
         * @since 3.1
         * @param aDrive Drive number EDriveA...EDriveZ
         * @param aOld The old password
         * @param aNew The new password
         * @return System wide error code
         */
        IMPORT_C TInt SetDrivePassword(
            const TInt aDrive,
            const TMediaPassword& aOld,
            const TMediaPassword& aNew );

        /**
         * Removes password from current drive
         * @since 3.1
         * @param aDrive Drive number EDriveA...EDriveZ
         * @param aPwd The current password
         * @return System wide error code
         */
        IMPORT_C TInt RemoveDrivePassword(
            const TInt aDrive,
            const TMediaPassword& aPwd );

        /**
         * Unlocks the current drive
         * @since 3.1
         * @param aDrive Drive number EDriveA...EDriveZ
         * @param aPwd The current password
         * @return System wide error code
         */
        IMPORT_C TInt UnlockDrive(
            const TInt aDrive,
            const TMediaPassword& aPwd );

       /**
         * Starts format process
         * @since 3.1
         * @param aDrive Drive number EDriveA...EDriveZ
         */
        IMPORT_C void StartFormatProcessL( const TInt aDrive );

       /**
         * Starts eject process
         * @since 3.2
         * @param aDrive Drive number EDriveA...EDriveZ
         */
        IMPORT_C void StartEjectProcessL( const TInt aDrive );

        /**
         * Initiates backup or restore process
         * @since 3.1
         * @param aProcess The process to start:
         *                 EBackupProcess or ERestoreProcess
         */
        IMPORT_C void StartBackupProcessL(
            MFileManagerProcessObserver::TFileManagerProcess aProcess );

        /**
         * Cancels ongoing process
         * @since 3.1
         * @param aProcess The process to cancel
         */
        IMPORT_C void CancelProcess(
            MFileManagerProcessObserver::TFileManagerProcess aProcess );

        /**
         * Get drive state
         * @since 3.1
         * @param aState For TFileManagerDriveInfo::TDriveState bits
         * @param aPath Only drive letter is used
         * @return System wide error code
         */
        IMPORT_C TInt DriveState( TUint32& aState, const TDesC& aPath ) const;

        /**
         * Get drive state
         * @since 3.1
         * @param aState For TFileManagerDriveInfo::TDriveState bits
         * @param aDrive
         * @return System wide error code
         */
        IMPORT_C TInt DriveState( TUint32& aState, const TInt aDrive ) const;

        /**
         * Determine if given file must be protected (kept on the device)
         * @since 3.1
         * @param aFullPath Full path to item
         * @param aIsProtected The protection status
         * @return System wide error code
         */
        IMPORT_C TInt IsDistributableFile( const TDesC& aFullPath,
                                           TBool& aIsProtected ) const;

        /**
         * Connects or disconnects remote drive
         * @since 3.1
         * @param aDrive Drive number EDriveA...EDriveZ
         * @param aConnect ETrue when connecting, EFalse when disconnecting
         * @return System wide error code
         */
        IMPORT_C TInt SetRemoteDriveConnection(
            const TInt aDrive, TBool aConnect );

        /**
         * Deletes the remote drive settings from remote storage framework
         * @since 3.1
         * @param aDrive Drive number EDriveA...EDriveZ
         * @return System wide error code
         */
        IMPORT_C TInt DeleteRemoteDrive( const TInt aDrive );

        /**
         * Gets drive root directory
         * @since 3.1
         * @param aDrive Drive number EDriveA...EDriveZ
         * @return Reference to root directory string
         */
        IMPORT_C TPtrC DriveRootDirectory( const TInt aDrive ) const;

        /**
         * Gets backup settings
         * @since 3.1
         * @return Reference to backup settings.
         */
        IMPORT_C CFileManagerBackupSettings& BackupSettingsL();

        /**
         * Gets restore settings
         * @since 3.1
         * @return Reference to restore settings.
         */
        IMPORT_C CFileManagerRestoreSettings& RestoreSettingsL();

        /**
         * Gets drive name
         * @since 3.1
         * @param aDrive Drive that name is required
         * @return Reference to drive name.
         */
        IMPORT_C TPtrC DriveName( const TInt aDrive );

        /**
         * Gets drive info
         * @since 3.1
         * @param aDrive Drive that info is required
         * @param aInfo For storing drive info
         */
        IMPORT_C void GetDriveInfoL(
            const TInt aDrive, TFileManagerDriveInfo& aInfo ) const;

        /**
         * Gets restore info array
         * @since 3.1
         * @param aArray For storing restore info
         * @param aDrive For getting info from specific drive
         */
        IMPORT_C void GetRestoreInfoArrayL(
            RArray< CFileManagerRestoreSettings::TInfo >& aArray,
            const TInt aDrive = KErrNotFound ) const;

        /**
         * Checks if any ejectable drive is present
         * @since 3.1
         * @return ETrue if present, otherwise EFalse
         */
        IMPORT_C TBool AnyEjectableDrivePresent() const;

        /**
         * Ensures that contents of a directory are up to date
         * including remote drives when current view is updated
         * @since 3.1
         */
        IMPORT_C void ForcedRefreshDirectory();

        /**
         * Calculates the total size of given items
         * @since 3.2
         * @param aIndexList List of indexes from current directory
         * @return Total size
         */
        IMPORT_C TInt64 GetFileSizesL(
            const CArrayFixFlat< TInt >& aIndexList );

        /**
         * Gets navigation level, the depth of backstep stack
         * @since 3.2
         * @return The navigation level
         */
        IMPORT_C TInt NavigationLevel() const;

        /**
        * Gets drive name and letter as combined formatted text.
        * @since 5.0
        * @param aDrive Drive number EDriveA...EDriveZ
        * @param aTextIdForDefaultName TextId of default name format string
        * @param aTextIdForName TextId of name format string
        * @return Text containing drive letter and name
        */
        IMPORT_C HBufC* GetFormattedDriveNameLC(
            const TInt aDrive,
            const TInt aTextIdForDefaultName,
            const TInt aTextIdForName = 0 ) const;

        /**
        * Returns info class of the item
        * @since 5.0
        * @param aIndex to current directory item
        * @return class which contains all necessary data for info box, receiver must delete it
        */
        IMPORT_C CFileManagerItemProperties* GetItemInfoLC( const TInt aIndex );

        /**
        * Sets search string.
        * @since 3.2
        * @param aSearchString Search string
        */
        IMPORT_C void SetSearchStringL( const TDesC& aSearchString );

        /**
        * Sets search folder.
        * @since 3.2
        * @param aSearchString Search folder
        */
        IMPORT_C void SetSearchFolderL( const TDesC& aSearchFolder );

        /**
        * Sets the directory with backsteps.
        * @since 3.2
        * @param aDirectory New current directory
        */
        IMPORT_C void SetDirectoryWithBackstepsL( const TDesC& aDirectory );

        /**
        * Stores current navigation index.
        * @since 3.2
        * @param aIndex The index to current view index.
        */
        IMPORT_C void SetCurrentIndex( const TInt aIndex );

        /**
        * Sets the sort method of the engine
        * @since 5.0
        * @param aSortMethod TSortMethod, the sort method type
        */
        IMPORT_C void SetSortMethod( TSortMethod aSortMethod );

        /**
        * Gets the sort method of the engine
        * @since 5.0
        * @return TSortMethod, the sort method type
        */
        IMPORT_C CFileManagerEngine::TSortMethod SortMethod() const;

        /**
        * Re-sorts the current view
        * @since 5.0
        */
        IMPORT_C void RefreshSort();

        /**
        * Gets the feature manager
        * @since 3.2
        */
        IMPORT_C CFileManagerFeatureManager& FeatureManager() const;
        
        /**
        * Set server application exit observer
        * @Param aObserver The ServerAppExit observer or NULL, ownership is not transferred
        */
        IMPORT_C void SetAppExitOb( MAknServerAppExitObserver* aObserver );
        
        /**
        * Deletes the backup archives selected using
        * CFileManagerRestoreSettings::SetSelection() method.
        * @since 5.0
        */
        IMPORT_C void DeleteBackupsL();
        
		/**
        * Check if a drive is mounted
        * @Param aDrive The target drive to check
		* @Return TBool to indicate check result
        * @since 5.2
        */
        IMPORT_C TBool IsDriverMounted( TInt aDrive );
        
    public:
    	
        /**
        * Set whether the file is a sis file
        * @Param aSisFile file is or not a sis file
        */
        void SetSisFile( TBool aSisFile );
        
        /*
        * Notice the ServerAppExit event
        * @Param aReason the reason of the server application exit
        */
        void NoticeServerAppExit( TInt aReason );
        
        /**
        * Sets embedded app status
        * @param aStatus ETrue if embedded app on, EFalse if not
        */
        void EmbeddedAppStatus( TBool aStatus );

        /* MFileManagerWaitNote */
        void ShowWaitDialogL( MAknBackgroundProcess& aProcess );

        //CMGXFileManager& MGXFileManagerL();

        RFs& Fs() const;

        TBool HasAppRemoteDriveSupport( TUid aUid );

        TBool IsRemoteDriveConnected( const TInt aDrive  ) const;

        TBool BackupFileExistsL( const TInt aDrive  ) const;

        // Ensures that drive info is refreshed on next update
        void ClearDriveInfo();

        // Cancels ongoing transfer
        void CancelTransfer( const TDesC& aFullPath );

        /**
        * Gets the latest backup time
        * @since 5.0
        * @param aBackupTime Stores the latest backup time
        * @return System wide error code
        */
        TInt LatestBackupTime( TTime& aBackupTime );

    private:
        /**
        * C++ default constructor.
        */
        CFileManagerEngine( RFs& aFs );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        // Adds current dir fullpath to given name, returns HBufC buffer which must be released
        HBufC* AddFullPathLC( const TDesC& aName, const TBool aIsFolder );

        // Sets file system event watcher for given path
        void SetFileSystemEventL( const TDesC& aFullPath = KNullDesC );

        // For checking mount state
        TInt CheckDriveMountL( TInt aDrive ) const;

        // For getting drive info
        void GetDriveInfoL(
            const TDesC& aPath, TFileManagerDriveInfo& aInfo ) const;

        // Opens file
        void OpenFileL( const TDesC& aFullPath );

        // Opens directory
        void OpenDirectoryL( const TDesC& aFullPath );

        // Checks is current drive available
        TBool CurrentDriveAvailable();

        /**
        * Start asynchronous disk event notification timer
        * @since 5.2
        */
        void StartDiskEventNotifyTimerAsyncL();

        /**
        * Stop asynchronous disk event notification timer
        * @since 5.2
        */
        void StopDiskEventNotifyTimerAsync();

    private:

        // Own: General File List Model is used via this object.
        CGflmNavigatorModel*        iNavigator;

        // Ref: For file operations
        RFs&                        iFs;

        // Normal or search state 
        TState                      iState;

        // Own: String which was used in latest search
        HBufC*                      iSearchString;

        // Own: String which is used for search process
        HBufC*                      iSearchProcessString;

        // Own: AO for async GFLM refresh operation
        CFileManagerRefresher*      iRefresher;

        // Ref: Handle to current observer
        MFileManagerProcessObserver* iProcessObserver;

        // Own: Handles file system events
        CFileManagerFileSystemEvent* iFileSystemEvent;

        // item name of current index, used to search current index
        TFileName                   iCurrentItemName;

        // Own: Folder size calculation
        CFileManagerActiveSize*     iActiveSize;

        // Own: Checks drive notifications
        CFileManagerFileSystemEvent* iDiskEvent;

        // Own: Indicates if embedded application is on
        TBool iEmbeddedApplicationOn;

        // Own: MG2 update notification object
        //CMGXFileManager*            iMgxFileManager;

        // Own: Compines services of couple classes
        CFileManagerUtils*          iUtils;

        // Own: Used for item filtering
        CFileManagerItemFilter* iItemFilter;

        // Own: Used for handling memory card etc functinalities
        CFileManagerRemovableDriveHandler* iRemovableDrvHandler;

        // Own: Used for handling remote drive functinalities
        CFileManagerRemoteDriveHandler* iRemoteDrvHandler;

        // Own: Used for opening files
        CFileManagerDocHandler* iDocHandler;

        // Own: Used for backup settings
        CFileManagerBackupSettings* iBackupSettings;

        // Own: Used for restore settings
        CFileManagerRestoreSettings* iRestoreSettings;

        // Own: Indicates last drive available status
        TBool iLastDriveAvailable;

        // Own: Stores current drive info
        TFileManagerDriveInfo iCurrentDriveInfo;

        // Own: Indicates if current drive info is refreshed
        TBool iCurrentDriveInfoRefreshed;

        // Own: Indicates if wait dialog is on
        TBool iWaitDialogOn;

        // Own: Stores drive name
        HBufC* iDriveName;

        // Own: Navigation indices
        RArray< TInt > iNavigationIndices;

        // Own: Used for rename
        CFileManagerActiveRename* iActiveRename;

        // Own: Used for storing search folder
        HBufC* iSearchFolder;

        // Own: Indicates file system event status
        TBool iFileSystemEventEnabled;

        // Own: Indicates if drive added or changed update is postponed
        TBool iDriveAddedOrChangedPostponed;

        // Own: Manages runtime variated features
        CFileManagerFeatureManager* iFeatureManager;
        
        // Ref: Handle to current ServerAppExit observer
        MAknServerAppExitObserver* iObserver;
        
        //Own: Whether the file was a sis type
        TBool iSisFile;

        //Own: postpone disk event notification while disk refresh is ongoing
        CPeriodic* iDelayedDiskEventNotify;
    };

#endif      // CFILEMANAGERENGINE_H
            
// End of File
