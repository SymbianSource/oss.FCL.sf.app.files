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
* Description:  Holds the array of CFileManagerFolderEntry
*
*/


// INCLUDE FILES
#include <barsread.h>				// TResourceReader
#include <bautils.h>				// BaflUtils::EnsurePathExists
#include <coemain.h>				// CCoeEnv
#ifdef RD_MULTIPLE_DRIVE
 #include <pathinfo.h>
 #include <driveinfo.h>
#endif // RD_MULTIPLE_DRIVE
#include "Cfilemanagerfolderarray.h"
#ifndef RD_MULTIPLE_DRIVE
 #include "cfilemanagerfolderentry.h"
#endif // RD_MULTIPLE_DRIVE
#include "CFileManagerCommonDefinitions.h"
#include "CFileManagerUtils.h"


// CONSTANTS
const TInt KCreateCheckMaskMax = 32;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerFolderArray::NewL
// 
// -----------------------------------------------------------------------------
// 
#ifdef RD_MULTIPLE_DRIVE
CFileManagerFolderArray* CFileManagerFolderArray::NewL(
        CFileManagerUtils& aUtils )
	{
	return new ( ELeave ) CFileManagerFolderArray( aUtils );
    }
#else // RD_MULTIPLE_DRIVE
CFileManagerFolderArray* CFileManagerFolderArray::NewL(
        TInt aResId, CFileManagerUtils& aUtils )
	{
	CFileManagerFolderArray* self =
	    new( ELeave ) CFileManagerFolderArray(
	        aUtils );

	CleanupStack::PushL( self );
	self->ConstructL( aResId );
	CleanupStack::Pop( self );

	return self;
	}
#endif // RD_MULTIPLE_DRIVE

// -----------------------------------------------------------------------------
// CFileManagerFolderArray::CFileManagerFolderArray
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerFolderArray::CFileManagerFolderArray(
        CFileManagerUtils& aUtils ) :
    iUtils( aUtils )
    {
    }

#ifndef RD_MULTIPLE_DRIVE
// -----------------------------------------------------------------------------
// CFileManagerFolderArray::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderArray::ConstructL( TInt aResId )
	{
    RArray< TInt > removableDrives;
    CleanupClosePushL( removableDrives );

    iUtils.DriveList( removableDrives, KDriveAttRemovable );
    TInt drvCount( removableDrives.Count() );

	TResourceReader reader;
    CCoeEnv::Static()->CreateResourceReaderLC( reader, aResId );
	TInt count( reader.ReadInt16() );

    TChar drvLetter( 0 );
    TBool isRemovable( EFalse );

	for( TInt i( 0 ); i < count; i++ )
		{
		CFileManagerFolderEntry* entry =
		    CFileManagerFolderEntry::NewLC( reader );
		iFolderArray.AppendL( entry );
		CleanupStack::Pop( entry );

        // The default folders are same to all removable drives.
        // Add the folders dynamically for undefined removable drives.
        TPtrC ptr( entry->Name() );
        if ( ptr[ 0 ] != drvLetter )
            {
            drvLetter = ptr[ 0 ];
            isRemovable = iUtils.IsRemovableDrive( ptr );
            }
		if ( isRemovable )
		    {
		    TInt baseDrv = TDriveUnit( ptr );
		    for( TInt j( 0 ); j < drvCount; j++ )
		        {
		        TInt drv( removableDrives[ j ] );
		        if ( drv != baseDrv )
		            {
		            entry = CFileManagerFolderEntry::NewLC( drv, ptr );
		            iFolderArray.AppendL( entry );
		            CleanupStack::Pop( entry );
		            }
		        }
		    }
		}

    CleanupStack::PopAndDestroy(); // reader
    CleanupStack::PopAndDestroy( &removableDrives );
	}
#endif // RD_MULTIPLE_DRIVE

// -----------------------------------------------------------------------------
// CFileManagerFolderArray::~CFileManagerFolderArray
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerFolderArray::~CFileManagerFolderArray()
    {
#ifndef RD_MULTIPLE_DRIVE
    iFolderArray.ResetAndDestroy();
    iFolderArray.Close();
#endif // RD_MULTIPLE_DRIVE
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderArray::Contains
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerFolderArray::Contains( const TDesC& aFolderName )
	{
#ifdef RD_MULTIPLE_DRIVE
    // Remote drives has no default folders
    if ( !iUtils.IsRemoteDrive( aFolderName ) )
        {
        TInt pathType( PathInfo::PathType( aFolderName ) );
        switch ( pathType )
            {
            // If non default folder but localized,
            // handle it here as default folder
            case PathInfo::ENotSystemPath:
                {
                TPtrC ptr( iUtils.LocalizedName( aFolderName ) );
                if ( ptr.Length() )
                    {
                    return ETrue;
                    }
                return EFalse;
                }
            // Ignore root folders
            case PathInfo::EPhoneMemoryRootPath: // FALL THROUGH
            case PathInfo::EMemoryCardRootPath: // FALL THROUGH
            case PathInfo::ERomRootPath:
                {
                return EFalse;
                }
            // Accept other folders
            default:
                {
                return ETrue;
                }
            }
        }
    return EFalse;
#else  // RD_MULTIPLE_DRIVE
	CFileManagerFolderEntry* entry = FindFolder( aFolderName );
	return ( entry ? ETrue : EFalse );
#endif // RD_MULTIPLE_DRIVE
	}

// -----------------------------------------------------------------------------
// CFileManagerFolderArray::CreateFolders
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderArray::CreateFolders(
        const TInt aDrive, const TBool aForced )
	{
    // If not forced creation, check if folders have been already created
    if ( !aForced && aDrive >= 0 && aDrive < KCreateCheckMaskMax  )
        {
        if ( ( 1 << aDrive ) & iCreateCheckMask )
            {
            return; // Folders have already been created
            }
        }
#ifdef RD_MULTIPLE_DRIVE
    if ( aDrive != KErrNotFound ) // Support only explicitly given drive id
        {
        TRAP_IGNORE( DoCreateFoldersL( aDrive ) );
        // Update folder creation mask
        iCreateCheckMask |= 1 << aDrive;
        }
#else // RD_MULTIPLE_DRIVE
    RFs& fs( iUtils.Fs() );
    TInt count( iFolderArray.Count() );
	for( TInt i( 0 ); i < count; i++ )
		{
		TPtrC defFolder( iFolderArray[ i ]->Name() );

        TInt drv = TDriveUnit( defFolder );
        if ( aDrive >= 0 )
            {
            // Apply drive filtering
            if ( drv != aDrive )
                {
                continue;
                }
            }

        // Update folder creation mask
        iCreateCheckMask |= 1 << drv;

        // Make sure that path exists
        fs.MkDirAll( defFolder );

        // Clear the possible hidden flag, all default folders
        // should be visible
        TEntry entry;
        if ( fs.Entry( defFolder, entry ) == KErrNone )
            {
            if( entry.IsHidden() )
                {
                // If setting fails, File Manager can still go on
                fs.SetEntry(
                    defFolder, entry.iModified, 0, KEntryAttHidden );
                }
            }
		}
#endif // RD_MULTIPLE_DRIVE
    }

#ifndef RD_MULTIPLE_DRIVE
// -----------------------------------------------------------------------------
// CFileManagerFolderArray::FindFolder
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFolderEntry* CFileManagerFolderArray::FindFolder(
        const TDesC& aFolderName )
	{
    if ( !iSorted )
        {
        TLinearOrder< CFileManagerFolderEntry > key(
            CFileManagerFolderEntry::CompareSort );
        iFolderArray.Sort( key );
        iSorted = ETrue;
        }

    TInt index( iFolderArray.FindInOrder(
        aFolderName, CFileManagerFolderEntry::CompareFind ) );
    if ( index >= 0 && index < iFolderArray.Count() )
        {
        // Match found
        return iFolderArray[ index ];
        }

    return NULL;
	}
#endif // RD_MULTIPLE_DRIVE

#ifdef RD_MULTIPLE_DRIVE
// -----------------------------------------------------------------------------
// CFileManagerFolderArray::DoCreateFolders
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderArray::DoCreateFoldersL( const TInt aDrive )
    {
    TUint drvStatus( 0 );
    RFs& fs( iUtils.Fs() );
    User::LeaveIfError( DriveInfo::GetDriveStatus( fs, aDrive, drvStatus ) );
    if ( !( drvStatus & DriveInfo::EDriveUserVisible ) ||
        ( drvStatus & ( DriveInfo::EDriveRemote |
                        DriveInfo::EDriveReadOnly |
                        DriveInfo::EDriveUsbMemory ) ) )
        {
        return; // Default folder creation is not allowed for this drive type
        }

    TEntry entry;
    CDesCArray* array = PathInfo::GetListOfPathsLC( aDrive );
    TInt count( array->MdcaCount() );
    for ( TInt i( 0 ); i < count; ++i )
        {
        TPtrC fullPath( array->MdcaPoint( i ) );
        TBool allow( ETrue );

        if ( drvStatus & DriveInfo::EDriveRemovable )
            {
            // Filter few folder types from physically removable memory cards
            TInt pathType( PathInfo::PathType( fullPath ) );
            switch( pathType )
                {
                case PathInfo::EGamesPath: // FALL THROUGH
                case PathInfo::EInstallsPath: // FALL THROUGH
                case PathInfo::EGsmPicturesPath: // FALL THROUGH
                case PathInfo::EMmsBackgroundImagesPath: // FALL THROUGH
                case PathInfo::EPresenceLogosPath:
                    {
                    allow = EFalse;
                    }
                default:
                    {
                    break;
                    }
                }
            }

        if ( allow )
            {
        fs.MkDirAll( fullPath ); // Ignore error

        // Clear the possible hidden flag, all default folders
        // should be visible
        if ( fs.Entry( fullPath, entry ) == KErrNone )
            {
            if( entry.IsHidden() )
                {
                // If setting fails, File Manager can still go on
                fs.SetEntry(
                    fullPath, entry.iModified, 0, KEntryAttHidden );
                    }
                }
            }
        }
    CleanupStack::PopAndDestroy( array );
    }
#endif // RD_MULTIPLE_DRIVE

//  End of File  
