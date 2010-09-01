/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Dialog for asking folder name from user
*
*/



#ifndef CFILEMANAGERFOLDERNAMEQUERYDLG_H
#define CFILEMANAGERFOLDERNAMEQUERYDLG_H

//  INCLUDES
#include <AknQueryDialog.h>     // CAknTextQueryDialog

// FORWARD DECLARATIONS
class CFileManagerEngine;

// CLASS DECLARATION
/**
 *  Dialog for querying folder names in File Manager
 *
 *  @lib FileManagerView.lib
 *  @since S60 2.0
 */
NONSHARABLE_CLASS(CFileManagerFolderNameQueryDlg) : public CAknTextQueryDialog
    {
    public:  // Constructors and destructor

        /**
         * Two-phased constructor.
         * @param aDataText Old folder name, this will be the default
         * @param aEngine Reference to File Manager engine.
         * @param aNameGeneration ETrue if name generation will be used,
         *                        EFalse otherwise.
         * @return Newly created query dialog.
         */
        static CFileManagerFolderNameQueryDlg* NewL(
            TDes& aDataText,
            CFileManagerEngine& aEngine,
            TBool aNameGeneration );

        /**
        * Destructor.
        */
        ~CFileManagerFolderNameQueryDlg();

    private: // from CAknTextQueryDialog
        /**
         * @see CAknTextQueryDialog
         */
        TBool OkToExitL( TInt aButtonId );
        
        /**
         * @see CAknTextQueryDialog
         */
        TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType );

    private:
        /**
         * C++ default constructor.
         */
        CFileManagerFolderNameQueryDlg( TDes& aDataText, CFileManagerEngine& aEngine );

        /**
         * Symbian OS 2nd phase constructor.
         * @param aNameGeneration ETrue if name generation will be used,
         *                        EFalse otherwise.
         */
        void ConstructL( TBool aNameGeneration, TDes& aDataText );

        /**
         * Performs exit checking
         * @param aButtonId Button identifier
         */
        TBool DoOkToExitL( TInt aButtonId );

    private:    // Data
        /// Ref: Reference to File Manager engine.
        CFileManagerEngine& iEngine;
        /// Own: Old folder name
        HBufC* iOldName;
        /// For blocking unwanted softkey events
        TBool iCallbackDisabled;
    };

#endif      // CFILEMANAGERFOLDERNAMEQUERYDLG_H
            
// End of File
