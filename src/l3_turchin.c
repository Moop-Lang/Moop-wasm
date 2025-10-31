// src/l3_turchin.c
// L3 Turchin Actor Runtime Implementation
// D-term coordination: Message passing, supervision, error handling

#include "l3_turchin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

static char* l3_strdup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* copy = malloc(len + 1);
    if (copy) strcpy(copy, str);
    return copy;
}

static char* l3_trim(const char* str) {
    if (!str) return NULL;

    // Skip leading whitespace
    while (*str && isspace(*str)) str++;

    if (*str == '\0') return l3_strdup("");

    // Find end
    const char* end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;

    size_t len = end - str + 1;
    char* trimmed = malloc(len + 1);
    if (trimmed) {
        memcpy(trimmed, str, len);
        trimmed[len] = '\0';
    }
    return trimmed;
}

static bool l3_starts_with(const char* str, const char* prefix) {
    if (!str || !prefix) return false;
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

static char* l3_substring(const char* str, size_t start, size_t len) {
    if (!str) return NULL;
    size_t str_len = strlen(str);
    if (start >= str_len) return l3_strdup("");

    if (start + len > str_len) len = str_len - start;

    char* sub = malloc(len + 1);
    if (sub) {
        memcpy(sub, str + start, len);
        sub[len] = '\0';
    }
    return sub;
}

// =============================================================================
// STATE MANAGEMENT
// =============================================================================

L3_ActorState* l3_state_create(void) {
    L3_ActorState* state = malloc(sizeof(L3_ActorState));
    if (!state) return NULL;

    state->capacity = 16;
    state->count = 0;
    state->keys = malloc(state->capacity * sizeof(char*));
    state->values = malloc(state->capacity * sizeof(char*));

    if (!state->keys || !state->values) {
        free(state->keys);
        free(state->values);
        free(state);
        return NULL;
    }

    return state;
}

bool l3_state_set(L3_ActorState* state, const char* key, const char* value) {
    if (!state || !key) return false;

    // Check if key exists
    for (size_t i = 0; i < state->count; i++) {
        if (strcmp(state->keys[i], key) == 0) {
            // Update existing
            free(state->values[i]);
            state->values[i] = l3_strdup(value);
            return true;
        }
    }

    // Add new key-value
    if (state->count >= state->capacity) {
        // Expand
        size_t new_cap = state->capacity * 2;
        char** new_keys = realloc(state->keys, new_cap * sizeof(char*));
        char** new_values = realloc(state->values, new_cap * sizeof(char*));

        if (!new_keys || !new_values) return false;

        state->keys = new_keys;
        state->values = new_values;
        state->capacity = new_cap;
    }

    state->keys[state->count] = l3_strdup(key);
    state->values[state->count] = l3_strdup(value);
    state->count++;

    return true;
}

const char* l3_state_get(L3_ActorState* state, const char* key) {
    if (!state || !key) return NULL;

    for (size_t i = 0; i < state->count; i++) {
        if (strcmp(state->keys[i], key) == 0) {
            return state->values[i];
        }
    }

    return NULL;
}

void l3_state_free(L3_ActorState* state) {
    if (!state) return;

    for (size_t i = 0; i < state->count; i++) {
        free(state->keys[i]);
        free(state->values[i]);
    }

    free(state->keys);
    free(state->values);
    free(state);
}

// =============================================================================
// MESSAGE QUEUE
// =============================================================================

static L3_MessageQueue* l3_queue_create(void) {
    L3_MessageQueue* queue = malloc(sizeof(L3_MessageQueue));
    if (!queue) return NULL;

    queue->capacity = 64;
    queue->count = 0;
    queue->head = 0;
    queue->tail = 0;
    queue->messages = malloc(queue->capacity * sizeof(L3_Message*));

    if (!queue->messages) {
        free(queue);
        return NULL;
    }

    return queue;
}

static bool l3_queue_push(L3_MessageQueue* queue, const char* event, const char* data) {
    if (!queue) return false;

    if (queue->count >= queue->capacity) {
        // Expand queue
        size_t new_cap = queue->capacity * 2;
        L3_Message** new_messages = malloc(new_cap * sizeof(L3_Message*));
        if (!new_messages) return false;

        // Copy existing messages
        for (size_t i = 0; i < queue->count; i++) {
            new_messages[i] = queue->messages[(queue->head + i) % queue->capacity];
        }

        free(queue->messages);
        queue->messages = new_messages;
        queue->capacity = new_cap;
        queue->head = 0;
        queue->tail = queue->count;
    }

    L3_Message* msg = malloc(sizeof(L3_Message));
    if (!msg) return false;

    msg->event = l3_strdup(event);
    msg->data = l3_strdup(data ? data : "{}");
    msg->timestamp = (unsigned long)time(NULL);

    queue->messages[queue->tail] = msg;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;

    return true;
}

static L3_Message* l3_queue_pop(L3_MessageQueue* queue) {
    if (!queue || queue->count == 0) return NULL;

    L3_Message* msg = queue->messages[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;

    return msg;
}

static void l3_queue_free(L3_MessageQueue* queue) {
    if (!queue) return;

    while (queue->count > 0) {
        L3_Message* msg = l3_queue_pop(queue);
        if (msg) {
            free(msg->event);
            free(msg->data);
            free(msg);
        }
    }

    free(queue->messages);
    free(queue);
}

// =============================================================================
// RUNTIME INITIALIZATION
// =============================================================================

L3_ActorRuntime* l3_runtime_init(void) {
    L3_ActorRuntime* runtime = malloc(sizeof(L3_ActorRuntime));
    if (!runtime) return NULL;

    runtime->actor_capacity = 32;
    runtime->actor_count = 0;
    runtime->actors = malloc(runtime->actor_capacity * sizeof(L3_Actor*));

    runtime->queue_count = 0;
    runtime->message_queues = malloc(runtime->actor_capacity * sizeof(L3_MessageQueue*));

    runtime->next_actor_id = 1;
    runtime->running = false;

    if (!runtime->actors || !runtime->message_queues) {
        free(runtime->actors);
        free(runtime->message_queues);
        free(runtime);
        return NULL;
    }

    printf("✨ L3 Turchin: Runtime initialized\n");

    return runtime;
}

void l3_start_runtime(L3_ActorRuntime* runtime) {
    if (runtime) {
        runtime->running = true;
        printf("▶️  L3 Turchin: Runtime started\n");
    }
}

void l3_stop_runtime(L3_ActorRuntime* runtime) {
    if (runtime) {
        runtime->running = false;
        printf("⏸️  L3 Turchin: Runtime stopped\n");
    }
}

// =============================================================================
// ACTOR SPAWNING
// =============================================================================

int l3_spawn_actor(L3_ActorRuntime* runtime, L3_ActorDefinition* def) {
    if (!runtime || !def) return -1;

    // Expand if needed
    if (runtime->actor_count >= runtime->actor_capacity) {
        size_t new_cap = runtime->actor_capacity * 2;
        L3_Actor** new_actors = realloc(runtime->actors, new_cap * sizeof(L3_Actor*));
        L3_MessageQueue** new_queues = realloc(runtime->message_queues, new_cap * sizeof(L3_MessageQueue*));

        if (!new_actors || !new_queues) return -1;

        runtime->actors = new_actors;
        runtime->message_queues = new_queues;
        runtime->actor_capacity = new_cap;
    }

    // Create actor instance
    L3_Actor* actor = malloc(sizeof(L3_Actor));
    if (!actor) return -1;

    actor->id = runtime->next_actor_id++;
    actor->name = l3_strdup(def->name);
    actor->role = l3_strdup(def->role);

    // Copy initial state
    actor->state = l3_state_create();
    for (size_t i = 0; i < def->initial_state->count; i++) {
        l3_state_set(actor->state,
                     def->initial_state->keys[i],
                     def->initial_state->values[i]);
    }

    // Copy handlers
    actor->handler_count = def->handler_count;
    actor->handlers = malloc(actor->handler_count * sizeof(L3_Handler*));
    for (size_t i = 0; i < def->handler_count; i++) {
        actor->handlers[i] = malloc(sizeof(L3_Handler));
        actor->handlers[i]->event_name = l3_strdup(def->handlers[i]->event_name);
        actor->handlers[i]->body_code = l3_strdup(def->handlers[i]->body_code);
    }

    // Create message queue
    L3_MessageQueue* queue = l3_queue_create();

    // Add to runtime
    runtime->actors[runtime->actor_count] = actor;
    runtime->message_queues[runtime->actor_count] = queue;
    runtime->actor_count++;
    runtime->queue_count++;

    printf("🎭 Spawned actor: %s (id: %d, role: \"%s\")\n",
           actor->name, actor->id, actor->role);

    return actor->id;
}

// =============================================================================
// MESSAGE SENDING
// =============================================================================

bool l3_send_message(L3_ActorRuntime* runtime, int actor_id, const char* event, const char* data) {
    if (!runtime || !event) return false;

    // Find actor index
    int actor_index = -1;
    for (size_t i = 0; i < runtime->actor_count; i++) {
        if (runtime->actors[i]->id == actor_id) {
            actor_index = i;
            break;
        }
    }

    if (actor_index == -1) {
        printf("❌ Actor %d not found\n", actor_id);
        return false;
    }

    L3_MessageQueue* queue = runtime->message_queues[actor_index];
    bool success = l3_queue_push(queue, event, data);

    if (success) {
        printf("📨 Sent %s to actor %d\n", event, actor_id);
    }

    return success;
}

int l3_get_actor_by_name(L3_ActorRuntime* runtime, const char* name) {
    if (!runtime || !name) return -1;

    for (size_t i = 0; i < runtime->actor_count; i++) {
        if (strcmp(runtime->actors[i]->name, name) == 0) {
            return runtime->actors[i]->id;
        }
    }

    return -1;
}

// =============================================================================
// TICK (Process one message per actor)
// =============================================================================

void l3_tick(L3_ActorRuntime* runtime) {
    if (!runtime) return;

    for (size_t i = 0; i < runtime->actor_count; i++) {
        L3_MessageQueue* queue = runtime->message_queues[i];
        if (queue->count > 0) {
            L3_Message* msg = l3_queue_pop(queue);
            if (msg) {
                L3_Actor* actor = runtime->actors[i];

                // Find handler
                L3_Handler* handler = NULL;
                for (size_t h = 0; h < actor->handler_count; h++) {
                    if (strcmp(actor->handlers[h]->event_name, msg->event) == 0) {
                        handler = actor->handlers[h];
                        break;
                    }
                }

                if (handler) {
                    printf("🎬 Actor %s handling: %s\n", actor->name, msg->event);

                    // Create execution context
                    L3_ExecutionContext ctx;
                    ctx.state = actor->state;
                    ctx.data = msg->data;
                    ctx.runtime = runtime;
                    ctx.actor_id = actor->id;
                    ctx.local_capacity = 16;
                    ctx.local_count = 0;
                    ctx.local_keys = malloc(ctx.local_capacity * sizeof(char*));
                    ctx.local_values = malloc(ctx.local_capacity * sizeof(char*));

                    // Execute handler
                    l3_execute_handler(handler->body_code, &ctx);

                    // Free local variables
                    for (size_t l = 0; l < ctx.local_count; l++) {
                        free(ctx.local_keys[l]);
                        free(ctx.local_values[l]);
                    }
                    free(ctx.local_keys);
                    free(ctx.local_values);
                } else {
                    printf("⚠️  No handler for %s on %s\n", msg->event, actor->name);
                }

                // Free message
                free(msg->event);
                free(msg->data);
                free(msg);
            }
        }
    }
}

// =============================================================================
// CLEANUP
// =============================================================================

void l3_free_actor_definition(L3_ActorDefinition* def) {
    if (!def) return;

    free(def->name);
    free(def->role);

    if (def->initial_state) {
        l3_state_free(def->initial_state);
    }

    for (size_t i = 0; i < def->handler_count; i++) {
        if (def->handlers[i]) {
            free(def->handlers[i]->event_name);
            free(def->handlers[i]->body_code);
            free(def->handlers[i]);
        }
    }
    free(def->handlers);
    free(def);
}

void l3_free_runtime(L3_ActorRuntime* runtime) {
    if (!runtime) return;

    for (size_t i = 0; i < runtime->actor_count; i++) {
        L3_Actor* actor = runtime->actors[i];
        if (actor) {
            free(actor->name);
            free(actor->role);
            l3_state_free(actor->state);

            for (size_t h = 0; h < actor->handler_count; h++) {
                if (actor->handlers[h]) {
                    free(actor->handlers[h]->event_name);
                    free(actor->handlers[h]->body_code);
                    free(actor->handlers[h]);
                }
            }
            free(actor->handlers);
            free(actor);
        }
    }

    for (size_t i = 0; i < runtime->queue_count; i++) {
        l3_queue_free(runtime->message_queues[i]);
    }

    free(runtime->actors);
    free(runtime->message_queues);
    free(runtime);
}

// =============================================================================
// PARSER - Quorum-style syntax
// =============================================================================

L3_ActorDefinition* l3_parse_actor(const char* turchin_code) {
    if (!turchin_code) return NULL;

    L3_ActorDefinition* def = malloc(sizeof(L3_ActorDefinition));
    if (!def) return NULL;

    def->name = NULL;
    def->role = NULL;
    def->initial_state = l3_state_create();
    def->handler_capacity = 8;
    def->handler_count = 0;
    def->handlers = malloc(def->handler_capacity * sizeof(L3_Handler*));

    char* code_copy = l3_strdup(turchin_code);
    char* line = strtok(code_copy, "\n");

    typedef enum { MODE_NONE, MODE_STATE, MODE_HANDLERS } ParseMode;
    ParseMode mode = MODE_NONE;

    char* current_handler_name = NULL;
    char* current_handler_body = NULL;
    size_t current_handler_body_len = 0;

    while (line != NULL) {
        char* trimmed = l3_trim(line);

        if (strlen(trimmed) == 0 || l3_starts_with(trimmed, "//")) {
            free(trimmed);
            line = strtok(NULL, "\n");
            continue;
        }

        // actor <Name>
        if (l3_starts_with(trimmed, "actor ")) {
            def->name = l3_substring(trimmed, 6, strlen(trimmed) - 6);
            char* name_trimmed = l3_trim(def->name);
            free(def->name);
            def->name = name_trimmed;
        }
        // role is "<description>"
        else if (l3_starts_with(trimmed, "role is ")) {
            char* role_str = l3_substring(trimmed, 8, strlen(trimmed) - 8);
            char* role_trimmed = l3_trim(role_str);
            free(role_str);

            // Remove quotes
            if (role_trimmed[0] == '"' || role_trimmed[0] == '\'') {
                size_t len = strlen(role_trimmed);
                def->role = l3_substring(role_trimmed, 1, len - 2);
                free(role_trimmed);
            } else {
                def->role = role_trimmed;
            }
        }
        // state has
        else if (strcmp(trimmed, "state has") == 0) {
            mode = MODE_STATE;
        }
        // handlers
        else if (strcmp(trimmed, "handlers") == 0) {
            mode = MODE_HANDLERS;

            // Finish current handler if any
            if (current_handler_name && current_handler_body) {
                if (def->handler_count >= def->handler_capacity) {
                    def->handler_capacity *= 2;
                    def->handlers = realloc(def->handlers, def->handler_capacity * sizeof(L3_Handler*));
                }

                L3_Handler* h = malloc(sizeof(L3_Handler));
                h->event_name = current_handler_name;
                h->body_code = current_handler_body;
                def->handlers[def->handler_count++] = h;

                current_handler_name = NULL;
                current_handler_body = NULL;
            }
        }
        // State: <key> is <value> or <key> -> <value>
        else if (mode == MODE_STATE && (strstr(trimmed, " is ") || strstr(trimmed, "->"))) {
            char* sep_pos = strstr(trimmed, "->");
            size_t sep_len = 2;

            if (!sep_pos) {
                sep_pos = strstr(trimmed, " is ");
                sep_len = 4;
            }

            size_t key_len = sep_pos - trimmed;
            char* key = l3_substring(trimmed, 0, key_len);
            char* key_trim = l3_trim(key);
            free(key);

            char* value_start = sep_pos + sep_len;
            char* value = l3_trim(value_start);

            // Parse value (simple parsing)
            if ((value[0] == '"' || value[0] == '\'') && strlen(value) > 1) {
                // String - remove quotes
                char* unquoted = l3_substring(value, 1, strlen(value) - 2);
                l3_state_set(def->initial_state, key_trim, unquoted);
                free(unquoted);
            } else {
                // Number or boolean
                l3_state_set(def->initial_state, key_trim, value);
            }

            free(key_trim);
            free(value);
        }
        // Handler: on <event>
        else if (l3_starts_with(trimmed, "on ")) {
            // Save previous handler
            if (current_handler_name && current_handler_body) {
                if (def->handler_count >= def->handler_capacity) {
                    def->handler_capacity *= 2;
                    def->handlers = realloc(def->handlers, def->handler_capacity * sizeof(L3_Handler*));
                }

                L3_Handler* h = malloc(sizeof(L3_Handler));
                h->event_name = current_handler_name;
                h->body_code = current_handler_body;
                def->handlers[def->handler_count++] = h;
            }

            // Start new handler
            char* event_name = l3_substring(trimmed, 3, strlen(trimmed) - 3);
            current_handler_name = l3_trim(event_name);
            free(event_name);

            current_handler_body = malloc(1);
            current_handler_body[0] = '\0';
            current_handler_body_len = 0;
        }
        // Handler body line
        else if (mode == MODE_HANDLERS && current_handler_name) {
            // Append to handler body
            size_t line_len = strlen(trimmed);
            size_t new_len = current_handler_body_len + line_len + 2; // +2 for \n and \0

            current_handler_body = realloc(current_handler_body, new_len);
            if (current_handler_body_len > 0) {
                strcat(current_handler_body, "\n");
            }
            strcat(current_handler_body, trimmed);
            current_handler_body_len = new_len - 1;
        }

        free(trimmed);
        line = strtok(NULL, "\n");
    }

    // Save last handler
    if (current_handler_name && current_handler_body) {
        if (def->handler_count >= def->handler_capacity) {
            def->handler_capacity *= 2;
            def->handlers = realloc(def->handlers, def->handler_capacity * sizeof(L3_Handler*));
        }

        L3_Handler* h = malloc(sizeof(L3_Handler));
        h->event_name = current_handler_name;
        h->body_code = current_handler_body;
        def->handlers[def->handler_count++] = h;
    }

    free(code_copy);

    printf("✅ Parsed actor: %s\n", def->name ? def->name : "(unnamed)");
    printf("   Role: %s\n", def->role ? def->role : "(no role)");
    printf("   State keys: %zu\n", def->initial_state->count);
    printf("   Handlers: %zu\n", def->handler_count);

    return def;
}

// =============================================================================
// EXPRESSION EVALUATION
// =============================================================================

static bool l3_is_number(const char* str) {
    if (!str || *str == '\0') return false;

    if (*str == '-' || *str == '+') str++;

    bool has_digit = false;
    bool has_dot = false;

    while (*str) {
        if (isdigit(*str)) {
            has_digit = true;
        } else if (*str == '.' && !has_dot) {
            has_dot = true;
        } else {
            return false;
        }
        str++;
    }

    return has_digit;
}

static void l3_set_local(L3_ExecutionContext* ctx, const char* key, const char* value) {
    if (!ctx || !key) return;

    // Check if exists
    for (size_t i = 0; i < ctx->local_count; i++) {
        if (strcmp(ctx->local_keys[i], key) == 0) {
            free(ctx->local_values[i]);
            ctx->local_values[i] = l3_strdup(value);
            return;
        }
    }

    // Add new
    if (ctx->local_count >= ctx->local_capacity) {
        ctx->local_capacity *= 2;
        ctx->local_keys = realloc(ctx->local_keys, ctx->local_capacity * sizeof(char*));
        ctx->local_values = realloc(ctx->local_values, ctx->local_capacity * sizeof(char*));
    }

    ctx->local_keys[ctx->local_count] = l3_strdup(key);
    ctx->local_values[ctx->local_count] = l3_strdup(value);
    ctx->local_count++;
}

static const char* l3_get_local(L3_ExecutionContext* ctx, const char* key) {
    if (!ctx || !key) return NULL;

    for (size_t i = 0; i < ctx->local_count; i++) {
        if (strcmp(ctx->local_keys[i], key) == 0) {
            return ctx->local_values[i];
        }
    }

    return NULL;
}

char* l3_evaluate_expression(const char* expr, L3_ExecutionContext* ctx) {
    if (!expr) return l3_strdup("");

    char* trimmed = l3_trim(expr);

    // String literal
    if (trimmed[0] == '"' || trimmed[0] == '\'') {
        char* result = l3_substring(trimmed, 1, strlen(trimmed) - 2);
        free(trimmed);
        return result;
    }

    // Number literal
    if (l3_is_number(trimmed)) {
        return trimmed; // Already allocated
    }

    // Boolean literal
    if (strcmp(trimmed, "true") == 0 || strcmp(trimmed, "false") == 0) {
        return trimmed;
    }

    // Arithmetic expression (contains operators) - CHECK THIS FIRST!
    if (strpbrk(trimmed, "+-*/()")) {
        // Replace state references
        char* replaced = l3_strdup(trimmed);

        // Simple replacement: state.X → value
        char* state_pos = strstr(replaced, "state.");
        while (state_pos) {
            // Find end of variable name
            char* var_start = state_pos + 6;
            char* var_end = var_start;
            while (*var_end && (isalnum(*var_end) || *var_end == '_')) var_end++;

            size_t var_len = var_end - var_start;
            char* var_name = l3_substring(var_start, 0, var_len);

            const char* var_value = l3_state_get(ctx->state, var_name);
            const char* value_str = var_value ? var_value : "0";

            // Build replacement
            size_t before_len = state_pos - replaced;
            size_t after_len = strlen(var_end);
            size_t new_len = before_len + strlen(value_str) + after_len + 1;

            char* new_replaced = malloc(new_len);
            memcpy(new_replaced, replaced, before_len);
            strcpy(new_replaced + before_len, value_str);
            strcat(new_replaced, var_end);

            free(var_name);
            free(replaced);
            replaced = new_replaced;

            state_pos = strstr(replaced, "state.");
        }

        // Replace local variables
        for (size_t i = 0; i < ctx->local_count; i++) {
            // Simple word boundary replacement
            char search[256];
            snprintf(search, sizeof(search), "%s", ctx->local_keys[i]);

            char* pos = strstr(replaced, search);
            if (pos) {
                // Check word boundaries
                bool is_start = (pos == replaced || !isalnum(*(pos - 1)));
                bool is_end = !isalnum(pos[strlen(search)]);

                if (is_start && is_end) {
                    // Replace
                    size_t before_len = pos - replaced;
                    size_t after_len = strlen(pos + strlen(search));
                    const char* value_str = ctx->local_values[i];
                    size_t new_len = before_len + strlen(value_str) + after_len + 1;

                    char* new_replaced = malloc(new_len);
                    memcpy(new_replaced, replaced, before_len);
                    strcpy(new_replaced + before_len, value_str);
                    strcat(new_replaced, pos + strlen(search));

                    free(replaced);
                    replaced = new_replaced;
                }
            }
        }

        // Simple evaluation (only numbers and operators)
        // For production, use proper expression parser
        // For now, use system() with bc calculator (Unix)
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "echo '%s' | bc 2>/dev/null", replaced);

        FILE* pipe = popen(cmd, "r");
        if (pipe) {
            char result[128];
            if (fgets(result, sizeof(result), pipe)) {
                // Remove newline
                size_t len = strlen(result);
                if (len > 0 && result[len - 1] == '\n') {
                    result[len - 1] = '\0';
                }

                pclose(pipe);
                free(replaced);
                free(trimmed);
                return l3_strdup(result);
            }
            pclose(pipe);
        }

        free(replaced);
    }

    // state.key (only if not arithmetic)
    if (l3_starts_with(trimmed, "state.")) {
        const char* key = trimmed + 6;
        const char* value = l3_state_get(ctx->state, key);
        free(trimmed);
        return value ? l3_strdup(value) : l3_strdup("0");
    }

    // Local variable lookup (only if not arithmetic)
    const char* local_val = l3_get_local(ctx, trimmed);
    if (local_val) {
        char* result = l3_strdup(local_val);
        free(trimmed);
        return result;
    }

    free(trimmed);
    return l3_strdup(expr);
}

bool l3_evaluate_condition(const char* condition, L3_ExecutionContext* ctx) {
    if (!condition) return false;

    char* trimmed = l3_trim(condition);

    // Handle comparisons
    const char* operators[] = {"<=", ">=", "==", "!=", "<", ">", NULL};

    for (int i = 0; operators[i]; i++) {
        char* op_pos = strstr(trimmed, operators[i]);
        if (op_pos) {
            size_t left_len = op_pos - trimmed;
            char* left_str = l3_substring(trimmed, 0, left_len);
            char* left_trimmed = l3_trim(left_str);
            free(left_str);

            char* right = op_pos + strlen(operators[i]);
            char* right_trimmed = l3_trim(right);

            char* left_val = l3_evaluate_expression(left_trimmed, ctx);
            char* right_val = l3_evaluate_expression(right_trimmed, ctx);

            double left_num = atof(left_val);
            double right_num = atof(right_val);

            bool result = false;
            if (strcmp(operators[i], "<") == 0) result = left_num < right_num;
            else if (strcmp(operators[i], ">") == 0) result = left_num > right_num;
            else if (strcmp(operators[i], "<=") == 0) result = left_num <= right_num;
            else if (strcmp(operators[i], ">=") == 0) result = left_num >= right_num;
            else if (strcmp(operators[i], "==") == 0) result = (left_num == right_num) || (strcmp(left_val, right_val) == 0);
            else if (strcmp(operators[i], "!=") == 0) result = (left_num != right_num) && (strcmp(left_val, right_val) != 0);

            free(left_trimmed);
            free(right_trimmed);
            free(left_val);
            free(right_val);
            free(trimmed);

            return result;
        }
    }

    // Boolean value
    char* val = l3_evaluate_expression(trimmed, ctx);
    bool result = (strcmp(val, "true") == 0) || (atoi(val) != 0);
    free(val);
    free(trimmed);

    return result;
}

// =============================================================================
// HANDLER EXECUTION (with control flow)
// =============================================================================

// Forward declaration
static void l3_execute_block(char** lines, size_t* line_idx, size_t line_count,
                             int base_indent, L3_ExecutionContext* ctx);

static int l3_get_indent(const char* line) {
    int indent = 0;
    while (line[indent] == ' ') indent++;
    return indent;
}

static void l3_execute_line(const char* trimmed, L3_ExecutionContext* ctx) {
    // self -> <message> [args] (arrow message to self)
    if (l3_starts_with(trimmed, "self ->")) {
        char* message_part = l3_trim(trimmed + 7);
        // For now, just handle log specially
        if (l3_starts_with(message_part, "log ")) {
            char* msg = l3_trim(message_part + 4);
            if (msg[0] == '"' || msg[0] == '\'') {
                size_t len = strlen(msg);
                char* unquoted = l3_substring(msg, 1, len - 2);
                printf("   📝 %s\n", unquoted);
                free(unquoted);
            } else {
                printf("   📝 %s\n", msg);
            }
            free(msg);
        }
        free(message_part);
    }
    // <Actor> -> <message> [args] (arrow message to actor)
    else if (strstr(trimmed, "->") && isupper(trimmed[0])) {
        char* arrow_pos = strstr(trimmed, "->");
        size_t target_len = arrow_pos - trimmed;
        char* target = l3_substring(trimmed, 0, target_len);
        char* target_trimmed = l3_trim(target);
        free(target);

        char* message_part = l3_trim(arrow_pos + 2);

        // Extract message name (first word)
        char* space_pos = strchr(message_part, ' ');
        char* message_name;
        if (space_pos) {
            size_t msg_len = space_pos - message_part;
            message_name = l3_substring(message_part, 0, msg_len);
        } else {
            message_name = l3_strdup(message_part);
        }

        // Message to another actor
        int target_id = l3_get_actor_by_name(ctx->runtime, target_trimmed);
        if (target_id != -1) {
            printf("   → %s -> %s\n", target_trimmed, message_name);
            l3_send_message(ctx->runtime, target_id, message_name, ctx->data);
        } else {
            printf("❌ Target actor \"%s\" not found\n", target_trimmed);
        }

        free(target_trimmed);
        free(message_part);
        free(message_name);
    }
    // let <var> -> <value> (arrow syntax)
    else if (l3_starts_with(trimmed, "let ")) {
        char* rest = trimmed + 4;
        char* arrow_pos = strstr(rest, "->");

        if (arrow_pos) {
            size_t var_len = arrow_pos - rest;
            char* var_name = l3_substring(rest, 0, var_len);
            char* var_trimmed = l3_trim(var_name);
            free(var_name);

            char* value_expr = l3_trim(arrow_pos + 2);
            char* value = l3_evaluate_expression(value_expr, ctx);

            l3_set_local(ctx, var_trimmed, value);
            printf("   📦 Let %s -> %s\n", var_trimmed, value);

            free(var_trimmed);
            free(value_expr);
            free(value);
        }
    }
    // <var> -> <value> or state.<key> -> <value> (arrow assignment)
    else if (strstr(trimmed, "->") && !l3_starts_with(trimmed, "let ") &&
             !l3_starts_with(trimmed, "if ") && !l3_starts_with(trimmed, "while ")) {
        char* arrow_pos = strstr(trimmed, "->");
        size_t target_len = arrow_pos - trimmed;
        char* target = l3_substring(trimmed, 0, target_len);
        char* target_trimmed = l3_trim(target);
        free(target);

        char* value_expr = l3_trim(arrow_pos + 2);
        char* value = l3_evaluate_expression(value_expr, ctx);

        if (l3_starts_with(target_trimmed, "state.")) {
            const char* key = target_trimmed + 6;
            l3_state_set(ctx->state, key, value);
            printf("   ✏️  Set state.%s -> %s\n", key, value);
        } else {
            l3_set_local(ctx, target_trimmed, value);
            printf("   📦 Set %s -> %s\n", target_trimmed, value);
        }

        free(target_trimmed);
        free(value_expr);
        free(value);
    }
    // log <message>
    else if (l3_starts_with(trimmed, "log ")) {
        char* message = trimmed + 4;
        char* msg_trimmed = l3_trim(message);

        // Remove quotes
        if (msg_trimmed[0] == '"' || msg_trimmed[0] == '\'') {
            size_t len = strlen(msg_trimmed);
            char* unquoted = l3_substring(msg_trimmed, 1, len - 2);
            printf("   📝 %s\n", unquoted);
            free(unquoted);
        } else {
            printf("   📝 %s\n", msg_trimmed);
        }

        free(msg_trimmed);
    }
}

static void l3_execute_block(char** lines, size_t* line_idx, size_t line_count,
                             int base_indent, L3_ExecutionContext* ctx) {
    while (*line_idx < line_count) {
        char* line = lines[*line_idx];
        char* trimmed = l3_trim(line);

        if (strlen(trimmed) == 0 || l3_starts_with(trimmed, "//")) {
            free(trimmed);
            (*line_idx)++;
            continue;
        }

        int indent = l3_get_indent(line);

        // If dedented past our level, stop
        if (indent < base_indent && strlen(trimmed) > 0) {
            free(trimmed);
            break;
        }

        // if <condition>
        if (l3_starts_with(trimmed, "if ")) {
            char* condition = l3_trim(trimmed + 3);
            bool result = l3_evaluate_condition(condition, ctx);
            free(condition);

            (*line_idx)++;

            // Collect if-block body
            size_t if_start = *line_idx;
            int if_indent = -1;

            while (*line_idx < line_count) {
                char* next_line = lines[*line_idx];
                char* next_trimmed = l3_trim(next_line);
                int next_indent = l3_get_indent(next_line);

                if (strlen(next_trimmed) > 0) {
                    if (if_indent == -1) {
                        if_indent = next_indent;
                    }

                    if (next_indent < if_indent) {
                        free(next_trimmed);
                        break;
                    }
                }

                free(next_trimmed);
                (*line_idx)++;
            }

            // Execute if-block if condition true
            if (result && if_indent != -1) {
                size_t save_idx = if_start;
                l3_execute_block(lines, &save_idx, *line_idx, if_indent, ctx);
            }

            free(trimmed);
            continue;
        }

        // while <condition>
        if (l3_starts_with(trimmed, "while ")) {
            char* condition = l3_trim(trimmed + 6);

            (*line_idx)++;

            // Collect while-block body
            size_t while_start = *line_idx;
            size_t while_end = *line_idx;
            int while_indent = -1;

            while (while_end < line_count) {
                char* next_line = lines[while_end];
                char* next_trimmed = l3_trim(next_line);
                int next_indent = l3_get_indent(next_line);

                if (strlen(next_trimmed) > 0) {
                    if (while_indent == -1) {
                        while_indent = next_indent;
                    }

                    if (next_indent < while_indent) {
                        free(next_trimmed);
                        break;
                    }
                }

                free(next_trimmed);
                while_end++;
            }

            // Execute while loop
            int iterations = 0;
            const int max_iterations = 10000;

            while (l3_evaluate_condition(condition, ctx) && iterations < max_iterations) {
                size_t loop_idx = while_start;
                if (while_indent != -1) {
                    l3_execute_block(lines, &loop_idx, while_end, while_indent, ctx);
                }
                iterations++;
            }

            if (iterations >= max_iterations) {
                printf("⚠️  While loop hit iteration limit (%d)\n", max_iterations);
            }

            *line_idx = while_end;
            free(condition);
            free(trimmed);
            continue;
        }

        // for <var> in <start> to <end>
        if (l3_starts_with(trimmed, "for ")) {
            // Parse: for i in 1 to 5
            char* rest = trimmed + 4;
            char* in_pos = strstr(rest, " in ");
            char* to_pos = strstr(rest, " to ");

            if (in_pos && to_pos) {
                size_t var_len = in_pos - rest;
                char* var_name = l3_substring(rest, 0, var_len);
                char* var_trimmed = l3_trim(var_name);
                free(var_name);

                char* start_str = l3_substring(in_pos + 4, 0, to_pos - (in_pos + 4));
                char* start_trimmed = l3_trim(start_str);
                free(start_str);

                char* end_str = l3_trim(to_pos + 4);

                char* start_val = l3_evaluate_expression(start_trimmed, ctx);
                char* end_val = l3_evaluate_expression(end_str, ctx);

                int start_num = atoi(start_val);
                int end_num = atoi(end_val);

                (*line_idx)++;

                // Collect for-block body
                size_t for_start = *line_idx;
                size_t for_end = *line_idx;
                int for_indent = -1;

                while (for_end < line_count) {
                    char* next_line = lines[for_end];
                    char* next_trimmed = l3_trim(next_line);
                    int next_indent = l3_get_indent(next_line);

                    if (strlen(next_trimmed) > 0) {
                        if (for_indent == -1) {
                            for_indent = next_indent;
                        }

                        if (next_indent < for_indent) {
                            free(next_trimmed);
                            break;
                        }
                    }

                    free(next_trimmed);
                    for_end++;
                }

                // Execute for loop
                for (int i = start_num; i <= end_num; i++) {
                    char num_buf[32];
                    snprintf(num_buf, sizeof(num_buf), "%d", i);
                    l3_set_local(ctx, var_trimmed, num_buf);

                    size_t loop_idx = for_start;
                    if (for_indent != -1) {
                        l3_execute_block(lines, &loop_idx, for_end, for_indent, ctx);
                    }
                }

                *line_idx = for_end;

                free(var_trimmed);
                free(start_trimmed);
                free(end_str);
                free(start_val);
                free(end_val);
                free(trimmed);
                continue;
            }
        }

        // Regular statement
        if (indent >= base_indent) {
            l3_execute_line(trimmed, ctx);
        }

        free(trimmed);
        (*line_idx)++;
    }
}

void l3_execute_handler(const char* body, L3_ExecutionContext* ctx) {
    if (!body || !ctx) return;

    // Split body into lines
    char* body_copy = l3_strdup(body);
    size_t line_count = 1;
    for (const char* p = body; *p; p++) {
        if (*p == '\n') line_count++;
    }

    char** lines = malloc(line_count * sizeof(char*));
    size_t idx = 0;

    char* line = strtok(body_copy, "\n");
    while (line != NULL && idx < line_count) {
        lines[idx++] = l3_strdup(line);
        line = strtok(NULL, "\n");
    }

    size_t line_idx = 0;
    l3_execute_block(lines, &line_idx, idx, 0, ctx);

    // Free lines
    for (size_t i = 0; i < idx; i++) {
        free(lines[i]);
    }
    free(lines);
    free(body_copy);
}
// Control flow implementation complete
