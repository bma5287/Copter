/*
 * Copyright (C) 2016  Intel Corporation. All rights reserved.
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <AP_gbenchmark.h>

#include <AP_Math/AP_GeodesicGrid.h>

static AP_GeodesicGrid grid;

static void BM_GeodesicGridSections(benchmark::State& state)
{
    Vector3f v, a, b, c;
    int section = state.range_x();

    grid.section_triangle(section, a, b, c);
    v = (a + b + c) / 3.0f;

    while (state.KeepRunning()) {
        int s = grid.section(v);
        gbenchmark_escape(&s);
    }
}

/* Benchmark each section */
BENCHMARK(BM_GeodesicGridSections)->DenseRange(0, 79);

BENCHMARK_MAIN()
