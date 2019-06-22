import os
import sys
import sqlite3

def main():
    db_file = sys.argv[1]
    legend_file = sys.argv[2]

    with open(legend_file, 'r') as lf:
        directory = os.path.dirname(legend_file)
        db = sqlite3.connect(db_file, detect_types=sqlite3.PARSE_DECLTYPES)
        c = db.cursor()
        query = '''
        insert into problems (id, description, content)
          values (?, ?, ?)
        '''
        for entry in lf:
            name, description = entry.strip().split(' - ', maxsplit=1)
            index = int(name.split('-')[1])
            with open(os.path.join(directory, name + '.desc'), 'r') as cf:
                c.execute(query, (index, description, cf.read()))
        db.commit()

if __name__ == '__main__':
    main()
