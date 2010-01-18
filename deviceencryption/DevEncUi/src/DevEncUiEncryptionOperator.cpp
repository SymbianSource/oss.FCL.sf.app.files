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
* Description:  Implementation of CDevEncUiEncryptionOperator.
*
*/

// INCLUDE FILES
#include <DevEncExternalCRKeys.h>
// Class includes
#include "DevEncLog.h"
#include "DevEncUids.hrh"
#include "DevEncUiEncryptionOperator.h"

#include "DevEncUiFileManager.h"

//System includes
#include <aknmessagequerydialog.h>
#include <aknViewAppUi.h>
#include <AknWaitNoteWrapper.h>
#include <aknnotewrappers.h> // for CAknErrorNote
#include <bautils.h> // For BAFL, localized resource files
#include <centralrepository.h>
#include <coemain.h>
#include <data_caging_path_literals.hrh> // for resource files dir
#include <DevEncUi.rsg>
#include <eikenv.h>
#include <e32property.h>
#include <etelmm.h>
#include <f32file.h>
#include <gsfwviewuids.h> // For general settings view UIDs (autolock)
#include <hwrmpowerstatesdkpskeys.h> // For power state
#include <SecUiSecuritySettings.h> // for autolock
#include <SettingsInternalCRKeys.h>
#include <StringLoader.h>
#include <utf.h> // For character conversion
#include <TerminalControl3rdPartyAPI.h>

#include <DevEncKeyUtils.h>

//User includes
#include "DevEncLog.h"
#include "DevEnc.hrh"
#include "DevEncUiCreateKeyProcess.h"
#include "DevEncDef.h"
#include "DevEncUiMemoryEntity.h"
#include "DevEncUiSecretMinMaxCodeQuery.h"
//for KMaxAutolockPeriod = 20;
#include "DevEncSession.h"

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::~CDevEncUiEncryptionOperator
//
// --------------------------------------------------------------------------
//
CDevEncUiEncryptionOperator::~CDevEncUiEncryptionOperator()
    {
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::NewL
//
// --------------------------------------------------------------------------
//
CDevEncUiEncryptionOperator* CDevEncUiEncryptionOperator::NewL( CEikonEnv& aEikonEnv,
                                                                CAknViewAppUi& aAppUi,
                                                                CRepository*& aCrSettings )
    {
    CDevEncUiEncryptionOperator* self =
        CDevEncUiEncryptionOperator::NewLC( aEikonEnv,
                                            aAppUi,
                                            aCrSettings );
    CleanupStack::Pop( self );
    return self;
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::NewLC
//
// --------------------------------------------------------------------------
//
CDevEncUiEncryptionOperator* CDevEncUiEncryptionOperator::NewLC( CEikonEnv& aEikonEnv,
                                                                 CAknViewAppUi& aAppUi,
                                                                 CRepository*& aCrSettings )
    {
    CDevEncUiEncryptionOperator* self =
        new ( ELeave ) CDevEncUiEncryptionOperator( aEikonEnv,
                                                    aAppUi,
                                                    aCrSettings );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::CDevEncUiEncryptionOperator
//
// --------------------------------------------------------------------------
//
CDevEncUiEncryptionOperator::CDevEncUiEncryptionOperator( CEikonEnv& aEikonEnv,
                                                          CAknViewAppUi& aAppUi,
                                                          CRepository*& aCrSettings )
    : iEikEnv( aEikonEnv ),
    iAppUi( aAppUi ),
    iCrSettings( aCrSettings )
    {
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::ConstructL
//
// --------------------------------------------------------------------------
//
void CDevEncUiEncryptionOperator::ConstructL()
    {
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::HandlePhoneMemEncryptRequestL()
// --------------------------------------------------------------------------
//
void CDevEncUiEncryptionOperator::HandlePhoneMemEncryptRequestL( CDevEncUiMemoryEntity* aMem, CDevEncUiMemoryEntity* aPrimaryMem )
    {
    // Check the battery level first
    if ( ! CheckBatteryL() )
        {
        // The battery level is too low and no charger is connected.
        // Show a note to the user.
        ErrorNoteL( R_DEVENCUI_TEXT_BATTERY_LOW );
        User::Leave( KErrNotReady );
        }

    if ( (aMem->State() == EDecrypted) || (aPrimaryMem && (aPrimaryMem->State() == EDecrypted)) )
        {
        // Encryption is off -> encrypt
        if ( PrepareEncryptionL() )
            {
            if ( aMem->State() == EDecrypted )
            	{
            	DFLOG( "Starting Mass Storage encryption" );
            	aMem->StartEncryptionL();
            	}
            if( aPrimaryMem && (aPrimaryMem->State() == EDecrypted) )
            	{
            	DFLOG( "Starting phone memory encryption" );
            	aPrimaryMem->StartEncryptionL();
                }
            }
        else
            {
            DFLOG( "Encryption cancelled" );
            }
        }
    else if ( (aMem->State() == EEncrypted) || (aPrimaryMem && (aPrimaryMem->State() == EEncrypted)) )
        {
        if ( PrepareDecryptionL() )
            {
            if ( aMem->State() == EEncrypted )
            	{
            	DFLOG( "Starting Mass Storage decryption" );
            	aMem->StartDecryptionL();
            	}
            if( aPrimaryMem && (aPrimaryMem->State() == EEncrypted) )
                {
                DFLOG( "Starting phone memory decryption" );
                aPrimaryMem->StartDecryptionL();
                }
            }
        else
            {
            DFLOG( "Decryption cancelled" );
            }
        }
    else
        {
        DFLOG2( "Phone memory busy, state = %d, no action", aMem->State() );
        }

    // Depending on the user's selection, the memory may be in the same
    // state as before or in a new state. This call will let all observers
    // know what state the memory ended up in.
    aMem->UpdateMemoryInfo();
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::PrepareEncryptionL()
// --------------------------------------------------------------------------
//
TBool CDevEncUiEncryptionOperator::PrepareEncryptionL()
    {
    TBool result( EFalse );

    // Show the confirmation query about autolock
    CAknMessageQueryDialog* yesNoQuery = new ( ELeave ) CAknMessageQueryDialog;
    if ( ! yesNoQuery->ExecuteLD( R_DEVENCUI_ENCRYPT_CONFIRMATION_QUERY_DIALOG ) )
        {
        // User answered no. Do nothing
        DFLOG( "User cancelled after autolock warning. No action." );
        return result;
        }

    // Autolock must be on before encryption can be enabled.
    // Get the current autolock status and the current max autolock period
    TInt lockTime = 0;
    TBuf8<21> autoLockPeriodBuf;
    
    RTerminalControl terminalCtrl;
    TInt ctrlConnect = terminalCtrl.Connect();
    DFLOG2( "DEVENC: terminal control connected %d", ctrlConnect );
    
    RTerminalControl3rdPartySession session;
    TInt retValue = session.Open( terminalCtrl );
    DFLOG2( "DEVENC: terminal control session open %d", retValue );

    TInt err2 = session.GetDeviceLockParameter( RTerminalControl3rdPartySession::ETimeout, autoLockPeriodBuf );
    DFLOG2( "DEVENC: max value get returned value %d", err2 );
    
    TLex8 lexAutolock( autoLockPeriodBuf );          
    lexAutolock.Val( lockTime );
    DFLOG2( "lockTime = %d", lockTime );
    
    //Get current max autolock period
    TBuf8<21> oldMaxPeriodBuf;
    TInt maxperioderr = session.GetDeviceLockParameter( RTerminalControl3rdPartySession::EMaxTimeout, oldMaxPeriodBuf );
    DFLOG2( "DEVENC: max value set returned value %d", maxperioderr );

#ifndef __WINS__
    // Setting the autolock works only in target

    if ( lockTime == 0 )
        {
        // Autolock disabled. Go to the autolock settings
        DFLOG( "Autolock currently disabled, going to settings" );

        // Create the filename of the SecUi resource file
        _LIT( KFileDrive, "z:" );
        _LIT( KSecUiResourceFileName, "SecUi.rsc" );
        TFileName filename;
        filename += KFileDrive;
        filename += KDC_RESOURCE_FILES_DIR;
        filename += KSecUiResourceFileName;

        // Convert to a localized resource filename if possible
        RFs fs;
        User::LeaveIfError( fs.Connect() );
        BaflUtils::NearestLanguageFile( fs, filename );

        // Load the SecUi resource file
        TInt rfileOffset( 0 );
        TRAPD( err, rfileOffset = iEikEnv.AddResourceFileL( filename ) );
        if ( err )
            {
            DFLOG2( "Add resource file error %d", err );
            }
        User::LeaveIfError( err );
        
        // Set maximum autolock period
        
        TBuf8<21> maxPeriodBuf;
        maxPeriodBuf.AppendNum( KMaxAutolockPeriod );
        TInt err3 = session.SetDeviceLockParameter( RTerminalControl3rdPartySession::EMaxTimeout, maxPeriodBuf );
        DFLOG2( "DEVENC: max value set returned value %d", err3 );

        // Let the user set an autolock period
        CSecuritySettings* secSett = CSecuritySettings::NewL();
        CleanupStack::PushL( secSett );
        TInt newLockTime = secSett->ChangeAutoLockPeriodL( lockTime );
        CleanupStack::PopAndDestroy( secSett );

        iEikEnv.DeleteResourceFile( rfileOffset );
        
        if ( newLockTime == 0 )
            {
            // User cancelled or set autolock off -> do nothing
            DFLOG( "User selected not to enable autolock. No action." );
            
            //restore the old maximum period value
            TInt err3 = session.SetDeviceLockParameter( RTerminalControl3rdPartySession::EMaxTimeout, oldMaxPeriodBuf );
            DFLOG2( "DEVENC: max value set returned value %d", err3 );
            
            //the autolock period is now at KMaxAutolockPeriod( =20 ) so we have to set it as it was previously
            err3 = session.SetDeviceLockParameter( RTerminalControl3rdPartySession::ETimeout, autoLockPeriodBuf );
            DFLOG2( "DEVENC: max value set returned value %d", err3 );
            
            session.Close();
            terminalCtrl.Close();
            return result;
            }
        
        TBuf8<21> newLockTimeBuf;
        newLockTimeBuf.AppendNum( newLockTime );
        
        TInt setResult = session.SetDeviceLockParameter( RTerminalControl3rdPartySession::ETimeout, newLockTimeBuf );
        if ( setResult )
            {
            // TBI: Log the failure and leave with error
            DFLOG2( "ERROR: session.SetDeviceLockParameter( RTerminalControl3rdPartySession::ETimeout, .. ) returned %d", setResult );
            session.Close();
            terminalCtrl.Close();
            return result;
            }
        lockTime = newLockTime;
        }
    else
    	{    	
    	DFLOG( "Autolock already enabled. Set only the current maximum period" );
    	
    	//By setting the max autolock period the autolock period is set to max if it was greater than that
    	TBuf8<21> maxPeriodBuf;
        maxPeriodBuf.AppendNum( KMaxAutolockPeriod );
        TInt err3 = session.SetDeviceLockParameter( RTerminalControl3rdPartySession::EMaxTimeout, maxPeriodBuf );
        DFLOG2( "DEVENC: max value set returned value %d", err3 );
        }
#endif // __WINS__

    DFLOG2( "Autolock time is %d", lockTime );

    // Autolock is now enabled, so we can proceed with the encryption
    // Warn the user about the long encryption process
    if ( MessageQueryL( R_DEVENCUI_TEXT_NOTE_ENCRYPTING ) )
        {
        result = ETrue;
        DFLOG( "Starting encryption" );
        }
    else
        {
        DFLOG( "Encryption cancelled" );
        //the encryption was cancelled. Let's set the old autolock values
        //restore the old maximum period value
        TInt err3 = session.SetDeviceLockParameter( RTerminalControl3rdPartySession::EMaxTimeout, oldMaxPeriodBuf );
        DFLOG2( "DEVENC: max value set returned value %d", err3 );
        
        //the autolock period is now at KMaxAutolockPeriod( =20 ) so we have to set it as it was previously
        err3 = session.SetDeviceLockParameter( RTerminalControl3rdPartySession::ETimeout, autoLockPeriodBuf );
        DFLOG2( "DEVENC: max value set returned value %d", err3 );
        }
    
    session.Close();
    terminalCtrl.Close();
    
    return result;
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::PrepareDecryptionL()
// --------------------------------------------------------------------------
//
TBool CDevEncUiEncryptionOperator::PrepareDecryptionL()
    {
    // Warn the user about the long decryption process
    return MessageQueryL( R_DEVENCUI_TEXT_NOTE_PHONE_DECRYPTING );
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::MessageQueryL()
// --------------------------------------------------------------------------
//
TBool CDevEncUiEncryptionOperator::MessageQueryL( TInt aResourceId,
                                                  TInt aSoftKeysId )
    {
    TBool result( EFalse );

    // Show a message query with OK and Cancel buttons and the specified text
    CAknMessageQueryDialog* query =
        new ( ELeave ) CAknMessageQueryDialog;
    query->PrepareLC( R_DEVENCUI_ENCRYPT_CONFIRMATION_QUERY_DIALOG );
    query->ButtonGroupContainer().SetCommandSetL( aSoftKeysId );
    HBufC* prompt = StringLoader::LoadLC( aResourceId, &iEikEnv );
    query->SetMessageTextL( *prompt );
    CleanupStack::PopAndDestroy( prompt );
    if ( query->RunLD() )
        {
        result = ETrue;
        }
    return result;
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::QueryDialog()
// --------------------------------------------------------------------------
//
TBool CDevEncUiEncryptionOperator::QueryDialogL( TInt aResourceId,
                                                 TInt aSoftKeysId )
    {
    TBool result( EFalse );
    CAknQueryDialog* query = CAknQueryDialog::NewL();
    query->PrepareLC( R_DEVENCUI_CONFIRMATION_QUERY ); // CleanupStack OK
    HBufC* prompt = StringLoader::LoadLC( aResourceId, &iEikEnv );
    query->SetPromptL( *prompt );
    query->ButtonGroupContainer().SetCommandSetL( aSoftKeysId );
    
    CleanupStack::PopAndDestroy( prompt );
    if ( query->RunLD() )
        {
        result = ETrue;
        }
    return result;
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::DataQueryDialogL()
// --------------------------------------------------------------------------
//
TBool CDevEncUiEncryptionOperator::DataQueryDialogL( TDes& aResult,
                                                     TInt aPromptResourceId,
                                                     TInt aMaxLength )
    {
    // The descriptor contained the prompt text for the query. The prompt
    // text can also be defined in the resource structure of the query
    HBufC* prompt = StringLoader::LoadLC( aPromptResourceId, &iEikEnv );

    // create dialog instance
    CAknTextQueryDialog* dlg =
        new( ELeave ) CAknTextQueryDialog( aResult, *prompt );

    CleanupStack::PopAndDestroy( prompt );

    // Sets the maximum length of the text editor
    dlg->SetMaxLength( aMaxLength );

    // Prepares the dialog, constructing it from the specified resource
    dlg->PrepareLC( R_DEVENCUI_DATA_QUERY );

    // Launch the dialog
    return dlg->RunLD();
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::HandleMmcEncryptRequestL()
// --------------------------------------------------------------------------
//
void CDevEncUiEncryptionOperator::HandleMmcEncryptRequestL( CDevEncUiMemoryEntity* aMem,
                                                            TInt aCommand )
    {
    // Check the battery level first
    if ( ! CheckBatteryL() )
        {
        // The battery level is too low and no charger is connected.
        // Show a note to the user.
        ErrorNoteL( R_DEVENCUI_TEXT_BATTERY_LOW );
        User::Leave( KErrNotReady );
        }

    TBool proceed( EFalse );
    switch ( aCommand )
        {
        case EDevEncUiCommandEncryptWithRestoredKey:
            {
            DFLOG( "Encrypt with restored key selected" );
            proceed = LoadAndSetKeyL(); // No cleanup stack needed
            break;
            }

        case EDevEncUiCommandEncryptAndSaveKey:
            {
            DFLOG( "Encrypt and save key selected" );
            proceed = CreateAndSaveKeyL(); // No cleanup stack needed
            if( proceed )
                {
                InfoNoteL( R_DEVENCUI_TEXT_NOTE_SAVEINFO, ETrue );
                DFLOG( "Key saved succesfully" );
                }
            break;
            }

        case EDevEncUiCommandEncryptWithoutSavingKey:
            {
            DFLOG( "Encrypt without saving key selected" );
            // Create an encryption key and take it in use
            // The key will remain in the driver until the next key is set,
            // and it can not be retrieved.
            proceed = CreateKeyL(); // No cleanup stack needed
            break;
            }

        case EDevEncUiCommandEncryptWithCurrentKey:
            {
            DFLOG( "Encrypt with current key selected" );
            proceed = ETrue;
            }
            
        default:
            {
            DFLOG( "Invalid command" );
            break;
            }
        }

    if ( aMem->State() == EUnmounted )
        {
        if( !proceed )
        	{
            // The encryption operation was cancelled, just go back to encryption view
            DFLOG( "No Mmc present and operation was cancelled -> no encryption" );
            }
        else
        	{
        	// The encryption operation was succesfull, go back to main view
            iAppUi.ActivateLocalViewL( TUid::Uid( EDevEncUiMainViewId ) );
        	proceed = EFalse;
            DFLOG( "No Mmc present -> no encryption" );
            }
        }

    if ( aMem->State() == ECorrupted )
        {
        if( !proceed )
        	{
        	DFLOG( "Mmc probably enc with another key -> wait for remount" );
            }
        else
        	{
        	// Mmc was probably encrypted with another key. We have just set a
            // new key, so now we have to wait until the state changes and hope
            // that the card is readable. If it is, a note will be shown to the
            // user. (See UI spec 2.7)
            InfoNoteL( R_DEVENCUI_TEXT_NOTE_MEMORYCARD_ENCRYPTED, ETrue );
            iAppUi.ActivateLocalViewL( TUid::Uid( EDevEncUiMainViewId ) );
            proceed = EFalse;
            DFLOG( "Mmc probably enc with another key -> wait for remount" );
            RemountMmcL();
            }
        }
    
    if ( proceed )
        {
        TInt encrypt( 0 );
        
        // Check DM admin status
        TInt dmControlSetting( 0 );
        iCrSettings->Get( KDevEncUiDmControl, dmControlSetting );
        
        if( ( dmControlSetting & KDmControlsMemoryCard ) && ( aCommand == EDevEncUiCommandEncryptWithCurrentKey ) )
        	{
        	//if the DM admin is controlling the encryption, encrypt straight away
        	encrypt = ETrue;
        	}
        else
        	{
        	encrypt = MessageQueryL( R_DEVENCUI_TEXT_NOTE_ENCRYPTING );
            }
        
        // Warn the user about the long encryption process
        if ( encrypt )
            {
            // Start the encryption
            DFLOG( "Starting encryption" );
            aMem->StartEncryptionL();
            // Go back to main view
            iAppUi.ActivateLocalViewL( TUid::Uid( EDevEncUiMainViewId ) );
            }
        else
            {
            DFLOG( "Encryption cancelled" );
            // Stay in the Encryption view
            // Show a note to the user about that the new key is in use,
            // but the card is still decrypted (UI spec 2.3)
            InfoNoteL( R_DEVENCUI_TEXT_NOTE_ENCRYPTIONINTERRUPT, ETrue );
            iAppUi.ActivateLocalViewL( TUid::Uid( EDevEncUiMainViewId ) );
            }
        }

    // Depending on the user's selection, the memory may be in the same
    // state as before or in a new state. This call will let all observers
    // know what state the memory ended up in.
    aMem->UpdateMemoryInfo();
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::CreateKeyL()
// --------------------------------------------------------------------------
//
TBool CDevEncUiEncryptionOperator::CreateKeyL()
    {
    TBool proceed( EFalse );
    CDevEncKeyUtils* utils = new ( ELeave ) CDevEncKeyUtils;
    CleanupStack::PushL( utils );
//    User::LeaveIfError(utils->Connect());
    TRequestStatus status;
    utils->CreateSetKey( status, KEncryptionKeyLength );
    User::WaitForRequest( status );
//    utils->Close();
    CleanupStack::PopAndDestroy( utils );
    if ( ! status.Int() )
        {
        // Save the Mmc encryption key status
        User::LeaveIfError( iCrSettings->Set( KDevEncUserSettingMemoryCard,
                                              ETrue ) );
        proceed = ETrue;
        }
    return proceed;
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::CreateAndSaveKeyL()
// --------------------------------------------------------------------------
//
TBool CDevEncUiEncryptionOperator::CreateAndSaveKeyL()
    {
    TBool proceed( EFalse );
    
    TBool passwordOk( EFalse );
    TBool keyFileNameOk( EFalse );
    TBool keyCreated( EFalse );
    TBool keySaved( EFalse );
    CFileStore* file( NULL );
    HBufC* keyFileName = HBufC::NewLC( KMaxFilenameLength );
    
    // Get the key password from the user
    HBufC8* password = HBufC8::NewLC( KMaxPasswordLength );
    TPtr8 passwordPtr = password->Des();
    passwordOk = PasswordQueryL( R_DEVENCUI_TEXT_PASSPHRASE_PROTECT,
                                 passwordPtr );

    CDevEncUiFileManager* fm = new ( ELeave ) CDevEncUiFileManager;
    CleanupStack::PushL( fm );
    fm->ConstructL();

    HBufC8* pkcs5Key( NULL );

    if ( passwordOk )
        {
        DFLOG( "Password entered" );
        // The user entered the correct password

        // Construct file path
        _LIT( KDevEncKeyFileExtension, ".pk5");
        TBuf<KMaxFileName> filePath;
        User::LeaveIfError( fm->KeyFolder( filePath ) );
        TInt pathLen( filePath.Length() );

        // Get the key file name from the user
        TBool filenameEntered( EFalse );
        TBool cancelPressed( EFalse );
        TPtr16 keyFileNamePtr = keyFileName->Des();
        while ( ( ! cancelPressed ) && ( ! file ) )
            {
            filenameEntered = DataQueryDialogL( keyFileNamePtr,
                    R_DEVENCUI_TEXT_FILENAME,
                    KMaxFilenameLength );
            if ( !filenameEntered )
                {
                // User pressed cancel, stop here
                cancelPressed = ETrue;
                }
            else
                {
                // User entered a file name. Try to open a file store
                filePath.Append( keyFileNamePtr );
                filePath.Append( KDevEncKeyFileExtension );
                TInt openError = fm->OpenFileStore( filePath,
                                                    file );
                if ( openError )
                    {
                    DFLOG2( "Error opening file: %d", openError );

                    // Notify the user that something went wrong
                    if ( openError == KErrAlreadyExists )
                        {
                        ErrorNoteL( R_DEVENCUI_TEXT_FILE_EXISTS );
                        }
                    else
                        {
                        ErrorNoteL( R_DEVENCUI_TEXT_BAD_NAME );
                        }
                    
                    // Remove the file name from the path and try again
                    filePath.Delete( pathLen, filePath.Length() - pathLen );
                    }
                else
                    {
                    DFLOG( "File opened:" );
                    DFLOGBUF( filePath );
                    keyFileNameOk = ETrue;
                    CleanupStack::PushL( file );
                    }
                }
            } // end of while
        }
    else
        {
        DFLOG( "Password incorrect" );
        }
    
    if ( keyFileNameOk )
        {
        // Create the encryption key
        CDevEncKeyUtils* utils = new ( ELeave ) CDevEncKeyUtils;
        CleanupStack::PushL( utils );
//        User::LeaveIfError(utils->Connect());

        TRequestStatus status;
        utils->CreateSetKey( status, pkcs5Key, *password, KEncryptionKeyLength );
        User::WaitForRequest( status );
//        utils->Close();
        CleanupStack::PopAndDestroy( utils );
        
        if ( status.Int() )
            {
            DFLOG2( "Error creating key: %d", status.Int() );
            }
        else
            {
            keyCreated = ETrue;
            CleanupStack::PushL( pkcs5Key );
            // Save the Mmc encryption key status
            User::LeaveIfError( iCrSettings->Set( KDevEncUserSettingMemoryCard,
                                                  ETrue ) );
            }
        }

    if ( keyCreated )
        {
        // Save the key
        fm->SaveKeyL( file, *pkcs5Key );
        CleanupStack::PopAndDestroy( pkcs5Key );
        keySaved = ETrue;
        }

    if ( keySaved )
        {
        // Finally, start the encryption
        proceed = ETrue;
        }

    // Cleanup on demand
    if ( file )
        {
        CleanupStack::PopAndDestroy( file );
        }
    if ( fm )
        {
        CleanupStack::PopAndDestroy( fm );
        }
    if ( password )
        {
        CleanupStack::PopAndDestroy( password );
        }
    if ( keyFileName )
        {
        CleanupStack::PopAndDestroy( keyFileName );
        }
    
    return proceed;
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::LoadAndSetKeyL()
// --------------------------------------------------------------------------
//
TBool CDevEncUiEncryptionOperator::LoadAndSetKeyL()
    {
    TBool proceed( EFalse );

    TBool keySelected( EFalse );
    TBool passwordOk( EFalse );
    TBool askPassword( ETrue );
    HBufC8* pkcs5Key( NULL );
    TBuf8<KMaxPasswordLength> password8;

    // Get a list of existing keys in the private directory
    CDevEncUiFileManager* fm = new ( ELeave ) CDevEncUiFileManager;
    CleanupStack::PushL( fm );
    fm->ConstructL();
    CDir* dir( NULL );
    fm->GetKeyListL( dir );

    DFLOG2( "Found %d keys", dir->Count() );
    
    if ( dir->Count() <= 0 )
        {
        ErrorNoteL( R_DEVENCUI_TEXT_NO_KEYS_FOUND );
        User::Leave( KErrNotReady );
        }
    
    // Create a list for the query
    iListQueryItemArray = new ( ELeave ) CDesCArrayFlat( dir->Count() );
    for ( TInt i = 0; i < dir->Count(); i++ )
        {
        TEntry entry( (*dir)[i] );
        HBufC* tmpFileName = entry.iName.AllocLC();
        iListQueryItemArray->AppendL( entry.iName );
        CleanupStack::PopAndDestroy( tmpFileName );
        }

    // Let the user select which key to load
    TInt index( 0 ); // the index of the selected item
    CAknListQueryDialog* query =
        new ( ELeave ) CAknListQueryDialog( &index );
    query->PrepareLC( R_DEVENCUI_LIST_QUERY );
    query->ButtonGroupContainer().SetCommandSetL( R_AVKON_SOFTKEYS_OK_BACK );
    query->SetItemTextArray( iListQueryItemArray );
    // Keep ownership of the item array
    query->SetOwnershipType( ELbmDoesNotOwnItemArray );
    if ( query->RunLD() )
        {
        DFLOG2( "User selected key %d", index );
        DFLOGBUF( ( *dir )[index].iName );
        // Load the key file.

        // Construct file path
        TBuf<KMaxFileName> filePath;
        DFLOG("Start find key folder");
        User::LeaveIfError( fm->KeyFolder( filePath ) );
        DFLOG("Stop find key folder");
    
        TFileName filename;
        filename.Append( filePath );
        filename.Append( ( *dir )[index].iName );
        fm->LoadKeyL( filename,
                      pkcs5Key );
        CleanupStack::PushL( pkcs5Key );
        keySelected = ETrue;
        DFLOG( "Key loaded" );
        }
    else
        {
        // User cancelled the query
        DFLOG( "Key file query cancelled by user, stopping here" );
        }
    
    while ( askPassword )
    	{
        if ( keySelected )
            {
            // Get the key password from the user
            passwordOk = PasswordQueryL( R_DEVENCUI_TEXT_PASSPHRASE_ASK,
                                         password8 );
            }
        else
        	{
        	//The user pressed back. Do not ask the password
        	askPassword = EFalse;
        	}

        if ( passwordOk )
            {
            DFLOG( "Password ok, setting key" );
            CDevEncKeyUtils* utils = new ( ELeave ) CDevEncKeyUtils;
            CleanupStack::PushL( utils );
//            User::LeaveIfError(utils->Connect());

            TRequestStatus status;
            utils->SetKey( status, *pkcs5Key, password8 );
            User::WaitForRequest( status );
//            utils->Close();
            CleanupStack::PopAndDestroy( utils );
            if ( status.Int() )
                {
                const TInt KErrBadPassphrase( -11000 );
                DFLOG2( "Error taking key in use: %d", status.Int() );
                if ( status.Int() == KErrBadPassphrase )
                    {
                    DFLOG( "Invalid password" );
                    ErrorNoteL( R_DEVENCUI_TEXT_INVALID_PASSWORD );
                    }
                else
                    {
                    DFLOG2( "Key read error %d", status.Int() );
                    ErrorNoteL( R_DEVENCUI_TEXT_KEY_ERROR );
                    askPassword = EFalse;
                    }
                }
            else
                {
                DFLOG( "New key is now in use" );
                // Save the Mmc encryption key status
                User::LeaveIfError( iCrSettings->Set( KDevEncUserSettingMemoryCard,
                                                      ETrue ) );
                proceed = ETrue;
                askPassword = EFalse;
                }
            }
        else
            {
            askPassword = EFalse;
            }
        }
    // Cleanup on demand
    if ( pkcs5Key )
        {
        CleanupStack::PopAndDestroy( pkcs5Key );
        }
    if ( fm )
        {
        CleanupStack::PopAndDestroy( fm );
        }

    return proceed;
    }
    

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::PasswordQueryL()
// --------------------------------------------------------------------------
//
TBool CDevEncUiEncryptionOperator::PasswordQueryL( TInt aPromptResourceId,
                                                   TDes8& aResult )
    {
    TBool passwordOk( EFalse );
    TBuf<KMaxPasswordLength> password;
    TBuf8<KMaxPasswordLength> password8;
    CDevEncUiSecretMinMaxCodeQuery* query =
        new ( ELeave ) CDevEncUiSecretMinMaxCodeQuery( password,
                                                   KMinPasswordLength,
                                                   KMaxPasswordLength );

    HBufC* prompt = StringLoader::LoadLC( aPromptResourceId, &iEikEnv );
    query->SetPromptL( *prompt );
    CleanupStack::PopAndDestroy( prompt );

    // Prompt the user for the key password
    if ( ( query->ExecuteLD( R_DEVENCUI_CODE_QUERY ) ) )
        {
        // This indicates that the user filled in some password and
        // pressed OK.
        passwordOk = ETrue;
        }
    else
        {
        DFLOG( "Key password dialog cancelled" );
        }

    if ( passwordOk )
        {
        // Convert the password to UTF-8
        if ( CnvUtfConverter::ConvertFromUnicodeToUtf8( password8,
                                                        password ) )
            {
            DFLOG( "Password conversion failed" );
            passwordOk = EFalse;
            }
        else
            {
            aResult.Copy( password8 );
            }
        }
    
    return passwordOk;
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::HandleMmcDecryptRequestL()
// --------------------------------------------------------------------------
//
void CDevEncUiEncryptionOperator::HandleMmcDecryptRequestL( CDevEncUiMemoryEntity* aMem,
                                                            TInt aCommand )
    {
    // Check the battery level first
    if ( ! CheckBatteryL() )
        {
        // The battery level is too low and no charger is connected.
        // Show a note to the user.
        ErrorNoteL( R_DEVENCUI_TEXT_BATTERY_LOW );
        User::Leave( KErrNotReady );
        }

    TBool proceed( EFalse );
    switch ( aCommand )
        {
        case EDevEncUiCommandDecrypt:
            {
            DFLOG( "Decrypt selected" );
            // Warn the user about the long decryption process
            proceed = MessageQueryL( R_DEVENCUI_TEXT_NOTE_DECRYPTING );
            break;
            }
        case EDevEncUiCommandDecryptAndTurnOffEncryption:
            {
            DFLOG( "Decrypt and turn off selected" );
            // Warn the user about the long decryption process,
            // and that the key will be destroyed
            proceed = MessageQueryL( R_DEVENCUI_TEXT_NOTE_DECRYPTINGDESTROY );
            // Save the Mmc encryption key status
            User::LeaveIfError( iCrSettings->Set( KDevEncUserSettingMemoryCard,
                                                  EFalse ) );
            // The key will actually be destroyed in the DevEncStarter
            // component, because the user may exit from the UI at any time.
            // By saving the encryption key status in the central repository,
            // we ensure that the Starter knows that it's supposed to destroy
            // the key when the operation is complete
            // TBI: Destroy the key in Starter after decryption is complete.
            break;
            }
        default:
            {
            break;
            }
        }

    if ( aMem->State() == EUnmounted )
        {
        // Just go back to main view, no decryption operation
        iAppUi.ActivateLocalViewL( TUid::Uid( EDevEncUiMainViewId ) );
        proceed = EFalse;
        DFLOG( "No Mmc present -> no decryption" );
        }
    
    if ( proceed )
        {
        // Start the decryption
        DFLOG( "Starting decryption" );
        aMem->StartDecryptionL();
        // Go back to main view
        iAppUi.ActivateLocalViewL( TUid::Uid( EDevEncUiMainViewId ) );
        }
    else
        {
        DFLOG( "Decryption cancelled" );
        // Stay in the decryption view
        }

    // Depending on the user's selection, the memory may be in the same
    // state as before or in a new state. This call will let all observers
    // know what state the memory ended up in.
    aMem->UpdateMemoryInfo();
    }

// ---------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::CheckBatteryL()
// Checks if there's enough battery power to update
// ---------------------------------------------------------------------------
//
TBool CDevEncUiEncryptionOperator::CheckBatteryL()
    {
#ifdef __WINS__

    // In the emulator, the battery level is always 0 and the charger is never
    // connected, so just return ETrue.
    return ETrue;

#else // __WINS__

    // Running on target. Check the real battery and charger status

    TInt chargingstatus( EChargingStatusError );
    TInt batterylevel( 1 );
    TBool enoughPower( EFalse );

    // Read battery
    DFLOG( "CDevEncUiEncryptionOperator::CheckBatteryL" );
    RProperty pw;
    User::LeaveIfError( pw.Attach( KPSUidHWRMPowerState, KHWRMBatteryLevel ) );
    User::LeaveIfError( pw.Get( batterylevel ) );
    pw.Close();

    User::LeaveIfError( pw.Attach( KPSUidHWRMPowerState, KHWRMChargingStatus ) );
    User::LeaveIfError( pw.Get( chargingstatus ));
    pw.Close();

    // Too low battery, power insufficient
    if ( batterylevel >= EBatteryLevelLevel4 )
        {
        enoughPower = ETrue;
        }
    // But charger is connected, power sufficient
    if ( ( chargingstatus != EChargingStatusError ) &&
         ( chargingstatus != EChargingStatusNotConnected ) )
        {
        enoughPower = ETrue;
        }

    DFLOG3( "Battery level: %d  (0..7), chargingstatus %d",
           batterylevel, chargingstatus );
    DFLOG2( "CheckBatteryL %d", ( enoughPower ? 1 : 0 ) );
    return enoughPower;

#endif // __WINS__
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::ErrorNoteL()
// --------------------------------------------------------------------------
//
void CDevEncUiEncryptionOperator::ErrorNoteL( TInt aResourceId,
                                              TBool aWaitingNote )
    {
    HBufC* message = StringLoader::LoadLC( aResourceId );
    CAknErrorNote* errorNote = new ( ELeave ) CAknErrorNote( aWaitingNote );
    errorNote->ExecuteLD( *message );
    CleanupStack::PopAndDestroy( message );
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::InfoNoteL()
// --------------------------------------------------------------------------
//
void CDevEncUiEncryptionOperator::InfoNoteL( TInt aResourceId,
                                             TBool aWaitingNote )
    {
    HBufC* message = StringLoader::LoadLC( aResourceId );
    CAknInformationNote* note = new ( ELeave ) CAknInformationNote( aWaitingNote );
    note->ExecuteLD( *message );
    CleanupStack::PopAndDestroy( message );
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::DestroyKeyQueryL()
//
// --------------------------------------------------------------------------
void CDevEncUiEncryptionOperator::DestroyKeyQueryL( TBool aShowMmcNote )
    {
    TBool destroy = QueryDialogL( R_DEVENCUI_TEXT_NOTE_DECRYPT_NOMEMRYCARD, R_AVKON_SOFTKEYS_YES_NO );
    if ( destroy )
        {
        DestroyKeyL();
        if ( aShowMmcNote )
            {
            InfoNoteL( R_DEVENCUI_TEXT_NOTE_MEMORYCARD_UNENCRYPTED );
            }
        }
    else
        {
        DFLOG( "User cancelled destroy key query" );
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::DestroyKeyL()
//
// --------------------------------------------------------------------------
void CDevEncUiEncryptionOperator::DestroyKeyL()
    {
    DFLOG( "CDevEncUiEncryptionOperator::DestroyKeyL" );
    CDevEncKeyUtils* utils = new ( ELeave ) CDevEncKeyUtils;
    CleanupStack::PushL( utils );
//    User::LeaveIfError(utils->Connect());

    TRequestStatus status;
    utils->ResetKey( status );
    User::WaitForRequest( status );
//    utils->Close();
    CleanupStack::PopAndDestroy( utils );
    if ( status.Int() )
        {
        DFLOG2( "DestroyKeyL error %d", status.Int() );
        }
    else
        {
        DFLOG( "DestroyKeyL done" );
        // We no longer have a key in the driver. Update the user setting.
        iCrSettings->Set( KDevEncUserSettingMemoryCard, 0 );
        }
    User::LeaveIfError( status.Int() );
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::SuggestMmcEncryptionL()
//
// --------------------------------------------------------------------------
void CDevEncUiEncryptionOperator::SuggestMmcEncryptionL(
                                                CDevEncUiMemoryEntity* aMem )
    {
    TInt encrypt( 0 );
    
    // Check DM admin status
    TInt dmControlSetting( 0 );
    iCrSettings->Get( KDevEncUiDmControl, dmControlSetting );
    
    if( dmControlSetting & KDmControlsMemoryCard )
    	{
    	//The admin is controlling the encryption, encrypt straight away without questions
    	encrypt = ETrue;
    	}
    else
    	{
    	encrypt = MessageQueryL( R_DEVENCUI_TEXT_NOTE_INSERTUNENCRYPTED,
                R_AVKON_SOFTKEYS_YES_NO );
        }
    
    if ( encrypt )
        {
        HandleMmcEncryptRequestL( aMem,
                                  EDevEncUiCommandEncryptWithCurrentKey );
        }
    else
        {
        DFLOG( "User selected not to encrypt the card" );
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::SuggestMmcDecryptionL()
//
// --------------------------------------------------------------------------
void CDevEncUiEncryptionOperator::SuggestMmcDecryptionL( CDevEncUiMemoryEntity* aMem )
    {
    TBool decrypt = MessageQueryL( R_DEVENCUI_TEXT_NOTE_INSERTENCRYPTED,
                                   R_AVKON_SOFTKEYS_YES_NO );
    if ( decrypt )
        {
        SuggestMmcImportKeyL( aMem, ETrue );
        }
    else
        {
        ErrorNoteL( R_DEVENCUI_TEXT_NOTE_INSERTOFF_UNUSEDMEMORYCARD );
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::SuggestMmcImportKeyL()
//
// --------------------------------------------------------------------------
void CDevEncUiEncryptionOperator::SuggestMmcImportKeyL(
                                                CDevEncUiMemoryEntity* /*aMem*/,
                                                TBool aFirstAttempt )
    {
    DFLOG( "CDevEncUiEncryptionOperator::SuggestMmcImportKeyL" );
    TBool import( ETrue );
    if ( aFirstAttempt )
        {
        DFLOG( "CDevEncUiEncryptionOperator::SuggestMmcImportKeyL => firstAttempt" );
        import = MessageQueryL( R_DEVENCUI_TEXT_NOTE_DECRYPT_LOADKEY,
                                R_AVKON_SOFTKEYS_YES_NO );
        }
    
    if ( import )
        {
        TBool reloadKey = ETrue; 
        // Loop while user chooses correct key or cancels operation 
        while ( reloadKey ) 
            { 
            TBool keySet = LoadAndSetKeyL(); 
            if ( !keySet ) 
                { 
                ErrorNoteL( R_DEVENCUI_TEXT_NOTE_UNUSEDMEMORYCARD ); 
                reloadKey = EFalse; 
                } 
            else 
                { 
                // New key is in use, now we need to remount the mmc 
                RemountMmcL(); 
                
                // Check that key is valid 
                TInt err = CheckMMCStatus();              

                if( err == KErrCorrupt )
                	{
                	ErrorNoteL( R_DEVENCUI_TEXT_NOTE_WRONGKEYFILE );
                	reloadKey = ETrue;
                	}
                else
                	{
                	reloadKey = EFalse;
                	}
                } 
            }
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::CheckMMCStatus()
//
// --------------------------------------------------------------------------
TInt CDevEncUiEncryptionOperator::CheckMMCStatus()
    {
    TVolumeInfo volInfo; 
    RFs fs; 
    TInt err =  fs.Connect(); 
    if (err == KErrNone)
        {
        fs.Volume( volInfo, EDriveF );
        fs.Close();
        }
    return err;
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::RemountMmc()
//
// --------------------------------------------------------------------------
void CDevEncUiEncryptionOperator::RemountMmcL()
    {
    CDevEncUiFileManager* fm = new ( ELeave ) CDevEncUiFileManager;
    CleanupStack::PushL( fm );
    fm->ConstructL();
    TInt error = fm->RemountMmc();
    CleanupStack::PopAndDestroy( fm );
    if ( error )
        {
        DFLOG2( "Mmc remount failed, error %d", error );
        User::Leave( error );
        }
    else
        {
        DFLOG( "Mmc remounted" );
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::ShowWrongKeyNoteL()
//
// --------------------------------------------------------------------------
void CDevEncUiEncryptionOperator::ShowWrongKeyNoteL()
    {
    ErrorNoteL( R_DEVENCUI_TEXT_NOTE_INSERTON_UNUSEDMEMORYCARD );
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::CheckNotesWithHeaderL()
//
// --------------------------------------------------------------------------
void CDevEncUiEncryptionOperator::CheckNotesWithHeaderL()
    {
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_DECRYPT_LOADKEY );
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_DECRYPT_NOMEMRYCARD );
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_DECRYPTING );
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_DECRYPTINGDESTROY );
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_ENCRYPTING );
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_ENCRYPTIONINTERRUPT );
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_INSERTENCRYPTED );
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_INSERTOFF_UNUSEDMEMORYCARD );
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_INSERTON_UNUSEDMEMORYCARD );
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_INSERTUNENCRYPTED );
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_LOCK );
    MessageQueryL( R_DEVENCUI_TEXT_NOTE_PHONE_DECRYPTING );
    }

// --------------------------------------------------------------------------
// CDevEncUiEncryptionOperator::CheckNotesL()
// --------------------------------------------------------------------------
//
void CDevEncUiEncryptionOperator::CheckNotesL()
    {
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_DECRYPT_LOADKEY );
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_DECRYPT_NOMEMRYCARD );
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_DECRYPTING );
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_DECRYPTINGDESTROY );
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_ENCRYPTING );
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_ENCRYPTIONINTERRUPT );
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_INSERTENCRYPTED );
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_INSERTOFF_UNUSEDMEMORYCARD );
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_INSERTON_UNUSEDMEMORYCARD );
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_INSERTUNENCRYPTED );
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_LOCK );
    QueryDialogL( R_DEVENCUI_TEXT_NOTE_PHONE_DECRYPTING );
    }

// END OF FILE
