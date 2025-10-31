# L3 Turchin - LLM Quick Reference

**For LLMs generating actor-based code**

---

## 🚀 Minimal API (7 functions)

```c
#include "l3_llm_friendly.h"

// 1. Create runtime
ActorRuntime* rt = actor_runtime_create();

// 2. Parse actor
Actor* actor = actor_parse(code);

// 3. Spawn actor
int id = actor_spawn(rt, actor);

// 4. Send message
actor_send(rt, id, "event_name");

// 5. Process tick
actor_tick(rt);

// 6. Get state
const char* value = actor_get_state(rt, id, "key");

// 7. Free runtime
actor_runtime_free(rt);
```

---

## 📝 Actor Syntax (Natural Language)

```
actor <Name>
    role is "<description>"

    state has
        <key> is <value>
        <key> is <value>

    handlers

    on <event>
        <statements>

    on <event>
        <statements>
```

---

## 🔧 Control Flow

```
if <condition>
    <statements>

while <condition>
    <statements>

for <var> in <start> to <end>
    <statements>
```

---

## 📋 Statements

```
state.<key> = <value>
let <var> = <value>
log "<message>"
send <Actor> <event>
```

---

## 🎯 Expressions

| Type | Example |
|------|---------|
| Number | `42`, `3.14` |
| String | `"hello"` |
| State ref | `state.count` |
| Local var | `i`, `x` |
| Arithmetic | `state.x + 1` |
| Comparison | `state.x > 5` |

---

## 💡 Complete Example

```c
#include "l3_llm_friendly.h"
#include <stdio.h>

int main() {
    // Runtime
    ActorRuntime* rt = actor_runtime_create();

    // Actor code
    const char* code =
        "actor Counter\n"
        "    role is \"Count to 10\"\n"
        "    state has\n"
        "        count is 0\n"
        "    handlers\n"
        "    on start\n"
        "        while state.count < 10\n"
        "            state.count = state.count + 1\n";

    // Spawn
    Actor* counter = actor_parse(code);
    int id = actor_spawn(rt, counter);

    // Execute
    actor_send(rt, id, "start");
    actor_tick(rt);

    // Result
    printf("Count: %s\n", actor_get_state(rt, id, "count"));

    // Cleanup
    actor_runtime_free(rt);
    return 0;
}
```

---

## 🎨 Macros (Optional Convenience)

```c
ActorRuntime* rt = ACTOR_CREATE();
Actor* actor = ACTOR_PARSE(code);
int id = ACTOR_SPAWN(rt, actor);
ACTOR_SEND(rt, id, "event");
ACTOR_TICK(rt);
const char* val = ACTOR_GET(rt, id, "key");
ACTOR_FREE(rt);
```

---

## 🏷️ Type Aliases (Domain-Specific)

### Agent Systems
```c
Agent* agent = agent_parse(code);
int id = agent_spawn(rt, agent);
agent_send(rt, id, "task");
```

### Microservices
```c
Service* svc = service_parse(code);
int id = service_deploy(rt, svc);
service_call(rt, id, "request");
```

### Game Entities
```c
Entity* entity = entity_parse(code);
int id = entity_create(rt, entity);
entity_trigger(rt, id, "collision");
```

---

## 📦 Common Patterns

### Counter
```
actor Counter
    state has
        count is 0
    handlers
    on increment
        state.count = state.count + 1
```

### Ping-Pong
```
actor Pinger
    handlers
    on start
        send Ponger ping

actor Ponger
    handlers
    on ping
        log "Pong!"
```

### For Loop Sum
```
actor Summer
    state has
        total is 0
    handlers
    on calculate
        for i in 1 to 100
            state.total = state.total + i
```

### Conditional Logic
```
actor Guard
    state has
        allowed is false
    handlers
    on check_access
        if state.allowed
            log "Access granted"
        else
            log "Access denied"
```

### State Machine
```
actor Light
    state has
        status is "off"
    handlers
    on toggle
        if state.status == "off"
            state.status = "on"
        else
            state.status = "off"
```

---

## 🔄 Multi-Actor Communication

```c
// Define actors
const char* sender =
    "actor Sender\n"
    "    handlers\n"
    "    on start\n"
    "        send Receiver process\n";

const char* receiver =
    "actor Receiver\n"
    "    state has\n"
    "        received is 0\n"
    "    handlers\n"
    "    on process\n"
    "        state.received = 1\n";

// Spawn both
Actor* s = actor_parse(sender);
Actor* r = actor_parse(receiver);
int s_id = actor_spawn(rt, s);
int r_id = actor_spawn(rt, r);

// Execute
actor_send(rt, s_id, "start");
actor_tick(rt);  // Sender processes
actor_tick(rt);  // Receiver processes
```

---

## ⚡ One-Liner Execution

```c
// Execute code string directly
const char* result = actor_eval(
    "actor Quick\n"
    "    state has\n"
    "        result is 42\n"
    "    handlers\n"
    "    on run\n"
    "        state.result = state.result * 2\n"
);
```

---

## 🐛 Common Mistakes

### ❌ Wrong
```c
actor_tick();  // Missing runtime parameter
```

### ✅ Right
```c
actor_tick(rt);
```

---

### ❌ Wrong
```
on start
state.count = 1  # No indentation
```

### ✅ Right
```
on start
    state.count = 1  # Indented
```

---

### ❌ Wrong
```c
ACTOR_SEND(rt, id, increment);  // Missing quotes
```

### ✅ Right
```c
ACTOR_SEND(rt, id, "increment");  // Quoted
```

---

## 📊 Decision Tree: Which API?

```
Do you need reversibility/time-travel?
├─ YES → Use Moop Stack Subset (src/l3_moop_integration.h)
└─ NO  → Continue...
         │
         Is this for production Moop apps?
         ├─ YES → Use Moop Stack Subset
         └─ NO  → Use LLM-Friendly Subset (l3_llm_friendly.h) ✅
```

---

## 🎯 Template for LLMs

```c
#include "l3_llm_friendly.h"
#include <stdio.h>

int main() {
    ActorRuntime* rt = ACTOR_CREATE();

    // TODO: Define actor code here
    const char* code =
        "actor <NAME>\n"
        "    role is \"<DESCRIPTION>\"\n"
        "    state has\n"
        "        <KEY> is <VALUE>\n"
        "    handlers\n"
        "    on <EVENT>\n"
        "        <STATEMENTS>\n";

    Actor* actor = ACTOR_PARSE(code);
    int id = ACTOR_SPAWN(rt, actor);

    // TODO: Send events
    ACTOR_SEND(rt, id, "<EVENT>");
    ACTOR_TICK(rt);

    // TODO: Get results
    printf("Result: %s\n", ACTOR_GET(rt, id, "<KEY>"));

    ACTOR_FREE(rt);
    return 0;
}
```

---

## 🔗 Compilation

```bash
gcc -std=c99 -I. \
    your_program.c \
    l3_llm_friendly.c \
    src/l3_turchin.c \
    -o your_program
```

---

## ✨ That's It!

**7 functions. Natural syntax. Infinite possibilities.**

For detailed docs, see: `L3_TWO_VERSIONS.md`
