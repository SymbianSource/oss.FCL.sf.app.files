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
* Description: 
*     An utility class to handle the UIDs and filename extensions used to
*     identify data types. It is also used to store the results per data type.
*
*/


#ifndef CMSENGINFOARRAY_H
#define CMSENGINFOARRAY_H

//  INCLUDES
#include <badesca.h>
#include <f32file.h>

// FORWARD DECLARATIONS
class CResourceFile;
class RResourceReader;


// CLASS DECLARATION

/**
*  An utility class to handle the UIDs and filename extensions
*  used to identify data types. It is also used to store
*  the results per data type
*/
class CMsengInfoArray : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CMsengInfoArray* NewL(TDriveNumber aDrive,
                                     TInt aNumberOfDataGroups,
                                     RFs& aFsSession,
                                     CResourceFile& aResFile);
        
        /**
        * Destructor.
        */
        ~CMsengInfoArray();

    public: // New functions
        
        /**
        * Get the scan results per UID.
        * @return Pointer to the array of scan results per UID.
        */
        inline CArrayFix<TInt64>* UidResults() const;

        /**
        * Get the scan results per filename extension.
        * @return Pointer to the array of scan results per extension.
        */
        inline CArrayFix<TInt64>* ExtResults() const;

        /**
        * Get the scan results per data group.
        * @return Pointer to the array of scan results per group.
        */
        inline CArrayFix<TInt64>* GroupResults() const;

        /**
        * Get UIDs to be scanned for.
        * @return Pointer to the array of UIDs.
        */
        inline const CArrayFix<TUid>& Uids() const;

        /**
        * Get the extensions to be scanned for.
        * @return Pointer to the array of filename extensions.
        */
        inline const CDesCArray& Exts() const;

        /**
        * Get the directories to be scanned.
        * @return Pointer to the array of directory paths.
        */
        inline const CDesCArray& Dirs() const;

        /**
        * Get the special data directories to be scanned.
        * @return Pointer to the array data dir - data group pairs.
        */
        inline const CDesCArray& DataDirs() const;

        /**
        * 
        * 
        */
        inline const CArrayFix<TInt>& DataDirGroups() const;
        
        /**
        * Get the files excluded from directory to be scanned.
        */
        inline const CArrayPtrFlat<CDesCArray>& DataDirExcludedFiles() const;

        /**
        * Get the drive that is currently scanned
        * @return Drive current drive
        */
        inline const TDriveNumber CurrentDrive() const;

        /**
        * Add new file size to the result array.
        * @param aUid The file UID type in question
        * @param aSize Size of the file
        */
        inline void AddSizeByUidL(TInt aUid, TInt64 aSize);

        /**
        * Add new file size to the result array.
        * @param aExt Filename extension type in question
        * @param aSize Size of the file
        */
        inline void AddSizeByExtL(TInt aExt, TInt64 aSize);

        /**
        * Add scan result af a specific data group.
        * @param aGroupIndex Data group in question
        * @param aSize Size of the file
        */
        inline void AddSizeByGroupL(TInt aGroupIndex, TInt64 aSize);

        /**
        * Query if the directory is in the list of directories to be 
        * excluded from scanning, or is a subdirectory of one
        * @param aDirectory Path of the directory
        * @return TBool
        */
        TBool IsExcludedDir(const TDesC& aDirectory) const;

        /**
        * This function is otherwise similar to BaflUtils::FolderExists, but
        * it also ETrue for drive root (e.g. "c:\")
        * @param aFs File server session
        * @param aDirectory Path of the directory
        */
        TBool FolderExists(RFs& aFs, const TDesC& aPath);
        
        /**
        * Query if the directory is in the list of directories to be 
        * excluded from scanning, or is a subdirectory of one
        * @param aDirectory Path of the directory
        * @return TBool
        */
        TBool IsSpecialDir(const TDesC& aDirectory) const;


    private:
       
        /**
        * C++ default constructor is prohibited.
        */
        CMsengInfoArray(TDriveNumber aDrive);

        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL(TInt aNumberOfDataGroups, RFs& aFsSession, CResourceFile& aResFile);

        // Prohibit copy constructor if not deriving from CBase.
        CMsengInfoArray( const CMsengInfoArray& );
        // Prohibit assigment operator if not deriving from CBase.
        CMsengInfoArray& operator= ( const CMsengInfoArray& );

    private:    // Data
        // Root directories for scanning
        CDesCArray* iDirArray;

        // Directories that are excluded from the normal scan
        CDesCArray* iExcludedDirArray;

        // Directories scanned as a whole excluding listed files
        CDesCArray* iDataDirArray;
        CArrayFix<TInt>* iDataDirGroupArray;
        CArrayPtrFlat<CDesCArray>* iDataDirExclArray;

        // The drive that is currently scanned
        TDriveNumber iCurrentScannedDrive;
        
        // The following  arrays are indexed using values from enumerations
        // TUidTypes and TExtTypes

        // Results are inserted into these arrays
        CArrayFix<TInt64>* iUidResultArray;
        CArrayFix<TInt64>* iExtResultArray;
        CArrayFix<TInt64>* iGroupResultArray;

        // UIDs and extensions, which are searched,
        // are in these  two arrays
        CArrayFix<TUid>* iUidArray;
        CDesCArray* iExtArray;
    };

#include "msenginfoarray.inl"

#endif      // CMSENGINFOARRAY_H   
            
// End of File
