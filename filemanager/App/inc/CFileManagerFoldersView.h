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
* Description:  View for folder
*
*/



#ifndef CFILEMANAGERFOLDERSVIEW_H
#define CFILEMANAGERFOLDERSVIEW_H

//  INCLUDES
#include "CFileManagerViewBase.h"
#include <MFileManagerFolderNaviObserver.h>

// FORWARD DECLARATIONS
class CAknNavigationControlContainer;
class CAknNavigationDecorator;
class CFileManagerFolderNavigationPane;
class CAknInfoPopupNoteController;

// CLASS DECLARATION
/**
 * Folders view of the application.
 */
class CFileManagerFoldersView : public CFileManagerViewBase,
                                public MFileManagerFolderNaviObserver
    {
    public: // Custom Message IDs
        /// custom message for opening from results view
        static const TUid KOpenFromSearchResultsView;

        // For custom message use
        enum TMessage
            {
            EOpenFromSearchResultsView = 1
            };

    public:  // Constructors and destructor
        /**
         * Two-phased constructor. Leaves created view
         * to cleanup stack.
         *
         * @return Newly constructed view 
         */
        static CFileManagerFoldersView* NewLC();
        
        /**
         * Destructor.
         */
        ~CFileManagerFoldersView();

    public: // New declarations

        /**
         * Forwards Resource change events to view
         * @since 2.0
         * @param aType type of change
         */
        void HandleResourceChangeL( TInt aType );

        /**
         * Forwards DirectoryChange information to active view
         * @since 2.0
         */
        void DirectoryChangedL();

    private: // From CAknView
        /**
         * @see CAknView
         */
        TUid Id() const;

        /**
         * @see CAknView
         */
        void HandleCommandL( TInt aCommand );

        /**
         * @see CAknView
         */
        TKeyResponse OfferKeyEventL(
            const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
         * @see CAknView
         */
        void DoActivateL(
            const TVwsViewId& aPrevViewId,
            TUid aCustomMessageId,
            const TDesC8& aCustomMessage);

        /**
         * @see CAknView
         */
        void DoDeactivate();

    private: // From CFileManagerViewBase
        /**
         * @see CFileManagerViewBase
         */
        CFileManagerContainerBase* CreateContainerL();

        /**
         * @see CFileManagerViewBase
         */
        void UpdateCbaL();

    private: // From MFileManagerFolderNaviObserver
        /**
         * @see MFileManagerFolderNaviObserver
         */
        void HandleFolderNaviEventL( TNaviEvent aEvent, TInt aValue );

    private: 
        /**
         * Refreshes title to contain name of the 
         * current folder.
         */
        void RefreshTitleL();

        /**
         * Standard C++ constructor.
         */
        CFileManagerFoldersView();

        /**
         * 2nd phase constructor.
         */
        void ConstructL();

        /**
         * Handles backstepping from the current folder.
         * @since 3.1
         * @param aBacksteps Number of backsteps to do
         */
        void BackstepL( TInt aBacksteps = 1 );

        /**
         * Gets folder name from specified folder level
         * @since 3.1
         * @param aFolderLevel Folder level
         * @return Folder name
         */
        TPtrC FolderName( const TInt aFolderLevel );

    private:    // Data
        /// Ref: Navi pane of the view
        CAknNavigationControlContainer* iNaviPane;
        /// Own: Navigation decorator for navi pane
        CAknNavigationDecorator* iNaviDecorator;
        /*
         * Ref: Navi control that draws folders in the navi pane. 
         * Ownership is given to navi pane after creation.
         */
        CFileManagerFolderNavigationPane* iNaviControl;

        // Current depth, with this folder view knows when 
        // user has gone up or down in folder tree
        TInt iFolderDepth;

        // Own: Initial folder depth.
        // View is closed after stepped back below this level.
        TInt iInitialFolderDepth;

        // Own: For showing info popups
        CAknInfoPopupNoteController* iPopupController;

    };

#endif      // CFILEMANAGERFOLDERSVIEW_H
            
// End of File
