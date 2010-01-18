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
* Description:  Mixin class for notifying observers about user selections.
*
*/

#ifndef DEVENCUI_CONTAINEREVENTCALLBACK_H_
#define DEVENCUI_CONTAINEREVENTCALLBACK_H_

#include "DevEnc.hrh"

class MContainerEventCallback
    {
    public:
        virtual void EncryptionStatusChangeReq(
            TDevEncUiMemoryType aType ) = 0;
    };

#endif /*DEVENCUI_CONTAINEREVENTCALLBACK_H_*/
