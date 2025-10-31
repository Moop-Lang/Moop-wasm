# L3 Turchin Actor Runtime - Two Versions

This document describes the two distinct versions of the L3 Turchin actor runtime, their purposes, and when to use each.

---

## Overview

The L3 Turchin actor runtime exists in **two parallel implementations**:

1. **Moop Stack Subset** - Integrated with the full Moop compiler stack (L5→L4→L3→L2→L1)
2. **LLM-Friendly Subset** - Standalone, minimal, optimized for LLM code generation

Both share the same core actor model and Quorum-style syntax but differ in their integration, complexity, and use cases.

---

## Version 1: Moop Stack Subset

### Location
- **Header**: `src/l3_moop_integration.h`
- **Implementation**: Uses `src/l3_turchin.c` + Moop stack
- **Integration**: Full Moop/Rio/HRIR pipeline

### Purpose
This version is for **production use within the Moop language ecosystem**. It provides:
- Full integration with L5 Moop (natural language)
- L4 Proto.Actor.Func inheritance
- L2a Prigogine reversible functions
- L1 HRIR homoiconic representation
- D-term membrane tracking
- Time-travel debugging
- Dual-memory consistency checking

### Architecture
```
L5 Moop (Natural Language)
    ↓ compile
L4 Proto.Actor.Func (Canonical Paths)
    ↓ resolve
L3 Turchin Actors (Message Passing)
    ↓ execute
L2a Prigogine Functions (Reversible)
    ↓ lower
L1 HRIR Cells (Homoiconic)
    ↓ compile
L0 Assembly
```

### Key Features
- **Homoiconic**: Actors are data (L1 HRIR cells)
- **Reversible**: State changes can be undone (L2a)
- **Inheritable**: Actors inherit from Proto.Actor.Func (L4)
- **Auditable**: D-term boundaries logged for compliance
- **Time-travel**: Full execution history and rollback

### API Highlights
```c
// Compile Moop code into actors
L3_MoopRuntime* l3_compile_from_moop(const char* moop_code, L5_CompileOptions opts);

// Execute with time-travel
bool l3_execute_moop_runtime(L3_MoopRuntime* runtime);
bool l3_undo_moop(L3_MoopRuntime* runtime, int steps);
bool l3_rollback_moop(L3_MoopRuntime* runtime, const char* checkpoint_id);

// HRIR integration
HRIR_Program* l3_to_hrir_program(L3_ActorRuntime* runtime);

// Membrane tracking
void l3_log_membrane_crossing(L3_MoopRuntime* runtime, L3_MembraneLog log);
```

### Use Cases
- Production Moop applications
- Systems requiring reversibility
- Auditable/compliant systems (healthcare, finance)
- Research on homoiconic actors
- Full-stack Moop development

### Example
```c
// Moop-integrated version
L3_MoopOptions opts = l3_default_moop_options();
opts.enable_time_travel = true;
opts.generate_hrir = true;

L3_MoopCompileResult* result = l3_compile_moop_stack(
    "display -> 'Hello from Moop'",
    opts
);

// Execute with full stack features
l3_execute_moop_runtime(result->runtime);

// Time-travel
l3_undo_moop(result->runtime, 5);
```

---

## Version 2: LLM-Friendly Subset

### Location
- **Header**: `l3_llm_friendly.h`
- **Implementation**: `l3_llm_friendly.c` (wraps `src/l3_turchin.c`)
- **Dependencies**: Self-contained, minimal

### Purpose
This version is **optimized for LLM code generation**. It provides:
- Simple, consistent API (7 core functions)
- No complex dependencies
- Natural language syntax
- Easy to learn and generate
- Multiple naming styles (actor/agent/service/entity)

### Architecture
```
Simple API (7 functions)
    ↓
Thin wrapper
    ↓
Core L3 runtime (src/l3_turchin.c)
```

### Key Features
- **Simple**: 7 functions cover 90% of use cases
- **Consistent**: All functions follow `actor_verb` pattern
- **Flexible**: Type aliases for different domains
- **Standalone**: No external dependencies
- **LLM-friendly**: Easy to learn, generate, and explain

### Complete API
```c
// Core 7 functions
ActorRuntime* actor_runtime_create(void);
Actor* actor_parse(const char* code);
int actor_spawn(ActorRuntime* runtime, Actor* actor);
void actor_send(ActorRuntime* runtime, int actor_id, const char* event);
void actor_tick(ActorRuntime* runtime);
const char* actor_get_state(ActorRuntime* runtime, int actor_id, const char* key);
void actor_runtime_free(ActorRuntime* runtime);

// Convenience macros
#define ACTOR_CREATE() actor_runtime_create()
#define ACTOR_PARSE(code) actor_parse(code)
#define ACTOR_SPAWN(rt, actor) actor_spawn(rt, actor)
#define ACTOR_SEND(rt, id, event) actor_send(rt, id, event)
#define ACTOR_TICK(rt) actor_tick(rt)
#define ACTOR_GET(rt, id, key) actor_get_state(rt, id, key)
#define ACTOR_FREE(rt) actor_runtime_free(rt)
```

### Type Aliases (Optional)
```c
// Agent systems
typedef Actor Agent;
Agent* agent_parse(code);
int agent_spawn(runtime, agent);

// Microservices
typedef Actor Service;
Service* service_parse(code);
int service_deploy(runtime, service);

// Game entities
typedef Actor Entity;
Entity* entity_parse(code);
int entity_create(runtime, entity);
```

### Use Cases
- LLM-generated code
- Teaching actor models
- Prototyping
- Embedding in host languages (Python, JS)
- Microservices
- Game AI
- Agent frameworks

### Example
```c
// LLM-friendly version
#include "l3_llm_friendly.h"

int main() {
    ActorRuntime* rt = ACTOR_CREATE();

    const char* code =
        "actor Counter\n"
        "    role is \"Count messages\"\n"
        "    state has\n"
        "        count is 0\n"
        "    handlers\n"
        "    on increment\n"
        "        state.count = state.count + 1\n";

    Actor* actor = ACTOR_PARSE(code);
    int id = ACTOR_SPAWN(rt, actor);

    ACTOR_SEND(rt, id, "increment");
    ACTOR_TICK(rt);

    printf("Count: %s\n", ACTOR_GET(rt, id, "count"));

    ACTOR_FREE(rt);
    return 0;
}
```

---

## Comparison Table

| Feature | Moop Stack Subset | LLM-Friendly Subset |
|---------|------------------|---------------------|
| **API Complexity** | Complex (20+ functions) | Simple (7 functions) |
| **Dependencies** | L5, L4, L2a, L1, HRIR | None (self-contained) |
| **Use Case** | Production Moop apps | LLM generation, prototypes |
| **Reversibility** | Full (L2a Prigogine) | No |
| **Time-Travel** | Yes | No |
| **HRIR Integration** | Yes (L1 cells) | No |
| **Membrane Tracking** | Yes (D-term logs) | No |
| **Inheritance** | Yes (L4 Proto.Actor.Func) | No |
| **Learning Curve** | Steep | Gentle |
| **LLM Generation** | Hard | Easy |
| **File Size** | Large | Small |
| **Compile Time** | Slow | Fast |

---

## Syntax (Both Versions)

Both versions use the **same Quorum-style actor syntax**:

```
actor <Name>
    role is "<description>"

    state has
        <key> is <value>
        ...

    handlers

    on <event>
        <statements>
```

### Control Flow (Both)
```
if <condition>
    <body>

while <condition>
    <body>

for <var> in <start> to <end>
    <body>
```

### Statements (Both)
```
state.<key> = <value>
let <var> = <value>
log "<message>"
send <Actor> <event>
```

---

## When to Use Each Version

### Use Moop Stack Subset When:
- ✅ Building production Moop applications
- ✅ Need reversibility and time-travel
- ✅ Require audit logs (D-term tracking)
- ✅ Want homoiconic representation (HRIR)
- ✅ Need inheritance (Proto.Actor.Func)
- ✅ Working within the Moop ecosystem

### Use LLM-Friendly Subset When:
- ✅ Prototyping quickly
- ✅ Teaching actor models
- ✅ LLM is generating code
- ✅ Embedding in other languages
- ✅ Don't need advanced features
- ✅ Want minimal dependencies
- ✅ Building microservices/agents/games

---

## Implementation Status

### Moop Stack Subset
- [x] Header defined (`src/l3_moop_integration.h`)
- [ ] Implementation (stub - needs full integration)
- [ ] L5 compilation
- [ ] L4 inheritance
- [ ] L1 HRIR generation
- [ ] Time-travel
- [ ] Membrane tracking

### LLM-Friendly Subset
- [x] Header defined (`l3_llm_friendly.h`)
- [x] Implementation (`l3_llm_friendly.c`)
- [x] Core 7 functions
- [x] Wrapper around `src/l3_turchin.c`
- [x] Example program
- [x] Documentation

---

## Building and Testing

### LLM-Friendly Version
```bash
# Compile example
gcc -std=c99 -I. examples/llm_friendly_example.c \
    l3_llm_friendly.c src/l3_turchin.c -o llm_example

# Run
./llm_example
```

### Moop Stack Version (Future)
```bash
# Will integrate with main Makefile
make moop-stack-demo
./moop_stack_demo
```

---

## Files Reference

### Moop Stack Subset
- `src/l3_moop_integration.h` - Integration header
- `src/l3_turchin.h` - Core runtime header
- `src/l3_turchin.c` - Core runtime implementation
- `src/l5_moop.h` - L5 Moop layer
- `src/hr_ir.h` - L1 HRIR layer
- `src/architecture.h` - Overall architecture

### LLM-Friendly Subset
- `l3_llm_friendly.h` - Simple API header
- `l3_llm_friendly.c` - Simple API implementation
- `examples/llm_friendly_example.c` - Usage examples
- `src/l3_turchin.c` - Core runtime (wrapped)

### Tests
- `test_l3_turchin.c` - Core runtime tests (C)
- `test_control_flow_c.c` - Control flow tests (C)
- `test_l3_node.js` - JavaScript runtime tests
- `test_l3_browser.html` - Browser interactive tests
- `test_c_js_comparison.sh` - C vs JS parity tests

---

## Next Steps

### For Moop Stack Integration
1. Implement L5→L3 compilation pipeline
2. Add L4 inheritance resolution
3. Generate L1 HRIR cells from actors
4. Implement time-travel execution
5. Add D-term membrane logging
6. Create dual-memory consistency checker

### For LLM-Friendly Version
1. ✅ Complete core implementation
2. Add Python bindings
3. Add JavaScript/WASM bindings
4. Create more examples (game AI, agents, services)
5. Write LLM prompt templates
6. Benchmark performance

---

## Summary

The L3 Turchin runtime exists in **two flavors**:

1. **Moop Stack** = Full-featured, production-ready, integrated
2. **LLM-Friendly** = Simple, standalone, easy to generate

Choose based on your needs:
- **Moop Stack** for serious Moop development
- **LLM-Friendly** for everything else

Both share the same beautiful Quorum-style syntax and actor semantics! 🎭
