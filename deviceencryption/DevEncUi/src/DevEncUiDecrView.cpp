/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of CDevEncUiDecrView.
*
*/

// INCLUDE FILES
// Class includes
#include "DevEncUiDecrView.h"

// System includes
#include <aknViewAppUi.h>	// CAknViewAppUi
#include <devencui.rsg>

// User includes
#include "DevEncLog.h"
#include "DevEnc.hrh"
#include "DevEncUiDecrViewContainer.h"
#include "DevEncUiMemoryEntity.h"

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CDevEncUiDecrView::CDevEncUiDecrView
//
// -----------------------------------------------------------------------------
//
CDevEncUiDecrView::~CDevEncUiDecrView()
	{
	delete iContainer;
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrView::NewL
//
// -----------------------------------------------------------------------------
//
CDevEncUiDecrView* CDevEncUiDecrView::NewL( RArray<CDevEncUiMemoryEntity*>& aMemEntities )
	{
	CDevEncUiDecrView* self = CDevEncUiDecrView::NewLC( aMemEntities );
	CleanupStack::Pop( self );
	return self;
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrView::NewLC
//
// -----------------------------------------------------------------------------
//
CDevEncUiDecrView* CDevEncUiDecrView::NewLC( RArray<CDevEncUiMemoryEntity*>& aMemEntities )
	{
	CDevEncUiDecrView* self = new ( ELeave ) CDevEncUiDecrView( aMemEntities );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrView::CDevEncUiDecrView
//
// -----------------------------------------------------------------------------
//
CDevEncUiDecrView::CDevEncUiDecrView( RArray<CDevEncUiMemoryEntity*>& aMemEntities )
    : iMemEntities( aMemEntities )
	{
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrView::Container
//
// -----------------------------------------------------------------------------
//
CDevEncUiDecrViewContainer* CDevEncUiDecrView::Container()
	{
	return iContainer;
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrView::GetAppUi
//
// -----------------------------------------------------------------------------
//
CDevEncUiAppUi& CDevEncUiDecrView::GetAppUi()
    {
    CAknViewAppUi* aknViewAppUi = AppUi();
    CDevEncUiAppUi* appUi = reinterpret_cast<CDevEncUiAppUi*>( aknViewAppUi );
    return ( *appUi );
    }

// -----------------------------------------------------------------------------
// CDevEncUiDecrView::ConstructL
//
// -----------------------------------------------------------------------------
//
void CDevEncUiDecrView::ConstructL()
	{
	BaseConstructL( R_DEVENCUI_DECRVIEW );
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrView::Id
//
// -----------------------------------------------------------------------------
//
TUid CDevEncUiDecrView::Id() const
	{
	return TUid::Uid( EDevEncUiDecrViewId );
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrView::DoActivateL
//
// -----------------------------------------------------------------------------
//
void CDevEncUiDecrView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
									 TUid /*aCustomMessageId*/,
									 const TDesC8& /*aCustomMessage*/ )
	{
	if ( !iContainer )
		{
		iContainer = CDevEncUiDecrViewContainer::NewL( AppUi()->ClientRect(),
                                                       *this );
		iContainer->SetMopParent( this );
		AppUi()->AddToStackL( *this, iContainer );
		}
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrView::DoDeactivate
//
// -----------------------------------------------------------------------------
//
void CDevEncUiDecrView::DoDeactivate()
	{
	if (iContainer)
		{
		AppUi()->RemoveFromStack( iContainer );
		delete iContainer;
		iContainer = NULL;
		}
	}

// -----------------------------------------------------------------------------
// CDevEncUiDecrView::HandleCommandL
//
// -----------------------------------------------------------------------------
//
void CDevEncUiDecrView::HandleCommandL( TInt aCommand )
	{

	switch ( aCommand )
		{

		case EAknSoftkeyBack:
			{
			AppUi()->HandleCommandL( EAknSoftkeyBack );
			break;
			}
		default:
			{
			AppUi()->HandleCommandL( aCommand );
			}
		}
	}

// --------------------------------------------------------------------------
//  Called by the framework when the application status pane
//  size is changed.  Passes the new client rectangle to the container.
// --------------------------------------------------------------------------
void CDevEncUiDecrView::HandleStatusPaneSizeChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

void CDevEncUiDecrView::HandleResourceChangeL( TInt aType )
    {
    if  ( aType == KEikDynamicLayoutVariantSwitch )
        {
        // do re-layout
        if ( iContainer )
            {
            iContainer->SetRect( ClientRect() );
            iContainer->HandleResourceChange( aType );
            }
        }
    }


// End of File
