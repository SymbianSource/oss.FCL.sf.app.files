/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  File list container in file manager
*
*/


#ifndef CFILEMANAGERFILELISTCONTAINER_H
#define CFILEMANAGERFILELISTCONTAINER_H

//  INCLUDES
#include "CFileManagerContainerBase.h"

// FORWARD DECLARATIONS
class CFileManagerDocument;
class CFileManagerAppUi;
class CAknSearchField;


// CLASS DECLARATION
/**
 * File list container in File Manager.
 * Replaces old phone memory, memory card, folder and search specific
 * containers.
 */
class CFileManagerFileListContainer : public CFileManagerContainerBase,
                                      public MCoeFocusObserver
    {
    public:
        enum TListType
            {
            EListMain = 0, // Main layout
            EListMemoryStore, // Memory store (Phone memory etc) layout
            EListFolder // Folder layout
            };

        static CFileManagerFileListContainer*
            CFileManagerFileListContainer::NewL(
                const TRect& aRect,
                const TInt aFocusedIndex,
                const TListType aType,
                const TInt aEmptyText,
                const TDesC& aHelpContext );

        /**
         * Destructor.
         */
        ~CFileManagerFileListContainer();

    private: // From CCoeControl
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent,TEventCode aType );

        void HandlePointerEventL( const TPointerEvent &aPointerEvent );

        void SizeChanged();

        TInt CountComponentControls() const;

        CCoeControl* ComponentControl( TInt aIndex ) const;

    public: // From CFileManagerContainerBase
        void SetListEmptyL();

        void RefreshListL( TInt aFocusedIndex );

        CEikTextListBox* CreateListBoxL();

        void UpdateCba();

        TInt ListBoxCurrentItemIndex();

        TInt ListBoxNumberOfItems();

        const CArrayFix< TInt >* ListBoxSelectionIndexes();

        TInt ListBoxSelectionIndexesCount();

        void ListBoxToggleItemL( TInt aIndex );

        TBool ListBoxIsItemSelected( TInt aIndex );

        void ListBoxSetTextL( const TDesC& aText );

        TBool IsSearchFieldVisible() const;

        void EnableSearchFieldL(
            TBool aEnable, const TDesC& aSearchText = KNullDesC );

        void SetCurrentItemIndexAfterSearch( TInt aIndex );

        void ListBoxSelectItemL( TInt aIndex );

        void ListBoxDeselectItem( TInt aIndex );
        
        TInt SearchFieldToListBoxIndex(TInt aIndex );

    private: // From MCoeFocusObserver
        void HandleChangeInFocus();

        void HandleDestructionOfFocusedItem();

    private:
        /**
         * Standard C++ constructor.
         */
        CFileManagerFileListContainer(
            const TListType aType );

        void ConstructL(
            const TRect& aRect,
            const TInt aFocusedIndex,
            const TInt aEmptyText,
            const TDesC& aHelpContext );

        TInt SearchFieldToListIndex( TInt aIndex );

        TInt ListToSearchFieldIndex( TInt aIndex );

        void PageScrollL( TBool aUp );

        TKeyResponse OfferSearchKeyEventL(
            const TKeyEvent& aKeyEvent, TEventCode aType );

    private:
        /**
         * Custom item drawer, to support the "no-folders-marked" feature.
         */
        class CItemDrawer : public CColumnListBoxItemDrawer
            {
            public:
                CItemDrawer(
                    CTextListBoxModel* aTextListBoxModel,
                    const CFont* aFont,
                    CColumnListBoxData* aColumnData,
                    CFileManagerFileListContainer& aContainer );
            public: // From CColumnListBoxItemDrawer
                TListItemProperties Properties( TInt aItemIndex ) const;
            private: // Data
                CFileManagerFileListContainer& iContainer;
            };

        /**
         * Custom listbox, to support the "no-folders-marked" feature.
         */
        class CListBox : public CAknSingleGraphicStyleListBox
            {
            public:
                CListBox( CFileManagerFileListContainer& aContainer );

            protected: // From CAknSingleGraphicStyleListBox
                void CreateItemDrawerL();

            private: // Data
                CFileManagerFileListContainer& iContainer;
            };

    private: // Data
        // Ref: Document of this application
        CFileManagerDocument* iDocument;
        // Ref: AppUi of this application
        CFileManagerAppUi* iAppUi;
        // List type bits
        TUint iType;
        // Indicates if the navigation is supported
        TBool iRightLeftNaviSupported;
        // Indicates if dragging is on
        TBool iDragging;
        // Stores dragging start point
        TPoint iDragStartPoint;
        // Own: Search field
        CAknSearchField* iSearchField;
        // Buffer for search text
        TFileName iSearchText;
        // Indicates if search field is enabled after refresh
        TBool iSearchFieldAfterRefresh;
        // Stores the list index to be restored after search ends
        TInt iIndexAfterSearch;
        // Stores search field enabled status
        TBool iSearchFieldEnabled;
        // Stores fast scroll disbaled status
        TBool iOwnFastScrollDisabled;
    };

#endif      // CFILEMANAGERFILELISTCONTAINER_H
            
// End of File
