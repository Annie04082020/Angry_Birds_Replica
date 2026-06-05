import json
import math

def main():
    json_path = "/home/anniehuang/Angry_Birds_Replica/Resources/levels/level_1_test.json"
    with open(json_path, "r") as f:
        data = json.load(f)

    # Slingshot group details
    # We only adjust Environment group objects
    # Environment group: scaleMultiplier = 1.5, offsetYPercent = -2.0, scalePivotX = 0, scalePivotY = 0
    # Formula for Environment coordinates in pixels:
    # x = xPercent * 24.0 * 1.5 = xPercent * 36.0
    # y = (yPercent * 13.5 - 2.0 * 13.5 * 1.5) * 1.5 = (yPercent * 13.5 - 40.5) * 1.5 = yPercent * 20.25 - 60.75
    # size.x = 36.0 * widthPercent
    # size.y = 20.25 * heightPercent

    WINDOW_WIDTH = 2400.0
    WINDOW_HEIGHT = 1350.0
    m_WorldFloorY = (0.5 - 404.0 / 563.0) * WINDOW_HEIGHT # approx -293.7389

    static_obstacles = []
    dynamic_objects = []

    # Map templates to identify static objects
    templates = {
        "WOOD_stage": {"isStatic": True},
        "WOOD_41x20": {"isStatic": False},
        "WOOD_205": {"isStatic": False},
        "WOOD_83x20": {"isStatic": False},
        "ICE_83x20": {"isStatic": False},
        "PIG_SMALL": {"isStatic": False}
    }

    for idx, obj in enumerate(data["objects"]):
        if obj.get("groupId") != "Environment":
            continue
        
        image_id = obj["imageId"]
        is_static = templates.get(image_id, {}).get("isStatic", False)

        # Parse sizes
        width_pct = obj.get("widthPercent", obj.get("heightPercent", 0.0))
        height_pct = obj.get("heightPercent", width_pct)
        if "widthPercent" not in obj and "heightPercent" in obj:
            width_pct = obj["heightPercent"]
            
        rot = obj.get("rotation", 0.0)

        w_px = 36.0 * width_pct
        h_px = 20.25 * height_pct
        half_w = w_px * 0.5
        half_h = h_px * 0.5
        
        # Rotated AABB half dimensions
        aabb_half_w = abs(math.cos(rot)) * half_w + abs(math.sin(rot)) * half_h
        aabb_half_h = abs(math.sin(rot)) * half_w + abs(math.cos(rot)) * half_h

        obj_data = {
            "index": idx,
            "imageId": image_id,
            "x": obj["xPercent"] * 36.0,
            "y": obj["yPercent"] * 20.25 - 60.75,
            "aabbHalfW": aabb_half_w,
            "aabbHalfH": aabb_half_h,
            "objRef": obj
        }

        if is_static:
            static_obstacles.append(obj_data)
        else:
            dynamic_objects.append(obj_data)

    # Sort dynamic objects by initial Y ascending (bottom first)
    dynamic_objects.sort(key=lambda o: o["y"])

    settled_objects = []

    for obj in dynamic_objects:
        obj_min_x = obj["x"] - obj["aabbHalfW"]
        obj_max_x = obj["x"] + obj["aabbHalfW"]
        
        # Floor support
        support_y = m_WorldFloorY + obj["aabbHalfH"]

        def check_support(other):
            other_min_x = other["x"] - other["aabbHalfW"]
            other_max_x = other["x"] + other["aabbHalfW"]
            
            # Check horizontal overlap
            overlap = min(obj_max_x, other_max_x) - max(obj_min_x, other_min_x)
            if overlap > 0.01:
                other_top = other["y"] + other["aabbHalfH"]
                # Must be below the object's initial Y
                if other_top <= obj["y"] - obj["aabbHalfH"] + 2.0:
                    candidate_y = other_top + obj["aabbHalfH"]
                    return candidate_y
            return None

        for other in static_obstacles:
            cand = check_support(other)
            if cand is not None and cand > support_y:
                support_y = cand

        for other in settled_objects:
            cand = check_support(other)
            if cand is not None and cand > support_y:
                support_y = cand

        # Set new settled Y
        obj["y"] = support_y
        # Convert Y back to yPercent
        # yPercent = (y + 60.75) / 20.25
        new_y_pct = (support_y + 60.75) / 20.25
        obj["objRef"]["yPercent"] = round(new_y_pct, 6)
        
        settled_objects.append(obj)

    # Write back to level JSON
    with open(json_path, "w") as f:
        json.dump(data, f, indent=4)

    print("Success: Settled all level objects geometrically!")

if __name__ == "__main__":
    main()
