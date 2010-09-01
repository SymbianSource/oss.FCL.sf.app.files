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
* Description:  Wraps task scheduler functionality
*
*/



#ifndef C_FILEMANAGERTASKSCHEDULER_H
#define C_FILEMANAGERTASKSCHEDULER_H


//  INCLUDES
#include <e32base.h>
#include <csch_cli.h>


// FORWARD DECLARATIONS
class CFileManagerEngine;


// CLASS DECLARATION
/**
 *  This class wraps task scheduler functionality
 *
 *  @since S60 3.1
 */
class CFileManagerTaskScheduler : public CBase
    {

public:  // New functions
    /**
     * Two-phased constructor.
     */
    static CFileManagerTaskScheduler* NewL(
        CFileManagerEngine& aEngine );

    /**
     * Destructor.
     */
    ~CFileManagerTaskScheduler();

    /**
     * Enables and disables backup schedule
     */
    void EnableBackupScheduleL( const TBool aEnable );

private:
    CFileManagerTaskScheduler(
        CFileManagerEngine& aEngine );

    void ConstructL();

    void CreateScheduleL();

    void DeleteScheduleL();

private: // Data
    /**
     * Reference to file manager engine
     * Not own.
     */
    CFileManagerEngine& iEngine;

    /**
     * Handle to task scheduler
     */
    RScheduler iScheduler;

    /**
     * Handle to schedule
     */
    TInt iScheduleHandle;

    };

#endif // C_FILEMANAGERBACKUPSCHEDULER_H   
            
// End of File
