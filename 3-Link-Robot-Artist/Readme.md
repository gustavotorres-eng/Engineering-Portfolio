# 3-DOF Robot Manipulator: Sliding Mode Control

![Robot Demo](media/demo_animation.gif)
*Simulation of a 3-link planar robot arm tracking a complex trajectory (Cursive 'a') using Sliding Mode Control.*

## 🚀 Project Overview
This project simulates the dynamics and control of a 3-Degree-of-Freedom (3-DOF) planar robot arm using **Mathematica**. The core objective was to design a robust non-linear controller capable of forcing the robot's end-effector to trace a specific path (a cursive letter 'a') despite model uncertainties and disturbances.

The simulation derives the Equations of Motion (EOM) from scratch using **Lagrangian Dynamics** and implements a **Sliding Mode Controller (SMC)** with a saturation layer to mitigate chattering.

## ⚙️ Technical Highlights
* **Symbolic Modeling:** Automated derivation of the Mass Matrix ($M$), Coriolis/Centrifugal Matrix ($C$), and Gravity Vector ($G$) using Euler-Lagrange equations.
* **Non-Linear Control:** Implemented Sliding Mode Control (SMC) to handle system non-linearities.
* **Chattering Reduction:** Replaced the standard `Sign` function with a `Saturation` function within the boundary layer $\Phi$ to smooth control input:
    $$sat(s/\Phi) = \begin{cases} s/\Phi & \text{if } |s/\Phi| \le 1 \\ \text{sgn}(s/\Phi) & \text{if } |s/\Phi| > 1 \end{cases}$$
* **Disturbance Rejection:** The controller successfully tracks the desired trajectory even with simulated disturbance torques applied to the joints.

## 📐 Mathematical Model
The system dynamics are modeled using the standard robotic equation:

$$M(q)\ddot{q} + C(q, \dot{q})\dot{q} + G(q) = \tau + \tau_{dist}$$

Where:
* $q$: Joint angle vector
* $M(q)$: Inertia matrix
* $C(q, \dot{q})$: Coriolis and Centrifugal forces
* $G(q)$: Gravitational torques
* $\tau$: Control input torque

## 💻 How to Run
1.  Ensure you have **Wolfram Mathematica** installed (Version 12.0+ recommended).
2.  Clone this repository:
    ```bash
    git clone [https://github.com/YourUsername/3-DOF-Robot-Controller.git](https://github.com/YourUsername/3-DOF-Robot-Controller.git)
    ```
3.  Open `src/Robot_SMC_Simulation.nb`.
4.  Run all cells to generate the derivation, solve the differential equations, and view the animation.

## 📊 Results
The controller demonstrates convergence of the tracking error to near-zero. The sliding surface variable $s$ remains bounded, proving the stability of the closed-loop system.

## 🛠 Tech Stack
* **Language:** Wolfram Language (Mathematica)
* **Methods:** Symbolic Computation, Numerical Differential Equation Solving (`NDSolve`)
