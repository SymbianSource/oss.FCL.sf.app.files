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
* Description:  Handles the copy/move operation
*
*/


// INCLUDE FILES
#include <bautils.h>
//#include <cmgxfilemanager.h>
//#include <mgxfilemanagerfactory.h>
#include "CFileManagerActiveExecute.h"
#include "MFileManagerProcessObserver.h"
#include "CFileManagerEngine.h"
#include "CFileManagerFileSystemIterator.h"
#include "Cfilemanagerindexiterator.h"
#include "CFileManagerCommonDefinitions.h"
#include "CFileManagerUtils.h"
#include "FileManagerDebug.h"
#include "CFileManagerThreadWrapper.h"

// CONSTANTS
const TInt KFileManagerNotificationArrayGranularity = 64;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::NewL
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C CFileManagerActiveExecute* CFileManagerActiveExecute::NewL( 
        CFileManagerEngine& aEngine,
        MFileManagerProcessObserver::TFileManagerProcess aOperation,
        MFileManagerProcessObserver& aObserver,
        CArrayFixFlat<TInt>& aIndexList,
        const TDesC& aToFolder )
    {
    CFileManagerActiveExecute* self = 
        new( ELeave ) CFileManagerActiveExecute(
            aEngine,
            aOperation,
            aObserver );
    CleanupStack::PushL( self );
    self->ConstructL( aIndexList, aToFolder );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::~CFileManagerActiveExecute
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C CFileManagerActiveExecute::~CFileManagerActiveExecute()
    {
    Cancel();
    delete iThreadWrapper;
    delete iItemIterator;
    delete iFullPath;
    delete iDestination;
    delete iToFolder;
    delete iIndexList;
    delete iChangedSrcItems;
    delete iChangedDstItems;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::ExecuteL
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerActiveExecute::ExecuteL( TFileManagerSwitch aOverWrite )
    {
    TInt error( KErrNone );
    
    if ( iCancelled )
        {
        TFileName newName;
        TParsePtrC parse( *iFullPath );
        if ( parse.NameOrExtPresent() )
            {
            newName.Copy( parse.NameAndExt() );
            }
        else
            {
            TPtrC name( iEngine.LocalizedName( *iFullPath ) );
            if ( name.Length() > 0 )
                {
                newName.Copy( name );
                }
            else
                {
                newName = BaflUtils::FolderNameFromFullName( *iFullPath );
                }
            }
        iObserver.ProcessFinishedL( iError, newName );
        return;
        }
    
    if ( aOverWrite == ENoOverWrite )
        {
        iItemIterator->CurrentL( &iSrc, &iDst, iItemType );
        }

    if ( iDst && iDst->Length() > KMaxFileName )
        {
        error = KErrBadName;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, error );              
        }
    else if ( iItemType == EFileManagerFile )
        {
#ifdef __KEEP_DRM_CONTENT_ON_PHONE
    if ( iSrc && iDst &&
        CFileManagerUtils::IsFromInternalToRemovableDrive( iFs, *iSrc, *iDst ) )
        {
        TBool protectedFile( EFalse );

        // silently ignore this file if it is protected, or if there
        // was an error in checking.
        // Did consider leaving, but what about eg KErrNotFound - eg another
        // process moving/deleting the file...
        TInt ret( iEngine.IsDistributableFile( *iSrc, protectedFile ) );
        if( protectedFile || ret != KErrNone )
            {
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            SetActive();
            return;
            }
        }
#endif
        DoOperation( aOverWrite );
        return;
        }
    else if ( iItemType == EFileManagerFolder )
        {
        if ( !iIsDstRemoteDrive && iEngine.IsNameFoundL( *iDst ) )
            {
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrAlreadyExists );
            SetActive();
            return;
            }
        DoOperation( aOverWrite );
        return;
        }
    else
        {
        // We must complete this
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, error );              
        }
    SetActive();
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::CancelExecution
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerActiveExecute::CancelExecution()
    {
    FUNC_LOG

    iCancelled = ETrue;

    if ( iSrc )
        {
        iEngine.CancelTransfer( *iSrc );
        }
    if ( iDst )
        {
        iEngine.CancelTransfer( *iDst );
        }
    delete iThreadWrapper; // Cancel thread
    iThreadWrapper = NULL;
    Cancel();
    TRAP_IGNORE( CompleteL( KErrCancel ) );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::DoCancel
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveExecute::DoCancel()
    {
    iCancelled = ETrue;

    if ( iSrc )
        {
        iEngine.CancelTransfer( *iSrc );
        }
    if ( iDst )
        {
        iEngine.CancelTransfer( *iDst );
        }
    delete iThreadWrapper; // Cancel thread
    iThreadWrapper = NULL;
    }

// ------------------------------------------------------------------------------
// CFileManagerActiveExecute::KErrNoneAction
//
// ------------------------------------------------------------------------------
//
void CFileManagerActiveExecute::KErrNoneActionL()
    {
    UpdateNotifications( EFalse, KErrNone );
    if ( iItemIterator->NextL() )
        {
        ExecuteL( ENoOverWrite );
        }
    else
        {
        if ( iOperation == MFileManagerProcessObserver::EMoveProcess && 
            iEngine.IsFolder( iIndexList->At( 0 ) ) )
            {
            if ( !iFinalizeMove )
                {
                // Finalize move in the thread, the finalizing way take time
                iFinalizeMove = ETrue;
                DoOperation( ENoOverWrite );
                return;
                }
            }
        UpdateNotifications( ETrue, KErrNone );
        iObserver.ProcessFinishedL( KErrNone );
        }
    
    }

// ------------------------------------------------------------------------------
// CFileManagerActiveExecute::KErrAlreadyExistsAction
//
// ------------------------------------------------------------------------------
//
void CFileManagerActiveExecute::KErrAlreadyExistsActionL()
    {
    TParsePtrC dstParse( *iDst );
    HBufC* name = HBufC::NewLC( KMaxFileName );
    TPtr ptrName( name->Des() );
    TBool doContinue( EFalse );

    // Depending on target file can it be delete, we ask overwrite or rename.
    // If source and target is same, then rename is only possible choice.
    if ( iItemType == EFileManagerFile && iEngine.CanDelete( *iDst ) && iSrc->FindF( *iDst ) )
        {
        if ( iObserver.ProcessQueryOverWriteL( *iDst, ptrName, iOperation ) )
            {
            ExecuteL( EOverWrite );
            }
        else
            {
            // user does not want to overwrite item and ptrName should now contain
            // user given new name
            if ( ptrName.Length() > 0 )
                {
                ptrName.Insert( 0, dstParse.DriveAndPath() );
                if ( !iDst->CompareF( ptrName ) )
                    {
                    DoOperation( ENoOverWrite );
                    }
                else
                    {
                    iDst->Des().Copy( ptrName );
                    // Overwrite, because user already queried by overwrite
                    DoOperation( EOverWrite );
                    }
                }
            else
                {
                // User is not willing to rename item, continue current operation
                doContinue = ETrue;
                }
            }
        }
    else
        {
        // item can't be overwrite
        if ( iObserver.ProcessQueryRenameL( *iDst, ptrName, iOperation ) )
            {
            if ( ptrName.Length() > 0 )
                {
                if ( iItemType == EFileManagerFile )
                    {
                    ptrName.Insert( 0, dstParse.DriveAndPath() );
                    iDst->Des().Copy( ptrName );
                    }
                else if ( iItemType == EFileManagerFolder )
                    {
                    TPtr ptr( iDst->Des() );
                    AddLastFolder( ptr, ptrName, *iToFolder );
                    iDestination->Des().Copy( ptr );
                    }
                // Overwrite, because user already queried by rename
                ExecuteL( EOverWrite );
                }
            else if ( iItemType == EFileManagerFolder )
                {
                iCancelled = ETrue;
                iError = KErrCancel;
                ExecuteL( ENoOverWrite );
                }
            else
                {
                // User is not willing to rename item, continue current operation
                doContinue = ETrue;
                }
            }
        else if ( iItemType == EFileManagerFolder )
            {
            iCancelled = ETrue;
            iError = KErrCancel;
            ExecuteL( ENoOverWrite );
            }
        else
            {
            // User is not willing to rename item, continue current operation
            doContinue = ETrue;
            }
        }

    CleanupStack::PopAndDestroy( name );

    if ( doContinue )
        {
        if ( iItemIterator->NextL() )
            {
            ExecuteL( ENoOverWrite );
            }
        else
            {
            UpdateNotifications( ETrue, KErrNone );
            iObserver.ProcessFinishedL( KErrNone );
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::RunL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveExecute::RunL()
    {
    CompleteL( iStatus.Int() );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::CompleteL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveExecute::CompleteL( TInt aError )
    {
    if ( iCancelled || !iItemIterator )
        {
        UpdateNotifications( ETrue, iError );
        TParsePtrC parse( CFileManagerUtils::StripFinalBackslash(
            *iDestination ) );
        if ( parse.NameOrExtPresent() )
            {
            iObserver.ProcessFinishedL( iError, parse.NameAndExt() );
            }
        else if ( parse.DrivePresent() )
            {
            iObserver.ProcessFinishedL( iError, parse.Drive() );
            }
        else
            {
            iObserver.ProcessFinishedL( iError );
            }
        return;
        }

    // Symbian returns KErrNone if source and destination
    // in moving is same. Here we have to treat it as error.
    if( iSrc && iDst )
        {
        if ( !iSrc->CompareF( *iDst ) && aError == KErrNone )
            {
            aError = KErrInUse;
            }
        }    

    ERROR_LOG1( "CFileManagerActiveExecute::CompleteL()-aError=%d", aError )

    switch ( aError )
        {
        case KErrNone:
            {
            KErrNoneActionL();
            break;
            }
        case KErrAlreadyExists:
            {
            KErrAlreadyExistsActionL();
            break;
            }
        case KErrCancel: // Suppressed errors
            {
            UpdateNotifications( ETrue, KErrNone );
            iObserver.ProcessFinishedL( KErrNone );
            break;
            }
        default:
            {
            if ( iSrc )
                {
                // Try rename when moving and the target file exists and is in use
                if ( iOperation == MFileManagerProcessObserver::EMoveProcess &&
                    aError == KErrInUse &&
                    iDst &&
                    iEngine.CanDelete( *iSrc ) &&
                    iEngine.IsNameFoundL( *iDst ) )
                    {
                    KErrAlreadyExistsActionL();
                    }
                else
                    {
                    UpdateNotifications( ETrue, aError );
                    TParsePtrC parse( *iSrc );
                    iObserver.ProcessFinishedL( aError, parse.NameAndExt() );
                    }
                }
            else
                {
                UpdateNotifications( ETrue, aError );
                iObserver.ProcessFinishedL( aError );
                }
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::RunError
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerActiveExecute::RunError(TInt aError)
    {
    return aError;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::CFileManagerActiveExecute
// 
// -----------------------------------------------------------------------------
// 
CFileManagerActiveExecute::CFileManagerActiveExecute( 
    CFileManagerEngine& aEngine,
    MFileManagerProcessObserver::TFileManagerProcess aOperation,
    MFileManagerProcessObserver& aObserver ) :
        CActive( CActive::EPriorityLow ), // Use low to avoid progress note mess up
        iEngine( aEngine ),
        iFs( aEngine.Fs() ),
        iOperation( aOperation ),
        iObserver( aObserver )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveExecute::ConstructL( CArrayFixFlat<TInt>& aIndexList,
                                            const TDesC& aToFolder )
    {
    iChangedSrcItems = new( ELeave ) CDesCArrayFlat(
        KFileManagerNotificationArrayGranularity );
    iChangedDstItems = new( ELeave ) CDesCArrayFlat(
        KFileManagerNotificationArrayGranularity );
    
    iToFolder = aToFolder.AllocL();
    TInt count( aIndexList.Count() );
    iIndexList = new( ELeave ) CArrayFixFlat< TInt >( count );
    for( TInt i( 0 ); i < count; ++i )
        {
        // make own copy of index list because caller may
        // destroy the original one.
        iIndexList->AppendL( aIndexList.At( i ) );
        }
    
    TInt index( iIndexList->At( iCurrentIndex ) );
    
    iFullPath = iEngine.IndexToFullPathL( index );

    TBool isDirectory( iEngine.IsFolder( index ) );

    iDestination = HBufC::NewL( KFmgrDoubleMaxFileName );

    if ( isDirectory )
        {
        TPtr ptr( iDestination->Des() );
        AddLastFolder( ptr, *iFullPath, *iToFolder );
        }
    else
        {
        iDestination->Des().Copy( aToFolder );
        }
    
    // Check that we are not copying/moving folder to inside it ( recursive copy )
    if ( isDirectory && !iDestination->FindF( *iFullPath ) && 
         iDestination->Length() > iFullPath->Length() )
        {
        iCancelled = ETrue;
        iError = KErrAccessDenied;
        }
    // Is destination path too long for file system
    else if ( iDestination->Length() > KMaxFileName )
        {
        iCancelled = ETrue;
        iError = KErrBadName;
        }
    else if ( isDirectory )
        {
        iItemIterator = CFileManagerFileSystemIterator::NewL(
            iFs, *iFullPath, *iDestination, iEngine );
        }
    else
        {
        iItemIterator = CFileManagerIndexIterator::NewL(
            iEngine, aIndexList, *iDestination );
        }

    // MG2 notification object
    //iMgxFileManager = &iEngine.MGXFileManagerL();

    // Check are operation source and target on the same drive
    TParsePtrC srcParse( *iFullPath  );
    TParsePtrC dstParse( *iDestination );
    TPtrC srcDrv( srcParse.Drive() );
    TPtrC dstDrv( dstParse.Drive() );
    iOperationOnSameDrive = !( srcDrv.CompareF( dstDrv ) );
    iIsSrcRemoteDrive = CFileManagerUtils::IsRemoteDrive( iFs, srcDrv );
    iIsDstRemoteDrive = CFileManagerUtils::IsRemoteDrive( iFs, dstDrv );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::AddLastFolder
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveExecute::AddLastFolder( TDes& aResult,
                                               const TDesC& aSrc, 
                                               const TDesC& aDst )
    {
    TInt lastBackslash = aSrc.LocateReverse( KFmgrBackslash()[0] );
    if ( lastBackslash != KErrNotFound )
        {
        // source is full path
        aResult.Copy( aSrc.Left( lastBackslash - 1 ) );
        // Last backslash is now temporary removed check next last backslash
        TInt secondLastBackslash( aResult.LocateReverse( KFmgrBackslash()[0] ) );
        // Now we know the coordinates of the last path
        aResult.Copy( iEngine.LocalizedName( aSrc ) );
        if ( aResult.Length() > 0 )
            {
            aResult.Insert( 0, aDst );
            aResult.Append( KFmgrBackslash );
            }
        else
            {
            aResult.Append( aDst );
            // Skip '\\'
            TInt startingPoint( secondLastBackslash + 1 );
            aResult.Append( aSrc.Mid( startingPoint, 
                                      lastBackslash - secondLastBackslash ) );
            }
        }
    else
        {
        // source is only one folder name
        aResult.Copy( aDst );
        aResult.Append( aSrc );
        aResult.Append( KFmgrBackslash );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::DoOperation
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerActiveExecute::DoOperation( TInt aSwitch )
    {
    // Source and destination must be different
    if ( iSrc && iDst && !iDst->CompareF( *iSrc ) )
        {
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrAlreadyExists );
        SetActive();
        return KErrAlreadyExists;
        }
    TInt err( KErrNone );

    iSwitch = aSwitch;

    if ( !iThreadWrapper )
        {
        TRAP( err, iThreadWrapper = CFileManagerThreadWrapper::NewL() );
        if ( err != KErrNone )
            {
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, err );
            SetActive();
            return err;
            }
        }
    if ( iThreadWrapper->IsThreadStarted() )
        {
        iThreadWrapper->ResumeThread();
        }
    else
        {
        err = iThreadWrapper->StartThread(
            *this, ENotifyStepFinished, EPriorityLess );
        if ( err != KErrNone )
            {
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, err );
            SetActive();
            return err;
            }
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::IsEmptyDir
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerActiveExecute::IsEmptyDir( const TDesC& aDir )
    {
    return !CFileManagerUtils::HasAny(
        iFs, aDir, KEntryAttMatchMask | KEntryAttNormal );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::ThreadCopyOrMoveStepL()
//
// -----------------------------------------------------------------------------
//
void CFileManagerActiveExecute::ThreadCopyOrMoveStepL()
    {
    FUNC_LOG

    INFO_LOG2( "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-%S=>%S",
         iSrc, iDst )

    TInt err( KErrNone );
    
    if ( iItemType == EFileManagerFolder )
        {
        // Handle folders
        err = iFs.MkDir( *iDst ); // Try faster way first
        LOG_IF_ERROR1(
            err,
            "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-MkDir,err=%d",
            err )
        if ( err != KErrNone && err != KErrCancel )
            {
            err = iFs.MkDirAll( *iDst );
            LOG_IF_ERROR1(
                err,
                "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-MkDirAll,err=%d",
                err )
            }
        if ( err == KErrNone )
            {
            TEntry entry;
            err = iFs.Entry( *iSrc, entry );
            if ( err == KErrNone )
                {
                iFs.SetEntry( *iDst, entry.iModified, entry.iAtt, 0 ); // Ignore error
                }
            }
        User::LeaveIfError( err );
        return;
        }
    
    // Handle files
    if ( iOperationOnSameDrive &&
        iOperation == MFileManagerProcessObserver::EMoveProcess )
        {
        INFO_LOG(
            "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-MoveInsideDrive" )

        if ( iSwitch == EOverWrite )
            {
            INFO_LOG( "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-Overwrite" )

            err = iFs.Replace( *iSrc, *iDst );
            }
        else
            {
            INFO_LOG( "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-NoOverwrite" )

            err = iFs.Rename( *iSrc, *iDst );
            }

        LOG_IF_ERROR1(
            err,
            "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-OnSameDrive,err=%d",
            err )

        if ( err == KErrNone || err == KErrCancel )
            {
            // Optimized move was successful or cancelled
            User::LeaveIfError( err );
            // If move the files in the same drive, the application
            // just calculate the amount of the files, so it should 
            // notify the observer that how many files have been moved.
            iBytesTransferredTotal++;
            TRAP_IGNORE( iObserver.ProcessAdvanceL(
                iBytesTransferredTotal ) ); 
            return;
            }
        }

    TInt64 fileSize( 0 );
    RFile64 srcFile;

    // Open source file
    if ( iOperation == MFileManagerProcessObserver::EMoveProcess )
        {
        INFO_LOG( "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-Move" )

        User::LeaveIfError( srcFile.Open(
            iFs, *iSrc, EFileRead | EFileShareExclusive ) );
        }
    else
        {
        INFO_LOG( "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-Copy" )

        User::LeaveIfError( srcFile.Open(
            iFs, *iSrc, EFileRead | EFileShareReadersOnly ) );
        }
    CleanupClosePushL( srcFile );
    User::LeaveIfError( srcFile.Size( fileSize ) );

    // Open destination file
    RFile64 dstFile;
    if ( iSwitch == EOverWrite )
        {
        INFO_LOG( "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-Overwrite" )

        User::LeaveIfError( dstFile.Replace(
            iFs, *iDst, EFileWrite | EFileShareExclusive ) );
        }
    else
        {
        INFO_LOG( "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-NoOverwrite" )

        User::LeaveIfError( dstFile.Create(
            iFs, *iDst, EFileWrite | EFileShareExclusive ) );
        }
    CleanupClosePushL( dstFile );

    dstFile.SetSize( fileSize ); // Setting the size first speeds up operation

    INFO_LOG1(
        "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-FileSize=%d",
         fileSize )

    // Create buffer and copy file data using it. 
    // Note that buffer size should not be too big to make it is possible 
    // to cancel the operation in reasonable time from the main thread. 

    // Move these to CenRep to make configuration and fine tuning easier.
    const TInt64 KBigBufSize = 0x40000; // 256KB
    const TInt64 KMediumBufSize = 0x10000; // 64KB
    const TInt64 KSmallBufSize = 0x2000; // 8KB

    HBufC8* buf = HBufC8::New(
        Max( KSmallBufSize, Min( fileSize, KBigBufSize ) ) );
    if ( !buf )
        {
        buf = HBufC8::New( KMediumBufSize );
        }
    if ( !buf )
        {
        buf = HBufC8::New( KSmallBufSize );
        }
    if ( !buf )
        {
        User::Leave( KErrNoMemory );
        }
    CleanupStack::PushL( buf );

    TPtr8 block( buf->Des() );
    
    INFO_LOG1(
        "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-BlockSize=%d",
         block.MaxSize() )

    while ( err == KErrNone && fileSize > 0 )
        {
        if ( iThreadWrapper->IsThreadCanceled() )
            {
            err = KErrCancel;
            break;
            }
        TInt blockSize( Min( fileSize, static_cast<TInt64>(block.MaxSize() ) ) );
        err = srcFile.Read( block, blockSize );
        if ( err == KErrNone )
            {
            if ( block.Length() == blockSize )
                {
                err = dstFile.Write( block, blockSize );
                if ( err == KErrNone )
                    {
                    fileSize -= blockSize;

                    // Do not update the latest file transfer progress here.
                    // Flushing file below may take a long time and
                    // progress indicator should not get full before it.
                    TRAP_IGNORE( iObserver.ProcessAdvanceL(
                        iBytesTransferredTotal ) );
                    iBytesTransferredTotal += blockSize;
                    }
                }
            else
                {
                err = KErrCorrupt;
                }
            }
        }

    INFO_LOG2(
        "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-BytesNotWritten=%d,err=%d",
         fileSize, err )

    CleanupStack::PopAndDestroy( buf );

    // Copy attributes
    TTime mod;
    if ( err == KErrNone )
        {
        err = srcFile.Modified( mod );

        INFO_LOG1(
            "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-ModifiedRead,err=%d",
            err )

        }
    if ( err == KErrNone )
        {
        err = dstFile.SetModified( mod );

        INFO_LOG1(
            "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-ModifiedWritten,err=%d",
            err )
        }
    TUint att( 0 );
    if ( err == KErrNone )
        {
        err = srcFile.Att( att );

        INFO_LOG2(
            "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-AttributesRead,err=%d,att=%d",
            err, att )
        }
    if ( err == KErrNone )
        {
        // Ignore fail, because some drives like remote drives
        // do not support attributes at all
        dstFile.SetAtt( att, ( ~att ) & KEntryAttMaskSupported );

        INFO_LOG(
            "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-AttributesWritten" )
        }
    // Flush file and finalize transfer progress of this file.
    // Don't flush if copying failed because it causes save dialog to appear 
    // when remote drives are involved.
    if ( err == KErrNone )
        {
        err = dstFile.Flush();

        INFO_LOG1(
            "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-Flushed,err=%d",
            err )
        }
    TRAP_IGNORE( iObserver.ProcessAdvanceL( iBytesTransferredTotal ) );

    CleanupStack::PopAndDestroy( &dstFile );
    CleanupStack::PopAndDestroy( &srcFile );

    // Delete source if move was succesful so far
    if ( err == KErrNone &&
        iOperation == MFileManagerProcessObserver::EMoveProcess )
        {
        // Ensure that read-only is removed before delete
        if ( att & KEntryAttReadOnly )
            {
            CFileManagerUtils::RemoveReadOnlyAttribute( iFs, *iSrc );
            }
        err = iFs.Delete( *iSrc );

        INFO_LOG1(
            "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-MoveSourceDeleted,err=%d",
            err )
        }

    // Delete incomplete destination if error
    if ( err != KErrNone )
        {
        // Ensure that read-only is removed before delete
        if ( att & KEntryAttReadOnly )
            {
            CFileManagerUtils::RemoveReadOnlyAttribute( iFs, *iDst );
            }
        iFs.Delete( *iDst );

        ERROR_LOG1(
            "CFileManagerActiveExecute::ThreadCopyOrMoveStepL-FailedDstDeleted,fail=%d",
            err )
        }

    User::LeaveIfError( err );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::IsThreadDone()
//
// -----------------------------------------------------------------------------
//
TBool CFileManagerActiveExecute::IsThreadDone()
    {
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::NotifyThreadClientL()
//
// -----------------------------------------------------------------------------
//
void CFileManagerActiveExecute::NotifyThreadClientL(
        TNotifyType aType, TInt aValue )
    {
    switch ( aType )
        {
        case ENotifyStepFinished:
            {
            CompleteL( aValue );
            break;
            }
        default:
            {
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::AppendArrayIfNotFound()
//
// -----------------------------------------------------------------------------
//
void CFileManagerActiveExecute::AppendArrayIfNotFound(
        CDesCArray& aArray, const TDesC& aFullPath )
    {
    // Append if not already appended to the last item
    TBool append( ETrue );
    TInt count( aArray.MdcaCount() );
    if ( count > 0 )
        {
        TPtrC ptr( aArray.MdcaPoint( count - 1 ) );
        if ( !ptr.Compare( aFullPath ) )
            {
            append = EFalse;
            }
        }
    if ( append )
        {
        TRAPD( err, aArray.AppendL( aFullPath ) );
        if ( err != KErrNone )
            {
            ERROR_LOG1(
                "CFileManagerActiveExecute::AppendArrayIfNotFound-err=%d",
                err )
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::FlushArray()
//
// -----------------------------------------------------------------------------
//
void CFileManagerActiveExecute::FlushArray( CDesCArray& aArray )
    {
    if ( aArray.MdcaCount() > 0 )
        {
//        TRAP_IGNORE( iMgxFileManager->UpdateL( aArray ) );
        aArray.Reset();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::UpdateNotifications()
//
// -----------------------------------------------------------------------------
//
void CFileManagerActiveExecute::UpdateNotifications(
        TBool aFlush, TInt aError )
    {
    // Append notification item if operation was successful and 
    // item does not already exist
    if ( aError == KErrNone && iSrc && iDst && iItemType == EFileManagerFile )
        {
        // Notifications are relevant only for local drives
        if ( iOperation == MFileManagerProcessObserver::EMoveProcess &&
             !iIsSrcRemoteDrive )
            {
            AppendArrayIfNotFound( *iChangedSrcItems, *iSrc );
            }
        if ( !iIsDstRemoteDrive )
            {
            AppendArrayIfNotFound( *iChangedDstItems, *iDst );
            }
        }
    if ( aFlush )
        {
        FlushArray( *iChangedSrcItems );
        FlushArray( *iChangedDstItems );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::ThreadFinalizeMoveStepL()
//
// -----------------------------------------------------------------------------
//
void CFileManagerActiveExecute::ThreadFinalizeMoveStepL()
    {
    FUNC_LOG

    HBufC* folderToDelete = HBufC::NewLC( KMaxFileName );
    TPtr ptrFolderToDelete( folderToDelete->Des() );
    CDirScan* dirScan = CDirScan::NewLC( iFs );
    dirScan->SetScanDataL( 
        *iFullPath, 
        KEntryAttNormal|KEntryAttHidden|KEntryAttSystem|
        KEntryAttDir|KEntryAttMatchExclusive, 
        ESortNone, 
        CDirScan::EScanUpTree );
    CDir* dir = NULL;
    dirScan->NextL( dir );
    while( dir )
        {
        CFileManagerUtils::RemoveReadOnlyAttribute(
            iFs, dirScan->FullPath() );
        CleanupStack::PushL( dir );
        TInt count( dir->Count() );
        for( TInt i( 0 ); i < count; ++i )
            {
            if ( iThreadWrapper->IsThreadCanceled() )
                {
                User::Leave( KErrCancel );
                }
            TPtrC abbrPath( dirScan->AbbreviatedPath() );
            const TEntry& entry( ( *dir )[ i ] );
            ptrFolderToDelete.Copy( *iFullPath );
            ptrFolderToDelete.Append( 
                abbrPath.Right( abbrPath.Length() - 1 ) );
            ptrFolderToDelete.Append( entry.iName );
            ptrFolderToDelete.Append( KFmgrBackslash );
#ifdef __KEEP_DRM_CONTENT_ON_PHONE
            if ( iSrc && iDst &&
                CFileManagerUtils::IsFromInternalToRemovableDrive( iFs, *iSrc, *iDst ) )
                {
                HBufC* targetFolderToDelete = HBufC::NewLC( KMaxFileName );
                TPtr ptrTargetFolderToDelete( targetFolderToDelete->Des() );
                ptrTargetFolderToDelete.Append( *iDestination );
                ptrTargetFolderToDelete.Append( 
                    abbrPath.Right( abbrPath.Length() - 1 ) );
                ptrTargetFolderToDelete.Append( entry.iName );
                ptrTargetFolderToDelete.Append( KFmgrBackslash );
                
                if ( IsEmptyDir( ptrFolderToDelete ) )
                    {
                    User::LeaveIfError( iFs.RmDir( ptrFolderToDelete ) );
                    }
                else if ( IsEmptyDir( ptrTargetFolderToDelete ))
                    {
                    User::LeaveIfError( iFs.RmDir( ptrTargetFolderToDelete ) );
                    }
                CleanupStack::PopAndDestroy( targetFolderToDelete );    
                }
            else
                {
                User::LeaveIfError( iFs.RmDir( ptrFolderToDelete ) );   
                }
#else
            User::LeaveIfError( iFs.RmDir( ptrFolderToDelete ) );
#endif
            }
        if ( iThreadWrapper->IsThreadCanceled() )
            {
            User::Leave( KErrCancel );
            }
        CleanupStack::PopAndDestroy( dir );
        dir = NULL;
        dirScan->NextL( dir );
        }
    CleanupStack::PopAndDestroy( dirScan );
    CleanupStack::PopAndDestroy( folderToDelete );
#ifdef __KEEP_DRM_CONTENT_ON_PHONE
    if ( iSrc && iDst &&
        CFileManagerUtils::IsFromInternalToRemovableDrive( iFs, *iSrc, *iDst ) )
        {
        if ( IsEmptyDir( *iFullPath ) )
            {
            User::LeaveIfError( iFs.RmDir( *iFullPath ) );
            }   
        }
    else
        {
        User::LeaveIfError( iFs.RmDir( *iFullPath ) );  
        }
#else
    User::LeaveIfError( iFs.RmDir( *iFullPath ) );
#endif
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::ThreadStepL()
//
// -----------------------------------------------------------------------------
//
void CFileManagerActiveExecute::ThreadStepL()
    {
    FUNC_LOG

    if ( !iFinalizeMove )
        {
        ThreadCopyOrMoveStepL();
        }
    else
        {
        ThreadFinalizeMoveStepL();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveExecute::ToFolder()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerActiveExecute::ToFolder()
    {
    if ( iToFolder )
        {
        return iToFolder->Des();
        }
    return TPtrC( KNullDesC );
    }

// End of File
