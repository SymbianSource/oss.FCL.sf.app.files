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
* Description:  Handles the copy/move operation
*
*/



#ifndef CFILEMANAGERACTIVEEXECUTE_H
#define CFILEMANAGERACTIVEEXECUTE_H

// INCLUDES
#include <e32base.h>
#include <f32file.h>
#include <badesca.h>
#include "FileManagerEngine.hrh"
#include "MFileManagerProcessObserver.h"
#include "MFileManagerThreadFunction.h"

// FORWARD DECLARATIONS
class CFileManagerEngine;
class MFileManagerItemIterator;
//class CMGXFileManager;
class CFileManagerThreadWrapper;

/**
*  CFileManagerActiveExecute
*
*  Handles the copy and move operations.
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerActiveExecute) : public CActive,
                                               public MFileManagerThreadFunction

    {

    public:
        
        /**
        * Overwrite file switch
        */
        enum TFileManagerSwitch
            {
            ENoOverWrite = 1,
            EOverWrite
            };
        
        /**
        * Two-phased constructor.
        * @since 2.0
        * @param aEngine    reference to CFileManagerEngine instance
        * @param aOperation defines type of operation copy or move
        * @param aObserver  reference to MFileManagerProcess implemented
        *                   instance
        * @param aIndexList list of indexes from current view which needs operation
        * @param aToFolder  folder where items are moved or copied
        * @return Newly constructed CFileManagerActiveExecute
        */
        IMPORT_C static CFileManagerActiveExecute* NewL( 
            CFileManagerEngine& aEngine,
            MFileManagerProcessObserver::TFileManagerProcess aOperation,
            MFileManagerProcessObserver& aObserver,
            CArrayFixFlat<TInt>& aIndexList,
            const TDesC& aToFolder );

        /**
        * Destructor
        */
        IMPORT_C ~CFileManagerActiveExecute();
        
        /**
        * Executes one operation at time, one file copy/move or directory creation
        * @since 2.0
        * @param aOverWrite Over write the file or not
        */
        IMPORT_C void ExecuteL( TFileManagerSwitch aOverWrite );
        
        /**
        * Cancel the operation
        * @since 2.0
        */
        IMPORT_C void CancelExecution();

        /**
        * Gets destination folder
        * @since 3.2
        */
        IMPORT_C TPtrC ToFolder();
        
    private:
        
        // From CActive
        void DoCancel();
        void RunL();
        TInt RunError(TInt aError);
        
    private:
        /**
        * C++ default constructor.
        */
        CFileManagerActiveExecute( CFileManagerEngine& aEngine,
            MFileManagerProcessObserver::TFileManagerProcess aOperation,
            MFileManagerProcessObserver& aObserver );
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( CArrayFixFlat<TInt>& aIndexList,
            const TDesC& aToFolder );
        
    private:
        
        /**
        * Item operation was succesfully carried on, move to next item
        */
        void KErrNoneActionL();
        
        /**
        * Item operation was not succesful because there was target item already
        * This function deals that situation
        */
        void KErrAlreadyExistsActionL();
        
        /**
        * Takes last folder name on other string and appends it to other
        * @param aResult Stores the result as full path
        * @param aSrc source full path which last folder name is taken
        * @param aDst target path where source last path is appended
        */
        void AddLastFolder( TDes& aResult, const TDesC& aSrc, const TDesC& aDst );

        /**
        * Performs a copy or move operation
        * 
        * @param aSwitch indicate destination file to be overwritten or not
        * @return system wide error code
        */
        TInt DoOperation( TInt aSwitch );

        /**
        * Checks if directory is empty
        * 
        * @param aDir Full path of the directory
        * @return ETrue if directory is empty, otherwise EFalse
        */
        TBool IsEmptyDir( const TDesC& aDir );

        /**
        * Completes operation
        * 
        * @param aError Operation result
        */
        void CompleteL( TInt aError );

        /**
        * Updates MG2 notifications
        * 
        * @param aForceFlush Indicates if notications are flushed
        * @param aError Indicates the result
        */
        void UpdateNotifications( TBool aFlush, TInt aError );

        static void AppendArrayIfNotFound(
            CDesCArray& aArray, const TDesC& aFullPath );

        void FlushArray( CDesCArray& aArray );

        void ThreadCopyOrMoveStepL();

        void ThreadFinalizeMoveStepL();

    private: // From MFileManagerThreadFunction

        void ThreadStepL();

        TBool IsThreadDone();

        void NotifyThreadClientL( TNotifyType aType, TInt aValue );

    private:
        
        // index to current CDir array
        TInt    iCurrentIndex;
        
        // Own: Source item with full path
        HBufC*  iFullPath;
        
        // Own: Destination item with full path
        HBufC*  iDestination;
        
        // Has user cancelled the operation
        TBool   iCancelled;
        
        // Source directory is empty so special operations are needed
        TBool   iEmptyDir;
        
        // Needed to get the item locations behind given index(es)
        CFileManagerEngine&             iEngine;

        // Ref: Shareable file server session
        RFs& iFs;

        // Move or copy operation
        MFileManagerProcessObserver::TFileManagerProcess    iOperation;
        
        // Own: Array of user selected items to be copied/moved
        CArrayFixFlat< TInt >*            iIndexList;
        
        // Own: user given destination folder
        HBufC*                          iToFolder;
        
        // Own: This is either CFileManagerIndexIterator or CFileManagerFileSystemIterator
        // depending of source item type, file or folder
        MFileManagerItemIterator*       iItemIterator;
        
        // Ref: File operation phases are signaled through this interface
        MFileManagerProcessObserver&    iObserver;

        // Ref: Current source item with full path
        // Needed for file already exist situations.
        HBufC* iSrc;
        
        // Ref: Current destination item with full path
        // Needed for file already exist situations.
        HBufC* iDst;
        
        // Total transferred bytes, used for progress note
        // except in same drive move operation
        TUint iBytesTransferredTotal;
        
        // Total transferred files, used for progress note
        // This is used if move operation is done inside drive
        TInt iFilesTransferredTotal;
        
        // Current error
        TInt iError;

        // Ref: MG2 update notification object
        //CMGXFileManager* iMgxFileManager;

        // Own: Thread wrapper for the operation
        CFileManagerThreadWrapper* iThreadWrapper;

        // Indicates the used operation switches
        TInt iSwitch;

        // Own: Buffer for storing changed items for MG2 notifications
        CDesCArray* iChangedSrcItems;

        // Own: Buffer for storing changed items for MG2 notifications
        CDesCArray* iChangedDstItems;

        // Indicates that the operation is done inside same drive
        TBool iOperationOnSameDrive;

        // Indicates that move operation is finalizing
        TBool iFinalizeMove;

        // Indicates that source of the operation is on remote drive
        TBool iIsSrcRemoteDrive;

        // Indicates that destination of the operation is on remote drive
        TBool iIsDstRemoteDrive;

        // Stores type of the current item
        TFileManagerTypeOfItem iItemType;
    };
    
#endif // CFILEMANAGERACTIVEEXECUTE_H
    
    // End of File
