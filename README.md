# August-Rio - Unified Programming System

A unified programming system combining Rio's layered reversible substrate with RioVN's canonical message dispatch, embodying **minimization through conceptual unification, synergy, and orthogonality**.

## Overview

This merged system integrates:
- **Rio**: L4 prototype language with reversible substrate (L4→L2a→L1)
- **RioVN**: Von Neumann canonicalization with forced hierarchy and governance
- **Unified Surface**: Two arrows (`->`, `<-`) with optional tags
- **Philosophy**: Reversible by default, explicit D-term boundaries

## Key Features

### Minimal Surface (Two Arrows Only)
```javascript
// Message passing (R-term by default)
math -> add a b
io -> output message

// Inheritance (S-term)
MathProto <- ObjectProto

// Optional tags for D-term boundaries
@irreversible io -> writeToDisk data
@io network -> sendPacket bytes
```

### Unified Entrypoint
```c
CompilationResult* compile(const char* code, CompilerOptions options);
```

### Orthogonal Options
```c
typedef struct {
    bool strict_mode;   // Enforce explicit D-term boundaries
    bool auto_hoist;    // Auto-generate synthetic hierarchy
    bool debug_mode;    // Verbose logging + stats
    bool l5_enhanced;   // Enable L5 homoiconic features (orthogonal)
} CompilerOptions;
```

## Architecture

### Layer Integration & Homoiconic Inheritance
- **L5 Moop**: Natural language homoiconic authoring (unified surface, orthogonal enhancement flag)
- **L4 Rio**: Prototypes + RioVN canonicalization (inherits homoiconicity from L3)
- **L3 Turchin**: Actors + D-term coordination membranes (inherits homoiconicity from L2a)
- **L2a Prigogine**: Reversible functions (R-term, inherits homoiconicity from L1)
- **L2b Prigogine**: Explicit D-term gates (homoiconic D-term logs, bypasses to L0)
- **L1 HRIR**: Homoiconic Reversible IR (self-describing operations)
- **L0 RISC-V**: Hardware execution (inherits reversibility from L1)
- **L0 Assembly**: Target with reversible execution

### Reversible Homoiconic Inheritance Chain
```
L1 HRIR Cells (reversible, homoiconic operations)
    ↓ inherits to
L2a Functions (reversible, homoiconic functions)
    ↓ inherits to
L3 Actors (reversible, homoiconic actors)
    ↓ inherits to
L4 root_proto (reversible, homoiconic prototypes)
```

### Dual-Memory Architecture
- **L1 Reversible Tape**: System-facing homoiconic execution with checkpoints/rollback
- **L2b D-term Membranes**: Irreversible logs with event cross-linking
- **Time-Travel Debugging**: Actor snapshots, undo/redo, selective rollback

### Philosophy Alignment

#### Minimalism
- Two operators at surface level
- Single compilation entrypoint
- Optional tags only when needed

#### Conceptual Unification
- Everything resolves to canonical `Proto.Actor.Func`
- Single AST with forced hierarchy internally
- Unified send semantics across all layers

#### Orthogonality
- Strict/autoHoist/debug vary independently
- Layers maintain contracts but share surface
- Options compose without conflicts

#### Synergy
- RioVN canonicalization + Rio reversibility = auditable system
- Layers cooperate through unified surface
- Governance + replay capabilities combine powerfully

## Build & Run

```bash
make clean && make
./august_rio_compiler
./august_rio_compiler --debug
./august_rio_compiler --json
./august_rio_compiler --l5-enhanced  # Enable L5 homoiconic features
./august_rio_compiler examples/sample.rio --debug --json
./august_rio_compiler examples/hello_l5.moop --l5-enhanced --debug

# Environment variable (orthogonal flag)
export MOOP_L5_ENHANCED=1
./august_rio_compiler examples/advanced_l5.moop --debug

# Demonstrations
make hrir-demo          # L1 HRIR homoiconic reversible operations
make consistency-demo   # Dual-memory system validation
make python-demo        # Python bindings demonstration
```

## CLI Options

- `--debug` - Enable verbose debug output
- `--strict` - Enforce explicit D-term tagging
- `--json` - Output results in JSON format for tooling integration
- `--no-auto-hoist` - Disable automatic hierarchy generation
- `--no-reversible` - Disable reversible default
- `--l5-enhanced` - Enable L5 homoiconic features (orthogonal)
- `file.rio` or `file.moop` - Load and compile a .rio or .moop file

## Makefile Targets

- `make all` - Build the compiler
- `make test` - Run with default demo
- `make debug` - Run in debug mode
- `make json` - Run with JSON output
- `make compile-sample` - Compile examples/sample.rio with debug + json
- `make hrir-demo` - Build and run L1 HRIR demonstration
- `make consistency-demo` - Build and run dual-memory consistency checker
- `make python-demo` - Build and run Python bindings demonstration
- `make help` - Show available targets and options

## Embeddable API

Rio+RioVN provides a clean C API for embedding the compiler in other applications:

### API Libraries
- **`librio.so`** - Shared library for dynamic linking
- **`librio.a`** - Static library for static linking

### Basic API Usage
```c
#include "rio_api.h"

// Create VM
RioVM* vm = rio_create_vm();

// Compile source code
RioCompileOptions options = rio_default_options();
options.json_output = true;

RioResult* result = rio_compile_string(vm, "MathProto <- ObjectProto\nmath -> add 5 3", options);

// Check result
if (rio_result_success(result)) {
    // Access compilation outputs
    const char* json = rio_result_json_output(result);
    RioStats stats = rio_result_stats(result);

    // Homoiconic AST access
    RioAST* ast = rio_result_get_ast(result);
    RioASTNode node = rio_ast_get_node(ast, 0);
}

// Cleanup
rio_free_result(result);
rio_destroy_vm(vm);
```

### API Features
- **VM Lifecycle**: Create/destroy virtual machines
- **Compilation**: Compile strings or files with full option control
- **Result Access**: Get canonical code, JSON output, inheritance relations
- **Homoiconicity**: Runtime AST inspection and manipulation
- **Inheritance Registry**: Query prototype relationships
- **Canonical Paths**: Parse and validate Proto.Actor.Func paths
- **Memory Management**: Automatic cleanup with proper resource handling

### Building the API
```bash
make api              # Build both shared and static libraries
make api-example      # Build and run API usage example
```

### API vs CLI
| Feature | CLI | API |
|---------|-----|-----|
| **Usage** | Command-line tool | Library integration |
| **Output** | Files/stdout | Programmatic access |
| **Embedding** | N/A | Full C API |
| **Homoiconicity** | Limited | Full AST access |
| **Integration** | Shell scripts | Native code |
| **Performance** | Startup overhead | Direct calls |

## Examples

### Basic Usage
```javascript
// Reversible by default
MathProto <- ObjectProto
CalculatorProto <- MathProto
math -> add 5 3
calc -> multiply result 2
io -> output final
```

### D-term Boundaries
```javascript
// Explicit irreversible operations
@irreversible io -> writeToDisk data
@io network -> sendPacket buffer
```

### Governance
```bash
# Strict mode enforces explicit D-term tagging
./rio_riovn_compiler --strict
```

## Philosophy

### Reversible by Default
- All operations are reversible unless explicitly tagged
- State evolves via invertible transforms
- Support for time-travel debugging and rollback
- Information preservation as optimization

### Explicit D-term Boundaries
- Irreversible operations require `@irreversible` or `@io` tags
- D-term membranes log all irreversible changes
- Strict mode enforces explicit tagging
- Auditable coordination boundaries

### Unified Governance
- Canonical paths for reflection and policy
- Inheritance cycle detection and validation
- Ambiguity resolution with diagnostics
- Performance optimization through caching

## Development Status

🚀 **Bootloader Phase Complete** - M1 achieved with file loading, CLI, and JSON output.

### ✅ Completed (M1 - Bootloader)
- Unified surface layer with two-arrow syntax
- File-based loader for .rio files
- CLI argument parsing (--json, --strict, --debug, etc.)
- JSON output for tooling integration
- Canonical registry demo
- Memory-safe parsing and cleanup
- Inheritance relationship tracking
- Sample programs and examples

### ✅ **Just Completed - Full HRIR & Dual-Memory System**
- **L1 HRIR Implementation**: Complete homoiconic reversible intermediate representation
- **Dual-Memory Architecture**: L1 reversible tape + L2b D-term membranes with cross-linking
- **Time-Travel Debugging**: Checkpoint/rollback, actor snapshots, undo/redo
- **Consistency Validation**: Automated checking of L1→L0 side effect reproduction
- **Python Bindings**: Native Python FFI with exception handling and homoiconic access
- **Embeddable C API**: `librio.so` and `librio.a` with full programmatic control

### 🔄 Ready for M2 (Integration)
- **CMS Integration**: Hook into Swift/JavaScriptCore via JSON API
- **Language Bindings**: Python, JavaScript, Rust FFI bindings
- **IDE Integration**: Cursor/VSCode extensions for live feedback
- **Tooling Support**: LSP server for Rio+RioVN

### 🚧 Planned (M3 - Runtime)
- Actor/mailbox skeleton with RR scheduler
- Checkpoint API for reversible execution
- Replay driver for time-travel debugging
- Live programming capabilities

### 📋 Next Steps
- **Immediate**: Test and validate dual-memory consistency checker
- **Short Term**: CMS integration via Swift API with HRIR access
- **Medium Term**: Complete actor runtime with L3 time-travel debugging
- **Long Term**: IDE integration with visual time-travel debugging

## Contributing

This project embodies the philosophy of **minimization through conceptual unification, synergy, and orthogonality**. All changes must maintain:

1. **Minimal surface** - Keep syntax surface clean and minimal
2. **Conceptual unification** - Everything resolves to canonical paths
3. **Orthogonality** - Features vary independently
4. **Synergy** - Components work together harmoniously
5. **Reversibility default** - Explicit D-term boundaries only

The merged Rio+RioVN system represents the pinnacle of this philosophy - providing maximal expressive power through minimal, unified design.
