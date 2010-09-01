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
* Description:  Global query dialog handling
*
*/



#ifndef C_FILEMANAGERLOBALQUERYDLG_H
#define C_FILEMANAGERLOBALQUERYDLG_H


//  INCLUDES
#include <e32base.h>


// FORWARD DECLARATIONS
class CAknGlobalConfirmationQuery;
class MFileManagerGlobalDlgObserver;


// CLASS DECLARATION
/**
 *  This class handles global query dialog
 *
 *  @lib FileManagerView.lib
 *  @since S60 3.1
 */
NONSHARABLE_CLASS(CFileManagerGlobalQueryDlg) : public CActive
    {

public:  // New functions
    /**
     * Two-phased constructor.
     */
	static CFileManagerGlobalQueryDlg* NewL();

    /**
     * Destructor
     */
    ~CFileManagerGlobalQueryDlg();

    /**
     * Sets global query observer
     *
     * @since S60 3.1
     * @param aObserver Pointer to observer
     */
    void SetObserver(
        MFileManagerGlobalDlgObserver* aObserver );

    /**
     * Cancels global query
     *
     * @since S60 3.1
     */
    void CancelDialog();

    /**
     * Shows global query
     *
     * @since S60 3.1
     * @param aText Text to display
     * @param aSkId Softkeys to display
     */
    void ShowDialogL(
        const TDesC& aText,
        const TInt aSkId = 0 );

    /**
     * Shows global query
     *
     * @since S60 3.1
     * @param aTextId TextId to display
     * @param aSkId Softkeys to display
     */
    void ShowDialogL(
        const TInt aTextId,
        const TInt aSkId = 0 );

    /**
     * Shows global query
     *
     * @since S60 3.1
     * @param aText Text to display
     * @param aSkId Softkeys to display
     * @param aBitmapFile Bitmap file to use
     * @param aImageId Bitmap id to use
     * @param aMaskId Bitmap mask id to use
     */
    void ShowDialogL(
        const TDesC& aText,
        const TInt aSkId,
        const TDesC& aBitmapFile,
        const TInt aImageId,
        const TInt aMaskId );

private: // From CActive
    /**
     * @see CActive
     */
    void DoCancel();

    /**
     * @see CActive
     */        
    void RunL();

private:
    /**
     * Constructors
     */
    CFileManagerGlobalQueryDlg();

    void ConstructL();

    /**
     * Notifies global query observer
     * @param aValue Value to notify
     */
    void NotifyObserver( const TInt aValue );

private: // Data   
    /**
     * Pointer to global query dialog
     * Own.
     */ 
    CAknGlobalConfirmationQuery* iQueryDialog;

    /**
     * Pointer to global query observer
     * Not own.
     */ 
    MFileManagerGlobalDlgObserver* iObserver;

    };

#endif  // C_FILEMANAGERLOBALQUERYDLG_H   
            
// End of File
