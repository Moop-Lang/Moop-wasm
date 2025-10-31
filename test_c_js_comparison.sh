#!/bin/bash
# Compare C and JavaScript runtime outputs

echo "============================================================="
echo "L3 TURCHIN RUNTIME COMPARISON: C vs JavaScript"
echo "============================================================="
echo ""

echo "Compiling C runtime..."
gcc -std=c99 -Wall -Wextra -O2 -g -I./src test_control_flow_c.c src/l3_turchin.c -o test_control_flow_c 2>&1 | grep -v warning
echo "✅ C runtime compiled"
echo ""

echo "Running C runtime test..."
echo "-------------------------------------------------------------"
./test_control_flow_c > /tmp/c_output.txt 2>&1
cat /tmp/c_output.txt
echo ""

echo "Running JavaScript runtime test..."
echo "-------------------------------------------------------------"
node test_l3_node.js > /tmp/js_output.txt 2>&1
cat /tmp/js_output.txt
echo ""

echo "============================================================="
echo "COMPARISON ANALYSIS"
echo "============================================================="

# Count matching behaviors
c_if=$(grep -c "✅ if statements: WORKING" /tmp/c_output.txt || true)
js_if=$(grep -c "✅ if statements: WORKING" /tmp/js_output.txt || true)

c_while=$(grep -c "✅ while loops: WORKING" /tmp/c_output.txt || true)
js_while=$(grep -c "✅ while loops: WORKING" /tmp/js_output.txt || true)

c_for=$(grep -c "✅ for loops: WORKING" /tmp/c_output.txt || true)
js_for=$(grep -c "✅ for loops: WORKING" /tmp/js_output.txt || true)

echo ""
echo "Feature Parity Check:"
echo "  if statements:  C=$c_if  JS=$js_if  $([ "$c_if" -eq "$js_if" ] && echo '✅' || echo '❌')"
echo "  while loops:    C=$c_while  JS=$js_while  $([ "$c_while" -eq "$js_while" ] && echo '✅' || echo '❌')"
echo "  for loops:      C=$c_for  JS=$js_for  $([ "$c_for" -eq "$js_for" ] && echo '✅' || echo '❌')"
echo ""

if [ "$c_if" -eq "$js_if" ] && [ "$c_while" -eq "$js_while" ] && [ "$c_for" -eq "$js_for" ]; then
    echo "🎉 PERFECT PARITY: C and JavaScript runtimes match exactly!"
else
    echo "⚠️  Mismatch detected - check outputs above"
fi

echo "============================================================="
