# 🦾 Adaptive Sliding-Mode Control of a 4-DOF Robot Manipulator

<div align="center">

![System Status](https://img.shields.io/badge/status-complete-brightgreen)
![Wolfram](https://img.shields.io/badge/platform-Wolfram%20Mathematica-red)
![Robotics](https://img.shields.io/badge/robotics-serial%20manipulator-blue)
![Control](https://img.shields.io/badge/control-adaptive%20SMC-purple)
![License](https://img.shields.io/badge/license-MIT-blue)

**A nonlinear robotics control system that performs multi-target pick-and-place under extreme modeling uncertainty**

*Tracks trajectories, adapts to unknown payloads, and remains stable through discontinuous dynamics*

[Demo](#-results) • [Control](#-control-model) • [Dynamics](#-robot-dynamics) • [Why this matters](#-why-this-matters)

---

![Manipulator Demo](media/robot_pick_place.gif)

*Four sequential pick-and-place operations inside a constrained diamond workspace*

</div>

---

## 🎯 Project Overview

This project implements a **fully nonlinear adaptive sliding-mode controller** for a **4-DOF planar robotic manipulator** tasked with retrieving four payloads from a **narrow diamond-shaped workspace** and returning them to a drop-off zone.

The controller is intentionally given the **wrong model**:

* Link masses are **20% underestimated**
* Payload mass is **57% underestimated**
* Payload mass changes **instantaneously** when picked up

Yet the robot still tracks trajectories and completes all four tasks robustly .

This is exactly the kind of uncertainty encountered in **real robots** — where perfect modeling is impossible.

---

## 🔑 Key Features

```
✓ Full Euler–Lagrange robot dynamics     ✓ Adaptive sliding-mode control
✓ 4-DOF serial manipulator              ✓ Unknown link & payload masses
✓ Time-varying system dynamics          ✓ Narrow collision-constrained workspace
✓ Multi-target pick-and-place           ✓ Impact & bouncing physics
✓ Inverse kinematics + trajectory gen   ✓ High-fidelity numerical integration
```

---

## 📊 Results

### 🎥 Simulation Videos

**Multi-Target Pickup Sequence**
*(Insert your exported animation here)*

```
media/sequential_pickup.mp4
```

**Single-Mass Trajectory (with payload pickup)**

```
media/mass1_pickup.mp4
```

---

### 📈 Controller Performance

| Metric                 | Result                         |
| ---------------------- | ------------------------------ |
| Pick-and-place cycles  | **4 successful**               |
| Total operation time   | **44 seconds**                 |
| Payload mass error     | **57% underestimated**         |
| Tracking recovery time | **≈ 1–2 s after pickup**       |
| Stability              | **Maintained for all targets** |

Tracking error spikes when mass is picked up, then rapidly converges back to zero — a hallmark of sliding-mode robustness .

---

### 📉 Analysis Plots (Placeholders)

```
plots/control_torque.png
plots/tracking_error.png
plots/mass_estimation.png
plots/joint_angles.png
```

These show:

* Torque jumps at pickup
* Tracking error recovery
* True vs estimated mass
* Joint angle evolution

---

## 🧠 Control Model

### Tracking Error

```math
\tilde q = q - q_d
```

### Sliding Surface

```math
s = \dot{\tilde q} + \Lambda \tilde q
```

### Reference Trajectory

```math
\dot q_r = \dot q_d - \Lambda \tilde q
```

### Regressor Form

```math
M(q)\ddot q_r + C(q,\dot q)\dot q_r + G(q) = Y(q,\dot q,\dot q_r,\ddot q_r)\,a
```

### Control Law

```math
\tau = Y(q,\dot q,\dot q_r,\ddot q_r)\,\hat a - K\,\text{sat}(s)
```

Where:

* **Y** is the regressor matrix
* **â** are estimated mass parameters
* **K** is the sliding-mode gain
* **sat(·)** is a boundary-layer saturation function to prevent chattering

This ensures stability even when the true mass changes mid-motion .

---

## ⚙️ Robot Dynamics

The 4-DOF manipulator is modeled using **Euler–Lagrange mechanics**:

```math
M(q)\ddot q + C(q,\dot q)\dot q + G(q) = \tau
```

Where:

* **M(q)** is the mass matrix
* **C(q, q̇)** contains Coriolis & centrifugal terms
* **G(q)** is gravity

These are computed symbolically and numerically solved using Mathematica’s **DAE-aware NDSolve** engine .

---

## 🧱 Robot Wall Impact & Bounce Physics (Contact Extension)

This “Part 2” extends the project beyond free-space manipulation into **contact dynamics** by simulating a **tip collision with a vertical wall**.

* The robot is commanded to reach a target **past the wall** (intentionally impossible without collision) 
* The simulation **detects collision** when the tip crosses `x ≥ wallX` and estimates pre-impact tip velocity 
* A **wall impulse model** applies restitution (normal direction) + Coulomb friction (tangential direction) 
* The robot then switches into a second phase: **bounce-back trajectory planning** starting from the **collision state** 

**Wall configuration (in code):**

* `wallX = 2.3 m` 
* `eWall = 0.7` (restitution) 
* `μWall = 0.1` (friction) 
* `mTip = 0.50 kg` effective tip mass 

---

### 🎥 Results (Wall Bounce)

**Animation (tip impacts wall, then bounces and re-tracks):**

```
media/robot_wall_bounce.mp4
```

**Static trajectory plot (Blue = approach, Green = bounce):**

```
plots/robot_wall_bounce_static.png
```

---

### 🔬 Impact Model (Impulse-Based)

The wall contact is modeled as an **instantaneous impulse update** on the tip velocity.

#### 1) Normal direction (restitution)

Normal axis is **x** (wall normal), so the x-velocity reverses with restitution:

```math
v_x^+ = -e_{\text{wall}} \, v_x^-
```

This is implemented directly in your `ApplyWallImpact()` function .

---

#### 2) Tangential direction (Coulomb friction impulse)

Tangential axis is **y**. First compute a normal impulse magnitude:

```math
J_n = m_{\text{tip}}\,|v_x^-|\,(1+e_{\text{wall}})
```

Then the friction impulse is:

```math
J_t = \mu_{\text{wall}}\,J_n
```

And the tangential velocity update uses a **sliding vs sticking** condition:

```math
\Delta v_t = J_t / m_{\text{tip}}
```
---
#### 3) Energy retained

You compute and print the energy retention across impact as:

```math
\eta = \frac{(v_x^+)^2 + (v_y^+)^2}{(v_x^-)^2 + (v_y^-)^2}
```

The code prints this directly as a percentage after impact .

---

### 🧩 Two-Phase Control Structure

The wall-bounce simulation is intentionally structured as:

**Phase 1 — Approach:** track a desired trajectory toward a target that lies past the wall.
**Phase 2 — Bounce:** once impact occurs, re-initialize the system at the collision angles/velocities and track a new desired trajectory away from the wall.

The output logs show the full sequence:

* collision detected at `t ≈ 1.56 s`
* pre/post impact velocities
* then bounce phase completes successfully 

---

### ✅ Why This Extension Is Important

Most student robotics projects stop at “trajectory tracking in free space.”

This demonstrates the next step toward real manipulation:

✅ **contact detection**
✅ **impulse-based collision response**
✅ **sliding vs sticking friction modes**
✅ **state reset after collision**
✅ **re-planning a stabilizing trajectory after impact**

That’s exactly the foundation for:

* compliant manipulation
* force-control extensions
* hybrid systems (continuous dynamics + discrete events)

---

## 🧪 Why This Matters

This project demonstrates **real robotics problems**:

<table>
<tr>
<td width="50%">

**🤖 Industrial Robotics**

* Pick-and-place with unknown product weights
* Robot arms in factories
* Adaptive manipulation

</td>
<td width="50%">

**🚀 Space Robotics**

* Time-varying mass properties
* Fuel & payload changes
* Nonlinear dynamics

</td>
</tr>
<tr>
<td width="50%">

**🩺 Surgical Robotics**

* Tools & tissue change mass
* Precision under uncertainty

</td>
<td width="50%">

**📦 Logistics Automation**

* Sorting robots
* Uncertain object properties

</td>
</tr>
</table>

---

## 📚 Want the Full Math?

A **graduate-level technical report** is included that derives:

* Robot dynamics
* Sliding-mode stability
* Regressor-based adaptive control
* Impact mechanics
* Performance analysis

📄 `report/Adaptive_Sliding_Mode_4DOF_Report.pdf`

This is written at the level of **Slotine & Li** and **Spong & Vidyasagar**.

---

## 👤 Author

**Gustavo Torres**

[![GitHub](https://img.shields.io/badge/GitHub-gustavotorr-181717?style=flat\&logo=github)](https://github.com/gustavotorr)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-Gustavo%20Torres-0077B5?style=flat\&logo=linkedin)](https://linkedin.com/in/gustavo-torres)

---

<div align="center">

**⭐ If you found this project interesting, consider giving it a star!**

*Advanced nonlinear control meets real robotic uncertainty*

</div>

