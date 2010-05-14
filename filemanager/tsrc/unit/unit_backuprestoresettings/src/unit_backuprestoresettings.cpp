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
* Description: This class is test case for backupsettings and restoresettings
*
*/

#include <QtTest/QtTest>

#include "fmbackupsettings.h"
#include "fmrestoresettings.h"
#include "fmbkupengine.h"
#include <QStringList>

class TestSettings: public QObject
{
    Q_OBJECT

private slots:
    void testTestSettingsContentAll();
    void testTestSettingsContentOne();
    void testTestSettingsContentTwo();

    void testContentToString();
    void testContentToString_data();

    void testSchedulingToString();
    void testSchedulingToString_data();

    void testWeekdayToString();
    void testWeekdayToString_data();

    void testFmBackupEntry();

    void testFmRestoreInfo();
    void testFmRestoreEntry();

    void cleanupTestCase();       // Finalize test data

private:
      FmBackupSettings *settings;
};

void TestSettings::cleanupTestCase()
{
    QCoreApplication::processEvents();
}

void TestSettings::testTestSettingsContentAll()
{
    FmBkupEngine *backupEngine = new FmBkupEngine( this );
    settings = new FmBackupSettings( backupEngine );
    QTime time = QTime::currentTime();
    settings->setContent(FmBackupSettings::EFileManagerBackupContentAll);
    settings->setScheduling(FmBackupSettings::EFileManagerBackupScheduleWeekly);
    settings->setWeekday(FmBackupSettings::EFileManagerBackupWeekdayMonday);
    settings->setTime(time);
    settings->setTargetDrive("C:");

    settings->save();
    delete settings;
    

    settings = new FmBackupSettings( backupEngine );
    settings->load();

    QVERIFY(settings->content() == FmBackupSettings::EFileManagerBackupContentAll);
    QVERIFY(settings->scheduling() == FmBackupSettings::EFileManagerBackupScheduleWeekly);
    QVERIFY(settings->weekday() == FmBackupSettings::EFileManagerBackupWeekdayMonday);
    QVERIFY( settings->time().toString() == time.toString() );
    QVERIFY(settings->targetDrive() == "C:");

    QList< FmBackupEntry* > backupEntryList = settings->backupEntryList();
    
    for( int i = 0; i< backupEntryList.count(); i++ )
    {
        FmBackupEntry* entry = backupEntryList[i];
        switch( entry->type() )
        {
        case FmBackupEntry::EContents:
            {
            QVERIFY( entry->title() == constFileManagerBackupSettingsTitleContents );
            break;
            }
        case FmBackupEntry::EScheduling:
            {
            QVERIFY( entry->title() == constFileManagerBackupSettingsTitleScheduling );
            break;
            }
        case FmBackupEntry::EWeekday:
            {
            QVERIFY( entry->title() == constFileManagerBackupSettingsTitleWeekday );
            break;
            }
        case FmBackupEntry::ETime:
            {
            QVERIFY( entry->title() == constFileManagerBackupSettingsTitleTime );
            break;
            }
        case FmBackupEntry::ETarget:
            {
            QVERIFY( entry->title() == constFileManagerBackupSettingsTitleTargetDrive );
            break;
            }

        }
        qDebug( entry->title().toUtf8().data() );
        qDebug( entry->tips().toUtf8().data() );
    }
    delete backupEngine;
}


void TestSettings::testTestSettingsContentOne()
{
    FmBkupEngine *backupEngine = new FmBkupEngine( this );
    settings = new FmBackupSettings( backupEngine );
    QTime time = QTime::currentTime();
    settings->setContent(FmBackupSettings::EFileManagerBackupContentSettings | 
        FmBackupSettings::EFileManagerBackupContentMessages);
    settings->setScheduling(FmBackupSettings::EFileManagerBackupScheduleWeekly);
    settings->setWeekday(FmBackupSettings::EFileManagerBackupWeekdayWednesday);
    settings->setTime(time);
    settings->setTargetDrive("E:");

    settings->save();
    delete settings;

    settings = new FmBackupSettings( backupEngine );
    settings->load();

    QVERIFY(settings->content() == quint32(FmBackupSettings::EFileManagerBackupContentSettings |
        FmBackupSettings::EFileManagerBackupContentMessages));
    QVERIFY(settings->scheduling() == FmBackupSettings::EFileManagerBackupScheduleWeekly);
    QVERIFY(settings->weekday() == FmBackupSettings::EFileManagerBackupWeekdayWednesday);
    QVERIFY( settings->time().toString() == time.toString() );
    QVERIFY(settings->targetDrive() == "E:");
    delete backupEngine;
}

void TestSettings::testTestSettingsContentTwo()
{
    FmBkupEngine *backupEngine = new FmBkupEngine( this );
    settings = new FmBackupSettings( backupEngine );
    QTime time = QTime::currentTime();
    settings->setContent(FmBackupSettings::EFileManagerBackupContentSettings | 
        FmBackupSettings::EFileManagerBackupContentMessages);
    settings->setScheduling(FmBackupSettings::EFileManagerBackupScheduleWeekly);
    settings->setWeekday(FmBackupSettings::EFileManagerBackupWeekdayWednesday);
    settings->setTime(time);
    settings->setTargetDrive("E:");

    settings->save();
    delete settings;

    settings = new FmBackupSettings( backupEngine );
    settings->load();

    QVERIFY(settings->content() == quint32( FmBackupSettings::EFileManagerBackupContentSettings |
        FmBackupSettings::EFileManagerBackupContentMessages ) );
    QVERIFY(settings->scheduling() == FmBackupSettings::EFileManagerBackupScheduleWeekly);
    QVERIFY(settings->weekday() == FmBackupSettings::EFileManagerBackupWeekdayWednesday);
    QVERIFY( settings->time().toString() == time.toString() );
    QVERIFY(settings->targetDrive() == "E:");
    delete backupEngine;
}

void TestSettings::testContentToString()
{
    QFETCH(quint32, param);
    QFETCH(QString, value);

    QCOMPARE(FmBackupSettings::contentToString( param ), value);
}

void TestSettings::testContentToString_data()
{
    QTest::addColumn<quint32>("param");
    QTest::addColumn<QString>("value");

    QTest::newRow("contentToString_0") << (quint32)FmBackupSettings::EFileManagerBackupContentAll << constFileManagerBackupSettingsContentAll;
    QTest::newRow("contentToString_1") << (quint32)FmBackupSettings::EFileManagerBackupContentSettings << constFileManagerBackupSettingsContentSettings;
    QTest::newRow("contentToString_2") << (quint32)FmBackupSettings::EFileManagerBackupContentMessages << constFileManagerBackupSettingsContentMessages;
    QTest::newRow("contentToString_3") << (quint32)FmBackupSettings::EFileManagerBackupContentContacts << constFileManagerBackupSettingsContentContacts;
    QTest::newRow("contentToString_4") << (quint32)FmBackupSettings::EFileManagerBackupContentCalendar << constFileManagerBackupSettingsContentCalendar;
    QTest::newRow("contentToString_5") << (quint32)FmBackupSettings::EFileManagerBackupContentBookmarks << constFileManagerBackupSettingsContentBookmarks;
    QTest::newRow("contentToString_6") << (quint32)FmBackupSettings::EFileManagerBackupContentUserFiles << constFileManagerBackupSettingsContentUserFiles;
}

void TestSettings::testSchedulingToString()
{
    QFETCH(int, param);
    QFETCH(QString, value);
    FmBkupEngine *backupEngine = new FmBkupEngine( this );
    settings = new FmBackupSettings( backupEngine );
    QCOMPARE(settings->schedulingToString( (FmBackupSettings::TFileManagerBackupSchedule)param ), value);
}

void TestSettings::testSchedulingToString_data()
{
    QTest::addColumn<int>("param");
    QTest::addColumn<QString>("value");

    QTest::newRow("schedulingToString_0") << (int)FmBackupSettings::EFileManagerBackupScheduleNever << constFileManagerBackupScheduleNever;
    QTest::newRow("schedulingToString_1") << (int)FmBackupSettings::EFileManagerBackupScheduleDaily << constFileManagerBackupScheduleDaily;
    QTest::newRow("schedulingToString_2") << (int)FmBackupSettings::EFileManagerBackupScheduleWeekly << constFileManagerBackupScheduleWeekly;
}

void TestSettings::testWeekdayToString()
{
    QFETCH(int, param);
    QFETCH(QString, value);

    QCOMPARE(FmBackupSettings::weekdayToString( (FmBackupSettings::TFileManagerBackupWeekday)param ), value);
}

void TestSettings::testWeekdayToString_data()
{

    QTest::addColumn<int>("param");
    QTest::addColumn<QString>("value");

    QTest::newRow("weekdayToString_0") << (int)FmBackupSettings::EFileManagerBackupWeekdayMonday << constFileManagerBackupWeekdayMonday;
    QTest::newRow("weekdayToString_1") << (int)FmBackupSettings::EFileManagerBackupWeekdayTuesday << constFileManagerBackupWeekdayTuesday;
    QTest::newRow("weekdayToString_2") << (int)FmBackupSettings::EFileManagerBackupWeekdayWednesday << constFileManagerBackupWeekdayWednesday;
    QTest::newRow("weekdayToString_3") << (int)FmBackupSettings::EFileManagerBackupWeekdayThursday << constFileManagerBackupWeekdayThursday;
    QTest::newRow("weekdayToString_4") << (int)FmBackupSettings::EFileManagerBackupWeekdayFriday << constFileManagerBackupWeekdayFirday;
    QTest::newRow("weekdayToString_5") << (int)FmBackupSettings::EFileManagerBackupWeekdaySaturday << constFileManagerBackupWeekdaySaturday;
    QTest::newRow("weekdayToString_6") << (int)FmBackupSettings::EFileManagerBackupWeekdaySunday << constFileManagerBackupWeekdaySunday;
}


void TestSettings::testFmBackupEntry()
{
    FmBackupEntry backupEntry( "title","tips", FmBackupEntry::EContents );
    QVERIFY( backupEntry.title() == "title" );
    QVERIFY( backupEntry.tips() == "tips" );
    QVERIFY( backupEntry.type() == FmBackupEntry::EContents );

    FmBackupEntry backupEntryCopy( backupEntry );
    QVERIFY( backupEntryCopy.title() == "title" );
    QVERIFY( backupEntryCopy.tips() == "tips" );
    QVERIFY( backupEntryCopy.type() == FmBackupEntry::EContents );

}

void TestSettings::testFmRestoreInfo()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    FmRestoreInfo restoreInfo((quint32)FmBackupSettings::EFileManagerBackupContentSettings, dateTime, "E:" );
    QVERIFY( restoreInfo.content() == (quint32)FmBackupSettings::EFileManagerBackupContentSettings );
    QVERIFY( restoreInfo.dateTime().toString() == dateTime.toString() );
    QVERIFY( restoreInfo.drive() == "E:" );

    FmRestoreInfo restoreInfoCopy( restoreInfo );
    QVERIFY( restoreInfoCopy.content() == restoreInfo.content() );
    QVERIFY( restoreInfoCopy.dateTime().toString() == restoreInfo.dateTime().toString() );
    QVERIFY( restoreInfoCopy.drive() == restoreInfo.drive() );

    FmRestoreInfo restoreInfoNext = restoreInfo;
    QVERIFY( restoreInfoNext.content() == restoreInfo.content() );
    QVERIFY( restoreInfoNext.dateTime().toString() == restoreInfo.dateTime().toString() );
    QVERIFY( restoreInfoNext.drive() == restoreInfo.drive() );
}

void TestSettings::testFmRestoreEntry()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    FmRestoreInfo restoreInfo((quint32)FmBackupSettings::EFileManagerBackupContentSettings, dateTime, "E:" );

    FmRestoreEntry restoreEntry( "displayText", restoreInfo );
    QVERIFY( restoreEntry.text() == "displayText");
    QVERIFY( restoreEntry.restoreInfo().content() == (quint32)FmBackupSettings::EFileManagerBackupContentSettings );
    QVERIFY( restoreEntry.restoreInfo().dateTime().toString() == dateTime.toString() );
    QVERIFY( restoreEntry.restoreInfo().drive() == "E:" );
}

QTEST_MAIN(TestSettings)
#include "unit_backuprestoresettings.moc"
