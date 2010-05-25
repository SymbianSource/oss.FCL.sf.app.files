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
* Description:  File Manager application UI class
*
*/



#ifndef CFILEMANAGERAPPUI_H
#define CFILEMANAGERAPPUI_H

//  INCLUDES
#include <aknViewAppUi.h>       // CAknViewAppUi
#include <TFileManagerDriveInfo.h>
#include <ssm/ssmstateawaresession.h>

#include "MFileManagerSchBackupObserver.h"

// FORWARD DECLARATIONS
class CAknNavigationControlContainer;
class CAknTitlePane;
class CAknNavigationDecorator;
class CSendUi;
class CAiwGenericParamList;
class CFileManagerIRReceiver;
class MFileManagerProcessObserver;
class CFileManagerTaskScheduler;
class CFileManagerSchBackupHandler;


// CLASS DECLARATION
/**
 * FileManager application UI class. 
 * An object of this class is created by the Symbian OS framework by a call to 
 * CFileManagerDocument::CreateAppUiL(). The application UI object creates 
 * and owns the application's views and handles system commands selected 
 * from the menu.
 */
class CFileManagerAppUi : public CAknViewAppUi,
                          public MCoeForegroundObserver,
                          public MFileManagerSchBackupObserver
    {
    public: // Constants and types
        static const TUid KFileManagerMainViewId;
        static const TUid KFileManagerMemoryStoreViewId;
        static const TUid KFileManagerFoldersViewId;
        static const TUid KFileManagerSearchResultsViewId;
        static const TUid KFileManagerBackupViewId;
        static const TUid KFileManagerRestoreViewId;

    public:  // Constructors and destructor
        /**
         * Standard C++ constructor.
         */
        CFileManagerAppUi();

        /**
         * Destructor.
         */
        ~CFileManagerAppUi();

        /**
         * Constructs the CFileManagerAppUi class.
         */
        void ConstructL();

    public:
        /**
         * Activates folders view if it is not already 
         * activated.
         * @since 2.0
         * @return  ETrue if activated
         *          EFalse if view already activated
         */
        TBool ActivateFoldersViewL();

        /**
         * Closes folders view and goes back to view in which the
         * application was before folders view was activated.
         * @since 2.0
         */
        void CloseFoldersViewL();

        /**
         * Activates search results view if it is not already 
         * activated.
         * @since 2.0
         */
        void ActivateSearchResultsViewL();

        /**
         * Closes search results view and goes back to view in which
         * the application was before search results view was 
         * activated
         * @since 2.0
         */
        void CloseSearchResultsViewL();

        /**
         * Restores the default title of application to title pane.
         * @since 2.0
         */
        void RestoreDefaultTitleL();
        /**
         * Returns the CSendUi2 of this application.
         *
         * @since 3.0
         * @return The CSendUi2 of this application.
         */
        CSendUi& SendUiL();
        /**
         * This is called if resource is changed. Needed for skin feature
         *
         * @since 2.0
         */
        void HandleResourceChangeL(TInt aType);

        /**
         * Is application foreground
         *
         * @since 2.0
         * @return ETrue if application is foreground
         *         EFalse if application is not foreground
         */
        TBool IsFmgrForeGround();

        /**
         * Activates main view
         * activated.
         * @since 3.1
         */
        void ActivateMainViewL();

        /**
         * Activates memory store view
         * activated.
         * @since 3.1
         */
        void ActivateMemoryStoreViewL();

        /**
         * Closes memory store view and goes back to main view
         * @since 3.1
         */
        void CloseMemoryStoreViewL();

        /**
         * Gets reference to drive info
         * @since 3.1
         */
        TFileManagerDriveInfo& DriveInfo();

        /**
         * Activates remote drive settings view
         * @since 3.1
         */
        void ActivateRemoteDriveSettingsViewL(
            const TDesC& aDriveName = KNullDesC );

#ifdef RD_FILE_MANAGER_BACKUP
        /**
         * Activates backup view if not already active
         * @since 3.1
         */
         void ActivateBackupViewL();

        /**
         * Closes backup view and goes back to main view
         * @since 3.1
         */
         void CloseBackupViewL();

        /**
         * Activates restore view if not already active
         *
         * @since 3.1
         * @param aDeleteBackup Indicates if delete backup selection is used.
         */
         void ActivateRestoreViewL( TBool aDeleteBackup = EFalse );

        /**
         * Closes restore view and goes back to backup view
         * @since 3.1
         */
         void CloseRestoreViewL();

		 /**
		  * Starts scheduled backup
		  */
         void StartSchBackupL();

		 /**
		  * Gets task scheduler
		  */
         CFileManagerTaskScheduler& TaskSchedulerL();

		 /**
		  * Gets scheduled backup handler
		  */
         CFileManagerSchBackupHandler& SchBackupHandlerL();
#endif // RD_FILE_MANAGER_BACKUP

        /**
         * Activates IR receive
         * @since 3.1
         */
        void StartIRReceiveL( MFileManagerProcessObserver& aObserver );

        /**
         * Stops IR receive
         * @since 3.1
         */
        void StopIRReceive();

        /**
         * Sets title
         * @since 3.1
         */
        void SetTitleL( const TDesC& aTitle );

        /**
         * Sets title
         * @since 3.1
         */
        void SetTitleL( const TInt aTitle );

        /**
         * Checks if view is file manager based view
         * @since 3.1
         */
        TBool IsFileManagerView( const TUid aViewUid );

        /**
        * Informs active view to update its cba
        * @since 3.2
        */
        void NotifyCbaUpdate();

        /**
        * Processes AIW input parameters
        * @since 5.0
        */
        void ProcessAiwParamListL( const CAiwGenericParamList& aInParams );

        /**
        * Indicates are all required input params received
        * @since 5.0
        */
        TBool WaitingForInputParams() const;

        /**
        * Exits the embedded open application if needed
        * @since 5.0
        */
        void ExitEmbeddedAppIfNeededL();

        /**
        * Checks is search view open
        * @since 5.0
        */
        TBool IsSearchViewOpen() const;

        /**
        * Save the time when backup/restore started.
        * @since 5.0
        */
        void BackupOrRestoreStarted();

        /**
        * Save the time when backup/restore ended.
        * @since 5.0
        */
        void BackupOrRestoreEnded();

        /**
        * Returns the time when backup/restore started.
        * @since 5.0
        */
        TTime BackupOrRestoreStartTime() const;

        /**
        * Returns the time when backup/restore ended.
        * @since 5.0
        */
        TTime BackupOrRestoreEndTime() const;

        /**
        * Resets the time when backup/restore start and end times.
        * @since 5.0
        */
        void ResetBackupOrRestoreEndTime();
        
        /**
        * after startup finish, the system state will get normal, backup/restore need check the status before get started.
        * @since 5.2
		* @return ETrue if system state is normal, EFalse if not
        */
        TBool IsSystemStateNormal() const;
        
    public: // MCoeForegroundObserver

        /** Handles the application coming to the foreground. */
        void HandleGainingForeground();
        /** Handles the application going into the background. */
        void HandleLosingForeground();

    public: // From CAknViewAppUi
        /**
         * @see CAknViewAppUi
         */
        void HandleViewDeactivation( const TVwsViewId &aViewIdToBeDeactivated, 
                                     const TVwsViewId &aNewlyActivatedViewId );

        /**
         * @see CAknViewAppUi
         */
        void HandleCommandL( TInt aCommand );

        /**
        * From CAknViewAppUi, called when screen layout changes 
        */
        void HandleScreenDeviceChangedL();

        /**
         * @see CAknViewAppUi
         */
        TErrorHandlerResponse HandleError( TInt aError,
            const SExtendedError& aExtErr, TDes& aErrorText, TDes& aContextText );

        /**
         * @see CAknViewAppUi
         */
         TBool ProcessCommandParametersL(
            TApaCommand aCommand,
            TFileName& aDocumentName,
            const TDesC8& aTail );

#ifdef RD_FILE_MANAGER_BACKUP
        /**
         * @see CAknViewAppUi
         */
        void ProcessMessageL( TUid aUid, const TDesC8& aParams );
#endif // RD_FILE_MANAGER_BACKUP

    public: // From MFileManagerSchBackupObserver
        /**
         * @see MFileManagerSchBackupObserver
         */
        void SchBackupFinishedL( TInt aError );

    private:
        /** Deletes IR receive after timeout. */
        static TInt DeleteIRReceiveCB( TAny* aPtr );

        /** Informs active view when screen device has changed. */
        void NotifyViewScreenDeviceChanged();

        void SetFolderToOpenAtStartup(
            const TDesC& aFullPath, TInt aSortMode );

        void OpenFolderViewAtStartupL();

        void CreateAndActivateLocalViewL(
            TUid aViewId, TUid aCustomMessageId, const TDesC8& aCustomMessage );

        void CreateAndActivateLocalViewL( TUid aViewId );

    private:    // Data
        /// Ref: Title pane of the application
        CAknTitlePane* iTitlePane;
        // Own: CSendAppUi for send menu item and sending files
        CSendUi* iSendUi;
        /// Uid of currently active view
        TUid iActiveView;
        /// Flag to indicate if search view is open
        TBool iSearchViewOpen;
        // For some reason IsForeGround() does not return correct state of the application
        // so own boolean is updated according MCoeForegroundObserver callbacks.
        TBool iForeGround;
        // Own: Infrared receiver for receive via IR functionality
        CFileManagerIRReceiver* iIRReceiver;
        // Own: Callback to delete CFileManagerIrReceiver
        CAsyncCallBack* iIRDeleteCB;
        // Own: Drive info
        TFileManagerDriveInfo iDriveInfo;
		// Task scheduler
        CFileManagerTaskScheduler* iTaskScheduler;
		// Handler for scheduled backup
        CFileManagerSchBackupHandler* iSchBackupHandler;
        // Flag to indicate if started for scheduled backup only
        TBool iSchBackupStandalone;
        // Indicates the folder to be opened at startup
        TFileName iFolderToOpenAtStartup;
        // Indicates the folder level at startup when the app is embedded
        TInt iInitialFolderLevel;
        // Indicates the parameter ready status
        TBool iWaitingForParams;
        
        // Own: Stores the start time
        TTime iManualBackupOrRestoreStarted;
        // Own: Stores the end time
        TTime iManualBackupOrRestoreEnded;
        
        // State aware session.
        RSsmStateAwareSession iSAS;
        
    };

#endif // CFILEMANAGERAPPUI_H
            
// End of File
