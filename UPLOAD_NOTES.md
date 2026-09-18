# Project upload

This repository contains the requested project source snapshot, including build configuration, vendor libraries/drivers with their original notices, 34 SLX paths, and 2 Simulink data dictionaries.

All 249 MATLAB `.m` scripts have byte-identical `.m.txt` companions in the same directories. Original source encodings and file bytes are preserved.

The 1486 source-snapshot files are listed in `upload_manifest.json` with their byte sizes, SHA-256 checksums and Git blob hashes. `upload_exclusions.json` lists 357 omitted items. Build caches and temporary files were excluded using the project's ignore rules before this list was assembled.

As requested, reproducible simulation results and non-runtime datasets are omitted. This includes the APSFSM source-replay MAT fixture and power-calibration CSV inputs; the replay tests and calibration fitting scripts require those inputs to be provided separately. Explanatory images, reference PDFs and Word paperwork are outside this code/model snapshot.

The existing local file `afo/SensorlessFocFOSMOExample/mcb_pmsm_foc_sensorless_f28379d.slx` is zero bytes and is preserved unchanged. The other 33 SLX packages were readable as ZIP archives. A read-only archive inspection found no project MAT/CSV references in those packages; two references point to MATLAB's built-in `ansi_tfl_tmw_new.mat`. This inspection is not a model execution test.

No source, algorithm, model behavior, A2L contents or local Git history was modified. No build, MATLAB replay, numerical simulation or bench tests were run for this upload. Local Git history and remote configuration are not imported by this file-snapshot upload.
