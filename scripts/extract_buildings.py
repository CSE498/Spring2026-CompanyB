#!/usr/bin/env python3

"""
CREDIT: CLAUDE
"""

"""
Extract building bounding boxes from full_map.svg.

Finds all <path> elements with fill="#c8bfb0" (the building tan color),
parses every numeric coordinate in `d`, applies the `transform="matrix(...)"`,
and computes axis-aligned bounding boxes. Then maps SVG viewBox space
(750x750) → output grid space (default 1000x1000).

Output: one line per building of the form:
    DrawBuilding(grid, wall, floor, x1, y1, x2, y2, {});
suitable for pasting into web_main.cpp's SetupVirusWorld.

Usage:
    python3 scripts/extract_buildings.py [grid_w] [grid_h]
Defaults: 1000 x 1000.
"""

import re
import sys
import xml.etree.ElementTree as ET

SVG_PATH = "assets/images/full_map.svg"
BUILDING_FILL = "#ffffff"  # placeholder, set below
TARGET_FILL = "#c8bfb0"

NUM_RE = re.compile(r"-?\d+\.?\d*(?:[eE][-+]?\d+)?")


def parse_matrix(transform: str):
    """Parse `matrix(a, b, c, d, e, f)` → 6-tuple."""
    if not transform or "matrix" not in transform:
        return (1.0, 0.0, 0.0, 1.0, 0.0, 0.0)
    nums = [float(n) for n in NUM_RE.findall(transform)]
    if len(nums) < 6:
        return (1.0, 0.0, 0.0, 1.0, 0.0, 0.0)
    return tuple(nums[:6])


def apply_matrix(m, x, y):
    a, b, c, d, e, f = m
    return (a * x + c * y + e, b * x + d * y + f)


def path_bbox(d_attr: str, matrix):
    """Extract every coordinate pair from `d`, transform, and bbox them.

    Treats every consecutive pair of numbers as (x, y). This is approximate
    — it ignores path command semantics (e.g. relative moves) — but for
    bounding boxes of mostly-absolute building outlines it gives a
    conservative and useful estimate.
    """
    nums = [float(n) for n in NUM_RE.findall(d_attr)]
    if len(nums) < 2:
        return None
    minx = miny = float("inf")
    maxx = maxy = float("-inf")
    for i in range(0, len(nums) - 1, 2):
        sx, sy = nums[i], nums[i + 1]
        x, y = apply_matrix(matrix, sx, sy)
        minx = min(minx, x)
        miny = min(miny, y)
        maxx = max(maxx, x)
        maxy = max(maxy, y)
    return (minx, miny, maxx, maxy)


def main():
    grid_w = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
    grid_h = int(sys.argv[2]) if len(sys.argv) > 2 else 1000

    tree = ET.parse(SVG_PATH)
    root = tree.getroot()

    # Read viewBox to normalize coordinates.
    vb = root.attrib.get("viewBox", "0 0 750 750").split()
    vb_w = float(vb[2])
    vb_h = float(vb[3])
    sx = grid_w / vb_w
    sy = grid_h / vb_h

    ns = {"svg": "http://www.w3.org/2000/svg"}
    # Allow both namespaced and unnamespaced lookups.
    paths = root.iter("{http://www.w3.org/2000/svg}path")

    bboxes = []
    for p in paths:
        if p.attrib.get("fill", "").lower() != TARGET_FILL:
            continue
        d = p.attrib.get("d", "")
        m = parse_matrix(p.attrib.get("transform", ""))
        bb = path_bbox(d, m)
        if bb is None:
            continue
        x1, y1, x2, y2 = bb
        # Map SVG → grid
        gx1 = max(0, min(grid_w - 1, int(round(x1 * sx))))
        gy1 = max(0, min(grid_h - 1, int(round(y1 * sy))))
        gx2 = max(0, min(grid_w - 1, int(round(x2 * sx))))
        gy2 = max(0, min(grid_h - 1, int(round(y2 * sy))))
        if gx2 <= gx1 or gy2 <= gy1:
            continue
        # Filter tiny artifacts (< 4x4 cells in 1000x1000 space).
        if (gx2 - gx1) < 4 or (gy2 - gy1) < 4:
            continue
        bboxes.append((gx1, gy1, gx2, gy2))

    # Drop boxes fully contained inside a larger box (nested artifacts).
    def contains(big, small):
        return (
            big[0] <= small[0]
            and big[1] <= small[1]
            and big[2] >= small[2]
            and big[3] >= small[3]
            and big != small
        )

    filtered = [b for b in bboxes if not any(contains(other, b) for other in bboxes)]

    # Drop boxes that overlap any other box; keep the larger of each pair.
    # This guarantees the final list is pairwise disjoint, so the renderer
    # doesn't need to clip walls against neighbors at draw time.
    def overlaps(a, b):
        return not (a[2] <= b[0] or b[2] <= a[0] or a[3] <= b[1] or b[3] <= a[1])

    def area(b):
        return (b[2] - b[0]) * (b[3] - b[1])

    changed = True
    while changed:
        changed = False
        n = len(filtered)
        drop = -1
        for i in range(n):
            for j in range(i + 1, n):
                if overlaps(filtered[i], filtered[j]):
                    drop = i if area(filtered[i]) < area(filtered[j]) else j
                    break
            if drop != -1:
                break
        if drop != -1:
            filtered.pop(drop)
            changed = True

    # Sort top-to-bottom, left-to-right.
    filtered.sort(key=lambda b: (b[1], b[0]))
    bboxes = filtered

    out_path = "source/Worlds/InfectiousBuildings.hpp"
    with open(out_path, "w") as f:
        f.write("/**\n")
        f.write(" * @file InfectiousBuildings.hpp\n")
        f.write(
            f" * @brief AUTO-GENERATED from {SVG_PATH} by scripts/extract_buildings.py.\n"
        )
        f.write(
            " * Building bounding boxes (x1, y1, x2, y2) for the campus virus sim.\n"
        )
        f.write(" **/\n\n")
        f.write("#pragma once\n\n")
        f.write("#include <array>\n#include <cstddef>\n\n")
        f.write("namespace cse498::infectious_buildings {\n\n")
        f.write(f"struct BuildingBox {{ std::size_t x1, y1, x2, y2; }};\n\n")
        f.write(
            f"inline constexpr std::array<BuildingBox, {len(bboxes)}> kBuildings = {{{{\n"
        )
        for x1, y1, x2, y2 in bboxes:
            f.write(f"    {{ {x1}, {y1}, {x2}, {y2} }},\n")
        f.write("}};\n\n")
        f.write("}  // namespace cse498::infectious_buildings\n")
    print(f"Wrote {len(bboxes)} buildings to {out_path}")


if __name__ == "__main__":
    main()
