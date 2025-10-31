# L3 Turchin Syntax Comparison

## Current Implementation vs. Design Spec

---

## ❌ Current Implementation (What we have now)

Uses `=` for assignment, traditional syntax:

```
actor Counter
    role is "Count messages"

    state has
        count is 0

    on increment
        state.count = state.count + 1
        log "Incremented"
```

**Issues:**
- Uses `=` instead of `->` for assignment
- Missing arrow operators
- No R-D-S tagging support
- Not consistent with Moop/Rio syntax

---

## ✅ Design Spec (What it should be)

Uses `->` for everything (arrow-based):

```
actor Counter
    role is "Count messages"

    state has
        count -> 0

    on increment
        state.count -> state.count + 1
        self -> log "Incremented"
```

**Features:**
- `->` for assignment AND messages
- `<-` for inheritance
- R-D-S tagging with `@io`, `@irreversible`
- Consistent with Moop/Rio stack
- Whitespace-based (no `end`)

---

## Key Syntax Rules

### 1. Assignment: Use `->` (not `=`)

❌ Wrong:
```
state.count = 10
let x = 5
```

✅ Right:
```
state.count -> 10
let x -> 5
```

### 2. Message Passing: Use `->`

❌ Wrong:
```
log "Hello"
send OtherActor process
```

✅ Right:
```
self -> log "Hello"
OtherActor -> process data
```

### 3. Inheritance: Use `<-`

❌ Wrong:
```
actor Counter extends BaseActor
```

✅ Right:
```
actor Counter <- BaseActor
```

### 4. D-term Tagging: Use `@io` or `@irreversible`

❌ Wrong (in strict mode):
```
display -> print result
```

✅ Right:
```
@io display -> print result
@irreversible file -> write "log.txt"
```

### 5. Blocks: Use indentation (no `end`)

❌ Wrong:
```
if count > 5
    log "Greater"
end
```

✅ Right:
```
if count > 5
    self -> log "Greater"
```

---

## Complete Example - Design Spec

```
actor Counter <- BaseActor
    role is "Advanced counter with logging"

    state has
        count -> 0
        max -> 100
        logs -> 0

    on start
        state.count -> 0
        self -> log "Counter started"

    on increment
        if state.count < state.max
            state.count -> state.count + 1
            self -> notify_change
        else
            @io display -> print "Max reached!"

    on notify_change
        let current -> state.count
        self -> log "Count is now" current
        state.logs -> state.logs + 1
        Logger -> record current

    on reset
        state.count -> 0
        self -> log "Counter reset"

actor Logger <- BaseActor
    state has
        events -> 0

    on record
        state.events -> state.events + 1
        @io file -> write "events.log"
```

---

## R-D-S Classification

### R-term (Reversible - default)
Everything uses `->` by default and is reversible:

```
state.count -> state.count + 1    // R-term
let x -> 10                        // R-term
self -> log "message"              // R-term
OtherActor -> process data         // R-term
```

### D-term (Dissipative - explicit tag)
Irreversible operations require tagging:

```
@io display -> print result
@irreversible file -> write "data.txt"
@io network -> sendHTTP url
```

### S-term (Structural)
Inheritance and structure:

```
actor Counter <- BaseActor         // S-term
Proto <- ParentProto               // S-term
```

---

## Grammar

```
Program       := Actor+
Actor         := "actor" Name ["<-" Parent] Block
Block         := IndentedLines
Statement     := Assignment | MessageSend | ControlFlow
Assignment    := Target "->" Expression
MessageSend   := [Tag] Target "->" Message [Args]
ControlFlow   := If | While | For
If            := "if" Condition Block
While         := "while" Condition Block
For           := "for" Var "in" Start "to" End Block
Tag           := "@io" | "@irreversible"
Target        := "state." Name | "self" | "let" Name | ActorName
```

---

## Migration Needed

To align current implementation with design spec:

1. **Parser changes:**
   - Accept `->` for assignment (currently uses `=`)
   - Parse `target -> message args` format
   - Support `@io` and `@irreversible` tags

2. **Syntax updates:**
   - `state.count = x` → `state.count -> x`
   - `log "msg"` → `self -> log "msg"`
   - Add D-term tag support

3. **Test updates:**
   - Rewrite all test cases with arrow syntax
   - Add R-D-S tagging tests

---

## Why Arrows?

1. **Consistency** - Same operator for all data flow
2. **Visual clarity** - `->` shows direction of data/control
3. **Moop integration** - Matches Rio/Moop stack syntax
4. **Simplicity** - One operator to rule them all
5. **LLM-friendly** - Easy pattern to learn and generate

The arrow `->` represents **flow**: data flows, messages flow, everything flows! 🏹
