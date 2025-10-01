# ChampSim Project: Hybrid Cache Replacement Policy

This project implements a **hybrid cache replacement policy** in the ChampSim simulator to reduce **Last-Level Cache (LLC)** conflict misses.

---

## Project Overview

The core of this project is a **dynamic replacement policy** that classifies cache sets as either *high-contention* or *low-contention* based on their eviction frequency.

- **Low-Contention Sets**: A standard, fast **LRU (Least Recently Used)** policy is applied.  
- **High-Contention Sets**: A more robust policy, **Ship++**, is applied to better manage cache lines and reduce thrashing.

By applying the more complex policy only where it's most needed, the system reduces conflict misses and improves overall throughput with minimal overhead.

---

## File Structure

This project involves creating and modifying the following files:

- `champsim/replacement/hybrid_policy.cc`: The C++ implementation of the custom hybrid replacement logic.  
- `champsim_config.json`: The JSON configuration file that defines the cache hierarchy and instructs ChampSim to use the `hybrid_policy` for the LLC.

---

## Build and Run Instructions

Follow these steps to configure, build, and run the simulation.

### Step 1: Configure the Build
Run the configuration script from the ChampSim root directory, pointing it to the `champsim_config.json` file:

```bash
./config.sh champsim_config.json
```

### Step 2: Compile the Simulator

Once the configuration is complete, compile the project using the make command. This will create a custom simulator executable in the bin/ directory.

```bash
make
```
The executable will be named according to the "executable_name" field in your JSON file (e.g., bin/hybrid_policy-bimodal-no-no-no-lru-1core).

### Step 3: Run the Simulation

Execute the compiled binary, providing a workload trace. The flags -w and -i specify the number of warmup and simulation instructions, respectively.

Make sure you have downloaded the required traces into the traces/ directory.

```bash
# Example using the omnetpp trace
./bin/hybrid_policy-bimodal-no-no-no-lru-1core -w 10000000 -i 100000000 -t traces/omnetpp_100M.trace.xz
```

### Step 4: Analyze the Results

After the simulation completes, it will generate a results file (e.g., results_100M.txt). Open this file and inspect the statistics for the LLC to evaluate the policy's performance. You will be looking for a reduction in the MISS count and MPKI (Misses Per Kilo-Instruction) compared to a baseline configuration (e.g., a pure LRU policy).

```bash
CPU 0 LLC
ACCESS:    2345678
HIT:       2109876
MISS:      235802
Miss Rate: 10.05%
MPKI:      2.358
```
