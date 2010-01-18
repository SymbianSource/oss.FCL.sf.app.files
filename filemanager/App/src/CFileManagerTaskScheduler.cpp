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



// INCLUDE FILES
#include <apacmdln.h>
#include <FileManagerSchDefinitions.h>
#include <FileManagerDebug.h>
#include <CFileManagerEngine.h>
#include <CFileManagerBackupSettings.h>
#include <FileManagerPrivateCRKeys.h>
#include "CFileManagerTaskScheduler.h"


// CONSTANTS
const TInt KSchedulerPriority = 32;
const TInt KSchedulerGranularity = 1;
const TInt KSchedulerRepeat = -1; // Repeat until deleted
const TInt KSchedulerTaskId = 0;
const TInt KDayNumStringLen = 4;
const TInt KTaskInterval = 1;
const TInt KTaskValidity = 1;
_LIT( KDayNumStr, "%d" );


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerTaskScheduler::CFileManagerTaskScheduler
// ----------------------------------------------------------------------------
//
CFileManagerTaskScheduler::CFileManagerTaskScheduler(
        CFileManagerEngine& aEngine ) :
    iEngine( aEngine ),
    iScheduleHandle( KErrNotFound )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerTaskScheduler::NewL
// ----------------------------------------------------------------------------
//
CFileManagerTaskScheduler* CFileManagerTaskScheduler::NewL(
        CFileManagerEngine& aEngine )
    {
    CFileManagerTaskScheduler* self =
        new( ELeave ) CFileManagerTaskScheduler( aEngine );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerTaskScheduler::ConstructL
// ----------------------------------------------------------------------------
// 
void CFileManagerTaskScheduler::ConstructL()
    {
    FUNC_LOG

    User::LeaveIfError( iScheduler.Connect() );
    TFileName name( KSchBackupStarterExe );
    User::LeaveIfError( iScheduler.Register( name, KSchedulerPriority ) );

    // Try to get schedule handle of previously made schedule
    CArrayFixFlat< TSchedulerItemRef >* array = 
        new( ELeave ) CArrayFixFlat< TSchedulerItemRef >(
            KSchedulerGranularity );
    CleanupStack::PushL( array );
    if ( iScheduler.GetScheduleRefsL( *array, EAllSchedules ) == KErrNone )
        {
        if ( array->Count() )
            {
            iScheduleHandle = array->At( 0 ).iHandle;
            }
        }
    CleanupStack::PopAndDestroy( array );
    }

// ----------------------------------------------------------------------------
// CFileManagerTaskScheduler::~CFileManagerTaskScheduler
// ----------------------------------------------------------------------------
// 
CFileManagerTaskScheduler::~CFileManagerTaskScheduler()
    {
    FUNC_LOG
    
    iScheduler.Close();
    }

// ----------------------------------------------------------------------------
// CFileManagerTaskScheduler::CreateScheduleL
// ----------------------------------------------------------------------------
// 
void CFileManagerTaskScheduler::CreateScheduleL()
    {
    FUNC_LOG

    CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );

    // Create schedule entry and store schedule handle for later use
    CArrayFixFlat< TScheduleEntryInfo2 >* array =
        new ( ELeave ) CArrayFixFlat<
            TScheduleEntryInfo2 >( KSchedulerGranularity );
    CleanupStack::PushL( array );

    TTsTime tsTime( settings.Time(), EFalse );
    TScheduleEntryInfo2 entry(
        tsTime, EDaily, KTaskInterval, KTaskValidity );
    array->AppendL( entry );

    TSchedulerItemRef ref;
    User::LeaveIfError( iScheduler.CreatePersistentSchedule( ref, *array ) );
    CleanupStack::PopAndDestroy( array );
    iScheduleHandle = ref.iHandle;

    // Add weekday to task data if weekly backup
    HBufC* data = HBufC::NewLC( KDayNumStringLen );
    if ( settings.Scheduling() == EFileManagerBackupScheduleWeekly )
        {
        TPtr ptr( data->Des() );
        ptr.Format( KDayNumStr, settings.Day() );
        }

    // Create scheduled task entry and enable it
    TTaskInfo taskInfo;
    taskInfo.iName = KSchBackupTaskName;
    taskInfo.iPriority = KSchedulerPriority;
    taskInfo.iTaskId = KSchedulerTaskId;
    taskInfo.iRepeat = KSchedulerRepeat;
    User::LeaveIfError( iScheduler.ScheduleTask(
        taskInfo, *data, iScheduleHandle ) );
    CleanupStack::PopAndDestroy( data );
    User::LeaveIfError( iScheduler.EnableSchedule( iScheduleHandle ) );
    }

// ----------------------------------------------------------------------------
// CFileManagerTaskScheduler::DeleteScheduleL
// ----------------------------------------------------------------------------
// 
void CFileManagerTaskScheduler::DeleteScheduleL()
    {
    if ( iScheduleHandle != KErrNotFound )
        {
        INFO_LOG( "CFileManagerTaskScheduler::DeleteScheduleL-Delete" )
        iScheduler.DisableSchedule( iScheduleHandle );
        iScheduler.DeleteTask( KSchedulerTaskId );
        iScheduler.DeleteSchedule( iScheduleHandle );
        iScheduleHandle = KErrNotFound;
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerTaskScheduler::EnableBackupScheduleL
// ----------------------------------------------------------------------------
// 
void CFileManagerTaskScheduler::EnableBackupScheduleL( const TBool aEnable )
    {
    // Delete old schedule first
    DeleteScheduleL();

    if ( aEnable )
        {
        CreateScheduleL();
        }
    }

//  End of File  
