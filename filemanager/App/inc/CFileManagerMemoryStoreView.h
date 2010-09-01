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
* Description:  Memory store view
*
*/



#ifndef C_FILEMANAGERMEMORYSTOREVIEW_H
#define C_FILEMANAGERMEMORYSTOREVIEW_H


//  INCLUDES
#include <aknview.h>
#include "CFileManagerViewBase.h"


// CLASS DECLARATION
/**
 * This class implements memory store view.
 *
 *  @since S60 3.1
 */
class CFileManagerMemoryStoreView : public CFileManagerViewBase
    {

public:  // Constructors and destructor
    /**
     * Two-phased constructor.Leaves created view
	 * to cleanup stack.
	 *
	 * @return Newly constructed view 
     */
	static CFileManagerMemoryStoreView* NewLC();
    
    /**
	 * Destructor.
     */
    ~CFileManagerMemoryStoreView();

public: // From CFileManagerViewBase
	/**
	 * @see CFileManagerViewBase
	 */
    void DirectoryChangedL();

	/**
	 * @see CFileManagerViewBase
	 */
	CFileManagerContainerBase* CreateContainerL();

	/**
	 * @see CFileManagerViewBase
	 */
	void UpdateCbaL();

private: // From CAknView
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

	/**
	 * @see CAknView
	 */
    void HandleCommandL( TInt aCommand );

private: // New functions
    /**
     * Checks memory store available
     */
    TBool DriveAvailableL( TBool aShowRemoteNotConnected );

    /**
     * Refreshes memory store title
     */
    void RefreshTitleL();

    /**
     * Handles open memory store command
     */
    void CmdOpenL();

    /**
     * Handles back command
     */
    void CmdBackL();

private:
    /**
    * C++ default constructor.
    */
    CFileManagerMemoryStoreView();

private:

    };

#endif // C_FILEMANAGERMEMORYSTOREVIEW_H   

// End of File
