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
* Description:  Util methods for File Manager
*
*/



#ifndef CFILEMANAGERUTILS_H
#define CFILEMANAGERUTILS_H


// INCLUDES
#include <e32base.h>
#include <badesca.h>
#include <f32file.h>


// FORWARD DECLARATIONS
class CFileManagerMimeIconArray;
class CFileManagerFolderArray;
class CCommonContentPolicy;
class CGflmNavigatorModel;
class CGflmGroupItem;
#if defined( __KEEP_DRM_CONTENT_ON_PHONE ) && defined ( FILE_MANAGER_USE_DISTRIBUTABLECHECKER )
 class CDistributableChecker;
#endif // __KEEP_DRM_CONTENT_ON_PHONE && FILE_MANAGER_USE_DISTRIBUTABLECHECKER
class CFileManagerFeatureManager;


/**
*  
*  Collection of utility functions for file manager
*
*  @lib FileManagerEngine.lib
*  @since 2.7
*/
NONSHARABLE_CLASS(CFileManagerUtils) : public CBase
    {
    public:
        /**
        * Two-phased constructors.
        */
        static CFileManagerUtils* NewL( 
            RFs& aFs,
            CGflmNavigatorModel& aNavigator,
            CFileManagerFeatureManager& aFeatureManager );

        /**
        * Destructor.
        */
        ~CFileManagerUtils();
    public: // New functions
        
        /**
        * Gets the reference to open file server session
        * @since 2.7
        * @return reference to open file server session
        */
        RFs& Fs() const;

        /**
        * Checks is given file forward locked
        * @since 2.7
        * @param aFullPath file name with full path
        * @return ETrue if file is forward locked, EFalse if not.
        */
        TBool IsForwardLockedFile( const TDesC& aFullPath );

        /**
        * Checks is given file drm protected
        * @since 2.7
        * @param aFullPath file name with full path
        * @return ETrue if file is drm protected, EFalse if not.
        */
        TBool IsDrmProtectedFile( const TDesC& aFullPath) const;
        
        /**
        * Gets the icon id of the item
        * @since 2.7
        * @param aFullPath Item with full path information
        * @return Icon id of the item. If not found other icon id is used.
        */
        TInt ResolveIconL( const TDesC& aFullPath ) const;

        /**
        * Checks is given folder a default folder
        * @since 2.7
        * @param aFullPath Item with full path information
        * @return ETrue if folder is default folder, EFalse if not.
        */
        TBool DefaultFolder( const TDesC& aFullPath ) const;

        /**
        * Checks is given file drm local data file (LDF)
        * @since 3.1
        * @param aFullPath file name with full path
        * @return ETrue if file is local data file, EFalse if not.
        */
        TBool IsDrmLocalDataFile( const TDesC& aFullPath ) const;

        /**
        * Checks given item that can it be deleted.
        * @since 3.1
        * @param aFullPath full path to item which is checked
        * @returns ETrue if item can be deleted, EFalse if not
        */
        TBool CanDelete( const TDesC& aFullPath ) const;

        /**
        * Returns the type of given item in CFileManagerItemProperties bitmask
        * @since 3.1
        * @param aFullPath full path to item
        * @return CFileManagerItemProperties bitmask
        */
        TUint32 FileTypeL( const TDesC& aFullPath );

        /**
        * Gets the icon id of the item
        * @since 3.1
        * @param aItem GFLM struct of the item
        * @return Icon id of the item. If not found other icon id is returned.
        */
        TInt ResolveIconL( CGflmGroupItem& aItem ) const;

        /**
        * Checks does given folder have any items with specified attributes
        * @since 3.1
        * @param aFs open file server session
        * @param aFullPath folder name with full path
        * @param aAttMask attribute mask
        * @return ETrue if folder has any specified items, EFalse if not.
        */
        static TBool HasAny(
            RFs& aFs, const TDesC& aFullPath,  TUint aAttMask );

        /**
        * Checks does given folder has any subfolder
        * @since 3.1
        * @param aFullPath folder name with full path
        * @return ETrue if folder has any subfolders, EFalse if not.
        */
        TBool HasSubFolders( const TDesC& aFullPath ) const;

        /**
        * Checks does given folder has any files
        * @since 3.1
        * @param aFullPath folder name with full path
        * @return ETrue if folder has any files, EFalse if not.
        */
        TBool HasFiles( const TDesC& aFullPath ) const;

        /**
        * Create default folders
        * @since 3.2
        * @param aDrive Drive which default folders are created
        * @param aForced Selects forced or normal creation
        */
        void CreateDefaultFolders(
            const TInt aDrive, const TBool aForced = ETrue ) const;

        /**
        * Checks is path located in remote drive
        * @since 3.1
        * @param aPath path to check
        * @return ETrue if remote drive. Otherwise EFalse.
        */
        TBool IsRemoteDrive( const TDesC& aPath ) const;

        /**
         * Determine if given file must be protected (kept on the device)
         * @since 3.1
         * @param aFullPath file to check
         * @param aIsProtected for storing result
         * @return System wide error code
         */
        TInt IsDistributableFile( const TDesC& aFullPath,
                                  TBool& aIsProtected );

        /**
        * Gets MIME type of file
        * @since 3.1
        * @return MIME type
        */
        TPtrC MimeTypeL( const TDesC& aFullPath ) const;

        /**
        * Gets drive id array with specified attribute mask
        * @since 3.2
        * @param aDrives array for ids
        * @param aAttMask drive attribute match mask
        */
        void DriveList(
            RArray< TInt >& aDrives, const TInt aAttMask ) const;

        /**
        * Checks is drive removable
        * @since 3.2
        * @param aPath path to check
        * @return ETrue if removable drive. Otherwise EFalse
        */
        TBool IsRemovableDrive( const TDesC& aPath ) const;

        /**
        * Gets item full path
        * @since 3.2
        * @param aDir Parent directory path
        * @param aEntry File system entry
        * @param aFullPath Full path to item
        */
        static void GetFullPath(
            const TDesC& aDir, const TEntry& aEntry, TDes& aFullPath );

        /**
        * Removes final backslash if exists
        * @since 3.2
        * @param aPath Directory path
        * @return Pointer without backslash
        */
        IMPORT_C static TPtrC StripFinalBackslash( const TDesC& aPath );

        /**
        * Ensures final backslash exists
        * @since 3.2
        * @param aPath Directory path
        */
        IMPORT_C static void EnsureFinalBackslash( TDes& aPath );

        /**
        * Removes all directional chars if exist
        * @since 3.2
        * @param aText The text
        * @return Pointer to cleaned up text, ownership is transferred
        */
        static HBufC* StripDirectionalCharsLC( const TDesC& aText );

        /**
        * Checks if final backslash exist
        * @since 3.2
        * @param aPath Directory path
        */
        IMPORT_C static TBool HasFinalBackslash( const TDesC& aPath );

        /**
        * Checks if file is copied/moved from internal to removable drive
        * @since 3.2
        * @param aFs Open file server session
        * @param aSrcPath Source directory path
        * @param aDstPath Destination directory path
        * @return ETrue if from internal to removable, otherwise EFalse
        */
        IMPORT_C static TBool IsFromInternalToRemovableDrive(
            RFs& aFs, const TDesC& aSrcPath, const TDesC& aDstPath );

        /**
        * Removes read only attribute if it is set for given item
        * @since 3.2
        * @param aFs Open file server session
        * @param aFullPath Full path of the item
        * @param aEntry Entry data of the item
        * @return System wide error code
        */
        static TInt RemoveReadOnlyAttribute(
            RFs& aFs, const TDesC& aFullPath, const TEntry& aEntry );

        /**
        * Removes read only attribute if it is set for given item
        * @since 3.2
        * @param aFs Open file server session
        * @param aFullPath Full path of the item
        * @return System wide error code
        */
        static TInt RemoveReadOnlyAttribute(
            RFs& aFs, const TDesC& aFullPath );

        /* Drive name layouts for selecting the most suitable text */
        enum TDriveNameLayout
            {
            EMainLayout = 0,
            ETitleLayout,
            ELayoutCount  // Must be the last
            };

        /**
        * Gets drive name and letter as combined single text.
        * @since 5.0
        * @param aDrive Drive number EDriveA...EDriveZ
        * @param aLayout Text layout. See TDriveNameLayout.
        * @return Text containing drive letter and name
        */
        HBufC* GetDriveNameL(
            const TInt aDrive,
            const TDriveNameLayout aLayout ) const;

        /**
        * Gets drive name and letter as combined formatted text.
        * @since 5.0
        * @param aDrive Drive number EDriveA...EDriveZ
        * @param aTextIdForDefaultName TextId of default name format string
        * @param aTextIdForName TextId of name format string
        * @return Text containing drive letter and name
        */
        HBufC* GetFormattedDriveNameLC(
            const TInt aDrive,
            const TInt aTextIdForDefaultName,
            const TInt aTextIdForName ) const;

        /**
        * Checks is path located in remote drive
        * @since 3.2
        * @param aFs Open file server session
        * @param aPath path to check
        * @return ETrue if remote drive, otherwise EFalse
        */
        IMPORT_C static TBool IsRemoteDrive( RFs& aFs, const TDesC& aPath );

        /**
        * Returns localized name of the given item
        * @since 3.2
        * @param aFullPath full path to item which is checked
        * @returns TPtrC reference to localized name,
        *          KNullDesC if no localization is used
        */
        TPtrC LocalizedName( const TDesC& aFullPath ) const;

        /**
        * Checks is name found, the comparison includes localized folder names
        * @since 3.2
        * @param aFullPath full path to item which is checked
        * @returns ETrue if name is found, otherwise EFalse. 
        */
        TBool IsNameFoundL( const TDesC& aFullPath ) const;
        
        /**
        * Flush Cache
        */
        void FlushCache() const;
        
    private:
        /**
        * C++ default constructor.
        */
        CFileManagerUtils( 
            RFs& aFs,  
            CGflmNavigatorModel& aNavigator,
            CFileManagerFeatureManager& aFeatureManager );

        void ConstructL();

        static void IsFlkOrLdfFileL(
            const TDesC& aFullPath, TBool& aIsFlk, TBool& aIsLdf );

        static TBool IsInternalDrive( RFs& aFs, const TInt aDrive );

        CDesCArray* GetDriveNameArrayLC(
            const TInt aDrive,
            TUint& aDriveStatus ) const;

        static TBool IsDrive(
            RFs& aFs, const TDesC& aPath, const TUint aAttMask );

        static TBool IsDrive(
            RFs& aFs, const TInt aDrive, const TUint aAttMask );

    private: 
    
        // Ref: reference to open file server session
        RFs&    iFs;

        //Own: List of default folders
        CFileManagerFolderArray*    iDefaultFolders;

        // Own: Handle to common content policy service to find 
        // out is file in a Closed Content List or forward locked
        CCommonContentPolicy*       iCommonContentPolicy;

        // Own: Mime icon mapper
        CFileManagerMimeIconArray*  iMimeIconMapper;

        // Ref: GFLM navigator instance
        CGflmNavigatorModel&        iNavigator;

#if defined( __KEEP_DRM_CONTENT_ON_PHONE ) && defined ( FILE_MANAGER_USE_DISTRIBUTABLECHECKER )
        // Own: content protection checking
        CDistributableChecker*      iDistributableChecker;
#endif // __KEEP_DRM_CONTENT_ON_PHONE && FILE_MANAGER_USE_DISTRIBUTABLECHECKER

        // Flag to indicate if feature is supported
        TBool iDrmFullSupported;

        // Ref: reference to feature manager
        CFileManagerFeatureManager& iFeatureManager;

    };

#endif      // CFILEMANAGERUTILS_H
            
// End of File
