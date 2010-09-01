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
* Description:  Goes through the list of given files
*
*/


// INCLUDE FILES
#include "Cfilemanagerindexiterator.h"
#include "CFileManagerEngine.h"
#include "CFileManagerCommonDefinitions.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerIndexIterator::NewL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerIndexIterator* CFileManagerIndexIterator::NewL
    ( CFileManagerEngine& aEngine, 
      CArrayFixFlat<TInt>& aIndexList, 
      const TDesC& aDstDir )
    {
    CFileManagerIndexIterator* self = new( ELeave )
        CFileManagerIndexIterator( aEngine, aIndexList, aDstDir );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerIndexIterator::CFileManagerIndexIterator
// 
// -----------------------------------------------------------------------------
// 
CFileManagerIndexIterator::CFileManagerIndexIterator
    ( CFileManagerEngine& aEngine, 
      CArrayFixFlat< TInt >& aIndexList, 
      const TDesC& aDstDir ):
    iEngine( aEngine ),
    iIndexList( aIndexList ),
    iDstDir( aDstDir )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerIndexIterator::~CFileManagerIndexIterator
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerIndexIterator::~CFileManagerIndexIterator()
    {
    delete iSrc;
    delete iDst;
    }

// -----------------------------------------------------------------------------
// CFileManagerIndexIterator::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerIndexIterator::ConstructL()
    {
    // We need double destination size
    // because source and target folder can be
    // KMaxFileName sized and those are append
    // to same buffer.
    iDst = HBufC::NewL( KFmgrDoubleMaxFileName );
    }

// -----------------------------------------------------------------------------
// CFileManagerIndexIterator::CurrentL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerIndexIterator::CurrentL(
        HBufC** aSrc,
        HBufC** aDst,
        TFileManagerTypeOfItem& aItemType )
    {
    delete iSrc;
    iSrc = NULL;
    iSrc = iEngine.IndexToFullPathL( iIndexList.At( iCurrentIndex ) );

    TPtr ptrDst( iDst->Des() );
    ptrDst.Copy( iDstDir );
    TParsePtrC parse( *iSrc );
    ptrDst.Append( parse.NameAndExt() );

    *aSrc = iSrc;
    *aDst = iDst;

    aItemType = EFileManagerFile;
    }

// -----------------------------------------------------------------------------
// CFileManagerIndexIterator::NextL
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerIndexIterator::NextL()
    {
    ++iCurrentIndex;
    if( iCurrentIndex < iIndexList.Count())
        {
        return ETrue;
        }

    return EFalse;
    }

// End of File
