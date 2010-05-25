/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Dialog for asking file name from user
*
*/


// INCLUDE FILES
#include <StringLoader.h>
#include <aknnotewrappers.h> 
#include <bautils.h>
#include <CFileManagerEngine.h>
#include <filemanagerview.rsg>
#include "CFileManagerFileNameQueryDlg.h"
#include "CFileManagerCommonDefinitions.h"
#include "CFileManagerItemProperties.h"
#include "FileManagerDlgUtils.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerFileNameQueryDlg::NewL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFileNameQueryDlg* CFileManagerFileNameQueryDlg::NewL(
        const TDesC& aOldName,
        TDes& aNewName,
        CFileManagerEngine& aEngine )
    {
    CFileManagerFileNameQueryDlg* self =
        new( ELeave ) CFileManagerFileNameQueryDlg(
            aNewName, aEngine );
    
    CleanupStack::PushL( self );
    self->ConstructL( aOldName );
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileNameQueryDlg::CFileManagerFileNameQueryDlg
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerFileNameQueryDlg::CFileManagerFileNameQueryDlg(
        TDes& aNewName,
        CFileManagerEngine& aEngine ) :
    CAknTextQueryDialog( aNewName ),
    iEngine( aEngine )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerFileNameQueryDlg::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileNameQueryDlg::ConstructL( const TDesC& aOldName )
    {
    TParsePtrC name( aOldName );
    Text().Copy( name.NameAndExt() );
    iOldName = aOldName.AllocL();

    // Strip any directionality markers to get pure name
    TPtr ptr( iOldName->Des() );
    AknTextUtils::StripCharacters( ptr, KFmgrDirectionalChars );
    }

// -----------------------------------------------------------------------------
// CFileManagerFileNameQueryDlg::~CFileManagerFileNameQueryDlg
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerFileNameQueryDlg::~CFileManagerFileNameQueryDlg()
    {
    delete iOldName;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileNameQueryDlg::DoOkToExitL
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerFileNameQueryDlg::DoOkToExitL( TInt aButtonId )
    {
    TBool result( CAknTextQueryDialog::OkToExitL( aButtonId ) );

    HBufC* userText = Text().AllocLC();
    TPtr ptrUserText( userText->Des() );
    ptrUserText.TrimRight();

    // Strip any directionality markers to get pure name
    AknTextUtils::StripCharacters( ptrUserText, KFmgrDirectionalChars );

    // Check file name
    TBool isValidName( EFalse );
    TParsePtrC oldName( *iOldName );
    if ( oldName.PathPresent() )
        {
        isValidName = iEngine.IsValidName(
            oldName.DriveAndPath(), *userText, EFalse );
        }
    else
        {
        isValidName = iEngine.IsValidName( KNullDesC, *userText, EFalse );
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
        if (queryControl)
            {
            CEikEdwin* edwin = static_cast< CEikEdwin* >(
                queryControl->ControlByLayoutOrNull( EDataLayout ) );
            if (edwin)
                {
                edwin->SetSelectionL( edwin->TextLength(), 0 );
                }
            }
        CleanupStack::PopAndDestroy( userText );
        return EFalse;
        }

    HBufC* userTextFullPath = HBufC::NewLC( KMaxFileName );
    TPtr ptrUserTextFullPath( userTextFullPath->Des() );

    ptrUserTextFullPath.Append( oldName.DriveAndPath() );
    ptrUserTextFullPath.Append( *userText );
    // if some other entry found with same name
    // not ok except if name same as original
    if( oldName.NameAndExt().CompareF( *userText ) &&
        ( iEngine.IsNameFoundL( ptrUserTextFullPath ) || 
          BaflUtils::FileExists(CCoeEnv::Static()->FsSession(), ptrUserTextFullPath)) )
        {
        TBool overWrite( EFalse );
        TUint32 fileType( 0 );

        TRAPD( err, fileType = iEngine.FileTypeL( ptrUserTextFullPath ) );
        if ( err != KErrNone && err != KErrNotFound && err != KErrAccessDenied )
            {
            User::Leave( err );
            }

        if( ( err == KErrNotFound ) ||
            ( err == KErrAccessDenied ) ||
            ( fileType & ( CFileManagerItemProperties::EOpen |
                           CFileManagerItemProperties::EReadOnly |
                           CFileManagerItemProperties::EFolder ) ) )
            {
            FileManagerDlgUtils::ShowInfoNoteL(
                R_QTN_FLDR_NAME_ALREADY_USED, Text() );
            }
        else
            {
            overWrite =
                FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
                    R_QTN_ITEM_OVERWRITE_QUERY, Text() );
            }

        if( !overWrite )
            {
            CAknQueryControl* queryControl = QueryControl();
            if (queryControl)
                {
                CEikEdwin* edwin = static_cast< CEikEdwin* >(
                    queryControl->ControlByLayoutOrNull( EDataLayout ) );
                if (edwin)
                    {
                    edwin->SetSelectionL( edwin->TextLength(), 0 );
                    }
                }
            result = EFalse;
            }
        }

    TParsePtrC newName( ptrUserText );
    if (  result && oldName.Ext().CompareF( newName.Ext() ) )
        {
        FileManagerDlgUtils::ShowWarningNoteL(
            R_QTN_FM_WARNING_FILE_EXTENSION );
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
TBool CFileManagerFileNameQueryDlg::OkToExitL( TInt aButtonId )
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
// CFileManagerFileNameQueryDlg::OfferKeyEventL
// 
// -----------------------------------------------------------------------------
// 
TKeyResponse CFileManagerFileNameQueryDlg::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType ) 
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
