import os
import sys
import glob
import subprocess
import requests
import yaml


def get_current_block(root):
    return max([int(os.path.basename(s)) for s in glob.glob(os.path.join(root, 'blocks', '*'))])


def evaluate_solution(problem, solution):
    r = requests.post('http://localhost:5001/verify', {'problem': problem, 'solution': solution})
    return r.json()


def main():
    yaml_file = sys.argv[1]
    solution_root = os.path.dirname(yaml_file)

    client_root = '/home/ubuntu/lambda-client'
    # client_root = '../lambda-client'
    current_block = get_current_block(client_root)
    block_root = os.path.join(client_root, 'blocks', str(current_block))
    task_file = os.path.abspath(os.path.join(block_root, 'task.desc'))
    with open(task_file, 'r') as f:
        task_text = f.read()
    puzzle_file = os.path.abspath(os.path.join(block_root, 'puzzle.cond'))

    best_score = (1 << 30)
    best_solution = ''
    with open(yaml_file, 'r') as f:
        config = yaml.load(f, Loader=yaml.FullLoader)
        for name in config:
            print('------------------------------')
            print(' ', name)
            print('------------------------------')
            cwd = os.path.join(solution_root, name)
            proc = subprocess.Popen(['./run.sh', task_file], cwd=cwd, stdout=subprocess.PIPE)
            outs, errs = proc.communicate()
            if proc.returncode != 0:
                print('Failed: `run.sh` returned {}.'.format(name, proc.returncode))
            else:
                solution = outs.decode('utf-8')
                r = evaluate_solution(task_text, solution)
                print(r['message'])
                if r['success'] and r['score'] < best_score:
                    best_score = r['score']
                    best_solution = solution
            print('')
    if best_solution == '':
        print('Any solvers cannot produce solution.')
        sys.exit(-1)
    with open('task.sol', 'w') as f:
        f.write(best_solution)

    print('------------------------------')
    print(' ', 'ikeda (puzzle)')
    print('------------------------------')
    cwd = os.path.join(solution_root, 'ikeda')
    proc = subprocess.Popen(['./run.sh', puzzle_file], cwd=cwd, stdout=subprocess.PIPE)
    outs, errs = proc.communicate()
    solution = outs.decode('utf-8')
    with open('puzzle.desc', 'w') as f:
        f.write(solution)

    with open('block_id', 'w') as f:
        f.write(str(current_block))


if __name__ == '__main__':
    main()
