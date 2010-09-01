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
* Description:  Scheduled backup result observer
*
*/



#ifndef M_FILEMANAGERSCHBACKUPOBSERVER_H
#define M_FILEMANAGERSCHBACKUPOBSERVER_H


//  INCLUDES
#include <e32base.h>


// CLASS DECLARATION
/**
 *  This class observes global scheduled backup notifications
 *
 *  @since S60 3.1
 */
class MFileManagerSchBackupObserver
    {

public:
    /**
     * Handles scheduled backup finished notification
     *
     * @since S60 3.1
     * @param aError Finished backup status
     */
    virtual void SchBackupFinishedL( TInt aError ) = 0;

    };

#endif // M_FILEMANAGERSCHBACKUPOBSERVER_H   
            
// End of File
