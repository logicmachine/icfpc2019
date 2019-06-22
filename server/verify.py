import os
import sqlite3

from tempfile import NamedTemporaryFile

from flask import Flask, g, abort, request, jsonify

from selenium import webdriver
from selenium.webdriver.chrome.options import Options


app = Flask(__name__)

def get_driver():
    if 'driver' not in g:
        options = Options()
        options.binary_location = '/usr/bin/chromium'
        options.add_argument('--headless')
        driver = webdriver.Chrome('chromedriver', options=options)
        driver.get('https://icfpcontest2019.github.io/solution_checker/')
        g.driver = driver
    return g.driver

def get_db():
    if 'db' not in g:
        g.db = sqlite3.connect('database.db', detect_types=sqlite3.PARSE_DECLTYPES)
    return g.db

def run_validation(problem_path, solution_path):
    driver = get_driver()
    driver.find_element_by_id('submit_task').send_keys(os.path.abspath(problem_path))
    driver.find_element_by_id('submit_solution').send_keys(os.path.abspath(solution_path))
    driver.find_element_by_id('execute_solution').click()
    while True:
        status = driver.find_element_by_id('output').text.strip()
        tokens = status.split(' ')
        if tokens[0] == 'Success!':
            return {'success': True, 'score': int(tokens[4]), 'message': status}
        elif tokens[0] == 'Pre-processing':
            continue
        else:
            return {'success': False, 'message': status}

@app.route('/verify', methods=['POST'])
@app.route('/verify/<int:problem_id>', methods=['POST'])
def verify(problem_id=None):
    conn = get_db()
    c = conn.cursor()
    if problem_id is None:
        problem = request.form['problem']
    else:
        query = 'select content from problems where id=?'
        row = c.execute(query, (problem_id,)).fetchone()
        if row is None:
            abort(404)
        problem = row[0]
    solution = request.form['solution']
    with NamedTemporaryFile() as pf, NamedTemporaryFile() as sf:
        pf.write(problem.encode('utf-8'))
        pf.flush()
        sf.write(solution.encode('utf-8'))
        sf.flush()
        status = run_validation(pf.name, sf.name)
    return jsonify(status)

if __name__ == '__main__':
    app.run(host='localhost', port=5001)
