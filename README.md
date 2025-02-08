The makefile in each sub-directory will create the "predictors" and "cache-sim" executables.

```
./<executable> traces/<input_trace> output.txt
```
will run the chosen test on the trace provided. The trace must be from the sub-directory's `trace/` directory.

`output.txt` can be compared to the corresopnding output in `correct_outputs/`.

Execution time for the shortest trace (~1 million instructions) is 30 seconds.