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
* Description:  Device Encryption UI settings item list.
*
*/

#ifndef DEVENCUI_SETTINGITEMLIST_H
#define DEVENCUI_SETTINGITEMLIST_H

// INCLUDES

// System includes
#include <aknsettingitemlist.h> // CAknSettingItemList

// User includes
#include "DevEnc.hrh"

// CLASS DECLARATION

class CDevEncUiSettingItemList : public CAknSettingItemList
	{
    public: // Constructors and destructor
    	CDevEncUiSettingItemList();
        ~CDevEncUiSettingItemList();

    // From CAknSettingItemList
        /**
         * Creates the actual setting items for the list, passing
         * ownership of them to the calling class.  Each setting
         * item has a piece of member data which it sets values in.
         */
    	CAknSettingItem* CreateSettingItemL( TInt identifier );
        
        // From CAknSettingItemList
        virtual void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);

        /**
         * Called by the framework whenever an item is selected.
         * Causes the edit page for the currently selected setting item to
         * be displayed and stores any changes made.
         * @param aIndex the index of the item to change
         * @param aCalledFromMenu If ETrue, opens a popup window to let the
         *   user select the state of this item. Otherwise the item state
         *   is just changed.
         */
    	void EditItemL ( TInt aIndex, TBool aCalledFromMenu );

    // Members
        /**
         * Gets the index of the currently selected item.
         * @return the index
         */
        TInt SelectedItem();

        /**
         * Opens a popup window to let the user select the state of this item
         */
    	void ChangeSelectedItemL();

        /**
          * Sets the text of a list item. Does not change the state.
          * @param aIndex the index of the item to change
          * @param aNewText the new text
          */
        void SetItemTextL( TInt aIndex, const TDesC& aNewText );

        /**
         * Sets the state of a list item.
          * @param aIndex the index of the item to change
          * @param aSetting the new state
         */
        void SetItemStateL( TInt aIndex, TBool aSetting );

        /**
         * Gets the state of a list item.
         * @param aIndex the index of the item to change
         * @param aSetting Contains the current setting when finished.
         */
        void ItemState( TInt aIndex, TBool& aSetting );
      
        /**
        * From CAknSettingItemList
        * 
        * Handles a change to the control's resources of type aType
        * which are shared across the environment, e.g. colors or fonts.
        *
        * @since 2.0
        * @param aType  Reason for the "resource" change, usually an system event UID
        */
        void HandleResourceChange( TInt aType );
        
    private:
        // From CAknSettingItemList
        /**
         * Called by framework when the view size is changed. Resizes the
         * setting list accordingly.
         */
        void SizeChanged();

    // Member data
        /* Holds the UI representation of the current state of the phone
         * memory encryption. This does not always correlate to the real memory
         * state, as this variable often contains a new setting that has not
         * been validated yet. */
    	TBool iPhoneMemSetting;

        /* Holds the UI representation of the current state of the memory
         * card encryption. This does not always correlate to the real memory
         * state, as this variable often contains a new setting that has not
         * been validated yet. */
    	TBool iMmcSetting;

        /** Owned. Holds the current state text of the phone memory */
        HBufC* iPhoneMemStateText;

        /** Owned. Holds the current state text of the memory card */
        HBufC* iMmcStateText;
	};

#endif	// #ifndef DEVENCUI_SETTINGITEMLIST_H

// End of File
