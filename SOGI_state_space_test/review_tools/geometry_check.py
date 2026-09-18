"""Check measured SOGI geometry and planned/applied route points without MATLAB.

Usage: python geometry_check.py route_input.json routes.json output_directory

The input capture must contain actual Position and PortConnectivity values.
The routes argument may contain planned points or points captured after route
application; --route-state labels this distinction in every summary. Structural
and geometry PASS do not certify actual text placement or manual readability.
"""
from __future__ import annotations

import argparse
import csv
import json
import math
import sys
from pathlib import Path

from route import as_list, segments, crosses, crowded_parallel, hit_rect, name_rect, rect_overlap, read_port


def same_point(a, b):
    return len(a) == 2 and len(b) == 2 and all(abs(x - y) <= 1e-8 for x, y in zip(a, b))


def net_key(line):
    return (str(line["sourceSid"]), int(line["sourcePort"]),
            str(line["destinationSid"]), int(line["destinationPort"]))


def block_type(block):
    return block.get("type", block.get("blockType", ""))


def finite_rect(rect):
    return isinstance(rect, (list, tuple)) and len(rect) == 4 and all(
        isinstance(v, (int, float)) and math.isfinite(v) for v in rect
    )


def minimum_size(kind):
    if kind in ("Inport", "Outport"):
        return (30, 20, "exact")
    if kind == "Sum":
        return (40, 40, "minimum")
    if kind == "UnitDelay":
        return (70, 40, "minimum")
    if kind == "SubSystem":
        return (300, 170, "minimum")
    if kind in ("Gain", "Product", "Switch", "DataTypeConversion", "Math", "MinMax", "Constant"):
        return (120, 40, "minimum")
    return None


def write_csv(path, rows, fieldnames):
    with path.open("w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def check_geometry(data, route_data, route_state="planned", clearance=20):
    checks, dimensions, port_rows = [], [], []
    routes_by_scope = {r["scope"]: r for r in as_list(route_data)}

    def add(scope, obj, name, status, measured="", limit="", detail=""):
        if isinstance(status, bool):
            status = "PASS" if status else "FAIL"
        checks.append({"Scope": scope, "Object": obj, "Check": name, "Status": status,
                       "Measured": measured, "Limit": limit, "Detail": detail})

    scopes = as_list(data["scopes"])
    scope_names = {s["path"] for s in scopes}
    add("ALL", "scopes", "route_scope_set", set(routes_by_scope) == scope_names,
        json.dumps(sorted(routes_by_scope)), json.dumps(sorted(scope_names)))
    add("ALL", "manual_review", "final_text_and_readability", "NOT_RUN", "", "manual inspection",
        "Measured/planned envelopes do not replace reviewing full labels, port names and annotations in the final model export.")
    for scope in scopes:
        path = scope["path"]
        blocks = {str(b["sid"]): b for b in as_list(scope["blocks"])}
        route_scope = routes_by_scope.get(path, {"lines": []})
        lines = as_list(route_scope.get("lines"))
        source_lines = as_list(scope.get("lines"))
        source_keys, route_keys = [net_key(line) for line in source_lines], [net_key(line) for line in lines]
        add(path, "nets", "net_identity_and_count", len(route_keys) == len(set(route_keys)) and sorted(source_keys) == sorted(route_keys),
            len(route_keys), len(source_keys), "Exact source/destination SID and one-based port identities; no added, omitted or duplicated nets.")
        bodies, names, annotations = [], [], []
        for sid, block in blocks.items():
            obj, kind, rect = block["name"], block_type(block), block["position"]
            valid = finite_rect(rect) and rect[2] > rect[0] and rect[3] > rect[1]
            add(path, obj, "finite_positive_position", valid, json.dumps(rect), "finite positive W/H")
            if not valid:
                continue
            x1, y1, x2, y2 = rect
            width, height = x2 - x1, y2 - y1
            in_ports = as_list(block.get("ports", {}).get("Inport"))
            out_ports = as_list(block.get("ports", {}).get("Outport"))
            dimensions.append({"Scope": path, "SID": sid, "Name": obj, "Type": kind,
                               "Left": x1, "Top": y1, "Right": x2, "Bottom": y2,
                               "Width": width, "Height": height, "Inports": len(in_ports),
                               "Outports": len(out_ports), "Font": block.get("fontName", ""),
                               "FontSize": block.get("fontSize", "")})
            add(path, obj, "actual_10_unit_grid", all(abs(v / 10 - round(v / 10)) < 1e-8 for v in rect),
                json.dumps(rect), "all four Position coordinates on 10-unit grid")
            font = block.get("fontName")
            add(path, obj, "font_name", str(font).lower() == "arial", font, "Arial")
            size = block.get("fontSize", 0)
            add(path, obj, "font_size", isinstance(size, (int, float)) and size == 14, size, 14,
                "This SOGI trial specifies 14 pt; shrinking text is not a layout fix.")
            minimum = minimum_size(kind)
            if minimum is None:
                add(path, obj, "category_dimensions", "NOT_RUN", f"{width} x {height}", "role/label-specific review",
                    "No fixed class minimum is inferred for this category; inspect measured interface labels and central whitespace.")
            else:
                min_width, min_height, policy = minimum
                good = width == min_width and height == min_height if policy == "exact" else width >= min_width and height >= min_height
                add(path, obj, "category_dimensions", good, f"{width} x {height}", f"{policy}: {min_width} x {min_height}")
            bodies.append((sid, obj, rect))
            if block.get("showName", "on") == "on":
                measured_text = all(isinstance(block.get(k), (int, float)) and math.isfinite(block[k]) and block[k] >= 0 for k in ("textWidth", "textHeight"))
                add(path, obj, "name_measurement_available", measured_text, "", "finite textWidth/textHeight")
                if measured_text:
                    names.append((sid, obj, name_rect(block, pad=0)))
            pitch = 40 if kind in ("SubSystem", "ModelReference") else 20
            for side, ports in (("Inport", in_ports), ("Outport", out_ports)):
                coordinates = []
                for number, port in enumerate(ports, 1):
                    try:
                        point = read_port(block, side, number)
                        coordinates.append(point)
                        port_rows.append({"Scope": path, "Block": obj, "SID": sid, "Side": side,
                                          "Port": number, "ConnectionX": point[0], "ConnectionY": point[1],
                                          "RequiredPitch": pitch})
                    except ValueError as problem:
                        add(path, obj, f"{side}_{number}_connection_position", False, "", "actual PortConnectivity", str(problem))
                if len(coordinates) > 1:
                    diffs = [b[1] - a[1] for a, b in zip(coordinates, coordinates[1:])]
                    add(path, obj, side + "_top_to_bottom_order", all(d > 0 for d in diffs), json.dumps(diffs), ">0")
                    add(path, obj, side + "_actual_pitch", all(d >= pitch - 1e-8 for d in diffs), min(diffs), pitch,
                        "Actual connection positions, not requested heights or inferred arrow offsets.")
        for number, annotation in enumerate(as_list(scope.get("annotations")), 1):
            rect = annotation["position"]
            add(path, f"annotation_{number}", "annotation_bounds", finite_rect(rect), json.dumps(rect), "finite measured bounds")
            if finite_rect(rect):
                annotations.append((f"annotation_{number}", rect))
        for i, (_, a_name, a_rect) in enumerate(bodies):
            for _, b_name, b_rect in bodies[i + 1:]:
                add(path, a_name + " / " + b_name, "block_body_overlap", not rect_overlap(a_rect, b_rect))
        for owner, a_name, a_rect in names:
            for sid, b_name, b_rect in bodies:
                if sid != owner:
                    add(path, a_name + " label / " + b_name, "name_body_overlap", not rect_overlap(a_rect, b_rect))
        for i, (_, a_name, a_rect) in enumerate(names):
            for _, b_name, b_rect in names[i + 1:]:
                add(path, a_name + " / " + b_name, "full_name_overlap", not rect_overlap(a_rect, b_rect))
        for note, rect in annotations:
            for _, obj, body in bodies:
                add(path, note + " / " + obj, "annotation_body_overlap", not rect_overlap(rect, body))
            for _, obj, name in names:
                add(path, note + " / " + obj, "annotation_name_overlap", not rect_overlap(rect, name))

        valid_lines = []
        for index, line in enumerate(lines):
            source_sid, source_port, dest_sid, dest_port = net_key(line)
            obj = f"{source_sid}.y{source_port} -> {dest_sid}.u{dest_port}"
            if source_sid not in blocks or dest_sid not in blocks:
                add(path, obj, "endpoint_block_present", False)
                continue
            sb, db = blocks[source_sid], blocks[dest_sid]
            points = line.get("points", [])
            valid_points = len(points) >= 2 and all(isinstance(p, (list, tuple)) and len(p) == 2 and all(isinstance(v, (int, float)) and math.isfinite(v) for v in p) for p in points)
            add(path, obj, "finite_route_points", valid_points)
            if not valid_points:
                continue
            source, target = read_port(sb, "Outport", source_port), read_port(db, "Inport", dest_port)
            add(path, obj, "source_identity_and_coordinate", same_point(points[0], source), json.dumps(points[0]), json.dumps(source))
            add(path, obj, "target_identity_and_coordinate", same_point(points[-1], target), json.dumps(points[-1]), json.dumps(target))
            line_segments = segments(points)
            orthogonal = all((a[0] == b[0] or a[1] == b[1]) and a != b for a, b in line_segments)
            add(path, obj, "orthogonal_nondegenerate_segments", orthogonal)
            add(path, obj, "forward_horizontal_order", all(b[0] >= a[0] for a, b in line_segments) and target[0] > source[0])
            add(path, obj, "horizontal_port_entry_exit", points[0][1] == points[1][1] and points[-2][1] == points[-1][1])
            first_run = points[1][0] - points[0][0]
            first_required = line.get("labelWidth", 0) + 20 if line.get("name") else 10
            add(path, obj, "first_horizontal_run", first_run >= first_required, first_run, first_required)
            if not orthogonal:
                continue
            for segment_index, (a, b) in enumerate(line_segments, 1):
                segment_name = f"{obj} segment {segment_index}"
                for sid, block_name, rect in bodies:
                    if sid not in (source_sid, dest_sid):
                        add(path, segment_name + " / " + block_name, "wire_block_interior", not hit_rect(a, b, rect), json.dumps([a, b]), json.dumps(rect))
                for _, block_name, rect in names:
                    add(path, segment_name + " / " + block_name, "wire_full_name", not hit_rect(a, b, rect))
                for note, rect in annotations:
                    add(path, segment_name + " / " + note, "wire_annotation", not hit_rect(a, b, rect))
            for i, (a, b) in enumerate(line_segments):
                for c, d in line_segments[i + 2:]:
                    add(path, obj, "self_intersection", not crosses(a, b, c, d))
            if line.get("name"):
                label_bounds = line.get("signalLabelPlannedBounds")
                add(path, obj, "actual_signal_label_bounds", "NOT_RUN", json.dumps(label_bounds), "actual exported label review",
                    "Only first-run width is enforced automatically; planned placement is not proof of actual label location.")
                if finite_rect(label_bounds):
                    for _, body_name, rect in bodies:
                        add(path, obj + " label / " + body_name, "planned_signal_label_body", not rect_overlap(label_bounds, rect))
                    for _, block_name, rect in names:
                        add(path, obj + " label / " + block_name, "planned_signal_label_name", not rect_overlap(label_bounds, rect))
                    for a, b in line_segments:
                        add(path, obj + " label", "planned_signal_label_own_route", not hit_rect(a, b, label_bounds))
            valid_lines.append((obj, line_segments, line))
        for i, (a_name, a_segments, a_line) in enumerate(valid_lines):
            for b_name, b_segments, b_line in valid_lines[i + 1:]:
                for ai, (a, b) in enumerate(a_segments, 1):
                    for bi, (c, d) in enumerate(b_segments, 1):
                        obj = f"{a_name} segment {ai} / {b_name} segment {bi}"
                        add(path, obj, "unrelated_wire_intersection_overlap_false_T", not crosses(a, b, c, d))
                        add(path, obj, "unrelated_parallel_clearance", not crowded_parallel(a, b, c, d, clearance), "", clearance)
                for label_line, other_name, other_segments in ((a_line, b_name, b_segments), (b_line, a_name, a_segments)):
                    rect = label_line.get("signalLabelPlannedBounds")
                    if finite_rect(rect):
                        for c, d in other_segments:
                            add(path, label_line.get("name", "label") + " / " + other_name,
                                "planned_signal_label_other_route", not hit_rect(c, d, rect))
        add(path, "scope", "manual_functional_flow_and_full_text", "NOT_RUN", "", "final export review",
            "Rightward point geometry does not prove semantic producer/consumer order across tags or identify all label rendering issues.")

    counts = {status: sum(row["Status"] == status for row in checks) for status in ("PASS", "FAIL", "NOT_RUN")}
    summary = {"status": "FAIL" if counts["FAIL"] else "PASS_WITH_NOT_RUN",
               "routeState": route_state, "counts": counts,
               "scope": "Measured block/port geometry and supplied route-point geometry only",
               "manualReview": "NOT_RUN", "compiledInterface": "NOT_RUN", "numericalValidation": "NOT_RUN",
               "note": "Supplied planned points must be recaptured after model application. Full labels and semantic flow require manual export review."}
    return checks, dimensions, port_rows, summary


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("routes", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--route-state", choices=("planned", "captured_after_application"), default="planned")
    args = parser.parse_args(argv)
    data = json.loads(args.input.read_text(encoding="utf-8-sig"))
    routes = json.loads(args.routes.read_text(encoding="utf-8-sig"))
    checks, dimensions, port_rows, summary = check_geometry(data, routes, args.route_state)
    args.output.mkdir(parents=True, exist_ok=True)
    write_csv(args.output / "geometry_checks.csv", checks, ["Scope", "Object", "Check", "Status", "Measured", "Limit", "Detail"])
    write_csv(args.output / "block_dimensions.csv", dimensions, ["Scope", "SID", "Name", "Type", "Left", "Top", "Right", "Bottom", "Width", "Height", "Inports", "Outports", "Font", "FontSize"])
    write_csv(args.output / "port_positions.csv", port_rows, ["Scope", "Block", "SID", "Side", "Port", "ConnectionX", "ConnectionY", "RequiredPitch"])
    write_csv(args.output / "geometry_failures.csv", [r for r in checks if r["Status"] == "FAIL"], ["Scope", "Object", "Check", "Status", "Measured", "Limit", "Detail"])
    (args.output / "geometry_summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps(summary, ensure_ascii=False, indent=2))
    return 1 if summary["counts"]["FAIL"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
