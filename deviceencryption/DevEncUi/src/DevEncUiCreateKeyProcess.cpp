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
* Description:  Implementation of CDevEncUiCreateKeyProcess.
*
*/

#include "DevEncLog.h"
#include "DevEncUiCreateKeyProcess.h"
#include <devencui.rsg>

CDevEncUiCreateKeyProcess::CDevEncUiCreateKeyProcess()
    {
    iCount = 0;
    }

CDevEncUiCreateKeyProcess::~CDevEncUiCreateKeyProcess()
    {
    }

void CDevEncUiCreateKeyProcess::ProcessFinished()
    {
    }

void CDevEncUiCreateKeyProcess::DialogDismissedL( TInt /*aButtonId*/ )
    {
    }

//TInt CDevEncUiCreateKeyProcess::CycleError( TInt aError )
//    {
//    return aError;
//    }

TBool CDevEncUiCreateKeyProcess::IsProcessDone() const
    {
    return ( iCount == 3 );
    }

void CDevEncUiCreateKeyProcess::StepL()
    {
    // Simulate processing
    User::After( 1000000 );
    iCount++;
    }

TBool CDevEncUiCreateKeyProcess::RunLD()
    {
    TBool returnvalue = EFalse;

    CDevEncUiCreateKeyProcess* process = new ( ELeave ) CDevEncUiCreateKeyProcess();
    CleanupStack::PushL( process );

    CAknWaitNoteWrapper* waitNoteWrapper =
       CAknWaitNoteWrapper::NewL();

    CleanupStack::PushL( reinterpret_cast<CBase*>( waitNoteWrapper ) );

    if ( waitNoteWrapper->ExecuteL( R_DEVENCUI_CREATING_KEY_WAITNOTE,
                                    *process,
                                    ETrue ) )
        {
        returnvalue = ETrue;
        }
    else //note was cancelled
        {
        returnvalue = EFalse;
        }

    CleanupStack::PopAndDestroy( waitNoteWrapper );
    CleanupStack::PopAndDestroy( process );
    return returnvalue;
    }
