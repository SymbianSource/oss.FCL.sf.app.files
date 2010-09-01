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
* Description:  This class is used to provide localized names
*
*/



// INCLUDE FILES
#include "CGflmItemLocalizer.h"
#include "GflmUtils.h"
#include "GFLM.hrh"
#include <CDirectoryLocalizer.h>


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmItemLocalizer::CGflmItemLocalizer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGflmItemLocalizer::CGflmItemLocalizer()
    {
    }

// -----------------------------------------------------------------------------
// CGflmItemLocalizer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CGflmItemLocalizer* CGflmItemLocalizer::NewL()
    {
    CGflmItemLocalizer* self = new( ELeave ) CGflmItemLocalizer();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CGflmItemLocalizer::ConstructL
// -----------------------------------------------------------------------------
//
void CGflmItemLocalizer::ConstructL()
    {
    iLocalizer = CDirectoryLocalizer::NewL();
    }

// -----------------------------------------------------------------------------
// CGflmItemLocalizer::~CGflmItemLocalizer
// -----------------------------------------------------------------------------
//
CGflmItemLocalizer::~CGflmItemLocalizer()
    {
    delete iLocalizer;
    // iWorkThreadLocalizer cannot be freed here because it was 
    // allocated by another thread
    }

// -----------------------------------------------------------------------------
// CGflmItemLocalizer::Localize()
// -----------------------------------------------------------------------------
//
TPtrC CGflmItemLocalizer::Localize( const TDesC& aPath ) const
    {
    return Localize( *iLocalizer, aPath );
    }

// -----------------------------------------------------------------------------
// CGflmItemLocalizer::LocalizeFromWorkThread()
// -----------------------------------------------------------------------------
//
TPtrC CGflmItemLocalizer::LocalizeFromWorkThread( const TDesC& aPath ) const
    {
    if ( iWorkThreadLocalizer )
        {
        return Localize( *iWorkThreadLocalizer, aPath );
        }
    return TPtrC( KNullDesC );
    }

// -----------------------------------------------------------------------------
// CGflmItemLocalizer::SetupWorkThreadLocalizerL()
// -----------------------------------------------------------------------------
//
void CGflmItemLocalizer::SetupWorkThreadLocalizerL()
    {
    if ( !iWorkThreadLocalizer )
        {
        iWorkThreadLocalizer = CDirectoryLocalizer::NewL();
        }
    }

// -----------------------------------------------------------------------------
// CGflmItemLocalizer::SetupWorkThreadLocalizer()
// -----------------------------------------------------------------------------
//
void CGflmItemLocalizer::ReleaseWorkThreadLocalizer()
    {
    delete iWorkThreadLocalizer;
    iWorkThreadLocalizer = NULL;
    }

// -----------------------------------------------------------------------------
// CGflmItemLocalizer::SetupWorkThreadLocalizer()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TPtrC CGflmItemLocalizer::Localize(
        CDirectoryLocalizer& aLocalizer, const TDesC& aPath ) const
    {
    aLocalizer.SetFullPath( aPath );
    if( aLocalizer.IsLocalized() )
        {
        // Ensure that localized name is legal.
        // Some localizations may have dot in the end that makes it 
        // invalid for file system point of view.
        return GflmUtils::StripFinalDot( aLocalizer.LocalizedName() );
        }
    return TPtrC( KNullDesC );
    }

//  End of File
