# -*- coding: utf-8 -*-

import os
from bs4 import BeautifulSoup

# answer/submit1, answer/submit2, ... に、それぞれのソースコードが計算した結果が入っている
# 1 〜 N まで、隙間なくフォルダが入っている必要がある。
basedir_template = "answer/submit{}"

# 出力ファイルフォーマット。 {:03} の部分に、 0埋めの整数が入ると思えばOK
scorefile_template = "prob-{:03}.desc"

# スコアボードを算出する問題が、1 〜 いくつまでか
problem_to = 300

# BeautifulSoup4 というモジュールが必要です
# $ pip3 install beautifulsoup4


def get_score(score_filepath):

    if os.path.exists(score_filepath):
        with open(score_filepath, 'r') as fin:
            return len(fin.readline().strip())
    else:
        return "-"


def score_list(dir_path):
    scores = []
    for seed in range(1, problem_to + 1):
        scorepath_template = "{}{}" + scorefile_template
        scorefile = scorepath_template.format(dir_path, os.sep, seed)
        scores.append(get_score(scorefile))
    return scores


def print_html(score_table, html_filepath):
    scoreboard_template = """
<!DOCTYPE html>
<html>
<head>
<link rel="stylesheet" type="text/css" href="scoreboard.css">
<title> submit result </title>
</head>
<body>
</body>
</html>
"""

    height = len(score_table)
    width = len(score_table[0])

    soup = BeautifulSoup(scoreboard_template, features='lxml')

    # title
    title = soup.new_tag("h1")
    title.string = "result for each seed"
    title['class'] = "title"
    soup.body.append(title)

    # table
    soup.body.append(soup.new_tag("table"))
    table = soup.body.table
    table["class"] = 'common'
    table.append(soup.new_tag("tr"))

    # add header
    header_list = ["prob id | submit"] + list(map(str, range(1, width + 1)))
    for header in header_list:
        item = soup.new_tag("th")
        item.string = header
        item['class'] = "col_header item"
        table.append(item)

    # add result for each seed
    for r in range(1, height + 1):
        row = soup.new_tag("tr")

        rh = soup.new_tag('td')
        rh.string = str(r)
        rh['class'] = "row_header item"
        row.append(rh)

        for c in range(width):
            item = soup.new_tag("td")
            item.string = str(score_table[r - 1][c])
            item['class'] = "item"
            row.append(item)
        table.append(row)

    with open(html_filepath, 'w') as fout:
        fout.write(str(soup))


def generate_scoreboard(html_filepath):

    iteration = 1

    score_table = []

    while True:
        basedir = basedir_template.format(iteration)
        if os.path.isdir(basedir):
            scores = score_list(basedir)
            score_table.append(scores)
            iteration += 1
        else:
            break

    translated = list(map(list, zip(*score_table)))

    print_html(translated, html_filepath)


if __name__ == "__main__":

    html_filepath = 'scoreboard.html'

    generate_scoreboard(html_filepath)
