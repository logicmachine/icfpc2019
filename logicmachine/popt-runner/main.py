import os
import re
import subprocess
import requests

from tempfile import NamedTemporaryFile
from joblib import Parallel, delayed

API_SERVER = 'http://icfpc.logicmachine.jp:5000'


def get_problems():
    r = requests.get('{}/index.json'.format(API_SERVER))
    problems = r.json()
    return [p['problem_id'] for p in problems]

def get_submissions(problem_id):
    r = requests.get('{}/details_json/{}'.format(API_SERVER, problem_id))
    raw_submissions = r.json()
    submissions = dict()
    best_score = raw_submissions[0]['score']
    for s in raw_submissions:
        if s['score'] > best_score * 1.5:
            continue
        prefix = re.match(r'[a-zA-Z]+', s['author']).group(0)
        if prefix in submissions:
            pass
        submissions[prefix] = {
            'problem_id': problem_id,
            'submission_id': s['id'],
            'author': s['author']
        }
    return submissions.values()

def process(info):
    problem_id = info['problem_id']
    submission_id = info['submission_id']
    author = info['author']
    problem = requests.get('{}/problems/{}'.format(API_SERVER, problem_id)).text
    solution = requests.get('{}/download/{}'.format(API_SERVER, submission_id)).text
    boosters = requests.get('{}/boosters/{}'.format(API_SERVER, submission_id)).text
    with NamedTemporaryFile() as pf, NamedTemporaryFile() as sf, NamedTemporaryFile() as bf:
        pf.write(problem.encode('utf-8'))
        pf.flush()
        sf.write(solution.encode('utf-8'))
        sf.flush()
        bf.write(boosters.encode('utf-8'))
        bf.flush()
        proc = subprocess.Popen(
            ['../patch-optimizer/a.out', pf.name, sf.name, bf.name],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        outs, errs = proc.communicate()
        if proc.returncode != 0:
            print('Failed to optimize {} (problem {})'.format(submission_id, problem_id))
        else:
            solution = outs.decode('utf-8')
            r = requests.post(
                '{}/submit'.format(API_SERVER),
                {
                    'problem_id': problem_id,
                    'author': author + '+popt9',
                    'solution': solution,
                    'boosters': boosters
                })

def main():
    problems = get_problems()
    submissions = []
    for problem in problems:
        submissions.extend(get_submissions(problem))
    print('Optimize {} submissions'.format(len(submissions)))
    Parallel(n_jobs=-1, verbose=9)([delayed(process)(p) for p in submissions])

if __name__ == '__main__':
    main()
