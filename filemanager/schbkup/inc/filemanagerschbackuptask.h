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
* Description:  Handles file manager scheduled backup task start
*
*/


#ifndef C_FILEMANAGERSCHBACKUPTASK_H
#define C_FILEMANAGERSCHBACKUPTASK_H


// INCLUDE FILES
#include <e32base.h>
#include "filemanagerschobserver.h"
#include "fmsystemstatemonitor.h"

// FORWARD DECLARATIONS
class CScheduledTask;
class CFileManagerSchSubscriber;
class CRepository;


// CLASS DECLARATION
/**
 *  This class handles scheduled backup task start functionality
 *
 *  @since S60 3.1
 */
class CFileManagerSchBackupTask : public CActive,
                                  public MFileManagerSchObserver,
                                  public MFmSystemStateMonitorObserver
    {

public:
    /**
     * Two-phased constructor.
     */
    static CFileManagerSchBackupTask* NewL( const CScheduledTask& aTask );

    /**
     * Destructor
     */
    ~CFileManagerSchBackupTask();

private: // From CActive
    void RunL();

    void DoCancel();

    TInt RunError( TInt aError );

private: // From FileManagerSchObserver
    void NotifyKeyChangeOrTimeoutL(
        const TUid& aCategory,
        const TUint aKey,
        const TBool aTimeout );

public: // From MFmSystemStateMonitorObserver
    /**
     * Called when the system state changed  
     * @since 5.2
     */
    void SystemStateChangedEvent();
    
private: // New methods
    /**
     * Constructors
     */
    CFileManagerSchBackupTask();

    void ConstructL( const CScheduledTask& aTask );

    /**
     * Starts backup task start procedure
     */
    void StartL();

    /**
     * Exists backup task start procedure
     */
    void Exit();

    /**
     * Retries backup task start procedure
     */
    void Retry();

    /**
     * Starts file manager to run backup
     */
    void StartFileManagerL();

    /**
     * Checks is backup required
     */
    TBool CheckBackupRequired();

    /**
     * Checks phone state is it ok to start backup
     */
    TBool CheckPhoneStateL();

private: // Data
    /**
     * Backup weekday, given from originator
     */
    TInt iDay;

    /**
     * Subscriber to observe phone and backup start states
     * Own.
     */
    CFileManagerSchSubscriber* iSubscriber;

    /**
     * Pointer to file manager settings in CenRep
     * Own.
     */
    CRepository* iCenRep;

    /**
     * Start attempts left
     */
    TInt iAttemptsLeft;
    
    /**
     * Own: Pointer to system state monitor   
     */
    CFmSystemStateMonitor* iSystemStateMonitor;

    };


#endif // C_FILEMANAGERSCHBACKUPTASK_H

// End of file
