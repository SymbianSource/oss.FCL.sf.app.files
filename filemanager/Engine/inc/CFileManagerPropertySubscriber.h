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
* Description:  Subscriber (Publish & Subscribe)
*
*/



#ifndef CFILEMANAGERPROPERTYSUBSCRIBER_H
#define CFILEMANAGERPROPERTYSUBSCRIBER_H

// INCLUDES
#include <e32base.h>
#include <e32property.h>
#include "MFileManagerPropertyObserver.h"

// CLASS DECLARATION
/**
*  This class provides P&S event subscriber
*
*  @lib FileManagerEngine.lib
*  @since 3.1
*/
NONSHARABLE_CLASS(CFileManagerPropertySubscriber) : public CActive
{
    public:
        /**
        * Two-phased constructor.
        */
        static CFileManagerPropertySubscriber* NewL(
            MFileManagerPropertyObserver& aObserver,
            const TUid& aCategory,
            const TUint aKey );

        /**
        * Destructor.
        */
        ~CFileManagerPropertySubscriber();

    private:
         CFileManagerPropertySubscriber(
            MFileManagerPropertyObserver& aObserver,
            const TUid& aCategory,
            const TUint aKey );

        void ConstructL();

        void Subscribe();

    private: // from CActive
        void RunL();
    
        TInt RunError( TInt aError );

        void DoCancel();

    private:
        // Ref: Reference to observer interface
        MFileManagerPropertyObserver& iObserver;
        // Own: PS handle
        RProperty iProperty;
        // Own: PS category Uid
        TUid iCategory;
        // Own: PS key id
        TUint iKey;
};

#endif // CFILEMANAGERPROPERTYSUBSCRIBER_H

// End of File
