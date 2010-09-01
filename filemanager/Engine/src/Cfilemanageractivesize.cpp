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


// INCLUDES
#include "Cfilemanageractivesize.h"

// CONSTANTS
const TInt KFileManagerProcessFilesPerCycle = 20;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerActiveSize::CFileManagerActiveSize
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerActiveSize::CFileManagerActiveSize( RFs& aFs ) :
    iFs( aFs )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveSize::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerActiveSize* CFileManagerActiveSize::NewL(
        RFs& aFs, const TDesC& aRootFolder )
    {
    CFileManagerActiveSize* self = new( ELeave ) CFileManagerActiveSize(
        aFs );
    CleanupStack::PushL( self );
    self->ConstructL( aRootFolder );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveSize::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveSize::ConstructL( const TDesC& aRootFolder )
    {
    iDirScan = CDirScan::NewL( iFs );
    // Set scanning from current directory,
    // take files and folder including the hidden and system files
    // No sorting needed
    iDirScan->SetScanDataL(
        aRootFolder,
        KEntryAttNormal | KEntryAttHidden | KEntryAttSystem,
        ESortNone );
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveSize::~CFileManagerActiveSize
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerActiveSize::~CFileManagerActiveSize()
    {
    delete iDirScan;
    delete iDir;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveSize::IsProcessDone
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerActiveSize::IsProcessDone() const
    {
    return iProcessDone;
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveSize::StepL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerActiveSize::StepL()
    {
    if ( !iDir )
        {
        iDirScan->NextL( iDir );
        if( !iDir )
            {
            iProcessDone = ETrue;
            return;
            }
        }

    TInt processCount( KFileManagerProcessFilesPerCycle );
    while( iFileCounter < iDir->Count() && processCount )
        {
        const TEntry& item( ( *iDir )[ iFileCounter ] );
        iSize += item.FileSize();
        ++iFileCounter;
        --processCount;
        }

    if ( iFileCounter >= iDir->Count() )
        {
        delete iDir;
        iDir = NULL;
        iFileCounter = 0;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerActiveSize::GetFolderSize()
// 
// -----------------------------------------------------------------------------
// 
TInt64 CFileManagerActiveSize::GetFolderSize() const
    {
    if( iProcessDone )
        {
        return iSize;
        }
    return KErrNotFound;
    }

// End of File
