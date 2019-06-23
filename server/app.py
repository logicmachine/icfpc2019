import sqlite3
import requests

from tempfile import TemporaryFile
from zipfile import ZipFile

from flask import Flask, g, request, render_template, make_response, abort, jsonify

app = Flask(__name__)

def get_db():
    if 'db' not in g:
        g.db = sqlite3.connect('database.db', detect_types=sqlite3.PARSE_DECLTYPES)
        g.db.row_factory = sqlite3.Row
    return g.db

@app.route('/')
def index():
    conn = get_db()
    query = '''
    select problem_id, problems.description, min(score), author, created_at
      from solutions
      inner join problems on problem_id = problems.id
      group by problem_id
    '''
    problems = [
        {
            'problem_id': row['problem_id'],
            'description': row['description'],
            'score': row['min(score)'],
            'author': row['author'],
            'created_at': row['created_at']
        }
        for row in conn.execute(query)
    ]
    return render_template('index.html', problems=problems)

@app.route('/index.json')
def index_json():
    conn = get_db()
    query = '''
    select solutions.id, problem_id, min(score), author, created_at
      from solutions
      inner join problems on problem_id = problems.id
      group by problem_id
    '''
    problems = [
        {
            'problem_id': row['problem_id'],
            'submission_id': row['id'],
            'score': row['min(score)'],
            'author': row['author'],
            'created_at': row['created_at']
        }
        for row in conn.execute(query)
    ]
    return jsonify(problems)

@app.route('/details/<int:problem_id>')
def details(problem_id):
    conn = get_db()
    problem = { 'problem_id': problem_id }
    submissions_query = '''
    select id, score, author, created_at
      from solutions
      where problem_id=?
      order by score asc
    '''
    submissions = [
        {
            'id': row['id'],
            'score': row['score'],
            'author': row['author'],
            'created_at': row['created_at']
        }
        for row in conn.execute(submissions_query, (problem_id,))
    ]
    return render_template('details.html', problem=problem, submissions=submissions)

@app.route('/download/<int:submission_id>')
def download(submission_id):
    conn = get_db()
    query = 'select problem_id, content from solutions where id=?'
    row = conn.execute(query, (submission_id,)).fetchone()
    if row is None:
        abort(404)
    name = 'prob-{:03d}-{:05d}.sol'.format(row['problem_id'], submission_id)
    response = make_response()
    response.data = row['content']
    response.headers['Content-Disposition'] = 'attachment; filename={}'.format(name)
    response.mimetype = 'text/plain'
    return response


@app.route('/submit', methods=['GET', 'POST'])
def submit():
    success = True
    message = ''
    if request.method == 'POST':
        conn = get_db()
        problem_id = int(request.form['problem_id'])
        solution = request.form['solution']
        author = request.form['author']
        r = requests.post(
            'http://localhost:5001/verify/{}'.format(problem_id),
            {'solution': solution})
        if r.status_code == 404:
            success = False
            message = 'Problem not found'
        else:
            response = r.json()
            success = response['success']
            message = response['message']
            if success:
                query = 'insert into solutions (problem_id, score, content, author) values (?, ?, ?, ?)'
                conn.execute(query, (problem_id, response['score'], solution, author))
                conn.commit()
    return render_template('submit.html', success=success, message=message)

@app.route('/make-zip')
def make_zip():
    conn = get_db()
    query = 'select problem_id, min(score), content from solutions group by problem_id'
    response = make_response()
    with TemporaryFile() as tf, ZipFile(tf, mode='w') as zf:
        for row in conn.execute(query):
            name = 'prob-{:03d}.sol'.format(row['problem_id'])
            zf.writestr(name, row['content'])
        zf.close()
        tf.seek(0)
        response.data = tf.read()
        response.headers['Content-Disposition'] = 'attachment; filename=submission.zip'
        response.mimetype = 'application/zip'
    return response

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
