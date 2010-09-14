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
* Description:  Setting list container in file manager
*
*/



// INCLUDE FILES
#include <filemanager.rsg>
#include <aknlists.h>
#include <barsread.h>
#include <eikclbd.h>
#include <gulicon.h>
#include <aknconsts.h>
#include <AknIconArray.h>
#include <avkon.mbg>
#include <filemanager.mbg>
#include <CFileManagerEngine.h>
#include <FileManagerEngine.hrh>
#include "FileManager.hrh"
#include "CFileManagerDocument.h"
#include "CFileManagerIconArray.h"
#include "CFileManagerSettingListContainer.h"


// needed because _LIT macro does not expand parameter, which is also macro
#define _CREATE_LIT(a,b) _LIT(a,b)

// CONSTANTS
const TInt KRestoreIconListGranularity = 3;
_CREATE_LIT( KFileManagerBitmapFile, filemanager_mbm_file_location );


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerSettingListContainer::CFileManagerSettingListContainer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerSettingListContainer::CFileManagerSettingListContainer(
        const TListType aType ) :
    iType( aType )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerSettingListContainer::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerSettingListContainer::ConstructL(
        const TRect& aRect,
        const TInt aFocusedIndex,
        MDesCArray* aArray,
        const TDesC& aHelpContext )
    {
    CFileManagerContainerBase::ConstructL( aRect, aFocusedIndex );
    SetTextArray( aArray );
    SetHelpContext( aHelpContext );
    }

// -----------------------------------------------------------------------------
// CFileManagerSettingListContainer::CreateListBoxL
// 
// -----------------------------------------------------------------------------
// 
CEikTextListBox* CFileManagerSettingListContainer::CreateListBoxL()
    {
    CEikTextListBox* listBox = NULL;

    switch ( iType )
        {
        case EListBackup:
            {
            listBox = CreateBackupListBoxL();
            break;
            }
        case EListRestore:
            {
            listBox = CreateRestoreListBoxL();
            break;
            }
        default:
            {
            break;
            }
        }

    return listBox;
    }

// -----------------------------------------------------------------------------
// CFileManagerSettingListContainer::CreateBackupListBoxL
// 
// -----------------------------------------------------------------------------
// 
CEikTextListBox* CFileManagerSettingListContainer::CreateBackupListBoxL()
    {
    CAknSettingStyleListBox* listBox = new( ELeave ) CAknSettingStyleListBox;
    CleanupStack::PushL( listBox );
    listBox->SetContainerWindowL( *this );
    listBox->ConstructL( this, EAknListBoxSelectionList );
    listBox->ItemDrawer()->ColumnData()->SetSkinEnabledL( ETrue );
    CleanupStack::Pop( listBox );
    return listBox;
    }

// -----------------------------------------------------------------------------
// CFileManagerSettingListContainer::CreateRestoreListBoxL
// 
// -----------------------------------------------------------------------------
// 
CEikTextListBox* CFileManagerSettingListContainer::CreateRestoreListBoxL()
    {
    CAknAppUi* appUi = static_cast< CAknAppUi* >( ControlEnv()->AppUi() );
    CFileManagerDocument* doc =
        static_cast< CFileManagerDocument* >( appUi->Document() );
    CAknDoubleGraphicStyleListBox* listBox =
        new( ELeave ) CAknDoubleGraphicStyleListBox;
    CleanupStack::PushL( listBox );
    listBox->SetContainerWindowL( *this );
    listBox->ConstructL( this, EAknListBoxMultiselectionList );
    listBox->ItemDrawer()->ColumnData()->SetSkinEnabledL( ETrue );

    // Create icon array and icons
    CAknIconArray* iconArray = new( ELeave ) CAknIconArray(
        KRestoreIconListGranularity );
    listBox->ItemDrawer()->
        ColumnData()->SetIconArray( iconArray );    // changes ownership

    CGulIcon* icon = CFileManagerIconArray::LoadIconL(
        KAvkonBitmapFile,
        EMbmAvkonQgn_indi_checkbox_on,
        EMbmAvkonQgn_indi_checkbox_on_mask,
        KAknsIIDQgnIndiCheckboxOn.iMajor,
        KAknsIIDQgnIndiCheckboxOn.iMinor,
        ETrue );
    CleanupStack::PushL( icon );
    iconArray->AppendL( icon );
    CleanupStack::Pop( icon );

    icon = CFileManagerIconArray::LoadIconL(
        KAvkonBitmapFile,
        EMbmAvkonQgn_indi_checkbox_off,
        EMbmAvkonQgn_indi_checkbox_off_mask,
        KAknsIIDQgnIndiCheckboxOff.iMajor,
        KAknsIIDQgnIndiCheckboxOff.iMinor,
        ETrue );
    CleanupStack::PushL( icon );
    iconArray->AppendL( icon );
    CleanupStack::Pop( icon );

    icon = CFileManagerIconArray::LoadIconL(
        KFileManagerBitmapFile,
        EMbmFilemanagerQgn_indi_mmc_add,
        EMbmFilemanagerQgn_indi_mmc_add_mask,
        KAknsIIDQgnIndiMmcAdd.iMajor,
        KAknsIIDQgnIndiMmcAdd.iMinor,
        ETrue );
    CleanupStack::PushL( icon );
    iconArray->AppendL( icon );
    CleanupStack::Pop( icon );

    icon = CFileManagerIconArray::LoadIconL(
        KFileManagerBitmapFile,
        EMbmFilemanagerQgn_indi_usb_memc,
        EMbmFilemanagerQgn_indi_usb_memc_mask,
        KAknsIIDQgnIndiMmcAdd.iMajor,
        KAknsIIDQgnIndiMmcAdd.iMinor,
        ETrue );
    CleanupStack::PushL( icon );
    iconArray->AppendL( icon );
    CleanupStack::Pop( icon );

    // Disable lists MSK observer because the container will be used
    // as observer
    listBox->EnableMSKObserver( EFalse );
    CleanupStack::Pop( listBox );
    return listBox;
    }

// -----------------------------------------------------------------------------
// CFileManagerSettingListContainer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerSettingListContainer* CFileManagerSettingListContainer::NewL(
        const TRect& aRect,
        const TInt aFocusedIndex,
        const TListType aType,
        MDesCArray* aArray,
        const TDesC& aHelpContext )
    {
    CFileManagerSettingListContainer* self =
        new( ELeave ) CFileManagerSettingListContainer( aType );
    CleanupStack::PushL( self );
    self->ConstructL( aRect, aFocusedIndex, aArray, aHelpContext );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerSettingListContainer::~CFileManagerSettingListContainer
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerSettingListContainer::~CFileManagerSettingListContainer()
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerSettingListContainer::OfferKeyEventL
// 
// -----------------------------------------------------------------------------
//
TKeyResponse CFileManagerSettingListContainer::OfferKeyEventL(
        const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TKeyResponse response = EKeyWasNotConsumed;

    switch( aKeyEvent.iCode )
        {
        case EKeyEnter: // FALLTHROUGH
        case EKeyOK:
            {
            // Notify restore list about selection change
            if ( iType == EListRestore )
                {
                static_cast< CAknAppUi* >( ControlEnv()->AppUi() )->
                    ProcessCommandL( EFileManagerSelectionKey );
                response = EKeyWasConsumed;
                }
            else
            	{
            	response = ListBox().OfferKeyEventL( aKeyEvent, aType );
            	}
            break;
            }
        case EKeyApplicationF:
            {
            // Suppress this from listbox,
            // otherwise empty multiselection list sometimes works badly on exit
            break;
            }
        default:
            {
            response = CFileManagerContainerBase::OfferKeyEventL(
                aKeyEvent, aType );
            }
        }
    return response;
    }

//  End of File  
