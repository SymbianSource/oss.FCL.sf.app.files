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
* Description:  Interface to handle async P&S property notifications
*
*/



#ifndef CFILEMANAGERPROPERTYOBSERVER_H
#define CFILEMANAGERPROPERTYOBSERVER_H

// INCLUDES
#include <e32base.h>


// CLASS DECLARATION
/**
*  Defines an interface for handling async P&S property changes.
*
*  @since 3.1
*/
class MFileManagerPropertyObserver
    {
    public:
        /**
        * Informs about property changes
        * @since 3.1
        * @param aCategory Property category Uid
        * @param aKey Property key id
        */
        virtual void PropertyChangedL(
            const TUid& aCategory, const TUint aKey ) = 0;
    };

#endif // CFILEMANAGERPROPERTYOBSERVER_H

// End of File
