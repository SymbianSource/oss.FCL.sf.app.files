/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
*     An interface class used by the scanning thread to send
*     events to the CMsengScanner instance that created the thread.
*
*/


#ifndef MSENGSCANOBSERVER_H
#define MSENGSCANOBSERVER_H

// CLASS REFERENCED
class CMsengScannerBase;

// CLASS DECLARATION

/**
*
*/
class MMsengScannerObserver
    {
    public:

        /**
        *
        */
        enum TScannerEvent
            {
            EScannerEventScanComplete = 0,
            EScannerEventScanError
            };

    public:
		
        /**
        *
        */
        virtual void HandleScannerEventL(
            TScannerEvent aEvent, const CMsengScannerBase& aScanner, TInt aRrror=KErrNone) = 0;
    };

#endif      // MSENGSCANOBSERVER_H
            
// End of File
