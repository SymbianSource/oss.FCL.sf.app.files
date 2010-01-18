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
* Description:  Launches file manager scheduled backup task
*
*/



// INCLUDE FILES
#include <e32base.h>
#include <f32file.h>
#include <s32file.h>
#include <schtask.h>
#include "FileManagerDebug.h"
#include "filemanagerschbackuptask.h"


// CONSTANTS
_LIT_SECURE_ID( KTaskSchedulerSID, 0x10005399 );


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// StartSchTaskL
// ---------------------------------------------------------------------------
//  
LOCAL_C void StartSchTaskL( RFile& aTaskFile )
    {
    INFO_LOG( "StartSchTaskL()-Started" )

	// Create and install the active scheduler we need
	CActiveScheduler* scheduler = new ( ELeave ) CActiveScheduler();
	CleanupStack::PushL( scheduler );
	CActiveScheduler::Install( scheduler );

    // Get scheduled task parameters
    CFileStore* store = CDirectFileStore::FromLC( aTaskFile );
	RStoreReadStream instream;
	instream.OpenLC( *store, store->Root() );

    TInt count( instream.ReadInt32L() );
    if ( !count )
        {
        User::Leave( KErrNotFound );
        }

    // Create the task to be started
    CScheduledTask* task = CScheduledTask::NewLC( instream );
    CFileManagerSchBackupTask* fmTask =
        CFileManagerSchBackupTask::NewL( *task );
    CleanupStack::PopAndDestroy( task );
    CleanupStack::PopAndDestroy( &instream );
    CleanupStack::PopAndDestroy( store );

	// Start scheduler to handle the backup launch.
	// This call exists after backup launch has finished.
	CActiveScheduler::Start();

	// Cleanup the task and scheduler
	delete fmTask;
	CleanupStack::PopAndDestroy( scheduler );

	INFO_LOG( "StartSchTaskL()-Finished" )
    }


// ======== GLOBAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// E32Main
// ---------------------------------------------------------------------------
//   
GLDEF_C TInt E32Main() 
    {
    __UHEAP_MARK;

    INFO_LOG( "FileManagerSchBackup-E32Main()-Started" )

    TInt err( KErrPermissionDenied );

    // Check process creator and start scheduled task
	if( User::CreatorSecureId() == KTaskSchedulerSID )
		{
        CTrapCleanup* cleanupStack = CTrapCleanup::New();
        if ( cleanupStack )
            {
		    INFO_LOG( "FileManagerSchBkup-E32Main()-Adopt task file" )

            RFile file;
            err = file.AdoptFromCreator(
                TScheduledTaskFile::FsHandleIndex(),
                TScheduledTaskFile::FileHandleIndex() );
            if ( err == KErrNone )
                {
                TRAP( err, StartSchTaskL( file ) );
                file.Close();
                }
            delete cleanupStack;
            }
        else
            {
            err = KErrNoMemory;
            }
		}

    LOG_IF_ERROR1( err, "FileManagerSchBackup-E32Main()-Error=%d", err )

    INFO_LOG( "FileManagerSchBackup-E32Main()-Finished" )

    __UHEAP_MARKEND;

    return err;
    }

// End of file
