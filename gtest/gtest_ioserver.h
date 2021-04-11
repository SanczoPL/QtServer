#include "gtest/gtest.h"
#include "gmock/gmock.h" 
using ::testing::AtLeast;


class Test_IOServer : public ::testing::Test {
protected:
	Test_IOServer(){}

	~Test_IOServer() override {}

	void SetUp() override{}

	void TearDown() override 
	{
	}
};

