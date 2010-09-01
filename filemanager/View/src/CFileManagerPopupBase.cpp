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
* Description:  Base class for file manager popups
*
*/



// INCLUDE FILES
#include <StringLoader.h>
#include <filemanagerview.rsg>
#include <FileManagerDebug.h>
#include <CFileManagerCommonDefinitions.h>
#include "CFileManagerPopupBase.h"
#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
 #include <aknmessagequerydialog.h>
#else // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
 #include <aknPopup.h>
 #include <aknlists.h>
#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
_LIT( KFmgrLineFeedWithClosingBoldTag, "</AknMessageQuery Bold>\n" );
#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerPopupBase::CFileManagerPopupBase
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerPopupBase::CFileManagerPopupBase()
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerPopupBase::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFileManagerPopupBase::ConstructL()
    {
#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    const TInt KLineFeedSpace = 2;
    const TInt KReservedForBoldTags = 600;
    TInt i( 0 );
    TInt len( 0 );
    MDesCArray* array = ConstructDataArrayL();

    // Reformat array to a single buffer for CAknMessageQueryDialog.
    for ( i = 0; i < array->MdcaCount(); i++ )
        {
        len += array->MdcaPoint( i ).Length() + KLineFeedSpace;
        len += KReservedForBoldTags; //Reserve some space for putting bold tags
        }
    iData = HBufC::New( len );
    if ( !iData )
        {
        delete array;
        User::LeaveNoMemory();
        }
    TPtr ptr = iData->Des();
    for ( i = 0; i < array->MdcaCount(); i++ )
        {
        if ( i > 0 )
            {
            ptr.Append( KFmgrLineFeed );
            ptr.Append( KFmgrLineFeed );
            }
        ptr.Append( KOpeningBoldTag );    // insert openning bold tag
        ptr.Append( array->MdcaPoint( i ) );
        TInt tmpPos = ptr.Find( KFmgrTab );
        ptr.Replace(tmpPos, 1, KFmgrLineFeedWithClosingBoldTag);
        }
//    AknTextUtils::ReplaceCharacters( ptr, KFmgrTab, KFmgrLineFeed()[0] );
    delete array;

#else // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    iListBox = new(ELeave) CAknDoublePopupMenuStyleListBox;
    iPopupList = CAknPopupList::NewL( iListBox,
                                      R_AVKON_SOFTKEYS_OK_EMPTY, 
                                      AknPopupLayouts::EMenuDoubleWindow );
    iListBox->ConstructL( iPopupList, EAknListBoxViewerFlags );

    MDesCArray* array = ConstructDataArrayL();
    iListBox->Model()->SetItemTextArray( array );

    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
        CEikScrollBarFrame::EOff,
        CEikScrollBarFrame::EAuto );
    iListBox->SetMopParent( this );

#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
    }
    
// -----------------------------------------------------------------------------
// CFileManagerPopupBase::~CFileManagerPopupBase
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerPopupBase::~CFileManagerPopupBase()
    {
#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    delete iTitle;
    delete iData;

#else // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    if( iIsDestroyed )
        {
        *iIsDestroyed = ETrue;
        }
    delete iListBox;

#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
    }

// -----------------------------------------------------------------------------
// CFileManagerPopupBase::ExecuteLD
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerPopupBase::ExecuteLD()
    {
#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    CleanupStack::PushL( this );
    CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL( *iData );
    dlg->PrepareLC( R_FILEMANAGER_POPUP_DIALOG );
    dlg->QueryHeading()->SetTextL( *iTitle );
    dlg->SetMopParent( this );
    TCallBack cb( CFileManagerPopupBase::LinkCallback, this );
    dlg->SetLink( cb );
    TBool ret( dlg->RunLD() );
    CleanupStack::PopAndDestroy( this );
    return ret;

#else // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    CFileManagerPopupBase* self = this;
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

#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
    }

// -----------------------------------------------------------------------------
// CFileManagerPopupBase::SetTitleL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerPopupBase::SetTitleL(const TDesC &aTitle)
    {
#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    if (iTitle)
        {
        delete iTitle;
        iTitle = NULL;
        }
    iTitle = aTitle.AllocL();

#else // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    iPopupList->SetTitleL( aTitle );

#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
    }

#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
// -----------------------------------------------------------------------------
// CFileManagerPopupBase::ActivateLinkL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerPopupBase::ActivateLinkL()
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerPopupBase::LinkCallback
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerPopupBase::LinkCallback(TAny* aPtr)
    {
    CFileManagerPopupBase* ptr = static_cast<CFileManagerPopupBase*>( aPtr );
    TRAPD( err, ptr->ActivateLinkL() );
    ERROR_LOG1( "CFileManagerPopupBase::LinkCallback()-err:%d", err )
    return err;
    }
#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

//  End of File  
