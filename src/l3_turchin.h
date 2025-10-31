// src/l3_turchin.h
// L3 Turchin Actor Runtime - D-term Coordination
// Implements actor spawning, message passing, and handler execution

#ifndef L3_TURCHIN_H
#define L3_TURCHIN_H

#include <stdbool.h>
#include <stddef.h>

// =============================================================================
// ACTOR DEFINITION
// =============================================================================

typedef struct L3_ActorState {
    char** keys;           // State key names
    char** values;         // State values (as strings)
    size_t count;
    size_t capacity;
} L3_ActorState;

typedef struct L3_Handler {
    char* event_name;
    char* body_code;       // Handler body (to be compiled)
} L3_Handler;

typedef struct L3_ActorDefinition {
    char* name;
    char* role;
    L3_ActorState* initial_state;
    L3_Handler** handlers;
    size_t handler_count;
    size_t handler_capacity;
} L3_ActorDefinition;

// =============================================================================
// MESSAGE QUEUE
// =============================================================================

typedef struct L3_Message {
    char* event;
    char* data;            // JSON string of message data
    unsigned long timestamp;
} L3_Message;

typedef struct L3_MessageQueue {
    L3_Message** messages;
    size_t count;
    size_t capacity;
    size_t head;           // For circular buffer
    size_t tail;
} L3_MessageQueue;

// =============================================================================
// ACTOR INSTANCE
// =============================================================================

typedef struct L3_Actor {
    int id;
    char* name;
    char* role;
    L3_ActorState* state;
    L3_Handler** handlers;
    size_t handler_count;
} L3_Actor;

// =============================================================================
// ACTOR RUNTIME
// =============================================================================

typedef struct L3_ActorRuntime {
    L3_Actor** actors;
    size_t actor_count;
    size_t actor_capacity;

    L3_MessageQueue** message_queues;
    size_t queue_count;

    int next_actor_id;
    bool running;
} L3_ActorRuntime;

// =============================================================================
// PARSER
// =============================================================================

// Parse Quorum-style actor syntax into definition
L3_ActorDefinition* l3_parse_actor(const char* turchin_code);

// Free actor definition
void l3_free_actor_definition(L3_ActorDefinition* def);

// =============================================================================
// RUNTIME API
// =============================================================================

// Initialize runtime
L3_ActorRuntime* l3_runtime_init(void);

// Spawn an actor from definition
int l3_spawn_actor(L3_ActorRuntime* runtime, L3_ActorDefinition* def);

// Send message to actor
bool l3_send_message(L3_ActorRuntime* runtime, int actor_id, const char* event, const char* data);

// Process one message per actor (single tick)
void l3_tick(L3_ActorRuntime* runtime);

// Start/stop runtime
void l3_start_runtime(L3_ActorRuntime* runtime);
void l3_stop_runtime(L3_ActorRuntime* runtime);

// Get actor by name
int l3_get_actor_by_name(L3_ActorRuntime* runtime, const char* name);

// Cleanup
void l3_free_runtime(L3_ActorRuntime* runtime);

// =============================================================================
// STATE MANAGEMENT
// =============================================================================

// Create empty state
L3_ActorState* l3_state_create(void);

// Add key-value to state
bool l3_state_set(L3_ActorState* state, const char* key, const char* value);

// Get value from state
const char* l3_state_get(L3_ActorState* state, const char* key);

// Free state
void l3_state_free(L3_ActorState* state);

// =============================================================================
// HANDLER EXECUTION
// =============================================================================

typedef struct L3_ExecutionContext {
    L3_ActorState* state;      // Actor state (mutable)
    const char* data;          // Message data (immutable)
    L3_ActorRuntime* runtime;  // Runtime (for sending messages)
    int actor_id;              // Current actor ID

    // Local variables (scoped to handler)
    char** local_keys;
    char** local_values;
    size_t local_count;
    size_t local_capacity;
} L3_ExecutionContext;

// Execute handler body with context
void l3_execute_handler(const char* body, L3_ExecutionContext* ctx);

// =============================================================================
// EXPRESSION EVALUATION
// =============================================================================

// Evaluate expression (returns string representation)
char* l3_evaluate_expression(const char* expr, L3_ExecutionContext* ctx);

// Evaluate boolean condition
bool l3_evaluate_condition(const char* condition, L3_ExecutionContext* ctx);

#endif // L3_TURCHIN_H
