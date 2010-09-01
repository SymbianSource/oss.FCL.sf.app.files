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
* Description:  Decryption view.
*
*/

#ifndef __DEVENCUI_DECRVIEW_H__
#define __DEVENCUI_DECRVIEW_H__

//INCLUDES

// System includes
#include <aknview.h>

//User Includes
#include "DevEncUiDocument.h"

// FORWARD DECLARATIONS
class CDevEncUiDecrViewContainer;
class CDevEncUiMemoryEntity;

class CDevEncUiDecrView: public CAknView
	{
    public: // constructors and destructor

        /**
         * Symbian OS 2 phase constructor.
         * Constructs the CDevEncUiDecrView using the NewLC method, popping
         * the constructed object from the CleanupStack before returning it.
         *
         * @param aRect The rectangle for this window
         * @return The newly constructed CDevEncUiDecrView
         */
        static CDevEncUiDecrView* NewL( RArray<CDevEncUiMemoryEntity*>& aMemEntities );

        /**
         * Symbian OS 2 phase constructor.
         * Constructs the CDevEncUiDecrView using the constructor and ConstructL
         * method, leaving the constructed object on the CleanupStack before returning it.
         *
         * @param aRect The rectangle for this window
         * @return The newly constructed CDevEncUiDecrView
         */
        static CDevEncUiDecrView* NewLC( RArray<CDevEncUiMemoryEntity*>& aMemEntities );
    	CDevEncUiDecrViewContainer* Container();

        /**
         * Destructor.  Frees up memory.
         */
        ~CDevEncUiDecrView();
    	CDevEncUiAppUi& GetAppUi();
    	
    protected:
        /**
        *  HandleStatusPaneSizeChange.
        *  Called by the framework when the application status pane
        *  size is changed.
        */
        void HandleStatusPaneSizeChange();

        void HandleResourceChangeL( TInt aType );

    private: // from CAknView

        /**
         * Called by the framework
         * @return The Uid for this view
         */
        TUid Id() const;

        /**
         * From CEikAppUi, takes care of command handling for this view.
         * @param aCommand command to be handled
         */
        void HandleCommandL( TInt aCommand );

        CDevEncUiDecrView( RArray<CDevEncUiMemoryEntity*>& aMemEntities );

        /**
         * Called by the framework when the view is activated.  Constructs the
         * container if necessary, setting this view as its MOP parent, and
         * adding it to the control stack.
         */
    	void DoActivateL( const TVwsViewId& aPrevViewId,
    	                  TUid aCustomMessageId,
    	                  const TDesC8& aCustomMessage );

        /**
         * Called by the framework when the view is deactivated.
         * Removes the container from the control stack and deletes it.
         */
        void DoDeactivate();

    private: // constructors

        /**
         * Symbian OS 2nd phase constructor.
         * Uses the superclass constructor to construct the view using the
         * R_EMCCVIEWSWITCHED_VIEW1 resource.
         */
        void ConstructL();

    private: // data
        /** Owned. What this view will display */
        CDevEncUiDecrViewContainer* iContainer;
        TUid iId;

        /** Not owned */
        RArray<CDevEncUiMemoryEntity*>& iMemEntities;

    };

#endif // __DEVENCUI_DECRVIEW_H__

// End of File
