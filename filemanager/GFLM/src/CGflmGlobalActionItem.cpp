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
* Description:  Global action item definitions
*
*/



// INCLUDE FILES
#include "CGflmGlobalActionItem.h"
#include <e32std.h>


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGflmGlobalActionItem::CGflmGlobalActionItem
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGflmGlobalActionItem::CGflmGlobalActionItem()
    {
    }

// -----------------------------------------------------------------------------
// CGflmGlobalActionItem::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CGflmGlobalActionItem::ConstructL( const TDesC& aCaption )
    {
    iCaption = aCaption.AllocL();
    }

// -----------------------------------------------------------------------------
// CGflmGlobalActionItem::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CGflmGlobalActionItem* CGflmGlobalActionItem::NewLC(
        TInt aId, const TDesC& aCaption )
    {
    CGflmGlobalActionItem* self = new( ELeave ) CGflmGlobalActionItem;

    CleanupStack::PushL( self );
    self->iId = aId;
    self->ConstructL( aCaption );

    return self;
    }

// -----------------------------------------------------------------------------
// CGflmGlobalActionItem::~CGflmGlobalActionItem
//
// -----------------------------------------------------------------------------
//
CGflmGlobalActionItem::~CGflmGlobalActionItem()
    {
    delete iCaption;

    }

// -----------------------------------------------------------------------------
// CGflmGlobalActionItem::Type()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
CGflmGroupItem::TGflmItemType CGflmGlobalActionItem::Type() const
    {
    return EGlobalActionItem;
    }

// -----------------------------------------------------------------------------
// CGflmGlobalActionItem::Name()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TPtrC CGflmGlobalActionItem::Name() const
    {
    return iCaption->Des();
    }

// -----------------------------------------------------------------------------
// CGflmGlobalActionItem::Date()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TTime CGflmGlobalActionItem::Date() const
    {
    // Date of an action item is undefined
    // Groups containing action items shouldn't be sorted by date
    return TTime( 0 );
    }

// -----------------------------------------------------------------------------
// CGflmGlobalActionItem::Size()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt64 CGflmGlobalActionItem::Size() const
    {
    return 0;
    }

// -----------------------------------------------------------------------------
// CGflmGlobalActionItem::Ext()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TPtrC CGflmGlobalActionItem::Ext() const
    {
    return TPtrC( KNullDesC );
    }

// -----------------------------------------------------------------------------
// CGflmGlobalActionItem::Id()
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CGflmGlobalActionItem::Id() const
    {
    return iId;
    }

//  End of File
