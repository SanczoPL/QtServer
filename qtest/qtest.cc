#include "qtest.h"
#include <QTest>
#include <QSignalSpy>

void Test_qtest::test_1()
{
	Logger->set_level(static_cast<spdlog::level::level_enum>(0));
}
void Test_qtest::waitForSignal(int timeout, QSignalSpy& spy, int deltaTime, int signalToCatch)
{
	int time{ 0 };
	while (spy.count() != signalToCatch)
	{
		time += deltaTime;
		QTest::qWait(deltaTime);
		if (time >= timeout)
		{
			break;
		}
	}
}


void Test_qtest::test_2()
{
	Logger->set_level(static_cast<spdlog::level::level_enum>(0));
}

QTEST_MAIN(Test_qtest)