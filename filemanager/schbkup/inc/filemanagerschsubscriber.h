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
* Description:  Handles P&S and CenRep notifications
*
*/


#ifndef C_FILEMANAGERSCHSUBSCRIBER_H
#define C_FILEMANAGERSCHSUBSCRIBER_H


// INCLUDES
#include <e32base.h>
#include <e32property.h>


// FORWARD DECLARATIONS
class MFileManagerSchObserver;
class CRepository;


// CLASS DECLARATION
/**
 *  This class handles P&S and CenRep notifications
 *
 *  @since S60 3.1
 */
class CFileManagerSchSubscriber : public CActive
    {

public:
    /**  Type of subscribed notifications */
    enum TType
        {
        ESubscribePS = 0,
        ESubscribeCR
        };

    /**
     * Two-phased constructor.
     */
    static CFileManagerSchSubscriber* NewL(
        MFileManagerSchObserver& aObserver,
        const TUid& aCategory,
        const TUint aKey,
        const TType aType,
        const TInt aTimeoutSecs );

    /**
     * Destructor
     */
    ~CFileManagerSchSubscriber();

private:  // New methods
    /**
     * Constructors
     */
    CFileManagerSchSubscriber(
        MFileManagerSchObserver& aObserver,
        const TUid& aCategory,
        const TUint aKey,
        const TType aType,
        const TInt aTimeoutSecs );

    void ConstructL();

    /**
     * Subscribes notification
     */
    void SubscribeL();

    /**
     * Timeout callback
     * @param aPtr Pointer to subcriber
     * @return System wide error code
     */
    static TInt TimeoutCB( void* aPtr );

    /**
     * Handles timeout
     */
    void Timeout();

private: // From CActive
    void RunL();
    
    TInt RunError( TInt aError );

    void DoCancel();

private: // Data
    /**
     * Reference to observer interface
     * Not own.
     */
    MFileManagerSchObserver& iObserver;

    /**
     * Pointer to CenRep
     * Own.
     */
    CRepository* iCenRep;

    /**
     * Handle to PS
     */
    RProperty iProperty;

    /**
     * Category Uid
     */
    TUid iCategory;

    /**
     * Key id
     */
    TUint iKey;

    /**
     * Subscriber type
     */
    TType iType;

    /**
     * Timeout seconds
     */
    TInt iTimeoutSecs;

    /**
     * Timer for timeout
     * Own.
     */
    CPeriodic* iTimer;

    /**
     * Indicates timeout status
     */
    TBool iTimeout;

    };

#endif // C_FILEMANAGERSCHSUBSCRIBER_H

// End of File
