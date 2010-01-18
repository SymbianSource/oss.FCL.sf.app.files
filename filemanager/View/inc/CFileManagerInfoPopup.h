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
* Description:  Popup to show the item information, view info
*
*/


#ifndef CFILEMANAGERINFOPOPUP_H
#define CFILEMANAGERINFOPOPUP_H

// INCLUDES
#include <e32def.h>
#include <drmuihandling.h>
#include "CFileManagerPopupBase.h"

// FORWARD DECLARATIONS
class CFileManagerItemProperties;
class CMSPUtil;
class CFileManagerFeatureManager;
class DRM::CDrmUiHandling;

// CLASS DECLARATION
/**
 * Popup dialog for showing detailed information about item.
 *
 *  @lib FileManagerView.lib
 *  @since S60 2.0
 */
NONSHARABLE_CLASS(CFileManagerInfoPopup) : public CFileManagerPopupBase
    {
    public:  // Constructors and destructor
        
        /**
         * Two-phased constructor.
         * @param aProperties Properties of the item that will be shown
         * @param aFeatureManager Reference to the feature manager
         * @return Newly created popup.
         */
        static CFileManagerInfoPopup* NewL(
            CFileManagerItemProperties& aProperties,
            const CFileManagerFeatureManager& aFeatureManager );
        
    private:
        
        /**
         * By default Symbian 2nd phase constructor is private.
         */
        virtual void ConstructL();
        
    private: // From CFileManagerPopupBase
        /**
         * @see CFileManagerPopupBase
         */
        virtual MDesCArray* ConstructDataArrayL();

#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
        /**
         * @see CFileManagerPopupBase
         */
        virtual void ActivateLinkL();
#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    private: // New Functions
        /**
         * Creates the name entry for data array.
         * @return Name entry.
         */
        HBufC* NameEntryLC();

        /**
         * Creates the type entry for data array.
         * @return Type entry.
         */
        HBufC* TypeEntryLC();

        /**
         * Creates the date entry for data array.
         * @return Date entry.
         */
        HBufC* DateEntryLC();

        /**
         * Creates the time entry for data array.
         * @return Time entry.
         */
        HBufC* TimeEntryLC();

        /**
         * Creates the size entry for data array.
         * @return Size entry.
         */
        HBufC* SizeEntryLC();

        /**
         * Creates the default folder entry for data array.
         * @return Default folder entry.
         */
        HBufC* DefaultFolderEntryLC();


        /**
         * Creates the string of given resource id and value
         * @param resource id of number string 
         * @param aValue value for string
         * @return HBufC* formatted string, caller must delete it
         */
        HBufC* ResourceStringValueLC( TInt aResId, TInt aValue );

        /**
         * C++ default constructor.
         */
        CFileManagerInfoPopup(
            CFileManagerItemProperties& aProperties,
            const CFileManagerFeatureManager& aFeatureManager );

        /**
         * Destructor.
         */
        ~CFileManagerInfoPopup();

    private: // Data
        /// Ref: Reference to properties data that will be shown in popup.
        CFileManagerItemProperties& iProperties;

        // Own: Memory State Popup utilities
        CMSPUtil* iUtil;

        // Ref: Reference to the feature manager
        const CFileManagerFeatureManager& iFeatureManager;

        // DRM UI Handler        
        DRM::CDrmUiHandling* iUiHandling;

    };

#endif      // CFILEMANAGERINFOPOPUP_H
            
// End of File
