/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  CCDevEncPasswdDlg declaration.
*
*/

#ifndef CDEVENCPASSWDDLG_H
#define CDEVENCPASSWDDLG_H

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		// For RTimer, link against: euser.lib
#include <eiknotapi.h>
#include <AknQueryDialog.h>
#include <aknnotewrappers.h>

class CDevEncPasswdDlg : public CActive, 
                         public MEikSrvNotifierBase2
    {
    public:
    	// Cancel and destroy
    	~CDevEncPasswdDlg();
    
    	// Two-phased constructor.
    	static CDevEncPasswdDlg* NewL();
    
    	// Two-phased constructor.
    	static CDevEncPasswdDlg* NewLC();
    
    public: // Functions from MEikSrvNotifierBase2
        /** Frees all resources owned by this notifier.
        
        This function is called by the notifier framework when all resources allocated 
        by notifiers should be freed. As a minimum, this function should delete this 
        object (i.e. delete this;).
        
        Note that it is important to implement this function correctly to avoid memory 
        leaks. */
        virtual void Release();
        /** Performs any initialisation that this notifier may require.
        
        The function is called when the notifier is loaded (when the plug-in DLL is 
        loaded). It is called only once.
        
        As a minimum, the function should return a TNotifierInfo instance describing 
        the notifier parameters. A good implementation would be to set this into a 
        data member, and then to return it. This is because the same information is 
        returned by Info().
        
        The function is safe to leave from, so it is possible, although rarely necessary, 
        to allocate objects as you would normally do in a ConstructL() function as 
        part of two-phase construction.
        
        @return Describes the parameters of the notifier. */
        virtual TNotifierInfo RegisterL();
        /** Gets the notifier parameters.
        
        This is usually the same information as returned by RegisterL() but can be 
        varied at run time.
        
        @return Describes the parameters of the notifier. */
        virtual TNotifierInfo Info() const;
        /** Starts the notifier.
        
        This is called as a result of a client-side call to RNotifier::StartNotifier(), 
        which the client uses to start a notifier from which it does not expect a 
        response.
        
        The function is synchronous, but it should be implemented so that it completes 
        as soon as possible, allowing the notifier framework to enforce its priority 
        mechanism.
        
        It is not possible to to wait for a notifier to complete before returning 
        from this function unless the notifier is likely to finish implementing its 
        functionality immediately.
        
        @param aBuffer Data that can be passed from the client-side. The format and 
        meaning of any data is implementation dependent.
        @return A pointer descriptor representing data that may be returned. The format 
        and meaning of any data is implementation dependent. */
        virtual TPtrC8 StartL(const TDesC8& aBuffer);
        /** Starts the notifier.
        
        This is called as a result of a client-side call to the asynchronous function 
        RNotifier::StartNotifierAndGetResponse(). This means that the client is waiting, 
        asynchronously, for the notifier to tell the client that it has finished its 
        work.
        
        It is important to return from this function as soon as possible, and derived 
        classes may find it useful to take a copy of the reply-slot number and 
        the RMessage object.
        
        The implementation of a derived class must make sure that Complete() is called 
        on the RMessage object when the notifier is deactivated.
        
        This function may be called multiple times if more than one client starts 
        the notifier.
        
        @param aBuffer Data that can be passed from the client-side. The format and 
        meaning of any data is implementation dependent.
        @param aReplySlot Identifies which message argument to use for the reply.
        This message argument will refer to a modifiable descriptor, a TDes8 type, 
        into which data can be returned. The format and meaning of any returned data 
        is implementation dependent.
        @param aMessage Encapsulates a client request. */
        virtual void StartL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage);
        
        /** Cancels an active notifier.
        
        This is called as a result of a client-side call to RNotifier::CancelNotifier().
        
        An implementation should free any relevant resources and complete any outstanding 
        messages, if relevant. */
        virtual void Cancel();
        /** Updates a currently active notifier with new data.
        
        This is called as a result of a client-side call to RNotifier::UpdateNotifier().
        
        @param aBuffer Data that can be passed from the client-side. The format and 
        meaning of any data is implementation dependent.
        @return A pointer descriptor representing data that may be returned. The format 
        and meaning of any data is implementation dependent. */
        virtual TPtrC8 UpdateL(const TDesC8& aBuffer);
    private:
    	// C++ constructor
    	CDevEncPasswdDlg();
    	
    	// Second-phase constructor
    	void ConstructL();
    	
    private: // From CActive
    	// Handle completion
    	void RunL();
    	
    	// How to cancel me
    	void DoCancel();
    	
    	// Override to handle leaves from RunL(). Default implementation causes
    	// the active scheduler to panic.
    	TInt RunError( TInt aError );
    
    private:
        TNotifierInfo           iInfo;
        CAknTextQueryDialog*    iDlg;
        CAknErrorNote*          iNote;
        RMessagePtr2            iMessage;        // Received message
        TInt                    iReplySlot;              // Reply slot
        TBool                   iNeedToCompleteMessage; // Flag for releasing messages
        TInt iResourceFileFlag;       // Flag for eikon env.
        CEikonEnv* iEikEnv;           // Local eikonenv
        RTimer                  iTimer;
    };

#endif // CDEVENCPASSWDDLG_H
