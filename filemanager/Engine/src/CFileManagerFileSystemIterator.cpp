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
* Description:  Goes through folder tree one item at time
*
*/



// INCLUDE FILES
#include "CFileManagerFileSystemIterator.h"
#include "CFileManagerEngine.h"
#include "CFileManagerCommonDefinitions.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerFileSystemIterator::NewL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFileSystemIterator* CFileManagerFileSystemIterator::NewL(
        RFs& aFs,
        const TDesC& aSrcDir,
        const TDesC& aDstDir,
        CFileManagerEngine& aEngine )
    {
    CFileManagerFileSystemIterator* self =
        new (ELeave) CFileManagerFileSystemIterator( aDstDir, aEngine );
    CleanupStack::PushL( self );
    self->ConstructL( aFs, aSrcDir );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileSystemIterator::CFileManagerFileSystemIterator
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFileSystemIterator::CFileManagerFileSystemIterator(
        const TDesC& aDstDir, CFileManagerEngine& aEngine ) :
    iCount( KErrNotFound ),
    iDstDir( aDstDir ),
    iEngine( aEngine )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerFileSystemIterator::~CFileManagerFileSystemIterator
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFileSystemIterator::~CFileManagerFileSystemIterator()
    {
    delete iSrcDir;
    delete iSrc;
    delete iDst;
    delete iDir;
    delete iDirScan;
    }

// -----------------------------------------------------------------------------
// CFileManagerFileSystemIterator::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileSystemIterator::ConstructL(
        RFs& aFs, const TDesC& aSrcDir )
    {
    iDirScan = CDirScan::NewL( aFs );
    // Go through the folders first
    iDirScan->SetScanDataL(
        aSrcDir,
        KEntryAttNormal | KEntryAttHidden | KEntryAttSystem |
        KEntryAttDir | KEntryAttMatchExclusive,
        ESortNone );
    iSrc = HBufC::NewL( KMaxFileName );
    // We need double destination size
    // because source and target folder can be
    // KMaxFileName sized and those are append
    // to same buffer.
    iDst = HBufC::NewL( KFmgrDoubleMaxFileName );
    iDst->Des().Copy( iDstDir );
    iDirScan->NextL( iDir );
    iSrcDir = aSrcDir.AllocL();
    iSrc->Des().Copy( aSrcDir );
    }

// -----------------------------------------------------------------------------
// CFileManagerFileSystemIterator::CurrentL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFileSystemIterator::CurrentL(
        HBufC** aSrc, HBufC** aDst, TFileManagerTypeOfItem& aItemType )
    {
    *aSrc = NULL;
    *aDst = NULL;

    if( iCount == KErrNotFound )
        {
        // Handle destination root folder first
        aItemType = EFileManagerFolder;
        *aSrc = iSrc; // contains iSrcDir
        *aDst = iDst; // contains iDstDir
        }
    else if( iDir && iCount < iDir->Count() )
        {
        // Handle folder contents
        if( !iFolderScanDone )
            {
            const TEntry& entry( ( *iDir )[ iCount ] );

            TPtr ptrSrc( iSrc->Des() );
            ptrSrc.Copy( iDirScan->FullPath() );
            ptrSrc.Append( entry.iName );
            ptrSrc.Append( KFmgrBackslash );

            TPtr ptrDst( iDst->Des() );
            ptrDst.Copy( iDstDir );
            ptrDst.Append( LocalizedAbbreviatedPath() );

            TPtrC ptr( iEngine.LocalizedName( *iSrc ) );
            if( ptr.Length() )
                {
                ptrDst.Append( ptr );
                }
            else
                {
                ptrDst.Append( entry.iName );
                }
            ptrDst.Append( KFmgrBackslash );

            aItemType = EFileManagerFolder;
            }
        else 
            {
            const TEntry& entry( ( *iDir )[ iCount ] );

            TPtr ptrSrc( iSrc->Des() );
            ptrSrc.Copy( iDirScan->FullPath() );
            ptrSrc.Append( entry.iName );

            TPtr ptrDst( iDst->Des() );
            ptrDst.Copy( iDstDir );
            ptrDst.Append( LocalizedAbbreviatedPath() );
            ptrDst.Append( entry.iName );

            aItemType = EFileManagerFile;
            }
        *aSrc = iSrc;
        *aDst = iDst;
        }
    else
        {
        aItemType = EFileManagerNoType;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFileSystemIterator::NextL
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerFileSystemIterator::NextL()
    {
    if ( !iDir )
       {
       return EFalse;
       }

    ++iCount;

    // Get next dir
    if( iCount >= iDir->Count() )
        {
        delete iDir;
        iDir = NULL;
        iCount = 0;
        iDirScan->NextL( iDir );
        iLocalizedAbbreviatedPath.Zero();
        }

    // Are we done
    if( iDir )
        {
        return ETrue;
        }
    
    // Folders have been scanned through, now same scanning with files
    if( !iFolderScanDone )
        {
        iDirScan->SetScanDataL( *iSrcDir,
            KEntryAttNormal | KEntryAttHidden | KEntryAttSystem,
            ESortNone );
        iDirScan->NextL( iDir );
        iLocalizedAbbreviatedPath.Zero();
        iCount = 0;
        iFolderScanDone = ETrue;
        return ETrue;
        }

    return EFalse;
    }

// ------------------------------------------------------------------------------
// CFileManagerFileSystemIterator::LocalizedAbbreviatedPath
//
// ------------------------------------------------------------------------------
//
TPtrC CFileManagerFileSystemIterator::LocalizedAbbreviatedPath()
    {
    TPtrC abbrPath( iDirScan->AbbreviatedPath() );

    if( !iLocalizedAbbreviatedPath.Length() && abbrPath.Length() > 1 )
        {
        iLocalizedAbbreviatedPath.Append(
            iEngine.LocalizedName( iDirScan->FullPath() ) );

        if( !iLocalizedAbbreviatedPath.Length() )
            {
            iLocalizedAbbreviatedPath.Append(
                abbrPath.Right( abbrPath.Length() - 1 ) );
            }
        else
            {
            iLocalizedAbbreviatedPath.Append( KFmgrBackslash );
            }
        }
    return iLocalizedAbbreviatedPath;
    }

// End of File
