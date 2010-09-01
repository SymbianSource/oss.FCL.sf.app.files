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
* Description:  Class for managing different types of memory on a 
*               very abstract level.
*
*/

#ifndef DEVENCUI_MEMORYENTITY_H
#define DEVENCUI_MEMORYENTITY_H

#include <e32base.h>
#include <eikenv.h>

#include "DevEnc.hrh"
#include "DevEncUiMemInfoObserver.h"
#include "DevEncDiskStatusObserver.h"

class CDevEncSession;
class CDevEncUiInfoObserver;

class CDevEncUiMemoryEntity : public CBase,
                              public MDiskStatusObserver
    {
    public:
        /**
         * Constructs the CDevEncUiMemoryEntity using the NewLC method, popping
         * the constructed object from the CleanupStack before returning it.
         *
         * @return The newly constructed CDevEncUiMemoryEntity
         */
    	static CDevEncUiMemoryEntity* NewL( CEikonEnv* aEikonEnv,
                                            TDevEncUiMemoryType aType );

        /**
         * Constructs the CDevEncUiMemoryEntity using the constructor and ConstructL
         * method, leaving the constructed object on the CleanupStack before returning it.
         *
         * @return The newly constructed CDevEncUiMemoryEntity
         */
    	static CDevEncUiMemoryEntity* NewLC( CEikonEnv* aEikonEnv,
                                             TDevEncUiMemoryType aType );

    	/**
         * Adds an observer to this object's list of observers.
         * @param aInfoObserver The observer to add
         * @leave Symbian error code
         */
        void AddObserverL( MDevEncUiMemInfoObserver* aInfoObserver );

        /**
         * Removes an observer from this object's list of observers.
         * Any errors are ignored.
         * @param aInfoObserver The observer to remove
         */
        void RemoveObserver( MDevEncUiMemInfoObserver* aInfoObserver );

        /**
         * Called periodically. Not intended for use by other classes.
         * @param aPtr Pointer to an instance of this class.
         */
        static TInt ProgressTick( TAny* aPtr );

        /**
         * Called periodically. Not intended for use by other classes.
         * @param aPtr Pointer to an instance of this class.
         */
        static TInt PollTick( TAny* aPtr );
        
        /**
        * Destructor.
        */
        virtual ~CDevEncUiMemoryEntity();

        /**
         * Starts the encryption of this memory.
         */
        void StartEncryptionL();

        /**
         * Starts the decryption of this memory.
         */
        void StartDecryptionL();

        void Cancel();

        /**
         * Gets the state of this memory entity.
         * @return the current state
         */
        TUint State() const;

        /**
         * Sends the current memory state information to all observers.
         * @return none
         */
        virtual void UpdateMemoryInfo();
        
        /*
         * From MDiskStatusObserver
         */
        void DiskStatusChangedL( TInt aStatus );

    private:
    // functions

        /**
         * C++ Constructor
         **/
        CDevEncUiMemoryEntity( CEikonEnv* aEikonEnv, TDevEncUiMemoryType aType );

        /**
         * Second-phase constructor
         **/
        virtual void ConstructL();

        /**
         * Sets the internal state and calls UpdateMemoryInfo()
         * @param aState the new state
         **/
        virtual void SetState( TUint aState );

        /**
         * Starts a timer to periodically update the memory state in the UI.
         * @param aInterval the polling interval
         **/
        void StartPolling( TTimeIntervalMicroSeconds32 aInterval );

        /**
         * Helper function, called by StartPolling.
         * Starts a timer to periodically update the memory state in the UI.
         * @param aInterval the polling interval
         **/
        void DoStartPollingL( TTimeIntervalMicroSeconds32 aInterval );

        /**
         * Fetches the encryption state and progress. Called periodically.
         **/
        virtual void CheckProgress();

        /**
         * Performs periodical processing of timeouts. Calls CheckProgress().
         **/
        virtual void DoProgressTick();

        /**
         * Performs periodical processing of timeouts.
         * Checks if the encryption status has changed.
         **/
        virtual void DoPollTickL();

        void RestoreAutolockSettings();

    // Data
        /** Not owned */
        CEikonEnv* iEikEnv;

        /** Owned */
        CDiskStatusObserver* iDiskStatusObserver;

        /** Owned */
        CPeriodic* iPeriodic;

        /** Owned */
        RArray<MDevEncUiMemInfoObserver*> iObservers;

        /** Owned */
        CDevEncSession* iSession;

        TUint iState;
        TUint iPrevState;
        const TDevEncUiMemoryType iType;
        TInt iPercentDone;
        TInt iPrevPercentDone;
    };

#endif	// DEVENCUI_MEMORYENTITY_H
