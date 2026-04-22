# Scoring Rubric

Prompt Trainer scores each dimension from 0 to 100, then computes the overall score as the average.

## 1. Clarity
Measures how specific and understandable the prompt is.
- Positive signals: explicit action verbs, clear objective terms, sufficient detail.
- Negative signals: vague words like "something", "stuff", "nice".

## 2. Context
Measures background and role framing.
- Positive signals: role definition ("you are"), audience, domain, background context.

## 3. Constraints
Measures explicit limits and requirements.
- Positive signals: rules such as "exactly", "at most", "must", and numeric limits.

## 4. Output format
Measures how clearly output structure is requested.
- Positive signals: requested format (JSON, bullets, markdown table, sections).

## 5. Examples
Measures whether example guidance is provided.
- Positive signals: "example", "input:", "output:", sample snippet.

## Feedback strategy
If a dimension score is below 60, the app adds a targeted suggestion for that area.
