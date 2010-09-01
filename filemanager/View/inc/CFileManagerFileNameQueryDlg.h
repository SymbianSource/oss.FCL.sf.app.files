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
* Description:  Dialog for asking file name from user
*
*/



#ifndef CFILEMANAGERFILENAMEQUERYDLG_H
#define CFILEMANAGERFILENAMEQUERYDLG_H

//  INCLUDES
#include <AknQueryDialog.h>     // CAknTextQueryDialog

// FORWARD DECLARATIONS
class CFileManagerEngine;

// CLASS DECLARATION
/**
 *  Dialog for querying file names in File Manager
 *
 *  @lib FileManagerView.lib
 *  @since S60 2.0
 */
NONSHARABLE_CLASS(CFileManagerFileNameQueryDlg) : public CAknTextQueryDialog
    {
    public:  // Constructors and destructor

        /**
         * Two-phased constructor.
         * @param aOldName Old name of the file, this will be the default name
         * @param aNewName User entered new name of the file.
         * @param aEngine Reference to File Manager engine.
         * @return Newly created query dialog.
         */
        static CFileManagerFileNameQueryDlg* NewL(
            const TDesC& aOldName,
            TDes& aNewName,
            CFileManagerEngine& aEngine );

        /**
        * Destructor.
        */
        ~CFileManagerFileNameQueryDlg();

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
        CFileManagerFileNameQueryDlg(
            TDes& aNewName, CFileManagerEngine& aEngine );

        /**
         * Symbian OS 2nd phase constructor.
         * @param aOldName Old name of the file, this will be the default name
         */
        void ConstructL( const TDesC& aOldName );

        /**
         * Performs exit checking
         * @param aButtonId Button identifier
         */
        TBool DoOkToExitL( TInt aButtonId );

    private:    // Data
        /// Ref: Reference to File Manager engine
        CFileManagerEngine& iEngine;
        /// Own: Old file name
        HBufC* iOldName;
        /// For blocking unwanted softkey events
        TBool iCallbackDisabled;
    };

#endif      // CFILEMANAGERFILENAMEQUERYDLG_H
            
// End of File
