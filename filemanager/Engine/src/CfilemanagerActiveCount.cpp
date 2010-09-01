/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Calculates given folder size
*
*/


// INCLUDE FILES
#include "CFilemanagerActiveCount.h"

// CONSTANTS
const TInt KFileManagerProcessFilesPerCycle = 20;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerActiveCount::CFileManagerActiveCount
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerActiveCount::CFileManagerActiveCount( RFs& aFs ) :
    iFs( aFs ),
    iFolders( KErrNotFound ),
    iFiles( KErrNotFound),
    iOpenFiles( KErrNotFound )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveCount::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerActiveCount* CFileManagerActiveCount::NewL( 
    RFs& aFs,
    const TDesC& aRootFolder,
    const CFileManagerItemProperties::TFileManagerFileType aType
    )
    {
    CFileManagerActiveCount* self = new( ELeave ) CFileManagerActiveCount( 
        aFs );
    CleanupStack::PushL( self );
    self->ConstructL( aRootFolder, aType );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveCount::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveCount::ConstructL( 
    const TDesC& aRootFolder,
    const CFileManagerItemProperties::TFileManagerFileType aType )
    {

    iDirScan = CDirScan::NewL( iFs );

    if ( aType == CFileManagerItemProperties::EFolder )
        {
        // Set scanning from current directory, take only folders
        // No sorting needed
        iDirScan->SetScanDataL(
            aRootFolder,
            KEntryAttDir | KEntryAttMatchExclusive,
            ESortNone );
        }
    else if ( aType == CFileManagerItemProperties::EFile ||
             aType == CFileManagerItemProperties::EOpen ||
             aType == CFileManagerItemProperties::EReadOnly )
        {
        // Set scanning from current directory, take only file
        // No sorting needed
        iDirScan->SetScanDataL(
            aRootFolder,
            KEntryAttNormal | KEntryAttHidden | KEntryAttSystem,
            ESortNone );
        }
    else
        {
        // Set scanning from current directory,
        // take file and folders except hidden and system files
        // No sorting needed
        iDirScan->SetScanDataL( aRootFolder, KEntryAttDir, ESortNone );
        }

    TRAPD( err, iDirScan->NextL( iDir ) );

    if ( err == KErrNone && iDir )
        {
        if ( aType == CFileManagerItemProperties::EOpen ||
             aType == CFileManagerItemProperties::EReadOnly )
            {
            iFiles = iDir->Count();
            iOpenFiles = 0;
            }
        else if ( aType == CFileManagerItemProperties::EFolder )
            {
            iFolders = iDir->Count();
            delete iDir;
            iDir = NULL;
            iProcessDone = ETrue;
            }
        else if ( aType == CFileManagerItemProperties::EFile )
            {
            iFiles = iDir->Count();
            delete iDir;
            iDir = NULL;
            iProcessDone = ETrue;
            }
        }

    }

// -----------------------------------------------------------------------------
// CFileManagerActiveCount::~CFileManagerActiveCount
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerActiveCount::~CFileManagerActiveCount()
    {
    delete iDirScan;
    delete iDir;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveCount::IsProcessDone
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerActiveCount::IsProcessDone() const
    {
    return iProcessDone;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveCount::StepL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveCount::StepL()
    {
    if( !iDir )
        {
        iProcessDone = ETrue;
        return;
        }

    TInt processCount( KFileManagerProcessFilesPerCycle );
    while( iFileCounter < iDir->Count() && processCount )
        {
        const TEntry& entry( ( *iDir )[ iFileCounter ] );
        iFileName.Copy( iDirScan->FullPath() );
        iFileName.Append( entry.iName );
        TBool fileIsOpen( EFalse );
        if ( KErrNone == iFs.IsFileOpen( iFileName, fileIsOpen ) )
            {
            if ( fileIsOpen )
                {
                ++iOpenFiles;
                }
            }
        ++iFileCounter;
        --processCount;
        }

    if ( iFileCounter >= iDir->Count() )
        {
        delete iDir;
        iDir = NULL;
        iDirScan->NextL( iDir );
        if ( !iDir )
            {
            iProcessDone = ETrue;
            }
        iFileCounter = 0;
        }

    }

// -----------------------------------------------------------------------------
// CFileManagerActiveCount::FolderCount()
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerActiveCount::FolderCount() const
    {
    if ( iProcessDone )
        {
        return iFolders;
        }
    return KErrNotFound;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveCount::FileCount()
//
// -----------------------------------------------------------------------------
//
TInt CFileManagerActiveCount::FileCount() const
    {
    if ( iProcessDone )
        {
        return iFiles;
        }
    return KErrNotFound;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveCount::OpenFiles()
//
// -----------------------------------------------------------------------------
//
TInt CFileManagerActiveCount::OpenFiles() const
    {
    if ( iProcessDone )
        {
        return iOpenFiles;
        }
    return KErrNotFound;
    }

// End of File
