# Prompt Trainer (C CLI)

Prompt Trainer is a dependency-free CLI app written in C that scores prompts and gives feedback to improve them.

## Features
- Rule-based evaluation (no external API required)
- Overall score (0-100)
- Per-dimension scores:
  - clarity
  - context
  - constraints
  - output_format
  - examples
- Actionable feedback suggestions
- Two modes:
  - single-command mode
  - interactive mode
- Human-readable and JSON output

## Build
```sh
make
```

## Run
Single command:
```sh
./bin/prompt-trainer -p "You are a tutor. Explain recursion in 5 bullet points with one example."
```

Positional prompt:
```sh
./bin/prompt-trainer Create a markdown table with 3 rows explaining git reset
```

Interactive mode:
```sh
./bin/prompt-trainer --interactive
```

JSON output:
```sh
./bin/prompt-trainer --json -p "Create a JSON response with exactly 3 objects."
```

## Test
```sh
make test
```

See `docs/rubric.md` for scoring details.
