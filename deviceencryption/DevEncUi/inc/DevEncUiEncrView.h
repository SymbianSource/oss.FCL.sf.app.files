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
* Description:  Encryption view.
*
*/

#ifndef __DEVENCUI_ENCRVIEW_H__
#define __DEVENCUI_ENCRVIEW_H__

//INCLUDES

// System includes
#include <aknview.h>

//User Includes
#include "DevEncUiDocument.h"

// FORWARD DECLARATIONS
class CDevEncUiEncrViewContainer;
class CDevEncUiEncryptionOperator;
class CDevEncUiMemoryEntity;

class CDevEncUiEncrView: public CAknView
    {
    public: // constructors and destructor
        static CDevEncUiEncrView* NewL( RArray<CDevEncUiMemoryEntity*>& aMemEntities );
        static CDevEncUiEncrView* NewLC( RArray<CDevEncUiMemoryEntity*>& aMemEntities );
        CDevEncUiEncrViewContainer* Container();
        ~CDevEncUiEncrView();
        CDevEncUiAppUi& GetAppUi();

        private: // from CAknView
        TUid Id() const;
        void HandleCommandL( TInt aCommand );
        CDevEncUiEncrView( RArray<CDevEncUiMemoryEntity*>& aMemEntities );

        void DoActivateL( const TVwsViewId& aPrevViewId,
                          TUid aCustomMessageId,
                          const TDesC8& aCustomMessage );

        void DoDeactivate();

        private: // constructors
        void ConstructL();

    protected:
        /**
        *  HandleStatusPaneSizeChange.
        *  Called by the framework when the application status pane
        *  size is changed.
        */
        void HandleStatusPaneSizeChange();

    private: // data
        /** Owned. What this view will display */
        CDevEncUiEncrViewContainer* iContainer;
        TUid iId;

        /** Not owned */
        RArray<CDevEncUiMemoryEntity*>& iMemEntities;
    };

#endif	// __DEVENCUI_ENCRVIEW_H__

// End of File
