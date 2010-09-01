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
* Description:  Global dialog handling
*
*/


#ifndef C_FILEMANAGERLOBALDLG_H
#define C_FILEMANAGERLOBALDLG_H


//  INCLUDES
#include <e32base.h>
#include "MFileManagerGlobalDlgObserver.h"


// FORWARD DECLARATIONS
class CFileManagerGlobalQueryDlg;
class CAknGlobalProgressDialog;
class CAknGlobalNote;


// CLASS DECLARATION
/**
 *  This class handles global dialog wrapping
 *
 *  @lib FileManagerView.lib
 *  @since S60 3.1
 */
class CFileManagerGlobalDlg : public CActive,
                              public MFileManagerGlobalDlgObserver
    {

public:  // New functions
    // Global dialog types
    enum TType
        {
        ECountdownQuery = 0,
        EQuery,
        EProgressDialog,
        EErrorNote,
        EInfoNote,
        EQueryWithWarningIcon,
        EQueryWithInfoIcon
        };

    /**
     * Two-phased constructor.
     */
	IMPORT_C static CFileManagerGlobalDlg* NewL();

    /**
     * Destructor
     */
    IMPORT_C ~CFileManagerGlobalDlg();

    /**
     * Sets global dialog observer
     *
     * @since S60 3.1
     * @param aObserver Pointer to observer
     */
    IMPORT_C void SetObserver(
        MFileManagerGlobalDlgObserver* aObserver );

    /**
     * Cancels global dialog
     *
     * @since S60 3.1
     */
    IMPORT_C void CancelDialog();

    /**
     * Shows global dialog
     *
     * @since S60 3.1
     * @param aType Global dialog type
     * @param aText Text to display
     * @param aSkId Softkeys to display
     */
    IMPORT_C void ShowDialogL(
        const CFileManagerGlobalDlg::TType aType,
        const TDesC& aText,
        const TInt aSkId = 0 );

    /**
     * Shows global dialog
     *
     * @since S60 3.1
     * @param aType Global dialog type
     * @param aTextId TextId to display
     * @param aSkId Softkeys to display
     */
    IMPORT_C void ShowDialogL(
        const CFileManagerGlobalDlg::TType aType,
        const TInt aTextId,
        const TInt aSkId = 0 );

    /**
     * Updates global progress dialog
     *
     * @since S60 3.1
     * @param aValue Current progress value
     * @param aFinalValue Final progress value
     */
    IMPORT_C void UpdateProgressDialog(
        const TInt aValue,
        const TInt aFinalValue = - 1);

    /**
     * Finishes global progress dialog
     *
     * @since S60 3.1
     */
    IMPORT_C void ProcessFinished();

private: // From CActive
    /**
     * @see CActive
     */
    void DoCancel();

    /**
     * @see CActive
     */        
    void RunL();

private: // From MFileManagerGlobalDlgObserver
    void HandleGlobalDlgResult(
        TInt aDlgType,
        TInt aDlgResult );

private:
    /**
     * Constructors
     */
    CFileManagerGlobalDlg();

    void ConstructL();

    /**
     * Countdown timer callback
     * @param aPtr Pointer to this class instance
     */
    static TInt CountdownCB( TAny* aPtr );

    /**
     * Countdown timer callback
     */
    void CountdownL();

    /**
     * Starts countdown
     * @param aTimeout Timeout value
     */
    void StartCountdownL( const TUint aTimeout );

    /**
     * Notifies global dialog observer
     * @param aValue Value to notify
     */
    void NotifyObserver( const TInt aValue );

private: // Data
    /**
     * Pointer to global query dialog
     * Own.
     */
    CFileManagerGlobalQueryDlg* iQueryDialog;

    /**
     * Pointer to global progress dialog
     * Own.
     */
    CAknGlobalProgressDialog* iProgressDialog;

    /**
     * Pointer to countdown step timer
     * Own.
     */
    CPeriodic* iCountdown;

    /**
     * Steps to countdown
     */
    TInt iCountdownSteps;

    /**
     * Pointer to global dialog observer
     * Not own.
     */
    MFileManagerGlobalDlgObserver* iObserver;

    /**
     * Global dialog type
     */
    TType iType;

    /**
     * Global progress dialog final value
     */
    TInt iFinalValue;

    /**
     * Pointer to countdown text
     * Own.
     */
    HBufC* iCountdownText;
    };

#endif // C_FILEMANAGERLOBALDLG_H   
            
// End of File
