/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  File list model
*
*/



// INCLUDE FILES
#include "CGflmFileListModel.h"
#include "MGflmItemGroup.h"
#include "CGflmItemGroupImpl.h"
#include "CGflmDirectoryListingCache.h"
#include "CGflmFileRecognizer.h"
#include "CGflmDriveResolver.h"
#include "CGflmItemLocalizer.h"
#include "CGflmFileSystemItem.h"
#include "CGflmGlobalActionItem.h"
#include "CGflmFileFinder.h"
#include "MGflmItemFilter.h"
#include "GflmUtils.h"
#include "GFLMConsts.h"
#include "GFLM.hrh"
#include <e32base.h>
#include <e32std.h>
#include <badesca.h>


// CONSTANTS

// Granularity of iGroups array
const TInt KItemGroupArrayGranularity = 3;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmFileListModel::CGflmFileListModel
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGflmFileListModel::CGflmFileListModel( RFs& aFs ) :
        CActive( CActive::EPriorityStandard ),
        iFs( aFs )
    {
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CGflmFileListModel* CGflmFileListModel::NewL( RFs& aFs )
    {
    CGflmFileListModel* self = new( ELeave ) CGflmFileListModel( aFs );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CGflmFileListModel::ConstructL()
    {
    FUNC_LOG

    CActiveScheduler::Add( this );

    iGroups = new( ELeave )
        CArrayPtrSeg< CGflmItemGroupImpl >
            ( KItemGroupArrayGranularity );
    iListingCache = CGflmDirectoryListingCache::NewL( iFs, iCanceled );
    iDriveResolver = CGflmDriveResolver::NewL( iFs );
    iFileRecognizer = CGflmFileRecognizer::NewL(
        iFs, KGFLMFileRecognizerCacheSize, iDriveResolver );
    iItemLocalizer = CGflmItemLocalizer::NewL();
    iFileFinder = CGflmFileFinder::NewL(
        iFs, *iItemLocalizer, *iDriveResolver, iCanceled );
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::~CGflmFileListModel
//
// -----------------------------------------------------------------------------
//
CGflmFileListModel::~CGflmFileListModel()
    {
    FUNC_LOG

    Cancel();
    if ( iGroups )
        {
        iGroups->ResetAndDestroy();
        delete iGroups;
        }
    delete iFileFinder;
    delete iListingCache;
    delete iFileRecognizer;
    delete iItemLocalizer;
    delete iDriveResolver;
    iSemaphore.Close();
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::RefreshListL()
// Creates and starts the work thread that refreshes the model
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileListModel::RefreshListL(
        TRequestStatus& aStatus, TGflmRefreshMode aRefreshMode )
    {
    FUNC_LOG

    if ( aStatus == KRequestPending )
        {
        return;
        }

    iCorrupt = ETrue;
    iClientStatus = &aStatus;
    *iClientStatus = KRequestPending;
    iClientId = RThread().Id();

    if ( iShutdown )
        {
        // Ensure client completion
        NotifyClient( KErrNone );
        return;
        }

    iCanceled = EFalse;

    if ( !IsActive() )
        {
        // Create the work thread to do refresh
        iSemaphore.Close();
        User::LeaveIfError( iSemaphore.CreateLocal( 0 ) );

        RThread workThread;
        TInt err( workThread.Create(
            KGFLMWorkThreadName,
            WorkThreadStart,
            KGFLMWorkThreadStackSize,
            NULL,
            this ) );
        LOG_IF_ERROR1( err, "CGflmFileListModel::RefreshListL-err1=%d", err )
        User::LeaveIfError( err );
        workThread.SetPriority( EPriorityLess );
        workThread.Logon( iStatus );
        SetActive();
        iRefreshMode = aRefreshMode;
        workThread.Resume();
        workThread.Close();
        }
    else
        {
        // Signal the work thread to do refresh
        iRefreshMode = aRefreshMode;
        iSemaphore.Signal();
        }
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::WorkThreadStart()
// An entry point function for the work thread
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CGflmFileListModel::WorkThreadStart( TAny* aPtr )
    {
    FUNC_LOG

    CGflmFileListModel* self = static_cast< CGflmFileListModel* >( aPtr );
    CTrapCleanup* cleanupStack = CTrapCleanup::New();
    if ( !cleanupStack )
        {
        ERROR_LOG1(
            "CGflmFileListModel::WorkThreadStart-err1=%d", KErrNoMemory )
        return KErrNoMemory;
        }

    TRAPD( err, self->iItemLocalizer->SetupWorkThreadLocalizerL() );
    LOG_IF_ERROR1( err, "CGflmFileListModel::WorkThreadStart-err2=%d", err )

    self->WorkThread();
    self->iItemLocalizer->ReleaseWorkThreadLocalizer();
    delete cleanupStack;

    return err;
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::WorkThread()
// -----------------------------------------------------------------------------
//
void CGflmFileListModel::WorkThread()
    {
    TInt err( KErrNone );
    do
        {

        // Do refresh
        iCorrupt = ETrue;
        err = KErrNone;

        TRAP( err, DoRefreshL() );
        if ( err == KErrNone )
            {
            iCorrupt = EFalse;
            }

        LOG_IF_ERROR1( err, "CGflmFileListModel::WorkThread-err=%d", err )

        NotifyClient( err );

        iSemaphore.Wait(); // Wait for refresh signal from main thread

        } while ( !iShutdown );

    if ( iShutdown )
        {
        // Disable list usage and ensure client completion
        iCorrupt = ETrue;
        NotifyClient( err );
        }
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::NotifyClient()
// -----------------------------------------------------------------------------
//
void CGflmFileListModel::NotifyClient( TInt aErr )
    {
    FUNC_LOG

    if ( iClientStatus )
        {
        RThread client;
        if ( client.Open( iClientId ) == KErrNone )
            {
            client.RequestComplete( iClientStatus, aErr );
            client.Close();
            }
        }
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::DoRefreshL()
// This method does the actual refreshing. It's not static so it can access
// member data and it can leave as it's trap harnessed in RefreshAndNotify()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CGflmFileListModel::DoRefreshL()
    {
    TIMESTAMP( "GFLM refresh files and folders started: " )

    INFO_LOG1(
        "CGflmFileListModel::DoRefreshL-iSearchEnabled=%d",
        iSearchEnabled )

    if ( iSearchEnabled )
        {
        User::LeaveIfError( iDriveResolver->RefreshDrives( iFilter ) );
        iFileFinder->RefreshL( iFilter, iSearchSortMethod, iRefreshMode );
        }
    else
        {
        if ( iRefreshMode == ERefreshItems )
            {
            iListingCache->ClearCache();
            }
        User::LeaveIfError( iDriveResolver->RefreshDrives( iFilter ) );

        TInt groupCount( iGroups->Count() );
        for ( TInt i( 0 ); i < groupCount; i++ )
            {
            iGroups->At( i )->RefreshL( iFilter, iSortMethod, iRefreshMode );
            }
        }

    TIMESTAMP( "GFLM refresh  files and folders ended: " )
	}

// -----------------------------------------------------------------------------
// CGflmFileListModel::CancelRefresh()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileListModel::CancelRefresh()
    {
    FUNC_LOG

    iCanceled = ETrue;
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::MdcaCount()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CGflmFileListModel::MdcaCount() const
    {
    if ( iCorrupt )
        {
        return 0;
        }
    if ( iSearchEnabled )
        {
        return iFileFinder->ItemCount();
        }

    // Count the total number of items in groups
    TInt groupCount( iGroups->Count() );
    TInt itemCount( 0 );
    for( TInt i( 0 ); i < groupCount; i++ )
        {
        itemCount += iGroups->At( i )->ItemCount();
        }
    return itemCount;
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::MdcaPoint()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TPtrC CGflmFileListModel::MdcaPoint( TInt aIndex ) const
    {
    if ( iSearchEnabled )
        {
        return iFileFinder->Item( aIndex )->Name();
        }

    // Return the formatted presentation by global index
    MGflmItemGroup* group = NULL;
    TInt localIndex( 0 );
    GetGroupAndLocalIndex( aIndex, localIndex, group );
    return group->Item( localIndex )->Name();
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::GetGroupAndLocalIndex()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CGflmFileListModel::GetGroupAndLocalIndex(
        const TInt aGlobalIndex,
        TInt& aLocalIndex,
        MGflmItemGroup*& aGroup ) const
    {
    // Calculate the group and the requested item's local index
    // from a global index
    TInt groupCount( iGroups->Count() );
    TInt offset( 0 );
    for( TInt i( 0 ); i < groupCount; i++ )
        {
        MGflmItemGroup* currentGroup = iGroups->At( i );
        TInt itemsInGroup( currentGroup->ItemCount() );
        if ( aGlobalIndex < itemsInGroup + offset )
            {
            // The global index resides in this group
            aGroup = iGroups->At( i );
            aLocalIndex = aGlobalIndex - offset;
            return;
            }
        offset += itemsInGroup;
        }

    // Group not found
    User::Panic( KGFLMPanicText, EGlobalIndexOutOfBounds );
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::Item()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C CGflmGroupItem* CGflmFileListModel::Item( TInt aIndex ) const
    {
    if ( iSearchEnabled )
        {
        return iFileFinder->Item( aIndex );
        }
    // Return the formatted presentation by global index
    MGflmItemGroup* group = NULL;
    TInt localIndex( 0 );
    GetGroupAndLocalIndex( aIndex, localIndex, group );
    return group->Item( localIndex );
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::ResetModel()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileListModel::ResetModel()
    {
    iGroups->ResetAndDestroy();
    iListingCache->ClearCache();
    iDriveResolver->ClearDrives();
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::FindGroupById()
// Used to provide clients with CGflmItemGroupImpl objects downcasted to
// CGflmItemGroup.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C MGflmItemGroup* CGflmFileListModel::FindGroupById( TInt aId )
    {
    TInt groupCount( iGroups->Count() );

    for( TInt i( 0 ); i < groupCount; i++ )
        {
        CGflmItemGroupImpl* group = iGroups->At( i );
        if ( aId == group->Id() )
            {
            return group;
            }
        }

    // Group not found
    User::Panic( KGFLMPanicText, EInvalidGroupId );

    // Execution never reaches this point
    // following line keeps the compiler happy
    return NULL;
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::CreateGroupL()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
EXPORT_C MGflmItemGroup* CGflmFileListModel::CreateGroupL( TInt aId )
    {
    // Scan the list of groups for identical IDs
    TInt groupCount( iGroups->Count() );
    for ( TInt i( 0 ); i < groupCount; i++ )
        {
        if ( iGroups->At( i )->Id() == aId )
            {
            User::Leave( KErrAlreadyExists );
            }
        }

    CGflmItemGroupImpl* newGroup = CGflmItemGroupImpl::NewLC(
        aId,
        iFs,
        *iListingCache,
        *iItemLocalizer,
        *iDriveResolver );

    iGroups->AppendL( newGroup );
    CleanupStack::Pop( newGroup );
    return newGroup;
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::CreateGroupL()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
EXPORT_C MGflmItemGroup* CGflmFileListModel::CreateGroupL(
        TInt aId, TUint aInclusion )
    {
    MGflmItemGroup* newGroup = CreateGroupL( aId );
    newGroup->SetInclusion( aInclusion );
    return newGroup;
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::SetCustomFilter()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
EXPORT_C void CGflmFileListModel::SetCustomFilter( MGflmItemFilter* aFilter )
    {
    iFilter = aFilter;
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::ResolveMimeTypeL()
// Uses the internal file recognition cache
// (other items were commented in a header).
// -----------------------------------------------------------------------------
EXPORT_C TPtrC CGflmFileListModel::ResolveMimeTypeL( const TDesC& aFilename )
    {
    return iFileRecognizer->RecognizeL( aFilename );
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::IsCorrupt()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
EXPORT_C TBool CGflmFileListModel::IsCorrupt() const
    {
    return iCorrupt;
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::LocalizedName()
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CGflmFileListModel::LocalizedName( const TDesC& aPath ) const
    {
    return iItemLocalizer->Localize( aPath );
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::ClearCache()
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileListModel::ClearCache()
    {
    iListingCache->ClearCache();
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::ClearDriveInfo()
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileListModel::ClearDriveInfo()
    {
    iDriveResolver->ClearDrives();
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::DriveFromPath()
// -----------------------------------------------------------------------------
//
EXPORT_C CGflmDriveItem* CGflmFileListModel::DriveFromPath(
        const TDesC& aPath ) const
    {
    if ( !iDriveResolver->DriveCount() )
        {
        ERROR_LOG( "CGflmFileListModel::DriveFromPath-Forced drive refresh" )
        iDriveResolver->RefreshDrives( iFilter );
        }
    return iDriveResolver->DriveFromPath( aPath );
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::RunL()
// -----------------------------------------------------------------------------
//
void CGflmFileListModel::RunL()
    {
    FUNC_LOG

    NotifyClient( iStatus.Int() );
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::RunError
// -----------------------------------------------------------------------------
//
TInt CGflmFileListModel::RunError( TInt /*aError*/ )
    {
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::DoCancel
// -----------------------------------------------------------------------------
//
void CGflmFileListModel::DoCancel()
    {
    FUNC_LOG

    iShutdown = ETrue;
    iSemaphore.Signal();
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::DriveFromId
// -----------------------------------------------------------------------------
//
EXPORT_C CGflmDriveItem* CGflmFileListModel::DriveFromId(
        const TInt aDrive ) const
    {
    if ( !iDriveResolver->DriveCount() )
        {
        ERROR_LOG( "CGflmFileListModel::DriveFromId-Forced drive refresh" )
        iDriveResolver->RefreshDrives( iFilter );
        }
    return iDriveResolver->DriveFromId( aDrive );
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::EnableSearchMode
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileListModel::EnableSearchMode( const TBool aEnabled )
    {
    iSearchEnabled = aEnabled;
    if ( iSearchEnabled )
        {
        iSearchSortMethod = EByMatch; // Default for search
        }
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::SearchMode
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CGflmFileListModel::SearchMode()
	{
	return iSearchEnabled;
	}

// -----------------------------------------------------------------------------
// CGflmFileListModel::SetSearchStringL
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileListModel::SetSearchStringL( const TDesC& aSearchString )
    {
    iFileFinder->SetSearchStringL( aSearchString );
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::SetSearchFolderL
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileListModel::SetSearchFolderL( const TDesC& aSearchFolder )
    {
    iFileFinder->SetSearchFolderL( aSearchFolder );
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::SetSortMethod
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileListModel::SetSortMethod( TGflmSortMethod aSortMethod )
    {
    if ( iSearchEnabled )
        {
        iSearchSortMethod = aSortMethod;
        }
    else if ( aSortMethod != EByMatch ) // EByMatch is allowed only for search
        {
        iSortMethod = aSortMethod;
        }
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::SortMethod
// -----------------------------------------------------------------------------
//
EXPORT_C TGflmSortMethod CGflmFileListModel::SortMethod() const
    {
    if ( iSearchEnabled )
        {
        return iSearchSortMethod;
        }
    return iSortMethod;
    }

// -----------------------------------------------------------------------------
// CGflmFileListModel::FlushCache()
// -----------------------------------------------------------------------------
//
EXPORT_C void CGflmFileListModel::FlushCache()
    {
    iFileRecognizer->FlushCache();
    }

//  End of File
