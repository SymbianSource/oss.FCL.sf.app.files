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
* Description:  Base class for all file manager views
*
*/



#ifndef CFILEMANAGERVIEWBASE_H
#define CFILEMANAGERVIEWBASE_H


//  INCLUDES
#include <aknview.h>
#include <AknServerApp.h>
#include <AknWaitDialog.h> 
#include <apparc.h>
#include <MFileManagerProcessObserver.h>
#include <TFileManagerDriveInfo.h>


// FORWARD DECLARATIONS
class CFileManagerContainerBase;
class CFileManagerEngine;
class CFileManagerActiveExecute;
class CEikMenuBar;
class CFileManagerActiveDelete;
class CFileManagerItemProperties;
class CAknQueryDialog;
class CAknWaitNoteWrapper;
class CFileManagerFeatureManager;
class MAknServerAppExitObserver;


// Defines maximum media password length in Unicode
const TUint KFmgrMaxMediaPassword = KMaxMediaPassword / 2; // Because of Unicode
// CLASS DECLARATION
/**
 * Base class for all views in File Manager applicaiton
 */
class CFileManagerViewBase : public CAknView,
                             public MProgressDialogCallback,
                             public MFileManagerProcessObserver,
                             public MAknServerAppExitObserver
    {
    protected:
        /**
         * Return value enumeration for CmdOpenL to indicate what
         * kind of item was opened or if there was an error during
         * opening.
         */
        enum TFileManagerOpenResult 
            { 
            EFolderOpened, 
            EFileOpened, 
            EOpenError 
            };
    public:  // Constructors and destructor
        /**
         * Two-phased constructor.
         */
        static CFileManagerViewBase* NewLC();
        
        /**
         * Timer calls this in order to update the progress bar.
         * @param aPtr pointer to object where is function to be call
         * when timer expires.
         * @return positive number if timer needs to be activated again
         *         0 if timer is not needed any more.
         */
        static TInt UpdateProgressBar( TAny* aPtr );

        /**
         * Timer calls this in order to start refresh progress dialog.
         * @param aPtr pointer to object where is function to be call
         * when timer expires.
         * @return positive number if timer needs to be activated again
         *         0 if timer is not needed any more.
         */
        static TInt RefreshProgressDelayedStart( TAny* aPtr );

        /** 
        * Stores current index 
        * @return ETrue if storing successfull
        *         EFalse if storing not succesfull
        */
        TBool StoreIndex();

        /**
         * Nulls everything which is related to progress bar
         */
        void ClearProgressBarL();

        /**
         * @return Current process in app
         */
        MFileManagerProcessObserver::TFileManagerProcess CurrentProcess();
        
        /**
         * Destructor.
         */
        virtual ~CFileManagerViewBase();

    public: // From MProgressDialogCallback
        void DialogDismissedL( TInt aButtonId );

    public://MAknServerAppExitObserver
        /**
         * @see MAknServerAppExitObserver
         */
        void HandleServerAppExit( TInt aReason );
        
    public: // From MFileManagerProcessObserver
        /**
         * @see MFileManagerProcessObserver
         */
        void ProcessFinishedL( TInt aError, const TDesC& aName );

        /**
         * @see MFileManagerProcessObserver
         */
        void ProcessAdvanceL( TInt aValue );

        /**
         * @see MFileManagerProcessObserver
         */
        void ProcessStartedL(
            MFileManagerProcessObserver::TFileManagerProcess aProcess,
            TInt aFinalValue );

        /**
         * @see MFileManagerProcessObserver
         */
        TBool ProcessQueryOverWriteL( 
            const TDesC& aOldName, 
            TDes& aNewName, 
            TFileManagerProcess aProcess );

        /**
         * @see MFileManagerProcessObserver
         */
        TBool ProcessQueryRenameL( 
            const TDesC& aOldName, 
            TDes& aNewName, 
            TFileManagerProcess aProcess );

        /**
         * @see MFileManagerProcessObserver
         */
        void RefreshStartedL();

        /**
         * @see MFileManagerProcessObserver
         */
        void RefreshStoppedL();

        /**
         * @see MFileManagerProcessObserver
         */
        void ShowWaitDialogL( MAknBackgroundProcess& aProcess);

        /**
         * @see MFileManagerProcessObserver
         */
        void Error( TInt aError );

        /**
         * @see MFileManagerProcessObserver
         */
        TInt NotifyL( TFileManagerNotify aType, TInt aData, const TDesC& aName );

    public:

        /* 
        * When view needs refresh, this is called.
        */
        virtual void DirectoryChangedL() = 0;

        /*
        * Checks if refresh is in progress
        */
        TBool IsRefreshInProgress();

        /**
         * Handles errors caused by file or folder deletion outside 
         * file manager while file manager is in foreground.
         * @param aError Error code.
         * @return ETrue if error was handled. Otherwise EFalse.
         */
        TBool HandleFileNotFoundL( TInt aError );

        /**
        * Handles screen layout changes
        */
        virtual void ScreenDeviceChanged();

#ifdef RD_FILE_MANAGER_BACKUP
        void StartSchBackupL();

        void SchBackupFinishedL();
#endif // RD_FILE_MANAGER_BACKUP

        /**
         * Updates cba
         */
        virtual void UpdateCbaL();

        /**
         * Notifies foreground status change
         */
        virtual void NotifyForegroundStatusChange( TBool aForeground );

    protected: // From CAknView
        /**
         * @see CAknView
         */
        void DoActivateL(
            const TVwsViewId& aPrevViewId,
            TUid aCustomMessageId,
            const TDesC8& aCustomMessage);

        /**
         * @see CAknView
         */
        void DoDeactivate();

        /**
         * @see CAknView
         */
        void HandleCommandL( TInt aCommand );

        /**
         * @see CAknView
         */
        void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

        /**
		 * @see CAknView
		 */
        void ProcessCommandL( TInt aCommand );

    protected: // New methods

        /**
         * Returns a container class that is derived from 
         * CFileManagerContainerBase. This is provided for derived
         * classes to create their own type of containers.
         * @return Newly created container.
         */
        virtual CFileManagerContainerBase* CreateContainerL() = 0;

        /**
         * Executes open command. Opens currently selected item in
         * list.
         * @return Code that tells if opened item was folder or
         *         file. EOpenError is returned if there was an
         *         error during open.
         */
        TFileManagerOpenResult CmdOpenL();

        /**
         * Executes delete command to delete current item
         * or if there are marked items, all of them are deleted.
         */
        void CmdDeleteL();

        /**
         * Creates new folder to currently active directory.
         */
        void CmdNewFolderL();

        /**
         * Renames currently active item.
         */
        void CmdRenameL();

        /**
         * Moves current item to folder that is queried using
         * Common File Dialogs. If there are marked items, all of
         * them are moved.
         */
        void CmdMoveToFolderL();

        /**
         * Copies current item to folder that is queried using
         * Common File Dialogs. If there are marked items, all of
         * them are copied.
         */
        void CmdCopyToFolderL();

        /**
         * Shows info popup dialog that contains information
         * about currently active item.
         */
        void CmdViewInfoL();

        /**
         * Shows memory status popup dialog that contains
         * information about current memory status of the
         * active memory.
         */
        void CmdMemoryStateL();

        /**
         * Executes find command.
         */
        void CmdFindL();

        /**
         * Receives file via infrared to currently active
         * folder.
         */
        void CmdReceiveViaIRL();

        /**
         * Runs move or copy operation. Parameter aToFolder is the destination
         * of operation.
         * @param aOperation Operation to execute.
         * @param aToFolder Destination folder of the operation.
         */
        void RunOperationL( 
            MFileManagerProcessObserver::TFileManagerProcess aOperation, 
            const TDesC& aToFolder );

        /**
         * Gets reference to drive info.
         */
        TFileManagerDriveInfo& DriveInfo() const;

        /**
         * Refreshes drive info.
         */
        void RefreshDriveInfoL();

        void StartProcessL(
            MFileManagerProcessObserver::TFileManagerProcess aProcess,
            TInt aValue = KErrNotFound );

        void CmdUnlockDriveL();

        void CmdFormatDriveL();

//        void CmdRenameDriveL();
//
//        void CmdSetDrivePasswordL();
//
//        void CmdChangeDrivePasswordL();
//
//        void CmdRemoveDrivePasswordL();
//
//        void CmdMemoryCardDetailsL();

        TInt UpdatePassword(
            TInt aDrive, const TDesC& aOldPwd, const TDesC& aPwd );

        TInt UnlockRemovePasswordL( TInt aDrive, TBool aRemove );

        /**
         * Sets remote drive connection state.
         *
         * @param aState ETrue to connect, EFalse to disconnect.
         */
        void SetRemoteDriveConnectionStateL( TBool aState );

        /**
         * Opens remote drive setting view.
         *
         * @param aDriveName Name of the remote drive.
         */
        void OpenRemoteDriveSettingsL(
            const TDesC& aDriveName = KNullDesC );

        /**
         * Checks if item is on disconnected remote drive.
         *
         * @param aProp Item properties.
         * @return ETrue if item is on disconnect drive. Otherwise EFalse.
         */
        TBool IsDisconnectedRemoteDrive(
            CFileManagerItemProperties& aProp );

        /**
         * Performs remote drive menu filtering.
         *
         * @param aMenuPane Menu instance to be filtered.
         */
        void RemoteDriveCommonFilteringL( CEikMenuPane& aMenuPane );

        /**
         * Renames current drive.
         *
         * @param aForceDefaultName ETrue to offer the default name by force.
         */
        void RenameDriveL( TBool aForceDefaultName );

        /**
         * Handles refresh directory command.
         */
        void CmdRefreshDirectoryL();

        /**
         * Shows eject query.
         */
        void ShowEjectQueryL();

        /**
         * Launches progress dialog with given final and initial values.
         * aOperation parameter is used to determine what resource
         * should be used when displaying dialog.
         * @param aFinalValue Final value of the created progress dialog.
         * @param aInitialValue Initial value of the created progress dialog.
         * @param aOperation Operation that this progress dialog represents
         * @param aImmediatelyVisible Dialog is immediately visible if ETrue
         */
        void LaunchProgressDialogL(
              TInt64 aFinalValue,
              TInt64 aInitialValue,
              MFileManagerProcessObserver::TFileManagerProcess aOperation,
              TBool aImmediatelyVisible = EFalse );

        TBool AskPathL( TDes& aPath, TInt aTextId );

        void SetCbaMskTextL( const TInt aTextId );

        void UpdateCommonCbaL();

        TBool IsDriveAvailable( const TDesC& aPath ) const;

        TBool IsDriveAvailable( const TInt aDrive ) const;

        TBool DriveReadOnlyMmcL( const TInt aDrive ) const;

        TBool DriveReadOnlyMmcL( const TDesC& aFullPath ) const;
        
        TBool CheckPhoneState() const;

        TBool StopProgressDialogAndStoreValues();

        void CmdSortL( TInt aCommand );

        CFileManagerFeatureManager& FeatureManager() const;

        void ShowDiskSpaceErrorL( const TDesC& aFolder );

        /**
         * Sets given media password string to empty and zero fills it.
         *
         * @param aPwd Media password to clean up.
         */
        static void EmptyPwd( TDes& aPwd );

        /**
         * Converts chars to media password.
         *
         * @param aWord Password to convert in Unicode.
         * @param aConverted Converted password.
         */
        static void ConvertCharsToPwd( const TDesC& aWord, TDes8& aConverted );

        /**
         * Gets drive information at current position.
         *
         * @param aInfo Stores drive information.
         * @return Drive identifier or KErrNotFound.
         */
        TInt DriveInfoAtCurrentPosL( TFileManagerDriveInfo& aInfo );

    protected:

        /**
         * C++ default constructor.
         */
        CFileManagerViewBase();

        /**
         * By default Symbian 2nd phase constructor is private.
         */
        virtual void ConstructL( TInt aResId );

    private: // New methods
        /**
         * Constructs an array that contains the indices
         * of marked items.
         * @return Array of marked items.
         */
        CArrayFixFlat<TInt>* MarkedArrayLC();

        /**
         * Gets paths of marked sendable files
         * @param Reference to get size of the sendable files
         * @return Array of sendable file items
         */
        CArrayFixFlat<TInt>* GetSendFilesLC( TInt& aSize );

        /**
         * Filters Edit menu in aMenuPane.
         */
        void MarkMenuFilteringL( CEikMenuPane& aMenuPane );

        /**
         * Creates a send ui query
         */
        void SendUiQueryL();

        /**
         * Timer calls this for updating the progress bar.
         */
        void DoUpdateProgressBar();

        /**
         * Returns true if all or any focused item or marked items are protected.
         *
         * The 'all' or 'any' mode of operation is controlled by the aMode parameter.
         *
         * @param  aMode    controls the 'all' or 'any' mode.  ETrue = 'all'.
         * @return ETrue    all chosen items are protected
         *         EFalse   at least one of the items are not protected
         */
        TBool AreChosenFilesProtectedL( TBool aMode );

        /**
         * Creates and displayes the Context sensitive menu
         */
        void ShowContextSensitiveMenuL();

        /**
         * Adds SendUi option to option menu
         */
        void AddSendOptionL(
            CEikMenuPane& aMenuPane,
            const TInt aCommandIdAfter );

        /**
         * Check is delete condition ok for given item
         */
        TBool DeleteStatusNotOkL( 
            CFileManagerItemProperties& aProp, 
            TInt aSelectionCount ) const;

        /**
         * Deletes focused or marked items from current listbox
         */
        void DeleteItemsL( TInt aIndex );
        
        /**
         * Checks if given index has InfoUrl.
         * @param aIndex given index
         * @return ETrue if given index has InfoUrl,
         *         EFalse otherwise.
         */
        TBool HasInfoUrlL( TInt aIndex );

        /**
         * Opens InfoUrl of given index in browser.
         * @param aIndex given index
         */
        void OpenInfoUrlL( TInt aIndex );

        /**
         * Checks file rights and shows note if expired.
         * @param aFullPath File to check
         * @return ETrue if rights are valid
         *         EFalse if rights are expired
         */
        TBool CheckFileRightsAndInformIfExpiredL( const TDesC& aFullPath );

        /**
         * Filters memory store menu in aMenuPane.
         */
        void MemoryStoreMenuFilteringL( CEikMenuPane& aMenuPane );

        /**
         * Filters organise menu in aMenuPane.
         */        
        void OrganiseMenuFilteringL( CEikMenuPane& aMenuPane );

        /**
         * Filters details menu in aMenuPane.
         */
        void DetailsMenuFilteringL( CEikMenuPane& aMenuPane );
//        
//        /**
//         * Filters memory card menu in aMenuPane.
//         */ 
//        void MemoryCardMenuFilteringL( CEikMenuPane& aMenuPane );
//
//        /**
//         * Filters memory card password menu in aMenuPane.
//         */
//        void MemoryCardPasswordMenuFilteringL( CEikMenuPane& aMenuPane );

        /**
         * Filters context sensitive menu in aMenuPane.
         */
        void ContextSensitiveMenuFilteringL( CEikMenuPane& aMenuPane );

        /**
         * Launches progress bar
         */
        void LaunchProgressBarL(
            TInt aDialogId,
            TInt aTextId,
            TInt64 aFinalValue,
            TInt64 aInitialValue,
            TBool aPeriodic,
            TBool aImmediatelyVisible );

		/**
		 * Starts refresh progress dialog
		 */
        void RefreshProgressDelayedStartL();

        void CheckPostponedDirectoryRefresh();

        void DenyDirectoryRefresh( TBool aDeny );

        void SortMenuFilteringL( CEikMenuPane& aMenuPane );

        void DoProcessFinishedL( TInt aError, const TDesC& aName );

        void DoLaunchProgressDialogAsync();

        static TInt LaunchProgressDialogAsync( TAny* aPtr );

        TInt FilesCountInSearchField();

    protected:    // Data
        /// Own: Container that this view handles.
        CFileManagerContainerBase*  iContainer;
        /// Ref: Reference to File Manager engine.
        CFileManagerEngine&         iEngine;
        /// Own: Progress dialog for iActiveExec
        CAknProgressDialog*         iProgressDialog;
        /// Own: Progress dialog for iActiveExec
        CAknProgressDialog*         iProgressDialogRefresh;
        /// Ref: Progress info of iProgressDialog
        CEikProgressInfo*           iProgressInfo;
        /// Own: Active object that executes copy and move operations
        CFileManagerActiveExecute*  iActiveExec;
        /// Own: Array of marked items in listbox
        CArrayFixFlat<TInt>*        iMarkedArray;
        /// Currently active process
        MFileManagerProcessObserver::TFileManagerProcess iActiveProcess;
        /// Own: Popup menu for showing context sensitive menu
        CEikMenuBar*                iPopupMenu;
        /// Own: This is used to update the progress bar
        CPeriodic*                  iPeriodic;
        // This is for progress bar 
        TInt64                        iTotalTransferredBytes;
        // This is for progress bar(in order to support over 2Gb files)
        // Own: for wait note wrapper, delete items
        CFileManagerActiveDelete*   iActiveDelete;
        // current listbox index
        TInt                        iIndex;
        // Own: Timer object to start refresh progress dialog
        CPeriodic*                  iRefreshProgressDelayedStart;
        // Own: Indicates that scheduled backup is pending
        TBool iSchBackupPending;
        // Own: Eject query dialog
        CAknQueryDialog* iEjectQueryDialog;
        // Own: Indicates if eject has been done
        TBool iEjectDone;
        // Own: Stores current progress value
        TInt64 iProgressCurrentValue;
        // Own: Stores final progress value
        TInt64 iProgressFinalValue;
        // Own: Denies directory refresh during query until selected operation gets started
        TBool iDirectoryRefreshDenied;
        // Own: Indicates postponed directory refresh
        TBool iDirectoryRefreshPostponed;
        // Own: For wait note stepping
        CAknWaitNoteWrapper* iWaitNoteWrapper;
        // Own: Indicates if there's a SendUi popup query dialog
        TBool iSendUiPopupOpened;

    };

#endif      // CFILEMANAGERVIEWBASE_H   
            
// End of File
