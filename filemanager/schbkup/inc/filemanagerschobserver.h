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
* Description:  Observes subscribed notifications
*
*/


#ifndef M_FILEMANAGERSCHOBSERVER_H
#define M_FILEMANAGERSCHOBSERVER_H


// INCLUDES
#include <e32base.h>


// CLASS DECLARATION
/**
 *  This class observes subscribed notifications
 *
 *  @since S60 3.1
 */
class MFileManagerSchObserver
    {

public:
    /**
     * Handles notification
     *
     * @since S60 3.1
     * @param aCategory Subcribed category uid
     * @param aKey Subcribed key id
     * @param aTimeout Is triggered by timeout or actual event
     */
    virtual void NotifyKeyChangeOrTimeoutL(
        const TUid& aCategory,
        const TUint aKey,
        const TBool aTimeout ) = 0;

    };

#endif // M_FILEMANAGERSCHOBSERVER_H

// End of File
