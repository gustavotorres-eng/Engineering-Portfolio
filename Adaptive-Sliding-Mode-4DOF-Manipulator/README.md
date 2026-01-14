# 🦾 Adaptive Sliding-Mode Control of a 4-DOF Manipulator  

A high-fidelity nonlinear robotics simulation demonstrating **robust, adaptive control** of a 4-degree-of-freedom serial manipulator performing **multi-target pick-and-place** under **severe parametric uncertainty** and **time-varying payload dynamics**.

This project implements a **regressor-based adaptive sliding-mode controller** on a **fully Lagrangian-modeled robot**, validating stability and tracking performance despite **20–57% mass estimation error** and sudden payload changes.

---

## 🚀 Project Highlights  

✔ 4-DOF planar robotic manipulator  
✔ Full Euler–Lagrange dynamics  
✔ Adaptive sliding-mode control  
✔ Unknown link & payload masses  
✔ Time-varying dynamics at pickup  
✔ Narrow, constrained workspace  
✔ Impact & bouncing physics  
✔ Multi-target autonomous operation  

---

## 🎯 What This Robot Does  

The robot must retrieve **four separate payloads** placed inside a **diamond-shaped workspace** with a **narrow entry gate**, transport them back to a drop-off zone, and repeat this cycle **without collision** or loss of stability.

Each payload has a **different mass**, and the controller is **intentionally given the wrong model** — including a **57% payload mass error** — forcing the controller to rely on **robust nonlinear control theory** instead of perfect modeling :contentReference[oaicite:0]{index=0}.

---

## 🧠 Control Architecture  

This system uses a **regressor-based Adaptive Sliding Mode Controller**:

\[
s = \dot{\tilde q} + \Lambda \tilde q
\]

\[
\tau = Y(q,\dot q,\dot q_r,\ddot q_r)\hat a - K \, \text{sat}(s)
\]

The control law guarantees **sliding surface convergence** even when:
- link masses are off by **20%**
- payload mass is underestimated by **57%**
- system mass changes **instantaneously** when objects are picked up :contentReference[oaicite:1]{index=1}.

To reduce chattering, the discontinuous sign function is replaced with a **boundary-layer saturation function**.

---

## ⚙️ What Makes This Hard  

This is not a kinematic or PID simulation.

The robot must deal with:

| Challenge | Why it’s difficult |
|--------|----------------|
| **Nonlinear dynamics** | Fully coupled 4-DOF equations of motion |
| **Unknown masses** | Controller must be robust to modeling error |
| **Time-varying dynamics** | Payload adds mass mid-motion |
| **Narrow workspace** | Collision-free motion required |
| **Multi-target sequencing** | 44 seconds of stable continuous control |
| **Impact physics** | Dropped objects bounce realistically |

---

## 📊 What Was Demonstrated  

### ✔ Robust Trajectory Tracking  
Tracking errors spike when payload is picked up — but return to zero within ~1–2 seconds despite **35% mass mismatch** :contentReference[oaicite:2]{index=2}.

### ✔ Sliding Surface Convergence  
The sliding surface stays bounded and converges after each disturbance, proving stability of the nonlinear controller.

### ✔ Torque Adaptation  
Joint torques jump at pickup time as the controller compensates for the new mass, but remain bounded and stable.

### ✔ Impact & Bouncing Physics  
Dropped objects obey Newton’s restitution law and Coulomb friction:
- ~49% energy retained per bounce
- ~13 bounces before settling
- sliding → sticking transition captured :contentReference[oaicite:3]{index=3}.

---

## 🧪 Numerical Implementation  

All dynamics and control laws were solved using **Mathematica’s DAE-capable NDSolve**, including:
- symbolic mass matrices
- Christoffel-based Coriolis terms
- regressor matrices
- adaptive control laws  
with automatic stiffness handling and high-accuracy integration :contentReference[oaicite:4]{index=4}.

---

## 📁 What’s in This Repository  

| Folder | Contents |
|------|--------|
| `mathematica/` | Full symbolic + numerical simulation code |
| `animations/` | Multi-target pickup animations |
| `plots/` | Torque, tracking error, and parameter plots |
| `report/` | Complete technical project report (PDF) |

---

## 📄 Want the Deep Dive?

This repository includes a **full graduate-level project report** that derives:

- Euler-Lagrange dynamics  
- Sliding-mode stability  
- regressor-based adaptive control  
- impact mechanics  
- performance analysis  

📄 **Read it here:**  
`/report/Adaptive_Sliding_Mode_4DOF_Report.pdf`

This document is written at the level of **Slotine & Li / Spong & Vidysagar** and shows the full math behind every result.

---

## 🧠 Why This Matters  

This project demonstrates real-world robotics control problems:

- Industrial pick-and-place with unknown payloads  
- Space robotics with changing mass properties  
- Surgical robotics with uncertain tool loads  
- Any system where **perfect modeling is impossible**

The control approach used here is exactly what is taught in **advanced nonlinear & adaptive robotics courses** — and implemented in research-grade systems.

---

## 🧑‍💻 Author  

**Gustavo Torres**  
M.S. Mechanical Engineering – Robotics & Control  
Manufacturing Engineer | Robotics & Physical AI  

---

If you’d like, next I can help you:
- choose the **best video or plot** to embed at the top of the README
- add **badges** (MATLAB, Mathematica, Control Theory, Robotics)
- or turn this into a **portfolio-ready GitHub landing page**

This project is strong enough to carry interviews by itself.
