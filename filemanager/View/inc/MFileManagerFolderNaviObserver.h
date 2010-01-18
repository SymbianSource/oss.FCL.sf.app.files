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
* Description:  Observer for folder navigation events
*
*/


#ifndef M_FILEMANAGERFOLDERNAVIOBSERVER_H
#define M_FILEMANAGERFOLDERNAVIOBSERVER_H


//  INCLUDES
#include <e32base.h>


// CLASS DECLARATION
/**
 *  This class observes folder navigation notifications
 *
 *  @since S60 3.1
 */
class MFileManagerFolderNaviObserver
    {

public:
    // Navigation event types
    enum TNaviEvent
        {
        ENaviTapDown = 0,
        ENaviTapUp,
        ENaviLongTap
        };

    /**
     * Handles folder navigation notification
     *
     * @since S60 3.1
     * @param aEvent Navigation event type
     * @param aValue Navigation event related value
     */
    virtual void HandleFolderNaviEventL(
        TNaviEvent aEvent, TInt aValue ) = 0;

    };

#endif // M_FILEMANAGERFOLDERNAVIOBSERVER_H   
            
// End of File
