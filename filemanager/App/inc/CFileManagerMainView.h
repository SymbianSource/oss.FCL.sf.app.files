/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Main view
*
*/



#ifndef C_FILEMANAGERMAINVIEW_H
#define C_FILEMANAGERMAINVIEW_H


//  INCLUDES
#include <aknview.h>
#include "CFileManagerViewBase.h"


// CLASS DECLARATION
/**
 * This class implements main view of the application.
 *
 *  @since S60 3.1
 */
class CFileManagerMainView : public CFileManagerViewBase
    {

public:  // Constructors and destructor
    /**
     * Two-phased constructor.Leaves created view
     * to cleanup stack.
     *
     * @return Newly constructed view 
     */
    static CFileManagerMainView* NewLC();
    
    /**
     * Destructor.
     */
    ~CFileManagerMainView();

public: // From CFileManagerViewBase
    /**
     * @see CFileManagerViewBase
     */
    void DirectoryChangedL();

    /**
     * @see CFileManagerViewBase
     */
    CFileManagerContainerBase* CreateContainerL();

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
    void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

    /**
     * @see CAknView
     */
    void HandleCommandL( TInt aCommand );

private: // From MFileManagerProcessObserver
    /**
     * @see MFileManagerProcessObserver
     */
    TInt NotifyL( TFileManagerNotify aType, TInt aData, const TDesC& aName );

private: // New functions
    /**
     * Filters main menu in aMenuPane.
     */
    void MainMenuFilteringL( CEikMenuPane& aMenuPane );

    /**
     * Filters remote drives menu in aMenuPane.
     */        
    void RemoteDrivesMenuFilteringL( CEikMenuPane& aMenuPane );

    /**
     * Handles memory store open command
     */
    void CmdOpenMemoryStoreL();

    /**
     * Handles eject command
     */
    void CmdEjectL();

#ifndef RD_FILE_MANAGER_BACKUP
    /**
     * Handles legacy backup command
     */
    void CmdBackupL();

    /**
     * Handles legacy restore command
     */
    void CmdRestoreL();
#endif // RD_FILE_MANAGER_BACKUP

    /**
     * Handles map remote drive command
     */
    void CmdMapRemoteDriveL();

    /**
     * Handles remote drive settings command
     */
    void CmdRemoteDriveSettingsL();

    /**
     * Handles remote drive delete command
     */
    void CmdRemoteDriveDeleteL();

    /**
     * Filters memory storage menu items.
     *
     * @param aMenuPane Menu instance to be filtered.
     */
    void MemoryStorageMenuFilteringL( CEikMenuPane& aMenuPane );

    /**
     * Handles memory storage details command.
     */
    void CmdMemoryStorageDetailsL();

    /**
     * Handles drive rename command.
     */
    void CmdRenameDriveL();

    /**
     * Handles drive password set command.
     */
    void CmdSetDrivePasswordL();

    /**
     * Handles drive password change command.
     */
    void CmdChangeDrivePasswordL();

    /**
     * Handles drive password remove command.
     */
    void CmdRemoveDrivePasswordL();

    /**
     * Gets drive at current position.
     *
     * @return Drive identifier or KErrNotFound.
     */
    TInt DriveAtCurrentPosL();

private:
    /**
    * C++ default constructor.
    */
    CFileManagerMainView();

    void ConstructL();

    };

#endif // C_FILEMANAGERMAINVIEW_H
            
// End of File
