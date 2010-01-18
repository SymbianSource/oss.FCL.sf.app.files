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
* Description:  Deletes items
*
*/


#ifndef CFILEMANAGERACTIVEDELETE_H
#define CFILEMANAGERACTIVEDELETE_H

// INCLUDES
#include <e32base.h>
#include <badesca.h>
#include <AknWaitNoteWrapper.h>
// FORWARD DECLARATIONS
class CFileManagerEngine;
class CDirScan;
//class CMGXFileManager;
class CFileManagerUtils;


// CLASS DECLARATION
/**
*  Deletes items
*  Delete operation is done one item at time, so that it can be cancelled
*  during operation.
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerActiveDelete) : public CBase,
                                              public MAknBackgroundProcess
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CFileManagerActiveDelete* NewL(
            CArrayFixFlat< TInt >& aIndexList,
            CFileManagerEngine& aEngine,
            CFileManagerUtils& aUtils );

        /**
        * Destructor.
        */
        IMPORT_C ~CFileManagerActiveDelete();
        
    public: // From MAknBackgroundProcess

        void StepL();
        
        TBool IsProcessDone() const;

        void DialogDismissedL( TInt aButtonId );

    public:
        
        /**
        * Returns last known error code
        * @since 2.0
        * @param aFileName a refence to reserved buffer
                 where file name related to error can be copied
        * @return common error code
        */        
        IMPORT_C TInt GetError(TDes& aFileName);

        /**
        * Returns count of deleted DRM items
        * @since 2.0
        * @param aTotalCount (out) returns total count of deleted items
        * @return count of deleted DRM items
        */        
        IMPORT_C TInt DeletedDrmItems( TInt& aTotalCount );

        
    private:
        /**
        * Constructor
        */        
        CFileManagerActiveDelete(
            RFs& aFs,
            CArrayFixFlat< TInt >& aIndexList,
            CFileManagerEngine& aEngine,
            CFileManagerUtils& aUtils );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
    private: 
        
        /**
        *  This function handles the directory item deletion.
        *  Deletes one item per call
        */
        TBool DeleteItemsInDirectoryL();

        /**
        *  This function handles the file deletion.
        */
        void DeleteFileL(
            const TDesC& aFullPath, const TBool aReadOnlyChecked = EFalse );

        /**
        *  This function checks is item file or directory.
        */
        TBool IsDir( const TDesC& aFullPath );

        /**
        *  This function sets the name returned to user.
        */
        void SetName( const TDesC& aFullPath, TBool aOverWrite = EFalse );

        /**
        *  This function checks is error ignored by delete.
        */
        TBool IsError( TInt aErr );

        /**
        *  This function sets the error returned to user.
        */
        void SetError( TInt aErr, const TDesC& aFullPath );

        /**
        *  This function flushes the notification array.
        */
        void FlushNotifications();

    private: // Data
        /// Ref: reference to engine created file server session
        RFs& iFs;

        /// Ref: array of items to be deleted from current view
        CArrayFixFlat< TInt >& iIndexList;
        
        /// Current index in given array
        TInt iCurrentIndex;
        
        /// When all folders are through, this is set
        TBool iProcessDone;

        /// Latest error
        TInt iError;
        
        /// Own: latest deleted file name or file name which delete has failed
        TFileName iFileName;
        
        /// current file index in CDirScan
        TInt iFileIndex;
        
        /// Own: Used to scan directories
        CDirScan* iDirScan;
        
        /// Own: Scan result is stored here
        CDir*     iDir;
                
        /// Count of not deleted items
        TInt iNotDeletedItems;
        
        /// Count of open items
        TInt iOpenFiles;
        
        /// Own: buffer for storing the current item name to be deleted
        HBufC* iStringBuffer;
        
        /// Ref: reference to engine instance
        CFileManagerEngine& iEngine;

        /// Own: full path of the item to be deleted
        HBufC* iFullPath;

        /// Count of deleted drm files
        TInt    iDeletedDrmItems;

        /// Count of deleted items
        TInt    iDeletedItems;

        // Ref: MG2 update notification object
        //CMGXFileManager*            iMgxFileManager;

        // Ref: reference to utils instance
        CFileManagerUtils& iUtils;

        // Own: Set if current drive is remote drive
        TBool iIsRemoteDrive;

        // Own: Buffer for storing delete items for MG2 notifications
        CDesCArray* iRemovedItems;

    };
    
#endif      // CFileManagerActiveFind_H
    
// End of File
    
