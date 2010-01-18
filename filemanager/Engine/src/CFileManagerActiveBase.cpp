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


// INCLUDES
#include <e32std.h>
#include <AknWaitNoteWrapper.h>
#include "CFileManagerActiveBase.h"
#include "CFileManagerThreadWrapper.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerActiveBase::CFileManagerActiveBase
// -----------------------------------------------------------------------------
//
CFileManagerActiveBase::CFileManagerActiveBase() :
        iResult( KErrCancel )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveBase::~CFileManagerActiveBase
// -----------------------------------------------------------------------------
//
CFileManagerActiveBase::~CFileManagerActiveBase()
    {
    iCanceled = ETrue;
    delete iThreadWrapper;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveBase::BaseConstructL
// -----------------------------------------------------------------------------
//
void CFileManagerActiveBase::BaseConstructL()
    {
    iThreadWrapper = CFileManagerThreadWrapper::NewL();
    User::LeaveIfError( iThreadWrapper->StartThread(
        *this,
        MFileManagerThreadFunction::ENotifyFinished,
        EPriorityNormal ) );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveBase::StepL
// -----------------------------------------------------------------------------
//
void CFileManagerActiveBase::StepL()
    {
    // Just do nothing. All processing is done by ThreadFunctionL().
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveBase::IsProcessDone
// -----------------------------------------------------------------------------
//
TBool CFileManagerActiveBase::IsProcessDone() const
    {
    return iDone;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveBase::DialogDismissedL
// -----------------------------------------------------------------------------
//
void CFileManagerActiveBase::DialogDismissedL( TInt aButtonId )
    {
    if ( aButtonId == EAknSoftkeyCancel )
        {
        CancelThreadFunction();
        iCanceled = ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveBase::ThreadStepL
// -----------------------------------------------------------------------------
//
void CFileManagerActiveBase::ThreadStepL()
    {
    ThreadFunctionL( iCanceled );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveBase::IsThreadDone
// -----------------------------------------------------------------------------
//
TBool CFileManagerActiveBase::IsThreadDone()
    {
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveBase::NotifyThreadClientL
// -----------------------------------------------------------------------------
//
void CFileManagerActiveBase::NotifyThreadClientL(
        TNotifyType /*aType*/, TInt aValue)
    {
    iResult = aValue;
    iDone = ETrue;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveBase::Result
// -----------------------------------------------------------------------------
//
TInt CFileManagerActiveBase::Result() const
    {
    return iResult;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveBase::CancelThreadFunction
// -----------------------------------------------------------------------------
//
void CFileManagerActiveBase::CancelThreadFunction()
    {
    }

// End of File
