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
 * 
 * Description:
 *     The header file of the query dialog utility of file manager
 */

#ifndef FMDLGUTILS_H
#define FMDLGUTILS_H

#include "fmcommon.h"

#include <QString>
#include <QStringList>
#include <QTime>

class FmDialog;
class HbAction;

class FmDlgUtils : public QObject
{
    Q_OBJECT
public:

     /**
     * Shows single select setting page
     *
     * @param title Title text
     * @param textList Text list for available settings
     * @param selectedIndex For storing selected index
     * @return true if selection made, otherwise false
     */
    static bool showSingleSettingQuery(
        const QString &title,
        const QStringList &textList,
        int &selectedIndex,
		const QString &associatedDrives = QString() );

    /**
     * Shows multi select setting page
     *
     * @param title Title text
     * @param textList Text list for available settings
     * @param selection For storing selected indexes as bitmask
     * @param dominantIndex Dominant index for select all behaviour
     * @return true if selection made, otherwise false
     */
	static bool showMultiSettingQuery(
        const QString &title,
        const QStringList &textList,
        quint32 &selection,
        int dominantIndex,
		const QString &associatedDrives = QString() );

    /**
     * Shows time setting page
     *
     * @param title Title text id
     * @param time Selected time
     * @return true if selection made, otherwise false
     */
    static bool showTimeSettingQuery(
        const QString &title, QTime &time,
		const QString &associatedDrives = QString() );

    static bool showTextQuery(     
       const QString &title, QString &driveName, bool isDimPrimaryActionWhenEmpty, int maxLength = -1,
	   const QString &associatedDrives = QString(), bool isReturnFalseWhenNoTextChanged = true );

    static bool showSinglePasswordQuery(     
       const QString &title, QString &pwd, int maxLength = -1,
	   const QString &associatedDrives = QString() );

    static bool showMultiPasswordQuery(     
       const QString &firstLabel, const QString &secondLabel, QString &pwd, int maxLength = -1,
	   const QString &associatedDrives = QString() );
    
    static bool question( const QString &questionText, const QString &primaryButtonText = tr("Yes"),
            const QString &secondaryButtonText = tr("No") );

    static void information( const QString &informationText );

    ~FmDlgUtils(void);
private:
    FmDlgUtils(void);
	static HbAction *executeDialog( FmDialog *dialog, const QString &associatedDrives );
};


#endif
