#!/bin/bash

root -b -q run_hit_merger.C\($1\)
root draw.C\($1\)
