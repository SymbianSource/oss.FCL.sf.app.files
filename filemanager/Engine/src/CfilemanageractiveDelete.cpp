/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Deletes files
*
*/


// INCLUDE FILES
#include "Cfilemanageractivedelete.h"
#include "MFileManagerProcessObserver.h"
#include "CFileManagerEngine.h"
#include "CFileManagerCommonDefinitions.h"
#include "CFileManagerUtils.h"
#include "FileManagerDebug.h"
//#include <cmgxfilemanager.h>

// CONSTANTS
const TInt KFileManagerDeletionPerStep = 20;
const TInt64 KFileManagerMaxStepTime = 1000000; // 1s
const TInt KFileManagerNotificationArrayGranularity = 64;


// ============================ LOCAL FUNCTIONS ================================
// -----------------------------------------------------------------------------
// GetTimeStamp
// -----------------------------------------------------------------------------
//
static TInt64 GetTimeStamp()
    {
    TTime time;
    time.UniversalTime();
    return time.Int64();
    }

// -----------------------------------------------------------------------------
// IsTimedOut
// -----------------------------------------------------------------------------
//
static TBool IsTimedOut( const TInt64& aStartTime )
    {
    TInt64 time( GetTimeStamp() );
    TBool ret( time - aStartTime > KFileManagerMaxStepTime );
    if ( ret )
        {
        INFO_LOG("CFileManagerActiveDelete-TimedOut");
        }
    return ret;
    }


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::CFileManagerActiveDelete
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerActiveDelete::CFileManagerActiveDelete(
        RFs& aFs,
        CArrayFixFlat< TInt >& aIndexList,
        CFileManagerEngine& aEngine,
        CFileManagerUtils& aUtils ) :
    iFs( aFs ),
    iIndexList( aIndexList ),
    iError( KErrNone ),
    iEngine( aEngine ),
    iUtils( aUtils )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerActiveDelete* CFileManagerActiveDelete::NewL(
        CArrayFixFlat< TInt >& aIndexList,
        CFileManagerEngine& aEngine,
        CFileManagerUtils& aUtils )
    {
    CFileManagerActiveDelete* self = new( ELeave ) CFileManagerActiveDelete(
        aEngine.Fs(), aIndexList, aEngine, aUtils );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveDelete::ConstructL()
    {
    iStringBuffer = HBufC::NewL( KMaxFileName );
    //MG2 notification object
    //iMgxFileManager = &iEngine.MGXFileManagerL();
    iRemovedItems = new( ELeave ) CDesCArrayFlat(
        KFileManagerNotificationArrayGranularity );
    iIsRemoteDrive = iUtils.IsRemoteDrive( iEngine.CurrentDirectory() );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::~CFileManagerActiveDelete
// Destructor
// -----------------------------------------------------------------------------
// 
EXPORT_C CFileManagerActiveDelete::~CFileManagerActiveDelete()
    {
    delete iStringBuffer;
    delete iFullPath;
    delete iDirScan;
    delete iDir;
    delete iRemovedItems;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::IsProcessDone
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerActiveDelete::IsProcessDone() const
    {
    return iProcessDone;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::DeleteItemsInDirectoryL
// -----------------------------------------------------------------------------
//
TBool CFileManagerActiveDelete::DeleteItemsInDirectoryL()
    {
    if ( !iDirScan )
        {
        iDirScan = CDirScan::NewL( iFs );
        // Set scanning from current directory, take all files
        // No sorting needed
        iDirScan->SetScanDataL( *iFullPath,
            KEntryAttNormal | KEntryAttHidden | KEntryAttSystem,
            ESortNone );
        }

    if ( iDir && iFileIndex < iDir->Count() )
        {
        // Delete file item
        const TEntry& item = ( *iDir )[ iFileIndex ];
        TPtr ptr( iStringBuffer->Des() );
        CFileManagerUtils::GetFullPath(
            iDirScan->FullPath(), item, ptr );
        CFileManagerUtils::RemoveReadOnlyAttribute( iFs, ptr, item );
        DeleteFileL( ptr, ETrue );
        ++iFileIndex;
        }
    else
        {
        // Fetch next directory
        delete iDir;
        iDir = NULL;
        iDirScan->NextL( iDir );
        iFileIndex = 0;

        if ( iDir )
            {
            CFileManagerUtils::RemoveReadOnlyAttribute(
                iFs, iDirScan->FullPath() );
            }
        }

    if ( !iDir )
        {
        // Items are deleted now, report done
        if ( !iNotDeletedItems )
            {
            // Delete all directories
            CFileMan* fileMan = CFileMan::NewL( iFs );
            SetError( fileMan->RmDir( *iFullPath ), *iFullPath );
            delete fileMan;
            }
        return ETrue;
        }
    return EFalse; // Still items left
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::StepL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveDelete::StepL()
    {
    FUNC_LOG

    TInt64 startTime( GetTimeStamp() );
    TInt indexCount( iIndexList.Count() );
    TInt i( KFileManagerDeletionPerStep );
    TBool timedOut( EFalse );

    while ( iCurrentIndex < indexCount && i && !timedOut )
        {
        TBool isItemDone( EFalse );

        // Fetch item path if missing
        if ( !iFullPath )
            {
            iFullPath = iEngine.IndexToFullPathL(
                iIndexList.At( iCurrentIndex ) );
            if ( IsDir( *iFullPath ) )
                {
                // Ignore default folders
                if ( iUtils.DefaultFolder( *iFullPath ) )
                    {
                    SetError( KErrFmgrDefaultFolder, *iFullPath );
                    isItemDone = ETrue;
                    }
                }
            }
        // Delete item
        if ( !isItemDone )
            {
            if ( IsDir( *iFullPath ) )
                {
                // Delete directory item
                while ( iCurrentIndex < indexCount && i && !isItemDone && !timedOut )
                    {
                    isItemDone = DeleteItemsInDirectoryL();
                    --i;

                    // Adjust amount of deleted files per step by consumed time. 
                    // This is an attempt to avoid long periods of time, 
                    // where the UI does not respond to user activity.
                    timedOut = IsTimedOut( startTime );
                    }
                }
            else
                {
                // Delete file item
                DeleteFileL( *iFullPath );
                isItemDone = ETrue;
                --i;

                // Adjust amount of deleted files per step by consumed time. 
                // This is an attempt to avoid long periods of time, 
                // where the UI does not respond to user activity.
                timedOut = IsTimedOut( startTime );
                }
            }

        // Move to next item if done
        if ( isItemDone )
            {
            delete iFullPath;
            iFullPath = NULL;
            ++iCurrentIndex;
            }
        }

    if ( iCurrentIndex >= indexCount )
        {
        iProcessDone = ETrue;
        FlushNotifications();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::DialogDismissedL
//
// -----------------------------------------------------------------------------
//
void CFileManagerActiveDelete::DialogDismissedL( TInt aButtonId )
    {
    if ( aButtonId == EAknSoftkeyCancel )
        {
        FlushNotifications();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::GetError
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C TInt CFileManagerActiveDelete::GetError( TDes& aFileName )
    {
    if( iFileName.Length() > 0)
        {
        aFileName.Zero();
        aFileName.Append( iFileName );
        }
    if ( iOpenFiles > 1 )
        {
        iError = KErrFmgrSeveralFilesInUse;
        }
    return iError;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::DeletedDrmItems
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C TInt CFileManagerActiveDelete::DeletedDrmItems(TInt& aTotalCount )
    {
    aTotalCount = iDeletedItems;
    return iDeletedDrmItems;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::DeleteFileL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveDelete::DeleteFileL(
        const TDesC& aFullPath, const TBool aReadOnlyChecked )
    {
#ifndef RD_DRM_RIGHTS_MANAGER_REMOVAL
    TBool isLocalDataFile( EFalse );
    TBool isDrmProtected( EFalse );
    if ( !iIsRemoteDrive )
        {
        // Check DRM protection
        isDrmProtected = iUtils.IsDrmProtectedFile( aFullPath );
        if ( isDrmProtected )
            {
            isLocalDataFile = iUtils.IsDrmLocalDataFile( aFullPath );
            }
        }
#endif // RD_DRM_RIGHTS_MANAGER_REMOVAL

    TInt err( iFs.Delete( aFullPath ) );
    if ( err == KErrAccessDenied && !aReadOnlyChecked )
        {
        // Remove readonly and retry
        TEntry entry;
        if( iFs.Entry( aFullPath, entry ) == KErrNone )
            {
            CFileManagerUtils::RemoveReadOnlyAttribute(
                iFs, aFullPath, entry );
            err = iFs.Delete( aFullPath );
            }
        }
    if ( !IsError( err ) )
        {
        ++iDeletedItems;
        // Notification is relevant only for local drives
        if ( !iIsRemoteDrive )
            {
            TRAPD( err2, iRemovedItems->AppendL( aFullPath ) );
            if ( err2 != KErrNone )
                {
                ERROR_LOG1(
                    "CFileManagerActiveExecute::DeleteFileL-NotificationAppend-err=%d",
                    err2 )
                }
            }
#ifndef RD_DRM_RIGHTS_MANAGER_REMOVAL
        // Inform deletion of DRM protected files except local data files
        if( isDrmProtected && !isLocalDataFile )
            {
            ++iDeletedDrmItems;
            SetName( aFullPath );
            }
#endif // RD_DRM_RIGHTS_MANAGER_REMOVAL
        }
    else
        {
        // Delete failed, update error info
        SetError( err, aFullPath );
        if ( err == KErrInUse )
            {
            ++iOpenFiles;
            }
        ++iNotDeletedItems;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::IsDir
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerActiveDelete::IsDir( const TDesC& aFullPath )
    {
    return CFileManagerUtils::HasFinalBackslash( aFullPath );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::SetName
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveDelete::SetName( const TDesC& aFullPath,
        TBool aOverWrite )
    {
    if ( !aOverWrite && iFileName.Length() )
        {
        return;
        }
    iFileName.Zero();
    if ( aFullPath.Length() )
        {
        if ( IsDir( aFullPath ) )
            {
            TParsePtrC parse( aFullPath.Left( aFullPath.Length() - 1 ) );
            iFileName.Append( parse.Name() );
            }
        else
            {
            TParsePtrC parse( aFullPath );
            iFileName.Append( parse.NameAndExt() );
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::IsError
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerActiveDelete::IsError( TInt aErr )
    {
    return ( aErr != KErrNone &&
            aErr != KErrCorrupt &&
            aErr != KErrNotFound &&
            aErr != KErrPathNotFound );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::SetError
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveDelete::SetError( TInt aErr, const TDesC& aFullPath )
    {
    if ( iError == KErrNone && IsError( aErr ) )
        {
        iError = aErr;
        SetName( aFullPath, ETrue );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveDelete::FlushNotifications
// -----------------------------------------------------------------------------
//
void CFileManagerActiveDelete::FlushNotifications()
    {
    if ( iRemovedItems->MdcaCount() > 0 )
        {
        //TRAP_IGNORE( iMgxFileManager->UpdateL( *iRemovedItems ) );
        iRemovedItems->Reset();
        }
    }

// End of file
