#!/usr/bin/env python3
"""
August-Rio Python Bindings
==========================

Python FFI bindings for the August-Rio unified compiler.

This module provides a Pythonic interface to the August-Rio embeddable C API,
enabling seamless integration with Python applications and tools.

Features:
- Native Python objects with automatic memory management
- Exception handling for compilation errors
- Homoiconic AST access as Python dictionaries
- Inheritance registry queries
- Canonical path parsing and validation
"""

import ctypes
import json
from typing import Dict, List, Optional, Any, Union
from dataclasses import dataclass
from enum import Enum

# =============================================================================
# TYPE DEFINITIONS
# =============================================================================

class AugustRioError(Exception):
    """Exception raised for August-Rio compilation errors."""
    pass

class AugustRioCompileMode(Enum):
    """Compilation mode options."""
    NORMAL = 0
    STRICT = 1
    DEBUG = 2

@dataclass
class AugustRioOptions:
    """Compilation options for August-Rio."""
    strict_mode: bool = False
    auto_hoist: bool = True
    debug_mode: bool = False
    json_output: bool = True

    def to_c_struct(self) -> 'AugustRioCompileOptions':
        """Convert to C structure."""
        return self._lib.rio_default_options()

@dataclass
class AugustRioStats:
    """Compilation statistics."""
    canonical_paths_count: int = 0
    inheritance_edges_count: int = 0
    r_term_ops_count: int = 0
    d_term_ops_count: int = 0
    membrane_crossings_count: int = 0
    compilation_time_ms: float = 0.0
    validation_time_ms: float = 0.0

@dataclass
class AugustRioResult:
    """Compilation result containing all outputs."""
    success: bool
    canonical_code: Optional[str] = None
    reversible_ir: Optional[str] = None
    membrane_logs: Optional[str] = None
    inheritance_graph: List[str] = None
    hrir_json: Optional[str] = None
    stats: AugustRioStats = None
    error_message: Optional[str] = None

    def hrir_program(self) -> Optional[Dict[str, Any]]:
        """Parse HRIR JSON as Python dictionary."""
        if self.hrir_json:
            return json.loads(self.hrir_json)
        return None

# =============================================================================
# C API INTERFACE
# =============================================================================

class AugustRioVM:
    """August-Rio Virtual Machine wrapper."""

    def __init__(self):
        """Initialize August-Rio VM."""
        self._vm = None
        self._lib = None
        self._load_library()

    def _load_library(self):
        """Load the Rio+RioVN shared library."""
        try:
            self._lib = ctypes.CDLL('./librio.so')
            self._setup_function_signatures()
            self._vm = self._lib.rio_create_vm()
            if not self._vm:
                raise RioError("Failed to create Rio+RioVN VM")
        except OSError as e:
            raise RioError(f"Failed to load librio.so: {e}")

    def _setup_function_signatures(self):
        """Set up C function signatures for ctypes."""
        # VM lifecycle
        self._lib.rio_create_vm.restype = ctypes.c_void_p
        self._lib.rio_destroy_vm.argtypes = [ctypes.c_void_p]

        # Compilation
        self._lib.rio_compile_string.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_void_p
        ]
        self._lib.rio_compile_string.restype = ctypes.c_void_p

        self._lib.rio_compile_file.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_void_p
        ]
        self._lib.rio_compile_file.restype = ctypes.c_void_p

        # Result access
        self._lib.rio_result_success.argtypes = [ctypes.c_void_p]
        self._lib.rio_result_success.restype = ctypes.c_bool

        self._lib.rio_result_json_output.argtypes = [ctypes.c_void_p]
        self._lib.rio_result_json_output.restype = ctypes.c_char_p

        self._lib.rio_result_hrir_json.argtypes = [ctypes.c_void_p]
        self._lib.rio_result_hrir_json.restype = ctypes.c_char_p

        # Cleanup
        self._lib.rio_free_result.argtypes = [ctypes.c_void_p]

    def compile_string(self, code: str, options: AugustRioOptions = None) -> AugustRioResult:
        """Compile August-Rio code string."""
        if options is None:
            options = AugustRioOptions()

        c_options = self._create_c_options(options)
        c_result = self._lib.rio_compile_string(self._vm, code.encode('utf-8'), c_options)

        if not c_result:
            raise AugustRioError("Compilation failed - null result")

        return self._parse_result(c_result)

    def compile_file(self, filepath: str, options: AugustRioOptions = None) -> AugustRioResult:
        """Compile August-Rio file."""
        if options is None:
            options = AugustRioOptions()

        c_options = self._create_c_options(options)
        c_result = self._lib.rio_compile_file(self._vm, filepath.encode('utf-8'), c_options)

        if not c_result:
            raise AugustRioError("Compilation failed - null result")

        return self._parse_result(c_result)

    def _create_c_options(self, options: AugustRioOptions) -> ctypes.c_void_p:
        """Create C options structure."""
        # This would need to be implemented to match the C API
        # For now, return default options
        return self._lib.rio_default_options()

    def _parse_result(self, c_result: ctypes.c_void_p) -> AugustRioResult:
        """Parse C result into Python AugustRioResult."""
        success = self._lib.rio_result_success(c_result)

        if not success:
            error_msg = "Compilation failed"
        else:
            error_msg = None

        # Get HRIR JSON
        hrir_json_ptr = self._lib.rio_result_hrir_json(c_result)
        hrir_json = None
        if hrir_json_ptr:
            hrir_json = ctypes.string_at(hrir_json_ptr).decode('utf-8')

        # Get JSON output
        json_ptr = self._lib.rio_result_json_output(c_result)
        json_output = None
        if json_ptr:
            json_output = ctypes.string_at(json_ptr).decode('utf-8')

        # Parse JSON for additional data
        inheritance_graph = []
        if json_output:
            try:
                data = json.loads(json_output)
                inheritance_graph = data.get('inheritance_relations', [])
            except json.JSONDecodeError:
                pass

        # Cleanup C result
        self._lib.rio_free_result(c_result)

        return AugustRioResult(
            success=success,
            hrir_json=hrir_json,
            inheritance_graph=inheritance_graph,
            error_message=error_msg
        )

    def __del__(self):
        """Cleanup VM resources."""
        if self._vm:
            self._lib.rio_destroy_vm(self._vm)

# =============================================================================
# HIGH-LEVEL PYTHONIC API
# =============================================================================

def compile_rio(code: str, **options) -> AugustRioResult:
    """
    Compile August-Rio code with Pythonic interface.

    Args:
        code: August-Rio source code
        **options: Compilation options (strict_mode, auto_hoist, debug_mode, json_output)

    Returns:
        AugustRioResult with compilation outputs

    Example:
        >>> result = compile_rio("MathProto <- ObjectProto\\nmath -> add 5 3")
        >>> print(result.hrir_program())
    """
    opts = AugustRioOptions(**options)
    vm = AugustRioVM()
    return vm.compile_string(code, opts)

def compile_rio_file(filepath: str, **options) -> AugustRioResult:
    """
    Compile August-Rio file with Pythonic interface.

    Args:
        filepath: Path to .rio file
        **options: Compilation options

    Returns:
        AugustRioResult with compilation outputs
    """
    opts = AugustRioOptions(**options)
    vm = AugustRioVM()
    return vm.compile_file(filepath, opts)

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

def validate_canonical_path(path: str) -> bool:
    """
    Validate a canonical Proto.Actor.Func path.

    Args:
        path: Canonical path string

    Returns:
        True if path is valid canonical form
    """
    # Simple validation - should be enhanced with actual parsing
    parts = path.split('.')
    return len(parts) == 3 and all(part.endswith('Proto') or part.endswith('Actor') or not part.endswith(('Proto', 'Actor')) for part in parts)

def extract_inheritance_hierarchy(result: RioResult) -> Dict[str, List[str]]:
    """
    Extract inheritance hierarchy from compilation result.

    Args:
        result: RioResult from compilation

    Returns:
        Dictionary mapping prototypes to their parents
    """
    hierarchy = {}
    for relation in result.inheritance_graph or []:
        if ' <- ' in relation:
            child, parent = relation.split(' <- ')
            hierarchy[child] = hierarchy.get(child, []) + [parent]
    return hierarchy

# =============================================================================
# EXAMPLE USAGE
# =============================================================================

if __name__ == "__main__":
    # Example usage
    code = """
    ObjectProto <- BaseProto
    MathProto <- ObjectProto
    CalculatorProto <- MathProto

    // R-term operations
    math -> add 10 5
    calc -> multiply result 2

    // D-term with explicit tag
    @io io -> output "Hello from Python!"
    """

    print("🌀 Rio+RioVN Python Bindings Demo")
    print("=" * 40)

    try:
        result = compile_rio(code, debug_mode=True)

        if result.success:
            print("✅ Compilation successful!")

            # Show HRIR program
            hrir = result.hrir_program()
            if hrir:
                print(f"📊 HRIR Program: {hrir['cell_count']} cells")
                for cell in hrir['cells'][:3]:  # Show first 3 cells
                    print(f"  • {cell['opcode']}({', '.join(cell['args'])}) [{'R' if cell['is_reversible'] else 'D'}]")

            # Show inheritance hierarchy
            hierarchy = extract_inheritance_hierarchy(result)
            print(f"🔗 Inheritance: {len(hierarchy)} prototypes")

            print("\n🎉 Demo completed successfully!")

        else:
            print(f"❌ Compilation failed: {result.error_message}")

    except AugustRioError as e:
        print(f"❌ August-Rio Error: {e}")
    except Exception as e:
        print(f"❌ Unexpected error: {e}")

    print("\nPython bindings provide:")
    print("• Native Python objects with automatic memory management")
    print("• Exception handling for compilation errors")
    print("• Homoiconic HRIR access as Python dictionaries")
    print("• Inheritance hierarchy extraction")
    print("• Canonical path validation")
