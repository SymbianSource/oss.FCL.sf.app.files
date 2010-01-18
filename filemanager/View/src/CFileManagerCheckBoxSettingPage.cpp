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
* Description:  Checkbox setting page
*
*/



// INCLUDE FILES
#include <akncheckboxsettingpage.h>
#include "CFileManagerCheckBoxSettingPage.h"


// ======== MEMBER FUNCTIONS ========
// ----------------------------------------------------------------------------
// CFileManagerCheckBoxSettingPage::CFileManagerCheckBoxSettingPage
// ----------------------------------------------------------------------------
// 
CFileManagerCheckBoxSettingPage::CFileManagerCheckBoxSettingPage(
        const TInt aResourceID,
        CSelectionItemList& aItemArray,
        const TInt aDominantItemIndex ) :
    CAknCheckBoxSettingPage( aResourceID, &aItemArray ),
    iItemsArray( aItemArray ),
    iDominantItemIndex( aDominantItemIndex )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerCheckBoxSettingPage::~CFileManagerCheckBoxSettingPage
// ----------------------------------------------------------------------------
// 
CFileManagerCheckBoxSettingPage::~CFileManagerCheckBoxSettingPage()
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerCheckBoxSettingPage::UpdateSelectionL
// ----------------------------------------------------------------------------
// 
void CFileManagerCheckBoxSettingPage::UpdateSettingL()   
    {
    CAknCheckBoxSettingPage::UpdateSettingL();
    UpdateSelection();
    }

// ----------------------------------------------------------------------------
// CFileManagerCheckBoxSettingPage::UpdateSelection
// ----------------------------------------------------------------------------
// 
void CFileManagerCheckBoxSettingPage::UpdateSelection()
    {
    TInt count( iItemsArray.Count() );
    if ( iDominantItemIndex < 0 || iDominantItemIndex > count  )
        {
        return; // Dominant item not used
        }

    CAknSetStyleListBox* listBox = ListBoxControl();
    CListBoxView* view = listBox->View();

    TInt selectedItem( listBox->CurrentItemIndex() );
    if ( selectedItem < 0 || selectedItem > count  )
        {
        return; // Invalid selection
        }

    TBool redraw( EFalse );

    // Deselect all other items if dominant item is selected
    if ( iDominantItemIndex == selectedItem &&
        iItemsArray.At( selectedItem )->SelectionStatus() )
        {
        for ( TInt i( 0 ); i < count; ++i )
            {
            if ( i != iDominantItemIndex )
                {
                iItemsArray.At( i )->SetSelectionStatus( EFalse );
                view->DeselectItem( i );
                }
            }
        redraw = ETrue;
        }
    // Deselect dominant item if any other item is selected
    else if ( iDominantItemIndex != selectedItem &&
             iItemsArray.At( iDominantItemIndex )->SelectionStatus() )
        {
        for ( TInt i( 0 ); i < count; ++i )
            {
            if ( i != iDominantItemIndex &&
                iItemsArray.At( i )->SelectionStatus() )
                {
                iItemsArray.At( iDominantItemIndex )->SetSelectionStatus(
                    EFalse );
                view->DeselectItem( iDominantItemIndex );
                redraw = ETrue;
                break;
                }
            }
        }

    // Hide ok if selection is empty
    TBool empty( ETrue );
    for ( TInt i( 0 ); i < count; ++i )
        {
        if ( iItemsArray.At( i )->SelectionStatus() )
            {
            empty = EFalse;
            break;
            }
        }
    CEikButtonGroupContainer* cba = Cba();
    if ( empty )
        {
        cba->MakeCommandVisible( EAknSoftkeyOk, EFalse );
        }
    else
        {
        cba->MakeCommandVisible( EAknSoftkeyOk, ETrue );
        }
    cba->DrawDeferred();

    if ( redraw )
        {
        listBox->DrawDeferred();
        }
    }

// End of File
