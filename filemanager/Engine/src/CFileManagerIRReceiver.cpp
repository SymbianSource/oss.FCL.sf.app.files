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
* Description:  Receives obex object via InfraRed, used to receive files
*
*/


// INCLUDE FILES
#include <obex.h>
#include "CFileManagerIRReceiver.h"
#include "MFileManagerProcessObserver.h"
#include "CFileManagerEngine.h"
#include "CFileManagerUtils.h"
#include "CFileManagerCommonDefinitions.h"
#include "FileManagerDebug.h"


// CONSTANTS
const TUint KDefaultObexPort = 65;
const TUint KBufferGranularity = 2048;
const TInt KFullPercentage = 100;
const TInt KInactiveTimeout = 60000000;

// Required for IR
_LIT( KTransportTinyTp, "IrTinyTP" );
_LIT8( KClassNameObex, "OBEX" );
_LIT8( KAttName, "IrDA:TinyTP:LsapSel" );
_LIT( KTemporaryFile, "__FileManagerIrdaReceive.tmp" );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerEngine::CFileManagerEngine
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerIRReceiver::CFileManagerIRReceiver(
        MFileManagerProcessObserver& aObserver,
        CFileManagerEngine& aEngine,
        RFs& aFs ) :
    iObserver( aObserver ),
    iEngine( aEngine ),
    iFs( aFs )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CFileManagerIRReceiver* CFileManagerIRReceiver::NewL(
        MFileManagerProcessObserver& aObserver,
        const TDesC& aPath,
        CFileManagerEngine& aEngine )
    {
    FUNC_LOG

    CFileManagerIRReceiver* self = new( ELeave ) CFileManagerIRReceiver(
            aObserver,
            aEngine,
            aEngine.Fs() );
    
    CleanupStack::PushL( self );
    self->ConstructL( aPath );
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerIRReceiver::ConstructL( const TDesC& aPath )
    {
    TPtrC ptrPath( CFileManagerUtils::StripFinalBackslash( aPath ) );
    TInt len( ptrPath.Length() +
        KFmgrBackslashSpace + KTemporaryFile().Length() );
    if ( len > KMaxFileName )
        {
        User::Leave( KErrBadName );
        }
    iTempFile = HBufC::NewL( len );
    TPtr ptr( iTempFile->Des() );
    ptr.Append( ptrPath );
    CFileManagerUtils::EnsureFinalBackslash( ptr );
    ptr.Append( KTemporaryFile );
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::ReceiveFileL
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerIRReceiver::ReceiveFileL()
    {
    FUNC_LOG

    ResetInactivityTimer();

    TObexIrProtocolInfo obexIrInfo;
    obexIrInfo.iAddr.SetPort( KDefaultObexPort );
    obexIrInfo.iTransport     = KTransportTinyTp;
    obexIrInfo.iClassName     = KClassNameObex;
    obexIrInfo.iAttributeName = KAttName;

    iObexServer = CObexServer::NewL( obexIrInfo );

    User::LeaveIfError( iObexServer->Start( this ) );
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::~CFileManagerIRReceiver
// Destructor
// -----------------------------------------------------------------------------
// 
EXPORT_C CFileManagerIRReceiver::~CFileManagerIRReceiver()
    {
    FUNC_LOG
    
    if ( iObexServer )
        {
        if ( iObexServer->IsStarted() )
            {
            iObexServer->Stop();
            }
        delete iObexServer;
        }
    delete iObexBufObject;

    if ( iTempFile )
        {
        iFs.Delete( *iTempFile );
        delete iTempFile;
        }
    delete iBuffer;
    delete iInactivityTimer;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::StopReceiving
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerIRReceiver::StopReceiving()
    {
    FUNC_LOG

    iError = KErrCancel;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::ErrorIndication
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerIRReceiver::ErrorIndication(TInt aError )
    {
    ERROR_LOG1( "CFileManagerIRReceiver::ErrorIndication-aError=%d", aError )

    if ( iError == KErrNone )
        {
        iError = aError;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::TransportUpIndication
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerIRReceiver::TransportUpIndication()
    {
    FUNC_LOG

    StopInactivityTimer();
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::TransportDownIndication
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerIRReceiver::TransportDownIndication()
    {
    FUNC_LOG

    TInt err( KErrNone );
    if ( iObexBufObject )
        {
        // transfer has been cancelled, set temp buffer so obexobject won't
        // create temp file again
        delete iBuffer;
        iBuffer = NULL;
        TRAP( err, iBuffer = CBufFlat::NewL( KBufferGranularity ) );
        if ( err == KErrNone )
            {
            TRAP( err, iObexBufObject->SetDataBufL( iBuffer ) );
            }
        }

    if ( iTempFile )
        {
        iFs.Delete( *iTempFile );
        }

    TRAP( err, iObserver.ProcessFinishedL( iError ) );
    if ( err != KErrNone )
        {
        iObserver.Error( err );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::TargetHeaderReceived
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerIRReceiver::TargetHeaderReceived(TDesC8& /* aTargetHeader */ )
    {
    FUNC_LOG

    return EFalse;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::ObexConnectIndication 
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerIRReceiver::ObexConnectIndication
    ( const TObexConnectInfo& /* aRemoteInfo */, 
      const TDesC8& /* aInfo */ )
    {
    FUNC_LOG

    if ( iError == KErrCancel )
        {
        return KErrCancel;
        }

    TRAPD( err, iObserver.ProcessStartedL( 
        MFileManagerProcessObserver::EIRReceiveProcess ) );

    return err;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::ObexDisconnectIndication
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerIRReceiver::ObexDisconnectIndication(const TDesC8& /* aInfo */ )
    {
    FUNC_LOG
    // from MObexServerNotify
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::PutRequestIndication
// 
// -----------------------------------------------------------------------------
// 
CObexBufObject* CFileManagerIRReceiver::PutRequestIndication()
    {
    FUNC_LOG

    // delete old buffer if it exists
    delete iObexBufObject;
    iObexBufObject = NULL;
    iFs.Delete( *iTempFile );

    TRAPD( err, iObexBufObject = CObexBufObject::NewL( NULL ) );
    if ( err == KErrNone )
        {
        TRAP( err, iObexBufObject->SetDataBufL( *iTempFile ) );
        }

    if ( err != KErrNone )
        {
        return NULL;
        }

    return iObexBufObject;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::PutPacketIndication
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerIRReceiver::PutPacketIndication()
    {
    FUNC_LOG

    if ( iError == KErrCancel )
        {
        return KErrCancel;
        }

    const TInt size( iObexBufObject->Length() );

    if ( !iDiskSpaceChecked )
        {
        TParsePtrC parse( *iTempFile );
        TRAP( iError, iEnoughSpace = iEngine.EnoughSpaceL(
            parse.DriveAndPath(), size,
            MFileManagerProcessObserver::EIRReceiveProcess ) );
        iDiskSpaceChecked = ETrue;
        }

    if ( iError == KErrNone && iEnoughSpace )
        {
        const TInt received( iObexBufObject->BytesReceived());
        TInt percent( 0 );
        if ( size != 0)
            {
            percent = KFullPercentage * received / size;
            }
        TRAPD( error, iObserver.ProcessAdvanceL( percent ) );
        if ( iError == KErrNone )
            {
            iError = error;
            }
        }
    else if ( !iEnoughSpace )
        {
        if ( iError == KErrNone )
            {
            iError = KErrDiskFull;
            }
        }

    return iError;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::PutCompleteIndication
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerIRReceiver::PutCompleteIndication()
    {
    FUNC_LOG

    if ( iError == KErrCancel )
        {
        iFs.Delete( *iTempFile );
        iDiskSpaceChecked = EFalse;
        return KErrNone;
        }

    TInt error( KErrNone );
    TFileName fileName;
    TParsePtrC parse( *iTempFile );
    fileName.Append( parse.DriveAndPath() );
    TPtrC objName( iObexBufObject->Name() );

    if ( fileName.Length() + objName.Length() > KMaxFileName )
        {
        iFs.Delete( *iTempFile );
        iDiskSpaceChecked = EFalse;
        iObserver.Error( KErrBadName );
        return KErrBadName;
        }

    fileName.Append( objName );
    delete iObexBufObject;
    iObexBufObject = NULL;
    TBool nameFound( EFalse );
    TRAP( error, nameFound = iEngine.IsNameFoundL( fileName ) );
    if ( error == KErrNone && nameFound )
        {
        TFileName name;
        TBool overWrite( ETrue );
        TRAP( error, overWrite = iObserver.ProcessQueryOverWriteL( 
            fileName, name, 
            MFileManagerProcessObserver::EIRReceiveProcess ) );
        
        if ( error == KErrNone )
            {
            if ( overWrite )
                {
                error = iFs.Delete( fileName );
                if ( error == KErrNone )
                    {
                    iFs.Rename( *iTempFile, fileName );
                    }
                if ( error != KErrNone )
                    {
                    iObserver.Error( error );
                    }
                }
            else 
                {
                TBool askAgain( ETrue );
                TRAP( error, nameFound = iEngine.IsNameFoundL( name ) );
                while( error == KErrNone && nameFound && askAgain )
                    {
                    TFileName newName;
                    TRAP( error, overWrite = 
                        iObserver.ProcessQueryOverWriteL( 
                            name, 
                            newName, 
                            MFileManagerProcessObserver::EIRReceiveProcess 
                            ) );
                    if ( error != KErrNone )
                        {
                        iError = error;
                        askAgain = EFalse;
                        name.Zero();
                        }
                    else if ( !overWrite )
                        {
                        name.Zero();
                        name.Append( newName );
                        }
                    else
                        {
                        askAgain = EFalse;
                        }
                    }
                if ( name.Length() > 0 )
                    {
                    TParsePtrC parse( *iTempFile );
                    name.Insert( 0, parse.DriveAndPath() );
                    iFs.Rename( *iTempFile, name );
                    }
                }
            }
        else
            {
            iError = error;
            }
        }
    else
        {

        iFs.Rename( *iTempFile, fileName );
        }
    iFs.Delete( *iTempFile );
    iDiskSpaceChecked = EFalse;

    return error;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::GetRequestIndication
// Empty because needed only for send functionality
// -----------------------------------------------------------------------------
// 
CObexBufObject* CFileManagerIRReceiver::GetRequestIndication
    (CObexBaseObject* /* aRequiredObject */)
    {
    FUNC_LOG

    return NULL;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::GetPacketIndication
// Empty because needed only for send functionality
// -----------------------------------------------------------------------------
// 
TInt CFileManagerIRReceiver::GetPacketIndication()
    {
    FUNC_LOG

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::GetCompleteIndication
// Empty because needed only for send functionality
// -----------------------------------------------------------------------------
// 
TInt CFileManagerIRReceiver::GetCompleteIndication()
    {
    FUNC_LOG

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::SetPathIndication
// Empty because setpath not supported
// -----------------------------------------------------------------------------
// 
TInt CFileManagerIRReceiver::SetPathIndication(
        const CObex::TSetPathInfo& /* aPathInfo*/ , const TDesC8& /* aInfo */)
    {
    FUNC_LOG

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::AbortIndication
// Empty because needed only for send functionality
// -----------------------------------------------------------------------------
// 
void CFileManagerIRReceiver::AbortIndication()
    {
    FUNC_LOG
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::ResetInactivityTimer
//
// -----------------------------------------------------------------------------
// 
void CFileManagerIRReceiver::ResetInactivityTimer()
    {
    FUNC_LOG

    StopInactivityTimer();

    TRAPD( err, iInactivityTimer =
        CPeriodic::NewL( CActive::EPriorityStandard ) );
    if ( err == KErrNone )
        {
        iInactivityTimer->Start(
            KInactiveTimeout,
            KInactiveTimeout,
            TCallBack( InactivityTimeout, this ) );
            
        INFO_LOG( "CFileManagerIRReceiver::ResetInactivityTimer-Timer started" )
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::InactivityTimeout
//
// -----------------------------------------------------------------------------
// 
TInt CFileManagerIRReceiver::InactivityTimeout( TAny* aPtr )
    {
    FUNC_LOG

    CFileManagerIRReceiver* self =
        static_cast< CFileManagerIRReceiver* >( aPtr );

    self->StopInactivityTimer();
    self->CloseConnection();

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::StopInactivityTimer
//
// -----------------------------------------------------------------------------
// 
void CFileManagerIRReceiver::StopInactivityTimer()
    {
    FUNC_LOG

    delete iInactivityTimer;
    iInactivityTimer = NULL;
    }

// -----------------------------------------------------------------------------
// CFileManagerIRReceiver::CloseConnection
//
// -----------------------------------------------------------------------------
// 
void CFileManagerIRReceiver::CloseConnection()
    {
    FUNC_LOG

    StopReceiving();

    if ( iObexServer )
        {
        if ( iObexServer->IsStarted() )
            {
            INFO_LOG( "CFileManagerIRReceiver::CloseConnection-Stop server" )

            iObexServer->Stop();
            }
        }

    TRAPD( err, iObserver.ProcessFinishedL( KErrCancel ) );
    if ( err != KErrNone )
        {
        iObserver.Error( err );
        }
    }

//  End of File  
