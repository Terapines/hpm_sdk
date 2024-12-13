#
# Copyright (c) 2021 hpmicro
# Copyright (C) 2020-2024 Terapines Technology (Wuhan) Co., Ltd
#
# SPDX-License-Identifier: BSD-3-Clause
#

export ZCC_TOOLCHAIN_PATH=
export HPM_SDK_TOOLCHAIN_VARIANT=zcc

if [ "X$MSYSTEM" "==" "X" ]; then
    if [ "X$name" "==" "Xenv.sh" ]; then
        echo "Please source this file, rather than executing it."
        exit
    fi
    env_name=$0
else
    env_name=$1
fi

script=$(cd -P -- "$(dirname -- "$env_name")" && printf '%s\n' "$(pwd -P)/$(basename -- "$env_name")")
if [ "X$MSYSTEM" "==" "X" ]; then
    export HPM_SDK_BASE=$(dirname "$script")
else
    export HPM_SDK_BASE=$script
fi

echo $HPM_SDK_BASE

echo "GNURISCV_TOOLCHAIN_PATH=$ZCC_TOOLCHAIN_PATH"
echo "HPM_SDK_TOOLCHAIN_VARIANT=$HPM_SDK_TOOLCHAIN_VARIANT"
echo "HPM_SDK_BASE=$HPM_SDK_BASE"

export OPENOCD_SCRIPTS=${HPM_SDK_BASE}/boards/openocd

