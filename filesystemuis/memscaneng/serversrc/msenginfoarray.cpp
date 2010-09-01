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
* Description: 
*     An utility class to handle the UIDs and filename extensions used to
*     identify data types. It is also used to store the results per data type.
*
*/



// SYSTEM INCLUDES
#include    <mseng.rsg>
#include    <bautils.h>
#include    <barsc.h>    // RResourceFile

// USER INCLUDES
#include    "msenginfoarray.h"
#include    "mseng.h"    // KMsengRscFilePath



// ================= MEMBER FUNCTIONS =======================


// ---------------------------------------------------------------------------
// CMsengInfoArray::CMsengInfoArray()
//
// C++ default constructor is prohibited
// ---------------------------------------------------------------------------
CMsengInfoArray::CMsengInfoArray(TDriveNumber aDrive)
: iCurrentScannedDrive(aDrive)
    {
    }

// ---------------------------------------------------------------------------
// CMsengInfoArray::NewL()
//
// Two-phased constructor.
// ---------------------------------------------------------------------------
CMsengInfoArray* CMsengInfoArray::NewL(TDriveNumber aDrive,
                                       TInt aNumberOfDataGroups,
                                       RFs& aFsSession,
                                       CResourceFile& aResFile)
    {
    CMsengInfoArray* self = new (ELeave) CMsengInfoArray(aDrive);
    
    CleanupStack::PushL( self );
    self->ConstructL(aNumberOfDataGroups, aFsSession, aResFile);
    CleanupStack::Pop( self );

    return self;
    }


// ---------------------------------------------------------------------------
// CMsengInfoArray::ConstructL()
//
// Symbian OS default constructor can leave.
// ---------------------------------------------------------------------------
void CMsengInfoArray::ConstructL(TInt aNumberOfDataGroups,
                                 RFs& aFsSession,
                                 CResourceFile& aResFile)
    {
    // Create data structures and initialize them
    // using values from enumerations TUidTypes and TExtTypes
    // and data from the resource file mseng.rss

    TInt index = -1; // index used in for-loops
    TInt length = -1; // length of resource array being read
    
    RResourceReader theReader;

    theReader.OpenLC( &aResFile, UIDARRAY );

    //the first WORD contains the number of elements in the resource
    length = theReader.ReadInt16L();

    // Create the array with appropriate granularity
    iUidResultArray = new (ELeave) CArrayFixFlat<TInt64>(length);
    
    // Initialize the array to contain zeros
    for(index=0; index<length; index++)
        {
        iUidResultArray->InsertL(index, 0);
        }

    // Next, create the array for the actual UIDs
    // and read them from the resource file
    iUidArray = new (ELeave) CArrayFixFlat<TUid>(length);
    for(index=0; index<length; index++)
        {
        const TInt typeindex = theReader.ReadInt8L();
        const TUid uid = TUid::Uid(theReader.ReadInt32L());
        //
        iUidArray->InsertL(typeindex, uid);
        }
    CleanupStack::PopAndDestroy(&theReader);

    // Read extarray in a similar way
    theReader.OpenLC( &aResFile, EXTARRAY );
    
    
    //the first WORD contains the number of elements in the resource
    length = theReader.ReadInt16L();
    
    // Create the array with appropriate granularity
    iExtResultArray = new (ELeave) CArrayFixFlat<TInt64>(length);
    // Initialize the array to contain zeros
    for(index=0; index<length; index++)
        {
        iExtResultArray->InsertL(index, 0);
        }
    // Next, create the array for the actual extensions
    // and read them from the resource file
    iExtArray = new (ELeave) CDesCArrayFlat(length);
    for(index=0; index<length; index++)
        {
        TInt typeindex = theReader.ReadInt8L();
        TPtrC ext = theReader.ReadTPtrCL();
        
        iExtArray->InsertL(typeindex, ext);
        }
    CleanupStack::PopAndDestroy( &theReader ); 

    // Create the array for results per group
    iGroupResultArray = new (ELeave) CArrayFixFlat<TInt64>(aNumberOfDataGroups);
    // Initialize the array to contain zeros
    for(index=0; index<aNumberOfDataGroups; index++)
        {
        iGroupResultArray->InsertL(index, 0);
        }

    // The directories to be scanned. Depends of which drive is scanned,
    // and the directories that are scanned as a whole (and excluded in the normal scan)
    _LIT(KPanic,"MSENG");
    __ASSERT_ALWAYS((CMseng::IsInternalDrive(aFsSession, iCurrentScannedDrive)
        || CMseng::IsRemovableDrive(aFsSession, iCurrentScannedDrive)),
        User::Panic(KPanic, KErrNotSupported));

    if( CMseng::IsInternalDrive(aFsSession, iCurrentScannedDrive) 
	    && !CMseng::IsMassStorageDrive( aFsSession, iCurrentScannedDrive ) )
        {        
        theReader.OpenLC( &aResFile, C_DIRECTORIES );
        iDirArray = theReader.ReadDesCArrayL();
        CleanupStack::PopAndDestroy( &theReader );
        //
        theReader.OpenLC( &aResFile, C_EXCLUDED_DIRECTORIES );
        iExcludedDirArray = theReader.ReadDesCArrayL();
        CleanupStack::PopAndDestroy( &theReader );
        //
        theReader.OpenLC( &aResFile, C_SPECIAL_DATADIRS );
        // reading later...
        
        }
    else // other drives except Phone Memory should be scanned from root folder.
        {
        theReader.OpenLC( &aResFile, E_DIRECTORIES );
        iDirArray = theReader.ReadDesCArrayL();
        CleanupStack::PopAndDestroy( &theReader );
        //
        theReader.OpenLC( &aResFile, E_EXCLUDED_DIRECTORIES );
        iExcludedDirArray = theReader.ReadDesCArrayL();
        CleanupStack::PopAndDestroy( &theReader );
        //
        theReader.OpenLC( &aResFile, E_SPECIAL_DATADIRS );
        // reading later...
        
        }

    // Apply correct drive letter in directory array names
    TInt dirCount = iDirArray->Count();
    for (TInt i=0; i<dirCount; i++)
        {
        HBufC* dirName = iDirArray->MdcaPoint(i).AllocLC();
        TPtr ptrName = dirName->Des();
        TBuf<1> drive;
        TChar ch;
        
        if ( RFs::DriveToChar( iCurrentScannedDrive, ch ) == KErrNone )
            {
            drive.Append(ch);
            ptrName.Replace(0, drive.Length(), drive);
            }
        iDirArray->Delete(i);
        iDirArray->InsertL(i, ptrName);
        CleanupStack::PopAndDestroy(dirName);
        }

    // Apply correct drive letter in excluded directory array names
    TInt exDirCount = iExcludedDirArray->Count();
    for (TInt i=0; i<exDirCount; i++)
        {
        HBufC* dirName = iExcludedDirArray->MdcaPoint(i).AllocLC();
        TPtr ptrName = dirName->Des();
        TBuf<1> drive;
        TChar ch;
        
        if ( RFs::DriveToChar( iCurrentScannedDrive, ch ) == KErrNone )
            {
            drive.Append(ch);
            ptrName.Replace(0, drive.Length(), drive);
            }
        iExcludedDirArray->Delete(i);
        iExcludedDirArray->InsertL(i, ptrName);
        CleanupStack::PopAndDestroy(dirName);
        }

    //the first WORD contains the number of elements in the resource
    length = theReader.ReadInt16L();

    // Create the arrays for special data dirs
    iDataDirArray = new (ELeave) CDesCArrayFlat(length);
    iDataDirGroupArray = new (ELeave) CArrayFixFlat<TInt>(length);
    iDataDirExclArray = new (ELeave) CArrayPtrFlat<CDesCArray>(length);

    // Read the array resource
    for(TInt i=0; i<length; i++)
        {
        TInt groupindex = theReader.ReadInt8L();
        TChar ch;
        HBufC* name = theReader.ReadHBufCL();
        CleanupStack::PushL(name);
        TPtr ptrName = name->Des();
        TBuf<1> drive;
        TBool driveValid = EFalse;
        
        if ( RFs::DriveToChar( iCurrentScannedDrive, ch ) == KErrNone )
            {
            driveValid = ETrue;
            drive.Append(ch);
            ptrName.Replace(0, drive.Length(), drive);
            }
            
        // Next WORD contains the number of excluded files
        TInt lengthExcl = theReader.ReadInt16L();
        TBool folderExists = EFalse;
        
        // Add directory to the list to be scanned
        if(driveValid && BaflUtils::FolderExists(aFsSession, ptrName))
            {
            folderExists = ETrue;
            iDataDirArray->AppendL(ptrName);
            iDataDirGroupArray->AppendL(groupindex);
            iDataDirExclArray->AppendL(NULL);
            
            CDesCArray* subarray = new (ELeave) CDesCArrayFlat( Max(lengthExcl, 1) );
            const TInt dirCount = iDataDirExclArray->Count();
            iDataDirExclArray->At(dirCount-1) = subarray;
            }
        
        for(TInt j=0; j<lengthExcl; j++)
            {
            TPtrC nameExcl = theReader.ReadTPtrCL();
            
            // Append special file only if folder exists
            if(folderExists)
                {
                const TInt dirCount = iDataDirExclArray->Count();
                iDataDirExclArray->At(dirCount-1)->AppendL( nameExcl );
                }
            }
        
        // If there was an error, we can assume it was because
        // the folder does not exist, and ignore the error.
        CleanupStack::PopAndDestroy( name );
        }
    CleanupStack::PopAndDestroy( &theReader );

#ifdef __SHOW_RDEBUG_PRINT_
    RDebug::Print(_L("CMsengInfoArray constructed. Printing current configuration.\n    Extensions:"));
    for(TInt j=0; j < Exts().Count(); j++)
        {
        HBufC* ext = Exts().MdcaPoint(j).AllocL();
        RDebug::Print(_L("    %d: %S"), j, ext);
        delete ext;
        }
    RDebug::Print(_L("    UIDs:"));
    for(TInt k=0; k < Uids().Count(); k++)
        {
        TUidName uid; 
        uid = Uids().At(k).Name();
        RDebug::Print(_L("    %d: %S"), k, &uid);
        }
#endif // __SHOW_RDEBUG_PRINT_
    }

// ---------------------------------------------------------------------------
// CMsengInfoArray::~CMsengInfoArray()
//
// Destructor
// ---------------------------------------------------------------------------
CMsengInfoArray::~CMsengInfoArray()
    {  
    // delete data structures
    delete iUidResultArray;
    delete iExtResultArray;
    delete iGroupResultArray;
    delete iUidArray;
    delete iExtArray;
    delete iDirArray;
    delete iExcludedDirArray;
    delete iDataDirArray;
    delete iDataDirGroupArray;
    if(iDataDirExclArray)
        {
        iDataDirExclArray->ResetAndDestroy();
        }
    delete iDataDirExclArray;
    }

// ---------------------------------------------------------------------------
// CMsengInfoArray::IsExcludedDir()
// 
//
// ---------------------------------------------------------------------------
TBool CMsengInfoArray::IsExcludedDir(const TDesC& aDirectory) const
    {
    TInt count = iExcludedDirArray->Count();
    for(TInt i=0; i<count; i++)
        {
        if(aDirectory.FindF(iExcludedDirArray->MdcaPoint(i)) == 0)
            {
            return ETrue;
            }
        }

    return EFalse;
    }

// ---------------------------------------------------------------------------
// CMsengInfoArray::FolderExists()
//
//
// ---------------------------------------------------------------------------
TBool CMsengInfoArray::FolderExists(RFs& aFs, const TDesC& aPath)
    {
    TBool result = EFalse;

    if(BaflUtils::FolderExists(aFs, aPath))
        {
        result = ETrue;
        }
    // BaflUtils::FolderExists return KErrBadName, if called with
    // only drive letter (like "c:\")
    else
        {
        TChar driveLetter;
        if( RFs::DriveToChar(CurrentDrive(), driveLetter) == KErrNone)
            {
            TBuf<1> driveName;
            driveName.Append(driveLetter);
            TInt cmp = aPath.CompareF(BaflUtils::RootFolderPath(driveName));
            result = (cmp == 0);
            }
        }

    return result;
    }

// ---------------------------------------------------------------------------
// CMsengInfoArray::IsSpecialDir()
//
//
// ---------------------------------------------------------------------------
TBool CMsengInfoArray::IsSpecialDir(const TDesC& aDirectory) const
    {
    TInt count = iDataDirArray->Count();
    for(TInt i=0; i<count; i++)
        {
        if(aDirectory.FindF(iDataDirArray->MdcaPoint(i)) == 0)
            {
            return ETrue;
            }
        }

    return EFalse;
    }

//  End of File  
