/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     Memory Scan Event Receiver
*
*
*/

#ifndef CMEMSCANEVENTRECEIVER_H
#define CMEMSCANEVENTRECEIVER_H

// SYSTEM INCLUDES
#include <e32base.h>
#include <badesca.h>
#include <memscanclient.h>


class MMsengUIHandler;


NONSHARABLE_CLASS(CMemScanEventReceiver) : public CActive
    {
    public:
        static CMemScanEventReceiver* NewL( MMsengUIHandler& aUIHandler );
        ~CMemScanEventReceiver();

    private:
        CMemScanEventReceiver( MMsengUIHandler& aUIHandler );
        void ConstructL();
        
    public: // API
        CDesCArray* DataGroupsL() const; 
        CArrayFix<TInt64>* ScanResultL() const;
        TBool ScanInProgress() const;
        TInt ScanL(TDriveNumber aDrive);

    private: // From CActive
        void RunL();
        void DoCancel();
        TInt RunError( TInt aError );
        
    private: // Internal functions
        void RegisterForScanEvents();

    private: // Data members
        MMsengUIHandler& iUIHandler;
        RMemScanClient iScanClient;
        TInt iEventError;
    };

    
#endif // CMEMSCANEVENTRECEIVER_H