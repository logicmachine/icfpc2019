import os
import subprocess
import requests

from tempfile import NamedTemporaryFile
from joblib import Parallel, delayed

API_SERVER = 'http://icfpc.logicmachine.jp:5000'

def process(info):
    author = info['author']
    if author.find('+popt') >= 0:
        return
    problem = requests.get('{}/problems/{}'.format(API_SERVER, info['problem_id'])).text
    solution = requests.get('{}/download/{}'.format(API_SERVER, info['submission_id'])).text
    with NamedTemporaryFile() as pf, NamedTemporaryFile() as sf:
        pf.write(problem.encode('utf-8'))
        pf.flush()
        sf.write(solution.encode('utf-8'))
        sf.flush()
        proc = subprocess.Popen(
            ['../patch-optimizer/a.out', pf.name, sf.name],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        outs, errs = proc.communicate()
        if proc.returncode != 0:
            print('Failed to optimize {} (problem {})'.format(info['submission_id'], info['problem_id']))
        else:
            solution = outs.decode('utf-8')
            r = requests.post(
                '{}/submit'.format(API_SERVER),
                {
                    'problem_id': info['problem_id'],
                    'author': info['author'] + '+popt7',
                    'solution': solution
                })

def main():
    r = requests.get('{}/index.json'.format(API_SERVER))
    submissions = r.json()
    Parallel(n_jobs=-1, verbose=5)([delayed(process)(p) for p in submissions])

if __name__ == '__main__':
    main()
