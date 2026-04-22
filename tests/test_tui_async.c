#include "tui/async.h"

#include <sys/select.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void) {
    TuiAsync async_state;
    TuiAsyncResult result;
    int tries = 0;
    int got_first = 0;
    int got_second = 0;

    expect_true(tui_async_init(&async_state) == 0, "async init should work");
    expect_true(tui_async_submit(&async_state, "Create a JSON list with exactly 2 items.") == 0, "submit should work");
    expect_true(tui_async_submit(&async_state, "Explain recursion with one example.") == 0, "second submit should queue while busy");
    expect_true(tui_async_is_busy(&async_state) == 1, "async should be busy after submit");

    memset(&result, 0, sizeof(result));
    for (tries = 0; tries < 10000; ++tries) {
        struct timeval tv;
        if (tui_async_poll(&async_state, &result) == 1) {
            if (!got_first) {
                got_first = 1;
            } else {
                got_second = 1;
                break;
            }
        }
        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        (void)select(0, NULL, NULL, NULL, &tv);
    }
    expect_true(tries < 10000, "async queued results should arrive");
    expect_true(got_first == 1, "first result should arrive");
    expect_true(got_second == 1, "second queued result should arrive");
    expect_true(result.ok == 1, "result should be ok");
    expect_true(result.score.overall_score > 0, "score should be populated");
    expect_true(strlen(result.prompt) > 0, "prompt should be echoed in result");

    tui_async_shutdown(&async_state);

    if (failures != 0) {
        fprintf(stderr, "Total failures: %d\n", failures);
        return 1;
    }
    printf("All tui async tests passed.\n");
    return 0;
}
