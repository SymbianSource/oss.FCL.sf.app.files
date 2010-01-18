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


#ifndef __MEMSCANSERV_H__
#define __MEMSCANSERV_H__

// SYSTEM INCLUDES
#include <e32base.h>

// USER INCLUDES
#include "memscanclientserver.h"
#include "msenguihandler.h"
#include "mseng.h"



// --------------------------------------------------------------------------
// Server's policy here
// --------------------------------------------------------------------------

//Total number of ranges
const TUint KMemScanServRangeCount = 2;

//Definition of the ranges of IPC numbers
const TInt KMemScanServRanges[KMemScanServRangeCount] = 
        {
        0,  // ECapabilityReadUserData
            /*
           	0   EMemScanStartScan,
	        1   EMemScanPrepareDataGroups,
	        2   EMemScanGetDataGroups,
	        3   EMemScanInProgress,
	        4   EMemScanRequestScanEvents,
	        5   EMemScanRequestScanEventsCancel,
	        6	EMemScanPrepareScanResults,
	        7   EMemScanGetScanResults
            */
            
        8   // ENotSupported - Non implemented function end of range check
        }; 

//Policy to implement for each of the above ranges        
const TUint8 KMemScanServElementsIndex[KMemScanServRangeCount] = 
        {
        0,	                          // 0th range: Policy 0 in Elements
        CPolicyServer::ENotSupported  // 1st range: Out of range IPC
        };

//Specific capability checks
const CPolicyServer::TPolicyElement KMemScanServElements[] = 
        {
        // Policy 0: Fail call if ReadUserData not present
        {_INIT_SECURITY_POLICY_C1(ECapabilityReadUserData), CPolicyServer::EFailClient}
        };

//Package all the above together into a policy
const CPolicyServer::TPolicy KMemScanServPolicy =
        {
        CPolicyServer::EAlwaysPass, // All connect attempts should pass
        KMemScanServRangeCount,	    // Number of ranges                                   
        KMemScanServRanges,	        // Ranges array
        KMemScanServElementsIndex,	// Elements<->Ranges index
        KMemScanServElements,		// Array of elements
        };



// --------------------------------------------------------------------------
// Server's panic codes here
// --------------------------------------------------------------------------
enum TMemScanServPanic
	{
	EPanicGeneral,
	EPanicIllegalFunction
	};

void PanicClient(const RMessagePtr2& aMessage,TMemScanServPanic aPanic);


// --------------------------------------------------------------------------
// Auto shutdown class (standard pattern)
// --------------------------------------------------------------------------
class CShutdown : public CTimer
	{
	enum TMemScanShutdown
	    {
	    EMemScanServShutdownDelay=0x200000
	    };
public:
	inline CShutdown();
	inline void ConstructL();
	inline void Start();
private:
	void RunL();
	};

// --------------------------------------------------------------------------
// Server
// --------------------------------------------------------------------------

class CMemScanServ : public CPolicyServer
	{
public:
	static CServer2* NewLC();
	void AddSession();
	void DropSession();
private:
	CMemScanServ();
	void ConstructL();
	CSession2* NewSessionL( const TVersion& aVersion, 
                            const RMessage2& aMessage) const;
                            
private:
	TInt iSessionCount;
	CShutdown iShutdown;
	};



// --------------------------------------------------------------------------
// Event object
// --------------------------------------------------------------------------
struct TMemScanEventPackage
    {
    TMemScanEvent iEvent;
    TInt iError;
    };


// --------------------------------------------------------------------------
// Session
// --------------------------------------------------------------------------
class CMemScanServSession : public CSession2, public MMsengUIHandler
	{
public:
	CMemScanServSession();
	void CreateL();
 private:
	~CMemScanServSession();
	inline CMemScanServ& Server();
	void ServiceL(const RMessage2& aMessage);
	void ServiceError(const RMessage2& aMessage,TInt aError);


private: // Client <-> server functions
	void MemScanL(const RMessage2& aMessage);
    void RequestScanEventsL(const RMessage2& aMessage);
    void RequestScanEventsCancel(const RMessage2& aMessage);
    void ScanInProgress(const RMessage2& aMessage);
    void PrepareDataGroupsL(const RMessage2& aMessage);
    void GetDataGroupsL(const RMessage2& aMessage);
    void PrepareScanResultsL(const RMessage2& aMessage);
    void GetScanResultsL(const RMessage2& aMessage);
       
private: // Internal utility functions
    void SendEventToClientL( TMemScanEvent aEventType, TInt aError=KErrNone );
    void AddNewEventToBufferL( TMemScanEvent aEventType, TInt aError);
    TBool IsEventReady() const;
    void DeliverOldestEventToClientL();
    
private: //From MMsengUIHandler
    void StartL();
    void QuitL(TInt aReason);
    void ErrorL(TInt aError);

private: // Member variables    
    CMseng* iMseng;
    RMessagePtr2 iScanEventMessage;
    RArray< TMemScanEventPackage > iEventBuffer;
    CBufBase* iTransferBuffer;
	};



#endif      // __MEMSCANSERV_H__
            
// End of File