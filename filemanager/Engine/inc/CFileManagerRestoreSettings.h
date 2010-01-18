/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Restore settings
*
*/


#ifndef C_FILEMANAGERBACKUPARRAY_H
#define C_FILEMANAGERBACKUPARRAY_H


// INCLUDE FILES
#include <e32base.h>
#include <badesca.h>


// FORWARD DECLARATIONS
class CFileManagerEngine;


// CLASS DECLARATION
/**
 *  This class is used for storing restore settings
 *
 *  @lib FileManagerEngine.lib
 *  @since S60 3.1
 */
NONSHARABLE_CLASS(CFileManagerRestoreSettings) : public CBase,
                                                 public MDesCArray
    {

public:
    /**  Restore icon type */
    enum TIconId
        {
        EIconCheckBoxOn = 0,
        EIconCheckBoxOff,
        EIconMemoryCard,
        EIconUsbMemory
        };

    /**  Restore info */
    NONSHARABLE_CLASS(TInfo)
        {
    public:
        TUint32 iContent;
        TTime iTime;
        TInt iDrive;
        };

    /**
     * Two-phased constructor.
     */
    static CFileManagerRestoreSettings* NewL(
        CFileManagerEngine& aEngine );

    /**
     * Destructor
     */
    ~CFileManagerRestoreSettings();

    /**
     * Sets restore selection
     *
     * @since S60 3.1
     */
    IMPORT_C void SetSelection( const TUint64& aSelection );

    /**
     * Refreshes restore settings
     *
     * @since S60 3.1
     */
    IMPORT_C void RefreshL();

    /**
     * Gets restore selection
     *
     * @since S60 3.1
     */
    void GetSelectionL( RArray< TInfo >& aInfoArray ) const;

public: // From MDesCArray
    TInt MdcaCount() const;

    TPtrC MdcaPoint( TInt aIndex ) const;

private:
    /**  Restore setting entry */
    NONSHARABLE_CLASS(CEntry) : public CBase
        {
    public:
        ~CEntry();

        HBufC* iText;
        TInfo iInfo;
        };

private:
    /**
     * Constructors
     */
    CFileManagerRestoreSettings( CFileManagerEngine& aEngine );

    void ConstructL();

    /**
     * Creates a new restore setting entry
     */
    CEntry* CreateEntryLC( const TInfo& aInfo );

    static HBufC* DateTimeStringLC( const TTime& aTime );

private: // Data
    /**
     * Restore setting entries
     */
    RPointerArray< CEntry > iList;

    /**
     * Reference to file manager engine
     * Not own.
     */
    CFileManagerEngine& iEngine;

    /**
     * Restore selection
     */
    TUint64 iSelection;

    };

#endif // C_FILEMANAGERBACKUPARRAY_H

// End of File
