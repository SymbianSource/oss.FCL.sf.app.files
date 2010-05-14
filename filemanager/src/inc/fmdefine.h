/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/
#ifndef FMDEFINE_H
#define FMDEFINE_H

// define for filemanager error
#define  FmErrNone                       0
#define  FmErrNotFound                  -1
#define  FmErrGeneral                   -2
#define  FmErrAlreadyExists             -3
#define  FmErrAccessDenied              -4
#define  FmErrCannotRemove              -5
#define  FmErrCannotCopy                -6
#define  FmErrDiskFull                  -7
#define  FmErrFileUsed                  -8
#define  FmErrFolderUsed                -9
#define  FmErrTypeFormatFailed          -10
#define  FmErrTypeFormatFailedUsed      -11
#define  FmErrAlreadyStarted            -12
#define  FmErrCancel                    -13
#define  FmErrWrongParam                -14
#define  FmErrUnKnown                   -15
#define  FmErrIsNotFileOrFolder         -16
#define  FmErrCannotMakeDir             -17
#define  FmErrSrcPathDoNotExist         -18
#define  FmErrDestPathDoNotExist        -19
#define  FmErrCopyDestToSubFolderInSrc  -20
#define  FmErrMoveDestToSubFolderInSrc  -21
#define  FmErrNotSupported              -22
#define  FmErrNotReady                  -23
#define  FmErrInUse                     -24
#define  FmErrPermissionDenied          -25
#define  FmErrBadName                   -26
#define  FmErrPathNotFound              -27
#define  FmErrRemoveDefaultFolder       -28
#define  FmErrLocked                    -29
#define  FmErrCorrupt                   -30

#define FmEstimateUpperLimit 90 // available mem/backup size*100%=90%, User selectable continuation
#define FmEstimateLowerLimit 10 // available mem/backup size*100%=10%, Backup will be interrupted

#define FmPlaceholderString " " // Placeholder for HbLabel because layout will be wrong when HbLabel is empty

#define Drive_C "C:/"
#define Drive_D "D:/"
#define Drive_Z "Z:/"
#define Folder_C_Data "C:/Data"

#define FmMaxLengthofDriveName           11
#define FmMaxLengthofDrivePassword       8

#endif 
