/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Dialog utilities
*
*/



// INCLUDE FILES
#include <avkon.rsg>
#include <StringLoader.h>
#include <aknradiobuttonsettingpage.h>
#include <akncheckboxsettingpage.h>
#include <aknmfnesettingpage.h>
#include <aknmessagequerydialog.h>
#include <aknnotewrappers.h>
#include <FileManagerView.rsg>
#include <CFileManagerEngine.h>
#include "FileManagerDlgUtils.h"
#include "CFileManagerMMCInfoPopup.h"
#include "CFileManagerCheckBoxSettingPage.h"
#include "CFileManagerFileNameQueryDlg.h"
#include "Cfilemanagerfoldernamequerydlg.h"
#include "CFileManagerInfoPopup.h"


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowOOfMSettingQueryL
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool FileManagerDlgUtils::ShowOOfMSettingQueryL(
        const TInt aTitleId,
        const MDesCArray* aTextArray,
        TInt& aSelectedIndex )
    {
    TInt index( aSelectedIndex );
    TBool ret( EFalse );
    HBufC* title = StringLoader::LoadLC( aTitleId );
    CAknRadioButtonSettingPage* dlg =
        new ( ELeave )CAknRadioButtonSettingPage(
            R_FILEMANAGER_SETTING_RADIOBUTTON_PAGE, index, aTextArray );
    CleanupStack::PushL( dlg );
    dlg->SetSettingTextL( *title );
    CleanupStack::Pop( dlg );
    if ( dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ) )
        {
        // Get new selected item and inform change
        if ( index != aSelectedIndex )
            {
            aSelectedIndex = index;
            ret = ETrue;
            }
        }
    CleanupStack::PopAndDestroy( title );
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowOOfMSettingQueryL
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool FileManagerDlgUtils::ShowOOfMSettingQueryL(
        const TInt aTitleId,
        const TInt aTextIds,
        TInt& aSelectedIndex )
    {
    CDesCArray* texts = CCoeEnv::Static()->ReadDesCArrayResourceL( aTextIds );
    CleanupStack::PushL( texts );
    TBool ret( ShowOOfMSettingQueryL( aTitleId, texts, aSelectedIndex ) );
    CleanupStack::PopAndDestroy( texts );
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowWeekdayQueryL
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool FileManagerDlgUtils::ShowWeekdayQueryL(
        const TInt aTitleId,
        TDay& aDay )
    {
    const TInt KDaysPerWeek = 7;
    TLocale locale;
    TDay startingDay( locale.StartOfWeek() );
    CDesCArray* texts = new ( ELeave ) CDesCArrayFlat( KDaysPerWeek );
    CleanupStack::PushL( texts );
    TInt j( startingDay % KDaysPerWeek );
    TDayName dayName;
    for ( TInt i( 0 ); i < KDaysPerWeek; i++ )
        {
        dayName.Set( static_cast< TDay >( j ) );
        texts->AppendL( dayName );
        j = ( j + 1 ) % KDaysPerWeek;
        }
    TInt index( 0 );
    if ( aDay >= startingDay )
        {
        index = aDay - startingDay;
        }
    else
        {
        index = KDaysPerWeek - startingDay + aDay;
        }
    TBool ret( ShowOOfMSettingQueryL( aTitleId, texts, index ) );
    if ( ret )
        {
        aDay = static_cast< TDay >( ( startingDay + index ) % KDaysPerWeek );
        }
    CleanupStack::PopAndDestroy( texts );
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowNOfMSettingQueryL
// ----------------------------------------------------------------------------
//
EXPORT_C TBool FileManagerDlgUtils::ShowNOfMSettingQueryL(
        const TInt aTitleId,
        const TInt aTextIds,
        TUint32& aSelection,
        const TInt aDominantIndex )
    {
    TBool ret( EFalse );

    // Get title and text array
    HBufC* title = StringLoader::LoadLC( aTitleId );
    CDesCArray* texts = CCoeEnv::Static()->ReadDesCArrayResourceL( aTextIds );
    CleanupStack::PushL( texts );

    // Create selection array
    TInt count( texts->MdcaCount() );
    CSelectionItemList* items = 
        new ( ELeave ) CSelectionItemList( count );
    CleanupStack::PushL( items );
    for ( TInt i( 0 ); i < count; i++ )
        {
        TBool check( EFalse );
        if ( aSelection & ( 1 << i ) )
            {
            check = ETrue;
            }
        TPtrC ptr( texts->MdcaPoint( i ) );
        CSelectableItem* selectionItem = new ( ELeave ) CSelectableItem(
                ptr, check );
        CleanupStack::PushL( selectionItem );
        selectionItem->ConstructL();
        items->AppendL( selectionItem );
        CleanupStack::Pop( selectionItem );
        }

    // Create multiselection list
    CFileManagerCheckBoxSettingPage* dlg =
        new ( ELeave ) CFileManagerCheckBoxSettingPage(
            R_FILEMANAGER_SETTING_CHECKBOX_PAGE, *items, aDominantIndex );
    CleanupStack::PushL( dlg );
    dlg->SetSettingTextL( *title );
    CleanupStack::Pop( dlg );
    if ( dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ) )
        {
        // Get new selection and inform changes
        TUint32 selection( 0 );
        for ( TInt i( 0 ); i < count; i++ )
            {
            if ( items->At( i )->SelectionStatus() )
                {
                selection |= 1 << i;
                }
            }
        if ( selection != aSelection )
            {
            aSelection = selection;
            ret = ETrue;
            }
        }
    items->ResetAndDestroy();
    CleanupStack::PopAndDestroy( items );
    CleanupStack::PopAndDestroy( texts );
    CleanupStack::PopAndDestroy( title );
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowTimeSettingQueryL
// ----------------------------------------------------------------------------
//
EXPORT_C TBool FileManagerDlgUtils::ShowTimeSettingQueryL(
        const TInt aTitleId, TTime& aTime )
    {
    TTime time( aTime );
    TBool ret( EFalse );
    HBufC* title = StringLoader::LoadLC( aTitleId );
    CAknTimeSettingPage* dlg =
        new (ELeave) CAknTimeSettingPage(
            R_FILEMANAGER_TIME_SETTING_PAGE,
            time);
    CleanupStack::PushL( dlg );
    dlg->SetSettingTextL( *title );
    CleanupStack::Pop( dlg );
    if ( dlg->ExecuteLD( CAknSettingPage::EUpdateWhenAccepted ) )
        {
        // Get new time and inform change
        if ( time != aTime )
            {
            aTime = time;
            ret = ETrue;
            }
        }
    CleanupStack::PopAndDestroy( title );
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowInfoQueryL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowInfoQueryL(
        const TDesC& aText )
    {
    CAknMessageQueryDialog* dlg =
        CAknMessageQueryDialog::NewL( const_cast< TDesC& >( aText ) );
    dlg->ExecuteLD( R_FILEMANAGER_INFO_QUERY );
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowInfoQueryL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowInfoQueryL(
        const TInt aTextId, const TDesC& aValue )
    {
    HBufC* text = NULL;
    if ( aValue.Length() )
        {
        text = StringLoader::LoadLC( aTextId, aValue );
        }
    else
        {
        text = StringLoader::LoadLC( aTextId );
        }
    ShowInfoQueryL( *text );
    CleanupStack::PopAndDestroy( text );
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowInfoQueryL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowInfoQueryL(
        const TInt aTextId, const TInt aValue )
    {
    HBufC* text = StringLoader::LoadLC( aTextId, aValue );
    ShowInfoQueryL( *text );
    CleanupStack::PopAndDestroy( text );
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowErrorNoteL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowErrorNoteL(
        const TInt aTextId, const TDesC& aValue )
    {
    HBufC* text = NULL;
    if ( aValue.Length() )
        {
        text = StringLoader::LoadLC( aTextId, aValue );
        }
    else
        {
        text = StringLoader::LoadLC( aTextId );
        }
    ShowErrorNoteL( *text );
    CleanupStack::PopAndDestroy( text );
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowConfirmNoteL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowConfirmNoteL(
        const TInt aTextId )
    {
    HBufC* text = StringLoader::LoadLC( aTextId );
    CAknConfirmationNote* confirmNote =
                        new( ELeave ) CAknConfirmationNote( ETrue );
    confirmNote->ExecuteLD( *text );
    CleanupStack::PopAndDestroy( text );
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowWarningNoteL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowWarningNoteL(
        const TInt aTextId )
    {
    HBufC* text = StringLoader::LoadLC( aTextId );
    CAknWarningNote* warningNote =
                        new( ELeave ) CAknWarningNote( ETrue );
    warningNote->ExecuteLD( *text );
    CleanupStack::PopAndDestroy( text );
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowConfirmQueryWithYesNoL
// ----------------------------------------------------------------------------
//
EXPORT_C TBool FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
        const TInt aTextId, const TDesC& aValue )
    {
    HBufC* text = NULL;
    if ( aValue.Length() )
        {
        text = StringLoader::LoadLC( aTextId, aValue );
        }
    else
        {
        text = StringLoader::LoadLC( aTextId );
        }

    TBool ret( ShowConfirmQueryWithYesNoL( *text ) );
    CleanupStack::PopAndDestroy( text );
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowConfirmQueryWithYesNoL
// ----------------------------------------------------------------------------
//
EXPORT_C TBool FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
        const TDesC& aText )
    {
    TBool ret( EFalse );
    CAknQueryDialog* dlg = CAknQueryDialog::NewL();
    if ( dlg->ExecuteLD( R_FILEMANAGER_CONFIRM_QUERY_WITH_YES_NO, aText ) )
        {
        ret = ETrue;
        }
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowConfirmQueryWithOkL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowConfirmQueryWithOkL(
        const TIcons aIcons,
        const TInt aTextId,
        const TDesC& aValue )
    {
    HBufC* text = NULL;
    if ( aValue.Length() )
        {
        text = StringLoader::LoadLC( aTextId, aValue );
        }
    else
        {
        text = StringLoader::LoadLC( aTextId );
        }
    ShowConfirmQueryWithOkL( aIcons, *text );
    CleanupStack::PopAndDestroy( text );
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowConfirmQueryWithOkL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowConfirmQueryWithOkL(
        const TIcons aIcons,
        const TInt aTextId,
        const TInt aValue )
    {
    HBufC* text = StringLoader::LoadLC( aTextId, aValue );
    ShowConfirmQueryWithOkL( aIcons, *text );
    CleanupStack::PopAndDestroy( text );
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowConfirmQueryWithOkL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowConfirmQueryWithOkL(
        const TIcons aIcons,
        const TDesC& aText )
    {
    TInt resId( 0 );
    switch ( aIcons )
        {
        case EInfoIcons:
            {
            resId = R_FILEMANAGER_INFO_CONFIRM_QUERY_WITH_OK;
            break;
            }
        case EErrorIcons:
            {
            resId = R_FILEMANAGER_ERROR_CONFIRM_QUERY_WITH_OK;
            break;
            }
        default:
            {
            break;
            }
        }
    if ( resId )
        {
        CAknQueryDialog* dlg = CAknQueryDialog::NewL();
        dlg->ExecuteLD( resId, aText );
        }
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowConfirmQueryWithOkCancelL
// ----------------------------------------------------------------------------
//
EXPORT_C TBool FileManagerDlgUtils::ShowConfirmQueryWithOkCancelL(
        const TDesC& aText )
    {
    TBool ret( EFalse );
    CAknQueryDialog* dlg = CAknQueryDialog::NewL();
    if ( dlg->ExecuteLD(
            R_FILEMANAGER_CONFIRM_QUERY_WITH_OK_CANCEL, aText ) )
        {
        ret = ETrue;
        }
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowConfirmQueryWithOkCancelL
// ----------------------------------------------------------------------------
//
EXPORT_C TBool FileManagerDlgUtils::ShowConfirmQueryWithOkCancelL(
        const TInt aTextId,
        const TDesC& aValue )
    {
    HBufC* text = NULL;
    if ( aValue.Length() )
        {
        text = StringLoader::LoadLC( aTextId, aValue );
        }
    else
        {
        text = StringLoader::LoadLC( aTextId );
        }
    TBool ret( ShowConfirmQueryWithOkCancelL( *text ) );
    CleanupStack::PopAndDestroy( text );
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowInfoNoteL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowInfoNoteL(
        const TDesC& aText )
    {
    CAknInformationNote* dlg = new(ELeave) CAknInformationNote( ETrue );
    dlg->ExecuteLD( aText );
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowInfoNoteL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowInfoNoteL(
        const TInt aTextId, const TDesC& aValue )
    {
    HBufC* text = NULL;
    if( aValue.Length() )
        {
        text = StringLoader::LoadLC( aTextId, aValue );
        }
    else
        {
        text = StringLoader::LoadLC( aTextId );
        }
    ShowInfoNoteL( *text );
    CleanupStack::PopAndDestroy( text );
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowInfoNoteL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowInfoNoteL(
        const TInt aTextId,  const TInt aValue )
    {
    HBufC* text = StringLoader::LoadLC( aTextId, aValue );
    ShowInfoNoteL( *text );
    CleanupStack::PopAndDestroy( text );
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowSimplePasswordQueryL
// ----------------------------------------------------------------------------
//
EXPORT_C TBool FileManagerDlgUtils::ShowSimplePasswordQueryL(
        const TInt aTitleId,
        TDes& aPwd )
    {
    TBool ret( EFalse );
    HBufC* title = StringLoader::LoadLC( aTitleId );
    ret = ShowSimplePasswordQueryL( *title, aPwd );
    CleanupStack::PopAndDestroy( title );
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowPasswordQueryL
// ----------------------------------------------------------------------------
//
EXPORT_C TBool FileManagerDlgUtils::ShowPasswordQueryL(
        TDes& aPwd )
    {
    HBufC* confirm = HBufC::NewLC( aPwd.MaxLength() );
    TPtr ptrConfirm( confirm->Des() );
    TBool ret( EFalse );
    // Execute dialog again and again until cancelled or passwords match
    TBool isDone( EFalse );
    while( !isDone  )
        {
        aPwd.FillZ( aPwd.MaxLength() );
        aPwd.Zero();
        ptrConfirm.FillZ( ptrConfirm.MaxLength() );
        ptrConfirm.Zero();

        CAknMultiLineDataQueryDialog* dlg =
                    CAknMultiLineDataQueryDialog::NewL(
                        aPwd, ptrConfirm );
        if ( dlg->ExecuteLD( R_FILEMANAGER_PASSWORD_QUERY ) )
            {
            // Check if the words match
            if ( aPwd.Compare( ptrConfirm ) )
                {
                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_PASSWORDS_DONT_MATCH_TEXT );
                }
            else
                {
                ret = ETrue;
                isDone = ETrue;
                }
            }
        else
            {
            isDone = ETrue;
            }
        }
    CleanupStack::PopAndDestroy( confirm );
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowMemoryStoreInfoPopupL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowMemoryStoreInfoPopupL(
        const TFileManagerDriveInfo& aInfo
         )
    {
    CFileManagerMMCInfoPopup* popup = CFileManagerMMCInfoPopup::NewL(
        aInfo );
    popup->ExecuteLD();
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowFileNameQueryL
// ----------------------------------------------------------------------------
//
EXPORT_C TBool FileManagerDlgUtils::ShowFileNameQueryL(
        const TInt aTitleId,
        const TDesC& aOldName,
        TDes& aNewName,
        CFileManagerEngine& aEngine )
    {
    TBool ret( EFalse );
    HBufC* title = StringLoader::LoadLC( aTitleId );
    CFileManagerFileNameQueryDlg* dlg =
        CFileManagerFileNameQueryDlg::NewL(
            aOldName, aNewName, aEngine );
    if ( dlg->ExecuteLD( R_FILEMANAGER_FILE_NAME_QUERY, *title  ) )
        {
        ret = ETrue;
        }
    CleanupStack::PopAndDestroy( title );
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowFolderNameQueryL
// ----------------------------------------------------------------------------
//
EXPORT_C TBool FileManagerDlgUtils::ShowFolderNameQueryL(
        const TInt aTitleId,
        TDes& aName,
        CFileManagerEngine& aEngine,
        const TBool aNameGeneration )
    {
    TBool ret( EFalse );
    HBufC* title = StringLoader::LoadLC( aTitleId );
    CFileManagerFolderNameQueryDlg* dlg =
        CFileManagerFolderNameQueryDlg::NewL(
            aName, aEngine, aNameGeneration );
    if ( dlg->ExecuteLD( R_FILEMANAGER_FILE_NAME_QUERY, *title  ) )
        {
        ret = ETrue;
        }
    CleanupStack::PopAndDestroy( title );
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowItemInfoPopupL(
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowItemInfoPopupL(
        CFileManagerItemProperties& aProperties,
        const CFileManagerFeatureManager& aFeatureManager )
    {
    CFileManagerInfoPopup* popup = CFileManagerInfoPopup::NewL(
        aProperties, aFeatureManager );
    popup->ExecuteLD();
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowSimplePasswordQueryL
// ----------------------------------------------------------------------------
//
EXPORT_C TBool FileManagerDlgUtils::ShowSimplePasswordQueryL(
        const TDesC& aText, TDes& aPwd )
    {
    TBool ret( EFalse );
    CAknTextQueryDialog* dlg =
        CAknTextQueryDialog::NewL( aPwd, CAknQueryDialog::ENoTone );
    if ( dlg->ExecuteLD( R_FILEMANAGER_SIMPLE_PASSWORD_QUERY, aText ) )
        {
        ret = ETrue;
        }
    return ret;
    }

// ----------------------------------------------------------------------------
// FileManagerDlgUtils::ShowErrorNoteL
// ----------------------------------------------------------------------------
//
EXPORT_C void FileManagerDlgUtils::ShowErrorNoteL( const TDesC& aText )
    {
    CAknErrorNote* dlg = new( ELeave ) CAknErrorNote( ETrue );
    dlg->ExecuteLD( aText );  
    }

//  End of File  
