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



// INCLUDE FILES
#include <FileManager.rsg>
#include <aknlists.h>
#include <barsread.h>
#include <eikclbd.h>
#include <gulicon.h>
#include <aknsfld.h>
#include <CFileManagerEngine.h>
#include <CFileManagerFeatureManager.h>
#include <FileManagerPrivateCRKeys.h>
#include "CFileManagerFileListContainer.h"
#include "CFileManagerDocument.h"
#include "CFileManagerAppUi.h"
#include "FileManager.hrh"
#include "CFileManagerIconArray.h"
#include <aknview.h>
#include <eikmenub.h>
#include <eikmenup.h> // CEikMenuPane
// CONSTANTS
const TInt KTouchGestureThreshold = 30; // Threshold could be stored in CenRep


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::CFileManagerFileListContainer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerFileListContainer::CFileManagerFileListContainer(
        const TListType aType ) :
    iType( aType )
    {
    iAppUi = static_cast< CFileManagerAppUi* >( ControlEnv()->AppUi() );
    iDocument = static_cast< CFileManagerDocument* >( iAppUi->Document() );
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::NewL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFileListContainer* CFileManagerFileListContainer::NewL(
        const TRect& aRect,
        const TInt aFocusedIndex,
        const TListType aType,
        const TInt aEmptyText,
        const TDesC& aHelpContext )
    {
    CFileManagerFileListContainer* self =
        new ( ELeave ) CFileManagerFileListContainer( aType );
    CleanupStack::PushL( self );
    self->ConstructL(
        aRect,
        aFocusedIndex,
        aEmptyText,
        aHelpContext );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::ConstructL(
        const TRect& aRect,
        const TInt aFocusedIndex,
        const TInt aEmptyText,
        const TDesC& aHelpContext )
    {
    CFileManagerFeatureManager& featureManager(
        iDocument->Engine().FeatureManager() );
    iRightLeftNaviSupported = featureManager.IsFeatureSupported(
        EFileManagerFeatureRightLeftNaviSupported );
    iOwnFastScrollDisabled = featureManager.IsFeatureSupported(
        EFileManagerFeatureOwnFastScrollDisabled );
    CFileManagerContainerBase::ConstructL( aRect, aFocusedIndex );
    SetEmptyTextL( aEmptyText );
    SetHelpContext( aHelpContext );
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::CreateListBoxL
// 
// -----------------------------------------------------------------------------
// 
CEikTextListBox* CFileManagerFileListContainer::CreateListBoxL()
    {
    switch ( iType )
        {
        case EListMain:
            {
            CAknDoubleLargeStyleListBox* listBox =
                new( ELeave ) CAknDoubleLargeStyleListBox();
            CleanupStack::PushL( listBox );
            listBox->SetContainerWindowL( *this );
            listBox->ConstructL( this, EAknGenericListBoxFlags |
                EAknListBoxMenuList | EAknListBoxLoopScrolling );
            CFormattedCellListBoxData* column = listBox->ItemDrawer()->ColumnData();
            column->SetIconArray( iDocument->IconArray() );
            column->SetSkinEnabledL( ETrue );
            column->EnableMarqueeL( ETrue );
            CleanupStack::Pop( listBox );
            return listBox;
            }
        case EListMemoryStore: // FALLTHROUGH
        case EListFolder:
            {
            CAknColumnListBox* listBox = new( ELeave )
                        CFileManagerFileListContainer::CListBox( *this );
            CleanupStack::PushL( listBox );
            listBox->SetContainerWindowL( *this );
            listBox->ConstructL( this, EAknGenericListBoxFlags |
                EAknListBoxMenuList | EAknListBoxLoopScrolling |
                EAknListBoxStylusMarkableList );
            CColumnListBoxData* column = listBox->ItemDrawer()->ColumnData();
            column->SetIconArray( iDocument->IconArray() );
            column->SetSkinEnabledL( ETrue );
            // Disable lists MSK observer because the container will be used
            // as observer
            listBox->EnableMSKObserver( EFalse );
            column->EnableMarqueeL( ETrue );
            // Create search field popup
            iSearchField = CAknSearchField::NewL(
                *this, CAknSearchField::EPopup, NULL, KMaxFileName );
            iSearchField->MakeVisible( EFalse );
            iSearchField->SetSkinEnabledL( ETrue );
            CCoeEnv::Static()->AddFocusObserverL( *this );
            CleanupStack::Pop( listBox );
            return listBox;
            }
        default:
            {
            User::Leave( KErrGeneral );
            break;
            }
        }

    return NULL;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::~CFileManagerFileListContainer
//
// -----------------------------------------------------------------------------
// 
CFileManagerFileListContainer::~CFileManagerFileListContainer()
    {
    CCoeEnv::Static()->RemoveFocusObserver( *this );

    if ( ListBoxExists() )
        {
        // before we destroy listbox in base class,
        // we have to set icon array to NULL
        // because we want to let document class own the icon array
        if ( iType == EListMain )
            {
            CAknDoubleLargeStyleListBox& listBox =
                static_cast< CAknDoubleLargeStyleListBox& >( ListBox() );
            listBox.ItemDrawer()->ColumnData()->SetIconArray( NULL );
            }
        else
            {
            CAknColumnListBox& listBox =
                static_cast< CAknColumnListBox& >( ListBox() );
            listBox.ItemDrawer()->ColumnData()->SetIconArray( NULL );
            }
        }
    delete iSearchField;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::OfferKeyEventL
// 
// -----------------------------------------------------------------------------
// 
TKeyResponse CFileManagerFileListContainer::OfferKeyEventL( 
        const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TKeyResponse response = OfferSearchKeyEventL( aKeyEvent, aType );
    if ( response == EKeyWasConsumed )
       {
       return response;
       }   
    
    TVwsViewId enabledViewId = TVwsViewId( KNullUid, KNullUid );
    iAppUi->GetActiveViewId( enabledViewId );
             
    TUid enabledViewUid = enabledViewId.iViewUid;
    CAknView* enabledView = iAppUi->View( enabledViewUid );
    CEikMenuBar* menuBar = NULL;
    
    if ( iAppUi->IsFileManagerView( enabledViewUid ) )
        {
        menuBar = enabledView->MenuBar();
        }
    switch( aKeyEvent.iCode )
        {
        case EKeyEnter: // FALLTHROUH
        case EKeyOK:
            {
            if ( ( menuBar != NULL ) && menuBar->ItemSpecificCommandsEnabled() )
                {
                iAppUi->ProcessCommandL( EFileManagerSelectionKey );
                response = EKeyWasConsumed;
                }
            break;
            }
        case EKeyDelete:    // FALLTHROUGH
        case EKeyBackspace:
            {
            if ( ( menuBar != NULL ) && menuBar->ItemSpecificCommandsEnabled() )
                {
                iAppUi->ProcessCommandL( EFileManagerDelete );
                response = EKeyWasConsumed;
                }
            break;
            }
        case EKeyLeftArrow: // FALLTHROUGH
        case EKeyRightArrow:
            {
            if ( iRightLeftNaviSupported )
                {
                // Depth navigation using arrows
                TInt commandId( EAknSoftkeyBack );
                if ( aKeyEvent.iCode == EKeyRightArrow )
                    {
                    commandId = EFileManagerOpen;
                    }
                iAppUi->ProcessCommandL( commandId );
                response = EKeyWasConsumed;
                }
            break;
            }
        case EKeyYes:
            {
            if ( AknLayoutUtils::PenEnabled() )
                {
                // Touch uses just the default functionality
                response = ListBox().OfferKeyEventL( aKeyEvent, aType );
                }
            else
                {
                iAppUi->ProcessCommandL( EFileManagerSend );
                response = EKeyWasConsumed;
                }
            break;
            }
        default:
            {
            response = ListBox().OfferKeyEventL( aKeyEvent, aType );
            break;
            }
        }
    return response;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::SetListEmptyL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::SetListEmptyL()
    {
    if ( IsSearchFieldVisible() )
        {
        iSearchField->GetSearchText( iSearchText );
        iSearchFieldAfterRefresh = ETrue;
        EnableSearchFieldL( EFalse );
        }
    CFileManagerContainerBase::SetListEmptyL();
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::RefreshListL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::RefreshListL( TInt aFocusedIndex )
    {
    iDocument->ClearStringCache();
    SetTextArray( iDocument->FileList() );
    CFileManagerContainerBase::RefreshListL( aFocusedIndex );
    if ( iSearchFieldAfterRefresh )
        {
        iSearchFieldAfterRefresh = EFalse;
        EnableSearchFieldL( ETrue, iSearchText );
        }
    // Inform user about OOM, suppress other errors
    TInt err( iDocument->LastError() );
    if ( err == KErrNoMemory )
        {
        ControlEnv()->HandleError( KErrNoMemory );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::HandleControlEventL
// From MCoeControlObserver, called by current listbox
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::HandleControlEventL(
        CCoeControl* /* aControl*/, TCoeEvent aEventType )
    {
    if ( aEventType == EEventStateChanged )
        {
        iAppUi->ProcessCommandL( EFileManagerCheckMark ); // Inform change
        }
    }

//-----------------------------------------------------------------------------
// CFileManagerFileListContainer::CItemDrawer::CItemDrawer
//-----------------------------------------------------------------------------
//
CFileManagerFileListContainer::CItemDrawer::CItemDrawer(
    CTextListBoxModel* aTextListBoxModel,
    const CFont* aFont,
    CColumnListBoxData* aColumnData,
    CFileManagerFileListContainer& aContainer ) :
        CColumnListBoxItemDrawer(
            aTextListBoxModel, aFont, aColumnData ),
        iContainer( aContainer )
    {
    }

//-----------------------------------------------------------------------------
// CFileManagerFileListContainer::CItemDrawer::Properties
//-----------------------------------------------------------------------------
//
TListItemProperties CFileManagerFileListContainer::CItemDrawer::Properties( 
        TInt aItemIndex ) const
    {
    aItemIndex = iContainer.SearchFieldToListIndex( aItemIndex );
    TListItemProperties prop(
        CColumnListBoxItemDrawer::Properties( aItemIndex ) );
    // Do not allow folder marking
    if ( iContainer.iDocument->Engine().IsFolder( aItemIndex ) )
        {
        prop.SetHiddenSelection( ETrue );
        }
    return prop;
    }

//-----------------------------------------------------------------------------
// CFileManagerFileListContainer::CListBox::CListBox
//-----------------------------------------------------------------------------
//
CFileManagerFileListContainer::CListBox::CListBox(
        CFileManagerFileListContainer& aContainer ) :
    CAknSingleGraphicStyleListBox(),
    iContainer( aContainer )
    {
    }

//-----------------------------------------------------------------------------
// CFileManagerFileListContainer::CListBox::CreateItemDrawerL
//-----------------------------------------------------------------------------
//
void CFileManagerFileListContainer::CListBox::CreateItemDrawerL()
    {
    CColumnListBoxData* data = CColumnListBoxData::NewL();
    CleanupStack::PushL( data );
    iItemDrawer = new ( ELeave ) CFileManagerFileListContainer::CItemDrawer(
        Model(),
        iEikonEnv->NormalFont(),
        data,
        iContainer );
    CleanupStack::Pop( data );
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::UpdateCba
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::UpdateCba()
    {
    iAppUi->NotifyCbaUpdate();
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::HandlePointerEventL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::HandlePointerEventL(
        const TPointerEvent &aPointerEvent )
    {
    if ( !iRightLeftNaviSupported )
        {
        CCoeControl::HandlePointerEventL( aPointerEvent );
        return;
        }
    if ( !AknLayoutUtils::PenEnabled() )
        {
        return;
        }
    TBool consumed( EFalse );
    switch ( aPointerEvent.iType )
        {
        case TPointerEvent::EButton1Down:
            {
            iDragStartPoint = aPointerEvent.iPosition;
            iDragging = EFalse;
            break;
            }
        case TPointerEvent::EDrag:
            {
            iDragging = ETrue;
            break;
            }
        case TPointerEvent::EButton1Up:
            {
            if ( iDragging )
                {
                iDragging = EFalse;
                // Solve drag direction and convert touch gesture to key event
                TInt xDelta( iDragStartPoint.iX - aPointerEvent.iPosition.iX );
                TInt keyCode( EKeyNull );
                if ( xDelta < -KTouchGestureThreshold )
                    {
                    keyCode = EKeyRightArrow;
                    }
                else if ( xDelta > KTouchGestureThreshold )
                    {
                    keyCode = EKeyLeftArrow;
                    }
                if ( keyCode != EKeyNull )
                    {
                    TKeyEvent keyEvent;
                    keyEvent.iCode = keyCode;
                    keyEvent.iScanCode = EStdKeyNull;
                    keyEvent.iModifiers = 0;
                    keyEvent.iRepeats = 1;
                    OfferKeyEventL( keyEvent, EEventKey );
                    consumed = ETrue;
                    }
                }
            break;
            }
        default:
            {
            iDragging = EFalse;
            break;
            }
        }
    if ( !consumed )
        {
        CCoeControl::HandlePointerEventL( aPointerEvent );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::SizeChanged
// 
// -----------------------------------------------------------------------------
//
void CFileManagerFileListContainer::SizeChanged()
    {
    if ( iSearchField )
        {
        AknFind::HandlePopupFindSizeChanged( this, &ListBox(), iSearchField );
        }
    else
        {
        CFileManagerContainerBase::SizeChanged();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::CountComponentControls
// 
// -----------------------------------------------------------------------------
//
TInt CFileManagerFileListContainer::CountComponentControls() const
    {
    TInt ret( CFileManagerContainerBase::CountComponentControls() );
    if ( iSearchField )
        {
        ++ret;
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ComponentControl
// 
// -----------------------------------------------------------------------------
//
CCoeControl* CFileManagerFileListContainer::ComponentControl( TInt aIndex ) const
    {
    if ( aIndex < CFileManagerContainerBase::CountComponentControls() )
        {
        return CFileManagerContainerBase::ComponentControl( aIndex );
        }
    if ( aIndex < CountComponentControls() )
        {
        return iSearchField;
        }
    return NULL;
    }    

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ListBoxCurrentItemIndex
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerFileListContainer::ListBoxCurrentItemIndex()
    {
    return SearchFieldToListIndex( ListBox().CurrentItemIndex() );
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ListBoxNumberOfItems
// 
// -----------------------------------------------------------------------------
//
TInt CFileManagerFileListContainer::ListBoxNumberOfItems()
    {
    if ( IsSearchFieldVisible() )
        {
        return static_cast< CAknFilteredTextListBoxModel* >(
            ListBox().Model() )->Filter()->FilteredNumberOfItems();
        }
    return CFileManagerContainerBase::ListBoxNumberOfItems();
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ListBoxSelectionIndexes
// 
// -----------------------------------------------------------------------------
// 
const CArrayFix< TInt >* CFileManagerFileListContainer::ListBoxSelectionIndexes()
    {
    if ( IsSearchFieldVisible() )
        {
        CAknListBoxFilterItems* filter =
            static_cast< CAknFilteredTextListBoxModel* >(
                ListBox().Model() )->Filter();

        if ( filter )
            {
            TRAPD( err, filter->UpdateSelectionIndexesL() );
            if ( err == KErrNone )
                {
                return filter->SelectionIndexes();
                }
            }
        return NULL;
        }
    return CFileManagerContainerBase::ListBoxSelectionIndexes();
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ListBoxSelectionIndexesCount
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerFileListContainer::ListBoxSelectionIndexesCount()
    {
    if ( IsSearchFieldVisible() )
        {
        CAknListBoxFilterItems* filter =
            static_cast< CAknFilteredTextListBoxModel* >(
                ListBox().Model() )->Filter();

        if ( filter )
            {
            TRAPD( err, filter->UpdateSelectionIndexesL() );
            if ( err == KErrNone )
                {
                return filter->SelectionIndexes()->Count();
                }
            }
        return 0;
        }
    return CFileManagerContainerBase::ListBoxSelectionIndexesCount();
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ListBoxToggleItemL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::ListBoxToggleItemL( TInt aIndex )
    {
    aIndex = ListToSearchFieldIndex( aIndex );
    CFileManagerContainerBase::ListBoxToggleItemL( aIndex );
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ListBoxIsItemSelected
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerFileListContainer::ListBoxIsItemSelected( TInt aIndex )
    {
    aIndex = ListToSearchFieldIndex( aIndex );
    return CFileManagerContainerBase::ListBoxIsItemSelected( aIndex );
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::SearchFieldToListIndex
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerFileListContainer::SearchFieldToListIndex( TInt aIndex )
    {
    if ( IsSearchFieldVisible() && aIndex >= 0 )
        {
        aIndex = static_cast< CAknFilteredTextListBoxModel* >(
            ListBox().Model() )->Filter()->FilteredItemIndex( aIndex );
        }
    return aIndex;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ListToSearchFieldIndex
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerFileListContainer::ListToSearchFieldIndex( TInt aIndex )
    {
    if ( IsSearchFieldVisible() && aIndex >= 0 )
        {
        aIndex = static_cast< CAknFilteredTextListBoxModel* >(
            ListBox().Model() )->Filter()->VisibleItemIndex( aIndex );
        }
    return aIndex;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ListBoxSetTextL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::ListBoxSetTextL( const TDesC& aText )
    {
    EnableSearchFieldL( EFalse );
    CFileManagerContainerBase::ListBoxSetTextL( aText );
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ListBoxSetTextL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::PageScrollL( TBool aUp )
    {
    CEikListBox& listBox( ListBox() );
    TInt numItems( listBox.Model()->NumberOfItems() );

    if ( numItems > 0 )
        {
        TInt lastIndex( numItems - 1 );
        if ( !aUp && listBox.View()->BottomItemIndex() == lastIndex )
            {
            listBox.SetCurrentItemIndex( lastIndex );
            }
        else
            {
            CListBoxView::TCursorMovement move( aUp ?
                CListBoxView::ECursorPrevScreen :
                CListBoxView::ECursorNextScreen );
            listBox.View()->MoveCursorL( move, CListBoxView::ENoSelection );
            listBox.SetCurrentItemIndex( listBox.CurrentItemIndex() );
            }
        DrawDeferred();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::IsSearchFieldVisible
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerFileListContainer::IsSearchFieldVisible() const
    {
    return ( iSearchField && iSearchField->IsVisible() );
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::EnableSearchFieldL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::EnableSearchFieldL(
        TBool aEnable, const TDesC& aSearchText )
    {
    if ( !iSearchField )
        {
        return;
        }

    CEikListBox& listBox( ListBox() );
    CAknFilteredTextListBoxModel* filteredModel =
        static_cast< CAknFilteredTextListBoxModel* >( listBox.Model() );

    if ( aEnable )
        {
        if ( !iSearchField->IsVisible() && listBox.Model()->NumberOfItems() )
            {
            iIndexAfterSearch = listBox.CurrentItemIndex();
            iSearchField->SetSearchTextL( aSearchText );
            if ( !filteredModel->Filter() )
                {
                filteredModel->CreateFilterL( &listBox, iSearchField );
                }
            filteredModel->Filter()->HandleItemArrayChangeL();
            iSearchField->MakeVisible( ETrue );
            iSearchField->SetFocus( ETrue );
            iSearchFieldEnabled = ETrue;
            }
        }
    else
        {
        iSearchFieldEnabled = EFalse;
        iSearchField->SetFocus( EFalse );
        iSearchField->MakeVisible( EFalse );
        iSearchField->ResetL();
        filteredModel->RemoveFilter();
        SetIndex( iIndexAfterSearch );
        }

    SizeChanged();
    UpdateCba();
    DrawDeferred();
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::SetCurrentItemIndexAfterSearch
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::SetCurrentItemIndexAfterSearch(
        TInt aIndex )
    {
    iIndexAfterSearch = aIndex;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::HandleChangeInFocus
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::HandleChangeInFocus()
    {
    // Update softkeys after search field has been canceled
    if ( ListBoxExists() &&
         IsFocused() &&
         iSearchFieldEnabled &&
         iSearchField &&
         !iSearchField->IsVisible() )
        {
        CAknFilteredTextListBoxModel* filteredModel =
            static_cast< CAknFilteredTextListBoxModel* >( ListBox().Model() );
        iSearchFieldEnabled = EFalse;
        TRAP_IGNORE( iSearchField->ResetL() );
        filteredModel->RemoveFilter();
        SetIndex( iIndexAfterSearch );
        UpdateCba();
        DrawDeferred();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::HandleDestructionOfFocusedItem
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileListContainer::HandleDestructionOfFocusedItem()
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::OfferSearchKeyEventL
// 
// -----------------------------------------------------------------------------
// 
TKeyResponse CFileManagerFileListContainer::OfferSearchKeyEventL(
        const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    if ( !iSearchField || aKeyEvent.iScanCode == EStdKeyYes )
        {
        return EKeyWasNotConsumed;
        }
    // Open search field on alpha digit        
    TBool isVisible( iSearchField->IsVisible() );
    if ( !isVisible &&
         aType == EEventKeyDown &&
         aKeyEvent.iScanCode )
        {
        TChar ch( aKeyEvent.iScanCode );
        if ( ch.IsAlphaDigit() )
            {
            EnableSearchFieldL( ETrue );
            return EKeyWasConsumed;
            }
        }
    // Close search field on clear-button if it's empty
    else if ( isVisible && aKeyEvent.iCode == EKeyBackspace )
        {
        iSearchField->GetSearchText( iSearchText );
        if ( !iSearchText.Length() )
            {
            EnableSearchFieldL( EFalse );
            return EKeyWasConsumed; 
            }
        }
    if ( isVisible )
        {
        TKeyResponse response( iSearchField->OfferKeyEventL( aKeyEvent, aType ) );
        UpdateCba();
        if ( response == EKeyWasConsumed )
            {
            return response;
            }
        }
    if ( !iOwnFastScrollDisabled )
        {
        if ( aKeyEvent.iCode == EKeyUpArrow && aKeyEvent.iRepeats > 0 )
            {
            PageScrollL( ETrue );
            return EKeyWasConsumed; 
            }
        if ( aKeyEvent.iCode == EKeyDownArrow && aKeyEvent.iRepeats > 0 )
            {
            PageScrollL( EFalse );
            return EKeyWasConsumed; 
            }
        }
    return EKeyWasNotConsumed;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ListBoxSelectItemL
// 
// -----------------------------------------------------------------------------
//
void CFileManagerFileListContainer::ListBoxSelectItemL( TInt aIndex )
    {
    aIndex = ListToSearchFieldIndex( aIndex );
    CFileManagerContainerBase::ListBoxSelectItemL( aIndex );
    }

// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::ListBoxDeselectItem
// 
// -----------------------------------------------------------------------------
//
void CFileManagerFileListContainer::ListBoxDeselectItem( TInt aIndex )
    {
    aIndex = ListToSearchFieldIndex( aIndex );
    CFileManagerContainerBase::ListBoxDeselectItem( aIndex );
    }
	
// -----------------------------------------------------------------------------
// CFileManagerFileListContainer::SearchFieldToListBoxIndex
// 
// -----------------------------------------------------------------------------
//
TInt CFileManagerFileListContainer::SearchFieldToListBoxIndex( TInt aIndex )
    {
    if ( !iSearchField || !iSearchField->IsVisible() )
       {
       return 0;
       }
    return SearchFieldToListIndex( aIndex );
    }
//  End of File  
