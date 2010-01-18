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
* Description:  Checkbox setting page
*
*/



#ifndef C_FILEMANAGERCHECKBOXSETTINGPAGE_H
#define C_FILEMANAGERCHECKBOXSETTINGPAGE_H


//  INCLUDES
#include <akncheckboxsettingpage.h>


// CLASS DECLARATION
/**
 *  This class implements checkbox setting page
 *
 *  @lib FileManagerView.lib
 *  @since S60 3.1
 */
NONSHARABLE_CLASS(CFileManagerCheckBoxSettingPage) :
        public CAknCheckBoxSettingPage
    {

public:
	/**
     * Constructor.
	 * @see CAknCheckBoxSettingPage
     */
    CFileManagerCheckBoxSettingPage(
        const TInt aResourceID,
        CSelectionItemList& aItemArray,
        const TInt aDominantItemIndex );

    ~CFileManagerCheckBoxSettingPage();

private: // From CAknCheckBoxSettingPage
    void UpdateSettingL();

private: // New functions
    void UpdateSelection();

private: // Data
    /**
     * Pointer to items array
     * Not own.
     */
    CSelectionItemList& iItemsArray;

    /**
     * Index of the dominant item
     */
    TInt iDominantItemIndex;

    };

#endif // C_FILEMANAGERCHECKBOXSETTINGPAGE_H

// End of File
