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
* Description:  Memory state popup implementation
*
*/


// SYSTEM INCLUDES
#include <e32std.h>
#include <AknProgressDialog.h>
#include <aknlists.h>
#include <ConeResLoader.h>
#include <bautils.h>
#include <StringLoader.h>
#include <aknPopup.h>
#include <memstatepopup.rsg>
#include <f32file.h>
#include <data_caging_path_literals.hrh> 

// USER INCLUDES
#include "msputil.h"
#include "cmemstatepopup.h"
#include "cmemstatepopupimpl.h"
#include "cmemscaneventreceiver.h"

//  CONSTANTS
_LIT( KDirAndFile,"Z:MemStatePopup.RSC" );
_LIT( KMemListSeparator, "\t" );

// Max length of a list row
// Has to be big enough to hold "%N %U"
const TInt KListItemMaxLength = 32;

// ---------------------------------------------------------------------------

CMemStatePopupImpl::~CMemStatePopupImpl( )
    {
    Cancel();
    // Close the resource
    iResLoader.Close( );

    if( iWaitDialog )
        {
        iWaitDialog->SetObserver( NULL );
        delete iWaitDialog;
        }

    delete iEventReceiver; // deletes also scan engine and server 
     
    delete iGroupNames;
    delete iListModel;
    delete iListBox;
    delete iScanResults;


    if( iOwnsUtil )
        {
        // Delete only if the actual dialog was launched
        delete iUtil;
        }

    // Try to delete also iPopup in case of leave
    // Must call CBase's destructor
    // because CAknPopupList destructor is protected
    delete ( CBase* ) iPopup;
    }

// ---------------------------------------------------------------------------

CMemStatePopupImpl::CMemStatePopupImpl( TDriveNumber aDrive, const TDesC& aTitle ) :
            CActive(EPriorityStandard),
            iFinished( EFalse ),
            iDrive( aDrive ),
            iTitle( const_cast<TDesC*> (&aTitle) ),
            iCoeEnv( *CCoeEnv::Static( ) ),
            iResLoader( iCoeEnv ),
            iOwnsUtil( ETrue )
    {
    CActiveScheduler::Add(this);
    }

// ---------------------------------------------------------------------------

// Used by GetUtilL
CMemStatePopupImpl::CMemStatePopupImpl( ) :
            CActive(EPriorityStandard),
            iFinished( EFalse ),
            iDrive( EDriveC ),
            iCoeEnv( *CCoeEnv::Static( ) ),
            iResLoader( iCoeEnv ),
            iOwnsUtil( EFalse )
    {
    }

// ---------------------------------------------------------------------------

void CMemStatePopupImpl::ConstructL( )
    {
    OpenResourceAndReadArrayL( );

    // Instantiate event receiver, memory scan engine and server
    iEventReceiver = CMemScanEventReceiver::NewL( *this );

    // Get data group names from engine
    iGroupNames = iEventReceiver->DataGroupsL( );

    // Create popup
    ConstructPopupL( );

    // Create wait dialog
    iWaitDialog = new( ELeave ) CAknWaitDialog(
                    reinterpret_cast< CEikDialog** >( &iWaitDialog ) );
    iWaitDialog->SetCallback( this );
    iWaitDialog->PrepareLC( R_MEMSTATE_WAIT_NOTE );
    
    StartObserver();
    }

// ---------------------------------------------------------------------------

void CMemStatePopupImpl::RunLD( TDriveNumber aDrive, const TDesC& aTitle )
    {
    CMemStatePopupImpl* self = new( ELeave ) CMemStatePopupImpl( aDrive, aTitle );
    CleanupStack::PushL( self );
    self->ConstructL( );

    // Run the actual process
    self->ExecuteL( );

    CleanupStack::PopAndDestroy( self );
    }


// ---------------------------------------------------------------------------

void CMemStatePopupImpl::GetUtilL( CMSPUtil*& aUtil )
    {
    CMemStatePopupImpl* self = new( ELeave ) CMemStatePopupImpl;
    CleanupStack::PushL( self );

    // Read the unit array from resource
    self->OpenResourceAndReadArrayL( );
    aUtil = self->iUtil; // iUtil is not deleted

    CleanupStack::PopAndDestroy( self );
    }

// ---------------------------------------------------------------------------

void CMemStatePopupImpl::StartL( )
    {
#ifdef _DEBUG
    RDebug::Print( _L( "Scanning started!" ) );
#endif
    }

// ---------------------------------------------------------------------------

void CMemStatePopupImpl::QuitL( TInt /*aReason*/ )
    {
#ifdef _DEBUG
    RDebug::Print( _L( "Scanning ended!" ) );
#endif
    if( !iFinished )
        {
        // Tell waitdialog that it can quit now
        iWaitDialog->ProcessFinishedL( );
        iFinished = ETrue;
        }
    }

// ---------------------------------------------------------------------------

void CMemStatePopupImpl::ErrorL( TInt aError )
    {
#ifdef _DEBUG
    RDebug::Print( _L( "Error scanning memory: %d" ), aError );
#endif

    // getting rid of UREL compiler warning
    if( !aError ) 
        {
        aError = KErrNone;
        }
    }

// ---------------------------------------------------------------------------

void CMemStatePopupImpl::DialogDismissedL( TInt aButtonId )
    {

    // Cancel is pressed while scanning is in progress
    if( iEventReceiver->ScanInProgress( ) )
        {
        iFinished = ETrue;
        iEventReceiver->Cancel( ); // Stop scanning
        }

    // Cancel is pressed while wait note is on screen (scanning may be completed)
    if( aButtonId == EAknSoftkeyCancel ) // instead of EAknSoftkeyDone
        {
        iPopup->CancelPopup( ); // Remove memory state popup from screen
        return;
        }

    // Get the new result array
    delete iScanResults;
    iScanResults = NULL;
    TRAPD( err, iScanResults = iEventReceiver->ScanResultL( ) );
    if( err != KErrNone )
        {
        iPopup->CancelPopup( ); // Remove memory state popup from screen
        return;
        }
    RefreshL( );
    }

// ---------------------------------------------------------------------------

void CMemStatePopupImpl::ConstructPopupL( )
    {
    iListBox = new( ELeave ) CMemStateListBox( );

    iPopup = CAknPopupList::NewL( iListBox, R_AVKON_SOFTKEYS_OK_EMPTY,
                            AknPopupLayouts::EMenuGraphicHeadingWindow );
    iPopup->SetTitleL( *iTitle );

    // Set up listbox
    iListBox->ConstructL( iPopup, EAknListBoxViewerFlags );

    // Create listbox model and give the model to the listbox
    iListModel = new( ELeave ) CDesCArraySeg( iGroupNames->Count( ) );

    // Create scroll indicator
    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame( )->SetScrollBarVisibilityL(
                    CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
    iListBox->Model( )->SetItemTextArray( iListModel );
    iListBox->Model( )->SetOwnershipType( ELbmDoesNotOwnItemArray );
    iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL( ETrue );
    iListBox->ItemDrawer()->ClearFlags( CListItemDrawer::ESingleClickDisabledHighlight );
    }

// ---------------------------------------------------------------------------

void CMemStatePopupImpl::ExecuteL( )
    {
    // Show wait dialog
    iWaitDialog->RunLD( );

    // Start the scanning
    User::LeaveIfError(iEventReceiver->ScanL( iDrive ));
    
    // Get the result array
    iScanResults = iEventReceiver->ScanResultL( );
   
    // Set all results zero, so that no actual results are shown
    // before the scan is done.
    NullifyResults( );

    // Update the list with initial values (0's)
    RefreshL( );

    // Show popup
    iPopup->ExecuteLD( );
    iPopup = NULL; // Has to be set NULL because deleted in destructor
    }

// ---------------------------------------------------------------------------

void CMemStatePopupImpl::RefreshL( )
    {
    //TBuf< KListItemMaxLength > result;
    TBuf< KListItemMaxLength > unitText;
    TBuf< KListItemMaxLength > tempText;
    TPtrC unit;
    TInt64 res64( 0 );
    TInt count( iScanResults ? iScanResults->Count( ) : 0 );
    TInt maxGroupName(0);
    
    for( TInt i = 0; i < iGroupNames->Count(); i++ )
        {
        if( maxGroupName < ( *iGroupNames )[ i ].Length() )
            {
            maxGroupName = ( *iGroupNames )[ i ].Length();
            }
        }
        
    // Allocate memory for size, unit text and actual group name
    HBufC* result = HBufC::NewLC( maxGroupName + 2*KListItemMaxLength );
    TPtr resultPtr( result->Des() );
    HBufC* unitFormatter = StringLoader::LoadLC( R_UNIT_FORMATTER, &iCoeEnv );

    iListModel->Reset( );

    // Go through the result array and append to the model as text
    for( TInt i = 0; i < count; i++ )
        {
        res64 = ( *iScanResults )[ i ];

        unit.Set( iUtil->SolveUnitAndSize( res64 ) );
        StringLoader::Format( tempText, *unitFormatter, 0, I64INT( res64 ) );
        StringLoader::Format( unitText, tempText, 1, unit );
        resultPtr.Zero( );

        // The actual listbox row is constructed here
        // iListModel is just an array of descriptors
        if ( User::Language() == ELangArabic )
            {
            resultPtr.Append( KRightToLeftMark );
            }
        resultPtr.Append( unitText );
        resultPtr.Append( KMemListSeparator );
        if( i < iGroupNames->Count() )
            {
            resultPtr.Append( ( *iGroupNames )[ i ] );
            }
        else
            {
            resultPtr.Append( ( *iGroupNames )[ 0 ] );
            }
        iListModel->AppendL( resultPtr );
        }

    CleanupStack::PopAndDestroy( unitFormatter );
    CleanupStack::PopAndDestroy( result );

    // Update the listbox
    iListBox->HandleItemAdditionL( );
    }

// ---------------------------------------------------------------------------

void CMemStatePopupImpl::NullifyResults( )
    {
    // Set all results to zero
    if( iScanResults )
        {
        TInt count( iScanResults->Count( ) );
        for( TInt t = 0; t < count; t++ )
            {
            ( *iScanResults )[ t ] = 0;
            }
        }
    }

// ---------------------------------------------------------------------------

void CMemStatePopupImpl::OpenResourceAndReadArrayL( )
    {
    // Open dll resource
    
    TParse* fp = new(ELeave) TParse(); 
    fp->Set(KDirAndFile, &KDC_RESOURCE_FILES_DIR, NULL); 
    TFileName fileName( fp->FullName() );
    delete fp;

    BaflUtils::NearestLanguageFile( iCoeEnv.FsSession( ),
                                    fileName );
    iResLoader.OpenL( fileName );

    // Read localised unit texts from resource
    TResourceReader reader;
    iCoeEnv.CreateResourceReaderLC( reader, R_ARRAY_UNITS );
    CDesCArrayFlat* units = reader.ReadDesCArrayL( );
    CleanupStack::PushL( units );
    iUtil = CMSPUtil::NewL( units ); // Give the array to the util class
    CleanupStack::Pop( units );
    CleanupStack::PopAndDestroy(); // Private HBufC8* variable
                                   // of CreateResourceReaderLC()
    }

// --------------------------------------------------------------------------- 
void CMemStatePopupImpl::RunL()
    {
    TVolumeInfo volInfo;
    TInt err = iCoeEnv.FsSession().Volume( volInfo, iDrive );
    if(err != KErrNone)
        {
        iPopup->CancelPopup( );
        }
    else
        {
        StartObserver();
        }
    }

// --------------------------------------------------------------------------- 
void CMemStatePopupImpl::DoCancel()
    {
    iCoeEnv.FsSession().NotifyChangeCancel();
    }

// --------------------------------------------------------------------------- 
void CMemStatePopupImpl::StartObserver()
    {
    if ( IsActive() ) 
        {
        Cancel();
        }
    iCoeEnv.FsSession().NotifyChange( ENotifyDisk, iStatus );
    SetActive();
    }

// End of File
