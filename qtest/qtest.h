#ifndef Q_TEST_H
#define Q_TEST_H

#include <QTest>
#include <QSignalSpy>
#include "configreader.h"


class Test_qtest : public QObject {
  Q_OBJECT

private:
	void waitForSignal(int timeout, QSignalSpy& spy, int deltaTime, int signalToCatch);
 private slots:
  void test_1();
  void test_2();
};

#endif // Q_TEST_H
