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
* Description:  Implementation of CDevEncUiAppUi.
*
*/

// INCLUDE FILES
#include <avkon.hrh>
#include <centralrepository.h>
#include <devencui.rsg>
#include <csxhelp/devenc.hlp.hrh>
#include <e32event.h>
#include <hlplch.h> // Help launcher
#include <StringLoader.h> // TBD: Needed?
#include <w32std.h>
#include <DevEncExternalCRKeys.h>

#include "DevEncLog.h"
#include "DevEnc.hrh"
#include "DevEncUi.pan"
#include "DevEncUiAppui.h"
#include "DevEncUiEncrView.h"
#include "DevEncUiEncryptionOperator.h"
#include "DevEncUiDecrView.h"

#include "DevEncUiMainView.h"
#include "DevEncUiMemoryEntity.h"
#include "DevEncUids.hrh"

#include <AknGlobalNote.h>

//Local definitions
const TInt KDevEncUiConstructionDelay( 1000000 );

// ============================ MEMBER FUNCTIONS ===============================

// --------------------------------------------------------------------------
// CDevEncUiAppUi::ConstructL()
// Symbian 2nd phase constructor can leave.
// --------------------------------------------------------------------------
//
void CDevEncUiAppUi::ConstructL()
    {
    // Initialise app UI with standard value.
    BaseConstructL( EAknEnableSkin | EAknSingleClickCompatible );

    DFLOG( ">>CDevEncUiAppUi::ConstructL" );

    User::LeaveIfError( iFs.Connect() );

    DFLOG( "CDevEncUiAppUi::ConstructL => Create drive path" );
    // Create private folder. TBD: Needed?
    TInt error = iFs.CreatePrivatePath( EDriveC );
    if ( error == KErrAlreadyExists )
        {
        error = KErrNone;
        }
    DFLOG( "CDevEncUiAppUi::ConstructL => Leave ?" );
    User::LeaveIfError( error );
    DFLOG( "CDevEncUiAppUi::ConstructL => No, constructing CR" );

    // Central repository settings
    iCrSettings = CRepository::NewL( TUid::Uid( KCRDevEncUiSettings ) );
    DFLOG( "CDevEncUiAppUi::ConstructL => Adding observer" );

    // Create MMC observer
    iObserver = CMmcObserver::NewL( this, &iFs );
    iObserver->StartObserver();

    // Call the MMC callback function once to get the initial card status
    MMCStatusChangedL();

    DFLOG( "CDevEncUiAppUi::ConstructL => constructing mass memory" );
    
    CDevEncUiMemoryEntity* massMemory = CDevEncUiMemoryEntity::NewLC( iEikonEnv, 
    		                                                            EPhoneMemory );
    
    DFLOG( "CDevEncUiAppUi::ConstructL => adding to mem entities" );
    iMemEntities.AppendL( massMemory );
    CleanupStack::Pop( massMemory );
    
    DFLOG( "CDevEncUiAppUi::ConstructL => constructing memory card" );
    
    CDevEncUiMemoryEntity* memoryCard = CDevEncUiMemoryEntity::NewLC( iEikonEnv,
        		                                                        EMemoryCard );
    
    DFLOG( "CDevEncUiAppUi::ConstructL => adding to mem entities" );
    iMemEntities.AppendL( memoryCard );
    CleanupStack::Pop( memoryCard );

    DFLOG( "CDevEncUiAppUi::ConstructL => constructing phone memory" );

    CDevEncUiMemoryEntity* phoneMemory = CDevEncUiMemoryEntity::NewLC( iEikonEnv,
                                                                       EPrimaryPhoneMemory );
    
    DFLOG( "CDevEncUiAppUi::ConstructL => adding to mem entities" );
    iMemEntities.AppendL( phoneMemory );
    CleanupStack::Pop( phoneMemory );

    DFLOG( "CDevEncUiAppUi::ConstructL() => Create enc operator" );

    // Create encryption operator
    iEncOperator = CDevEncUiEncryptionOperator::NewL( *iEikonEnv,
                                                      *this,
                                                      iCrSettings );

    DFLOG( "CDevEncUiAppUi::ConstructL() => Create views" );

    // Create views
    iMainView = CDevEncUiMainView::NewL( iMemEntities,
                                         iCrSettings,
                                         iMmcStatus );

    AddViewL( iMainView ); // transfer ownership

    iEncryptionView = CDevEncUiEncrView::NewL( iMemEntities );
    AddViewL( iEncryptionView ); // transfer ownership

    iDecryptionView = CDevEncUiDecrView::NewL( iMemEntities );
    AddViewL( iDecryptionView ); // transfer ownership

    SetDefaultViewL( *iMainView );
    
    DFLOG( "ConstructL 6" );

    iConstructionOnGoing = ETrue;
    
    iTimer = CDevEncUiTimer::NewL( this );
    iTimer->After( KDevEncUiConstructionDelay );

    DFLOG( "<<CDevEncUiAppUi::ConstructL" );
    }

// --------------------------------------------------------------------------
// CDevEncUiAppUi::CDevEncUiAppUi()
// C++ default constructor can NOT contain any code, that might leave.
// --------------------------------------------------------------------------
//
CDevEncUiAppUi::CDevEncUiAppUi()
    {
    // No implementation required
    }

// --------------------------------------------------------------------------
// CDevEncUiAppUi::~CDevEncUiAppUi()
// Destructor.
// --------------------------------------------------------------------------
//
CDevEncUiAppUi::~CDevEncUiAppUi()
    {
    DFLOG( ">>CDevEncUiAppUi::~CDevEncUiAppUi" );
    delete iObserver;
    iFs.Close();
    delete iEncOperator;
    if (iMemEntities.Count())
        iMemEntities[ EMemoryCard ]->RemoveObserver( this );
    for ( TInt i = 0; i < iMemEntities.Count(); i++ )
        {
        delete iMemEntities[i];
        }
    iMemEntities.Close();
    delete iCrSettings;
    delete iTimer;
    }

// --------------------------------------------------------------------------
// CDevEncUiAppUi::HandleCommandL()
// Takes care of command handling.
// --------------------------------------------------------------------------
//
void CDevEncUiAppUi::HandleCommandL( TInt aCommand )
    {
    DFLOG( ">>CDevEncUiAppUi::HandleCommandL" );

    switch( aCommand )
        {
        // These commands are specific to the memory card
        case EDevEncUiCommandEncryptWithoutSavingKey: // fall through
        case EDevEncUiCommandEncryptAndSaveKey:
        case EDevEncUiCommandEncryptWithRestoredKey:
            {
            // Pass the MMC memory entity to the operator, along with the command
            iEncOperator->HandleMmcEncryptRequestL( iMemEntities[ EMemoryCard ],
                                                    aCommand );
            break;
            }

        case EDevEncUiCommandDecrypt: // fall through
        case EDevEncUiCommandDecryptAndTurnOffEncryption:
            {
            // Pass the MMC memory entity to the operator, along with the command
            iEncOperator->HandleMmcDecryptRequestL( iMemEntities[ EMemoryCard ],
                                                    aCommand );
            break;
            }

        case EDevEncUiCommandHelp:
            {
            DisplayHelpL();
            break;
            }

        case EAknSoftkeyBack:
            {
            // Get the current view
            TVwsViewId wsViewId;
            TInt err = GetActiveViewId( wsViewId );
            if ( ( err != KErrNone ) ||
                 ( wsViewId.iAppUid.iUid != KDevEncUiUid ) )
                {
                User::Leave( err );
                }

            // Go to the main view from the encryption and decryption views
            if ( wsViewId.iViewUid.iUid == EDevEncUiEncrViewId ||
                 wsViewId.iViewUid.iUid == EDevEncUiDecrViewId )
                {
                ActivateLocalViewL( TUid::Uid( EDevEncUiMainViewId ) );
                }
            if ( wsViewId.iViewUid.iUid == EDevEncUiMainViewId )
            	{
            	Exit();
            	}
            break;
            }

        case EEikCmdExit: // fall through
        case EAknSoftkeyExit:
            {
            Exit();
            break;
            }

        default:
            Panic( EDevEncUi );
            break;
        }
    }

// --------------------------------------------------------------------------
// Called by the framework when the app is moved to or from foreground.
//
// --------------------------------------------------------------------------
//
void CDevEncUiAppUi::HandleForegroundEventL( TBool aForeground )
    {
    DFLOG( "CDevEncUiAppUi::HandleForegroundEventL" );
    DFLOG2( "CDevEncUiAppUi::HandleForegroundEventL => aForeground = %d ", aForeground );
    CAknAppUi::HandleForegroundEventL( aForeground );
    
    iForeground = aForeground;
    
    if( iConstructionOnGoing )
        {
        DFLOG( "CDevEncUiAppUi::HandleForegroundEventL => Construction on going" );
        if( iTimer )
        	{
        	iTimer->Cancel();
            }
        iTimer->After( KDevEncUiConstructionDelay );
        }
    }

// --------------------------------------------------------------------------
// Called by the framework before the options menu is shown.
// Sets the correct menu items.
// --------------------------------------------------------------------------
//
void CDevEncUiAppUi::DynInitMenuPaneL( TInt /*aResourceId*/,
                                       CEikMenuPane* /*aMenuPane*/ )
    {
    }

// --------------------------------------------------------------------------
// Called by the framework before the context-sensitive help is shown.
// Sets the correct menu items.
// --------------------------------------------------------------------------
//
//CArrayFix<TCoeHelpContext>* CDevEncUiAppUi::HelpContextL()
//    {
//    TCoeHelpContext& aContext
//    }

// --------------------------------------------------------------------------
// Called by the framework before the context-sensitive help is shown.
// Sets the correct menu items.
// --------------------------------------------------------------------------
void CDevEncUiAppUi::DisplayHelpL()
    {
    CArrayFix<TCoeHelpContext>* contexts = 
        new ( ELeave ) CArrayFixFlat<TCoeHelpContext>( 1 );
    CleanupStack::PushL( contexts );
    TCoeHelpContext context;
    context.iMajor = TUid::Uid( KDevEncUiUid );

    // Get the currently active view
    TVwsViewId wsViewId;
    TInt err = GetActiveViewId( wsViewId );
    if ( ( err != KErrNone ) ||
         ( wsViewId.iAppUid.iUid != KDevEncUiUid ) )
        {
        User::Leave( err );
        }
    
    // Set the help context for the current view
    switch ( wsViewId.iViewUid.iUid )
        {
        case EDevEncUiEncrViewId:
            {
            context.iContext = KES_HLP_ENCRYPTION_ENCRYPT;
            }
            break;
        case EDevEncUiDecrViewId:
            {
            context.iContext = KES_HLP_ENCRYPTION_DECRYPT;
            }
            break;
        default: // Main view
            {
            context.iContext = KES_HLP_ENCRYPTION_MAIN;
            }
            break;
        }
 
    contexts->AppendL( context );
    CleanupStack::Pop( contexts ); 
    HlpLauncher::LaunchHelpApplicationL( iCoeEnv->WsSession(), contexts );              
    }

// --------------------------------------------------------------------------
// Called by CDevEncMmcObserver when MMC status changes.
// 
// --------------------------------------------------------------------------
void CDevEncUiAppUi::MMCStatusChangedL()
    {
    DFLOG( "CDevEncUiAppUi::MMCStatusChangedL" );
    TVolumeInfo volInfo;

    TInt err = iFs.Volume( volInfo, /*EDriveE*/EDriveF );
    switch ( err )
        {
        case KErrNone:
            {
            // Readable MMC inserted
            DFLOG( "MMC inserted" );
            iMmcStatus = EMmcOk;
            break;
            }

        case KErrNotReady:
            {
            // MMC ejected
            DFLOG( "MMC ejected" );
            iMmcStatus = EMmcNotPresent;
            break;
            }

        case KErrCorrupt:
            {
            // Corrupt or unformatted MMC, or wrong key
            DFLOG( "MMC corrupt, unformatted or encrypted with other key" );
            iMmcStatus = EMmcNotReadable;
            break;
            }

        default:
            {
            DFLOG2( "RFs::Volume returned error %d", err );
            break;
            }
        } // switch
    }

// --------------------------------------------------------------------------
// Called by memory entity when memory status changes.
// 
// --------------------------------------------------------------------------
void CDevEncUiAppUi::UpdateInfo( TDevEncUiMemoryType aType,
                                 TUint aState,
                                 TUint /*aProgress*/ )
    {
    DFLOG( "CDevEncUiAppUi::UpdateInfo" );
    TInt error( KErrNone );
    TRAP( error, DoUpdateInfoL( aType, aState ) );
    DFLOG2( "CDevEncUiAppUi::DoUpdateInfo returned %d", error );
    }

// --------------------------------------------------------------------------
// Helper function called by UpdateInfo when memory status changes.
// 
// --------------------------------------------------------------------------
void CDevEncUiAppUi::DoUpdateInfoL( TDevEncUiMemoryType aType,
                                    TUint aState )
    {
    DFLOG( ">>CDevEncUiAppUi::DoUpdateInfoL" );
    if ( aType != EMemoryCard )
        {
        return;
        }

    // Prevent re-entry
    if ( iStatusUpdateOngoing )
        {
        return;
        }
    iStatusUpdateOngoing = ETrue;

    DFLOG3( "CDevEncUiAppUi::DoUpdateInfoL, type %d, state %d",
            aType, aState );
    DFLOG2( "Mmc state %d", iMmcStatus );
    DFLOG2( "Enc state %d", iMmcEncState );
    
    // Check encryption key status
    TInt mmcKeyInDriver( 0 );
    iCrSettings->Get( KDevEncUserSettingMemoryCard, mmcKeyInDriver );
    
    if ( ( iMmcStatus == EMmcOk ) && 
         ( iMmcEncState == EUnmounted ) &&
         ( aState == EDecrypted ) )
        {
        // A readable memory card has just been inserted
    
        if ( mmcKeyInDriver )
            {
            // We have a valid encryption key, but the newly inserted
            // card is decrypted. Ask if the user wants to encrypt.
            // (see UI spec 2.13)
            iEncOperator->SuggestMmcEncryptionL(
                                           iMemEntities[ EMemoryCard ] );
            }
        }

    if ( ( iMmcStatus == EMmcNotReadable ) && 
         ( iMmcEncState == EUnmounted ) &&
         ( aState == ECorrupted ) )
        {
        // An unreadable memory card has just been inserted
        
        if ( mmcKeyInDriver )
            {
            // We seem to have the wrong encryption key in the driver.
            // Inform the user that the card cannot be used.
            // (see UI spec 2.16)
            iEncOperator->ErrorNoteL(
                            R_DEVENCUI_TEXT_NOTE_INSERTON_UNUSEDMEMORYCARD );
            }
        else
            {
            DFLOG( "CDevEncUiAppUi::DoUpdateInfoL => SuggestMmcDecryption" );
            // We have no encryption key in the driver.
            // Ask if the user wants to import a key and decrypt.
            // (see UI spec 2.17)
            iEncOperator->SuggestMmcDecryptionL(
                                       iMemEntities[ EMemoryCard ] );
            }
        }

    iMmcEncState = aState;
    iStatusUpdateOngoing = EFalse;
    DFLOG( "<<CDevEncUiAppUi::DoUpdateInfoL" );
    }

// --------------------------------------------------------------------------
// Called by the CDevEncUiTimer when the event is generated.
// 
// --------------------------------------------------------------------------
void CDevEncUiAppUi::Timeout()
	{
	DFLOG( "CDevEncUiAppUi::Timeout()" );	
	
	if( iForeground )
		{
		DFLOG( "CDevEncUiAppUi::Timeout() => Construction completed" );
		//the construction is completed now
		iConstructionOnGoing = EFalse;
		// This class needs to observe memory card status changes.
        // (This call results in a call to UpdateInfo, hence we need to be fully
        // constructed before this.)
	    TRAP_IGNORE(iMemEntities[ EMemoryCard ]->AddObserverL( this ));
	    
		}
	}

// -----------------------------------------------------------------------------
// CDevEncUiAppUi::ProcessCommandParametersL
// From CEikAppUi
// -----------------------------------------------------------------------------
//
TBool CDevEncUiAppUi::ProcessCommandParametersL(
    TApaCommand /*aCommand*/,
    TFileName& /*aDocumentName*/,
    const TDesC8& /*aTail*/ )
    {
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CDevEncUiAppUi::OpenFileL
// -----------------------------------------------------------------------------
//
void CDevEncUiAppUi::OpenFileL(const TDesC& /*aFilename*/)
    {
    DFLOG("CDevEncUiAppUi::OpenFileL");
    }

// -----------------------------------------------------------------------------
// CDevEncUiAppUi::OpenFileL()
// -----------------------------------------------------------------------------
//
void CDevEncUiAppUi::OpenFileL(RFile& aFile)
    {
    _LIT(KTargetDir,"c:\\data\\others\\");
    TBuf16<100> destinationPath;
    destinationPath.Copy( KTargetDir );
    TBuf16<250> sourcePath;
    aFile.FullName( sourcePath );
    
    // if the paths are the same, the copy operation will not be executed
    if ( sourcePath.Left( destinationPath.Length() ).CompareF( destinationPath ) )
    	{
        // copy the file to Others folder
        TRAPD(err, CopyL( sourcePath, destinationPath ));
    
        if( err == KErrNone )
            {
            iEncOperator->InfoNoteL( R_DEVENCUI_TEXT_NOTE_SAVEINFO, ETrue );
            ActivateLocalViewL( TUid::Uid( EDevEncUiMainViewId ) );
            }
        else
    	    {
            Exit();
    	    }
    	}
    else
    	{
    	ActivateLocalViewL( TUid::Uid( EDevEncUiMainViewId ) );
    	}
	}

// -----------------------------------------------------------------------------
// CDevEncUiAppUi::Copy()
// -----------------------------------------------------------------------------
//
void CDevEncUiAppUi::CopyL(const TDesC &anOld, const TDesC &aNew)
	{   
	CFileMan* fileMan=CFileMan::NewL( iFs );
	CleanupStack::PushL( fileMan );
 
	User::LeaveIfError(fileMan->Copy( anOld, aNew ));
 
	CleanupStack::PopAndDestroy( fileMan );
	}

// End of File
