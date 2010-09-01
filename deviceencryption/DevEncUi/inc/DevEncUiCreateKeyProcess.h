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
* Description:  Shows a progress notification during the creation of keys.
*
*/

#ifndef DEVENCUI_CREATEKEYPROCESS_H_
#define DEVENCUI_CREATEKEYPROCESS_H_

#include <AknWaitNoteWrapper.h>
#include <e32base.h>

class CDevEncUiCreateKeyProcess : public CBase,
                                  public MAknBackgroundProcess

    {
    public:
        virtual ~CDevEncUiCreateKeyProcess();
        static TBool RunLD();

    private:
        CDevEncUiCreateKeyProcess();
        void ProcessFinished();
        void DialogDismissedL( TInt aButtonId );
        TBool IsProcessDone() const;
        void StepL();

    // Data
        TInt iCount;
    };

#endif /*DEVENCUI_CREATEKEYPROCESS_H_*/
