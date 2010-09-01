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
* Description:  Base class for file manager popups
*
*/



#ifndef CFILEMANAGERPOPUPBASE_H
#define CFILEMANAGERPOPUPBASE_H

// INCLUDES
#include <coecntrl.h>

// FORWARD DECLARATIONS
class MDesCArray;
#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
class CAknMessageQueryDialog;
#else // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
class CAknDoublePopupMenuStyleListBox;
class CAknPopupList;
#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

// CLASS DECLARATION
/**
 * Base class for all popup dialogs in File Manager
 *
 *  @lib FileManagerView.lib
 *  @since S60 2.0
 */
NONSHARABLE_CLASS(CFileManagerPopupBase) : public CCoeControl
    {
    public:  // Constructors and destructor        
        /**
         * Executes the dialog.
         * @return ETrue if dialog was dismissed using OK,
         *         EFalse otherwise.
         */
        TBool ExecuteLD();
    protected: 
        /**
         * Destructor.
         */
        ~CFileManagerPopupBase();

    protected: // New functions
        /**
         * Constructs the information that this popup shows.
         * @return Array of items that will be shown in popup.
         */
        virtual MDesCArray* ConstructDataArrayL() = 0;

        /**
         * C++ default constructor.
         */
        CFileManagerPopupBase();

        /**
         * By default Symbian 2nd phase constructor is private.
         */
        virtual void ConstructL();

        /**
         * Sets the title of popup dialog.
         * @param aTitle New title of popup dialog.
         */
        void SetTitleL(const TDesC &aTitle);

#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
        /**
         * Activates CAknMessageQueryDialog link action.
         */
        virtual void ActivateLinkL();
#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    private:
#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
        /**
         * Callback to implement CAknMessageQueryDialog link action.
         */
        static TInt LinkCallback(TAny* aPtr);
#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    private:    // Data
#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
        // Own: Dialog title.
        HBufC* iTitle;
        // Own: Dialog data.
        HBufC* iData;
#else // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
        // Own: Popup list that is the actual dialog that is shown.
        CAknPopupList* iPopupList;
        /// Own: Listbox that is given to CAknPopupList constructor.
        CAknDoublePopupMenuStyleListBox* iListBox;
        /// Ref: Keeps track if destructor is already called.
        TBool* iIsDestroyed;
#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
    };

#endif      // CFILEMANAGERPOPUPBASE_H

// End of File
