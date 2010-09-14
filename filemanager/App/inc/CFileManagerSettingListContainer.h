/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Setting list container in file manager
*
*/



#ifndef CFILEMANAGERSETTINGLISTCONTAINER_H
#define CFILEMANAGERSETTINGLISTCONTAINER_H

//  INCLUDES
#include "CFileManagerContainerBase.h"


// CLASS DECLARATION
/**
 * Container for backup view.
 */
class CFileManagerSettingListContainer : public CFileManagerContainerBase
    {
    public:  // Constructors and destructor
        enum TListType
            {
            EListBackup = 0, // Backup settings layout
            EListRestore // Restore selection layout
            };

        /**
         * Two-phased constructor.
         */
        static CFileManagerSettingListContainer* NewL(
            const TRect& aRect,
            const TInt aFocusedIndex,
            const TListType aType,
            MDesCArray* aArray,
            const TDesC& aHelpContext = KNullDesC );

        /**
         * Destructor.
         */
        ~CFileManagerSettingListContainer();

    private: // From CFileManagerContainerBase
        CEikTextListBox* CreateListBoxL();

        TKeyResponse OfferKeyEventL(
            const TKeyEvent& aKeyEvent, TEventCode aType );

    private:
        /**
        * C++ default constructor.
        */
        CFileManagerSettingListContainer( const TListType aType );

        void ConstructL(
            const TRect& aRect,
            const TInt aFocusedIndex,
            MDesCArray* aArray,
            const TDesC& aHelpContext );

        CEikTextListBox* CreateBackupListBoxL();

        CEikTextListBox* CreateRestoreListBoxL();

    private: // Data
        // Setting list type
        TListType iType;

    };

#endif      // CFILEMANAGERSETTINGLISTCONTAINER_H
            
// End of File
