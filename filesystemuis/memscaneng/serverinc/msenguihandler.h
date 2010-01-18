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
* Description: 
*     A pure virtual interface class used by the UI to handle events from  
*     the Memory Scan Engine. 
*
*/


#ifndef MMSENGUIHANDLER_H
#define MMSENGUIHANDLER_H

//  INCLUDES
#include <e32base.h>
#include <e32std.h>

// FORWARD DECLARATIONS
class CMsengInfoArray;

// CONSTANTS

// CLASS DECLARATION

/**
*  An interface class used by the engine to send information to the calling class.
*/
class MMsengUIHandler
    {
    public: // New functions

        /**
        * The engine tells that it has successfully initialized itself and 
        * started the scanning operation.
        */
        virtual void StartL()=0;

        /**
        * The last method called when the scanning is either finished or canceled.
        * @param aReason The quit reason, can be KErrNone or KErrCancel.
        */
        virtual void QuitL(TInt aReason)=0;

        /**
        * The engine notifies the calling class if some error has happened
        * @param aError Error code.
        */
        virtual void ErrorL(TInt aError)=0;
    };

#endif      // MMSENGUIHANDLER_H   
            
// End of File
