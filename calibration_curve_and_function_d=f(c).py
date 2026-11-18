import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import PchipInterpolator



d = np.array([2, 3, 4, 5, 6, 7, 8, 9, 10, 11], dtype=float)   # mm

C_circuit = np.array([
    0.231, 0.231, 0.216, 0.202, 0.202,
    0.202, 0.188, 0.188, 0.188, 0.188
])

C_second = np.array([
    0.981, 0.837, 0.765, 0.692, 0.664,
    0.635, 0.620, 0.606, 0.592, 0.577
])

C_R_rise = np.array([
    0.418, 0.390, 0.361, 0.346, 0.332,
    0.332, 0.317, 0.317, 0.317, 0.303
])

assert len(d) == len(C_circuit) == len(C_second) == len(C_R_rise) == 10



EPS = 1e-4   # nF, tiny shift used only to separate repeated C values


def build_dC(C, d, eps=EPS):

    order = np.lexsort((-d, C))          
    C_s = C[order].astype(float).copy()
    d_s = d[order].copy()

    for value in np.unique(C_s):
        idx = np.where(C_s == value)[0]
        n = len(idx)
        if n > 1:
            C_s[idx] += (np.arange(n) - (n - 1) / 2) * eps

    assert np.all(np.diff(C_s) > 0), "C is not strictly increasing"
    return PchipInterpolator(C_s, d_s), C_s, d_s



def fmt(v):
    """Format a number as a valid C float literal."""
    s = f"{v:.9g}"
    if "." not in s and "e" not in s:
        s += ".0"
    return s + "f"


def to_c_function(f, name="calculateDistance"):

    xs = f.x
    n = len(xs) - 1
    lines = [f"float {name}(float x) {{"]

    # Below the measured range: clamp to the first distance
    lines.append(f"  if (x <= {fmt(xs[0])}) {{")
    lines.append(f"    return {fmt(float(f(xs[0])))};")
    lines.append("  }")

    # One cubic polynomial per segment (Horner form)
    for i in range(n):
        a, b, c, e = f.c[:, i]
        lines.append(f"  else if (x < {fmt(xs[i + 1])}) {{")
        lines.append(f"    float t = x - {fmt(xs[i])};")
        lines.append(
            f"    return (({fmt(a)} * t + {fmt(b)}) * t + {fmt(c)}) * t + {fmt(e)};"
        )
        lines.append("  }")

    # Above the measured range: clamp to the last distance
    lines.append("  else {")
    lines.append(f"    return {fmt(float(f(xs[-1])))};")
    lines.append("  }")
    lines.append("}")
    return "\n".join(lines)




datasets = [
    (C_circuit, "R=100k", "red",   False, "distance_circuit.c"),
    (C_R_rise,  "R=400k",  "green", False, "distance_rise_R.c"),
    (C_second,  "R=1M",      "blue",  True,  "distance_multimeter.c"),
]



fig, axes = plt.subplots(1, 3, figsize=(16, 5))

for ax, (C, title, color, show_c, file_name) in zip(axes, datasets):

    f, C_s, d_s = build_dC(C, d)

   
    assert np.allclose(f(C_s), d_s)

    ax.scatter(C, d, color=color, edgecolors="black",
               s=65, zorder=3, label="Original Measurements")

    C_smooth = np.linspace(C_s.min(), C_s.max(), 2000)
    ax.plot(C_smooth, f(C_smooth), color=color, linewidth=2.5,
            label="PCHIP: d=f(C)")

    # ----- C code -----
    c_code = to_c_function(f)

    with open(file_name, "w", encoding="utf-8") as file:
        file.write(c_code + "\n")

    if show_c:
        print("====================================")
        print(title)
        print(f"(repeated C values shifted by {EPS} nF)")
        print("====================================")
        print("\n// ===== Paste this in your ESP32 code =====")
        print(c_code)

        test_C = 0.35
        print("\nExample calculation:")
        print(f"Input capacitance = {test_C} nF")
        print(f"Displacement = {float(f(test_C)):.6f} mm")

    ax.set_title(title)
    ax.set_xlabel("Capacitance (nF)")
    ax.set_ylabel("Displacement (mm)")
    ax.grid(True, linestyle="--", alpha=0.6)
    ax.legend()

fig.suptitle("Displacement vs Capacitance", fontsize=14)
plt.tight_layout()
plt.show()
