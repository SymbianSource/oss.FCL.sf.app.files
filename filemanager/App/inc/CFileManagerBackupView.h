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
* Description:  View for backup settings
*
*/



#ifndef C_FILEMANAGERBACKUPVIEW_H
#define C_FILEMANAGERBACKUPVIEW_H


//  INCLUDES
#include <aknview.h>
#include "CFileManagerViewBase.h"


// CLASS DECLARATION
/**
 *  This class is used for backup settings view
 *
 *  @since S60 3.1
 */
class CFileManagerBackupView : public CFileManagerViewBase
    {

public:  // Constructors and destructor
    /**
     * Two-phased constructor.Leaves created view
	 * to cleanup stack.
	 *
	 * @return Newly constructed view 
     */
	static CFileManagerBackupView* NewLC();

    /**
	 * Destructor.
     */
    ~CFileManagerBackupView();

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
    void NotifyForegroundStatusChange( TBool aForeground );

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

	/**
	 * @see CAknView
	 */
    void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

private: // New functions
    /**
     * Handles change command
     */
    void CmdChangeL();

    /**
     * Handles backup command
     */
    void CmdBackupL();
 
    /**
     * Handles restore command
     */
    void CmdRestoreL();

    /**
     * Changes backup contents
     */
    void ChangeContentsL();

    /**
     * Changes backup scheduling
     */
    void ChangeSchedulingL();

    /**
     * Changes backup weekday
     */
    void ChangeWeekdayL();

    /**
     * Changes backup time
     */
    void ChangeTimeL();

    /**
     * Changes backup target drive
     */
    void ChangeTargetDriveL();

    /**
     * Handles command back
     */
    void CmdBackL();

    /**
     * Updates schedule state
     */
    void UpdateScheduleL();

    /**
     * Refreshes all settings
     */
    void RefreshSettingsL();

    /**
     * Handles delete backup command
     */
    void CmdDeleteBackupL();

    /**
     * Filters backup menu items.
     *
     * @param aMenuPane Menu instance to be filtered.
     */
    void BackupMenuFilteringL( CEikMenuPane& aMenuPane );

private:
    /**
    * C++ default constructor.
    */
    CFileManagerBackupView();

private: // Data

    };

#endif // C_FILEMANAGERBACKUPVIEW_H   
            
// End of File
