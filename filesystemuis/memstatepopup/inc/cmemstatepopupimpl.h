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
* Description:  Memory state popup implementation
*
*/


#ifndef __CMEMORYSTATEPOPUPIMPL_H__
#define __CMEMORYSTATEPOPUPIMPL_H__

// SYSTEM INCLUDES
#include <e32base.h>
#include <AknWaitDialog.h>
#include <ConeResLoader.h>
#include <msenguihandler.h>

//  FORWARD DECLARATIONS
class CAknPopupList;
class CAknSingleHeadingPopupMenuStyleListBox;
class CMemScanEventReceiver;
class CMSPUtil;

//  CLASS DEFINITION
NONSHARABLE_CLASS(CMemStatePopupImpl) :	
                        public CActive, 
						public MMsengUIHandler,
						public MProgressDialogCallback
	{
	public:
		//   Destructor
		virtual ~CMemStatePopupImpl( );

	private:
		//   Second phase
		void ConstructL( );

		//   C++ constructors
		CMemStatePopupImpl( TDriveNumber aDrive, const TDesC& aTitle );
		CMemStatePopupImpl( );

	public:
		/**
		* Launches the memory scan popup
		* @param aDrive The drive to be scanned
		* @param aTitle Title of the popup
		*/
		static void RunLD( TDriveNumber aDrive, const TDesC& aTitle );

		/**
		* Get an instance of CMSPUtil class, containing the unit text array
		* @param aUtil Reference to a CMSPUtil pointer,
		*		 updated to point to the instance created
		*/
		static void GetUtilL( CMSPUtil*& aUtil );

	public:
		// From MMsengUIHandler
		void StartL( );
		void QuitL( TInt /*aReason*/ );
		void ErrorL( TInt aError );

		// From MProgressDialogCallback
		virtual void DialogDismissedL( TInt /*aButtonId*/ );

	private:
	
	    // From CActive
        void RunL();
        void DoCancel();
        
		/**
		* Starts observing drive dismounts.
		*/
        void StartObserver( );

		/**
		* Construct popup and ready it for use.
		*/
		void ConstructPopupL( );

		/**
		* Shows wait dialog and start scanning.
		*/
		void ExecuteL( );

		/**
		* Refresh the data shown in listbox.
		*/
		void RefreshL( );

		/**
		* Set the results to zero.
		*/
		void NullifyResults( );

		/**
		* Opens the resource and reads unit array from it.
		*/
		void OpenResourceAndReadArrayL( );

	private:
		// Not implemented
		CMemStatePopupImpl( const CMemStatePopupImpl& );
		CMemStatePopupImpl& operator=( CMemStatePopupImpl& );

	private:
        /**
        * Container class used to prevent all pointer events 
        * passing from listbox to popup-list.
        */
        NONSHARABLE_CLASS(CMemStateListBox) : public CAknSingleHeadingPopupMenuStyleListBox
            {
        	public:
        		//   Destructor
        		virtual ~CMemStateListBox( ) {};
            };

	private:
		CDesCArray* iGroupNames;
		CArrayFix< TInt64 >* iScanResults;
		CDesCArray* iListModel;
        CMemStateListBox* iListBox;
		CAknWaitDialog* iWaitDialog;
		TBool iFinished;
		TDriveNumber iDrive;
		CAknPopupList* iPopup;
		TDesC* iTitle;
		CCoeEnv& iCoeEnv;
		RConeResourceLoader iResLoader;
		CMSPUtil* iUtil;
		TBool iOwnsUtil;
        CMemScanEventReceiver* iEventReceiver;
	};

#endif      //  __CMEMORYSTATEPOPUPIMPL_H__

// End of File
