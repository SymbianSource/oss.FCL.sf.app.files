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
* Description:  Implementation of CDevEncUiEncrViewContainer.
*
*/

// INCLUDE FILES

// Class includes
#include "DevEncUiEncrViewContainer.h"

// System Includes
#include <aknlists.h>
#include <barsread.h>
#include <coemain.h>
#include <DevEncUi.rsg>
#include <eikenv.h>

// User Includes
#include "DevEncLog.h"
#include "DevEnc.hrh"
#include "DevEncUi.pan"
#include "DevEncUiAppui.h"
#include "DevEncUiDocument.h"
#include "DevEncUiEncrView.h"

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::NewLC
//
// --------------------------------------------------------------------------
//
CDevEncUiEncrViewContainer* CDevEncUiEncrViewContainer::NewLC( const TRect& aRect,
                                                               CAknView& aOwningView )
	{
    CDevEncUiEncrViewContainer* self =
        new ( ELeave ) CDevEncUiEncrViewContainer( aOwningView );
    CleanupStack::PushL( self );
    self->ConstructL( aRect );
    return self;
	}

// --------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::NewL
//
// --------------------------------------------------------------------------
//
CDevEncUiEncrViewContainer* CDevEncUiEncrViewContainer::NewL(
                                                      const TRect& aRect,
                                                      CAknView& aOwningView )
	{
    CDevEncUiEncrViewContainer* self = NewLC( aRect, aOwningView );
    CleanupStack::Pop( self );
    return self;
	}


// --------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::ConstructL
//
// --------------------------------------------------------------------------
//
void CDevEncUiEncrViewContainer::ConstructL( const TRect& aRect )
	{
    // Create a window for this application view
    CreateWindowL();

    CreateListL();

    // Set the windows size
    SetRect( aRect );

    // Activate the window, which makes it ready to be drawn
    ActivateL();
	}

// --------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::CreateListL
//
// --------------------------------------------------------------------------
//
void CDevEncUiEncrViewContainer::CreateListL()
    {
    // First phase construction
    iListBox = new ( ELeave ) CAknDoubleStyle2ListBox;
    iListBox->SetContainerWindowL( *this );

    // Second Phase Construction
    TResourceReader reader;
    iEikonEnv->CreateResourceReaderLC( reader, R_DEVENCUI_ENCRYPTION_LISTBOX );
    iListBox->ConstructFromResourceL( reader );
    CleanupStack::PopAndDestroy(); // reader (Can't pass T-classes as parameter)

    iListBox->SetListBoxObserver( this );
    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
        CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
    }

// --------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::CDevEncUiEncrViewContainer
//
// --------------------------------------------------------------------------
//
CDevEncUiEncrViewContainer::CDevEncUiEncrViewContainer(
                                                      CAknView& aOwningView )
 :	iOwningView( aOwningView )
	{
    // No implementation required
	}

// --------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::CDevEncUiEncrViewContainer
//
// --------------------------------------------------------------------------
//
CDevEncUiEncrViewContainer::~CDevEncUiEncrViewContainer()
	{
    delete iListBox;
	}

// --------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::HandleListBoxEventL
//
// --------------------------------------------------------------------------
//
void CDevEncUiEncrViewContainer::HandleListBoxEventL( CEikListBox* /*aListBox*/,
                          TListBoxEvent aEventType )
    {
    switch ( aEventType )
        {
        case EEventItemDoubleClicked: // FALLTHROUGH
        case EEventEnterKeyPressed:
            {
            if ( iListBox->CurrentItemIndex() == 0 )
                {
                iOwningView.HandleCommandL( EDevEncUiCommandEncryptWithoutSavingKey );
                }
            if ( iListBox->CurrentItemIndex() == 1 )
                {
                iOwningView.HandleCommandL( EDevEncUiCommandEncryptAndSaveKey );
                }
            if ( iListBox->CurrentItemIndex() == 2 )
                {
                iOwningView.HandleCommandL( EDevEncUiCommandEncryptWithRestoredKey );
                }
            }
            break;
        default:
            break;
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::CountComponentControls
// Return number of controls inside this container
// --------------------------------------------------------------------------
//
TInt CDevEncUiEncrViewContainer::CountComponentControls() const
    {
    return 1;
    }

// --------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::ComponentControl
//
// --------------------------------------------------------------------------
//
CCoeControl* CDevEncUiEncrViewContainer::ComponentControl( TInt aIndex ) const
    {
    switch ( aIndex )
        {
        case 0:
            return iListBox;
        default:
            return NULL;
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::Draw
//
// --------------------------------------------------------------------------
//
void CDevEncUiEncrViewContainer::Draw( const TRect& aRect ) const
    {
    CWindowGc& gc = SystemGc();
    gc.Clear( aRect );
    }

// --------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::OfferKeyEventL
//
// --------------------------------------------------------------------------
//
TKeyResponse CDevEncUiEncrViewContainer::OfferKeyEventL(
                                                  const TKeyEvent& aKeyEvent,
                                                  TEventCode aType )
    {
    if ( !iListBox )
        {
        return EKeyWasNotConsumed;
        }
    if ( aKeyEvent.iCode == EKeyDevice3 /* OK key */ )
        {
        if ( iListBox->CurrentItemIndex() == 0 )
            {
            iOwningView.HandleCommandL( EDevEncUiCommandEncryptWithoutSavingKey );
            }
        if ( iListBox->CurrentItemIndex() == 1 )
            {
            iOwningView.HandleCommandL( EDevEncUiCommandEncryptAndSaveKey );
            }
        if ( iListBox->CurrentItemIndex() == 2 )
            {
            iOwningView.HandleCommandL( EDevEncUiCommandEncryptWithRestoredKey );
            }
        return EKeyWasConsumed;
        }
    return iListBox->OfferKeyEventL( aKeyEvent, aType );
    }

// --------------------------------------------------------------------------
// CDevEncUiEncrViewContainer::SizeChanged()
//
// --------------------------------------------------------------------------
//
 void CDevEncUiEncrViewContainer::SizeChanged()
    {
    if ( iListBox )
        {
        iListBox->SetExtent( TPoint( 0, 0 ), iListBox->MinimumSize() );
        }
    }

//End Of File


