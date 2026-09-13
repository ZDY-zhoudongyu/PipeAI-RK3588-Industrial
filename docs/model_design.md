# PipeAI Model Design

## Overview

PipeAI is designed as a physics-guided spatial-temporal learning system.

Pipeline:

Sensor Data
 ->
Preprocessing
 ->
Physical Representation
 ->
Spatial Feature Learning
 +
Temporal Feature Learning
 ->
Fusion
 ->
Classification


## Motivation

The model does not directly learn raw sensor values only.
It introduces physical deformation representation to improve interpretability.

## Branches

Spatial Branch:
- learns deformation-related spatial patterns

Temporal Branch:
- learns time evolution patterns

Fusion Head:
- combines multi-domain features
