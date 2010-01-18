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
* Description:  Backup settings
*
*/


#ifndef C_FILEMANAGERBACKUPSETTINGS_H
#define C_FILEMANAGERBACKUPSETTINGS_H


// INCLUDE FILES
#include <e32base.h>
#include <badesca.h>


// FORWARD DECLARATIONS
class CFileManagerEngine;
class CRepository;


// CLASS DECLARATION
/**
 *  This class is used for storing backup settings
 *
 *  @lib FileManagerEngine.lib
 *  @since S60 3.1
 */
NONSHARABLE_CLASS(CFileManagerBackupSettings) : public CBase,
                                                public MDesCArray
    {

public:
    /**  Backup setting type */
    enum TSettingType
        {
        ENone = 0,
        EContents,
        EScheduling,
        EWeekday,
        ETime,
        ETarget
        };

    /**
     * Two-phased constructor.
     */
    static CFileManagerBackupSettings* NewL(
        CFileManagerEngine& aEngine );

    /**
     * Destructor
     */
    ~CFileManagerBackupSettings();

    /**
     * Sets backup contents
     *
     * @since S60 3.1
     * @param aContent Content bitmask
     */
    IMPORT_C void SetContent( const TUint32 aContent );

    /**
     * Sets backup scheduling
     *
     * @since S60 3.1
     * @param aScheduling Scheduling type
     */
    IMPORT_C void SetScheduling( const TInt aScheduling );

    /**
     * Sets backup weekday
     *
     * @since S60 3.1
     * @param aDay Backup weekday (See TDay)
     */
    IMPORT_C void SetDay( const TInt aDay );

    /**
     * Sets backup time
     *
     * @since S60 3.1
     * @param aTime Time from 00:00
     */
    IMPORT_C void SetTime( const TTime& aTime );

    /**
     * Sets backup target drive
     *
     * @since S60 3.1
     * @param aDrive Target drive (See TDriveNumber)
     */
    IMPORT_C void SetTargetDrive( const TInt aDrive );

    /**
     * Gets backup contents
     *
     * @since S60 3.1
     * @return Content bitmask
     */
    IMPORT_C TUint32 Content() const;

    /**
     * Gets backup scheduling
     *
     * @since S60 3.1
     * @return Scheduling type
     */
    IMPORT_C TInt Scheduling() const;

    /**
     * Gets backup weekday
     *
     * @since S60 3.1
     * @return Backup weekday (See TDay)
     */
    IMPORT_C TInt Day() const;

    /**
     * Gets backup time
     *
     * @since S60 3.1
     * @return Backup time
     */
    IMPORT_C const TTime& Time() const;

    /**
     * Gets backup target drive
     *
     * @since S60 3.1
     * @return Backup time
     */
    IMPORT_C TInt TargetDrive() const;

    /**
     * Saves backup settings
     *
     * @since S60 3.1
     */
    IMPORT_C void SaveL();

    /**
     * Gets setting type at position
     *
     * @since S60 3.1
     * @param aIndex Position
     * @return Setting type
     */
    IMPORT_C TInt SettingAt( const TInt aIndex );

    /**
     * Refreshes backup settings
     *
     * @since S60 3.1
     */
    IMPORT_C void RefreshL();

    /**
     * Gets textid from content bit
     *
     * @since S60 3.1
     * @param aContent content bit
     * @return Textid
     */
    static TInt ContentToTextId( const TUint32 aContent );

    /**
     * Gets allowed drive attribute match mask
     *
     * @since S60 3.1
     * @return drive attribute mask
     */
    IMPORT_C TUint32 AllowedDriveAttMatchMask() const;

public: // From MDesCArray
    TInt MdcaCount() const;

    TPtrC MdcaPoint( TInt aIndex ) const;

private:
    /**  Backup setting entry */
    NONSHARABLE_CLASS(CEntry) : public CBase
        {
    public:
        ~CEntry();

        HBufC* iText;
        TSettingType iType;
        };

private:
    /**
     * Constructors
     */
    CFileManagerBackupSettings( CFileManagerEngine& aEngine );

    void ConstructL();

    /**
     * Gets the count of contents selected
     */
    TInt ContentsSelected() const;

    /**
     * Creates backup setting entry
     */
    CEntry* CreateEntryLC(
        const TSettingType aType,
        const TInt aTitleId,
        const TInt aTextId,
        const TInt aValue = 0 );

    /**
     * Creates backup setting entry
     */
    CEntry* CreateEntryLC(
        const TSettingType aType,
        const TInt aTitleId,
        const TDesC& aText );

    /**
     * Creates backup setting content entry
     */
    CEntry* CreateContentsEntryLC();

    /**
     * Creates backup setting time entry
     */
    CEntry* CreateTimeEntryLC();

    /**
     * Loads saved backup settings
     */
    void LoadL();

    /**
     * Refreshes backup setting list entries
     */
    void RefreshListL();

    /**
     * Creates backup setting target drive entry
     */
    CEntry* CreateTargetDriveEntryLC();

    /**
     * Checks for multiple backup targets
     */
    TBool HasMultipleBackupTargets();

private: // Data
    /**
     * Reference to file manager engine
     * Not own.
     */
    CFileManagerEngine& iEngine;

    /**
     * Content bitmask
     */
    TUint32 iContent;

    /**
     * Scheduling type
     */
    TInt iScheduling;

    /**
     * Scheduled backup weekday
     */
    TInt iDay;

    /**
     * Scheduled backup time from 00:00
     */
    TTime iTime;

    /**
     * Backup target drive
     */
    TInt iTargetDrive;

    /**
     * Backup setting list items
     */
    RPointerArray< CEntry > iList;

    /**
     * Allowed drive match mask
     */
    TUint32 iAllowedDriveMatchMask;

    /**
     * Pointer to CenRep
     * Own.
     */
	CRepository* iCenRep;

    /**
     * Feature configuration
     */
    TInt iFileManagerFeatures;

    };

#endif // C_FILEMANAGERBACKUPSETTINGS_H

// End of File
