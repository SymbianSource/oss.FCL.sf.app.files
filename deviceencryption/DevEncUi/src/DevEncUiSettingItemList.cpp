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
* Description:  Implementation of CDevEncUiSettingItemList.
*
*/

// INCLUDE FILES

// Class include
#include "DevEncUiSettingItemList.h"

// System include
#include <StringLoader.h>
#include <devencui.rsg>

// User include

// Constants
const TInt KOffTextIndex( 0 );
const TInt KOnTextIndex( 1 );
const TInt KOffEnumerationValue( 0 );
const TInt KOnEnumerationValue( 1 );

// ================= MEMBER FUNCTIONS =======================

// --------------------------------------------------------------------------
// CDevEncUiSettingItemList::CDevEncUiSettingItemList()
// --------------------------------------------------------------------------
//
CDevEncUiSettingItemList::CDevEncUiSettingItemList()
	{
	}

// --------------------------------------------------------------------------
// CDevEncUiSettingItemList::~CDevEncUiSettingItemList()
// --------------------------------------------------------------------------
//
CDevEncUiSettingItemList::~CDevEncUiSettingItemList()
    {
    }

// --------------------------------------------------------------------------
// CDevEncUiSettingItemList::SizeChanged()
// --------------------------------------------------------------------------
//
void CDevEncUiSettingItemList::SizeChanged()
    {
    if ( ListBox() )
        {
        ListBox()->SetRect( Rect() );
        ListBox()->HandleResourceChange( KEikDynamicLayoutVariantSwitch );
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiSettingItemList::HandleResourceChange()
// --------------------------------------------------------------------------
//
void CDevEncUiSettingItemList::HandleResourceChange( TInt aType )
    {
    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        // do re-layout
        if ( ListBox() )
            {
            ListBox()->SetRect( Rect() );
            ListBox()->HandleResourceChange( aType );
            }
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiSettingItemList::CreateSettingItemL()
// --------------------------------------------------------------------------
//
CAknSettingItem* CDevEncUiSettingItemList::CreateSettingItemL( TInt aIdentifier )
	{
	CAknSettingItem* settingItem = NULL;

	switch ( aIdentifier )
		{
		case EPhoneEncryptionSettingItem:
			settingItem =
                new ( ELeave ) CAknBinaryPopupSettingItem( aIdentifier,
                                                           iPhoneMemSetting );
			break;
        case EMemoryCardEncryptionSettingItem:
            settingItem =
                new ( ELeave ) CAknBinaryPopupSettingItem( aIdentifier,
                                                           iMmcSetting );
            break;
		}
	return settingItem; // passing ownership
	}

// --------------------------------------------------------------------------
// CDevEncUiSettingItemList::HandleListBoxEventL()
// --------------------------------------------------------------------------
//
void CDevEncUiSettingItemList::HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
	{
    if ( aEventType == EEventItemDoubleClicked )
        {
        TKeyEvent event;
        event.iCode = EKeyEnter;
        event.iScanCode = EStdKeyEnter;
        event.iRepeats = 0;
        iCoeEnv->SimulateKeyEventL( event, EEventKey );
        return;
        }
    CAknSettingItemList::HandleListBoxEventL(aListBox, aEventType);
	}

// --------------------------------------------------------------------------
// CDevEncUiSettingItemList::ChangeSelectedItemL()
// --------------------------------------------------------------------------
//
void CDevEncUiSettingItemList::ChangeSelectedItemL()
	{
    if ( ! ListBox() )
        {
        User::Leave( KErrNotFound );
        }
	EditItemL( ListBox()->CurrentItemIndex(), ETrue );
	}

// --------------------------------------------------------------------------
// CDevEncUiSettingItemList::SelectedItem()
// --------------------------------------------------------------------------
//
TInt CDevEncUiSettingItemList::SelectedItem()
    {
     if ( ! ListBox() )
         {
         return -1;
         }
    return ListBox()->CurrentItemIndex();
    }


 // --------------------------------------------------------------------------
 // CDevEncUiSettingItemList::EditItemL()
 // --------------------------------------------------------------------------
 //
void CDevEncUiSettingItemList::EditItemL( TInt aIndex, TBool aCalledFromMenu )
	{
	CAknSettingItemList::EditItemL( aIndex, aCalledFromMenu );
	(*SettingItemArray())[aIndex]->StoreL();
	}

// --------------------------------------------------------------------------
// CDevEncUiSettingItemList::SetItemTextL()
// --------------------------------------------------------------------------
//
void CDevEncUiSettingItemList::SetItemTextL( TInt aIndex, const TDesC& aNewText )
    {
    // Sanity check
    if ( aIndex > ( SettingItemArray()->Count() - 1 ) )
        {
        User::Leave( KErrOverflow );
        }
    // Get hold of the existing texts for the selected item
    // The binary popup setting item has two texts, one for each setting.
    // Just to be sure, we set them both.
    CAknBinaryPopupSettingItem* settingItem =
        reinterpret_cast<CAknBinaryPopupSettingItem*>
            ( ( *SettingItemArray() )[aIndex] );
    CArrayPtr<CAknEnumeratedText>* textArray =
        settingItem->EnumeratedTextArray();
    CAknEnumeratedText* enumTextOn = textArray->At( KOnTextIndex );
    CAknEnumeratedText* enumTextOff = textArray->At( KOffTextIndex );

    // Delete the previous texts thoroughly
    HBufC* prevTextOn = enumTextOn->Value();
    HBufC* prevTextOff = enumTextOff->Value();
    textArray->Delete( KOnTextIndex );
    textArray->Delete( KOffTextIndex );
    delete prevTextOn;
    delete prevTextOff;

    // Set the new enumeration values (matching the ones in the rss file)
    enumTextOn->SetEnumerationValue( KOnEnumerationValue );
    enumTextOff->SetEnumerationValue( KOffEnumerationValue );

    // Set the new texts
    enumTextOn->SetValue( aNewText.AllocL() ); // Ownership transferred to array
    enumTextOff->SetValue( aNewText.AllocL() ); // Ownership transferred to array
    textArray->AppendL( enumTextOn );
    textArray->AppendL( enumTextOff );

    // The list updates itself
    HandleChangeInItemArrayOrVisibilityL();
    }

// --------------------------------------------------------------------------
// CDevEncUiSettingItemList::SetItemStateL()
// --------------------------------------------------------------------------
//
void CDevEncUiSettingItemList::SetItemStateL( TInt aIndex, TBool aSetting )
    {
    if ( aIndex == EPhoneMemory )
        {
        iPhoneMemSetting = aSetting;
        }
    else
        {
        iMmcSetting = aSetting;
        }
    // Load the new value to the framework's internal data
    ( *SettingItemArray() )[ aIndex ]->LoadL();
    }

// --------------------------------------------------------------------------
// CDevEncUiSettingItemList::ItemState()
// --------------------------------------------------------------------------
//
void CDevEncUiSettingItemList::ItemState( TInt aIndex, TBool& aSetting )
   {
   // Fetch the current value from the framework's internal data
   if ( aIndex == EPhoneMemory )
       {
       aSetting = iPhoneMemSetting;
       }
   else
       {
       aSetting = iMmcSetting;
       }
   }

// END OF FILE
