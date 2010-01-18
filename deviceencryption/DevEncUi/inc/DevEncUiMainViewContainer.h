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
* Description:  Container for the main view.
*
*/

#ifndef __DEVENCUI_MAINVIEW_CONTAINER_H__
#define __DEVENCUI_MAINVIEW_CONTAINER_H__

//INCLUDES

//System Includes
#include <coecntrl.h>
#include <aknlists.h>
#include <aknview.h>
#include <DevEncExternalCRKeys.h>

//User Includes

#include "DevEnc.hrh"
#include "DevEncUiContainerEventCallback.h"
#include "DevEncUiMemInfoObserver.h"

//FORWARD DECLARATIONS
class CDevEncUiDocument;
class CDevEncUiSettingItemList;
class CRepository;
class CDevEncUiMemoryEntity;

class CDevEncUiMainViewContainer : public CCoeControl,
                                   public MDevEncUiMemInfoObserver
    {
    public:
    	virtual TInt CountComponentControls() const;
        /**
         * @function NewL
         * @abstract Creates a CDevEncUiMainViewContainer object, which will draw itself to aRect
         * @param aRect A rectangle that defines the size and location of the displayable area
         * for the view
         * @param aDocument the document
         **/
        static CDevEncUiMainViewContainer* NewL( const TRect& aRect,
												 RArray<CDevEncUiMemoryEntity*>& aMemEntities,
                                                 MContainerEventCallback& aCallback,
                                                 CRepository*& aCrSettings,
                                                 TInt& aMmcStatus );
    
        /**
         * @function NewLC
         * @abstract Creates a CDevEncUiMainViewContainer object, which will draw itself to aRect
         * @param aRect A rectangle that defines the size and location of the displayable area
         * for the view
         * @param aDocument the document
         **/
        static CDevEncUiMainViewContainer* NewLC( const TRect& aRect,
												  RArray<CDevEncUiMemoryEntity*>& aMemEntities,
                                                  MContainerEventCallback& aCallback,
                                                  CRepository*& aCrSettings,
                                                  TInt& aMmcStatus );
        ~CDevEncUiMainViewContainer();
    
        /**
         * From CCoeControl. Called by the framework whenever a key event occurs.
         * Passes the key event to the settings list if it is not null, otherwise returns
         * EKeyWasNotConsumed
         * @param aKeyEvent the Key event which occured, e.g. select key pressed
         * @param aType the type of Key event which occurred, e.g. key up, key down
         * @return TKeyResponse EKeyWasNotConsumed if the key was not processed,
         * EKeyWasConsumed if it was
         */
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );
    
        /**
         * @function ConstructL
         * @abstract Performs the second phase construction, setting the bounding
         * rectangle to aRect
         * @param aRect the display area for the view
         **/
        void ConstructL(const TRect& aRect);
    
        TInt SelectedItem();
    
        /**
         * Shows a popup dialog to let user set the state of the selected item.
         * @return ETrue if desired state of the selected item was changed
         */
        TBool ChangeSelectedItemL();
    
        /**
         * From MDevEncUiMemInfoObserver
         * */
        void UpdateInfo( TDevEncUiMemoryType aType,
                         TUint aState,
                         TUint aProgress = 0 );
    
    protected:
        virtual CCoeControl* ComponentControl(TInt aIndex) const;
    	
        /**
         * From CCoeControl
         * */
        void SizeChanged();
    	
        /**
         * From CCoeControl
         * */
        void HandleResourceChange(TInt aType);
    
    private:
        CDevEncUiMainViewContainer( RArray<CDevEncUiMemoryEntity*>& aMemEntities,
									MContainerEventCallback& aCallback,
                                    CRepository*& aCrSettings,
                                    TInt& aMmcStatus );
        void DoUpdateInfoL( TDevEncUiMemoryType aType,
                            TUint aState,
                            TUint aProgress /*= 0*/ );
        
    private:
        /** Owned */
        MContainerEventCallback& iCallback;

        /** Owned */
        CDevEncUiSettingItemList* iSettingItemList;

        /** Not owned */
        RArray<CDevEncUiMemoryEntity*>& iMemEntities;
        
        /** Not owned */
        CRepository*& iCrSettings;
        
        /** Not owned */
        TInt& iMmcStatus;
};


#endif // __DEVENCUI_MAINVIEW_CONTAINER_H__

//End of File
