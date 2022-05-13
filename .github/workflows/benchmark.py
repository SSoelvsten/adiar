import re
import io
import selectors
import subprocess
import sys


# ------------------------------------------------------------------------------
# From 'nawatts/capture-and-print-subprocess-output.py'

def capture_subprocess_output(subprocess_args):
    # Start subprocess
    # bufsize = 1 means output is line buffered
    # universal_newlines = True is required for line buffering
    process = subprocess.Popen(subprocess_args,
                               bufsize=1,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT,
                               universal_newlines=True)

    # Create callback function for process output
    buf = io.StringIO()
    def handle_output(stream, mask):
        # Because the process' output is line buffered, there's only ever one
        # line to read when this function is called
        line = stream.readline()
        buf.write(line)
        sys.stdout.write(line)

    # Register callback for an "available for read" event from subprocess' stdout stream
    selector = selectors.DefaultSelector()
    selector.register(process.stdout, selectors.EVENT_READ, handle_output)

    # Loop until subprocess is terminated
    while process.poll() is None:
        # Wait for events and handle them with their registered callbacks
        events = selector.select()
        for key, mask in events:
            callback = key.data
            callback(key.fileobj, mask)

    # Get process return code
    return_code = process.wait()
    selector.close()

    success = (return_code == 0)

    # Store buffered output
    output = buf.getvalue()
    buf.close()

    return (success, output)


# ------------------------------------------------------------------------------
# Branch switching
def remote_name(remote):
    return remote.replace('/','_')


def git_remove_remote(remote):
    if (remote == 'origin'):
        return # origin should not be purged

    subprocess.run(['git', 'remote', 'remove', remote_name(remote)])


def git_add_remote(remote):
    if (remote == 'origin'):
        return # origin hopefully exists

    git_remove_remote(remote)
    subprocess.run(['git', 'remote', 'add', remote_name(remote), f'https://github.com/{remote}.git'])


def git_fetch():
    subprocess.run(['git', 'fetch', '--all'])


def git_checkout(remote, branch):
    subprocess.run(['git', 'checkout', f'{remote_name(remote)}/{branch}'])
    subprocess.run(['git', 'checkout', 'update', '--init', '--recursive'])
    subprocess.run(['git', 'status'])

# ------------------------------------------------------------------------------
# Running benchmarks
def repetitions_for(n):
    if n <= 12:
        return 16
    elif n == 13:
        return 8
    elif n == 14:
        return 4
    else:
        return 1

def run_queens(n):
    out_txt = capture_subprocess_output(['make', 'example/queens', 'N='+str(n), 'M=8096'])[1]

    matches = re.findall("time:\s*([0-9\.]+)\s*s", out_txt)
    timing = sum([float(t) for t in matches])
    return timing


# ------------------------------------------------------------------------------
# Markdown helper functions
def markdown_table(args, timings):
    header = ' | '.join([f'{r}/{b}' for (r,b) in args])
    line   = '-|-'.join([re.sub('.','-',f'{r}/{b}') for (r,b) in args])

    # assumes every value is a list of the same length
    number_of_rows = len(timings[args[0][0]][args[0][1]])
    rows = []

    for i in range(0, number_of_rows):
        rows.append(' | '.join([t + ' ' * (len(b)-len(t))
                                for (b, t) in [(f'{r}/{b}', f'{timings[r][b][i]:.2f}') for (r,b) in args]]))

    return '\n'.join(['| '+ header +' |', '|-'+ line +'-|'] + ['| '+ r +' |' for r in rows])


def bold(txt):
    return "**"+txt+"**"


def spoiler(txt, summary):
    return '> ' + ('<details>\n' +
                   f'<summary><b>{summary}</b></summary>\n\n'
                   + txt + '\n' +
                   '</details>').replace('\n', '\n> ')


def performance_report(n, args, timings):
    output_txt = (f'# Benchmark Report `{args[0]}/{args[1]}`\n' +
                  f'Minimum running time for {n}-Queens: {min(timings[args[0][1]])}s\n\n' +
                  spoiler(f'Running times (s) for {n}-Queens:\n' +
                          ', '.join([str(t) for t in timings[args[0][1]]]), 'Raw Data'))

    return (0, output_txt)


def comparison_report(n, args, timings):
    # Create table of raw data
    raw_data_txt = spoiler(f'Running times (s) for {n}-Queens:\n' + markdown_table(args, timings),
                           'Raw Data')

    # Compute minimum
    for (r,b) in args:
        timings[r][b] = [min(timings[r][b])]

    minimum_data_txt = (f'Minimum running time (s) for {n}-Queens:\n' +
                        markdown_table(args, timings) + '\n')

    output_txt = ''

    # Compute differences
    reference_t = min(timings[args[0][0]][args[0][1]])
    tested_t = [(rb, min(timings[rb[0]][rb[1]])) for rb in args[1::]]

    diffs = []

    if reference_t > 0.0:
        diffs = [(rb, (t - reference_t) / reference_t)
                 for (rb,t) in tested_t]
    else:
        diffs = [(rb, float('inf') if t > 0 else float(0))
                 for (rb,t) in tested_t]

    # Check if any diffs violate the threshold
    worst_diff = 0.0 if len(diffs) == 0 else max([t for (rb,t) in diffs])
    report_color = 'green' if worst_diff < 0.01 else ('yellow' if worst_diff < 0.05 else 'red')
    exit_code = -1 if report_color == 'red' else 0

    diffs_txt = [f'`{rb[0]}/{rb[1]}` ' + ('does not impact performance'
                                          if t == 0.0
                                          else ('is '+('an improvement' if t < 0.0 else 'a regression')+
                                                f' of {abs(t)*100.0:.2f}%'))
                 for (rb,t) in diffs]

    output_txt = (f'# Benchmark Report :{report_color}_circle:\n' +
                  bold(' and '.join(diffs_txt) + f' (compared to `{args[0][0]}/{args[0][1]}`).') +
                  '\n\n' +
                  minimum_data_txt + '\n' +
                  raw_data_txt)

    return (exit_code, output_txt)


# ------------------------------------------------------------------------------
# Main
def main_current(n):
    timings = []

    print('Queens', end='', flush=True)

    for run in range(0, repetitions_for(n)):
        print(' .', end='', flush=True)
        timings.append(run_queens(n))

    print('\n  time: ', min(timings), ' s')

def main_compare(n, remote_branch_pairs):
    for remote in list(set([r for (r,b) in remote_branch_pairs])):
        git_add_remote(remote)

    git_fetch()

    # Initialise results data
    timings  = {  }

    for (remote, branch) in remote_branch_pairs:
        timings[remote] = {}

    for (remote, branch) in remote_branch_pairs:
        timings[remote][branch] = []

    # Run benchmarks on all branches as fairly as possible
    for run in range(0, repetitions_for(n)):
        for (remote, branch) in remote_branch_pairs:
            git_checkout(remote, branch)
            timings[remote][branch].append(run_queens(n))

    for remote in list(set([r for (r,b) in remote_branch_pairs])):
        git_remove_remote(remote)

    exit_code = 0
    with open('benchmark.out', 'w') as file:
        report = (comparison_report(n, remote_branch_pairs, timings)
                  if len(remote_branch_pairs) > 1
                  else performance_report(n, remote_branch_pairs, timings))

        exit_code = report[0]

        print(report[1])
        file.write(report[1])

    return exit_code


def main(cmd_args):
    if len(cmd_args) < 2:
        print("Please provide the 'N' as the first argument")
        exit(-1)

    cmd_program = cmd_args[0]
    cmd_N = int(cmd_args[1])
    cmd_branches = cmd_args[2::1]

    exit_code = 0

    if len(cmd_branches) == 0:
        main_current(cmd_N)

    else:
        if cmd_branches == 1:
            cmd_branches = ['origin', cmd_branches[1]]

        if len(cmd_branches) % 2 != 0:
            print('Please provide pairs of remote , branch')
            exit(-1)

        exit_code = main_compare(cmd_N,
                                 list(zip(*[iter(cmd_branches[0::])]*2)))

    exit(exit_code)

if __name__ == '__main__':
    main(sys.argv)
