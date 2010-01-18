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
* Description:  Background thread abstraction
*
*/


#ifndef MFILEMANAGERTHREADFUNCTION_H
#define MFILEMANAGERTHREADFUNCTION_H


// INCLUDES
#include <e32base.h>


// CLASS DECLARATION
/**
*  Defines an interface for background thread abstraction.
*
*  @since 3.1
*/
class MFileManagerThreadFunction
    {
    public: // Background thread space executed callbacks
        
        /**
        * Inits background thread owned handles and data.
        * Executed always on background thread start.
        * @since 3.1
        */
        virtual void InitThreadL() {}

        /**
        * Releases background thread owned handles and data.
        * Executed always on background thread exit.
        * @since 3.1
        */
        virtual void ReleaseThread() {}

        /**
        * Runs background thread.
        * @since 3.1
        */
        virtual void ThreadStepL() = 0;

        /**
        * Checks is background thread done.
        * @since 3.1
        * @return ETrue if thread is done and should be finished,
        *         EFalse if ThreadStepL should be executed.
        */
        virtual TBool IsThreadDone() = 0;


    public: // Client thread space executed callbacks
        enum TNotifyType // Notify type flags
            {
            ENotifyNone = 0x0,
            ENotifyError = 0x1,
            ENotifyFinished = 0x2, // Notifies thread's death
            ENotifyStepFinished = 0x4 // Notifies step completion
            };
        /**
        * Notifies client about background thread events.
        * Background thread is stopped during notification and 
        * it is safe to modify shared data to handle errors etc.
        * @since 3.1
        * @param aType notification type
        * @param aValue notification value
        */
        virtual void NotifyThreadClientL(
            TNotifyType aType, TInt aValue ) = 0;

    };

#endif // MFILEMANAGERTHREADFUNCTION_H

// End of File
