"""Route measured single-target Simulink connections without changing a model.

Usage: python route.py route_input.json routes.json

The pure intersection/envelope functions are adapted from the existing
SOGI_discrete_test/route_sogi_port_layout.py; its main is never imported or run.
This tool only writes JSON. Each horizontal segment moves right, all segments
are orthogonal, and unrelated parallel segments retain --clearance units.
Actual PortConnectivity positions are mandatory. No input-arrow offset is
guessed. Signal labels with no measured height/bounds receive a conservative
planned envelope and remain marked for manual verification.
"""
from __future__ import annotations

import argparse
import heapq
import json
import math
import sys
from functools import lru_cache
from pathlib import Path


def as_list(value):
    return [value] if isinstance(value, dict) else value or []


def segments(points):
    return list(zip(points, points[1:]))


def simplify(points):
    result = []
    for value in points:
        value = tuple(value)
        if result and value == result[-1]:
            continue
        if len(result) > 1 and (
            result[-2][0] == result[-1][0] == value[0]
            or result[-2][1] == result[-1][1] == value[1]
        ):
            result.pop()
        result.append(value)
    return result


def crosses(a, b, c, d):
    """Closed intersection: unrelated touching endpoints and false T count."""
    if a[1] == b[1] and c[1] == d[1]:
        return a[1] == c[1] and max(min(a[0], b[0]), min(c[0], d[0])) <= min(
            max(a[0], b[0]), max(c[0], d[0])
        )
    if a[0] == b[0] and c[0] == d[0]:
        return a[0] == c[0] and max(min(a[1], b[1]), min(c[1], d[1])) <= min(
            max(a[1], b[1]), max(c[1], d[1])
        )
    if a[0] == b[0]:
        a, b, c, d = c, d, a, b
    return min(a[0], b[0]) <= c[0] <= max(a[0], b[0]) and min(c[1], d[1]) <= a[1] <= max(c[1], d[1])


def hit_rect(a, b, rect):
    """Test open rectangle interior; caller supplies content padding."""
    x1, y1, x2, y2 = rect
    if a[0] == b[0]:
        return x1 < a[0] < x2 and max(a[1], b[1]) > y1 and min(a[1], b[1]) < y2
    if a[1] == b[1]:
        return y1 < a[1] < y2 and max(a[0], b[0]) > x1 and min(a[0], b[0]) < x2
    return True


def crowded_parallel(a, b, c, d, clearance=20):
    if a[1] == b[1] and c[1] == d[1]:
        overlap = min(max(a[0], b[0]), max(c[0], d[0])) - max(min(a[0], b[0]), min(c[0], d[0]))
        return overlap > 0 and abs(a[1] - c[1]) < clearance
    if a[0] == b[0] and c[0] == d[0]:
        overlap = min(max(a[1], b[1]), max(c[1], d[1])) - max(min(a[1], b[1]), min(c[1], d[1]))
        return overlap > 0 and abs(a[0] - c[0]) < clearance
    return False


def name_rect(block, pad=4):
    x1, y1, x2, y2 = block["position"]
    center = (x1 + x2) / 2
    width, height = block["textWidth"], block["textHeight"]
    y = y1 - 2 - height if block.get("namePlacement") == "alternate" else y2 + 2
    return [center - width / 2 - pad, y - pad, center + width / 2 + pad, y + height + pad]


def inflate(rect, pad):
    return [rect[0] - pad, rect[1] - pad, rect[2] + pad, rect[3] + pad]


def rect_overlap(a, b):
    return max(a[0], b[0]) < min(a[2], b[2]) and max(a[1], b[1]) < min(a[3], b[3])


def finite_vector(value, count, label):
    if not isinstance(value, (list, tuple)) or len(value) != count or not all(
        isinstance(v, (int, float)) and math.isfinite(v) for v in value
    ):
        raise ValueError(f"{label}: expected {count} finite numeric coordinates, got {value!r}")
    return tuple(value)


class RoutingFailure(RuntimeError):
    def __init__(self, message, details=None):
        super().__init__(message)
        self.details = details or {}


def first_conflicts(a, b, obstacles, occupied, clearance):
    conflicts = []
    for obstacle in obstacles:
        if hit_rect(a, b, obstacle["rect"]):
            conflicts.append({"kind": "content", "label": obstacle["label"], "rect": obstacle["rect"]})
    for wire in occupied:
        c, d = wire["segment"]
        if crosses(a, b, c, d):
            conflicts.append({"kind": "wire_intersection", "label": wire["label"], "segment": [c, d]})
        elif crowded_parallel(a, b, c, d, clearance):
            conflicts.append({"kind": "parallel_clearance", "label": wire["label"], "segment": [c, d], "required": clearance})
    return conflicts


def route(source, target, obstacles, occupied, first_run=10, clearance=20, last_run=10):
    """A* on an orthogonal visibility grid; east/up/down moves only.

    The source and destination stubs are explicit, making label length and
    final horizontal port entry constraints independent of path simplification.
    Rectangle edges, free channels and preceding wire offsets define events.
    """
    if target[0] <= source[0]:
        raise RoutingFailure("Destination must be strictly right of source", {"source": source, "target": target})
    if target[0] - source[0] < first_run + last_run:
        raise RoutingFailure("Insufficient horizontal space for the named first segment and final port stub", {
            "source": source, "target": target, "available": target[0] - source[0],
            "requiredFirstRun": first_run, "requiredLastRun": last_run,
            "suggestion": "Move the destination column right or shorten the reviewed signal display label.",
        })

    def clear(a, b):
        return not first_conflicts(a, b, obstacles, occupied, clearance)

    # A direct horizontal line is preferable and has no artificial bend.
    if source[1] == target[1] and clear(source, target):
        return [source, target]

    start = (source[0] + first_run, source[1])
    finish = (target[0] - last_run, target[1])
    for label, a, b in (("source", source, start), ("destination", finish, target)):
        conflicts = first_conflicts(a, b, obstacles, occupied, clearance)
        if conflicts:
            raise RoutingFailure(f"The mandatory {label} horizontal stub is blocked", {
                "stub": [a, b], "conflicts": conflicts,
                "suggestion": "Increase column/row spacing around the measured port or label.",
            })

    xs = {start[0], finish[0]}
    ys = {source[1], target[1]}
    for obstacle in obstacles:
        x1, y1, x2, y2 = obstacle["rect"]
        xs.update((x1 - clearance, x1, x2, x2 + clearance))
        ys.update((y1 - clearance, y1, y2, y2 + clearance))
    for wire in occupied:
        for x, y in wire["segment"]:
            xs.update((x - clearance, x, x + clearance))
            ys.update((y - clearance, y, y + clearance))
    # Endpoint-relative channels handle non-grid port coordinates exactly.
    for x, y in (source, target):
        xs.update((x - clearance, x + clearance))
        ys.update((y - 4 * clearance, y - clearance, y + clearance, y + 4 * clearance))
    y_min, y_max = min(ys), max(ys)
    ys.update((y_min - 2 * clearance, y_max + 2 * clearance))
    x_values = sorted(x for x in xs if start[0] <= x <= finish[0])
    y_values = sorted(ys)
    x_index = {x: i for i, x in enumerate(x_values)}
    y_index = {y: i for i, y in enumerate(y_values)}
    initial = (x_index[start[0]], y_index[start[1]], "E")
    goal_xy = (x_index[finish[0]], y_index[finish[1]])

    @lru_cache(maxsize=None)
    def edge_clear(ix, iy, jx, jy):
        return clear((x_values[ix], y_values[iy]), (x_values[jx], y_values[jy]))

    def heuristic(ix, iy):
        return finish[0] - x_values[ix] + abs(finish[1] - y_values[iy])

    queue = [(heuristic(initial[0], initial[1]), 0.0, initial)]
    costs = {initial: 0.0}
    previous = {}
    goal = None
    expanded = 0
    while queue:
        _, cost, node = heapq.heappop(queue)
        if cost != costs.get(node):
            continue
        ix, iy, direction = node
        expanded += 1
        if (ix, iy) == goal_xy:
            goal = node
            break
        choices = ((ix + 1, iy, "E"), (ix, iy - 1, "N"), (ix, iy + 1, "S"))
        for jx, jy, next_direction in choices:
            if not (0 <= jx < len(x_values) and 0 <= jy < len(y_values)):
                continue
            if not edge_clear(ix, iy, jx, jy):
                continue
            length = abs(x_values[jx] - x_values[ix]) + abs(y_values[jy] - y_values[iy])
            bend_cost = 40 if direction != next_direction else 0
            next_cost = cost + length + bend_cost
            next_node = (jx, jy, next_direction)
            if next_cost < costs.get(next_node, math.inf):
                costs[next_node] = next_cost
                previous[next_node] = node
                heapq.heappush(queue, (next_cost + heuristic(jx, jy), next_cost, next_node))
    if goal is None:
        raise RoutingFailure("No clear rightward orthogonal path exists in the measured visibility channels", {
            "source": source, "target": target, "nodesExpanded": expanded,
            "gridColumns": len(x_values), "gridRows": len(y_values),
            "obstacles": obstacles, "occupiedWireSegments": occupied,
            "suggestion": "Inspect reported envelopes; move producers/consumers or separate independent rows. Do not permit diagonal or leftward detours.",
        })
    path = []
    while True:
        path.append((x_values[goal[0]], y_values[goal[1]]))
        if goal == initial:
            break
        goal = previous[goal]
    return simplify([source] + list(reversed(path)) + [target])


def read_port(block, side, number):
    ports = as_list(block.get("ports", {}).get(side))
    if not isinstance(number, (int, float)) or int(number) != number or not 1 <= number <= len(ports):
        raise ValueError(f"{block['name']} {side}: invalid port index {number}, available {len(ports)}")
    port = ports[int(number) - 1]
    if "connectionPosition" not in port:
        raise ValueError(f"{block['name']} {side} {number}: actual connectionPosition is required")
    return finite_vector(port["connectionPosition"], 2, f"{block['name']} {side} {number}")


def prepare_scope(scope, label_height):
    blocks = {str(b["sid"]): b for b in as_list(scope["blocks"])}
    if len(blocks) != len(as_list(scope["blocks"])):
        raise ValueError(f"{scope['path']}: duplicate block sid")
    content = []
    for sid, block in blocks.items():
        rect = finite_vector(block["position"], 4, block["name"])
        if rect[2] <= rect[0] or rect[3] <= rect[1]:
            raise ValueError(f"{block['name']}: nonpositive block dimensions {rect}")
        content.append({"kind": "body", "sid": sid, "label": block["name"] + " body", "rect": inflate(rect, 4)})
        if block.get("showName", "on") == "on":
            for key in ("textWidth", "textHeight"):
                if key not in block or not math.isfinite(block[key]) or block[key] < 0:
                    raise ValueError(f"{block['name']}: measured {key} is required for full-name routing")
            content.append({"kind": "block_name", "sid": sid, "label": block["name"] + " full name", "rect": name_rect(block)})
    for number, annotation in enumerate(as_list(scope.get("annotations"))):
        content.append({"kind": "annotation", "label": annotation.get("text", f"annotation {number + 1}"),
                        "rect": inflate(finite_vector(annotation["position"], 4, "annotation bounds"), 4)})
    pending = []
    source_endpoints, destination_endpoints = set(), set()
    for number, line in enumerate(as_list(scope.get("lines"))):
        source_sid, target_sid = str(line["sourceSid"]), str(line["destinationSid"])
        if source_sid not in blocks or target_sid not in blocks:
            raise ValueError(f"Line {number}: source or destination sid is absent")
        source_key = (source_sid, line["sourcePort"])
        target_key = (target_sid, line["destinationPort"])
        if source_key in source_endpoints or target_key in destination_endpoints:
            raise ValueError(f"{scope['path']}: branching/shared endpoints are outside this single-source single-destination tool contract: line {number}")
        source_endpoints.add(source_key)
        destination_endpoints.add(target_key)
        source = read_port(blocks[source_sid], "Outport", line["sourcePort"])
        target = read_port(blocks[target_sid], "Inport", line["destinationPort"])
        name = line.get("name", "")
        width = line.get("labelWidth", 0)
        if name and (not isinstance(width, (int, float)) or not math.isfinite(width) or width <= 0):
            raise ValueError(f"{scope['path']}: named line {name!r} requires a positive measured labelWidth")
        label_rect = None
        label_measured = False
        if name:
            height = line.get("labelHeight", label_height)
            if not isinstance(height, (int, float)) or not math.isfinite(height) or height <= 0:
                raise ValueError(f"{name}: labelHeight must be finite and positive")
            label_rect = [source[0] + 10, source[1] - height - 5, source[0] + 10 + width, source[1] - 5]
            # The output is a planned location. Even measured old labelBounds
            # do not prove a newly routed line will retain those actual bounds.
            label_measured = "labelHeight" in line
        pending.append({"line": line, "source": source, "target": target, "order": number,
                        "sourceSid": source_sid, "targetSid": target_sid,
                        "label": f"{blocks[source_sid]['name']}.y{line['sourcePort']} -> {blocks[target_sid]['name']}.u{line['destinationPort']}",
                        "labelRect": label_rect, "labelHeightMeasured": label_measured,
                        "firstRun": width + 20 if name else 10})
    return blocks, content, pending


def route_scope(scope, clearance=20, label_height=24):
    _, content, pending = prepare_scope(scope, label_height)
    orders = [
        ("short_local_first", sorted(pending, key=lambda p: (abs(p["target"][0] - p["source"][0]) + abs(p["target"][1] - p["source"][1]), p["target"][1]))),
        ("named_constrained_first", sorted(pending, key=lambda p: (not bool(p["line"].get("name")), p["target"][0] - p["source"][0] - p["firstRun"], p["target"][1]))),
        ("source_row_first", sorted(pending, key=lambda p: (p["source"][1], p["target"][1], p["source"][0]))),
    ]
    attempts = []
    for order_name, order in orders:
        occupied, label_obstacles, done = [], [], []
        try:
            for item in order:
                obstacles = [r for r in content if not (
                    r["kind"] == "body" and r.get("sid") in (item["sourceSid"], item["targetSid"])
                )] + label_obstacles
                own_label = item["labelRect"]
                if own_label:
                    conflicts = [{"kind": "label_content_overlap", "content": r} for r in content + label_obstacles if rect_overlap(own_label, r["rect"])]
                    conflicts += [{"kind": "label_wire_overlap", "wire": wire} for wire in occupied if hit_rect(*wire["segment"], own_label)]
                    if conflicts:
                        raise RoutingFailure("The proposed first-segment signal label overlaps measured content", {
                            "labelBounds": own_label, "conflicts": conflicts,
                            "suggestion": "Increase source-column clearance or place this label on a separately reviewed segment.",
                        })
                    obstacles = obstacles + [{"kind": "own_signal_label", "label": item["line"]["name"], "rect": own_label}]
                points = route(item["source"], item["target"], obstacles, occupied, item["firstRun"], clearance)
                for a, b in segments(points):
                    if not (a[0] == b[0] or a[1] == b[1]) or b[0] < a[0]:
                        raise AssertionError("Router produced a diagonal or leftward segment")
                occupied.extend({"label": item["label"], "segment": [a, b]} for a, b in segments(points))
                if own_label:
                    label_obstacles.append({"kind": "signal_label", "label": item["line"]["name"], "rect": own_label})
                routed = dict(item["line"], points=points,
                              firstHorizontalRun=points[1][0] - points[0][0],
                              requiredFirstHorizontalRun=item["firstRun"],
                              signalLabelPlannedBounds=own_label,
                              signalLabelHeightMeasured=item["labelHeightMeasured"],
                              signalLabelActualBoundsStatus="NOT_RUN" if own_label else "NOT_APPLICABLE",
                              signalLabelManualReviewRequired=bool(own_label))
                done.append((item["order"], routed))
            return {"scope": scope["path"], "routingOrder": order_name, "clearance": clearance,
                    "lines": [line for _, line in sorted(done)]}
        except RoutingFailure as problem:
            attempts.append({"routingOrder": order_name, "line": item["label"],
                             "source": item["source"], "target": item["target"],
                             "routedBeforeFailure": len(done), "message": str(problem), "details": problem.details})
    raise RoutingFailure(f"{scope['path']}: routing failed in all {len(orders)} orderings", {"scope": scope["path"], "attempts": attempts})


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="Measured route_input.json")
    parser.add_argument("output", type=Path, help="Output routes.json (same list-of-scopes form as the old tool)")
    parser.add_argument("--clearance", type=float, default=20, help="Minimum unrelated parallel-wire spacing (default 20)")
    parser.add_argument("--label-height", type=float, default=24, help="Planned label height when measurement is missing; always requires manual review")
    args = parser.parse_args(argv)
    if not math.isfinite(args.clearance) or args.clearance < 20:
        parser.error("--clearance must be finite and at least 20")
    if not math.isfinite(args.label_height) or args.label_height <= 0:
        parser.error("--label-height must be finite and positive")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    data = json.loads(args.input.read_text(encoding="utf-8-sig"))
    results = []
    try:
        for scope in as_list(data["scopes"]):
            result = route_scope(scope, args.clearance, args.label_height)
            results.append(result)
            print(f"{scope['path']}: routed {len(result['lines'])} nets ({result['routingOrder']})")
    except (RoutingFailure, ValueError, KeyError) as problem:
        failure_path = args.output.with_name(args.output.stem + ".failure.json")
        failure = {"status": "FAIL", "input": str(args.input.resolve()),
                   "message": str(problem), "details": getattr(problem, "details", {}),
                   "successfulScopes": [r["scope"] for r in results],
                   "outputWasWritten": False,
                   "warning": "Any pre-existing routes output is stale and must not be applied."}
        failure_path.write_text(json.dumps(failure, ensure_ascii=False, indent=2), encoding="utf-8")
        print(f"FAIL: {problem}\nDetails: {failure_path}\nNo new routes output was written.", file=sys.stderr)
        return 2
    args.output.write_text(json.dumps(results, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"Wrote {args.output}; review named-line label placement after applying point edits.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
