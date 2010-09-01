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
* Description:  Implementation of CDevEncUiEncrView.
*
*/

// INCLUDE FILES
// Class includes
#include "DevEncUiEncrView.h"

// System includes
#include <aknViewAppUi.h>	// CAknViewAppUi
#include <devencui.rsg>

// User includes
#include "DevEncLog.h"
#include "DevEnc.hrh"
#include "DevEncUiEncrViewContainer.h"
#include "DevEncUiMemoryEntity.h"

// ================= MEMBER FUNCTIONS =======================

/**
* Destructor.  Frees up memory for the iLabel.
*/
CDevEncUiEncrView::~CDevEncUiEncrView()
	{
	delete iContainer;
	}

/**
* Symbian OS 2 phase constructor.
* Constructs the CDevEncUiEncrView using the NewLC method, popping
* the constructed object from the CleanupStack before returning it.
*
* @param aRect The rectangle for this window
* @return The newly constructed CDevEncUiEncrView
*/
CDevEncUiEncrView* CDevEncUiEncrView::NewL( RArray<CDevEncUiMemoryEntity*>& aMemEntities )
	{
	CDevEncUiEncrView* self = CDevEncUiEncrView::NewLC( aMemEntities );
	CleanupStack::Pop( self );
	return self;
	}

/**
* Symbian OS 2 phase constructor.
* Constructs the CDevEncUiEncrView using the constructor and ConstructL
* method, leaving the constructed object on the CleanupStack before returning it.
*
* @param aRect The rectangle for this window
* @return The newly constructed CDevEncUiEncrView
*/
CDevEncUiEncrView* CDevEncUiEncrView::NewLC( RArray<CDevEncUiMemoryEntity*>& aMemEntities )
	{
	CDevEncUiEncrView* self = new ( ELeave ) CDevEncUiEncrView( aMemEntities );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

CDevEncUiEncrView::CDevEncUiEncrView( RArray<CDevEncUiMemoryEntity*>& aMemEntities )
    : iMemEntities( aMemEntities )
	{
	}

CDevEncUiEncrViewContainer* CDevEncUiEncrView::Container()
	{
	return iContainer;
	}

CDevEncUiAppUi& CDevEncUiEncrView::GetAppUi()
    {
    CAknViewAppUi* aknViewAppUi = AppUi();
    CDevEncUiAppUi* appUi = reinterpret_cast<CDevEncUiAppUi*>( aknViewAppUi );
    return ( *appUi );
    }

/**
* Symbian OS 2nd phase constructor.
* Uses the superclass constructor to construct the view using the
* R_EMCCVIEWSWITCHED_VIEW1 resource.
*/
void CDevEncUiEncrView::ConstructL()
	{
	BaseConstructL( R_DEVENCUI_ENCRVIEW );
	}

/**
* Called by the framework
* @return The Uid for this view
*/
TUid CDevEncUiEncrView::Id() const
	{
	return TUid::Uid( EDevEncUiEncrViewId );
	}

/**
* Called by the framework when the view is activated.  Constructs the
* container if necessary, setting this view as its MOP parent, and
* adding it to the control stack.
*/
void CDevEncUiEncrView::DoActivateL( const TVwsViewId& /*aPrevViewId*/ ,
									 TUid /*aCustomMessageId*/,
									 const TDesC8& /*aCustomMessage*/ )
	{
	if ( ! iContainer )
		{
		iContainer = CDevEncUiEncrViewContainer::NewL( AppUi()->ClientRect(),
                                                       *this );
		iContainer->SetMopParent( this );
		AppUi()->AddToStackL( *this, iContainer );
		}
	}

/**
* Called by the framework when the view is deactivated.
* Removes the container from the control stack and deletes it.
*/
void CDevEncUiEncrView::DoDeactivate()
	{
	if ( iContainer )
		{
		AppUi()->RemoveFromStack( iContainer );
		delete iContainer;
		iContainer = NULL;
		}
	}

/**
* From CEikAppUi, takes care of command handling for this view.
*
* @param aCommand command to be handled
*/
void CDevEncUiEncrView::HandleCommandL( TInt aCommand )
	{
    DFLOG2( "CDevEncUiEncrView::HandleCommandL %d", aCommand );

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
void CDevEncUiEncrView::HandleStatusPaneSizeChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// End of File
