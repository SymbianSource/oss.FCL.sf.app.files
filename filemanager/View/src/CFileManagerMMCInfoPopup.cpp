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
* Description:  Memory card info popup
*
*/



// INCLUDE FILES
#include <StringLoader.h>
#include <aknPopup.h>
#include <aknlists.h>
#include <filemanagerview.rsg>
#include <cmemstatepopup.h>
#include "CFileManagerMMCInfoPopup.h"
#include "TFileManagerDriveInfo.h"


// CONSTANTS
_LIT( KSeparator, "\t" );
const TUint KDefaultItemCount = 4;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CFileManagerMMCInfoPopup::CFileManagerMMCInfoPopup
// ---------------------------------------------------------------------------
//
CFileManagerMMCInfoPopup::CFileManagerMMCInfoPopup()
    {
    }

// ---------------------------------------------------------------------------
// CFileManagerMMCInfoPopup::ConstructL
// ---------------------------------------------------------------------------
//
void CFileManagerMMCInfoPopup::ConstructL(
        const TFileManagerDriveInfo& aInfo )
    {
    // Get unit utility instance
    CMemStatePopup::GetUtilL( iUtil );

    iListBox = new ( ELeave ) CAknSingleHeadingPopupMenuStyleListBox;
    iPopupList = CAknPopupList::NewL(
        iListBox,
        R_AVKON_SOFTKEYS_OK_EMPTY,
        AknPopupLayouts::EMenuGraphicHeadingWindow );

    HBufC* title = StringLoader::LoadLC( R_QTN_FMGR_MEMCARD_INFO_HEADING );
    iPopupList->SetTitleL( *title );
    CleanupStack::PopAndDestroy( title );

    iListBox->ConstructL( iPopupList, EAknListBoxViewerFlags );

    MDesCArray* array = ConstructDataArrayL( aInfo );
    iListBox->Model()->SetItemTextArray( array );

    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
        CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
    iListBox->SetMopParent( this );
    }

// ---------------------------------------------------------------------------
// CFileManagerMMCInfoPopup::~CFileManagerMMCInfoPopup
// ---------------------------------------------------------------------------
// 
CFileManagerMMCInfoPopup::~CFileManagerMMCInfoPopup()
    {
    if( iIsDestroyed )
        {
        *iIsDestroyed = ETrue;
        }
    delete iListBox;
    delete iUtil;
    }

// ---------------------------------------------------------------------------
// CFileManagerMMCInfoPopup::NewL
// ---------------------------------------------------------------------------
// 
CFileManagerMMCInfoPopup* CFileManagerMMCInfoPopup::NewL(
        const TFileManagerDriveInfo& aInfo )
    {
    CFileManagerMMCInfoPopup* self =
        new( ELeave ) CFileManagerMMCInfoPopup();

    CleanupStack::PushL( self );
    self->ConstructL( aInfo );
    CleanupStack::Pop( self );

    return self;
    }

// ---------------------------------------------------------------------------
// CFileManagerMMCInfoPopup::ExecuteLD
// ---------------------------------------------------------------------------
// 
TBool CFileManagerMMCInfoPopup::ExecuteLD()
    {
    CFileManagerMMCInfoPopup* self = this;
    CleanupStack::PushL( self );

    TBool thisDestroyed( EFalse );
    iIsDestroyed = &thisDestroyed;

    TBool ret( EFalse );
    TRAPD( err, ( ret = iPopupList->ExecuteLD() ));

    // just in case check is this base object still on,
    // before using member variable
    if( !thisDestroyed )
        {
        iPopupList = NULL;
        }

    if( thisDestroyed )
        {
        CleanupStack::Pop( self );
        }
    else
        {
        CleanupStack::PopAndDestroy( self );
        }

    User::LeaveIfError( err );
    return ret;
    }

// ---------------------------------------------------------------------------
// CFileManagerMMCInfoPopup::ConstructDataArrayL
// ---------------------------------------------------------------------------
// 
MDesCArray* CFileManagerMMCInfoPopup::ConstructDataArrayL(
            const TFileManagerDriveInfo& aInfo )
    {
    CDesCArray* array = new( ELeave ) CDesCArrayFlat( KDefaultItemCount );
    CleanupStack::PushL( array );
    
    // Name
    HBufC* entry = NameEntryLC( aInfo );
    array->AppendL( *entry );
    CleanupStack::PopAndDestroy( entry );

    // Capacity
    entry = SizeEntryLC( R_QTN_FMGR_CARD_INFO_SIZE, aInfo.iCapacity );
    array->AppendL( *entry );
    CleanupStack::PopAndDestroy( entry ); 

    // Used
    entry = SizeEntryLC( R_QTN_FMGR_CARD_INFO_USED,
        aInfo.iCapacity - aInfo.iSpaceFree );
    array->AppendL( *entry );
    CleanupStack::PopAndDestroy( entry ); 

    // Free
    entry = SizeEntryLC( R_QTN_FMGR_CARD_INFO_FREE, aInfo.iSpaceFree );
    array->AppendL( *entry );
    CleanupStack::PopAndDestroy( entry );

    CleanupStack::Pop( array ); 
    return array;
    }

// ---------------------------------------------------------------------------
// CFileManagerMMCInfoPopup::NameEntryLC
// ---------------------------------------------------------------------------
// 
HBufC* CFileManagerMMCInfoPopup::NameEntryLC(
        const TFileManagerDriveInfo& aInfo )
    {
    HBufC* title = StringLoader::LoadLC( R_QTN_FMGR_CARD_INFO_NAME );
    HBufC* name = NULL;
    if ( aInfo.iName.Length() )
        {
        name = aInfo.iName.AllocLC();
        }
    else
        {
        name = StringLoader::LoadLC( R_QTN_FMGR_CARD_INFO_DEFAULT_NAME );
        }
    HBufC* dataStr = HBufC::NewL( title->Length() + name->Length() + 1 );
    TPtr dataPtr( dataStr->Des() );

    dataPtr.Append( *title );
    dataPtr.Append( KSeparator );
    dataPtr.Append( *name );

    CleanupStack::PopAndDestroy( name );
    CleanupStack::PopAndDestroy( title );
    CleanupStack::PushL( dataStr );
    return dataStr;
    }

// ---------------------------------------------------------------------------
// CFileManagerMMCInfoPopup::SizeEntryLC
// ---------------------------------------------------------------------------
// 
HBufC* CFileManagerMMCInfoPopup::SizeEntryLC(
        TInt aTextId, TInt64 aSize )
    {
    TPtrC unit( iUtil->SolveUnitAndSize( aSize ) );
    HBufC* title = StringLoader::LoadLC( aTextId );
    HBufC* sizeAndUnit = StringLoader::LoadLC(
        R_QTN_FMGR_CARD_INFO_DATA, unit, I64INT( aSize ) );
    HBufC* dataStr = HBufC::NewL(
        title->Length() + sizeAndUnit->Length() + 1 );
    TPtr dataPtr( dataStr->Des() );

    dataPtr.Append( *title );
    dataPtr.Append( KSeparator );
    dataPtr.Append( *sizeAndUnit );

    CleanupStack::PopAndDestroy( sizeAndUnit );
    CleanupStack::PopAndDestroy( title );
    CleanupStack::PushL( dataStr );
    return dataStr;    
    }

//  End of File  
