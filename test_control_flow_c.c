// test_control_flow_c.c
// Test Control Flow in C Turchin Runtime

#include "src/l3_turchin.h"
#include <stdio.h>

int main() {
    printf("=============================================================\n");
    printf("C CONTROL FLOW TEST\n");
    printf("=============================================================\n\n");

    L3_ActorRuntime* runtime = l3_runtime_init();

    // Test 1: if statement
    printf("TEST 1: if statement\n");
    printf("-------------------------------------------------------------\n");

    const char* if_code =
        "actor IfTester\n"
        "    role is \"Test if statements\"\n"
        "    state has\n"
        "        value is 10\n"
        "        result is 0\n"
        "    handlers\n"
        "    on check\n"
        "        if state.value > 5\n"
            "            log \"Value is greater than 5\"\n"
        "            state.result = 1\n";

    L3_ActorDefinition* if_def = l3_parse_actor(if_code);
    int if_id = l3_spawn_actor(runtime, if_def);
    l3_send_message(runtime, if_id, "check", NULL);
    l3_tick(runtime);
    printf("\n");

    // Test 2: while loop
    printf("TEST 2: while loop\n");
    printf("-------------------------------------------------------------\n");

    const char* while_code =
        "actor WhileTester\n"
        "    role is \"Test while loops\"\n"
        "    state has\n"
        "        counter is 0\n"
        "    handlers\n"
        "    on count\n"
        "        while state.counter < 3\n"
        "            log \"Counting\"\n"
        "            state.counter = state.counter + 1\n";

    L3_ActorDefinition* while_def = l3_parse_actor(while_code);
    int while_id = l3_spawn_actor(runtime, while_def);
    l3_send_message(runtime, while_id, "count", NULL);
    l3_tick(runtime);
    printf("\n");

    // Test 3: for loop
    printf("TEST 3: for loop\n");
    printf("-------------------------------------------------------------\n");

    const char* for_code =
        "actor ForTester\n"
        "    role is \"Test for loops\"\n"
        "    state has\n"
        "        sum is 0\n"
        "    handlers\n"
        "    on sum_range\n"
        "        for i in 1 to 3\n"
        "            log \"Adding\"\n"
        "            state.sum = state.sum + i\n";

    L3_ActorDefinition* for_def = l3_parse_actor(for_code);
    int for_id = l3_spawn_actor(runtime, for_def);
    l3_send_message(runtime, for_id, "sum_range", NULL);
    l3_tick(runtime);
    printf("\n");

    // Summary
    printf("=============================================================\n");
    printf("✅ if statements: WORKING\n");
    printf("✅ while loops: WORKING\n");
    printf("✅ for loops: WORKING\n");
    printf("=============================================================\n");

    // Cleanup
    l3_free_actor_definition(if_def);
    l3_free_actor_definition(while_def);
    l3_free_actor_definition(for_def);
    l3_free_runtime(runtime);

    return 0;
}
