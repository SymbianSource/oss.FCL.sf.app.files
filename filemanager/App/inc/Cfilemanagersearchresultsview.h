/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  View for search results
*
*/


#ifndef CFILEMANAGERSEARCHRESULTSVIEW_H
#define CFILEMANAGERSEARCHRESULTSVIEW_H

//  INCLUDES
#include "CFileManagerViewBase.h"

// CONSTANTS
_LIT8( KFileManagerSearchViewRefreshMsg, "FmgrRefresh" ); 

// FORWARD DECLARATIONS
class CFileManagerSearchFileArray;
class CAknNavigationControlContainer;
class CAknNavigationDecorator;
// CLASS DECLARATION
/**
 * Search results view of the application.
 */
class CFileManagerSearchResultsView : public CFileManagerViewBase
    {
    public:  // Constructors and destructor        
        /**
         * Two-phased constructor.Leaves created view
		 * to cleanup stack.
		 *
		 * @return Newly constructed view 
         */
		static CFileManagerSearchResultsView* NewLC();
        
        /**
		 * Destructor.
         */
        ~CFileManagerSearchResultsView();

	public:

		/**
		 * Forwards DirectoryChange information to active view
         * @since 2.0
		 */
		void DirectoryChangedL();

	private: // From CAknView
		/**
		 * @see CAknView
		 */
		void HandleCommandL( TInt aCommand );

		/**
		 * @see CAknView
		 */
		void DoActivateL(
		    const TVwsViewId& aPrevViewId,
		    TUid aCustomMessageId,
		    const TDesC8& aCustomMessage );

		/**
		 * @see CAknView
		 */
		void DoDeactivate();

		/**
		 * @see CAknView
		 */
		TUid Id() const;

	private: // From CFileManagerViewBase
		/**
		 * @see CFileManagerViewBase
		 */
		CFileManagerContainerBase* CreateContainerL();

		/**
		 * @see CFileManagerViewBase
		 */
		void UpdateCbaL();

    private:
        /**
         * Standard C++ constructor.
         */
        CFileManagerSearchResultsView();

		/**
		 * 2nd phase constructor.
		 */
		void ConstructL();

        void CmdBackL();

        void RefreshTitleL();

    private:    // Data
		/// Ref: Navi pane of the view
		CAknNavigationControlContainer* iNaviPane;

   		/// Ref: Pointer to empty navidecorator, needed for popping up correct decorator
		CAknNavigationDecorator* iNaviDecorator;

    };

#endif      // CFILEMANAGERSEARCHRESULTSVIEW_H   
            
// End of File
