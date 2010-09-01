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
* Description:  Defines an interface for handling engine events
*
*/



#ifndef MFILEMANAGERPROCESSOBSERVER_H
#define MFILEMANAGERPROCESSOBSERVER_H

class MAknBackgroundProcess;


// CLASS DECLARATION
/**
*  Defines an interface for handling engine events.
*
*  @since 2.0
*/
class MFileManagerProcessObserver
    {
    public:
        /* Process which is observed*/
        enum TFileManagerProcess
            {
            ENoProcess = 0,
            ECopyProcess,
            EMoveProcess,
            EIRReceiveProcess,
            EDeleteProcess,
            EFormatProcess,
            EBackupProcess,
            ERestoreProcess,
            EEjectProcess,
            ESchBackupProcess,
            EFileOpenProcess
            };

        /** Event which is notified */
        enum TFileManagerNotify
            {
            ENotifyActionSelected = 0, // Action item is selected
            ENotifyDisksChanged, // Disk state changes (disk added, removed etc)
            ENotifyBackupMemoryLow, // Low memory for storing backup data
            ENotifyForcedFormat, // Forced format
            ENotifyFileOpenDenied // File open denied (DRM checks before open etc)
            };

        /**
        * Start the refresh process
        * @since 2.0
        */
        virtual void RefreshStartedL() = 0;

        /**
        * Stops the refresh process
        * @since 2.0
        */
        virtual void RefreshStoppedL() = 0;

        /**
        * Process has been started
        * @since 2.0
        * @param aProcess Started process
        * @param aFinalValue Executed progress final value
        */
        virtual void ProcessStartedL(
            TFileManagerProcess aProcess,
            TInt aFinalValue = 0 ) = 0;

        /**
        * Process finished
        * @since 2.0
        * @param aError Finish status
        * @param aName Finish status related text
        */
        virtual void ProcessFinishedL(
            TInt aError, const TDesC& aName = KNullDesC ) = 0;

        /**
        * During execution of process bytes (file copy) or
        * percentage (Infrared file receiving)
        * is updated via this method
        * @since 2.0
        * @param aValue Execution progress related value
        */
        virtual void ProcessAdvanceL( TInt aValue ) = 0;

        /**
        * Query file overwrite from user
        * @since 2.0
        * @param aOldFileName old name of the item
        * @param aNewFileName new name of the item
        * @param aProcess Move or Copy in progress
        * @return ETrue if user want's to overwrite the name, EFalse if not
        */
        virtual TBool ProcessQueryOverWriteL(
            const TDesC& aOldFileName,
            TDes& aNewFileName,
            TFileManagerProcess aProcess ) = 0;

        /**
        * Query rename item from user
        * @param aOldFileName old name of the item
        * @param aNewFileName new name of the item
        * @param aProcess Move or Copy in progress
        * @since 2.0
        * @return ETrue if user want's to change the name, EFalse if not
        */
        virtual TBool ProcessQueryRenameL(
            const TDesC& aOldFileName,
            TDes& aNewFileName,
            TFileManagerProcess aProcess ) = 0;

        /**
        * Creates and displays wait note
        * @since 2.0
        * @param aProcess reference to object which implements
        *                 the Wait Note Wrapper API
        */
        virtual void ShowWaitDialogL( MAknBackgroundProcess& aProcess ) = 0;

        /**
        * Engine reports that there is an error
        * @param aError error code
        * @since 2.0
        */
        virtual void Error( TInt aError ) = 0;

        /**
        * Handles engine notification
        * @since 3.1
        * @param aType Notification type
        * @param aData Notification related data
        * @param aName Notification related text
        * @return A notification related value
        */
        virtual TInt NotifyL(
            TFileManagerNotify aType,
            TInt aData = 0,
            const TDesC& aName = KNullDesC ) = 0;

    };

#endif // MFILEMANAGERPROCESSOBSERVER_H

// End of File