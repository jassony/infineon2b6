"""Read-only checks of exported Simulink port-aware layout snapshots.

Run with Python's standard library, without MATLAB or Simulink:
    python check_port_layout.py before.json after.json --output-dir report_dir

Only checks.json and checks.csv in the output directory are written.  The
snapshots and models are never modified.  Text geometry is supplied by the
exporter; this checker does not turn character-count estimates into measured
text.  External names default to centered placement, two model units from the
block edge.  An optional nameBounds field overrides that placement.  Missing
measurements are NOT_RUN.  This is a geometric check, not manual or numerical
acceptance; line-label bounds must be supplied separately for their coverage.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import csv
import json
import math
from pathlib import Path


EPS = 1e-7


def seq(value):
    """Accept JSON arrays and MATLAB singleton-struct encodings."""
    if value is None or value == {}:
        return []
    return value if isinstance(value, list) else [value]


def number(value):
    return float(value)


def close(a, b):
    try:
        return abs(number(a) - number(b)) <= EPS
    except (TypeError, ValueError):
        return False


def same_point(a, b):
    return len(a) == len(b) == 2 and all(close(x, y) for x, y in zip(a, b))


def valid_vector(value, length):
    return (isinstance(value, list) and len(value) == length
            and all(isinstance(x, (int, float)) and math.isfinite(x) for x in value))


def rect_overlap(a, b):
    return (min(a[2], b[2]) - max(a[0], b[0]) > EPS
            and min(a[3], b[3]) - max(a[1], b[1]) > EPS)


def segment_rect(a, b, box):
    """True only when a nonzero segment enters a rectangle's open interior."""
    if close(a[0], b[0]):
        return (box[0] + EPS < a[0] < box[2] - EPS
                and min(max(a[1], b[1]), box[3])
                - max(min(a[1], b[1]), box[1]) > EPS)
    if close(a[1], b[1]):
        return (box[1] + EPS < a[1] < box[3] - EPS
                and min(max(a[0], b[0]), box[2])
                - max(min(a[0], b[0]), box[0]) > EPS)
    return False


def segment_contact(a, b, c, d):
    """Describe all contacts of two nonzero orthogonal segments."""
    ah = close(a[1], b[1])
    ch = close(c[1], d[1])
    if ah == ch:
        axis, fixed = (0, 1) if ah else (1, 0)
        if not close(a[fixed], c[fixed]):
            return None
        lo = max(min(a[axis], b[axis]), min(c[axis], d[axis]))
        hi = min(max(a[axis], b[axis]), max(c[axis], d[axis]))
        if hi < lo - EPS:
            return None
        return {"kind": "collinear_overlap" if hi - lo > EPS else "endpoint_touch",
                "axis": axis, "from": lo, "to": hi, "fixed": a[fixed]}
    h1, h2, v1, v2 = (a, b, c, d) if ah else (c, d, a, b)
    p = [v1[0], h1[1]]
    if not (min(h1[0], h2[0]) - EPS <= p[0] <= max(h1[0], h2[0]) + EPS
            and min(v1[1], v2[1]) - EPS <= p[1] <= max(v1[1], v2[1]) + EPS):
        return None
    end_h = same_point(p, h1) or same_point(p, h2)
    end_v = same_point(p, v1) or same_point(p, v2)
    kind = "endpoint_touch" if end_h and end_v else "false_T" if end_h or end_v else "crossing"
    return {"kind": kind, "point": p}


def parallel_clearance(a, b, c, d, minimum=20):
    """Return inadequate independent-channel clearance with positive overlap.

    This layout application adopts 20 model units between independent
    parallel routing channels.  Zero crossings alone do not establish visual
    separation.  Callers exclude segments belonging to the same source net.
    """
    ah, av = close(a[1], b[1]), close(a[0], b[0])
    ch, cv = close(c[1], d[1]), close(c[0], d[0])
    if ah and ch:
        projection, normal, orientation = 0, 1, "horizontal"
    elif av and cv:
        projection, normal, orientation = 1, 0, "vertical"
    else:
        return None
    overlap_start = max(min(a[projection], b[projection]), min(c[projection], d[projection]))
    overlap_end = min(max(a[projection], b[projection]), max(c[projection], d[projection]))
    gap = abs(a[normal] - c[normal])
    if overlap_end - overlap_start > EPS and gap + EPS < minimum:
        return {"orientation": orientation, "clearance": gap, "required": minimum,
                "projectionOverlap": [overlap_start, overlap_end]}
    return None


def name_box(block):
    if str(block.get("showName", "off")).lower() not in ("on", "true", "1"):
        return None
    supplied = block.get("nameBounds")
    if valid_vector(supplied, 4):
        return supplied
    width, height = block.get("textWidth"), block.get("textHeight")
    if not isinstance(width, (int, float)) or not isinstance(height, (int, float)):
        return None
    if not math.isfinite(width) or not math.isfinite(height) or min(width, height) <= 0:
        return None
    if block.get("orientation", "right") not in ("right", "left"):
        return None
    left, top, right, bottom = block["position"]
    center = (left + right) / 2
    y = top - 2 - height if block.get("namePlacement") == "alternate" else bottom + 2
    return [center - width / 2, y, center + width / 2, y + height]


def edges(scope):
    return Counter((str(x["sourceSid"]), str(x["sourcePort"]),
                    str(x["destinationSid"]), str(x["destinationPort"]), x.get("name", ""))
                   for x in seq(scope.get("lines")))


def port_signature(block):
    return {kind: sorted(str(p["number"]) for p in seq(ports))
            for kind, ports in block.get("ports", {}).items() if seq(ports)}


def port_position(block, kind, index):
    """Use Simulink connection coordinates, not its input-arrow handle origin.

    PortHandles.Position and Block.PortConnectivity.Position can differ by
    the drawn input-arrow length.  The exporter records connectionPosition
    from PortConnectivity; exact equality is still required, with no visual
    offset tolerance.  The raw position remains the port-spacing source.
    """
    matches = [p.get("connectionPosition", p.get("position"))
               for p in seq(block.get("ports", {}).get(kind))
               if str(p.get("number")) == str(index)]
    return matches[0] if len(matches) == 1 and valid_vector(matches[0], 2) else None


def union_box(boxes):
    if not boxes:
        return None
    return [min(b[0] for b in boxes), min(b[1] for b in boxes),
            max(b[2] for b in boxes), max(b[3] for b in boxes)]


def scope_extent(scope):
    """Exporter-supplied body/text/annotation bounds plus every wire point."""
    boxes, missing = [], []
    for b in seq(scope.get("blocks")):
        if valid_vector(b.get("position"), 4):
            boxes.append(b["position"])
            label = name_box(b)
            if label:
                boxes.append(label)
            elif str(b.get("showName", "off")).lower() == "on":
                missing.append("block name: " + b["name"])
    for line in seq(scope.get("lines")):
        for p in seq(line.get("points")):
            if valid_vector(p, 2):
                boxes.append([p[0], p[1], p[0], p[1]])
        if line.get("name"):
            if valid_vector(line.get("labelBounds"), 4):
                boxes.append(line["labelBounds"])
            else:
                missing.append("line label: " + line["name"])
    for a in seq(scope.get("annotations")):
        if valid_vector(a.get("position"), 4):
            boxes.append(a["position"])
        else:
            missing.append("annotation: " + str(a.get("tag", "")))
    bounds = union_box(boxes)
    return {"bounds": bounds, "width": bounds[2] - bounds[0] if bounds else None,
            "height": bounds[3] - bounds[1] if bounds else None,
            "coverage": "complete_exported_bounds" if not missing else "partial",
            "missing": missing}


class Checker:
    def __init__(self):
        self.checks = []
        self.metrics = []

    def add(self, scope, target, check, ok, detail=None):
        status = "NOT_RUN" if ok is None else "PASS" if ok else "FAIL"
        self.checks.append({"scope": scope, "target": target, "check": check,
                            "status": status, "detail": detail})

    def run(self, before, after):
        old_scopes = {s["path"]: s for s in seq(before.get("scopes"))}
        new_scopes = {s["path"]: s for s in seq(after.get("scopes"))}
        self.add("ALL", "scope set", "scope_identity", old_scopes.keys() == new_scopes.keys(),
                 {"before": sorted(old_scopes), "after": sorted(new_scopes)})
        for path, scope in new_scopes.items():
            old = old_scopes.get(path)
            if old is not None:
                self.compare(old, scope)
            self.geometry(scope)
            self.metrics.append({"scope": path,
                                 "before": scope_extent(old) if old is not None else None,
                                 "after": scope_extent(scope)})
        return {"description": "Snapshot geometry only; manual, numerical and compiled checks are not run.",
                "text_geometry": "Measured dimensions from exporter; default centered name placement is a geometric proxy unless nameBounds supplied.",
                "routing_application_rule": "This application uses >=20 model units between different-net parallel segments with positive projection overlap; shared-source nets are excluded.",
                "summary": dict(Counter(c["status"] for c in self.checks)),
                "metrics": self.metrics, "checks": self.checks}

    def compare(self, before, after):
        path = after["path"]
        old = {str(b["sid"]): b for b in seq(before.get("blocks"))}
        new = {str(b["sid"]): b for b in seq(after.get("blocks"))}
        self.add(path, "blocks", "block_identity", old.keys() == new.keys(),
                 {"removed": sorted(old.keys() - new.keys()), "added": sorted(new.keys() - old.keys())})
        self.add(path, "connections", "topology_and_line_names_unchanged", edges(before) == edges(after),
                 {"removed": list((edges(before) - edges(after)).elements()),
                  "added": list((edges(after) - edges(before)).elements())})
        for sid in old.keys() & new.keys():
            a, b = old[sid], new[sid]
            changes = {key: {"before": a.get(key), "after": b.get(key)}
                       for key in ("name", "type", "dialog") if a.get(key) != b.get(key)}
            self.add(path, b["name"], "identity_and_dialog_unchanged", not changes, changes)
            self.add(path, b["name"], "port_signature_unchanged", port_signature(a) == port_signature(b),
                     {"before": port_signature(a), "after": port_signature(b)})

    def geometry(self, scope):
        path = scope["path"]
        blocks = {str(b["sid"]): b for b in seq(scope.get("blocks"))}
        valid = {}
        labels = []
        groups = defaultdict(list)
        for sid, b in blocks.items():
            target = b["name"]
            pos = b.get("position")
            position_ok = valid_vector(pos, 4) and pos[2] > pos[0] and pos[3] > pos[1]
            self.add(path, target, "finite_positive_rectangle", position_ok, pos)
            if not position_ok:
                continue
            valid[sid] = b
            w, h = pos[2] - pos[0], pos[3] - pos[1]
            self.add(path, target, "block_grid_10", all(close(x / 10, round(x / 10)) for x in pos), pos)
            self.add(path, target, "font_Arial_14", b.get("fontName") == "Arial" and close(b.get("fontSize"), 14) if b.get("fontSize") is not None else None,
                     {"font": b.get("fontName"), "size": b.get("fontSize")})
            specified = isinstance(b.get("minW"), (int, float)) and isinstance(b.get("minH"), (int, float))
            self.add(path, target, "category_minimum", w + EPS >= b["minW"] and h + EPS >= b["minH"] if specified else None,
                     {"actual": [w, h], "minimum": [b.get("minW"), b.get("minH")]})
            if b.get("group"):
                groups[str(b["group"])].append((target, w, h))
            label = name_box(b)
            if str(b.get("showName", "off")).lower() == "on":
                self.add(path, target, "external_name_measurement", True if label else None,
                         {"bounds": label, "source": "nameBounds" if b.get("nameBounds") else "centered model with supplied text dimensions"})
                if label:
                    labels.append((sid, target, label))
            self.port_spacing(path, b)
            self.inside_content(path, b)
        for group, members in groups.items():
            sizes = {(x[1], x[2]) for x in members}
            self.add(path, group, "group_size_consistency", len(sizes) == 1, members)
        body_items = list(valid.items())
        for i, (sid, b) in enumerate(body_items):
            collisions = [v["name"] for other_sid, v in body_items[i + 1:]
                          if other_sid != sid and rect_overlap(b["position"], v["position"])]
            self.add(path, b["name"], "block_body_collisions", not collisions, collisions)
        for i, (sid, name, box) in enumerate(labels):
            hits = [other_name for _, other_name, other_box in labels[i + 1:] if rect_overlap(box, other_box)]
            self.add(path, name, "external_name_collisions", not hits, hits)
            padded = [box[0] - 5, box[1] - 5, box[2] + 5, box[3] + 5]
            close_names = [other_name for _, other_name, other_box in labels[i + 1:]
                           if rect_overlap(padded, [other_box[0] - 5, other_box[1] - 5,
                                                    other_box[2] + 5, other_box[3] + 5])]
            self.add(path, name, "external_name_clearance_10", not close_names, close_names)
            hits = [b["name"] for other_sid, b in valid.items()
                    if sid != other_sid and rect_overlap(box, b["position"])]
            self.add(path, name, "external_name_over_block", not hits, hits)
        self.wires(path, scope, valid, labels)
        for i, annotation in enumerate(seq(scope.get("annotations"))):
            self.add(path, annotation.get("tag") or "annotation_" + str(i), "annotation_font_14",
                     close(annotation.get("fontSize", 0), 14), annotation.get("fontSize"))

    def port_spacing(self, path, block):
        pos, sides = block["position"], defaultdict(list)
        for kind, ports in block.get("ports", {}).items():
            for p in seq(ports):
                point = p.get("position")
                if not valid_vector(point, 2):
                    self.add(path, block["name"], "port_coordinate", False, {"kind": kind, "port": p})
                    continue
                # Ports can lie just outside the block outline; use nearest edge.
                distances = {"left": abs(point[0] - pos[0]), "right": abs(point[0] - pos[2]),
                             "top": abs(point[1] - pos[1]), "bottom": abs(point[1] - pos[3])}
                side = min(distances, key=distances.get)
                sides[side].append({"number": p.get("number"), "kind": kind, "point": point})
        for side, ports in sides.items():
            axis = 1 if side in ("left", "right") else 0
            ports.sort(key=lambda p: p["point"][axis])
            distances = [q["point"][axis] - p["point"][axis] for p, q in zip(ports, ports[1:])]
            required = block.get("pitch")
            if isinstance(required, dict):
                required = required.get(side)
            defined = isinstance(required, (int, float)) and math.isfinite(required)
            ok = all(d + EPS >= required for d in distances) if defined else None
            if not distances:
                ok = True  # The requirement has no adjacent pair on this side.
            self.add(path, block["name"], "actual_port_pitch_" + side, ok,
                     {"count": len(ports), "minimum": min(distances) if distances else None,
                      "required": required, "ports": ports})

    def inside_content(self, path, block):
        pos = block["position"]
        width, height = pos[2] - pos[0], pos[3] - pos[1]
        labels = block.get("insideLabels", {})
        left, right = seq(labels.get("left")), seq(labels.get("right"))
        hierarchy = bool(left or right)
        icon_width = block.get("iconTextWidth", 0) or 0
        icon_height = block.get("iconTextHeight", 0) or 0
        if hierarchy:
            measured = all(isinstance(x.get("width"), (int, float)) and isinstance(x.get("height"), (int, float))
                           and x["width"] > 0 and x["height"] > 0 for x in left + right)
            if not measured:
                self.add(path, block["name"], "inside_labels_fit", None, "Missing label measurements")
                return
            central = max(80, icon_width)
            required = 40 + max((x["width"] for x in left), default=0) + central + max((x["width"] for x in right), default=0)
            self.add(path, block["name"], "inside_label_width", width + EPS >= required,
                     {"actualWidth": width, "requiredWidth": required, "central": central, "sideMargins": 20})
            for side, names, kind in (("left", left, "Inport"), ("right", right, "Outport")):
                ports = sorted(seq(block.get("ports", {}).get(kind)), key=lambda p: number(p["number"]))
                if len(names) != len(ports):
                    self.add(path, block["name"], "inside_label_pairing_" + side, False,
                             {"labels": len(names), "ports": len(ports)})
                    continue
                extents = [(p["position"][1] - name["height"] / 2, p["position"][1] + name["height"] / 2)
                           for name, p in zip(names, ports)]
                margins = [min(a - pos[1], pos[3] - b) for a, b in extents]
                gaps = [b[0] - a[1] for a, b in zip(extents, extents[1:])]
                self.add(path, block["name"], "inside_label_clearance_" + side,
                         all(x + EPS >= 10 for x in margins + gaps),
                         {"edgeMargins": margins, "textGaps": gaps, "required": 10})
        if block.get("iconText"):
            measured = icon_width > 0 and icon_height > 0
            # Margins are category-specific design values, not a universal
            # 40-high minimum for the standard 30-high tag shape.
            margin_y = 5 if block.get("type") in ("From", "Goto") else 10
            self.add(path, block["name"], "icon_text_fit",
                     width + EPS >= icon_width + 20 and height + EPS >= icon_height + 2 * margin_y if measured else None,
                     {"block": [width, height], "text": [icon_width, icon_height], "horizontalMargin": 10,
                      "verticalMargin": margin_y})

    def wires(self, path, scope, blocks, labels):
        routes = []
        for index, line in enumerate(seq(scope.get("lines"))):
            src, dst = str(line.get("sourceSid")), str(line.get("destinationSid"))
            target = f'{src}:{line.get("sourcePort")} -> {dst}:{line.get("destinationPort")}'
            points = line.get("points")
            valid_points = isinstance(points, list) and len(points) >= 2 and all(valid_vector(p, 2) for p in points)
            self.add(path, target, "wire_points", valid_points, None if valid_points else points)
            if not valid_points:
                continue
            source = port_position(blocks.get(src, {}), "Outport", line.get("sourcePort"))
            destination = port_position(blocks.get(dst, {}), "Inport", line.get("destinationPort"))
            self.add(path, target, "wire_endpoints", bool(source and destination and same_point(source, points[0]) and same_point(destination, points[-1])),
                     {"sourcePort": source, "first": points[0], "destinationPort": destination, "last": points[-1]})
            pairs = list(zip(points, points[1:]))
            invalid = [i for i, (a, b) in enumerate(pairs) if same_point(a, b) or not (close(a[0], b[0]) or close(a[1], b[1]))]
            self.add(path, target, "orthogonal_nonzero_segments", not invalid, invalid)
            body_hits = [{"block": b["name"], "segment": i} for sid, b in blocks.items() if sid not in (src, dst)
                         for i, (a, z) in enumerate(pairs) if segment_rect(a, z, b["position"])]
            self.add(path, target, "wire_through_unrelated_block", not body_hits, body_hits)
            name_hits = [{"block": name, "segment": i} for _, name, box in labels
                         for i, (a, z) in enumerate(pairs) if segment_rect(a, z, box)]
            self.add(path, target, "wire_through_external_name", not name_hits, name_hits)
            annotation_hits = [{"annotation": note.get("tag"), "segment": i}
                               for note in seq(scope.get("annotations")) if valid_vector(note.get("position"), 4)
                               for i, (a, z) in enumerate(pairs) if segment_rect(a, z, note["position"])]
            self.add(path, target, "wire_through_annotation", not annotation_hits, annotation_hits)
            if line.get("name"):
                self.add(path, target, "line_label_bounds", True if valid_vector(line.get("labelBounds"), 4) else None,
                         line.get("labelBounds"))
            routes.append({"index": index, "target": target, "net": (src, str(line.get("sourcePort"))),
                           "segments": [(i, a, b) for i, (a, b) in enumerate(pairs) if i not in invalid]})
        for i, first in enumerate(routes):
            hits = []
            close_channels = []
            for second in routes[i + 1:]:
                if first["net"] == second["net"]:
                    continue
                for j, a, b in first["segments"]:
                    for k, c, d in second["segments"]:
                        contact = segment_contact(a, b, c, d)
                        if contact:
                            hits.append({"other": second["target"], "segment": j, "otherSegment": k, **contact})
                        clearance = parallel_clearance(a, b, c, d)
                        if clearance:
                            close_channels.append({"other": second["target"], "segment": j,
                                                   "otherSegment": k, **clearance})
            self.add(path, first["target"], "different_net_contacts", not hits, hits)
            self.add(path, first["target"], "different_net_parallel_clearance_20", not close_channels, close_channels)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    default_dir = Path(__file__).resolve().parent / "reports" / "port_aware_layout"
    parser.add_argument("before", nargs="?", type=Path, default=default_dir / "before.json")
    parser.add_argument("after", nargs="?", type=Path, default=default_dir / "after.json")
    parser.add_argument("--output-dir", type=Path, default=default_dir)
    args = parser.parse_args()
    before = json.loads(args.before.read_text(encoding="utf-8-sig"))
    after = json.loads(args.after.read_text(encoding="utf-8-sig"))
    result = Checker().run(before, after)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / "checks.json").write_text(json.dumps(result, indent=2, ensure_ascii=False), encoding="utf-8")
    with (args.output_dir / "checks.csv").open("w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=("scope", "target", "check", "status", "detail"))
        writer.writeheader()
        for item in result["checks"]:
            writer.writerow({**item, "detail": json.dumps(item["detail"], ensure_ascii=False)})
    print(json.dumps(result["summary"], ensure_ascii=False))
    return 1 if result["summary"].get("FAIL", 0) else 0


if __name__ == "__main__":
    raise SystemExit(main())
