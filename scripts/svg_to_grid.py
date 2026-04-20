#!/usr/bin/env python3
"""
CREDIT: CLAUDE
"""

"""
svg_to_grid.py
Convert a road-map SVG (white-stroke paths) to an ASCII .grid file.

Each white-stroke path is rasterized onto an NxM grid:
  '.' = road cell
  '#' = wall/terrain cell

Roads are always rasterized exactly 1 cell wide (half_w = 0.5).
For clean 1-cell roads, design the SVG so road centerlines fall on
half-integer grid coordinates (e.g. x=100.5, y=250.5).  A cell at
(col, row) has its center at (col+0.5, row+0.5), so a centerline at
N+0.5 hits exactly one column/row of cells.

The SVG viewBox is normalized to the target grid dimensions so that
cell (col, row) maps exactly to SVG display point (col+0.5, row+0.5)
when the grid and SVG share the same canvas size.  For a true 1:1
mapping set the SVG viewBox to "0 0 <grid_w> <grid_h>".

Usage:
  python3 svg_to_grid.py <input.svg> <output.grid> [grid_w] [grid_h]

Defaults:
  grid_w = 1000, grid_h = 1000
"""

import math
import re
import sys
import xml.etree.ElementTree as ET

# ---------------------------------------------------------------------------
# SVG utilities
# ---------------------------------------------------------------------------

SVG_NS = re.compile(r"\{[^}]*\}")


def strip_ns(tag: str) -> str:
    return SVG_NS.sub("", tag)


def parse_viewbox(root) -> tuple[float, float, float, float] | None:
    """Return (min_x, min_y, width, height) from viewBox, or None."""
    vb = root.get("viewBox")
    if vb:
        parts = re.split(r"[\s,]+", vb.strip())
        if len(parts) == 4:
            return tuple(float(p) for p in parts)
    w = root.get("width")
    h = root.get("height")
    if w and h:
        try:
            return (0.0, 0.0, float(w), float(h))
        except ValueError:
            pass
    return None


def parse_matrix(transform: str) -> tuple[float, ...] | None:
    """
    Parse 'matrix(a,b,c,d,e,f)' → (a,b,c,d,e,f).
    Returns None if the transform is missing or not a matrix.
    """
    if not transform:
        return None
    m = re.search(
        r"matrix\(\s*"
        r"([-+]?[\d.eE+\-]+)\s*,\s*([-+]?[\d.eE+\-]+)\s*,\s*"
        r"([-+]?[\d.eE+\-]+)\s*,\s*([-+]?[\d.eE+\-]+)\s*,\s*"
        r"([-+]?[\d.eE+\-]+)\s*,\s*([-+]?[\d.eE+\-]+)\s*\)",
        transform,
    )
    if m:
        return tuple(float(m.group(i)) for i in range(1, 7))
    return None


def apply_matrix(mat: tuple[float, ...], x: float, y: float) -> tuple[float, float]:
    """Apply SVG matrix(a,b,c,d,e,f) to point (x,y)."""
    a, b, c, d, e, f = mat
    return a * x + c * y + e, b * x + d * y + f


def matrix_scale(mat: tuple[float, ...]) -> float:
    """Uniform scale factor of the matrix (geometric mean of x and y scales)."""
    a, b, c, d, _e, _f = mat
    sx = math.hypot(a, b)
    sy = math.hypot(c, d)
    return (sx + sy) / 2.0


# ---------------------------------------------------------------------------
# SVG path sampler (handles M/L/H/V/C/S/Q/Z)
# ---------------------------------------------------------------------------


def _sample_cubic(p0, p1, p2, p3, n: int = 12):
    pts = []
    for i in range(n + 1):
        t = i / n
        mt = 1 - t
        x = mt**3 * p0[0] + 3 * mt**2 * t * p1[0] + 3 * mt * t**2 * p2[0] + t**3 * p3[0]
        y = mt**3 * p0[1] + 3 * mt**2 * t * p1[1] + 3 * mt * t**2 * p2[1] + t**3 * p3[1]
        pts.append((x, y))
    return pts


def _sample_quadratic(p0, p1, p2, n: int = 8):
    pts = []
    for i in range(n + 1):
        t = i / n
        mt = 1 - t
        x = mt**2 * p0[0] + 2 * mt * t * p1[0] + t**2 * p2[0]
        y = mt**2 * p0[1] + 2 * mt * t * p1[1] + t**2 * p2[1]
        pts.append((x, y))
    return pts


def _tokenize(d: str):
    return re.findall(
        r"[MmLlHhVvCcSsQqZz]|[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?", d
    )


def sample_path(d: str, min_dist: float = 1.0) -> list[tuple[float, float]]:
    """Return sampled (x,y) points along the SVG path d-string."""
    tokens = _tokenize(d)
    raw: list[tuple[float, float]] = []
    cur = (0.0, 0.0)
    start = (0.0, 0.0)
    last_ctrl = None
    cmd = None
    i = 0

    def consume(n):
        nonlocal i
        vals = [float(tokens[i + j]) for j in range(n)]
        i += n
        return vals

    while i < len(tokens):
        tok = tokens[i]
        if tok.isalpha():
            cmd = tok
            i += 1
            if cmd not in ("S", "s", "C", "c"):
                last_ctrl = None
            continue

        if cmd in ("M", "m"):
            x, y = consume(2)
            if cmd == "m":
                x += cur[0]
                y += cur[1]
            cur = (x, y)
            start = cur
            raw.append(cur)
            cmd = "L" if cmd == "M" else "l"

        elif cmd in ("L", "l"):
            x, y = consume(2)
            if cmd == "l":
                x += cur[0]
                y += cur[1]
            cur = (x, y)
            raw.append(cur)

        elif cmd in ("H", "h"):
            x = consume(1)[0]
            if cmd == "h":
                x += cur[0]
            cur = (x, cur[1])
            raw.append(cur)

        elif cmd in ("V", "v"):
            y = consume(1)[0]
            if cmd == "v":
                y += cur[1]
            cur = (cur[0], y)
            raw.append(cur)

        elif cmd in ("C", "c"):
            x1, y1, x2, y2, x, y = consume(6)
            if cmd == "c":
                x1 += cur[0]
                y1 += cur[1]
                x2 += cur[0]
                y2 += cur[1]
                x += cur[0]
                y += cur[1]
            raw.extend(_sample_cubic(cur, (x1, y1), (x2, y2), (x, y))[1:])
            last_ctrl = (x2, y2)
            cur = (x, y)

        elif cmd in ("S", "s"):
            x2, y2, x, y = consume(4)
            if cmd == "s":
                x2 += cur[0]
                y2 += cur[1]
                x += cur[0]
                y += cur[1]
            x1 = 2 * cur[0] - last_ctrl[0] if last_ctrl else cur[0]
            y1 = 2 * cur[1] - last_ctrl[1] if last_ctrl else cur[1]
            raw.extend(_sample_cubic(cur, (x1, y1), (x2, y2), (x, y))[1:])
            last_ctrl = (x2, y2)
            cur = (x, y)

        elif cmd in ("Q", "q"):
            x1, y1, x, y = consume(4)
            if cmd == "q":
                x1 += cur[0]
                y1 += cur[1]
                x += cur[0]
                y += cur[1]
            raw.extend(_sample_quadratic(cur, (x1, y1), (x, y))[1:])
            last_ctrl = (x1, y1)
            cur = (x, y)

        elif cmd in ("Z", "z"):
            cur = start
            raw.append(cur)
        else:
            i += 1

    if not raw:
        return []

    # Downsample: keep points at least min_dist apart
    out = [raw[0]]
    for p in raw[1:]:
        dx = p[0] - out[-1][0]
        dy = p[1] - out[-1][1]
        if dx * dx + dy * dy >= min_dist * min_dist:
            out.append(p)
    return out


# ---------------------------------------------------------------------------
# Rasterization
# ---------------------------------------------------------------------------


def rasterize_segment(
    grid: list[bytearray],
    p1: tuple[float, float],
    p2: tuple[float, float],
    half_w: float,
    grid_w: int,
    grid_h: int,
) -> None:
    """Mark all cells whose center lies within half_w of segment p1→p2 as road."""
    x1, y1 = p1
    x2, y2 = p2
    dx = x2 - x1
    dy = y2 - y1
    seg_len_sq = dx * dx + dy * dy
    hw_sq = half_w * half_w

    col_min = max(0, int(math.floor(min(x1, x2) - half_w)))
    col_max = min(grid_w - 1, int(math.ceil(max(x1, x2) + half_w)))
    row_min = max(0, int(math.floor(min(y1, y2) - half_w)))
    row_max = min(grid_h - 1, int(math.ceil(max(y1, y2) + half_w)))

    for row in range(row_min, row_max + 1):
        cy = row + 0.5
        for col in range(col_min, col_max + 1):
            cx = col + 0.5
            if seg_len_sq < 1e-12:
                dist_sq = (cx - x1) ** 2 + (cy - y1) ** 2
            else:
                t = ((cx - x1) * dx + (cy - y1) * dy) / seg_len_sq
                t = max(0.0, min(1.0, t))
                px = x1 + t * dx
                py = y1 + t * dy
                dist_sq = (cx - px) ** 2 + (cy - py) ** 2
            if dist_sq <= hw_sq:
                grid[row][col] = ord(".")


# ---------------------------------------------------------------------------
# Main conversion
# ---------------------------------------------------------------------------

ROAD_STROKE = "#ffffff"


def svg_to_grid(
    svg_path: str,
    grid_path: str,
    grid_w: int = 1000,
    grid_h: int = 1000,
) -> None:
    tree = ET.parse(svg_path)
    root = tree.getroot()

    vb = parse_viewbox(root)
    if vb is None:
        print(
            "warning: no viewBox found, using width/height as canvas", file=sys.stderr
        )
        vb = (
            0.0,
            0.0,
            float(root.get("width", grid_w)),
            float(root.get("height", grid_h)),
        )

    vb_x, vb_y, vb_w, vb_h = vb
    scale_x = grid_w / vb_w
    scale_y = grid_h / vb_h
    print(
        f"viewBox={vb}  →  grid {grid_w}×{grid_h}  scale=({scale_x:.4f}, {scale_y:.4f})"
    )

    # Grid: bytearray of '#' per row (fast to write)
    grid = [bytearray(b"#" * grid_w) for _ in range(grid_h)]

    road_paths = 0
    segments_rasterized = 0

    for elem in root.iter():
        if strip_ns(elem.tag) != "path":
            continue

        # Accept both attribute and inline-style stroke
        stroke = elem.get("stroke") or ""
        style = elem.get("style") or ""
        if "stroke:#ffffff" in style.replace(" ", "").lower():
            stroke = "#ffffff"
        if stroke.lower() != ROAD_STROKE:
            continue

        d = elem.get("d") or ""
        if not d:
            continue

        # Parse optional matrix transform on this element
        mat = parse_matrix(elem.get("transform") or "")

        # Always 1 cell wide: mark only the cell whose center is nearest the
        # segment centerline.  Requires road centerlines at half-integer grid
        # coordinates (N+0.5) for clean, non-overlapping 1-cell roads.
        half_w = 0.5

        # Sample the path in local coordinates
        pts_local = sample_path(d, min_dist=0.5)
        if len(pts_local) < 2:
            continue

        # Transform local → viewBox → grid
        if mat is not None:
            pts_vb = [apply_matrix(mat, x, y) for x, y in pts_local]
        else:
            pts_vb = pts_local

        pts_grid = [((x - vb_x) * scale_x, (y - vb_y) * scale_y) for x, y in pts_vb]

        # Rasterize each segment
        for i in range(len(pts_grid) - 1):
            rasterize_segment(
                grid, pts_grid[i], pts_grid[i + 1], half_w, grid_w, grid_h
            )
            segments_rasterized += 1

        road_paths += 1

    road_cells = sum(row.count(ord(".")) for row in grid)
    print(
        f"rasterized {road_paths} road paths, {segments_rasterized} segments, "
        f"{road_cells} road cells ({100 * road_cells / (grid_w * grid_h):.1f}% coverage)"
    )

    with open(grid_path, "w") as f:
        for row in grid:
            f.write(row.decode("ascii") + "\n")

    print(f"wrote {grid_path}  ({grid_w}×{grid_h})")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <input.svg> <output.grid> [grid_w] [grid_h]")
        sys.exit(1)

    svg_in = sys.argv[1]
    grid_out = sys.argv[2]
    w = int(sys.argv[3]) if len(sys.argv) > 3 else 1000
    h = int(sys.argv[4]) if len(sys.argv) > 4 else 1000

    svg_to_grid(svg_in, grid_out, w, h)
