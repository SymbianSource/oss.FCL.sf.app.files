/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Class wraps rename operation
*
*/


// INCLUDES
#include <e32std.h>
#include "CFileManagerActiveRename.h"
#include "CFileManagerEngine.h"
#include "CFileManagerUtils.h"
#include "CFileManagerCommonDefinitions.h"
#include "FileManagerDebug.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerActiveRename::CFileManagerActiveRename
// -----------------------------------------------------------------------------
//
CFileManagerActiveRename::CFileManagerActiveRename(
        CFileManagerEngine& aEngine,
        CFileManagerUtils& aUtils ) :
    iEngine( aEngine ),
    iUtils( aUtils ),
    iFs( aEngine.Fs() )
    {
    FUNC_LOG
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveRename::~CFileManagerActiveRename
// -----------------------------------------------------------------------------
//
CFileManagerActiveRename::~CFileManagerActiveRename()
    {
    FUNC_LOG

    delete iName;
    delete iNewName;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveRename::NewL
// -----------------------------------------------------------------------------
//
CFileManagerActiveRename* CFileManagerActiveRename::NewL(
        CFileManagerEngine& aEngine,
        CFileManagerUtils& aUtils,
        const TDesC& aName,
        const TDesC& aNewName )
    {
    FUNC_LOG

    CFileManagerActiveRename* self = new ( ELeave ) CFileManagerActiveRename(
        aEngine, aUtils );
    CleanupStack::PushL( self );
    self->ConstructL( aName, aNewName );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveRename::ConstructL
// -----------------------------------------------------------------------------
//
void CFileManagerActiveRename::ConstructL(
        const TDesC& aName, const TDesC& aNewName )
    {
    FUNC_LOG

    iName = aName.AllocL();
    iNewName = aNewName.AllocL();
    // Remove white spaces from end, file server also ignores those
    iNewName->Des().TrimRight();
    iName->Des().TrimRight();

    iIsRemoteDrive = iUtils.IsRemoteDrive( aName );
    BaseConstructL();
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveRename::ThreadFunctionL
// -----------------------------------------------------------------------------
//
void CFileManagerActiveRename::ThreadFunctionL( const TBool& aCanceled )
    {
    FUNC_LOG

    TInt maxSubPath( 0 );
    TBool isFolder( CFileManagerUtils::HasFinalBackslash( *iName ) );
    if ( isFolder && !iIsRemoteDrive )
        {
        // On local drives, solve maximum subfolder path to avoid too long paths
        maxSubPath = ThreadGetMaxSubfolderPathL( aCanceled );
        }
    if ( iNewName->Length() + maxSubPath > KMaxFileName )
        {
        User::Leave( KErrBadName );
        }
    if ( isFolder )
        {
        User::LeaveIfError( iFs.Rename( *iName, *iNewName ) );
        }
    else
        {
        // Replace the target item if exist
        // if we have came here, it is already asked that
        // user want's to overwrite other item
        User::LeaveIfError( iFs.Replace( *iName, *iNewName ) );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveRename::ThreadGetMaxSubfolderPathL
// -----------------------------------------------------------------------------
//
TInt CFileManagerActiveRename::ThreadGetMaxSubfolderPathL(
        const TBool& aCanceled )
    {
    FUNC_LOG

    CDirScan* dirScan = CDirScan::NewLC( iFs );

    // Set scanning from current directory, 
    // take files and folder including the hidden and system files
    // No sorting needed
    dirScan->SetScanDataL(
        *iName,
        KEntryAttNormal | KEntryAttDir | KEntryAttHidden | KEntryAttSystem,
        ESortNone );

    TInt ret( 0 );
    CDir* dir = NULL;
    dirScan->NextL( dir );
    while( dir )
        {
        CleanupStack::PushL( dir );
        if ( aCanceled )
            {
            User::Leave( KErrCancel );
            }
        TInt count( dir->Count() );
        for ( TInt i( 0 ); i < count; ++i )
            {
            if ( aCanceled )
                {
                User::Leave( KErrCancel );
                }
            const TEntry& entry( ( *dir )[ i ] );
            TPtrC abbrPath( dirScan->AbbreviatedPath() );
            TInt abbrPathLen( abbrPath.Length() );
            if ( abbrPathLen && abbrPath[ 0 ] == KFmgrBackslash()[ 0 ] )
                {
                --abbrPathLen; // Initial backslash is already included
                }
            TInt pathLen( abbrPathLen + entry.iName.Length() );
            if( entry.IsDir() ) // Add space for trailing backslash
                {
                ++pathLen;
                }
            if( pathLen > ret )
                {
                ret = pathLen;
                }
            if ( pathLen > KMaxFileName )
                {
                User::Leave( KErrBadName );
                }
            }
        CleanupStack::PopAndDestroy( dir );
        dirScan->NextL( dir );
        }
    CleanupStack::PopAndDestroy( dirScan );
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveRename::CancelThreadFunction
// -----------------------------------------------------------------------------
//
void CFileManagerActiveRename::CancelThreadFunction()
    {
    FUNC_LOG

    if ( iName )
        {
        iEngine.CancelTransfer( *iName );
        }
    }

// End of File
