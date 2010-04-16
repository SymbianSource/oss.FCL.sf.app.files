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
        int &selectedIndex );

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
        int dominantIndex );

    /**
     * Shows time setting page
     *
     * @param title Title text id
     * @param time Selected time
     * @return true if selection made, otherwise false
     */
    static bool showTimeSettingQuery(
        const QString &title, QTime &time );

    static bool showTextQuery(     
       const QString &title, QString &driveName );

    static bool showConfirmPasswordQuery(     
       const QString &title, const QString &driveName, QString &pwd );

    static bool showChangePasswordQuery(     
       const QString &firstLabel, const QString &secondLabel, QString &newPassword );

    ~FmDlgUtils(void);
private:
    FmDlgUtils(void);
};


#endif
