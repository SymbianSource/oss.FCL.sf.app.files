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
* Description:  Container for the decryption view.
*
*/

#ifndef __DEVENCUI_GRAPHIC_VIEW_CONTAINER_H__
#define __DEVENCUI_GRAPHIC_VIEW_CONTAINER_H__

//INCLUDES

//System Includes
#include <coecntrl.h>
#include <aknview.h>
#include <eiklbo.h>

//User Includes

//FORWARD DECLARATIONS
class CAknDoubleStyle2ListBox;

class CDevEncUiDecrViewContainer : public CCoeControl,
                                   public MEikListBoxObserver
    {
public:
    /**
     * @function NewLC
     * @abstract Creates a CDevEncUiEncrViewContainer object, which will
     * draw itself to aRect
     * @param aRect A rectangle that defines the size and location of the
     * displayable area for the view
     * @param aDocument the document
    **/
    static CDevEncUiDecrViewContainer* NewL(const TRect& aRect, CAknView& aOwningView);
    /**
     * @function NewL
     * @abstract Creates a CDevEncUiEncrViewContainer object, which will draw
     * itself to aRect
     * @param aRect A rectangle that defines the size and location of the
     * displayable area for the view
     * @param aDocument the document
     **/
    static CDevEncUiDecrViewContainer* NewLC(const TRect& aRect, CAknView& aOwningView);

    /**
     * Destructor.
     **/
    ~CDevEncUiDecrViewContainer();

    // from MEikListBoxObserver
    void HandleListBoxEventL( CEikListBox* aListBox,
                              TListBoxEvent aEventType );

    /**
     * From CCoeControl. Called by the framework in compound controls
     * @return The number of controls in this CSimpleListContainer
     */
    TInt CountComponentControls() const;

    /**
     * From CCoeControl. Called by the framework in compound controls
     * @param The index of the control to return
     * @return The control for aIndex
     */
    CCoeControl* ComponentControl( TInt aIndex ) const;

    /**
     * Called by the framework to draw this control.  Clears the area in
     * aRect.
     * @param aRect in which to draw
     */
    void Draw( const TRect& aRect ) const;

    /**
     * Called by the framework whenever a key event occurs.
     * @param aKeyEvent the Key event which occured, e.g. select key pressed
     * @param aType the type of Key event which occurred, e.g. key up, key down
     * @return TKeyResponse EKeyWasNotConsumed if the key was not processed,
     * EKeyWasConsumed if it was
     */
    TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                 TEventCode aType );
    /**
     * Called by framework when the view size is changed.  Resizes the
     * iLabel accordingly.
     */
     void SizeChanged();

private:
    /**
     * @function CDevEncUiEncrViewContainer
     * @abstract Performs the first stage construction
     * @param aOwningView the view that owns this container
     **/
    CDevEncUiDecrViewContainer(CAknView& aOwningView/*CDevEncUiDocument* aDocument*/);

    /**
     * @function ConstructL
     * @abstract Performs the second phase construction, setting the bounding
     * rectangle to aRect
     * @param aRect the display area for the view
     **/
    void ConstructL(const TRect& aRect);

    /**
     * Constructs the iListBox, setting its window.
     */
    void CreateListL();

private:
	CAknView& iOwningView;
    CAknDoubleStyle2ListBox* iListBox;
};


#endif // __DEVENCUI_GRAPHIC_VIEW_CONTAINER_H__
