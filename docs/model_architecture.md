# Model Architecture

Input:

Radar channels + IMU

Architecture:

Input
 |
 +----------------+
 |                |
Spatial Branch  Temporal Branch
 |                |
Spatial Feature Temporal Feature
 |                |
 +------ Fusion --+
        |
   Classification


The design separates:
- spatial deformation patterns
- temporal evolution patterns
