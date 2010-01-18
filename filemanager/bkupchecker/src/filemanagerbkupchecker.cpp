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
* Description:  Rule based application launch during backup and restore.
*
*/


// INCLUDES
#include <bautils.h>
#include <barsc2.h>
#include <barsread2.h>
#include <e32property.h>
#include <data_caging_path_literals.hrh>
#include <filemanagerbkupchecker.rsg>
#include <AknGlobalNote.h>
#include <AknSgcc.h>
#include <ecom/implementationproxy.h>

#include "FileManagerDebug.h"
#include "filemanagerprivatepskeys.h"
#include "FileManagerUID.h"
#include "filemanagerbkupchecker.h"

// CONSTANTS
_LIT(KMsengRscFilePath,"filemanagerbkupchecker.rsc");
_LIT(KThreadName,"BkupCheckerThread"); 

#ifdef __SKIP_PS_IN_TEST_
// Controlling of KUidBackupRestoreKey only possible with SID 0x10202D56
// That is why we have to variate running of test code using testVariable
extern TInt testVariable;
#endif

// Define the interface UIDs.
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY( 0x1020508A, CFileManagerBkupChecker::NewL )
    };


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// ProcessExists
// ---------------------------------------------------------------------------
//
static TBool ProcessExists( const TSecureId& aSecureId )
    {
    _LIT( KFindPattern, "*" );
    TFindProcess finder(KFindPattern);
    TFullName processName;
    while( finder.Next( processName ) == KErrNone )
        {
        RProcess process;
        if ( process.Open( processName ) == KErrNone )
            {
            TSecureId processId( process.SecureId() );
            process.Close();
            if( processId == aSecureId )
                {
                return ETrue;
                }
            }
        }
    return EFalse;
    }

// ---------------------------------------------------------------------------
// GetFileManagerBurStatus
// ---------------------------------------------------------------------------
//
static TInt GetFileManagerBurStatus()
    {
    TInt status( EFileManagerBkupStatusUnset );
    TInt err( RProperty::Get(
        KPSUidFileManagerStatus, KFileManagerBkupStatus, status ) );
    if ( err != KErrNone )
        {
        status = EFileManagerBkupStatusUnset;
        }
    else if( status == EFileManagerBkupStatusBackup || 
             status == EFileManagerBkupStatusRestore )
        {
        const TSecureId KFileManagerUid(KFileManagerUID3);
        // Check file manager process just if bur state detected
        if( !ProcessExists( KFileManagerUid ) )
            {
            status = EFileManagerBkupStatusUnset;
            }
        }

    INFO_LOG2( "GetFileManagerBurStatus, status %d, err %d", status, err )

    return status;
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// ImplementationGroupProxy.
//
// -----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
    return ImplementationTable;
    }


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CFileManagerBkupChecker::CFileManagerBkupChecker
//
// ---------------------------------------------------------------------------
//
CFileManagerBkupChecker::CFileManagerBkupChecker()
    {
    }

// ---------------------------------------------------------------------------
// CFileManagerBkupChecker::NewL
//
// ---------------------------------------------------------------------------
//
CFileManagerBkupChecker* CFileManagerBkupChecker::NewL()
	{
	CFileManagerBkupChecker* self = new (ELeave) CFileManagerBkupChecker();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
	}

// ---------------------------------------------------------------------------
// CFileManagerBkupChecker::ConstructL
//
// ---------------------------------------------------------------------------
//
void CFileManagerBkupChecker::ConstructL()
    {
    FUNC_LOG
    RFs fsSession;
    
    // Connect to File Server
    User::LeaveIfError(fsSession.Connect());
    CleanupClosePushL(fsSession);
    
    // Get resource drive from dll location
    TFileName dllFileName;
    Dll::FileName( dllFileName );
    TParsePtrC dllParse( dllFileName );
    TFileName fileName;
#ifdef __SKIP_PS_IN_TEST_
    _LIT(KDriveZ,"Z:");
    fileName.Copy( KDriveZ );
#else
    // Drive is parsed normally from dll-location in order to support
    // installing/patching of component.
    fileName.Copy( dllParse.Drive() );
#endif
    fileName.Append( KDC_RESOURCE_FILES_DIR );
    fileName.Append( KMsengRscFilePath );
    BaflUtils::NearestLanguageFile( fsSession, fileName );
    //
    TEntry entry;
    User::LeaveIfError( fsSession.Entry( fileName, entry ) );
    // if file does not exist, leaves with KErrNotFound
    
    CResourceFile *resFile;
    resFile = CResourceFile::NewLC( fsSession, fileName, 0, entry.iSize );
    
    resFile->ConfirmSignatureL();
    
    /////////////////////////////////////////////////////////
    //Initialize white-list of applications from resource file

    RResourceReader theReader;
    theReader.OpenLC( resFile, ALLOWEDUIDS );
    
    //the first WORD contains the number of elements in the resource
    TInt numberOfUIDs = theReader.ReadInt16L();
    
    for( TInt i = 0; i < numberOfUIDs; i++)
        {
        TUint32 uid = theReader.ReadInt32L();
        iUids.Append(uid);
        INFO_LOG1( "CFileManagerBkupChecker::ConstructL, Application 0x%x added in white-list", iUids[i] )
        }
    CleanupStack::PopAndDestroy( &theReader );
    
    //Initialize information note texts from resource file
    theReader.OpenLC( resFile, R_QTN_FMGR_BACKUP_APPLAUNCH_PREVENTED );
    iBackupNote = theReader.ReadHBufCL();
    CleanupStack::PopAndDestroy( &theReader );
    
    theReader.OpenLC( resFile, R_QTN_FMGR_RESTORE_APPLAUNCH_PREVENTED );
    iRestoreNote = theReader.ReadHBufCL();
    CleanupStack::PopAndDestroy( &theReader );
    
    CleanupStack::PopAndDestroy( resFile );
    CleanupStack::PopAndDestroy( &fsSession );
    }


// ---------------------------------------------------------------------------
// CFileManagerBkupChecker::~CFileManagerBkupChecker
//
// ---------------------------------------------------------------------------
//
CFileManagerBkupChecker::~CFileManagerBkupChecker()
    {
    delete iBackupNote;
    delete iRestoreNote;
    iUids.Close();
    }

// ---------------------------------------------------------------------------
// CFileManagerBkupChecker::OkayToLaunchL
//
// ---------------------------------------------------------------------------
//
CAppLaunchChecker::TAppLaunchCode CFileManagerBkupChecker::OkayToLaunchL(const TUid aAppToLaunch, 
    TApaTaskList& /* aTaskList */)
	{ 
    FUNC_LOG
	CAppLaunchChecker::TAppLaunchCode launch = CAppLaunchChecker::EAppLaunchIndifferent;
	
	TInt burState( 0 );
	TInt burErr = RProperty::Get( KUidSystemCategory, KUidBackupRestoreKey, burState ); 
	
	if(burErr == KErrNone)
	    {
	    INFO_LOG2
	    ( 
	    "CFileManagerBkupChecker::OkayToLaunchL, Application 0x%x, KUidBackupRestoreKey status %d",
	    aAppToLaunch.iUid, burState 
	    )
	    
	    TBURPartType burType = static_cast< TBURPartType >( burState & KBURPartTypeMask );
	    TInt fmBurStatus( GetFileManagerBurStatus() );
        
        // We can't rely on just p&s value. Additional check is carried out in ValidateBUROngoing.
#ifdef __SKIP_PS_IN_TEST_
        // Run additional validation check in test mode just to cover all use cases
        ValidateBUROngoing();
#else
	    if( fmBurStatus == EFileManagerBkupStatusBackup ||
	        fmBurStatus == EFileManagerBkupStatusRestore ||
	        ( ( burType == EBURBackupPartial || burType == EBURBackupFull ||
	            burType == EBURRestorePartial || burType == EBURRestoreFull ) &&
	            ValidateBUROngoing() ) )
#endif
	        {
	        launch = CAppLaunchChecker::EAppLaunchDecline;
	        
	        TInt count( iUids.Count() );
	        
        	for( TInt i = 0; i < count; i++ )
        	    {
        	    if(iUids[i] == aAppToLaunch.iUid)
                    {
                    INFO_LOG1
                    ( 
                    "CFileManagerBkupChecker::OkayToLaunchL, Application 0x%x in white-list", 
                    iUids[i] 
                    )
                    launch = CAppLaunchChecker::EAppLaunchIndifferent;
                    break;
                    }
        	    }
            
            if( launch == CAppLaunchChecker::EAppLaunchDecline )
                {
                INFO_LOG1
                (
                "CFileManagerBkupChecker::OkayToLaunchL, Application 0x%x launch prevented", 
                aAppToLaunch.iUid 
                )

                iIsBackup = ( fmBurStatus == EFileManagerBkupStatusBackup ||
                              burType == EBURBackupPartial ||
                              burType == EBURBackupFull );

                RThread thread;
                
                TInt err = thread.Create(
                    KThreadName, ThreadFunction, KDefaultStackSize, NULL, this );
                    
                INFO_LOG1("CFileManagerBkupChecker::OkayToLaunchL, thread err %d", err)

                if ( err == KErrNone )
                    {
                    TRequestStatus status;
                    
                    thread.Rendezvous( status );
                    thread.Resume();
                    
                    // Wait until thread has copy of note text.
                    User::WaitForRequest( status );

                    INFO_LOG1("CFileManagerBkupChecker::OkayToLaunchL, thread exit %d", status.Int())
                    }
                
                thread.Close();
                }
	        }
	    }

	return launch;
	}

// ---------------------------------------------------------------------------
// CFileManagerBkupChecker::ValidateBUROngoing
//
// ---------------------------------------------------------------------------
//
TBool CFileManagerBkupChecker::ValidateBUROngoing()
    {
    TBool err(EFalse);
    _LIT( KFindPattern, "*" );
    const TSecureId KSBEUid(0x10202D56);
    const TSecureId KFileManagerUid(KFileManagerUID3);
    const TSecureId KPCConnectivityUid(0x101F99F6);
    TBool serverRunning(EFalse);
    TBool client1Running(EFalse);
    TBool client2Running(EFalse);
    
    // If SBE panics, File Manager and PC-connectivity server are supposed to 
    // re-establish connection to server and set BUR-mode back to normal.
    // If SBE client panics, server is supposed to set BUR-mode back to normal.
    // However, it might be reasonable to validate also that BUR client is
    // up and running. E.g. if both server and client panic in sequence, BUR
    // state might stay as backup or restore and we never let application run.
    // We have to search by UID, because process can have localized name
    TFindProcess finder( KFindPattern );
    TFullName processName;

    while( finder.Next( processName ) == KErrNone )
        {
        RProcess process;
        const TInt r = process.Open( processName );
        if  ( r == KErrNone )
            {
            const TSecureId processId = process.SecureId();
            process.Close();
            
            if( processId == KSBEUid )
                {
                serverRunning = ETrue;
                }
            else if( processId == KFileManagerUid )
                {
                client1Running = ETrue;
                }
            else if( processId == KPCConnectivityUid )
                {
                client2Running = ETrue;
                }
            }
        }

    INFO_LOG2("CFileManagerBkupChecker::ValidateBUROngoing, %x status %d", 
        KSBEUid.iId, serverRunning);
    INFO_LOG2("CFileManagerBkupChecker::ValidateBUROngoing, %x status %d", 
        KFileManagerUid.iId, client1Running);
    INFO_LOG2("CFileManagerBkupChecker::ValidateBUROngoing, %x status %d", 
        KPCConnectivityUid.iId, client2Running);

    if( serverRunning && (client1Running || client2Running) )
        {
        err = ETrue;
        }

    return err;
    }

// ----------------------------------------------------------------------------
// CFileManagerBkupChecker::ThreadFunction()
//
// ----------------------------------------------------------------------------
TInt CFileManagerBkupChecker::ThreadFunction( TAny* ptr )
    {
    FUNC_LOG

    CFileManagerBkupChecker* self =
        static_cast< CFileManagerBkupChecker* >( ptr );

    CTrapCleanup* cleanupStack = CTrapCleanup::New();
    if ( !cleanupStack )
        {
        return KErrNoMemory;
        }
        
    TRAPD( err, self->ThreadFunctionL() );
    
    INFO_LOG1("CFileManagerBkupChecker::ThreadFunction, ThreadFunctionL err %d", err)

    delete cleanupStack;

    return err;
    }

// ----------------------------------------------------------------------------
// CFileManagerBkupChecker::ThreadFunctionL()
//
// ----------------------------------------------------------------------------
void CFileManagerBkupChecker::ThreadFunctionL( )
    {
    FUNC_LOG

    HBufC* note = NULL;
    
#ifdef __SKIP_PS_IN_TEST_
    if(!testVariable)
        {
        note = iBackupNote->AllocLC();
        }
    else
        {
        note = iRestoreNote->AllocLC();
        }
#else
    if( iIsBackup )
	    {
	    note = iBackupNote->AllocLC();
	    }
	else
	    {
	    note = iRestoreNote->AllocLC();
	    }
#endif

    // Once we have locally allocated note string, we can signal main thread.
    RThread::Rendezvous( KErrNone );
    
    RAknUiServer aknSrv;
    TInt err( aknSrv.Connect() );
    INFO_LOG1("CFileManagerBkupChecker::ThreadFunctionL, connect err %d", err)
    User::LeaveIfError( err );
    CleanupClosePushL( aknSrv );
	
    aknSrv.ShowGlobalNoteL( *note, EAknGlobalInformationNote );

    CleanupStack::PopAndDestroy( &aknSrv );
    CleanupStack::PopAndDestroy( note );
    }

