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


#ifndef __MEMSCANCLIENT_H__
#define __MEMSCANCLIENT_H__

#include <e32std.h>
#include <f32file.h>        // TDriveNumber
#include <badesca.h>        // Descriptor arrays

#include "memscanclientserver.h"

// ---------------------------------------------------------------------------
// Client
// ---------------------------------------------------------------------------

class RMemScanClient : public RSessionBase
	{
public:
    IMPORT_C RMemScanClient();
	IMPORT_C TInt Connect();
	IMPORT_C void Close();

public: // API	
	IMPORT_C TInt Scan(const TDriveNumber aDrive);
	IMPORT_C CDesCArray* DataGroupsL() const;
	IMPORT_C void RequestScanEvents( TInt& aError, TRequestStatus& aRequestStatus );
	IMPORT_C void RequestScanEventsCancel();
	IMPORT_C TBool ScanInProgress( ) const;
	IMPORT_C CArrayFix<TInt64>* ScanResultL() const;

private:
    TPtr8 iScanEventPackagePointer;
	};


#endif //__MEMSCANCLIENT_H__
