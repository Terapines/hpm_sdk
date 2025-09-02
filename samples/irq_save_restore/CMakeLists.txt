# Copyright (c) 2021 HPMicro
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.13)

set(RV_ABI "ilp32d")
set(RV_ARCH "rv32gc_zp052b")

find_package(hpm-sdk REQUIRED HINTS $ENV{HPM_SDK_BASE})

project(irq_save_restore)

sdk_compile_options("-mirq-save-restore=gpr-used,fpr-used")

sdk_app_src(src/test.c src/unknowncall.S)
generate_ide_projects()
