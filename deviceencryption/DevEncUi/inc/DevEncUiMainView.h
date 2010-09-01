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
* Description:  Main view of the application.
*
*/

#ifndef __DEVENCUI_MAINVIEW_H__
#define __DEVENCUI_MAINVIEW_H__

//INCLUDES

//System includes
#include <aknview.h>	// CAknView
#include <centralrepository.h>

//User Includes
#include "DevEncUiContainerEventCallback.h"
#include "DevEncUiDocument.h"

// FORWARD DECLARATIONS
class CDevEncUiEncryptionOperator;
class CDevEncUiMainViewContainer;
class CDevEncUiMemoryEntity;

class CDevEncUiMainView: public CAknView,
                         public MContainerEventCallback
	{
    public:
    // constructors and destructor
    	static CDevEncUiMainView* NewL( RArray<CDevEncUiMemoryEntity*>& aMemEntities,
                                        CRepository*& aSettings,
                                        TInt& aMmcStatus );
    	static CDevEncUiMainView* NewLC( RArray<CDevEncUiMemoryEntity*>& aMemEntities,
                                         CRepository*& aSettings,
                                         TInt& aMmcStatus );
    	~CDevEncUiMainView();
    	CDevEncUiAppUi& GetAppUi();

    // from MContainerEventCallback
        void EncryptionStatusChangeReq( TDevEncUiMemoryType aType );

    protected:
        void DynInitMenuPaneL( TInt aResourceId,
                               CEikMenuPane* aMenuPane );

        /**
        *  HandleStatusPaneSizeChange.
        *  Called by the framework when the application status pane
        *  size is changed.
        */
        void HandleStatusPaneSizeChange();

    private:
    // from CAknView
    	TUid Id() const;
    	void HandleCommandL( TInt aCommand );
    	CDevEncUiMainView( RArray<CDevEncUiMemoryEntity*>& aMemEntities,
                           CRepository*& aSettings,
                           TInt& aMmcStatus );

    	void DoActivateL( const TVwsViewId& aPrevViewId,
                          TUid aCustomMessageId,
                          const TDesC8& aCustomMessage );

    	void DoDeactivate();

        // constructors
    	void ConstructL();

        void HandleMemoryCardEncryptRequestL( CDevEncUiMemoryEntity* aMem );

        void DoEncryptionStatusChangeReqL( TDevEncUiMemoryType aType );

        TBool DmControlsMemory( TDevEncUiMemoryType aType,
                                TInt aDmControlSetting );

        void ErrorNoteL( TInt aResourceId );
        
    // data
        /** ID of this view */
        TUid iId;

        /** Owned */
    	CDevEncUiMainViewContainer* iContainer;

        /** Not owned */
        RArray<CDevEncUiMemoryEntity*>& iMemEntities;

        /** Owned */
        CDevEncUiEncryptionOperator* iEncOperator;
        
        /** Not owned */
        CRepository*& iCrSettings;
        
        /** Not owned */
        TInt& iMmcStatus;
	};

#endif	// __DEVENCUI_MAINVIEW_H__

// End of File
