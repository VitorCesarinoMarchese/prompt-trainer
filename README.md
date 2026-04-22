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
  - terminal UI mode (`--tui`)
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

TUI mode:
```sh
./bin/prompt-trainer --tui
```

JSON output:
```sh
./bin/prompt-trainer --json -p "Create a JSON response with exactly 3 objects."
```

## TUI keyboard shortcuts
- Enter: submit prompt
- Shift+Enter: insert newline
- Up/Down: recall input history (empty input) or scroll output
- PgUp/PgDn: page scroll output
- Ctrl+L: clear input
- Esc or Ctrl+C: quit

## TUI behavior notes
- If evaluator is busy, new submissions are queued automatically.
- On small terminals, TUI switches to compact layout mode.
- `--tui` requires interactive TTY stdin/stdout.
- Input textbox wraps long lines to panel width and keeps cursor aligned with typing position.
- Response panel uses structured output with score header, dimension list, and weak-dimension improvements only.

Example TUI response block:
```text
Score 100/100

- Clarity: 25
- Context: 30
- Constraints: 0
- Format: 30
- Examples: 0

Improvements:
1. Improve clarity: use specific action verbs and remove vague terms.
2. Add context: define role, audience, and domain/background.
3. Add constraints: include explicit limits, rules, or measurable requirements.
```

## TUI dependencies
TUI mode links against:
- `ncurses`
- `pthread`

## Test
```sh
make test
```

See `docs/rubric.md` for scoring details.
