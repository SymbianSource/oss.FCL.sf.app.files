/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Background thread functionality wrapper
*
*/



#ifndef CFILEMANAGERTHREADWRAPPER_H
#define CFILEMANAGERTHREADWRAPPER_H


// INCLUDES
#include <e32base.h>
#include "MFileManagerThreadFunction.h"


// CLASS DECLARATION
/**
*  The class implements a background thread functionality wrapper
*
*  @lib FileManagerEngine.lib
*  @since 3.1
*/
NONSHARABLE_CLASS(CFileManagerThreadWrapper) : public CActive
    {
    public:
        static CFileManagerThreadWrapper* NewL();

        ~CFileManagerThreadWrapper();        

    public: // New functions
        /**
        * Starts background thread. Can be used only by client thread.
        * @since 3.1
        * @param aFunction Reference to background thread abstraction.
        * @param aPriority background thread priority
        * @param aNotify client notify flags
        *        see MFileManagerThreadFunction::TNotifyType
        * @return System wide error code
        */
        TInt StartThread(
            MFileManagerThreadFunction& aFunction,
            TUint aNotify,
            TThreadPriority aPriority );

        /**
        * Cancels background thread.
        * Background thread is exited when ongoing ThreadStepL is finished.
        * Can be used only by client thread.
        * @since 3.1
        */
        void CancelThread();

        /**
        * Checks if background thread has been canceled.
        * Can be used by both client and background threads.
        * E.g. checks can be done inside long running ThreadStepL.
        * @since 3.1
        * @return ETrue if canceled, otherwise EFalse
        */
        TBool IsThreadCanceled() const;

        /**
        * Resumes thread when called after error or step finished 
        * notification. Can only be used by client thread.
        * @since 3.2
        */
        void ResumeThread();

        /**
        * Checks if thread has been started
        * notification. Can only be used by client thread.
        * @since 3.2
        * @return ETrue if started, otherwise EFalse
        */
        TBool IsThreadStarted() const;

    private: // From CActive
        void RunL();

        TInt RunError( TInt aErr );

        void DoCancel();

    private:
        CFileManagerThreadWrapper();

        void ConstructL();

        static TInt ThreadFunction( TAny* ptr );

        void ThreadFunctionL();

        TInt NotifyClientAndWaitConfirm( TInt aErr );

        void DoNotifyL( TInt aErr );

        NONSHARABLE_CLASS(CNotifyObserver) : public CActive
            {
            public:
                static CNotifyObserver* NewL(
                    CFileManagerThreadWrapper& aWrapper );

                ~CNotifyObserver();

                void Activate();

                void Complete( RThread& aThread, TInt aResult );

            private: // From CActive
                void RunL();

                TInt RunError( TInt aErr );

                void DoCancel();

            private:
                CNotifyObserver( CFileManagerThreadWrapper& aWrapper );

            private:
                CFileManagerThreadWrapper& iWrapper;
            };

    private: // Data
        // Own: For synchronising client and background thread
        RSemaphore iSemaphore;
        // Own: Client thread id
        TThreadId iClientId;
        // Ref: Pointer to thread function
        MFileManagerThreadFunction* iFunction;
        // Own: Client notify type flags, Client writes and thread only reads
        TUint iNotify;
        // Own: Thread cancel indicator, Client writes and thread only reads
        TBool iCancel;
        // Own: Observer wrapper to notify client about thread events
        CNotifyObserver* iNotifyObserver;
        // Own: Thread resume pending indicator, Client reads and writes
        TBool iResumePending;
    };

#endif // CFILEMANAGERTHREADWRAPPER_H

// End of File
