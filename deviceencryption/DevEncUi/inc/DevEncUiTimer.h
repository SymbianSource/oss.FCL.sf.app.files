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
* Description:  
*
*/

#ifndef DEVENCUITIMER_H_
#define DEVENCUITIMER_H_

// INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS

class MDevEncUiTimerCallback
    {
    public:
        virtual void Timeout()=0;
    };

class CDevEncUiTimer : public CTimer
    {
    public:
        static CDevEncUiTimer* NewL( MDevEncUiTimerCallback* aCallback );
        ~CDevEncUiTimer();

        void StartAppL( const TUint32& aUid );

    protected:
        // from CActive
        void RunL();
        void DoCancel();
        TInt RunError( TInt aError );

    private:
    // Functions
        CDevEncUiTimer( MDevEncUiTimerCallback* aCallback );

        void ConstructL();

    // Data
        MDevEncUiTimerCallback* iCallback; // parent
    };
    
#endif /*DEVENCUITIMER_H_*/
