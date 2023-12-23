#pragma once

#include <QtTest/QtTest>

class GridTest : public QObject {
	Q_OBJECT
private slots:
	static void test1Match();
	static void test2Match();
	static void test3Match();
	static void testHasSolution();

public:
	GridTest() = default;
};
