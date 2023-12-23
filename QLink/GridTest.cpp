#include "GridTest.hpp"

#include "game/Grid.hpp"

void GridTest::test1Match() {
	Grid grid;
	grid.Initialize(3, 3);
	grid.Set({1, 1}, 1);
	grid.Set({1, 3}, 1);
	QVERIFY(grid.IsLinked({1, 1}, {1, 3}, nullptr));
	QVERIFY(!grid.IsLinked({1, 2}, {1, 3}, nullptr));
	QVERIFY(!grid.IsLinked({1, 0}, {1, 3}, nullptr));
}

void GridTest::test2Match() {
	Grid grid;
	grid.Initialize(3, 3);
	grid.Set({1, 1}, 1);
	grid.Set({2, 3}, 1);
	QVERIFY(grid.IsLinked({1, 1}, {2, 3}, nullptr));
	QVERIFY(!grid.IsLinked({1, 2}, {2, 3}, nullptr));
	QVERIFY(!grid.IsLinked({1, 0}, {2, 3}, nullptr));
}

void GridTest::test3Match() {
	Grid grid;
	grid.Initialize(3, 3);
	grid.Set({1, 1}, 1);
	grid.Set({1, 2}, 2);
	grid.Set({1, 3}, 1);
	QVERIFY(grid.IsLinked({1, 1}, {1, 3}, nullptr));
	QVERIFY(!grid.IsLinked({1, 2}, {1, 3}, nullptr));
	QVERIFY(!grid.IsLinked({1, 0}, {1, 3}, nullptr));
}

void GridTest::testHasSolution() {
	Grid grid;

	grid.Initialize(3, 3);
	QVERIFY(!grid.HasSolution(nullptr));

	grid.Set({1, 1}, 1);
	grid.Set({1, 2}, 2);
	grid.Set({1, 3}, 1);
	QVERIFY(grid.HasSolution(nullptr));

	grid.Initialize(2, 2);
	grid.Set({1, 1}, 1);
	grid.Set({1, 2}, 2);
	grid.Set({2, 1}, 2);
	grid.Set({2, 2}, 1);
	QVERIFY(!grid.HasSolution(nullptr));
}

QTEST_MAIN(GridTest)
