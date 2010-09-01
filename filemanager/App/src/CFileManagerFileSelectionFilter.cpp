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
* Description:  Filters PlatSec dirs from file selection
*
*/



// INCLUDE FILES
#include <f32file.h>
#include "CFileManagerFileSelectionFilter.h"
#include "CFileManagerEngine.h"
#include "CFileManagerCommonDefinitions.h"


// ============================ MEMBER FUNCTIONS ===============================
// -----------------------------------------------------------------------------
// CFileManagerFileSelectionFilter::CFileManagerFileSelectionFilter
// 
// -----------------------------------------------------------------------------
//
CFileManagerFileSelectionFilter::CFileManagerFileSelectionFilter(
        CFileManagerEngine& aEngine )
        : iEngine( aEngine )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerFileSelectionFilter::Accept
// 
// -----------------------------------------------------------------------------
//
TBool CFileManagerFileSelectionFilter::Accept( const TDesC& aDriveAndPath,
                                               const TEntry& aEntry ) const
    {
    if ( aEntry.IsHidden() || aEntry.IsSystem() )
        {
        return EFalse;
        }
    if ( !aEntry.IsDir() )
        {
        return ETrue;
        }
    HBufC* fullPath = HBufC::New( KMaxPath );
    if ( !fullPath )
        {
        return EFalse;
        }
    TPtr ptr( fullPath->Des() );
    ptr.Copy( aDriveAndPath );
    ptr.Append( aEntry.iName );
    ptr.Append( KFmgrBackslash );
    TBool isVisible( !iEngine.IsSystemFolder( ptr ) );
    delete fullPath;
    return isVisible;
    }

//  End of File
