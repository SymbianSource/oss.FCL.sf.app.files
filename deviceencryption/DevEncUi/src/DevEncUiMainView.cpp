/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of CDevEncUiMainView.
*
*/

// INCLUDES

//  Class include
#include "DevEncUiMainView.h"

// System includes
#include <aknnotewrappers.h>
#include <aknViewAppUi.h>	// CAknViewAppUi
#include <DevEncUi.rsg>
#include <e32cmn.h>
#include <StringLoader.h>
#include <DevEncExternalCRKeys.h>

// User includes
#include "DevEncLog.h"
#include "DevEnc.hrh"
#include "DevEncUiEncryptionOperator.h"

#include "DevEncUiMainViewContainer.h"
#include "DevEncUiMemoryEntity.h"

// ================= MEMBER FUNCTIONS =======================

/**
* Destructor.  Frees up memory for the iLabel.
*/
CDevEncUiMainView::~CDevEncUiMainView()
	{
    for ( TInt i = 0; i < iMemEntities.Count(); i++ )
        {
        iMemEntities[i]->RemoveObserver( iContainer );
        }
	delete iContainer;
    delete iEncOperator;
	}

/**
* Symbian OS 2 phase constructor.
* Constructs the CDevEncUiMainView using the NewLC method, popping
* the constructed object from the CleanupStack before returning it.
*
* @param aRect The rectangle for this window
* @return The newly constructed CDevEncUiMainView
*/
CDevEncUiMainView* CDevEncUiMainView::NewL(
                               RArray<CDevEncUiMemoryEntity*>& aMemEntities,
                               CRepository*& aCrSettings,
                               TInt& aMmcStatus )
	{
	CDevEncUiMainView* self = CDevEncUiMainView::NewLC( aMemEntities,
                                                        aCrSettings,
                                                        aMmcStatus );
	CleanupStack::Pop( self );
	return self;
	}

/**
* Symbian OS 2 phase constructor.
* Constructs the CDevEncUiMainView using the constructor and ConstructL
* method, leaving the constructed object on the CleanupStack before returning it.
*
* @param aRect The rectangle for this window
* @return The newly constructed CDevEncUiMainView
*/
CDevEncUiMainView* CDevEncUiMainView::NewLC(
                               RArray<CDevEncUiMemoryEntity*>& aMemEntities,
                               CRepository*& aCrSettings,
                               TInt& aMmcStatus )
	{
	CDevEncUiMainView* self =
        new ( ELeave ) CDevEncUiMainView( aMemEntities,
                                          aCrSettings,
                                          aMmcStatus );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

CDevEncUiMainView::CDevEncUiMainView( RArray<CDevEncUiMemoryEntity*>& aMemEntities,
                                      CRepository*& aCrSettings,
                                      TInt& aMmcStatus )
    : iMemEntities( aMemEntities ),
      iCrSettings( aCrSettings ),
      iMmcStatus( aMmcStatus )
	{
	}

/**
* Symbian OS 2nd phase constructor.
* Uses the superclass constructor to construct the view
*/
void CDevEncUiMainView::ConstructL()
	{
	BaseConstructL( R_DEVENCUI_MAINVIEW );

    iContainer = CDevEncUiMainViewContainer::NewL( AppUi()->ClientRect(),
												   iMemEntities,
                                                   *this,
                                                   iCrSettings,
                                                   iMmcStatus );
    iContainer->SetMopParent( this );

    for ( TInt i = 0; i < iMemEntities.Count(); i++ )
        {
        iMemEntities[i]->AddObserverL( iContainer );
        }
    }

/**
* Called by the framework
* @return The Uid for this view
*/
TUid CDevEncUiMainView::Id() const
	{
	return TUid::Uid( EDevEncUiMainViewId );
	}

/**
* Called by the framework when the view is activated.
* Adds the container to the control stack.
*/
void CDevEncUiMainView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
									 TUid /*aCustomMessageId*/,
									 const TDesC8& /*aCustomMessage*/)
    {
    AppUi()->AddToStackL( *this, iContainer );
    
    if ( iEikonEnv->StartedAsServerApp() )
        {
        CEikButtonGroupContainer* cba = Cba();
        cba->SetCommandSetL( R_AVKON_SOFTKEYS_OPTIONS_BACK );
        }
    }

/**
* Called by the framework when the view is deactivated.
* Removes the container from the control stack.
*/
void CDevEncUiMainView::DoDeactivate()
	{
	if ( iContainer )
		{
		AppUi()->RemoveFromStack( iContainer );
		}
	}

/**
* From CEikAppUi, takes care of command handling for this view.
*
* @param aCommand command to be handled
*/
void CDevEncUiMainView::HandleCommandL( TInt aCommand )
	{

	switch ( aCommand )
		{
        case EDevEncUiCommandChange:
            {
            // Show the toggle popup
            if ( iContainer->ChangeSelectedItemL() )
                {
                // The user made a new selection
                TDevEncUiMemoryType memoryType =
                    static_cast<TDevEncUiMemoryType>( iContainer->SelectedItem() );
                EncryptionStatusChangeReq( memoryType );
                }
            break;
            }
		default:
			{
			AppUi()->HandleCommandL( aCommand );
			break;
			}
		}
	}

void CDevEncUiMainView::DynInitMenuPaneL( TInt aResourceId,
                                          CEikMenuPane* aMenuPane )
    {
    if ( ! iContainer ||
         iMemEntities.Count() == 0 )
        {
        return;
        }

    // Disable the "Change" menu item if an encryption operation is ongoing
    if ( aResourceId == R_DEVENCUI_MENU_MAIN )
        {
        TBool proceed( ETrue );
        TInt dmControl( 0 );
        TInt err = iCrSettings->Get( KDevEncUiDmControl, dmControl );
        if ( err )
            {
            DFLOG2( "Cenrep error %d", err );
            User::Leave( err );
            }

        // Check if phone memory encryption is controlled by dev management
        if ( ( dmControl & KDmControlsPhoneMemory ) &&
             ( iContainer->SelectedItem() == EPhoneMemory ) )
            {
            // Selected memory is controlled by DM -> disable "Change"
            aMenuPane->SetItemDimmed( EDevEncUiCommandChange, ETrue );
            proceed = EFalse;
            }

        // Check if memory card encryption is controlled by dev management
        if ( ( dmControl & KDmControlsMemoryCard ) &&
             ( iContainer->SelectedItem() == EMemoryCard ) )
            {
            // Selected memory is controlled by DM -> disable "Change"
            aMenuPane->SetItemDimmed( EDevEncUiCommandChange, ETrue );
            proceed = EFalse;
            }

        if ( !proceed )
            {
            DFLOG( "Hiding Change, item is controlled by DM" );
            return;
            }

        // Show the "Change" item if encryption status can be changed
        TBool allowChange( EFalse );
        if ( ( iMemEntities[ EMemoryCard ]->State() == EUnmounted  ||
               iMemEntities[ EMemoryCard ]->State() == EDecrypted ||
               iMemEntities[ EMemoryCard ]->State() == EEncrypted ) &&
               ( iMemEntities[ EPhoneMemory ]->State() == EDecrypted ||
                 iMemEntities[ EPhoneMemory ]->State() == EEncrypted ) )
            {
            allowChange = ETrue;
            }

        if ( allowChange )
            {
            aMenuPane->SetItemDimmed( EDevEncUiCommandChange, EFalse );
            }
        else
            {
            aMenuPane->SetItemDimmed( EDevEncUiCommandChange, ETrue );
            }
        }
    }



CDevEncUiAppUi& CDevEncUiMainView::GetAppUi()
	{
	CAknViewAppUi* aknViewAppUi = AppUi();
    CDevEncUiAppUi* appUi = reinterpret_cast<CDevEncUiAppUi*>( aknViewAppUi );
	return ( *appUi );
	}

void CDevEncUiMainView::EncryptionStatusChangeReq( TDevEncUiMemoryType aType )
    {
    // Can't do anything about errors here
    TInt error( KErrNone );
    TRAP( error, DoEncryptionStatusChangeReqL( aType ) );
    if ( error )
        {
        DFLOG2( "DoEncryptionStatusChangeReqL error %d", error );
        }
    }

// TBI: Async this function and create a subfunction with -L
void CDevEncUiMainView::DoEncryptionStatusChangeReqL( TDevEncUiMemoryType aType )
    {
    DFLOG( "CDevEncUiMainView::DoEncryptionStatusChangeReqL" );

    // Check if we are allowed to start an encryption operation
    TInt dmControl( 0 );
    TInt err = iCrSettings->Get( KDevEncUiDmControl, dmControl );
    if ( err )
        {
        DFLOG2( "Cenrep error %d", err );
        User::Leave( err );
        }
    if ( DmControlsMemory( aType, dmControl ) )
        {
        // Encryption controlled by admin
        ErrorNoteL( R_DEVENCUI_NOTE_ADMIN_CONTROL );
        return;
        }

    // Create the encryption operator if necessary
    if ( ! iEncOperator )
        {
        iEncOperator = CDevEncUiEncryptionOperator::NewL( *iEikonEnv,
                                                          *AppUi(),
                                                          iCrSettings );
        }

    // Get the currently highlighted memory type
    CDevEncUiMemoryEntity* mem = iMemEntities[ aType ];

    DFLOG3( "Mem type: %d, state: %d", aType, mem->State() );

    if ( aType == EPhoneMemory )
        {
        CDevEncUiMemoryEntity* phoneMemory = iMemEntities[ EPrimaryPhoneMemory];
        iEncOperator->HandlePhoneMemEncryptRequestL( mem, phoneMemory );
        }
    else // aType = EMemoryCard
        {
        HandleMemoryCardEncryptRequestL( mem );
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiMainView::ErrorNoteL()
// TBI: Create base class and move this there
// --------------------------------------------------------------------------
//
void CDevEncUiMainView::ErrorNoteL( TInt aResourceId )
    {
    HBufC* message = StringLoader::LoadLC( aResourceId );
    CAknErrorNote* errorNote = new ( ELeave ) CAknErrorNote;
    errorNote->ExecuteLD( *message );
    CleanupStack::PopAndDestroy( message );
    }

// TBI: Create base class and move this there
TBool CDevEncUiMainView::DmControlsMemory( TDevEncUiMemoryType aType,
                                           TInt aDmControlSetting )
    {
    TBool result( EFalse );
    if ( ( aType == EPhoneMemory ) &&
         ( aDmControlSetting & KDmControlsPhoneMemory ) )
        {
        result = ETrue;
        }
    if ( ( aType == EMemoryCard ) &&
         ( aDmControlSetting & KDmControlsMemoryCard ) )
        {
        result = ETrue;
        }
    return result;
    }

// --------------------------------------------------------------------------
// CDevEncUiMainView::HandleMemoryCardEncryptRequestL()
// --------------------------------------------------------------------------
//
void CDevEncUiMainView::HandleMemoryCardEncryptRequestL( CDevEncUiMemoryEntity* aMem )
    {
    // Check if the driver has a known encryption key
    TInt mmcEncrOn( 0 );
    iCrSettings->Get( KDevEncUserSettingMemoryCard, mmcEncrOn );
    
    if ( aMem->State() == EDecrypted ||
         aMem->State() == EUnmounted )
        {
        if ( mmcEncrOn )
            {
            // Mmc decrypted or ejected but encryption is on ->
            // suggest destruction of key
            DFLOG( "Mmc decrypted, encryption key set -> destroy key?" );
            if ( aMem->State() == EDecrypted )
                {
                // Mmc inserted -> indicate that it is decrypted
                // (see UI spec 2.12)
                iEncOperator->DestroyKeyQueryL( ETrue );
                }
            else
                {
                // Mmc ejected -> no need to show note
                // (see UI spec 2.11)
                iEncOperator->DestroyKeyQueryL( EFalse );
                }
            }
        else
            {
            // Mmc decrypted and encryption is off -> show encryption menu
            DFLOG( "Mmc decrypted, encryption key not set -> enc menu" );
            AppUi()->ActivateLocalViewL( TUid::Uid( EDevEncUiEncrViewId ) );
            }
        }
    else if ( aMem->State() == EEncrypted )
        {
        // Encryption is on -> show decryption menu
        DFLOG( "Mmc encrypted -> decr menu" );
        AppUi()->ActivateLocalViewL( TUid::Uid( EDevEncUiDecrViewId ) );
        }
    else if ( aMem->State() == ECorrupted )
        {
        if ( mmcEncrOn )
            {
            // Mmc seems to be encrypted with another key.
            // Ask if the user wants to import another key.
            // (See UI spec 2.10)
            DFLOG( "CDevEncUiMainView::HandleMemoryCardEncryptRequestL => Mmc encrypted with another key, enc on -> import key" );
            iEncOperator->SuggestMmcImportKeyL( aMem, ETrue );
            }
        else
            {
            // Mmc is probably encrypted with another key -> show encryption
            // menu to let user choose a key
            DFLOG( "Mmc encrypted, encryption key not set -> enc menu" );
            AppUi()->ActivateLocalViewL( TUid::Uid( EDevEncUiEncrViewId ) );
            }
        }
    else
        {
        DFLOG2( "Mmc busy, state = %d, no action", aMem->State() );
        }
    // Depending on the user's selection, the memory may be in the same
    // state as before or in a new state. This call will let all observers
    // know what state the memory ended up in.
    aMem->UpdateMemoryInfo();

    }

// --------------------------------------------------------------------------
//  Called by the framework when the application status pane
//  size is changed.  Passes the new client rectangle to the container.
// --------------------------------------------------------------------------
void CDevEncUiMainView::HandleStatusPaneSizeChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// End of File
