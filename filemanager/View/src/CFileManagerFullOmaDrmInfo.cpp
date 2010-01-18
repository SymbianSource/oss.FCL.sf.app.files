/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Gets the item DRM information
*
*/


// INCLUDE FILES
#include <coemain.h>
#include <StringLoader.h>
#include <DRMHelper.h>
#include <DRMRights.h>
#include <AknUtils.h>
#include <FileManagerView.rsg>
#include <FileManagerDebug.h>
#include <drmuihandling.h>
#include "CFileManagerFullOmaDrmInfo.h"

// CONSTANTS
_LIT( KSeparator, "\t" );
#ifndef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
_LIT( KDateFormat1, "%1" );
_LIT( KDateFormat2, "%2" );
_LIT( KDateFormat3, "%3" );
_LIT( KTimeFormatBefore, " %-B %J:%T" );
_LIT( KTimeFormatAfter, " %J:%T %+B" );
_LIT( KEmptyChar, " " );
const TInt KDateStringLen = 20;

const TUint KSecondSeparator = 1;
const TUint KThirdSeparator = 2;


const TInt KPlayRights = 0;
const TInt KDisplayRights = 1;
const TInt KExecuteRights = 2;
const TInt KPrintRights = 3;
#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM


// ============================ LOCAL FUNCTIONS ================================

// -----------------------------------------------------------------------------
// AppendLabelAndDataToArrayL
// -----------------------------------------------------------------------------
static void AppendLabelAndDataToArrayL( CDesCArray& aArray,
                                 const TDesC& aLabel,
                                 const TDesC& aData )
    {
    HBufC* dataStr = HBufC::NewLC( aLabel.Length() +
                                   KSeparator().Length() +
                                   aData.Length() );

    TPtr dataPtr( dataStr->Des() );
    dataPtr.Append( aLabel );
    dataPtr.Append( KSeparator );
    dataPtr.Append( aData );

    AknTextUtils::LanguageSpecificNumberConversion( dataPtr );

    aArray.AppendL( dataPtr );

    CleanupStack::PopAndDestroy( dataStr );
    }
    
#ifndef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
// -----------------------------------------------------------------------------
// ResetAndDestroy
// -----------------------------------------------------------------------------
static void ResetAndDestroy( TAny* aPtr )
    {
    RPointerArray< CDRMHelperRightsConstraints >* array =
        static_cast< RPointerArray<CDRMHelperRightsConstraints>* >( aPtr );
    array->ResetAndDestroy();
    array->Close();
    }

// -----------------------------------------------------------------------------
// IsFutureRights
// -----------------------------------------------------------------------------
static TBool IsFutureRights(
        RPointerArray<CDRMRightsConstraints>& aRightsArray )
    {
    TBool ret( EFalse );
    const TInt count( aRightsArray.Count() );
    for( TInt i = 0 ; i < count ; ++i )
        {
        CDRMRightsConstraints* rights = aRightsArray[i];
        TUint32 expiration( 0 );
        TUint32 constType( 0 );
        TInt topPriorityValue( CDRMRights::EInvalidRights );
        if( rights )
            {
            TInt currentPriorityValue( rights->GetConstraintInfo( expiration, constType ) );
            if( currentPriorityValue > topPriorityValue )
                {
                // priority is higher than previous rigths
                topPriorityValue = currentPriorityValue;
                if( CDRMRights::EFutureRights == expiration )
                    {
                    ret = ETrue;
                    }
                else
                    {
                    // no future rights
                    ret = EFalse;
                    }
                }
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// DateTimeStringLC
// -----------------------------------------------------------------------------
static HBufC* DateTimeStringLC( const TTime& aTime )
    {
    TBuf<KDateStringLen> dateStr;
    TBuf<KDateStringLen> timeStr;
    TBuf<KDateStringLen> dateStrFormat;

    // Localized date separator form
    TLocale local;
    dateStrFormat.Append( KDateFormat1 );
    dateStrFormat.Append( local.DateSeparator( KSecondSeparator ) );
    dateStrFormat.Append( KDateFormat2 );
    dateStrFormat.Append( local.DateSeparator( KThirdSeparator ) );
    dateStrFormat.Append( KDateFormat3 );
    aTime.FormatL( dateStr, dateStrFormat );

    if ( local.AmPmSymbolPosition() == ELocaleBefore )
        {
        aTime.FormatL( timeStr, KTimeFormatBefore );
        }
    else
        {
        aTime.FormatL( timeStr, KTimeFormatAfter );
        }

    HBufC* buf = HBufC::NewLC( dateStr.Length() +
                               KEmptyChar().Length() +
                               timeStr.Length() );
    TPtr ptrBuffer( buf->Des() );
    ptrBuffer.Append( dateStr );
    ptrBuffer.Append( KEmptyChar );
    ptrBuffer.Append( timeStr );

    return buf;
    }

// -----------------------------------------------------------------------------
// DateTimeL
// -----------------------------------------------------------------------------
static void DateTimeL( CDesCArray& aArray,
                const TTime& aTime,
                TInt aResourceId,
                const TDesC& aType )
    {
    HBufC* label = StringLoader::LoadLC( aResourceId, aType );
    HBufC* dateTime = DateTimeStringLC( aTime );

    AppendLabelAndDataToArrayL( aArray, *label, *dateTime );

    CleanupStack::PopAndDestroy( dateTime );
    CleanupStack::PopAndDestroy( label );
    }

// -----------------------------------------------------------------------------
// FillCounterInfoL
// -----------------------------------------------------------------------------
static void FillCounterInfoL( CDesCArray& aArray,
                       CDRMHelperRightsConstraints& aRights,
                       const TDesC& aType )
    {
    TUint32 count( 0 );
    TUint32 timedCount( 0 );
    TUint32 ignore1( 0 );
    TUint32 ignore2( 0 );
    TTimeIntervalSeconds ignore3( 0 );
    TInt err( KErrNone );

    TRAPD( errCount, aRights.GetCountersL( count, ignore1 ) );
    TRAPD( errTimedCount, aRights.GetTimedCountL( timedCount, ignore2,
        ignore3 ) );

    if ( errCount == KErrNone && errTimedCount == KErrNone )
        {
        // Both counts present, use minimum
        count = Min( count, timedCount );
        err = KErrNone;
        }
    else if ( errCount == KErrNone )
        {
        // Use count
        err = KErrNone;
        }
    else if ( errTimedCount == KErrNone )
        {
        // Use timed count
        count = timedCount;
        err = KErrNone;
        }
    else
        {
        // Neither regular nor timed count constraint
        // present, return error
        err = KErrNotFound;
        }

    if ( err == KErrNone )
        {
        // "Times left (%U)"
        HBufC* label =
                StringLoader::LoadLC( R_QTN_DRM_MGR_DET_UTL_X, aType );

        HBufC* data = NULL;

        if ( count == 1 )
            {
            // "1 count"
            data = StringLoader::LoadLC( R_QTN_DRM_MGR_DET_1_COUNT );
            }
        else
            {
            // "%N counts"
            data = StringLoader::LoadLC( R_QTN_DRM_MGR_DET_N_COUNTS, count );
            }

        AppendLabelAndDataToArrayL( aArray, *label, *data );

        CleanupStack::PopAndDestroy( data );
        CleanupStack::PopAndDestroy( label );
        }
    }

// -----------------------------------------------------------------------------
// SplitTime
// -----------------------------------------------------------------------------
//
static void SplitTime( const TTimeIntervalSeconds& aInterval,
        TInt& aIntYrs, TInt& aIntMon, TInt& aIntDay,
        TInt& aIntHrs, TInt& aIntMin, TInt& aIntSec )
    {
    const TInt KSecsInMin( 60 );
    const TInt KSecsInHour( KSecsInMin * 60 );
    const TInt KSecsInDay( KSecsInHour * 24 );

    // includes leap year day
    const TInt KLastMonthIndex = 11;
    const TInt KMaxDaysInMonths[] = {
            31,  62,  92, 123, 153, 184,
            215, 245, 276, 306, 337, 366 };

    // calculate full days
    TInt temp( aInterval.Int() / KSecsInDay );

    // calculate full years, calculate without leap year for user to get the 
    // longest time possible
    aIntYrs = temp / ( KMaxDaysInMonths[KLastMonthIndex] - 1 );

    // calc remainder days
    temp = temp % ( KMaxDaysInMonths[KLastMonthIndex] - 1 );

    aIntMon = 0;

    TInt i( 0 );
    if ( temp >= KMaxDaysInMonths[0] )
        {
        for ( i = 0; i < KLastMonthIndex; i++ )
            {
            // found correct amount of months
            if ( temp >= KMaxDaysInMonths[i] && temp < KMaxDaysInMonths[i+1] )
                {
                // i now contains amount of full months (+1 because of table index)
                aIntMon = i + 1;
                break;
                }
            }
        }

    // calc remainder days = allSecs - secsInFullYears - secsInFullMonts
    if( temp >= KMaxDaysInMonths[i] )
        {
        aIntDay = temp - KMaxDaysInMonths[i];
        }
    else
        {
        aIntDay = temp;
        }

    // calculate remainder secs
    temp = aInterval.Int() % KSecsInDay;

    aIntHrs = temp / KSecsInHour;

    // calculate remainder secs
    temp = temp % KSecsInHour;

    aIntMin = temp / KSecsInMin;

    // calculate remainder secs
    aIntSec = temp % KSecsInMin;
    }

// -----------------------------------------------------------------------------
// AddSinglePartOfTimeL
// -----------------------------------------------------------------------------
//
static void AddSinglePartOfTimeL( TInt aNumOfElements,
        TInt aResourceIdSingle,
        TInt aResourceIdOneFinal, 
        TInt aResourceIdTwoFour,
        TInt aResourceIdFiveZero, 
        CDesCArrayFlat* aStrings )
    {
    const TInt KDigitFive = 5;
    const TInt KDigitNine = 9;
    const TInt KDigitTen = 10;
    const TInt KDigitEleven = 11;
    const TInt KDigitFourTeen = 14;
    const TInt KDigitHundred = 100;
    HBufC* stringHolder = NULL;
    TInt finalOneDigit( aNumOfElements % KDigitTen );
    TInt finalTwoDigits( aNumOfElements % KDigitHundred );

    if ( aNumOfElements == 1 )
        {
        stringHolder = StringLoader::LoadLC( aResourceIdSingle );
        }
    else if ( finalOneDigit == 1 && finalTwoDigits != KDigitEleven )
        {
        stringHolder = StringLoader::LoadLC( aResourceIdOneFinal, 
            aNumOfElements );
        }
    else if ( finalOneDigit == 0 || 
             ( finalOneDigit >= KDigitFive && finalOneDigit <= KDigitNine ) ||
             ( finalTwoDigits >= KDigitEleven && finalTwoDigits <= KDigitFourTeen ) )
        {
        stringHolder = StringLoader::LoadLC( aResourceIdFiveZero,
            aNumOfElements );
        }
    else
        {
        stringHolder = StringLoader::LoadLC( aResourceIdTwoFour,
            aNumOfElements );
        }

    if ( aStrings )
        {
        aStrings->AppendL( *stringHolder );
        }

    CleanupStack::PopAndDestroy ( stringHolder );
    }

// -----------------------------------------------------------------------------
// AddPartsOfTimeLC
// -----------------------------------------------------------------------------
//
static HBufC* AddPartsOfTimeLC( TInt aIntYrs, TInt aIntMon, TInt aIntDay, 
                         TInt aIntHrs, TInt aIntMin, TInt aIntSec )
    {
    // Only the two most meaningful data will be showed
    TInt numOfData( 0 );
    const TInt KMaxDataItems = 2;

    CDesCArrayFlat* strings = new ( ELeave ) CDesCArrayFlat( KMaxDataItems );
    CleanupStack::PushL( strings );

    if ( aIntYrs > 0 )
        {
        AddSinglePartOfTimeL( aIntYrs, 
            R_QTN_DRM_NBR_OF_YEARS_ONE,
            R_QTN_DRM_NBR_OF_YEARS_ONE_FINAL,
            R_QTN_DRM_NBR_OF_YEARS_TWO_FOUR,
            R_QTN_DRM_NBR_OF_YEARS_FIVE_ZERO,
            strings );
        numOfData++;
        }

    if ( aIntMon > 0 )
        {
        //  Second type not provided because 11 is the maximum
        AddSinglePartOfTimeL( aIntMon, 
            R_QTN_DRM_NBR_OF_MONTHS_ONE,
            0,       
            R_QTN_DRM_NBR_OF_MONTHS_TWO_FOUR, 
            R_QTN_DRM_NBR_OF_MONTHS_FIVE_ZERO, 
            strings );
        numOfData++;
        }

    // Only if years or months were missing
    if ( aIntDay > 0 && numOfData < KMaxDataItems )
        {
        AddSinglePartOfTimeL( aIntDay, 
            R_QTN_DRM_NBR_OF_DAYS_ONE,
            R_QTN_DRM_NBR_OF_DAYS_ONE_FINAL,
            R_QTN_DRM_NBR_OF_DAYS_TWO_FOUR, 
            R_QTN_DRM_NBR_OF_DAYS_FIVE_ZERO, 
            strings );
        numOfData++;
        }

    if ( aIntHrs > 0 && numOfData < KMaxDataItems )
        {
        AddSinglePartOfTimeL( aIntHrs, 
            R_QTN_DRM_NBR_OF_HOURS_ONE, 
            R_QTN_DRM_NBR_OF_HOURS_ONE_FINAL,
            R_QTN_DRM_NBR_OF_HOURS_TWO_FOUR, 
            R_QTN_DRM_NBR_OF_HOURS_FIVE_ZERO, 
            strings );
        numOfData++;
        }

    if ( aIntMin > 0 && numOfData < KMaxDataItems )
        {
        AddSinglePartOfTimeL( aIntMin, 
            R_QTN_DRM_NBR_OF_MINS_ONE,
            R_QTN_DRM_NBR_OF_MINS_ONE_FINAL,
            R_QTN_DRM_NBR_OF_MINS_TWO_FOUR, 
            R_QTN_DRM_NBR_OF_MINS_FIVE_ZERO, 
            strings );
        numOfData++;
        }

    // If interval is 0, then it shows "0 seconds" anyway
    if ( ( aIntSec > 0 && numOfData < KMaxDataItems ) || numOfData == 0 )
        {
        AddSinglePartOfTimeL( aIntSec, 
            R_QTN_DRM_NBR_OF_SECS_ONE, 
            R_QTN_DRM_NBR_OF_SECS_ONE_FINAL,
            R_QTN_DRM_NBR_OF_SECS_TWO_FOUR, 
            R_QTN_DRM_NBR_OF_SECS_FIVE_ZERO, 
            strings );
        numOfData++;
        }

    HBufC* stringHolder = NULL;
    if ( numOfData == 1 )
        {
        stringHolder = StringLoader::LoadL( R_QTN_DRM_MGR_DET_INTER,
            strings->MdcaPoint(0) );
        }
    else
        {
        stringHolder = StringLoader::LoadL( R_QTN_DRM_MGR_DET_INTER_TWO,
            *strings );
        }

    CleanupStack::PopAndDestroy( strings );
    
    CleanupStack::PushL( stringHolder );

    return stringHolder;
    }

// -----------------------------------------------------------------------------
// FillUsageTimeLeftInfoL
// -----------------------------------------------------------------------------
static void FillUsageTimeLeftInfoL( CDesCArray& aArray,
                             const TTimeIntervalSeconds& aInterval,
                             const TDesC& aType,
                             TBool aIsAccumulatedTime )
    {
    TInt years( 0 );
    TInt months( 0 );
    TInt days( 0 );
    TInt hours( 0 );
    TInt minutes( 0 );
    TInt seconds( 0 );
    HBufC* label = NULL;
    HBufC* data = NULL;

    if ( aIsAccumulatedTime )
        {
        // "Usage time left"
        label = StringLoader::LoadLC( R_QTN_DRM_MGR_DET_ACCUM_TIME_LEFT  );
        }
    else
        {
        // "Time left (%U)"
        label = StringLoader::LoadLC( R_QTN_DRM_MGR_DET_UDL_X, aType );
        }

    SplitTime( aInterval, years, months, days, hours, minutes, seconds );
    data = AddPartsOfTimeLC( years, months, days, hours, minutes, seconds );

    AppendLabelAndDataToArrayL( aArray, *label, *data );

    CleanupStack::PopAndDestroy( data );
    CleanupStack::PopAndDestroy( label );
    }

// -----------------------------------------------------------------------------
// FillIntervalInfoL
// -----------------------------------------------------------------------------
static void FillIntervalInfoL( CDesCArray& aArray,
                        CDRMHelperRightsConstraints& aRights,
                        const TDesC& aType )
    {
    TTimeIntervalSeconds intervalSeconds( 0 );

    TRAPD( err, aRights.GetIntervalL( intervalSeconds ) );

    if ( err != KErrNotFound && err != KErrNone )
        {
        User::Leave( err );
        }

    if ( err == KErrNone )
        {
        TTime intervalStartTime( 0 );

        TRAP( err, aRights.GetIntervalStartL( intervalStartTime ) );

        if ( ( err != KErrNotFound ) && ( err != KErrNone ) )
            {
            User::Leave( err );
            }

        if ( err == KErrNotFound )
            {
            // "Times status (%U)"
            HBufC* label =
                StringLoader::LoadLC( R_QTN_DRM_MGR_DET_UTS_X, aType );

            // "Not activated"
            HBufC* data = StringLoader::LoadLC( R_QTN_DRM_MGR_DET_NOT_ACT );

            AppendLabelAndDataToArrayL( aArray, *label, *data );

            CleanupStack::PopAndDestroy( data );
            CleanupStack::PopAndDestroy( label );

            // "Time left (%U)"
            FillUsageTimeLeftInfoL( aArray, intervalSeconds, aType, EFalse );
            }
        else
            {
            TTime endTime( intervalStartTime );
            endTime += intervalSeconds;

            // "Valid from (%U)"
            DateTimeL( aArray, intervalStartTime,
                                        R_QTN_DRM_MGR_DET_RVF_X, aType );

            // "Valid until (%U)"
            DateTimeL( aArray, endTime, R_QTN_DRM_MGR_DET_RVT_X, aType );
            }
        }
    }

// -----------------------------------------------------------------------------
// FillTimeInfoL
// -----------------------------------------------------------------------------
static void FillTimeInfoL( CDesCArray& aArray,
                    CDRMHelperRightsConstraints& aRights,
                    const TDesC& aType )
    {
    TTime startTime;
    TRAPD( err, aRights.GetStartTimeL( startTime ) );
    if ( err != KErrNotFound && err != KErrNone )
        {
        User::Leave( err );
        }
    if ( err == KErrNone )
        {
        // "Valid from (%U)"
        DateTimeL( aArray, startTime, R_QTN_DRM_MGR_DET_RVF_X, aType );
        }

    TTime endTime;
    TRAP( err, aRights.GetEndTimeL( endTime ) );
    if ( err != KErrNotFound && err != KErrNone )
        {
        User::Leave( err );
        }
    if ( err == KErrNone )
        {
        // "Valid until (%U)"
        DateTimeL( aArray, endTime, R_QTN_DRM_MGR_DET_RVT_X, aType );
        }
    }

// -----------------------------------------------------------------------------
// FillAccumulatedTimeInfoL
// -----------------------------------------------------------------------------
static void FillAccumulatedTimeInfoL( CDesCArray& aArray,
                               CDRMHelperRightsConstraints& aRights,
                               const TDesC& aType )
    {
    TTimeIntervalSeconds accumSeconds( 0 );
    TRAPD( err, aRights.GetAccumulatedTimeL( accumSeconds ));
    if ( err != KErrNotFound && err != KErrNone )
        {
        User::Leave( err );
        }
    if ( err == KErrNone )
        {
        // "Usage time left"
        FillUsageTimeLeftInfoL( aArray, accumSeconds, aType, ETrue );
        }
    }

// -----------------------------------------------------------------------------
// FillDrmInfoL
// -----------------------------------------------------------------------------
static void FillDrmInfoL( CDesCArray& aArray,
                   CDRMHelperRightsConstraints* aRights,
                   TInt aResourceId )
    {
    // Check if no rights at all
    if ( !aRights )
        {
        return;
        }

    // Rights type is either "Play", "Display", "Execute" or "Print"
    HBufC* type = StringLoader::LoadLC( aResourceId );

    // Check if full rights
    if ( aRights->FullRights() )
        {
        // "Rights (%U)"
        HBufC* label =
                StringLoader::LoadLC( R_QTN_DRM_MGR_DET_FULL_X, *type );

        // "Unlimited"
        HBufC* data = StringLoader::LoadLC( R_QTN_DRM_MGR_DET_UNLIMITED );

        AppendLabelAndDataToArrayL( aArray, *label, *data );

        CleanupStack::PopAndDestroy( data );
        CleanupStack::PopAndDestroy( label );
        CleanupStack::PopAndDestroy( type );
        return;                           // full rights -> return
        }

    // Get detailed counter constraint information
    FillCounterInfoL( aArray, *aRights, *type );

    // Get detailed interval constraint information (start time + duration)
    FillIntervalInfoL( aArray, *aRights, *type );

    // Get detailed time constraint information (start time + end time)
    FillTimeInfoL( aArray, *aRights, *type );

    // Get detailed accumulated time constraint information (duration)
    FillAccumulatedTimeInfoL( aArray, *aRights, *type );

    CleanupStack::PopAndDestroy( type );
    }

#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerFullOmaDrmInfo
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CFileManagerFullOmaDrmInfo::CFileManagerFullOmaDrmInfo()
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerFullOmaDrmInfo::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerFullOmaDrmInfo* CFileManagerFullOmaDrmInfo::NewL( CDesCArray& aArray, 
                                                         const TDesC& aFullPath,
                                                         CCoeEnv& aCoeEnv )
    {
    CFileManagerFullOmaDrmInfo* self = NewLC( aArray, aFullPath, aCoeEnv );
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerFullOmaDrmInfo::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerFullOmaDrmInfo* CFileManagerFullOmaDrmInfo::NewLC( CDesCArray& aArray, 
                                                          const TDesC& aFullPath,
                                                          CCoeEnv& aCoeEnv )
    {
    CFileManagerFullOmaDrmInfo* self = new( ELeave ) CFileManagerFullOmaDrmInfo();
 
    CleanupStack::PushL( self );
    self->ConstructL( aArray, aFullPath, aCoeEnv );

    return self;
    }
    
// -----------------------------------------------------------------------------
// CFileManagerFullOmaDrmInfo::~CFileManagerFullOmaDrmInfo
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerFullOmaDrmInfo::~CFileManagerFullOmaDrmInfo()
    {
    }

#ifndef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

// -----------------------------------------------------------------------------
// CImageInfoPopupList::ConstructL
// -----------------------------------------------------------------------------
//
void CFileManagerFullOmaDrmInfo::ConstructL( CDesCArray& aItemArray,
                                          const TDesC& aFileName,
                                          CCoeEnv& aCoeEnv )
    {
    TBool expired ( EFalse );
    TBool sendingAllowed( EFalse );
    RPointerArray<CDRMHelperRightsConstraints> tempArr;
    TCleanupItem cleanupItem( ResetAndDestroy, &tempArr );
    CleanupStack::PushL( cleanupItem );
    tempArr.AppendL( NULL ); // Play
    tempArr.AppendL( NULL ); // Display
    tempArr.AppendL( NULL ); // Execute
    tempArr.AppendL( NULL ); // Print

    CDRMHelper* drmHelper = CDRMHelper::NewLC( aCoeEnv );

    TRAPD( err, drmHelper->GetRightsDetailsL(
                    aFileName,
                    0, // Details for everything
                    expired,
                    sendingAllowed,
                    tempArr[KPlayRights],
                    tempArr[KDisplayRights],
                    tempArr[KExecuteRights],
                    tempArr[KPrintRights] ) );
    TBool future( EFalse );
    if( expired || err == CDRMRights::ENoRights )
        {
        // Check future rights
        RPointerArray<CDRMRightsConstraints> fArr;
        TCleanupItem cleanupItem( ResetAndDestroy, &fArr );
        CleanupStack::PushL( cleanupItem );
        fArr.AppendL( NULL ); // Play
        fArr.AppendL( NULL ); // Display
        fArr.AppendL( NULL ); // Execute
        fArr.AppendL( NULL ); // Print

        TBool tmp( EFalse );
        TBool tmp2( EFalse );
        TRAP( err, drmHelper->GetRightsDetailsL( aFileName,
                                 0,
                                 tmp,
                                 tmp2,
                                 fArr[KPlayRights],
                                 fArr[KDisplayRights],
                                 fArr[KExecuteRights],
                                 fArr[KPrintRights] ) );

        if( err == KErrNone || err == CDRMRights::ENoRights )
            {
            future = IsFutureRights( fArr );
            }
        CleanupStack::PopAndDestroy( &fArr );

        ERROR_LOG2( "CFileManagerFullOmaDrmInfo::ConstructL-err:%d,future:%d",
            err, future )
        }
    CleanupStack::PopAndDestroy( drmHelper );

    ERROR_LOG3( "CFileManagerFullOmaDrmInfo::ConstructL-err:%d,expired:%d,sendingAllowed:%d",
        err, expired, sendingAllowed )

    // Check if the rights are valid or expired
    // Should probably be able to append this information also when the
    // rights have expired.
    // "Status"
    HBufC* label = StringLoader::LoadLC( R_QTN_DRM_MGR_DET_STAT );
    HBufC* data = NULL;
    // "Valid" or "Expired" or "Not valid yet"
    TInt resId( R_QTN_DRM_MGR_DET_VALID );
    if( future )
        {
        resId = R_QTN_DRM_MGR_DET_NOT_VALID_YET;
        }
    else if ( expired || err == CDRMRights::ENoRights )
        {
        resId = R_QTN_DRM_MGR_DET_EXP;
        }
    data = StringLoader::LoadLC( resId );
    AppendLabelAndDataToArrayL( aItemArray, *label, *data );
    CleanupStack::PopAndDestroy( data );
    CleanupStack::PopAndDestroy( label );

    // "Play"
    FillDrmInfoL( aItemArray, tempArr[KPlayRights], R_QTN_DRM_MGR_DET2_PLAY );

    // "Display"
    FillDrmInfoL( aItemArray, tempArr[KDisplayRights], R_QTN_DRM_MGR_DET2_DISPLAY );

    // "Execute"
    FillDrmInfoL( aItemArray, tempArr[KExecuteRights], R_QTN_DRM_MGR_DET2_EXECUTE );

    // "Print"
    FillDrmInfoL( aItemArray, tempArr[KPrintRights], R_QTN_DRM_MGR_DET2_PRINT );

    CleanupStack::PopAndDestroy( &tempArr );

    // Check whether sending is allowed or not

    // "Sending"
    label = StringLoader::LoadLC( R_QTN_DRM_MGR_DET_CS );
    data = NULL;
    // "Allowed" or "Forbidden"
    resId = ( sendingAllowed ? R_QTN_DRM_MGR_DET_ALLOWED
                             : R_QTN_DRM_MGR_DET_FORBID );
    data = StringLoader::LoadLC( resId );
    AppendLabelAndDataToArrayL( aItemArray, *label, *data );
    CleanupStack::PopAndDestroy( data );
    CleanupStack::PopAndDestroy( label );
    }

#else // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

// -----------------------------------------------------------------------------
// CImageInfoPopupList::ConstructL
// -----------------------------------------------------------------------------
//
void CFileManagerFullOmaDrmInfo::ConstructL( CDesCArray& aItemArray,
                                          const TDesC& /*aFileName*/,
                                          CCoeEnv& /*aCoeEnv*/ )
    {
    /*
    TBool expired ( EFalse );
    TBool sendingAllowed( EFalse );
    RPointerArray<CDRMHelperRightsConstraints> tempArr;
    TCleanupItem cleanupItem( ResetAndDestroy, &tempArr );
    CleanupStack::PushL( cleanupItem );
    tempArr.AppendL( NULL ); // Play
    tempArr.AppendL( NULL ); // Display
    tempArr.AppendL( NULL ); // Execute
    tempArr.AppendL( NULL ); // Print

    CDRMHelper* drmHelper = CDRMHelper::NewLC( aCoeEnv );

    TRAPD( err, drmHelper->GetRightsDetailsL(
                    aFileName,
                    0, // Details for everything
                    expired,
                    sendingAllowed,
                    tempArr[KPlayRights],
                    tempArr[KDisplayRights],
                    tempArr[KExecuteRights],
                    tempArr[KPrintRights] ) );
    TBool future( EFalse );
    if( expired || err == CDRMRights::ENoRights )
        {
        // Check future rights
        RPointerArray<CDRMRightsConstraints> fArr;
        TCleanupItem cleanupItem( ResetAndDestroy, &fArr );
        CleanupStack::PushL( cleanupItem );
        fArr.AppendL( NULL ); // Play
        fArr.AppendL( NULL ); // Display
        fArr.AppendL( NULL ); // Execute
        fArr.AppendL( NULL ); // Print

        TBool tmp( EFalse );
        TBool tmp2( EFalse );
        TRAP( err, drmHelper->GetRightsDetailsL( aFileName,
                                 0,
                                 tmp,
                                 tmp2,
                                 fArr[KPlayRights],
                                 fArr[KDisplayRights],
                                 fArr[KExecuteRights],
                                 fArr[KPrintRights] ) );

        if( err == KErrNone || err == CDRMRights::ENoRights )
            {
            future = IsFutureRights( fArr );
            }
        CleanupStack::PopAndDestroy( &fArr );

        ERROR_LOG2( "CFileManagerFullOmaDrmInfo::ConstructL-err:%d,future:%d",
            err, future )
        }
    CleanupStack::PopAndDestroy( drmHelper );

    ERROR_LOG3( "CFileManagerFullOmaDrmInfo::ConstructL-err:%d,expired:%d,sendingAllowed:%d",
        err, expired, sendingAllowed )

    if ( tempArr[KPlayRights] ||
        tempArr[KDisplayRights] ||
        tempArr[KExecuteRights] ||
        tempArr[KPrintRights] )
        {
    */
        // Add link to display rights details
    _LIT( KLinkTagStart, "<AknMessageQuery Link>" );
    _LIT( KLinkTagEnd, "</AknMessageQuery Link>" );

    HBufC* linkTitle = StringLoader::LoadLC( R_QTN_FMGR_DRM_DET_LINK );
    HBufC* linkText = StringLoader::LoadLC( R_QTN_FMGR_DRM_DET_LINK_VIEW );
    HBufC* dataStr = HBufC::NewLC( KLinkTagStart().Length() +
                                   linkText->Length() +
                                   KLinkTagEnd().Length() );
    TPtr dataPtr( dataStr->Des() );
    dataPtr.Append( KLinkTagStart );
    dataPtr.Append( *linkText );
    dataPtr.Append( KLinkTagEnd );
    AppendLabelAndDataToArrayL( aItemArray, *linkTitle, *dataStr );
    CleanupStack::PopAndDestroy( dataStr );
    CleanupStack::PopAndDestroy( linkText );
    CleanupStack::PopAndDestroy( linkTitle );
    /*
        }
    else
        {
        // Add forward-lock details
        // "Status"
        HBufC* label = StringLoader::LoadLC( R_QTN_DRM_MGR_DET_STAT );
        HBufC* data = NULL;
        // "Valid" or "Expired" or "Not valid yet"
        TInt resId( R_QTN_DRM_MGR_DET_VALID );
        if( future )
            {
            resId = R_QTN_DRM_MGR_DET_NOT_VALID_YET;
            }
        else if ( expired || err == CDRMRights::ENoRights )
            {
            resId = R_QTN_DRM_MGR_DET_EXP;
            }
        data = StringLoader::LoadLC( resId );
        AppendLabelAndDataToArrayL( aItemArray, *label, *data );
        CleanupStack::PopAndDestroy( data );
        CleanupStack::PopAndDestroy( label );

        // "Sending"
        label = StringLoader::LoadLC( R_QTN_DRM_MGR_DET_CS );
        data = NULL;
        // "Allowed" or "Forbidden"
        resId = ( sendingAllowed ? R_QTN_DRM_MGR_DET_ALLOWED
                                 : R_QTN_DRM_MGR_DET_FORBID );
        data = StringLoader::LoadLC( resId );
        AppendLabelAndDataToArrayL( aItemArray, *label, *data );
        CleanupStack::PopAndDestroy( data );
        CleanupStack::PopAndDestroy( label );
        }

    CleanupStack::PopAndDestroy( &tempArr );
    */
    }

// -----------------------------------------------------------------------------
// CImageInfoPopupList::ViewDetailsL
// -----------------------------------------------------------------------------
//
void CFileManagerFullOmaDrmInfo::ViewDetailsL(
        const TDesC& aFileName,
        DRM::CDrmUiHandling* aUiHandling )
    {
    RFs fs;
    User::LeaveIfError( fs.Connect() );
    CleanupClosePushL( fs );
    
    RFile64 drmFile;
    User::LeaveIfError( drmFile.Open( 
            fs, aFileName, EFileRead | EFileShareReadersOrWriters ) );
    
    CleanupClosePushL( drmFile );
    
    TRAPD( err, aUiHandling->ShowDetailsViewL( drmFile ) );
    
    CleanupStack::PopAndDestroy( &drmFile );
    CleanupStack::PopAndDestroy( &fs );
    
    }

#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

// End of file
