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
* Description:  Memory Scan Client/Server definitions
*
*/



#ifndef __MEMSCANCLIENTSERVER_H__
#define __MEMSCANCLIENTSERVER_H__


_LIT(KMemScanServName,"memscanserv");
_LIT(KMemScanServImg,"memscanserv");    // EXE name
_LIT(KMemScanServerPanicCategory, "MemScanServer");

const TUid KMemScanServUid3={0x10207386}; 

const TInt KMemScanServMajor = 1;
const TInt KMemScanServMinor = 0;
const TInt KMemScanServBuild = 0;

const TInt KMesArg0 = 0;

// Language specific buffer size is assumed to vary between 80-150
const TInt KMemScanServTransferBufferExpandSize = 50;

enum TMemScanEvent
    {
    EMemScanEventScanningStarted = 0,
    EMemScanEventScanningFinished,
    EMemScanEventScanningError
    };

// when modifying these, notice the server's policy    
enum TMemScanServMessages
    {
    EMemScanStartScan,
    EMemScanPrepareDataGroups,
    EMemScanGetDataGroups,
    EMemScanInProgress,
    EMemScanRequestScanEvents,
    EMemScanRequestScanEventsCancel,
    EMemScanPrepareScanResults,
    EMemScanGetScanResults
    };
    
enum TMemScanServerClientPanic
    {
    EMemScanServerPanicRequestedScanEventsTwice = 0,
    };

#endif// __MEMSCANCLIENTSERVER_H__
