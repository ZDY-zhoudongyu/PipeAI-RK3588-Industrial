# Training Pipeline

Training flow:

Dataset
 ->
Signal Preprocessing
 ->
Physical Representation
 ->
Spatial Branch
+
Temporal Branch
 ->
Fusion Head
 ->
Classification Loss
 ->
Checkpoint


The training pipeline is separated from deployment runtime.
