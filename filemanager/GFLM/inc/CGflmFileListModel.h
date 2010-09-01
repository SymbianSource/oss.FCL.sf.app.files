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
* Description:  File list model
*
*/



#ifndef CGFLMFILELISTMODEL_H
#define CGFLMFILELISTMODEL_H


//  INCLUDES
#include <e32base.h>
#include <badesca.h>        // MDesCArray
#include <f32file.h>        // RFs
#include "GFLM.hrh"


// FORWARD DECLARATIONS
class MGflmItemGroup;
class CGflmItemGroupImpl;
class CGflmGroupItem;
class MGflmItemFilter;
class CGflmDirectoryListingCache;
class CGflmFileRecognizer;
class CGflmItemLocalizer;
class CGflmDriveResolver;
class CGflmDriveItem;
class CGflmFileFinder;


// CLASS DECLARATION
/**
*  A class that implements the basic file list functionality.
*  @lib GFLM.lib
*  @since 2.0
*/
class CGflmFileListModel : public CActive, public MDesCArray
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @since 3.2
        * @param aFs Shareable file server session, see RFs::ShareProtected()
        * @return Pointer to a CGflmFileListModel instance
        */
        IMPORT_C static CGflmFileListModel* NewL( RFs& aFs );

        /**
        * Destructor.
        */
        virtual ~CGflmFileListModel();

    public: // New functions

        /**
        * Refreshes the model
        * @since 2.0
        * @param aStatus A reference to a TRequestStatus object that
        *        is used to notify the client thread about completion of
        *        the refresh
        * @param aRefreshMode Refresh mode
        */
        IMPORT_C void RefreshListL(
            TRequestStatus& aStatus,
            TGflmRefreshMode aRefreshMode = ERefreshItems );

        /**
        * Cancels the asynchronous refresh request
        * @since 2.0
        */
		IMPORT_C void CancelRefresh();

        /**
        * Obtains a pointer to a CGflmGroupItem object.
        * The item is indexed by a global index.
        * Raises a panic (EGlobalIndexOutOfBounds) if the index refers to a
        * non-existent item.
        * @since 2.0
        * @param aIndex Global index of the item.
        * @return A pointer to a CGflmGroupItem object.
        */
        IMPORT_C CGflmGroupItem* Item( TInt aIndex ) const;

        /**
        * Resets the model. All item groups are destroyed.
        * @since 2.0
        */
        IMPORT_C void ResetModel();

        /**
        * Obtains a pointer to a MGflmItemGroup object.
        * Raises a panic (EInvalidGroupId) if a group is not found with given id.
        * @since 2.0
        * @param aId Id of the group
        * @return A pointer to a MGflmItemGroup object.
        */
        IMPORT_C MGflmItemGroup* FindGroupById( TInt aId );

        /**
        * Creates a new item group with a given id.
        * @since 2.0
        * @param aId The id given to the created group
        * @return A pointer to the created MGflmItemGroup object
        */
        IMPORT_C MGflmItemGroup* CreateGroupL( TInt aId );

        /**
        * Creates a new item group with a given id.
        * @since 3.1
        * @param aId The id given to the created group
        * @param aInclusion The group item inclusion
        * @return A pointer to the created MGflmItemGroup object
        */
        IMPORT_C MGflmItemGroup* CreateGroupL(
            TInt aId, TUint aInclusion );

        /**
        * Recognizes a file and resolves it's MIME-type.
        * This method accesses GFLM's internal recognizer cache
        * and is thus recomended for efficiency. NOTE: The file doesn't
        * have to belong to the current state of the model.
        * @since 2.0
        * @param aFilename Full path of the file to be recognized
        * @return A pointer descriptor containing the MIME-type of the item.
        *         If the MIME-type couldn't be recognized, returns KNullDesC
        */
        IMPORT_C TPtrC ResolveMimeTypeL( const TDesC& aFilename );

        /**
        * Sets the item filter object for the model to use.
        * If aOwned is ETrue the model gets the ownership of the object.
        * @since 2.0
        * @param aFilter A pointer to a MGflmItemFilter object to be used
        */
        IMPORT_C void SetCustomFilter( MGflmItemFilter* aFilter );

        /**
        * Returns the validity of the model. If the refresh operation failed
        * or was canceled the model is corrupt and it's items shouldn't be
        * accessed
        * @since 2.0
        * @return ETrue if the model is corrupt. EFalse if the model is in
        *         valid state.
        */
        IMPORT_C TBool IsCorrupt() const;

        /**
        * Returns the localized name of a file or a directory
        * @since 2.0
        * @param aPath Full path to the file or directory
        * @return A pointer descriptor containing the localized name
        *         without the full path
        */
        IMPORT_C TPtrC LocalizedName( const TDesC& aPath ) const;

        /**
        * Clears the contents of the file cache
        * @since 2.0
        */
        IMPORT_C void ClearCache();

        /**
        * Clears the contents of the drive info
        * @since 3.1
        */
        IMPORT_C void ClearDriveInfo();

        /**
        * Gets drive item from given path
        * @since 3.1
        * @param aPath Given path
        * @return A pointer to drive item (not owned)
        *         or NULL if drive not found.
        */
        IMPORT_C CGflmDriveItem* DriveFromPath( const TDesC& aPath ) const;

        /**
        * Gets drive item from given id
        * @since 3.2
        * @param aDrive Given drive id
        * @return A pointer to drive item (not owned)
        *         or NULL if drive not found.
        */
        IMPORT_C CGflmDriveItem* DriveFromId( const TInt aDrive ) const;

        /**
        * Enables and disables search mode
        * @since 3.2
        * @param aEnabled Enable or disable
        */
        IMPORT_C void EnableSearchMode( const TBool aEnabled );

        /**
        * Get search mode
        * @since 5.0
        * @return search mode
        */       
        IMPORT_C TBool SearchMode();
        /**
        * Sets search string. Wild cards are allowed.
        * @since 3.2
        * @param aSearchString Given search string
        */
        IMPORT_C void SetSearchStringL( const TDesC& aSearchString );

        /**
        * Sets search folder
        * @since 3.2
        * @param aSearchFolder Given search folder
        */
        IMPORT_C void SetSearchFolderL( const TDesC& aSearchFolder );

        /**
        * Sets sort method
        * @since 5.0
        * @param aSortMethod Given sort method
        */
        IMPORT_C void SetSortMethod( TGflmSortMethod aSortMethod );

        /**
        * Gets sort method
        * @since 5.0
        * @return Currently used sort method
        */
        IMPORT_C TGflmSortMethod SortMethod() const;
        
        /**
        * Flush cache in CGflmFileRecognizer
        */
        IMPORT_C void FlushCache();
        
    public: // MDesCArray

        TInt MdcaCount() const;

        TPtrC MdcaPoint( TInt aIndex ) const;

    private: // CActive

        void RunL();

        TInt RunError( TInt aError );

        void DoCancel();

    protected:  // New functions

        /**
        * C++ default constructor.
        */
        CGflmFileListModel( RFs& aFs );

        /**
        * 2nd phase constructor
        */
        void ConstructL();

    private:

        static TInt WorkThreadStart( TAny* aPtr );

        void WorkThread();

        void NotifyClient( TInt aErr );

        void GetGroupAndLocalIndex(
            const TInt aGlobalIndex,
            TInt& aLocalIndex,
            MGflmItemGroup*& aGroup ) const;

        void DoRefreshL();

    protected:  // Data

        // Ref: An open shareable file server session
        RFs& iFs;

        // Request status of the work thread client
        TRequestStatus* iClientStatus;

        // Dynamic array containing all the CGflmItemGroupImpl objects that
        // belong to the model. Owned.
        CArrayPtrSeg< CGflmItemGroupImpl >* iGroups;

        // Instance of CGflmDirectoryListingCache. Owned.
        CGflmDirectoryListingCache* iListingCache;

        // Instance of CGflmFileRecognizer. Owned.
        CGflmFileRecognizer* iFileRecognizer;

        // Instance of CGflmItemLocalizer. Owned.
        CGflmItemLocalizer* iItemLocalizer;

        // Instance of CGflmDriveResolver. Owned.
        CGflmDriveResolver* iDriveResolver;

        // A boolean indicating wether the model is corrupt and needs to be
        // refreshed
        TBool iCorrupt;

        // A pointer to a class implementing MGflmItemFilter interface
        MGflmItemFilter* iFilter;

        // Own: For synchronising client and work thread
        RSemaphore iSemaphore;

        // Own: Client thread id to be informed
        TThreadId iClientId;

        // A boolean indicating wether the model is shutting down
        // Main thread is allowed write and work thread is allowed to read only
        TBool iShutdown;

        // A boolean indicating wether the refresh is canceled
        // Main thread is allowed write and work thread is allowed to read only
        TBool iCanceled;

        // A boolean indicating if search mode enabled or disabled
        // Main thread is allowed write and work thread is allowed to read only
        TBool iSearchEnabled;

        // Own: For finding files and directories from file system
        CGflmFileFinder* iFileFinder;

        // Own: Stores currently used sort method
        // Main thread is allowed write and work thread is allowed to read only
        TGflmSortMethod iSortMethod;

        // Own: Stores currently used sort method in search mode
        // Main thread is allowed write and work thread is allowed to read only
        TGflmSortMethod iSearchSortMethod;

        // Own: Stores currently used refresh mode
        // Main thread is allowed write and work thread is allowed to read only
        TGflmRefreshMode iRefreshMode;

    };

#endif      // CGFLMFILELISTMODEL_H

// End of File
