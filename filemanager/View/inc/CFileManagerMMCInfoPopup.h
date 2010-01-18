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
* Description:  Memory card info popup
*
*/



#ifndef C_FILEMANAGERMMCINFOPOPUP_H
#define C_FILEMANAGERMMCINFOPOPUP_H


// INCLUDES
#include <coecntrl.h>
#include <badesca.h>


// FORWARD DECLARATIONS
class CAknSingleHeadingPopupMenuStyleListBox;
class CAknPopupList;
class TFileManagerDriveInfo;
class CMSPUtil;


// CLASS DECLARATION
/**
 *  This class displays memory card info
 *
 *  @lib FileManagerView.lib
 *  @since S60 3.1
 */
NONSHARABLE_CLASS(CFileManagerMMCInfoPopup) : public CCoeControl
    {

public:  // Constructors and destructor
    /**
     * Two-phased constructor.
     * @param aProperties Properties of the item that will be shown
     * @return Newly created popup.
     */
    static CFileManagerMMCInfoPopup* NewL(
        const TFileManagerDriveInfo& aInfo );

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
	~CFileManagerMMCInfoPopup();

private: // New functions
    /**
     * C++ default constructor.
     */
    CFileManagerMMCInfoPopup();

    /**
     * By default Symbian 2nd phase constructor is private.
     */
    void ConstructL( const TFileManagerDriveInfo& aInfo );

    /**
     * Constructs data array
     * @param aProperties Properties of the item that will be shown
     * @return Data array
     */
	MDesCArray* ConstructDataArrayL(
	    const TFileManagerDriveInfo& aInfo );

    /**
     * Creates name entry
     * @param aProperties Properties of the item that will be shown
     * @return Name entry
     */
    HBufC* NameEntryLC(
        const TFileManagerDriveInfo& aInfo );

    /**
     * Creates size entry with text and size
     * @param aTextId Text to display
     * @param aSize Size to display
     * @return Size entry
     */
    HBufC* SizeEntryLC( TInt aTextId, TInt64 aSize );

private:  // Data
    /**
     * Popup list that is the actual dialog that is shown
     * Own.
     */
	CAknPopupList* iPopupList;

    /**
     * Listbox that is given to CAknPopupList constructor
     * Own.
     */
	CAknSingleHeadingPopupMenuStyleListBox* iListBox;

    /**
     * Keeps track if destructor is already called
     * Not own.
     */
	TBool* iIsDestroyed;

    /**
     * Memory State Popup utilities
     * Own.
     */
	CMSPUtil* iUtil;

    };

#endif // C_FILEMANAGERMMCINFOPOPUP_H

// End of File
