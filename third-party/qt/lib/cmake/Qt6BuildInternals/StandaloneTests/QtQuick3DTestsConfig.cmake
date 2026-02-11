# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(__standalone_parts_qt_packages_args
    QT_MODULE_PACKAGES
        Quick3DUtils
        Quick3DAssetImport
        Quick3DRuntimeRender
        Quick3D
        Quick3DParticles
        Quick3DParticleEffects
        Quick3DAssetUtils
        Quick3DEffects
        Quick3DHelpers
        Quick3DHelpersImpl
        Quick3DIblBaker
        Quick3DGlslParserPrivate
        Quick3DXr
)
qt_internal_find_standalone_parts_qt_packages(__standalone_parts_qt_packages_args)
