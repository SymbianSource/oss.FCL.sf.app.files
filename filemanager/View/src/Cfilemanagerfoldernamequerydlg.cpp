/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Dialog for asking folder name from user
*
*/



// INCLUDE FILES
#include <StringLoader.h>
#include <aknnotewrappers.h> 
#include <bautils.h>
#include <CFileManagerEngine.h>
#include <CFileManagerUtils.h>
#include <FileManagerView.rsg>
#include "Cfilemanagerfoldernamequerydlg.h"
#include "CFileManagerCommonDefinitions.h"
#include "FileManagerDlgUtils.h"
#include "CFileManagerItemProperties.h"


// CONSTANTS
// If number is 10 or more 2 digits are needed to display it
const TInt KTwoDigitNeeded = 10;
_LIT( KFormatStringTwoDigit, "%02d" );
_LIT( KFormatString, "%d" );
const TInt KFormatBufLen = 16;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerFolderNameQueryDlg::NewL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFolderNameQueryDlg* CFileManagerFolderNameQueryDlg::NewL(
        TDes& aDataText,
        CFileManagerEngine& aEngine,
        TBool aNameGeneration )
    {
    CFileManagerFolderNameQueryDlg* self = 
        new( ELeave ) CFileManagerFolderNameQueryDlg( aDataText, aEngine );
    
    CleanupStack::PushL( self );
    self->ConstructL( aNameGeneration, aDataText );
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNameQueryDlg::CFileManagerFolderNameQueryDlg
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerFolderNameQueryDlg::CFileManagerFolderNameQueryDlg( 
                                    TDes& aDataText, 
                                    CFileManagerEngine& aEngine ) :
    CAknTextQueryDialog( aDataText ),
    iEngine( aEngine )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNameQueryDlg::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderNameQueryDlg::ConstructL( TBool aNameGeneration, 
                                                 TDes& aDataText )
    {
    if( aNameGeneration )
        {
        TInt i( 1 );
        HBufC* folderName =
            StringLoader::LoadLC( R_FILEMANAGER_DEFAULT_FOLDER_NAME );
        TBuf< KFormatBufLen > numBuf;

        while( iEngine.IsNameFoundL( *folderName ) )
            {
            CleanupStack::PopAndDestroy( folderName );
            numBuf.Zero();
            if( i < KTwoDigitNeeded ) 
                {
                numBuf.Format( KFormatStringTwoDigit, i );
                }
            else
                {
                numBuf.Format( KFormatString, i );
                }
            AknTextUtils::DisplayTextLanguageSpecificNumberConversion(
                numBuf );
            folderName = StringLoader::LoadLC(
                R_FILEMANAGER_DEFAULT_FOLDER_NAME_N, numBuf );
            ++i;
            }
        Text().Copy( *folderName );
        CleanupStack::PopAndDestroy( folderName );
        }
    else
        {
        iOldName = aDataText.AllocL();
        TParsePtrC parse(
            CFileManagerUtils::StripFinalBackslash( *iOldName ) );
        if ( parse.PathPresent() )
            {
            Text().Copy( parse.NameAndExt() );
            }
        else
            {
            Text().Copy( *iOldName );
            }
            
        // Strip any directionality markers to get pure name
        TPtr ptr( iOldName->Des() );
        AknTextUtils::StripCharacters( ptr, KFmgrDirectionalChars );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNameQueryDlg::~CFileManagerFolderNameQueryDlg
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerFolderNameQueryDlg::~CFileManagerFolderNameQueryDlg()
    {
    delete iOldName;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNameQueryDlg::DoOkToExitL
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerFolderNameQueryDlg::DoOkToExitL( TInt aButtonId )
    {
    TBool result( CAknTextQueryDialog::OkToExitL( aButtonId ) );

    HBufC* userText = Text().AllocLC();
    TPtr ptrUserText( userText->Des() );

    // Strip any directionality markers to get pure name
    AknTextUtils::StripCharacters( ptrUserText, KFmgrDirectionalChars );

    // Check folder name
    TBool isValidName( EFalse );
    TPtrC empty( KNullDesC );
    TParsePtrC oldName( CFileManagerUtils::StripFinalBackslash(
        iOldName ? *iOldName : empty ) );
    if ( oldName.PathPresent() )
        {
        isValidName = iEngine.IsValidName(
            oldName.DriveAndPath(), *userText, ETrue );
        }
    else
        {
        isValidName = iEngine.IsValidName( KNullDesC, *userText, ETrue );
        }

    if( !isValidName )
        {
        if( iEngine.IllegalChars( *userText ) )
            {
            FileManagerDlgUtils::ShowInfoNoteL(
                R_QTN_FLDR_ILLEGAL_CHARACTERS );
            }
        else
            {
            FileManagerDlgUtils::ShowInfoNoteL(
                R_QTN_FLDR_BAD_FILE_NAME );
            }
        CAknQueryControl* queryControl = QueryControl();
        if ( queryControl )
            {
            CEikEdwin* edwin = static_cast< CEikEdwin* >( queryControl->
                ControlByLayoutOrNull( EDataLayout ) );
            if ( edwin )
                {
                edwin->SetSelectionL( edwin->TextLength(), 0 );
                }
            }
        CleanupStack::PopAndDestroy( userText );
        return EFalse;
        }

    HBufC* userTextFullPath = HBufC::NewLC( KMaxFileName );
    TPtr ptrUserTextFullPath( userTextFullPath->Des() );
    if ( oldName.PathPresent() )
        {
        ptrUserTextFullPath.Append( oldName.DriveAndPath() );
        }

    ptrUserTextFullPath.Append( *userText );

    // if entry with same name - not ok except if name same as original
    if( iEngine.IsNameFoundL( ptrUserTextFullPath ) &&
        ( !iOldName || ( iOldName &&
                         iOldName->Des().CompareF( ptrUserTextFullPath ) ) ) )
        {
        FileManagerDlgUtils::ShowInfoNoteL(
                    		 R_QTN_WMLBM_NAME_ALREADY_USED );

        CAknQueryControl* queryControl = QueryControl();
        if ( queryControl )
            {
            CEikEdwin* edwin = static_cast< CEikEdwin* >
                ( queryControl->ControlByLayoutOrNull( EDataLayout ) );
            if ( edwin )
                {
                edwin->SetSelectionL( edwin->TextLength(), 0 );
                }
            }
        result = EFalse;
        }
    CleanupStack::PopAndDestroy( userTextFullPath );
    CleanupStack::PopAndDestroy( userText );
    return result;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileNameQueryDlg::OkToExitL
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerFolderNameQueryDlg::OkToExitL( TInt aButtonId )
    {
    if ( iCallbackDisabled )
        {
        return EFalse; // Block unwanted softkey events
        }
    TBool ret( EFalse );
    iCallbackDisabled = ETrue;
    TRAPD( err, ret = DoOkToExitL( aButtonId ) );
    iCallbackDisabled = EFalse;
    User::LeaveIfError( err );
    return ret;
    }
    
// -----------------------------------------------------------------------------
// CFileManagerFolderNameQueryDlg::OfferKeyEventL
// 
// -----------------------------------------------------------------------------
// 
TKeyResponse CFileManagerFolderNameQueryDlg::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType ) 
    { 
    TKeyResponse response = EKeyWasNotConsumed;
    if ( aType == EEventKey && aKeyEvent.iCode == EKeyEnter ) 
        { 
        response = EKeyWasConsumed;
        } 
    else
        {
        response = CAknTextQueryDialog::OfferKeyEventL(aKeyEvent, aType); 
        }
    return response;
    }

//  End of File  
