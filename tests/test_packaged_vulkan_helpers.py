# SPDX-License-Identifier: GPL-2.0-or-later
"""Test fixtures and helper methods for packaged Vulkan driver tests."""

import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from paths import ROOT

spec = importlib.util.spec_from_file_location("bbport_vulkan", ROOT / "launcher/bbport_vulkan.py")
vulkan = importlib.util.module_from_spec(spec)
spec.loader.exec_module(vulkan)


class BasePackagedVulkanTestCase(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.icds = self.root / "icds"
        self.libs = self.root / "host-libs"
        self.icds.mkdir()
        self.libs.mkdir()
        self.env = {
            "BB_DATA_DIR": str(self.root / "data"),
            "BB_BUNDLED_VK_DRIVER_FILES": "/bundled/radeon.json:/bundled/intel.json",
            "LD_LIBRARY_PATH": "/bundled/lib"
        }

    def library(self, name, bits=64):
        path = self.libs / name
        path.write_bytes(b"\x7fELF" + bytes([2 if bits == 64 else 1, 1]) +
                         b"\0" * 12 + b"\x3e\0")
        return path

    def manifest(self, library="libGLX_nvidia.so.0"):
        path = self.icds / "nvidia_icd.json"
        path.write_text(json.dumps({
            "file_format_version": "1.0.0",
            "ICD": {"library_path": library, "api_version": "1.3.0"}
        }))
        return path

    def configure(self):
        return vulkan.configure(self.env, (self.icds,), (self.libs,))
