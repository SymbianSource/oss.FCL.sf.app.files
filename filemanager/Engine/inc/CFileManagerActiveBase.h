/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Wraps wait note and real work thread
*
*/



#ifndef CFILEMANAGERACTIVEBASE_H
#define CFILEMANAGERACTIVEBASE_H

// INCLUDES
#include <e32std.h>
#include <AknWaitNoteWrapper.h>
#include "MFileManagerThreadFunction.h"

// FORWARD DECLARATIONS
class CFileManagerThreadWrapper;

// CLASS DECLARATION
/**
* Class wraps wait note and real work thread to avoid long running steps 
* in the main thread that freezes the UI.
*
*  @lib FileManagerEngine.lib
*  @since 3.2
*/
NONSHARABLE_CLASS(CFileManagerActiveBase) : public CBase,
                              public MAknBackgroundProcess,
                              public MFileManagerThreadFunction
    {
    public:
        /**
        * Constructor for subclasses.
        */
        void BaseConstructL();

        /**
        * Destructor.
        */
        ~CFileManagerActiveBase();

        TInt Result() const;

    private: // From MAknBackgroundProcess
        void StepL();

        TBool IsProcessDone() const;

        void DialogDismissedL( TInt aButtonId );

    private: // From MFileManagerThreadFunction
        void ThreadStepL();

        TBool IsThreadDone();

        void NotifyThreadClientL( TNotifyType aType, TInt aValue );

    protected:
        /**
        * For subclasses to handle all processing in work thread
        */
        virtual void ThreadFunctionL( const TBool& aCanceled ) = 0;

        /**
        * For subclasses to request cancel in work thread
        */
        virtual void CancelThreadFunction();

    protected:
        /**
        * C++ default constructor.
        */
        CFileManagerActiveBase();

    private: // Data
        // Wraps work thread control. Own.
        CFileManagerThreadWrapper* iThreadWrapper;

        // Indicates if background process is done or not
        TBool iDone;

        // Indicates if thread is canceled
        TBool iCanceled;

        // Indicates the result
        TInt iResult;

    };

#endif      // CFILEMANAGERACTIVEBASE_H
            
// End of File

