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

#include <aknappui.h>
#include <aknQueryControl.h>
#include <AknQueryDialog.h>
#include <aknsoundsystem.h>
#include <devencui.rsg>
#include <eikseced.h>

#include "DevEncLog.h"
#include "DevEnc.hrh"
#include "DevEncUiSecretMinMaxCodeQuery.h"

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CCodeQueryDialog::CCodeQueryDialog()
// C++ constructor
// ----------------------------------------------------------
//
CDevEncUiSecretMinMaxCodeQuery::CDevEncUiSecretMinMaxCodeQuery(
                                                            TDes& aDataText,
                                                            TInt aMinLength,
                                                            TInt aMaxLength )
    : CAknTextQueryDialog( aDataText, ENoTone ),
      iMinLength( aMinLength ),
      iMaxLength( aMaxLength )
    {
    }
//
// ----------------------------------------------------------
// CDevEncUiSecretMinMaxCodeQuery::~CodeQueryDialog()
// Destructor
// ----------------------------------------------------------
//
CDevEncUiSecretMinMaxCodeQuery::~CDevEncUiSecretMinMaxCodeQuery()
    {
    }
//
// ----------------------------------------------------------
// CDevEncUiSecretMinMaxCodeQuery::PreLayoutDynInitL()
// Called by framework before dialog is shown
// ----------------------------------------------------------
//
void CDevEncUiSecretMinMaxCodeQuery::PreLayoutDynInitL()
    {
    CAknTextQueryDialog::PreLayoutDynInitL();
    SetMaxLength( iMaxLength );
    }
//
// ---------------------------------------------------------
// CDevEncUiSecretMinMaxCodeQuery::OfferKeyEventL
// called by framework when any key is pressed
// ---------------------------------------------------------
//
TKeyResponse CDevEncUiSecretMinMaxCodeQuery::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                                             TEventCode aType )
    {
    // '#' key
    if ( aKeyEvent.iScanCode == EStdKeyHash  && aType == EEventKeyUp )
        {
        TryExitL( EEikBidOk );
        return EKeyWasConsumed;
        }

    // '*' key
    if ( aKeyEvent.iCode == '*' )
        {
        return EKeyWasConsumed;
        }

    // end key
    if ( aKeyEvent.iCode == EKeyPhoneEnd )
        {
        TryExitL( EAknSoftkeyCancel );
        return EKeyWasConsumed;
        }

    return CAknTextQueryDialog::OfferKeyEventL( aKeyEvent,aType );
    }

// ---------------------------------------------------------
// CDevEncUiSecretMinMaxCodeQuery::OkToExitL()
// Called by framework when the softkey is pressed
// ---------------------------------------------------------
TBool CDevEncUiSecretMinMaxCodeQuery::OkToExitL( TInt aButtonId )
    {
    TInt length = 0;
    TBool returnvalue( EFalse );

    switch( aButtonId )
        {
        case EAknSoftkeyOk:
            {
            CAknQueryControl* control = QueryControl();
            if ( control )
                {
                length = control->GetTextLength();
                }
            if ( length < iMinLength )
                {
                returnvalue = EFalse;
                }
            else
                {
                returnvalue = CAknTextQueryDialog::OkToExitL( aButtonId );
                }
            }
        break;

        case EEikBidCancel: //Also includes EAknSoftkeyCancel as they have the same numerical value
            {
            returnvalue = ETrue;
            break;
            }
        default:
            break;
        }
    return returnvalue;
    }

void CDevEncUiSecretMinMaxCodeQuery::UpdateLeftSoftKeyL()
    {
    CAknQueryControl* control = QueryControl();
    if ( control )
        {
        TBool makeVisible( control->GetTextLength() >= iMinLength );
        MakeLeftSoftkeyVisible( makeVisible );
        }
    }

// End of file
