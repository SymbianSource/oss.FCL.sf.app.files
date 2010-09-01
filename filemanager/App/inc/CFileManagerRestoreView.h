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
* Description:  View for restore settings
*
*/



#ifndef C_FILEMANAGERRESTOREVIEW_H
#define C_FILEMANAGERRESTOREVIEW_H


//  INCLUDES
#include <aknview.h>
#include "CFileManagerViewBase.h"


// CLASS DECLARATION
/**
 *  This class is used for restore settings view
 *
 *  @since S60 3.1
 */
class CFileManagerRestoreView : public CFileManagerViewBase
    {

public: // Custom Message IDs
    // Custom message for deleting backups
    static const TUid KDeleteBackups;

    // For custom messages
    enum TMessage
        {
        EDeleteBackups = 1
        };

public:  // Constructors and destructor
    /**
     * Two-phased constructor.Leaves created view
     * to cleanup stack.
     *
     * @return Newly constructed view 
     */
    static CFileManagerRestoreView* NewLC();
    
    /**
     * Destructor.
     */
    ~CFileManagerRestoreView();

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
    
    /**
     * Updates cba
     */
    virtual void UpdateCbaL();

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
    
public: // From MFileManagerProcessObserver
    /**
     * @see MFileManagerProcessObserver
     */
    void ProcessFinishedL( TInt aError, const TDesC& aName = KNullDesC );

private: // New functions
    /**
     * Handles ok command
     */
    void CmdOkL();

    /**
     * Handles cancel command
     */
    void CmdCancelL();

    /**
     * Handles select command
     */
    void CmdSelectL();

    /**
     * Handles check mark command
     */
    void CmdCheckMarkL();

    /**
     * Refreshes all settings
     */
    void RefreshSettingsL();

    /**
     * Restores list selection after refresh
     */
    void RestoreSelectionL();

    /**
     * Updates cba according to list selection
     */
    void UpdateCbaFromSelectionL();

private:
    /**
    * C++ default constructor.
    */
    CFileManagerRestoreView();

private: // Data
    // Indicates if view is started into delete backup mode
    TBool iDeleteBackups;

    /**
     * For storing list selection during refresh
     */
    TUint64 iSelection;

    };

#endif // C_FILEMANAGERRESTOREVIEW_H   
            
// End of File
