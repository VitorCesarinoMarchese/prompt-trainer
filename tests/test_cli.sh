#!/usr/bin/env sh
set -eu

CLI="./bin/prompt-trainer"

if [ ! -x "$CLI" ]; then
  echo "FAIL: binary not found at $CLI"
  exit 1
fi

JSON_OUT=$($CLI --json -p "You are a product manager. Create a JSON list with exactly 3 ideas. Example output: [{\"idea\":\"x\"}]")
echo "$JSON_OUT" | grep '"overall"' >/dev/null
echo "$JSON_OUT" | grep '"dimensions"' >/dev/null

TEXT_OUT=$($CLI Write a short bullet list of two git tips with clear titles)
echo "$TEXT_OUT" | grep 'Overall score:' >/dev/null
echo "$TEXT_OUT" | grep 'Dimension breakdown:' >/dev/null

INTERACTIVE_OUT=$(printf "You are a coding mentor. Explain pointers in C with 2 examples.\n" | $CLI --interactive)
echo "$INTERACTIVE_OUT" | grep 'Feedback:' >/dev/null

echo "All CLI tests passed."
