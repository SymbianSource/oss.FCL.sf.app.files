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
* Description:  Implementation of CDevEncUiMainViewContainer.
*
*/


// INCLUDE FILES

// Class includes
#include "DevEncUiMainViewContainer.h"

//System includes
#include <centralrepository.h>
#include <coemain.h>
#include <eikenv.h>
#include <DevEncEngineConstants.h>
#include <StringLoader.h>
#include <devencui.rsg>

//User includes
#include "DevEncLog.h"
#include "DevEncUi.pan"
#include "DevEncUiAppui.h"
#include "DevEncUiDocument.h"
#include "DevEncUiMainView.h"
#include "DevEncUiSettingItemList.h"
#include "DevEncUiMemoryEntity.h"

// Local definitions
const TInt KPercent100( 100 );

// ================= MEMBER FUNCTIONS =======================

// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::NewLC()
// --------------------------------------------------------------------------
//
CDevEncUiMainViewContainer* CDevEncUiMainViewContainer::NewLC(
                                const TRect& aRect,
                                RArray<CDevEncUiMemoryEntity*>& aMemEntities,
                                MContainerEventCallback& aCallback,
                                CRepository*& aCrSettings,
                                TInt& aMmcStatus )
	{
    CDevEncUiMainViewContainer* self =
        new ( ELeave ) CDevEncUiMainViewContainer( aMemEntities,
												   aCallback,
                                                   aCrSettings,
                                                   aMmcStatus );

    CleanupStack::PushL( self );
    self->ConstructL(aRect);
    return self;
	}

// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::NewL()
// --------------------------------------------------------------------------
//
CDevEncUiMainViewContainer* CDevEncUiMainViewContainer::NewL(
                                const TRect& aRect,
                                RArray<CDevEncUiMemoryEntity*>& aMemEntities,
                                MContainerEventCallback& aCallback,
                                CRepository*& aCrSettings,
                                TInt& aMmcStatus )
	{
    CDevEncUiMainViewContainer* self = NewLC( aRect,
											  aMemEntities,
                                              aCallback,
                                              aCrSettings,
                                              aMmcStatus );
    CleanupStack::Pop( self );
    return self;
	}


// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::ConstructL()
// --------------------------------------------------------------------------
//
void CDevEncUiMainViewContainer::ConstructL(const TRect& aRect)
	{
    // Create a window for this application view
    CreateWindowL();
    // TBD: Give the list some settings
    iSettingItemList = new ( ELeave ) CDevEncUiSettingItemList();
    iSettingItemList->SetMopParent( this );
    iSettingItemList->ConstructFromResourceL( R_DEVENCUI_SETTING_ITEM_LIST );
    SetRect( aRect );
    ActivateL();
 	}


// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::CDevEncUiMainViewContainer()
// --------------------------------------------------------------------------
//
CDevEncUiMainViewContainer::CDevEncUiMainViewContainer(
								RArray<CDevEncUiMemoryEntity*>& aMemEntities,
                                MContainerEventCallback& aCallback,
                                CRepository*& aCrSettings,
                                TInt& aMmcStatus )
	: iMemEntities( aMemEntities ),
	  iCallback( aCallback ),
      iCrSettings( aCrSettings ),
      iMmcStatus( aMmcStatus )
	{
    // No implementation required
	}

// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::CDevEncUiMainViewContainer()
// --------------------------------------------------------------------------
//
CDevEncUiMainViewContainer::~CDevEncUiMainViewContainer()
	{
    delete iSettingItemList;
	}

// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::CountComponentControls()
// --------------------------------------------------------------------------
//
TInt CDevEncUiMainViewContainer::CountComponentControls() const
	{
	return 1;
	}

// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::ComponentControl()
// --------------------------------------------------------------------------
//
CCoeControl* CDevEncUiMainViewContainer::ComponentControl( TInt aIndex ) const
	{
	CCoeControl* returnedItem( NULL );
	switch (aIndex)
		{
		case 0:
			returnedItem = iSettingItemList;
			break;
		default:
			break;
		}
	return returnedItem;
	}

// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::SizeChanged()
// --------------------------------------------------------------------------
//
void CDevEncUiMainViewContainer::SizeChanged()
    {
    if ( iSettingItemList )
        {
        TRect mainPaneRect; 
        AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, 
                                           mainPaneRect );        
        iSettingItemList->SetRect( mainPaneRect );
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::HandleResourceChange()
// --------------------------------------------------------------------------
//
void CDevEncUiMainViewContainer::HandleResourceChange( TInt aType )
    {    
    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        if ( iSettingItemList )
            {
            TRect mainPaneRect; 
            AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, 
                                               mainPaneRect );
            iSettingItemList->SetRect( mainPaneRect ); 
            iSettingItemList->HandleResourceChange( aType );
            }
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::OfferKeyEventL()
// --------------------------------------------------------------------------
//
TKeyResponse CDevEncUiMainViewContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                                          TEventCode aType )
    {
    if ( aKeyEvent.iCode == EKeyDevice3 /* OK key */ || 
    	 aKeyEvent.iCode == EKeyEnter /* Enter key*/ )
        {
        // User selected a list item. Proceed to toggle the status
        TInt selectedItem = iSettingItemList->SelectedItem();
        if ( selectedItem < 0 )
            {
            return EKeyWasConsumed;
            }
        TDevEncUiMemoryType type = static_cast<TDevEncUiMemoryType>( selectedItem );
        iCallback.EncryptionStatusChangeReq( type );
        return EKeyWasConsumed;
        }
    else
        {
        if ( iSettingItemList )
            return iSettingItemList->OfferKeyEventL( aKeyEvent, aType );
        else
            return EKeyWasNotConsumed;
        }
    }

 // --------------------------------------------------------------------------
 // CDevEncUiMainViewContainer::SelectedItem()
 // --------------------------------------------------------------------------
 //
TInt CDevEncUiMainViewContainer::SelectedItem()
    {
    if ( !iSettingItemList )
        {
        return -1;
        }
    return iSettingItemList->SelectedItem();
    }

// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::ChangeSelectedItemL()
// --------------------------------------------------------------------------
//
TBool CDevEncUiMainViewContainer::ChangeSelectedItemL()
    {
    TBool itemChanged( EFalse );
    if ( !iSettingItemList )
        {
        return itemChanged;
        }
    // Get index of currently selected item
    TInt selectedItem = iSettingItemList->SelectedItem();
    if ( selectedItem < 0 )
        {
        User::Leave( KErrNotFound );
        }

    // Fetch the current setting for later comparison
    TBool currentSetting( EFalse );
    iSettingItemList->ItemState( selectedItem, currentSetting );

    // Let the user change the value (synchronously during this call)
    iSettingItemList->ChangeSelectedItemL();

    // Compare the new setting for changes
    TBool newSetting( EFalse );
    iSettingItemList->ItemState( selectedItem, newSetting );

    // Notify our callback observer if the setting was changed. This always
    // eventually results in a call to UpdateInfo, which means that if the
    // user decides to interrupt the operation, the values in the setting
    // item list are updated accordingly.
    if ( newSetting != currentSetting )
        {
        itemChanged = ETrue;
        }
    return itemChanged;
    }

// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::UpdateInfo()
// --------------------------------------------------------------------------
//
void CDevEncUiMainViewContainer::UpdateInfo( TDevEncUiMemoryType aType,
                                             TUint aState,
                                             TUint aProgress /* = 0*/ )
    {
    DFLOG( ">>CDevEncUiMainViewContainer::UpdateInfoL" );
    TInt err( KErrNone );
    TRAP( err, DoUpdateInfoL( aType, aState, aProgress ) );
    if ( err )
        {
        DFLOG2( "CDevEncUiMainViewContainer::UpdateInfoL => error is %d", err );
        // TBI: Log the error
        }
    DFLOG( "<<CDevEncUiMainViewContainer::UpdateInfoL" );   
    }

// --------------------------------------------------------------------------
// CDevEncUiMainViewContainer::DoUpdateInfoL()
// --------------------------------------------------------------------------
//
void CDevEncUiMainViewContainer::DoUpdateInfoL( TDevEncUiMemoryType aType,
                                                TUint aState,
                                                TUint aProgress /*= 0*/ )
    {
    DFLOG( ">>CDevEncUiMainViewContainer::DoUpdateInfoL" );
    DFLOG2( "CDevEncUiMainViewContainer::DoUpdateInfoL, type %d", aType );
    DFLOG3( "New state %d, new progress %d", aState, aProgress );

    if ( aType > EPrimaryPhoneMemory )
        {
        // Unknown memory type
        User::Leave( KErrUnknown );
        }
    if ( aState > ECorrupted )
        {
        // Unknown state
        User::Leave( KErrCorrupt );
        }
    if ( aProgress > KPercent100 )
        {
        // Invalid progress value
        User::Leave( KErrOverflow );
        }
    
    DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => Passed first area of error ");
    
    HBufC* stateText( NULL );
   
    switch ( aState )
        {
    	case EDecrypted:
    		{
    		if ( ( aType == EMemoryCard ) && ( iMmcStatus == EMmcOk ) )
    			{
                DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => Memory card OK "); 
                // The Mmc is readable and decrypted, but there may be a key
                // in the driver. In that case, we need to show "On" even
                // though the card itself is not encrypted.
                TBool mmcEncKeyInDriver( EFalse );
                User::LeaveIfError(
                    iCrSettings->Get( KDevEncUserSettingMemoryCard,
                                      mmcEncKeyInDriver ) );
                if ( mmcEncKeyInDriver )
                    {
                    DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => MMCKey in driver ");
                    // "On"
                    stateText = StringLoader::LoadLC(
                        R_DEVENCUI_TEXT_ENCRYPTION_ON,
                        iEikonEnv );
                    // Update the On/Off popup settings 
                    iSettingItemList->SetItemStateL( aType, ETrue );
                    }
                else
                    {
                    DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => MMCKey NOT in driver ");
                    // "Off"
                    stateText = StringLoader::LoadLC(
                        R_DEVENCUI_TEXT_ENCRYPTION_OFF,
                        iEikonEnv );
                    // Update the On/Off popup settings 
                    iSettingItemList->SetItemStateL( aType, EFalse );
                    }
                }
    		else if (aType == EMemoryCard)
    			{
    			stateText = StringLoader::LoadLC( R_DEVENCUI_TEXT_ENCRYPTION_OFF, iEikonEnv );
    			iSettingItemList->SetItemStateL( aType, EFalse );
    			}
            else
                {
                DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => Phone Memory or Mass Storage ");
                if (aType == EPhoneMemory) // Mass Storage
                	{
                	if ( iMemEntities.Count() > EPrimaryPhoneMemory )
                		{
                		TUint phoneMemState = iMemEntities[EPrimaryPhoneMemory]->State();
                		DFLOG2( "CDevEncUiMainViewContainer::DoUpdateInfoL, Now Mass Storage, need Phone Memory State: %d", phoneMemState);
                		if (phoneMemState == ECorrupted)
                			{
                			stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_CORRUPTED, iEikonEnv);
                			}
                		else
                			{
                            stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_ENCRYPTION_OFF, iEikonEnv);
                			}
                		}
                	else
                		{
                		DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL, Now Mass Storage, no Phone Memory");
                        stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_ENCRYPTION_OFF, iEikonEnv);
                		}
                	}
                else // aType == EPrimaryPhoneMemory, means Phone Memory
                	{
                	if ( iMemEntities.Count() > EPhoneMemory )
                		{
                		TUint massStoreState = iMemEntities[EPhoneMemory]->State();
                		DFLOG2( "CDevEncUiMainViewContainer::DoUpdateInfoL, Now Phone Memory, need Mass Storage State: %d", massStoreState);
                		if (massStoreState == ECorrupted)
                			{
                			stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_CORRUPTED, iEikonEnv);
                			}
                		else
                			{
                			stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_ENCRYPTION_OFF, iEikonEnv);
                			}
                		}
                	else
                		{
                		DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL, Now Phone Memory, no Mass Storage");
                		stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_ENCRYPTION_OFF, iEikonEnv);
                		}
                	}

                // Update the On/Off popup settings 
                iSettingItemList->SetItemStateL( EPhoneMemory, EFalse );
                }
            break;
            }
        case EEncrypting:
            {
            DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => EEncrypting ");
            stateText = StringLoader::LoadLC(
                R_DEVENCUI_TEXT_ENCRYPT_PROCESS,
                //R_DEVENCUI_TEXT_DECRYPT,
                aProgress,
                iEikonEnv );
            break;
            }
        case EEncrypted:
            {
            DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => Phone Memory or Mass Storage ");
            if (aType == EPhoneMemory) // Mass Storage
            	{
            	if ( iMemEntities.Count() > EPrimaryPhoneMemory )
            		{
            		TUint phoneMemState = iMemEntities[EPrimaryPhoneMemory]->State();
            		DFLOG2( "CDevEncUiMainViewContainer::DoUpdateInfoL, Now Mass Storage, need Phone Memory State: %d", phoneMemState);
            		if (phoneMemState == ECorrupted)
            			{
            			stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_CORRUPTED, iEikonEnv);
            			}
            		else if (phoneMemState == EDecrypted)
            			{
            			stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_ENCRYPTION_OFF, iEikonEnv);
            			}
            		else
            			{
            			stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_ENCRYPTION_ON, iEikonEnv);
            			}
            		}
            	else
            		{
            		DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL, Now Mass Storage, no Phone Memory");
            		stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_ENCRYPTION_ON, iEikonEnv);
            		}
            	}
            else  if (aType == EPrimaryPhoneMemory)// Phone Memory
            	{
            	if ( iMemEntities.Count() > EPhoneMemory )
            		{
            		TUint massStoreState = iMemEntities[EPhoneMemory]->State();
            		DFLOG2( "CDevEncUiMainViewContainer::DoUpdateInfoL, Now Phone Memory, need Mass Storage State: %d", massStoreState);
            		if (massStoreState == ECorrupted)
            			{
            			stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_CORRUPTED, iEikonEnv);
            			}
            		else if (massStoreState == EDecrypted)
            			{
            			stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_ENCRYPTION_OFF, iEikonEnv);
            			}
            		else
            			{
            			stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_ENCRYPTION_ON, iEikonEnv);
            			}
            		}
            	else
            		{
            		DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL, Now Phone Memory, no Mass Storage");
            		stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_ENCRYPTION_ON, iEikonEnv);
            		}
            	}
            else // Memory Card
            	{
            	stateText = StringLoader::LoadLC(R_DEVENCUI_TEXT_ENCRYPTION_ON, iEikonEnv);
            	}
            // Update the On/Off popup settings 
            if ( aType != EPrimaryPhoneMemory )
            	{
            	iSettingItemList->SetItemStateL( aType, ETrue );
            	}
            else
            	{
            	iSettingItemList->SetItemStateL( EPhoneMemory, ETrue );
            	}
            break;
            }
        case EDecrypting:
            {
                DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => EDecrypting ");
            stateText = StringLoader::LoadLC(
                R_DEVENCUI_TEXT_DECRYPT_PROCESS,
                aProgress,
                iEikonEnv );
            break;
            }
        case ECorrupted: // fall through
        case EUnmounted:
        default:
            {
            if ( aType == EMemoryCard )
                {
                    DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => MMC Corrupted ");
                // Mmc is a) not inserted, b) corrupted or c) encrypted with
                // another key. Check the user setting, and show "On" if any
                // key is in the driver.
                TBool mmcEncKeyInDriver( EFalse );
                User::LeaveIfError(
                    iCrSettings->Get( KDevEncUserSettingMemoryCard,
                                      mmcEncKeyInDriver ) );
                if ( mmcEncKeyInDriver )
                    {
                        DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => MMC On ");
                    // "On"
                    stateText = StringLoader::LoadLC(
                        R_DEVENCUI_TEXT_ENCRYPTION_ON,
                        iEikonEnv );
                    // Update the On/Off popup settings 
                    iSettingItemList->SetItemStateL( aType, ETrue );
                    }
                else
                    {
                        DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => MMC Off ");
                    // "Off"
                    stateText = StringLoader::LoadLC(
                        R_DEVENCUI_TEXT_ENCRYPTION_OFF,
                        iEikonEnv );
                    // Update the On/Off popup settings 
                    iSettingItemList->SetItemStateL( aType, EFalse );
                    }
                }
            else
                {
                    DFLOG( "CDevEncUiMainViewContainer::DoUpdateInfoL => NON OF THE PREVIOUS");
                stateText = StringLoader::LoadLC(
                    R_DEVENCUI_TEXT_CORRUPTED,
                    iEikonEnv );
                }
            break;
            }
        } // End of switch

    if (aType != EPrimaryPhoneMemory)
    	{
    	iSettingItemList->SetItemTextL( aType, *stateText );
    	}
    else
    	{
    	iSettingItemList->SetItemTextL( EPhoneMemory, *stateText );
    	}
    CleanupStack::PopAndDestroy( stateText );

    DFLOG( ">>CDevEncUiMainViewContainer::DoUpdateInfoL" );
    }


//End of File
