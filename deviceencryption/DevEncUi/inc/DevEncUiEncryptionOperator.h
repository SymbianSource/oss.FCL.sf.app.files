/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Performs required preparations before encryption 
*               operations can place.
*
*/

#ifndef DEVENCUI_ENCRYPTIONOPERATOR_H_
#define DEVENCUI_ENCRYPTIONOPERATOR_H_

//INCLUDES

//System includes
#include <avkon.rsg>
#include <e32base.h>
#include <badesca.h> // for CDesCArrayFlat

//User Includes

// FORWARD DECLARATIONS
class CDevEncUiMemoryEntity;
class CEikonEnv;
class CAknViewAppUi;
class CRepository;

class CDevEncUiEncryptionOperator: public CBase
    {
    public:
    // Constructors and destructor
        static CDevEncUiEncryptionOperator* NewL( CEikonEnv& aEikonEnv,
                                                  CAknViewAppUi& aAppUi,
                                                  CRepository*& aCrSettings );
        static CDevEncUiEncryptionOperator* NewLC( CEikonEnv& aEikonEnv,
                                                   CAknViewAppUi& aAppUi,
                                                   CRepository*& aCrSettings );
        ~CDevEncUiEncryptionOperator();

    // Functions related to encryption
        void HandlePhoneMemEncryptRequestL( CDevEncUiMemoryEntity* aMem, CDevEncUiMemoryEntity* aPrimaryMem = NULL );

        void HandleMmcEncryptRequestL( CDevEncUiMemoryEntity* aMem,
                                       TInt aCommand );

        void HandleMmcDecryptRequestL( CDevEncUiMemoryEntity* aMem,
                                       TInt aCommand );

        void DestroyKeyQueryL( TBool aShowMmcNote );
        
        void BatteryStatusCallback();

        void SuggestMmcEncryptionL( CDevEncUiMemoryEntity* aMem );

        void SuggestMmcDecryptionL( CDevEncUiMemoryEntity* aMem );

        void SuggestMmcImportKeyL( CDevEncUiMemoryEntity* aMem,
                                   TBool aFirstAttempt );
        
        void ShowWrongKeyNoteL();

        /**
         * Displays an error note.
         * @param aResourceId the error text to display
         */
        void ErrorNoteL( TInt aResourceId,
                         TBool aWaitingNote = ETrue );

        /**
         * Displays an information note.
         * @param aResourceId the text to display
         */
        void InfoNoteL( TInt aResourceId,
                        TBool aWaitingNote = EFalse );

    private:
        CDevEncUiEncryptionOperator( CEikonEnv& aEikonEnv,
                                     CAknViewAppUi& aAppUi,
                                     CRepository*& aCrSettings );

        // Second-phase constructor
        void ConstructL();

        TBool PrepareEncryptionL();

        TBool PrepareDecryptionL();

        void SetPowerMenuStatusL( TBool aOpen );

        TBool MessageQueryL( TInt aResourceId,
                             TInt aSoftKeysId = R_AVKON_SOFTKEYS_OK_CANCEL );

        TBool QueryDialogL( TInt aResourceId,
                            TInt aSoftKeysId = R_AVKON_SOFTKEYS_OK_CANCEL );

        TBool DataQueryDialogL( TDes& aResult,
                                TInt aPromptResourceId,
                                TInt aMaxLength );

        /**
         * Displays a query dialog to let the user enter a password.
         * @param aPromptResourceId the prompt text to display
         * @param aPassword On return, contains the password
         * @return ETrue if the user entered a password, EFalse if
         *   the dialog was cancelled.
         */
        TBool PasswordQueryL( TInt aPromptResourceId,
                              TDes8& aPassword );

        /**
         * Checks if there's enough battery power to update.
         * @return ETrue if battery OK or charger connected
         */
        TBool CheckBatteryL();

        /**
         * Prompts the user for a key name and password, then creates
         * a pkcs#5 encrypted key using the common utility component.
         * @return ETrue if the key was created and saved successfully
         */
        TBool CreateAndSaveKeyL();

        /**
         * Lets the user select a key and enter a password, then loads
         * the specified key and takes it in use using the common utility
         * component.
         * @return ETrue if the key was loaded and set successfully
         */
        TBool LoadAndSetKeyL();

        /**
         * Creates a random key and sets it in the security driver.
         * @return ETrue if the key was created and set successfully
         */
        TBool CreateKeyL();

        /**
         * Resets the key in the security driver (fills with null chars).
         * Before destroying the key, a confirmation query is shown to
         * the user.
         */
        void DestroyKeyL();

        void RemountMmcL();
        
        TInt CheckMMCStatus();

        // For testing, remove
        void CheckNotesL();
        void CheckNotesWithHeaderL();


    // data
        /** Not owned */
        CEikonEnv& iEikEnv;

        /** Not owned */
        CAknViewAppUi& iAppUi;
        
        /** Owned */
        CDesCArrayFlat* iListQueryItemArray;
        
        /** Not owned */
        CRepository*& iCrSettings;
    };

#endif /* DEVENCUI_ENCRYPTIONOPERATOR_H_ */

// END OF FILE
