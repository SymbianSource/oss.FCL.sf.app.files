/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Handler for scheduled backup
*
*/


#ifndef C_FILEMANAGERSCHBACKUPHANDLER_H
#define C_FILEMANAGERSCHBACKUPHANDLER_H


//  INCLUDES
#include <e32base.h>
#include <MFileManagerGlobalDlgObserver.h>


// FORWARD DECLARATIONS
class CFileManagerEngine;
class CFileManagerGlobalDlg;
class MFileManagerSchBackupObserver;
class CRepository;


// CLASS DECLARATION
/**
 *  This class handles scheduled backup
 *
 *  @since S60 3.1
 */
class CFileManagerSchBackupHandler :
        public CBase,
        public MFileManagerGlobalDlgObserver
    {

public:
    /**
     * Two-phased constructor.
     */
	static CFileManagerSchBackupHandler* NewL(
	    CFileManagerEngine& aEngine );

    /**
	 * Destructor.
     */
    ~CFileManagerSchBackupHandler();

    /**
	 * Starts backup with confirm note.
     */
    void StartBackupWithConfirm();

    /**
	 * Sets backup observer
	 * @param aObserver Pointer to observer or NULL.
	 *                 Ownership is not transferred.
     */
    void SetObserver(
        MFileManagerSchBackupObserver* aObserver );

    /**
	 * Informs process finished
	 * @param aError System wide error code
	 * @param aName Error related descriptor
     */
    void ProcessFinishedL( TInt aError, const TDesC& aName );

    /**
	 * Informs process advance
	 * @param aValue Current progress value
     */
    void ProcessAdvanceL( TInt aValue );

    /**
	 * Informs process started
	 * @param aFinalValue Final progress value
     */
    void ProcessStartedL( TInt aFinalValue );

    /**
	 * Cancels scheduled backup launcher
     */
    void CancelBackupStarter();

private: // From MFileManagerGlobalDialogObserver
    /**
     * @see MFileManagerGlobalDialogObserver
     */
    void HandleGlobalDlgResult(
        TInt aDialogType,
        TInt aDialogResult );

private:
    CFileManagerSchBackupHandler(
        CFileManagerEngine& aEngine );

    void ConstructL();

    void StartBackupWithConfirmL();

    void StartBackupL();

    void StartBackup();

    void InformObserver( const TInt aValue );

    TBool IsPowerSavingModeOnL();

    TBool IsTargetDriveAvailableL( const TFileManagerDriveInfo& aDrvInfo );

private: // Data
    /**
     * Reference to file manager engine
     * Not own.
     */
    CFileManagerEngine& iEngine;

    /**
     * For global dialog handling
     * Own.
     */
    CFileManagerGlobalDlg* iGlobalDlg;

    /**
     * Pointer to scheduled backup observer
     * Not own.
     */
    MFileManagerSchBackupObserver* iObserver;

    /**
     * To indicate is backup ongoing
     */
    TBool iBackupOngoing;

    /**
     * Pointer to CenRep
     * Own.
     */
    CRepository* iCenRep;
    };

#endif  // C_FILEMANAGERSCHBACKUPHANDLER_H   
            
// End of File
