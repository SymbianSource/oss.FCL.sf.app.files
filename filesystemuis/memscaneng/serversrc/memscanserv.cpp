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
* Description:  Memory Scan Server
*
*/



// SYSTEM INCLUDES
#include <e32svr.h>
#include <s32mem.h> // RBufWriteStream

// USER INCLUDES
#include "memscanserv.h"
#include "memscanutils.h" // traces

    
// ---------------------------------------------------------------------------
// Server startup code
// ---------------------------------------------------------------------------

// Perform all server initialisation, in particular creation of the
// scheduler and server and then run the scheduler
// 
static void RunServerL()
    {
    // naming the server thread after the server helps to debug panics
    User::LeaveIfError(User::RenameThread(KMemScanServName));

    // create and install the active scheduler we need
    CActiveScheduler* scheduler=new(ELeave) CActiveScheduler;
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);
    // create the server (leave it on the cleanup stack)
    CMemScanServ::NewLC();
    // Initialisation complete, now signal the client

    RProcess::Rendezvous(KErrNone);

    // Ready to run
    TRACES( RDebug::Print(_L("MemScanServ: server fully running")) );
    CActiveScheduler::Start();
    // Cleanup the server and scheduler
    CleanupStack::PopAndDestroy(2, scheduler);
    }

// Server process entry-point
TInt E32Main()
    {
    __UHEAP_MARK;
    TRACES( RDebug::Print(_L("MemScanServ: E32Main")) ); 
    CTrapCleanup* cleanup=CTrapCleanup::New();
    TInt r=KErrNoMemory;
    if (cleanup)
        {
        TRAP(r,RunServerL());
        delete cleanup;
        }
    __UHEAP_MARKEND;
    return r;
    }
    
// RMessagePtr2::Panic() also completes the message. This is:
// (a) important for efficient cleanup within the kernel
// (b) a problem if the message is completed a second time
void PanicClient(const RMessagePtr2& aMessage,TMemScanServPanic aPanic)
    {
    _LIT(KPanic,"MemScanServ");
    aMessage.Panic(KPanic,aPanic);
    }

    
// ---------------------------------------------------------------------------
// CShutDown
// ---------------------------------------------------------------------------
inline CShutdown::CShutdown()
    :CTimer(-1)
    {
    CActiveScheduler::Add(this);
    }
    
inline void CShutdown::ConstructL()
    {
    CTimer::ConstructL();
    }

inline void CShutdown::Start()
    {
    TRACES( RDebug::Print(_L("MemScanServ: starting shutdown timeout")) );
    After(EMemScanServShutdownDelay);
    }

void CShutdown::RunL()
    {
    TRACES( RDebug::Print(_L("MemScanServ: server timeout ... closing")) );
    CActiveScheduler::Stop();
    }

// ---------------------------------------------------------------------------
// CMemScanServ
// ---------------------------------------------------------------------------
inline CMemScanServ::CMemScanServ()
    :CPolicyServer(0, KMemScanServPolicy, ESharableSessions)
    {
    }
    
CServer2* CMemScanServ::NewLC()
    {
    TRACES( RDebug::Print(_L("MemScanServ: CMemScanServ::NewLC")) );
    CMemScanServ* self=new(ELeave) CMemScanServ;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// 2nd phase construction - ensure the timer and server objects are running
void CMemScanServ::ConstructL()
    {
    StartL(KMemScanServName);
    iShutdown.ConstructL();
    // ensure the server still exits even if the 1st client fails to connect
    if( !iShutdown.IsActive() )
        {
        iShutdown.Start();
        }
    }


// Create a new client session.
CSession2* CMemScanServ::NewSessionL(const TVersion& aVersion, const RMessage2&) const
    {
    TRACES( RDebug::Print(_L("MemScanServ: CMemScanServ::NewSessionL")) );

    // Client-Server version check
	TVersion version(KMemScanServMajor, KMemScanServMinor, KMemScanServBuild);
	if( !User::QueryVersionSupported( version, aVersion ) )
	    {
	    User::Leave( KErrNotSupported );
	    }

    return new (ELeave) CMemScanServSession();
    }

// A new session is being created
// Cancel the shutdown timer if it was running
void CMemScanServ::AddSession()
    {
    TRACES( RDebug::Print(_L("MemScanServ: CMemScanServ::AddSession")) );
    ++iSessionCount;
    iShutdown.Cancel();
    }

// A session is being destroyed
// Start the shutdown timer if it is the last session.
void CMemScanServ::DropSession()
    {
    TRACES( RDebug::Print(_L("MemScanServ: CMemScanServ::DropSession")) );
    if (--iSessionCount==0)
        {
        if( !iShutdown.IsActive() )
            {
            iShutdown.Start();
            }
        }
    }


// ---------------------------------------------------------------------------
// CMemScanServSession
// ---------------------------------------------------------------------------
inline CMemScanServSession::CMemScanServSession()
    {
    TRACES( RDebug::Print(_L("MemScanServer: CMemScanServSession::CMemScanServSession")); )
    }
    
inline CMemScanServ& CMemScanServSession::Server()
    {
    return *static_cast<CMemScanServ*>(const_cast<CServer2*>(CSession2::Server()));
    }

// 2nd phase construct for sessions - called by the CServer framework
void CMemScanServSession::CreateL()
    {
    TRACES( RDebug::Print(_L("MemScanServ: CMemScanServSession::CreateL")); )
    Server().AddSession();
        
    // Create a transfer buffer
    iTransferBuffer = CBufFlat::NewL(KMemScanServTransferBufferExpandSize);
    }

CMemScanServSession::~CMemScanServSession()
    {
    TRACES( RDebug::Print(_L("MemScanServ: CMemScanServSession::~CMemScanServSession")); )
    
    
    delete iTransferBuffer;
    delete iMseng;
    
    
    iEventBuffer.Close();
    Server().DropSession();
    }


// Handle a client request.
// Leaving is handled by CMemScanServSession::ServiceError() which reports
// the error code to the client
void CMemScanServSession::ServiceL(const RMessage2& aMessage)
    {
    TRACES( RDebug::Print(_L("MemScanServ: CMemScanServSession::ServiceL; %d"),aMessage.Function()); )
    switch (aMessage.Function())
        {
        case EMemScanPrepareDataGroups:
            {
            PrepareDataGroupsL( aMessage );
            break;
            }
        case EMemScanGetDataGroups:
            {
            GetDataGroupsL( aMessage );
            break;
            }            
        case EMemScanStartScan:
            {
            MemScanL( aMessage );
            break;
            }
        case EMemScanPrepareScanResults:
            {
            PrepareScanResultsL( aMessage );
            break;
            }
        case EMemScanGetScanResults:
            {
            GetScanResultsL( aMessage );
            break;
            }                       
        case EMemScanRequestScanEvents:
            {
            RequestScanEventsL( aMessage );
            break;
            }
        case EMemScanRequestScanEventsCancel:
            {
            RequestScanEventsCancel( aMessage );
            break;
            }
        case EMemScanInProgress:
            {
            ScanInProgress( aMessage );
            break;
            }

        default:
            {
            TRACES( RDebug::Print(_L("MemScanServ: CMemScanServSession::ServiceL; %d"),aMessage.Function()); )
            PanicClient(aMessage,EPanicIllegalFunction);
            break;
            }
            
        }
    }

// Handle an error from CMemScanServSession::ServiceL()
void CMemScanServSession::ServiceError(const RMessage2& aMessage,TInt aError)
    {
    TRACES( RDebug::Print(_L("MemScanServ: CMemScanServSession::ServiceError %d"),aError); )
    CSession2::ServiceError(aMessage,aError);
    }


// ***************************************************************************
// Internal utility functions
// ***************************************************************************


// ---------------------------------------------------------------------------
// CMemScanServSession::PrepareDataGroupsL()
//
// ---------------------------------------------------------------------------
void CMemScanServSession::PrepareDataGroupsL(const RMessage2& aMessage)
    {
    // Create scan engine if it does not exist
    if(!iMseng)
        {
        iMseng = CMseng::NewL(*this);
        }
    
    // Get data group name array
    CDesCArray* dataGroupArray = iMseng->DataGroupsL();
    CleanupStack::PushL(dataGroupArray);
   
   
    // *** Start externalizing the data group array to transfer buffer

    // Clear the buffer
    iTransferBuffer->Reset();

    // Set buffer for the stream
    RBufWriteStream stream(*iTransferBuffer);
    CleanupClosePushL(stream);
    
    // Write number of fields in array to stream 
    TInt count = dataGroupArray->MdcaCount();
    stream.WriteInt32L(count);
    
    // Write each field in array to stream
    for(TInt i=0; i<count; i++)
        {
        TInt length = dataGroupArray->MdcaPoint(i).Length();
        stream.WriteInt32L(length); // writes datagroup name length to stream
        const TPtrC group = dataGroupArray->MdcaPoint(i);
        stream << group; // writes one datagroup to stream
        }
    
    stream.CommitL();
    CleanupStack::PopAndDestroy(&stream);
    CleanupStack::PopAndDestroy(dataGroupArray);
    
    // *** externalizing done
    
    
    // Write the size of transfer buffer back to client
    TPckgBuf<TInt> size(iTransferBuffer->Size());
    aMessage.WriteL(0, size);
    
    // complete the message
    aMessage.Complete( KErrNone );
    }


// ---------------------------------------------------------------------------
// CMemScanServSession::GetDataGroupsL()
//
// ---------------------------------------------------------------------------
void CMemScanServSession::GetDataGroupsL(const RMessage2& aMessage)
    {
    // Get the prepared data groups
    aMessage.WriteL( KMesArg0, iTransferBuffer->Ptr(0)); 
        
    aMessage.Complete( KErrNone );
    }
    
// ---------------------------------------------------------------------------
// CMemScanServSession::PrepareScanResultsL()
//
// ---------------------------------------------------------------------------
void CMemScanServSession::PrepareScanResultsL(const RMessage2& aMessage)
    {
    // Get scan results from server
    CArrayFix<TInt64>* resultArray = iMseng->ScanResultL();
    CleanupStack::PushL(resultArray);

    // *** Start externalizing the result array to transfer buffer
    
    // Clear the buffer
    iTransferBuffer->Reset();
        
    // Set buffer for the stream
    RBufWriteStream stream(*iTransferBuffer);
    CleanupClosePushL(stream);
    
    // Write number of fields in array to stream 
    TInt count = resultArray->Count();
    stream.WriteInt32L(count);    
    
    // Write each field in array to stream
    for(TInt i=0; i<count; i++)
        {
        const TInt64 result = resultArray->At(i);
        stream << result; // writes one data result to stream
        }
    
    stream.CommitL();
    CleanupStack::PopAndDestroy(&stream);
    CleanupStack::PopAndDestroy(resultArray);
    
    // *** externalizing done
    
    
    // Write the size of transfer buffer back to client
    TPckgBuf<TInt> size(iTransferBuffer->Size());
    aMessage.WriteL(0, size);    
    
    
    aMessage.Complete( KErrNone );    
    }


// ---------------------------------------------------------------------------
// CMemScanServSession::GetScanResultsL()
//
// ---------------------------------------------------------------------------
void CMemScanServSession::GetScanResultsL(const RMessage2& aMessage)
    {
    // Get the prepared scan results
    aMessage.WriteL( KMesArg0, iTransferBuffer->Ptr(0));    
    
    aMessage.Complete( KErrNone ); 
    }


// ---------------------------------------------------------------------------
// CMemScanServSession::MemScanL()
//
// ---------------------------------------------------------------------------
void CMemScanServSession::MemScanL(const RMessage2& aMessage)
    {
    TRACES( RDebug::Print(_L("MemScanServ: CMemScanServSession::ScanL")); )
    
     // Get the first integer parameter of message     
    TDriveNumber drive = TDriveNumber(aMessage.Int0()); 
        
    iMseng->ScanL( drive );
    aMessage.Complete( KErrNone );
    }


// ---------------------------------------------------------------------------
// CMemScanServSession::RequestScanEventsL()
//
// ---------------------------------------------------------------------------
void CMemScanServSession::RequestScanEventsL(const RMessage2& aMessage)
    {
    if  ( iScanEventMessage.IsNull() ) 
        {
        // We want check that the client hasn't requested scan events
        // twice in a row. The client is only allowed to have one 
        // scan event request outstanding at any given time.
        //
        // Since the iScanEventMessage was null (i.e. its not been
        // initialised) then its safe to store the client's message
        // for completion later on when the scan engine has a real event.
        
        // Save the clients message for later until we receive an
        // event callback from the scan engine.
        iScanEventMessage = aMessage;

        // If we have at least one event ready to send to the client, then
        // we deliver it to the client immediately. This could be possible
        // if the client is slow to process an earlier event.
        const TBool haveAtLeastOneEventPending = IsEventReady();
        if  ( haveAtLeastOneEventPending )
            {
            // We must deliver the oldest event to the client.
            DeliverOldestEventToClientL(); // this will complete aMessage immediately.
            }
        }
    else
        {
        // The client has already asked for scan events as we still
        // have an existing (valid) iScanEventMessage object.
        //
        // This would imply a programming error in the client code
        // so we punish the client by panicking it.
        aMessage.Panic( KMemScanServerPanicCategory, EMemScanServerPanicRequestedScanEventsTwice );
        }
    }


// ---------------------------------------------------------------------------
// CMemScanServSession::RequestScanEventsCancel()
//
// ---------------------------------------------------------------------------
void CMemScanServSession::RequestScanEventsCancel(const RMessage2& aMessage)
    {
    // We only are able to cancel a client request if the client actually
    // requested something.
    // We can check whether a request is pending by using the IsNull method
    // on our outstanding request object ("iScanEventMessage").
    if  ( iScanEventMessage.IsNull() == EFalse ) 
        {
        // The client has made a request, and we need to cancel it.
        iScanEventMessage.Complete( KErrCancel );
        }
        
        
    // If the client wants to cancel events, we should also empty
    // the event buffer.
    iEventBuffer.Reset();
    
    aMessage.Complete( KErrNone );
    }


// ---------------------------------------------------------------------------
// CMemScanServSession::ScanInProgress()
//
// ---------------------------------------------------------------------------
void CMemScanServSession::ScanInProgress(const RMessage2& aMessage)
    {
    TBool scanInProgress = iMseng->ScanInProgress();
    aMessage.Complete(static_cast<TInt> (scanInProgress));
    }



// From MMsengUIHandler:
// ===========================================================================

// ---------------------------------------------------------------------------
// CMemScanServSession::StartL()
// ---------------------------------------------------------------------------
void CMemScanServSession::StartL()
    {
    SendEventToClientL( EMemScanEventScanningStarted );
    }

// ---------------------------------------------------------------------------
// CMemScanServSession::QuitL()
// ---------------------------------------------------------------------------
void CMemScanServSession::QuitL(TInt aReason)
    {
    SendEventToClientL( EMemScanEventScanningFinished, aReason );
    }

// ---------------------------------------------------------------------------
// CMemScanServSession::Error()
// ---------------------------------------------------------------------------
void CMemScanServSession::ErrorL(TInt aError)
    {
    SendEventToClientL( EMemScanEventScanningError, aError );
    }

// ===========================================================================



    
// ---------------------------------------------------------------------------
// CMemScanServSession::SendEventToClientL()
//
// ---------------------------------------------------------------------------
void CMemScanServSession::SendEventToClientL( TMemScanEvent aEventType,
                                              TInt aError )
    {
    // We need to tell the client about the event that has taken place. 
    // The client event API expects to receive the event type, i.e. what 
    // kind of "thing" just happened, and also any associated error value 
    // (e.g. "Nothing went wrong" or, "we ran out of memory").
    
    AddNewEventToBufferL( aEventType, aError );
    DeliverOldestEventToClientL();
    }

// ---------------------------------------------------------------------------
// CMemScanServSession::AddNewEventToBufferL()
//
// ---------------------------------------------------------------------------
void CMemScanServSession::AddNewEventToBufferL( TMemScanEvent aEventType,
                                                TInt aError )
    {
    TMemScanEventPackage event;
    event.iEvent = aEventType;
    event.iError = aError;
    
    // Add the event to the event buffer. We will send this event to the 
    // client when the client is ready to accept it.
    iEventBuffer.AppendL( event );
    }

// ---------------------------------------------------------------------------
// CMemScanServSession::IsEventReady()
//
// ---------------------------------------------------------------------------
TBool CMemScanServSession::IsEventReady() const
    {
    // Returns whether we have at least one event in the buffer ready to send
    // to the client.
    const TInt count = iEventBuffer.Count();
    return ( count > 0 );
    }


// ---------------------------------------------------------------------------
// CMemScanServSession::DeliverOldestEventToClientL()
//
// ---------------------------------------------------------------------------
void CMemScanServSession::DeliverOldestEventToClientL()
    {
    // Fetch the oldest event from the buffer and deliver it
    // to the client.
    if  ( iScanEventMessage.IsNull() == EFalse && IsEventReady() )
        {
        // This next block of code converts the error number to look like
        // a descriptor, since this is the only way of writing to the
        // client's address space. 
        //
        // We check that the client actually requested scan events before
        // we try and write to its address space. If we don't do this
        // then the kernel will panic our code with KERN-SVR 0
        // ("you're trying to use a null message object")
        const TMemScanEventPackage& event = iEventBuffer[ 0 ];
        
        TPckgC<TInt> associatedErrorAsDescriptor( event.iError );
        iScanEventMessage.WriteL( 0, associatedErrorAsDescriptor );
        
        // Now that we have written the error value, its safe to complete
        // the clients asynchronous request which will end up calling
        // the client's RunL method.
        iScanEventMessage.Complete( event.iEvent );

        // We've delivered the oldest event to the client, so now 
        // its safe to discard it.
        iEventBuffer.Remove( 0 );
        }
    }
 

// End of File
