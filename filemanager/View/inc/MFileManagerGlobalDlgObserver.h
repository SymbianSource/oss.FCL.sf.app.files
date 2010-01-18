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
* Description:  Global dialog result observer
*
*/


#ifndef M_FILEMANAGERGLOBALDLGOBSERVER_H
#define M_FILEMANAGERGLOBALDLGOBSERVER_H


//  INCLUDES
#include <e32base.h>


// CLASS DECLARATION
/**
 *  This class observes global dialog notifications
 *
 *  @since S60 3.1
 */
class MFileManagerGlobalDlgObserver
    {

public:
    /**
     * Handles global dialog result value
     *
     * @since S60 3.1
     * @param aDialogType Global dialog type
     * @param aDialogResult Global dialog result value
     */
    virtual void HandleGlobalDlgResult(
        TInt aDialogType,
        TInt aDialogResult ) = 0;

    };

#endif // M_FILEMANAGERGLOBALDLGOBSERVER_H   
            
// End of File
