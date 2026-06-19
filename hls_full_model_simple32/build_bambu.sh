#!/bin/bash
set -e
# HLS4ML insert_bambu_command BEGIN
bambu  firmware/myproject.cpp --top-fname=myproject -lm -Ifirmware/ac_types --compiler=I386_CLANG16 --generate-interface=INFER -v4
# HLS4ML insert_bambu_command END

# HLS4ML insert_final_report_copying BEGIN

# HLS4ML insert_final_report_copying END
