import re
import subprocess
import sys

# ------------------------------------------------------------------------------
# Branch switching
def git_checkout(branch_name):
    #print('git', 'checkout', branch_name)
    subprocess.run(['git', 'checkout', branch_name], capture_output=True)
    return

# ------------------------------------------------------------------------------
# Running benchmarks
QUEENS_N = 14

def run_queens():
    out = subprocess.run(['make', 'example/queens', 'N='+str(QUEENS_N), 'M=8096'],
                         capture_output=True,
                         text=True)

    # The output ends on the error due to the CMake warnings during build
    out_txt = out.stderr

    matches = re.findall("time:\s*([0-9\.]+)\s*s", out_txt)
    timing = sum([float(t) for t in matches])
    return timing

# ------------------------------------------------------------------------------
# Markdown helper functions
def markdown_table(timings):
    keys = list(timings.keys())

    header = ' | '.join(keys)
    line   = '-|-'.join([re.sub('.','-',b) for b in keys])

    # assumes every value is a list of the same length
    number_of_rows = len(timings[keys[0]])
    rows = []

    for r in range(0, number_of_rows):
        rows.append(' | '.join([t + ' ' * (len(b)-len(str(t)))
                                  for (b, t) in [(b, str(timings[b][r])) for b in keys]]))

    return '\n'.join(['| '+ header +' |', '|-'+ line +'-|'] + ['| '+ r +' |' for r in rows])

def bold(txt):
    return "**"+txt+"**"

def spoiler(txt, summary):
    return '> ' + ('<details>\n' +
                   f'<summary><b>{summary}</b></summary>\n\n'
                   + txt + '\n' +
                   '</details>').replace('\n', '\n> ')

def performance_report(branches, timings):
    output_txt = (f'# Benchmark Report `{branches[0]}`\n' +
                  f'Minimum running time for {QUEENS_N}-Queens: {min(timings[branches[0]])}s\n\n' +
                  spoiler(f'Running times (s) for {QUEENS_N}-Queens:\n' + ', '.join([str(t) for t in timings[branches[0]]]), 'Raw Data'))

    return (0, output_txt)

def comparison_report(branches, timings):
    # Create table of raw data
    raw_data_txt = spoiler(f'Running times (s) for {QUEENS_N}-Queens:\n' + markdown_table(timings), 'Raw Data')

    # Compute minimum
    for k in timings.keys():
        timings[k] = [min(timings[k])]

    minimum_data_txt = (f'Minimum running time (s) for {QUEENS_N}-Queens:\n' +
                        markdown_table(timings) + '\n')

    output_txt = ''

    # Compute differences
    reference_t = min(timings[branches[0]])
    tested_t = [(b, min(timings[b])) for b in branches[1::]]

    diffs = [(b, (t - reference_t) / reference_t) for (b,t) in tested_t]

    # Check if any diffs violate the threshold
    worst_diff = 0.0 if len(diffs) == 0 else max([t for (b,t) in diffs])
    report_color = 'green' if worst_diff < 0.01 else ('yellow' if worst_diff < 0.05 else 'red')
    exit_code = -1 if report_color == 'red' else 0

    diffs_txt = [f'`{b}` ' + ('does not impact performance'
                              if t == 0.0
                              else ('is '+('an improvement' if t < 0.0 else 'a regression')+f' of {abs(t)*100.0:.2f}%'))
                 for (b,t) in diffs]

    output_txt = (f'# Benchmark Report :{report_color}_circle:\n' +
                  bold(' and '.join(diffs_txt) + ' (compared to `'+branches[0]+'`).') + '\n\n' +
                  minimum_data_txt + '\n' +
                  raw_data_txt)

    return (exit_code, output_txt)

# ------------------------------------------------------------------------------
# Main
REPETITIONS = 4

if __name__ == '__main__':
    if len(sys.argv) == 1:
        timings = []

        print('Queens', end='', flush=True)

        for run in range(0, REPETITIONS):
            print(' .', end='', flush=True)
            timings.append(run_queens())

        print('\n  time: ', min(timings), ' s')

    elif len(sys.argv) > 1:
        branches = [b for b in sys.argv[1::]]
        timings = { }

        for branch in branches:
            timings[branch] = []

        # Run benchmarks on all branches as fairly as possible
        for run in range(0, REPETITIONS):
            for branch in branches:
                git_checkout(branch)
                timings[branch].append(run_queens())

        exit_code = 0
        with open('benchmark.out', 'w') as file:
            report = (comparison_report(branches, timings)
                      if len(branches) > 1
                      else performance_report(branches, timings))

            exit_code = report[0]

            print(report[1])
            file.write(report[1])

        exit(exit_code)
    else:
        print('bad state: len(sys.argv) == 0')
        exit(-1)
