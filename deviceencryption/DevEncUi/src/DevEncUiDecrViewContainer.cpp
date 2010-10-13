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
* Description:  Implementation of CDevEncUiDecrViewContainer.
*
*/

// INCLUDE FILES

// Class includes
#include "DevEncUiDecrViewContainer.h"

// System Includes
#include <aknlists.h>
#include <barsread.h>
#include <coemain.h>
#include <devencui.rsg>
#include <eikenv.h>

// User Includes
#include "DevEncLog.h"
#include "DevEnc.hrh"
#include "DevEncUi.pan"
#include "DevEncUiAppui.h"
#include "DevEncUiDocument.h"
#include "DevEncUiDecrView.h"

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::NewLC
//
// -----------------------------------------------------------------------------
//
CDevEncUiDecrViewContainer* CDevEncUiDecrViewContainer::NewLC( const TRect& aRect,
                                                               CAknView& aOwningView )
	{
    CDevEncUiDecrViewContainer* self =
        new ( ELeave ) CDevEncUiDecrViewContainer( aOwningView );
    CleanupStack::PushL( self );
    self->ConstructL( aRect );
    return self;
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::NewL
//
// -----------------------------------------------------------------------------
//
CDevEncUiDecrViewContainer* CDevEncUiDecrViewContainer::NewL( const TRect& aRect,
                                                              CAknView& aOwningView )
	{
    CDevEncUiDecrViewContainer* self = NewLC( aRect, aOwningView );
    CleanupStack::Pop( self );
    return self;
	}


// -----------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::ConstructL
//
// -----------------------------------------------------------------------------
//
void CDevEncUiDecrViewContainer::ConstructL( const TRect& aRect )
	{
    // Create a window for this application view
    CreateWindowL();

    CreateListL();

    // Set the windows size
    SetRect( aRect );

    // Activate the window, which makes it ready to be drawn
    ActivateL();
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::CreateListL
//
// -----------------------------------------------------------------------------
//
void CDevEncUiDecrViewContainer::CreateListL()
    {
    // First phase construction
    iListBox = new ( ELeave ) CAknDoubleStyle2ListBox;
    iListBox->SetContainerWindowL( *this );

    // Second Phase Construction
    TResourceReader reader;
    iEikonEnv->CreateResourceReaderLC( reader, R_DEVENCUI_DECRYPTION_LISTBOX );
    iListBox->ConstructFromResourceL( reader );
    CleanupStack::PopAndDestroy(); // reader (Can't pass T-classes as parameter)

    iListBox->SetListBoxObserver( this );
    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
        CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
    }

// -----------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::CDevEncUiDecrViewContainer
//
// -----------------------------------------------------------------------------
//
CDevEncUiDecrViewContainer::CDevEncUiDecrViewContainer( CAknView& aOwningView )
 :	iOwningView( aOwningView )
    {
    // No implementation required
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::CDevEncUiDecrViewContainer
//
// -----------------------------------------------------------------------------
//
CDevEncUiDecrViewContainer::~CDevEncUiDecrViewContainer()
	{
    delete iListBox;
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::HandleListBoxEventL
//
// -----------------------------------------------------------------------------
//
void CDevEncUiDecrViewContainer::HandleListBoxEventL( CEikListBox* /*aListBox*/,
                          TListBoxEvent aEventType )
    {
    switch ( aEventType )
        {
        case EEventItemDoubleClicked: // FALLTHROUGH
        case EEventEnterKeyPressed:
            {
            if ( iListBox->CurrentItemIndex() == 0 )
                {
                iOwningView.HandleCommandL( EDevEncUiCommandDecrypt );
                }
            if ( iListBox->CurrentItemIndex() == 1 )
                {
                iOwningView.HandleCommandL( EDevEncUiCommandDecryptAndTurnOffEncryption );
                }
            }
            break;
        default:
            break;
        }
    }

// -----------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::CountComponentControls
// Return number of controls inside this container
// -----------------------------------------------------------------------------
//
TInt CDevEncUiDecrViewContainer::CountComponentControls() const
    {
    return 1;
    }

// -----------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::ComponentControl
//
// -----------------------------------------------------------------------------
//
CCoeControl* CDevEncUiDecrViewContainer::ComponentControl( TInt aIndex ) const
    {
    switch ( aIndex )
        {
        case 0:
            return iListBox;
        default:
            return NULL;
        }
    }

// -----------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::Draw
//
// -----------------------------------------------------------------------------
//
void CDevEncUiDecrViewContainer::Draw( const TRect& aRect ) const
    {
    CWindowGc& gc = SystemGc();
    gc.Clear( aRect );
    }

// -----------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::OfferKeyEventL
//
// -----------------------------------------------------------------------------
//
TKeyResponse CDevEncUiDecrViewContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                                         TEventCode aType )
    {
    if ( !iListBox )
        {
        return EKeyWasNotConsumed;
        }
    if ( aKeyEvent.iCode == EKeyDevice3 /* OK key */ )
        {
        if ( iListBox->CurrentItemIndex() < 0 )
            {
            return EKeyWasConsumed;
            }
        // User selected a list item. Notify the callback instance.
        if ( iListBox->CurrentItemIndex() == 0 )
            {
            iOwningView.HandleCommandL( EDevEncUiCommandDecrypt );
            }
        if ( iListBox->CurrentItemIndex() == 1 )
            {
            iOwningView.HandleCommandL( EDevEncUiCommandDecryptAndTurnOffEncryption );
            }
        return EKeyWasConsumed;
        }
    return iListBox->OfferKeyEventL( aKeyEvent, aType );
    }

// --------------------------------------------------------------------------
// CDevEncUiDecrViewContainer::SizeChanged()
//
// --------------------------------------------------------------------------
//
 void CDevEncUiDecrViewContainer::SizeChanged()
    {
    if ( iListBox )
        {
        iListBox->SetExtent( TPoint( 0, 0 ), iListBox->MinimumSize() );
        }
    }

//End Of File


