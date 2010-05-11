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
*     The actual "engine". 
*
*
*/


// INCLUDE FILES


// SYSTEM INCLUDES
#include    <mseng.rsg>
#include    <bautils.h>
#include    <driveinfo.h>

// USER INCLUDES
#include    "mseng.h"
#include    "mseng.hrh"
#include    "msengscanner.h"

// ========================= MEMBER FUNCTIONS ================================

// ---------------------------------------------------------------------------
// CMseng::CMseng()
//
// C++ default constructor. Can NOT contain any code, that might leave.
// ---------------------------------------------------------------------------


    
CMseng::CMseng( MMsengUIHandler& aUIHandler ) :
    iUIHandler(aUIHandler),
    iFreeMemory(0)
    {
    }    

// ---------------------------------------------------------------------------
// CMseng::NewL()
//
// Two-phased constructor.
// ---------------------------------------------------------------------------

EXPORT_C CMseng* CMseng::NewL(MMsengUIHandler& aUIHandler)
    {
    CMseng* self = new (ELeave) CMseng(aUIHandler);
       
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// ---------------------------------------------------------------------------
// CMseng::ConstructL()
//
// Symbian OS default constructor can leave.
// ---------------------------------------------------------------------------

void CMseng::ConstructL()
    {
    #ifdef __SHOW_RDEBUG_PRINT_
    RDebug::Print(_L("** CMseng::ConstructL()... starting **"));
    #endif // __SHOW_RDEBUG_PRINT_

    // Connect to File Server
    User::LeaveIfError(iFsSession.Connect());

    // Open the resource file
    TParse* fp = new(ELeave) TParse(); 
    fp->Set(KMsengRscFilePath, &KDC_RESOURCE_FILES_DIR, NULL); 
    TFileName fileName( fp->FullName() );
    delete fp;


    BaflUtils::NearestLanguageFile( iFsSession, fileName );
    //
    TEntry entry;
    User::LeaveIfError( iFsSession.Entry( fileName, entry ) );
    // if file does not exist, leaves with KErrNotFound

    iResFile = CResourceFile::NewL( iFsSession, fileName, 0, entry.iSize );
    
    iResFile->ConfirmSignatureL();
    

    /////////////////////////////////////////////////////////
    //create data structures and initialize them from resource file

    TInt index = -1; // index used in for-loops
    TInt subindex = -1; // index used in for-loops inside another for-loop
    TInt length = -1; // length of resource array being read
    TInt sublength = -1; // length of sub-array inside array resource

    RResourceReader theReader;
    theReader.OpenLC( iResFile, DATAGROUPNAMEARRAY );
    

    //the first WORD contains the number of elements in the resource
    iNumberOfDataGroups = theReader.ReadInt16L();
    
    CleanupStack::PopAndDestroy( &theReader );
    
    

    /////////////////////////////////////////////////////////
    // Read the resource containing the data needed to create
    // mapping between data groups and UIDs
    //
    theReader.OpenLC( iResFile, DATAGROUPUIDARRAY );  
   
    //the first WORD contains the number of elements in the resource
    length = theReader.ReadInt16L();

   
    // Create array with such granularity that reallocation is unnecessary
    // initialize array to contain null pointers 
    iDataGroupUidArray = new (ELeave) CArrayPtrFlat<CIntArray>(iNumberOfDataGroups);
    for(index=0; index<iNumberOfDataGroups; index++)
        {
        iDataGroupUidArray->AppendL(NULL);
        }
    TInt groupindex; // value from enum TDataGroups
    // Read the array resource
    for(index=0; index<length; index++)
        {
        // Read one enum TDataGroups value
        groupindex = theReader.ReadInt8L();
        
        // Read the sub-array. First WORD contains array length.
        sublength = theReader.ReadInt16L();
        
        // Create new CUidArray with appropriate granularity
        // and insert it into the main array
        CIntArray* subarray = new (ELeave) CIntArray(iNumberOfDataGroups);
        CleanupStack::PushL(subarray);
        if( groupindex < iDataGroupUidArray->Count() )
            {
            iDataGroupUidArray->At(groupindex) = subarray;
            }
        
        // Read the subarray resource
        for(subindex=0; subindex<sublength; subindex++)
            {
            // uidtype matches one value fron enum TUidTypes
            TInt uidtype = theReader.ReadInt8L(); 
            if( groupindex < iDataGroupUidArray->Count() )
                {
                iDataGroupUidArray->At(groupindex)->InsertL(subindex,uidtype);
                }
            }
        CleanupStack::Pop( subarray );
        }
    CleanupStack::PopAndDestroy( &theReader );

    /////////////////////////////////////////////////////////
    // Read the resource containing the data needed to create
    // mapping between data groups and extensions 
    //
    theReader.OpenLC( iResFile, DATAGROUPEXTARRAY );

    //the first WORD contains the number of elements in the resource
    length = theReader.ReadInt16L();
    // Create array with such granularity that reallocation is unnecessary
    // Initialize it to contain null pointers, since some cells can leave empty
    iDataGroupExtArray = new (ELeave) CArrayPtrFlat<CIntArray>(iNumberOfDataGroups);
    for(index=0; index<iNumberOfDataGroups; index++)
        {
        iDataGroupExtArray->AppendL(NULL);
        }
    // Read the array resource
    for(index=0; index<length; index++)
        {
        // Read one enum TDataGroups value
        groupindex = theReader.ReadInt8L();
        // Read the sub-array. First WORD contains array length.
        sublength = theReader.ReadInt16L();
        // Create new CIntArray with appropriate granularity
        // and insert it into the main array
        CIntArray* subarray = new (ELeave) CIntArray(sublength);
        CleanupStack::PushL(subarray);
        if( groupindex < iDataGroupExtArray->Count() )
            {
            iDataGroupExtArray->At(groupindex) = subarray;
            }
        
        // Read the subarray resource
        for(subindex=0; subindex<sublength; subindex++)
            {
            // exttype matches one value fron enum TUidTypes
            TInt exttype = theReader.ReadInt8L();
            if( groupindex < iDataGroupExtArray->Count() )
                {
                iDataGroupExtArray->At(groupindex)->InsertL(subindex,exttype);
                }
            }
        CleanupStack::Pop( subarray );
        }

    CleanupStack::PopAndDestroy( &theReader );
    
    //instantiate scanner
    iScanner = new (ELeave) CMsengScanner(iUIHandler, *iResFile);
    }
    

// ---------------------------------------------------------------------------
// CMseng::~CMseng()
// 
// Destructor.
// ---------------------------------------------------------------------------

EXPORT_C CMseng::~CMseng()
    {
#ifdef __SHOW_RDEBUG_PRINT_
    RDebug::Print(_L("** CMseng::~CMseng(). Finished. **"));
#endif // __SHOW_RDEBUG_PRINT_

    delete iScanner;
    
    // Pointer arrays: elements must be deleted before deleting array
    if(iDataGroupUidArray)
        {
        iDataGroupUidArray->ResetAndDestroy();
        }
    delete iDataGroupUidArray;
    
    
    if(iDataGroupExtArray)
        {
        iDataGroupExtArray->ResetAndDestroy();
        }
    delete iDataGroupExtArray;
  
        
    delete iResFile;
    
    iFsSession.Close();       
    }



// ---------------------------------------------------------------------------
// CMseng::DataGroupsL()
//
// Get a descriptor array containing the names of the data groups.
// ---------------------------------------------------------------------------

EXPORT_C CDesCArray* CMseng::DataGroupsL() const
    {
#ifdef __SHOW_RDEBUG_PRINT_
    RDebug::Print(_L("CMseng::GetDataGroupsL() called."));
#endif // __SHOW_RDEBUG_PRINT_


    // Create the array for the data group names with appropriate granularity
    CDesCArray* dataGroupNameArray = new (ELeave) CDesCArrayFlat(iNumberOfDataGroups);
    CleanupStack::PushL(dataGroupNameArray);

    // Read the resource containing data group names 
    // and put them to resultArray
    
    RResourceReader theReader;
    theReader.OpenLC( iResFile, DATAGROUPNAMEARRAY );
    
    
    // The first WORD contains the number of elements in the resource
    // (actually this is already in iNumberOfDataGroups)

    TInt length = theReader.ReadInt16L();
    __ASSERT_DEBUG(iNumberOfDataGroups == length, User::Panic(_L("CMseng::DataGroupsL"), KErrGeneral));
    
    // Read the data group names from resource file and insert to array
    TInt groupindex; // value from enum TDataGroups
    for(TInt index=0; index<length; index++)
        {
        groupindex = theReader.ReadInt8L();
        TPtrC name = theReader.ReadTPtrCL();
        // Copy the name to the right place in the array
        dataGroupNameArray->InsertL(groupindex, name);
        }
    CleanupStack::PopAndDestroy( &theReader );

    // Return the array of data groups
    CleanupStack::Pop( dataGroupNameArray );

#ifdef __SHOW_RDEBUG_PRINT_
// print the data group array
    RDebug::Print(_L("Printing the Data Groups:"));
    for(TInt k = 0; k < dataGroupNameArray->Count(); k++)
        {
        HBufC* groupName = dataGroupNameArray->MdcaPoint(k).AllocL();
        RDebug::Print( _L("    %d: %S"), k, groupName);
        delete groupName;
        }
#endif // __SHOW_RDEBUG_PRINT_

    return dataGroupNameArray;
    
    }

// ---------------------------------------------------------------------------
// CMseng::ScanResultL()
//
// Returns an array of scan results
// ---------------------------------------------------------------------------

EXPORT_C CArrayFix<TInt64>* CMseng::ScanResultL() const
    {
#ifdef __SHOW_RDEBUG_PRINT_
    RDebug::Print(_L("CMseng::ScanResultL() called. Starting to calculate result..."));
#endif // __SHOW_RDEBUG_PRINT_

    // Create the result array (with such granularity that reallocations do not happen)
    CArrayFix<TInt64>* resultArray = new (ELeave) CArrayFixFlat<TInt64>(iNumberOfDataGroups);
    CleanupStack::PushL(resultArray);

    // Get result arrays from scanner 
    const CArrayFix<TInt64>* extResultArray = iScanner->ExtResults();
    const CArrayFix<TInt64>* uidResultArray = iScanner->UidResults();
    const CArrayFix<TInt64>* groupResultArray = iScanner->GroupResults();

    // Initialize the result array from the array of initial result
    for (TInt i = 0; i < iNumberOfDataGroups; i++)
        {
        if( i < groupResultArray->Count() )
            {
            resultArray->AppendL(groupResultArray->At(i));
            }
        }

    //Calculate the results and put them to the array
    
    // Find results for each data group
    for(TInt groupindex = 0; groupindex < iNumberOfDataGroups; groupindex++)
        {
        // For one data group, the UIDs belonging to this group are listed in 
        // iDataGroupExtArray. For each of these UIDs, add the result to the total result.

        // If the examined data group does not have associated UIDs, 
        // iDataGroupUidArray->At(groupindex) is a NULL pointer.
        if(iDataGroupUidArray->At(groupindex))
            {
            TInt count = iDataGroupUidArray->At(groupindex)->Count();
            for(TInt uidindex = 0; uidindex < count; uidindex++)
                {
                resultArray->At(groupindex) += 
                    uidResultArray->At( iDataGroupUidArray->At(groupindex)->At(uidindex) );
                }
            }

        // The extension results are collected in a similar manner

        // If the examined data group does not have associated UIDs, 
        // iDataGroupUidArray->At(groupindex) is a NULL pointer
        if(iDataGroupExtArray->At(groupindex))
            {
            TInt count = iDataGroupExtArray->At(groupindex)->Count();
            for(TInt extindex = 0; extindex < count; extindex++)
                {
                resultArray->At(groupindex) += 
                    extResultArray->At( iDataGroupExtArray->At(groupindex)->At(extindex) );
                }
            }
        }
    // Calculate "Free memory" and "All device data"
    TInt64 totalMemory;
    TInt64 freeMemory;
    DiskInfoL(totalMemory, freeMemory, iScanner->CurrentDrive());
#ifdef __SHOW_RDEBUG_PRINT_
    RDebug::Print(_L("CMseng::ScanresultL(): iFreeMemory %d, freeMemory %d"), (TUint32)iFreeMemory, (TUint32)freeMemory);
#endif
    // For some reason there is sometimes 16 kB difference in free memory when scanning started
    // vs. scanning ended (16 kB more at the end of scanning) and latter one is incorrect.
    // That is why free memory detected in the beginning of scanning taken into account. 
    if(iFreeMemory)
        {
        freeMemory = iFreeMemory;
        }
    else
        {
        iFreeMemory = freeMemory;
        }
    
    // "Free memory" is the memory currently available
    resultArray->At(EGroupFreeMemory) = freeMemory;
    // "All Device Data" is all memory used
    resultArray->At(EGroupAllDeviceData) = (totalMemory - freeMemory);
    
    // Calculate how much files not falling to any predefined category consume
    TInt64 others( 0 );
    for( TInt i = EGroupCalendar; i < iNumberOfDataGroups; i++ )
        {
        others += resultArray->At( i );
        }
    
    // This should never happen, but just in case check that negative count is not established.
    if( resultArray->At(EGroupAllDeviceData) - others < 0 )
        {
        resultArray->At( EGroupOthers ) = 0;
        }
    else
        {
        resultArray->At( EGroupOthers ) = resultArray->At(EGroupAllDeviceData) - others;
        }

// write the result array to log file
#ifdef __SHOW_RDEBUG_PRINT_
    RDebug::Print(_L("CMseng::ScanresultL(): current result array -"));
    // note that the log macros cannot handle TInt64
    for(TInt k = 0; k < resultArray->Count(); k++)
        {
        const TInt KMaxChars = 32;
        TBuf<KMaxChars> num;
        num.Num(resultArray->At(k));
        RDebug::Print(num);
        }
#endif // __SHOW_RDEBUG_PRINT_

    CleanupStack::Pop( resultArray );
    return resultArray;
    }

// ---------------------------------------------------------------------------
// CMseng::ScanInProgress()
//
// Return ETrue if there is scanning going on, otherwise EFalse.
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CMseng::ScanInProgress() const
    {
    if(iScanner) 
        {
        return iScanner->HaveActiveScanners();
        }
    else
        {
        return EFalse;
        }
    }

// ---------------------------------------------------------------------------
// CMseng::DiskInfoL
//
// Retrieves information about disk usage.
// ---------------------------------------------------------------------------
//
EXPORT_C void CMseng::DiskInfoL(TInt64& aTotal, TInt64& aFree, const TDriveNumber aVolume) const
    {

    TVolumeInfo vinfo;
    User::LeaveIfError(iFsSession.Volume(vinfo, aVolume));
    aTotal = TInt64(vinfo.iSize);
    aFree = TInt64(vinfo.iFree);

	}

// ---------------------------------------------------------------------------
// CMseng::MemInfoL
//
// Retrieves information about RAM usage.
// ---------------------------------------------------------------------------
//
EXPORT_C void CMseng::MemInfoL(TInt64& aTotal, TInt64& aFree)
    {
    TMemoryInfoV1Buf membuf;
    User::LeaveIfError(UserHal::MemoryInfo(membuf));
    TMemoryInfoV1 minfo = membuf();
    aTotal = minfo.iTotalRamInBytes;
    aFree = minfo.iFreeRamInBytes;
    }

// ---------------------------------------------------------------------------
// CMseng::ScanL()
//
// First scan the specific data files.
// Then scan directories that are scanned for the 
// size of all files. Then call scanner's ScanL.
// ---------------------------------------------------------------------------
//
EXPORT_C void CMseng::ScanL(TDriveNumber aDrive)
    {
    __ASSERT_ALWAYS( (CMseng::IsInternalDrive(iFsSession, aDrive)
        || CMseng::IsRemovableDrive(iFsSession, aDrive)), User::Leave(KErrNotSupported) );

    // Scanning started.
    iUIHandler.StartL();
    
    // Start scanning memory, check that not already doing it
    TInt err = iScanner->ScanL(aDrive, iNumberOfDataGroups, iFsSession);
    if(err != KErrNone) // can be only KErrNone or KErrInUse
        {
        iUIHandler.ErrorL(KErrInUse);
        }        
    }

// ---------------------------------------------------------------------------
// CMseng::Cancel()
// 
// ---------------------------------------------------------------------------
//
EXPORT_C void CMseng::Cancel()
    {
#ifdef __SHOW_RDEBUG_PRINT_
    RDebug::Print(_L("CMseng::Cancel() called. canceling scanning..."));
#endif // __SHOW_RDEBUG_PRINT_
    
    iScanner->Cancel();
    }

// -----------------------------------------------------------------------------
// CMseng::IsInternalDrive
// -----------------------------------------------------------------------------
//
TBool CMseng::IsInternalDrive( RFs& aFs, TInt aDrv )
    {
    TDriveInfo drvInfo;
    if ( aFs.Drive( drvInfo, aDrv ) == KErrNone )
        {
         if ( !( drvInfo.iDriveAtt & KDriveAttInternal ) &&
              drvInfo.iDriveAtt & ( KDriveAttRemovable | KDriveAttRemote ) )
            {
            return EFalse;
            }
        }
    else
        {
        return EFalse;
        }
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CMseng::IsRemovableDrive
// -----------------------------------------------------------------------------
//
TBool CMseng::IsRemovableDrive( RFs& aFs, TInt aDrv )
    {
    TDriveInfo drvInfo;
    if ( aFs.Drive( drvInfo, aDrv ) == KErrNone )
        {
         if ( !( drvInfo.iDriveAtt & KDriveAttRemovable ) )
            {
            return EFalse;
            }
        }
    else
        {
        return EFalse;
        }
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CMseng::IsMassStorageDrive
// -----------------------------------------------------------------------------
//
TBool CMseng::IsMassStorageDrive( RFs& aFs, TInt aDrv )
    {
    
    TUint drvStatus( 0 );
    TInt err( DriveInfo::GetDriveStatus( aFs, aDrv, drvStatus ) );
    if ( err != KErrNone )
        {
        return EFalse;
        }
    
    if ( ( drvStatus & DriveInfo::EDriveInternal ) &&
        ( drvStatus & DriveInfo::EDriveExternallyMountable ) )
        {
        return ETrue;
        }
    return EFalse;
    }

//  End of File
