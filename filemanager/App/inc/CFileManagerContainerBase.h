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
* Description:  Base class for all file manager containers
*
*/


#ifndef CFILEMANAGERCONTAINERBASE_H
#define CFILEMANAGERCONTAINERBASE_H


//  INCLUDES
#include <coecntrl.h>
#include <badesca.h>
#include <eiklbo.h>
#include <eikcmobs.h>


// FORWARD DECLARATIONS
class CEikTextListBox;


// CLASS DECLARATION
/**
 * Base class for all containers in File Manager
 */
class CFileManagerContainerBase : public CCoeControl, 
                                  public MEikListBoxObserver, 
                                  public MCoeControlObserver,
                                  public MEikCommandObserver,
                                  public MListBoxSelectionObserver
    {
    public:  // destructor
        /**
         * Destructor.
         */
        ~CFileManagerContainerBase();

    protected: // MEikListBoxObserver
        void HandleListBoxEventL( CEikListBox* aListBox, TListBoxEvent aEventType );

    protected: // MCoeControlObserver
        void HandleControlEventL( CCoeControl* aControl, TCoeEvent aEventType );

    protected: // From CCoeControl
        /**
         * @see CCoeControl
         */
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
         * @see CCoeControl
         */
        void SizeChanged();

        /**
         * @see CCoeControl
         */
        TInt CountComponentControls() const;

        /**
         * @see CCoeControl
         */
        CCoeControl* ComponentControl( TInt aIndex ) const;

        /**
         * @see CCoeControl
         */
        void FocusChanged( TDrawNow aDrawNow );

		/**
		 * @see CCoeControl
		 */
        void GetHelpContext( TCoeHelpContext& aContext ) const;

    public: // From MEikCommandObserver
		/**
		 * @see MEikCommandObserver
		 */
        void ProcessCommandL( TInt aCommandId );

    public: // New functions
        /**
         * Sets list empty and removes empty text until refreshed.
         */
        virtual void SetListEmptyL();

        /**
         * Stores the focus, so when control is reseted, focus can
         * be set back to current position. 
         *
         * @param aFocusedIndex index to be stored
         */
        void SetIndex( TInt aFocusedIndex );

        /**
         * Sets text array.
         */
        void SetTextArray( MDesCArray* aArray );

        /**
         * Sets help context.
         */
        void SetHelpContext( const TDesC& aHelpContext );

        /**
         * Sets empty text.
         */
        void SetEmptyTextL( TInt aTextId );

        /**
         * Refreshes the container.
         * @param aFocusedIndex Index of the item that will be focused
         *                      after refresh.
         */
        virtual void RefreshListL( TInt aFocusedIndex );

        /**
         * Gets selection mode status
         */
        TBool SelectionModeEnabled() const;

        /**
         * Updates cba
         */
        virtual void UpdateCba();

        /**
         * Gets index of current list item
         * @return Index of current list item or KErrNotFound if list is empty.
         */
        virtual TInt ListBoxCurrentItemIndex();

        /**
         * Gets number of list items
         * @return Number of list items.
         */
        virtual TInt ListBoxNumberOfItems();

        /**
         * Gets indexes of selection
         * @return Array of selected indexes. Ownership is not transferred.
         */
        virtual const CArrayFix< TInt >* ListBoxSelectionIndexes();

        /**
         * Gets number of selected list items
         * @return Number of selected list items.
         */
        virtual TInt ListBoxSelectionIndexesCount();

        /**
         * Toggles list item selection status
         * @param aIndex Index of list item.
         */
        virtual void ListBoxToggleItemL( TInt aIndex );

        /**
         * Checks list item selection status
         * @param aIndex Index of list item.
         * @return ETrue if item is selected. Otherwise EFalse.
         */
        virtual TBool ListBoxIsItemSelected( TInt aIndex );

        /**
         * Selects all list items
         */
        virtual void ListBoxSelectAllL();

        /**
         * Clears list selection
         */
        virtual void ListBoxClearSelection();

        /**
         * Sets text showm in empty list view.
         * @param aText Text to be shown
         */
        virtual void ListBoxSetTextL( const TDesC& aText );

        /**
         * Checks is search field visible
         * @return ETrue if search field is visible. Otherwise EFalse.
         */
        virtual TBool IsSearchFieldVisible() const;

        /**
         * Enables and disables the search field.
         * @param aEnable New status of search field.
         * @param aSearchText Text to search
         */
        virtual void EnableSearchFieldL(
            TBool aEnable, const TDesC& aSearchText = KNullDesC );

        /**
         * Sets current item index after search
         * @param aIndex Current item index.
         */
        virtual void SetCurrentItemIndexAfterSearch( TInt aIndex );

        /**
         * Selects list item
         * @param aIndex Index of list item.
         */
        virtual void ListBoxSelectItemL( TInt aIndex );

        /**
         * Deselects list item
         * @param aIndex Index of list item.
         */
        virtual void ListBoxDeselectItem( TInt aIndex );
        
        /**
         * 
         * 
         */
        virtual void ListBoxSetSelectionIndexesL(const CArrayFixFlat<TInt>* 
                                                          aSelectionIndexes);
        /**
         * search filed index to listbox index
         * @param aIndex index in search filed
         * @return index in listbox
         */
        virtual TInt SearchFieldToListBoxIndex( TInt aIndex );
    protected:
        /**
         * Standard C++ constructor.
         */
        CFileManagerContainerBase();

        virtual void ConstructL(
            const TRect& aRect,
            const TInt aFocusedIndex );

        virtual CEikTextListBox* CreateListBoxL() = 0;

        TBool ListBoxExists() const;

        CEikListBox& ListBox();

    private: // From MListBoxSelectionObserver
        void SelectionModeChanged(
            CEikListBox* aListBox, TBool aSelectionModeEnabled );

    private:
        void SetEmptyArrayL();

    private: // Data
        // Own: Pointer to listbox
        CEikTextListBox* iListBox;
        // Own: Help context name
        TCoeContextName iHelpContext;
        // Own: Empty text
        HBufC* iEmptyText;
        // Ref: Pointer to text array
        MDesCArray* iArray;
        // Own: Stores selection mode status
        TBool iSelectionModeEnabled;

    };

#endif      // CFILEMANAGERCONTAINERBASE_H
            
// End of File
