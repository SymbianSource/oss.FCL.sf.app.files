/*
* Copyright (c) 2006-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Memory Scan Client
*
*/



// SYSTEM INCLUDES
#include <e32svr.h>
#include <s32mem.h> // RBufReadStream

// USER INCLUDES
#include "memscanclient.h"
#include "memscanutils.h" // traces


// Standard server startup code
// 
static TInt StartServer()
	{
	TRACES( RDebug::Print(_L("MemScanClient: Starting server...")) );
  
	RProcess server;
	TInt r=server.Create(KMemScanServImg,KNullDesC);

	if (r!=KErrNone)
		{
		TRACES( RDebug::Print(_L("MemScanClient: server start failed %d"),r) );
		return r;
		}
	TRequestStatus stat;
	server.Rendezvous(stat);
	if (stat!=KRequestPending)
		server.Kill(0);		// abort startup
	else
		server.Resume();	// logon OK - start the server
	TRACES( RDebug::Print(_L("MemScanClient: Started")) );
	User::WaitForRequest(stat);		// wait for start or death
	// we can't use the 'exit reason' if the server panicked as this
	// is the panic 'reason' and may be '0' which cannot be distinguished
	// from KErrNone
	r=(server.ExitType()==EExitPanic) ? KErrGeneral : stat.Int();
	server.Close();
	return r;
	}

EXPORT_C RMemScanClient::RMemScanClient()
: iScanEventPackagePointer( NULL, 0, 0 )
    {
    }
    

// This is the standard retry pattern for server connection
EXPORT_C TInt RMemScanClient::Connect()
	{
	TVersion version(KMemScanServMajor, KMemScanServMinor, KMemScanServBuild);
	TInt retry=2;
	for (;;)
		{
		TInt r=CreateSession(KMemScanServName, version, 1);
		if (r!=KErrNotFound && r!=KErrServerTerminated)
			return r;
		if (--retry==0)
			return r;
		r=StartServer();
		if (r!=KErrNone && r!=KErrAlreadyExists)
			return r;
		}
	}
	
EXPORT_C void RMemScanClient::Close()
	{
	RSessionBase::Close();  //basecall
	}


// ***************************************************************************
// Client Server functions
// ***************************************************************************


// ---------------------------------------------------------------------------
// RMemScanClient::DataGroupsL()
//
// ---------------------------------------------------------------------------
EXPORT_C CDesCArray* RMemScanClient::DataGroupsL() const
    {
    // Ask server to externalize the data group array to buffer
    // and to provide size of buffer 
    TPckgBuf<TInt> bufferSize;
    User::LeaveIfError(SendReceive( EMemScanPrepareDataGroups, TIpcArgs(&bufferSize) ));
    
    // Create a buffer of sufficient size in order to fetch the
	// buffer from the server
	CBufBase* buffer = CBufFlat::NewL( bufferSize() ); //buffer granularity
	CleanupStack::PushL( buffer );
	buffer->ResizeL( bufferSize() ); //buffer size

	// Now fetch the transfer buffer from the server
	TPtr8 pBuffer(buffer->Ptr(0));
	User::LeaveIfError(SendReceive( EMemScanGetDataGroups, TIpcArgs(&pBuffer) ));
    
    // *** Start internalizing the buffer ***
    
    RBufReadStream stream(*buffer);
    CleanupClosePushL(stream);
	
	// Read the number of data groups from the beginning of the stream
	const TInt count = stream.ReadInt32L();
	
    // Create the array for the data group names with appropriate granularity
	CDesCArray* dataGroupNameArray = new (ELeave) CDesCArrayFlat(count);
    CleanupStack::PushL( dataGroupNameArray );
    
	
	// Read each group name to array
    for(TInt i=0; i<count; i++)
        {
        // Read number of bytes in varying length descriptor
        const TInt length = stream.ReadInt32L();
        // Read datagroup name
        HBufC* group = HBufC::NewLC( stream, length );
        dataGroupNameArray->AppendL( *group );
        CleanupStack::PopAndDestroy( group );
        }
    
    // *** Internalizing done ***    
    
    CleanupStack::Pop( dataGroupNameArray );
    CleanupStack::PopAndDestroy( &stream );   
    CleanupStack::PopAndDestroy( buffer );

     
    return dataGroupNameArray;
    }       


// ---------------------------------------------------------------------------
// RMemScanClient::ScanResultL()
//
// ---------------------------------------------------------------------------
EXPORT_C CArrayFix<TInt64>* RMemScanClient::ScanResultL() const
	{
    // Ask server to externalize the scan result array to buffer
    // and to provide size of buffer
	TPckgBuf<TInt> bufferSize;
	User::LeaveIfError(SendReceive(EMemScanPrepareScanResults, TIpcArgs( &bufferSize )));
	
	// Create a buffer of sufficient size in order to fetch the
	// buffer from the server
	CBufBase* buffer = CBufFlat::NewL( bufferSize() ); //buffer granularity
	CleanupStack::PushL( buffer );
	buffer->ResizeL( bufferSize() ); //buffer size

	// Now fetch the transfer buffer from the server
	TPtr8 pBuffer(buffer->Ptr(0));
	TIpcArgs args(&pBuffer);
	User::LeaveIfError(SendReceive( EMemScanGetScanResults, args ));
	
	// *** Start internalizing the buffer ***
	
    RBufReadStream stream(*buffer);
    CleanupClosePushL(stream);
    
	// Read the number of results from the beginning of the stream
	const TInt count = stream.ReadInt32L();
	
    // Create the result array with such granularity
    // that reallocations do not happen
    CArrayFix<TInt64>* resultArray = new (ELeave) CArrayFixFlat<TInt64>(count);
    CleanupStack::PushL(resultArray);
	
	
    // Read scan results for each data group and write them to result array
    for(TInt i=0; i<count; i++)
        {    
        TInt64 result;
        stream >> result;
        resultArray->AppendL(result);
        }
        
    // *** Internalizing done ***        
    
    CleanupStack::Pop( resultArray );
    CleanupStack::PopAndDestroy( &stream );
    CleanupStack::PopAndDestroy( buffer );
	
	
	return resultArray;
	}

// ---------------------------------------------------------------------------
// RMemScanClient::Scan()
//
// ---------------------------------------------------------------------------
EXPORT_C TInt RMemScanClient::Scan(TDriveNumber aDrive)
	{
	TInt err = SendReceive( EMemScanStartScan, TIpcArgs( aDrive ) );
	return err;
	}


// ---------------------------------------------------------------------------
// RMemScanClient::RequestScanEvents()
//
// ---------------------------------------------------------------------------
EXPORT_C void RMemScanClient::RequestScanEvents( TInt& aError, TRequestStatus& aRequestStatus )
    {
	iScanEventPackagePointer.Set( (TUint8*) &aError, sizeof(TInt), sizeof(TInt) );

	TIpcArgs args( &iScanEventPackagePointer );
	SendReceive( EMemScanRequestScanEvents, args, aRequestStatus );
    }


// ---------------------------------------------------------------------------
// RMemScanClient::RequestScanEventsCancel()
//
// ---------------------------------------------------------------------------
EXPORT_C void RMemScanClient::RequestScanEventsCancel()
    {
    // There is nothing client can do if cancelling went wrong, 
    // so we don't return any error
	SendReceive( EMemScanRequestScanEventsCancel );
    }
	

// ---------------------------------------------------------------------------
// RMemScanClient::ScanInProgress()
//
// ---------------------------------------------------------------------------	
EXPORT_C TBool RMemScanClient::ScanInProgress() const
    {
    TBool result = EFalse;
    TInt err = SendReceive(EMemScanInProgress);
	if( err < 0 )
	    {
	    result = EFalse;
	    }
	else
	    {
	    result = static_cast<TBool> (err);
	    }
	return result;
    }


// End of File
