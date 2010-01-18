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
* Description:  
*
*/

#ifndef DEVENCUISECRETMINMAXCODEQUERY_H_
#define DEVENCUISECRETMINMAXCODEQUERY_H_

#include <AknQueryDialog.h>
#include <aknnotedialog.h>
#include "DevEnc.hrh"

class CDevEncUiSecretMinMaxCodeQuery : public CAknTextQueryDialog
    {
    public://construction and destruction
        /**
        * C++ Constructor.
        * @param aDataText TDes& (code which is entered in query)
        * @param aMinLength TInt (code min length)
        * @param aMaxLength TInt (code max length)
        */
        CDevEncUiSecretMinMaxCodeQuery( TDes& aDataText,
                                        TInt aMinLength,
                                        TInt aMaxLength );
        /**
        * Destructor.
        */
        ~CDevEncUiSecretMinMaxCodeQuery();

    protected://from CAknTextQueryDialog
        /**
        * From CAknTextQueryDialog. This function is called by the UIKON
        * dialog framework just before the dialog is activated, after it has
        * called PreLayoutDynInitL() and the dialog has been sized.
        */
        void PreLayoutDynInitL();

        /**
        * From CAknTextQueryDialog. This function is called by the UIKON
        * framework if the user activates a button in the button panel.
        * It is not called if the Cancel button is activated,
        * unless the EEikDialogFlagNotifyEsc flag is set.
        * @param aButtonId  The ID of the button that was activated
        * @return Should return ETrue if the dialog should exit,
        *   and EFalse if it should not.
        */
        TBool OkToExitL( TInt aButtonId );

        /**
        * From CAknTextQueryDialog. This function is called by the UIKON
        * dialog framework just after a key is pressed
        * @param aKeyEvent TKeyEvent&
        * @param aType TEventCode
        */
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                     TEventCode aType );

        /**
        * From MAknQueryControlObeserver. Overrides the default
        * implementation in CAknQueryDialog. Gets called by framework when
        * text in editor is edited.
        */
        void UpdateLeftSoftKeyL();

    private: // Data
        TInt    iMinLength;
        TInt    iMaxLength;
    };

#endif /*DEVENCUISECRETMINMAXCODEQUERY_H_*/
